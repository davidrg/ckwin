#include "kcmddown.hxx"
#include "ikextern.h"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdDownload::KCmdDownload( K_GLOBAL* g )
    : KCmdProc( g )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdDownload::~KCmdDownload()
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdDownload::killActive()
{
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdDownload::doInitDialog( HWND hwnd )
{
    KCmdProc::doInitDialog( hwnd );

    SendDlgItemMessage( hWnd, IDC_RADIOBINARY
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );

    SendDlgItemMessage( hWnd, IDC_RADIOSTORELITERALLY
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );

    SendDlgItemMessage( hWnd, IDC_CHECKREJECTUNKOWNCHARSET
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );

    SendDlgItemMessage( hWnd, IDC_CHECKKEEPINCOMPLETEFILES
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );

    SendDlgItemMessage( hWnd, IDC_RADIODISCARDPATHNAMES
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );

    SendDlgItemMessage( hWnd, IDC_RADIOBACKUP
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );

    SendDlgItemMessage( hWnd, IDC_SLIDERWINDOWSIZE
        , TBM_SETRANGE, (WPARAM)TRUE, MAKELONG(1,32) );

    SendDlgItemMessage( hWnd, IDC_SLIDERPACKETLENGTH
        , TBM_SETRANGE, (WPARAM)TRUE, MAKELONG(40,9024) );

	HWND hctrl = GetDlgItem( hWnd, IDC_COMBOFILECHARACTERSET );
	for( int i = 0; i < nfilc; i++ ) {
		if( !fcstab[i].flgs )
			SendMessage( hctrl, CB_ADDSTRING, 0, (LPARAM) (fcstab[i].kwd) );
	}

	hctrl = GetDlgItem( hWnd, IDC_COMBOTRANSFERCHARACTERSET );
	for(int i = 0; i < ntcs; i++ ) {
		if( !tcstab[i].flgs )
			SendMessage( hctrl, CB_ADDSTRING, 0, (LPARAM) (tcstab[i].kwd) );
	}

    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KCmdDownload::doFilenameGroup( int id )
{
    HWND hedit = GetDlgItem( hWnd, IDC_EDITSTOREAS );
    if( id == IDC_RADIOSTOREAS ) {
        EnableWindow( hedit, TRUE );
    }
    else {
        // IDC_RADIOSTORELITERALLY
        // IDC_RADIOSTORECONVERTED
        if( IsWindowEnabled( hedit ) )
            EnableWindow( hedit, FALSE );
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdDownload::doCommand( HWND hPar, WORD code
        , WORD idCtrl, HWND hCtrl )
{
    Bool done = FALSE;
    switch( idCtrl )
    {
        case IDC_RADIOSTORELITERALLY:
        case IDC_RADIOSTORECONVERTED:
        case IDC_RADIOSTOREAS:
            doFilenameGroup( idCtrl );
            break;
    }

    return done;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdDownload::doScroll( HWND hPar, WORD pos
        , WORD scrollcode, HWND hCtrl )
{
    DWORD curpos = SendMessage( hCtrl, TBM_GETPOS, 0, 0 );

    LONG ctrlID = GetWindowLong( hCtrl, GWL_ID );
    if( ctrlID == IDC_SLIDERWINDOWSIZE ) {
        SetDlgItemInt( hWnd, IDC_EDITWINDOWSIZE, curpos, TRUE );
    }
    else if( ctrlID == IDC_SLIDERPACKETLENGTH ) {
        SetDlgItemInt( hWnd, IDC_EDITPACKETLENGTH, curpos, TRUE );
    }

    return TRUE;
}
