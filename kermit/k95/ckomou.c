char *ckomouv = "Mouse Support 10.0, 1 Oct 2022";

/* C K O M O U   --  Kermit mouse support for OS/2 systems */

/*
  Author: Jeffrey Altman (jaltman@secure-endpoints.com)
            Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#include "ckcdeb.h"
#ifdef OS2MOUSE
#ifdef NT
#include <windows.h>
#else /* NT */
#define INCL_WINSHELLDATA
#define INCL_VIO
#define INCL_MOU
#define INCL_ERRORS
#define INCL_DOSPROCESS
#define INCL_DOSSESMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVIOCTL
#define INCL_WINCLIPBOARD
#define INCL_DOSDATETIME
#define INCL_DOSASYNCTIMER
#include <os2.h>
#undef COMMENT                /* COMMENT is defined in os2.h */
#endif /* NT */

#include "ckuusr.h"
#include "ckcker.h"
#include "ckocon.h"
#include "ckokey.h"
#include "ckoreg.h"

#ifdef NT
#ifndef NOSCROLLWHEEL
#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))
#endif /* GET_WHEEL_DELTA_WPARAM */
#endif /* NOSCROLLWHEEL */
#endif /* NT */

static BOOL SelectionValid = 0 ;
con_event mousemap[MMBUTTONMAX][MMSIZE] ;
#ifdef NT
static USHORT Vrow, Vcol, Event ;
static USHORT lastrow=0, lastcol=0 ;
static struct {
    int state ;
    USHORT row   ;
    USHORT col   ;
    } b1 = {0,0,0}, b2 = {0,0,0}, b3  = {0,0,0};
int mouseon = 0 ;
#else /* NT */
static HMOU hMouse = 0 ;
static int dblclickspeed = 500 ;
static TID tidMouse = 0 ;
#endif /* NT */

extern int tnlm;
extern int tt_status[VNUM];
extern vik_rec vik;                     /* Very Important Keys    */

#define THRDSTKSIZ      131072  /* Needed for Mouse Thread */

int MouseCurX, MouseCurY;
int MouseDebug = 0;

/* Terminal Mouse Tracking Mode, one of:
 * MOUSEREPORTING_NONE       No mouse tracking
 * MOUSEREPORTING_X10        X10 mouse tracking - send position on press
 * MOUSEREPORTING_X11        X11 (xterm) mouse tracking - send position and
 *                          modifiers on mouse press and mouse release
 * MOUSEREPORTING_DISABLE    Don't allow the remote host to turn mouse tracking
 *                          on.
 **/
int      mouse_reporting_mode = MOUSEREPORTING_NONE;

/* TRUE - send reports *instead* of any action defined for that mouse button and
 *        modifier combination (K95 doesn't handle any mouse input).
 * FALSE - mouse reports are only sent if no other action is mapped for that
 *         mouse button and modifier combination. For example, if right-click
 *         is set to \Kpaste then right-clicks will not be sent to the remote
 *         application.
 */
BOOL     mouse_reporting_override = FALSE;

/*
Potential mouse events:
   MOUSE_MOTION
   MOUSE_MOTION_WITH_BN1_DOWN
   MOUSE_BN1_DOWN
   MOUSE_MOTION_WITH_BN2_DOWN
   MOUSE_BN2_DOWN
   MOUSE_MOTION_WITH_BN3_DOWN
   MOUSE_BN3_DOWN

*/

static BOOL ThreeButton = 0 ;  /* 0 -two buttons, 1 - three buttons */

int
mousebuttoncount()
{
    return(ThreeButton?3:2);
}

#ifndef NT
int
querydblclickspeed( void )
{
    CHAR data[8] ;
    ULONG size = 8 ;

    if ( PrfQueryProfileData( HINI_USERPROFILE, "PM_ControlPanel",
        "DoubleClickSpeed", data, &size ) )
        return atoi( data ) ;
    else
        return 500 ;  /* the OS/2 default is 500 milliseconds */
}
#endif /* NT */

char *
mousename( int button, int event )
{
    static char mousename[64] ;
    char temp[17] ;

    mousename[0] = '\0' ;
    if (button == MMB4 || button == MMB5) {
        strcpy(mousename, "Wheel");
    } else {
        strcpy(mousename, "Button ") ;
#ifdef NT
        ckstrncat(mousename, _itoa(button+1,temp,10), sizeof(mousename) ) ;
#else /* NT */
        ckstrncat(mousename, itoa(button+1,temp,10), sizeof(mousename) ) ;
#endif /* NT */
    }

    ckstrncat(mousename, " ", sizeof(mousename) ) ;
    if ( event & MMCTRL )
        ckstrncat( mousename, "Ctrl-", sizeof(mousename) ) ;
    if ( event & MMALT )
        ckstrncat( mousename, "Alt-", sizeof(mousename) ) ;
    if ( event & MMSHIFT )
        ckstrncat( mousename, "Shift-", sizeof(mousename) ) ;
    if ( event & MMDRAG )
        ckstrncat( mousename, "Drag", sizeof(mousename) ) ;
    else if ( event & MMDBL )
        ckstrncat( mousename, "Double-Click", sizeof(mousename) ) ;
    else if (button != MMB4 && button != MMB5) {
        /* For the purposes of the mouse event system, we pretend the mouse
         * wheel is a pair of buttons. One notch away from the user is one click
         * of button 4, and one notch towards the user is one click of button 5.
         * But the user isn't clicking and we don't provide other non-click
         * options (dragging the wheel makes no sense) so we don't do this for
         * buttons 4 and 5. */
        ckstrncat (mousename, "Click", sizeof(mousename) ) ;
    }

    if (button == MMB4) {
        /* Mouse wheel up */
        ckstrncat(mousename, "Up", sizeof(mousename) );
    } else if (button == MMB5) {
        /* Mouse wheel down */
        ckstrncat(mousename, "Down", sizeof(mousename) );
    }

    return mousename ;
}


void
mousemapinit( int button, int event )
{
    int x,y, resetall ;

    resetall = button < 0 || event < 0 ||
        button >= MMBUTTONMAX || event >= MMEVENTSIZE ;

    if ( resetall )
        for ( x = 0 ; x < MMBUTTONMAX ; x++ ) {
           for ( y = 0 ; y < MMSIZE ; y++ )
              if ( y == MMCLICK ) /* MMCLICK AKA MMWHEEL */
              {
                 mousemap[x][y].type = kverb ;
                 mousemap[x][y].kverb.id = F_KVERB | K_IGNORE ;
              }
              else mousemap[x][y].type = error ;
        }

#ifndef NOSCROLLWHEEL
    if ( resetall || button == MMB4 && event == MMWHEEL) {
        /* Assign wheel up event */
        mousemap[MMB4][MMWHEEL].type = kverb ;
        mousemap[MMB4][MMWHEEL].kverb.id = F_KVERB | K_UPONE ;
    }

    if ( resetall || button == MMB5 && event == MMWHEEL) {
        /* Assign wheel down event */
        mousemap[MMB5][MMWHEEL].type = kverb ;
        mousemap[MMB5][MMWHEEL].kverb.id = F_KVERB | K_DNONE ;
    }

    if ( resetall || button == MMB4 && event == MMCTRL | MMWHEEL) {
        /* Assign wheel ctrl+up event */
        mousemap[MMB4][MMCTRL | MMWHEEL].type = kverb ;
        mousemap[MMB4][MMCTRL | MMWHEEL].kverb.id = F_KVERB | K_UPSCN ;
    }

    if ( resetall || button == MMB5 && event == MMCTRL | MMWHEEL) {
        /* Assign wheel ctrl+down event */
        mousemap[MMB5][MMCTRL | MMWHEEL].type = kverb ;
        mousemap[MMB5][MMCTRL | MMWHEEL].kverb.id = F_KVERB | K_DNSCN ;
    }
#endif

    if ( resetall || button == MMB1 && event == MMDBL ) {
        /* Assign Cursor Positioning */
    mousemap[MMB1][MMDBL].type = kverb ;
    mousemap[MMB1][MMDBL].kverb.id = F_KVERB | K_MOUSE_CURPOS ;
    if ( !resetall )
        return ;
    }

    if ( resetall || button == MMB1 && event == MMCTRL | MMCLICK ) {
        /* Assign URL Execution */
    mousemap[MMB1][MMCTRL|MMCLICK].type = kverb ;
    mousemap[MMB1][MMCTRL|MMCLICK].kverb.id = F_KVERB | K_MOUSE_URL ;
    if ( !resetall )
        return ;
    }

    if ( resetall || button == MMB1 && event == MMDRAG ) {
    mousemap[MMB1][MMDRAG].type = kverb ;
    mousemap[MMB1][MMDRAG].kverb.id = F_KVERB | K_MARK_COPYCLIP ;
    if ( !resetall )
        return ;
    }

    if ( resetall || button == MMB2 && event == MMDRAG ) {
    mousemap[MMB2][MMDRAG].type = kverb ;
    mousemap[MMB2][MMDRAG].kverb.id = F_KVERB | K_MARK_COPYHOST ;
    if ( !resetall )
        return ;
    }

    if ( resetall || button == MMB1 && event == MMCTRL | MMDRAG ) {
    mousemap[MMB1][MMCTRL | MMDRAG].type = kverb ;
    mousemap[MMB1][MMCTRL | MMDRAG].kverb.id =
    F_KVERB | K_MARK_COPYHOST ;
    if ( !resetall )
        return ;
    }

    if ( resetall || button == MMB1 && event == MMALT | MMDRAG) {
    mousemap[MMB1][MMALT | MMDRAG].type = kverb ;
    mousemap[MMB1][MMALT | MMDRAG].kverb.id = F_KVERB | K_MARK_COPYCLIP_NOEOL ;
    if ( !resetall )
        return ;
    }

    if ( resetall || button == MMB2 && event == MMALT | MMDRAG ) {
    mousemap[MMB2][MMALT | MMDRAG].type = kverb ;
    mousemap[MMB2][MMALT | MMDRAG].kverb.id = F_KVERB | K_MARK_COPYHOST_NOEOL ;
    if ( !resetall )
        return ;
    }

    if ( resetall || button == MMB1 && event == MMCTRL | MMALT | MMDRAG ) {
    mousemap[MMB1][MMCTRL | MMALT | MMDRAG].type = kverb ;
    mousemap[MMB1][MMCTRL | MMALT | MMDRAG].kverb.id = F_KVERB | K_MARK_COPYHOST_NOEOL ;
    if ( !resetall )
        return ;
    }

    if ( resetall || button == MMB1 && event == MMSHIFT | MMCTRL | MMDRAG ) {
    mousemap[MMB1][MMSHIFT | MMCTRL | MMDRAG].type = kverb ;
    mousemap[MMB1][MMSHIFT | MMCTRL | MMDRAG].kverb.id =
    F_KVERB | K_DUMP ;
    if ( !resetall )
        return ;
    }

    if ( resetall || button == MMB2 && event == MMDBL ) {
        mousemap[MMB2][MMDBL].type = kverb ;
        mousemap[MMB2][MMDBL].kverb.id = F_KVERB | K_PASTE ;
        if ( !resetall )
            return ;
    }
    if ( resetall || button == MMB3 && event == MMDBL ) {
        mousemap[MMB3][MMDBL].type = kverb ;
        mousemap[MMB3][MMDBL].kverb.id = F_KVERB | K_PASTE ;
        if ( !resetall )
            return ;
    }

    if ( !resetall ) {
       if ( event == MMCLICK ) /* AKA MMWHEEL */
       {
           mousemap[button][event].type = kverb ;
           mousemap[button][event].kverb.id = F_KVERB | K_IGNORE ;
       }
       else mousemap[button][event].type = error ;
       return ;
    }
}

