#include "kcserver.hxx"
#include "kcliserv.hxx"
#include "kstatus.hxx"
#include "ktoolbar.hxx"
#include "kmenu.hxx"
#include "ksysmets.hxx"
#include "ikterm.h"

const int numButtons = 10;
const int numBitmaps = 7;
ToolBitmapDef tbButtonsCS[] = {
 { { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 }, 0 },
 { { 0, ID_UPLOAD, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 }, "Upload" },
 { { 1, ID_DOWNLOAD, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 }, "Download" },
 { { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 }, 0 },
 { { 2, ID_ACTION_DIRECTORY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 }, "Directory" },
 { { 3, ID_ACTION_CD, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 }, "Change Directory" },
 { { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 }, 0 },
 { { 4, ID_ACTION_FINISH, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 }, "Finish" },
 { { 5, ID_ACTION_BYE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 }, "Bye" },
 { { 6, ID_ACTION_INTERRUPT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 }, "Interrupt" },
};
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KClientServer::KClientServer( K_GLOBAL* kg )
    : KAppWin( kg, IDR_CSMENU, IDR_TOOLBARCS )
{
    client = new KClientClientServ( kg, VCS );
    setClient( client );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KClientServer::~KClientServer()
{
    delete client;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClientServer::getCreateInfo( K_CREATEINFO* info )
{
    info->classname = KWinClassName;
    info->text      = CServerTitle;
    info->style     = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    info->x         = kglob->settings[ CServer_Left ];
    info->y         = kglob->settings[ CServer_Top ];
    info->width     = CW_USEDEFAULT;
    info->height    = 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KClientServer::createWin( KWin* par )
{
    inCreate( TRUE );

    KAppWin::createWin( par );

    menu->createMenu( hInst, this );
    client->createWin( this );

    status->createWin( this );
    int itemp[3] = { 190, 130, 0 };
    status->setParts( 3, itemp );
    status->setText( 0, "one" );
    status->setText( 1, "two" );
    status->setText( 2, "three" );

    toolbar->initButtons( numButtons, numBitmaps, &(*tbButtonsCS) );
    toolbar->createWin( this );

    int cwid, chi;
    client->getSize( cwid, chi );

    int tbwid, tbhi;
    toolbar->getSize( tbwid, tbhi );

    int swid, shi;
    status->getSize( swid, shi );

    int yclient = tbhi - kglob->sysMets->edgeHeight();
    SetWindowPos( client->hwnd(), 0, 0, yclient, 0, 0
        , SWP_NOSIZE | SWP_NOZORDER );

    int newWid = cwid + (2 * kglob->sysMets->sizeframeWidth());
    int newHi = chi + tbhi + shi + (2 * kglob->sysMets->sizeframeHeight())
        + kglob->sysMets->menuHeight()
        + kglob->sysMets->captionHeight();

    inCreate( FALSE );

    SetWindowPos( hWnd, 0, 0, 0, newWid, newHi, SWP_NOMOVE | SWP_NOZORDER );
    
    ShowWindow( hWnd, SW_SHOWNORMAL );
    SetForegroundWindow( hWnd );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KClientServer::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool done = FALSE;
    switch( msg )
    {
        case WM_NOTIFY:
            done = toolbar->message( hwnd, msg, wParam, lParam );
            break;

        case WM_CLOSE:
            show( FALSE );
            done = TRUE;
            break;

        case WM_ACTIVATE:
        case WM_SIZING:
        case WM_EXITSIZEMOVE:
        case WM_GETMINMAXINFO:
        case WM_DRAWITEM:
            done = KAppWin::message( hwnd, msg, wParam, lParam );
            break;

        case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
                case ID_OPTIONS_FONT:
                case ID_OPTIONS_CUSTOMIZESTATUSBAR:
                case ID_HELP_ABOUT:
                case ID_HELP_HELPONKERMIT:
                case ID_WINDOW_TERMINAL:
                case ID_WINDOW_COMMAND:
                case ID_WINDOW_CLIENTSERVER:
                    KAppWin::message( hwnd, msg, wParam, lParam );
                    break;

                case ID_FILE_EXIT:
                    show( FALSE );
                    break;
            }
            done = TRUE;
            break;
        }
    }

    return done;
}

