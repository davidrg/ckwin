#include <windowsx.h>
#include "ktermin.hxx"
#include "kmenu.hxx"
#include "ktoolbar.hxx"
#include "kstatus.hxx"
#include "kclient.hxx"
#include "kdwnload.hxx"
#include "kfont.hxx"
#include "ksysmets.hxx"
#include "ikcmd.h"
#include "ikterm.h"
#include "ikui.h"
#include "ikextern.h"

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif


#ifndef NOTOOLBAR
const int numButtons = 3;
const int numBitmaps = 2;
ToolBitmapDef tbButtons[] = {
 { { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 }, 0 },
 { { 0, ID_TOOLBAR_CONNECT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 }, "Switch to K95 Dialer" },
 { { 1, ID_TOOLBAR_EXIT, TBSTATE_ENABLED, TBSTYLE_CHECK, 0L, 0 }, "Switch to/from Command Screen" },
// { { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 }, 0 },
// { { 4, ID_DOWNLOAD, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 }, "Download Files" },
// { { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 }, 0 },
// { { 6, ID_WINDOW_COMMAND, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 }, "Command Window" },
// { { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 }, 0 }
};
#endif

extern "C" {
/* This is declared in ckotio.c and set to 1 when we're on NT 3.51
 * (and 3.50 and 3.1) */
extern int nt351;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KTerminal::KTerminal( K_GLOBAL* kg )
    : KAppWin( kg, kg->noClose ? IDR_TERMMENU_NOCLOSE : IDR_TERMMENU, IDR_TOOLBARTERMINAL ),
    toolbar_disabled(FALSE), menuInitialized(FALSE), firstShow(TRUE)
{
    client = new KClient( kg, VTERM );
    setClient( client );

	// Disable toolbar button & menu item for dialer
	if (!DialerExists() && toolbar) {
#ifndef NOTOOLBAR
		// Disable toolbar button
		BYTE state = tbButtons[1].tbbutton.fsState;
		state = state & ~TBSTATE_ENABLED;
		tbButtons[1].tbbutton.fsState = state;

		// And anything else in the GUI that starts the dialer
		noDialer = TRUE;
#endif
	} else {
		noDialer = FALSE;
	}	
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KTerminal::~KTerminal()
{
    delete client;
}

void
KTerminal::show( Bool bVisible )
{
    if ( firstShow ) {
        ShowWindow( hWnd, kglob->nCmdShow );
        firstShow = FALSE;
    } else {
        ShowWindow( hWnd, bVisible ? SW_SHOW : SW_HIDE );
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KTerminal::setTermType( char* tt )
{
#ifndef NOTOOLBAR
    if ( toolbar )
        toolbar->setTermType( tt );
#endif
}

void KTerminal::setRemoteCharset( char* cs )
{
#ifndef NOTOOLBAR
    if ( toolbar )
        toolbar->setCharset( cs );
#endif
}

void KTerminal::setFont( char* fnt, int h )
{
#ifndef NOTOOLBAR
    if ( toolbar ) {
        toolbar->setFontName( fnt );
        toolbar->setFontHeight( h );
    }
#endif
    kglob->fontHeight = h;
}

void KTerminal::setKFont(KFont * f) 
{
    KAppWin::setKFont(f);
    char * facename = f->getFaceName();
    int    h = f->getFontPointsH();
    int    w = f->getFontPointsW();

    if ( facename )
        ckstrncpy(kglob->faceName,facename,sizeof(kglob->faceName));
    kglob->fontHeight = h;
    kglob->fontWidth = w;

    // Lucida substs only take place if the font is Lucida Console
    win95lucida = !ckstrcmp(facename,"lucida console",-1,0);
    // HSL substs occur whenever the font does not support HSLs
    win95hsl = ckstrcmp(facename,"everson mono terminal",-1,0);
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KTerminal::getCreateInfo( K_CREATEINFO* info )
{
    info->classname = KWinClassName;
    info->text      = TermTitle;
    info->style     = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    info->x         = kglob->settings[ Terminal_Left ];
    info->y         = kglob->settings[ Terminal_Top ];
    info->width     = CW_USEDEFAULT;
    info->height    = 0;
}

void KTerminal::initMenu(void)
{
    KAppWin::initMenu();
    if (!menu)
        return;
    menu->createMenu( hInst, this );

    if ( autodl && !adl_ask ) {
        menu->setCheck(ID_ACTION_AUTODOWNLOAD_ON);
        menu->unsetCheck(ID_ACTION_AUTODOWNLOAD_OFF);
        menu->unsetCheck(ID_ACTION_AUTODOWNLOAD_ASK);
    } else if ( autodl && adl_ask ){
        menu->unsetCheck(ID_ACTION_AUTODOWNLOAD_ON);
        menu->unsetCheck(ID_ACTION_AUTODOWNLOAD_OFF);
        menu->setCheck(ID_ACTION_AUTODOWNLOAD_ASK);
    } else {
        menu->unsetCheck(ID_ACTION_AUTODOWNLOAD_ON);
        menu->unsetCheck(ID_ACTION_AUTODOWNLOAD_OFF);
        menu->setCheck(ID_ACTION_AUTODOWNLOAD_ASK);
    }

    if ( cmask == 0177 ) {
        menu->setCheck(ID_ACTION_7BIT);
        menu->unsetCheck(ID_ACTION_8BIT);
    } else {
        menu->unsetCheck(ID_ACTION_7BIT);
        menu->setCheck(ID_ACTION_8BIT);
    }

    menu->unsetCheck(ID_ACTION_CAPTURE);
    menu->unsetCheck(ID_ACTION_PRINTERCOPY);
    menu->unsetCheck(ID_ACTION_DEBUG);
    menu->unsetCheck(ID_ACTION_PCTERM);
    menu->unsetCheck(ID_ACTION_KEYCLICK);
    if ( kglob->mouseEffect == TERM_MOUSE_CHANGE_FONT )
        menu->setCheck(ID_ACTION_RESIZE_FONT);
    else
        menu->unsetCheck(ID_ACTION_RESIZE_FONT);
    if ( kglob->mouseEffect == TERM_MOUSE_CHANGE_DIMENSION )
        menu->setCheck(ID_ACTION_RESIZE_DIMENSION);
    else
        menu->unsetCheck(ID_ACTION_RESIZE_DIMENSION);
    menu->setCheck(ID_ACTION_GUI_DIALOGS);

    if ( tt_url_hilite ) 
        menu->setCheck(ID_ACTION_URL_HIGHLIGHT);
    else
        menu->unsetCheck(ID_ACTION_URL_HIGHLIGHT);

    if ( xitwarn == 1 ) {
        menu->setCheck(ID_ACTION_WARNING_ON);
        menu->unsetCheck(ID_ACTION_WARNING_OFF);
        menu->unsetCheck(ID_ACTION_WARNING_ALWAYS);
    } else if ( xitwarn == 2 ){
        menu->unsetCheck(ID_ACTION_WARNING_ON);
        menu->unsetCheck(ID_ACTION_WARNING_OFF);
        menu->setCheck(ID_ACTION_WARNING_ALWAYS);
    } else {
        menu->unsetCheck(ID_ACTION_WARNING_ON);
        menu->unsetCheck(ID_ACTION_WARNING_OFF);
        menu->setCheck(ID_ACTION_WARNING_ALWAYS);
    }

    if ( autolocus == 1 ) {
        menu->setCheck(ID_ACTION_AUTO_LOCUS_ON);
        menu->unsetCheck(ID_ACTION_AUTO_LOCUS_OFF);
        menu->unsetCheck(ID_ACTION_AUTO_LOCUS_ASK);
    } else if ( autolocus == 2 ){
        menu->unsetCheck(ID_ACTION_AUTO_LOCUS_ON);
        menu->unsetCheck(ID_ACTION_AUTO_LOCUS_OFF);
        menu->setCheck(ID_ACTION_AUTO_LOCUS_ASK);
    } else {
        menu->unsetCheck(ID_ACTION_AUTO_LOCUS_ON);
        menu->unsetCheck(ID_ACTION_AUTO_LOCUS_OFF);
        menu->setCheck(ID_ACTION_AUTO_LOCUS_ASK);
    }

    if ( locus )
        menu->setCheck(ID_ACTION_LOCUS);
    else
        menu->unsetCheck(ID_ACTION_LOCUS);

	if (noDialer)
		getMenu()->enable(ID_CONNECT, FALSE);
}

void KTerminal::disableMenu( void )
{
    if (!menu)
        return;

#ifndef COMMENT
    menu->enable(ID_CONNECT,FALSE);
    menu->enable(ID_FILE_HANGUP, FALSE);
    menu->enable(ID_FILE_EXIT,FALSE);
    menu->enable(ID_ACTION_AUTODOWNLOAD_ON, FALSE);
    menu->enable(ID_ACTION_AUTODOWNLOAD_OFF, FALSE);
    menu->enable(ID_ACTION_AUTODOWNLOAD_ASK, FALSE);
    // menu->enable(ID_AUTO_LOCUS, FALSE);
    menu->enable(ID_ACTION_AUTO_LOCUS_ON, FALSE);
    menu->enable(ID_ACTION_AUTO_LOCUS_OFF, FALSE);
    menu->enable(ID_ACTION_AUTO_LOCUS_ASK, FALSE);
    // menu->enable(ID_BYTESIZE, FALSE);
    menu->enable(ID_ACTION_7BIT, FALSE);
    menu->enable(ID_ACTION_8BIT, FALSE);
    menu->enable(ID_ACTION_CAPTURE, FALSE);
    menu->enable(ID_ACTION_PRINTERCOPY, FALSE);
    menu->enable(ID_ACTION_DEBUG, FALSE);
    // menu->enable(ID_WARNING, FALSE);
    menu->enable(ID_ACTION_WARNING_ALWAYS, FALSE);
    menu->enable(ID_ACTION_WARNING_OFF, FALSE);
    menu->enable(ID_ACTION_WARNING_ON, FALSE);
    menu->enable(ID_ACTION_GUI_DIALOGS, FALSE);
    menu->enable(ID_ACTION_KEYCLICK, FALSE);
    menu->enable(ID_ACTION_LOCUS, FALSE);
    menu->enable(ID_ACTION_PCTERM, FALSE);
    // menu->enable(ID_RESIZE, FALSE);
    menu->enable(ID_ACTION_RESIZE_FONT, FALSE);
    menu->enable(ID_ACTION_RESIZE_DIMENSION, FALSE);
    menu->enable(ID_ACTION_URL_HIGHLIGHT, FALSE);
	menu->enable(ID_DOWNLOAD, FALSE);
#else
    destroyMenu();
    client->endSizing(TRUE);
#endif
}

void KTerminal::disableClose( void )
{
    HMENU hMenu;
    UINT  pos;
    UINT  count;

    hMenu = GetSystemMenu(hWnd, FALSE);
    count = GetMenuItemCount(hMenu);
    for (pos = 0; pos < count; pos++) {
        if (GetMenuItemID(hMenu, pos) == SC_CLOSE) {
            RemoveMenu(hMenu, pos, MF_BYPOSITION);
            RemoveMenu(hMenu, pos-1, MF_BYPOSITION);
            break;
        }
    }

    if (!menu)
        return;

    menu->enable(ID_FILE_EXIT,FALSE);
}

void KTerminal::disableToolbar( void )
{
    toolbar_disabled = TRUE;
#ifdef COMMENT
    EnableWindow(toolbar->hwnd(),FALSE);
    delete toolbar;
    toolbar = 0;
    client->endSizing(TRUE);
#endif /* COMMENT */
#if 0
    // apparently deleting the existing toolbar is a bad idea
    KToolBar * oldtb = toolbar;
    KToolBar * newtb = new KToolBar( kglob, IDR_TOOLBARTERMINAL );
    newtb->initButtons( numButtons, numBitmaps, &(*tbButtons) );
    delete oldtb;
    newtb->createWin( this );
    toolbar = newtb;
#endif
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KTerminal::createWin( KWin* par )
{
    inCreate( TRUE );

    KAppWin::createWin( par );

    initMenu();
    client->createWin( this );

    int i=0,j=0;
    KFont * kfont = client->getFont();
    if (kfont) {
        char * faceName = kfont->getFaceName();

    if (ntermfont == 0)
        BuildFontTable(&term_font, &_term_font, &ntermfont);
    for( i = 0; i < ntermfont; i++ )
    {
        if ( !strcmp(term_font[i].kwd,faceName) )
            break;
    }       
        if (i < ntermfont ) {
            tt_font = term_font[i].kwval;
            tt_font_size = kfont->getFontPointsH();
        }
    }

    if (status) {
        status->createWin( this );
        int itemp[5] = { 350, 240, 130, 80, 0 };
        status->setParts( 5, itemp );
        status->setText( STATUS_CMD_TERM, "Command Screen" );
        status->setText( STATUS_LOCUS, "Locus: Local" );
        status->setText( STATUS_PROTO, "" );
        status->setText( STATUS_CURPOS, "1, 1" );
        status->setText( STATUS_HW, "80 x 25");
    }

    int tbwid=0, tbhi=0;
#ifndef NOTOOLBAR
    if ( toolbar ) {
        toolbar->initButtons( numButtons, numBitmaps, &(*tbButtons) );
        toolbar->createWin( this );
        toolbar->createTermTypeCombo();
        toolbar->createFontNameCombo();
        toolbar->createFontHeightCombo();
        toolbar->createCharsetCombo();
        toolbar->getSize( tbwid, tbhi );
    }
#endif

    int cwid=0, chi=0;
    client->getSize( cwid, chi );

    int swid=0, shi=0;
    if ( status )
        status->getSize( swid, shi );

    int mhi= menu ? kglob->sysMets->menuHeight() : 0;

    int yclient = tbhi - kglob->sysMets->edgeHeight();
    SetWindowPos( client->hwnd(), 0, 0, yclient, 0, 0
        , SWP_NOSIZE | SWP_NOZORDER );

    int newWid = cwid + (2 * kglob->sysMets->sizeframeWidth());
    int newHi = chi + tbhi + shi + mhi + (2 * kglob->sysMets->sizeframeHeight())
        + kglob->sysMets->captionHeight();

    inCreate( FALSE );

    setHwndKUI( hWnd );		// For console windows

    SetWindowPos( hWnd, NULL, 0, 0, newWid, newHi
		, SWP_NOMOVE | SWP_NOZORDER );
    
    if (kglob->noClose) {
        disableClose();
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KTerminal::browseFile( eFileType filetype )
{
    char szFilter[MAX_PATH] = "\0";
    char szFileTitle[MAX_PATH] = "\0";
    char szFile[MAX_PATH] = "\0";

    OPENFILENAME OpenFileName;
    memset( &OpenFileName, '\0', sizeof(OPENFILENAME) );

    switch( filetype )
    {
    case eCommandFile:
        OpenFileName.lpstrFilter = "Script Files (*.kmd)\0*.log\0All Files (*.*)\0*.*\0";
        OpenFileName.lpstrTitle = "Execute Command File";
        break;
    case eKeyMapFile:
        OpenFileName.lpstrFilter = "Key Map Files (*.kmf)\0*.kmf\0All Files (*.*)\0*.*\0";
        OpenFileName.lpstrTitle = "Open Key Map File";
        break;
    case eTextFile:
        OpenFileName.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        OpenFileName.lpstrTitle = "Open Text File";
        break;
    case eCaptureFile:
        OpenFileName.lpstrFilter = "Capture Files (*.cap)\0*.cap\0All Files (*.*)\0*.*\0";
        OpenFileName.lpstrTitle = "Open Capture File";
        break;
    }

    OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = hWnd;
    OpenFileName.hInstance         = hInst;
    OpenFileName.lpstrCustomFilter = (LPTSTR) NULL;
    OpenFileName.nMaxCustFilter    = 0L;
    OpenFileName.nFilterIndex      = 0L;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = sizeof(szFile);
    OpenFileName.lpstrFileTitle    = szFileTitle;
    OpenFileName.nMaxFileTitle     = sizeof(szFileTitle);
    OpenFileName.lpstrInitialDir   = zgtdir();
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = 0;
    OpenFileName.lCustData         = 0;

    OpenFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST 
        | OFN_HIDEREADONLY
#ifndef CKT_NT35_OR_31
        | OFN_EXPLORER
#endif
        ;

    if( GetOpenFileName( &OpenFileName ) )
    {
        char* c = new char[strlen(szFile) + 10];
        strcpy( c, "take " );
        switch( filetype )
        {
        case eCommandFile:
            // take a command file
            //
            strcat( c, szFile );
            strcat( c, "\n" );
            putCmdString( c );
            break;
        case eKeyMapFile:
            // fix-me: open a key map file
            //
            break;
        case eTextFile:
            // open a text file
            //
            ShellExecute( hWnd, "open", "notepad.exe", szFile, NULL, SW_SHOW );
            break;

        case eCaptureFile:
            // open a capture file
            //
            break;
        }
        delete c;
    }
}       

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KTerminal::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool done = FALSE;
    debug(F111,"KTerminal::message","msg",msg);
    switch( msg )
    {
#ifndef NOTOOLBAR
    case WM_NOTIFY:
        debug(F111,"KTerminal::message","WM_NOTIFY",msg);
        if ( toolbar && !toolbar_disabled)
            done = toolbar->message( hwnd, msg, wParam, lParam );
        else 
            done = TRUE;
        break;
#endif

#ifdef COMMENT
    case WM_MENUSELECT:
        debug(F111,"KTerminal::message","WM_MENUSELECT",msg);
        {
            ushort item = LOWORD(wParam);  // id of selection
            ushort flags = HIWORD(wParam);  // menu flags
            HMENU hMenu = (HMENU)lParam;     // menu w/selection

            if( (flags & MF_SEPARATOR) || (flags & MF_SYSMENU) )
                break;

            char buf[256];
            memset( buf, '\0', 256 );
            _itoa( item, buf, 10 );
            strcat( buf, " <- was selected!!!" );
            status->setText( 0, buf );
            break;
        }
#endif /* COMMENT */

    case WM_CLOSE:
        debug(F111,"KTerminal::message","WM_CLOSE",msg);
        PostMessage( hWnd, WM_REQUEST_CLOSE_KERMIT, 0, 0 );
        done = TRUE;
        break;

    case WM_NCLBUTTONDOWN:
        if (nt351) {
            /* On NT 3.x we emulate WM_SIZING and WM_EXITSIZEMOVE via
             * intercepting WM_NCLBUTTONDOWN and implementing the size loop
             * ourselves */
            done = OnNCLButtonDown(hwnd, FALSE, (int) (short) LOWORD(lParam),
                                   (int) (short) HIWORD(lParam), (UINT)(wParam));
        }
        break;

    case WM_SIZING:
    case WM_EXITSIZEMOVE:
    case WM_ACTIVATE:
    case WM_SIZE:
    case WM_GETMINMAXINFO:
    case WM_DRAWITEM:
        debug(F111,"KTerminal::message","WM_{ACTIVATE,SIZING,EXITSIZEMOVE,GETMINMAXINFO,DRAWITEM,INITMENU}",msg);
        done = KAppWin::message( hwnd, msg, wParam, lParam );
        break;

    case WM_INITMENU:
        if (!menuInitialized) {
            initMenu();
            menuInitialized = TRUE;
        }
        done = TRUE;
        break;

    case WM_QUERYENDSESSION:
        debug(F111,"KTerminal::message","WM_QUERYENDSESSION",msg);
        done = TRUE;
#if _MSC_VER > 1010
        if ( lParam & ENDSESSION_LOGOFF ) {
#else
        /* Visual C++ <= 4.1: lparam == TRUE on logoff, FALSE on shutdown
         * (on Windows 95 only according to the docs) */
        if (lParam) {
#endif
            debug(F100,"ENDSESSION_LOGOFF","",0);
            if ( startflags & 128 ) {
                debug(F100,"startflags & 128","",0);
                done = FALSE;
            }
        }
        break;

    case WM_ENDSESSION:
        debug(F111,"KTerminal::message","WM_QUERYENDSESSION",msg);
#if _MSC_VER > 1010
        if ( wParam && (lParam & ENDSESSION_LOGOFF) ) {
#else
        /* Visual C++ <= 4.1: lparam == TRUE on logoff, FALSE on shutdown
         * (on Windows 95 only according to the docs) */
        if (lParam) {
#endif
            debug(F100,"ENDSESSION_LOGOFF","",0);
            if ( startflags & 128 ) {
                debug(F100,"startflags & 128","",0);
                done = TRUE;
            }
        }
        break;

    case WM_COMMAND:
        debug(F111,"KTerminal::message","WM_COMMAND",msg);
        debug(F111,"KTerminal::message WM_COMMAND","LOWORD(wParam)",LOWORD(wParam));
        {
            switch( LOWORD(wParam) )
            {
            case ID_TOOLBAR_CONNECT:
            case ID_CONNECT:
                debug(F111,"KTerminal::message WM_COMMAND","ID_CONNECT",LOWORD(wParam));
                {
                    //KConnect connect( kglob, this );
                    //connect.show();
                    StartDialer();
                    break;
                }
            case ID_FILE_HANGUP:
                {
                    debug(F111,"KTerminal::message WM_COMMAND","ID_FILE_HANGUP",LOWORD(wParam));
                    if ( hupok(1) )
                        dokverb(vmode,K_HANGUP);
                    break;
                }
            case ID_DOWNLOAD:
                debug(F111,"KTerminal::message WM_COMMAND","ID_DOWNLOAD",LOWORD(wParam));
                {
                    KDownLoad download( kglob, TRUE, FALSE );
                    download.createWin( this );
                    download.show();
                    break;
                }
            case ID_FILE_EXIT:
                debug(F111,"KTerminal::message WM_COMMAND","ID_FILE_EXIT",LOWORD(wParam));
                SendMessage( hWnd, WM_REQUEST_CLOSE_KERMIT, 0, 0 );
                break;

            case IDC_COMBOTERMINALTYPE:
            case IDC_COMBOFONTNAME:
            case IDC_COMBOFONTHEIGHT:
            case IDC_COMBOCHARSET:
                debug(F111,"KTerminal::message WM_COMMAND","IDC_COMBOTERMINALTYPE",LOWORD(wParam));
                // the toolbar combobox
                //
                if ( toolbar && !toolbar_disabled )
                    done = toolbar->message( hwnd, msg, wParam, lParam );
                else {
                    done = TRUE;
                }
                break;

            case ID_FILE_EXECUTECOMMANDFILE:
                debug(F111,"KTerminal::message WM_COMMAND","ID_FILE_EXECUTECOMMANDFILE",LOWORD(wParam));
                browseFile( eCommandFile );
                break;
            case ID_FILE_LOADKEYMAPFILE:
                debug(F111,"KTerminal::message WM_COMMAND","ID_FILE_LOADKEYMAPFILE",LOWORD(wParam));
                browseFile( eKeyMapFile );
                break;
            case ID_FILE_OPENTEXT:
                debug(F111,"KTerminal::message WM_COMMAND","ID_FILE_OPENTEXT",LOWORD(wParam));
                browseFile( eTextFile );
                break;
            case ID_FILE_OPENCAPTURE:
                debug(F111,"KTerminal::message WM_COMMAND","ID_FILE_OPENCAPTURE",LOWORD(wParam));
                browseFile( eCaptureFile );
                break;

            case ID_TOOLBAR_EXIT:
            case ID_ACTION_EXIT:
                putkverb(vmode, K_EXIT);
                break;

            case ID_ACTION_HELP:
                dokverb(vmode, K_HELP);
                break;

            case ID_ACTION_PASTE:
                dokverb(vmode, K_PASTE);
                break;

            case ID_ACTION_RESET:
                dokverb(VTERM,K_RESET);
                break;

            case ID_ACTION_CLEARSCREEN:
                dokverb(vmode,K_CLRSCRN);
                break;

            case ID_ACTION_CLEARSCROLLBACK:
                dokverb(vmode,K_CLRSCROLL);
                break;

            case ID_ACTION_BREAKSENDONE:
                dokverb(VTERM,K_BREAK);
                break;

            case ID_ACTION_SEND_LBREAK:
                dokverb(VTERM,K_LBREAK);
                break;

            case ID_ACTION_AYT:
                dokverb(VTERM,K_TN_AYT);
                break;

            case ID_ACTION_COMPOSECHARACTER:
                putkverb(vmode,K_COMPOSE);
                break;

            case ID_ACTION_COMPOSEUNICODE:
                putkverb(vmode,K_C_UNI16);
                break;

            case ID_ACTION_DEBUG:
                dokverb(VTERM,K_DEBUG);
                break;

            case ID_ACTION_AUTODOWNLOAD:
                dokverb(VTERM,K_AUTODOWN);
                break;

            case ID_ACTION_CAPTURE:
                dokverb(VTERM,K_SESSION);
                break;

            case ID_ACTION_PRINTERCOPY:
                dokverb(VTERM,K_PRTAUTO);
                break;

            case ID_ACTION_7BIT:
                setcmask(7);
                break;

            case ID_ACTION_8BIT:
                setcmask(8);
                break;

            case ID_ACTION_URL_HIGHLIGHT:
                dokverb(VTERM,K_URLHILT);
                break;

            case ID_ACTION_PCTERM:
                dokverb(VTERM,K_PCTERM);
                break;

            case ID_ACTION_MARK_START:
                dokverb(vmode,K_MARK_START);
                break;

            case ID_ACTION_MARK_CANCEL:
                dokverb(vmode,K_MARK_CANCEL);
                break;

            case ID_ACTION_MARK_COPY_CLIP:
                dokverb(vmode,K_MARK_COPYCLIP);
                break;

            case ID_ACTION_MARK_COPY_HOST:
                dokverb(vmode,K_MARK_COPYHOST);
                break;

            case ID_ACTION_SEND_NUL:
                dokverb(vmode,K_NULL);
                break;

            case ID_ACTION_TN_IP:
                dokverb(vmode,K_TN_IP);
                break;

            case ID_ACTION_KEYCLICK:
                dokverb(vmode,K_KEYCLICK);
                break;

            case ID_ACTION_RESIZE_NONE:
                kglob->mouseEffect = TERM_MOUSE_NO_EFFECT;
                if ( menu ) {
                    menu->unsetCheck(ID_ACTION_RESIZE_FONT);
                    menu->unsetCheck(ID_ACTION_RESIZE_DIMENSION);
                }
                break;

            case ID_ACTION_RESIZE_FONT:
                kglob->mouseEffect = TERM_MOUSE_CHANGE_FONT;
                if ( menu ) {
                    menu->setCheck(ID_ACTION_RESIZE_FONT);
                    menu->unsetCheck(ID_ACTION_RESIZE_DIMENSION);
                }
                break;
                
            case ID_ACTION_RESIZE_DIMENSION:
                kglob->mouseEffect = TERM_MOUSE_CHANGE_DIMENSION;
                if ( menu ) {
                    menu->unsetCheck(ID_ACTION_RESIZE_FONT);
                    menu->setCheck(ID_ACTION_RESIZE_DIMENSION);
                }
                break;

            case ID_ACTION_WARNING_ALWAYS:
                setexitwarn(2);
                break;

            case ID_ACTION_WARNING_OFF:
                setexitwarn(0);
                break;

            case ID_ACTION_WARNING_ON:
                setexitwarn(1);
                break;

            case ID_ACTION_GUI_DIALOGS:
                setguidialog(!gui_dialog);
                break;

            case ID_ACTION_AUTO_LOCUS_ASK:
                setautolocus(2);
                break;

            case ID_ACTION_AUTO_LOCUS_OFF:
                setautolocus(0);
                break;

            case ID_ACTION_AUTO_LOCUS_ON:
                setautolocus(1);
                break;

            case ID_ACTION_LOCUS:
                setlocus(!locus,0);
                break;

            case ID_ACTION_AUTODOWNLOAD_ASK:
                setautodl(1,1);
                break;

            case ID_ACTION_AUTODOWNLOAD_OFF:
                setautodl(0,0);
                break;

            case ID_ACTION_AUTODOWNLOAD_ON:
                setautodl(1,0);
                break;

            /* When adjusting URLs here, remember to update the dialer too.
             * Equivalents to the below menu items live in kconnect.cpp */

            case ID_HELP_K95MANUAL:
                readManual();
                break;
            case ID_WEB_KERMIT:
                browse("http://www.kermitproject.org/");
                break;
            case ID_WEB_K95:
                browse("http://www.kermitproject.org/ckw10beta.html");
                break;
            case ID_WEB_NEWS:
                browse("http://www.kermitproject.org/ckw10beta.html");
                break;
            case ID_WEB_PATCH:
                browse("http://www.kermitproject.org/k95upgrade.html");
                break;
            case ID_WEB_SUPPORT:
                browse("http://www.kermitproject.org/support.html");
                break;
            case ID_WEB_FAQ:
                browse("http://www.kermitproject.org/k95faq.html");
                break;

            case ID_OPTIONS_FONT:
            case ID_OPTIONS_CUSTOMIZESTATUSBAR:
            case ID_HELP_ABOUT:
            case ID_HELP_HELPONKERMIT:
            case ID_WINDOW_TERMINAL:
            case ID_WINDOW_COMMAND:
                debug(F111,"KTerminal::message WM_COMMAND","ID_OPTIONS_{FONT,COLOR,CUSTOMIZESTATUSBAR},HELP_{ABOUT,HELPONKERMIT},WINDOW_{TERMINAL,COMMAND,CLIENTSERVER}",LOWORD(wParam));
            default:
                KAppWin::message( hwnd, msg, wParam, lParam );
                break;
            }   
            done = TRUE;
            break;
        }
    }
    return done;
}

/*
 * Gets a rect representing one edge of the overall resize rect.
 */
void GetEdgeRect(RECT rect, int cyFrame, int cxFrame,
                 UINT edge, RECT* result) {

    int width = (rect.right - rect.left);
    int height = (rect.bottom - rect.top);

    switch(edge) {
        case WMSZ_TOP:
            result->left = rect.left;
            result->top = rect.top;
            result->right = width;
            result->bottom = cyFrame;
            break;
        case WMSZ_BOTTOM:
            result->left = rect.left + cxFrame;
            result->top = rect.bottom - cyFrame;
            result->right = width - cxFrame;
            result->bottom = cyFrame;
            break;
        case WMSZ_RIGHT:
            result->left = rect.right - cxFrame;
            result->top = rect.top + cyFrame;
            result->right = cxFrame;
            result->bottom = height - cyFrame - cyFrame;
            break;
        case WMSZ_LEFT:
            result->left = rect.left;
            result->top = (rect.top + cyFrame);
            result->right = cxFrame;
            result->bottom = height - cyFrame;
            break;
    }

    result->right += result->left;
    result->bottom += result->top;
}


#ifdef NT35_RESIZE_RECT

#define PATBLT_RECT(hdc, rect) \
    PatBlt(hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, PATINVERT);
/*              X          Y         Width                   Height */

#define RECT_EQ(rc1, rc2) (rc1.top == rc2.top && rc1.bottom == rc2.bottom && \
    rc1.left == rc2.left && rc1.right == rc2.right)

/* TODO: This doesn't work very well. Its slow and tends to leave bits of rect
 *       behind, especially when resizing at the corners. Its only here because
 *       as bad as it is, its probably better than having no resize rectangle
 *       at all.
 *
 * TODO: Also, the window size popup isn't being drawn - not an issue with
 *       the resize rect but rather a bug hiding somewhere in the custom
 *       window resize modal loop.
 *
 * This code is *only* used on Windows NT 3.x because we have to override the
 * default window size modal loop in order to emulate WM_SIZING and
 * WM_EXITSIZEMOVE which means we have to also implement the resize rectangle
 * that's normally provided by Windows.
 *
 * This function is called on button down and button up with rect == rcPrevious,
 * as well during mouse move when a WM_SIZING event is generated
 *
 * Parameters:
 *   hdc  - Device Context to draw to
 *   hwnd - The window being resized
 *   rect - The new window dimensions
 *   rcPrevious - The previous window dimensions (last time this was called)
 *   edge - the edge or edges being resized (eg, WMSZ_BOTTOMRIGHT)
 */
void DrawResizeRect(HDC hdc, HWND hwnd, RECT rect, RECT rcPrevious, UINT edge)
{
    int cxFrame, cyFrame;
    BOOL drawLeft=TRUE, drawRight=TRUE, drawTop=TRUE, drawBottom=TRUE;
    BOOL drawLeftPrev=TRUE, drawRightPrev=TRUE, drawTopPrev=TRUE, drawBottomPrev=TRUE;

    // Rect components - old and new
    RECT rcLeft, rcRight, rcTop, rcBottom;
    RECT rcLeftPrev, rcRightPrev, rcTopPrev, rcBottomPrev;
    RECT rcLeftIsect, rcRightIsect, rcTopIsect, rcBottomIsect;
    int leftIsect = 0, rightIsect = 0, topIsect = 0, bottomIsect = 0;

    // If the rect has changed from previous at all
    BOOL forceDraw = !RECT_EQ(rect, rcPrevious);

    // Get the resizing border dimensions
    cxFrame = GetSystemMetrics(SM_CXFRAME);
    cyFrame = GetSystemMetrics(SM_CYFRAME);

    // Get rect component coordinates & dimensions & intersection
    GetEdgeRect(rect, cyFrame, cxFrame, WMSZ_LEFT, &rcLeft);
    GetEdgeRect(rcPrevious, cyFrame, cxFrame, WMSZ_LEFT, &rcLeftPrev);
    leftIsect = IntersectRect(&rcLeftIsect, &rcLeftPrev, &rcLeft);

    GetEdgeRect(rect, cyFrame, cxFrame, WMSZ_RIGHT, &rcRight);
    GetEdgeRect(rcPrevious, cyFrame, cxFrame, WMSZ_RIGHT, &rcRightPrev);
    rightIsect = IntersectRect(&rcRightIsect, &rcRightPrev, &rcRight);

    GetEdgeRect(rect, cyFrame, cxFrame, WMSZ_TOP, &rcTop);
    GetEdgeRect(rcPrevious, cyFrame, cxFrame, WMSZ_TOP, &rcTopPrev);
    topIsect = IntersectRect(&rcTopIsect, &rcTopPrev, &rcTop);

    GetEdgeRect(rect, cyFrame, cxFrame, WMSZ_BOTTOM, &rcBottom);
    GetEdgeRect(rcPrevious, cyFrame, cxFrame, WMSZ_BOTTOM, &rcBottomPrev);
    bottomIsect = IntersectRect(&rcBottomIsect, &rcBottomPrev, &rcBottom);

    if (leftIsect) {
        // the right dimension will always equal as it's just the frame width

        if (rcLeft.left != rcLeftPrev.left) {
            // It's moved slightly to one side but not by its full width
            if (rcLeft.left > rcLeftPrev.left) {
                // Moved right
                rcLeft.left = rcLeftIsect.right;
                rcLeftPrev.right = rcLeftIsect.left;
            } else {
                // Moved left
                rcLeft.right = rcLeftIsect.left;
                rcLeftPrev.left = rcLeftIsect.right;
            }
        } else if (rcLeft.top != rcLeftPrev.top) {
            // It's been extended up
            drawLeftPrev = FALSE;
            rcLeft.bottom = rcLeftPrev.top;
        } else if (rcLeft.bottom != rcLeftPrev.bottom) {
            // It's been extended down
            drawLeftPrev = FALSE;
            rcLeft.top = rcLeftPrev.bottom;
        } else {
            // It hasn't moved at all. Don't redraw it or it will be erased.
            drawLeft = forceDraw;
        }
    }

    if (rightIsect) {
        // the right dimension will always equal as it's just the frame width

        if (rcRight.left != rcRightPrev.left) {
            // It's moved slightly to one side but not by its full width.
            if (rcRight.left > rcRightPrev.left) {
                // Moved right
                rcRight.left = rcRightIsect.right;
                rcRightPrev.right = rcRightIsect.left;
            } else {
                // Moved left
                rcRight.right = rcRightIsect.left;
                rcRightPrev.left = rcRightIsect.right;
            }
        } else if (rcRight.top != rcRightPrev.top) {
            // It's been extended up
            drawRightPrev = FALSE;
            rcRight.bottom = rcRightPrev.top;
        } else if (rcRight.bottom != rcRightPrev.bottom) {
            // It's been extended down
            drawRightPrev = FALSE;
            rcRight.top = rcRightPrev.bottom;
        } else {
            // It hasn't moved at all. Don't redraw it or it will be erased.
            drawRight = forceDraw;
        }
    }

    if (topIsect) {
        // The bottom dimension will always equal as it's just the frame height

        if (rcTop.top != rcTopPrev.top) {
            // It's moved up or down slightly but not by its full height
            if (rcTop.top > rcTopPrev.top) {
                // Moved up
                rcTop.top = rcTopIsect.bottom;
                rcTopPrev.bottom = rcTopIsect.top;
            } else {
                // Moved down
                rcTop.bottom = rcTopIsect.top;
                rcTopPrev.top = rcTopIsect.bottom;
            }
        } else if (rcTop.right != rcTopPrev.right) {
            // It's been extended to the right
            drawTopPrev = FALSE;
            rcTop.left = rcTopPrev.right;
        } else if (rcTop.left != rcTopPrev.left) {
            // It's been extended to the left
            drawTopPrev = FALSE;
            rcTop.right = rcTopPrev.left;
        } else {
            // It hasn't moved at all. Don't redraw it or it will be erased.
            drawTop = forceDraw;
        }
    }

    if (bottomIsect) {
        // The bottom dimension will always equal as it's just the frame height

        if (rcBottom.top != rcBottomPrev.top) {
            // It's moved up or down slightly but not by its full height
            if (rcBottom.top > rcBottomPrev.top) {
                // Moved up
                rcBottom.top = rcBottomIsect.bottom;
                rcBottomPrev.bottom = rcBottomIsect.top;
            } else {
                // Moved down
                rcBottom.bottom = rcBottomIsect.top;
                rcBottomPrev.top = rcBottomIsect.bottom;
            }
        } else if (rcBottom.right != rcBottomPrev.right) {
            // It's been extended to the right
            drawBottomPrev = FALSE;
            rcBottom.left = rcBottomPrev.right;
        } else if (rcBottom.left != rcBottomPrev.left) {
            // It's been extended to the left
            drawBottomPrev = FALSE;
            rcBottom.right = rcBottomPrev.left;
        } else {
            // It hasn't moved at all. Don't redraw it or it will be erased.
            drawBottom = forceDraw;
        }
    }

    // Draw everything!
    if (drawTopPrev)    PATBLT_RECT(hdc, rcTopPrev);
    if (drawTop)        PATBLT_RECT(hdc, rcTop);
    if (drawRightPrev)  PATBLT_RECT(hdc, rcRightPrev);
    if (drawRight)      PATBLT_RECT(hdc, rcRight);
    if (drawBottomPrev) PATBLT_RECT(hdc, rcBottomPrev);
    if (drawBottom)     PATBLT_RECT(hdc, rcBottom);
    if (drawLeftPrev)   PATBLT_RECT(hdc, rcLeftPrev);
    if (drawLeft)       PATBLT_RECT(hdc, rcLeft);

}
#endif /* NT35_RESIZE_RECT */

/* Code for emulating WM_SIZING and WM_EXITSIZEMOVE events on Windows NT 3.x
 */
int KTerminal::OnNCLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y,
                     UINT codeHitTest) {
    UINT edge;
    MSG  msg;
    RECT rc;
    BOOL track = TRUE;

    // nt351 really means NT 3.x, not specifically NT 3.51
    if (!nt351) {
        // No need to emulate WM_SIZING or WM_EXITSIZEMOVE - they're supported
        // natively.
        return FALSE;
    }

    // We don't care about double-clicks here.
    if (fDoubleClick)
        return FALSE;

    switch (codeHitTest) {
        case HTRIGHT:
            edge = WMSZ_RIGHT;
            break;
        case HTLEFT:
            edge = WMSZ_LEFT;
            break;
        case HTTOP:
            edge = WMSZ_TOP;
            break;
        case HTBOTTOM:
            edge = WMSZ_BOTTOM;
            break;
        case HTTOPLEFT:
            edge = WMSZ_TOPLEFT;
            break;
        case HTTOPRIGHT:
            edge = WMSZ_TOPRIGHT;
            break;
        case HTBOTTOMLEFT:
            edge = WMSZ_BOTTOMLEFT;
            break;
        case HTBOTTOMRIGHT:
            edge = WMSZ_BOTTOMRIGHT;
            break;

        // For any other hit test possibilities, let Windows deal with it.
        default:
            return FALSE;
    }

    GetWindowRect(hwnd, &rc);

#ifdef NT35_RESIZE_RECT
    // ------ Prepare resources for drawing the Resize Rect ------
    HBRUSH hb;
    BITMAP bm;
    HBITMAP hbm;
    HDC hdc;

    // See the KB Article Q68569 for information about how to draw the
    // resizing rectangle.  That's where this pattern comes from.
    WORD aZigzag[] = { 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA };

    // Fill out the bitmap structure for the PatBlt calls later
    bm.bmType = 0;
    bm.bmWidth = 8;
    bm.bmHeight = 8;
    bm.bmWidthBytes = 2;
    bm.bmPlanes = 1;
    bm.bmBitsPixel = 1;
    bm.bmBits = aZigzag;

    hbm = CreateBitmapIndirect(&bm);
    hb = CreatePatternBrush(hbm);

    // By specifying NULL for the HWND in GetDC(), we get the DC for the
    // entire screen.
    hdc = GetDC(NULL);
    SelectObject(hdc, hb);
    // ------ END Prepare resources for drawing the Resize Rect ------

    // Start drawing the resize rect
    DrawResizeRect(hdc, hwnd, rc, rc, edge);

#endif /* NT35_RESIZE_RECT */

    // Capture the mouse so we can receive mouse events from outside the window.
    SetCapture(hwnd);
    while (track) {
        // Go into a PeekMessage() look waiting for mouse messages. This creates
        // a modal sort of situation preventing you from switching away from the
        // resize without releasing the mouse button first.
        while (!PeekMessage(&msg, hwnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
        	WaitMessage();

        switch(msg.message) {
            case WM_MOUSEMOVE:
                POINT pt;
#ifdef NT35_RESIZE_RECT
                RECT rcPrevious = rc;
#endif /* NT35_RESIZE_RECT */

                // Adjust rc to the new window size
                pt.x = GET_X_LPARAM(msg.lParam);
                pt.y = GET_Y_LPARAM(msg.lParam);

                ClientToScreen(hwnd, &pt);

                switch(edge) {
                    case WMSZ_RIGHT:
                        rc.right = pt.x;// + rc.left;
                        break;
                    case WMSZ_BOTTOM:
                        rc.bottom = pt.y;// + rc.top;
                        break;
                    case WMSZ_BOTTOMLEFT:
                        rc.bottom = pt.y;// + rc.top;
                        rc.left = pt.x;
                        break;
                    case WMSZ_BOTTOMRIGHT:
                        rc.bottom = pt.y;// + rc.top;
                        rc.right = pt.x;// + rc.left;
                        break;
                    case WMSZ_LEFT:
                        rc.left = pt.x;
                        break;
                    case WMSZ_TOP:
                        rc.top = pt.y;
                        break;
                    case WMSZ_TOPLEFT:
                        rc.top = pt.y;
                        rc.left = pt.x;
                        break;
                    case WMSZ_TOPRIGHT:
                        rc.top = pt.y;
                        rc.right = pt.x;// + rc.left;
                        break;
                }

                KAppWin::message(hwnd, WM_USER_SIZING, edge, (LONG)&rc);

#ifdef NT35_RESIZE_RECT
                if (!RECT_EQ(rc, rcPrevious)) {
                    DrawResizeRect(hdc, hwnd, rc, rcPrevious, edge);
                }
#endif /* NT35_RESIZE_RECT */

                break;

            case WM_LBUTTONUP:
                track = FALSE;

#ifdef NT35_RESIZE_RECT
                // Erase the resize rect
                DrawResizeRect(hdc, hwnd, rc, rc, edge);
#endif /* NT35_RESIZE_RECT */

                MoveWindow( hwnd,
                            rc.left,
                            rc.top,
                            rc.right - rc.left,
                            rc.bottom - rc.top,
                            TRUE);

                KAppWin::message(hwnd, WM_USER_EXITSIZEMOVE, 0, 0);
                break;
        }

    }

#ifdef NT35_RESIZE_RECT
    // Clean up Resize Rect resources
    ReleaseDC(NULL, hdc);
    DeleteObject(hb);
    DeleteObject(hbm);
#endif /* NT35_RESIZE_RECT */

    ReleaseCapture();
    return TRUE;
}

