#ifdef NT
char *connv = "Win32 CONNECT command 8.0.232, 20 Oct 2003";
#else /* NT */
char *connv = "OS/2 CONNECT command 8.0.232, 20 Oct 2003";
#endif /* NT */

/* C K O C O N  --  Kermit CONNECT command for OS/2 and Windows */

/*
  Authors: Frank da Cruz <fdc@columbia.edu>,
             Kermit Project, Columbia University, New York City.
           Jeffrey E Altman <jaltman@secure-endpoints.com>
             Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004,
    Trustees of Columbia University in the City of New York.
    All rights reserved.


  Originally adapted to OS/2 by Chris Adie of Edinburgh University, Scotland
  (C.Adie@edinburgh.ac.uk), 1988, from the UNIX Kermit CONNECT module by Frank
  da Cruz, Columbia University.  VT102 terminal emulation originally by Chris
  Adie, 1988 ("If the code looks a bit funny sometimes, it's because it was
  machine translated to C.")  (From what??)

  Adapted to c-Kermit 5A by Kai Uwe Rommel (rommel@informatik.tu-muenchen.de),
  1992-93 (present address: rommel@ars.muc.de).

  Many changes by Frank da Cruz (fdc@columbia.edu): numerous bug fixes (tabs,
  cursor/attributes save/restore, scrolling, keypad modes, etc), new SET
  TERMINAL commands, TCP/IP TELNET support, printer support, character-set
  support, APC escape sequence handling, print/dump screen, complete rewrite
  of character attribute handling & screen reversal and rollback, addition of
  VT220 and ANSI emulations, keyboard verbs, Compose key, Hebrew features,
  Cyrillic features, context-sensitive popup screens, cosmetic improvements,
  commentary, etc: 1992-present.  (Hopefully much of the funny-looking code
  looks less funny now.)

  Massive improvements by Jeffrey Altman <jaltman@secure-endpoints.com>:
  changes to keyboard handling, mouse support, ...: 1993 to present.
  (191) new connect mode screen handling, scrollback, ...
  (192) Win32 support, many new emulations, new keyboard scan codes, ... */

/*
 * =============================#includes=====================================
 */

#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckcker.h"             /* Kermit definitions */
#include "ckcasc.h"             /* ASCII character symbols */
#include "ckcxla.h"             /* Character set translation */
#include "ckcnet.h"             /* Network support */
#include "ckuusr.h"             /* For terminal type definitions, etc. */

#include <ctype.h>              /* Character types */
#include <io.h>                 /* File io function declarations */
#include <process.h>            /* Process-control function declarations */
#include <stdlib.h>             /* Standard library declarations */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#ifdef NT
#include <windows.h>
#include <tapi.h>
#include "ckntap.h"
#include "cknwin.h"
#ifdef KUI
#include "kui/ikui.h"
#endif /* KUI */
#else /* NT */
#ifdef OS2MOUSE
#define INCL_MOU
#endif /* OS2MOUSE */

#define INCL_WIN
#define INCL_VIO
#define INCL_ERRORS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVIOCTL
#define INCL_WINCLIPBOARD
#define INCL_DOSDATETIME
#include <os2.h>
#undef COMMENT                /* COMMENT is defined in os2.h */
#endif /* NT */

#include "ckowin.h"
#include "ckcuni.h"
#include "ckocon.h"             /* definitions common to console routines */
#include "ckokey.h"

#ifdef CK_NETBIOS
#include "ckonbi.h"
extern UCHAR NetBiosRemote[] ;
#endif /* CK_NETBIOS */


/*
 *
 * =============================externals=====================================
 */
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR);  /* Character set xlate */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR);  /* functions. */
extern int language;            /* Current language. */
extern struct langinfo langs[]; /* Language info. */
extern struct csinfo fcsinfo[]; /* File character set info */
extern int tcsr, tcsl;          /* Terminal character sets, remote & local. */
extern int tcs_transp;
extern struct _vtG G[4];         /* G0->G3 character sets */

extern int tnlm, tn_nlm;        /* Terminal newline mode, ditto for TELNET */
extern int me_binary, u_binary; /* Telnet binary modes */
extern int tt_crd;              /* Carriage-return display mode */
extern int tt_lfd;              /* Line-feed display mode */
extern int tt_bell;             /* How to handle incoming Ctrl-G characters */
extern long     speed, vernum;
extern int      local, escape, duplex, parity, flow, seslog, ttyfd,
                cmdmsk, cmask, sosi, xitsta, debses, mdmtyp, carrier, what;
extern int      cflg, cnflg, stayflg, tt_escape;
extern int      network, nettype, ttnproto;
extern int     tt_status[VNUM] ;
extern int     quiet, tn_exit, exitonclose;
extern int     SysInited;
#ifndef NOSPL
extern struct mtab *mactab;             /* Main macro table */
extern int nmac;                        /* Number of macros */
#endif /* NOSPL */

extern KEY      *keymap;
extern MACRO    *macrotab;
extern char     ttname[];
extern char *versio, *ckxsys;

#ifdef COMMENT
extern HAB hab ;
#endif /* COMMENT */

extern enum markmodes markmodeflag[VNUM] ;

/*
 * =============================variables==============================
 */

/*
  These are RGB bits for the fore- and background colors in the PC's video
  adapter, 3 bits for each color.  These default values can be changed by the
  SET TERMINAL COLOR command (in ckuus7.c) or by CSI3x;4xm escape sequences
  from the host.
*/
extern unsigned char     colorhelp;
extern unsigned char     colorcmd;

int     scrninitialized[VNUM] = {0,0,0} ;
bool    scrollflag[VNUM]  = {0,0,0}, flipscrnflag[VNUM] = {0,0,0};
extern bool scrollstatus[] ;
bool    viewonly = FALSE ;              /* View only terminal mode */
int     updmode         = TTU_FAST ;    /* Fast/Smooth scrolling */
int     priority        = XYP_REG ;

int tn_bold = 0;                        /* TELNET negotiation bold */
int esc_exit = 0;                       /* Escape back = exit */
char * esc_msg;

long waittime;                          /* Timeout on CTS during CONNECT */
#define INTERVAL 100L

char termessage[MAXTERMCOL];
#ifdef CK_APC
extern int apcactive;                   /* Application Program Command (APC) */
#endif /* CK_APC */
#ifdef IKSD
extern int inserver;
#endif /* IKSD */
#ifdef KUI
int gui_dialog = 1;
#else
int gui_dialog = 0;
#endif /* KUI */

char * keydefptr = NULL;
int keymac = 0;
int keymacx = -1 ;
int pmask = 0377 ;
extern videobuffer vscrn[];

ascreen                                 /* For saving screens: */
  vt100screen,                          /* terminal screen */
  commandscreen ;                       /* OS/2 screen */

extern unsigned char                    /* Video attribute bytes */
  attribute,                            /* Current video attribute byte */
  savedattribute,                       /* Saved video attribute byte */
  defaultattribute;                     /* Default video attribute byte */

int wherex[VNUM]={1,1,1};               /* Screen column, 1-based */
int wherey[VNUM]={1,1,1};               /* Screen row, 1-based */

int ConnectMode ;
int quitnow, hangnow, outshift, tcs, langsv;

#ifdef NT
char answerback[81] = "K-95\r";         /* Default answerback */
#else /* NT */
char answerback[81] = "K-95 for OS/2\r";/* Default answerback */
#endif /* NT */
char useranswerbk[60] = "" ;            /* User answerback extension */
int  safeanswerbk = TRUE ;              /* Require Kermit Product Prefix */
char usertext[(MAXTERMCOL) + 1];        /* Status line and its parts */
char statusline[MAXTERMCOL + 1];
char exittext[(20) + 1];
#define HLPTXTLEN 41
char helptext[HLPTXTLEN];
char filetext[(20) + 1];
#define HSTNAMLEN 41
char hostname[HSTNAMLEN];

#ifdef NT
CK_CURSORINFO crsr_command={88,0,8,1};
#else
CK_CURSORINFO crsr_command={-88,0,8,1};
#endif /* NT */

extern bool     cursoron[VNUM]  = {TRUE,TRUE,TRUE};       /* For speed, turn off when busy */
extern bool     cursorena[VNUM] = {TRUE,TRUE,TRUE};       /* Cursor enabled / disabled */

extern bool     printon;                /* Printer is on */
extern bool     xprint;                 /* Controller print in progress */
extern bool     aprint;                 /* Auto print in progress */
extern bool     uprint;                 /* User/transparent print in progress */
extern bool     keylock;
extern bool     deccolm;                /* 80/132-column mode */
extern bool     decscnm;                /* Normal/reverse screen mode */
extern struct _vtG G[4];                /* Graphic Set definitions */

/*
  Terminal parameters that can also be set externally by SET commands.
  Formerly they were declared and initialized here, and had different
  names, as shown in the comments.  Now they are declared and
  initialized in ckuus7.c.  - fdc
*/
bool tt_hebrew = 0;                     /* Keyboard is in Hebrew mode */

/* SET TERMINAL values ... */

extern struct tt_info_rec tt_info[] ;   /* Indexed by terminal type */
extern int max_tt;                      /* Highest terminal type */

extern int tt_arrow;                    /* Arrow-key mode */
extern int tt_keypad;                   /* Keypad mode */
extern int tt_wrap;                     /* Autowrap */
extern int tt_type;                     /* Terminal type */
extern int tt_type_mode;                /* Terminal type */
extern int tt_cursor;                   /* Cursor type */
extern int tt_answer;                   /* Answerback enabled/disabled */
extern int tt_scrsize[];                /* Scrollback buffer size */
extern int tt_roll[];                   /* Scrollback style */
extern int tt_rows[];                   /* Screen rows */
extern int tt_cols[];                   /* Screen columns */
extern int tt_cols_usr;
extern int tt_szchng[VNUM];
extern int cmd_rows;                    /* Screen rows */
extern int cmd_cols;                    /* Screen columns */
extern int tt_ctstmo;                   /* CTS timeout */
extern int tt_pacing;                   /* Output-pacing */
extern int tt_updmode;                  /* Terminal Screen Update Mode */
extern bool escapestatus[VNUM] ;        /* are we in ESCAPE mode? */
/* extern int tt_idlesnd_tmo;           /* Idle Send Timeout, disabled */
extern char * tt_idlesnd_str;           /* Idle Send String, none */
extern int tt_idlelimit;                /* Auto-exit Connect when idle */
extern int tt_idleact;
extern int tt_timelimit;
static time_t keypress_t=0;             /* Time of last keypress */
static time_t idlesnd_t=0;              /* Time of last idle send */
int escstate ;
USHORT marginbot;                       /* Bottom of same, 1-based */
int tt_async = 0;                       /* asynchronous connect mode? */
int col_init = 0, row_init = 0;
/*
   For pushing back input characters,
   e.g. converting 8-bit controls to 7-bit escape sequences.
*/
int f_pushed = 0, c_pushed = 0, f_popped = 0;

unsigned char sgrcols[8] = {0, 4, 2, 6, 1, 5, 3, 7};

/* Function prototypes */
#ifndef NOSETKEY
/* Key names */
char *keyname(unsigned long);           /* Names of keys, by scan code */
extern vik_rec vik;
#endif /* NOSETKEY */

/* Thread stuff... */
#define THRDSTKSIZ      131072          /* Needed for Mouse Thread */

extern TID tidRdComWrtScr,
    tidConKbdHandler,
    tidTermScrnUpd;

extern BYTE vmode ;

int
  oskipesc = 0,                         /* Esc seq recognizer for keys... */
  os2_outesc = ES_NORMAL;

#ifndef NOLOCAL
#ifndef NT
#define CK_BORDER                       /* Allow border colors */
#endif /* NT */

int cmd_border = -1;

void
setborder() {
#ifdef CK_BORDER
    extern HVIO VioHandle ;
    extern unsigned char borderattribute ;
    VIOOVERSCAN vo;                     /* Set border (overscan) color */

    vo.cb = sizeof(vo);                 /* for terminal emulation window */
    vo.type = 1;
    vo.color = borderattribute ;
    VioSetState((PVOID) &vo, VioHandle );
#endif /* CK_BORDER */
}

void
saveborder() {                          /* Save command-screen border */
#ifdef CK_BORDER
    extern HVIO VioHandle ;
    VIOOVERSCAN vo;
    vo.cb = sizeof(vo);
    vo.type = 1;
    VioGetState((PVOID) &vo, VioHandle);
    cmd_border = vo.color;
#endif /* CK_BORDER */
}

void
restoreborder() {
#ifdef CK_BORDER
    extern HVIO VioHandle ;
    VIOOVERSCAN vo;                     /* Restore command-screen border */
    if (cmd_border < 0)                 /* None saved. */
      return;
    vo.cb = sizeof(vo);
    vo.type = 1;
    vo.color = cmd_border;
    cmd_border = 0;
    VioSetState((PVOID) &vo, VioHandle);
#endif /* CK_BORDER */
}

/* Save current status of screen */
void
SaveTermMode(int wherex, int wherey) {
    debug(F101,"x_save wherex","",wherex);
    debug(F101,"x_save wherey","",wherey);
#ifndef KUI
    if (GetMode( &vt100screen.mi ))
        return;
#endif /* KUI */
    vt100screen.ox = wherex;
    vt100screen.oy = wherey;
    vt100screen.att = attribute;
    vt100screen.scrncpy ;  /* not used for terminal mode */
}

/* Restore a saved screen */
void
RestoreTermMode(void) {
    extern int tt_modechg;
    int omode = vmode ;

#ifndef KUI
    if ( tt_modechg == TVC_ENA )
        SetMode( &vt100screen.mi ) ;
    else if ( tt_modechg == TVC_W95 ) {
        CK_VIDEOMODEINFO wmi = vt100screen.mi ;
        wmi.col = 80 ;
        SetMode( &wmi ) ;
    }
#endif /* KUI */
    vmode = VTERM ;
    VscrnIsDirty(omode);
    VscrnForceFullUpdate();
    VscrnIsDirty(VTERM);
}

/* Save current status of command screen */
void
SaveCmdMode(int wherex, int wherey) {
    debug(F101,"x_save wherex","",wherex);
    debug(F101,"x_save wherey","",wherey);
#ifndef KUI
    if (GetMode( &commandscreen.mi ))
        return;
    commandscreen.mi.row = (commandscreen.mi.row > MAXSCRNROW ?
                             MAXSCRNROW : commandscreen.mi.row) ;
    commandscreen.mi.col = (commandscreen.mi.col > MAXSCRNCOL ?
                             MAXSCRNCOL : commandscreen.mi.col);
#ifdef NT
    commandscreen.mi.sbcol = commandscreen.mi.col;
    commandscreen.mi.sbrow = commandscreen.mi.row;
#endif /* NT */
#endif /* KUI */
    commandscreen.ox = wherex;
    commandscreen.oy = wherey;
    commandscreen.att = colorcmd;
}

/* Restore a saved command screen */
void
RestoreCmdMode() {
    extern int tt_modechg;
    int omode = vmode ;

#ifndef KUI
    if ( tt_modechg == TVC_ENA )
        SetMode( &commandscreen.mi ) ;
    else if ( tt_modechg == TVC_W95 ) {
        CK_VIDEOMODEINFO wmi = commandscreen.mi ;
        wmi.col = 80 ;
        SetMode( &wmi ) ;
    }
#endif /* KUI */
#ifdef COMMENT
    colorcmd = commandscreen.att;
    wherey[VTERM] = commandscreen.oy;
    wherex[VTERM] = commandscreen.ox;
#endif /* COMMENT */
    debug(F101,"x_rest wherex","",commandscreen.ox);
    debug(F101,"x_rest wherey","",commandscreen.oy);
    SetCurPos( commandscreen.oy-1, commandscreen.ox-1 ) ;
   /* lgotoxy no longer moves */
   /* the physical cursor     */
    vmode = VCMD ;
    VscrnIsDirty(omode);
    VscrnForceFullUpdate();
    VscrnIsDirty(VCMD);
}
#endif /* NOLOCAL */