#ifndef NT
APIRET
os2_mouseshow( void )
{
APIRET rc = 0 ;
NOPTRRECT PtrArea ;
CK_VIDEOMODEINFO ModeData ;

if ( hMouse )
    {
    GetMode( &ModeData ) ;

    PtrArea.row = ModeData.row - 1 ;
    PtrArea.col = 0 ;
    PtrArea.cRow = ModeData.row - 1;
    PtrArea.cCol = ModeData.col - 1 ;

    MouRemovePtr( &PtrArea, hMouse ) ;
    }
return rc ;
}

APIRET
os2_mousehide( void )
{
APIRET rc = 0 ;
NOPTRRECT PtrArea ;
CK_VIDEOMODEINFO ModeData ;

if (hMouse)
    {
    GetMode( &ModeData ) ;

    PtrArea.row = 0 ;
    PtrArea.col = 0 ;
    PtrArea.cRow = ModeData.row - 1;
    PtrArea.cCol = ModeData.col - 1 ;

    MouRemovePtr( &PtrArea, hMouse ) ;
    }
return rc ;
}
#endif /* NT */

APIRET
os2_mouseon( void )
{
    APIRET rc = 0 ;

#ifdef NT
#ifdef KUI
    /* GUI version of K95 */
    int buttonCount;

    buttonCount = GetSystemMetrics(SM_CMOUSEBUTTONS);

    debug(F101,"os2_mouseon Button Count","", buttonCount) ;

    if (buttonCount == 0) return rc; /* No mouse installed */

    ThreeButton = (buttonCount > 2);
    mouseon = TRUE;
    debug(F100, "Mouse ON", "", 0);
#else
    /* Console version of K95 */
    extern HANDLE KbdHandle ;
    DWORD mode=0, count=0 ;

    GetNumberOfConsoleMouseButtons(&count) ;
    ThreeButton = ( count == 3 ) ;
    debug(F101,"os2_mouseon Button Count","",count ) ;

    debug(F111,"os2_mouseon","KbdHandle",KbdHandle);
    GetConsoleMode( KbdHandle, &mode ) ;
    debug(F111,"os2_mouseon GetConsoleMode","mode",mode);
    mode |= ENABLE_MOUSE_INPUT ;
    debug(F111,"os2_mouseon","mode",mode);
    rc = SetConsoleMode( KbdHandle, mode ) ;
    mouseon = TRUE ;
    debug(F111,"os2_mouseon SetConsoleMode","rc",rc) ;
#endif /* KUI */
#else /* NT */
PTRLOC    PtrPos ;
NOPTRRECT PtrArea ;
CK_VIDEOMODEINFO ModeData ;
USHORT    ButtonCount ;
USHORT    EventMask ;
extern    BYTE vmode ;

    if (!tidMouse) {
    rc = MouOpen( 0, &hMouse ) ;
    debug(F101,"Mouse On","",rc) ;
    }

    if ( !rc ) {
        rc = MouDrawPtr( hMouse ) ;
        GetMode( &ModeData ) ;

        if ( IsOS2FullScreen() ) {
            PtrPos.row = ModeData.row - 1 ;
            PtrPos.col = VscrnGetWidth(vmode)-1 ;
            MouSetPtrPos( &PtrPos, hMouse ) ;
        }

    PtrArea.row = ModeData.row - 1 ;
    PtrArea.col = 0 ;
    PtrArea.cRow = ModeData.row - 1;
    PtrArea.cCol = ModeData.col - 1 ;

    MouRemovePtr( &PtrArea, hMouse ) ;

    MouGetNumButtons( &ButtonCount, hMouse ) ;
    debug(F101,"os2_mouseon Button Count","",ButtonCount ) ;

    if ( ButtonCount == 3 )
        {
        ThreeButton = 1 ;
        EventMask      = MOUSE_BN1_DOWN |
        MOUSE_MOTION_WITH_BN1_DOWN |
        MOUSE_BN2_DOWN |
        MOUSE_MOTION_WITH_BN2_DOWN |
        MOUSE_BN3_DOWN |
        MOUSE_MOTION_WITH_BN3_DOWN ;
        }
    else  /* 2 buttons */
        {
        ThreeButton = 0 ;
        EventMask      = MOUSE_BN1_DOWN |
        MOUSE_MOTION_WITH_BN1_DOWN |
        MOUSE_BN2_DOWN |
        MOUSE_MOTION_WITH_BN2_DOWN ;
        }
    MouSetEventMask( &EventMask, hMouse ) ;

    dblclickspeed = querydblclickspeed() ;

        if (!tidMouse) {
        tidMouse = _beginthread( &os2_mouseevt, 0, THRDSTKSIZ, 0 ) ;
        }
    }
#endif /* NT */
return rc ;
}



void
mouseurl( int mode, USHORT row, USHORT col )
{
#ifdef BROWSER
    extern char browsopts[];
    extern char browser[];
    char      cmd[1024]="", tmpbuf[1024]="";
    int rc, x=0;
#ifdef NT
    STARTUPINFO si;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
#else /* NT */
    STARTDATA sd;
    PID       pid;
    ULONG     idSession;
#endif /* NT */
    const char * url;

    if ( VscrnURL( mode, row, col ) ) {
        bleep(BP_FAIL);
        return ;
    }

    url = GetURL();

    if (browsopts[0]) {
        x = ckindex("%1",(char *)browsopts,0,0,1);
        if (x > 0)
            browsopts[x] = 's';         /* x is 1-based */
        else
            x = ckindex("%s",(char *)browsopts,0,0,1);
    }
    if (x)
        sprintf(tmpbuf,browsopts,url);
    else
        sprintf(tmpbuf,"%s %s",browsopts,url);
    debug(F110,"mouseurl options and url",tmpbuf,0);

#ifdef NT
    if ( (GetURLType() == HYPERLINK_UNC) || !browser[0] ) {
        if ( !Win32ShellExecute((char*)url) )
            bleep(BP_FAIL);
        debug(F111,"mouseurl ShellExecute",url,GetLastError());
    }
    else {
    memset( &sa, 0, sizeof(SECURITY_ATTRIBUTES) );     //  Initialize struct
    sa.nLength=sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor=NULL;
    sa.bInheritHandle=1;

    memset( &si, 0, sizeof(STARTUPINFO) );     //  Initialize struct
    si.cb          = sizeof(STARTUPINFO);
    si.dwFlags     = STARTF_USESHOWWINDOW;
    si.wShowWindow = 1;        //  Don't show the console window (DOS box)

    sprintf(cmd,"%s %s",browser,tmpbuf);
    rc = CreateProcess ( NULL, cmd, NULL, NULL, FALSE, // bInheritHandler
                         CREATE_NEW_PROCESS_GROUP,
                         NULL, NULL, &si, &pi);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    debug(F111,"mouseurl",cmd,rc?0:GetLastError());
    }
#else /* NT */
    memset(&sd,0,sizeof(STARTDATA));
    sd.Length = sizeof(STARTDATA);
    sd.Related = SSF_RELATED_INDEPENDENT;
    sd.FgBg = SSF_FGBG_FORE;
    sd.TraceOpt = SSF_TRACEOPT_NONE;
    sd.PgmTitle = "K-95 Browser";
    sd.PgmName = browser;
    sd.PgmInputs = tmpbuf;
    sd.TermQ = 0;
    sd.Environment = 0;
    sd.InheritOpt = SSF_INHERTOPT_SHELL;
    sd.SessionType = SSF_TYPE_DEFAULT;
    sd.IconFile = 0;
    sd.ObjectBuffer = cmd;
    sd.ObjectBuffLen = 1024;
    rc = DosStartSession( &sd, &idSession, &pid );
    debug(F111,"mouseurl",cmd,rc);
#endif /* NT */
#endif /* BROWSER */
}

