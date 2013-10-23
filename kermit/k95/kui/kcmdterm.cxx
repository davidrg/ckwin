#include "kcmdterm.hxx"
#include "kcmd.h"
#include "kdefs.h"
#include "ikextern.h"
#include "ikcmd.h"
#include "karray.hxx"

typedef struct _HwndDlgID {
	long dlgid;
	HWND hwnd;
} HwndDlgID;

KCmdTerminal* cmdterm = 0;
extern "C" {
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL CALLBACK childDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	BOOL b = FALSE;
	if( cmdterm )
		b = cmdterm->advMessage( hwnd, msg, wParam, lParam );
	return b;
}
}

// number of controls on the property page
//
static const int NUM_CONTROLS = 17;

// the starting index of the actual controls in the list
//
static const int CONTROL_START_IDX = 2;

static int controls[NUM_CONTROLS] = {
    0,              // command string associated with the choice
    IDC_PROPLIST,   // property listbox
    IDC_PROPEDIT,
    IDC_PROPCOMBO,
    IDC_PROPRADIO1,
    IDC_PROPRADIO2,
    IDC_PROPRADIO3,
    IDC_PROPRADIO4,
    IDC_PROPRADIO5,
    IDC_PROPRADIO6,
    IDC_PROPRADIO7,
    IDC_PROPRADIO8,
    IDC_STATICCOMBO1,
    IDC_STATICCOMBO2,
    IDC_PROPCOMBO2,
    IDC_PROPCOMBO3,
	IDD_FONTADVANCED
};

/* NOTE: We should add "SET TERMINAL ESCAPE" as synonym for SET ESCAPE */

