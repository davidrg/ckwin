#include "kprop.hxx"
#include "kcmdproc.hxx"
#include "kcmdcom.hxx"
#include "kcmdup.hxx"
#include "kcmddown.hxx"
#include "kcmdlog.hxx"
#include "kcmdterm.hxx"
#include "kcmdprot.hxx"
#include "kcmdmous.hxx"
#include "kcmdscri.hxx"

#include "ikterm.h"

propPageAssoc kpropPages[] = {
    { ID_OPTIONS_MOUSE          , IDD_MOUSEOPTIONS      , "Mouse"          },
    { ID_OPTIONS_UPLOAD         , IDD_UPLOADPROPERTIES  , "Upload"         },
    { ID_OPTIONS_DOWNLOAD       , IDD_DOWNLOADPROPERTIES, "Download"       },
    { ID_OPTIONS_LOGFILES       , IDD_LOGFILEPROPERTIES , "Log Files"      },
    { ID_OPTIONS_COMMUNICATIONS , IDD_PROPERTYLIST      , "Communications" },
    { ID_OPTIONS_TERMINAL       , IDD_PROPERTYLIST      , "Terminal"       },
    { ID_OPTIONS_PROTOCOL       , IDD_PROPERTYLIST      , "Protocol"       },
    { ID_OPTIONS_SCRIPTS        , IDD_SCRIPTOPTIONS     , "Scripts"        },
    0
};

KProperty* curProp = 0;
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL APIENTRY KPropDlgProc( HWND hDlg, UINT message, UINT wParam, LONG lParam )
{
    Bool ret = FALSE;
    if( curProp )
        ret = curProp->message( hDlg, message, wParam, lParam );

    return (BOOL)ret;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KProperty::KProperty( K_GLOBAL* kg )
    : KWin( kg )
    , topPage( 0 )
    , cmdArray( 0 )
    , currentCmdProc( 0 )
{
    curProp = this;

    cmdArray = new cmdCallback* [numPropPages];
    for( int i = 0; i < numPropPages; i++ )
        cmdArray[i] = 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KProperty::~KProperty()
{
    cmdCallback* cmdcall;
    for( int i = 0; i < numPropPages; i++ )
    {
        cmdcall = cmdArray[i];
        delete cmdcall;
    }
    delete [] cmdArray;

    curProp = 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
cmdCallback* KProperty::findCmdCallback( HWND hfind )
{
    cmdCallback* callback;
    for( int i = 0; i < numPropPages; i++ )
    {
        callback = cmdArray[i];
        if( callback && callback->hwndCmd == hfind )
            return callback;
    }
    return 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KProperty::createWin( KWin* par )
{
    parent = par;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KProperty::turnPage( int tp ) 
{ 
    topPage = tp; 
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KProperty::show( Bool bVisible )
{
    // setup all the property sheets
    //
    PROPSHEETPAGE psp[numPropPages];
    for( int i = 0; i < numPropPages; i++ )
    {
        memset( &(psp[i]), '\0', sizeof(PROPSHEETPAGE) );
        psp[i].dwSize = sizeof(PROPSHEETPAGE);
        psp[i].dwFlags = PSP_USETITLE;
        psp[i].hInstance = hInst;
        psp[i].pszTemplate = MAKEINTRESOURCE(kpropPages[i].dlgid);
        psp[i].pszIcon = NULL;
        psp[i].pfnDlgProc = KPropDlgProc;
        psp[i].pszTitle = kpropPages[i].pagename;
        psp[i].lParam = (LPARAM)i;  // store index of page
    }

    // setup the property notebook
    //
    PROPSHEETHEADER psh;
    memset( &psh, '\0', sizeof( PROPSHEETHEADER ) );

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE;
    psh.hwndParent = parent->hwnd();
    psh.hInstance = hInst;
    psh.pszCaption = 0;
    psh.nPages = numPropPages;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

    // flip to the indicated page
    //
    for( i = 0; i < numPropPages; i++ )
    {
        if( kpropPages[i].command == topPage ) {
            psh.nStartPage = i;
            break;
        }
    }

    PropertySheet( &psh );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KProperty::initDialog( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    if( !hWnd )
        hWnd = GetParent( hwnd );

    PROPSHEETPAGE* psp = (PROPSHEETPAGE*) lParam;
    KCmdProc* newproc = 0;

    switch( kpropPages[psp->lParam].command )
    {
        case ID_OPTIONS_COMMUNICATIONS:
            newproc = new KCmdCommunications( kglob );
            break;
        case ID_OPTIONS_TERMINAL:
            newproc = new KCmdTerminal( kglob );
            break;
        case ID_OPTIONS_PROTOCOL:
            newproc = new KCmdProtocol( kglob );
            break;
        case ID_OPTIONS_UPLOAD:
            newproc = new KCmdUpload( kglob );
            break;
        case ID_OPTIONS_DOWNLOAD:
            newproc = new KCmdDownload( kglob );
            break;
        case ID_OPTIONS_LOGFILES:
            newproc = new KCmdLogfiles( kglob );
            break;
        case ID_OPTIONS_SCRIPTS:
            newproc = new KCmdScript( kglob );
            break;
        case ID_OPTIONS_MOUSE:
            newproc = new KCmdMouse( kglob );
            break;
    }

    cmdCallback* cc = new cmdCallback;
    cc->cmdproc = newproc;
    cc->hwndCmd = hwnd;
    cc->propsheetpage = psp;

    cmdArray[psp->lParam] = cc;
    currentCmdProc = newproc;

    Bool done = FALSE;
    if( currentCmdProc )
        done = currentCmdProc->doInitDialog( hwnd );

    return done;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KProperty::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool done = FALSE;
    switch( msg )
    {
        case WM_INITDIALOG:
            done = initDialog( hwnd, msg, wParam, lParam );
            break;

        case WM_COMMAND:
            if( currentCmdProc ) {
                done = currentCmdProc->doCommand( hwnd, HIWORD(wParam)
                        , LOWORD(wParam), (HWND)lParam );
            }
            break;

        case WM_HSCROLL:
            if( currentCmdProc ) {
                done = currentCmdProc->doScroll( hwnd, HIWORD(wParam)
                        , LOWORD(wParam), (HWND)lParam );
            }
            break;

        case WM_NOTIFY:
        {
            cmdCallback* callback = findCmdCallback( hwnd );
            if( !callback )
                break;

            switch( ((NMHDR FAR *)lParam)->code ) 
            {
                case PSN_SETACTIVE:
                {
                    // assign the current command proc
                    currentCmdProc = callback->cmdproc;

                    // set the active page's title on the titlebar
                    //
                    char buf[256] = "\0";
                    strcat( buf, kpropTitle );
                    int pageindex = callback->propsheetpage->lParam;
                    strcat( buf, kpropPages[pageindex].pagename );
                    SetWindowText( hWnd, buf );

                    SetWindowLong( hwnd, DWL_MSGRESULT, FALSE );
                    break;
                }

                case PSN_APPLY:
                {
                    Bool b = callback->cmdproc->verifyChanges();
                    SetWindowLong( hwnd, DWL_MSGRESULT, b ? FALSE : TRUE );
                    break;
                }

                case PSN_KILLACTIVE:
                {
                    Bool b = callback->cmdproc->killActive();
                    SetWindowLong( hwnd, DWL_MSGRESULT, b ? FALSE : TRUE );
                    break;
                }

                case PSN_RESET:
                    SetWindowLong( hwnd, DWL_MSGRESULT, FALSE );
                    break;
            }
            done = TRUE;
        }
    }

    return done;
}
