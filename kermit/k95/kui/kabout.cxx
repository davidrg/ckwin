#include "kabout.hxx"
#include "ikextern.h"

// there should only be one about box showing at a time.
//
static KAbout* kabout = 0;

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
extern "C" {
BOOL CALLBACK AboutDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    return (BOOL)kabout->message( hwnd, msg, wParam, lParam );
}
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KAbout::KAbout( K_GLOBAL* kg, KWin* par )
    : KWin( kg )
    , anotherIsRunning( FALSE )
{
    // make sure there's only one instance of this object running
    //
    if( !kabout ) {
        parent = par;
        kabout = this;
    }
    else {
        parent = 0;
        anotherIsRunning = TRUE;
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KAbout::~KAbout()
{
    if( !anotherIsRunning )
        kabout = 0;
}
    
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KAbout::show( Bool bVisible )
{
    if( anotherIsRunning ) {
        kabout->takeFocus();
        return;
    }
    
    DialogBox( hInst
        , MAKEINTRESOURCE(IDD_ABOUTBOX)
        , parent->hwnd()
        , AboutDlgProc );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KAbout::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool done = FALSE;
    switch( msg )
    {
        case WM_INITDIALOG:
        {
            hWnd = hwnd;

            LONG style;
            HWND hctrl;

            // set the registration info
            //
            int dsn = isregistered(NULL);

            if ( dsn == 91340 || dsn == 23567 || dsn == 12496 ) {
                char* p = "THIS COPY OF KERMIT SOFTWARE HAS BEEN PIRATED.";
                hctrl = GetDlgItem( hwnd, IDC_STATICREG1 );
                SetWindowText( hctrl, p );

                p = "";
                hctrl = GetDlgItem( hwnd, IDC_STATICREG2 );
                SetWindowText( hctrl, p );

                p = "";
                hctrl = GetDlgItem( hwnd, IDC_STATICREG3 );
                SetWindowText( hctrl, p );
            }
            else if ( dsn == 99 || dsn == 98        /* Demo */
                  || dsn >= 100 && dsn < 10000       /* Academic Site License */
                  || dsn >= 10000 && dsn < 10000000  /* Retail Package */
                  || dsn >= 10000000                 /* Commercial Site License */
                  )
        {
            char* p = get_reg_name();
            hctrl = GetDlgItem( hwnd, IDC_STATICREG1 );
            SetWindowText( hctrl, p );

            p = get_reg_corp();
            hctrl = GetDlgItem( hwnd, IDC_STATICREG2 );
            SetWindowText( hctrl, p );

            p = get_reg_sn();
            hctrl = GetDlgItem( hwnd, IDC_STATICREG3 );
            SetWindowText( hctrl, p );
        } else {
            char* p = "Unregistered";
            hctrl = GetDlgItem( hwnd, IDC_STATICREG1 );
            SetWindowText( hctrl, p );

            p = "";
            hctrl = GetDlgItem( hwnd, IDC_STATICREG2 );
            SetWindowText( hctrl, p );

            p = "";
            hctrl = GetDlgItem( hwnd, IDC_STATICREG3 );
            SetWindowText( hctrl, p );

        }
            hctrl = GetDlgItem( hwnd, IDC_STATICSUNKEN1 );
            style = GetWindowLong( hctrl, GWL_STYLE ) | WS_CLIPSIBLINGS;
            SetWindowLong( hctrl, GWL_STYLE, style );
            hctrl = GetDlgItem( hwnd, IDC_STATICSUNKEN2 );
            style = GetWindowLong( hctrl, GWL_STYLE ) | WS_CLIPSIBLINGS;
            SetWindowLong( hctrl, GWL_STYLE, style );
            hctrl = GetDlgItem( hwnd, IDOK );
            SetFocus( hctrl );
            done = TRUE;
            break;
        }

        case WM_COMMAND:
        case WM_CLOSE:
            EndDialog( hwnd, TRUE );
            done = TRUE;
            break;
    }

    return done;
}