void                                    /* Push from CONNECT mode to OS/2 */
os2push() {                             /* not just for CONNECT mode anymore */
#ifndef NOPUSH
#ifndef KUI
    CK_VIDEOMODEINFO mi;
#endif /* KUI */
    int connectmode = IsConnectMode() ;
    extern int nopush;
    if ( nopush )
    {
        bleep( BP_WARN ) ;
        return;
    }

#ifndef KUI
    if (GetMode(&mi))
         return;
#endif /* KUI */
    if ( connectmode ) {
        SaveTermMode(wherex[VTERM],wherey[VTERM]) ;
        concooked();
#ifdef NT
        setint();
#endif
    }

    RequestScreenMutex(SEM_INDEFINITE_WAIT);
    clearcmdscreen();

    if ( connectmode ) {
        restorecursormode();
        puts("Enter EXIT to return to Kermit-95 Connect Mode.");
    }
    else
        puts("Enter EXIT to return to Kermit-95 Command Mode.");

    zshcmd("");

    if ( connectmode ) {
        conraw();
        connoi();
#ifndef KUI
        SetMode(&mi);
#endif /* KUI */
        setborder();                    /* Put back CONNECT screen border */
        setcursormode();
        RestoreTermMode();
    }
    ReleaseScreenMutex();

    if ( connectmode ) {
        cursoron[VTERM] = FALSE ;      /* Force update just in case */
        VscrnIsDirty(VTERM);
    }
    else
        VscrnIsDirty(VCMD);
#endif /* NOPUSH */
}

/*---------------------------------------------------------------------------*/
/* bleep                                                                     */
/*---------------------------------------------------------------------------*/
int beepfreq = DEF_BEEP_FREQ ;                  /* Beep Frequency */
int beeptime = DEF_BEEP_TIME ;                  /* Beep Duration  */

void
bleep(short int type) {
    debug(F101,"bleep","",type);
#ifdef IKSD
    if ( inserver ) {
        if ( ttyfd != -1 )
            printf("%c",BEL);
        return;
    }
#endif /* IKSD */

#ifndef IKSDONLY
    switch(tt_bell) {                   /* Follows TERMINAL BELL setting. */
    case XYB_AUD | XYB_BEEP:            /* AUDIBLE - BEEP */
        switch (type) {
#ifndef NT
        case BP_NOTE:
            DosBeep( WinQuerySysValue(HWND_DESKTOP,SV_NOTEFREQ),
                     WinQuerySysValue(HWND_DESKTOP,SV_NOTEDURATION));
            break;
        case BP_WARN:
            DosBeep( WinQuerySysValue(HWND_DESKTOP,SV_WARNINGFREQ),
                     WinQuerySysValue(HWND_DESKTOP,SV_WARNINGDURATION));
            break;
        case BP_FAIL:
            DosBeep( WinQuerySysValue(HWND_DESKTOP,SV_ERRORFREQ),
                     WinQuerySysValue(HWND_DESKTOP,SV_ERRORDURATION));
            break;
        case BP_BEL:
            DosBeep(beepfreq,beeptime);
            break;
        default:
            DosBeep(DEF_BEEP_FREQ,DEF_BEEP_TIME);
#else /* NT */
        case BP_BEL:
            Beep(beepfreq,beeptime);
            break;
        default:
            Beep(DEF_BEEP_FREQ,DEF_BEEP_TIME ) ;
#endif /* NT */
        }
        return;

      case XYB_AUD | XYB_SYS:       /* AUDIBLE - SYSTEM-SOUNDS */
        switch (type) {
#ifndef NT
        case BP_NOTE:
            WinAlarm(HWND_DESKTOP, WA_NOTE);
            break;
        case BP_WARN:
            WinAlarm(HWND_DESKTOP, WA_WARNING);
            break;
        case BP_FAIL:
        case BP_BEL:
        default:
            WinAlarm(HWND_DESKTOP, WA_ERROR);
            break;
#ifdef COMMENT
        case BP_BEL:
            DosBeep(beepfreq,beeptime);
            break;
        default:
            DosBeep(DEF_BEEP_FREQ,DEF_BEEP_TIME);
#endif /* COMMENT */
#else /* NT */
        case BP_NOTE:
            MessageBeep( MB_ICONQUESTION );
            break;
        case BP_WARN:
            MessageBeep( MB_ICONASTERISK );
            break;
        case BP_FAIL:
        case BP_BEL:
        default:
            MessageBeep( MB_ICONEXCLAMATION );
            break;
#ifdef COMMENT
        case BP_BEL:
            Beep(beepfreq,beeptime);
            break;
        default:
            MessageBeep(0xFFFFFFFF);            /* Standard Beep */
#endif /* COMMENT */
#endif /* NT */
        }
        return;

    case XYB_VIS:                               /* VISIBLE */
        if ( IsConnectMode() ) {                /* in terminal mode */
            if ( flipscrnflag[VTERM] )          /* Flash the screen */
                flipscrnflag[VTERM] = FALSE ;
            else
                flipscrnflag[VTERM] = TRUE ;
            VscrnForceFullUpdate();
            VscrnIsDirty(VTERM);
            msleep(250);                        /* for 250 msec */
            if ( flipscrnflag[VTERM] )          /* Flash the screen */
                flipscrnflag[VTERM] = FALSE ;
            else
                flipscrnflag[VTERM] = TRUE ;
            VscrnForceFullUpdate();
            VscrnIsDirty(VTERM);
        }
        else {                                  /* in command mode */
            if ( flipscrnflag[VCMD] )           /* Flash the screen */
                flipscrnflag[VCMD] = FALSE ;
            else
                flipscrnflag[VCMD] = TRUE ;
            /* reversescreen(VTERM); */         /* Flash the screen */
            VscrnForceFullUpdate();
            VscrnIsDirty(VCMD);
            msleep(250);                        /* for 250 msec */
            if ( flipscrnflag[VCMD] )           /* Flash the screen */
                flipscrnflag[VCMD] = FALSE ;
            else
                flipscrnflag[VCMD] = TRUE ;
            /* reversescreen(VTERM); */         /* Flash the screen */
            VscrnForceFullUpdate();
            VscrnIsDirty(VCMD);
        }
        return;

      default:                          /* NONE or other */
        return;
    }
#endif /* IKSDONLY */
}

#ifndef NOLOCAL
/*             */
/* getcmdcolor */
/*             */
void
getcmdcolor(void)
{
#ifndef KUI
    viocell cell ;
    USHORT  x,y ;
    USHORT  length = 1;
    GetCurPos( &x, &y ) ;
    ReadCellStr( &cell, &length, x, y ) ;
    colorcmd = cell.a ;
#endif /* KUI */
}

/*---------------------------------------------------------------------------*/
/* clearcmdscreen                                                            */
/*---------------------------------------------------------------------------*/
void
clearcmdscreen(void) {
    viocell         cell ;

    ttgcwsz();
    cell.c = ' ' ;
    cell.a = colorcmd ;
    WrtNCell(cell, cmd_cols * (cmd_rows+1), 0, 0);
    SetCurPos( 0, 0 ) ;
}

/*---------------------------------------------------------------------------*/
/* clearscrollback                                                           */
/*---------------------------------------------------------------------------*/
void
clearscrollback( BYTE vmode ) {
    ULONG bufsize = VscrnGetBufferSize(vmode) ;

    VscrnSetBufferSize( vmode, 256 ) ;
    VscrnSetBufferSize( vmode, bufsize ) ;
    scrollstatus[vmode] = FALSE ;
    scrollflag[vmode] = FALSE ;
    cursoron[vmode] = FALSE ;
    cleartermscreen(vmode) ;
}

/*---------------------------------------------------------------------------*/
/* cleartermscreen                                                               */
/*---------------------------------------------------------------------------*/
void
cleartermscreen( BYTE vmode ) {
    int             x,y ;
    videoline *     line ;

    for ( y = 0 ; y < VscrnGetHeight(vmode) ; y++ ) {
        line = VscrnGetLineFromTop(vmode,y) ;
        line->width = VscrnGetWidth(vmode) ;
        line->vt_line_attr = VT_LINE_ATTR_NORMAL ;
        for ( x = 0 ; x < MAXTERMCOL ; x++ ) {
            line->cells[x].c = ' ' ;
            line->cells[x].a = vmode == VTERM ? attribute : colorcmd ;
            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
            }
        }
    lgotoxy(vmode,1, 1);
    if ( IsConnectMode() || vmode != VTERM )
        VscrnIsDirty(vmode);
    else {
        vt100screen.ox = vt100screen.oy = 1 ;
        }
}

/* POPUPHELP  --  Give help message for connect.  */
static int helpcol, helprow;
static int helpwidth;

videopopup *
helpstart(int w, int h, int gui) {               /* Start help window */
    videopopup * pPopup = malloc(sizeof(videopopup)) ;

    if (pPopup == NULL)
        return(NULL);

#ifdef KUI
    if ( gui ) {
        helpcol = 0;        
        helprow = -1;
        pPopup->width = w;
        pPopup->height = h;
        pPopup->gui = 1;
    } else
#endif /* KUI */
    {
        helpcol = helprow = 0 ;
        pPopup->a = colorhelp ;
        pPopup->width = w + 4 ;   /* 2 for border, plus 2 for padding */
        pPopup->height = h + 2 ;  /* 2 for border */
        pPopup->c[helprow][helpcol++] = 201; /* IBM upper left box corner double */
        pPopup->gui = 0;
        for ( helpcol ; helpcol < pPopup->width-1 ; helpcol++ )
            pPopup->c[helprow][helpcol] = 205 ;
                                             /* Center box bar horizontal double */
        pPopup->c[helprow][helpcol] = 187;   /* Upper right box corner double */
    }
    return pPopup ;
}

void
helpline(videopopup *pPopup, char *s) {                 /* Add line to popup help */
    int l;

    l = strlen(s);                      /* Length of this line */
    helprow++;
    helpcol=0;

#ifdef KUI
    if ( pPopup->gui ) {
        /* Add the string */
        for ( helpcol ; helpcol < l && helpcol < pPopup->width; helpcol++ )
            pPopup->c[helprow][helpcol] = s[helpcol] ;

        /* Pad with NUL */
        for ( helpcol ; helpcol < pPopup->width ; helpcol++ )
            pPopup->c[helprow][helpcol] = NUL;
    } else
#endif /* KUI */
    {
        pPopup->c[helprow][helpcol++] = 186;   /* IBM center bar vertical double */

        /* Add the string */
        for ( helpcol ; helpcol <= l && helpcol < (pPopup->width-2); helpcol++ )
            pPopup->c[helprow][helpcol] = s[helpcol-1] ;

        /* Pad with spaces */
        for ( helpcol ; helpcol < pPopup->width-1 ; helpcol++ )
            pPopup->c[helprow][helpcol] = SP;

        pPopup->c[helprow][helpcol++] = 186;   /* IBM center bar vertical double */
    }
}

void
helpend(videopopup * pPopup) {          /* End of popup help box */
    helprow++;
    helpcol=0;
#ifdef KUI
    if ( !pPopup->gui )
#endif /* KUI */
    {
        pPopup->c[helprow][helpcol++] = 200; /* IBM lower left box corner double */
        for ( helpcol ; helpcol < pPopup->width-1 ; helpcol++ )
            pPopup->c[helprow][helpcol] = 205; /* Center box bar horizontal double */

        pPopup->c[helprow][helpcol++] = 188; /* Lower right box corner double */
    }
}

int
popuperror(int mode, char * msg ) {
    videopopup * pPopup = NULL ;
    int c=0;
    con_event evt ;
    extern int holdscreen;

    save_status_line();                 /* Save current status line */
    strcpy(usertext, " ERROR: Press (almost) any key to continue.");
    exittext[0] = helptext[0] = hostname[0] = NUL;

    pPopup = helpstart(strlen(msg), 1, gui_dialog);
    helpline( pPopup, msg ) ;
    helpend(pPopup);                    /* Write bottom of help panel */

#ifdef KUI
    if ( pPopup->gui ) {
        c = gui_videopopup_dialog(pPopup, 0);
    } else 
#endif /* KUI */
    {
#ifdef OS2ONLY
        VscrnSetPopup( mode, pPopup ) ;
        /* wait until user presses a key */
        evt = congev(mode,0);
        while ( evt.type != key && evt.type != kverb) {
            if ( evt.type != error )
                bleep( BP_WARN ) ;
            evt = congev(mode,0);
        }
        if ( evt.type == kverb )
            c = F_KVERB | evt.kverb.id ;
        else  {
            c = 0 ;
        }
        VscrnResetPopup(mode) ;            /* This frees the Popup structure */
#else /* OS2ONLY */
        ;
#endif /* OS2ONLY */
    }
    restore_status_line();
    return (c);
}

int
popupdemo(int mode, int secsleft ) {
    return(0);
}



int
popup_readtext(int mode, char * preface, char * prmpt, char * buffer, int buflen, int timo ) {
    videopopup * pPopup = NULL ;
    con_event evt ;
    char inputline[256];
    char prompt[1024];
    int input_width,popup_width,i,j,len=0;
    CHAR x1, * p, * q;
    int lines;
    int resp_line;

    if ( prmpt ) {
        ckstrncpy(prompt,prmpt,1024);
    }
    else
        strcpy(prompt,"Enter text:");

    if ( !buffer || buflen <= 0 )
        return(-1);

    i = 0;
    lines=1;

    if ( preface ) {
        lines += 2;
        p = q = preface;
        while ( *p ) {
            if ( *p == LF ) {
                lines++;
                if ( p - q > i )
                    i = p - q;
                q = p;
            }
            p++;
        }
        if ( q == preface )
            i = p - q;
    }

    p = q = prompt;
    while ( *p ) {
        if ( *p == LF ) {
            lines++;
            if ( p - q > i )
                i = p - q;
            q = p;
        }
        p++;
    }
    if ( q == prompt )
        i = p - q;
    j = VscrnGetWidth(mode);

    popup_width = (i > j - 10 || buflen > j - 10) ? j - 10 :
        (buflen > i) ? buflen : i;
    input_width = (buflen <= popup_width ? buflen : popup_width)-1;

    save_status_line();                 /* Save current status line */
    strcpy(usertext, " Enter text followed by Enter to continue");
    exittext[0] = helptext[0] = hostname[0] = NUL;

    pPopup = helpstart(popup_width, lines+1, 0);

    if ( preface ) {
	char * cp_preface = NULL;
	makestr(&cp_preface, preface);
        p = q = cp_preface;
        while ( *p ) {
            if ( *p == LF ) {
                *p = '\0';
                if ( *(p-1) == CR )
                    *(p-1) = '\0';
                helpline( pPopup, q );
                q = p+1;
            }
            p++;
        }
        helpline( pPopup, q );
        helpline( pPopup, "" );
	free(cp_preface);
    }

    p = q = prompt;
    while ( *p ) {
        if ( *p == LF ) {
            *p = '\0';
            if ( *(p-1) == CR )
                *(p-1) = '\0';
            helpline( pPopup, q );
            q = p+1;
        }
        p++;
    }
    helpline( pPopup, q );

    resp_line = helprow;
    sprintf(inputline,"[%*s]",input_width," ");
    helpline(pPopup,inputline);
    helpend(pPopup);                    /* Write bottom of help panel */
    VscrnSetPopup( mode, pPopup );

    buffer[0] = '\0';
    len = 0;

    do {
        VscrnIsDirty(vmode);  /* status line needs to be updated */

        evt = congev(vmode,timo > 0 ? timo : -1) ;
        switch (evt.type) {
        case key:
#ifdef COMMENT
            x1 = mapkey(evt.key.scancode); /* Get value from keymap */
#else
            x1 = evt.key.scancode ;
#endif
            break;
#ifndef NOKVERBS
        case kverb:
            x1 = evt.kverb.id | F_KVERB;
            switch ( evt.kverb.id & ~F_KVERB ) {
            case K_KPENTER  :
            case K_WYENTER  :
            case K_WYRETURN :
            case K_TVIENTER :
            case K_TVIRETURN:
            case K_HPENTER  :
            case K_HPRETURN :
                x1 = CR;
                break;
            default:
                bleep(BP_WARN);
                continue;
            }
            break;
#endif /* NOKVERBS */
        case macro:
            if ( evt.macro.string )
                free( evt.macro.string ) ;
            x1 = 0;
            break;
        case literal:
            if ( evt.literal.string )
                free( evt.literal.string ) ;
            x1 = 0;
            break;
        case error:
            if ( timo ) {
                len = -1;
                goto popup_exit;
            }
        case esc:
        case csi:
        default:
            x1 = 0 ;
        }

        if ( x1 >= ' ' && x1 <= 126 || x1 >= 128 && x1 <= 255 )
        {
            if ( len >= buflen - 1 ) {
                bleep(BP_WARN);
            }
            else {
                buffer[len] = x1 ;
                buffer[len+1] = '\0' ;
            }
        }
        else if ( x1 == 8 || x1 == 127 )
        {
            buffer[len-1] = '\0' ;
        }
        else if ( x1 == CR )
        {
            break;
        }
        else if ( x1 == 21 )
        {
            buffer[0] = '\0' ;
            len = 0 ;
        }
        else if ( x1 == ESC ) {
            buffer[0] = '\0' ;
            len = 0;
            break;
        }
        else
        {
            bleep(BP_FAIL);
        }

        len = strlen(buffer);
        helprow = resp_line;    /* we know the internal representation */
        sprintf(inputline,"[%-*s]",input_width,
                 (len > input_width) ? &buffer[len-input_width] : buffer);
        helpline(pPopup,inputline);
    } while ( TRUE );

  popup_exit:
    VscrnResetPopup(mode) ;            /* This frees the Popup structure */
    restore_status_line();
    return (len);
}