void
mousecurpos( int mode, USHORT orow, USHORT ocol, USHORT nrow, USHORT ncol, BOOL kverb )
{
#ifdef NT
    extern int win32ScrollUp, win32ScrollDown;
#endif /* NT */

    RequestKeyStrokeMutex( mode, SEM_INDEFINITE_WAIT);
#ifdef NT
    /* only set the scroll flag if we are using kverbs and not keystrokes */
    if ( kverb ) {
        win32ScrollUp = (nrow < 1);
        win32ScrollDown = (nrow >= (VscrnGetHeight(mode)
                                     -1 -(tt_status[mode]?1:0)));
    }
#endif /* NT */

    for (; ocol > ncol; ocol-- )
    {
        !kverb ?
          putkey( mode, 4389 ) :
              putkverb( mode, F_KVERB | K_LFARR ) ; /* LEFT */
    }
    for (; orow > nrow; orow-- )
    {
        !kverb ?
          putkey( mode, 4390 ) :
              putkverb( mode, F_KVERB | K_UPARR ) ; /* UP */
    }
    for (; orow < nrow; orow++ )
    {
        !kverb ?
          putkey( mode, 4392 ) :
          putkverb( mode, F_KVERB | K_DNARR ) ; /* DOWN */
    }
    for (; ocol < ncol; ocol++ )
    {
        !kverb ?
          putkey( mode, 4391 ) :
          putkverb( mode, F_KVERB | K_RTARR ) ; /* RIGHT */
    }
    ReleaseKeyStrokeMutex(mode) ;
}

void mouse_report(int x_coord, int y_coord, int button, BOOL ctrl, BOOL shift, BOOL meta,
                  BOOL pressed) {
    /* b, x, and y are all sent as (value+32) to ensure its a printable
     * character. C-Kermit coordinates are 0-based, mouse report cordinates
     * are 1-based (1,1 is the top left corner) */
    extern BYTE vmode;
    char x = x_coord + 1;
    char y = y_coord + 1;
    char b = '\0';
    char report[32] = "\033[M   \0";
    char modifiers = 0;
    int len;

    if (!MOUSE_REPORTING_ACTIVE(mouse_reporting_mode, vmode)) {
        return; /* Mouse tracking isn't on - nothing to do. */
    }

    /* K95 numbers buttons from 1, but we need to send buttons numbered from 0 */
    b = button - 1;

    if (b > 2) {
        /* Buttons 4 and 5 are send as buttons 1 and 2 with 64 added
         * to the event code */
        b -= 3;
        b += 64;
    }

    if (shift) modifiers += 0x04;
    if (meta) modifiers += 0x08;
    if (ctrl) modifiers += 0x10;

    if (MOUSE_REPORTING_TEST_FLAG(mouse_reporting_mode,
                                  MOUSEREPORTING_SGR)) {
        if (b > 2) {
          /*  b += 64;*/
        }

        b += modifiers;

        len = sprintf(report, "\033[<%d;%d;%d%c", b, x, y, pressed ? 'M' : 'm');
        sendcharsduplex(report,len,TRUE);
        return;
    }



    if (MOUSE_REPORTING_TEST_FLAG(mouse_reporting_mode,
                                  MOUSEREPORTING_URXVT)) {

        if (!pressed) {
            b = 3;
        }

        b += modifiers + 32;

        len = sprintf(report, "\033[%d;%d;%dM", b, x, y);
        sendcharsduplex(report,len,TRUE);
        return;
    }

    /* The X11 and X10 protocols send the coordinates as printable characters
     * so we've got to add 32 */
    x += 32;
    y += 32;

    if (MOUSE_REPORTING_TEST_FLAG(mouse_reporting_mode,
                                  MOUSEREPORTING_X11)) {

        if (x_coord > 223 || y_coord > 223) {
            if (x_coord > 223)
                debug(F111, "Not sending mouse report - X coordinate out of range", "X", x_coord);
            if (y_coord > 223)
                debug(F111, "Not sending mouse report - Y coordinate out of range", "Y", x_coord);
            return;
        }

        /* Send:
         *   ESC [ M bxy
         * Where:
         *   b = more complicated (see below)
         *   x = x_coord + 32
         *   y = y_coord + 32
         */

        /* b:
         * ---
         *  | 7 6 5 4 3 2 |    1 0 |
         *  | modifiers   | button |
         *
         *  Where modifiers the following added together:
         *    shift - 0x04
         *    meta  - 0x08
         *    ctrl  - 0x10
         *  And button is:
         *    0 - LMB
         *    1 - Middle (3 button mouse) or RMB (2 button mouse)
         *    2 - RMB (3 button mouse)
         *    3 - button released
         *  For buttons 4 and 5, add 64
         */

        if (!pressed) {
            b = 3;
        }

        if (modifiers > 0) {
            b |= modifiers;
        }
    } else if (MOUSE_REPORTING_TEST_FLAG(mouse_reporting_mode, MOUSEREPORTING_X10)) {
        /* X10 mouse tracking only sends a report on mouse down */
        if (!pressed) return;
        if (button > 3) return; /* Unsupported mouse button */

        if (x_coord > 223 || y_coord > 223) {
            if (x_coord > 223)
                debug(F111, "Not sending mouse report - X coordinate out of range", "X", x_coord);
            if (y_coord > 223)
                debug(F111, "Not sending mouse report - Y coordinate out of range", "Y", x_coord);
            return;
        }

        /* Send:
         *   ESC [ M bxy
         * Where:
         *   b = (button-1) + 32
         *   x = x_coord + 32
         *   y = y + 32
         */
    }

    b += 32;

    /* Write: ESC [ M bxy */
    report[3] = b;
    report[4] = x;
    report[5] = y;

    sendcharsduplex(report,6,TRUE);
}

#ifdef  NT

/* Some handy macros to figure out of a particular mouse event is bound to
 * something other than \Kignore */
#define MOUSE_EVENT_IGNORED(button, event) \
                (mousemap[button][event].type == kverb && \
                (mousemap[button][event].kverb.id & ~(F_KVERB)) == K_IGNORE)
#define MOUSE_DRAG_EVENT_IGNORED(button, event) (MOUSE_EVENT_IGNORED(button, event | MMDRAG))

