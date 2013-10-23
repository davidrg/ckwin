#include "kcmdlog.hxx"
#include "ikextern.h"

typedef struct _logFileStruct {
    HWND hwnd;
    char* logfile;
} logFileStruct;

static const int TRANSACTIONS_LOG   = 0;
static const int PACKETS_LOG        = 1;
static const int DEBUG_LOG          = 2;
static const int numLogFiles        = 3;

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdLogfiles::KCmdLogfiles( K_GLOBAL* g )
    : KCmdProc( g )
{
    files = new logFileStruct* [numLogFiles];
    for( int i = 0; i < numLogFiles; i++ ) {
        files[i] = new logFileStruct;
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdLogfiles::~KCmdLogfiles()
{
    logFileStruct* logf;
    for( int i = 0; i < numLogFiles; i++ )
    {
        logf = files[i];
        delete logf->logfile;
        delete logf;
    }
        
    delete files;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdLogfiles::killActive()
{
    return TRUE;
}

/*------------------------------------------------------------------------
    initialize the logfiles
------------------------------------------------------------------------*/
Bool KCmdLogfiles::doInitDialog( HWND hwnd )
{
    KCmdProc::doInitDialog( hwnd );

    logFileStruct* lfs = files[TRANSACTIONS_LOG];

	// set the log file names
	//
    lfs->hwnd = GetDlgItem( hWnd, IDC_EDITTRANSACTIONS );
    lfs->logfile = newstr( trafil );
    SetWindowText( lfs->hwnd, lfs->logfile );

    lfs = files[PACKETS_LOG];
    lfs->hwnd = GetDlgItem( hWnd, IDC_EDITPACKETS );
    lfs->logfile = newstr( pktfil );
    SetWindowText( lfs->hwnd, lfs->logfile );

    lfs = files[DEBUG_LOG];
    lfs->hwnd = GetDlgItem( hWnd, IDC_EDITDEBUG );
    lfs->logfile = newstr( debfil );
    SetWindowText( lfs->hwnd, lfs->logfile );

    SendDlgItemMessage( hWnd, IDC_RADIOOVERWRITETRANSACTIONS
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );
    SendDlgItemMessage( hWnd, IDC_RADIOOVERWRITEPACKETS
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );
    SendDlgItemMessage( hWnd, IDC_RADIOOVERWRITEDEBUG
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );

    //IDC_RADIOAPPENDTRANSACTIONS
    //IDC_RADIOAPPENDPACKETS
    //IDC_RADIOAPPENDDEBUG

    SendDlgItemMessage( hWnd, IDC_CHECKCLOSEAFTERTRANFERTRANSACTIONS
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );
    SendDlgItemMessage( hWnd, IDC_CHECKCLOSEAFTERTRANFERPACKETS
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );
    SendDlgItemMessage( hWnd, IDC_CHECKCLOSEAFTERTRANFERDEBUG
        , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );


    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KCmdLogfiles::browseFile( int logID )
{
    char szFilter[] = "Log Files (*.log)\0*.log\0All Files (*.*)\0*.*\0";
    char         szFile[MAX_PATH]      = "\0";
    char         szFileTitle[MAX_PATH] = "\0";

    strcpy( szFile, (files[logID])->logfile );

    OPENFILENAME OpenFileName;
    memset( &OpenFileName, '\0', sizeof(OPENFILENAME) );

    OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = hWnd;
    OpenFileName.hInstance         = hInst;
    OpenFileName.lpstrFilter       = szFilter;
    OpenFileName.lpstrCustomFilter = (LPTSTR) NULL;
    OpenFileName.nMaxCustFilter    = 0L;
    OpenFileName.nFilterIndex      = 0L;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = sizeof(szFile);
    OpenFileName.lpstrFileTitle    = szFileTitle;
    OpenFileName.nMaxFileTitle     = sizeof(szFileTitle);
    OpenFileName.lpstrInitialDir   = zgtdir();
    OpenFileName.lpstrTitle        = "Browse Log File";
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = 0; //"*.log";
    OpenFileName.lCustData         = 0;

    OpenFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST 
        | OFN_HIDEREADONLY | OFN_EXPLORER;

    if( GetSaveFileName( &OpenFileName ) ) {
        delete (files[logID])->logfile;
        (files[logID])->logfile = newstr( szFile );
        SetWindowText( (files[logID])->hwnd, szFile );
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdLogfiles::doCommand( HWND hPar, WORD code
        , WORD idCtrl, HWND hCtrl )
{
    switch( idCtrl )
    {
        case IDC_BUTTONBROWSETRANSACTIONS:
            browseFile( TRANSACTIONS_LOG );
            break;
        case IDC_BUTTONBROWSEPACKETS:
            browseFile( PACKETS_LOG );
            break;
        case IDC_BUTTONBROWSEDEBUG:
            browseFile( DEBUG_LOG );
            break;

        case IDC_RADIOOVERWRITETRANSACTIONS:
        case IDC_RADIOOVERWRITEPACKETS:
        case IDC_RADIOOVERWRITEDEBUG:
            break;
        case IDC_RADIOAPPENDTRANSACTIONS:
        case IDC_RADIOAPPENDPACKETS:
        case IDC_RADIOAPPENDDEBUG:
            break;
        case IDC_CHECKCLOSEAFTERTRANFERTRANSACTIONS:
        case IDC_CHECKCLOSEAFTERTRANFERPACKETS:
        case IDC_CHECKCLOSEAFTERTRANFERDEBUG:
            break;
    }
    return TRUE;
}