static k_cell actionList[][NUM_CONTROLS] = {
{ {TERM_ESCAPE,         ""}, {0,"escape-character"     },{2,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
//{ {0,                   ""}, {0,"key"                  },{2,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_ANSWER,         "terminal"},{0,"answerback"           },{0,0}, {0,0}, {1,"on"}, {1,"off"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_APC,            "terminal"},{0,"apc"                  },{0,0}, {0,0}, {1,"on"}, {1,"off"}, {1,"unchecked"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_ARROW,          "terminal"},{0,"arrow-keys"           },{0,0}, {0,0}, {1,"application"}, {1,"cursor"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_AUTODL,         "terminal"},{0,"autodownload"         },{0,0}, {0,0}, {1,"on"}, {1,"off"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_BELL,           "terminal"},{0,"bell"                 },{0,0}, {0,0}, {1,"none"}, {1,"visible"}, {1,"beep"}, {1,"system sounds"}, {1,"(unknown)"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_BYTESIZE,       "terminal"},{0,"bytesize"             },{2,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
//{ {TERM_CHAR_LOCAL,     "terminal"},{0,"character-set local"  },{0,0}, {3,(char*)K_TERMCHARSET}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
//{ {TERM_CHAR_REMOTE,    "terminal"},{0,"character-set remote" },{0,0}, {3,(char*)K_TRANSFERCHARSET}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_CHAR_REMOTE,    "terminal"},{0,"character-set remote" },{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {4,(char*)IDD_FONTADVANCED} },
{ {TERM_CODEPAGE,       "terminal"},{0,"code-page"            },{2,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_COLOR_BORDER,   "terminal"},{0,"color border"         },{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {1,"foreground"}, {0,0}, {3,(char*)K_COLORSET}, {0,0} },
{ {TERM_COLOR_HELP,     "terminal"},{0,"color help-text"      },{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {1,"foreground"}, {1,"background"}, {3,(char*)K_COLORSET}, {3,(char*)K_COLORSET} },
{ {TERM_COLOR_SELECTION,"terminal"},{0,"color selection"      },{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {1,"foreground"}, {1,"background"}, {3,(char*)K_COLORSET}, {3,(char*)K_COLORSET} },
{ {TERM_COLOR_STATUS,   "terminal"},{0,"color status-line"    },{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {1,"foreground"}, {1,"background"}, {3,(char*)K_COLORSET}, {3,(char*)K_COLORSET} },
{ {TERM_COLOR_TERM,     "terminal"},{0,"color terminal-screen"},{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {1,"foreground"}, {1,"background"}, {3,(char*)K_COLORSET}, {3,(char*)K_COLORSET} },
{ {TERM_COLOR_UNDERLINE,"terminal"},{0,"color underlined-text"},{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {1,"foreground"}, {1,"background"}, {3,(char*)K_COLORSET}, {3,(char*)K_COLORSET} },
{ {TERM_CRD,            "terminal"},{0,"cr-display"           },{0,0}, {0,0}, {1,"crlf"}, {1,"normal"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_CURSOR,         "terminal"},{0,"cursor"               },{0,0}, {0,0}, {1,"full"}, {1,"half"}, {1,"underline"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_DEBUG,          "terminal"},{0,"debug"                },{0,0}, {0,0}, {1,"on"}, {1,"off"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_ECHO,           "terminal"},{0,"echo"                 },{0,0}, {0,0}, {1,"local"}, {1,"remote"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_HEIGHT,         "terminal"},{0,"height"               },{2,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_KEYPAD,         "terminal"},{0,"keypad-mode"          },{0,0}, {0,0}, {1,"application"}, {1,"numeric"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_SOSI,           "terminal"},{0,"locking-shift"        },{0,0}, {0,0}, {1,"on"}, {1,"off"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_NLM,            "terminal"},{0,"newline-mode"         },{0,0}, {0,0}, {1,"on"}, {1,"off"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_PACING,         "terminal"},{0,"output-pacing"        },{2,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_ROLL,           "terminal"},{0,"roll-mode"            },{0,0}, {0,0}, {1,"insert"}, {1,"overwrite"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_UPD_MODE,       "terminal"},{0,"screen-update"        },{0,0}, {0,0}, {1,"fast"}, {1,"smooth"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_SCRSIZE,        "terminal"},{0,"scrollback"           },{2,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_CTSTMO,         "terminal"},{0,"transmit-timeout"     },{2,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_NAME,           "terminal"},{0,"type"                 },{0,0}, {3,(char*)K_TERMTYPESET }, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_VIDEO_CHANGE,   "terminal"},{0,"video-change"         },{0,0}, {0,0}, {1,"enabled"}, {1,"disabled"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_WIDTH,          "terminal"},{0,"width"                },{2,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
{ {TERM_WRAP,           "terminal"},{0,"wrap"                 },{0,0}, {0,0}, {1,"on"}, {1,"off"}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },
    0
};



/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdTerminal::KCmdTerminal( K_GLOBAL* g )
    : KCmdProc( g )
{
    hlist = 0;
    lastAction = -1;
	dlgArray = 0;
	dlgArrayCount = 0;
	cmdterm = this;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdTerminal::~KCmdTerminal()
{
	HwndDlgID* hid;
	for( long i = 0; i < dlgArrayCount; i++ ) {
		hid = (HwndDlgID*)dlgArray->get( i );
		DestroyWindow( hid->hwnd );
		delete hid;
	}
	cmdterm = 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdTerminal::doInitDialog( HWND hwnd )
{
    KCmdProc::doInitDialog( hwnd );

    hlist = GetDlgItem( hWnd, IDC_PROPLIST );
    if( !hlist )
        return TRUE;

    int i = 0;
    k_cell* ca = &(actionList[i][1]);
    while( ca->text )
    {
        SendMessage( hlist, LB_ADDSTRING, 0, (LPARAM) ca->text );
        i++;
        ca = &(actionList[i][1]);
    }

    PostMessage( hlist, LB_SETCURSEL, (WPARAM) 0, (LPARAM) 0 );
    doAction( 0 );

    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdTerminal::doCommand( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl )
{
    switch( code )
    {
        case LBN_SELCHANGE:
        {
            if( hCtrl == hlist ) {
                int sel = (int)SendMessage( hlist, LB_GETCURSEL, 0, 0 );
                doAction( sel );
            }
            break;
        }
    
    }

    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdTerminal::killActive()
{
    // make sure the current action showing is saved
    if( lastAction != -1 ) {
        saveAction( lastAction );
    }
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
char* KCmdTerminal::getCurrentVal( HWND hctrl )
{
    int textlen = GetWindowTextLength( hctrl );
    if( !textlen ) {
        return 0;
    }

    char* c = new char[ textlen + 1 ];
    memset( c, '\0', textlen + 1 );
    GetWindowText( hctrl, c, textlen + 1 );
    return c;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KCmdTerminal::saveAction( int action )
{
    k_cell* ac = &(actionList[action][0]);
    k_cell* pcell;

    char* curval = 0;
    HWND hctrl = 0;
    Bool done = FALSE;
    for( int i = CONTROL_START_IDX; i < NUM_CONTROLS && !done; i++ )
    {
        pcell = &( ac[i] );
        if( pcell->action > ACTION_NONE )
        {
            hctrl = GetDlgItem( hWnd, controls[i] );

            switch( pcell->action )
            {
                case ACTION_SHOW_BUTTON:    // check if it's selected
                {
                    if( SendMessage( hctrl, BM_GETCHECK, 0, 0 ) ) {
                        curval = getCurrentVal( hctrl );
                        done = TRUE;
                    }
                    break;
                }
                case ACTION_SHOW_EDIT:      // check for changed text
                {
                    curval = getCurrentVal( hctrl );
                    done = TRUE;
                    break;
                }
                case ACTION_SHOW_COMBO:     // check for current selection
                {
                    curval = getCurrentVal( hctrl );
                    done = TRUE;
                    break;
                }
            }
        }
    }

    // add the new value of the property
    if( curval ) {
        pcell = &( ac[0] );

        // check if the value is different from the previous value
        //
        char* prevval = ::getVar( pcell->action );
        if( prevval && _stricmp( prevval, curval ) == 0 ) {
            delete curval;
            return;
        }

        char cmd[512];
        memset( cmd, '\0', 512 );
        strcat( cmd, "set " );

        // first position is the group name
        strcat( cmd, pcell->text );
        strcat( cmd, " " );

        // second position is the name of the property
        pcell = &( ac[1] );
        strcat( cmd, pcell->text );
        strcat( cmd, " " );

        strcat( cmd, curval );
        strcat( cmd, "\r" );
        bufferCommand( cmd );
        delete curval;
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KCmdTerminal::showDialog( long dlgid )
{
	HwndDlgID* hid;
	for( long i = 0; i < dlgArrayCount; i++ ) {
		hid = (HwndDlgID*)dlgArray->get( i );
		if( hid->dlgid == dlgid ) {
			ShowWindow( hid->hwnd, SW_SHOWNORMAL );
			return;
		}
	}

	hid = new HwndDlgID;
	hid->dlgid = dlgid;
	hid->hwnd = CreateDialog( hInst, MAKEINTRESOURCE(dlgid)
		, hWnd, childDlgProc );

	if( !dlgArray )
		dlgArray = new KArray();
	dlgArray->put( (void*)hid, dlgArrayCount++ );

	SetParent( hid->hwnd, hWnd );
	ShowWindow( hid->hwnd, SW_SHOWNORMAL );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KCmdTerminal::hideDialog( long dlgid )
{
	HwndDlgID* hid;
	for( long i = 0; i < dlgArrayCount; i++ ) {
		hid = (HwndDlgID*)dlgArray->get( i );
		if( hid->dlgid == dlgid ) {
			ShowWindow( hid->hwnd, SW_HIDE );
			return;
		}
	}
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KCmdTerminal::doAction( int action )
{
    if( lastAction == action )
        return;

    k_cell *acundo = 0, *ac = 0, *pcell = 0, *pcellundo = 0;
    HWND hctrl = 0, hctrlundo = 0;
    int i = 0, ctrlid = 0;

    if( lastAction != -1 ) {
        // write the previous action to the queue
        //
        acundo = &(actionList[lastAction][0]);
        saveAction( lastAction );
    }
    ac = &(actionList[action][0]);
    for( i = CONTROL_START_IDX; i < NUM_CONTROLS; i++ )
    {
        pcell = &( ac[i] );
        ctrlid = controls[i];
		if( pcell->action == ACTION_SHOW_DIALOG ) {
			showDialog( (long)pcell->text );
		}
        else if( pcell->action > ACTION_NONE )
        {
            hctrl = GetDlgItem( hWnd, ctrlid );
            if( !IsWindowVisible( hctrl ) )
                ShowWindow( hctrl, SW_SHOWNORMAL );

            k_cell* globcell = &(ac[0]);
            char* var = 0;
            if( globcell->action ) {
                // retrieve the current value of the kermit variable
                var = ::getVar( globcell->action );
            }

            switch( pcell->action )
            {
                case ACTION_SHOW_BUTTON:
                {
                    Bool b = var && (_stricmp( var, pcell->text ) == 0);
                    SendMessage( hctrl, BM_SETCHECK, (WPARAM)b, 0 );
                    SetWindowText( hctrl, pcell->text );
                    break;
                }
                case ACTION_SHOW_EDIT:
                {
                    SetWindowText( hctrl, var ? var : "" );
                    break;
                }
                case ACTION_SHOW_COMBO:
                {
                    //SetWindowText( hctrl, var ? var : "" );
                    SendMessage( hctrl, CB_RESETCONTENT, 0, 0 );
					if( (int)(pcell->text) == K_TERMTYPESET ) {
						for( int i = 0; i < nttyp; i++ ) {
							if( !ttyptab[i].flgs )
								SendMessage( hctrl, CB_ADDSTRING, 0
									, (LPARAM) (ttyptab[i].kwd) );
						}
						SendMessage( hctrl, CB_SELECTSTRING, -1, (LPARAM)var );
					}
					else if( (int)(pcell->text) == K_COLORSET ) {
						for( int i = 0; i < nclrs; i++ ) {
							if( !ttyclrtab[i].flgs )
								SendMessage( hctrl, CB_ADDSTRING, 0
									, (LPARAM) (ttyclrtab[i].kwd) );
						}
						if( ctrlid == IDC_PROPCOMBO2 ) {
							// strip out foreground color
							//
							char* fore = strtok( var, "," );
							SendMessage( hctrl, CB_SELECTSTRING, -1, (LPARAM)fore );
						}
						else if( ctrlid == IDC_PROPCOMBO3 ) {
							// strip out background color
							//
							char* fore = strtok( var, "," );
							fore = strtok( NULL, "," );
							SendMessage( hctrl, CB_SELECTSTRING, -1, (LPARAM)fore );
						}

					}
//					else if( (int)(pcell->text) == K_TERMCHARSET ) {
//						for( int i = 0; i < ntermc; i++ ) {
//							if( !ttcstab[i].flgs )
//								SendMessage( hctrl, CB_ADDSTRING, 0
//									, (LPARAM) (ttcstab[i].kwd) );
//						}
//					}
					else if( (int)(pcell->text) == K_TRANSFERCHARSET ) {					
						for( int i = 0; i < ntcs; i++ ) {
							if( !tcstab[i].flgs )
								SendMessage( hctrl, CB_ADDSTRING, 0
									, (LPARAM) (tcstab[i].kwd) );
						}
					}
                    break;
                }
            }
        }
        else if( acundo )
        {
            pcellundo = &( acundo[i] );
			if( pcellundo->action == ACTION_SHOW_DIALOG ) {
				hideDialog( (long)pcellundo->text );
			}
            else if( pcellundo->action > ACTION_NONE ) {
                hctrlundo = GetDlgItem( hWnd, controls[i] );
                ShowWindow( hctrlundo, SW_HIDE );
            }
        }
    }

    hctrl = GetDlgItem( hWnd, IDC_GROUPSETTINGS );
    pcell = &(actionList[action][1]);
    SetWindowText( hctrl, pcell->text );

    lastAction = action;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdTerminal::doGLGR( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl )
{
	Bool done = TRUE;
	switch( idCtrl )
	{
		case IDC_CHECKGLG0:
			SendMessage( hCtrl, BM_SETCHECK, BST_CHECKED, 0 );
			SendDlgItemMessage( hPar, IDC_CHECKGLG1, BM_SETCHECK, BST_UNCHECKED, 0 );
			break;
		case IDC_CHECKGLG1:
			SendMessage( hCtrl, BM_SETCHECK, BST_CHECKED, 0 );
			SendDlgItemMessage( hPar, IDC_CHECKGLG0, BM_SETCHECK, BST_UNCHECKED, 0 );
			break;

		case IDC_CHECKGRG1:
			SendMessage( hCtrl, BM_SETCHECK, BST_CHECKED, 0 );
			SendDlgItemMessage( hPar, IDC_CHECKGRG2, BM_SETCHECK, BST_UNCHECKED, 0 );
			SendDlgItemMessage( hPar, IDC_CHECKGRG3, BM_SETCHECK, BST_UNCHECKED, 0 );
			break;
		case IDC_CHECKGRG2:
			SendMessage( hCtrl, BM_SETCHECK, BST_CHECKED, 0 );
			SendDlgItemMessage( hPar, IDC_CHECKGRG1, BM_SETCHECK, BST_UNCHECKED, 0 );
			SendDlgItemMessage( hPar, IDC_CHECKGRG3, BM_SETCHECK, BST_UNCHECKED, 0 );
			break;
		case IDC_CHECKGRG3:
			SendMessage( hCtrl, BM_SETCHECK, BST_CHECKED, 0 );
			SendDlgItemMessage( hPar, IDC_CHECKGRG1, BM_SETCHECK, BST_UNCHECKED, 0 );
			SendDlgItemMessage( hPar, IDC_CHECKGRG2, BM_SETCHECK, BST_UNCHECKED, 0 );
			break;
	}
	return done;
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdTerminal::doAdvCommand( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl )
{
	Bool done = TRUE;
	switch( idCtrl )
	{
		case IDC_CHECKGLG0:
		case IDC_CHECKGLG1:
		case IDC_CHECKGRG1:
		case IDC_CHECKGRG2:
		case IDC_CHECKGRG3:
			done = doGLGR( hPar, code, idCtrl, hCtrl );
			break;
		default:
			done = FALSE;
			break;
	}
	return done;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL KCmdTerminal::advMessage( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
	BOOL done = FALSE;
	switch( msg )
	{
		case WM_INITDIALOG:
		{
			HWND hG0 = GetDlgItem( hwnd, IDC_COMBOG0 );
			HWND hG1 = GetDlgItem( hwnd, IDC_COMBOG1 );
			HWND hG2 = GetDlgItem( hwnd, IDC_COMBOG2 );
			HWND hG3 = GetDlgItem( hwnd, IDC_COMBOG3 );
			for( int i = 0; i < ntcs; i++ ) {
				if( !tcstab[i].flgs ) {
					SendMessage( hG0, CB_ADDSTRING, 0, (LPARAM) (tcstab[i].kwd) );
					SendMessage( hG1, CB_ADDSTRING, 0, (LPARAM) (tcstab[i].kwd) );
					SendMessage( hG2, CB_ADDSTRING, 0, (LPARAM) (tcstab[i].kwd) );
					SendMessage( hG3, CB_ADDSTRING, 0, (LPARAM) (tcstab[i].kwd) );
				}
			}

			// find the current settings
			//
			long id = -1;
			char* g = getVar( TERM_CHAR_GL );
			if( strcmp( g, "G0" ) == 0 ) id = IDC_CHECKGLG0;
			else if( strcmp( g, "G1" ) == 0 ) id = IDC_CHECKGLG1;
			if( id != -1 )
				doGLGR( hwnd, 0, (WORD)id, GetDlgItem( hwnd, id ) );
			g = getVar( TERM_CHAR_GR );
			if( strcmp( g, "G1" ) == 0 ) id = IDC_CHECKGRG1;
			else if( strcmp( g, "G2" ) == 0 ) id = IDC_CHECKGRG2;
			else if( strcmp( g, "G3" ) == 0 ) id = IDC_CHECKGRG3;
			if( id != -1 )
				doGLGR( hwnd, 0, (WORD)id, GetDlgItem( hwnd, id ) );
			g = getVar( TERM_CHAR_G0 );
			if( *g != '\0' ) SendMessage( hG0, CB_SELECTSTRING, -1, (LPARAM)g );
			g = getVar( TERM_CHAR_G1 );
			if( *g != '\0' ) SendMessage( hG1, CB_SELECTSTRING, -1, (LPARAM)g );
			g = getVar( TERM_CHAR_G2 );
			if( *g != '\0' ) SendMessage( hG2, CB_SELECTSTRING, -1, (LPARAM)g );
			g = getVar( TERM_CHAR_G3 );
			if( *g != '\0' ) SendMessage( hG3, CB_SELECTSTRING, -1, (LPARAM)g );
			break;
		}
		
		case WM_COMMAND:
			done = doAdvCommand( hwnd, HIWORD(wParam), LOWORD(wParam), (HWND)lParam );
			break;
	
	}
	return done;
}
