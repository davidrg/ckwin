#include "kcmdup.hxx"
#include "ikextern.h"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdUpload::KCmdUpload( K_GLOBAL* g )
    : KCmdProc( g )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdUpload::~KCmdUpload()
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdUpload::killActive()
{
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdUpload::doInitDialog( HWND hwnd )
{
    KCmdProc::doInitDialog( hwnd );

    SendDlgItemMessage( hWnd, IDC_RADIOBINARY
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );

    SendDlgItemMessage( hWnd, IDC_RADIOSENDLITERALLY
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );

    SendDlgItemMessage( hWnd, IDC_RADIOSENDWITH
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );

    SendDlgItemMessage( hWnd, IDC_RADIOFILE
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );

    HWND hslider = GetDlgItem( hWnd, IDC_SLIDERWINDOWSIZE );
    SendMessage( hslider, TBM_SETRANGE, (WPARAM)TRUE, MAKELONG(1,32) );

	HWND hctrl = GetDlgItem( hWnd, IDC_COMBOFILECHARACTERSET );
	for( int i = 0; i < nfilc; i++ ) {
		if( !fcstab[i].flgs )
			SendMessage( hctrl, CB_ADDSTRING, 0, (LPARAM) (fcstab[i].kwd) );
	}

	hctrl = GetDlgItem( hWnd, IDC_COMBOTRANSFERCHARACTERSET );
	for( i = 0; i < ntcs; i++ ) {
		if( !tcstab[i].flgs )
			SendMessage( hctrl, CB_ADDSTRING, 0, (LPARAM) (tcstab[i].kwd) );
	}

    return TRUE;
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KCmdUpload::doFilenameGroup( int id )
{
    HWND hedit = GetDlgItem( hWnd, IDC_EDITSENDAS );
    if( id == IDC_RADIOSENDAS ) {
        EnableWindow( hedit, TRUE );
    }
    else {
        // IDC_RADIOSENDCONVERTED
        // IDC_RADIOSENDLITERALLY
        if( IsWindowEnabled( hedit ) )
            EnableWindow( hedit, FALSE );
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KCmdUpload::doDispositionGroup( int id )
{
    HWND hemail = GetDlgItem( hWnd, IDC_EDITEMAILADDRESS );
    HWND hprinter = GetDlgItem( hWnd, IDC_EDITPRINTER );
    HWND hprnOptions = GetDlgItem( hWnd, IDC_EDITPRINTEROPTIONS );

    Bool emailEnabled = IsWindowEnabled( hemail );
    Bool printerEnabled = IsWindowEnabled( hprinter );
    Bool prnOptionsEnabled = IsWindowEnabled( hprnOptions );

    switch( id )
    {
        case IDC_RADIOFILE:
            if( emailEnabled )
                EnableWindow( hemail, FALSE );
            if( printerEnabled ) {
                EnableWindow( hprinter, FALSE );
                EnableWindow( hprnOptions, FALSE );
            }
            break;
        case IDC_RADIOEMAILADDRESS:
            EnableWindow( hemail, TRUE );
            if( printerEnabled ) {
                EnableWindow( hprinter, FALSE );
                EnableWindow( hprnOptions, FALSE );
            }
            break;
        case IDC_RADIOPRINTER:
            if( emailEnabled )
                EnableWindow( hemail, FALSE );
            EnableWindow( hprinter, TRUE );
            EnableWindow( hprnOptions, TRUE );
            
            break;
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdUpload::doCommand( HWND hPar, WORD code
        , WORD idCtrl, HWND hCtrl )
{
    Bool done = FALSE;
    switch( idCtrl )
    {
        case IDC_RADIOSENDAS:
        case IDC_RADIOSENDCONVERTED:
        case IDC_RADIOSENDLITERALLY:
            doFilenameGroup( idCtrl );
            done = TRUE;
            break;

        case IDC_RADIOFILE:
        case IDC_RADIOEMAILADDRESS:
        case IDC_RADIOPRINTER:
            doDispositionGroup( idCtrl );
            done = TRUE;
            break;
    }

    return done;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdUpload::doScroll( HWND hPar, WORD pos
        , WORD scrollcode, HWND hCtrl )
{
    LONG ctrlID = GetWindowLong( hCtrl, GWL_ID );
    if( ctrlID == IDC_SLIDERWINDOWSIZE )
    {
        DWORD curpos = SendMessage( hCtrl, TBM_GETPOS, 0, 0 );
        SetDlgItemInt( hWnd, IDC_EDITWINDOWSIZE, curpos, TRUE );
    }

    return TRUE;
}