int
popup_readpass(int mode, char * preface, char * prmpt, char * buffer, int buflen, int timo ) {
    videopopup * pPopup = NULL ;
    con_event evt ;
    char inputline[256];
    char prompt[1024];
    int input_width,popup_width,i,j,len=0;
    CHAR x1, * p, * q;
    int lines;
    int resp_line;

    if ( prmpt ) {
        ckstrncpy(prompt,prmpt,1024);
    }
    else
        strcpy(prompt,"Enter password:");

    if ( !buffer || buflen <= 0 )
        return(-1);

    i = 0;
    lines=1;

    if ( preface ) {
        lines += 2;
        p = q = preface;
        while ( *p ) {
            if ( *p == LF ) {
                lines++;
                if ( p - q > i )
                    i = p - q;
                q = p;
            }
            p++;
        }
        if ( q == preface )
            i = p - q;
    }

    p = q = prompt;
    while ( *p ) {
        if ( *p == LF ) {
            lines++;
            if ( p - q > i )
                i = p - q;
            q = p;
        }
        p++;
    }
    if ( q == prompt )
        i = p - q;
    j = VscrnGetWidth(mode);

    popup_width = (i > j - 10 || buflen > j - 10) ? j - 10 :
        (buflen > i) ? buflen : i;
    input_width = (buflen <= popup_width ? buflen : popup_width)-1;

    save_status_line();                 /* Save current status line */
    strcpy(usertext, " Enter password followed by Enter to continue");
    exittext[0] = helptext[0] = hostname[0] = NUL;

    pPopup = helpstart(popup_width, lines+1, 0);

    if ( preface ) {
	char * cp_preface = NULL;
	makestr(&cp_preface, preface);
        p = q = cp_preface;
        while ( *p ) {
            if ( *p == LF ) {
                *p = '\0';
                if ( *(p-1) == CR )
                    *(p-1) = '\0';
                helpline( pPopup, q );
                q = p+1;
            }
            p++;
        }
        helpline( pPopup, q );
        helpline( pPopup, "" );
	free(cp_preface);
    }

    p = q = prompt;
    while ( *p ) {
        if ( *p == LF ) {
            *p = '\0';
            if ( *(p-1) == CR )
                *(p-1) = '\0';
            helpline( pPopup, q );
            q = p+1;
        }
        p++;
    }
    helpline( pPopup, q );

    resp_line = helprow;
    sprintf(inputline,"[%*s]",input_width," ");
    helpline(pPopup,inputline);
    helpend(pPopup);                    /* Write bottom of help panel */
    VscrnSetPopup( mode, pPopup );

    buffer[0] = '\0';
    len = 0;

    do {
        VscrnIsDirty(vmode);  /* status line needs to be updated */

        evt = congev(vmode,timo > 0 ? timo : -1) ;
        switch (evt.type) {
        case key:
#ifdef COMMENT
            x1 = mapkey(evt.key.scancode); /* Get value from keymap */
#else
            x1 = evt.key.scancode ;
#endif
            break;
#ifndef NOKVERBS
        case kverb:
            x1 = evt.kverb.id | F_KVERB;
            switch ( evt.kverb.id & ~F_KVERB ) {
            case K_KPENTER  :
            case K_WYENTER  :
            case K_WYRETURN :
            case K_TVIENTER :
            case K_TVIRETURN:
            case K_HPENTER  :
            case K_HPRETURN :
                x1 = CR;
                break;
            default:
                bleep(BP_WARN);
                continue;
            }
            break;
#endif /* NOKVERBS */
        case macro:
            if ( evt.macro.string )
                free( evt.macro.string ) ;
            x1 = 0;
            break;
        case literal:
            if ( evt.literal.string )
                free( evt.literal.string ) ;
            x1 = 0;
            break;
        case error:
            if ( timo ) {
                len = -1;
                goto popup_exit;
            }
        case esc:
        case csi:
        default:
            x1 = 0 ;
        }

        if ( x1 >= ' ' && x1 <= 126 || x1 >= 128 && x1 <= 255 )
        {
            if ( len >= buflen - 1 ) {
                bleep(BP_WARN);
            }
            else {
                buffer[len] = x1 ;
                buffer[len+1] = '\0' ;
            }
        }
        else if ( x1 == 8 || x1 == 127 )
        {
            buffer[len-1] = '\0' ;
        }
        else if ( x1 == CR )
        {
            break;
        }
        else if ( x1 == 21 )
        {
            buffer[0] = '\0' ;
            len = 0 ;
        }
        else if ( x1 == ESC ) {
            buffer[0] = '\0' ;
            len = 0;
            break;
        }
        else
        {
            bleep(BP_FAIL);
        }

        len = strlen(buffer);
        helprow = resp_line;    /* we know the internal representation */
        sprintf(inputline,"[%*s]",input_width," ");
        memset(&inputline[1],'*',len<input_width?len:input_width);
        helpline(pPopup,inputline);
    } while ( TRUE );

  popup_exit:
    VscrnResetPopup(mode) ;            /* This frees the Popup structure */
    restore_status_line();
    return (len);
}

char fkeylabel[16][32] = {"","","","","","","","",
                              "","","","","","","",""};
int
fkeypopup(int mode) {
    videopopup * pPopup = NULL ;
    int c,i;
    con_event evt ;
    char buffer[64];

    save_status_line();                 /* Save current status line */
    strcpy(usertext, " Function Key Labels: Press (almost) any key to continue.");
    exittext[0] = helptext[0] = hostname[0] = NUL;

    if ( ISWYSE(tt_type_mode) ) {
        pPopup = helpstart(32, 11, gui_dialog);
        helpline( pPopup, " Wyse Function Key Labels" ) ;
        helpline( pPopup, "" ) ;
        for ( i=0 ; i<8 ; i++ ) {
            sprintf(buffer,"  F%-2d:  \"%s\"",i+1,fkeylabel[i]);
            helpline( pPopup, buffer );
        }
        helpline( pPopup, "" ) ;
    }
    else if ( ISHP(tt_type_mode) ) {
        pPopup = helpstart(32, 19, gui_dialog );
        helpline( pPopup, " HP Function Key Labels" ) ;
        helpline( pPopup, "" ) ;
        for ( i=0 ; i<16 ; i++ ) {
            sprintf(buffer,"  F%-2d:  \"%s\"",i+1,fkeylabel[i]);
            helpline( pPopup, buffer );
        }
        helpline( pPopup, "" ) ;
    }
    else if ( ISBA80(tt_type_mode) ) {
        pPopup = helpstart(32, 12, gui_dialog );
        helpline( pPopup, " BA80 Function Key Labels" ) ;
        helpline( pPopup, "" ) ;
        for ( i=0 ; i<9 ; i++ ) {
            sprintf(buffer,"  F%-2d:  \"%s\"",i+1,fkeylabel[i]);
            helpline( pPopup, buffer );
        }
        helpline( pPopup, "" ) ;
    }
    else {
        pPopup = helpstart(32, 1, gui_dialog );
        helpline( pPopup, " No Function Key Labels" ) ;
    }

    helpend(pPopup);                    /* Write bottom of help panel */
#ifdef KUI
    if ( pPopup->gui )
        c = gui_videopopup_dialog(pPopup, 0);
    else
#endif /* KUI */
    {
        VscrnSetPopup( mode, pPopup ) ;

        /* wait until user presses a key */
        evt = congev(mode,0);
        while ( evt.type != key
#ifndef NOKVERBS
                && evt.type != kverb
#endif /* NOKVERBS */
                ) {
            if ( evt.type != error )
                bleep( BP_WARN ) ;
            evt = congev(mode,0);
        }
#ifndef NOKVERBS
        if ( evt.type == kverb )
            c = F_KVERB | evt.kverb.id ;
        else
#endif /* NOKVERBS */
        {
            c = 0 ;
        }   
        VscrnResetPopup(mode) ;            /* This frees the Popup structure */
    }

    restore_status_line();
    return (c);
}

