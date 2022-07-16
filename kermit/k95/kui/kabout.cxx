#include "kabout.hxx"
#include "ikextern.h"
#include <stdio.h>

// there should only be one about box showing at a time.
//
static KAbout* kabout = 0;

extern "C" {
    extern char *ck_s_ver;
}

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

            // Set the version string
            #define VERSION_LEN 128
            char version[VERSION_LEN];
            _snprintf(version, VERSION_LEN, "Version %s", ck_s_ver);

            SendDlgItemMessage(hWnd, IDC_VERSION, WM_SETTEXT, 0,
                    (LPARAM) version);

            HWND hctrl;

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