void
win32MouseEvent( int mode, MOUSE_EVENT_RECORD r )
{
    extern int win32ScrollUp, win32ScrollDown;
    extern BYTE vmode;
    position   * ppos ;
    char buffer[1024] ;

    if (!mouseon) return; /* The mouse is disabled */

    if ( MouseDebug ) {
        int needcomma = 0;
        printf("Mouse Event: (%d,%d)",r.dwMousePosition.X,r.dwMousePosition.Y);

        if ( r.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED ) {
            printf(" B1");
            needcomma = 1;
        }
        if ( r.dwButtonState & (ThreeButton ? FROM_LEFT_2ND_BUTTON_PRESSED :
                                 RIGHTMOST_BUTTON_PRESSED) ) {
            if ( needcomma )
                printf(",B2");
            else {
                printf(" B2");
                needcomma = 1;
            }
        }
        if ( r.dwButtonState & (ThreeButton ? RIGHTMOST_BUTTON_PRESSED : 0) ) {
            if ( needcomma )
                printf(",B3");
            else {
                printf(" B3");
            }
        }
        needcomma = 0;


        if ( r.dwControlKeyState & CONTROL ) {
            printf(" Ctrl");
            needcomma = 1;
        }
        if ( r.dwControlKeyState & ALT ) {
            if ( needcomma )
                printf("-Alt");
            else {
                printf(" Alt");
                needcomma = 1;
            }
        }
        if ( r.dwControlKeyState & SHIFT ) {
            if ( needcomma )
                printf("-Shift");
            else
               printf(" Shift");
        }
        needcomma = 0;

        if ( r.dwEventFlags == 0 ) {
            printf(" Click");
        } else {
            if ( r.dwEventFlags & DOUBLE_CLICK ) {
                printf(" Double");
                needcomma = 1;
            }
            if ( r.dwEventFlags & MOUSE_MOVED ) {
                if ( needcomma )
                    printf("-Move");
                else {
                    printf(" Move");
                    needcomma = 1;
                }
            }
#ifndef NOSCROLLWHEEL
#ifdef MOUSE_WHEELED
            if ( r.dwEventFlags & MOUSE_WHEELED ) {
                int zDelta = GET_WHEEL_DELTA_WPARAM(r.dwButtonState) / WHEEL_DELTA;

                debug(F111, "MOUSE_WHEELED", "zdelta", zDelta);

                if (zDelta > 0) {
                    if ( needcomma )
                        printf("-Wheel FW");
                    else {
                        printf(" Wheel FW");
                        needcomma = 1;
                    }
                } else {
                    if ( needcomma )
                        printf("-Wheel BW");
                    else {
                        printf(" Wheel BW");
                        needcomma = 1;
                    }
                }
            }
#endif /* MOUSE_WHEELED */
#endif /* NOSCROLLWHEEL */
        }
        printf("\n");
        return;
    }

    /* \V() variables */
    MouseCurX = r.dwMousePosition.X;
    MouseCurY = r.dwMousePosition.Y;

    Event = 0 ;
    if ( r.dwControlKeyState & SHIFT )
        Event |= MMSHIFT ;
    if ( r.dwControlKeyState & ALT )
        Event |= MMALT ;
    if ( r.dwControlKeyState & CONTROL )
        Event |= MMCTRL ;

    if (mouse_reporting_override && MOUSE_REPORTING_ACTIVE(mouse_reporting_mode, vmode)) {
        /* Mouse reporting is currently active and we're set to forward *all*
         * mouse events to the remote host regardless of what that input may be
         * mapped to within K95.*/

#ifndef NOSCROLLWHEEL
        if (r.dwEventFlags & MOUSE_WHEELED) {
            /*
             * xterm sends mouse wheel events as buttons 4 and 5. Each click is
             * one press without a release.
             */
            int button;
            int zDelta = GET_WHEEL_DELTA_WPARAM(r.dwButtonState) / WHEEL_DELTA;

            if (zDelta > 0) {
                /* Positive is foreward / up scroll */
                button = 4;
            } else {
                /* Negative is backward/ down scroll */
                button = 5;
                zDelta = zDelta * -1;
            }

            do {
                mouse_report(r.dwMousePosition.X,
                             r.dwMousePosition.Y,
                             button,
                             r.dwControlKeyState & CONTROL,
                             r.dwControlKeyState & SHIFT,
                             r.dwControlKeyState & ALT,
                             TRUE);
                zDelta--;
            } while (zDelta > 0);
        } else
#endif
        {
            static int button = 0;
            int previous_button = button;

            /* Figure out which button was pressed (if any) */
            button = 0;
            if (r.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) button = 1;
            if (r.dwButtonState & (ThreeButton ? FROM_LEFT_2ND_BUTTON_PRESSED :
                                     RIGHTMOST_BUTTON_PRESSED)) button = 2;
            if (r.dwButtonState & (ThreeButton ? RIGHTMOST_BUTTON_PRESSED : 0))
                button = 3;

            /* If button == 0 that means whatever mouse button was pressed before
             * has been released. */
            mouse_report(r.dwMousePosition.X,
                         r.dwMousePosition.Y,
                         button == 0 ? previous_button : button,
                         r.dwControlKeyState & CONTROL,
                         r.dwControlKeyState & SHIFT,
                         r.dwControlKeyState & ALT,
                         button != 0);
        }

        return;
    }

   if ( TRUE )
   {
       /* Details for mouse reporting (if active) */
       int mr_button = -1;
       int mr_event = Event;

       sprintf(buffer, "  Event: fs:%3x row:%3d col:%3d",
                r.dwButtonState,
                r.dwMousePosition.Y, r.dwMousePosition.X) ;
       debug(F110,"win32MouseEvent",buffer,0) ;

       ppos = VscrnGetCurPos(mode) ;
       Vrow = ppos->y ;
       Vcol = ppos->x ;

       if ( r.dwMousePosition.Y >= VscrnGetHeight(mode)
            -(tt_status[mode]?1:0) )
           r.dwMousePosition.Y = VscrnGetHeight(mode) -(1+(tt_status[mode]?1:0)) ;
       if ( r.dwMousePosition.Y < 0 )
           r.dwMousePosition.Y = 0 ;
       if ( r.dwMousePosition.X >= VscrnGetWidth(mode) )
           r.dwMousePosition.X = VscrnGetWidth(mode) -1 ;
       if ( r.dwMousePosition.X < 0 )
           r.dwMousePosition.X = 0 ;

       if ( (r.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && b1.state == 0 ) {
           if (  !SelectionValid &&
                (r.dwEventFlags & DOUBLE_CLICK) ) {
               /* Double Click */
               debug( F100, "mouse B1 double click", "", 0 ) ;
               b1.state = 2 ;
               b1.row = r.dwMousePosition.Y ;
               b1.col = r.dwMousePosition.X ;
           }
           else {
               /* Single Click */
               b1.state = 1 ;
               b1.row = r.dwMousePosition.Y ;
               b1.col = r.dwMousePosition.X ;
               debug( F100, "mouse B1 single click", "", 0 ) ;
           }

           mr_button = MMB1;
           mr_event |= (b1.state == 1 ? MMCLICK : MMDBL);
       }
       else if ( (r.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && (r.dwEventFlags & MOUSE_MOVED) ) {
           debug( F100, "mouse B1 drag", "", 0 ) ;

           mr_button = MMB1;
           mr_event |= MMDRAG;

           /* Only start the drag event if we've actually got something bound
            * to it */
           if (!MOUSE_DRAG_EVENT_IGNORED(MMB1, Event)) {
               if ( b1.state == 1 && !SelectionValid && (MouseCurY != b1.row || MouseCurX != b1.col)) {
                   SelectionValid = 1 ;
                   RequestKeyStrokeMutex( mode, SEM_INDEFINITE_WAIT);
                   putkverb( mode, F_KVERB | K_MARK_START ) ;
                   mousecurpos( mode, Vrow, Vcol, b1.row, b1.col, TRUE ) ;
                   putkverb( mode, F_KVERB | K_MARK_START ) ;
                   mousecurpos( mode, b1.row, b1.col, MouseCurY, MouseCurX, TRUE );
                   ReleaseKeyStrokeMutex(mode) ;
               }
               else if ( b1.state == 1 && SelectionValid &&
                         ( lastrow != r.dwMousePosition.Y ||
                           lastcol != r.dwMousePosition.X )
                         ) {
                   mousecurpos( mode, lastrow, lastcol, r.dwMousePosition.Y,
                                r.dwMousePosition.X, TRUE ) ;
               }
           }
       }

       if ( (r.dwButtonState & (ThreeButton ? FROM_LEFT_2ND_BUTTON_PRESSED : RIGHTMOST_BUTTON_PRESSED)) && b2.state == 0 ) {
           if (  !SelectionValid &&
                (r.dwEventFlags & DOUBLE_CLICK) ) {
               /* Double Click */
               debug( F100, "mouse B2 double click", "", 0 ) ;
               b2.state = 2 ;
               b2.row = r.dwMousePosition.Y ;
               b2.col = r.dwMousePosition.X ;
           }
           else {
               /* Single Click */
               b2.state = 1 ;
               b2.row = r.dwMousePosition.Y ;
               b2.col = r.dwMousePosition.X ;
               debug( F100, "mouse B2 single click", "", 0 ) ;
           }

           mr_button = MMB2;
           mr_event |= (b2.state == 1 ? MMCLICK : MMDBL);
       }
       else if ( (r.dwButtonState & (ThreeButton ? FROM_LEFT_2ND_BUTTON_PRESSED : RIGHTMOST_BUTTON_PRESSED)) &&
                 (r.dwEventFlags & MOUSE_MOVED)) {
           debug( F100, "mouse B2 drag", "", 0 ) ;

           mr_button = MMB2;
           mr_event |= MMDRAG;

           /* Only start the drag event if we've actually got something bound
            * to it */
           if (!MOUSE_DRAG_EVENT_IGNORED(MMB2, Event)) {
               if ( b2.state == 1 && !SelectionValid && (MouseCurY != b2.row || MouseCurX != b2.col) ) {
                   SelectionValid = 1 ;
                   RequestKeyStrokeMutex( mode, SEM_INDEFINITE_WAIT);
                   putkverb( mode, F_KVERB | K_MARK_START ) ;
                   mousecurpos( mode, Vrow, Vcol, b2.row, b2.col, TRUE ) ;
                   putkverb( mode, F_KVERB | K_MARK_START ) ;
                   mousecurpos( mode, b2.row, b2.col, MouseCurY, MouseCurX, TRUE );
                   ReleaseKeyStrokeMutex(mode) ;
               }
               else if ( b2.state == 1 && SelectionValid &&
                         ( lastrow != r.dwMousePosition.Y ||
                           lastcol != r.dwMousePosition.X )
                         ) {
                   mousecurpos( mode, lastrow, lastcol, r.dwMousePosition.Y,
                                r.dwMousePosition.X, TRUE ) ;
               }
           }
       }

       if ( (r.dwButtonState & (ThreeButton ? RIGHTMOST_BUTTON_PRESSED : 0)) && b3.state == 0 ) {
           if (  !SelectionValid &&
                (r.dwEventFlags & DOUBLE_CLICK) ) {
               /* Double Click */
               b3.state = 2 ;
               debug( F100, "mouse B3 double click","",0) ;
           }
           else {
               /* Single Click */
               b3.state = 1 ;
               b3.row = r.dwMousePosition.Y ;
               b3.col = r.dwMousePosition.X ;
               debug( F100, "mouse B3 single click","",0) ;
           }

           mr_button = MMB3;
           mr_event |= (b3.state == 1 ? MMCLICK : MMDBL);
       }
       else if ( (r.dwButtonState & (ThreeButton ? RIGHTMOST_BUTTON_PRESSED : 0)) && (r.dwEventFlags & MOUSE_MOVED) ) {
           debug( F100, "mouse B3 drag","",0) ;

           mr_button = MMB3;
           mr_event |= MMDRAG;

           /* Only start the drag event if we've actually got something bound
            * to it */
           if (!MOUSE_DRAG_EVENT_IGNORED(MMB3, Event)) {
               if ( b3.state == 1 && !SelectionValid && (MouseCurY != b3.row || MouseCurX != b3.col) ) {
                   SelectionValid = 1 ;
                   RequestKeyStrokeMutex( mode, SEM_INDEFINITE_WAIT);
                   putkverb( mode, F_KVERB | K_MARK_START ) ;
                   mousecurpos( mode, Vrow, Vcol, b3.row, b3.col, TRUE ) ;
                   putkverb( mode, F_KVERB | K_MARK_START ) ;
                   mousecurpos( mode, b3.row, b3.col, MouseCurY, MouseCurX, TRUE );
                   ReleaseKeyStrokeMutex(mode) ;
               }
               else if ( b3.state == 1 && SelectionValid &&
                         ( lastrow != r.dwMousePosition.Y ||
                           lastcol != r.dwMousePosition.X )
                         ) {
                   mousecurpos( mode, lastrow, lastcol, r.dwMousePosition.Y,
                                r.dwMousePosition.X, TRUE ) ;
               }
           }
       }

       if (!mouse_reporting_override && mr_button != -1 &&
           MOUSE_EVENT_IGNORED(mr_button, mr_event)) {

           /* Mouse button press */

           /* Nothing is mapped for this mouse event - send a mouse report
            * if reporting is active */
           mouse_report(r.dwMousePosition.X,
                        r.dwMousePosition.Y,
                        mr_button + 1, /* K95 buttons start at 0 */
                        r.dwControlKeyState & CONTROL,
                        r.dwControlKeyState & SHIFT,
                        r.dwControlKeyState & ALT,
                        TRUE); /* button pressed */

           if (mr_event & MMDBL) {
               /* Double Click - send as to press/release events (the release
                * for the second press will come later when the user actually
                * releases the button) */
               mouse_report(r.dwMousePosition.X,
                        r.dwMousePosition.Y,
                        mr_button + 1, /* K95 buttons start at 0 */
                        r.dwControlKeyState & CONTROL,
                        r.dwControlKeyState & SHIFT,
                        r.dwControlKeyState & ALT,
                        FALSE); /* button released */

               mouse_report(r.dwMousePosition.X,
                        r.dwMousePosition.Y,
                        mr_button + 1, /* K95 buttons start at 0 */
                        r.dwControlKeyState & CONTROL,
                        r.dwControlKeyState & SHIFT,
                        r.dwControlKeyState & ALT,
                        TRUE); /* button pressed */
           }
       }
       mr_button = -1;
       mr_event = Event;

#ifndef NOSCROLLWHEEL
       if (r.dwEventFlags & MOUSE_WHEELED) {
           int zDelta = GET_WHEEL_DELTA_WPARAM(r.dwButtonState) / WHEEL_DELTA;
           int button;

           /* Rolling the wheel is always a click - never a double click or a
            * a drag because that wouldn't make much sense */
           Event |= MMWHEEL ;

           if (zDelta > 0) {
               /* Positive is foreward / up scroll */
               button = MMB4;
           } else {
               /* Negative is backward/ down scroll */
               button = MMB5;
               zDelta = zDelta * -1;
           }

           do {
               if (!mouse_reporting_override && MOUSE_EVENT_IGNORED(button, Event)) {
                   mouse_report(r.dwMousePosition.X,
                         r.dwMousePosition.Y,
                         button + 1,
                         r.dwControlKeyState & CONTROL,
                         r.dwControlKeyState & SHIFT,
                         r.dwControlKeyState & ALT,
                         TRUE);
               } else {
                   putevent( mode, mousemap[button][Event] ) ;
               }
               zDelta--;
           } while (zDelta > 0);
           putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
       }
#endif /* NOSCROLLWHEEL */

       if ( !(r.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) ) {
           /* button 1 may have been released */
           if ( b1.state == 1 ) {
               debug( F100, "mouse B1 single click released", "" , 0 );
               Event |= MMCLICK ;
               if ( SelectionValid ) {
                   Event |= MMDRAG ;
               }

               mr_button = MMB1;
               mr_event |= MMCLICK;

               /* Handle special Mouse Kverbs */
               if ( mousemap[MMB1][Event].type == kverb ) {
                   switch (mousemap[MMB1][Event].kverb.id & ~(F_KVERB)) {
                   case K_MOUSE_CURPOS:
                       putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
                       mousecurpos( mode, Vrow, Vcol, r.dwMousePosition.Y,
                                   r.dwMousePosition.X, FALSE ) ;
                       break;
                   case K_MOUSE_URL:
                       putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
                       mouseurl( mode, r.dwMousePosition.Y,
                                r.dwMousePosition.X );
                       break;
                   case K_MARK_COPYCLIP:
                   case K_MARK_COPYHOST:
                   case K_MARK_COPYCLIP_NOEOL:
                   case K_MARK_COPYHOST_NOEOL:
                   case K_DUMP:
                       putevent( mode, mousemap[MMB1][Event] ) ;
                       putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
                       break;
                   case K_MOUSE_MARK:
                   case K_IGNORE:
                       break;
                   default:
                       putevent( mode, mousemap[MMB1][Event] ) ;
                       putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
                   }
               }
               else if (mousemap[MMB1][Event].type != error) {
                   putevent( mode, mousemap[MMB1][Event] ) ;
                   putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
               }
               else {
                   putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
               }
               SelectionValid = 0 ;
               b1.state = 0 ;
               b1.row = 0 ;
               b1.col = 0 ;
#ifdef NT
               win32ScrollUp = win32ScrollDown = 0;
#endif /* NT */
           }
           else if ( b1.state == 2 ) {
               debug( F100, "mouse B1 double click released", "" , 0 );
               Event |= MMDBL ;

               mr_button = MMB1;
               mr_event |= MMDBL;

               /* Handle special Mouse Kverbs */
               if ( mousemap[MMB1][Event].type == kverb)
               {
                   switch (mousemap[MMB1][Event].kverb.id & ~(F_KVERB)) {
                   case K_MOUSE_MARK:
                   case K_IGNORE:
                       break;
                   case K_MOUSE_CURPOS:
                       mousecurpos( mode, Vrow, Vcol, r.dwMousePosition.Y,
                                    r.dwMousePosition.X, FALSE ) ;
                       break;
                   case K_MOUSE_URL:
                       mouseurl( mode, r.dwMousePosition.Y,
                                 r.dwMousePosition.X );
                       break;
                   default:
                       putevent( mode, mousemap[MMB1][Event] ) ;
                   }
               }
               else if ( mousemap[MMB1][Event].type != error ) {
                   putevent( mode, mousemap[MMB1][Event] ) ;
               }
               SelectionValid = 0 ;
               b1.state = 0 ;
               b1.row = 0 ;
               b1.col = 0 ;
#ifdef NT
               win32ScrollUp = win32ScrollDown = 0;
#endif /* NT */
           }
       }

       if ( !(r.dwButtonState & ( ThreeButton ? FROM_LEFT_2ND_BUTTON_PRESSED : RIGHTMOST_BUTTON_PRESSED ) ) ) {
           /* button 2 may have been released */
           if ( b2.state == 1 ) {
               debug( F100, "mouse B2 single click released", "" , 0 );
               Event |= MMCLICK ;
               if ( SelectionValid ) {
                   Event |= MMDRAG ;
               }

               mr_button = MMB2;
               mr_event |= MMCLICK;

               /* Handle special Mouse Kverbs */
               if ( mousemap[MMB2][Event].type == kverb)
               {
                   switch (mousemap[MMB2][Event].kverb.id & ~(F_KVERB)) {
                   case K_MOUSE_MARK:
                   case K_IGNORE:
                       break;
                   case K_MOUSE_CURPOS:
                       putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
                       mousecurpos( mode, Vrow, Vcol, r.dwMousePosition.Y,
                                    r.dwMousePosition.X, FALSE ) ;
                       break;
                   case K_MOUSE_URL:
                       putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
                       mouseurl( mode, r.dwMousePosition.Y,
                                   r.dwMousePosition.X );
                       break;
                   case K_MARK_COPYCLIP:
                   case K_MARK_COPYHOST:
                   case K_MARK_COPYCLIP_NOEOL:
                   case K_MARK_COPYHOST_NOEOL:
                   case K_DUMP:
                       putevent( mode, mousemap[MMB2][Event] ) ;
                       putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
                       break;
                   default:
                       putevent( mode, mousemap[MMB2][Event] ) ;
                       putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
                   }
               }
               else if (mousemap[MMB2][Event].type != error ) {
                   putevent( mode, mousemap[MMB2][Event] ) ;
                   putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
               }
               else {
                   putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
               }
               SelectionValid = 0 ;
               b2.state = 0 ;
               b2.row = 0 ;
               b2.col = 0 ;
#ifdef NT
               win32ScrollUp = win32ScrollDown = 0;
#endif /* NT */
           }
           else if ( b2.state == 2 ) {
               debug( F100, "mouse B2 double click released", "" , 0 );
               Event |= MMDBL ;

               mr_button = MMB2;
               mr_event |= MMDBL;

               /* Handle special Mouse Kverbs */
               if ( mousemap[MMB2][Event].type == kverb ) {
                   switch (mousemap[MMB2][Event].kverb.id & ~(F_KVERB)) {
                   case K_MOUSE_MARK:
                   case K_IGNORE:
                       break;
                   case K_MOUSE_CURPOS:
                       mousecurpos( mode, Vrow, Vcol, r.dwMousePosition.Y,
                                    r.dwMousePosition.X, FALSE ) ;
                       break;
                   case K_MOUSE_URL:
                       mouseurl( mode, r.dwMousePosition.Y,
                                 r.dwMousePosition.X );
                       break;
                   default:
                       putevent( mode, mousemap[MMB2][Event] ) ;
                   }
               }
               else if ( mousemap[MMB2][Event].type != error ) {
                   putevent( mode, mousemap[MMB2][Event] ) ;
               }
               SelectionValid = 0 ;
               b2.state = 0 ;
               b2.row = 0 ;
               b2.col = 0 ;
#ifdef NT
               win32ScrollUp = win32ScrollDown = 0;
#endif /* NT */
           }
       }

       if ( !(r.dwButtonState & (ThreeButton ? RIGHTMOST_BUTTON_PRESSED : 0) ) ) {
           /* button 3 may have been released */
           if ( b3.state == 1 ) {
               debug( F100, "mouse B3 single click released", "" , 0 );
               Event |= MMCLICK ;
                if ( SelectionValid ) {
                    Event |= MMDRAG ;
                }

                mr_button = MMB3;
                mr_event |= MMCLICK;

                /* Handle special Mouse Kverbs */
                if ( mousemap[MMB3][Event].type == kverb ) {
                     switch (mousemap[MMB3][Event].kverb.id & ~(F_KVERB)) {
                     case K_MOUSE_MARK:
                     case K_IGNORE:
                           break;
                     case K_MOUSE_CURPOS:
                           putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
                           mousecurpos( mode, Vrow, Vcol, r.dwMousePosition.Y,
                           r.dwMousePosition.X, FALSE ) ;
                           break;
                     case K_MOUSE_URL:
                         putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
                         mouseurl( mode, r.dwMousePosition.Y,
                                   r.dwMousePosition.X );
                         break;
                     case K_MARK_COPYCLIP:
                     case K_MARK_COPYHOST:
                     case K_MARK_COPYCLIP_NOEOL:
                     case K_MARK_COPYHOST_NOEOL:
                     case K_DUMP:
                         putevent( mode, mousemap[MMB3][Event] ) ;
                         putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
                         break;
                     default:
                         putevent( mode, mousemap[MMB3][Event] ) ;
                         putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
                     }
                  }
                else if ( mousemap[MMB3][Event].type != error ) {
                     putevent( mode,mousemap[MMB3][Event] ) ;
                    putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
                }
               else {
                   putkverb( mode, F_KVERB | K_MARK_CANCEL ) ;
               }
                  SelectionValid = 0 ;
                  b3.state = 0 ;
                  b3.row = 0 ;
                  b3.col = 0 ;
#ifdef NT
               win32ScrollUp = win32ScrollDown = 0;
#endif /* NT */
            }
            else if ( b3.state == 2 ) {
                debug( F100, "mouse B3 double click released", "" , 0 );
                Event |= MMDBL ;

                mr_button = MMB3;
                mr_event |= MMDBL;

               /* Handle special Mouse Kverbs */
                if ( mousemap[MMB3][Event].type == kverb ) {
               switch (mousemap[MMB3][Event].kverb.id & ~(F_KVERB)) {
               case K_MOUSE_MARK:
               case K_IGNORE:
                  break;
               case K_MOUSE_CURPOS:
                   mousecurpos( mode, Vrow, Vcol, r.dwMousePosition.Y,
                                r.dwMousePosition.X, FALSE ) ;
                  break;
               case K_MOUSE_URL:
                   mouseurl( mode, r.dwMousePosition.Y,
                             r.dwMousePosition.X );
                   break;
               default:
                   putevent( mode, mousemap[MMB3][Event] ) ;
               }
                }
                else if ( mousemap[MMB3][Event].type != error ) {
                    putevent( mode, mousemap[MMB3][Event] ) ;
                }
                SelectionValid = 0 ;
                b3.state = 0 ;
                b3.row = 0 ;
                b3.col = 0 ;
#ifdef NT
                win32ScrollUp = win32ScrollDown = 0;
#endif /* NT */
            }
       }

       if (!mouse_reporting_override && mr_button != -1 &&
           MOUSE_EVENT_IGNORED(mr_button, mr_event)) {

           debug(F100, "Reporting MB Release.", "", 0);

           /* Mouse button release. If this is for a double click, the remote
            * host has already received a press, a release, and a second press*/

           /* Nothing is mapped for this mouse event - send a mouse report
            * if reporting is active */
           mouse_report(r.dwMousePosition.X,
                        r.dwMousePosition.Y,
                        mr_button + 1, /* K95 buttons start at 0 */
                        r.dwControlKeyState & CONTROL,
                        r.dwControlKeyState & SHIFT,
                        r.dwControlKeyState & ALT,
                        FALSE); /* button released */
       }

       lastrow = r.dwMousePosition.Y ;
       lastcol = r.dwMousePosition.X ;
   }
   else  /* No Mouse Event */ {
   char buffer[1024] ;
      if ( SelectionValid ) {
         ppos = VscrnGetCurPos(mode) ;
         Vrow = ppos->y ;
         Vcol = ppos->x ;

         if ( lastrow == 0 &&
               VscrnGetBegin(mode) != VscrnGetScrollTop(mode)+Vrow ) {
               putkverb( mode, F_KVERB | K_UPONE ) ;
               }
         else if ( lastrow == VscrnGetHeight(mode) -2 &&
               VscrnGetEnd(mode) != VscrnGetScrollTop(mode)+Vrow ) {
               putkverb( mode, F_KVERB | K_DNONE ) ;
               }
         sprintf(buffer, "AutoScroll: fs:%3x row:%3d col:%3d",
         r.dwButtonState,
         r.dwMousePosition.Y, r.dwMousePosition.X) ;
         debug(F110,"win32MouseEvent",buffer,0) ;
      }
   #ifdef COMMENT
      else {
      sprintf(buffer, "NoEvent: fs:%3x row:%3d col:%3d",
      r.dwButtonState,
      r.dwMousePosition.Y, r.dwMousePosition.X) ;
      debug(F110,"win32MouseEvent",buffer,0) ;
      }
   #endif /* COMMENT */
   }
}

#else /* NT */
void
os2_mouseevt(void *pArgList) {
APIRET rc ;
MOUEVENTINFO MouseEventInfo ;
USHORT       ReadType = MOU_NOWAIT ;
position   * ppos ;
USHORT Vrow, Vcol, n, ShiftState, Event ;
USHORT lastrow=0, lastcol=0 ;
ULONG postcount ;
extern BYTE vmode;

struct {
    int state ;
    USHORT row   ;
    USHORT col   ;
    HTIMER timer ;
    HEV    sem ;
    } b1 = {0,0,0,0,(HEV) 0}, b2 = {0,0,0,0,(HEV) 0}, b3  = {0,0,0,0,(HEV) 0};

ResetThreadPrty() ;

DosCreateEventSem( NULL, &b1.sem, DC_SEM_SHARED, FALSE ) ;
DosCreateEventSem( NULL, &b2.sem, DC_SEM_SHARED, FALSE ) ;
DosCreateEventSem( NULL, &b3.sem, DC_SEM_SHARED, FALSE ) ;

while (hMouse)
    {
        memset( &MouseEventInfo, 0, sizeof( MOUEVENTINFO ) ) ;
        rc = MouReadEventQue( &MouseEventInfo, &ReadType, hMouse ) ;
        if ( rc == NO_ERROR && MouseEventInfo.time ) {
            char buffer[1024] ;
            ShiftState = getshiftstate() ;
            Event = 0 ;
            if ( ShiftState & SHIFT )
                Event |= MMSHIFT ;
            if ( ShiftState & ALT )
                Event |= MMALT ;
            if ( ShiftState & CONTROL )
                Event |= MMCTRL ;

            sprintf(buffer, "  Event: fs:%3x time:%10d row:%3d col:%3d",
                     MouseEventInfo.fs, MouseEventInfo.time,
                     MouseEventInfo.row, MouseEventInfo.col) ;
            debug(F110,"os2_mouseevt",buffer,0) ;
            if ( MouseDebug )
                printf(buffer);

            /* \V() variables */
            MouseCurX = MouseEventInfo.col;
            MouseCurY = MouseEventInfo.row;

            ppos = VscrnGetCurPos(vmode) ;
            Vrow = ppos->y ;
            Vcol = ppos->x ;

        if ( MouseEventInfo.row == VscrnGetHeight(vmode)
             -(tt_status[vmode]?1:0) )
            MouseEventInfo.row-- ;

            if ( MouseEventInfo.fs & MOUSE_BN1_DOWN ) {
                DosQueryEventSem( b1.sem, &postcount ) ;
            if ( b1.state == 1 &&
                !SelectionValid &&
                !postcount &&
                MouseEventInfo.row == b1.row &&
                MouseEventInfo.col == b1.col ) {
                    /* Double Click */
                debug( F100, "mouse B1 double click", "", 0 ) ;
                b1.state = 2 ;
                }
                else {
                    /* Single Click */
                b1.state = 1 ;
                b1.row = MouseEventInfo.row ;
                b1.col = MouseEventInfo.col ;
                DosResetEventSem( b1.sem, &postcount ) ;
                debug( F100, "mouse B1 single click", "", 0 ) ;
                }
            }
            else if ( MouseEventInfo.fs & MOUSE_MOTION_WITH_BN1_DOWN ) {
            debug( F100, "mouse B1 drag", "", 0 ) ;
                if ( b1.state == 1 && !SelectionValid ) {
                SelectionValid = 1 ;
                RequestKeyStrokeMutex( vmode, SEM_INDEFINITE_WAIT);
                putkverb( vmode, F_KVERB | K_MARK_START ) ;
                mousecurpos( vmode, Vrow, Vcol, b1.row, b1.col, TRUE ) ;
                putkverb( vmode, F_KVERB | K_MARK_START ) ;
                ReleaseKeyStrokeMutex(vmode) ;
                }
            else if ( b1.state == 1 && SelectionValid &&
                ( lastrow != MouseEventInfo.row ||
                lastcol != MouseEventInfo.col )
                ) {
                mousecurpos( vmode, lastrow, lastcol, MouseEventInfo.row,
                MouseEventInfo.col, TRUE ) ;
                }
            }

            if ( MouseEventInfo.fs & MOUSE_BN2_DOWN ) {
                DosQueryEventSem( b2.sem, &postcount ) ;
            if ( b2.state == 1 &&
                !SelectionValid &&
                !postcount &&
                MouseEventInfo.row == b2.row &&
                MouseEventInfo.col == b2.col ) {
                    /* Double Click */
                debug( F100, "mouse B2 double click", "", 0 ) ;
                b2.state = 2 ;
                }
                else {
                    /* Single Click */
                b2.state = 1 ;
                b2.row = MouseEventInfo.row ;
                b2.col = MouseEventInfo.col ;
                DosResetEventSem( b2.sem, &postcount ) ;
                debug( F100, "mouse B2 single click", "", 0 ) ;
                }
            }
            else if ( MouseEventInfo.fs & MOUSE_MOTION_WITH_BN2_DOWN ) {
            debug( F100, "mouse B2 drag", "", 0 ) ;
                if ( b2.state == 1 && !SelectionValid ) {
                SelectionValid = 1 ;
                RequestKeyStrokeMutex( vmode, SEM_INDEFINITE_WAIT);
                putkverb( vmode, F_KVERB | K_MARK_START ) ;
                mousecurpos( vmode, Vrow, Vcol, b2.row, b2.col, TRUE ) ;
                putkverb( vmode, F_KVERB | K_MARK_START ) ;
                ReleaseKeyStrokeMutex(vmode) ;
                }
            else if ( b2.state == 1 && SelectionValid &&
                ( lastrow != MouseEventInfo.row ||
                lastcol != MouseEventInfo.col )
                ) {
                mousecurpos( vmode, lastrow, lastcol, MouseEventInfo.row,
                MouseEventInfo.col, TRUE ) ;
                }
            }

            if ( MouseEventInfo.fs & MOUSE_BN3_DOWN ) {
                DosQueryEventSem( b3.sem, &postcount );
            if ( b3.state == 1 &&
                !SelectionValid &&
                !postcount &&
                MouseEventInfo.row == b3.row &&
                MouseEventInfo.col == b3.col ) {
                    /* Double Click */
                b3.state = 2 ;
                debug( F100, "mouse B3 double click","",0) ;
                }
                else {
                    /* Single Click */
                b3.state = 1 ;
                b3.row = MouseEventInfo.row ;
                b3.col = MouseEventInfo.col ;
                DosResetEventSem( b3.sem, &postcount ) ;
                debug( F100, "mouse B3 single click","",0) ;
                }
            }
            else if ( MouseEventInfo.fs & MOUSE_MOTION_WITH_BN3_DOWN ) {
            debug( F100, "mouse B3 drag","",0) ;
                if ( b3.state == 1 && !SelectionValid ) {
                SelectionValid = 1 ;
                RequestKeyStrokeMutex( vmode, SEM_INDEFINITE_WAIT);
                putkverb( vmode, F_KVERB | K_MARK_START ) ;
                mousecurpos( vmode, Vrow, Vcol, b3.row, b3.col, TRUE ) ;
                putkverb( vmode, F_KVERB | K_MARK_START ) ;
                ReleaseKeyStrokeMutex(vmode) ;
                }
            else if ( b3.state == 1 && SelectionValid &&
                ( lastrow != MouseEventInfo.row ||
                lastcol != MouseEventInfo.col )
                ) {
                mousecurpos( vmode, lastrow, lastcol, MouseEventInfo.row,
                MouseEventInfo.col, TRUE ) ;
                }
            }

        if ( !(MouseEventInfo.fs &
            ( MOUSE_BN1_DOWN | MOUSE_MOTION_WITH_BN1_DOWN) ) ) {
                    /* button 1 may have been released */
                if ( b1.state == 1 ) {
                    debug( F100, "mouse B1 single click released", "" , 0 );
                    if ( SelectionValid ) {
                    Event |= MMDRAG ;

                        /* Handle special Mouse Kverbs */
                        if ( mousemap[MMB1][Event].type == kverb ) {
                        switch (mousemap[MMB1][Event].kverb.id & ~(F_KVERB)) {
                        case K_MOUSE_CURPOS:
                            putkverb( vmode, F_KVERB | K_MARK_CANCEL ) ;
                            mousecurpos( vmode, Vrow, Vcol, MouseEventInfo.row,
                            MouseEventInfo.col, FALSE ) ;
                            break;
                        case K_MOUSE_URL:
                            putkverb( vmode, F_KVERB | K_MARK_CANCEL ) ;
                            mouseurl( vmode, MouseEventInfo.row,
                                      MouseEventInfo.col );
                            break;
                        case K_MARK_COPYCLIP:
                        case K_MARK_COPYHOST:
                        case K_MARK_COPYCLIP_NOEOL:
                        case K_MARK_COPYHOST_NOEOL:
                        case K_DUMP:
                            putevent( vmode, mousemap[MMB1][Event] ) ;
                            break;
                        case K_MOUSE_MARK:
                            break;
                        default:
                            putkverb( vmode, F_KVERB | K_MARK_CANCEL ) ;
                            putevent( vmode, mousemap[MMB1][Event] ) ;
                        }
                    }
                        else /* if ( mousemap[MMB1][Event].type != error ) */ {
                            putkverb( vmode, F_KVERB | K_MARK_CANCEL ) ;
                            putevent( vmode, mousemap[MMB1][Event] ) ;
                        }
                    SelectionValid = 0 ;
                    b1.state = 0 ;
                    b1.row = 0 ;
                    b1.col = 0 ;
                    }
                else {
                    DosResetEventSem( b1.sem, &postcount ) ;
                    DosAsyncTimer( dblclickspeed, (HSEM) b1.sem, &b1.timer ) ;
                    }
                }
                else if ( b1.state == 2 ) {
                    debug( F100, "mouse B1 double click released", "" , 0 );
                    Event |= MMDBL ;

                    /* Handle special Mouse Kverbs */
                    if ( mousemap[MMB1][Event].type == kverb ) {
                    switch (mousemap[MMB1][Event].kverb.id & ~(F_KVERB)) {
                    case K_MOUSE_MARK:
                        break;
                    case K_MOUSE_CURPOS:
                        mousecurpos( vmode, Vrow, Vcol, MouseEventInfo.row,
                        MouseEventInfo.col, FALSE ) ;
                        break;
                    case K_MOUSE_URL:
                        mouseurl( vmode, MouseEventInfo.row,
                                  MouseEventInfo.col );
                        break;
                    default:
                        putevent( vmode, mousemap[MMB1][Event] ) ;
                    }
                }
                    else /* if ( mousemap[MMB1][Event].type != error ) */ {
                        putevent( vmode, mousemap[MMB1][Event] ) ;
                    }
                b1.state = 0 ;
                b1.row = 0 ;
                b1.col = 0 ;
                }
            }

        if ( !(MouseEventInfo.fs &
            ( MOUSE_BN2_DOWN | MOUSE_MOTION_WITH_BN2_DOWN) ) ) {
                    /* button 2 may have been released */
                if ( b2.state == 1 ) {
                    debug( F100, "mouse B2 single click released", "" , 0 );
                    if ( SelectionValid ) {
                    Event |= MMDRAG ;

                        /* Handle special Mouse Kverbs */
                        if ( mousemap[MMB2][Event].type == kverb ) {
                        switch (mousemap[MMB2][Event].kverb.id & ~(F_KVERB)) {
                        case K_MOUSE_MARK:
                            break;
                        case K_MOUSE_CURPOS:
                            putkverb( vmode,F_KVERB | K_MARK_CANCEL ) ;
                            mousecurpos( vmode, Vrow, Vcol, MouseEventInfo.row,
                            MouseEventInfo.col, FALSE ) ;
                            break;
                        case K_MOUSE_URL:
                            putkverb( vmode, F_KVERB | K_MARK_CANCEL ) ;
                            mouseurl( vmode, MouseEventInfo.row,
                                      MouseEventInfo.col );
                            break;
                        case K_MARK_COPYCLIP:
                        case K_MARK_COPYHOST:
                        case K_MARK_COPYCLIP_NOEOL:
                        case K_MARK_COPYHOST_NOEOL:
                        case K_DUMP:
                            putevent( vmode, mousemap[MMB2][Event] ) ;
                            break;
                        default:
                            putkverb( vmode, F_KVERB | K_MARK_CANCEL ) ;
                            putevent( vmode, mousemap[MMB2][Event] ) ;
                        }
                    }
                        else /* if ( mousemap[MMB2][Event].type != error ) */ {
                            putkverb( vmode, F_KVERB | K_MARK_CANCEL ) ;
                            putevent( vmode, mousemap[MMB2][Event] ) ;
                        }
                    SelectionValid = 0 ;
                    b2.state = 0 ;
                    b2.row = 0 ;
                    b2.col = 0 ;
                    }
                else {
                    DosResetEventSem( b2.sem, &postcount ) ;
                    DosAsyncTimer( dblclickspeed, (HSEM) b2.sem, &b2.timer ) ;
                    }
                }
                else if ( b2.state == 2 ) {
                    debug( F100, "mouse B2 double click released", "" , 0 );
                Event |= MMDBL ;

                    /* Handle special Mouse Kverbs */
                    if ( mousemap[MMB2][Event].type == kverb) {
                    switch (mousemap[MMB2][Event].kverb.id & ~(F_KVERB)) {
                    case K_MOUSE_MARK:
                        break;
                    case K_MOUSE_CURPOS:
                        mousecurpos( vmode, Vrow, Vcol, MouseEventInfo.row,
                        MouseEventInfo.col, FALSE ) ;
                        break;
                    case K_MOUSE_URL:
                        mouseurl( vmode, MouseEventInfo.row,
                                  MouseEventInfo.col );
                        break;
                    default:
                        putevent( vmode, mousemap[MMB2][Event] ) ;
                    }
                }
                    else /* if ( mousemap[MMB2][Event].type != error ) */ {
                        putevent( vmode, mousemap[MMB2][Event] ) ;
                    }
                b2.state = 0 ;
                b2.row = 0 ;
                b2.col = 0 ;
                }
            }

        if ( !(MouseEventInfo.fs &
            ( MOUSE_BN3_DOWN | MOUSE_MOTION_WITH_BN3_DOWN) ) ) {
                    /* button 3 may have been released */
                if ( b3.state == 1 ) {
                    debug( F100, "mouse B3 single click released", "" , 0 );
                    if ( SelectionValid ) {
                    Event |= MMDRAG ;

                        /* Handle special Mouse Kverbs */
                        if ( mousemap[MMB3][Event].type == kverb ) {
                        switch (mousemap[MMB3][Event].kverb.id & ~(F_KVERB)) {
                        case K_MOUSE_MARK:
                            break;
                        case K_MOUSE_CURPOS:
                            putkverb( vmode, F_KVERB | K_MARK_CANCEL ) ;
                            mousecurpos( vmode, Vrow, Vcol, MouseEventInfo.row,
                            MouseEventInfo.col, FALSE ) ;
                            break;
                        case K_MOUSE_URL:
                            putkverb( vmode, F_KVERB | K_MARK_CANCEL ) ;
                            mouseurl( vmode, MouseEventInfo.row,
                                      MouseEventInfo.col );
                            break;
                        case K_MARK_COPYCLIP:
                        case K_MARK_COPYHOST:
                        case K_MARK_COPYCLIP_NOEOL:
                        case K_MARK_COPYHOST_NOEOL:
                        case K_DUMP:
                            putevent( vmode, mousemap[MMB3][Event] ) ;
                            break;
                        default:
                            putkverb( vmode, F_KVERB | K_MARK_CANCEL ) ;
                            putevent( vmode, mousemap[MMB3][Event] ) ;
                        }
                    }
                        else /* if ( mousemap[MMB3][Event].type != error ) */ {
                            putkverb( vmode, F_KVERB | K_MARK_CANCEL ) ;
                            putevent( vmode, mousemap[MMB3][Event] ) ;
                        }
                    SelectionValid = 0 ;
                    b3.state = 0 ;
                    b3.row = 0 ;
                    b3.col = 0 ;
                    }
                else {
                    DosResetEventSem( b3.sem, &postcount ) ;
                    DosAsyncTimer( dblclickspeed, (HSEM) b3.sem, &b3.timer ) ;
                    }
                }
                else if ( b3.state == 2 ) {
                    debug( F100, "mouse B3 double click released", "" , 0 );
                Event |= MMDBL ;

                    /* Handle special Mouse Kverbs */
                    if ( mousemap[MMB3][Event].type == kverb ) {
                    switch (mousemap[MMB3][Event].kverb.id & ~(F_KVERB)) {
                    case K_MOUSE_MARK:
                        break;
                    case K_MOUSE_CURPOS:
                        mousecurpos( vmode, Vrow, Vcol, MouseEventInfo.row,
                        MouseEventInfo.col, FALSE ) ;
                        break;
                    case K_MOUSE_URL:
                        mouseurl( vmode, MouseEventInfo.row,
                                  MouseEventInfo.col );
                        break;
                    default:
                        putevent( vmode, mousemap[MMB3][Event] ) ;
                    }
                }
                    else /* if ( mousemap[MMB3][Event].type != error ) */ {
                        putevent( vmode, mousemap[MMB3][Event] ) ;
                    }
                b3.state = 0 ;
                b3.row = 0 ;
                b3.col = 0 ;
                }
            }

        lastrow = MouseEventInfo.row ;
        lastcol = MouseEventInfo.col ;
        }

        else  /* No Mouse Event */ {
        char buffer[1024] ;
            if ( SelectionValid ) {
            ppos = VscrnGetCurPos(vmode) ;
            Vrow = ppos->y ;
            Vcol = ppos->x ;

            if ( lastrow == 0 &&
                VscrnGetBegin(vmode) != VscrnGetScrollTop(vmode)+Vrow ) {
                putkverb( vmode, F_KVERB | K_UPONE ) ;
                }
            else if ( lastrow == VscrnGetHeight(vmode) -(tt_status[vmode]?2:1) &&
                VscrnGetEnd(vmode) != VscrnGetScrollTop(vmode)+Vrow
                       ) {
                putkverb( vmode, F_KVERB | K_DNONE ) ;
                }
            sprintf(buffer, "AutoScroll: fs:%3x time:%10d row:%3d col:%3d",
            MouseEventInfo.fs, MouseEventInfo.time,
            MouseEventInfo.row, MouseEventInfo.col) ;
            debug(F110,"os2_mouseevt",buffer,0) ;
            }
            else if ( b1.state == 1 &&
                b1.timer &&
                !DosQueryEventSem( b1.sem, &postcount ) &&
                postcount ) {
            debug( F100, "mouse B1 timeout, must be a click", "", 0 ) ;
            Event |= MMCLICK ;

                /* Handle special Mouse Kverbs */
                if ( mousemap[MMB1][Event].type == kverb ) {
                switch (mousemap[MMB1][Event].kverb.id & ~(F_KVERB)) {
                case K_MOUSE_MARK:
                    break;
                case K_MOUSE_CURPOS:
                    mousecurpos( vmode, Vrow, Vcol, lastrow, lastcol, FALSE ) ;
                    break;
                case K_MOUSE_URL:
                    mouseurl( vmode, lastrow,
                              lastcol );
                    break;
                default:
                    putevent( vmode, mousemap[MMB1][Event] ) ;
                }
            }
                else /* if ( mousemap[MMB1][Event].type != error ) */ {
                    putevent( vmode, mousemap[MMB1][Event] ) ;
                }
            b1.state = 0 ;
            b1.row = 0 ;
            b1.col = 0 ;
            DosStopTimer( b1.timer ) ;
            b1.timer = 0 ;
            }
            else if ( b2.state == 1 &&
                b2.timer &&
                !DosQueryEventSem( b2.sem, &postcount ) &&
                postcount ) {
            debug( F100, "mouse B2 timeout, must be a click", "", 0 ) ;
            Event |= MMCLICK ;

                /* Handle special Mouse Kverbs */
                if ( mousemap[MMB2][Event].type == kverb ) {
                switch (mousemap[MMB2][Event].kverb.id & ~(F_KVERB)) {
                case K_MOUSE_MARK:
                    break;
                case K_MOUSE_CURPOS:
                    mousecurpos( vmode, Vrow, Vcol, lastrow, lastcol, FALSE ) ;
                    break;
                case K_MOUSE_URL:
                    mouseurl( vmode, lastrow,
                              lastcol );
                    break;
                default:
                    putevent( vmode, mousemap[MMB2][Event] ) ;
                }
            }
                else /* if ( mousemap[MMB2][Event].type != error ) */ {
                    putevent( vmode, mousemap[MMB2][Event] ) ;
                }
            b2.state = 0 ;
            b2.row = 0 ;
            b2.col = 0 ;
            DosStopTimer( b2.timer ) ;
            b2.timer = 0 ;
            }
            else if ( b3.state == 1 &&
                b3.timer &&
                !DosQueryEventSem( b3.sem, &postcount ) &&
                postcount ) {
            debug( F100, "mouse B3 timeout, must be a click", "", 0 ) ;
            Event |= MMCLICK ;

                /* Handle special Mouse Kverbs */
                if ( mousemap[MMB3][Event].type == kverb ) {
                switch (mousemap[MMB3][Event].kverb.id & ~(F_KVERB)) {
                case K_MOUSE_MARK:
                    break;
                case K_MOUSE_CURPOS:
                    mousecurpos( vmode, Vrow, Vcol, lastrow, lastcol, FALSE ) ;
                    break;
                case K_MOUSE_URL:
                    mouseurl( vmode, lastrow,
                              lastcol );
                    break;
                default:
                    putevent( vmode, mousemap[MMB3][Event] ) ;
                }
            }
                else /* if ( mousemap[MMB3][Event].type != error ) */ {
                    putevent( vmode, mousemap[MMB3][Event] ) ;
                }
            b3.state = 0 ;
            b3.row = 0 ;
            b3.col = 0 ;
            DosStopTimer( b3.timer ) ;
            b3.timer = 0 ;
            }

#ifdef COMMENT
            else {
            sprintf(buffer, "NoEvent: fs:%3x time:%10d row:%3d col:%3d",
            MouseEventInfo.fs, MouseEventInfo.time,
            MouseEventInfo.row, MouseEventInfo.col) ;
            debug(F110,"os2_mouseevt",buffer,0) ;
            }
#endif /* COMMENT */
        }
    msleep(1) ;
    }

DosCloseEventSem( b1.sem ) ;
DosCloseEventSem( b2.sem ) ;
DosCloseEventSem( b3.sem ) ;
ckThreadEnd(pArgList);
}
#endif /* NT */

APIRET
os2_mouseoff( void ) {
APIRET rc = 0 ;

#ifdef NT
extern HANDLE KbdHandle ;
DWORD mode ;

    mouseon = FALSE ;
    GetConsoleMode( KbdHandle, &mode ) ;
    mode &= ~ENABLE_MOUSE_INPUT ;
    rc = SetConsoleMode( KbdHandle, mode ) ;
    debug(F101,"Mouse Off","",rc) ;

#else /* NT */
    if ( hMouse ) {
    rc = MouClose( hMouse ) ;
    debug(F101,"Mouse Off","",rc) ;
    hMouse = 0 ;
    DosWaitThread( &tidMouse, DCWW_WAIT ) ;
    debug(F100,"os2_mouevt() thread dead","",0) ;
    tidMouse = 0 ;
    }
#endif /* NT */
return rc ;
}

#endif /* OS2MOUSE */


