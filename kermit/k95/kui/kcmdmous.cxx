#include "kcmdmous.hxx"
#include "karray.hxx"

#define BUTTON_ONE          0x0200
#define BUTTON_TWO          0x0400
#define BUTTON_THREE        0x0800
#define ACTION_CLICK        0x0020
#define ACTION_DRAG         0x0040
#define ACTION_DOUBLECLICK  0x0080
#define MODIFIER_CTRL       0x0002
#define MODIFIER_ALT        0x0004
#define MODIFIER_SHIFT      0x0008

typedef struct _K_MOUSEDEFINITION {
    long def;
    char* cmd;
} K_MOUSEDEFINITION;

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdMouse::KCmdMouse( K_GLOBAL* g )
    : KCmdProc( g )
    , lastAction( 0L )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdMouse::~KCmdMouse()
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KCmdMouse::doAction()
{
    if( !lastAction )
        return;

    char buf[512];
    strcpy( buf, "set mouse button " );
    if( lastAction & BUTTON_ONE )
        strcat( buf, "1 " );
    else if( lastAction & BUTTON_TWO )
        strcat( buf, "2 " );
    else if( lastAction & BUTTON_THREE )
        strcat( buf, "3 " );

    Bool none = TRUE;
    if( lastAction & MODIFIER_CTRL ) {
        strcat( buf, "ctrl" );
        none = FALSE;
    }
    if( lastAction & MODIFIER_ALT ) {
        if( lastAction & MODIFIER_CTRL )
            strcat( buf, "-" );
        strcat( buf, "alt" );
        none = FALSE;
    }
    if( lastAction & MODIFIER_SHIFT ) {
        if( (lastAction & MODIFIER_ALT) || (lastAction & MODIFIER_CTRL) )
            strcat( buf, "-" );
        strcat( buf, "shift" );
        none = FALSE;
    }
    if( none )
        strcat( buf, "none" );
    strcat( buf, " " );

    if( lastAction & ACTION_CLICK )
        strcat( buf, "click " );
    else if( lastAction & ACTION_DRAG )
        strcat( buf, "drag " );
    else if( lastAction & ACTION_DOUBLECLICK )
        strcat( buf, "double-click " );

    HWND hctrl = GetDlgItem( hWnd, IDC_EDITDEFINITION );
    int textlen = GetWindowTextLength( hctrl );
    if( textlen ) {
        char* c = new char[ textlen + 1 ];
        memset( c, '\0', textlen + 1 );
        GetWindowText( hctrl, c, textlen + 1 );
        strcat( buf, c );
        delete c;
    }

    bufferCommand( buf );
    lastAction = 0L;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdMouse::killActive()
{
    doAction();
    lastAction = 0L;
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdMouse::doInitDialog( HWND hwnd )
{
    KCmdProc::doInitDialog( hwnd );

    // TODO: initialize mouse activate
    //

    SendDlgItemMessage( hWnd, IDC_RADIOONE, BM_SETCHECK
        , (WPARAM) BST_CHECKED, (LPARAM) 0 );
    SendDlgItemMessage( hWnd, IDC_RADIOCLICK, BM_SETCHECK
        , (WPARAM) BST_CHECKED, (LPARAM) 0 );

    processDefinition();

    switch( kglob->mouseEffect )
    {
//        case TERM_MOUSE_NO_EFFECT:
//            SendDlgItemMessage( hWnd, IDC_RADIONOCHANGE
//                , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );
//            break;
        case TERM_MOUSE_CHANGE_FONT:
            SendDlgItemMessage( hWnd, IDC_RADIOCHANGEFONT
                , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );
            break;
        case TERM_MOUSE_CHANGE_DIMENSION:
            SendDlgItemMessage( hWnd, IDC_RADIOCHANGETERMINAL
                , BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0 );
            break;
    }
    return TRUE;
}

            
/*------------------------------------------------------------------------
    buffer the previous definition.
    find the definition for the new combination and display it.
------------------------------------------------------------------------*/
void KCmdMouse::processDefinition()
{
    // TODO: find the definition for the new combination and display it.
    //

    doAction();

    // button group: (one,two,three)
    //
    if( SendDlgItemMessage( hWnd, IDC_RADIOONE, BM_GETCHECK, 0, 0 ) ) {
        lastAction |= BUTTON_ONE;
    }
    else if( SendDlgItemMessage( hWnd, IDC_RADIOTWO, BM_GETCHECK, 0, 0 ) ) {
        lastAction |= BUTTON_TWO;
    }
    else if( SendDlgItemMessage( hWnd, IDC_RADIOTHREE, BM_GETCHECK, 0, 0 ) ) {
        lastAction |= BUTTON_THREE;
    }

    // action group: (click,drag,double-click)
    //
    if( SendDlgItemMessage( hWnd, IDC_RADIOCLICK, BM_GETCHECK, 0, 0 ) ) {
        lastAction |= ACTION_CLICK;
    }
    else if( SendDlgItemMessage( hWnd, IDC_RADIODRAG, BM_GETCHECK, 0, 0 ) ) {
        lastAction |= ACTION_DRAG;
    }
    else if( SendDlgItemMessage( hWnd, IDC_RADIODOUBLECLICK, BM_GETCHECK, 0, 0 ) ) {
        lastAction |= ACTION_DOUBLECLICK;
    }

    // modifier group: (alt,ctrl,shift)
    //
    if( SendDlgItemMessage( hWnd, IDC_CHECKCTRL, BM_GETCHECK, 0, 0 ) ) {
        lastAction |= MODIFIER_CTRL;
    }
    if( SendDlgItemMessage( hWnd, IDC_CHECKALT, BM_GETCHECK, 0, 0 ) ) {
        lastAction |= MODIFIER_ALT;
    }
    if( SendDlgItemMessage( hWnd, IDC_CHECKSHIFT, BM_GETCHECK, 0, 0 ) ) {
        lastAction |= MODIFIER_SHIFT;
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdMouse::doCommand( HWND hPar, WORD code
        , WORD idCtrl, HWND hCtrl )
{
    switch( idCtrl )
    {
        case IDC_RADIOCHANGEFONT:
            kglob->mouseEffect = TERM_MOUSE_CHANGE_FONT;
            break;
        case IDC_RADIOCHANGETERMINAL:
            kglob->mouseEffect = TERM_MOUSE_CHANGE_DIMENSION;
            break;
//        case IDC_RADIONOCHANGE:
//            kglob->mouseEffect = TERM_MOUSE_NO_EFFECT;
//            break;

        case IDC_BUTTONCLEAR:
            bufferCommand( "set mouse clear\r" );
            break;
        case IDC_RADIOON:
            bufferCommand( "set mouse activate on\r" );
            break;
        case IDC_RADIOOFF:
            bufferCommand( "set mouse activate off\r" );
            break;

        case IDC_RADIOONE:
        case IDC_RADIOTWO:
        case IDC_RADIOTHREE:
        case IDC_RADIOCLICK:
        case IDC_RADIODRAG:
        case IDC_RADIODOUBLECLICK:
        case IDC_CHECKALT:
        case IDC_CHECKCTRL:
        case IDC_CHECKSHIFT:
            processDefinition();
            break;

    }


    return TRUE;
}
