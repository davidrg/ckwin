#include "kui.hxx"
#include "ktermin.hxx"
#include "kfont.hxx"
#include "ktoolbar.hxx"
#include "khwndset.hxx"
#include "ikcmd.h"
#include "ikextern.h"

#ifndef NOTOOLBAR

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
LRESULT CALLBACK TBComboWndProc( HWND hWnd, UINT umsg, WPARAM wparam, LPARAM lparam )
{
    KToolBar* win = (KToolBar*) kglob->hwndset->find( hWnd );
        if( !win )
            return 0L;

    switch( umsg )
    {    
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        {
            HWND hwndTT = (HWND) SendMessage( win->hwnd(), TB_GETTOOLTIPS, 0, 0 );
            if( hwndTT )
            {
                MSG msg;
                msg.lParam = lparam;
                msg.wParam = wparam;
                msg.message = umsg;
                msg.hwnd = hWnd;
                SendMessage( hwndTT, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msg );
            }
            break;
        }
    }

    return CallWindowProc( win->getComboProc(), hWnd, umsg, wparam, lparam );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KToolBar::KToolBar( K_GLOBAL* kg, int resid )
    : KWin( kg )
    , hWndTermTypeCombo( 0 )
    , hWndFontNameCombo( 0 )
    , hWndFontHeightCombo( 0 )
    , hWndCharsetCombo( 0 )
    , comboFont( 0 )
    , comboProc( 0 )
    , buttonDefs( 0 )
    , numButtons( 0 )
    , numBitmaps( 0 )
    , toolbarid( resid )
    , visible( TRUE )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KToolBar::~KToolBar()
{
    ToolBitmapDef* tbd;
    for( int i = 0; i < numButtons; i++ )
    {
        tbd = &(buttonDefs[i]);
        delete tbd->helptext;
    }
    delete buttonDefs;

    if( comboFont )
        DeleteObject( comboFont );
    if( hWndTermTypeCombo )
        DestroyWindow( hWndTermTypeCombo );
    if ( hWndFontNameCombo )
        DestroyWindow( hWndFontNameCombo );
    if ( hWndFontHeightCombo )
        DestroyWindow( hWndFontHeightCombo );
    if ( hWndCharsetCombo )
        DestroyWindow( hWndCharsetCombo );
    if (!DestroyWindow(hWnd)) {
        printf("DestroyWindow failed: %u\n",GetLastError());
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KToolBar::initButtons( int numbut, int numbit, ToolBitmapDef* def )
{
    numButtons = numbut;
    numBitmaps = numbit;
    buttonDefs = new ToolBitmapDef [numButtons];

    ToolBitmapDef* tbd;
    char* text;
    for( int i = 0; i < numButtons; i++ )
    {
        tbd = &(buttonDefs[i]);
        memset( tbd, '\0', sizeof(ToolBitmapDef) );
        memcpy( &(tbd->tbbutton), &((def[i]).tbbutton), sizeof(TBBUTTON) );
        text = (def[i]).helptext;
        if( text ) {
            size_t len = strlen( text );
            tbd->helptext = new char[len+1];
            memset( tbd->helptext, '\0', len+1 );
            strncpy( tbd->helptext, text, len );
        }
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KToolBar::createWin( KWin* par )
{
#ifndef NOTOOLBAR
    parent = par;

    TBBUTTON* tb = new TBBUTTON [numButtons];
    for( int i = 0; i < numButtons; i++ )
    {
        memcpy( &(tb[i]), &((buttonDefs[i]).tbbutton), sizeof(TBBUTTON) );
    }

    DWORD style = WS_CHILD | WS_BORDER | WS_VISIBLE | WS_CLIPSIBLINGS
                    | TBSTYLE_TOOLTIPS | CCS_TOP;

    // create the toolbar
    //
    hWnd = CreateToolbarEx( parent->hwnd()  // parent
                , style                     // toolbar styles
                , (UINT) ::GetGlobalID()           // toolbar id
                , numBitmaps                // number of bitmaps
                , hInst                     // bitmap instance
                , toolbarid                 // resource id for bitmap
                , (LPCTBBUTTON) tb          // address of buttons
                , numButtons                // number of buttons
                , 16, 16                    // width & height of buttons
                , 16, 16                    // width & height of bitmaps
                , sizeof(TBBUTTON) );       // structure size

    delete tb;
#endif
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
#if defined(_MSC_VER) && _MSC_VER < 1300
char* KToolBar::findBubbleHelp( UINT idx )
#else
char* KToolBar::findBubbleHelp( UINT_PTR idx )
#endif
{
    for( int i = 0; i < numButtons; i++ )
    {
        if( (buttonDefs[i]).tbbutton.idCommand == idx )
            return (buttonDefs[i]).helptext;
    }

    return 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KToolBar::size( int width, int height )
{
    // toolbar will resize itself at top of window
    SendMessage( hWnd, TB_AUTOSIZE, 0L, 0L );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KToolBar::setVisible( Bool visible ) {
    if (visible == this->visible) return;

    if (!visible) {
        // Hide the toolbar
        ShowWindow(hwnd(), SW_HIDE);
        SetParent(hwnd(), NULL);
    } else {
        // Show the toolbar
        ShowWindow(hwnd(), SW_SHOW);
        SetParent(hwnd(), parent->hwnd());
    }

    this->visible = visible;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KToolBar::setCharset( char* cs )
{
    if( hWndCharsetCombo )
        SendMessage( hWndCharsetCombo, CB_SELECTSTRING
                   , (WPARAM)-1, (LPARAM)(LPCSTR)cs );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KToolBar::setTermType( char* tt )
{
    if( hWndTermTypeCombo )
        SendMessage( hWndTermTypeCombo, CB_SELECTSTRING
                   , (WPARAM)-1, (LPARAM)(LPCSTR)tt );
}

/*------------------------------------------------------------------------
 ------------------------------------------------------------------------*/
void KToolBar::setFontName( char* fnt )
{
    if( hWndFontNameCombo )
        SendMessage( hWndFontNameCombo, CB_SELECTSTRING
                   , (WPARAM)-1, (LPARAM)(LPCSTR)fnt );
}

void KToolBar::setFontHeight( int h )
{
    if( hWndFontHeightCombo ) {
        char buf[16];
        ckstrncpy(buf,ckitoa(h/2),16);
        if ( h % 2 )
            ckstrncat(buf,".5",16);
        SendMessage( hWndFontHeightCombo, CB_SELECTSTRING
                   , (WPARAM)-1, (LPARAM)buf );
    }
}
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KToolBar::createTermTypeCombo()
{
    DWORD comboStyle = WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL |
                    CBS_HASSTRINGS | CBS_DROPDOWNLIST;

    RECT rect;
    SendMessage( hWnd, TB_GETITEMRECT, 0, (LPARAM)&rect );


    // create the terminal emulation combobox
    //
    hWndTermTypeCombo = CreateWindowEx( 0L          // no extended styles
                                , "COMBOBOX"                // class name
                                , ""                        // default text
                                , comboStyle                // styles
                                , 80, rect.top              // position
                                , 80, 150                   // dimensions
                                , hWnd                      // parent window
                                , (HMENU) IDC_COMBOTERMINALTYPE // ID
                                , hInst                     // current instance
                                , NULL );                   // no class data

    // create the font used by the combobox...
    // the default font is too big and makes the combo overlap the toolbar
    // in certain screen resolutions
    //
    LOGFONT logfont;
    if( SystemParametersInfo( SPI_GETICONTITLELOGFONT
                              , sizeof( LOGFONT )
                              , &logfont
                              , 0 ) ) {
        comboFont = CreateFontIndirect( &logfont );

        SendMessage( hWndTermTypeCombo, WM_SETFONT
                     , (WPARAM) comboFont, (LPARAM) MAKELONG( (WORD) 1, 0 ) );
    }

    // initialize the combobox with the terminal types
    // definitions from ckermit
    //
    for( int i = 0; i < nttyp; i++ )
    {
        if( !ttyptab[i].flgs )
            SendMessage( hWndTermTypeCombo, CB_ADDSTRING, 0, 
                         (LPARAM)(ttyptab[i].kwd) );
    }

    char* c = ::getVar( TERM_NAME );
    setTermType( c );

    // retrieve the tool tips hand
    //
    HWND hwndTT = (HWND) SendMessage( hWnd, TB_GETTOOLTIPS, 0, 0 );
    if( hwndTT )
    {
        TOOLINFO tinfo;
        memset( &tinfo, '\0', sizeof(TOOLINFO) );
        tinfo.cbSize = sizeof(TOOLINFO);
        tinfo.uFlags = TTF_IDISHWND | TTF_CENTERTIP;
        tinfo.lpszText = (LPSTR) "Terminal Type";
        tinfo.hwnd = hWnd;
#if defined(_MSC_VER) && _MSC_VER < 1300
        tinfo.uId = (UINT) hWndTermTypeCombo;
#else
        tinfo.uId = (UINT_PTR) hWndTermTypeCombo;
#endif
        tinfo.hinst = hInst;

        SendMessage( hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO) &tinfo );
    }

    // subclass the combobox for tool tip stuff
    //
#ifdef _WIN64
    comboProc = (WNDPROC) SetWindowLongPtr(
            hWndTermTypeCombo, GWLP_WNDPROC, (LONG_PTR) TBComboWndProc );
#else /* _WIN64 */
    comboProc = (WNDPROC) SetWindowLong( hWndTermTypeCombo
            , GWL_WNDPROC, (LONG)TBComboWndProc );
#endif /* _WIN64 */
    kglob->hwndset->add( this, hWndTermTypeCombo );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KToolBar::createCharsetCombo()
{
    DWORD comboStyle = WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL |
                    WS_HSCROLL | CBS_HASSTRINGS | CBS_DROPDOWNLIST;

    RECT rect;
    SendMessage( hWnd, TB_GETITEMRECT, 0, (LPARAM)&rect );


    // create the terminal emulation combobox
    //
    hWndCharsetCombo = CreateWindowEx( 0L          // no extended styles
                                , "COMBOBOX"                // class name
                                , ""                        // default text
                                , comboStyle                // styles
                                , 406, rect.top             // position
                                , 110, 150                  // dimensions
                                , hWnd                      // parent window
                                , (HMENU) IDC_COMBOCHARSET // ID
                                , hInst                     // current instance
                                , NULL );                   // no class data

    // create the font used by the combobox...
    // the default font is too big and makes the combo overlap the toolbar
    // in certain screen resolutions
    //
    LOGFONT logfont;
    if( SystemParametersInfo( SPI_GETICONTITLELOGFONT
                              , sizeof( LOGFONT )
                              , &logfont
                              , 0 ) ) {
        comboFont = CreateFontIndirect( &logfont );

        SendMessage( hWndCharsetCombo, WM_SETFONT
                     , (WPARAM) comboFont, (LPARAM) MAKELONG( (WORD) 1, 0 ) );
    }

    // initialize the combobox with the terminal types
    // definitions from ckermit
    //
    for( int i = 0; i < ntxrtab; i++ )
    {
        if( !txrtab[i].flgs &&
            strcmp("transparent",txrtab[i].kwd))
            SendMessage( hWndCharsetCombo, CB_ADDSTRING, 0, 
                         (LPARAM)(txrtab[i].kwd) );
    }

    char* c = ::getVar( TERM_CHAR_REMOTE );
    setCharset( c );

    // retrieve the tool tips hand
    //
    HWND hwndTT = (HWND) SendMessage( hWnd, TB_GETTOOLTIPS, 0, 0 );
    if( hwndTT )
    {
        TOOLINFO tinfo;
        memset( &tinfo, '\0', sizeof(TOOLINFO) );
        tinfo.cbSize = sizeof(TOOLINFO);
        tinfo.uFlags = TTF_IDISHWND | TTF_CENTERTIP;
        tinfo.lpszText = (LPSTR) "SET TERMINAL REMOTE-CHARSET";
        tinfo.hwnd = hWnd;
#if defined(_MSC_VER) && _MSC_VER < 1300
        tinfo.uId = (UINT) hWndCharsetCombo;
#else
        tinfo.uId = (UINT_PTR) hWndCharsetCombo;
#endif
        tinfo.hinst = hInst;

        SendMessage( hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO) &tinfo );
    }

    // subclass the combobox for tool tip stuff
    //
#ifdef _WIN64
    comboProc = (WNDPROC) SetWindowLongPtr(
            hWndCharsetCombo, GWLP_WNDPROC, (LONG_PTR) TBComboWndProc );
#else /* _WIN64 */
    comboProc = (WNDPROC) SetWindowLong( hWndCharsetCombo
            , GWL_WNDPROC, (LONG)TBComboWndProc );
#endif /* _WIN64 */
    kglob->hwndset->add( this, hWndCharsetCombo );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KToolBar::createFontNameCombo()
{
    DWORD comboStyle = WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL |
                    WS_HSCROLL | CBS_HASSTRINGS | CBS_DROPDOWNLIST;

    RECT rect;
    SendMessage( hWnd, TB_GETITEMRECT, 0, (LPARAM)&rect );


    // create the terminal emulation combobox
    //
    hWndFontNameCombo = CreateWindowEx( 0L          // no extended styles
                                , "COMBOBOX"                // class name
                                , ""                        // default text
                                , comboStyle                // styles
                                , 176, rect.top             // position
                                , 160, 150                  // dimensions
                                , hWnd                      // parent window
                                , (HMENU) IDC_COMBOFONTNAME // ID
                                , hInst                     // current instance
                                , NULL );                   // no class data

    // create the font used by the combobox...
    // the default font is too big and makes the combo overlap the toolbar
    // in certain screen resolutions
    //
    LOGFONT logfont;
    if( SystemParametersInfo( SPI_GETICONTITLELOGFONT
                              , sizeof( LOGFONT )
                              , &logfont
                              , 0 ) ) {
        comboFont = CreateFontIndirect( &logfont );

        SendMessage( hWndFontNameCombo, WM_SETFONT
                     , (WPARAM) comboFont, (LPARAM) MAKELONG( (WORD) 1, 0 ) );
    }

    // initialize the combobox with the available font types
    //
    if (ntermfont == 0)
        BuildFontTable(&term_font, &_term_font, &ntermfont);
    for( int i = 0; i < ntermfont; i++ )
    {
        if( !term_font[i].flgs )
            SendMessage( hWndFontNameCombo, CB_ADDSTRING, 0, 
                         (LPARAM)(strdup(term_font[i].kwd)) );
    }

    char* c = ::getVar( TERM_FONT );
    setFontName( c );

    // retrieve the tool tips hand
    //
    HWND hwndTT = (HWND) SendMessage( hWnd, TB_GETTOOLTIPS, 0, 0 );
    if( hwndTT )
    {
        TOOLINFO tinfo;
        memset( &tinfo, '\0', sizeof(TOOLINFO) );
        tinfo.cbSize = sizeof(TOOLINFO);
        tinfo.uFlags = TTF_IDISHWND | TTF_CENTERTIP;
        tinfo.lpszText = (LPSTR) "Font Name";
        tinfo.hwnd = hWnd;
#if defined(_MSC_VER) && _MSC_VER < 1300
        tinfo.uId = (UINT) hWndFontNameCombo;
#else
        tinfo.uId = (UINT_PTR) hWndFontNameCombo;
#endif
        tinfo.hinst = hInst;

        SendMessage( hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO) &tinfo );
    }

    // subclass the combobox for tool tip stuff
    //
#ifdef _WIN64
    comboProc = (WNDPROC) SetWindowLongPtr(
            hWndFontNameCombo, GWLP_WNDPROC, (LONG_PTR) TBComboWndProc );
#else /* _WIN64 */
    comboProc = (WNDPROC) SetWindowLong( hWndFontNameCombo
            , GWL_WNDPROC, (LONG)TBComboWndProc );
#endif /* _WIN64 */
    kglob->hwndset->add( this, hWndFontNameCombo );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KToolBar::createFontHeightCombo()
{
    DWORD comboStyle = WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL |
                    CBS_HASSTRINGS | CBS_DROPDOWNLIST | ES_NUMBER;

    RECT rect;
    SendMessage( hWnd, TB_GETITEMRECT, 0, (LPARAM)&rect );


    // create the terminal emulation combobox
    //
    hWndFontHeightCombo = CreateWindowEx( 0L          // no extended styles
                                , "COMBOBOX"                // class name
                                , ""                        // default text
                                , comboStyle                // styles
                                , 346, rect.top         // position
                                , 50, 150                  // dimensions
                                , hWnd                      // parent window
                                , (HMENU) IDC_COMBOFONTHEIGHT // ID
                                , hInst                     // current instance
                                , NULL );                   // no class data

    // create the font used by the combobox...
    // the default font is too big and makes the combo overlap the toolbar
    // in certain screen resolutions
    //
    LOGFONT logfont;
    if( SystemParametersInfo( SPI_GETICONTITLELOGFONT
                              , sizeof( LOGFONT )
                              , &logfont
                              , 0 ) ) {
        comboFont = CreateFontIndirect( &logfont );

        SendMessage( hWndFontHeightCombo, WM_SETFONT
                     , (WPARAM) comboFont, (LPARAM) MAKELONG( (WORD) 1, 0 ) );
    }

    // initialize the combobox with the standard font sizes
    //
    for( int i = 1; i < 48; i++ )
    {
        char buf[16];
        ckstrncpy(buf,ckitoa(i),16);
        SendMessage( hWndFontHeightCombo, CB_ADDSTRING, 0, 
                     (LPARAM)buf );
        ckstrncat(buf,".5",16);
        SendMessage( hWndFontHeightCombo, CB_ADDSTRING, 0, 
                     (LPARAM)buf );
    }

    char* c = ::getVar( TERM_FONT_SIZE );       /* returns half-points */
    if ( c )
        setFontHeight( atoi(c) );

    // retrieve the tool tips hand
    //
    HWND hwndTT = (HWND) SendMessage( hWnd, TB_GETTOOLTIPS, 0, 0 );
    if( hwndTT )
    {
        TOOLINFO tinfo;
        memset( &tinfo, '\0', sizeof(TOOLINFO) );
        tinfo.cbSize = sizeof(TOOLINFO);
        tinfo.uFlags = TTF_IDISHWND | TTF_CENTERTIP;
        tinfo.lpszText = (LPSTR) "Font Height (points)";
        tinfo.hwnd = hWnd;
#if defined(_MSC_VER) && _MSC_VER < 1300
        tinfo.uId = (UINT) hWndFontHeightCombo;
#else
        tinfo.uId = (UINT_PTR) hWndFontHeightCombo;
#endif
        tinfo.hinst = hInst;

        SendMessage( hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO) &tinfo );
    }

    // subclass the combobox for tool tip stuff
    //
#ifdef _WIN64
    comboProc = (WNDPROC) SetWindowLongPtr(
            hWndFontHeightCombo, GWLP_WNDPROC, (LONG_PTR) TBComboWndProc );
#else /* _WIN64 */
    comboProc = (WNDPROC) SetWindowLong( hWndFontHeightCombo
            , GWL_WNDPROC, (LONG)TBComboWndProc );
#endif /* _WIN64 */
    kglob->hwndset->add( this, hWndFontHeightCombo );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KToolBar::message( HWND hpar, UINT msg, WPARAM wParam, LPARAM lParam )
{
    Bool done = FALSE;
    switch( msg )
    {
        case WM_NOTIFY:
        {
            switch( ((LPNMHDR) lParam)->code )
            {
                case TTN_NEEDTEXT:
                {
                    // Display the ToolTip text.
                    LPTOOLTIPTEXT tip = (LPTOOLTIPTEXT)lParam;
                    tip->lpszText = findBubbleHelp( tip->hdr.idFrom );
                    done = TRUE;
                    break;
                }

                default:
                    break;
            }
            break;
        }
        case WM_COMMAND:
        {
            switch (HIWORD(wParam))
            {
            case CBN_SELCHANGE:
                if ( (HWND)lParam == hWndTermTypeCombo ) {
                    // user is changing the terminal type
                    //
                    LRESULT cursel = SendMessage( hWndTermTypeCombo, CB_GETCURSEL, 0, 0 );
                    if( cursel >= 0 ) {
                        long len = (long)SendMessage( hWndTermTypeCombo, CB_GETLBTEXTLEN, cursel, 0 );
                        char* tmp = new char[len+1];
                        SendMessage( hWndTermTypeCombo, CB_GETLBTEXT, cursel, (LPARAM)tmp );

                        for ( int i=0; i<nttyp; i++ ) {
                            if ( !strcmp(tmp,ttyptab[i].kwd) ) {
                                settermtype(ttyptab[i].kwval,1);
#ifdef TNCODE
                                /* So we send the correct terminal name to the host if it asks for it */
                                ttnum = -1;	   /* Last Telnet Terminal Type sent */
                                ttnumend = 0;  /* end of list not found */
#endif /* TNCODE */
                                break;
                            }
                        }
                        delete tmp;
                    }
                } else if ( (HWND)lParam == hWndCharsetCombo ) {
                    // user is changing the terminal type
                    //
                    LRESULT cursel = SendMessage( hWndCharsetCombo, CB_GETCURSEL, 0, 0 );
                    if( cursel >= 0 ) {
                        long len = (long)SendMessage( hWndCharsetCombo, CB_GETLBTEXTLEN, cursel, 0 );
                        char* tmp = new char[len+1];
                        SendMessage( hWndCharsetCombo, CB_GETLBTEXT, cursel, (LPARAM)tmp );

                        for ( int i=0; i<ntxrtab; i++ ) {
                            if ( !strcmp(tmp,txrtab[i].kwd) ) {
                                setremcharset(txrtab[i].kwval, 4 /* TT_GR_ALL */);
                                break;
                            }
                        }
                        delete tmp;
                    }
                } else if ( (HWND)lParam == hWndFontNameCombo ) {
                    // user is changing the terminal font
                    //
                    LRESULT cursel = SendMessage( hWndFontNameCombo, CB_GETCURSEL, 0, 0 );
                    if( cursel >= 0 ) {
                        long len = (long)SendMessage( hWndFontNameCombo, CB_GETLBTEXTLEN, cursel, 0 );
                        char* tmp = new char[len+1];
                        SendMessage( hWndFontNameCombo, CB_GETLBTEXT, cursel, (LPARAM)tmp );

                        for ( int i=0; i<ntermfont; i++ ) {
                            if ( !strcmp(tmp,term_font[i].kwd) ) {
                                tt_font = term_font[i].kwval;
                            }
                        }

                        kui->getTerminal()->setKFont(new KFont(tmp, kui->getTerminal()->getKFont()->getFontPointsH()));
                        delete tmp;
                    }
                } else if ( (HWND)lParam == hWndFontHeightCombo ) {
                    // user is changing the terminal font
                    //
                    LRESULT cursel = SendMessage( hWndFontHeightCombo, CB_GETCURSEL, 0, 0 );
                    if( cursel >= 0 ) {
                        long len = (long)SendMessage( hWndFontHeightCombo, CB_GETLBTEXTLEN, cursel, 0 );
                        char* tmp = new char[len+1], *p;
                        int halfpoint = 0;
                        SendMessage( hWndFontHeightCombo, CB_GETLBTEXT, cursel, (LPARAM)tmp );

                        for ( p=tmp; *p; p++) {
                            if ( *p == '.' ) {
                                halfpoint = 1;
                                *p = NUL;
                                break;
                            }
                        }
                        tt_font_size = 2 * atoi(tmp) + halfpoint;
                        kui->getTerminal()->setKFont(new KFont(kui->getTerminal()->getKFont()->getFaceName(),
                                                               tt_font_size));
                        delete tmp;
                    }
                }
                break;
            case CBN_SELENDOK:
            case CBN_SELENDCANCEL:
                kui->getTerminal()->takeFocus();
                break;
            }
            break;
        }
        default:
            break;
    }

    return done;
}

#endif
