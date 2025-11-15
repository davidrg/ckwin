#include "kabout.hxx"
#include "ikextern.h"
#include <ckover.h>
#include <stdio.h>

// there should only be one about box showing at a time.
//
static KAbout* kabout = 0;

extern "C" {
    extern char *ck_s_ver;
#ifdef BETATEST
    extern char *ck_s_test, *ck_s_tver;
#endif /* BETATEST */
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
extern "C" {
#ifdef _WIN64
INT_PTR
#else
BOOL
#endif
CALLBACK AboutDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    return kabout->message( hwnd, msg, wParam, lParam );
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
Bool KAbout::message( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    Bool done = FALSE;
    switch( msg )
    {
        case WM_INITDIALOG:
        {
            hWnd = hwnd;

            // Set the version string
            #define VERSION_LEN 256
            char version[VERSION_LEN];
#ifdef BETATEST
            _snprintf(version, VERSION_LEN, "Version %s %s.%s (C-Kermit %s)", K95_VERSION_MAJ_MIN_REV, ck_s_test, ck_s_tver, ck_s_ver);
#else
            _snprintf(version, VERSION_LEN, "Version %s (C-Kermit %s)", K95_VERSION_MAJ_MIN_REV, ck_s_ver);
#endif

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

