#include "kflstat.hxx"
#include "kflstat.h"
#include "karray.hxx"
#include "kprogres.hxx"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL APIENTRY KFlStatDlgProc( HWND hDlg, UINT message, UINT wParam, LONG lParam )
{
    Bool ret = FALSE;
    if( filestatus )
        ret = filestatus->message( hDlg, message, wParam, lParam );

    return (BOOL)ret;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KFileStatus::KFileStatus( K_GLOBAL* kg )
    : KWin( kg )
    , hcurrentdir( 0 )
    , hsending( 0 )
    , hfiletype( 0 )
    , hfilesize( 0 )
    , htimeleft( 0 )
    , htransferrate( 0 )
    , hwindowslots( 0 )
    , hgroupmsg( 0 )
    , heditmsg( 0 )
    , hspinmsg( 0 )
    , hgrouperr( 0 )
    , hediterr( 0 )
    , hspinerr( 0 )
    , hpackettype( 0 )
    , hpacketcount( 0 )
    , hpacketlength( 0 )
    , hpacketretry( 0 )
    , bytesTrans( 0L )
    , prevPacketLength( 0L )
{
    messages = new KArray();
    numMessages = 0;
    errors = new KArray();
    numErrors = 0;

    progress = new KProgress();

    // global file status object!
    filestatus = this;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KFileStatus::~KFileStatus()
{
    delete progress;

    char* c;
    long i;
    for( i = 0; i < numMessages; i++ )
    {
        c = (char*)messages->get( i );
        delete c;
    }
    delete messages;

    for( i = 0; i < numErrors; i++ )
    {
        c = (char*)errors->get( i );
        delete c;
    }
    delete errors;

    filestatus = 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFileStatus::createWin( KWin* par )
{
    parent = par;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFileStatus::show( Bool bVisible )
{
    DialogBox( hInst, MAKEINTRESOURCE( IDD_FILETRANSFERSTATUS )
        , parent->hwnd(), KFlStatDlgProc );
}

/*------------------------------------------------------------------------
    update the file status display with the passed information
------------------------------------------------------------------------*/
void KFileStatus::setProperty( UINT propid, LONG value )
{
    char c[64];
    switch( propid )
    {
        case CW_NAM:   /* Filename */
        {
            char* name = (char*)value;
            int namelen = strlen( name );
            if( namelen ) {
                int len = GetWindowTextLength( hsending );
                char* tmp = new char[len + namelen + 6];  // 6 for the arrow
                tmp[0] = '\0';
           //
           // fix-me: when sending multiple files, not blanking out correctly
           //
           //     if( len ) { 
           //         // text is null, no need to get it
           //         //
           //         GetWindowText( hsending, tmp, len + 1 );
           //         strcat( tmp, " => " );
           //     }
           //
                strcat( tmp, name );
                SetWindowText( hsending, tmp );
                delete tmp;
            }
            else
                SetWindowText( hsending, "" );
            break;
        }
        case CW_DIR:   /* Current directory */
            SetWindowText( hcurrentdir, (char*)value );
            break;
        case CW_TYP:   /* File type */
            SetWindowText( hfiletype, (char*)value );
            break;
        case CW_SIZ:   /* File size */
            _ltoa( value, c, 10 );
            SetWindowText( hfilesize, c );
            break;
        case CW_PCD:   /* Percent done */
            progress->paint( value );
            break;
        case CW_TR:    /* Time remaining */
            SetWindowText( htimeleft, (char*)value );
            break;
        case CW_CP:    /* Chars per sec */
            _ltoa( value, c, 10 );
            SetWindowText( htransferrate, c );
            break;
        case CW_WS:    /* Window slots */
            SetWindowText( hwindowslots, (char*)value );
            break;
        case CW_PT:    /* Packet type */
            SetWindowText( hpackettype, (char*)value );
            break;
        case CW_PC:    /* Packet count */
            _ltoa( value, c, 10 );
            SetWindowText( hpacketcount, c );
            break;
        case CW_FFC:
            bytesTrans = value;
            _ltoa( bytesTrans, c, 10 );
            SetWindowText( hbytestrans, c );
            break;
        case CW_PL:    /* Packet length */
            prevPacketLength = value;
            _ltoa( value, c, 10 );
            SetWindowText( hpacketlength, c );
            break;
        case CW_PR:    /* Packet retry */
            _itoa( value, c, 10 );
            SetWindowText( hpacketretry, c );
            break;
        case CW_ERR:   /* Error message */
        {
            char* cc = (char*)value;
            if( *cc == '\0' )   // don't add blank messages
                break;

            char* msg = newstr( cc );
            SetWindowText( hediterr, msg );
            errors->put( msg, numErrors++ );
            char tmp[64];
            sprintf( tmp, "Errors: %d of %d", numErrors, numErrors );
            SetWindowText( hgrouperr, tmp );
            SendMessage( hspinerr, UDM_SETRANGE
                    , 0L, MAKELONG(0,numErrors-1) );
            SendMessage( hspinerr, UDM_SETPOS
                    , 0L, MAKELONG((short)numErrors-1,0) );
            break;
        }
        case CW_MSG:   /* Info message */
        {
            char* cc = (char*)value;
            if( *cc == '\0' )   // don't add blank messages
                break;

            char* msg = newstr( cc );
            SetWindowText( heditmsg, msg );
            messages->put( msg, numMessages++ );
            char tmp[64];
            sprintf( tmp, "Messages: %d of %d", numMessages, numMessages );
            SetWindowText( hgroupmsg, tmp );
            SendMessage( hspinmsg, UDM_SETRANGE
                    , 0L, MAKELONG(0,numMessages-1) );
            SendMessage( hspinmsg, UDM_SETPOS
                    , 0L, MAKELONG((short)numMessages-1,0) );
            break;
        }
        case CW_PAR:   /* Parity */
            break;
        case CW_LIN:   /* Communication device */
            break;
        case CW_SPD:   /* Communication speed */
            break;
    }
}

/*------------------------------------------------------------------------
    KFileStatus is self cleaning when the dialog is dismissed
------------------------------------------------------------------------*/
void KFileStatus::close()
{
    EndDialog( hWnd, TRUE );
    delete this;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KFileStatus::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    Bool done = FALSE;
    switch( msg )
    {
        case WM_INITDIALOG:
        {
            hWnd = hwnd;
            HWND hprogress = GetDlgItem( hWnd, IDC_PROGRESS );
            progress->createWin( hWnd, hprogress );
            ShowWindow( hprogress, SW_HIDE );

            hcurrentdir   = GetDlgItem( hWnd, IDC_STATICCURRENTDIRECTORY );
            hsending      = GetDlgItem( hWnd, IDC_STATICSENDING );
            hfiletype     = GetDlgItem( hWnd, IDC_STATICFILETYPE );
            hfilesize     = GetDlgItem( hWnd, IDC_STATICFILESIZE );
            htimeleft     = GetDlgItem( hWnd, IDC_STATICTIMELEFT );
            htransferrate = GetDlgItem( hWnd, IDC_STATICTRANSFERRATE );
            hwindowslots  = GetDlgItem( hWnd, IDC_STATICWINDOWSLOTS );
            hgroupmsg     = GetDlgItem( hWnd, IDC_GROUPMESSAGE );
            heditmsg      = GetDlgItem( hWnd, IDC_EDITMESSAGE );
            hspinmsg      = GetDlgItem( hWnd, IDC_SPINMESSAGE );
            hgrouperr     = GetDlgItem( hWnd, IDC_GROUPERROR );
            hediterr      = GetDlgItem( hWnd, IDC_EDITERROR );
            hspinerr      = GetDlgItem( hWnd, IDC_SPINERROR );
            hpackettype   = GetDlgItem( hWnd, IDC_STATICPACKETTYPE );
            hpacketcount  = GetDlgItem( hWnd, IDC_STATICPACKETCOUNT );
            hpacketlength = GetDlgItem( hWnd, IDC_STATICPACKETLENGTH );
            hpacketretry  = GetDlgItem( hWnd, IDC_STATICPACKETRETRY );
            hbytestrans   = GetDlgItem( hWnd, IDC_STATICBYTESTRANSFERED );

            // initialize the spin buttons
            //
            SendMessage( hspinmsg, UDM_SETRANGE, 0L, MAKELONG(0,0) );
            SendMessage( hspinerr, UDM_SETRANGE, 0L, MAKELONG(0,0) );


            // animation control ???
            //        HWND hanimation = GetDlgItem(hwnd, IDC_ANIMATION);
            //        //Animate_Stop( hanimation );
            //        Animate_Open( hanimation, MAKEINTRESOURCE(IDR_AVICLIP) );
            //        Animate_Play( hanimation, 0, -1, -1 );

            // set the title
            //
            int len = strlen(kglob->hostName) + 23;
            char* c = new char[len+1];
            strcpy( c, kglob->hostName );
            strcat( c, " - File Transfer Status" );
            SetWindowText( hWnd, c );
            delete c;
        }
        case WM_PAINT:
            progress->paint( -1 );
            break;

        case WM_COMMAND:
        {
            if( HIWORD(wParam) == 0 ) {  // from menu
                if( LOWORD( wParam ) == WM_DESTROY ) {
                    close();
                    done = TRUE;
                    break;
                }
            }
            switch( LOWORD(wParam) )
            {
                case IDC_BUTTONEXIT:
                    close();
                    break;
                case IDC_BUTTONCANCELFILE:
                    break;
                case IDC_BUTTONCANCELGROUP:
                    break;
            }
            done = TRUE;
            break;
        }

        case WM_VSCROLL:
        {
            if( LOWORD(wParam) == SB_THUMBPOSITION )
            {
                // spin button messages
                //
                int pos = HIWORD(wParam);
                char tmp[64];
                if( (HWND)lParam == hspinmsg ) {
                    char* c = (char*)messages->get( pos );
                    SetWindowText( heditmsg, c );
                    sprintf( tmp, "Messages: %d of %d", pos + 1, numMessages );
                    SetWindowText( hgroupmsg, tmp );
                }
                else if( (HWND)lParam == hspinerr ) {
                    char* c = (char*)errors->get( pos );
                    SetWindowText( hediterr, c );
                    sprintf( tmp, "Errors: %d of %d", pos + 1, numErrors );
                    SetWindowText( hgrouperr, tmp );
                }
            }
            break;
        }
    }
    return done;
}

