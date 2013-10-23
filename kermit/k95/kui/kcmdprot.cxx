#include "kcmdprot.hxx"
#include "kcmd.h"
#include "ikextern.h"

static const int NUM_CONTROLS = 17;

static int controls[NUM_CONTROLS] = {
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
    IDC_PROPEDIT2,
    IDC_PROPEDIT3
};

static k_cell actionList[][NUM_CONTROLS] = {
{ {0,"attributes"}                  ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"attributes all"}              ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"attributes character-set"}    ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"attributes date"}             ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"attributes disposition"}      ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"attributes length"}           ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"attributes system-id"}        ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"attributes type"}             ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"block-check"}                 ,{0,0}, {0,0}, {1,"1"}, {1,"2"}, {1,"3"}, {1,"blank-free-2"} },
{ {0,"buffers"}                     ,{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {1,"send buffer"}, {1,"receive buffer"}, {0,0}, {0,0}, {2,0}, {2,0} },
{ {0,"control-character prefixed"}  ,{2,0} },
{ {0,"control-character unprefixed"},{2,0} },
{ {0,"file bytesize"}               ,{0,0}, {0,0}, {1,"7"}, {1,"8"} },
{ {0,"file character-set"}          ,{0,0}, {3,(char*)K_FILECHARSET} },
{ {0,"file collision"}              ,{0,0}, {0,0}, {1,"append"}, {1,"backup"}, {1,"discard"}, {1,"overwrite"}, {1,"rename"}, {1,"update"} },
{ {0,"file download-directory"}     ,{2,0} },
{ {0,"file incomplete"}             ,{0,0}, {0,0}, {1,"discard"}, {1,"keep"} },
{ {0,"file names"}                  ,{0,0}, {0,0}, {1,"converted"}, {1,"literal"} },
{ {0,"file type"}                   ,{0,0}, {0,0}, {1,"binary"}, {1,"text"} },
{ {0,"handshake"}                   ,{0,0}, {0,0}, {1,"bell"}, {1,"code"}, {1,"cr"}, {1,"esc"}, {1,"lf"}, {1,"none"}, {1,"xoff"}, {1,"xon" } },
{ {0,"language"}                    ,{0,0}, {3,(char*)K_LANGUAGESET} },
{ {0,"prefixing"}                   ,{0,0}, {0,0}, {1,"all"}, {1,"cautious"}, {1,"minimal"}, {1,"none"} },
{ {0,"protocol"}                    ,{0,0}, {0,0}, {1,"kermit"}, {1,"xmodem"}, {1,"ymodem"}, {1,"ymodem-g"}, {1,"zmodem"} },
{ {0,"receive control-prefix"}      ,{2,0} },
{ {0,"receive end-of-packet"}       ,{2,0} },
{ {0,"receive packet-length"}       ,{2,0} },
{ {0,"receive pad-character"}       ,{2,0} },
{ {0,"receive padding"}             ,{2,0} },
{ {0,"receive pathnames"}           ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"receive pause"}               ,{2,0} },
{ {0,"receive quote"}               ,{2,0} },
{ {0,"receive start-of-packet"}     ,{2,0} },
{ {0,"receive timeout"}             ,{2,0} },
{ {0,"retry-limit"}                 ,{2,0} },
{ {0,"send control-prefix"}         ,{2,0} },
{ {0,"send end-of-packet"}          ,{2,0} },
{ {0,"send packet-length"}          ,{2,0} },
{ {0,"send pad-character"}          ,{2,0} },
{ {0,"send padding"}                ,{2,0} },
{ {0,"send pathnames"}              ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"send pause"}                  ,{2,0} },
{ {0,"send quote"}                  ,{2,0} },
{ {0,"send start-of-packet"}        ,{2,0} },
{ {0,"send timeout"}                ,{2,0} },
{ {0,"server display"}              ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"server timeout"}              ,{2,0} },
{ {0,"transfer character-set"}      ,{0,0}, {0,0}, {1,"ascii"}, {1,"cyrillic-iso"}, {1,"hebrew-iso"}, {1,"japanese-euc"}, {1,"latin1-iso"}, {1,"latin2-iso"}, {1,"transparent"} },
{ {0,"transfer locking-shift"}      ,{0,0}, {0,0}, {1,"on"}, {1,"off"}, {1,"forced"} },
{ {0,"transmit echo"}               ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"transmit eof"}                ,{2,0} },
{ {0,"transmit fill"}               ,{2,0} },
{ {0,"transmit linefeed"}           ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"transmit locking-shift"}      ,{0,0}, {0,0}, {1,"on"}, {1,"off"} },
{ {0,"transmit pause"}              ,{2,0} },
{ {0,"transmit prompt"}             ,{2,0} },
{ {0,"unknown-char-set"}            ,{0,0}, {0,0}, {1,"discard"}, {1,"keep"} },
{ {0,"window-size"}                 ,{2,0} },
    0
};

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdProtocol::KCmdProtocol( K_GLOBAL* g )
    : KCmdProc( g )
{
    lastAction = -1;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdProtocol::~KCmdProtocol()
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdProtocol::killActive()
{
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdProtocol::doInitDialog( HWND hwnd )
{
    KCmdProc::doInitDialog( hwnd );

    hlist = GetDlgItem( hWnd, IDC_PROPLIST );
    if( !hlist )
        return TRUE;

    int i = 0;
    k_cell* ac = &(actionList[i][0]);
    while( ac->text )
    {
        SendMessage( hlist, LB_ADDSTRING, 0, (LPARAM) ac->text );
        i++;
        ac = &(actionList[i][0]);
    }

    PostMessage( hlist, LB_SETCURSEL, (WPARAM) 0, (LPARAM) 0 );
    doAction( 0 );

    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdProtocol::doCommand( HWND hPar, WORD code
        , WORD idCtrl, HWND hCtrl )
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
void KCmdProtocol::doAction( int action )
{
    if( lastAction == action )
        return;

    k_cell *acundo = 0, *ac = 0, *pcell = 0, *pcellundo = 0;
    HWND hctrl = 0, hctrlundo = 0;
    int i = 0, ctrlid = 0;

    if( lastAction != -1 )
        acundo = &(actionList[lastAction][0]);
    ac = &(actionList[action][0]);
    for( i = 1; i < NUM_CONTROLS; i++ )
    {
        pcell = &( ac[i] );
        ctrlid = controls[i];
        if( pcell->action > ACTION_NONE ) 
        {
            hctrl = GetDlgItem( hWnd, ctrlid );
            if( !IsWindowVisible( hctrl ) )
                ShowWindow( hctrl, SW_SHOWNORMAL );

            switch( pcell->action )
            {
                case ACTION_SHOW_BUTTON:
                    SetWindowText( hctrl, pcell->text );
                    break;
                case ACTION_SHOW_EDIT:
                    break;
                case ACTION_SHOW_COMBO:
                {
                    SendMessage( hctrl, CB_RESETCONTENT, 0, 0 );
					if( (int)(pcell->text) == K_LANGUAGESET ) {
						for( int i = 0; i < nlng; i++ ) {
							if( !lngtab[i].flgs )
								SendMessage( hctrl, CB_ADDSTRING, 0
									, (LPARAM) (lngtab[i].kwd) );
						}
					}
					else if( (int)(pcell->text) == K_FILECHARSET ) {
						for( int i = 0; i < nfilc; i++ ) {
							if( !fcstab[i].flgs )
								SendMessage( hctrl, CB_ADDSTRING, 0
									, (LPARAM) (fcstab[i].kwd) );
						}
					}
                    break;
                }
            }
        }
        else if( acundo )
        {
            pcellundo = &( acundo[i] );
            if( pcellundo->action > ACTION_NONE ) {
                hctrlundo = GetDlgItem( hWnd, controls[i] );
                ShowWindow( hctrlundo, SW_HIDE );
            }
        }
    }

    hctrl = GetDlgItem( hWnd, IDC_GROUPSETTINGS );
    pcell = &(actionList[action][0]);
    SetWindowText( hctrl, pcell->text );
    lastAction = action;
}
