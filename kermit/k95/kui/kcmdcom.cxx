#include "kcmdcom.hxx"

char* terminalProperties[] = {
    "character-set",
    "bytesize",
    "code-page",
    "output-pacing",
    "scrollback",
    "transmit-timeout",
    "answerback",
    "apc",
    "arrow-key",
    "cr-display",
    "debug",
    "echo",
    "keypad-mode",
    "locking-shift",
    "newline-mode",
    "roll-mode",
    "screen-update",
    "type",
    "video-change",
    "wrap",
    0
};

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdCommunications::KCmdCommunications( K_GLOBAL* g )
    : KCmdProc( g )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdCommunications::~KCmdCommunications()
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdCommunications::killActive()
{
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdCommunications::doInitDialog( HWND hwnd )
{
    KCmdProc::doInitDialog( hwnd );

    HWND hlist = GetDlgItem( hWnd, IDC_PROPLIST );
    if( !hlist )
        return TRUE;

    int i = 0;
    while( terminalProperties[i] )
    {
        SendMessage( hlist, LB_ADDSTRING, 0, (LPARAM) terminalProperties[i] );
        i++;
    }
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdCommunications::doCommand( HWND hPar, WORD code
        , WORD idCtrl, HWND hCtrl )
{
    return TRUE;
}
