#include "kui.hxx"
#include "ktermin.hxx"
#include "kflstat.hxx"
#include "khwndset.hxx"
#include "kmenu.hxx"
#include "kflstat.h"
#include "ikui.h"
#include "ikcmd.h"
#include "ikextern.h"

KFileStatus* filestatus = 0;    // global file status

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
extern "C" {
LRESULT CALLBACK KWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    Bool ret = kui->message( hwnd, msg, wParam, lParam );
    if( !ret )
        return DefWindowProc( hwnd, msg, wParam, lParam );
    return 1L;
}
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Kui::Kui( HINSTANCE hInstance )
    : terminal( 0 )
    , dialogHwnd( 0 )
{
    // initialize the win95 common controls
    //
    InitCommonControls();
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Kui::~Kui()
{
    delete terminal;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
int Kui::init()
{
    WNDCLASS wc;
    memset( &wc, '\0', sizeof(WNDCLASS) );

    wc.style            = CS_DBLCLKS; // | CS_SAVEBITS | CS_BYTEALIGNWINDOW;
    wc.lpszClassName    = KWinClassName;
    wc.hInstance        = kglob->hInst;
    wc.lpfnWndProc      = (WNDPROC) KWndProc;
    wc.hCursor          = LoadCursor( NULL, IDC_ARROW );
    wc.hIcon            = LoadIcon( kglob->hInst, MAKEINTRESOURCE(IDI_ICONK95) );
    wc.lpszMenuName     = NULL;
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;

    RegisterClass( &wc );

    terminal = new KTerminal( kglob );
    terminal->createWin( 0 );
    terminal->show();
    terminal->takeFocus();
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void Kui::cleanup()
{
    DWORD id = GetWindowThreadProcessId( terminal->hwnd(), NULL );
    PostThreadMessage( id, WM_CLOSE_KERMIT, 0, 0 );
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void Kui::start()
{
    MSG msg;
    while( GetMessage( (LPMSG) &msg, 0, 0, 0 ) != FALSE )
    {
        if( !dialogHwnd || !IsDialogMessage( dialogHwnd, (LPMSG) &msg ) ) {
            switch ( msg.message ) {
            case WM_KEYDOWN:
            case WM_KEYUP:
                break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
                {
#ifndef COMMENT
                con_event evt = mapkey(msg.wParam | KEY_ALT);
                if ( os2gks && (evt.type == error ) )
#endif /* COMMENT */
                    TranslateMessage( (LPMSG) &msg );
                }
                break;
            default:
                TranslateMessage( (LPMSG) &msg );
            }
            DispatchMessage( (LPMSG) &msg );
        }
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void Kui::savePositions()
{
    int x, y;

    terminal->getPos( x, y );
    kglob->settings[ Terminal_Left ] = x;
    kglob->settings[ Terminal_Top ]  = y;
    terminal->show( FALSE );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void Kui::setProperty( int propid, long param1, long param2 )
{
    switch( propid )
    {
    case KUI_TITLE: {
        strcpy( kglob->hostName, (char*)param1 );
        char c[256];
        if( terminal ) {
            strcpy( c, kglob->hostName );
#ifdef COMMENT
            if( *kglob->hostName != '\0' )
                strcat( c, " - " );
            strcat( c, TermTitle );
#endif /* COMMENT */
            SetWindowText( terminal->hwnd(), c );
        }
        break;
    }

    case KUI_FILE_TRANSFER: {
        if( terminal ) {
            switch( param1 )
            {
            case CW_NAM:   /* Filename */
            case CW_TYP:   /* File type */
            case CW_TR:    /* Time remaining */
            case CW_WS:    /* Window slots */
            case CW_PT:    /* Packet type */
            case CW_DIR:   /* Current directory */
            case CW_ERR:   /* Error message */
            case CW_MSG: { /* Info message */
                char* c = newstr( (char*)param2 );
                param2 = (long) c;
                break;
            }
            }
            PostMessage( terminal->hwnd(), WM_GOTO_FILETRANSFER_WINDOW,
                         (WPARAM)param1, (LPARAM)param2 );
        }   
        break;
    }

    case KUI_TERM_TYPE:
        if( terminal ) {
            for ( int i=0; i<nttyp; i++ ) {
                if ( ttyptab[i].kwval == param1 && !ttyptab[i].flgs) {
                    terminal->setTermType( ttyptab[i].kwd );
                    break;
                }
            }
        }
        break;

    case KUI_TERM_REMCHARSET:
        if( terminal ) {
            for ( int i=0; i<ntxrtab; i++ ) {
                if ( txrtab[i].kwval == param1 )
                    terminal->setRemoteCharset( txrtab[i].kwd );
            }
        }
        break;

    case KUI_TERM_FONT:
        if( terminal ) {
            for ( int i=0; i<ntermfont; i++ ) {
                if ( term_font[i].kwval == param1 ) {
                    KFont * kfont = new KFont(term_font[i].kwd,param2);
                    terminal->setKFont(kfont);
                    terminal->setFont( term_font[i].kwd, 
                                       kfont->getFontPointsH());
                }
            }
        }
        break;

    case KUI_TERM_CMASK:
        if ( terminal ) {
            KMenu * menu = terminal->getMenu();
            if ( !menu )
                break;
            if ( param1 == 7 ) {
                menu->setCheck(ID_ACTION_7BIT);
                menu->unsetCheck(ID_ACTION_8BIT);
            } else if ( param1 == 8 ) {
                menu->unsetCheck(ID_ACTION_7BIT);
                menu->setCheck(ID_ACTION_8BIT);
            }
        }
        break;

    case KUI_TERM_AUTODOWNLOAD:
        if ( terminal ) {
            KMenu * menu = terminal->getMenu();
            if ( !menu )
                break;
#ifdef COMMENT
            if ( param1 )
                menu->setCheck(ID_ACTION_AUTODOWNLOAD);
            else
                menu->unsetCheck(ID_ACTION_AUTODOWNLOAD);
#else /* COMMENT */
            if ( param1 == 1 ) {
                menu->setCheck(ID_ACTION_AUTODOWNLOAD_ON);
                menu->unsetCheck(ID_ACTION_AUTODOWNLOAD_OFF);
                menu->unsetCheck(ID_ACTION_AUTODOWNLOAD_ASK);
            } else if ( param1 == 2 ){
                menu->unsetCheck(ID_ACTION_AUTODOWNLOAD_ON);
                menu->unsetCheck(ID_ACTION_AUTODOWNLOAD_OFF);
                menu->setCheck(ID_ACTION_AUTODOWNLOAD_ASK);
            } else {
                menu->unsetCheck(ID_ACTION_AUTODOWNLOAD_ON);
                menu->unsetCheck(ID_ACTION_AUTODOWNLOAD_OFF);
                menu->setCheck(ID_ACTION_AUTODOWNLOAD_ASK);
            }
#endif /* COMMENT */
        }
        break;

    case KUI_TERM_CAPTURE:
        if ( terminal ) {
            KMenu * menu = terminal->getMenu();
            if ( !menu )
                break;
            if ( param1 )
                menu->setCheck(ID_ACTION_CAPTURE);
            else
                menu->unsetCheck(ID_ACTION_CAPTURE);
        }
        break;

    case KUI_TERM_PRINTERCOPY:
        if ( terminal ) {
            KMenu * menu = terminal->getMenu();
            if ( !menu )
                break;
            if ( param1 )
                menu->setCheck(ID_ACTION_PRINTERCOPY);
            else
                menu->unsetCheck(ID_ACTION_PRINTERCOPY);
        }
        break;

    case KUI_TERM_DEBUG:
        if ( terminal ) {
            KMenu * menu = terminal->getMenu();
            if ( !menu )
                break;
            if ( param1 )
                menu->setCheck(ID_ACTION_DEBUG);
            else
                menu->unsetCheck(ID_ACTION_DEBUG);
        }
        break;

    case KUI_TERM_PCTERM:
        if ( terminal ) {
            KMenu * menu = terminal->getMenu();
            if ( !menu )
                break;
            if ( param1 )
                menu->setCheck(ID_ACTION_PCTERM);
            else
                menu->unsetCheck(ID_ACTION_PCTERM);
        }
        break;

    case KUI_TERM_KEYCLICK:
        if ( terminal ) {
            KMenu * menu = terminal->getMenu();
            if ( !menu )
                break;
            if ( param1 )
                menu->setCheck(ID_ACTION_KEYCLICK);
            else
                menu->unsetCheck(ID_ACTION_KEYCLICK);
        }
        break;

    case KUI_TERM_RESIZE:
        if ( terminal ) {
            KMenu * menu = terminal->getMenu();
            if ( !menu )
                break;
            if ( param1 == 0 ) {
                menu->unsetCheck(ID_ACTION_RESIZE_FONT);
                menu->unsetCheck(ID_ACTION_RESIZE_DIMENSION);
            } else if ( param1 == 1 ) {
                menu->setCheck(ID_ACTION_RESIZE_FONT);
                menu->unsetCheck(ID_ACTION_RESIZE_DIMENSION);
            } else if ( param1 == 2 ){
                menu->unsetCheck(ID_ACTION_RESIZE_FONT);
                menu->setCheck(ID_ACTION_RESIZE_DIMENSION);
            }
        }
        break;

    case KUI_TERM_URL_HIGHLIGHT:
        if ( terminal ) {
            KMenu * menu = terminal->getMenu();
            if ( !menu )
                break;
            if ( param1 ) {
                menu->setCheck(ID_ACTION_URL_HIGHLIGHT);
            } else {
                menu->unsetCheck(ID_ACTION_URL_HIGHLIGHT);
            }
        }
        break;

    case KUI_AUTO_LOCUS:
        if ( terminal ) {
            KMenu * menu = terminal->getMenu();
            if ( !menu )
                break;
            if ( param1 == 1 ) {
                menu->setCheck(ID_ACTION_AUTO_LOCUS_ON);
                menu->unsetCheck(ID_ACTION_AUTO_LOCUS_OFF);
                menu->unsetCheck(ID_ACTION_AUTO_LOCUS_ASK);
            } else if ( param1 == 2 ){
                menu->unsetCheck(ID_ACTION_AUTO_LOCUS_ON);
                menu->unsetCheck(ID_ACTION_AUTO_LOCUS_OFF);
                menu->setCheck(ID_ACTION_AUTO_LOCUS_ASK);
            } else {
                menu->unsetCheck(ID_ACTION_AUTO_LOCUS_ON);
                menu->unsetCheck(ID_ACTION_AUTO_LOCUS_OFF);
                menu->setCheck(ID_ACTION_AUTO_LOCUS_ASK);
            }
        }
        break;

    case KUI_EXIT_WARNING:
        if ( terminal ) {
            KMenu * menu = terminal->getMenu();
            if ( !menu )
                break;
            if ( param1 == 1 ) {
                menu->setCheck(ID_ACTION_WARNING_ON);
                menu->unsetCheck(ID_ACTION_WARNING_OFF);
                menu->unsetCheck(ID_ACTION_WARNING_ALWAYS);
            } else if ( param1 == 2 ){
                menu->unsetCheck(ID_ACTION_WARNING_ON);
                menu->unsetCheck(ID_ACTION_WARNING_OFF);
                menu->setCheck(ID_ACTION_WARNING_ALWAYS);
            } else {
                menu->unsetCheck(ID_ACTION_WARNING_ON);
                menu->unsetCheck(ID_ACTION_WARNING_OFF);
                menu->setCheck(ID_ACTION_WARNING_ALWAYS);
            }
        }
        break;

    case KUI_GUI_DIALOGS:
        if ( terminal ) {
            KMenu * menu = terminal->getMenu();
            if ( !menu )
                break;
            if ( param1 )
                menu->setCheck(ID_ACTION_GUI_DIALOGS);
            else
                menu->unsetCheck(ID_ACTION_GUI_DIALOGS);
        }
        break;

    case KUI_GUI_CLOSE:
        if ( terminal ) {
            if ( !param1 )
                terminal->disableClose();  // need to disable close
        }
        break;

    case KUI_GUI_TOOLBAR:
        if ( terminal ) {
            if ( !param1 )
                terminal->disableToolbar();  // need to disable toolbar
        }
        break;

    case KUI_GUI_MENUBAR:
        if ( terminal ) {
            if ( !param1 )
                terminal->disableMenu();    // need to disable menu
        }
        break;

    case KUI_LOCUS:
        if ( terminal ) {
            KMenu * menu = terminal->getMenu();
            KToolBar * toolbar = terminal->getToolbar();
            if ( !menu && !toolbar )
                break;
            if ( param1) {
                terminal->setStatusText(STATUS_LOCUS,"Locus: Local");
                if ( menu )
                    menu->setCheck(ID_ACTION_LOCUS);
                if ( toolbar )
                    SendMessage(toolbar->hwnd(),
                             TB_CHECKBUTTON, ID_ACTION_LOCUS, 
                             MAKELONG(1,0));
            } else {
                terminal->setStatusText(STATUS_LOCUS,"Locus: Remote");
                if ( menu )
                    menu->unsetCheck(ID_ACTION_LOCUS);
                if ( toolbar )
                    SendMessage(toolbar->hwnd(),
                             TB_CHECKBUTTON, ID_ACTION_LOCUS, 
                             MAKELONG(0,0));
            }
        }
        break;

    default:
        break;
    }

}
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void Kui::getProperty( int propid, long param1, long param2 )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool Kui::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool done = FALSE;
    KWin* win = kglob->hwndset->find( hwnd );
    if( !win )
        return done;
        
    switch( msg )
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            if (BeginPaint( hwnd, &ps ))
                win->paint();
            EndPaint( hwnd, &ps );
            done = TRUE;
            break;
        }
    case WM_ERASEBKGND:
            done = TRUE;
            break;

    case WM_REQUEST_CLOSE_KERMIT:
        {
            // close down ckermit
            Bool hangupOK = hupok(0);
            if( hangupOK ) {
                if( terminal )
                    terminal->show( FALSE );
                doexit(GOOD_EXIT,xitsta);
            }
            break;
        }

    case WM_CLOSE_KERMIT:
        delete this;
        PostQuitMessage(0);
        break;


    case WM_GOTO_TERMINAL_WINDOW:
        terminal->show();
        terminal->takeFocus();
        break;

    case WM_GOTO_FILETRANSFER_WINDOW:
        {
            if( !filestatus && terminal ) {
                // repost the first message that triggered the create
                // because it gets discarded for some reason...
                // probably DialogBox() in KFileStatus
                //
                PostMessage( terminal->hwnd(), WM_GOTO_FILETRANSFER_WINDOW
                        , wParam, lParam );

                terminal->show();
                filestatus = new KFileStatus( kglob );
                filestatus->createWin( terminal );
                filestatus->show();
                break;
            }

            filestatus->setProperty( wParam, lParam );

            switch( wParam )
            {
                case CW_NAM:   /* Filename */
                case CW_TYP:   /* File type */
                case CW_TR:    /* Time remaining */
                case CW_WS:    /* Window slots */
                case CW_PT:    /* Packet type */
                case CW_DIR:   /* Current directory */
                case CW_ERR:   /* Error message */
                case CW_MSG:   /* Info message */
                {
                    char* c = (char*)lParam;
                    delete c;
                    break;
                }
            }
            break;
        }    

        default:
            done = win->message( hwnd, msg, wParam, lParam );
            break;
    }

    return done;
}