int
popuphelp(int mode, enum helpscreen x) {
#ifndef NOKVERBS
    videopopup * pPopup = NULL ;
    int c, n, z;
    char line[81];
    char kn[40];
    char *s;
    static int whichscreen = 0 ;
    con_event evt ;
#ifdef OS2MOUSE
    int button, event ;
#endif /* OS2MOUSE */

    char *hlpmsg[] = {
        "",
        " Command:   (SPACE to cancel)",
        "",
#ifdef NT
        "    C    to return to the K-95 prompt",
#else /* NT */
        "    C    to return to the Kermit prompt",
#endif /* NT */
        "    U    to hangUp and return to the prompt",
#ifdef NT
        "    Q    to hangup and Quit K-95",
#else /* NT */
        "    Q    to hangup and Quit Kermit",
#endif /* NT */
        "    !    to enter the system command processor",
        "    F    to print or File the current screen",
        "",
        "    0    (zero) to send a null",
        "   ^%c    to send the escape character",
        "    B    to send a BREAK signal",
        "    L    to send a Long BREAK",
        "",
#ifdef TNCODE
        "    I    to Interrupt process (TELNET)",
        "    A    to send \"Are you there?\" (TELNET)",
        "",
#endif /* TNCODE */
        "    \\    backslash escape (end with ENTER):",
        "         \\nnn   decimal code, like \\199",
        "         \\Onnn  octal code, like \\o307",
        "         \\Xhh   hexadecimal code, like \\xC7",
        "" };
#define HELPSIZE (sizeof(hlpmsg)/sizeof(char *))

    char *abouthlpmsg[] = {
        " By Jeffrey Eric Altman, Secure Endpoints Inc.",
        "    Frank da Cruz, Columbia University",
        " Copyright (C) 1985, 2013",
        " Trustees of Columbia University in the City of New York.",
        "",
        "  Type INTRO   for a brief introduction to the Kermit Command screen.",
        "  Type VERSION for version and copyright information.",
        "  Type HELP    followed by a command name for help about a specific command.",
        "  Type NEWS    for news about new features.",
        "  Type BUG     to learn how to get technical support.",
        "  Press ?      (question mark) at the prompt, or anywhere within a command,",
        "               for a menu (context-sensitive help, menu on demand).",
        ""
    };
#define ABOUTHLPSIZE (sizeof(abouthlpmsg)/sizeof(char *))

    char *l2ckmsg[] = {
        " COMPOSE SEQUENCES FOR MULTINATIONAL CHARACTERS",
        "",
        " Press two keys to compose the desired character:",
        "",
        " First  Second  Result",
        "   '     Letter   Letter with acute accent",
        "   #     Letter   Letter with double acute accent",
        "   %%     Letter   Letter with breve accent",
        "   \"     Letter   Letter with diaeresis/trema/umlaut",
        "   &     Letter   Letter with caron/hacek",
        "   ^     Letter   Letter with circumflex",
        "   *     Letter   Letter with ring above",
        "   ,     Letter   Letter with cedilla",
        "   =     Letter   Letter with ogonek",
        "   .     Letter   Letter with dot above",
        "   L       /      L with stroke",
        "   s       s      German sharp s / Ess-Zet"

#ifdef KUI
,       "   n       b      No-break space"
#endif /* KUI */
    };
#define L2COMPOSESIZE (sizeof(l2ckmsg)/sizeof(char *))

    char *cmposmsg[] = {
        " COMPOSE SEQUENCES FOR MULTINATIONAL CHARACTERS",
        "",
        " Press two keys to compose the desired character:",
        "",
        " First  Second  Result",
        "   '     Vowel   Vowel with acute accent",
        "   `     Vowel   Vowel with grave accent",
        "   \"     Vowel   Vowel with diaeresis/trema/umlaut",
        "   ^     Vowel   Vowel with circumflex",
        "   *       A     A ring",
        "   ,       C     C cedilla",
        "   ~     N,A,O   Letter with tilde",
        "   A       E     AE digraph",
        "   O       /     O with stroke",
        "   s       s     German sharp s",
        "   n       b     No-break space"
        "",
        " And others; see the documentation.",
    };
#define COMPOSESIZE (sizeof(cmposmsg)/sizeof(char *))

    char *ucs2msg[] = {
        "Enter exactly 4 hexadecimal digits",
        "corresponding to a Unicode character.",
        "Examples:",
        "",
        " 0041  Latin Capital Letter A",
        " 00de  Latin Capital Letter Thorn",
        " 03c8  Greek Small Letter Psi",
        " 0428  Cyrillic Capital Letter Sha",
        " 054c  Armenian Capital Letter Ra",
        " 05d0  Hebrew Letter Alef",
        " 20ac  Euro Symbol",
        " 30a2  Katakana Letter A",
        "",
        "Results depend on your font.  Works best",
        "with SET TERMINAL REMOTE-CHARACTER-SET UTF8.",
    };
#define UCS2SIZE (sizeof(ucs2msg)/sizeof(char *))

     char *srchhlpmsg[] = {
         " SCROLLBACK SEARCH",
         " ",
         " Use Delete and Ctrl-U for editing,",
         " Esc to cancel, Enter to start the search.",
         ""
     };
#define SEARCHHLPSIZE (sizeof(srchhlpmsg)/sizeof(char *))

    char * bkmrkhlpmsg[] = {
        " BOOKMARKS",
        " ",
        " Type a bookmark number, 0 through 9.",
        " Press the Space bar to cancel.",
        ""
    };
#define BKMRKHLPSIZE (sizeof(bkmrkhlpmsg)/sizeof(char *))

    vikinit() ;                         /* Update VIK Table */
    save_status_line();                 /* Save current status line */

    switch (x) {
    case hlp_search:
        pPopup = helpstart(52, SEARCHHLPSIZE, gui_dialog);
        for ( c=0; c<SEARCHHLPSIZE; c++ ) {
            s = srchhlpmsg[c] ;
            if ( strchr( s, '%') ) {
                sprintf(line, s, ctl(escape));
                helpline( pPopup, line ) ;
            } else
              helpline( pPopup, s ) ;
        }
        break;

    case hlp_bookmark:
        pPopup = helpstart(52, BKMRKHLPSIZE, gui_dialog);
        for ( c = 0; c < BKMRKHLPSIZE; c++ ) {
            s = bkmrkhlpmsg[c] ;
            if ( strchr( s, '%') ) {
                sprintf(line, s, ctl(escape));
                helpline( pPopup, line ) ;
            } else
              helpline( pPopup, s ) ;
        }
        break;

    case hlp_compose:                 /* Compose help popup */
        z = (tcsl == FC_CP852) ? L2COMPOSESIZE : COMPOSESIZE;
        pPopup = helpstart(52, z, gui_dialog);
        for (c = 0; c < z; c++) {
            s = (tcsl == TX_CP852) ? l2ckmsg[c] : cmposmsg[c];
            if (strchr(s, '%')) {
                sprintf(line, s, ctl(escape));
                helpline( pPopup, line);
            } else
              helpline( pPopup, s);
        }
        break;

    case hlp_ucs2:                   /* UCS2 help popup */
        pPopup = helpstart(52, UCS2SIZE, gui_dialog);
        for (c = 0; c < UCS2SIZE; c++) {
            s = ucs2msg[c];
            if (strchr(s, '%')) {
                sprintf(line, s, ctl(escape));
                helpline( pPopup, line);
            } else
              helpline( pPopup, s);
        }
        break;

    case hlp_escape:  /* escape help popup */
        pPopup = helpstart(47, HELPSIZE, gui_dialog);
        for (c = 0; c < HELPSIZE; c++) {
            s = hlpmsg[c];
            if (strchr(s, '%')) {
                sprintf(line, s, ctl(escape));
                helpline( pPopup, line);
            } else
              helpline( pPopup, s);
        }
        break;

    case hlp_rollback:
        strcpy(usertext, " Press (almost) any key to restore the screen");
        exittext[0] = helptext[0] = hostname[0] = NUL;

        s = " SCREEN SCROLL KEYS";      /* Box title */
        n = 22;                         /* How many lines for help box */
        if (vik.upscn   < 256) n--;     /* Deduct for verbs not assigned */
        if (vik.upone   < 256) n--;
        if (vik.dnscn   < 256) n--;
        if (vik.dnone   < 256) n--;
        if (vik.homscn  < 256) n--;
        if (vik.endscn  < 256) n--;
        if ( vik.lfone < 256 ) n--;
        if ( vik.lfpg < 256 ) n--;
        if ( vik.lfall < 256 ) n--;
        if ( vik.rtone < 256 ) n--;
        if ( vik.rtpg < 256 ) n--;
        if ( vik.rtall < 256 ) n--;
        if (vik.dump    < 256) n--;
        if (vik.exit    < 256) n--;
        if (vik.markstart < 256) n--;
        if (vik.help < 256) n--;
        pPopup = helpstart(66, n, gui_dialog);      /* Make popup window 66 x n */
        helpline( pPopup, s);           /* 1 */
        helpline( pPopup, "");          /* 2 */

        if (vik.upscn > 255 && keyname(vik.upscn)) {
            ckstrncpy(kn,keyname(vik.upscn),40);
            sprintf(line,"  %-34.33s%s",kn,"Scroll backward one screen");
            helpline( pPopup, line);    /* 3 */
        }
        if (vik.upone > 255 && keyname(vik.upone)) {
            ckstrncpy(kn,keyname(vik.upone),40);
            sprintf(line,"  %-34.33s%s",kn,"Scroll backward one line");
            helpline( pPopup, line);    /* 4 */
        }
        if (vik.dnscn > 255 && keyname(vik.dnscn)) {
            ckstrncpy(kn,keyname(vik.dnscn),40);
            sprintf(line,"  %-34.33s%s",kn,"Scroll forward one screen");
            helpline( pPopup, line);    /* 5 */
        }
        if (vik.dnone > 255 && keyname(vik.dnone)) {
            ckstrncpy(kn,keyname(vik.dnone),40);
            sprintf(line,"  %-34.33s%s",kn,"Scroll forward one line");
            helpline( pPopup, line);    /* 6 */
        }
        if (vik.homscn > 255 && keyname(vik.homscn)) {
            ckstrncpy(kn,keyname(vik.homscn),40);
            sprintf(line,"  %-34.33s%s",kn,"Scroll back to beginning");
            helpline( pPopup, line);            /* 7 */
        }
        if (vik.endscn > 255 && keyname(vik.endscn)) {
            ckstrncpy(kn,keyname(vik.endscn),40);
            sprintf(line,"  %-34.33s%s",kn,"Scroll forward to end");
            helpline( pPopup, line);    /* 8 */
        }
        if (vik.lfone > 255 && keyname(vik.lfone)) {
            ckstrncpy(kn,keyname(vik.lfone),40);
            sprintf(line,"  %-34.33s%s",kn,"Scroll left one column");
            helpline( pPopup, line);    /* 9 */
        }
        if (vik.lfpg > 255 && keyname(vik.lfpg)) {
            ckstrncpy(kn,keyname(vik.lfpg),40);
            sprintf(line,"  %-34.33s%s",kn,"Scroll left eight columns");
            helpline( pPopup, line);    /* 9 */
        }
        if (vik.lfall > 255 && keyname(vik.lfall)) {
            ckstrncpy(kn,keyname(vik.lfall),40);
            sprintf(line,"  %-34.33s%s",kn,"Scroll left to first column");
            helpline( pPopup, line);    /* 9 */
        }
        if (vik.rtone > 255 && keyname(vik.rtone)) {
            ckstrncpy(kn,keyname(vik.rtone),40);
            sprintf(line,"  %-34.33s%s",kn,"Scroll right one column");
            helpline( pPopup, line);    /* 9 */
        }
        if (vik.rtpg > 255 && keyname(vik.rtpg)) {
            ckstrncpy(kn,keyname(vik.rtpg),40);
            sprintf(line,"  %-34.33s%s",kn,"Scroll right eight columns");
            helpline( pPopup, line);    /* 9 */
        }
        if (vik.rtall > 255 && keyname(vik.rtall)) {
            ckstrncpy(kn,keyname(vik.rtall),40);
            sprintf(line,"  %-34.33s%s",kn,"Scroll right to last column");
            helpline( pPopup, line);    /* 9 */
        }
        if (vik.markstart > 255 && keyname(vik.markstart)) {
            ckstrncpy(kn,keyname(vik.markstart),40);
            sprintf(line,"  %-34.33s%s",kn,"Begin selection (mark mode)");
            helpline( pPopup, line);    /* 11 */
        }
        if (vik.dump > 255 && keyname(vik.dump)) {
            ckstrncpy(kn,keyname(vik.dump),40);
            sprintf(line,"  %-34.33s%s",kn,"Print the visible screen");
            helpline( pPopup, line);    /* 9 */
        }
        if (vik.exit > 255 && keyname(vik.exit)) {
            ckstrncpy(kn,keyname(vik.exit),40);
            sprintf(line,"  %-34.33s%s",kn,"Return to Command screen");
            helpline( pPopup, line);    /* 10 */
        }
        if (vik.help > 255 && keyname(vik.help)) {
            ckstrncpy(kn,keyname(vik.help),40);
#ifdef KUI
            if ( pPopup->gui )
                sprintf(line,"  %-34.33s%s",kn,"Context Help");
            else
#endif /* KUI */
                sprintf(line,"  %-34.33s%s",kn,"More help...");
            helpline( pPopup, line);    /* 18 */
        }
        helpline( pPopup, "");          /* 12 */
        helpline( pPopup, tt_roll ?
                 " ROLL-MODE is INSERT: New data appears at end of buffer." :
                 " ROLL-MODE is OVERWRITE: New data appears on current screen."
                 );                     /* 13 */
        sprintf(line, " Scrollback buffer size: %d lines.",
                VscrnGetBufferSize(mode)
                );
        helpline(pPopup, line); /* 14 */
        helpline(pPopup," Use SET TERMINAL SCROLLBACK to change it."); /* 13 */
        break;

    case hlp_normal:
        if ( mode == VTERM ) {
            strcpy(usertext, " Press (almost) any key to restore the screen");
            exittext[0] = helptext[0] = hostname[0] = NUL;

            n = 24;                         /* How many lines */
            s = " IMPORTANT KEYS";
            if (vik.flipscn < 256) n--;
            if (vik.debug   < 256) n--;
            if (vik.reset   < 256) n--;
            if (vik.prtctrl < 256) n--;
            if (vik.prtauto < 256) n--;
            if (vik.printff < 256) n--;
            if (vik.compose < 256) n--;
            if (vik.ucs2 < 256) n--;
            if (vik.fnkeys  < 256) n--;
            if (vik.help < 256) n--;
            if ( vik.exit < 256 && !tt_escape ) n-- ;
            if ( vik.hangup < 256 && !tt_escape ) n-- ;
            if ( vik.os2 < 256 && !tt_escape ) n--;
            if ( vik.xbreak < 256 && !tt_escape ) n--;
            if ( vik.lbreak < 256 && !tt_escape ) n--;
            if ( vik.dump < 256 && !tt_escape ) n--;
            if (vik.quit    < 256) n--;
            if (vik.status    < 256) n--;
            if (vik.paste < 256) n--;
            if ( !tt_escape ) n -= 3 ;

            if (network) n--;               /* long break not displayed */

            pPopup = helpstart(60, n, gui_dialog);      /* Make popup window 50 x n */
            helpline( pPopup, s);           /* 1 */
            helpline( pPopup, "");          /* 2 */

            if (vik.exit > 255 && keyname(vik.exit))
                ckstrncpy(kn,keyname(vik.exit),40);
            else if ( tt_escape )
                sprintf(kn, "Ctrl-%c?", ctl(escape));
            else
                kn[0] = NUL ;
            if ( kn[0] ) {
                sprintf(line,"  %-25.24s%s",kn,"Return to Command screen");
                helpline( pPopup, line);        /* 3 */
            }

            if (vik.hangup > 255 && keyname(vik.hangup))
                ckstrncpy(kn,keyname(vik.hangup),40);
            else if     ( tt_escape )
            sprintf(kn, "Ctrl-%cH", ctl(escape));
            else
                kn[0] = NUL ;
            if ( kn[0] ) {
                sprintf(line,"  %-25.24s%s",kn,"Hang up the connection");
                helpline( pPopup, line);        /* 4 */
            }

            if (vik.quit > 255 && keyname(vik.quit)) {
                ckstrncpy(kn,keyname(vik.quit),40);
                sprintf(line,"  %-25.24s%s",kn,"Hangup and Quit");
                helpline( pPopup, line);    /* 10 */
            }
            if (vik.os2 > 255 && keyname(vik.os2))
                ckstrncpy(kn,keyname(vik.os2),40);
            else if ( tt_escape )
                sprintf(kn, "Ctrl-%c!", ctl(escape));
            else
                kn[0] = NUL ;
            if ( kn[0] ) {
                sprintf(line,"  %-25.24s%s",kn,"Enter system command processor");
                helpline( pPopup, line);        /* 5 */
            }

            if (vik.xbreak > 255 && keyname(vik.xbreak))
                ckstrncpy(kn,keyname(vik.xbreak),40);
            else if ( tt_escape )
                sprintf(kn, "Ctrl-%cB", ctl(escape));
            else
                kn[0] = NUL ;
            if ( kn[0] ) {
                sprintf(line,"  %-25.24s%s",kn,
                     (network && IS_TELNET()) ?
                     "Send TELNET BREAK" :
                     "Send a BREAK signal"
                     );
                helpline( pPopup, line);        /* 6 */
            }

            if (!network) {
                if ((vik.lbreak > 255) && (keyname(vik.lbreak)))
                    ckstrncpy(kn,keyname(vik.lbreak),40);
                else if ( tt_escape )
                    sprintf(kn, "Ctrl-%cL", ctl(escape));
                else
                    kn[0] = NUL ;
                if ( kn[0] ) {
                sprintf(line,"  %-25.24s%s",kn,"Send a Long BREAK signal");
                helpline( pPopup, line);    /* 7 */
                }
            }
            if (vik.dump > 255 && keyname(vik.dump))
                ckstrncpy(kn,keyname(vik.dump),40);
            else if ( tt_escape )
                sprintf(kn, "Ctrl-%cF", ctl(escape));
            else
                kn[0] = NUL ;
            if ( kn[0] ) {
            sprintf(line,"  %-25.24s%s",kn,"Print the current screen");
            helpline( pPopup, line);        /* 8 */
            }

            if (vik.prtctrl > 255 && keyname(vik.prtctrl)) {
                ckstrncpy(kn,keyname(vik.prtctrl),40);
                sprintf(line,"  %-25.24s%s",
                         kn,
                         xprint ?
                         "Stop printing screen lines" :
                         "Copy screen lines to printer"
                         );
                helpline( pPopup, line);    /* 9, maybe */
            }
            if (vik.prtauto > 255 && keyname(vik.prtauto)) {
                ckstrncpy(kn,keyname(vik.prtauto),40);
                sprintf(line,"  %-25.24s%s%s",kn,"Turn copy-to-printer ",
                         aprint ? "OFF" : "ON");
                helpline( pPopup, line);    /* 9, maybe */
            }
            if (vik.printff > 255 && keyname(vik.printff)) {
                ckstrncpy(kn,keyname(vik.printff),40);
                sprintf(line,"  %-25.24s%s",kn,"Print formfeed / page eject");
                helpline( pPopup, line);    /* 10, maybe */
            }
            if (vik.flipscn > 255 && keyname(vik.flipscn)) {
                ckstrncpy(kn,keyname(vik.flipscn),40);
                sprintf(line,"  %-25.24s%s%s",kn,
                         (decscnm ? "Normal" : "Reverse"),
                         " screen video");
                helpline( pPopup, line);    /* 13, maybe */
            }
            if (vik.debug > 255 && keyname(vik.debug)) {
                ckstrncpy(kn,keyname(vik.debug),40);
                sprintf(line,"  %-25.24s%s%s",kn,"Turn terminal debugging ",
                         debses ? "OFF" : "ON");
                helpline( pPopup, line);    /* 14, maybe */
            }
            if (vik.reset > 255 && keyname(vik.reset)) {
                ckstrncpy(kn,keyname(vik.reset),40);
                sprintf(line,"  %-25.24s%s",kn,"Reset the terminal emulator");
                helpline( pPopup, line);    /* 15, maybe */
            }
            if (vik.compose > 255 && keyname(vik.compose)) {
                ckstrncpy(kn,keyname(vik.compose),40);
                sprintf(line,"  %-25.24s%s",kn,"Compose a special character");
                helpline( pPopup, line);    /* 16, maybe */
            }
            if (vik.ucs2 > 255 && keyname(vik.ucs2)) {
                ckstrncpy(kn,keyname(vik.ucs2),40);
                sprintf(line,"  %-25.24s%s",kn,"Compose a Unicode character");
                helpline( pPopup, line);    /* 17, maybe */
            }
            if (vik.status > 255 && keyname(vik.status)) {
                ckstrncpy(kn,keyname(vik.status),40);
                sprintf(line,"  %-25.24s%s",kn,"Toggle Status Line");
                helpline( pPopup, line);    /* 18 */
            }
            if (vik.fnkeys > 255 && keyname(vik.fnkeys)) {
                ckstrncpy(kn,keyname(vik.fnkeys),40);
                sprintf(line,"  %-25.24s%s",kn,"Display Fn key labels");
                helpline( pPopup, line);    /* 19 */
            }
            if (vik.paste > 255 && keyname(vik.paste)) {
                ckstrncpy(kn,keyname(vik.paste),40);
                sprintf(line,"  %-25.24s%s",kn,"Paste clipboard text");
                helpline( pPopup, line);    /* 20 */
            }
            if (vik.help > 255 && keyname(vik.help)) {
                ckstrncpy(kn,keyname(vik.help),40);
#ifdef KUI
                if ( pPopup->gui )
                    sprintf(line,"  %-25.24s%s",kn,"Help");
                else
#endif /* KUI */
                    sprintf(line,"  %-25.24s%s",kn,"More help...");
                helpline( pPopup, line);    /* 21 */
            }
            if ( tt_escape ) {
            helpline( pPopup, "");          /* 22 */
            sprintf(line," or the CONNECT-mode escape character Ctrl-%c",
                     ctl(escape));
            helpline( pPopup, line);        /* 23 */
            helpline( pPopup, " followed by ? for additional commands."); /* 22 */
            }
        }
        else {
            pPopup = helpstart(76, ABOUTHLPSIZE+2, gui_dialog);
            sprintf(line," %s, for %s",versio,ckxsys);
            helpline( pPopup, line );
            for ( c = 0; c < ABOUTHLPSIZE; c++ ) {
                s = abouthlpmsg[c] ;
                if ( strchr( s, '%') ) {
                    sprintf(line, s, ctl(escape));
                    helpline( pPopup, line ) ;
                } else
                    helpline( pPopup, s ) ;
            }
            if (vik.help > 255 && keyname(vik.help)) {
                ckstrncpy(kn,keyname(vik.help),40);
#ifdef KUI
                if ( pPopup->gui )
                    sprintf(line,"  Press <Cancel> to exit, or OK for more help");
                else
#endif /* KUI */
                    sprintf(line,"  Press <SPACE> to exit, or %s for more help",kn);
            }
            else {
#ifdef KUI
                if ( pPopup->gui )
                    sprintf(line,"  Press <Cancel> to exit");
                else
#endif /* KUI */
                    sprintf(line,"  Press <SPACE> to exit");
            }
            helpline( pPopup, line);

        }
        break;

    case hlp_markmode:
        strcpy(usertext, " Press (almost) any key to restore the screen");
        exittext[0] = helptext[0] = hostname[0] = NUL;

        s = " MARK MODE KEYS"; /* Box title */
        if ( markmodeflag[mode] == inmarkmode ) /* how many lines ? */
          n = 13 ;
        else
          n = 16 ;
        if (vik.upscn   < 256) n--;     /* Deduct for verbs not assigned */
        if (vik.upone   < 256) n--;
        if (vik.dnscn   < 256) n--;
        if (vik.dnone   < 256) n--;
        if (vik.homscn  < 256) n--;
        if (vik.endscn  < 256) n--;
        if (vik.markstart < 256) n-- ;
        if (vik.markcancel < 256) n-- ;
        if ( markmodeflag[mode] == marking ) {
            if (vik.dump < 256) n--;
            if (vik.copyclip < 256) n-- ;
            if (vik.copyhost < 256) n-- ;
        }
        pPopup = helpstart(65, n, gui_dialog);      /* Make popup window 70 x n */
        helpline( pPopup, s);           /* 1 */
        helpline( pPopup, "");          /* 2 */

        if (vik.upscn > 255 && keyname(vik.upscn)) {
            ckstrncpy(kn,keyname(vik.upscn),40);
            sprintf(line,"  %-26.25s%s",kn,
                    "Extend selection backward one screen");
            helpline( pPopup, line);    /* 3 */
        }
        if (vik.upone > 255 && keyname(vik.upone)) {
            ckstrncpy(kn,keyname(vik.upone),40);
            sprintf(line,"  %-26.25s%s",kn,
                    "Extend selection backward one line");
            helpline( pPopup, line);    /* 4 */
        }
        if (vik.dnscn > 255 && keyname(vik.dnscn)) {
            ckstrncpy(kn,keyname(vik.dnscn),40);
            sprintf(line,"  %-26.25s%s",kn,
                    "Extend selection forward one screen");
            helpline( pPopup, line);    /* 5 */
        }
        if (vik.dnone > 255 && keyname(vik.dnone)) {
            ckstrncpy(kn,keyname(vik.dnone),40);
            sprintf(line,"  %-26.25s%s",kn,
                    "Extend selection forward one line");
            helpline( pPopup, line);    /* 6 */
        }
        if (vik.homscn > 255 && keyname(vik.homscn)) {
            ckstrncpy(kn,keyname(vik.homscn),40);
            sprintf(line,"  %-26.25s%s",kn,
                    "Extend selection to top of buffer");
            helpline( pPopup, line);    /* 7 */
        }
        if (vik.endscn > 255 && keyname(vik.endscn)) {
            ckstrncpy(kn,keyname(vik.endscn),40);
            sprintf(line,"  %-26.25s%s",kn,
                    "Extend selection to bottom of buffer");
            helpline( pPopup, line);    /* 8 */
        }
        if (vik.exit > 255 && keyname(vik.exit)) {
            ckstrncpy(kn,keyname(vik.exit),40);
            sprintf(line,"  %-26.25s%s",kn,"Return to Command screen");
            helpline( pPopup, line);    /* 9 */
        }
        if (vik.markstart > 255 && keyname(vik.markstart)) {
            ckstrncpy(kn,keyname(vik.markstart),40);
            sprintf(line,"  %-26.25s%s",kn, markmodeflag[mode] == inmarkmode ?
                    "Begin selection" :
                    "Anchor selection at current position"
                    );
            helpline( pPopup, line);    /* 10 */
        }
        if (vik.markcancel > 255 && keyname(vik.markcancel)) {
            ckstrncpy(kn,keyname(vik.markcancel),40);
            sprintf(line,"  %-26.25s%s",kn, "Cancel selection" );
            helpline( pPopup, line);    /* 11 */
        }
        if ( markmodeflag[mode] == marking ) {
            if (vik.copyclip > 255 && keyname(vik.copyclip)) {
                ckstrncpy(kn,keyname(vik.copyclip),40);
                sprintf(line,"  %-26.25s%s",kn,
                        "Copy selection to clipboard" );
                helpline( pPopup, line); /* 12 */
            }
            if (vik.copyhost > 255 && keyname(vik.copyhost)) {
                ckstrncpy(kn,keyname(vik.copyhost),40);
                sprintf(line,"  %-26.25s%s",kn,
                        "Transmit selection" );
                helpline( pPopup, line); /* 13 */
            }
            if (vik.dump > 255 && keyname(vik.dump)) {
                ckstrncpy(kn,keyname(vik.dump),40);
                sprintf(line,"  %-26.25s%s",kn,
                        "Copy selection to printer (or file)");
                helpline( pPopup, line); /* 14 */
            }
        }
        helpline( pPopup, "");          /* 12/15 */
        helpline( pPopup,               /* 13/16 */
                  " You can also use the arrow keys to modify the selection.");
        break;

#ifdef OS2MOUSE
    case hlp_mouse:
        strcpy(usertext, " Press (almost) any key to restore the screen");
        exittext[0] = helptext[0] = hostname[0] = NUL;

        n = 21;                         /* How many lines */
        s = " MOUSE ACTIONS";
        if (!vik.mou_curpos)   n--;     /* Deduct for verbs not assigned */
        if (!vik.mou_copyhost) n--;
        if (!vik.mou_copyclip) n--;
        if (!vik.mou_paste)    n--;
        if (!vik.mou_dump)     n--;
        if (!vik.mou_mark)     n--;
        if (!vik.mou_url)      n--;
        if (vik.help < 256)    n--;
        if (!vik.backsrch)     n--;
        if (!vik.backnext)     n--;
        if (!vik.fwdsrch)      n--;
        if (!vik.fwdnext)      n--;
        if (!vik.setbook)      n--;
        if (!vik.gobook)       n--;
        if (!vik.xgoto)        n--;

        pPopup = helpstart(70, n, gui_dialog);      /* Make popup window 70 x n */
        helpline( pPopup, s);           /* 1 */
        helpline( pPopup, "");          /* 2 */

        if (vik.mou_curpos) {
            ckstrncpy(kn,mousename(vik.mou_curpos>>5,vik.mou_curpos&0x1F),40);
            sprintf(line,"  %-31.30s%s",kn,"Move cursor to new position");
            helpline( pPopup, line);    /* 3 */
        }
        if (vik.mou_copyhost) {
           ckstrncpy(kn,mousename(vik.mou_copyhost>>5,vik.mou_copyhost&0x1F),40);
            sprintf(line,"  %-31.30s%s",kn,"Select text and transmit it");
            helpline( pPopup, line);    /* 4 */
        }
        if (vik.mou_copyclip) {
           ckstrncpy(kn,mousename(vik.mou_copyclip>>5,vik.mou_copyclip&0x1F),40);
            sprintf(line,"  %-31.30s%s",kn,"Select and copy to Clipboard");
            helpline( pPopup, line);     /* 5 */
        }
        if (vik.mou_dump) {
            ckstrncpy(kn,mousename(vik.mou_dump>>5,vik.mou_dump&0x1F),40);
            sprintf(line,
                    "  %-31.30s%s",kn,"Select and copy to printer (or file)");
            helpline( pPopup, line);    /* 6 */
        }
        if (vik.mou_mark) {
            ckstrncpy(kn,mousename(vik.mou_mark>>5,vik.mou_mark&0x1F),40);
            sprintf(line,"  %-31.30s%s",kn,"Select but don't copy");
            helpline( pPopup, line);    /* 7 */
        }
        if (vik.mou_paste) {
            ckstrncpy(kn,mousename(vik.mou_paste>>5,vik.mou_paste&0x1F),40);
            sprintf(line,
                    "  %-31.30s%s",kn,"Transmit from clipboard");
            helpline( pPopup, line);    /* 8 */
        }
        if (vik.mou_url) {
            ckstrncpy(kn,mousename(vik.mou_url>>5,vik.mou_url&0x1F),40);
            sprintf(line,"  %-31.30s%s",kn,"Start browser with URL");
            helpline( pPopup, line);    /* 7 */
        }
        helpline( pPopup, "");          /* 9 */
        helpline( pPopup, " SEARCHING AND BOOKMARKS"); /* 10 */
        helpline( pPopup, "");          /* 11 */
        if (vik.backsrch > 255 && keyname(vik.backsrch)) {
            ckstrncpy(kn,keyname(vik.backsrch),40);
            sprintf(line,"  %-31.30s%s",kn,"Search backward");
            helpline( pPopup, line);    /* 12 */
        }
        if (vik.backnext > 255 && keyname(vik.backnext)) {
            ckstrncpy(kn,keyname(vik.backnext),40);
            sprintf(line,"  %-31.30s%s",kn,"Next occurrence backward");
            helpline( pPopup, line);    /* 13 */
        }
        if (vik.fwdsrch > 255 && keyname(vik.fwdsrch)) {
            ckstrncpy(kn,keyname(vik.fwdsrch),40);
            sprintf(line,"  %-31.30s%s",kn,"Search forward");
            helpline( pPopup, line);    /* 14 */
        }
        if (vik.fwdnext > 255 && keyname(vik.fwdnext)) {
            ckstrncpy(kn,keyname(vik.fwdnext),40);
            sprintf(line,"  %-31.30s%s",kn,"Next occurrence forward");
            helpline( pPopup, line);    /* 15 */
        }
        if (vik.setbook > 255 && keyname(vik.setbook)) {
            ckstrncpy(kn,keyname(vik.setbook),40);
            sprintf(line,"  %-31.30s%s",kn,"Set bookmark");
            helpline( pPopup, line);    /* 16 */
        }
        if (vik.gobook > 255 && keyname(vik.gobook)) {
            ckstrncpy(kn,keyname(vik.gobook),40);
            sprintf(line,"  %-31.30s%s",kn,"Go to bookmark");
            helpline( pPopup, line);    /* 17 */
        }
        if (vik.xgoto > 255 && keyname(vik.xgoto)) {
            ckstrncpy(kn,keyname(vik.xgoto),40);
            sprintf(line,"  %-31.30s%s",kn,"Go to line");
            helpline( pPopup, line);    /* 18 */
        }
        helpline( pPopup, "");          /* 19 */
        if (vik.help > 255 && keyname(vik.help)) {
            ckstrncpy(kn,keyname(vik.help),40);
#ifdef KUI
            if ( pPopup->gui )
                sprintf(line," %-32.31s%s",kn,"Help");
            else
#endif /* KUI */
                sprintf(line," %-32.31s%s",kn,"Back to first help screen...");
            helpline( pPopup, line);    /* 20 */
        }
        break;
#endif /* OS2MOUSE */
    } /* switch */

    helpend(pPopup);                    /* Write bottom of help panel */
#ifdef KUI
    if ( pPopup->gui )
        c = gui_videopopup_dialog(pPopup, 0);
    else
#endif /* KUI */
    {
    VscrnSetPopup( mode, pPopup ) ;
    /* wait until user presses a key */
    evt = congev(mode,-1);
    while ( evt.type != key
#ifndef NOKVERBS
            && evt.type != kverb
#endif /* NOKVERBS */
            ) {
        if ( evt.type != error )
            bleep( BP_WARN ) ;
        evt = congev(mode,-1);
    }
#ifndef NOKVERBS
    if ( evt.type == kverb )
        c = F_KVERB | evt.kverb.id ;
    else
#endif /* NOKVERBS */
    {
        c = 0 ;
    }
    VscrnResetPopup(mode) ;            /* This frees the Popup structure */
    }

    restore_status_line();
    return (c);
#else /* NOKVERBS */
    return(-1);
#endif /* NOKVERBS */
}
#endif /* NOLOCAL */


