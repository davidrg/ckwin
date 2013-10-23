#include "kfontdlg.hxx"
#include "khwndset.hxx"
#include "ikextern.h"

extern "C" {
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
UINT APIENTRY KFontDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    Bool b = FALSE;
	KFontDialog* fontdlg = (KFontDialog*) kglob->hwndset->find( hwnd );
	if( fontdlg ) {
		b= fontdlg->message( hwnd, msg, wParam, lParam );
	}
	else if( msg == WM_INITDIALOG ) {
		CHOOSEFONT* cf = (CHOOSEFONT*)lParam;
		fontdlg = (KFontDialog*)cf->lCustData;
		b = fontdlg->message( hwnd, msg, wParam, lParam );
	}

    return (UINT)b;
}

}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KFontDialog::KFontDialog( K_GLOBAL* kg, KWin* par )
    : KWin( kg )
{
    parent = par;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KFontDialog::~KFontDialog()
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFontDialog::show( PLOGFONT logfont )
{
    CHOOSEFONT chf;
    memset( &chf, '\0', sizeof(CHOOSEFONT) );

    chf.lStructSize = sizeof(CHOOSEFONT);
    chf.hwndOwner = parent->hwnd();

    HDC hdc = GetDC( chf.hwndOwner );
    chf.hDC = CreateCompatibleDC( hdc );
    ReleaseDC( chf.hwndOwner, hdc );

    logfont->lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;

    chf.lpLogFont = logfont;
    chf.hInstance = hInst;
    chf.nFontType = SCREEN_FONTTYPE;
#ifdef COMMENT
    chf.Flags = CF_FIXEDPITCHONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT
        | CF_NOSCRIPTSEL | CF_ENABLEHOOK | CF_ENABLETEMPLATE;
#else
    chf.Flags = CF_SCALABLEONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT
        | CF_NOSCRIPTSEL | CF_ENABLEHOOK | CF_ENABLETEMPLATE;
#endif 
    chf.lCustData = (LPARAM)this;
    chf.lpfnHook = KFontDlgProc;
    chf.lpTemplateName = MAKEINTRESOURCE(IDD_FONTDIALOG);

//  chf.iPointSize;         // 10 * size in points of selected font
//  chf.rgbColors;          // returned text color
//  chf.lCustData;          // data passed to hook fn.
//  chf.lpszStyle;
//  chf.nSizeMin;           // minimum pt size allowed &
//  chf.nSizeMax;           // max pt size allowed if

    ChooseFont( &chf );

#ifdef FONT_DEBUG
    if ( logfont->lfPitchAndFamily & DEFAULT_PITCH )
        printf("Default Pitch\n");
    if ( logfont->lfPitchAndFamily & FIXED_PITCH )
        printf("Fixed Pitch\n");
    if ( logfont->lfPitchAndFamily & VARIABLE_PITCH )
        printf("Variable Pitch\n");
    if ( logfont->lfPitchAndFamily & FF_DECORATIVE )
        printf("Decorative\n");
    if ( logfont->lfPitchAndFamily & FF_DONTCARE )
        printf("Dont Care\n");
    if ( logfont->lfPitchAndFamily & FF_MODERN )
        printf("Modern\n");
    if ( logfont->lfPitchAndFamily & FF_ROMAN )
        printf("Script\n");
    if ( logfont->lfPitchAndFamily & FF_SWISS )
        printf("Swiss\n");
#endif /* FONT_DEBUG */
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KFontDialog::doCommand( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl )
{
	Bool done = TRUE;
	switch( idCtrl )
	{
		case IDC_CHECKKERMITFONTS:
		{
			// find the current state
			//
			long state = SendMessage( hCtrl, BM_GETCHECK, 0, 0 );
			HWND hfont = GetDlgItem( hPar, 1136 );
			HWND hsize = GetDlgItem( hPar, 1138 );
			if( state == BST_CHECKED ) {
				EnableWindow( hfont, FALSE );
				EnableWindow( hsize, FALSE );
			}
			else if( state == BST_UNCHECKED ) {
				EnableWindow( hfont, TRUE );
				EnableWindow( hsize, TRUE );
			}
			break;
		}

		default:
			done = FALSE;
	}

	return done;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KFontDialog::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool done = FALSE;
    switch( msg )
    {
    case WM_INITDIALOG:
        {
            associateHwnd( hWnd = hwnd );

            // fix-me should check to see what the default is
            //
            HWND hcheck = GetDlgItem( hwnd, IDC_CHECKKERMITFONTS );
            SendMessage( hcheck, BM_SETCHECK, BST_UNCHECKED, 0 );
            doCommand( hwnd, 0, IDC_CHECKKERMITFONTS, hcheck );
            break;
        }
    case WM_COMMAND:
            done = doCommand( hwnd, HIWORD(wParam), LOWORD(wParam), (HWND)lParam );
            break;
        }   
    return done;
}


