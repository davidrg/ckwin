#include "kappwin.hxx"
#include "kmenu.hxx"
#include "ktoolbar.hxx"
#include "kstatus.hxx"
#include "kclient.hxx"
#include "kfontdlg.hxx"
#include "kabout.hxx"
#include "kfont.hxx"
#include "ksysmets.hxx"
#include "kszpopup.hxx"
#include "ikterm.h"
#include "ikcmd.h"
#include <stdlib.h>
#include <process.h>
#include <sys/stat.h>
extern "C" {
    extern int vmode;
    extern char exedir[];
    extern int  tt_status[];
};

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KAppWin::KAppWin( K_GLOBAL* kg, int menuid, int toolbarid )
    : KWin( kg )
    , sizepop( 0 )
    , menu(0)
    , toolbar(0)
    , status(0)
    , menuId(menuid)
{
    if ( !kg->noMenuBar )
        createMenu();
    if ( !kg->noToolBar )
        toolbar = new KToolBar( kg, toolbarid );
    if ( !kg->noStatus )
        status = new KStatus( kg );
    client = 0;
    wmSize = SIZE_RESTORED;
}

void KAppWin::createMenu(void)
{
    menu = new KMenu( menuId );
}

void KAppWin::destroyMenu(void)
{
    delete menu;
    menu = 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KAppWin::~KAppWin()
{
    delete sizepop;
    delete status;
    delete toolbar;
    delete menu;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KAppWin::createWin( KWin* par )
{
    KWin::createWin( par );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KAppWin::takeFocus()
{
    if( client )
        client->takeFocus();
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KAppWin::size( int width, int height )
{
    if( inCreate() )
        return;

    if ( toolbar )
        toolbar->size( width, height );
    if ( status )
        status->size( width, height );

    if( client ) {
        int x, y, w, h;
        getClientCoord( x, y, w, h );
        client->size( w, h );
    }
}

/*------------------------------------------------------------------------
    determine the coordinates of the client area
------------------------------------------------------------------------*/
void KAppWin::getClientCoord( int& x, int& y, int& w, int& h )
{
    int tbh = 0, tbw = 0;
    if ( toolbar )
        toolbar->getSize( tbw, tbh );

    int sth = 0, stw = 0;
    if ( status )
        status->getSize( stw, sth );

    RECT rect;
    GetClientRect( hWnd, &rect );
    x = 0;
    y = tbh - kglob->sysMets->edgeHeight();
    w = rect.right;
    h = rect.bottom - tbh - sth + kglob->sysMets->edgeHeight();
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KAppWin::sizing( UINT fwSide, LPRECT lpr )
{
    switch( kglob->mouseEffect )
    {
        case TERM_MOUSE_CHANGE_FONT:        // change the font
            sizeFontSetDim( fwSide, lpr );
            break;

        case TERM_MOUSE_NO_EFFECT:          // limit the size
            sizeFixed( fwSide, lpr );
            break;

        case TERM_MOUSE_CHANGE_DIMENSION:   // let the window size freely
        default:
        {
            if( !sizepop ) {
                sizepop = new KSizePopup( kglob );
                sizepop->createWin( this );
            }
            int w, h;
            client->getEndSize( w, h );
            if ( tt_status[vmode] )
                h--;
            sizepop->paint( w, h );
            break;
        }
    }
    return TRUE;
}

int KAppWin::sizeFont( LPRECT lpr, int force )
{
    int diffw = 0, diffh = 0;
    RECT rect;
    if ( IsZoomed(hWnd) ) {
        GetWindowRect(GetDesktopWindow(), &rect);
    } else {
        GetWindowRect(hWnd, &rect);
    }

    if ( !force ) {
        int parw = rect.right - rect.left;
        int parh = rect.bottom - rect.top;

        diffw = (lpr->right - lpr->left) - parw;
        diffh = (lpr->bottom - lpr->top) - parh;

        if (diffw == 0 && diffh == 0)
            return 0;
    }
    
    int cliw, clih;
    client->getSize( cliw, clih );
    cliw += diffw;
    clih += diffh;

    int clientWidth = 0, clientHeight = 0;
    ::getDimensions( vmode /* client->getClientID() */, 
                     &clientWidth, &clientHeight );

    int wishFontW = (cliw / clientWidth) - (cliw % clientWidth ? 0 : 1); 
    int wishFontH = (int)(clih / clientHeight / tt_linespacing[vmode]) - (clih % clientHeight ? 0 : 1);

    if( wishFontW == 0 )
        wishFontW = -1;
    if( wishFontH == 0 )
        wishFontH = -1;

    KFont* kfont = client->getFont();

    if( !kfont->tryFont( wishFontW, wishFontH, client->hdc() ) ) {
        memcpy( lpr, &rect, sizeof(RECT) );
        return 0;
    }

    client->setInterSpacing( kfont );
    setFont( kfont->getFaceName(), kfont->getFontPointsH() );

    return 1;
}

/*------------------------------------------------------------------------
    size the panel according to the font
------------------------------------------------------------------------*/
void KAppWin::sizeFontSetDim( UINT fwSide, LPRECT lpr )
{
    if (!sizeFont(lpr,0))
        return;

    // pass in FALSE to prevent the client from sizing the parent
    //
    client->setDimensions( FALSE );

    int maxw = 0, maxh = 0;
    client->getMaxpDim( maxw, maxh );

    switch( fwSide )
    {
        case WMSZ_BOTTOM:       // Bottom edge
        case WMSZ_BOTTOMRIGHT:  // Bottom-right corner
        case WMSZ_RIGHT:        // Right edge
            lpr->right = lpr->left + maxw;
            lpr->bottom = lpr->top + maxh;
            break;

        case WMSZ_TOPRIGHT:     // Top-right corner
        case WMSZ_TOP:          // Top edge
            lpr->right = lpr->left + maxw;
            lpr->top = lpr->bottom - maxh;
            break;

        case WMSZ_TOPLEFT:      // Top-left corner
            lpr->left = lpr->right - maxw;
            lpr->top = lpr->bottom - maxh;
            break;

        case WMSZ_BOTTOMLEFT:   // Bottom-left corner
        case WMSZ_LEFT:         // Left edge
            lpr->left = lpr->right - maxw;
            lpr->bottom = lpr->top + maxh;
            break;
    }

    SetWindowPos( hWnd, 0, lpr->left, lpr->top, 
                  lpr->right - lpr->left, lpr->bottom - lpr->top, 
                  SWP_NOZORDER );
    client->paint();
}



/*------------------------------------------------------------------------
    prevent the user from sizing beyond the limits
------------------------------------------------------------------------*/
void KAppWin::sizeFixed( UINT fwSide, LPRECT lpr )
{
    int maxw = 0, maxh = 0;
    if( !client || !client->getMaxpDim( maxw, maxh ) )
        return;

    RECT rect;
    if ( IsZoomed(hWnd) ) {
        GetWindowRect(GetDesktopWindow(), &rect);
    } else {
        GetWindowRect(hWnd, &rect);
    }

    int w = lpr->right - lpr->left;
    int h = lpr->bottom - lpr->top;

    switch( fwSide )
    {
        case WMSZ_BOTTOM:       // Bottom edge
        case WMSZ_BOTTOMRIGHT:  // Bottom-right corner
        case WMSZ_RIGHT:        // Right edge
            if( w != maxw ) {
                lpr->left = rect.left;
                lpr->right = lpr->left + maxw;
            }
            if( h != maxh ) {
                lpr->top = rect.top;
                lpr->bottom = lpr->top + maxh;
            }
            break;

        case WMSZ_TOPRIGHT:     // Top-right corner
            if( w != maxw ) {
                lpr->left = rect.left;
                lpr->right = lpr->left + maxw;
            }
            if( h != maxh ) {
                lpr->bottom = rect.bottom;
                lpr->top = lpr->bottom - maxh;
            }
            break;

        case WMSZ_BOTTOMLEFT:   // Bottom-left corner
            if( w != maxw ) {
                lpr->right = rect.right;
                lpr->left = lpr->right - maxw;
            }
            if( h != maxh ) {
                lpr->top = rect.top;
                lpr->bottom = lpr->top + maxh;
            }
            break;

        case WMSZ_TOP:          // Top edge
        case WMSZ_TOPLEFT:      // Top-left corner
        case WMSZ_LEFT:         // Left edge
            if( w != maxw ) {
                lpr->right = rect.right;
                lpr->left = lpr->right - maxw;
            }
            if( h != maxh ) {
                lpr->bottom = rect.bottom;
                lpr->top = lpr->bottom - maxh;
            }
            break;
    }
}

/*------------------------------------------------------------------------
    prevent the user from sizing beyond the limits
------------------------------------------------------------------------*/
void KAppWin::sizeLimit( UINT fwSide, LPRECT lpr )
{
    int maxw = 0, maxh = 0;
    if( !client || !client->getMaxpDim( maxw, maxh ) )
        return;

    RECT rect;
    if ( IsZoomed(hWnd) ) {
        GetWindowRect(GetDesktopWindow(), &rect);
    } else {
        GetWindowRect(hWnd, &rect);
    }

    int w = lpr->right - lpr->left;
    int h = lpr->bottom - lpr->top;

    switch( fwSide )
    {
        case WMSZ_BOTTOM:       // Bottom edge
        case WMSZ_BOTTOMRIGHT:  // Bottom-right corner
        case WMSZ_RIGHT:        // Right edge
            if( w > maxw ) {
                lpr->left = rect.left;
                lpr->right = lpr->left + maxw;
            }
            if( h > maxh ) {
                lpr->top = rect.top;
                lpr->bottom = lpr->top + maxh;
            }
            break;

        case WMSZ_TOPRIGHT:     // Top-right corner
            if( w > maxw ) {
                lpr->left = rect.left;
                lpr->right = lpr->left + maxw;
            }
            if( h > maxh ) {
                lpr->bottom = rect.bottom;
                lpr->top = lpr->bottom - maxh;
            }
            break;

        case WMSZ_BOTTOMLEFT:   // Bottom-left corner
            if( w > maxw ) {
                lpr->right = rect.right;
                lpr->left = lpr->right - maxw;
            }
            if( h > maxh ) {
                lpr->top = rect.top;
                lpr->bottom = lpr->top + maxh;
            }
            break;

        case WMSZ_TOP:          // Top edge
        case WMSZ_TOPLEFT:      // Top-left corner
        case WMSZ_LEFT:         // Left edge
            if( w > maxw ) {
                lpr->right = rect.right;
                lpr->left = lpr->right - maxw;
            }
            if( h > maxh ) {
                lpr->bottom = rect.bottom;
                lpr->top = lpr->bottom - maxh;
            }
            break;
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
//void KAppWin::showClipboard()
//{
//	if( !OpenClipboard( hWnd ) )
//		return;
//
//	HANDLE h = GetClipboardData( CF_TEXT );
//	char* c = (char*)h;
//
//	CloseClipboard();
//}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KAppWin::initMenu()
{
    Bool textselected = FALSE;
    // fix-me: determine if any text has been selected
    //

    Bool clipboarddata = FALSE;
    // determine if there is any clipboard data to paste
    //
    if( OpenClipboard( hWnd ) ) {
        HANDLE h = GetClipboardData( CF_TEXT );
        char* c = (char*)h;

        CloseClipboard();
        clipboarddata = TRUE;
    }
	if (menu)
	    menu->setCopyPaste( textselected, clipboarddata );
}

void KAppWin::setKFont(KFont * f) 
{
    if ( client ) {
        client->setFont(f);
        int maxw = 0, maxh = 0;
        client->getMaxpDim( maxw, maxh );
        SetWindowPos( hWnd, 0, 0, 0, maxw, maxh
                      , SWP_NOZORDER | SWP_NOMOVE );
        client->paint();
        setFont(f->getFaceName(), f->getFontPointsH());
    }
}

static HWND se_hwnd;
static void Real_Win32ShellExecute( void * _object )
{
    HINSTANCE error;
    char * object = (char *) _object;

    error = ShellExecute(se_hwnd, 0, object, 0, 0, SW_SHOWNORMAL);

#ifdef COMMENT
    if (((DWORD)error) <= 32)
    {
        printf("%s %s %d","Win32 ShellExecute failure",object,error);
        switch ( (DWORD)error ) {
        case 0:	
            printf("%s %s %d","Win32 ShellExecute","The operating system is out of memory or resources.",0);
            break;
        case ERROR_BAD_FORMAT:
            printf("%s %s %d","Win32 ShellExecute","The .EXE file is invalid (non-Win32 .EXE or error in .EXE image).",0);
            break;
        case SE_ERR_ACCESSDENIED:
            printf("%s %s %d","Win32 ShellExecute","The operating system denied access to the specified file.",0);
            break;
        case SE_ERR_ASSOCINCOMPLETE:
            printf("%s %s %d","Win32 ShellExecute","The filename association is incomplete or invalid.",0);
            break;
        case SE_ERR_DDEBUSY:
            printf("%s %s %d","Win32 ShellExecute","The DDE transaction could not be completed because other DDE transactions were being processed.",0);
            break;
        case SE_ERR_DDEFAIL:
            printf("%s %s %d","Win32 ShellExecute","The DDE transaction failed.",0);
            break;
        case SE_ERR_DDETIMEOUT:
            printf("%s %s %d","Win32 ShellExecute","The DDE transaction could not be completed because the request timed out.",0);
            break;
        case SE_ERR_DLLNOTFOUND:
            printf("%s %s %d","Win32 ShellExecute","The specified dynamic-link library was not found.",0);
            break;
        case SE_ERR_FNF:
            printf("%s %s %d","Win32 ShellExecute","The specified file was not found.",0);
            break;
        case SE_ERR_NOASSOC:
            printf("%s %s %d","Win32 ShellExecute","There is no application associated with the given filename extension.",0);
            break;
        case SE_ERR_OOM:
            printf("%s %s %d","Win32 ShellExecute","There was not enough memory to complete the operation.",0);
            break;
        case SE_ERR_PNF:
            printf("%s %s %d","Win32 ShellExecute","The specified path was not found.",0);
            break;
        case SE_ERR_SHARE:
            printf("%s %s %d","Win32 ShellExecute","A sharing violation occurred.",0);
            break;
        default:
            printf("%s %s %d","Win32 ShellExecute","Unknown error",0);
        }
    }
    else {
        printf("%s %s %d","Win32 ShellExecute success",object,error);
    }
#endif /* COMMENT */
    CloseHandle((HINSTANCE)error);
}

int KAppWin::Win32ShellExecute( char * object )
{
    ::se_hwnd = hwnd();
    DWORD tid = (DWORD) _beginthread( Real_Win32ShellExecute, 
                            65535, 
                            (void *)object 
                            ) ;
    return (tid != 0xffffffff);
}


int KAppWin::browse(char * url)
{
    return(Win32ShellExecute( url ));
}

int KAppWin::readManual(void)
{
    char manpath[512];
    struct stat buf;
    int i;

    /* Need to change directory to the DOCS\\MANUAL directory */
    /* before starting the manual.  Otherwise, Netscape may   */
    /* be unable to find the referential links.               */

    ckmakmsg(manpath,512, exedir, "DOCS\\MANUAL\\KERMIT95.HTM", NULL, NULL);
    for ( i=strlen(manpath); i > 0 && manpath[i] != '\\'; i-- )
        if (manpath[i] == '/')
            manpath[i] = '\\';

    if ( stat(manpath,&buf) >= 0 ) {
        int drive;
        char cwd[256], cwd2[256];

        drive = _getdrive();

        if ( drive != (tolower(manpath[0])-'a'+1) ) {
            _chdrive(tolower(manpath[0])-'a'+1);
        }
        _getcwd( cwd, 255 );

        for ( i=strlen(manpath); i > 0 && manpath[i] != '\\'; i-- );

        if ( i != 0 ) {
            manpath[i] = '\0';
            _chdir(&manpath[2]);  /* skip the drive letter */
            manpath[i] = '\\';
            _getcwd( cwd2, 255 );
        }

        browse(manpath);
        _chdir(cwd);
        _chdrive(drive);
        return(1);
    }
    return(0);
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KAppWin::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool done = FALSE;
    //debug(F111,"KAppWin::message","msg",msg);
    switch( msg )
    {
    case WM_ACTIVATE:
        //printf("KAppWin WM_ACTIVATE wParam=%d\n", wParam);
        //debug(F111,"KAppWin::message","WM_ACTIVATE",msg);
        if( LOWORD(wParam) != WA_INACTIVE ) {
            client->takeFocus();
            done = TRUE;
        }
        break;

    case WM_SIZING:{
        LPRECT lpRect = (LPRECT)lParam;
        //printf("KAppWin WM_SIZING wParam=%d top=%d bottom=%d left=%d right=%d\n",
        //        wParam, lpRect->top, lpRect->bottom, lpRect->left, lpRect->right);
        //debug(F111,"KAppWin::message","WM_SIZING",msg);
        BYTE keyState[256];
        int  save = kglob->mouseEffect;
        if (GetKeyboardState(keyState)) {
            if ( (keyState[VK_CONTROL] & 128) && 
                !(keyState[VK_SHIFT] & 128) && 
                !(keyState[VK_MENU] & 128))
                kglob->mouseEffect = TERM_MOUSE_CHANGE_FONT;
            else if ( !(keyState[VK_CONTROL] & 128) && 
                      (keyState[VK_SHIFT] & 128) && 
                      !(keyState[VK_MENU] & 128))
                kglob->mouseEffect = TERM_MOUSE_CHANGE_DIMENSION;
        //    else 
        //        kglob->mouseEffect = TERM_MOUSE_NO_EFFECT;
        }
        done = sizing( wParam, lpRect );
        kglob->mouseEffect = save;
        break;
    }

    case WM_SIZE:
        //printf("KAppWin WM_SIZE wParam=%d height=%d width=%d\n",
        //        wParam, HIWORD(lParam), LOWORD(lParam));
        //debug(F111,"KAppWin::message","WM_SIZE",msg);
        size( LOWORD(lParam), HIWORD(lParam) );
        switch ( wParam ) {
        case SIZE_RESTORED:
            if ( wmSize == SIZE_MAXIMIZED ) {
                RECT rect;
                double newLineSpacing = tt_linespacing[vmode];
                rect.top = 0;
                rect.left = 0;
                rect.bottom = HIWORD(lParam);
                rect.right = LOWORD(lParam);
                tt_linespacing[vmode] = kglob->saveLineSpacing;
                switch (kglob->saveMouseEffect) {
                case TERM_MOUSE_CHANGE_FONT:
                    sizeFont(&rect,1);
                    client->paint();
                    break;
                case TERM_MOUSE_CHANGE_DIMENSION:
                    if( sizepop )
                        sizepop->show( FALSE );
                    client->endSizing();
                    break;
                case TERM_MOUSE_NO_EFFECT:
                    client->setDimensions(TRUE);
                    client->endSizing();
                    break;
                }
                tt_linespacing[vmode] = newLineSpacing;
            }
            break;
        case SIZE_MINIMIZED:
            break;
        case SIZE_MAXIMIZED:
            if ( wmSize == SIZE_RESTORED ) {
                RECT rect;
                rect.top = 0;
                rect.left = 0;
                rect.bottom = HIWORD(lParam);
                rect.right = LOWORD(lParam);
                kglob->saveMouseEffect = kglob->mouseEffect;
                kglob->saveLineSpacing = tt_linespacing[vmode];
                switch (kglob->mouseEffect) {
                case TERM_MOUSE_CHANGE_FONT:
                    sizeFont(&rect,0);
                    client->paint();
                    break;
                case TERM_MOUSE_CHANGE_DIMENSION:
                    if( sizepop )
                        sizepop->show( FALSE );
                    client->endSizing();
                    break;
                case TERM_MOUSE_NO_EFFECT:
                    client->endSizing();
                    break;
                }
            }
            break;  
        case SIZE_MAXSHOW:
        case SIZE_MAXHIDE:
        default:
            break;
        }
        wmSize = wParam;
        break;

    case WM_EXITSIZEMOVE: {
        //printf("KAppWin WM_EXITSIZEMOVE\n");
        //debug(F111,"KAppWin::message","WM_EXITSIZEMOVE",msg);
        BYTE keyState[256];
        int  save = kglob->mouseEffect;
        if (GetKeyboardState(keyState)) {
            if ( (keyState[VK_CONTROL] & 128) && 
                !(keyState[VK_SHIFT] & 128) && 
                !(keyState[VK_MENU] & 128))
                kglob->mouseEffect = TERM_MOUSE_CHANGE_FONT;
            else if ( !(keyState[VK_CONTROL] & 128) && 
                      (keyState[VK_SHIFT] & 128) && 
                      !(keyState[VK_MENU] & 128))
                kglob->mouseEffect = TERM_MOUSE_CHANGE_DIMENSION;
        //    else 
        //        kglob->mouseEffect = TERM_MOUSE_NO_EFFECT;
        }
        if( sizepop )
            sizepop->show( FALSE );
        client->endSizing();
        kglob->mouseEffect = save;
        break;
    }
    case WM_GETMINMAXINFO:
        //printf("KAppWin WM_GETMINMAXINFO\n");
        //debug(F111,"KAppWin::message","WM_GETMINMAXINFO",msg);
        // use of this code prevents Maximize from changing the 
        // window dimensions.
        if ( kglob->mouseEffect == TERM_MOUSE_NO_EFFECT )
        {
            int maxw = 0, maxh = 0;
            if( !client->getMaxpDim( maxw, maxh ) )
                break;

            LPMINMAXINFO info = (LPMINMAXINFO)lParam;
            info->ptMaxSize.x = maxw;
            info->ptMaxSize.y = maxh;
            info->ptMaxPosition.x = 0;
            info->ptMaxPosition.y = 0;
        }
        break;

    case WM_DRAWITEM:
        //debug(F111,"KAppWin::message","WM_DRAWITEM",msg);
        done = status->message( hwnd, msg, wParam, lParam );
        break;

    case WM_INITMENU:
        //debug(F111,"KAppWin::message","WM_INITMENU",msg);
        initMenu();
        break;

    case WM_COMMAND:
        //debug(F111,"KAppWin::message","WM_COMMAND",msg);
        {
            //debug(F111,"KAppWin::message WM_COMMAND","LOWORD(wParam)",LOWORD(wParam));
            switch( LOWORD(wParam) )
            {
            case ID_OPTIONS_FONT:
                {
                    //debug(F111,"KAppWin::message WM_COMMAND","ID_OPTIONS_FONT",LOWORD(wParam));
                    LOGFONT logfont;
                    PLOGFONT curfont = client->getFont()->getLogFont();
                    memcpy( &logfont, curfont, sizeof(LOGFONT) );
                    
                    KFontDialog fontdlg( kglob, this );
                    fontdlg.show( &logfont );

                    if( memcmp( &logfont, curfont, sizeof(LOGFONT) ) != 0 ) {
                        KFont* newFont = new KFont( &logfont );
                        setKFont( newFont );
                    }
                    break;
                }
            case ID_OPTIONS_CUSTOMIZESTATUSBAR:
                {
                    //debug(F111,"KAppWin::message WM_COMMAND","ID_OPTIONS_CUSTOMIZESTATUSBAR",LOWORD(wParam));
                    //Bool stat = menu->toggleCheck( 
                    //    ID_OPTIONS_CUSTOMIZESTATUSBAR );
                    status->customize();
                    break;
                }
            case ID_HELP_ABOUT:
                {
                    //debug(F111,"KAppWin::message WM_COMMAND","ID_HELP_ABOUT",LOWORD(wParam));
                    KAbout about( kglob, this );
                    about.show();
                    break;
                }
            case ID_HELP_HELPONKERMIT:
                {
                    //debug(F111,"KAppWin::message WM_COMMAND","ID_HELP_HELPONKERMIT",LOWORD(wParam));
                    char tmp[256];
                    char* c = ::getVar( LOG_HOME_DIR );
                    strcpy( tmp, c );
                    strcat( tmp, "kermit95.hlp" );
                    WinHelp( hWnd, tmp, HELP_INDEX, 0 );
                    break;
                }

//            case ID_WINDOW_TERMINAL:
//                debug(F111,"KAppWin::message WM_COMMAND","ID_WINDOW_TERMINAL",LOWORD(wParam));
//                PostMessage( hWnd, WM_GOTO_TERMINAL_WINDOW, 0, 0 );
//                break;
//            case ID_WINDOW_COMMAND:
//                debug(F111,"KAppWin::message WM_COMMAND","ID_WINDOW_COMMAND",LOWORD(wParam));
//                PostMessage( hWnd, WM_GOTO_COMMAND_WINDOW, 0, 0 );
//                break;
            case ID_WINDOW_CLIPBOARD:
                //debug(F111,"KAppWin::message WM_COMMAND","ID_WINDOW_CLIPBOARD",LOWORD(wParam));
                ShellExecute( hWnd, "open", "clipbrd.exe", NULL, NULL, SW_SHOW );
                break;
            case ID_WINDOW_TEXTEDIT:
                //debug(F111,"KAppWin::message WM_COMMAND","ID_WINDOW_TEXTEDIT",LOWORD(wParam));
                ShellExecute( hWnd, "open", "notepad.exe", NULL, NULL, SW_SHOW );
                break;
            }
            break;
        }
    }
    return done;
}