int                                     /* NOTE: not static, on purpose */
ttgwsiz() {                             /* Get Terminal Window Size */
#ifndef NOLOCAL
#ifndef KUI
   CK_VIDEOMODEINFO mi ;

   if (GetMode(&mi))
       return 1;
   debug(F101,"ttgwsiz mi.row","",mi.row);
   debug(F101,"ttgwsiz mi.col","",mi.col);
   debug(F101,"ttgwsiz tt_rows","",tt_rows[VTERM]);
   debug(F101,"ttgwsiz tt_cols","",tt_cols[VTERM]);

   if ( mi.row != tt_rows[VTERM] + (tt_status[VTERM]?1:0) || mi.col != tt_cols[VTERM] ) {
       tt_rows[VTERM] = (mi.row > MAXSCRNROW ? MAXSCRNROW : mi.row)- (tt_status[VTERM]?1:0) ;
       tt_cols[VTERM] = (mi.col > MAXSCRNCOL ? MAXSCRNCOL : mi.col);
       checkscreenmode();
   }
#endif /* KUI */
#endif /* NOLOCAL */
   return((tt_rows[VTERM] < 1 || tt_cols[VTERM] < 1) ? 0 : 1);
}

int                                     /* NOTE: not static, on purpose */
ttgcwsz() {                             /* Get Command Window Size */
#ifndef NOLOCAL
#ifndef KUI
    CK_VIDEOMODEINFO mi ;
    extern int tt_modechg;

#ifdef IKSD
    if ( inserver )
        return(0);
#endif /* IKSD */

    if (GetMode(&mi))
        return 1;

    debug(F101,"ttgcwsiz mi.row","",mi.row);
    debug(F101,"ttgcwsiz mi.col","",mi.col);
    debug(F101,"ttgcwsiz cmd_rows","",cmd_rows);
    debug(F101,"ttgcwsiz cmd_cols","",cmd_cols);
    if ( tt_modechg == TVC_W95 && ( mi.row > cmd_rows || mi.col > cmd_cols) ||
         tt_modechg == TVC_ENA && (mi.row != cmd_rows + (tt_status[VCMD]?1:0) ||
                                   mi.col != cmd_cols) ) {
        tt_rows[VCMD] = cmd_rows = (mi.row > MAXSCRNROW ? MAXSCRNROW : mi.row) - (tt_status[VCMD]?1:0);
        tt_cols[VCMD] = cmd_cols = (mi.col > MAXSCRNCOL ? MAXSCRNCOL : mi.col);
    }
#endif /* KUI */
#endif /* NOLOCAL */
   return((cmd_rows < 1 || cmd_cols < 1) ? 0 : 1);
}

#ifndef NOLOCAL
/* CHECKSCREENMODE  --  Make sure we are in a usable mode */
/* JEFFA ??? - I don't know what checkscreenmode() should do */

void
checkscreenmode() {
#ifndef KUI
    CK_VIDEOMODEINFO  m;
#endif /* KUI */

    debug(F100,"checkscreenmode","",0);
#ifndef KUI
    if (GetMode(&m))
        return;

    ttgcwsz();
#endif /* KUI */

    if ( VscrnGetBufferSize(VTERM) != tt_scrsize[VTERM] || VscrnGetWidth(VTERM) <= 0
        || VscrnGetHeight(VTERM) <= 0 || tt_cols[VTERM] <= 0 || tt_rows[VTERM] <= 0 ) {
        scrninitialized[VTERM] = 0;

        debug(F101,"tt_cols","",tt_cols[VTERM]);
        debug(F101,"tt_rows","",tt_rows[VTERM]);
#ifndef KUI
        debug(F101,"m.row","",m.row);
        debug(F101,"m.col","",m.col);

        if ( tt_cols[VTERM] <= 0 || tt_rows[VTERM] <= 0 ) {
            tt_cols[VTERM] = min(MAXTERMCOL, m.col);
            tt_rows[VTERM] = min(MAXTERMROW, m.row-1);
        }
#endif /* KUI */

        if ( VscrnGetWidth(VTERM) <= 0 ) {
            VscrnSetWidth( VTERM, tt_cols[VTERM] );
        }

        if ( VscrnGetHeight(VTERM) <= 0 ) {
            debug(F101,"tt_rows[VTERM]","",tt_rows[VTERM]);
            VscrnSetHeight( VTERM, tt_rows[VTERM]+(tt_status[VTERM]?1:0) );
        }

        marginbot = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
    }

#ifndef KUI
    vt100screen.mi = m;
    vt100screen.mi.row = tt_rows[VTERM] + (tt_status[VTERM]?1:0) ;
    vt100screen.mi.col = tt_cols[VTERM] ;
#ifdef NT
    vt100screen.mi.sbrow = vt100screen.mi.row;
    vt100screen.mi.sbcol = vt100screen.mi.col;
#endif /* NT */
#endif /* KUI */
    vt100screen.att = defaultattribute ;
}

void
setcursormode() {
#ifdef NT
    CK_CURSORINFO vci={88,0,8,1};
#else
    CK_CURSORINFO vci={-88,0,8,1};
    extern HVIO VioHandle ;
    VIOINTENSITY vi;
#endif /* NT */
    int cell, bottom, top;

    if (!GetCurType(&vci))
        crsr_command = vci;

#ifdef NT
    switch (tt_cursor) {
        case 0:
            vci.yStart = 12 ;
            break;
        case 1:
            vci.yStart = 50 ;
            break;
        case 2:
            vci.yStart = 88 ;
            break;
        }
    SetCurType(&vci);
#else /* NT */
    vci.cEnd   = -100;
    switch (tt_cursor) {
        case 0:
            vci.yStart = -90 ;
            break;
        case 1:
            vci.yStart = -50 ;
            break;
        case 2:
            vci.yStart = -10 ;
            break;
        }
    SetCurType(&vci);
/*
  Note: For some reason, choosing vi.fs = 0 makes *everything* blink.
  Receipt of "turn-on-blink" escape sequences turn off blinking.  Thus,
  everything is backwards.  But when vi.fs = 1, the sense is not reversed.
  A puzzle.
*/
    vi.cb = sizeof(vi);
    vi.type = 2;                        /* High-intensity vs blinking */
    vi.fs = 1;                          /* 0 = blinking, 1 = hi intensity */
    VioSetState((PVOID) &vi, VioHandle);
#endif /* NT */
}

