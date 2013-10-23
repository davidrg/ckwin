#include "kupload.hxx"
#include "kprop.hxx"
#include "kflstat.hxx"
#include "karray.hxx"
#include "kflname.hxx"
#include "ikextern.h"
#include "ikcmd.h"

// defines for 'setFileStats()'
const int KUPLOAD_ADD        = 0;
const int KUPLOAD_REMOVE     = 1;
const int KUPLOAD_CLEAR_ALL  = 2;

KUpload* upload;

extern "C" {
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL APIENTRY KUpDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    if( upload )
        return (BOOL) upload->message( hwnd, msg, wParam, lParam );
    return (BOOL)FALSE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
LRESULT CALLBACK KListViewProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
    if( upload )
    {
        if( msg == WM_LBUTTONDBLCLK )
            upload->dblClickListView( LOWORD(lparam), HIWORD(lparam) );

        return CallWindowProc( upload->getListViewProc()
                    , hwnd, msg, wparam, lparam );
    }
    return FALSE;
}

Bool reverseOne = FALSE;
Bool reverseTwo = FALSE;
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
int CALLBACK ListViewCompareProc( LPARAM one, LPARAM two, LPARAM sort )
{
    K_FILENAME* fn1 = (K_FILENAME*) one;
    K_FILENAME* fn2 = (K_FILENAME*) two;
    if( !fn1 || !fn2 )
        return 0;

    int ret = 0;
    if( sort == 0 ) {
        ret = lstrcmp( fn1->name, fn2->name );
		if( reverseOne ) {
			if( ret == -1 )
				ret = 1;
			else if( ret == 1 )
				ret = -1;
		}
	}
    else if ( sort == 1 ) {
        ret = lstrcmp( fn1->mode, fn2->mode );
		if( reverseTwo ) {
			if( ret == -1 )
				ret = 1;
			else if( ret == 1 )
				ret = -1;
		}
	}


    return ret;
}
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KUpload::KUpload( K_GLOBAL* kg, KFileName* kfn )
    : KWin( kg )
    , hwndpar( 0 )  // dialog box hwnd
    , hlist( 0 )    // selected files listbox hwnd
    , hcombo( 0 )
    , hfilecount( 0 )
    , hbytecount( 0 )
    , hupload( 0 )
    , numFiles( 0 )
    , totByteCount( 0 )
    , kfilename( kfn )
{
    listViewProc = 0;

    ::upload = this;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KUpload::~KUpload()
{
    ::upload = 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KUpload::showUpload()
{
    char szFilter[] = "All Files (*.*)\0*.*\0";
    char         szFile[MAX_PATH]      = "\0";
    char         szFileTitle[MAX_PATH] = "\0";

    OPENFILENAME OpenFileName;
    memset( &OpenFileName, '\0', sizeof(OPENFILENAME) );

    OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = parent->hwnd();
    OpenFileName.hInstance         = hInst;
    OpenFileName.lpstrFilter       = szFilter;
    OpenFileName.lpstrCustomFilter = (LPTSTR) NULL;
    OpenFileName.nMaxCustFilter    = 0L;
    OpenFileName.nFilterIndex      = 0L;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = sizeof(szFile);
    OpenFileName.lpstrFileTitle    = szFileTitle;
    OpenFileName.nMaxFileTitle     = sizeof(szFileTitle);
    OpenFileName.lpstrInitialDir   = "\\";  // fixme get from var
    OpenFileName.lpstrTitle        = "Upload Files";
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = "*.*";
    OpenFileName.lCustData         = 0;

    OpenFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST 
        | OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE 
        | OFN_EXPLORER;
        //| OFN_ALLOWMULTISELECT; // doesn't work properly !!!!
        // if you select too many files at once, it craps out.

    OpenFileName.lpfnHook = (LPOFNHOOKPROC)KUpDlgProc;
    OpenFileName.lpTemplateName = (LPTSTR)MAKEINTRESOURCE(IDD_UPLOAD);

    return (Bool) GetOpenFileName( &OpenFileName );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KUpload::createWin( KWin* par )
{
    parent = par;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KUpload::initDialog( HWND hwnd )
{
    // initialize handles
    //
    hWnd = hwnd;
    hwndpar     = GetParent( hwnd );    // get the real dialog box hwnd

    hlist       = GetDlgItem( hwnd, IDC_LISTSELECTEDFILES );
    hcombo      = GetDlgItem( hwnd, IDC_PROTOCOLS );
    hfilecount  = GetDlgItem( hwnd, IDC_STATICFILES );
    hbytecount  = GetDlgItem( hwnd, IDC_STATICBYTES );
    hupload     = GetDlgItem( hwndpar, IDOK );

    initListView();

    // change "Open" push button to "Upload" button
    //
    SetWindowText( hupload, "&Upload" );

    // fill the combobox with protocol choices
    //
	for( int i = 0; i < nprotos; i++ ) {
		if( !protos[i].flgs )
			SendMessage( hcombo, CB_ADDSTRING, 0, (LPARAM) (protos[i].kwd) );
	}

	// fix-me: find the current protocol for file transfer
	//
    SendMessage( hcombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );

    // adjust the tabbing so all controls are accessible
    // through the keyboard
    //
    HWND hcancel = GetDlgItem( hwndpar, IDCANCEL );

    HWND statprotocol = GetDlgItem( hWnd, IDC_STATICPROTOCOL );
    SetWindowPos( statprotocol, hcancel
        , 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );

    SetWindowPos( hcombo, statprotocol
        , 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );

    HWND statselected = GetDlgItem( hWnd, IDC_STATICSELECTED );
    SetWindowPos( statselected, hcombo
        , 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );

    SetWindowPos( hlist, statselected
        , 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );

    HWND hremove = GetDlgItem( hWnd, IDC_REMOVE );
    SetWindowPos( hremove, hlist
        , 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );

    HWND hremoveall = GetDlgItem( hWnd, IDC_REMOVEALL );
    SetWindowPos( hremoveall, hremove
        , 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );

    HWND hlogs = GetDlgItem( hWnd, IDC_LOG );
    SetWindowPos( hlogs, hremoveall
        , 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );

    HWND hoptions = GetDlgItem( hWnd, IDC_OPTIONS );
    SetWindowPos( hoptions, hlogs
        , 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KUpload::initListView()
{
    RECT rect;
    GetWindowRect( hlist, &rect );
    int vscroll = GetSystemMetrics( SM_CXVSCROLL );
    int yborder = GetSystemMetrics( SM_CYFRAME );
    int modewid = 45;
    int filewid = rect.right - rect.left - vscroll - yborder - modewid;

    LV_COLUMN lvC;				// list view column structure
    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvC.fmt = LVCFMT_LEFT;  // left-align column
    lvC.cx = filewid;            // width of column in pixels
    lvC.pszText = "Name";
    lvC.iSubItem = 0;
    ListView_InsertColumn( hlist, 0, &lvC );

    lvC.cx = modewid;            // width of column in pixels
    lvC.pszText = "Mode";
    lvC.iSubItem = 1;
    ListView_InsertColumn( hlist, 1, &lvC );

    // subclass the listview
    //
    listViewProc = (WNDPROC) SetWindowLong( hlist, GWL_WNDPROC
                        , (LONG) KListViewProc );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KUpload::fileSelect()
{
    // get the full path of the folder
    char fullpath[MAX_PATH * 2];
    CommDlg_OpenSave_GetFolderPath( hwndpar, fullpath, MAX_PATH );

    char buf[MAX_PATH];
    CommDlg_OpenSave_GetSpec( hwndpar, buf, MAX_PATH );

    // when the file is in the root directory, a '\\' has already
    // been appended to the fullpath.
    //
    int len = strlen( fullpath );
    if( fullpath[len-1] != '\\' )
        strcat( fullpath, "\\" );

    strcat( fullpath, buf );

    addFileToList( fullpath );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KUpload::addFileToList( char* file )
{
    if( !file || !lstrlen( file ) ) 
        return;

    LV_FINDINFO lvfi;
    memset( &lvfi, '\0', sizeof(LV_FINDINFO) );
    lvfi.flags = LVFI_STRING;
    lvfi.psz = file;

    int ret = ListView_FindItem( hlist, -1, &lvfi );
    if( ret != -1 )
        return;     // matching item found

    K_FILENAME* fn = kfilename->add( file, "binary" );

    LV_ITEM lvI;
	memset( &lvI, '\0', sizeof(LV_ITEM) );
    lvI.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM; //LVIF_IMAGE;
    lvI.state = 0;      
    lvI.stateMask = 0;  
    lvI.iItem = kfilename->numFiles();
    lvI.iSubItem = 0;

    lvI.pszText = LPSTR_TEXTCALLBACK; 
    lvI.cchTextMax = MAX_PATH;
    lvI.iImage = 0;
    lvI.lParam = (LONG)fn;

    int i = ListView_InsertItem( hlist, &lvI );
//    ListView_SetItemText( hlist, kfilename->numFiles()
//              , 1, LPSTR_TEXTCALLBACK );

    setFileStats( file, KUPLOAD_ADD );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KUpload::removeFile( Bool bAll )
{
    if( bAll ) {
        ListView_DeleteAllItems( hlist );
        setFileStats( 0, KUPLOAD_CLEAR_ALL );

        kfilename->removeAll();
    }
    else {
        LV_ITEM lvitem;
        int item = 0;
        Bool found = FALSE;
        K_FILENAME* fn;
        do {
            item = ListView_GetNextItem( hlist, -1, LVNI_ALL | LVNI_SELECTED );
            if( item != -1 ) {
                lvitem.iItem = item;
                lvitem.iSubItem = 0;
                lvitem.mask = LVIF_PARAM;
                ListView_GetItem( hlist, &lvitem );

                fn = (K_FILENAME*) lvitem.lParam;
                setFileStats( fn->name, KUPLOAD_REMOVE );

                kfilename->remove( fn );

                ListView_DeleteItem( hlist, item );
            }
        } while( item != -1 );

    }
}

/*------------------------------------------------------------------------
    set the number of files and the total file size
------------------------------------------------------------------------*/
void KUpload::setFileStats( char* file, int filestat )
{
    if( filestat == KUPLOAD_CLEAR_ALL ) {
        numFiles = 0;
        totByteCount = 0;
        SetWindowText( hfilecount, "0" );
        SetWindowText( hbytecount, "0" );
        return;
    }

    numFiles += (filestat == KUPLOAD_REMOVE) ? -1 : 1;

    char c[64];
    _itoa( numFiles, c, 10 );
    SetWindowText( hfilecount, c );

    SECURITY_ATTRIBUTES seca;
    seca.nLength = sizeof( SECURITY_ATTRIBUTES );
    seca.lpSecurityDescriptor = NULL;
    seca.bInheritHandle = TRUE;
                 
    HANDLE hfile = CreateFile( file, 0, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL );
    if( hfile == (HANDLE)-1 )
        return;

    DWORD filesize = GetFileSize( hfile, NULL );
    CloseHandle( hfile );
    if( filesize == (DWORD)-1 )
        return;

    if( filestat == KUPLOAD_REMOVE )
        totByteCount -= filesize;
    else
        totByteCount += filesize;

    sprintf( c, "%.0f", totByteCount );
    SetWindowText( hbytecount, c );
}

/*------------------------------------------------------------------------
    options for upload and protocol
------------------------------------------------------------------------*/
void KUpload::showPropertyPage( int command )
{
    KProperty prop( kglob );
    prop.createWin( this );
    if( command == IDC_LOG )
        prop.turnPage( ID_OPTIONS_LOGFILES );
    else if( command == IDC_OPTIONS )
        prop.turnPage( ID_OPTIONS_UPLOAD );
                // ID_OPTIONS_PROTOCOL // who had focus last ???
    prop.show();
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KUpload::initDone()
{
    // make sure the entire dialog box is visible - just center vertically
    // (system doesn't position dlg correctly along y-axis)
    //
    RECT rect;
    GetWindowRect( hwndpar, &rect );
    int h = rect.bottom - rect.top;
    int screenh = GetSystemMetrics( SM_CYSCREEN );
    int ypos = (screenh - h)/2;
    ypos = ypos < 0 ? 0 : ypos;

    SetWindowPos( hwndpar, 0, rect.left, ypos, 0, 0
        , SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KUpload::fileUpload()
{
    EndDialog( GetParent(hWnd), TRUE );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KUpload::notifyListView( LONG lParam )
{
    LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
    NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;

    switch(pLvdi->hdr.code)
    {
        case LVN_GETDISPINFO:
        {
            K_FILENAME* fn = (K_FILENAME*) pLvdi->item.lParam;
            if( pLvdi->item.iSubItem == 0 )
                pLvdi->item.pszText = fn->name;
            else
                pLvdi->item.pszText = fn->mode;
            break;
        }

		case LVN_FIRST - 77:	//0xFFFFFF4F unicode! for LVN_GETDISPINFO in NT
		{
            K_FILENAME* fn = (K_FILENAME*) pLvdi->item.lParam;
			char* text;
            if( pLvdi->item.iSubItem == 0 )
                text = fn->name;
            else 
                text = fn->mode;

			unsigned short buf[1024];
			MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, text, -1
				, buf, 1024 );
			pLvdi->item.pszText = (char*) buf;
			break;
		}

        case LVN_COLUMNCLICK:
		{
            // The user clicked a column header - sort by this criterion.
            ListView_SortItems( pNm->hdr.hwndFrom,
                                ListViewCompareProc,
                                (LPARAM)(pNm->iSubItem));

			if( pNm->iSubItem == 0 )
				reverseOne = reverseOne ? FALSE : TRUE;
			else if( pNm->iSubItem == 1 )
				reverseTwo = reverseTwo ? FALSE : TRUE;
            break;
		}
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KUpload::dblClickListView( int x, int y )
{
    int col1wid = ListView_GetColumnWidth( hlist, 0 );
    int col2wid = ListView_GetColumnWidth( hlist, 1 );
    if( x < col1wid || x > col1wid + col2wid )
        return;

    LV_HITTESTINFO lvhitinfo;
    memset( &lvhitinfo, '\0', sizeof(LV_HITTESTINFO) );
    lvhitinfo.pt.x = 10;
    lvhitinfo.pt.y = y;

    int item = ListView_HitTest( hlist, &lvhitinfo );
    if( item == -1 )
        return;

    LV_ITEM lvitem;
    lvitem.iItem = item;
    lvitem.iSubItem = 0;
    lvitem.mask = LVIF_PARAM;
    ListView_GetItem( hlist, &lvitem );

    kfilename->toggle( (K_FILENAME*) lvitem.lParam );

    ListView_Update( hlist, item );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KUpload::setProtocol()
{
    long index = SendMessage( hcombo, CB_GETCURSEL, 0, 0 );
    long len = SendMessage( hcombo, CB_GETLBTEXTLEN, index, 0 );
    char* lbtext = new char[len+1];
    SendMessage( hcombo, CB_GETLBTEXT, index, (LPARAM)lbtext );

    char buf[1024] = "set protocol ";
    strcat( buf, lbtext );
    strcat( buf, "\n" );

    ::putCmdString( buf );

    delete lbtext;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KUpload::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool done = FALSE;

    switch( msg )
    {
    case WM_INITDIALOG:
        initDialog( hwnd );
        break;

    case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
            case IDC_REMOVE:
                removeFile();
                break;
            case IDC_REMOVEALL:
                removeFile( TRUE );
                break;
            case IDC_LOG:
                showPropertyPage( IDC_LOG );
                break;
            case IDC_OPTIONS:
                showPropertyPage( IDC_OPTIONS );
                break;
                //case IDC_PROTOCOLS:
                //	setProtocol();
                //	break;
            default:
                break;
            }   
            done = TRUE;
            break;
        }

    case WM_NOTIFY:
        {
            LPNMHDR nmhdr = (LPNMHDR)lParam;
            if( nmhdr->idFrom == IDC_LISTSELECTEDFILES ) {
                notifyListView( lParam );
                break;
            }   

            LPOFNOTIFY not = (LPOFNOTIFY)lParam;
            switch( not->hdr.code )
            {
            case CDN_FILEOK:
                {
                    HWND hfocus = GetFocus();
                    if( hfocus == hupload ) {
                        // set the protocol used in the file transfer
                        //
                        setProtocol();
                        fileUpload();
                    }
                    else {
                        fileSelect();
                    }

                    // don't close the dialog
                    SetWindowLong( hwnd, DWL_MSGRESULT, 1L );
                    done = TRUE;
                    break;
                }
            case CDN_INITDONE:
                initDone();
                break;
            case CDN_SELCHANGE:
            case CDN_TYPECHANGE:
            case CDN_FOLDERCHANGE:
            default:
                break;
            }   
            break;
        }       
    }
    return done;
}
