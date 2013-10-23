#include "kcmdscri.hxx"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdScript::KCmdScript( K_GLOBAL* g )
    : KCmdProc( g )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdScript::~KCmdScript()
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdScript::killActive()
{
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdScript::doInitDialog( HWND hwnd )
{
    KCmdProc::doInitDialog( hwnd );

    HWND hslider = GetDlgItem( hWnd, IDC_SLIDEROUTPUTPACING );
    SendMessage( hslider, TBM_SETRANGE, (WPARAM)TRUE, MAKELONG(0,10000) );
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdScript::doCommand( HWND hPar, WORD code
        , WORD idCtrl, HWND hCtrl )
{
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdScript::doScroll( HWND hPar, WORD pos
        , WORD scrollcode, HWND hCtrl )
{
    DWORD curpos = SendMessage( hCtrl, TBM_GETPOS, 0, 0 );
    SetDlgItemInt( hWnd, IDC_EDITOUTPUTPACING, curpos, TRUE );

    return TRUE;
}