void
restorecursormode() {
    debug(F100,"restorecursormode","",0);
    SetCurType(&crsr_command);
}

static void
doesc(int c) {
    int x;
    CHAR d, temp[8];

    while (1) {
        if (tt_escape && c == escape) { /* Send escape character */
            sendchar((char)c);
            return;
        } else if (isupper(c))  /* Or else look it up below. */
            c = tolower(c);

        switch (c) {

        case 'c':               /* Return to prompt */
        case '\03':
            SetConnectMode(0,CSX_ESCAPE);
            return;

        case 'u':               /* Hangup and return to command mode */
            hangnow = 1;
            SetConnectMode(0,CSX_USERDISC);
            strcpy(termessage, "Hangup.\n");
            return;

        case 'q':               /* Hangup and quit */
            SetConnectMode(0,CSX_USERDISC);
            hangnow = 1;
            quitnow = 1;
            strcpy(termessage, "Hangup and quit.\n");
            return;

#ifndef NOPUSH
        case '!': {             /* Push to OS/2 */
            os2push();
            return;
       }
#endif /* NOPUSH */

        case 'b':               /* Send a BREAK signal */
        case '\02':
#ifdef CKLEARN
            {
                con_event evt;
                evt.type = kverb;
                evt.kverb.id = K_BREAK;
                learnkeyb(evt, LEARN_KEYBOARD);
            }
#endif /* CKLEARN */
            ttsndb();
            return;

        case 'l':               /* Send a Long BREAK signal */
        case '\014':
#ifdef CKLEARN
            {
                con_event evt;
                evt.type = kverb;
                evt.kverb.id = K_LBREAK;
                learnkeyb(evt, LEARN_KEYBOARD);
            }
#endif /* CKLEARN */
            ttsndlb();
            return;

        case 'f': {              /* Print/dump screen */
            extern bool xprintff;
            x = xprintff;
            xprintff = 0;
            prtscreen(VTERM,1,VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0));
            xprintff = x;
            return;
        }

        case '0':               /* Send a NUL character */
            c = '\0';
            sendchar((char)c);
            return;

#ifdef NETCONN
#ifdef TCPSOCKET
        case 'i':                       /* Send TELNET Interrupt Process */
#ifdef CKLEARN
            {
                con_event evt;
                evt.type = kverb;
                evt.kverb.id = K_TN_IP;
                learnkeyb(evt, LEARN_KEYBOARD);
            }
#endif /* CKLEARN */
            do_tn_cmd((CHAR) TN_IP);
            return;

        case 'a':                       /* "Are You There?" */
#ifdef CKLEARN
            {
                con_event evt;
                evt.type = kverb;
                evt.kverb.id = K_TN_AYT;
                learnkeyb(evt, LEARN_KEYBOARD);
            }
#endif /* CKLEARN */
            do_tn_cmd((CHAR) TN_AYT);
            return;
#endif /* TCPSOCKET */
#endif /* NETCONN */

        case '\\':
            {
            char kbuf[32];
            char *kbp = kbuf, *text = usertext + 2;

                *kbp++ = c;

            strcpy(helptext, " \\");
            strcpy(exittext,"Exit: ENTER");
            helptext[0] = 0;
            VscrnIsDirty(VTERM);  /* status line needs to be updated */

                while (((c = (coninc(0) & cmdmsk)) != '\r') && (c != '\n')) {
                if ( c == '\b' ) {
                    if ( kbp > kbuf + 1 ) {
                                *--kbp = 0;
                        *--text = 0;
                        }
                    }
                else if ( kbp - kbuf < sizeof(kbuf) - 2 ) {
                            *kbp++ = c;
                    *text++ = c;
                    *text = 0;
                    }
                VscrnIsDirty(VTERM);  /* status line needs to be updated */
                }

                *kbp = 0; kbp = kbuf;
                c = xxesc(&kbp);        /* Interpret it */

                if (c >= 0) {
                    sendcharduplex((char)c,FALSE);
                    return;
                } else {                /* Invalid backslash code. */
                    bleep(BP_WARN);
                    return;
                }
            }
            return;

        case SP:                /* Space, ignore */
            return;

        default:                /* Other */
            bleep(BP_WARN);
            return;             /* Invalid esc arg, beep */
        }
    }
}

int
kbdlocked(void)
{
    extern int tt_rkeys[];

    if ( keylock )
        return 1;

    if (scrollflag[VTERM]) {
        switch ( tt_rkeys[VTERM] ) {
        case TTRK_IGN:
            return 1;
        case TTRK_RST:
            scrollflag[VTERM] = 0;
            scrollstatus[VTERM] = 0;
            ipadl25();
            os2settitle(NULL, 1);
            VscrnIsDirty(VTERM);
            return 0;
        case TTRK_SND:
            return 0;
        }
    }
    return 0;
}

/* C O N K B D H A N D L E R */

void
con2host(con_event evt)
{
    int c;

    switch ( evt.type ) {
    case key:
        c = evt.key.scancode ;
        if (tt_escape && c == escape) { /* Is it the escape character? */
            save_status_line();         /* Yes, save current status line.  */
            escapestatus[VTERM] = TRUE ;
            esc25(0);                   /* Make new one. */
            evt = congev(VTERM,0);
            while ( evt.type != key ) {
                bleep( BP_WARN ) ;
                evt = congev(VTERM,0);
            }

            c = evt.key.scancode ;
            if (c == '?' || c == 'h' || c == 'H') { /* They want help menu. */
                esc25(1);               /* Show it */
                c = popuphelp(VTERM,hlp_escape);    /* Then get real escape command */
            }
            if (c != escape) {
                doesc(c);               /* And process it */
                escapestatus[VTERM] = FALSE ;
                restore_status_line();      /* Then restore status line */
                if ( tt_idlelimit /* || tt_idlesnd_tmo */)
                    idlesnd_t = keypress_t = time(NULL);/* Remember the time */
                break;
            }
            restore_status_line();
        }

        /* Ordinary character */
#ifdef CKLEARN
        if (c == CR)
            learnkeyb(evt, LEARN_NEUTRAL);
        else
            learnkeyb(evt, LEARN_KEYBOARD);
#endif /* CKLEARN */
        vt100key(c);                /* go process it. */
        if ( tt_idlelimit /*|| tt_idlesnd_tmo*/ )
            idlesnd_t = keypress_t = time(NULL);/* Remember the time */
        break;

#ifndef NOKVERBS
    case kverb:
#ifdef CKLEARN
        switch ( evt.kverb.id | F_KVERB ) {
        case F_KVERB | K_KPENTER:
        case F_KVERB | K_WYENTER:
        case F_KVERB | K_WYRETURN:
        case F_KVERB | K_TVIENTER:
        case F_KVERB | K_TVIRETURN:
        case F_KVERB | K_HPENTER:
        case F_KVERB | K_HPRETURN:
            learnkeyb(evt, LEARN_NEUTRAL);
            break;
        default:
            learnkeyb(evt, LEARN_KEYBOARD);
            break;
        }
#endif /* CKLEARN */
        dokverb( VTERM, evt.kverb.id ) ;
        if ( tt_idlelimit /*|| tt_idlesnd_tmo*/ )
            idlesnd_t = keypress_t = time(NULL);/* Remember the time */
        break;
#endif /* NOKVERBS */
    case macro:
        if ( !kbdlocked() ) {
#ifdef CKLEARN
            int len = strlen(evt.macro.string);
            if (evt.macro.string[len-1] == CR ||
                evt.macro.string[len-2] == CR && evt.macro.string[len-1] == LF)
                learnkeyb(evt,LEARN_NEUTRAL);
            else
                learnkeyb(evt, LEARN_KEYBOARD);
#endif /* CKLEARN */
            sendkeydef( evt.macro.string, FALSE ) ;
            if ( tt_idlelimit /*|| tt_idlesnd_tmo*/ )
                idlesnd_t = keypress_t = time(NULL);/* Remember the time */
        }
        if ( evt.macro.string )
            free( evt.macro.string ) ;
        break;

    case literal:
        if ( !kbdlocked() ) {
#ifdef CKLEARN
            int len = strlen(evt.literal.string);
            if (evt.literal.string[len-1] == CR ||
                evt.literal.string[len-2] == CR && evt.literal.string[len-1] == LF)
                learnkeyb(evt,LEARN_NEUTRAL);
            else
                learnkeyb(evt, LEARN_KEYBOARD);
#endif /* CKLEARN */
            sendkeydef( evt.literal.string, TRUE ) ;
            if ( tt_idlelimit /*|| tt_idlesnd_tmo*/ )
                idlesnd_t = keypress_t = time(NULL);/* Remember the time */
        }
        if ( evt.literal.string )
            free( evt.literal.string ) ;
        break;

    case esc:
        if ( !kbdlocked() ) {
            char buf[2] ;
#ifdef CKLEARN
            learnkeyb(evt, LEARN_KEYBOARD);
#endif /* CKLEARN */
            buf[0] = evt.esc.key & ~F_ESC ;
            buf[1] = '\0' ;
            sendescseq(buf) ;
            if ( tt_idlelimit /*|| tt_idlesnd_tmo*/ )
                idlesnd_t = keypress_t = time(NULL);/* Remember the time */
        }
        break;

    case csi:
        if ( !kbdlocked() ) {
            char buf[3] ;
#ifdef CKLEARN
            learnkeyb(evt, LEARN_KEYBOARD);
#endif /* CKLEARN */
            buf[0] = '[' ;
            buf[1] = evt.csi.key & ~F_CSI ;
            buf[2] = '\0' ;
            sendescseq(buf);
            if ( tt_idlelimit /*|| tt_idlesnd_tmo*/ )
                idlesnd_t = keypress_t = time(NULL);/* Remember the time */
        }
        break;
#ifdef OS2MOUSE
      case mouse:
        debug( F100, "con2host - a mouse event arrived, ignored","",0);
        if ( tt_idlelimit /*|| tt_idlesnd_tmo*/ )
            idlesnd_t = keypress_t = time(NULL);/* Remember the time */
        break;
#endif /* OS2MOUSE */
      case error:
        if ( ttyfd != -1 && ttyfd != -2 &&
             /* tt_idlesnd_tmo */ tt_idlelimit &&
             ((tt_idleact == IDLE_OUT && tt_idlesnd_str && tt_idlesnd_str[0]) ||
               tt_idleact == IDLE_TNOP || tt_idleact == IDLE_TAYT) &&
             IsConnectMode() &&
             (time(NULL) - idlesnd_t) > /* tt_idlesnd_tmo */ tt_idlelimit ) {
            switch (tt_idleact) {
            case IDLE_OUT:
                debug( F111, "con2host - idle send string sent",
                       tt_idlesnd_str,/*tt_idlesnd_tmo*/ tt_idlelimit);
                sendkeydef( tt_idlesnd_str, FALSE ) ;
                break;
            case IDLE_TNOP:
                debug(F100, "con2host - idle Telnet NOP","",0);
                do_tn_cmd((CHAR) TN_NOP);
                break;
            case IDLE_TAYT:
                debug(F100, "con2host - idle Telnet AYT","",0);
                do_tn_cmd((CHAR) TN_AYT);
                break;
            }
            idlesnd_t = time(NULL);     /* Remember the time */
        }
        else
            debug( F100, "con2host - an error event arrived, ignored","",0);
        break;
    }

}

void
conkbdhandler(void *pArgList) {
    int c, cm, cx, tx, evtcnt;
    int prty = priority, boost = FALSE;
    con_event evt ;
    extern int Shutdown;
#ifdef OS2PM
    HAB oldhab = hab ;

    hab = WinInitialize(0);
    debug ( F101, "WinInitialize hab","",hab) ;
#endif /* OS2PM */

    PostConKbdHandlerThreadSem();
#ifdef NT
    setint();
    SetThreadPrty(priority,isWin95() ? 2 : 11);
#else
    SetThreadPrty(priority,4);
#endif

    while ( !Shutdown ) {         /* Read the keyboard and write to comms line */

        if (
#ifdef KUI
#ifndef K95G
             0 &&
#endif /* K95G */
#endif /* KUI */
             (!IsConnectMode() ||
#ifdef COMMENT
               what != W_CONNECT ||
#endif /* COMMENT */
             VscrnIsPopup(VTERM) ||
             ttyfd == -1
#ifdef CK_TAPI
             || ttyfd == -2
#endif /* CK_TAPI */
             )) {
            /* debug(F110,"conkbdhandler",
               "Waiting for CONNECT_MODE or HOLDSCREEN",0); */
            if (prty != -1 ) {
                prty = -1;      /* lie */
                SetThreadPrty(XYP_REG,0);
            }
            msleep(500);
            continue;
        }

        evtcnt = evtinbuf(VTERM);
        if ( evtcnt >= 5 && (!boost || prty != priority)) {
#ifdef NT
            SetThreadPrty(priority,isWin95() ? 6 : 13);
#else
            SetThreadPrty(priority,6);
#endif
            boost = TRUE;
            prty = priority;
        }
        else if ( evtcnt <= 0 && (boost || prty != priority)) {
#ifdef NT
            SetThreadPrty(priority,isWin95() ? 3 : 11);
#else
            SetThreadPrty(priority,4);
#endif
            boost = FALSE;
            prty = priority;
        }

        evt = congev( VTERM,                    /* Read the VTERM event queue */
#ifdef COMMENT
                      tt_idlesnd_tmo?tt_idlesnd_tmo:-1
#else
                      (tt_idlelimit &&
                        (tt_idleact == IDLE_OUT || tt_idleact == IDLE_TNOP ||
                         tt_idleact == IDLE_TAYT)) ? tt_idlelimit : -1
#endif /* COMMENT */
                      );
        con2host(evt);
    }  /* while (IsConnectMode()) */

#ifdef COMMENT
    WinTerminate(hab) ;
    hab = oldhab ;
#endif /* COMMENT */
    PostConKbdHandlerThreadDownSem();  /* signal thread termination */
    ckThreadEnd(pArgList) ;
}

/* Update the Answerback message
   with current terminal type
   and version number
*/
void
updanswerbk( void )
{
    if ( safeanswerbk ) {
        sprintf(answerback,
                 useranswerbk[0] ? "%s %ld %s_%s\n" : "%s %ld %s\n",
             "K-95",
             vernum,
             tt_info[tt_type].x_name,
             useranswerbk
            );
    }
    else {
        ckstrncpy( answerback, useranswerbk, 80 ) ;
        ckstrncat( answerback, "\n", 80 ) ;
    }

}

/*  C O N E C T  --  Perform CONNECT command  */

static time_t start_t = 0;
static time_t now_t = 0;
static int viewonly_sav = 0;
static bool keylocksav = 0;
static int  holdscreensav;

extern int term_io;
extern int holdscreen;


void
isconnect()
{
    /* ResetThreadPrty();   already done */
    while (IsConnectMode()) {
        msleep( 500 ) ;
        if ( apcactive == APC_INACTIVE || !tt_async ) {
            if ( tt_timelimit || tt_idlelimit )
                now_t = time(NULL);
            if (viewonly)
                continue;

            if ( ttchk() < 0 )
                SetConnectMode(0,CSX_HOSTDISC);

#ifdef BETATEST
            if (!term_io)
                debug(F100,"term_io is false while Connect monitor thread running\r\n","",0);
#endif /* BETATEST */

#ifdef IKS_OPTION
            if (TELOPT_SB(TELOPT_KERMIT).kermit.u_start)
                SetConnectMode(0,CSX_IKSD);
#endif /* IKS_OPTION */

#ifdef COMMENT
#ifdef KUI
            if (evtinbuf(VCMD))
                SetConnectMode(0,CSX_ESCAPE);
#endif /* KUI */
#endif /* COMMENT */

            if (tt_timelimit && (now_t - start_t >= tt_timelimit))
                SetConnectMode(0,CSX_TIME);

            if (tt_idlelimit && (now_t - keypress_t >= tt_idlelimit)) {
                switch ( tt_idleact ) {
                case IDLE_RET:
                    SetConnectMode(0,CSX_IDLE);
                    break;
                case IDLE_EXIT:
                    quitnow = 1;
                    hangnow = 1;
                    strcpy(termessage, "Hangup and quit.\n");
                    SetConnectMode(0,CSX_USERDISC);
                    break;
                case IDLE_HANG:
                    hangnow = 1;
                    strcpy(termessage, "Hangup.\n");
                    SetConnectMode(0,CSX_USERDISC);
                    break;
                }
            }
        }
    }
}

