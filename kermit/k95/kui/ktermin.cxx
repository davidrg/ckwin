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

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KTerminal::KTerminal( K_GLOBAL* kg )
    : KAppWin( kg, kg->noClose ? IDR_TERMMENU_NOCLOSE : IDR_TERMMENU, IDR_TOOLBARTERMINAL ),
    toolbar_disabled(FALSE), menuInitialized(FALSE), firstShow(TRUE)
{
    client = new KClient( kg, VTERM );
    setClient( client );

	// Disable toolbar button & menu item for dialer
	if (!DialerExists()) {
		// Disable toolbar button
		BYTE state = tbButtons[1].tbbutton.fsState;
		state = state & ~TBSTATE_ENABLED;
		tbButtons[1].tbbutton.fsState = state;

		// And anything else in the GUI that starts the dialer
		noDialer = TRUE;
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
    if ( toolbar )
        toolbar->setTermType( tt );
}

void KTerminal::setRemoteCharset( char* cs )
{
    if ( toolbar )
        toolbar->setCharset( cs );
}

void KTerminal::setFont( char* fnt, int h )
{
    if ( toolbar ) {
        toolbar->setFontName( fnt );
        toolbar->setFontHeight( h );
    }
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
    if ( toolbar ) {
        toolbar->initButtons( numButtons, numBitmaps, &(*tbButtons) );
        toolbar->createWin( this );
        toolbar->createTermTypeCombo();
        toolbar->createFontNameCombo();
        toolbar->createFontHeightCombo();
        toolbar->createCharsetCombo();
        toolbar->getSize( tbwid, tbhi );
    }

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
        | OFN_HIDEREADONLY | OFN_EXPLORER;

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
    case WM_NOTIFY:
        debug(F111,"KTerminal::message","WM_NOTIFY",msg);
        if ( toolbar && !toolbar_disabled)
            done = toolbar->message( hwnd, msg, wParam, lParam );
        else 
            done = TRUE;
        break;

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

    case WM_ACTIVATE:
    case WM_SIZING:
    case WM_SIZE:
    case WM_EXITSIZEMOVE:
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
        if ( lParam & ENDSESSION_LOGOFF ) {
            debug(F100,"ENDSESSION_LOGOFF","",0);
            if ( startflags & 128 ) {
                debug(F100,"startflags & 128","",0);
                done = FALSE;
            }
        }
        break;

    case WM_ENDSESSION:
        debug(F111,"KTerminal::message","WM_QUERYENDSESSION",msg);
        if ( wParam && (lParam & ENDSESSION_LOGOFF) ) {
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
                    KDownLoad download( kglob );
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

            case ID_HELP_K95MANUAL:
                readManual();
                break;
            case ID_WEB_KERMIT:
                browse("http://www.kermit-project.org/");
                break;
            case ID_WEB_K95:
                browse("http://www.kermit-project.org/k95.html");
                break;
            case ID_WEB_NEWS:
                browse("http://www.kermit-project.org/k95news.html");
                break;
            case ID_WEB_PATCH:
                browse("http://www.kermit-project.org/k95upgrade.html");
                break;
            case ID_WEB_SUPPORT:
                browse("http://www.kermit-project.org/support.html");
                break;
            case ID_WEB_FAQ:
                browse("http://www.kermit-project.org/k95faq.html");
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