int
unconect1() {
    int x;

#ifdef CKLEARN
    if (learning && learnfp)
      fputs("\n",learnfp);
#endif /* CKLEARN */

    if ( ttchk() < 0 ) {
#ifdef NETDLL
        if ( network && nettype == NET_DLL && ttyfd != -1 )
            ttclos(0);
#endif /* NETDLL */
        if ( ttyfd == -1
#ifdef CK_TAPI
             || ttyfd == -2
#endif /* CK_TAPI */
             )
            /* Discard unsent keystrokes if the connection has been lost */
            while( evtinbuf(VTERM) ) {
                con_event evt;
                getevent( VTERM, &evt );
            }
    }

    if ( !quiet && !viewonly && !termessage[0] ) {
        if ( (!network 
#ifdef TN_COMPORT
	      || istncomport()
#endif /* TN_COMPORT */
	      ) && carrier != CAR_OFF && (ttgmdm() & BM_DCD) == 0)
            strcpy( termessage, "Carrier required but not detected.\n");
        else if ( ttyfd == -1 )
            strcpy( termessage, "Connection closed.\n");
        else if ( tt_timelimit && (now_t - start_t >= tt_timelimit) )
            strcpy( termessage, "Timeout.\n");
        else if ( tt_idlelimit && (tt_idleact >= IDLE_RET && tt_idleact <= IDLE_HANG) &&
                  (now_t - keypress_t >= tt_idlelimit) )
            strcpy( termessage, "Idle timeout.\n");
    }

    if (termessage[0] != '\0')
        printf(termessage);

#ifdef IKS_OPTION
    if ( tt_async ) {                   /* From doconect() */
        if (TELOPT_U(TELOPT_KERMIT) &&
             TELOPT_SB(TELOPT_KERMIT).kermit.u_start &&
             !tcp_incoming && !quiet && ttchk() >= 0
             ) {
            printf("\r\nEnter Client/Server Mode...  Use:\r\n\r\n");
            printf(
" REMOTE LOGIN <user> <password> to log in to the server if necessary.\r\n");
            printf(" SEND and GET for file transfer.\r\n");
            printf(" REMOTE commands for file management.\r\n");
            printf(" FINISH to terminate Client/Server mode.\r\n");
            printf(" BYE to terminate and close connection.\r\n");
            printf(" REMOTE HELP for additional information.\r\n\r\n");
        }
    }
#endif /* IKS_OPTION */

    if ( network ) {
        if ( ttyfd == -1 )
          DialerSend( OPT_KERMIT_HANGUP, 0 ) ;
    } else {
        if ( (carrier != CAR_OFF) &&    /* Carrier dropped */
            ((ttgmdm() & BM_DCD) == 0) )
          DialerSend( OPT_KERMIT_HANGUP, 0 ) ;
    }
    debug( F100,"Exiting connect mode","",0) ;
    SetThreadPrty(priority,3);

    viewonly = viewonly_sav;            /* Restore viewonly state */
    keylock = keylocksav ;              /* Restore keylock state */
    holdscreen = holdscreensav;         /* Restore holdscreen state */
    language = langsv;                  /* Restore language */
    concooked();                        /* Restore console to "cooked" mode */
#ifdef NT
    connoi();
#endif

#ifndef NT
#ifndef KUI
#ifdef OS2MOUSEX
    if ( tt_mouse ) {
        os2_mouseoff() ;
    }
#endif /* OS2MOUSE */
#endif /* KUI */
#endif /* NT */

    SaveTermMode(wherex[VTERM],wherey[VTERM]);

#ifdef PCFONTS
    if (IsOS2FullScreen())
      os2ResetFont() ;
#endif /* PCFONTS */
    return(0);
}

int
unconect2() {
    int x;

    x = cursoron[VTERM];
    cursoron[VTERM] = TRUE;             /* Needed for lgotoxy()... */
    restoreborder();                    /* Put back command-screen border */
    RestoreCmdMode();                   /* Put back command screen */
    restorecursormode();                /* Put command-screen cursor back */
    cursoron[VTERM] = x;                /* Remember this for next time... */
    vmode = VCMD ;
    VscrnIsDirty(VTERM);                /* Force TermScrnUpd() to unblock */
    debug( F111,"unconect2()","hangnow",hangnow);
    if (hangnow) {
        DialerSend( OPT_KERMIT_HANGUP, 0 ) ;
#ifndef NODIAL
        if (mdmhup() < 1)
#endif /* NODIAL */
            tthang();
    }
    debug( F111,"unconect2()","quitnow",quitnow);
    if (quitnow) {
        if (isWin95())
            sleep(1);
        doexit(GOOD_EXIT,0);
    }

    debug(F111,"unconect2()","tt_async",tt_async);
    if ( tt_async ) {                   /* From doconect() */
        if ( ttchk() < 0 ) {
            debug(F110,"unconect2()","ttchk() < 0",0);
#ifdef NETCONN
            debug(F111,"unconect2()","network",network);
            debug(F111,"unconect2()","tn_exit",tn_exit);
            debug(F111,"unconect2()","exitonclose",exitonclose);
            if (network && tn_exit)
                doexit(GOOD_EXIT,xitsta);         /* Exit with good status */
#endif /* NETCONN */
            /* Exit on disconnect if the port is not open or carrier detect */
            else if (exitonclose)
                doexit(GOOD_EXIT,xitsta);
        }
        concb((char)escape);                /* Restore console for commands */
        setint();
    }
    what = W_COMMAND;                   /* Back in command mode. */
    return(1);
}

int
conect(int async) {
    USHORT          len, x, y;
    int             i, c, cm;   /* c is a character, but must be signed
                                 * integer to pass thru -1, which is the
                                 * modem disconnection signal, and is
                                 * different from the character 0377 */
    char errmsg[50], *erp, ac, bc;
    extern int cmdlvl,tlevel;
#ifdef OS2MOUSE
    int button, event ;
    extern int tt_mouse ;
#endif /* OS2MOUSE */
#ifdef NT
    DWORD ExitCode ;
#endif /* NT */
#ifdef CK_TAPI
    extern int tttapi;
    extern int tapipass;
#endif /* CK_TAPI */
#ifdef IKS_OPTION
    int do_iks = 0;
#ifdef CK_AUTODL
    extern int autodl;
#endif /* CK_AUTODL */
    extern int quiet, tcp_incoming;
#endif /* IKS_OPTION */
#ifdef ANYSSH
    extern int ssh_cas;
    extern char * ssh_cmd;
#endif /* ANYSSH */

    viewonly_sav = viewonly;
    keylocksav = keylock ;
    holdscreensav  = holdscreen;
    now_t = time(NULL);/* Set the timers */

    tt_async = async;

    debug(F111,"conect()","tt_timelimit",tt_timelimit);
    debug(F111,"conect()","tt_idlelimit",tt_idlelimit);
    debug(F111,"conect()","start_t",start_t);
    debug(F111,"conect()","now_t",now_t);
    debug(F111,"conect()","tt_async",tt_async);

    /* do not reset timers if we are returning from an APC command */
#ifdef CK_APC
    if ( apcactive == APC_INACTIVE )
#endif /* CK_APC */
        idlesnd_t = keypress_t = start_t = now_t;       /* reset timers */

    term_io = TRUE;             /* handle I/O in the terminal emulator */
#ifdef CK_APC
    apcactive = APC_INACTIVE ;  /* reset APC flag in case we are returning */
#endif /* CK_APC */

#ifdef KUI
#ifndef K95G
    if ( evtinbuf(VCMD) )       /* If the user is entering commands into  */
        return(1) ;             /* the command window, return immediately */
#endif /* K95G */
#endif /* KUI */

    /*
     * The problem.  If there is stuff in the Local Echo Buffer
     * when the connection is dropped, the user is never able to
     * see it.  So if they try to start CONNECT mode we feed the
     * data into the emulator and then fail.
     */
    if ((ttyfd == -1
#ifdef CK_TAPI
          || (!tttapi && ttyfd == -2)
#endif /* CK_TAPI */
          ) && (ttchk() > 0)) {
        int ch;
        debug(F110,"conect()","local echo buffer copied to terminal",0);
        while ((ch = ttinc(-50))>=0) {
            scriptwrtbuf(ch);
        }
    }
    else if ( !viewonly ) {    /* View only requires no connection checks */
        if (!network) {
            if (ttyfd == -1) {
                printf("Sorry, you must first open a device or connection.\n");
                return (0);
            }

            if (speed < 0) {
                printf("Sorry, you must set speed first.\n");
                return (0);
            }
        }
#ifdef SSHBUILTIN
        if ( network && nettype == NET_SSH && ssh_cas && ssh_cmd && 
             !(strcmp(ssh_cmd,"kermit") && strcmp(ssh_cmd,"sftp"))) {
            printf("?SSH Subsystem active: %s\n", ssh_cmd);
            return(0);
        }
#endif /* SSHBUILTIN */
        if (tt_escape && ((escape < 0) || (escape > 0177))) {
            printf("Escape character out of range: %d\n", escape);
            return (0);
        }
#ifdef IKS_OPTION
        do_iks = (ttyfd == -1) && ((local && network && IS_TELNET())
                                    || (!local && sstelnet));
#endif /* IKS_OPTION */

        if (ttopen(ttname, &local, network ? -nettype : mdmtyp, 0) < 0) {
            erp = errmsg;
            sprintf(erp, "Sorry, can't open %s", ttname);
            perror(errmsg);
            return (0);
        }

#ifdef IKS_OPTION
        if ( do_iks ) {
            if (TELOPT_U(TELOPT_KERMIT)) {
                /* If the remote side is in a state of IKS START-SERVER    */
                /* we request that the state be changed.  We will detect   */
                /* a failure to adhere to the request when we call ttinc() */
                if (!iks_wait(KERMIT_REQ_STOP,0) && !tcp_incoming) {
                    if (!quiet) {
                        printf("\r\nEnter Client/Server Mode...  Use:\r\n\r\n");
                        printf(
 " REMOTE LOGIN <user> <password> to log in to the server if necessary.\r\n");
                        printf(" SEND and GET for file transfer.\r\n");
                        printf(" REMOTE commands for file management.\r\n");
                        printf(" FINISH to terminate Client/Server mode.\r\n");
                        printf(" BYE to terminate and close connection.\r\n");
                        printf(" REMOTE HELP for additional information.\r\n\r\n");
                    }
                    return(0);      /* Failure */
                }
            }

            /* Let our peer know our state. */
#ifdef CK_AUTODL
            if (TELOPT_ME(TELOPT_KERMIT)) {
                if (autodl && !TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
                    tn_siks(KERMIT_START);      /* Send Kermit-Server Start */
                } else if (!autodl && TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
                    tn_siks(KERMIT_STOP);
                }
            }
#else /* CK_AUTODL */
            if (TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
                tn_siks(KERMIT_STOP);
            }
#endif /* CK_AUTODL */
        }
#endif /* IKS_OPTION */


        /* Condition console terminal and communication line */
        if (ttvt(speed, flow) < 0) {
#ifdef CK_TAPI
            if ( tttapi && !tapipass )
                printf("Sorry, you must DIAL a number first.\n");
            else
#endif /* CK_TAPI */
                printf("Sorry, Can't condition communication line\n");
            return (0);
        }

#ifdef CK_TAPI
        if ( tttapi && !tapipass && ttyfd == -2 ) {
            printf("?Sorry, you must DIAL a number first\n");
            return (0);
        }
#endif /* CK_TAPI */
        if (!network && carrier != CAR_OFF && (ttgmdm() & BM_DCD) == 0) {
#ifndef NOHINTS
            extern int hints;
#endif /* NOHINTS */
            printf("?Carrier required but not detected on \"%s\".\n",ttname);

#ifndef NOHINTS
            if (!hints)
                return(0);

            printf("***********************************\n");
            printf(
           "Hint: Use the DIAL command, or else SET CARRIER-WATCH OFF.\n");
            printf(
          "If you want to make a network connection, use SET HOST or TELNET.\n"
                   );
            printf("***********************************\n\n");
#endif /* NOHINTS */
            return (0);
        }
    } /* if (!viewonly) */


    now_t = time(NULL);/* Set the timers */
    tt_async = async;

    debug(F111,"conect()","tt_timelimit",tt_timelimit);
    debug(F111,"conect()","tt_idlelimit",tt_idlelimit);
    debug(F111,"conect()","start_t",start_t);
    debug(F111,"conect()","now_t",now_t);
    debug(F111,"conect()","tt_async",tt_async);

    /* do not reset timers if we are returning from an APC command */
#ifdef CK_APC
    if ( apcactive == APC_INACTIVE )
#endif /* CK_APC */
        idlesnd_t = keypress_t = start_t = now_t;       /* reset timers */

    term_io = TRUE;             /* handle I/O in the terminal emulator */
#ifdef CK_APC
    apcactive = APC_INACTIVE ;  /* reset APC flag in case we are returning */
#endif /* CK_APC */

    /* Will Kermit exit when user escapes back? */
    esc_exit = (cflg || cnflg) && !stayflg;
    esc_msg  = esc_exit ? "Exit:" :
#ifdef COMMENT
#ifndef NOSPL
                 cmdlvl == 0
#else
                 tlevel == -1
#endif /* NOSPL */
                     ? "Prompt:" : "Command:"
#else /* COMMENT */
                         "Command:"
#endif /* COMMENT */
                         ;

    keymac = 0;                         /* Initialize \Kmacro flag */
    keymacx = -1;                       /* and index */

    outshift = 0;                       /* Initial SO shift state. */
    langsv = language;                  /* No language rules during CONNECT! */
    language = L_USASCII;
    if (debses) {                       /* Session debugging? */
        escstate = ES_NORMAL;           /* No escape-sequence processing */
        tt_wrap = TRUE;                 /* Always wrap */
    }
    pmask = (parity ? 0177 : 0377) & cmask ;

    /* initialize DEC Graphic Set Translation functions if necessary */
    for ( i = 0 ; i < 4 ; i++ ) {
        if ( G[i].init ) {
            if ( tcs_transp ) {
                G[i].rtoi = NULL;
                G[i].itol = NULL;
                G[i].ltoi = NULL;
                G[i].itor = NULL;
            } else {
                G[i].rtoi = xl_u[G[i].designation];
                G[i].itol = xl_tx[tcsl] ;
                G[i].ltoi = xl_u[tcsl] ;
                G[i].itor = xl_tx[G[i].designation];
            }
            G[i].init = FALSE ;
        }
    }
    os2_outesc = ES_NORMAL;                     /* Initial state of recognizer */

#ifdef CK_BORDER
    saveborder();                       /* Save command-screen border */
#endif /* CK_BORDER */

    checkscreenmode();                  /* Initialize terminal emulator */
    setcursormode();
    GetCurPos(&y, &x);                  /* Command screen cursor position */
    SaveCmdMode(x+1,y+1);               /* Remember Command screen */
    RestoreTermMode();                  /* Put up previous terminal screen */
#ifndef KUI
    scrninit();
#ifndef NT
#ifdef OS2MOUSEX
    if ( tt_mouse ) {
        os2_mouseon() ;
        }
#endif /* OS2MOUSE */
#endif /* NT */
#endif /* KUI */

#ifdef PCFONTS
    if (IsOS2FullScreen())
      os2SetFont() ;
#endif /* PCFONTS */

    /* configure cursor */
    {
        int cursor_offset = (scrollstatus[VTERM] && tt_roll[VTERM]) ?
          (vscrn[VTERM].top + vscrn[VTERM].linecount
           - vscrn[VTERM].scrolltop)%vscrn[VTERM].linecount : 0 ;
        if ( !cursorena[VTERM] ||
            vscrn[VTERM].cursor.y + cursor_offset >= VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0) ) {
            cursoron[VTERM] = TRUE ;
        } else {
            cursoron[VTERM] = FALSE ;
        }
    }

    conraw();                           /* Put console in raw mode */
    connoi();                           /* No interrupts from console */
    ipadl25();                          /* Put up status line */
    if ( scrollstatus[VTERM] )
      scrollstatusline() ;
    waittime = tt_ctstmo * 1000L;       /* CTS timeout */

#ifdef CKLEARN
    if (learning) {                     /* Learned script active... */
        learnbp = 0;                    /* INPUT buffer pointer */
        learnbc = 0;                    /* INPUT buffer count */
        learnst = 0;                    /* State (0 = neutral, none) */
        learnt1 = (ULONG) time(0);
    }
#endif /* CKLEARN */

    /* Create a thread to read the comms line and write to the screen */

    SetConnectMode(1,CSX_INTERNAL);     /* So thread 2 doesn't end at once */
    what = W_CONNECT;                   /* Keep track of what we're doing */
    quitnow = 0;
    hangnow = 0;
    termessage[0] = 0;
    if ( viewonly ) {
        keylock = TRUE ;
        holdscreen = TRUE;
    }
    VscrnIsDirty(VTERM);
    VscrnIsDirty(VCMD);

    if ( !async )
        isconnect();
    else
        _beginthread(isconnect,
#ifdef OS2ONLY
                      0,
#endif /* OS2ONLY */
                      65536, 0);

    if (tt_timelimit && (now_t - start_t >= tt_timelimit))
        return(0);
    if (tt_idlelimit && (tt_idleact == IDLE_RET) &&
         (now_t - keypress_t >= tt_idlelimit))
        return(0);
    return (1);                         /* Success */
}

void
SmoothScroll( void ) {
    updmode = TTU_SMOOTH ;
}

void
JumpScroll( void ) {
    updmode = TTU_FAST ;
}

void
SetConnectMode( BOOL mode, int ExitCode ) {
    con_event evt;
    extern int cx_status, cmdlvl;
#ifdef DCMDBUF
    extern struct cmdptr *cmdstk;
#else
    extern struct cmdptr cmdstk[];
#endif /* DCMDBUF */
#ifdef KUI
    char buf[30];
#endif /* KUI */

    if (RequestConnectModeMutex( 200 ) )
		return;
    if ( ConnectMode != mode ) {
        debug(F111,"SetConnectMode change","from",ConnectMode);
        debug(F111,"SetConnectMode change","to",mode);
        ConnectMode = mode ;
        evt.type = error;

        switch ( mode ) {
        case VTERM:
            putevent(VTERM,evt);
            PostTerminalModeSem();
            ResetCommandModeSem();
            cx_status = CSX_INTERNAL;
#ifdef KUI
            KuiSetTerminalStatusText(STATUS_CMD_TERM, "Terminal Screen");
            KuiSetTerminalConnectButton(1);
            KuiSetTerminalStatusText(STATUS_PROTO, (char *)protoString());
            ckmakmsg(buf,30,ckitoa(vscrn[mode].cursor.x+1),", ",
                      ckitoa(vscrn[mode].cursor.y+1),NULL);
            KuiSetTerminalStatusText(STATUS_CURPOS, buf);
            ckmakmsg(buf,30,ckitoa(vscrn[mode].width)," x ",
                      ckitoa(vscrn[mode].height-(tt_status[vmode]?1:0)),NULL);
            KuiSetTerminalStatusText(STATUS_HW, buf);
#endif /* KUI */
            break;
        case VCMD:
            /* putevent(VCMD,evt); */
            unconect1();
            if ( tt_async ) {
#ifndef NOKVERBS
				if ((keymac > 0) && (keymacx > -1)) { /* Executing a keyboard macro? */
					/* Set up the macro and return */
					/* Do not clear the keymac flag */
					dodo(keymacx,NULL,CF_KMAC|cmdstk[cmdlvl].ccflgs);
				}
#endif /* NOKVERBS */
                resconn();
			}
            PostCommandModeSem();
            ResetTerminalModeSem();
            unconect2();
            cx_status = ExitCode;
#ifdef KUI
            KuiSetTerminalStatusText(STATUS_CMD_TERM, "Command Screen");
            KuiSetTerminalConnectButton(0);
            KuiSetTerminalStatusText(STATUS_PROTO, (char *)protoString());
            ckmakmsg(buf,30,ckitoa(vscrn[mode].cursor.x+1),", ",
                      ckitoa(vscrn[mode].cursor.y+1),NULL);
            KuiSetTerminalStatusText(STATUS_CURPOS, buf);
            ckmakmsg(buf,30,ckitoa(vscrn[mode].width)," x ",
                      ckitoa(vscrn[mode].height-(tt_status[vmode]?1:0)),NULL);
            KuiSetTerminalStatusText(STATUS_HW, buf);
#endif /* KUI */
            break;
        default:
            if (mode)
                unconect1();
            if ( tt_async )
                resconn();
            ResetTerminalModeSem();
            ResetCommandModeSem();
            if ( mode )
                unconect2();
            cx_status = ExitCode;
#ifdef KUI
            KuiSetTerminalStatusText(0, "(unknown)");
#endif /* KUI */
        }
        msleep(50);           /* Give other threads a chance to catch up */
    }
    ReleaseConnectModeMutex() ;
}

int
StartConnectThreads( void )
{
    int i=0;
    extern int Shutdown;

    Shutdown = FALSE;
    tidRdComWrtScr = (TID) ckThreadBegin( &rdcomwrtscr,
                                          THRDSTKSIZ, 0, TRUE, 0 ) ;
    if ( tidRdComWrtScr == (TID) -1 ) {
        printf("Sorry, can't create thread\n");
        SetConnectMode(0,CSX_INTERNAL) ;
        fatal("Sorry, can't create thread.");
        return(0);
    }
    i = 0 ;
    while ( !WaitAndResetRdComWrtScrThreadSem( 1000 ) ) {
        debug(F100,"Waiting for RdComWrtScrThreadSem","",0) ;
        printf("Waiting for RdComWrtScrThreadSem\n") ;
        if ( ++i > 5 ) {
            printf("Sorry, giving up ...\n");
            SetConnectMode(0,CSX_INTERNAL) ;
            return(0);
        }
    }

    /* Use a separate thread for Keyboard handling */
    tidConKbdHandler = (TID) ckThreadBegin( &conkbdhandler,
                                            THRDSTKSIZ, 0, TRUE, 0 ) ;
    if ( tidConKbdHandler == (TID) -1) {
        printf("Sorry, can't create thread\n");
        SetConnectMode(0,CSX_INTERNAL) ;
        fatal("Sorry, can't create thread.");
        return(0);
    }
    i = 0 ;
    while ( !WaitAndResetConKbdHandlerThreadSem( 2500 ) ) {
        debug(F100,"Waiting for ConKbdHandlerThreadSem","",0) ;
        printf("Waiting for ConKbdHandlerThreadSem\n") ;
        if ( ++i > 5 ) {
           printf("Sorry, giving up...\n");
            SetConnectMode(0,CSX_INTERNAL) ;
            return(0);
       }
    }
    return(1);
}

int
StopConnectThreads( int wait4ever )
{
    int i=0;
#ifdef NT
    DWORD ExitCode ;
#endif /* NT */
    extern int Shutdown;

    if ( !tidConKbdHandler )
        return(0);

    debug(F111,"StopConnectThreads","wait4ever",wait4ever);

    Shutdown = TRUE;
    puterror(VTERM);

    while ( !tt_async &&
            !WaitAndResetConKbdHandlerThreadDownSem( 1000 ) ) {
        debug(F100,"Waiting for ConKbdHandlerThreadDownSem","",0) ;
#ifdef NT
        ExitCode = STILL_ACTIVE ;
        if ( GetExitCodeThread( tidConKbdHandler, &ExitCode ) )
            if ( ExitCode != STILL_ACTIVE ) {
              debug( F111,"ConKbdHandler not active","ExitCode",ExitCode);
              break;
            }
#endif /* NT */
        if (!wait4ever && ++i > 5 ) {
            printf("ConKbdHandlerThreadThread did not die\n");
            debug(F100,"ConKbdHandlerThread did not die.","",0);
            break;
        }
    }
    debug(F100,"ConKbdHandlerThread might be dead","",0);

    i = 0;
    if ( !viewonly ) {
        while ( !WaitAndResetRdComWrtScrThreadDownSem( 1000 ) ) {
            debug(F100,"Waiting for RdComWrtScrThread to die","",0);
#ifdef NT
            ExitCode = STILL_ACTIVE ;
            if ( GetExitCodeThread( tidRdComWrtScr, &ExitCode ) )
              if ( ExitCode != STILL_ACTIVE ) {
                  debug( F111,"RdComWrtScrThread not active",
                        "ExitCode",ExitCode);
                  break;
              }
#endif /* NT */
                if (!wait4ever && ++i > 5 ) {
                    printf("RdComWrtScrThread did not die\n");
                    debug(F100,"RdComWrtScrThread did not die.","",0);
                    break;
                }
        }
        debug(F100,"RdComWrtScrThread might be dead","",0);
    }

    i=0;
#ifndef NT
    DosWaitThread( &tidRdComWrtScr, DCWW_WAIT );
#endif /* NT */
    debug(F100,"Connect mode threads are dead","",0) ;
    puterror(VTERM);
    return(1);
}

/* The routines that follow provide an interface between the command parser
 * and the actual functionality used to implement the command when it is K95
 * specific.  This way KUI can call the routines without using the command
 * parser.  

 * This function accepts Window Height and Width.  The Window Height includes
 * the status line if there is one.  This is important to remember for the 
 * difference between the terminal and command windows which may have different
 * values for tt_status.
 */

#ifdef KUI
int
kui_setheightwidth(int x, int y)
{
    tt_szchng[VTERM] = (tt_status[VTERM]?2:1);
    tt_rows[VTERM] = y - (tt_status[VTERM]?1:0);
    tt_cols[VTERM] = x;
    tt_cols_usr = x;
    VscrnSetWidth( VTERM, x);
    VscrnInit( VTERM );         /* Height set here */
#ifdef TNCODE
    if (TELOPT_ME(TELOPT_NAWS))
        tn_snaws();
#endif /* TNCODE */
#ifdef RLOGCODE
    if (TELOPT_ME(TELOPT_NAWS))
        rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
    ssh_snaws();
#endif /* SSHBUILTIN */

    tt_szchng[VCMD] = (tt_status[VCMD]?2:1);
    tt_rows[VCMD] = cmd_rows = y - (tt_status[VCMD]?1:0);
    tt_cols[VCMD] = x;
    VscrnSetWidth(VCMD, x);
    cmd_cols = x;
    VscrnInit(VCMD);
    SetCols(VCMD);

    return(1);
}
#endif /* KUI */

int
os2_settermwidth(int x)
{
    if (x % 2) {
        printf("\n?The width must be an even value\n.");
        return(0);
    }

    if ( IsOS2FullScreen() ) {
        if ( x != 40 && x != 80 && x != 132 ) {
            printf("\n?The width must be 40, 80,");
#ifdef NT
                  printf(" or 132 under Windows 95.\n.");
#else /* NT */
                  printf(" or 132 in a Full Screen session.\n.");
#endif /* NT */
            return(0);
        }
    } else {
        if ( !IsWARPed() && x != 80 ) {
            printf("\n?OS/2 version is pre-WARP: the width must equal ");
            printf("80 in a Windowed Session\n.");
            return(0);
        }
        if (x < 20 || x > MAXTERMCOL ) {
            printf("\n?The width must be between 20 and %d\n.",MAXTERMCOL);
            return(0);
        }
    }

#ifdef KUI
    return(kui_setheightwidth(x,tt_rows[VTERM]+(tt_status[VTERM]?1:0)));
#else /* KUI */
    if (x > 8192/(tt_rows[VTERM]+1)) {
        printf("\n?The max screen area is 8192 cells: %d(rows) x %d(cols) = %d cells.\n",
                tt_rows[VTERM]+1,x,x*(tt_rows[VTERM]+1));
        return(0);
    }

    tt_cols[VTERM] = x;
    tt_cols_usr = x;
    if (SysInited) {
        VscrnSetWidth( VTERM, x);
#ifdef TNCODE
        if (TELOPT_ME(TELOPT_NAWS))
            tn_snaws();
#endif /* TNCODE */
#ifdef RLOGCODE
        if (TELOPT_ME(TELOPT_NAWS))
            rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
        ssh_snaws();
#endif /* SSHBUILTIN */
    }
/*
   We do not set tt_szchng here because that would result in the screen buffer
   being reallocated and the screen cleared.  But that is not necessary when
   only the screen width is being changed since the buffer allocates the full
   width
*/
    return(1);
#endif /* KUI */
}

int
os2_settermheight(int x)
{
#ifdef KUI
    return(kui_setheightwidth(tt_cols[VTERM],x+(tt_status[VTERM]?1:0)));
#else /* KUI */
    if (x > 8192/(tt_cols[VTERM])) {
        printf("\n?The max screen area is 8192 cells: %d(rows) x %d(cols) = %d cells.\n",
                x,tt_cols[VTERM],x*(tt_cols[VTERM]));
        return(0);
    }

    tt_szchng[VTERM] = (tt_status[VTERM]?2:1);
    tt_rows[VTERM] = x;
    if (SysInited) {
        VscrnInit( VTERM );             /* Height set here */
#ifdef TNCODE
        if (TELOPT_ME(TELOPT_NAWS))
            tn_snaws();
#endif /* TNCODE */
#ifdef RLOGCODE
        if (TELOPT_ME(TELOPT_NAWS))
            rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
        ssh_snaws();
#endif /* SSHBUILTIN */
    }
    return(1);
#endif /* KUI */
}

int
os2_setcmdheight(int x)
{
    extern int tt_modechg;

    if (IsOS2FullScreen()) {
        if ( tt_status[VCMD] ) {
            if (x != 24 && x != 42 && x != 49 && x != 59) {
                printf("\n?The height must be 24, 42, 49");
#ifdef NT
                printf(" or 59 under Windows 95.\n.");
#else /* NT */
                printf(" or 59 in a Full Screen session.\n.");
#endif /* NT */
                return(0);
            }
        } else {
            if (x != 25 && x != 43 && x != 50 && x != 60) {
                printf("\n?The height must be 25, 43, 50");
#ifdef NT
                printf(" or 60 under Windows 95.\n.");
#else /* NT */
                printf(" or 60 in a Full Screen session.\n.");
#endif /* NT */
                return(0);
            }
        }
    } else if (tt_modechg == TVC_W95) {
        if (tt_status[VCMD]) {
            if (x != 24 && x != 42 && x != 59) {
                printf("\n?The height must be 24, 42, 49");
#ifdef NT       
                printf(" under Windows 95.\n.");
#else /* NT */  
                printf(" in a Full Screen session.\n.");
#endif /* NT */
                return(0);
            }
        } else {
            if (x != 25 && x != 43 && x != 50) {
                printf("\n?The height must be 25, 43, 50");
#ifdef NT       
                printf(" under Windows 95.\n.");
#else /* NT */  
                printf(" in a Full Screen session.\n.");
#endif /* NT */ 
                return(0);
            }
        }
    } else {
        if (x < 8 || x > MAXTERMROW ) {
            printf("\n?The height must be between 8 and %d\n.",
                    MAXTERMROW);
            return(0);
        }
    }
#ifdef KUI
    return(kui_setheightwidth(tt_cols[VCMD],x+(tt_status[VCMD]?1:0)));
#else /* KUI */
    if (x > 8192/tt_cols[VCMD]) {
        printf("\n?The max screen area is 8192 cells: %d(rows) x %d(cols) = %d cells.\n",
                x,tt_cols[VCMD],x*tt_cols[VCMD]);
                return(0);
    }

    tt_szchng[VCMD] = (tt_status[VCMD]?2:1);
    tt_rows[VCMD] = cmd_rows = x;
    VscrnInit(VCMD);
    SetCols(VCMD);
    return(1);
#endif /* KUI */
}

int
os2_setcmdwidth(int x)
{
    if (IsOS2FullScreen()) {
        if (x != 40 && x != 80 && x != 132) {
            printf("\n?The width must be 40, 80,");
#ifdef NT
            printf(" or 132 under Windows 95.\n.");
#else /* NT */
            printf(" or 132 in a Full Screen session.\n.");
#endif /* NT */
            return(0);
        }
    } else {
        if (!IsWARPed() && x != 80) {
            printf("\n?OS/2 version is pre-WARP: the width must");
            printf("equal 80 in a Windowed Session\n.");
            return(0);
        }
        if (x < 20 || x > MAXTERMCOL) {
            printf("\n?The width must be between 20 and %d\n.",MAXTERMCOL);
            return(0);
        }
    }
#ifdef KUI
    return(kui_setheightwidth(x,tt_rows[VCMD]+(tt_status[VCMD]?1:0)));
#else /* KUI */
    if (x > 8192/(tt_rows[VCMD]+1)) {
        printf("\n?The max screen area is 8192 cells: %d(rows) x %d(cols) = %d cells.\n",
                tt_rows[VCMD]+1,x,x*(tt_rows[VCMD]+1));
        return(0);
    }
    tt_cols[VCMD] = x;
    VscrnSetWidth(VCMD, x);
    cmd_cols = x;
    SetCols(VCMD);
    return(1);
#endif /* KUI */
}
#endif /* NOLOCAL */
