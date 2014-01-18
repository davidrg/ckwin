/* C K O C O 3 . C */

/*
  Authors: Frank da Cruz (fdc@columbia.edu),
             Columbia University Academic Information Systems, New York City.
           Jeffrey E Altman (jaltman@secure-endpoints.com)
             Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of
  New York.  All rights reserved.  This copyright notice must not be removed,
  altered, or obscured.
*/

#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckcker.h"             /* Kermit definitions */
#include "ckcasc.h"             /* ASCII character symbols */
#include "ckcnet.h"             /* Network support */
#include "ckuusr.h"             /* For terminal type definitions, etc. */
#include "ckcxla.h"             /* Character set translation */
#include "ckcuni.h"             /* Unicode Character Set Translations */
#ifndef NOLOCAL
#include "ckowys.h"
#include "ckodg.h"
#include "ckoava.h"
#include "ckotvi.h"
#include "ckohzl.h"
#include "ckohp.h"
#include "ckovc.h"
#include "ckoi31.h"
#include "ckoqnx.h"
#include "ckoadm.h"
#endif /* NOLOCAL */

#include <ctype.h>              /* Character types */
#include <io.h>                 /* File io function declarations */
#include <process.h>            /* Process-control function declarations */
#include <stdlib.h>             /* Standard library declarations */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define DECLED

#ifdef NT
#include <windows.h>
#else /* NT */

#ifdef OS2MOUSE
#define INCL_MOU
#endif /* OS2MOUSE */

#define INCL_NOPM
#define INCL_VIO
#define INCL_ERRORS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVIOCTL
#define INCL_WINCLIPBOARD
#include <os2.h>
#undef COMMENT                /* COMMENT is defined in os2.h */
#endif /* NT */
#include "ckocon.h"             /* definitions common to console routines */

#ifndef NOLOCAL
#include "ckokey.h"
#include "ckotek.h"
#include "ckowin.h"

#ifdef CK_NETBIOS
#include "ckonbi.h"
extern UCHAR NetBiosRemote[] ;
#endif /* CK_NETBIOS */

#ifdef KUI
#include "ikui.h"
#endif /* KUI */

#ifdef NETCONN
#ifdef TCPSOCKET
#ifdef CK_NAWS
_PROTOTYP( int tn_snaws, (void) );
#ifdef RLOGCODE
_PROTOTYP( int rlog_naws, (void) ) ;
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
_PROTOTYP( int ssh_snaws, (void));
#endif /* SSHBUILTIN */
extern int me_naws;
#endif /* CK_NAWS */

extern int tn_rem_echo;
extern int tcp_incoming;
#endif /* TCPSOCKET */
#endif /* NETCONN */

_PROTOTYP(void vtescape, (void));
_PROTOTYP(void vt100, (unsigned short vtch));
#endif /* NOLOCAL */

/*
 *
 * =============================externals=====================================
 */
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR);  /* Character set xlate */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR);  /* functions. */
extern char     *printername;
extern int      printpipe;
#ifdef BPRINT
extern int  printbidi;                  /* SET BPRINTER (bidirectional) */
#endif /* BPRINT */
bool winprint = FALSE;
FILE *lst=NULL;                 /* List (print) device */
#ifdef NT
_PROTOTYP(int Win32PrtOpen, (char *));
_PROTOTYP(int Win32PrtWrite, (char *, int));
_PROTOTYP(int Win32PrtClose, (void));
#endif /* NT */
static time_t printerclose_t=0;
bool     printon        = FALSE; /* Printer is on */

#ifdef NT
_PROTOTYP( FILE * win95popen, (const char *cmd, const char *mode) );
_PROTOTYP( int win95pclose, (FILE *pipe) );
#define popen _popen
#define pclose _pclose
#endif /* NT */

#ifdef COMMENT
_PROTOTYP( FILE * popen, (const char *cmd, const char *mode));
_PROTOTYP( int pclose, (FILE *pipe));
#endif /* COMMENT */

#ifndef NOLOCAL
extern struct csinfo fcsinfo[]; /* File character set info */
extern int tcsr, tcsl;          /* Terminal character sets, remote & local. */
extern int prncs;               /* Printer Character Set */
extern int tcs_transp;

extern int ttmdm;
extern int tnlm, tn_nlm;        /* Terminal newline mode, ditto for TELNET */
extern int tn_b_nlm ;           /* TELNET BINARY newline mode */
extern int tt_crd;              /* Carriage-return display mode */
extern int tt_lfd;              /* Line-feed display mode */
extern int tt_bell;             /* How to handle incoming Ctrl-G characters */
extern int tt_type, tt_type_mode ;
extern int tt_status[VNUM];           /* Terminal status line displayed */
extern int tt_status_usr[VNUM];
extern int tt_modechg;          /* Terminal Video-Change (80 or 132 cols) */
extern int tt_senddata;         /* May data be sent to the host */
extern int tt_hidattr;          /* Attributes do not occupy a space */
#ifdef PCTERM
extern int tt_pcterm;
#endif /* PCTERM */
extern int ttyfd, Shutdown ;
#ifdef CK_TAPI
extern int tttapi;
#endif /* CK_TAPI */
#ifdef KUI
extern CKFLOAT  tt_linespacing[VNUM];
#endif /* KUI */
extern long     speed, vernum;
extern int      local, escape, duplex, parity, flow, seslog, pmask,
                cmdmsk, cmask, sosi, xitsta, debses, mdmtyp, carrier, what;
extern int      cflg, cnflg, stayflg, tt_escape, tt_scroll;
extern int      network, nettype, ttnproto, protocol, inautodl;
extern int cmdlvl,tlevel, ckxech;
extern int ttnum;                               /* from ckcnet.c */
extern int tt_async;
#ifndef NOXFER
extern int      autodl, adl_ask;
int adl_kc0 = TRUE,
    adl_zc0 = TRUE;     /* Process autodownload C0 chars by emulation */
extern int adl_err, xferstat;
extern CHAR stchr;
#endif /* NOXFER */
int duplex_sav = -1 ;

#ifndef NOSPL
extern struct mtab *mactab;             /* Main macro table */
extern int nmac;                        /* Number of macros */
#endif /* NOSPL */
#ifdef TNCODE
extern int tn_deb;
#endif /* TNCODE */

extern KEY      *keymap;
extern MACRO    *macrotab;
extern char     ttname[], sesfil[];
#ifndef NODIAL
extern char     * d_name;
#endif /* NODIAL */

extern int  scrninitialized[] ;
extern bool scrollflag[] ;
extern bool viewonly ;           /* View Only Terminal mode */
extern int  updmode ;            /* Fast/Smooth scrolling */
extern int priority ;
extern TID  tidRdComWrtScr ;

#ifdef CK_XYZ
extern int p_avail ;
#endif /* CK_XYZ */

extern int beepfreq, beeptime ;
extern int pwidth, pheight;
extern int win95lucida, win95hsl;

/*
 * =============================variables==============================
 */

/*
  These are RGB bits for the fore- and background colors in the PC's video
  adapter, 3 bits for each color.  These default values can be changed by the
  SET TERMINAL COLOR command (in ckuus7.c) or by CSI3x;4xm escape sequences
  from the host.
*/
unsigned char     colornormal     = 0x17;
unsigned char     colorunderline  = 0x47;
unsigned char     colordebug      = 0x47;
unsigned char     colorreverse    = 0x71;
unsigned char     colorgraphic    = 0x17;
#ifdef COMMENT
unsigned char     colorstatus     = 0x37;
unsigned char     colorhelp       = 0x37;
#else /* COMMENT */
unsigned char     colorstatus     = 0x71;
unsigned char     colorhelp       = 0x71;
#endif /* COMMENT */
unsigned char     colorselect     = 0xe0;
unsigned char     colorborder     = 0x01;
unsigned char     coloritalic     = 0x27;

int bgi = FALSE, fgi = FALSE ;
unsigned char     colorcmd        = 0x07;
int colorreset    = TRUE ;  /* reset on CSI 0 m - use normal colors */
int erasemode     = FALSE ; /* Use current colors when erasing characters */
int user_erasemode= FALSE ; /* Use current colors when erasing characters */
int trueblink     = TRUE ;
int truereverse   = TRUE ;
int trueunderline = TRUE ;
int truedim       = TRUE ;
#ifdef KUI
int trueitalic    = TRUE ;
#else /* KUI */
int trueitalic    = FALSE ;
#endif /* KUI */

extern enum markmodes markmodeflag[VNUM] = {notmarking, notmarking,
                                                notmarking, notmarking} ;

extern int tn_bold;                     /* TELNET negotiation bold */
extern int esc_exit;                    /* Escape back = exit */
extern char * esc_msg;
extern int interm ;                     /* INPUT echo state */
extern BYTE vmode ;

long waittime;                  /* Timeout on CTS during CONNECT */
#define INTERVAL 100L

char termessage[MAXTERMCOL];

#ifdef CK_APC
extern int apcactive;                   /* Application Program Command (APC) */
int apcrecv = 0;
int dcsrecv = 0;                        /* Device Control String (DCS) */
int oscrecv = 0;                        /* Operating System Command (OSC) */
int pmrecv  = 0;                        /* Private Message (PM) */
int pu1recv = 0;                        /* 97801-5xx Private Use One (PU1) */
int pu2recv = 0;                        /* 97801-5xx Private Use Two (PU2) */
int c1strrecv = 0;                      /* Other generic C1 strings */
extern int apcstatus;                   /* items ... */
#ifdef DCMDBUF
extern char *apcbuf;
#else
extern char apcbuf[];
#endif /* DCMDBUF */
int apclength = 0;
int apcbuflen = APCBUFLEN - 2;
#endif /* CK_APC */

extern char * keydefptr;
extern int keymac;
extern int keymacx ;

bool scrollstatus[VNUM] = {FALSE,FALSE,FALSE,FALSE} ;
bool escapestatus[VNUM] = {FALSE,FALSE,FALSE,FALSE} ;
extern int tt_idlelimit;                /* Auto-exit Connect when idle */
extern int tt_timelimit;                /* Auto-exit Connect after time */
extern bool flipscrnflag[] ;


extern videobuffer vscrn[];

ascreen                                 /* For saving screens: */
  vt100screen,                          /* terminal screen */
  commandscreen ;                       /* OS/2 screen */

extern ascreen mousescreen; /* Screen during mouse actions */

extern unsigned char                    /* Video attribute bytes */
    attribute=NUL,                      /* Current video attribute byte */
    underlineattribute=NUL,
    savedattribute[VNUM]={0,0,0,0},       /* Saved video attribute byte */
    saveddefaultattribute[VNUM]={0,0,0,0},/* Saved video attribute byte */
    savedunderlineattribute[VNUM]={0,0,0,0},/* Saved video attribute byte */
    defaultattribute=NUL,               /* Default video attribute byte */
    italicattribute=NUL,                /* Default video attribute byte */
    saveditalicattribute[VNUM]={0,0,0,0},
    reverseattribute=NUL,
    savedreverseattribute[VNUM]={0,0,0,0},
    graphicattribute=NUL,
    savedgraphicattribute[VNUM]={0,0,0,0},
    borderattribute=NUL,
    savedborderattribute[VNUM]={0,0,0,0};

vtattrib attrib={0,0,0,0,0,0,0,0,0,0},
         savedattrib[VNUM]={{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0}},
         cmdattrib={0,0,0,0,0,0,0,0,0,0};

int wherex[];                           /* Screen column, 1-based */
int wherey[];                           /* Screen row, 1-based */
int margintop = 1 ;                     /* Top of scrolling region, 1-based */
int marginbot = 24 ;                    /* Bottom of same, 1-based */
int marginleft = 1;
int marginright = 80;

int quitnow, hangnow, outshift, tcs, langsv;

int term_io = TRUE;                     /* Terminal emulator performs I/O */

int prevchar = 0;                       /* Last char written to terminal screen */

extern char answerback[81];             /* answerback */
char usertext[(MAXTERMCOL) + 1];        /* Status line and its parts */
char statusline[MAXTERMCOL + 1];
char hoststatusline[MAXTERMCOL + 1];
char exittext[(20) + 1];
#define HLPTXTLEN 41
char helptext[HLPTXTLEN];
char filetext[(20) + 1];
char savefiletext[(20) + 1] = { NUL };
#define HSTNAMLEN 41
char hostname[HSTNAMLEN];

#define DEFTABS \
"0\
T0000000T0000000T0000000T0000000T0000000T0000000T0000000T0000000T0000000\
T0000000T0000000T0000000T0000000T0000000T0000000T0000000T0000000T0000000\
T00000";

char htab[MAXTERMCOL+2] = DEFTABS       /* Default tab settings */

static int achar;                       /* Global - current character */
int tt_utf8 = 0;                        /* Is UTF8 mode active ? */

struct _vtG G[4] = {
    TX_ASCII,   TX_ASCII,  cs94, cs94, TRUE,  TRUE, TRUE, NULL, NULL, NULL, NULL, TRUE,
    TX_8859_1,  TX_8859_1, cs96, cs96, FALSE, TRUE, TRUE, NULL, NULL, NULL, NULL, TRUE,
    TX_8859_1,  TX_8859_1, cs96, cs96, FALSE, TRUE, TRUE, NULL, NULL, NULL, NULL, TRUE,
    TX_8859_1,  TX_8859_1, cs96, cs96, FALSE, TRUE, TRUE, NULL, NULL, NULL, NULL, TRUE
    };
struct _vtG *GL = &G[0], *SSGL = NULL;   /* GL and single shift GL */
struct _vtG *GR = &G[2];                 /* GR */
struct _vtG *GNOW = &G[0];
struct _vtG savedG[VNUM][4];
struct _vtG *savedGL[VNUM] = {NULL,NULL,NULL,NULL},
            *savedGR[VNUM] = {NULL,NULL,NULL,NULL},
            *savedSSGL[VNUM] = {NULL,NULL,NULL,NULL} ;
static int  Qsaved = FALSE;              /* QANSI charset shifts */
struct _vtG QsavedG[4],
            *QsavedGL = NULL,
            *QsavedGR = NULL,
            *QsavedSSGL = NULL;

bool     printregion    = FALSE; /* Print extent = full screen */
bool     xprintff       = FALSE; /* Print formfeed */
bool     turnonprinter  = FALSE; /* Time to turn on printer */
bool     xprint         = FALSE; /* Controller print in progress */
bool     aprint         = FALSE; /* Auto-print in progress */
bool     cprint         = FALSE; /* Copy-print in progress (for debug) */
bool     uprint         = FALSE; /* Transparent print */
bool     turnoffprinter = FALSE; /* Time to turn it off */

bool     wrapit    = FALSE;
bool     literal_ch= FALSE;
bool     screenon  = TRUE;
extern bool     cursorena[];       /* Cursor enabled / disabled */
extern bool     cursoron[] ;       /* Cursor state on/off       */
bool     relcursor = FALSE;
bool     keylock   = FALSE;
bool     vt52graphics = FALSE;

bool     saverelcursor[VNUM]={FALSE,FALSE,FALSE,FALSE},
         saved[VNUM]={FALSE,FALSE,FALSE,FALSE};
int      savedwrap[VNUM]={FALSE,FALSE,FALSE,FALSE} ;
int      savedrow[VNUM] = {0,0,0,0};
int      savedcol[VNUM] = {0,0,0,0};

bool     deccolm = FALSE;               /* 80/132-column mode */
bool     decscnm = FALSE;               /* Normal/reverse screen mode */
int      decscnm_usr = FALSE;           /* User default for decscnm */
bool     decnrcm = FALSE ;              /* National Replacement Charset mode */
int      decnrcm_usr = FALSE;           /* NRC mode - user default */
bool     decsasd = SASD_TERMINAL ;      /* Active Status Display */
int      decssdt = SSDT_INDICATOR ;     /* Status Display Type */
bool     deckbum = FALSE ;              /* Keyboard (Typewriter/DP) */
bool     decsace = FALSE;               /* DECSACE */
int      savdecbkm = 0 ;                /* User default Backspace Mode */
bool     erm = FALSE ;                  /* Erasure Mode  VT300 */
bool     crm = FALSE ;                  /* Control Mode  VT300 */
bool     decled[4] = {FALSE,FALSE,FALSE,FALSE} ; /* DEC LEDs */
bool     insertmode = FALSE;            /* Insert/replace mode */
bool     sco8bit = FALSE;               /* SCO ANSI 8th bit quoting */
bool     scoBCS2 = FALSE;               /* SCO ANSI BCS2 */
bool     scocompat = FALSE;             /* SCO ANSI Backward Compatibility Mode */
extern int wyse8bit;                    /* WYSE ASCII 8th bit quoting */
bool     keyclick  = FALSE ;
int      dec_upss = TX_8859_1 ;
int      dec_lang = VTL_NORTH_AM;       /* DEC VT Language = North American */
int      dec_nrc  = TX_ASCII;           /* DEC NRC for use during NRC Mode  */
int      dec_kbd  = TX_8859_1;          /* DEC Keyboard character set       */

/*
  Terminal parameters that can also be set externally by SET commands.
  Formerly they were declared and initialized here, and had different
  names, as shown in the comments.  Now they are declared and
  initialized in ckuus7.c.  - fdc
*/
int tt_kb_mode = KBM_EN ;               /* Keyboard is in English mode */
int tt_kb_glgr = FALSE;                 /* Kbd Follows GL/GR charset */
bool send_c1 = FALSE;                   /* Flag for sending C1 controls */
extern int send_c1_usr ;                /* User default for send_c1 */

/*
  VT220 and higher Pn's for terminal ID string are (* = Not supported):
     1 - 132 columns
     2 - Printer port
  *  3 - ReGIS graphics
  *  4 - Sixel graphics
     6 - DECSED - Selective erase
  *  7 - DRCS - Soft character sets
     8 - UDK - User-defined keys
     9 - National Replacement Character Sets can be designated by host
  * 12 - Serbo-Croation (SCS)
  * 13 - Local editing
    15 - Technical character set
  * 16 - Locator device port
  * 18 - Windowing Capability
  * 19 - Dual sessions
  * 21 - Horizontal Scrolling
    22 - Color
    23 - Greek
  * 24 - Turkish
    42 - ISO Latin-2
    44 - PC Term
    45 - Soft-key mapping
    46 - ASCII Terminal emulation
*/
struct tt_info_rec tt_info[] = {        /* Indexed by terminal type */
    "TTY", {NULL},                              "",                    /* Teletype */
    "D200", {"DG200","DATA-GENERAL-200",NULL},  "o#!J ",               /* Data General 200 */
    "D210", {"DG210","DATA-GENERAL-210",NULL},  "o#(HY",               /* Data General 210 */
    "D217", {"DG217","DATA-GENERAL-217",NULL},  "o#(HY",               /* Data General 217 */
    "HP2621", {"HP2621P","HP2621A","HP-2621",NULL},"2621A",            /* HP 2621A */
    "HPTERM", {"HP-TERM","X-HPTERM",NULL},      "X-hpterm",            /* HP TERM */
    "H1500", {"HZ1500","HAZELTINE-1500",NULL},  "",                    /* Hazeltine 1500 */
    "VC404", {"VOLKER-CRAIG-404",NULL},         "",                    /* Volker Craig VC4404/404 */
    "WY30", {"WYSE-30","WYSE30",NULL},          "30\r",                /* WYSE-30 */
    "WY50", {"WYSE-50","WYSE50",NULL},          "50\r",                /* WYSE-50 */
    "WY60", {"WYSE-60","WYSE60",NULL},          "60\r",                /* WYSE-60 */
    "WY160", {"WYSE-160","WYSE160",NULL},       "160\r",               /* WYSE-160 */
    "QNX",  {"QNX4",NULL},                      "",                    /* QNX */
    "QANSI", {NULL},                            "",                    /* QNX ANSI */
    "VT52",{"DEC-VT52",NULL},                   "/Z",                  /* DEC VT52 */
    "H19", {"HEATH","ZENITH","Z19",NULL},       "/K",                  /* Heath-19 */
    "IBM3151", {"I3151",NULL},                   "",                   /* IBM 3151 */
    "SCOANSI", {"SCO-ANSI","ANSI-850","ANSI-8859",NULL},    "",        /* SCO ANSI */
    "AT386",  {"386AT","ATT6386",NULL},         "",                    /* AT-386 */
    "ANSI",   {"DOSANSI","ANSI.SYS","PCANSI",NULL},"",                 /* "ANSI"  */
    "VIP7809", {NULL},                          "7813  P GC  A\003",   /* Honeywell MOD400 4.0 */
    "LINUX",  {NULL},                           "",                    /* Linux */
    "HFT", {"IBM8512","IBM8513",NULL},          "[?1;2c",              /* IBM HFT */
    "AIXTERM", {"AIXTERM-M",NULL},              "[?1;2c",              /* IBM AIXTERM */
    "SUN", {"SUN-COLOR",NULL},                  "",                    /* SUN Console */
    "BA80-08", {"BA80",NULL},                   "",                    /* Nixdorf BA80 */
    "BETERM",  {NULL},                          "",                    /* BEOS ANSI */
    "VT100", {"DEC-VT100","VT100-AM",NULL},     "[?1;2c",              /* DEC VT100 */
    "VT102", {"DEC-VT102",NULL},                "[?6c",                /* DEC VT102 */
    "VT220", {"DEC-VT220","DEC-VT200","VT200",NULL}, "[?62;1;2;6;8;9;15;44c",  /* DEC VT220 */
    "VT220PC", {"DEC-VT220-PC","DEC-VT200-PC","VT200PC",NULL}, "[?62;1;2;6;8;9;15;44c",  /* DEC VT220 w/ PC keyboard */
    "VT320", {"DEC-VT320","DEC-VT300","VT300",NULL}, "[?63;1;2;6;8;9;15;44c",  /* DEC VT320 */
    "VT320PC", {"DEC-VT320-PC","DEC-VT300-PC","VT300PC",NULL}, "[?63;1;2;6;8;9;15;44c",  /* DEC VT320 w/ PC keyboard */
    "WY370", {"WYSE-370","WYSE370","WY350",NULL},"[?63;1;2;6;8;9;15;44c",  /* WYSE 370 (same as VT320) */
    "97801", {"SNI-97801",NULL},                "[?62;1;2;6;8;9;15;44c",  /* Sinix 97801 */
    "AAA", { "ANNARBOR", "AMBASSADOR",NULL}, "11;00;00", /* Ann Arbor Ambassador */
#ifdef COMMENT
    "VT420", {"DEC-VT420","DEC-VT400","VT400",NULL},    "[?64;1;2;6;8;9;15;22;23;42;44;45;46c",       /* DEC VT420 */
    "VT525", {"DEC-VT525","DEC-VT500","VT500",NULL},    "[?65;1;2;6;8;9;15;22;23;42;44;45;46c",       /* DEC VT520 */
#endif /* COMMENT */
    "TVI910", {"TELEVIDEO-910","TVI910+""910",NULL},    "TVS 910 REV.I\r",        /* TVI 910+ */
    "TVI925", {"TELEVIDEO-925","925",NULL},     "TVS 925 REV.I\r",        /* TVI 925  */
    "TVI950", {"TELEVIDEO-950","950",NULL},     "1.0,0\r",                /* TVI 950  */
    "ADM3A",  {NULL}, "", /* LSI ADM 3A */
    "ADM5",   {NULL}, "", /* LSI ADM 5 */
    "VTNT",   {NULL},                           "",                       /* Microsoft NT VT */
    "IBM3101",{"I3101",NULL},   ""                       /* IBM 31xx */
};
int max_tt = TT_MAX;                    /* Highest terminal type */

/* SET TERMINAL values ... */

extern int tt_arrow;                    /* Arrow-key mode */
extern int tt_keypad;                   /* Keypad mode */
extern int tt_shift_keypad;             /* Keypad Shift mode */
extern int tt_wrap;                     /* Autowrap */
extern int tt_type;                     /* Terminal type */
extern int tt_cursor_usr;               /* Cursor type */
extern int tt_cursorena_usr;            /* Cursor enabled by user */
extern int tt_cursor;                   /* Active cursor mode */
extern int tt_answer;                   /* Answerback enabled/disabled */
extern int tt_scrsize[];                /* Scrollback buffer size */
extern int tt_roll[];                   /* Scrollback style */
extern int tt_rows[];                   /* Screen rows */
extern int tt_cols[];                   /* Screen columns */
       int tt_cols_usr = 80;            /* User default screen width */
int tt_szchng[VNUM] = {1,1,1,0}; /* Screen Size Changed */
extern int cmd_rows;                    /* Screen rows */
extern int cmd_cols;                    /* Screen columns */
extern int tt_ctstmo;                   /* CTS timeout */
extern int tt_pacing;                   /* Output-pacing */
extern int tt_mouse;                    /* Mouse */
extern int tt_updmode;                  /* Terminal Screen Update Mode */
extern int tt_url_hilite;
extern int tt_url_hilite_attr;
int tt_type_vt52 = TT_VT52 ;            /* Terminal Type Mode before entering VT52 mode */
int      holdscreen = FALSE ;

int      escstate = ES_NORMAL;
int      escnext = 1;
int      esclast = 0;

int marginbell = FALSE, marginbellcol = 72 ;

extern int autoscroll ;                 /* WYSE ASCII Term AutoScroll Mode */
extern int writeprotect ;               /* WYSE ASCII WriteProtect Mode */
extern int protect ;                    /* WYSE ASCII Protect Mode */
extern int wysegraphics ;               /* WYSE ASCII Graphics Mode */
extern vtattrib WPattrib ;              /* WYSE ASCII WriteProtect Mode Attrib */
extern vtattrib defWPattrib ;           /* WYSE ASCII default WP Mode Attrib */
extern int attrmode ;                   /* WYSE ASCII Attribute Mode */
extern int wy_keymode ;                 /* WYSE ASCII Key Appl Mode */
extern int wy_enhanced ;                /* WYSE ASCII Enhanced Mode */
extern int wy_widthclr ;                /* WYSE ASCII Width Clear Mode */
extern int wy_autopage ;                /* WYSE ASCII Auto Page Mode */
extern int wy_monitor ;                 /* WYSE ASCII Monitor Mode */
extern int wy_nullsuppress;             /* WYSE ASCII Null Suppression */
extern int tvi_ic ;                     /* TVI insert character */
extern int wy_block;                    /* TVI block mode (FALSE = Conversation mode) */
extern int hzgraphics;                  /* Hazeltine graphics mode */

int tt_sac = SP;                        /* Spacing Attribute Character */
int sni_pagemode = FALSE;               /* 97801 Page Mode */
int sni_pagemode_usr = FALSE;           /* 97801 Page Mode (User Default) */
int sni_scroll_mode = FALSE;            /* 97801 Roll (power-on) or Scroll */
int sni_scroll_mode_usr = FALSE;        /* 97801 Roll or Scroll (User Default) */
int sni_chcode = TRUE;                  /* 97801 CH.CODE mode */
int sni_chcode_usr = TRUE;              /* 97801 CH.CODE mode */
int sni_chcode_7 = TRUE;                /* 97801 CH.CODE key enabled 7-bit mode */
int sni_chcode_8 = TRUE;                /* 97801 CH.CODE key enabled 8-bit mode */
int sni_bitmode = 8;                    /* 97801 CH.CODE 8-bit mode */
CHAR sni_kbd_firmware[7]="920031";      /* 97801 Keyboard Firmware Version */
CHAR sni_term_firmware[7]="830851";     /* 97801 Terminal Firmware Version */

/* Escape-sequence processing buffer */

unsigned short escbuffer[ESCBUFLEN+1];

/*
   For pushing back input characters,
   e.g. converting 8-bit controls to 7-bit escape sequences.
*/
static int f_pushed = 0, c_pushed = 0, f_popped = 0;

int sgrcolors = TRUE;                   /* Process SGR Color Commands */

static
unsigned char sgrcols[8] = {
/* Black   */ 0,
/* Red     */ 4,
/* Green   */ 2,
/* Brown   */ 6,
/* Blue    */ 1,
/* Magenta */ 5,
/* Cyan    */ 3,
/* White   */ 7
};

static
unsigned char isocols[8] = {
/* Black   */ 0,
/* Blue    */ 1,
/* Green   */ 2,
/* Cyan    */ 3,
/* Red     */ 4,
/* Magenta */ 5,
/* Yellow  */ 6,
/* White   */ 7
};

/* Function prototypes */
#ifndef NOTTOCI
int ttoci(char c);
#endif /* NOTTOCI */

int status_saved = -1;

extern int os2_outesc ;                     /* Esc seq recognizer for keys... */

/* BA80 Function Key Label Reader */
static char ba80_fkey_buf[256]="", *ba80_fkey_ptr=NULL;
static int  ba80_fkey_read = 0;

/* VTNT variables */

#define VTNT_BUFSZ      36864
#define VTNT_MIN_READ   42
static char vtnt_buf[VTNT_BUFSZ]="";
static unsigned long  vtnt_index = 0;
static unsigned long  vtnt_read  = VTNT_MIN_READ;

/* DEC F-key strings */
char * fkeys[] = {
 "[11~","[12~","[13~","[14~","[15~",    /* F1  - F5 PC Keyboard */
 "[17~","[18~","[19~","[20~","[21~",    /* F6  - F10 */
 "[23~","[24~","[25~","[26~","[28~",    /* F11 - F15 */
 "[29~","[31~","[32~","[33~","[34~"     /* F16 - F20 */
};

/* DEC Editing key strings */
char * ekeys[] = {
    "[1~",      /* Find */
    "[2~",      /* Insert */
    "[3~",      /* Remove */
    "[4~",      /* Select */
    "[5~",      /* Previous Screen */
    "[6~",      /* Next Screen */
    "[H"        /* PC Home */
};

/* DEC User Defined Key strings */
/* Also used for SCO ANSI UDK strings */
/* There are 15 DEC strings, and 60 AT386 strings */
/* And 65 Wyse characters */
/* And close to 108 DG keys (regular and unix modes) */
/* 153 SNI 97801 keys */
char * udkfkeys[200];  /* let udkreset() initialize them */
int udklock = 0 ;      /* Are User Defined Keys locked? */

#ifndef NOKVERBS
extern struct keytab kverbs[] ;         /* Kverbs table  */
extern int nkverbs ;                    /* Num of Kverbs */

/* Compose-key support structures and tables */

struct compose_key_tab {
    CHAR c1;                            /* First char of compose sequence */
    CHAR c2;                            /* Second char of compose sequence */
    CHAR c3;                            /* Character that it maps to */
};

struct compose_key_tab l1ktab[] = {     /* The Latin-1 Compose Key Table */
    'N', 'B', (CHAR) 0xA0,		/* No-Break Space */
    'n', 'b', (CHAR) 0xA0,		/* No-Break Space */
    'A', '`', (CHAR) 0xC0,              /* A grave */
    '`', 'A', (CHAR) 0xC0,              /* A grave */
    'A', (CHAR) 0x27, (CHAR) 0xC1,      /* A acute */
    (CHAR) 0x27, 'A', (CHAR) 0xC1,      /* A acute */
    'A', '^', (CHAR) 0xC2,              /* A circumflex */
    '^', 'A', (CHAR) 0xC2,              /* A circumflex */
    'A', '~', (CHAR) 0xC3,              /* A tilde */
    '~', 'A', (CHAR) 0xC3,              /* A tilde */
    'A', '"', (CHAR) 0xC4,              /* A diaeresis */
    '"', 'A', (CHAR) 0xC4,              /* A diaeresis */
    'A', '*', (CHAR) 0xC5,              /* A ring above */
    '*', 'A', (CHAR) 0xC5,              /* A ring above */
    'A', 'E', (CHAR) 0xC6,              /* A with E */
    'C', ',', (CHAR) 0xC7,              /* C Cedilla */
    ',', 'C', (CHAR) 0xC7,              /* C Cedilla */
    'E', '`', (CHAR) 0xC8,              /* E grave */
    '`', 'E', (CHAR) 0xC8,              /* E grave */
    'E', (CHAR) 0x27, (CHAR) 0xC9,      /* E acute */
    (CHAR) 0x27, 'E', (CHAR) 0xC9,      /* E acute */
    'E', '^', (CHAR) 0xCA,              /* E circumflex */
    '^', 'E', (CHAR) 0xCA,              /* E circumflex */
    'E', '"', (CHAR) 0xCB,              /* E diaeresis */
    '"', 'E', (CHAR) 0xCB,              /* E diaeresis */
    'I', '`', (CHAR) 0xCC,              /* I grave */
    '`', 'I', (CHAR) 0xCC,              /* I grave */
    'I', (CHAR) 0x27, (CHAR) 0xCD,      /* I acute */
    (CHAR) 0x27, 'I', (CHAR) 0xCD,      /* I acute */
    'I', '^', (CHAR) 0xCE,              /* I circumflex */
    '^', 'I', (CHAR) 0xCE,              /* I circumflex */
    'I', '"', (CHAR) 0xCF,              /* I diaeresis */
    '"', 'I', (CHAR) 0xCF,              /* I diaeresis */
    '-', 'D', (CHAR) 0xD0,              /* Icelandic Eth */
    'D', '-', (CHAR) 0xD0,              /* Icelandic Eth */
    'N', '~', (CHAR) 0xD1,              /* N tilde */
    '~', 'N', (CHAR) 0xD1,              /* N tilde */
    'O', 'E', (CHAR) 0xD7,      /* OE digraph (not really in Latin-1) */
    'O', '`', (CHAR) 0xD2,              /* O grave */
    '`', 'O', (CHAR) 0xD2,              /* O grave */
    'O', (CHAR) 0x27, (CHAR) 0xD3,      /* O acute */
    (CHAR) 0x27, 'O', (CHAR) 0xD3,      /* O acute */
    'O', '^', (CHAR) 0xD4,              /* O circumflex */
    '^', 'O', (CHAR) 0xD4,              /* O circumflex */
    'O', '~', (CHAR) 0xD5,              /* O tilde */
    '~', 'O', (CHAR) 0xD5,              /* O tilde */
    'O', '"', (CHAR) 0xD6,              /* O diaeresis */
    '"', 'O', (CHAR) 0xD6,              /* O diaeresis */
    'O', '/', (CHAR) 0xD8,              /* O oblique stroke */
    '/', 'O', (CHAR) 0xD8,              /* O oblique stroke */
    'U', '`', (CHAR) 0xD9,              /* U grave */
    '`', 'U', (CHAR) 0xD9,              /* U grave */
    'U', (CHAR) 0x27, (CHAR) 0xDA,      /* U acute */
    (CHAR) 0x27, 'U', (CHAR) 0xDA,      /* U acute */
    'U', '^', (CHAR) 0xDB,              /* U circumflex */
    '^', 'U', (CHAR) 0xDB,              /* U circumflex */
    'U', '"', (CHAR) 0xDC,              /* U diaeresis */
    '"', 'U', (CHAR) 0xDC,              /* U diaeresis */
    'Y', (CHAR) 0x27, (CHAR) 0xDD,      /* Y acute */
    (CHAR) 0x27, 'Y', (CHAR) 0xDD,      /* Y acute */
    'H', 'T', (CHAR) 0xDE,              /* Icelandic Thorn */
    'T', 'H', (CHAR) 0xDE,              /* Icelandic Thorn */
    's', 's', (CHAR) 0xDF,              /* German scharfes s */
    'a', '`', (CHAR) 0xE0,              /* a grave */
    '`', 'a', (CHAR) 0xE0,              /* a grave */
    'a', (CHAR) 0x27, (CHAR) 0xE1,      /* a acute */
    (CHAR) 0x27, 'a', (CHAR) 0xE1,      /* a acute */
    'a', '^', (CHAR) 0xE2,              /* a circumflex */
    '^', 'a', (CHAR) 0xE2,              /* a circumflex */
    'a', '~', (CHAR) 0xE3,              /* a tilde */
    '~', 'a', (CHAR) 0xE3,              /* a tilde */
    'a', '"', (CHAR) 0xE4,              /* a diaeresis */
    '"', 'a', (CHAR) 0xE4,              /* a diaeresis */
    'a', '*', (CHAR) 0xE5,              /* a ring above */
    '*', 'a', (CHAR) 0xE5,              /* a ring above */
    'a', 'e', (CHAR) 0xE6,              /* a with e */
    'c', ',', (CHAR) 0xE7,              /* c cedilla */
    ',', 'c', (CHAR) 0xE7,              /* c cedilla */
    'e', '`', (CHAR) 0xE8,              /* e grave */
    '`', 'e', (CHAR) 0xE8,              /* e grave */
    'e', (CHAR) 0x27, (CHAR) 0xE9,      /* e acute */
    (CHAR) 0x27, 'e', (CHAR) 0xE9,      /* e acute */
    'e', '^', (CHAR) 0xEA,              /* e circumflex */
    '^', 'e', (CHAR) 0xEA,              /* e circumflex */
    'e', '"', (CHAR) 0xEB,              /* e diaeresis */
    '"', 'e', (CHAR) 0xEB,              /* e diaeresis */
    'i', '`', (CHAR) 0xEC,              /* i grave */
    '`', 'i', (CHAR) 0xEC,              /* i grave */
    'i', (CHAR) 0x27, (CHAR) 0xED,      /* i acute */
    (CHAR) 0x27, 'i', (CHAR) 0xED,      /* i acute */
    'i', '^', (CHAR) 0xEE,              /* i circumflex */
    '^', 'i', (CHAR) 0xEE,              /* i circumflex */
    'i', '"', (CHAR) 0xEF,              /* i diaeresis */
    '"', 'i', (CHAR) 0xEF,              /* i diaeresis */
    '-', 'd', (CHAR) 0xF0,              /* Icelandic eth */
    'd', '-', (CHAR) 0xF0,              /* Icelandic eth */
    'n', '~', (CHAR) 0xF1,              /* n tilde */
    '~', 'n', (CHAR) 0xF1,              /* n tilde */
    'o', 'e', (CHAR) 0xF7,      /* oe digraph (not really in Latin-1) */
    'o', '`', (CHAR) 0xF2,              /* o grave */
    '`', 'o', (CHAR) 0xF2,              /* o grave */
    'o', (CHAR) 0x27, (CHAR) 0xF3,      /* o acute */
    (CHAR) 0x27, 'o', (CHAR) 0xF3,      /* o acute */
    'o', '^', (CHAR) 0xF4,              /* o circumflex */
    '^', 'o', (CHAR) 0xF4,              /* o circumflex */
    'o', '~', (CHAR) 0xF5,              /* o tilde */
    '~', 'o', (CHAR) 0xF5,              /* o tilde */
    'o', '"', (CHAR) 0xF6,              /* o diaeresis */
    '"', 'o', (CHAR) 0xF6,              /* o diaeresis */
    '-', ':', (CHAR) 0xF7,              /* Division sign */
    'o', '/', (CHAR) 0xF8,              /* o oblique stroke */
    '/', 'o', (CHAR) 0xF8,              /* o oblique stroke */
    'u', '`', (CHAR) 0xF9,              /* u grave */
    '`', 'u', (CHAR) 0xF9,              /* u grave */
    'u', (CHAR) 0x27, (CHAR) 0xFA,      /* u acute */
    (CHAR) 0x27, 'u', (CHAR) 0xFA,      /* u acute */
    'u', '^', (CHAR) 0xFB,              /* u circumflex */
    '^', 'u', (CHAR) 0xFB,              /* u circumflex */
    'u', '"', (CHAR) 0xFC,              /* u diaeresis */
    '"', 'u', (CHAR) 0xFC,              /* u diaeresis */
    'y', (CHAR) 0x27, (CHAR) 0xFD,      /* y acute */
    (CHAR) 0x27, 'y', (CHAR) 0xFD,      /* y acute */
    'h', 't', (CHAR) 0xFE,              /* Icelandic thorn */
    't', 'h', (CHAR) 0xFE,              /* Icelandic thorn */
    'y', '"', (CHAR) 0xFF,              /* y diaeresis */
    '"', 'y', (CHAR) 0xFF,              /* y diaeresis */
    '+', '+', '#',                      /* Number sign */
    'A', 'A', '@',                      /* At sign */
    'a', 'a', '@',                      /* At sign */
    '(', '(', '[',                      /* Left bracket */
    '/', '/', '\\',                     /* Backslash */
    '/', '<', '\\',                     /* Backslash */
    ')', ')', ']',                      /* Right bracket */
    '(', '-', '{',                      /* Left brace */
    '/', '^', '|',                      /* Vertical bar */
    ')', '-', '}',                      /* Right brace */
    '!', '!', (CHAR) 0xA1,              /* Inverted exclamation */
    'C', '/', (CHAR) 0xA2,              /* Cent sign */
    'c', '/', (CHAR) 0xA2,              /* Cent sign */
    'L', '-', (CHAR) 0xA3,              /* Pound sterling sign */
    'l', '-', (CHAR) 0xA3,              /* Pound sterling sign */
    'Y', '-', (CHAR) 0xA5,              /* Yen sign */
    'y', '-', (CHAR) 0xA5,              /* Yen sign */
    '|', '|', (CHAR) 0xA6,              /* Broken bar */
    'S', 'O', (CHAR) 0xA7,              /* Paragraph sign */
    'S', '!', (CHAR) 0xA7,              /* Paragraph sign */
    'S', '0', (CHAR) 0xA7,              /* Paragraph sign */
    's', 'o', (CHAR) 0xA7,              /* Paragraph sign */
    's', '!', (CHAR) 0xA7,              /* Paragraph sign */
    's', '0', (CHAR) 0xA7,              /* Paragraph sign */
    'X', 'O', (CHAR) 0xA4,              /* Currency sign */
    'X', '0', (CHAR) 0xA4,              /* Currency sign */
    'x', 'o', (CHAR) 0xA4,              /* Currency sign */
    'x', '0', (CHAR) 0xA4,              /* Currency sign */
    'C', 'O', (CHAR) 0xA9,              /* Copyright sign */
    'C', '0', (CHAR) 0xA9,              /* Copyright sign */
    'c', 'o', (CHAR) 0xA9,              /* Copyright sign */
    'c', '0', (CHAR) 0xA9,              /* Copyright sign */
    'A', '_', (CHAR) 0xAA,              /* Feminine ordinal */
    'a', '_', (CHAR) 0xAA,              /* Feminine ordinal */
    '<', '<', (CHAR) 0xAB,              /* Left angle quotation */
    '-', ',', (CHAR) 0xAC,              /* Not sign */
    '-', '-', (CHAR) 0xAD,              /* Soft hyphen */
    'R', 'O', (CHAR) 0xAE,              /* Registered Trademark */
    'r', 'o', (CHAR) 0xAE,              /* Registered Trademark */
    '-', '^', (CHAR) 0xAF,              /* Macron */
    '0', '^', (CHAR) 0xB0,              /* Degree sign */
    '+', '-', (CHAR) 0xB1,              /* Plus or minus sign */
    '2', '^', (CHAR) 0xB2,              /* Superscript 2 */
    '3', '^', (CHAR) 0xB3,              /* Superscript 3 */
    (CHAR) 0x27, (CHAR) 0x27, (CHAR) 0xB4, /* Acute accent */
    '/', 'U', (CHAR) 0xB5,              /* Micro sign */
    'U', '/', (CHAR) 0xB5,              /* Micro sign */
    '/', 'u', (CHAR) 0xB5,              /* Micro sign */
    'u', '/', (CHAR) 0xB5,              /* Micro sign */
    'P', '!', (CHAR) 0xB6,              /* Pilcrow sign */
    'p', '!', (CHAR) 0xB6,              /* Pilcrow sign */
    '.', '^', (CHAR) 0xB7,              /* Middle dot */
    ',', ',', (CHAR) 0xB8,              /* Cedilla */
    '1', '^', (CHAR) 0xB9,              /* Superscript 1 */
    'O', '_', (CHAR) 0xBA,              /* Masculine ordinal */
    'o', '_', (CHAR) 0xBA,              /* Masculine ordinal */
    '>', '>', (CHAR) 0xBB,              /* Right angle quotation */
    '1', '4', (CHAR) 0xBC,              /* Fraction one quarter */
    '1', '2', (CHAR) 0xBD,              /* Fraction one half */
    '?', '?', (CHAR) 0xBF,              /* Inverted question mark */
    'x', 'x', (CHAR) 0xD7,              /* Multiplication sign */
    'X', 'X', (CHAR) 0xD7,              /* Multiplication sign */
    /*
    The following group is for entering ASCII characters that are otherwise
    impossible to enter, e.g. because they are dead-key combinations.
    */
    (CHAR)0x27, '.' ,(CHAR) 0x27,       /* Apostrophe */
    '.', (CHAR)0x27, (CHAR) 0x27,       /* Apostrophe */
    '^', '^', '^',                      /* Circumflex or caret */
    '`', '`', '`',                      /* Grave accent or backquote */
    '~', '~', '~',                      /* Tilde */
    ',', '.', ',',                      /* Comma */
    '.', ',', ',',                      /* Comma */
    '"', '"', '"',                      /* Doublequote */
    ':', ':', (CHAR) 0xA8               /* Diaeresis */

};
int nl1ktab = (sizeof(l1ktab) / sizeof(struct compose_key_tab));

struct compose_key_tab l2ktab[] = {     /* The Latin-2 Compose Key Table */
    'N', 'B', (CHAR) 0xA0,		/* No-Break Space */
    'A', C_OGONEK, (CHAR) 0xA1,         /* A ogonek */
    C_OGONEK, 'A', (CHAR) 0xA1,         /* A ogonek */
    C_BREVE, C_BREVE, (CHAR) 0xF4,      /* Breve */
    'L', '/', (CHAR) 0xA3,              /* L with stroke */
    '/', 'L', (CHAR) 0xA3,              /* L with stroke */
    'X', 'O', (CHAR) 0xA4,              /* Currency sign */
    'X', '0', (CHAR) 0xA4,              /* Currency sign */
    'x', 'o', (CHAR) 0xA4,              /* Currency sign */
    'x', '0', (CHAR) 0xA4,              /* Currency sign */
    'L', C_CARON, 0xA5,                 /* L caron */
    C_CARON, 'L', 0xA5,                 /* L caron */
    'S', C_ACUTE, (CHAR) 0xA6,          /* S acute */
    C_ACUTE, 'S', (CHAR) 0xA6,          /* S acute */
    'S', 'O', (CHAR) 0xA7,              /* Paragraph sign */
    'S', '!', (CHAR) 0xA7,              /* Paragraph sign */
    'S', '0', (CHAR) 0xA7,              /* Paragraph sign */
    's', 'o', (CHAR) 0xA7,              /* Paragraph sign */
    's', '!', (CHAR) 0xA7,              /* Paragraph sign */
    's', '0', (CHAR) 0xA7,              /* Paragraph sign */
    ':', ':', (CHAR) 0xA8,              /* Diaeresis */
    'S', C_CARON, (CHAR) 0xA9,          /* S caron */
    C_CARON, 'S', (CHAR) 0xA9,          /* S caron */
    'S', C_CEDILLA, (CHAR) 0xAA,        /* S cedilla */
    C_CEDILLA, 'S', (CHAR) 0xAA,        /* S cedilla */
    'T', C_CARON, (CHAR) 0xAB,          /* T caron */
    C_CARON, 'T', (CHAR) 0xAB,          /* T caron */
    'Z', C_ACUTE, (CHAR) 0xAC,          /* Z acute */
    C_ACUTE, 'Z', (CHAR) 0xAC,          /* Z acute */
    '-', '-', (CHAR) 0xAD,              /* Soft hyphen */
    'Z', C_CARON, (CHAR) 0xAE,          /* Z caron */
    C_CARON, 'Z', (CHAR) 0xAE,          /* Z caron */
    'Z', C_DOT, (CHAR) 0xAF,            /* Z dot above */
    C_DOT, 'Z', (CHAR) 0xAF,            /* Z dot above */
    '0', '^', (CHAR) 0xB0,              /* Degree sign */
    'a', C_OGONEK, (CHAR) 0xB1,         /* a ogonek */
    C_OGONEK, 'a', (CHAR) 0xB1,         /* a ogonek */
    C_OGONEK, C_OGONEK, (CHAR) 0xB2,    /* Ogonek */
    'l', '/', (CHAR) 0xB3,              /* l with stroke */
    '/', 'l',(CHAR) 0xB3,               /* l with stroke */
    C_ACUTE, C_ACUTE, (CHAR) 0xB4,      /* Acute accent */
    'l', C_CARON, (CHAR) 0xB5,          /* l caron */
    C_CARON, 'l', (CHAR) 0xB5,          /* l caron */
    's', C_ACUTE, (CHAR) 0xB6,          /* s acute */
    C_ACUTE, 's', (CHAR) 0xB6,          /* s acute */
    C_CARON, C_CARON, (CHAR) 0xB7,      /* Caron */
    C_CEDILLA, C_CEDILLA, (CHAR) 0xB8,  /* Cedilla */
    's', C_CARON, (CHAR) 0xB9,          /* s caron */
    C_CARON, 's', (CHAR) 0xB9,          /* s caron */
    's', C_CEDILLA, (CHAR) 0xBA,        /* s cedilla */
    C_CEDILLA, 's', (CHAR) 0xBA,        /* s cedilla */
    't', C_CARON, (CHAR) 0xBB,          /* t caron */
    C_CARON, 't', (CHAR) 0xBB,          /* t caron */
    'z', C_ACUTE, (CHAR) 0xBC,          /* z acute */
    C_ACUTE, 'z', (CHAR) 0xBC,          /* z acute */
    C_DACUTE, C_DACUTE, (CHAR) 0xBD,    /* Double acute */
    'z', C_CARON, (CHAR) 0xBE,          /* z caron */
    C_CARON, 'z', (CHAR) 0xBE,          /* z caron */
    'z', C_DOT, (CHAR) 0xBF,            /* z dot above */
    C_DOT, 'z', (CHAR) 0xBF,            /* z dot above */
    'R', C_ACUTE, (CHAR) 0xC0,          /* R acute */
    C_ACUTE, 'R', (CHAR) 0xC0,          /* R acute */
    'A', C_ACUTE, (CHAR) 0xC1,          /* A acute */
    C_ACUTE, 'A', (CHAR) 0xC1,          /* A acute */
    'A', '^', (CHAR) 0xC2,              /* A circumflex */
    '^', 'A', (CHAR) 0xC2,              /* A circumflex */
    'A', C_BREVE, (CHAR) 0xC3,          /* A breve */
    C_BREVE, 'A', (CHAR) 0xC3,          /* A breve */
    'A', '"', (CHAR) 0xC4,              /* A diaeresis */
    '"', 'A', (CHAR) 0xC4,              /* A diaeresis */
    'L', C_ACUTE, (CHAR) 0xC5,          /* L acute */
    C_ACUTE, 'L', (CHAR) 0xC5,          /* L acute */
    'C', C_ACUTE, (CHAR) 0xC6,          /* C acute */
    C_ACUTE, 'C', (CHAR) 0xC6,          /* C acute */
    'C', ',', (CHAR) 0xC7,              /* C cedilla */
    ',', 'C', (CHAR) 0xC7,              /* C cedilla */
    'C', C_CARON, (CHAR) 0xC8,          /* C caron */
    C_CARON, 'C', (CHAR) 0xC8,          /* C caron */
    'E', C_ACUTE, (CHAR) 0xC9,          /* E acute */
    C_ACUTE, 'E', (CHAR) 0xC9,          /* E acute */
    'E', C_OGONEK, (CHAR) 0xCA,         /* E ogonek */
    C_OGONEK, 'E', (CHAR) 0xCA,         /* E ogonek */
    'E', '"', (CHAR) 0xCB,              /* E diaeresis */
    '"', 'E', (CHAR) 0xCB,              /* E diaeresis */
    'E', C_CARON, (CHAR) 0xCC,          /* E caron */
    C_CARON, 'E', (CHAR) 0xCC,          /* E caron */
    'I', C_ACUTE, (CHAR) 0xCD,          /* I acute */
    C_ACUTE, 'I', (CHAR) 0xCD,          /* I acute */
    'I', '^', (CHAR) 0xCE,              /* I circumflex */
    '^', 'I', (CHAR) 0xCE,              /* I circumflex */
    C_CARON, 'D', (CHAR) 0xCF,          /* D caron */
    'D', C_CARON, (CHAR) 0xCF,          /* D caron */
    '-', 'D', (CHAR) 0xD0,              /* D stroke */
    'D', '-', (CHAR) 0xD0,              /* D stroke */
    'N', C_ACUTE, (CHAR) 0xD1,          /* N acute */
    C_ACUTE, 'N', (CHAR) 0xD1,          /* N acute */
    'N', C_CARON, (CHAR) 0xD2,          /* N caron */
    C_CARON, 'N', (CHAR) 0xD2,          /* N caron */
    'O', C_ACUTE, (CHAR) 0xD3,          /* O acute */
    C_ACUTE, 'O', (CHAR) 0xD3,          /* O acute */
    'O', '^', (CHAR) 0xD4,              /* O circumflex */
    '^', 'O', (CHAR) 0xD4,              /* O circumflex */
    'O', C_DACUTE, (CHAR) 0xD5,         /* O double acute */
    C_DACUTE, 'O', (CHAR) 0xD5,         /* O double acute */
    'O', '"', (CHAR) 0xD6,              /* O diaeresis */
    '"', 'O', (CHAR) 0xD6,              /* O diaeresis */
    'x', 'x', (CHAR) 0xD7,              /* Multiplication sign */
    'X', 'X', (CHAR) 0xD7,              /* Multiplication sign */
    'R', C_CARON, (CHAR) 0xD8,          /* R caron */
    C_CARON, 'R', (CHAR) 0xD8,          /* R caron */
    'U', C_RING, (CHAR) 0xD9,           /* U ring */
    C_RING, 'U', (CHAR) 0xD9,           /* U ring */
    'U', C_ACUTE, (CHAR) 0xDA,          /* U acute */
    C_ACUTE, 'U', (CHAR) 0xDA,          /* U acute */
    'U', C_DACUTE, (CHAR) 0xDB,         /* U double acute */
    C_DACUTE, 'U', (CHAR) 0xDB,         /* U double acute */
    'U', '"', (CHAR) 0xDC,              /* U diaeresis */
    '"', 'U', (CHAR) 0xDC,              /* U diaeresis */
    'Y', C_ACUTE, (CHAR) 0xDD,          /* Y acute */
    C_ACUTE, 'Y', (CHAR) 0xDD,          /* Y acute */
    'T', ',', (CHAR) 0xDE,              /* T cedilla */
    ',', 'T', (CHAR) 0xDE,              /* T cedilla */
    's', 's', (CHAR) 0xDF,              /* German sharp s */
    'r', C_ACUTE, (CHAR) 0xE0,          /* r acute */
    C_ACUTE, 'r', (CHAR) 0xE0,          /* r acute */
    'a', C_ACUTE, (CHAR) 0xE1,          /* a acute */
    C_ACUTE, 'a', (CHAR) 0xE1,          /* a acute */
    'a', '^', (CHAR) 0xE1,              /* a circumflex */
    '^', 'a', (CHAR) 0xE1,              /* a circumflex */
    'a', C_BREVE, (CHAR) 0xE3,          /* a breve */
    C_BREVE, 'a', (CHAR) 0xE3,          /* a breve */
    'a', '"', (CHAR) 0xE4,              /* a diaeresis */
    '"', 'a', (CHAR) 0xE4,              /* a diaeresis */
    'l', C_ACUTE, (CHAR) 0xE5,          /* l acute */
    C_ACUTE, 'l', (CHAR) 0xE5,          /* l acute */
    'c', C_ACUTE, (CHAR) 0xE6,          /* c acute */
    C_ACUTE, 'c', (CHAR) 0xE6,          /* c acute */
    'c', ',', (CHAR) 0xE7,              /* c cedilla */
    ',', 'c', (CHAR) 0xE7,              /* c cedilla */
    'c', C_CARON, (CHAR) 0xE8,          /* c caron */
    C_CARON, 'c', (CHAR) 0xE8,          /* c caron */
    'e', C_ACUTE, (CHAR) 0xE9,          /* e acute */
    C_ACUTE, 'e', (CHAR) 0xE9,          /* e acute */
    'e', C_OGONEK, (CHAR) 0xEA,         /* e ogonek */
    C_OGONEK, 'e', (CHAR) 0xEA,         /* e ogonek */
    'e', '"', (CHAR) 0xEB,              /* e diaeresis */
    '"', 'e', (CHAR) 0xEB,              /* e diaeresis */
    'e', C_CARON, (CHAR) 0xEC,          /* e caron */
    C_CARON, 'e', (CHAR) 0xEC,          /* e caron */
    'i', C_ACUTE, (CHAR) 0xED,          /* i acute */
    C_ACUTE, 'i', (CHAR) 0xED,          /* i acute */
    'i', '^', (CHAR) 0xEE,              /* i circumflex */
    '^', 'i', (CHAR) 0xEE,              /* i circumflex */
    'd', C_CARON, (CHAR) 0xEF,          /* d caron */
    C_CARON, 'd', (CHAR) 0xEF,          /* d caron */
    '-', 'd', (CHAR) 0xF0,              /* d stroke */
    'd', '-', (CHAR) 0xF0,              /* d stroke */
    'n', C_ACUTE, (CHAR) 0xF1,          /* n acute */
    C_ACUTE, 'n', (CHAR) 0xF1,          /* n acute */
    'n', C_CARON, (CHAR) 0xF2,          /* n caron */
    C_CARON, 'n', (CHAR) 0xF2,          /* n caron */
    'o', C_ACUTE, (CHAR) 0xF3,          /* o acute */
    C_ACUTE, 'o', (CHAR) 0xF3,          /* o acute */
    'o', '^', (CHAR) 0xF4,              /* o circumflex */
    '^', 'o', (CHAR) 0xF4,              /* o circumflex */
    'o', C_DACUTE, (CHAR) 0xF5,         /* o double acute */
    C_DACUTE, 'o', (CHAR) 0xF5,         /* o double acute */
    'o', '"', (CHAR) 0xF6,              /* o diaeresis */
    '"', 'o', (CHAR) 0xF6,              /* o diaeresis */
    '-', ':', (CHAR) 0xF7,              /* Division sign */
    'r', C_CARON, (CHAR) 0xF8,          /* r caron */
    C_CARON, 'r', (CHAR) 0xF8,          /* r caron */
    'u', C_RING, (CHAR) 0xF9,           /* u ring */
    C_RING, 'u', (CHAR) 0xF9,           /* u ring */
    'u', C_ACUTE, (CHAR) 0xFA,          /* u acute */
    C_ACUTE, 'u', (CHAR) 0xFA,          /* u acute */
    'u', C_DACUTE, (CHAR) 0xFB,         /* u double acute */
    C_DACUTE, 'u', (CHAR) 0xFB,         /* u double acute */
    'u', '"', (CHAR) 0xFC,              /* u diaeresis */
    '"', 'u', (CHAR) 0xFC,              /* u diaeresis */
    'y', C_ACUTE, (CHAR) 0xFD,          /* y acute */
    C_ACUTE, 'y', (CHAR) 0xFD,          /* y acute */
    't', ',', (CHAR) 0xFE,              /* t cedilla */
    ',', 't', (CHAR) 0xFE,              /* t cedilla */
    C_DOT, C_DOT, (CHAR) 0xFF,          /* Dot above */
    /*
    The following group is for entering ASCII characters that are otherwise
    impossible to enter, e.g. because they are dead-key combinations.
    */
    '+', '+', '#',                      /* Number sign */
    'A', 'A', '@',                      /* At sign */
    'a', 'a', '@',                      /* At sign */
    '(', '(', '[',                      /* Left bracket */
    '/', '/', '\\',                     /* Backslash */
    '/', '<', '\\',                     /* Backslash */
    ')', ')', ']',                      /* Right bracket */
    '(', '-', '{',                      /* Left brace */
    '/', '^', '|',                      /* Vertical bar */
    ')', '-', '}',                      /* Right brace */
    (CHAR)0x27, '.', C_ACUTE,           /* Apostrophe */
    '.', (CHAR)0x27, C_ACUTE,           /* Apostrophe */
    '^', '^', '^',                      /* Circumflex or caret */
    '`', '`', '`',                      /* Grave accent or backquote */
    '~', '~', '~',                      /* Tilde */
    ',', '.', ',',                      /* Comma */
    '.', ',', ',',                      /* Comma */
    '"', '"', '"',                      /* Doublequote */
    ':', ':', (CHAR) 0xA8               /* Diaeresis */
};
int nl2ktab = (sizeof(l2ktab) / sizeof(struct compose_key_tab));

vik_rec vik;
#endif /* NOKVERBS */


USHORT
xldecgrph( CHAR c ) {
#ifdef COMMENT
    if ( isunicode() ) {
        return(c);
    }
    else {
        static unsigned char graphicset[32] = { /* VT100/200 graphic characters */
            0x20,
            0x04, 0xB0, 0x1A, 0x17, 0x1B, 0x19, 0xF8, 0xF1,
            0x15, 0x12, 0xD9, 0xBF, 0xDA, 0xC0, 0xC5, 0xC4,
            0xC4, 0xC4, 0xC4, 0xC4, 0xC3, 0xB4, 0xC1, 0xC2,
            0xB3, 0xF3, 0xF2, 0xE3, 0x9D, 0x9C, 0xFA
        };
        return graphicset[c>127 ? c-222 : c-95] ;
    }
#else /* COMMENT */
    USHORT cx = 0;
    if ( isunicode() ) {
        cx = (*xl_u[TX_DECSPEC])(c);
        if ( win95hsl && cx >= 0x23BA && cx <= 0x23BD )
            cx = tx_hslsub(cx);
        else if ( cx >= 0xE000 && cx <= 0xF8FF )
            cx = tx_usub(cx);
        if (win95lucida && cx > 0x017f)
            cx = tx_lucidasub(cx);
    }
    else {
        cx = (*xl_u[TX_DECSPEC])(c);
        if ( GNOW->itol )
            cx = (*GNOW->itol)(cx);
    }
    return(cx);
#endif /* COMMENT */
}

USHORT
xldectech( CHAR c ) {
    USHORT cx = 0;
    if ( isunicode() ) {
        cx = (*xl_u[TX_DECTECH])(c);
        if ( win95hsl && cx >= 0x23BA && cx <= 0x23BD )
            cx = tx_hslsub(cx);
        else if ( cx >= 0xE000 && cx <= 0xF8FF )
            cx = tx_usub(cx);
        if (win95lucida && cx > 0x017f)
            cx = tx_lucidasub(cx);
    }
    else {
#ifdef COMMENT
    static unsigned char graphicset[94] = { /* VT200 technical characters */
             '?', '?', '?', '?', '?', '?', '?',
        '?', '?', '?', '?', '?', '?', '?', '?',
        '?', '?', '?', '?', '?', '?', '?', '?',
        '?', ' ', ' ', ' ', '?', '?', '?', '?',
        '?', '?', '?', '?', '?', '?', '?', '?',
        '?', '?', '?', '?', '?', '?', '?', '?',
        '?', '?', ' ', '?', ' ', ' ', '?', '?',
        '?', '?', '?', '?', '?', '?', '?', '?',
        '?', '?', '?', '?', '?', '?', '?', '?',
        '?', '?', '?', '?', '?', '?', '?', '?',
        '?', '?', '?', '?', '?', '?', '?', '?',
        '?', '?', '?', '?', '?', '?', '?'
    };
        cx = graphicset[c>127 ? c-160 : c-33] ;
#else /* COMMENT */
        cx = (*xl_u[TX_DECTECH])(c);
        if ( GNOW->itol )
            cx = (*GNOW->itol)(cx);
#endif /* COMMENT */
    }
    return(cx);
}

USHORT
xlh19grph( CHAR c ) {
    USHORT cx=0;
    if ( isunicode() ) {
        cx = (*xl_u[TX_H19GRAPH])(c);
        if ( win95hsl && cx >= 0x23BA && cx <= 0x23BD )
            cx = tx_hslsub(cx);
        else if ( cx >= 0xE000 && cx <= 0xF8FF )
            cx = tx_usub(cx);
        if (win95lucida && cx > 0x017f)
            cx = tx_lucidasub(cx);
    }
    else {
#ifdef COMMENT
    static unsigned char graphicset[33] = { /* Heath-19 graphic characters */
        0xFA, 0xDD, 0x5C/*0xB3*/, 0xC4, 0xC5,
        0xBF, 0xD9, 0xC0, 0xDA, 0xF1,
        0x1A, 0xDB, 0xFE, 0x19, 0xDC,
        0xDC, 0xDF, 0xDF, 0xDF, 0xDE,
        0x2F/*0xDE*/, 0xC2, 0xB4, 0xC1, 0xC3,
        0x58, 0x2F, 0x5C, 0x16, 0x5F,
        0xB3, 0xB3, 0X14
    };
        cx =  graphicset[c>127 ? c-211 : c-94] ;
#else
        cx = (*xl_u[TX_H19GRAPH])(c);
        if ( GNOW->itol )
            cx = (*GNOW->itol)(cx);
#endif /* COMMENT */
    }
    return(cx);
}

#ifdef CKLEARN
ULONG learnt1;
char learnbuf[LEARNBUFSIZ] = { NUL, NUL };
int  learnbc = 0;
int  learnbp = 0;
int  learnst = 0;

VOID
learnreset()
{
    learnbuf[0] = NUL;
    learnbc = 0;
    learnbp = 0;
    learnst = 0;
}

VOID
learnnet(int c)
{
    if (learning && learnst != LEARN_KEYBOARD) {
        learnbuf[learnbp++] = c;
        if ( learnbp >= LEARNBUFSIZ )
            learnbp = 0;
        learnbc++;
        learnst = LEARN_NET;
    }
}

VOID
learnkeyb(con_event evt, int state) {   /* Learned script keyboard character */
    char buf[64];
    char * ptr=NULL;
    int cc;

    if (!learning || !learnfp)
      return;

    switch (evt.type) {
    case key:
        cc = evt.key.scancode;
        if (cc == 0) {
            ptr = "\\N";
        } else if (cc < SP || cc > 126 && cc < 160) {
            ckmakmsg(buf,64,"\\{",ckitoa((int)cc),"}",NULL);
            ptr = buf;
        } else {
            buf[0] = evt.key.scancode ;
            buf[1] = '\0';
            ptr = buf;
        }
        break;

#ifndef NOKVERBS
    case kverb: {
        extern struct keytab kverbs[];
        extern int nkverbs;
        int i;

        for ( i=0; i<nkverbs; i++ ) {
            if ( kverbs[i].kwval == evt.kverb.id ) {
                ckmakmsg(buf,64,"\\K{",kverbs[i].kwd,"}",NULL);
                ptr = buf;
            }
        }
        break;
    }
#endif /* NOKVERBS */
    case macro:
        ptr = evt.macro.string;
        break;

    case literal:
        ptr = evt.literal.string;
        break;

    case esc:
        buf[0] = ESC;
        buf[1] = evt.esc.key & ~F_ESC ;
        buf[2] = '\0' ;
        ptr = buf;
        break;

    case csi:
        buf[0] = ESC;
        buf[1] = '[' ;
        buf[2] = evt.csi.key & ~F_CSI ;
        buf[3] = '\0' ;
        ptr = buf;
        break;
    }

    switch (learnst) {                  /* Learn state... */
      case LEARN_NEUTRAL:
      case LEARN_NET:
        if (learnbc > 0) {              /* Have net characters? */
            char nbuf[LEARNBUFSIZ];
            int i, j, n;
            ULONG t;

            t = (ULONG) time(0);        /* Calculate INPUT timeout */
            j = t - learnt1;
            j += (j / 4) > 0 ? (j / 4) : 1; /* Add some slop */
            if (j < 2) j = 2;               /* 2 seconds minimum */

            fputs("\r\nINPUT ",learnfp);/* Give INPUT command for them */
            fputs(ckitoa(j),learnfp);
            fputs(" {",learnfp);
            learnt1 = t;

            n = LEARNBUFSIZ;
            if (learnbc < LEARNBUFSIZ) {  /* Circular buffer */
                n = learnbc;              /*  hasn't wrapped yet. */
                learnbp = 0;
            }
            j = 0;                      /* Copy to linear buffer */
            for (i = 0; i < n; i++) {   /* Number of chars in circular buf */

                cc = learnbuf[(learnbp + i) % LEARNBUFSIZ];

                /* Later account for prompts that end with a newline? */

                if (cc == CR && j > 0) {
                    if (nbuf[j-1] != LF)
                      j = 0;
                }
                nbuf[j++] = cc;
            }
            for (i = 0; i < j; i++) {   /* Now copy out the buffer */
                cc = nbuf[i];           /* interpreting control chars */
                if (cc == 0) {          /* We don't INPUT NULs */
                    continue;
                } else if (cc < SP ||   /* Controls need quoting */
                           cc > 126 && cc < 160) {
                    ckmakmsg(buf,64,"\\{",ckitoa((int)cc),"}",NULL);
                    fputs(buf,learnfp);
                } else {                /* Plain character */
                    putc(cc,learnfp);
                }
            }
            fputs("}\nIF FAIL STOP 1 INPUT timeout",learnfp);
            learnbc = 0;
        }
        learnbp = 0;
        fputs("\nPAUSE 1\nOUTPUT ",learnfp); /* Emit OUTPUT and fall thru */

      case LEARN_KEYBOARD:
        if ( ptr )
            fputs(ptr,learnfp);
    }
    learnst = state;
}
#endif /* CKLEARN */
#endif /* NOLOCAL */

/* ------------------------------------------------------------------ */
/* Printer functions                                                  */
/* ------------------------------------------------------------------ */

/* Text2PS */

/*
  Postscript Prolog, to be inserted at the beginning of the output file.
  The %% Comments are to make the file conformant with Adobe's "Postscript
  File Structuring Conventions", which allow page-oriented operations in
  Postscript previewers, page pickers, etc.
*/
char *prolog[] = {                      /* Standard prolog */
    "%!PS-Adobe-1.0",                   /* Works with Postscript 1.0 */
    "%%Title: oofa",
    "%%DocumentFonts: Courier CourierLatin1",
    "%%Creator: K95",
    "%%Pages: (atend)",
    "%%EndComments",
/*
  Postscript font reencoding.  The standard encoding does not have the
  characters needed for Latin-1.

  Unfortunately, the font reencoding methods listed in the Postscript
  Cookbook simply do not work with the Apple Laserwriter (even though they
  did work with other Postscript devices).  The method described in the
  Adobe PostScript Language Reference Manual (2nd Ed) to change from the
  StandardEncoding vector to the ISOLatin1Encoding vector works only with
  the LaserWriter-II, but not older LaserWriters.

  This method, suggested by Bur Davis at Adobe, works around the fact that
  Courier was a "stroke font" in pre-version-47.0 Postscript, in which many of
  the accented characters are composed from other characters (e.g. i-grave =
  dotless i + grave).  It is probably not the most efficient possible solution
  (an iterative method might be better), but it works.
*/
    "/CourierLatin1 /Courier findfont dup dup maxlength dict begin",
    "{",
    "    1 index /FID ne { def } { pop pop } ifelse",
    "} forall",
    "/Encoding exch 1 index get 256 array copy def",
/*
  The following characters are added at the C1 positions 128-153, for printing
  non-Latin1 character sets such as IBM code pages, DEC MCS, NeXT, etc.  Note
  that we cannot use characters from the Symbol font.  Characters from
  different fonts cannot be mixed.  Anyway, the Symbol font is not fixed-width.
*/
    "Encoding 128 /quotesingle put",
    "Encoding 129 /quotedblleft put",
    "Encoding 131 /fi put",
    "Encoding 132 /endash put",
    "Encoding 133 /dagger put",
    "Encoding 134 /periodcentered put",
    "Encoding 135 /bullet put",
    "Encoding 136 /quotesinglbase put",
    "Encoding 137 /quotedblbase put",
    "Encoding 138 /quotedblright put",
    "Encoding 139 /ellipsis put",
    "Encoding 140 /perthousand put",
    "Encoding 141 /dotaccent put",
    "Encoding 142 /hungarumlaut put",
    "Encoding 143 /ogonek put",
    "Encoding 144 /caron put",
    "Encoding 145 /fl put",
    "Encoding 146 /emdash put",
    "Encoding 147 /Lslash put",
    "Encoding 148 /OE put",
    "Encoding 149 /lslash put",
    "Encoding 150 /oe put",
    "Encoding 151 /florin put",
    "Encoding 152 /fraction put",
    "Encoding 153 /daggerdbl put",
/*
  The following six characters are required for pre-47.0 PostScript versions,
  which compose accented Courier characters by putting together the base
  character and the accent.
*/
    "Encoding 154 /dotlessi put",
    "Encoding 155 /grave put",
    "Encoding 156 /circumflex put",
    "Encoding 157 /tilde put",
    "Encoding 158 /breve put",
    "Encoding 159 /ring put",
/*
  The remainder follow the normal ISO 8859-1 encoding.
*/
    "Encoding 160 /space put",
    "Encoding 161 /exclamdown put",
    "Encoding 162 /cent put",
    "Encoding 163 /sterling put",
    "Encoding 164 /currency put",
    "Encoding 165 /yen put",
    "Encoding 166 /brokenbar put",
    "Encoding 167 /section put",
    "Encoding 168 /dieresis put",
    "Encoding 169 /copyright put",
    "Encoding 170 /ordfeminine put",
    "Encoding 171 /guillemotleft put",
    "Encoding 172 /logicalnot put",
    "Encoding 173 /hyphen put",
    "Encoding 174 /registered put",
    "Encoding 175 /macron put",
    "Encoding 176 /degree put",
    "Encoding 177 /plusminus put",
    "Encoding 178 /twosuperior put",
    "Encoding 179 /threesuperior put",
    "Encoding 180 /acute put",
    "Encoding 181 /mu put",
    "Encoding 182 /paragraph put",
    "Encoding 183 /bullet put",
    "Encoding 184 /cedilla put",
    "Encoding 185 /onesuperior put",
    "Encoding 186 /ordmasculine put",
    "Encoding 187 /guillemotright put",
    "Encoding 188 /onequarter put",
    "Encoding 189 /onehalf put",
    "Encoding 190 /threequarters put",
    "Encoding 191 /questiondown put",
    "Encoding 192 /Agrave put",
    "Encoding 193 /Aacute put",
    "Encoding 194 /Acircumflex put",
    "Encoding 195 /Atilde put",
    "Encoding 196 /Adieresis put",
    "Encoding 197 /Aring put",
    "Encoding 198 /AE put",
    "Encoding 199 /Ccedilla put",
    "Encoding 200 /Egrave put",
    "Encoding 201 /Eacute put",
    "Encoding 202 /Ecircumflex put",
    "Encoding 203 /Edieresis put",
    "Encoding 204 /Igrave put",
    "Encoding 205 /Iacute put",
    "Encoding 206 /Icircumflex put",
    "Encoding 207 /Idieresis put",
    "Encoding 208 /Eth put",
    "Encoding 209 /Ntilde put",
    "Encoding 210 /Ograve put",
    "Encoding 211 /Oacute put",
    "Encoding 212 /Ocircumflex put",
    "Encoding 213 /Otilde put",
    "Encoding 214 /Odieresis put",
    "Encoding 215 /multiply put",
    "Encoding 216 /Oslash put",
    "Encoding 217 /Ugrave put",
    "Encoding 218 /Uacute put",
    "Encoding 219 /Ucircumflex put",
    "Encoding 220 /Udieresis put",
    "Encoding 221 /Yacute put",
    "Encoding 222 /Thorn put",
    "Encoding 223 /germandbls put",
    "Encoding 224 /agrave put",
    "Encoding 225 /aacute put",
    "Encoding 226 /acircumflex put",
    "Encoding 227 /atilde put",
    "Encoding 228 /adieresis put",
    "Encoding 229 /aring put",
    "Encoding 230 /ae put",
    "Encoding 231 /ccedilla put",
    "Encoding 232 /egrave put",
    "Encoding 233 /eacute put",
    "Encoding 234 /ecircumflex put",
    "Encoding 235 /edieresis put",
    "Encoding 236 /igrave put",
    "Encoding 237 /iacute put",
    "Encoding 238 /icircumflex put",
    "Encoding 239 /idieresis put",
    "Encoding 240 /eth put",
    "Encoding 241 /ntilde put",
    "Encoding 242 /ograve put",
    "Encoding 243 /oacute put",
    "Encoding 244 /ocircumflex put",
    "Encoding 245 /otilde put",
    "Encoding 246 /odieresis put",
    "Encoding 247 /divide put",
    "Encoding 248 /oslash put",
    "Encoding 249 /ugrave put",
    "Encoding 250 /uacute put",
    "Encoding 251 /ucircumflex put",
    "Encoding 252 /udieresis put",
    "Encoding 253 /yacute put",
    "Encoding 254 /thorn put",
    "Encoding 255 /ydieresis put",
    "currentdict end definefont",
/*
  Set the font and define functions for adding lines and printing pages.
*/
    "/CourierLatin1 findfont 11 scalefont setfont",
    "/StartPage{/sv save def 48 765 moveto}def",
    "/ld -11.4 def",                    /* Line spacing */
    "/yline 765 def",                   /* Position of top line */
    "/U{show",                          /* Show line, don't advance */
    "  48 yline moveto}def",
    "/S{show",                          /* Show line, move to next line */
    "  /yline yline ld add def",
    "  48 yline moveto}def",
    "/L{ld mul yline add /yline exch def", /* Move down n lines  */
    "  48 yline moveto}def",
    "/EndPage{showpage sv restore}def",
    "%%EndProlog",                      /* End of prolog. */
    "%%Page: 1 1",                      /* Number the first page. */
    "StartPage",                        /* And start it. */
    ""                                  /* Empty string = end of array. */
};                                      /* End PS Prolog */

#define NOESCSEQ        /* We don't support this, should be no need. */
#ifndef NOESCSEQ        /* Swallow ANSI escape and control sequences */
#define ESCSEQ          /* unless -DNOESCSEQ given on cc command line. */
#endif /* NOESCSEQ */

#define PS_WIDTH 80     /* Portrait printer line width, characters */
#define PS_LENGTH 66    /* Portrait printer page length, lines */
#define MAXPSLENGTH 256

/* TextPS Globals */

static int hpos = 0,                    /* Character number in line buffer */
  maxhpos = 0,                          /* Longest line in buffer */
  page = 0,                             /* Page number */
  line = 0,                             /* Line number */
  blank = 0,                            /* Blank line count */
  pagefull = 0,                         /* Flag for page overflow */
  psflag = 0,                           /* Flag for file already postscript */
  proflg = 0,                           /* Prolog done */
  shift = 0,                            /* Shift state */
  ps_escape = 0;

int ps_width = PS_WIDTH;                /* Paper width, characters */
int ps_length = PS_LENGTH;              /* Paper length, characters */

int txt2ps = 0;                         /* Flag set if we convert to PS */

/* Data structures */

/*
  buf is the line buffer.  columns (indexed by hpos) are the characters
  in the line, rows are overstruck lines.  At display time (see addline),
  buf is treated as a 3-dimensional array, with the extra dimension being
  for wraparound.  The total size of the buffer is 80 chars per line times
  66 lines per page times 10 levels of overstriking = 52,800.  This allows
  files that contain absolutely no linefeeds to still print correctly.
*/
#define BUFNUM 10                       /* Number of overstrike buffers */
#define BUFWID 5280                     /* Max characters per line */

unsigned char buf[BUFNUM][BUFWID];      /* Line buffers */
unsigned char outbuf[400];              /* Output buffer */
int linesize[BUFNUM];                   /* Size of each line in buffer */
int bufs[MAXPSLENGTH];                  /* # overstrike buffers per line */

/* Line and page display routines */

/* Forward declarations */

void addline();                         /* Add line to page */
void addchar();                         /* Add character to line */
void newpage();                         /* New page */

void
clearbuf() {                            /* Clear line buffer */
    int i;
/*
  Note: if a loop is used instead of memset, this program runs
  veeeery slooooooowly.
*/
    memset(buf,SP,BUFNUM * BUFWID);     /* Clear buffers and counts */
    for (i = 0; i < BUFNUM; linesize[i++] = -1) ;
    for (i = 0; i < ps_length; bufs[i++] = 0) ;
    hpos = 0;                           /* Reset line buffer pointer */
    maxhpos = 0;                        /* And maximum line ps_length */
}

void
doprolog() {                            /* Output the PostScript prolog */
    int i;
    CHAR crlf[2] = { CR, LF };

    for (i = 0; *prolog[i]; i++) {
#ifdef NT
        if ( winprint ) {
            int rc;
            rc = Win32PrtWrite( prolog[i], strlen(prolog[i]) );
            debug(F111,"txt2ps_char","Win32PrtWrite rc",rc);
            rc = Win32PrtWrite( crlf, 2 );
        }
        else
#endif /* NT */
        if ( lst ) {
            int rc;
            rc = fwrite( prolog[i], 1, strlen(prolog[i]), lst );
            debug(F111,"txt2ps_char","fwrite rc",rc);
            rc = fwrite( crlf, 1, 2, lst );
        }
        proflg++;
    }
}

void
addchar(c) unsigned char c; {           /* Add character to line buffer */
    int i, m;
    int * bp;

    if (c < SP || c == DEL) c = SP;     /* ASCII controls become spaces. */

    for (i = 0; i < BUFNUM; i++) {      /* Find first */
        if (hpos > linesize[i]) {       /* available overstrike buffer */
            buf[i][hpos] = c;           /* Deposit character */
            linesize[i] = hpos;         /* Remember size of this buffer. */
            m = hpos / ps_width;        /* Line-wrap segment number. */
            if (bufs[m] < i) {          /* Highest overstrike buffer used */
#ifdef OS2ONLY
                bp = &bufs[m];          /*   (os2 compiler bug)           */
                *bp = i;                /*   for this line-wrap segment.  */
#else /* OS2ONLY */
                bufs[m] = i;            /*   for this line-wrap segment   */
#endif /* OS2ONLY */
            }
            break;
        }
    }
    if (hpos > maxhpos) maxhpos = hpos; /* Remember maximum line position. */
    if (++hpos >= BUFWID)               /* Increment line position. */
      addline();                        /* If buffer full, dump it. */
}

void
addline() {                             /* Add a line to the current page */
    int i, j, k, m, n, y, wraps;
    unsigned char *p, *q, c;
    CHAR lf = LF;

    if (line == 0 && page == 1) {       /* First line of file? */
        if (!strncmp(buf[0],"%!",2)) {  /* Already Postscript? */
            psflag++;                   /* Yes, set this flag & just copy */
            buf[0][hpos] = '\0';        /* Trim trailing blanks */
#ifdef NT
            if ( winprint ) {               /* Print this line */
                int rc;
                rc = Win32PrtWrite( buf[0], strlen(buf[0]) );
                debug(F111,"txt2ps_char","Win32PrtWrite rc",rc);
                rc = Win32PrtWrite( &lf, 1 );
            }
            else
#endif /* NT */
            if ( lst ) {
                int rc;
                rc = fwrite( buf[0], 1, strlen(buf[0]), lst );
                debug(F111,"txt2ps_char","fwrite rc",rc);
                rc = fwrite( &lf, 1, 1, lst );
            }
            return;
        } else if (!proflg) {           /* Not Postscript, print prolog. */
            doprolog();
        }
    }
    if (linesize[0] < 0) {              /* If line is empty, */
        blank++;                        /* just count it. */
        return;
    }
    if (blank > 0) {                    /* Any previous blank lines? */
        if (blank == 1) {               /* One */
#ifdef NT
        if ( winprint ) {
            int rc;
            rc = Win32PrtWrite( "()S\n", 4 );
            debug(F111,"txt2ps_char","Win32PrtWrite rc",rc);
        }
        else
#endif /* NT */
        if ( lst ) {
            int rc;
            rc = fwrite( "()S\n", 1, 4, lst );
            debug(F111,"txt2ps_char","fwrite rc",rc);
        }
        }
        else {                          /* Many */
            CHAR buf[16];
            sprintf(buf,"%d L\n",blank);        /* safe */
#ifdef NT
        if ( winprint ) {
            int rc;
            rc = Win32PrtWrite( buf, strlen(buf) );
            debug(F111,"txt2ps_char","Win32PrtWrite rc",rc);
        }
        else
#endif /* NT */
        if ( lst ) {
            int rc;
            rc = fwrite( buf, 1, strlen(buf), lst );
            debug(F111,"txt2ps_char","fwrite rc",rc);
        }
        }
    }
    line += blank;                      /* Count the blank lines */
    blank = 0;                          /* Reset blank line counter */

    wraps = maxhpos / ps_width;         /* Number of times line will wrap */
    if (wraps > ps_length) wraps = ps_length;   /* (within reason) */

    for (k = 0; k <= wraps; k++) {      /* For each wrapped line */
        m = k * ps_width;                       /* Starting position in buffer */
        for (i = 0; i <= bufs[k]; i++) { /* For each overstrike buffer */
            y = linesize[i] + 1;        /* Actual character count */
            if (y <= m)                 /* Nothing there, next buffer. */
              continue;
            /* Ending position of this wrap region in buffer. */
            n = (y < m + ps_width) ? y : m + ps_width;
            q = outbuf;
            *q++ = '(';                 /* Start text arg */
            for (j = m, p = buf[i]+m; j < n; j++) { /* For each character */
                c = *p++;
                if (c == '(' || c == ')' || c =='\\') /* Quote specials */
                  *q++ = '\\';          /*  with backslash. */
                if ((int) c < 128)      /* Insert 7-bit character literally */
                  *q++ = c;
                else {                  /* Insert 8-bit character */
                    *q++ = '\\';        /* as octal backslash ps_escape */
                    *q++ = (c >> 6) + '0'; /* (this avoids call to sprintf) */
                    *q++ = ((c >> 3) & 07) + '0';
                    *q++ = (c & 07) + '0';
                }
            }
            *q = '\0';
#ifdef NT
            if ( winprint ) {
                int rc;
                rc = Win32PrtWrite( outbuf, strlen(outbuf) );
                if ( i == bufs[k] )
                    rc = Win32PrtWrite( ")S\n", 3 );
                else
                    rc = Win32PrtWrite( ")U\n", 3 );
                debug(F111,"txt2ps_char","Win32PrtWrite rc",rc);
            }
            else
#endif /* NT */
            if ( lst ) {
                int rc;
                rc = fwrite( outbuf, 1, strlen(outbuf), lst );
                if ( i == bufs[k] )
                    rc = fwrite( ")S\n", 1, 3, lst );
                else
                    rc = fwrite( ")U\n", 1, 3, lst );
                debug(F111,"txt2ps_char","fwrite rc",rc);
            }
        }
    }
    clearbuf();                         /* Clear line buffer */
    line += wraps + 1;                  /* Increment line number */
    if (line > (ps_length - 1)) {               /* If page is full */
        newpage();                      /* print it and start new one */
        pagefull = 1;
    }
}

void
newpage() {                             /* Print current page, start new one */
    CHAR buf[80];
    if (pagefull) {                     /* If we just overflowed onto a */
        pagefull = 0;                   /* new page, but then got a formfeed */
        return;                         /* immediately after... */
    }
    if (!proflg)                        /* Do prolog if not done already */
      doprolog();                       /*  (in case file starts with ^L) */
    if (hpos)                           /* Add any partial line */
      addline();
    line = hpos = 0;                    /* Reset line, advance page */
    page++;
    sprintf(buf,"EndPage\n%%%%Page: %d %d\nStartPage\n",page,page); /* safe */
#ifdef NT
    if ( winprint ) {
        int rc;
        rc = Win32PrtWrite( buf, strlen(buf) );
        debug(F111,"txt2ps_char","Win32PrtWrite rc",rc);
    }
    else
#endif /* NT */
    if ( lst ) {
        int rc;
        rc = fwrite( buf, 1, strlen(buf), lst );
        debug(F111,"txt2ps_char","fwrite rc",rc);
    }
    blank = 0;
}

VOID
txt2ps_begin()
{
    hpos = line = psflag = 0;           /* Initialize these... */
    maxhpos = 0, pagefull = 0, proflg = 0, shift = 0;
    ps_escape = blank = 0;
    page = 1;

    clearbuf();                         /* Clear line buffer. */
}

VOID
txt2ps_end()
{
    CHAR buf[80];
    if (!psflag) {                      /* Done. If not postscript already, */
        if (hpos)                       /* if last line was not empty, */
          addline();                    /* add it to the page. */
        if (page != 1 || line != 0) {   /* Add trailer. */
            sprintf(buf,"EndPage\n%%%%Trailer\n%%%%Pages: %d\n",page); /* safe */
#ifdef NT
            if ( winprint ) {
                int rc;
                rc = Win32PrtWrite( buf, strlen(buf) );
                debug(F111,"txt2ps_char","Win32PrtWrite rc",rc);
            }
            else
#endif /* NT */
            if ( lst ) {
                int rc;
                rc = fwrite( buf, 1, strlen(buf), lst );
                debug(F111,"txt2ps_char","fwrite rc",rc);
            }
        }
    }
}

VOID
txt2ps_char(CHAR c)
{
    if (psflag) {                       /* File already postscript? */
#ifdef NT
        if ( winprint ) {
            int rc;
            rc = Win32PrtWrite( &c, 1 );
            debug(F111,"txt2ps_char","Win32PrtWrite rc",rc);
        }
        else
#endif /* NT */
        if ( lst ) {
            int rc;
            rc = fwrite( &c, 1, 1, lst );
            debug(F111,"txt2ps_char","fwrite rc",rc);
        }
        return;
    }

#ifdef ESCSEQ
    if (ps_escape) {                       /* Swallow ANSI ps_escape sequences */
        switch (ps_escape) {
        case 1:                 /* ESC */
            if (c < 040 || c > 057) /* Not intermediate character */
                ps_escape = 0;
            continue;
        case 2:                 /* CSI */
            if (c < 040 || c > 077)     /* Not parameter or intermediate */
                ps_escape = 0;
            continue;
        default:                        /* Bad ps_escape value, */
            ps_escape = 0;         /* shouldn't happen. */
            break;
        }
    }
#endif /* ESCSEQ */

    if (shift && c > 31 && c < 127)
        c |= 0200;                      /* Handle shift state. */

    if (pagefull && c != 014)   /* Spurious blank page suppression */
        pagefull = 0;

    switch (c) {                        /* Handle the input character */
    case 010:                   /* Backspace */
        hpos--;
        if (hpos < 0) hpos = 0;
        return;

    case 011:                   /* Tab */
        hpos = (hpos | 7) + 1;
        return;

    case 012:                   /* Linefeed */
        addline();                      /* Add the line to the page */
        return;

    case 014:                   /* Formfeed */
        newpage();                      /* Print current page */
        return;

    case 015:                   /* Carriage return */
        hpos = 0;                       /* Back to left margin */
        return;

    case 016:                   /* Shift-Out */
        shift = 1;                      /* Set shift state */
        return;

    case 017:                   /* Shift-In */
        shift = 0;                      /* Reset shift state */
        return;

#ifdef ESCSEQ                           /* Swallow ANSI ps_escape sequences */
/*
  1 = ANSI ps_escape sequence
  2 = ANSI control sequence
*/
    case 033:                   /* ESC or 7-bit CSI */
        ps_escape = ((c = getchar()) == 0133) ? 2 : 1;
        if (c != 033 && c != 0133 && c != 233) /* Not ANSI after all */
            ungetc(c,stdin);            /* put it back, avoid loops */
        return;

    case 0233:                  /* 8-bit CSI */
        if (charset == LATIN1) {
            ps_escape = 2;         /* 0233 is graphic char on PC, etc */
            return;
        }                               /* Otherwise fall thru & print it */
#endif /* ESCSEQ */

    default:
        addchar(c);
    }
}


void
printeron() {
    extern int noprinter, printertype;
    extern char * printsep;

    debug(F111,"printeron()","printon",printon);
    debug(F111,"printeron()","noprinter",noprinter);
    debug(F111,"printeron()","printertype",printertype);
    debug(F110,"printeron() printsep",printsep,0);

    printerclose_t = 0;

    if (printon || noprinter || printertype == PRT_NON)
        /* It's already on or should be ignored */
        return;

    if (printeropen()) {                /* Open OK? */

#ifndef NOLOCAL
        if ( !savefiletext[0] )
          ckstrncpy(savefiletext,filetext,sizeof(filetext));
        if ( is_xprint() )
          ckstrncpy(filetext,"CTRL PRINT",sizeof(filetext));
        else if ( is_cprint() )
            ckstrncpy( filetext, "COPY PRINT",sizeof(filetext));
        else if ( is_aprint() )
          ckstrncpy(filetext,"AUTO PRINT",sizeof(filetext));
        else if ( is_uprint() )
          ckstrncpy(filetext,"USER PRINT",sizeof(filetext));
        else
            ckstrncpy(filetext, "PRINTER ON",sizeof(filetext));

        VscrnIsDirty(VTERM);  /* status line needs to be updated */
#endif /* NOLOCAL */
        printon = TRUE;                 /* and printer is on. */
    }
#ifndef NOLOCAL
    else {
        char errormsg[MAXTERMCOL];
        debug(F100,"printeron() failed","",0);
        ckmakmsg(errormsg,sizeof(errormsg),
                 " ERROR: Unable to open device \"",
                 printername?printername:"prn","\"",NULL );
        popuperror(vmode,errormsg);
    }
#endif /* NOLOCAL */

    /* if there is a separator page, print it */
    if ( printsep ) {
        FILE * file = fopen(printsep,"rb");
        char   buf[512];
        int    count=0;

        if ( file ) {
            while ((count = fread(buf,sizeof(char),512,file)) > 0)
                prtstr(buf,count);
            fclose(file);
        }
    }
}

void
printeroff() {                          /* Turn off printer */
    extern char * printterm;
    extern int    printtimo;
    debug(F111,"printeroff()","printon",printon);
    debug(F110,"printeroff() printterm",printterm,0);
    debug(F111,"printeroff()","printtimo",printtimo);

    if (!printon)                       /* It's already off. */
      return;

    if ( printterm ) {
        prtstr( printterm, strlen(printterm) );
    }

#ifdef BPRINT
    if ( !printbidi ) {
#endif /* BPRINT */
        if ( printtimo ) {
            printerclose_t = time(NULL) + printtimo;
        }
        else if ( !printerclose() )
            return;
#ifdef BPRINT
    }
#endif /* BPRINT */


    printon = FALSE;

#ifndef NOLOCAL
    if ( is_xprint() )
        ckstrncpy(filetext,"CTRL PRINT",sizeof(filetext));
    else if ( is_cprint() )
        ckstrncpy( filetext, "COPY PRINT",sizeof(filetext));
    else if ( is_aprint() )
        ckstrncpy(filetext,"AUTO PRINT",sizeof(filetext));
    else if ( is_uprint() )
        ckstrncpy(filetext,"USER PRINT",sizeof(filetext));
    else if (savefiletext[0]) {         /* Fix status line */
        ckstrncpy(filetext,savefiletext,sizeof(filetext));
        savefiletext[0] = NUL;
    }

    VscrnIsDirty(VTERM);  /* status line needs to be updated */
#endif /* NOLOCAL */
}

#ifndef NOLOCAL
int
is_noprint()
{
    if ( cprint )
        return(0);
    if ( xprint )
        return(0);
    if ( aprint )
        return(0);
    if ( uprint )
        return(0);
    return(1);
}

int
is_uprint()
{
    if ( cprint )
        return(0);
    if ( xprint )
        return(0);
    if ( aprint )
        return(0);
    if ( uprint )
        return(1);
    return(0);
}

int
is_aprint()
{
    if ( cprint )
        return(0);
    if ( xprint )
        return(0);
    if ( aprint )
        return(1);
    return(0);
}

int
is_cprint()
{
    if ( cprint )
        return(1);
    return(0);
}

int
is_xprint()
{
    if ( cprint )
        return(0);
    if ( xprint )
        return(1);
    return(0);
}
#endif /* NOLOCAL */

int
printeropen()
{
    extern int printertype;
#ifdef BPRINT
    if ( printbidi )
        return TRUE;
#endif /* BPRINT */

    if ( winprint || lst )              /* Already open? */
        return TRUE;

    if ( printertype == PRT_NON )
        return FALSE;

    if (!printername)                   /* If printer not redirected */
        lst = fopen("prn", "wb");       /* open the PRN device */
    else if ( printpipe || printertype == PRT_PIP ) {
        if ( !printpipe )
            printpipe = 1;
        lst = (FILE *) popen(printername, "wb" ) ;/* open the piped command */
    }
#ifdef NT
    else if (printertype == PRT_WIN) {
        if ( Win32PrtOpen( printername ) ) {
            debug(F111,"Win32PrtOpen",printername,1);
            winprint = TRUE;
            lst = (FILE *)-1;
        }
    }
#endif /* NT */
    else if ( printertype == PRT_DOS ) {
#ifdef NT
        HANDLE hPrt;
        lst = fopen(printername, "ab"); /* open the file in append mode. */
        if ( lst ) {
            hPrt = (HANDLE)_get_osfhandle(_fileno(lst));
            if ( hPrt != INVALID_HANDLE_VALUE ) {
                prtcfg(hPrt);
            }
        }
#else /* NT */
        lst = fopen(printername, "ab"); /* open the file in append mode. */
#endif /* NT */
    } else if ( printertype == PRT_FIL ) {
        lst = fopen(printername, "ab"); /* open the file in append mode. */
    }

    if ( txt2ps )                       /* Init Txt2PS conversions */
        txt2ps_begin();

    return (lst || winprint);
}

int
printerclose()
{
    extern int printertype;

    printerclose_t = 0;   /* printerclose no longer needs to be called */

    if ( txt2ps )
        txt2ps_end();                   /* Complete Txt2PS conversions */

#ifdef BPRINT
    if ( printbidi )
        return FALSE;
#endif /* BPRINT */

    if ( !(winprint || lst) )           /* Printer Closed? */
        return TRUE;

#ifdef NT
    if ( printertype == PRT_WIN && winprint ) {
        int rc = Win32PrtClose();
        debug(F111,"Win32PrtClose",printername,rc);
        winprint = FALSE;
        lst = NULL;
    }
    else
#endif
    if (lst)
    {
        if ( printpipe && printertype == PRT_PIP )
        {
            pclose(lst);
        }
        else if ( printertype == PRT_DOS || printertype == PRT_FIL )
            fclose(lst);
        lst = NULL;
    }
    return !(winprint || lst);
}

void
prtchar( char c )
{
    int turnoffprinter = FALSE ;
    int rc = 0;

    debug(F101,"prtchar","",c);
#ifdef BPRINT
    if ( printbidi ) {
        rc = bprtwrite( &c, 1 );
        debug(F111,"prtchar","bprtwrite rc",rc);
        return;
    }
#endif /* BPRINT */

    if (printon == FALSE)           /* Printer already on? */
    {
        printeron() ;               /* Turn on printer */
        turnoffprinter = TRUE ;     /* Remember to turn it off */
    }

    if ( txt2ps ) {
        txt2ps_char(c);
    } else
#ifdef NT
    if ( winprint ) {
        rc = Win32PrtWrite( &c, 1 );
        debug(F111,"prtchar","Win32PrtWrite rc",rc);
    }
    else
#endif /* NT */
    if ( lst ) {
        rc = fwrite( &c, 1, 1, lst );
        debug(F111,"prtchar","fwrite rc",rc);
    }

    if ( turnoffprinter )
        printeroff();
}

void
prtstr( char * s, int len )
{
    int turnoffprinter = FALSE ;
    int rc = 0;

    debug(F111,"prtstr",s,len);

#ifdef BPRINT
    if ( printbidi ) {
        rc = bprtwrite( s, len );
        debug(F111,"prtstr","bprtwrite rc",rc);
        return;
    }
#endif /* BPRINT */

    if (printon == FALSE)                       /* Printer already on? */
    {
        printeron() ;               /* Turn on printer */
        turnoffprinter = TRUE ;     /* Remember to turn it off */
    }

    if ( txt2ps ) {
        int i = 0;
        for ( ;i<len;i++ )
            txt2ps_char(s[i]);
    } else
#ifdef NT
    if ( winprint ) {
        rc = Win32PrtWrite( s, len );
        debug(F111,"prtstr","Win32PrtWrite rc",rc);
    }
    else
#endif /* NT */
    if ( lst ) {
        rc = fwrite( s, 1, len, lst );
        debug(F111,"prtstr","fwrite rc",rc);
    }

    if ( turnoffprinter )
        printeroff();
}

void
prtfile( char * filename )
{
    FILE * file=NULL;
    char   buf[512];
    int    count=0;
    int turnoffprinter = FALSE ;
    int rc = 0;

    debug(F110,"prtfile",filename,0);

    if (printon == FALSE)           /* Printer already on? */
    {
        printeron() ;               /* Turn on printer */
        turnoffprinter = TRUE ;     /* Remember to turn it off */
    }

    file = fopen(filename,"rb");
    if ( file ) {
        while ((count = fread(buf,sizeof(char),512,file)) > 0) {
            if ( txt2ps ) {
                int i = 0;
                extern int fcharset;
                USHORT uch;
                int charset;

                charset = fc2tx( fcharset );

                for ( ;i<count;i++ ) {
                    if ( charset >= 0 ) {
                        uch = xl_u[charset](buf[i]);
                        txt2ps_char(xl_tx[TX_8859_1](uch));
                    } else
                        txt2ps_char(buf[i]);
                }
            } else
                prtstr(buf,count);
        }
        fclose(file);
    }

    if ( turnoffprinter )
        printeroff();
}

#ifndef NOLOCAL
/* ----------------------------------------------------------------- */
/* PrtLine - Copy the specified line on screen to printer.           */
/* parameters = Line to print 1-based, terminating character.        */
/* ----------------------------------------------------------------- */
void
prtline(int line, unsigned short achar) {
    int    j;
    USHORT n;
    viocell cells[MAXTERMCOL];
    char    outbuf[MAXTERMCOL + 1];
    int     turnoffprinter = FALSE ;

    if (printon == FALSE)                       /* Printer already on? */
    {
        printeron() ;               /* Turn on printer */
        turnoffprinter = TRUE ;     /* Remember to turn it off */
    }

    if (printon) {                              /* If printer on */
        n = VscrnGetWidth(VTERM) * sizeof(viocell);     /* Line width, incl attributes */
        /* Internally, screen lines are 0-based, so "i-1". */
        memcpy(cells,VscrnGetCells(VTERM,line-1),n);

        for (j = 0; j < VscrnGetWidth(VTERM); j++) {    /* Strip away the attribute bytes */
            if ( isunicode() )
            {
                if (GNOW->itol )
                    outbuf[j] = (*GNOW->itol)(cells[j].c);
            }
            else {
                outbuf[j] =(cells[j].c & 0xFF);
            }
        }

        for (j = VscrnGetWidth(VTERM) - 1; j >= 0; j--) /* Strip trailing blanks */
          if (outbuf[j] != SP)
            break;
        outbuf[j+1] = '\r';
        outbuf[j+2] = achar;
        outbuf[j+3] = NUL;              /* Terminate string with NUL */
        if ( ISTVI(tt_type_mode) || ISWYSE(tt_type_mode) )
            prtstr(outbuf,strlen(outbuf)+1);        /* Print the NUL */
        else
            prtstr(outbuf,strlen(outbuf));
    }

    if ( turnoffprinter )
        printeroff();
}

/* ----------------------------------------------------------------- */
/* PrtScreen - Copy lines on screen to printer.                      */
/* parameters = Top line to print, bottom line to print, 1-based.    */
/* ----------------------------------------------------------------- */
void
prtscreen(BYTE vmode, int top, int bot) {
    int    i, j, first, last, ch;
    USHORT n;
    viocell cells[MAXTERMCOL];
    char    outbuf[MAXTERMCOL + 1];
    int     turnoffprinter = FALSE ;

    if (top < 1 || bot > VscrnGetHeight(vmode)-(tt_status[vmode]?1:0))         /* Args out of bounds? */
      return;

    if (printon == FALSE)                       /* Printer already on? */
    {
        printeron() ;               /* Turn on printer */
        turnoffprinter = TRUE ;     /* Remember to turn it off */
    }

    if (printon) {                              /* If printer on */
        n = VscrnGetWidth(vmode) * sizeof(viocell);     /* Line width, incl attributes */
        for (i = top-1; i < bot; i++) {  /* For each screen line, i... */
            /* Internally, screen lines are 0-based, so "i-1". */

            if (scrollflag[vmode]&& tt_roll[vmode])
              memcpy(cells,VscrnGetCells(vmode,VscrnGetScrollTop(vmode)
                                          -VscrnGetTop(vmode)+i),n) ;
            else
              memcpy(cells,VscrnGetCells(vmode,i),n);

            for (j = 0; j < VscrnGetWidth(vmode); j++) { /* Strip away the attribute bytes */
                if ( isunicode() )
                {
                    ch = (xl_tx[prncs])(cells[j].c);
                }
                else {
#ifndef COMMENT
                    ch = (xl_tx[prncs])((xl_u[tcsl])(cells[j].c & 0xFF));
#else /* COMMENT */
                    ch =(cells[j].c & 0xFF);
#endif /* COMMENT */
                }
                outbuf[j] = (ch >= 0 ? ch : '?');
            }

            for (j = VscrnGetWidth(vmode) - 1; j >= 0; j--) /* Strip trailing blanks */
              if (outbuf[j] != SP)
                break;
            outbuf[j+1] = '\r';
            outbuf[j+2] = '\n';
            outbuf[j+3] = NUL;          /* Terminate string with NUL */
            if ( ISTVI(tt_type_mode) || ISWYSE(tt_type_mode) )
                prtstr(outbuf,strlen(outbuf)+1);        /* Print the NUL */
            else
                prtstr(outbuf,strlen(outbuf));
        }

        if ( xprintff )
            prtchar(FF);

        if ( turnoffprinter )
            printeroff();
    }
}

/* ------------------------------------------------------------------ */
/* End of Printer functions                                           */
/* ------------------------------------------------------------------ */

/*
   C H K A E S  --  Check ANSI Escape Sequence state.
   Previously used for both input and output, now (edit 190) used only
   for output of keystrokes.  Input is handled in cwrite().
*/
int
chkaes(int esc, char c) {
    if (c == CAN || c == SUB)           /* CAN and SUB cancel any sequence */
      esc = ES_NORMAL;
    else                                /* Otherwise */
      switch (esc) {                    /* enter state switcher */

        case ES_NORMAL:                 /* NORMAL state */
          if (c == ESC)                 /* Got an ESC */
            esc = ES_GOTESC;            /* Change state to GOTESC */
          break;                        /* Otherwise stay in NORMAL state */

        case ES_GOTESC:                 /* GOTESC state */
          if (c == '[')                 /* Left bracket after ESC is CSI */
            esc = ES_GOTCSI;            /* Change to GOTCSI state */
          else if (c == 'P' || (c > 0134 && c < 0140)) /* P, ], ^, or _ */
            esc = ES_STRING;            /* Switch to STRING-absorption state */
          else if (c > 057 && c < 0177) /* Final character '0' thru '~' */
            esc = ES_NORMAL;            /* Back to normal */
          else if (c != ESC)            /* ESC in an escape sequence... */
            esc = ES_ESCSEQ;            /* starts a new escape sequence */
          break;                        /* Intermediate or ignored ctrl char */

        case ES_ESCSEQ:                 /* ESCSEQ -- in an escape sequence */
          if (c > 057 && c < 0177)      /* Final character '0' thru '~' */
            esc = ES_NORMAL;            /* Return to NORMAL state. */
          else if (c == ESC)            /* ESC ... */
            esc = ES_GOTESC;            /* starts a new escape sequence */
          break;                        /* Intermediate or ignored ctrl char */

        case ES_GOTCSI:                 /* GOTCSI -- In a control sequence */
          if (c > 077 && c < 0177)      /* Final character '@' thru '~' */
            esc = ES_NORMAL;            /* Return to NORMAL. */
          else if (c == ESC)            /* ESC ... */
            esc = ES_GOTESC;            /* starts over. */
          break;                        /* Intermediate or ignored ctrl char */

        case ES_STRING:                 /* Inside a string */
          if (c == ESC)                 /* ESC may be 1st char of terminator */
            esc = ES_TERMIN;            /* Go see. */
          break;                        /* Absorb all other characters. */

        case ES_TERMIN:                 /* May have a string terminator */
          if (c == '\\')                /* which must be backslash */
            esc = ES_NORMAL;            /* If so, back to NORMAL */
          else                          /* Otherwise */
            esc = ES_STRING;            /* Back to string absorption. */
      }
    return(esc);
}

char * 
protoString(void)
{
    static char buf[24];

    if ( viewonly )
       ckstrncpy( buf, "VIEW ONLY",sizeof(buf)) ;    
    else if ( ttyfd == -1
#ifdef CK_TAPI
         || (tttapi && ttyfd == -2) 
#endif /* CK_TAPI */
         ) {
        ckstrncpy(buf,"Not Connected",sizeof(buf));
    } else if ( !network )
        sprintf(buf, "Serial %ld", speed);         /* safe */
    else
        switch ( nettype ) {
#ifdef SUPERLAT
        case NET_SLAT:
            ckstrncpy(buf, "SuperLAT",sizeof(buf) ) ;
            break;
#endif /* SUPERLAT */

#ifdef DECNET
        case NET_DEC:
            switch ( ttnproto ) {
            case NP_LAT:
                ckstrncpy(buf, "DECnet LAT",sizeof(buf));
                break;
            case NP_CTERM:
                ckstrncpy(buf, "DECnet CTERM",sizeof(buf));
                break;
            default:
                ckstrncpy(buf, "DECnet",sizeof(buf));
                break;
            }
            break;
#endif /* DECNET */

#ifdef SSHBUILTIN
        case NET_SSH:
            ckstrncpy(buf, (char *)ssh_proto_ver(),sizeof(buf));
            break;
#endif /* SSHBUILTIN */

        case NET_TCPB:
            switch ( ttnproto ) {
            case NP_TELNET:
                ckstrncpy(buf, "TELNET",sizeof(buf));
#ifdef CK_AUTHENTICATION
                {
                    int ck_tn_authenticated(void);
                    if ( sstelnet ? TELOPT_U(TELOPT_AUTHENTICATION) :
                         TELOPT_ME(TELOPT_AUTHENTICATION) ) {
                        switch ( ck_tn_authenticated() ) {
                        case AUTHTYPE_KERBEROS_V4:
                            ckstrncat(buf,":K4",sizeof(buf));
                            break;
                        case AUTHTYPE_KERBEROS_V5:
                            ckstrncat(buf,":K5",sizeof(buf));
                            break;
                        case AUTHTYPE_SRP:
                            ckstrncat(buf,":SRP",sizeof(buf));
                            break;
                        case AUTHTYPE_NTLM:
                            ckstrncat(buf,":NTLM",sizeof(buf));
                            break;
#ifdef COMMENT
                        case AUTHTYPE_SSL:
                            /* Not really authentication */
                            ckstrncat(buf,":SSL",sizeof(buf));
                            break;
#endif /* COMMENT */
                        }
                    }
                }
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
                {
                    int ck_tn_encrypting(void);
                    int ck_tn_decrypting(void);
#ifdef CK_SSL
                    extern int tls_active_flag, ssl_active_flag;
#endif /* CK_SSL */

                    if ( TELOPT_ME(TELOPT_ENCRYPTION) ||
                         TELOPT_U(TELOPT_ENCRYPTION) ) {
                        switch ( (ck_tn_encrypting()?1:0) |
                                 (ck_tn_decrypting()?2:0))
                        {
                        case 0:
                            ckstrncat(buf,":pp",sizeof(buf));
                            break;
                        case 1:
                            ckstrncat(buf,":Ep",sizeof(buf));
                            break;
                        case 2:
                            ckstrncat(buf,":pD",sizeof(buf));
                            break;
                        case 3:
                            ckstrncat(buf,":ED",sizeof(buf));
                            break;
                        }
                    }
#ifdef CK_SSL
                    else if (tls_active_flag) {
                        ckstrncat(buf,":TLS",sizeof(buf));
                    } else if (ssl_active_flag) {
                        ckstrncat(buf,":SSL",sizeof(buf));
                    }
#endif /* CK_SSL */
                }
#endif /* CK_ENCRYPTION */
                break;
            case NP_RLOGIN:
                ckstrncpy(buf, "LOGIN",sizeof(buf));
                break;
            case NP_K4LOGIN:
                ckstrncpy(buf, "K4LOGIN",sizeof(buf));
                break;
            case NP_EK4LOGIN:
                ckstrncpy(buf, "EK4LOGIN",sizeof(buf));
                break;
            case NP_K5LOGIN:
                ckstrncpy(buf, "K5LOGIN",sizeof(buf));
                break;
            case NP_EK5LOGIN:
                ckstrncpy(buf, "EK5LOGIN",sizeof(buf));
                break;
            case NP_KERMIT:
                ckstrncpy(buf, "KERMIT",sizeof(buf));
                break;
            case NP_SSL:
	    case NP_SSL_RAW:
                ckstrncpy(buf, "SSL",sizeof(buf));
                break;
            case NP_TLS:
	    case NP_TLS_RAW:
                ckstrncpy(buf, "TLS",sizeof(buf));
                break;
            case NP_K5U2U:
                ckstrncpy(buf, "K5U2U",sizeof(buf));
                break;
            case NP_TCPRAW:
            default:
                ckstrncpy(buf, "TCP/IP",sizeof(buf));
                break;
            }
            break;
        case NET_PIPE:
            ckstrncpy(buf, "Named Pipe",sizeof(buf));
            break;
        case NET_BIOS:
            ckstrncpy(buf, "NetBIOS",sizeof(buf));
            break;
        case NET_FILE:
            ckstrncpy(buf, "FILE",sizeof(buf));
            break;
        case NET_CMD:
            ckstrncpy(buf, "COMMAND",sizeof(buf));
            break;
        case NET_DLL:
            ckstrncpy(buf, "DLL",sizeof(buf));
            break;
        default:
            ckstrncpy(buf, "(unknown)",sizeof(buf));
        }
    return(buf);
}

/* ------------------------------------------------------------------ */
/* ipadl25 - Make normal CONNECT-mode status line                     */
/* ------------------------------------------------------------------ */
void
ipadl25() {
    int i;

    vikinit() ;                         /* Update VIK Table */

    if ( IS97801(tt_type_mode) ) {
        if ( sni_chcode )
            ckstrncpy(usertext, "CH.CODE",(MAXTERMCOL) + 1);
        else
            ckstrncpy(usertext, "ch.code",(MAXTERMCOL) + 1);
        if ( sni_bitmode == 8 )
            ckstrncat(usertext,"-8",(MAXTERMCOL) + 1);
        else
            ckstrncat(usertext,"-7",(MAXTERMCOL) + 1);
    } else {
#ifdef DECLED
        ckstrncpy(usertext," ",(MAXTERMCOL) + 1);
        for ( i=0; i<4 ; i++)
            ckstrncat( usertext, decled[i] ? "\x0F " : "\x07 ",
                       (MAXTERMCOL) + 1) ;
#else /* DECLED */
        strncpy(usertext," K-95 ",(MAXTERMCOL) + 1);
#endif /* DECLED */
    }
#ifdef PCTERM
    if ( tt_pcterm ) {
        ckstrncat(usertext,"PCTERM",(MAXTERMCOL) + 1);
    } else
#endif /* PCTERM */
    if (debses) {
        ckstrncat(usertext,"DEBUG",(MAXTERMCOL) + 1);
    } else if (tt_type_mode > -1 && tt_type_mode <= max_tt) {
        ckstrncat(usertext,tt_info[tt_type_mode].x_name,(MAXTERMCOL) + 1);

        switch ( tt_kb_mode ) {
        case KBM_HE:
            ckstrncat(usertext,"-H",(MAXTERMCOL) + 1);
            break;
        case KBM_RU:
            ckstrncat(usertext,"-R",(MAXTERMCOL) + 1);
            break;
        case KBM_EM:
            ckstrncat(usertext,"-E",(MAXTERMCOL) + 1);
            break;
        case KBM_WP:
            ckstrncat(usertext,"-W",(MAXTERMCOL) + 1);
            break;
        }
    }

    if ( viewonly )
       ckstrncpy( filetext, "VIEW ONLY",sizeof(filetext)) ;
    else if (printon) {
        if ( is_xprint() )
            ckstrncpy( filetext, "CTRL PRINT" ,sizeof(filetext)) ;
        else if ( is_cprint() )
            ckstrncpy( filetext, "COPY PRINT",sizeof(filetext) ) ;
        else if ( is_aprint() )
            ckstrncpy( filetext, "AUTO PRINT",sizeof(filetext) ) ;
        else if ( is_uprint() )
            ckstrncpy( filetext, "USER PRINT",sizeof(filetext) ) ;
        else
            ckstrncpy(filetext,"PRINTER ON",sizeof(filetext));
    } else 
        ckstrncpy( filetext, protoString(), sizeof(filetext));

#ifdef PCTERM
    if ( tt_pcterm ) {
        strcpy(helptext,"Ctrl-CAPSLOCK to deactivate");        /* safe */
    } else
#endif /* PCTERM */
    if (ISVTNT(tt_type_mode) && ttnum != -1)
        strcpy(helptext, "No Help" ) ;
#ifndef NOKVERBS
    else if (vik.help > 255 && keyname(vik.help))
      ckmakmsg(helptext, sizeof(helptext), "Help: ", keyname(vik.help),
                NULL,NULL);
#endif /* NOKVERBS */
    else if ( tt_escape )
      sprintf(helptext, "Help: ^%c?", ctl(escape));     /* safe */
    else
        strcpy(helptext, "No Help" ) ;

#ifdef PCTERM
    if ( tt_pcterm ) {
        exittext[0] = '\0';
    } else
#endif /* PCTERM */
    if (ISVTNT(tt_type_mode) && ttnum != -1)
        strcpy(exittext, "No Exit" ) ;
#ifndef NOKVERBS
    else if (vik.exit > 255 && keyname(vik.exit))
        ckmakmsg(exittext, sizeof(exittext), esc_msg, " ", keyname(vik.exit), NULL);
#endif /* NOKVERBS */
    else if ( tt_escape )
        sprintf(exittext, "%s ^%c?", esc_msg, ctl(escape));     /* safe */
#ifndef NOKVERBS
    else if ( vik.quit > 255 && keyname(vik.quit) )
        ckmakmsg( exittext, sizeof(exittext), "Quit: ", keyname(vik.quit),
                  NULL, NULL);
#endif /* NOKVERBS */
    else
        strcpy( exittext, "No Exit" ) ;

   if ( viewonly )
      hostname[0] = '\0' ;
   else
   {
      if (!network) {
#ifndef NODIAL
         if ( d_name )
            strncpy(hostname, d_name, 40);
         else
#endif /* NODIAL */
            strncpy(hostname, ttname, 40);
#ifdef NT
         _strupr(hostname);
#else /* NT */
         strupr(hostname);
#endif /* NT */
      }
      else {
         strncpy(hostname, ttname, 40);
#ifdef CK_NETBIOS
         if ( ( nettype == NET_BIOS ) && ( *ttname == '*' ) )
            strncpy(hostname+1, NetBiosRemote, NETBIOS_NAME_LEN ) ;
#endif /* CK_NETBIOS */
      }
   }
    VscrnIsDirty(VTERM);  /* status line needs to be updated */
}

/*
   sendchar() simply sends a byte to the host.  No translation,
   no local echo buffer, no half-duplex mode.
*/
int
sendchar(unsigned char c) {
/*
   NOTE: ttoci() uses an ioctl() that has been observed to hang, inexplicably,
   on some systems (e.g. high-end PS/2s that haven't been rebooted in a month,
   and have been going back and forth between SLIP and Kermit.)  In fact,
   this was reported only once, and rebooting the system made the problem go
   away.

   A NEWER NOTE: As of 8 May 94, ttoci() uses DosWrite, but first does an
   ioctl() to make sure the connection is OK.  Hopefully, this combines the
   the benefits of both methods: failure detection, buffering, flow control.
*/
    int x, i = 0;
    long wait = 0;
    con_event evt ;


/*
  Send a character to the serial line in immediate mode, checking to avoid
  overwriting a character already waiting to be sent.  If we fail, e.g. because
  FLOW is RTS/CTS and CTS is not on (like when modems are retraining), enter
  a retry loop up to the timeout limit.
*/
    x = ttoci(dopar(c));                /* Try to send the character */
    if (x > 0 ||                        /* Transmission error */
         x == -1) {                     /* Connection failed  */
        KEY k;
        int w, oldw;

        debug(F101,"sendchar blocked char","",c);
        oldw = 0;                       /* For updating status line once/sec */
        save_status_line();             /* Save current status line */
        escapestatus[VTERM] = TRUE ;
        do {
            w = (waittime - wait) / 1000L;
            if (w != oldw) {            /* Countdown timer in status line */
                oldw = w;
                sprintf(usertext, " TRANSMISSION BLOCKED: %d",w);       /* safe */
                helptext[0] = '\0';
                VscrnIsDirty(VTERM);  /* status line needs to be updated */
            }
            if (evtinbuf(VTERM) > 0) {          /* Is an event waiting? */
                evt = congev(VTERM,0);          /* Read it */
                switch (evt.type) {
                case key:
                    k = evt.key.scancode ;
                    break;
#ifndef NOKVERBS
                case kverb:
                    k = evt.kverb.id | F_KVERB ;
                    break;
#endif /* NOKVERBS */
                case macro:
                    if ( evt.macro.string )
                        free( evt.macro.string ) ;
                    k = -1;
                    break;
                case literal:
                    if ( evt.literal.string )
                        free( evt.literal.string ) ;
                    k = -1;
                    break;
                case csi:
                case esc:
                default:
                    k = -1 ;
                }
                debug(F101,"BLOCKED key","",k);
#ifndef NOKVERBS
                if (k == K_EXIT | F_KVERB) { /* Let them escape back... */
                    SetConnectMode(FALSE,CSX_ESCAPE);      /* this way */
                    return 0;
                } else if (k == (F_KVERB | K_QUIT)) {
                    hangnow = 1;
                    quitnow = 1;
                    strcpy(termessage, "Hangup and quit.\n");
                    SetConnectMode(FALSE,CSX_USERDISC);
                    return 0;
                } else
#endif /* NOKVERBS */
                    if ( tt_escape && k == escape) { /* or this way */
                    evt = congev(VTERM,0);              /* Read it */
                    switch (evt.type) {
                    case key:
                        k = evt.key.scancode ;
                        break;
#ifndef NOKVERBS
                    case kverb:
                        k = evt.kverb.id | F_KVERB;
                        break;
#endif /* NOKVERBS */
                    case macro:
                        if ( evt.macro.string )
                            free( evt.macro.string ) ;
                        k = -1;
                        break;
                    case literal:
                        if ( evt.literal.string )
                            free( evt.literal.string ) ;
                        k = -1;
                        break;
                    case csi:
                    case esc:
                    default:
                        k = -1 ;
                    }
                    if ((k == 'c') || (k == 'C') || (k == 3)) {
                        SetConnectMode(FALSE,CSX_ESCAPE);
                        return 0;
                    } else bleep(BP_WARN);
                } else bleep(BP_WARN);          /* Anything else, just beep. */
            }
            msleep(INTERVAL);           /* Sleep a bit */
            wait += INTERVAL;           /* Go round till limit exhausted */
            x = ttoci(dopar(c));        /* Try to send again */
        } while ((x > 0 || x == -1) && (wait < waittime)
            && (ttyfd != -1 && ttyfd != -2));
        restore_status_line();
        escapestatus[VTERM] = FALSE ;
    }
    if (x < 0 || wait >= waittime) {            /* Go back to prompt if we failed */
        while (evtinbuf(VTERM) > 0)
        {
            evt = congev(VTERM,0);      /* Flush any remaining typeahead */
            if ( evt.type == macro )
            {
                if ( evt.macro.string )
                    free( evt.macro.string ) ;
            }
            else if ( evt.type == literal ) {
                if ( evt.literal.string )
                    free( evt.literal.string );
            }
        }
        SetConnectMode(FALSE,CSX_IOERROR);
        if (network)
            sprintf(termessage,
                     "Can't transmit to network, error status: %d\n", x);       /* safe */
        else {
            ckmakxmsg(termessage, sizeof(termessage),
                      "Serial port blocked",
                     ( x & 0x01 /* TX_WAITING_FOR_CTS */ )
                     ? "\nTx waiting for CTS to be turned ON" : "",
                     ( x & 0x02 /* TX_WAITING_FOR_DSR  */ )
                     ? "\nTx waiting for DSR to be turned ON" : "",
                     ( x & 0x04 /* TX_WAITING_FOR_DCD */ )
                     ? "\nTx waiting for DCD to be turned ON" : "",
                     ( x & 0x08 /* TX_WAITING_FOR_XON */ )
                     ? "\nTx waiting because XOFF received" : "",
                     ( x & 0x10 /* TX_WAITING_TO_SEND_XON */ )
                     ? "\nTx waiting because XOFF transmitted" : "",
                     ( x & 0x20 /* TX_WAITING_WHILE_BREAK_ON */ )
                     ? "\nTx because BREAK is being transmitted" : "",
                     ( x & 0x40 /* TX_WAITING_TO_SEND_IMM */ )
                     ? "\nCharacter waiting to transmit immediately" : "",
                     ( x & 0x80 /* RX_WAITING_FOR_DSR */ )
                     ? "\nRx waiting for DSR to be turned ON" : "",
                       "\n", NULL, NULL );
        }
    }
    return 0;
}

void
sendcharduplex(unsigned char key, int no_xlate ) {
    unsigned char csave;
    unsigned short xkey, xkey7;
    CHAR * bytes = NULL;
    int count = 1,i=0;

    key &= cmdmsk;                      /* Do any requested masking */
    csave = key;

    /* HACK ALERT */
    /* The following reference to tt_kb_mode is done to try to allow */
    /* the russian and hebrew modes to work within the current       */
    /* architecture of K95.  The translation tables for Russian and  */
    /* Hebrew modes are hard coded values belonging to CP866 and     */
    /* CP862.  If they are translated, they will be destroyed.       */
    /* We also only make the change here since at the present time   */
    /* the translations are only single characters, and not strings. */

    debug(F111,"sendcharduplex","no_xlate",no_xlate);

    if ( !no_xlate &&                   /* Translating this string, and */
         os2_outesc == ES_NORMAL )      /* If not inside escape seq.. */
    {                                   /* Translate character sets */
        count = ltorxlat(key, &bytes);
        if ( count <= 0 ) {
            debug(F111,"sendcharduplex ltorxlat()","key",key);
            debug(F111,"sendcharduplex ltorxlat()","count",count);
            return;
        }
        key = bytes[0];
    }
    os2_outesc = chkaes( os2_outesc, key ) ;

    for (i=0 ;i<count;i++ ) {
        if ( i>0 ) {
            key = bytes[i];
        }

        if (sosi
#ifdef NETCONN
#ifdef TCPSOCKET
             || network && (IS_TELNET() && !TELOPT_ME(TELOPT_BINARY))
#endif /* TCPSOCKET */
#endif /* NETCONN */
             ) {
            /* Shift-In/Out selected? or not a BINARY Telnet session */
            if (cmask == 0177 || pmask == 0177) { /* In 7-bit environment? */
                if (key & 0200) {           /* 8-bit character? */
                    if (outshift == 0) {    /* If not shifted, */
                        sendchar(SO);       /* shift. */
                        outshift = 1;
                    }
                } else {
                    if (outshift == 1) {    /* 7-bit character */
                        sendchar(SI);       /* If shifted, */
                        outshift = 0;       /* unshift. */
                    }
                }
            }
        }

        key = key & cmask & pmask; /* Apply Kermit-to-host mask now. */
        if (key == CR) {            /* Handle TERMINAL NEWLINE */
            int stuff = -1, stuff2 = -1;
            if (tnlm) {                     /* TERMINAL NEWLINE ON */
                stuff = LF;                 /* Stuff LF */
            }
#ifdef NETCONN
#ifdef TCPSOCKET
    /* TELNET NEWLINE MODE */
            if (network)
            {
                if (IS_TELNET())
                {
                    switch (TELOPT_ME(TELOPT_BINARY) ? tn_b_nlm : tn_nlm) { /* NVT or BINARY */
                    case TNL_CR:
                        break;
                    case TNL_CRNUL:
                        stuff2 = stuff ;
                        stuff  = NUL ;
                        break;
                    case TNL_CRLF:
                        stuff2 = stuff ;
                        stuff = LF ;
                        break;
                    }
                }
#ifdef RLOGCODE
                else if (IS_RLOGIN())
                {
                    switch (tn_b_nlm) { /* always BINARY */
                    case TNL_CR:
                        break;
                    case TNL_CRNUL:
                        stuff2 = stuff ;
                        stuff  = NUL ;
                        break;
                    case TNL_CRLF:
                        stuff2 = stuff ;
                        stuff = LF ;
                        break;
                    }
                }
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                else if (IS_SSH())
                {
                    switch (tn_b_nlm) { /* always BINARY */
                    case TNL_CR:
                        break;
                    case TNL_CRNUL:
                        stuff2 = stuff ;
                        stuff  = NUL ;
                        break;
                    case TNL_CRLF:
                        stuff2 = stuff ;
                        stuff = LF ;
                        break;
                    }
                }
#endif /* SSHBUILTIN */
            }
#endif /* TCPSOCKET */
#endif /* NETCONN */

            if (stuff > -1) {
                if ( network ) {                /* on a network we want to send  */
                    char crlf[3] ; int cnt=2 ;  /* both characters in one packet */
                    crlf[0] = dopar(key) ;       /* so we use ttol() instead of   */
                    crlf[1] = dopar(stuff) ;    /* sendchar()                    */
                    if (stuff2 > -1) {
                        cnt++ ;
                        crlf[2] = stuff2 ;
                    }

                    if ( wy_block ) {
                        le_puts(crlf,cnt);
                    } else {
                        ttol(crlf,cnt) ;
                    }
                    if (duplex && !wy_block) {
                        le_putchar(key);
                        le_putchar(stuff);
                        if (stuff2 > -1)
                            le_putchar(stuff2);
                    }
                    return ;                    /* we are done */
                }
                else  /* not network */
                {
                    sendchar(dopar(key));   /* Send the CR */
                    if (duplex) {               /* If local echoing... */
                        le_putchar(key);    /*   echo to screen */
                    }
                    key = stuff;
                    csave = key;
                }
            }
            else if (tt_crd && duplex) {    /* CR-DISPLAY CRLF & local echo */
                le_putchar(CR);
                csave = LF;
                key = CR;                   /* ... but only send a CR */
            }
        }
#ifdef TNCODE
        /* If user types the 0xff character (TELNET IAC), it must be doubled. */
        else
            if (dopar(key) == IAC && network && IS_TELNET())
                sendchar(IAC); /* and the other one just below. */
#endif /* TNCODE */

        sendchar(dopar(key));
        if (duplex) {
            le_putchar(csave);
        }
    }
}

int
sendchars(unsigned char * s, int len) {
    int x, i = 0;
    long wait = 0;
    con_event evt ;

    debug(F111,"sendchars","len of s",len);

/*
  Send a character to the serial line in immediate mode, checking to avoid
  overwriting a character already waiting to be sent.  If we fail, e.g. because
  FLOW is RTS/CTS and CTS is not on (like when modems are retraining), enter
  a retry loop up to the timeout limit.
*/

    if (tt_pacing > 0 &&                /* taking care of pacing   */
         !wy_block) {                   /* if everything goes well */
        x = len;
        for ( i=0 ; i<len ; i++ ) {
            if (ttoc(dopar(s[i])) <= 0)
            {
                x = i;
                break;
            }
            msleep(tt_pacing);
        }
    }
    else {
        if ( parity ) {
            for ( i=0 ; i<len ; i++ )
                s[i]=dopar(s[i]);
        }
        if ( wy_block ) {
            le_puts(s,len);
            x = len;
        }
        else
            x = ttol(s,len);                    /* Try to send the string */
    }
    if (x<len) {                                /* Transmission error */
        KEY k;
        int w, oldw;

        debug(F100,"sendchars blocked","",0);
        oldw = 0;                       /* For updating status line once/sec */
        save_status_line();             /* Save current status line */
        escapestatus[VTERM] = TRUE ;
        do {
            w = (waittime - wait) / 1000L;
            if (w != oldw) {            /* Countdown timer in status line */
                oldw = w;
                sprintf(usertext, " TRANSMISSION BLOCKED: %d",w);       /* safe */
                helptext[0] = '\0';
                VscrnIsDirty(VTERM);  /* status line needs to be updated */
            }
            if (evtinbuf(VTERM) > 0) {          /* Is an event waiting? */
                evt = congev(VTERM,0);          /* Read it */
                switch (evt.type) {
                case key:
                    k = evt.key.scancode ;
                    break;
#ifndef NOKVERBS
                case kverb:
                    k = evt.kverb.id | F_KVERB ;
                    break;
#endif /* NOKVERBS */
                case macro:
                    if ( evt.macro.string )
                        free( evt.macro.string ) ;
                    k = -1;
                    break;
                case literal:
                    if ( evt.literal.string )
                        free( evt.literal.string ) ;
                    k = -1;
                    break;
                case csi:
                case esc:
                default:
                    k = -1 ;
                }
                debug(F101,"BLOCKED key","",k);
#ifndef NOKVERBS
                if (k == (F_KVERB | K_EXIT)) { /* Let them escape back... */
                    SetConnectMode(FALSE,CSX_ESCAPE);      /* this way */
                    return 0;
                } else if (k == (F_KVERB | K_QUIT)) {
                    SetConnectMode(0,CSX_USERDISC);
                    hangnow = 1;
                    quitnow = 1;
                    strcpy(termessage, "Hangup and quit.\n");
                    return 0;
                } else
#endif /* NOKVERBS */
                    if (tt_escape && k == escape) { /* or this way */
                    evt = congev(VTERM,0);              /* Read it */
                    switch (evt.type) {
                    case key:
                        k = evt.key.scancode ;
                        break;
#ifndef NOKVERBS
                    case kverb:
                        k = evt.kverb.id | F_KVERB;
                        break;
#endif /* NOKVERBS */
                    case macro:
                        if ( evt.macro.string )
                            free( evt.macro.string ) ;
                        k = -1;
                        break;
                    case literal:
                        if ( evt.literal.string )
                            free( evt.literal.string ) ;
                        k = -1;
                        break;
                    case csi:
                    case esc:
                    default:
                        k = -1 ;
                    }
                    if ((k == 'c') || (k == 'C') || (k == 3)) {
                        SetConnectMode(FALSE,CSX_ESCAPE);
                        return 0;
                    }
                    else
                        bleep(BP_WARN);
                }
                else
                    bleep(BP_WARN);             /* Anything else, just beep. */
            }
            msleep(INTERVAL);           /* Sleep a bit */
            wait += INTERVAL;           /* Go round till limit exhausted */
            if ( x>0 ) {                /* Something was sent last time */
                s+=x ;                  /* So advance the pointer */
                len-=x ;                /* And reduce the length */
            }
            x = ttol(s,len);    /* Try to send again */
        } while ((x <= 0) && (wait < waittime));
        restore_status_line();
        escapestatus[VTERM] = FALSE ;
    }
    if (wait >= waittime) {             /* Go back to prompt if we failed */
        while (evtinbuf(VTERM) > 0)
        {
            evt = congev(VTERM,0);      /* Flush any remaining typeahead */
            if ( evt.type == macro && evt.macro.string )
                free( evt.macro.string ) ;
            else if ( evt.type == literal && evt.literal.string )
                free( evt.literal.string );
        }
        SetConnectMode(FALSE,CSX_IOERROR);
        x = 0 ;      /* ttol() doesn't return error status */
        if (network)
            sprintf(termessage,
                     "Can't transmit to network, error status: %d\n", x);
        else {
            ckmakxmsg(termessage, sizeof(termessage),
                      "Serial port blocked",
                     ( x & 0x01 /* TX_WAITING_FOR_CTS */ )
                     ? "\nTx waiting for CTS to be turned ON" : "",
                     ( x & 0x02 /* TX_WAITING_FOR_DSR  */ )
                     ? "\nTx waiting for DSR to be turned ON" : "",
                     ( x & 0x04 /* TX_WAITING_FOR_DCD */ )
                     ? "\nTx waiting for DCD to be turned ON" : "",
                     ( x & 0x08 /* TX_WAITING_FOR_XON */ )
                     ? "\nTx waiting because XOFF received" : "",
                     ( x & 0x10 /* TX_WAITING_TO_SEND_XON */ )
                     ? "\nTx waiting because XOFF transmitted" : "",
                     ( x & 0x20 /* TX_WAITING_WHILE_BREAK_ON */ )
                     ? "\nTx because BREAK is being transmitted" : "",
                     ( x & 0x40 /* TX_WAITING_TO_SEND_IMM */ )
                     ? "\nCharacter waiting to transmit immediately" : "",
                     ( x & 0x80 /* RX_WAITING_FOR_DSR */ )
                     ? "\nRx waiting for DSR to be turned ON" : "",
                       "\n", NULL, NULL );
        }
    }
    return 0;
}

void
sendcharsduplex(unsigned char * s, int len, int no_xlate ) {
    int i,j,n ;
    static unsigned char * sendbuf = NULL;
    static buflen = 0;
    unsigned char * bufptr, *stuffptr ;
    CHAR * bytes = NULL;
    int count = 1;

    /* count number of CRs that might require stuffing of LFs */
    for ( i=0,n=0 ;i<len;i++ ) {
        if ( s[i] == CR ) {
            n++;
        }
    }
    debug(F111,"sendcharsduplex","len of s",len);
    debug(F111,"sendcharsduplex","num of CRs",n);
    debug(F111,"sendcharsduplex","old buflen",buflen);
    debug(F111,"sendcharsduplex","no_xlate",no_xlate);

    if ( (tt_utf8 ? 3*len : (len+n+1)) > buflen && sendbuf != NULL ) {
        free(sendbuf);
        sendbuf = NULL;
        buflen = 0;
    }

    if ( sendbuf == NULL ) {
        sendbuf = (unsigned char *)malloc((tt_utf8 ? 3*len : (len+n+1)));
        if ( sendbuf == NULL )
            return;
        buflen = (tt_utf8 ? 3*len : (len+n+1));
    }
    debug(F111,"sendcharsduplex","new buflen",buflen);

   stuffptr = bufptr = sendbuf - 1 ;
   for ( i = 0 ; i < len ; i++ )
   {
       bufptr = ++stuffptr ;
       *bufptr = s[i] & cmdmsk;         /* Do any requested masking */

       if ( !no_xlate &&                /* If translating this string, and */
            os2_outesc == ES_NORMAL) {  /* If not inside escape seq.. */
           count = ltorxlat(*bufptr, &bytes);
           if ( count > 0 )
               *bufptr = bytes[0];
       }

       for ( j=0;j<count;j++ ) {
           if ( j>0 ) {
               bufptr = ++stuffptr;
               *bufptr = bytes[++j];
           }
           os2_outesc = chkaes( os2_outesc, *bufptr ) ;

       if (sosi
#ifdef NETCONN
#ifdef TCPSOCKET
            || (network && IS_TELNET() && !TELOPT_ME(TELOPT_BINARY))
#endif /* TCPSOCKET */
#endif /* NETCONN */
            ) {
        /* Shift-In/Out selected? or not a BINARY Telnet session */
           if (cmask == 0177 || pmask == 0177) { /* In 7-bit environment? */
               if (*bufptr & 0200) {            /* 8-bit character? */
                   if (outshift == 0) { /* If not shifted, */
                       /* sendchar(SO); /* shift. */
                       *(++stuffptr) = SO ;
                       outshift = 1;
                   }
               }
               else {
                   if (outshift == 1) { /* 7-bit character */
                       /* sendchar(SI); /* If shifted, */
                       *(++stuffptr) = SI ;
                       outshift = 0;    /* unshift. */
                   }
               }
           }
       }

       *bufptr = *bufptr & cmask & pmask; /* Apply Kermit-to-host mask now. */
       if (duplex && !wy_block) {
           le_putchar(*bufptr) ;
       }
       if (*bufptr == CR) {             /* Handle TERMINAL NEWLINE */
           if (tnlm) {                  /* TERMINAL NEWLINE ON */
               *(++stuffptr) = LF;                      /* Stuff LF */
           }
#ifdef NETCONN
#ifdef TCPSOCKET
       /* TELNET NEWLINE MODE */
           if (network)
           {
               if (IS_TELNET())
               {
                   switch (TELOPT_ME(TELOPT_BINARY) ? tn_b_nlm : tn_nlm) { /* NVT or BINARY */
                   case TNL_CR:
                       break;
                   case TNL_CRNUL:
                       *(++stuffptr)  = dopar(NUL) ;
                       break;
                   case TNL_CRLF:
                       *(++stuffptr) = dopar(LF) ;
                       break;
                   }
               }
#ifdef RLOGCODE
               else if (IS_RLOGIN())
               {
                   switch (tn_b_nlm) { /* always BINARY */
                   case TNL_CR:
                       break;
                   case TNL_CRNUL:
                       *(++stuffptr)  = dopar(NUL) ;
                       break;
                   case TNL_CRLF:
                       *(++stuffptr) = dopar(LF) ;
                       break;
                   }
               }
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
               else if (IS_SSH())
               {
                   switch (tn_b_nlm) { /* always BINARY */
                   case TNL_CR:
                       break;
                   case TNL_CRNUL:
                       *(++stuffptr)  = dopar(NUL) ;
                       break;
                   case TNL_CRLF:
                       *(++stuffptr) = dopar(LF) ;
                       break;
                   }
               }
#endif /* SSHBUILTIN */
           }
#endif /* TCPSOCKET */
#endif /* NETCONN */

           if ( duplex & tt_crd && !wy_block) { /* CR-DISPLAY CRLF & local echo */
               le_putchar(LF);
           }
       } /* CR */
#ifdef TNCODE
       else {
    /* If user types the 0xff character (TELNET IAC), it must be doubled. */
           if (dopar(*bufptr) == IAC && network && IS_TELNET())
               *(++stuffptr) = dopar(IAC); /* and the other one just below. */
       }
#endif /* TNCODE */
       *bufptr = dopar(*bufptr) ;

       } /* for (j) */
   }    /* for (i) */
    *(stuffptr+1) = '\0' ; /* add NULL for debug.log */
    debug(F111,"sendcharsduplex","len of sendbuf",strlen(sendbuf));
    sendchars( sendbuf, (stuffptr - sendbuf+1) ) ;
}


/*  S E N D E S C S E Q  --  Send ESC followed by the given string  */
/*
  Used for sending host-requested reports, as well as for sending
  F-keys, arrow keys, etc.  Translation, shifting, etc, are not issues here.
*/
int
sendescseq(CHAR *s) {
    char c;
    unsigned char sendstr[24], * p  ;

    /* Handle 7-bit vs 8-bit escape sequences...*/

    if (send_c1 && ((*s == '[' || *s == 'O'))) /* 8-bit C1 controls... */
    {
       sendstr[0] = (*s++ ^ (CHAR) 0x40) | (CHAR) 0x80;
    }
    else                                /* Or 7-bit escape sequences... */
    {
        if ( ISDG200(tt_type) )
            sendstr[0] = XRS ;
        else
            sendstr[0] = ESC;
    }
    p = sendstr + 1 ;

    if (duplex && !wy_block) {          /* Half duplex, echo it to the */
        le_putchar(sendstr[0]);
    }

    for ( ; *s ; s++)
    {
        *p = *s ;
        p++ ;
        if (duplex && !wy_block) {
            le_putchar(*s);
        }
    }
    *p = '\0' ;   /* tack on a Null for debug.log */
    sendchars(sendstr,p-sendstr);
    return(0) ;
}



/* ------------------------------------------------------------------ */
/* IsDoubleWidth -                                                    */
/* ------------------------------------------------------------------ */
bool
isdoublewidth( unsigned short y )     /* based from 1 */
{
    return VscrnGetLineVtAttr(VTERM,y-1) & VT_LINE_ATTR_DOUBLE_WIDE ;
}

/* ------------------------------------------------------------------ */
/* CursorNextLine -                                                   */
/* ------------------------------------------------------------------ */
void
cursornextline() {
    if ( decsasd == SASD_TERMINAL ) {
        /* Due to a log from dcombeer I am no longer sure that */
        /* cursornextline() or cursorprevline() is affected by */
        /* Origin mode                                         */

        if (marginbot > wherey[VTERM]) {
            if ( printon && is_aprint() ) {
                prtline( wherey[VTERM], LF ) ;
            }
            lgotoxy(VTERM,1, wherey[VTERM] + 1);
        } else if ( wy_autopage ) {
            if ( printon && is_aprint() ) {
                prtline( wherey[VTERM], LF ) ;
            }
            lgotoxy(VTERM, 1, margintop);
        } else if (ISVT100(tt_type_mode) || ISANSI(tt_type_mode)) {
            wrtch(CR);
            wrtch(LF);
        }
    }
    else if ( (ISWYSE(tt_type_mode) || ISTVI(tt_type_mode)) && autoscroll
              && !protect ){
        wrtch(CR);
        wrtch(LF);
    }

    if ( wrapit )
        wrapit = FALSE;
}

/* ------------------------------------------------------------------ */
/* CursorPrevLine -                                                   */
/* ------------------------------------------------------------------ */
void
cursorprevline() {
    if ( printon && is_aprint() ) {
        prtline( wherey[VTERM], LF ) ;
    }
    if ( decsasd == SASD_TERMINAL ) {
        /* Due to a log from dcombeer I am no longer sure that */
        /* cursornextline() or cursorprevline() is affected by */
        /* Origin mode                                         */

        if (margintop != wherey[VTERM])
            lgotoxy(VTERM, 1, wherey[VTERM] - 1);
        else if ( wy_autopage )
            lgotoxy(VTERM, 1, marginbot);
    }
    if ( wrapit )
        wrapit = FALSE;
}

/* ------------------------------------------------------------------ */
/* CursorUp -                                                         */
/* ------------------------------------------------------------------ */
void
cursorup(int wrap) {
    if ( printon && is_aprint() ) {
        prtline( wherey[VTERM], LF ) ;
    }
    if ( decsasd == SASD_TERMINAL ) {
        if ((relcursor ? margintop : 1) != wherey[VTERM])
            lgotoxy(VTERM, wherex[VTERM], wherey[VTERM] - 1);
        else if ( wrap ||
                  ISWYSE(tt_type_mode) ||
                  ISTVI(tt_type_mode) ||
                  ISHZL(tt_type_mode) ||
                  ISDG200(tt_type_mode))
            lgotoxy(VTERM, wherex[VTERM],
                     (relcursor ? marginbot :
                       VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0)));
    }
    if ( wrapit )
        wrapit = FALSE;
}

/* ------------------------------------------------------------------ */
/* CursorDown -                                                       */
/* ------------------------------------------------------------------ */
void
cursordown(int wrap) {
    if ( decsasd == SASD_TERMINAL ) {
        if ((relcursor ? marginbot :
              VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0)) > wherey[VTERM])
        {
            if ( printon && is_aprint() ) {
                prtline( wherey[VTERM], LF ) ;
            }
            lgotoxy(VTERM,wherex[VTERM], wherey[VTERM] + 1);
        } else if ( wrap ||
                    ((ISWYSE(tt_type_mode) ||
                     ISTVI(tt_type_mode) ||
                     ISHZL(tt_type_mode) ||
                     ISDG200(tt_type_mode)) &&
                    !autoscroll || protect || wy_autopage) )
        {
            if ( printon && is_aprint() ) {
                prtline( wherey[VTERM], LF ) ;
            }
            lgotoxy(VTERM, wherex[VTERM], (relcursor ? margintop : 1));
        } else if ( (ISWYSE(tt_type_mode) || ISTVI(tt_type_mode)) &&
                  autoscroll && !protect)
            wrtch(LF);
    }
    if ( wrapit )
        wrapit = FALSE;
}

/* ------------------------------------------------------------------ */
/* CursorRight -                                                      */
/* ------------------------------------------------------------------ */
void
cursorright(int wrap) {
    if ( decsasd == SASD_STATUS ) {
        if (wherex[VSTATUS] < VscrnGetWidth(VTERM))
           lgotoxy(VSTATUS, wherex[VSTATUS]+1,1);
    }
    else {
        char dwl = isdoublewidth(wherey[VTERM]) ;
        if (wherex[VTERM] < ( dwl ? VscrnGetWidth(VTERM) / 2 : VscrnGetWidth(VTERM)))
            lgotoxy( VTERM, wherex[VTERM] + 1, wherey[VTERM]);
        else if ( wrap ||
                  ISUNIXCON(tt_type_mode) ||
                  ISWYSE(tt_type_mode) ||
                  ISTVI(tt_type_mode) ||
#ifdef COMMENT
                  ISHFT(tt_type_mode) ||
#endif /* COMMENT */
                  ISDG200(tt_type_mode)) {
            cursornextline();
            if ( wrapit ) {
                cursorright(0);
                wrapit = FALSE ;
            }
        }
    }
    if ( wrapit )
        wrapit = FALSE;
}

/* ------------------------------------------------------------------ */
/* CursorLeft -                                                       */
/* ------------------------------------------------------------------ */
void
cursorleft(int wrap) {
    char dwl = isdoublewidth(wherey[VTERM]) ;

    if ( decsasd == SASD_STATUS ) {
        if (wherex[VSTATUS] > 1)
            lgotoxy( VSTATUS, wherex[VSTATUS] - 1, 1);
    }
    else {
        if (wherex[VTERM] > 1)
            lgotoxy( VTERM,
                     wherex[VTERM] - 1,
                     wherey[VTERM]);
        else if ( wrap ||
                  (ISUNIXCON(tt_type_mode) ||
#ifdef COMMENT
                    ISHFT(tt_type_mode) ||
#endif /* COMMENT */
                    ISWYSE(tt_type_mode) ||
                    ISTVI(tt_type_mode) ||
                    ISDG200(tt_type_mode))) {
            lgotoxy( VTERM,
                     ( dwl ? VscrnGetWidth(VTERM) / 2 : VscrnGetWidth(VTERM)),
                     wherey[VTERM] - 1 );
        }
    }
    if ( wrapit )
        wrapit = FALSE;
}

/* ------------------------------------------------------------------ */
/* ReverseScreen                                                                              */
/* ------------------------------------------------------------------ */
void
reversescreen(BYTE vmode) {
    int         r, c, width;

#ifdef ONETERMUPD
    viocell *   cell=NULL ;

    for (r = 0; r < VscrnGetHeight(vmode)-(tt_status[VTERM]?1:0); r++) {          /* Loop for each row */
        width = VscrnGetLineWidth(vmode,r) ;
        for (c = 0;c < width;c++) { /* Loop for each character in row */
            cell = VscrnGetCell( vmode, c, r ) ;
            cell->a = byteswapcolors(cell->a);
        }
    }
#else
    if ( IsConnectMode() ) {  /* In Terminal Mode */
        viocell *   cell=NULL ;

        for (r = 0; r < VscrnGetHeight(vmode)-(tt_status[VTERM]?1:0); r++) {      /* Loop for each row */
            width = VscrnGetLineWidth(vmode,r) ;
            for (c = 0;c < width;c++) { /* Loop for each character in row */
                cell = VscrnGetCell( vmode, c, r ) ;
                cell->a = byteswapcolors(cell->a);
                }
            }
        }
    else {
        int x =0;
        USHORT          n=0;
        viocell   cells[MAXSCRNCOL];

        n = cmd_cols ;
        for (r = 0; r < cmd_rows; r++) {        /* Loop for each row */
           ReadCellStr(cells, &n, r, 0);        /* Read this row from the screen */
           for (c = 0; c < cmd_cols; c++) {     /* Loop for each character in row */
                cells[c].a = swapcolors(cells[c].a);
                }
            WrtCellStr(cells, n, r, 0);  /* Write the row back. */
        }
    }
#endif /* ONETERMUPD */
}

/* ------------------------------------------------------------------ */
/* FlipScreen                                                                             */
/* ------------------------------------------------------------------ */
void                            /* Flip screen between */
flipscreen(BYTE vmode) {        /* tell Vscrn code to swap foreground     */
                               /* and background colors on screen update */
    if ( vmode == VTERM ) {
        decscnm = decscnm ? FALSE : TRUE ;
        defaultattribute =
            swapcolors(defaultattribute);
        underlineattribute =
            swapcolors(underlineattribute);
        italicattribute = swapcolors(italicattribute);
        reverseattribute=swapcolors(reverseattribute);
        graphicattribute=swapcolors(graphicattribute);
        attribute = swapcolors( attribute );
    } else if ( vmode == VCMD ) {
        colorcmd = swapcolors(colorcmd);
    }
    reversescreen(vmode);
}

int
savscrbk(mode,name,disp) int mode; char * name; int disp; {
    char *tp;
    static struct filinfo xx;
    int savfil, i, j, k;
    char buf[1024];

    zclose(ZMFILE);

    if (disp) {
        xx.bs = 0; xx.cs = 0; xx.rl = 0; xx.org = 0; xx.cc = 0;
        xx.typ = 0; xx.dsp = XYFZ_A; xx.os_specific = '\0';
        xx.lblopts = 0;
        savfil = zopeno(ZMFILE,name,NULL,&xx);
    } else savfil = zopeno(ZMFILE,name,NULL,NULL);

    if (savfil) {
        int    i, j, first, last;
        USHORT n;
        viocell cells[MAXTERMCOL];
        char    outbuf[MAXTERMCOL + 1];
        int     turnoffprinter = FALSE ;
        ULONG   beg, top, end;

        beg = VscrnGetBegin(mode);
        top = VscrnGetTop(mode);
        end = VscrnGetEnd(mode);

        n = VscrnGetWidth(mode) * sizeof(viocell);      /* Line width, incl attributes */
        for (i = beg; i != end; i = (i+1)%VscrnGetBufferSize(mode)) {
            /* For each scrollback line, i... */
            memcpy(cells,VscrnGetCells(mode,i-top),n);

            for (j = 0; j < VscrnGetWidth(mode); j++) { /* Strip away the attribute bytes */
                if ( isunicode() )
                {
                    if (GNOW->itol )
                        outbuf[j] = (*GNOW->itol)(cells[j].c);
                }
                else {
                    outbuf[j] =(cells[j].c & 0xFF);
                }
                if (outbuf[j] == NUL)           /* Don't allow NULs in the stream */
                    outbuf[j] = SP;
            }

            for (j = VscrnGetWidth(mode) - 1; j >= 0; j--) /* Strip trailing blanks */
                if (outbuf[j] != SP)
                    break;
            outbuf[j+1] = NUL;          /* Terminate string with NUL */
            zsoutl(ZMFILE,outbuf);
        }
        zclose(ZMFILE);
        return(1);
    } else {
        return(0);
    }
}

int
pnumber(int *achar) {
    int num = 0;
    while (isdigit(*achar)) {           /* Get number */
        num = (num * 10) + (*achar) - 48;
            *achar = (escnext<=esclast)?escbuffer[escnext++]:0;
    }
    return (num);
}

/* ----------------------------------------------------------------- */
/* Clear Functions                                                   */
/* ----------------------------------------------------------------- */

unsigned char
geterasecolor( int vmode )
{
    unsigned char erasecolor = 0 ;

    switch ( vmode ) {
    case VSTATUS:
        if ( !ISVT220(tt_type_mode) ) {
            erasecolor = colorstatus ;
            break;
        }
        /* else fall through and act as VTERM */
    case VTERM:
        if ( erasemode == 0 ) {
            erasecolor = attribute ;
        }
        else {
            erasecolor = defaultattribute ;
        }
        break;

    case VCMD:
    default:
        erasecolor = colorcmd;
    }
    return erasecolor;
}

void
clrscreen( BYTE vmode, CHAR fillchar ) {
    int             y;
    videoline *     line ;

    /* This function becomes really simple with the new model
       since all we do is move the top of the screen down in
       the vscrn buffer by the size of the screen.
    */

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( IS97801(tt_type_mode) ) {
        VscrnScroll(vmode,UPWARD,margintop-1,
                     marginbot-1,
                     marginbot-margintop+1,
                     margintop==1 &&
                     marginbot==(VscrnGetHeight(vmode)-(tt_status[vmode]?1:0)),
                     fillchar);
    }
    else {
        VscrnScroll(vmode,UPWARD,
                     0,VscrnGetHeight(vmode)-(tt_status[vmode]?2:1),
                     VscrnGetHeight(vmode)-(tt_status[vmode]?1:0),
                     TRUE,fillchar);
    }
}

void
clrtoeoln( BYTE vmode, CHAR fillchar ) {
    int x ;
    videoline * line = NULL ;
    unsigned char cellcolor = geterasecolor(vmode);

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    /* take care of current line */
    line = VscrnGetLineFromTop( vmode,wherey[vmode]-1 ) ;
    for ( x=wherex[vmode]-1 ; x < MAXTERMCOL ; x++ )
        {
        line->cells[x].c = fillchar ;
        line->cells[x].a = cellcolor ;
        line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
        }
}

void
clreoscr_escape( BYTE vmode, CHAR fillchar ) {
    int x,y,h;
    videoline * line = NULL;
    unsigned char cellcolor = geterasecolor(vmode) ;

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if (wherex[vmode] == 1 && wherey[vmode] == 1) {
        clrscreen(vmode,fillchar);
        return;
    }

    RequestVscrnMutex(vmode, SEM_INDEFINITE_WAIT) ;
    /* take care of current line */
    line = VscrnGetLineFromTop(vmode,wherey[vmode]-1) ;
    for ( x=wherex[vmode]-1 ; x <MAXTERMCOL ; x++ )
    {
        line->cells[x].c = fillchar ;
        line->cells[x].a = cellcolor;
        line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
    }
    line->vt_line_attr = VT_LINE_ATTR_NORMAL ;

    /* now take care of additional lines */
    if ( IS97801(tt_type_mode) )
        h = marginbot-1;
    else
        h = VscrnGetHeight(vmode)-(tt_status[vmode]?1:0) ;
    for ( y=wherey[vmode] ; y<h ; y++)
    {
        line = VscrnGetLineFromTop(vmode,y) ;
        for ( x=0 ; x <MAXTERMCOL ; x++ )
        {
            line->cells[x].c = fillchar ;
            line->cells[x].a = cellcolor;
            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
        }
        line->vt_line_attr = VT_LINE_ATTR_NORMAL ;
    }
    ReleaseVscrnMutex(vmode);
}

void
clrboscr_escape( BYTE vmode, CHAR fillchar ) {
    int x,y,h;
    videoline * line = NULL;
    unsigned char cellcolor = geterasecolor(vmode) ;

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    /* now take care of first wherey[VTERM]-1 lines */
    if ( IS97801(tt_type_mode) )
        h = margintop-1;
    else
        h = 0;
    for ( y=h ; y<wherey[vmode]-1 ; y++ )
        {
        line = VscrnGetLineFromTop(vmode,y) ;
        for ( x=0 ; x <MAXTERMCOL ; x++ )
            {
            line->cells[x].c = fillchar ;
            line->cells[x].a = cellcolor;
            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
            }
        line->vt_line_attr = VT_LINE_ATTR_NORMAL ;
        }

    /* take care of current line */
    line = VscrnGetLineFromTop(vmode,wherey[vmode]-1) ;
    for ( x=0 ; x < wherex[vmode] ; x++ )
        {
        line->cells[x].c = fillchar ;
        line->cells[x].a = cellcolor;
        line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
        }
    line->vt_line_attr = VT_LINE_ATTR_NORMAL ;
}

void
clrregion( BYTE vmode, CHAR fillchar ) {
    int             y;
    videoline *     line ;

    /* This function becomes really simple with the new model
       since all we do is move the top of the screen down in
       the vscrn buffer by the size of the screen.
    */

    if ( wherey[VTERM] < margintop ||
         wherey[VTERM] > marginbot )
        return;

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    VscrnScroll(vmode,UPWARD,margintop-1,
                 marginbot-1,
                 marginbot-margintop+1,
                 margintop==1 &&
                 marginbot==(VscrnGetHeight(vmode)-(tt_status[vmode]?1:0)),
                 fillchar);
}

void
clreoreg_escape( BYTE vmode, CHAR fillchar ) {
    int x,y,h;
    videoline * line = NULL;
    unsigned char cellcolor = geterasecolor(vmode) ;

    if ( wherey[VTERM] < margintop ||
         wherey[VTERM] > marginbot )
        return;

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if (wherex[vmode] == 1 && wherey[vmode] == 1) {
        clrscreen(vmode,fillchar);
        return;
    }

    RequestVscrnMutex(vmode, SEM_INDEFINITE_WAIT) ;
    /* take care of current line */
    line = VscrnGetLineFromTop(vmode,wherey[vmode]-1) ;
    for ( x=wherex[vmode]-1 ; x <MAXTERMCOL ; x++ )
    {
        line->cells[x].c = fillchar ;
        line->cells[x].a = cellcolor;
        line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
    }
    line->vt_line_attr = VT_LINE_ATTR_NORMAL ;

    /* now take care of additional lines */
    h = marginbot-1;
    for ( y=wherey[vmode] ; y<h ; y++)
    {
        line = VscrnGetLineFromTop(vmode,y) ;
        for ( x=0 ; x <MAXTERMCOL ; x++ )
        {
            line->cells[x].c = fillchar ;
            line->cells[x].a = cellcolor;
            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
        }
        line->vt_line_attr = VT_LINE_ATTR_NORMAL ;
    }
    ReleaseVscrnMutex(vmode);
}

void
clrboreg_escape( BYTE vmode, CHAR fillchar ) {
    int x,y,h;
    videoline * line = NULL;
    unsigned char cellcolor = geterasecolor(vmode) ;

    if ( wherey[VTERM] < margintop ||
         wherey[VTERM] > marginbot )
        return;

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    /* now take care of first wherey[VTERM]-1 lines */
    h = margintop-1;
    for ( y=h ; y<wherey[vmode]-1 ; y++ )
        {
        line = VscrnGetLineFromTop(vmode,y) ;
        for ( x=0 ; x <MAXTERMCOL ; x++ )
            {
            line->cells[x].c = fillchar ;
            line->cells[x].a = cellcolor;
            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
            }
        line->vt_line_attr = VT_LINE_ATTR_NORMAL ;
        }

    /* take care of current line */
    line = VscrnGetLineFromTop(vmode,wherey[vmode]-1) ;
    for ( x=0 ; x < wherex[vmode] ; x++ )
        {
        line->cells[x].c = fillchar ;
        line->cells[x].a = cellcolor;
        line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
        }
    line->vt_line_attr = VT_LINE_ATTR_NORMAL ;
}

void
clrbol_escape( BYTE vmode, CHAR fillchar ) {
    videoline * line = NULL ;
    int x ;
    unsigned char cellcolor = geterasecolor(vmode) ;

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    /* take care of current line */
    line = VscrnGetLineFromTop(vmode,wherey[vmode]-1) ;
    for ( x=0 ; x < wherex[vmode] ; x++ )
        {
        line->cells[x].c = fillchar ;
        line->cells[x].a = cellcolor;
        line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
        }
}

void
clrline_escape( BYTE vmode, CHAR fillchar ) {
    videoline * line = NULL ;
    int x ;
    unsigned char cellcolor = geterasecolor(vmode) ;

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    /* take care of current line */
    line = VscrnGetLineFromTop(vmode,wherey[vmode]-1) ;
    for ( x=0 ; x < MAXTERMCOL ; x++ )
    {
        line->cells[x].c = fillchar ;
        line->cells[x].a = cellcolor;
        line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
    }
}

void
clrcol_escape( BYTE vmode, CHAR fillchar ) {
    int ys ;
    int x  = wherex[VTERM]-1 ;
    int y ;
    viocell cell = { fillchar, geterasecolor(vmode) } ;
    vtattrib vta ={0,0,0,0,0,0,0,0,0,0};

    if ( fillchar == NUL )
        cell.c = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( IS97801(tt_type_mode) ) {
        y = margintop-1;
        ys = marginbot-1;
    }
    else {
        ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
        y = 0;
    }
    for ( ; y<ys ; y++ )
        VscrnWrtCell( VTERM, cell, vta, y, x ) ;
}

/* Clears a rectangle from current cursor position to row,col */
/* using fillchar.                                            */
void
clrrect_escape( BYTE vmode, int top, int left, int bot, int right, CHAR fillchar )
{
    int startx, starty, endx, endy, l, x ;
    videoline * line = NULL ;
    unsigned char cellcolor = geterasecolor(vmode) ;

    if ( left < right ) {
        startx = left - 1 ;
        endx   = right - 1 ;
    }
    else {
        startx = right - 1 ;
        endx = left - 1 ;
    }

    if ( top < bot ) {
        starty = top - 1 ;
        endy   = bot - 1 ;
    }
    else {
        starty = bot - 1 ;
        endy = top - 1 ;
    }

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    /* check to see if any of the lines are protected */
    /* if so, abort                                   */

    for ( l=starty ; l <= endy ; l++ )
        if ( VscrnGetLineFromTop( vmode, l )->vt_line_attr & WY_LINE_ATTR_PROTECTED )
            return ;

    /* so now we just need to clear each row */

    for ( l=starty ; l <= endy ; l++ ) {
        line = VscrnGetLineFromTop( vmode, l ) ;
        for ( x=startx ; x <= endx ; x++ )
        {
            line->cells[x].c = fillchar ;
            line->cells[x].a = cellcolor;
            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
        }

    }
}

/* ----------------------------------------------------------------- */
/* Selective Clear Functions                                         */
/* ----------------------------------------------------------------- */
void
selclrscreen( BYTE vmode, CHAR fillchar ) {
    int             x=0, y=0, y2=0, linecount = VscrnGetBufferSize(vmode) ;
    videoline *     line=NULL, * newline = NULL ;

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    VscrnScroll(vmode,UPWARD,0,VscrnGetHeight(vmode)-(tt_status[vmode]?2:1),
                 VscrnGetHeight(vmode)-(tt_status[vmode]?1:0),TRUE,fillchar);

    /* Okay, so now we have scrolled the screen.  But the protected */
    /* fields need to be copied back to the new current screen      */

    for ( y = linecount - VscrnGetHeight(vmode) + (tt_status[vmode]?1:0) ;
          y < linecount ; y++,y2++ ) {
        line = VscrnGetLineFromTop( vmode,y ) ;
        newline = VscrnGetLineFromTop( vmode,y2 ) ;
        for ( x = 0 ; x < MAXTERMCOL ; x++ ) {
            if ( line->vt_char_attrs[x] & VT_CHAR_ATTR_PROTECTED ) {
                newline->cells[x] = line->cells[x] ;
                newline->vt_char_attrs[x] = line->vt_char_attrs[x] ;
                }
            }
        }
}


void
selclrtoeoln( BYTE vmode, CHAR fillchar ) {
    int x ;
    videoline * line = NULL ;
    unsigned char cellcolor = geterasecolor(vmode) ;

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    /* take care of current line */
    line = VscrnGetLineFromTop( vmode,wherey[vmode]-1 ) ;
    for ( x=wherex[vmode]-1 ; x < MAXTERMCOL ; x++ )
        {
        if ( !(line->vt_char_attrs[x] & VT_CHAR_ATTR_PROTECTED ) ) {
            line->cells[x].c = fillchar ;
            line->cells[x].a = cellcolor;
            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
            }
        }
}

void
selclreoscr_escape( BYTE vmode, CHAR fillchar ) {
    int x,y;
    videoline * line = NULL;
    unsigned char cellcolor = geterasecolor(vmode) ;

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if (wherex[vmode] == 1 && wherey[vmode] == 1) {
        selclrscreen(vmode, fillchar);
        return;
    }

    /* take care of current line */
    line = VscrnGetLineFromTop(vmode,wherey[vmode]-1) ;
    for ( x=wherex[vmode]-1 ; x <MAXTERMCOL ; x++ )
        {
        if ( !(line->vt_char_attrs[x] & VT_CHAR_ATTR_PROTECTED ) ) {
            line->cells[x].c = fillchar ;
            line->cells[x].a = cellcolor;
            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
            }
        }

    /* now take care of additional lines */
    for ( y=wherey[vmode] ; y<VscrnGetHeight(vmode)-(tt_status[vmode]?1:0) ; y++ )
        {
        line = VscrnGetLineFromTop(vmode,y) ;
        for ( x=0 ; x <MAXTERMCOL ; x++ )
            {
            if ( !(line->vt_char_attrs[x] & VT_CHAR_ATTR_PROTECTED ) ) {
                line->cells[x].c = fillchar ;
                line->cells[x].a = cellcolor;
                line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
                }
            }
        }
}

void
selclrboscr_escape( BYTE vmode, CHAR fillchar ) {
    int x,y;
    videoline * line = NULL;
    unsigned char cellcolor = geterasecolor(vmode) ;

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    /* now take care of first wherey[vmode]-1 lines */
    for ( y=0 ; y<wherey[vmode]-1 ; y++ )
        {
        line = VscrnGetLineFromTop(vmode,y) ;
        for ( x=0 ; x <MAXTERMCOL ; x++ )
            {
            if ( !(line->vt_char_attrs[x] & VT_CHAR_ATTR_PROTECTED ) ) {
                line->cells[x].c = fillchar ;
                line->cells[x].a = cellcolor;
                line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
                }
            }
        }

    /* take care of current line */
    line = VscrnGetLineFromTop(vmode,wherey[vmode]-1) ;
    for ( x=0 ; x < wherex[vmode] ; x++ )
        {
        if ( !(line->vt_char_attrs[x] & VT_CHAR_ATTR_PROTECTED ) ) {
            line->cells[x].c = fillchar ;
            line->cells[x].a = cellcolor;
            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
            }
        }
}

void
selclrbol_escape( BYTE vmode, CHAR fillchar ) {
    videoline * line = NULL ;
    int x ;
    unsigned char cellcolor = geterasecolor(vmode) ;

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    /* take care of current line */
    line = VscrnGetLineFromTop(vmode,wherey[vmode]-1) ;
    for ( x=0 ; x < wherex[vmode] ; x++ )
        {
        if ( !(line->vt_char_attrs[x] & VT_CHAR_ATTR_PROTECTED ) ) {
            line->cells[x].c = fillchar ;
            line->cells[x].a = cellcolor;
            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
            }
        }
}

void
selclrline_escape( BYTE vmode, CHAR fillchar ) {
    videoline * line = NULL ;
    int x ;
    unsigned char cellcolor = geterasecolor(vmode);

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    /* take care of current line */
    line = VscrnGetLineFromTop(vmode,wherey[vmode]-1) ;
    for ( x=0 ; x < MAXTERMCOL ; x++ )
    {
        if ( !(line->vt_char_attrs[x] & VT_CHAR_ATTR_PROTECTED ) ) {
            line->cells[x].c = fillchar ;
            line->cells[x].a = cellcolor;
            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
        }
    }
}


void
selclrcol_escape( BYTE vmode, CHAR fillchar ) {
    int ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
    int x  = wherex[VTERM]-1 ;
    int y ;
    viocell cell = { fillchar, geterasecolor(vmode) } ;
    vtattrib vta ={0,0,0,0,0,0,0,0,0,0};

    if ( fillchar == NUL )
        cell.c = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    for ( y=0 ; y<ys ; y++ ) {
        if ( !VscrnGetVtCharAttr(VTERM,x,y).unerasable )
            VscrnWrtCell( VTERM, cell, vta, y, x ) ;
    }
}

/* Clears a rectangle from current cursor position to row,col */
/* using fillchar.                                            */
void
selclrrect_escape( BYTE vmode, int top, int left, int bot, int right,
                   CHAR fillchar )
{
    int startx, starty, endx, endy, l, x ;
    videoline * line = NULL ;
    unsigned char cellcolor = geterasecolor(vmode) ;

    if ( left < right ) {
        startx = left - 1 ;
        endx   = right - 1 ;
    }
    else {
        startx = right - 1 ;
        endx = left - 1 ;
    }

    if ( top < bot ) {
        starty = top - 1 ;
        endy   = bot - 1 ;
    }
    else {
        starty = bot - 1 ;
        endy = top - 1 ;
    }

    if ( fillchar == NUL )
        fillchar = SP ;
    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    /* check to see if any of the lines are protected */
    /* if so, abort                                   */

    for ( l=starty ; l <= endy ; l++ )
        if ( VscrnGetLineFromTop( vmode, l )->vt_line_attr & WY_LINE_ATTR_PROTECTED )
            return ;

    /* so now we just need to clear each row */

    for ( l=starty ; l <= endy ; l++ ) {
        line = VscrnGetLineFromTop( vmode, l ) ;
        for ( x=startx ; x <= endx ; x++ )
        {
            if ( !(line->vt_char_attrs[x] & VT_CHAR_ATTR_PROTECTED ) ) {
                line->cells[x].c = fillchar ;
                line->cells[x].a = cellcolor;
                line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL ;
            }
        }
    }
}

/* Draws a box from current cursor to row,col */
void
boxrect_escape( BYTE vmode, int row, int col )
{
    int brow, bcol, erow, ecol, x, y ;
    viocell cell ;
    vtattrib vta = {0,0,0,0,0,0,0,0,0,0,0};

    if ( vmode == VTERM ) {
        cell.a = attribute ;
        vta = attrib ;
    }
    else
        cell.a = colorcmd ;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( wherey[vmode] < row ) {
        brow = wherey[vmode] ;
        erow = row ;
    }
    else {
        brow = row ;
        erow = wherey[vmode] ;
    }

    if ( wherex[vmode] < col ) {
        bcol = wherex[vmode] ;
        ecol = col ;
    }
    else {
        bcol = col ;
        ecol = wherex[vmode] ;
    }

    cell.c = '|' ;
    for ( y = brow ; y <= erow ; y++ ) {
        VscrnWrtCell( vmode, cell, vta, y, bcol ) ;
    }
    for ( y = brow ; y <= erow ; y++ ) {
        VscrnWrtCell( vmode, cell, vta, y, ecol ) ;
    }
    cell.c = '-' ;
    for ( x = bcol ; x <= ecol ; x++ ) {
        VscrnWrtCell( vmode, cell, vta, brow, x ) ;
    }
    for ( x = bcol ; x <= ecol ; x++ ) {
        VscrnWrtCell( vmode, cell, vta, erow, x ) ;
    }
}

void
decdwl_escape(bool dwlflag) {
    videoline * line = NULL ;
    unsigned char   linenumber=0;
    unsigned char   newx=0;
    viocell       * cells = NULL ;
    int             i=0;
    char buffer[MAXTERMCOL+1] ;

    /* DECDWL */
    line = VscrnGetLineFromTop(VTERM,wherey[VTERM]-1) ;
    cells = line->cells ;

     if ( dwlflag != line->vt_line_attr ) {
         /* change size */
       line->vt_line_attr = dwlflag ;
    }
}

void
udkreset( void )
{
    int i;
    udklock = FALSE;                   /* Unlock User Defined Keys */
    for ( i=0 ; i<200 ; i++ )          /* Reset the User Defined Keys */
    {
       if ( udkfkeys[i] )
         free(udkfkeys[i]);
        udkfkeys[i] = NULL ;
    }

    if ( ISTVI(tt_type_mode) ) {
        udkfkeys[0]  = strdup("\01@\r");        /* F1 */
        udkfkeys[1]  = strdup("\01A\r");
        udkfkeys[2]  = strdup("\01B\r");
        udkfkeys[3]  = strdup("\01C\r");
        udkfkeys[4]  = strdup("\01D\r");
        udkfkeys[5]  = strdup("\01E\r");
        udkfkeys[6]  = strdup("\01F\r");
        udkfkeys[7]  = strdup("\01G\r");
        udkfkeys[8]  = strdup("\01H\r");
        udkfkeys[9]  = strdup("\01I\r");
        udkfkeys[10] = strdup("\01J\r");
        udkfkeys[11] = strdup("\01K\r");
        udkfkeys[12] = strdup("\01L\r");
        udkfkeys[13] = strdup("\01M\r");
        udkfkeys[14] = strdup("\01N\r");
        udkfkeys[15] = strdup("\01O\r");        /* F16 */
        udkfkeys[16] = strdup("\01`\r");        /* Shift-F1 */
        udkfkeys[17] = strdup("\01a\r");
        udkfkeys[18] = strdup("\01b\r");
        udkfkeys[19] = strdup("\01c\r");
        udkfkeys[20] = strdup("\01d\r");
        udkfkeys[21] = strdup("\01e\r");
        udkfkeys[22] = strdup("\01f\r");
        udkfkeys[23] = strdup("\01g\r");
        udkfkeys[24] = strdup("\01h\r");
        udkfkeys[25] = strdup("\01i\r");
        udkfkeys[26] = strdup("\01j\r");
        udkfkeys[27] = strdup("\01k\r");
        udkfkeys[28] = strdup("\01l\r");
        udkfkeys[29] = strdup("\01m\r");
        udkfkeys[30] = strdup("\01n\r");
        udkfkeys[31] = strdup("\01o\r");        /* Shift-F16 */
        udkfkeys[32] = strdup("\x08");          /* Backspace */
        udkfkeys[33] = strdup("\x1BT");         /* Clr Line */
        udkfkeys[34] = strdup("\x1Bt");         /* Shift Clr Line */
        udkfkeys[35] = strdup("\x1BY");         /* Clr Page */
        udkfkeys[36] = strdup("\x1By");         /* Shift Clr Page */
        udkfkeys[37] = strdup("\x1BW");         /* Del Char */
        udkfkeys[38] = strdup("\x1BR");         /* Del Line */
        udkfkeys[39] = strdup("\x0D");          /* Enter */
        udkfkeys[40] = strdup("\x1B");          /* Escape */
        udkfkeys[41] = strdup("\x1E");          /* Home */
        udkfkeys[42] = strdup("\x1B{");         /* Shift Home */
        udkfkeys[43] = strdup("\x1Bq");         /* Insert */
        udkfkeys[44] = strdup("\x1BQ");         /* Ins Char */
        udkfkeys[45] = strdup("\x1B\x45");      /* Ins Line */
        udkfkeys[46] = strdup("\x1BK");         /* Page Next*/
        udkfkeys[47] = strdup("\x1BJ");         /* Page Prev*/
        udkfkeys[48] = strdup("\x1Br");         /* Replace */
        udkfkeys[49] = strdup("\x0D");          /* Return */
        udkfkeys[50] = strdup("\x09");          /* Tab */
        udkfkeys[51] = strdup("\x1BI");         /* Shift Tab */
        udkfkeys[52] = strdup("\x1BP");         /* Print Screen */
        udkfkeys[53] = NULL;                    /* Shift Esc */
        udkfkeys[54] = strdup("\x08");          /* Shift BS */
        udkfkeys[55] = NULL ;                   /* Shift Enter */
        udkfkeys[56] = NULL ;                   /* Shift Return */
        udkfkeys[57] = strdup("\x0b");          /* Up Arrow - VT */
        udkfkeys[58] = strdup("\x16");          /* Dn Arrow - SYN */
        udkfkeys[59] = strdup("\x08");          /* Left Arrow - BS */
        udkfkeys[60] = strdup("\x0C");          /* Right Arrow - FF */
        udkfkeys[61] = NULL ;                   /* Shift Up Arrow */
        udkfkeys[62] = NULL ;                   /* Shift Dn Arrow */
        udkfkeys[63] = NULL ;                   /* Shift Left Arrow */
        udkfkeys[64] = NULL ;                   /* Shift Right Arrow */
        udkfkeys[65] = strdup("\0337");         /* Send */
        udkfkeys[66] = strdup("\0336");         /* Shift-Send */
    }

    if ( ISWYSE(tt_type_mode) ) {
        udkfkeys[0]  = strdup("\01@\r");        /* F1 */
        udkfkeys[1]  = strdup("\01A\r");
        udkfkeys[2]  = strdup("\01B\r");
        udkfkeys[3]  = strdup("\01C\r");
        udkfkeys[4]  = strdup("\01D\r");
        udkfkeys[5]  = strdup("\01E\r");
        udkfkeys[6]  = strdup("\01F\r");
        udkfkeys[7]  = strdup("\01G\r");
        udkfkeys[8]  = strdup("\01H\r");
        udkfkeys[9]  = strdup("\01I\r");
        udkfkeys[10] = strdup("\01J\r");
        udkfkeys[11] = strdup("\01K\r");
        udkfkeys[12] = strdup("\01L\r");
        udkfkeys[13] = strdup("\01M\r");
        udkfkeys[14] = strdup("\01N\r");
        udkfkeys[15] = strdup("\01O\r");
        udkfkeys[16] = strdup("\01P\r");
        udkfkeys[17] = strdup("\01Q\r");
        udkfkeys[18] = strdup("\01R\r");
        udkfkeys[19] = strdup("\01S\r");        /* F20 */

        udkfkeys[20] = strdup("\01`\r");        /* Shift-F1 */
        udkfkeys[21] = strdup("\01a\r");
        udkfkeys[22] = strdup("\01b\r");
        udkfkeys[23] = strdup("\01c\r");
        udkfkeys[24] = strdup("\01d\r");
        udkfkeys[25] = strdup("\01e\r");
        udkfkeys[26] = strdup("\01f\r");
        udkfkeys[27] = strdup("\01g\r");
        udkfkeys[28] = strdup("\01h\r");
        udkfkeys[29] = strdup("\01i\r");
        udkfkeys[30] = strdup("\01j\r");
        udkfkeys[31] = strdup("\01k\r");
        udkfkeys[32] = strdup("\01l\r");
        udkfkeys[33] = strdup("\01m\r");
        udkfkeys[34] = strdup("\01n\r");
        udkfkeys[35] = strdup("\01o\r");
        udkfkeys[36] = strdup("\01p\r");
        udkfkeys[37] = strdup("\01q\r");
        udkfkeys[38] = strdup("\01r\r");
        udkfkeys[39] = strdup("\01s\r");        /* Shift-F20 */

        udkfkeys[40] = strdup("\x08");          /* Backspace */
        udkfkeys[41] = strdup("\x1BT");         /* Clr Line */
        udkfkeys[42] = strdup("\x1Bt");         /* Shift Clr Line */
        udkfkeys[43] = strdup("\x1BY");         /* Clr Page */
        udkfkeys[44] = strdup("\x1By");         /* Shift Clr Page */
        udkfkeys[45] = strdup("\x1BW");         /* Del Char */
        udkfkeys[46] = strdup("\x1BR");         /* Del Line */
        udkfkeys[47] = strdup("\x0D");          /* Enter */
        udkfkeys[48] = strdup("\x1B");          /* Escape */
        udkfkeys[49] = strdup("\x1E");          /* Home */
        udkfkeys[50] = strdup("\x1B{");         /* Shift Home */
        udkfkeys[51] = strdup("\x1Bq");         /* Insert */
        udkfkeys[52] = strdup("\x1BQ");         /* Ins Char */
        udkfkeys[53] = strdup("\x1B\x45");      /* Ins Line */
        udkfkeys[54] = strdup("\x1BK");         /* Page Next*/
        udkfkeys[55] = strdup("\x1BJ");         /* Page Prev*/
        udkfkeys[56] = strdup("\x1Br");         /* Replace */
        udkfkeys[57] = strdup("\x0D");          /* Return */
        udkfkeys[58] = strdup("\x09");          /* Tab */
        udkfkeys[59] = strdup("\x1BI");         /* Shift Tab */
        udkfkeys[60] = strdup("\x1BP");         /* Print Screen */
        udkfkeys[61] = NULL;                    /* Shift Esc */
        udkfkeys[62] = strdup("\x08");          /* Shift BS */
        udkfkeys[63] = NULL ;                   /* Shift Enter */
        udkfkeys[64] = NULL ;                   /* Shift Return */
        udkfkeys[65] = strdup("\x0b");          /* Up Arrow - VT */
        udkfkeys[66] = strdup("\x0a");          /* Dn Arrow - LF */
        udkfkeys[67] = strdup("\x08");          /* Left Arrow - BS */
        udkfkeys[68] = strdup("\x0c");          /* Right Arrow - NL */
        udkfkeys[69] = NULL ;                   /* Shift Up Arrow */
        udkfkeys[70] = NULL ;                   /* Shift Dn Arrow */
        udkfkeys[71] = NULL ;                   /* Shift Left Arrow */
        udkfkeys[72] = NULL ;                   /* Shift Right Arrow */
        udkfkeys[73] = strdup("\x1B\x37");      /* Send */
        udkfkeys[74] = NULL ;                   /* Shift Send */
    }

    if ( ISAT386(tt_type_mode) ) {
        udkfkeys[0]  = strdup("\x1BOP");
        udkfkeys[1]  = strdup("\x1BOQ");
        udkfkeys[2]  = strdup("\x1BOR");
        udkfkeys[3]  = strdup("\x1BOS");
        udkfkeys[4]  = strdup("\x1BOT");
        udkfkeys[5]  = strdup("\x1BOU");
        udkfkeys[6]  = strdup("\x1BOV");
        udkfkeys[7]  = strdup("\x1BOW");
        udkfkeys[8]  = strdup("\x1BOX");
        udkfkeys[9]  = strdup("\x1BOY");
        udkfkeys[10] = strdup("\x1BOZ");
        udkfkeys[11] = strdup("\x1BOA");
        udkfkeys[12] = strdup("\x1BOp");
        udkfkeys[13] = strdup("\x1BOq");
        udkfkeys[14] = strdup("\x1BOr");
        udkfkeys[15] = strdup("\x1BOs");
        udkfkeys[16] = strdup("\x1BOt");
        udkfkeys[17] = strdup("\x1BOu");
        udkfkeys[18] = strdup("\x1BOv");
        udkfkeys[19] = strdup("\x1BOw");
        udkfkeys[20] = strdup("\x1BOx");
        udkfkeys[21] = strdup("\x1BOy");
        udkfkeys[22] = strdup("\x1BOz");
        udkfkeys[23] = strdup("\x1BOa");
        udkfkeys[24] = strdup("\x1BOP");
        udkfkeys[25] = strdup("\x1BOQ");
        udkfkeys[26] = strdup("\x1BOR");
        udkfkeys[27] = strdup("\x1BOS");
        udkfkeys[28] = strdup("\x1BOT");
        udkfkeys[29] = strdup("\x1BOU");
        udkfkeys[30] = strdup("\x1BOV");
        udkfkeys[31] = strdup("\x1BOW");
        udkfkeys[32] = strdup("\x1BOX");
        udkfkeys[33] = strdup("\x1BOY");
        udkfkeys[34] = strdup("\x1BOZ");
        udkfkeys[35] = strdup("\x1BOA");
        udkfkeys[36] = strdup("\x1BOp");
        udkfkeys[37] = strdup("\x1BOq");
        udkfkeys[38] = strdup("\x1BOr");
        udkfkeys[39] = strdup("\x1BOs");
        udkfkeys[40] = strdup("\x1BOt");
        udkfkeys[41] = strdup("\x1BOu");
        udkfkeys[42] = strdup("\x1BOv");
        udkfkeys[43] = strdup("\x1BOw");
        udkfkeys[44] = strdup("\x1BOx");
        udkfkeys[45] = strdup("\x1BOy");
        udkfkeys[46] = strdup("\x1BOz");
        udkfkeys[47] = strdup("\x1BOa");
        udkfkeys[48] = strdup("\x1B[H");
        udkfkeys[49] = strdup("\x1B[A");
        udkfkeys[50] = strdup("\x1B[V");
        udkfkeys[51] = strdup("\x1B[S");
        udkfkeys[52] = strdup("\x1B[D");
        udkfkeys[53] = strdup("\x1B[G");
        udkfkeys[54] = strdup("\x1B[C");
        udkfkeys[55] = strdup("\x1B[T");
        udkfkeys[56] = strdup("\x1B[Y");
        udkfkeys[57] = strdup("\x1B[B");
        udkfkeys[58] = strdup("\x1B[U");
        udkfkeys[59] = strdup("\x1B[@");
        udkfkeys[60] = strdup("\x1B[2");
    }

    if ( ISSCO(tt_type_mode) ) {
        udkfkeys[0]  = strdup("\x1B[M");
        udkfkeys[1]  = strdup("\x1B[N");
        udkfkeys[2]  = strdup("\x1B[O");
        udkfkeys[3]  = strdup("\x1B[P");
        udkfkeys[4]  = strdup("\x1B[Q");
        udkfkeys[5]  = strdup("\x1B[R");
        udkfkeys[6]  = strdup("\x1B[S");
        udkfkeys[7]  = strdup("\x1B[T");
        udkfkeys[8]  = strdup("\x1B[U");
        udkfkeys[9]  = strdup("\x1B[V");
        udkfkeys[10] = strdup("\x1B[W");
        udkfkeys[11] = strdup("\x1B[X");
        udkfkeys[12] = strdup("\x1B[Y");
        udkfkeys[13] = strdup("\x1B[Z");
        udkfkeys[14] = strdup("\x1B[a");
        udkfkeys[15] = strdup("\x1B[b");
        udkfkeys[16] = strdup("\x1B[c");
        udkfkeys[17] = strdup("\x1B[d");
        udkfkeys[18] = strdup("\x1B[e");
        udkfkeys[19] = strdup("\x1B[f");
        udkfkeys[20] = strdup("\x1B[g");
        udkfkeys[21] = strdup("\x1B[h");
        udkfkeys[22] = strdup("\x1B[i");
        udkfkeys[23] = strdup("\x1B[j");
        udkfkeys[24] = strdup("\x1B[k");
        udkfkeys[25] = strdup("\x1B[l");
        udkfkeys[26] = strdup("\x1B[m");
        udkfkeys[27] = strdup("\x1B[n");
        udkfkeys[28] = strdup("\x1B[o");
        udkfkeys[29] = strdup("\x1B[p");
        udkfkeys[30] = strdup("\x1B[q");
        udkfkeys[31] = strdup("\x1B[r");
        udkfkeys[32] = strdup("\x1B[s");
        udkfkeys[33] = strdup("\x1B[t");
        udkfkeys[34] = strdup("\x1B[u");
        udkfkeys[35] = strdup("\x1B[v");
        udkfkeys[36] = strdup("\x1B[w");
        udkfkeys[37] = strdup("\x1B[x");
        udkfkeys[38] = strdup("\x1B[y");
        udkfkeys[39] = strdup("\x1B[z");
        udkfkeys[40] = strdup("\x1B[@");
        udkfkeys[41] = strdup("\x1B[[");
        udkfkeys[42] = strdup("\x1B[\\");
        udkfkeys[43] = strdup("\x1B[]");
        udkfkeys[44] = strdup("\x1B[^");
        udkfkeys[45] = strdup("\x1B[_");
        udkfkeys[46] = strdup("\x1B[`");
        udkfkeys[47] = strdup("\x1B[{");
        udkfkeys[48] = strdup("\x1B[H");
        udkfkeys[49] = strdup("\x1B[A");
        udkfkeys[50] = strdup("\x1B[I");
        udkfkeys[51] = strdup("-");
        udkfkeys[52] = strdup("\x1B[D");
        udkfkeys[53] = strdup("\x1B[E");
        udkfkeys[54] = strdup("\x1B[C");
        udkfkeys[55] = strdup("+");
        udkfkeys[56] = strdup("\x1B[F");
        udkfkeys[57] = strdup("\x1B[B");
        udkfkeys[58] = strdup("\x1B[G");
        udkfkeys[59] = strdup("\x1B[L");
    }

    if ( ISLINUX(tt_type_mode) ) {
        udkfkeys[0]  = strdup("\x1B[[A");
        udkfkeys[1]  = strdup("\x1B[[B");
        udkfkeys[2]  = strdup("\x1B[[C");
        udkfkeys[3]  = strdup("\x1B[[D");
        udkfkeys[4]  = strdup("\x1B[[E");
        udkfkeys[5]  = strdup("\x1B[17~");
        udkfkeys[6]  = strdup("\x1B[18~");
        udkfkeys[7]  = strdup("\x1B[19~");
        udkfkeys[8]  = strdup("\x1B[20~");
        udkfkeys[9]  = strdup("\x1B[21~");
        udkfkeys[10] = strdup("\x1B[23~");
        udkfkeys[11] = strdup("\x1B[24~");
        udkfkeys[12] = strdup("\x1B[25~");
        udkfkeys[13] = strdup("\x1B[26~");
        udkfkeys[14] = strdup("\x1B[28~");
        udkfkeys[15] = strdup("\x1B[29~");
        udkfkeys[16] = strdup("\x1B[31~");
        udkfkeys[17] = strdup("\x1B[32~");
        udkfkeys[18] = strdup("\x1B[33~");
        udkfkeys[19] = strdup("\x1B[34~");
        udkfkeys[20] = strdup("\x1B[35~");
        udkfkeys[21] = strdup("\x1B[36~");
        udkfkeys[22] = strdup("\x1B[37~");
        udkfkeys[23] = strdup("\x1B[38~");
        udkfkeys[24] = strdup("\x1B[39~");
        udkfkeys[25] = strdup("\x1B[40~");
        udkfkeys[26] = strdup("\x1B[41~");
        udkfkeys[27] = strdup("\x1B[42~");
        udkfkeys[28] = strdup("\x1B[43~");
        udkfkeys[29] = strdup("\x1B[44~");
        udkfkeys[30] = strdup("\x1B[45~");
        udkfkeys[31] = strdup("\x1B[46~");
        udkfkeys[32] = strdup("\x1B[47~");
        udkfkeys[33] = strdup("\x1B[48~");
        udkfkeys[34] = strdup("\x1B[49~");
        udkfkeys[35] = strdup("\x1B[50~");
        udkfkeys[36] = strdup("\x1B[51~");
        udkfkeys[37] = strdup("\x1B[52~");
        udkfkeys[38] = strdup("\x1B[53~");
        udkfkeys[39] = strdup("\x1B[54~");
        udkfkeys[40] = strdup("\x1B[55~");
        udkfkeys[41] = strdup("\x1B[56~");
        udkfkeys[42] = strdup("\x1B[57~");
        udkfkeys[43] = strdup("\x1B[58~");
        udkfkeys[44] = strdup("\x1B[59~");
        udkfkeys[45] = strdup("\x1B[60~");
        udkfkeys[46] = strdup("\x1B[61~");
        udkfkeys[47] = strdup("\x1B[62~");
        udkfkeys[48] = strdup("\x1B[1~");       /* home */
        udkfkeys[49] = strdup("\x1B[A");        /* up */
        udkfkeys[50] = strdup("\x1B[5~");
        udkfkeys[51] = strdup("-");
        udkfkeys[52] = strdup("\x1B[D");
        udkfkeys[53] = strdup("\x1B[G");
        udkfkeys[54] = strdup("\x1B[C");
        udkfkeys[55] = strdup("+");
        udkfkeys[56] = strdup("\x1B[4~");
        udkfkeys[57] = strdup("\x1B[B");
        udkfkeys[58] = strdup("\x1B[6~");
        udkfkeys[59] = strdup("\x1B[2~");
        udkfkeys[60] = strdup("\x1B[3~");
    }

    if ( ISHP(tt_type_mode) ) {
        udkfkeys[0]  = strdup("\x1Bp");
        udkfkeys[1]  = strdup("\x1Bq");
        udkfkeys[2]  = strdup("\x1Br");
        udkfkeys[3]  = strdup("\x1Bs");
        udkfkeys[4]  = strdup("\x1Bt");
        udkfkeys[5]  = strdup("\x1Bu");
        udkfkeys[6]  = strdup("\x1Bv");
        udkfkeys[7]  = strdup("\x1Bw");
        udkfkeys[8]  = NULL;
        udkfkeys[9]  = NULL;
        udkfkeys[10] = NULL;
        udkfkeys[11] = NULL;
        udkfkeys[12] = NULL;
        udkfkeys[13] = NULL;
        udkfkeys[14] = NULL;
        udkfkeys[15] = NULL;
        udkfkeys[16] = strdup("\x1Bi");
        udkfkeys[17] = strdup("\r");
        udkfkeys[18] = strdup("\r");
    }

    if ( ISDG200(tt_type_mode) ) {
        /* function keys */
        udkfkeys[0]  = strdup("\036q");
        udkfkeys[1]  = strdup("\036r");
        udkfkeys[2]  = strdup("\036s");
        udkfkeys[3]  = strdup("\036t");
        udkfkeys[4]  = strdup("\036u");
        udkfkeys[5]  = strdup("\036v");
        udkfkeys[6]  = strdup("\036w");
        udkfkeys[7]  = strdup("\036x");
        udkfkeys[8]  = strdup("\036y");
        udkfkeys[9]  = strdup("\036z");
        udkfkeys[10] = strdup("\036{");
        udkfkeys[11] = strdup("\036|");
        udkfkeys[12] = strdup("\036}");
        udkfkeys[13] = strdup("\036~");
        udkfkeys[14] = strdup("\036p");
        udkfkeys[15] = strdup("\036a");
        udkfkeys[16] = strdup("\036b");
        udkfkeys[17] = strdup("\036c");
        udkfkeys[18] = strdup("\036d");
        udkfkeys[19] = strdup("\036e");
        udkfkeys[20] = strdup("\036f");
        udkfkeys[21] = strdup("\036g");
        udkfkeys[22] = strdup("\036h");
        udkfkeys[23] = strdup("\036i");
        udkfkeys[24] = strdup("\036j");
        udkfkeys[25] = strdup("\036k");
        udkfkeys[26] = strdup("\036l");
        udkfkeys[27] = strdup("\036m");
        udkfkeys[28] = strdup("\036n");
        udkfkeys[29] = strdup("\036`");
        udkfkeys[30] = strdup("\0361");
        udkfkeys[31] = strdup("\0362");
        udkfkeys[32] = strdup("\0363");
        udkfkeys[33] = strdup("\0364");
        udkfkeys[34] = strdup("\0365");
        udkfkeys[35] = strdup("\0366");
        udkfkeys[36] = strdup("\0367");
        udkfkeys[37] = strdup("\0368");
        udkfkeys[38] = strdup("\0369");
        udkfkeys[39] = strdup("\036:");
        udkfkeys[40] = strdup("\036;");
        udkfkeys[41] = strdup("\036<");
        udkfkeys[42] = strdup("\036=");
        udkfkeys[43] = strdup("\036>");
        udkfkeys[44] = strdup("\0360");
        udkfkeys[45] = strdup("\036!");
        udkfkeys[46] = strdup("\036\"");
        udkfkeys[47] = strdup("\036#");
        udkfkeys[48] = strdup("\036$");
        udkfkeys[49] = strdup("\036%");
        udkfkeys[50] = strdup("\036&");
        udkfkeys[51] = strdup("\036'");
        udkfkeys[52] = strdup("\036(");
        udkfkeys[53] = strdup("\036)");
        udkfkeys[54] = strdup("\036*");
        udkfkeys[55] = strdup("\036+");
        udkfkeys[56] = strdup("\036,");
        udkfkeys[57] = strdup("\036-");
        udkfkeys[58] = strdup("\036.");
        udkfkeys[59] = strdup("\036 ");

        /* arrow keys - native */
        udkfkeys[60] = strdup("\027");
        udkfkeys[61] = strdup("\032");
        udkfkeys[62] = strdup("\031");
        udkfkeys[63] = strdup("\030");
        udkfkeys[64] = strdup("\036\027");
        udkfkeys[65] = strdup("\036\032");
        udkfkeys[66] = strdup("\036\031");
        udkfkeys[67] = strdup("\036\030");

        /* other - native */
        udkfkeys[68] = strdup("\014");
        udkfkeys[69] = strdup("\036\\");
        udkfkeys[70] = strdup("\036]");
        udkfkeys[71] = strdup("\013");
        udkfkeys[72] = strdup("\036^");
        udkfkeys[73] = strdup("\036_");
        udkfkeys[74] = strdup("\036\021");
        udkfkeys[75] = strdup("\010");
        udkfkeys[76] = strdup("\014");
        udkfkeys[77] = strdup("\036X");
        udkfkeys[78] = strdup("\036Y");
        udkfkeys[79] = strdup("\013");
        udkfkeys[80] = strdup("\036Z");
        udkfkeys[81] = strdup("\036{");
        udkfkeys[82] = strdup("\036\001");
        udkfkeys[83] = strdup("\031");
        udkfkeys[84] = strdup("\036\010");

        /* arrow keys - unix mode */
        udkfkeys[85] = strdup("\036PA");
        udkfkeys[86] = strdup("\036PB");
        udkfkeys[87] = strdup("\036PD");
        udkfkeys[88] = strdup("\036PC");
        udkfkeys[89] = strdup("\036Pa");
        udkfkeys[90] = strdup("\036Pb");
        udkfkeys[91] = strdup("\036Pd");
        udkfkeys[92] = strdup("\036Pc");

        /* other - unix mode */
        udkfkeys[93] = strdup("\036PH");
        udkfkeys[94] = strdup("\036\\");
        udkfkeys[95] = strdup("\036]");
        udkfkeys[96] = strdup("\036PE");
        udkfkeys[97] = strdup("\036^");
        udkfkeys[98] = strdup("\036_");
        udkfkeys[99] = strdup("\036P0");
        udkfkeys[100] = strdup("\036PF");
        udkfkeys[101] = strdup("\036PH");
        udkfkeys[102] = strdup("\036X");
        udkfkeys[103] = strdup("\036Y");
        udkfkeys[104] = strdup("\036PE");
        udkfkeys[105] = strdup("\036Z");
        udkfkeys[106] = strdup("\036[");
        udkfkeys[107] = strdup("\036P1");
        udkfkeys[108] = strdup("\177");
        udkfkeys[109] = strdup("\036Pf");
    }

    if ( IS97801(tt_type_mode) ) {
        udkfkeys[0]   = strdup("\033@");
        udkfkeys[1]   = strdup("\033A");
        udkfkeys[2]   = strdup("\033B");
        udkfkeys[3]   = strdup("\033C");
        udkfkeys[4]   = strdup("\033D");
        udkfkeys[5]   = strdup("\033F");
        udkfkeys[6]   = strdup("\033G");
        udkfkeys[7]   = strdup("\033H");
        udkfkeys[8]   = strdup("\033I");
        udkfkeys[9]   = strdup("\033J");
        udkfkeys[10]  = strdup("\033K");
        udkfkeys[11]  = strdup("\033L");
        udkfkeys[12]  = strdup("\033M");
        udkfkeys[13]  = strdup("\033N");
        udkfkeys[14]  = strdup("\033O");
        udkfkeys[15]  = strdup("\033P");
        udkfkeys[16]  = strdup("\0330");
        udkfkeys[17]  = strdup("\033_");
        udkfkeys[18]  = strdup("\033d");
        udkfkeys[19]  = strdup("\033T");
        udkfkeys[20]  = strdup("\033V");
        udkfkeys[21]  = strdup("\033X");
        udkfkeys[22]  = strdup("\0334");
        udkfkeys[23]  = strdup("\033g");
        udkfkeys[24]  = strdup("\x09");
        udkfkeys[25]  = strdup("\033[Z");
        udkfkeys[26]  = strdup("\033^");
        udkfkeys[27]  = strdup("\033>");
        udkfkeys[28]  = strdup("\033m");
        udkfkeys[29]  = strdup("\004");
        udkfkeys[30]  = strdup("\033[S");
        udkfkeys[31]  = strdup("\033[T");
        udkfkeys[32]  = strdup("\033[H");
        udkfkeys[33]  = strdup("\033[P");
        udkfkeys[34]  = strdup("\033[A");
        udkfkeys[35]  = strdup("\033p");
        udkfkeys[36]  = strdup("\033[D");
        udkfkeys[37]  = strdup("\033!");
        udkfkeys[38]  = strdup("\033[C");
        udkfkeys[39]  = strdup("\033[M");
        udkfkeys[40]  = strdup("\033[B");
        udkfkeys[41]  = strdup("00");
        udkfkeys[42]  = strdup("\033z");
        udkfkeys[43]  = strdup("\033]");
        udkfkeys[44]  = strdup("\033[@");
        udkfkeys[45]  = strdup("\033o");
        udkfkeys[46]  = strdup("\033[L");
        udkfkeys[47]  = strdup("\0336");
        udkfkeys[48]  = strdup("\0337");
        udkfkeys[49]  = strdup("\0338");
        udkfkeys[50]  = strdup("\033l");
        udkfkeys[51]  = strdup("\033Z");
        udkfkeys[52]  = strdup("\033 ");
        udkfkeys[53]  = strdup("\033;");
        udkfkeys[54]  = strdup("\033\"");
        udkfkeys[55]  = strdup("\033#");
        udkfkeys[56]  = strdup("\033$");
        udkfkeys[57]  = strdup("\033%");
        udkfkeys[58]  = strdup("\033&");
        udkfkeys[59]  = strdup("\033\\");
        udkfkeys[60]  = strdup("\033<");
        udkfkeys[61]  = strdup("\033=");
        udkfkeys[62]  = strdup("\033 ");
        udkfkeys[63]  = strdup("\033+");
        udkfkeys[64]  = strdup("\033,");
        udkfkeys[65]  = strdup("\033_");
        udkfkeys[66]  = strdup("\033.");
        udkfkeys[67]  = strdup("\033/");
        udkfkeys[68]  = strdup("\0331");
        udkfkeys[69]  = strdup("\0332");
        udkfkeys[70]  = strdup("\0333");
        udkfkeys[71]  = strdup("\033U");
        udkfkeys[72]  = strdup("\033W");
        udkfkeys[73]  = strdup("\033Y");
        udkfkeys[74]  = strdup("\0335");
        udkfkeys[75]  = strdup("\033g");
        udkfkeys[76]  = strdup("\033[Z");
        udkfkeys[77]  = strdup("\033[Z");
        udkfkeys[78]  = strdup("\033^");
        udkfkeys[79]  = strdup("\033>");
        udkfkeys[80]  = strdup("\033m");
        udkfkeys[81]  = strdup("\033~");
        udkfkeys[82]  = strdup("\033[S");
        udkfkeys[83]  = strdup("\033[T");
        udkfkeys[84]  = strdup("\033q");
        udkfkeys[85]  = strdup("\033[P");
        udkfkeys[86]  = strdup("\033[A");
        udkfkeys[87]  = strdup("\033p");
        udkfkeys[88]  = strdup("\0339");
        udkfkeys[89]  = strdup("\033~");
        udkfkeys[90]  = strdup("\033:");
        udkfkeys[91]  = strdup("\033[M");
        udkfkeys[92]  = strdup("\033[B");
        udkfkeys[93]  = strdup("00");
        udkfkeys[94]  = strdup("\033z");
        udkfkeys[95]  = strdup("\033]");
        udkfkeys[96]  = strdup("\033[@");
        udkfkeys[97]  = strdup("\033o");
        udkfkeys[98]  = strdup("\033[L");
        udkfkeys[99]  = strdup("\0336");
        udkfkeys[100]  = strdup("\0337");
        udkfkeys[101]  = strdup("\0338");
        udkfkeys[102] = strdup("\033l");
        udkfkeys[103] = strdup("\033Z");
        /* Ctrl key definitions default to NULL */
    }

    if ( ISBA80(tt_type_mode) ) {
        udkfkeys[0]   = strdup("\033[=N1\015");
        udkfkeys[1]   = strdup("\033[=N2\015");
        udkfkeys[2]   = strdup("\033[=N3\015");
        udkfkeys[3]   = strdup("\033[=N4\015");
        udkfkeys[4]   = strdup("\033[=N5\015");
        udkfkeys[5]   = strdup("\033[=N6\015");
        udkfkeys[6]   = strdup("\033[=N7\015");
        udkfkeys[7]   = strdup("\033[=N8\015");
        udkfkeys[8]   = strdup("\033[=N9\015");
        udkfkeys[9]   = strdup("\033[=>a");
        udkfkeys[10]  = strdup("\033[=>b");
        udkfkeys[11]  = strdup("\033[=>c");
        udkfkeys[12]  = strdup("\033[=>d");
        udkfkeys[13]  = strdup("\033[=>e");
        udkfkeys[14]  = strdup("\033[=>f");
        udkfkeys[15]  = strdup("\033[=>g");
        udkfkeys[16]  = strdup("\033[=>h");
        udkfkeys[17]  = strdup("\033[=>i");
        udkfkeys[18]  = strdup("\033[=>j");
        udkfkeys[19]  = strdup("\033[=>k");
        udkfkeys[20]  = strdup("\033[=>l");
        udkfkeys[21]  = strdup("\033[=>m");
        udkfkeys[22]  = strdup("\033[=>n");
        udkfkeys[23]  = strdup("\033[=>o");
        udkfkeys[24]  = strdup("\033[=>p");
        udkfkeys[25]  = strdup("\033[=>q");
        udkfkeys[26]  = strdup("\033[=>r");
        udkfkeys[27]  = strdup("\033[=>s");
        udkfkeys[28]  = strdup("\033[=>t");
        udkfkeys[29]  = strdup("\033[=>u");
        udkfkeys[30]  = strdup("\033[=>v");
        udkfkeys[31]  = strdup("\033[=>w");
        udkfkeys[32]  = strdup("\033[=>x");
        udkfkeys[33]  = strdup("\033[=<0");
        udkfkeys[34]  = strdup("\033[=<1");
        udkfkeys[35]  = strdup("\033[=<2");
        udkfkeys[36]  = strdup("\033[=<4");
        udkfkeys[37]  = strdup("\033[=<5");
        udkfkeys[38]  = strdup("\033[=<6");
        udkfkeys[39]  = strdup("\033[=<7");
        udkfkeys[40]  = strdup("\033[=<8");
        udkfkeys[41]  = strdup("\033[=<9");
        udkfkeys[42]  = strdup("\033[=<f");
        udkfkeys[43]  = strdup("\033[=<g");
        udkfkeys[44]  = strdup("\033[=<p");
        udkfkeys[45]  = strdup("\033[=<B");
        udkfkeys[46]  = strdup("\033[=<F");
        udkfkeys[47]  = strdup("\033[=<G");
        udkfkeys[48]  = strdup("\033[=<H");
        udkfkeys[49]  = strdup("\033[=<I");
        udkfkeys[50]  = strdup("\033[=<J");
        udkfkeys[51]  = strdup("\033[=<K");
        udkfkeys[52]  = strdup("\033[=<L");
        udkfkeys[53]  = strdup("\033[=<M");
        udkfkeys[54]  = strdup("\033[=<O");
        udkfkeys[55]  = strdup("\033[=<P");
        udkfkeys[56]  = strdup("\033[=<Q");
        udkfkeys[57]  = strdup("\033[=<R");
        udkfkeys[58]  = strdup("\033[=<T");
        udkfkeys[59]  = strdup("\033[=<U");
        udkfkeys[60]  = strdup("\033[=<V");
        udkfkeys[61]  = strdup("\033[=<W");
    }

    if (ISSUN(tt_type_mode)) {
        udkfkeys[0]   = strdup("\033[192z");   /* K_SUN_STOP        */
        udkfkeys[1]   = strdup("\033[193z");   /* K_SUN_AGAIN       */
        udkfkeys[2]   = strdup("\033[194z");   /* K_SUN_PROPS       */
        udkfkeys[3]   = strdup("\033[195z");   /* K_SUN_UNDO        */
        udkfkeys[4]   = strdup("\033[196z");   /* K_SUN_FRONT       */
        udkfkeys[5]   = strdup("\033[197z");   /* K_SUN_COPY        */
        udkfkeys[6]   = strdup("\033[198z");   /* K_SUN_OPEN        */
        udkfkeys[7]   = strdup("\033[199z");   /* K_SUN_PASTE       */
        udkfkeys[8]   = strdup("\033[200z");   /* K_SUN_FIND        */
        udkfkeys[9]   = strdup("\033[201z");   /* K_SUN_CUT         */
        udkfkeys[10]  = strdup("\033[207z");   /* K_SUN_HELP        */
    }

    if (ISIBM31(tt_type_mode)) {
        extern int i31_lta;
        int i,j;

        udkfkeys[0]   = strdup("\033a\003");   /* K_I31_F01         */
        udkfkeys[1]   = strdup("\033b\003");   /* K_I31_F02         */
        udkfkeys[2]   = strdup("\033c\003");   /* K_I31_F03         */
        udkfkeys[3]   = strdup("\033d\003");   /* K_I31_F04         */
        udkfkeys[4]   = strdup("\033e\003");   /* K_I31_F05         */
        udkfkeys[5]   = strdup("\033f\003");   /* K_I31_F06         */
        udkfkeys[6]   = strdup("\033g\003");   /* K_I31_F07         */
        udkfkeys[7]   = strdup("\033h\003");   /* K_I31_F08         */
        udkfkeys[8]   = strdup("\033i\003");   /* K_I31_F09         */
        udkfkeys[9]   = strdup("\033j\003");   /* K_I31_F10         */
        udkfkeys[10]  = strdup("\033k\003");   /* K_I31_F11         */
        udkfkeys[11]  = strdup("\033l\003");   /* K_I31_F12         */
        udkfkeys[12]  = strdup("\033!a\003");  /* K_I31_F13         */
        udkfkeys[13]  = strdup("\033!b\003");  /* K_I31_F14         */
        udkfkeys[14]  = strdup("\033!c\003");  /* K_I31_F15         */
        udkfkeys[15]  = strdup("\033!d\003");  /* K_I31_F16         */
        udkfkeys[16]  = strdup("\033!e\003");  /* K_I31_F17         */
        udkfkeys[17]  = strdup("\033!f\003");  /* K_I31_F18         */
        udkfkeys[18]  = strdup("\033!g\003");  /* K_I31_F19         */
        udkfkeys[19]  = strdup("\033!h\003");  /* K_I31_F20         */
        udkfkeys[20]  = strdup("\033!i\003");  /* K_I31_F21         */
        udkfkeys[21]  = strdup("\033!j\003");  /* K_I31_F22         */
        udkfkeys[22]  = strdup("\033!k\003");  /* K_I31_F23         */
        udkfkeys[23]  = strdup("\033!l\003");  /* K_I31_F24         */
        udkfkeys[24]  = strdup("\033\"a\003"); /* K_I31_F25         */
        udkfkeys[25]  = strdup("\033\"b\003"); /* K_I31_F26         */
        udkfkeys[26]  = strdup("\033\"c\003"); /* K_I31_F27         */
        udkfkeys[27]  = strdup("\033\"d\003"); /* K_I31_F28         */
        udkfkeys[28]  = strdup("\033\"e\003"); /* K_I31_F29         */
        udkfkeys[29]  = strdup("\033\"f\003"); /* K_I31_F30         */
        udkfkeys[30]  = strdup("\033\"g\003"); /* K_I31_F31         */
        udkfkeys[31]  = strdup("\033\"h\003"); /* K_I31_F32         */
        udkfkeys[32]  = strdup("\033\"i\003"); /* K_I31_F33         */
        udkfkeys[33]  = strdup("\033\"j\003"); /* K_I31_F34         */
        udkfkeys[34]  = strdup("\033\"k\003"); /* K_I31_F35         */
        udkfkeys[35]  = strdup("\033\"l\003"); /* K_I31_F36         */
        udkfkeys[36]  = strdup("\033!m\003");  /* K_I31_PA1         */
        udkfkeys[37]  = strdup("\033!n\003");  /* K_I31_PA2         */
        udkfkeys[38]  = strdup("\033!o\003");  /* K_I31_PA3         */
        udkfkeys[39]  = strdup("\033!z");      /* K_I31_RESET       */
        udkfkeys[40]  = strdup("\033\"A");     /* K_I31_JUMP        */
        udkfkeys[41]  = strdup("\033L\003");   /* K_I31_CLEAR       */
        udkfkeys[42]  = strdup("\033I");       /* K_I31_ERASE_EOF   */
        udkfkeys[43]  = strdup("\033J");       /* K_I31_ERASE_EOP   */
        udkfkeys[44]  = strdup("\033K");       /* K_I31_ERASE_INP   */
        udkfkeys[45]  = strdup("\033P");       /* K_I31_INSERT_CHAR */
        udkfkeys[46]  = strdup("\033P \008");  /* K_I31_INSERT_SPACE*/
        udkfkeys[47]  = strdup("\033Q");       /* K_I31_DELETE      */
        udkfkeys[48]  = strdup("\033N");       /* K_I31_INS_LN      */
        udkfkeys[49]  = strdup("\033O");       /* K_I31_DEL_LN      */
        udkfkeys[50]  = strdup("\033U\003");   /* K_I31_PRINT_LINE  */
        udkfkeys[51]  = strdup("\033V\003");   /* K_I31_PRINT_MSG   */
        udkfkeys[52]  = strdup("\033W\003");   /* K_I31_PRINT_SHIFT */
        udkfkeys[53]  = strdup("\033 W\003");  /* K_I31_CANCEL      */
        udkfkeys[54]  = strdup("\033!8\003");  /* K_I31_SEND_LINE   */
        udkfkeys[55]  = strdup("\033 8\003");  /* K_I31_SEND_MSG    */
        udkfkeys[56]  = strdup("\0338\003");   /* K_I31_SEND_PAGE   */
        udkfkeys[57]  = strdup("\033H");       /* K_I31_HOME        */
        udkfkeys[58]  = strdup("\0332");       /* K_I31_BACK_TAB    */

        if ( i31_lta != ETX ) {
            for ( i=0;i<=58;i++ ) {
                for ( j=strlen(udkfkeys[i])-1;j>=0;j-- ) {
                    if ( udkfkeys[i][j] == ETX )
                        udkfkeys[i][j] = i31_lta;
                }
            }
        }
    }
}

void
showudk( void )
{
#ifndef NOKVERBS
    int i;
    printf("\n%s User Defined Keys:\n",
            ISSCO(tt_type)?"SCOANSI":
            ISAT386(tt_type)?"AT386":
            ISLINUX(tt_type)?"LINUX":
            ISANSI(tt_type)?"ANSI":
            ISWYSE(tt_type)?"WYSE":
            ISTVI(tt_type)?"TVI":
            ISH19(tt_type)?"HEATH":
            ISDG200(tt_type)?"DG":
            ISHP(tt_type)?"HP":
            IS97801(tt_type)?"SNI 97801-5XX":
            ISBA80(tt_type)?"Nixdorf BA80":
            ISIBM31(tt_type)?"IBM 31x1":
            ISSUN(tt_type)?"SUN":
            "DEC VT");
    printf("  Lock: %s\n\n", udklock ? "on": "off" ) ;

    if ( IS97801(tt_type) ) {
        for ( i=0; i<(K_SNI_MAX-K_SNI_MIN+1) ; i++ ) {
            if ( udkfkeys[i] && udkfkeys[i][0] < SP )
                printf( "  snif%d = \\{%d}%s\n",i+1,
                        udkfkeys[i][0], &(udkfkeys[i][1]));
            else
                printf( "  snif%d = %s\n",i+1,
                        udkfkeys[i]?udkfkeys[i]:"(unassigned)");
        }
    }
    else if ( ISSUN(tt_type) ) {
        for ( i=0; i<=10 ; i++ ) {
            char * s = "" ;
            switch ( i ) {
            case 0: s = "sunstop"  ; break ;
            case 1: s = "sunagain" ; break ;
            case 2: s = "sunprops" ; break ;
            case 3: s = "sunundo"  ; break ;
            case 4: s = "sunfront" ; break ;
            case 5: s = "suncopy"  ; break ;
            case 6: s = "sunopen"  ; break ;
            case 7: s = "sunpaste" ; break ;
            case 8: s = "sunfind"  ; break ;
            case 9: s = "suncut"   ; break ;
            case 10: s = "sunhelp" ; break ;
            }
            if ( udkfkeys[i] && udkfkeys[i][0] < SP )
                printf( "  %s = \\{%d}%s\n",s,
                        udkfkeys[i][0], &(udkfkeys[i][1]));
            else
                printf( "  %s = %s\n",s,
                        udkfkeys[i]?udkfkeys[i]:"(unassigned)");
        }
    }
    else if ( ISUNIXCON(tt_type) ) {
        for ( i=0; i<61 ; i++ ) {
            if ( udkfkeys[i] && udkfkeys[i][0] < SP )
                printf( "  ansif%-3d = \\{%d}%s\n",i+1,
                        udkfkeys[i][0], &(udkfkeys[i][1]));
            else
                printf( "  ansif%-3d = %s\n",i+1,
                        udkfkeys[i]?udkfkeys[i]:"(unassigned)");
        }
    } 
    else if ( ISVT100(tt_type) )
        for ( i=1; i<=20 ; i++ )
        {
            if ( udkfkeys[i-1] && udkfkeys[i-1][0] < SP ) {
                if ( i==15 )
                    printf("  udkdo   = \\{%d}%s\n",
                            udkfkeys[i-1][0],&(udkfkeys[i-1][1]));
                else if ( i==16 )
                    printf("  udkhelp = \\{%d}%s\n",
                            udkfkeys[i-1][0],&(udkfkeys[i-1][1]));
                else
                    printf("  udkf%-3d = \\{%d}%s\n",i,
                            udkfkeys[i-1][0],&(udkfkeys[i-1][1]));
            }
            else {
                if ( i==15 )
                    printf("  udkdo   = %s\n",
                            udkfkeys[i-1]?udkfkeys[i-1]:"(unassigned)");
                else if ( i==16 )
                    printf("  udkhelp = %s\n",
                            udkfkeys[i-1]?udkfkeys[i-1]:"(unassigned)");
                else
                    printf("  udkf%-3d = %s\n",i,
                            udkfkeys[i-1]?udkfkeys[i-1]:"(unassigned)");
            }
        }
    else if ( ISWYSE(tt_type) ) {
        for ( i=0; i<20 ; i++ ) {
            if ( udkfkeys[i] && udkfkeys[i][0] < SP )
                printf( "  wyf%-3d = \\{%d}%s\n",i+1,
                        udkfkeys[i][0], &(udkfkeys[i][1]));
            else
                printf( "  wyf%-3d = %s\n",i+1,
                        udkfkeys[i]?udkfkeys[i]:"(unassigned)");
        }
        for ( i=20; i<40 ; i++ ) {
            if ( udkfkeys[i] && udkfkeys[i][0] < SP )
                printf( "  wysf%-3d = \\{%d}%s\n",i+1-20,
                        udkfkeys[i][0], &(udkfkeys[i][1]));
            else
                printf( "  wysf%-3d = %s\n",i+1-20,
                        udkfkeys[i]?udkfkeys[i]:"(unassigned)");
        }
        for ( i=40; i<75 ; i++ ) {
            char * s = "" ;
            switch ( i ) {
            case 40: s = "wybs"         ; break ;
            case 41: s = "wyclrln"      ; break ;
            case 42: s = "wysclrln"     ; break ;
            case 43: s = "wyclrpg"      ; break ;
            case 44: s = "wysclrpg"     ; break ;
            case 45: s = "wydelchar"    ; break ;
            case 46: s = "wydelln"      ; break ;
            case 47: s = "wyenter"      ; break ;
            case 48: s = "wyesc"        ; break ;
            case 49: s = "wyhome"       ; break ;
            case 50: s = "wyshome"      ; break ;
            case 51: s = "wyinsert"     ; break ;
            case 52: s = "wyinschar"    ; break ;
            case 53: s = "wyinsln"      ; break ;
            case 54: s = "wypgnext"     ; break ;
            case 55: s = "wypgprev"     ; break ;
            case 56: s = "wyreplace"    ; break ;
            case 57: s = "wyreturn"     ; break ;
            case 58: s = "wytab"        ; break ;
            case 59: s = "wystab"       ; break ;
            case 60: s = "wyprtscn"     ; break ;
            case 61: s = "wysesc"       ; break ;
            case 62: s = "wysbs"        ; break ;
            case 63: s = "wysenter"     ; break ;
            case 64: s = "wysreturn"    ; break ;
            case 65: s = "wyuparr"      ; break ;
            case 66: s = "wydnarr"      ; break ;
            case 67: s = "wylfarr"      ; break ;
            case 68: s = "wyrtarr"      ; break ;
            case 69: s = "wysuparr"     ; break ;
            case 70: s = "wysdnarr"     ; break ;
            case 71: s = "wyslfarr"     ; break ;
            case 72: s = "wysrtarr"     ; break ;
            case 73: s = "wysend"       ; break ;
            case 74: s = "wyssend"      ; break ;
            }
            if ( udkfkeys[i] && udkfkeys[i][0] < SP )
                printf( "  %s = \\{%d}%s\n",s,
                        udkfkeys[i][0], &(udkfkeys[i][1]));
            else
                printf( "  %s = %s\n",s,
                        udkfkeys[i]?udkfkeys[i]:"(unassigned)");
        }

    }
    else if ( ISTVI(tt_type) ) {
        for ( i=0; i<15 ; i++ ) {
            if ( udkfkeys[i] && udkfkeys[i][0] < SP )
                printf( "  tvif%-3d = \\{%d}%s\n",i+1,
                        udkfkeys[i][0], &(udkfkeys[i][1]));
            else
                printf( "  tvif%-3d = %s\n",i+1,
                        udkfkeys[i]?udkfkeys[i]:"(unassigned)");
        }
        for ( i=16; i<31 ; i++ ) {
            if ( udkfkeys[i] && udkfkeys[i][0] < SP )
                printf( "  tvisf%-3d = \\{%d}%s\n",i+1-16,
                        udkfkeys[i][0], &(udkfkeys[i][1]));
            else
                printf( "  tvisf%-3d = %s\n",i+1-16,
                        udkfkeys[i]?udkfkeys[i]:"(unassigned)");
        }
        for ( i=32; i<67 ; i++ ) {
            char * s = "" ;
            switch ( i ) {
            case 32: s = "tvibs"        ; break ;
            case 33: s = "tviclrln"     ; break ;
            case 34: s = "tvisclrln"    ; break ;
            case 35: s = "tviclrpg"     ; break ;
            case 36: s = "tvisclrpg"    ; break ;
            case 37: s = "tvidelchar"   ; break ;
            case 38: s = "tvidelln"     ; break ;
            case 39: s = "tvienter"     ; break ;
            case 40: s = "tviesc"       ; break ;
            case 41: s = "tvihome"      ; break ;
            case 42: s = "tvishome"     ; break ;
            case 43: s = "tviinsert"    ; break ;
            case 44: s = "tviinschar"   ; break ;
            case 45: s = "tviinsln"     ; break ;
            case 46: s = "tvipgnext"    ; break ;
            case 47: s = "tvipgprev"    ; break ;
            case 48: s = "tvireplace"   ; break ;
            case 49: s = "tvireturn"    ; break ;
            case 50: s = "tvitab"       ; break ;
            case 51: s = "tvistab"      ; break ;
            case 52: s = "tviprtscn"    ; break ;
            case 53: s = "tvisesc"       ; break ;
            case 54: s = "tvisbs"        ; break ;
            case 55: s = "tvisenter"     ; break ;
            case 56: s = "tvisreturn"    ; break ;
            case 57: s = "tviuparr"      ; break ;
            case 58: s = "tvidnarr"      ; break ;
            case 59: s = "tvilfarr"      ; break ;
            case 60: s = "tvirtarr"      ; break ;
            case 61: s = "tvisuparr"     ; break ;
            case 62: s = "tvisdnarr"     ; break ;
            case 63: s = "tvislfarr"     ; break ;
            case 64: s = "tvisrtarr"     ; break ;
            case 65: s = "tvisend"       ; break ;
            case 66: s = "tvissend"      ; break ;
            }
            if ( udkfkeys[i] && udkfkeys[i][0] < SP )
                printf( "  %s = \\{%d}%s\n",s,
                        udkfkeys[i][0], &(udkfkeys[i][1]));
            else
                printf( "  %s = %s\n",s,
                        udkfkeys[i]?udkfkeys[i]:"(unassigned)");
        }
    }
    else if ( ISDG200(tt_type) ) {
        for ( i=0; i<110 ; i++ ) {
            if ( udkfkeys[i] &&
                 (udkfkeys[i][0] < SP || udkfkeys[i][0] >= 127) )
                printf( "  dgf%-3d = \\{%d}%s\n",i+1,
                        udkfkeys[i][0], &(udkfkeys[i][1]));
            else
                printf( "  dgf%-3d = %s\n",i+1,
                        udkfkeys[i]?udkfkeys[i]:"(unassigned)");
        }
    }
    else if ( ISHP(tt_type) ) {
        for ( i=0; i<=15 ; i++ ) {
            if ( udkfkeys[i] && udkfkeys[i][0] < SP )
                printf( "  hpf%d = \\{%d}%s\n",i+1,
                        udkfkeys[i][0], &(udkfkeys[i][1]));
            else
                printf( "  hpf%d = %s\n",i+1,
                        udkfkeys[i]?udkfkeys[i]:"(unassigned)");
        }
        if ( udkfkeys[16] && udkfkeys[16][0] < SP )
            printf( "  hpbacktab = \\{%d}%s\n",
                    udkfkeys[16][0], &(udkfkeys[16][1]));
        else
            printf( "  hpbacktab = %s\n",
                    udkfkeys[16]?udkfkeys[16]:"(unassigned)");
        if ( udkfkeys[17] && udkfkeys[17][0] < SP )
            printf( "  hpenter = \\{%d}%s\n",
                    udkfkeys[17][0], &(udkfkeys[17][1]));
        else
            printf( "  hpenter = %s\n",
                    udkfkeys[17]?udkfkeys[17]:"(unassigned)");
        if ( udkfkeys[18] && udkfkeys[18][0] < SP )
            printf( "  hpreturn = \\{%d}%s\n",
                    udkfkeys[18][0], &(udkfkeys[18][1]));
        else
            printf( "  hpreturn = %s\n",
                    udkfkeys[18]?udkfkeys[18]:"(unassigned)");

    }
    else if ( ISIBM31(tt_type) ) {
        for ( i=0; i<(K_I31_MAX-K_I31_MIN+1) ; i++ ) {
            if ( udkfkeys[i] && udkfkeys[i][0] < SP )
                printf( "  i31_f%d = \\{%d}%s\n",i+1,
                        udkfkeys[i][0], &(udkfkeys[i][1]));
            else
                printf( "  i31_f%d = %s\n",i+1,
                        udkfkeys[i]?udkfkeys[i]:"(unassigned)");
        }
    }
    printf("\n");
#endif /* NOVKERBS */
}

void
SNI_bitmode(int bits) {
    int i;
    int cs;

    sni_bitmode = bits;
    if ( sni_bitmode == 7 ) {
                /* Load National character set into G0/G1/G2/G3 */
                /* GL to G0, GR to G1                           */
                /* National keyboard is active                  */
                for ( i = 0 ; i < 4 ; i++ ) {
                        G[i].designation = dec_nrc;
                        G[i].size = cs94 ;
                        G[i].c1 = TRUE ;
                        G[i].national = CSisNRC(dec_nrc);
                        G[i].rtoi = xl_u[dec_nrc];
                        if ( isunicode() ) {
                                G[i].itol = NULL ;
                                G[i].ltoi = NULL ;
                        }
            else {
                G[i].itol = xl_tx[tcsl] ;
                G[i].ltoi = xl_u[tcsl] ;
            }
            G[i].itor = xl_tx[dec_nrc];
        }
        GL = &G[0];
        GR = &G[1];
    } else if ( sni_bitmode == 8 ) {
        /* US-ASCII to G0 (GL) */
        /* Brackets to G1      */
        /* Blanks to G2        */
        /* Latin 1 to G3 (GR)  */
        /* Latin 1 keyboard is active */
        for ( i = 0 ; i < 4 ; i++ ) {
            switch ( i ) {
            case 0: cs = TX_ASCII; break;
            case 1: cs = TX_SNIBRACK; break;
            case 2: cs = TX_SNIBLANK; break;
            case 3: cs = TX_8859_1; break;
            default:
                cs = TX_ASCII;
            }

            G[i].designation = cs;
            G[i].size = cs94 ;
            G[i].c1 = TRUE ;
            G[i].national = CSisNRC(cs);
            G[i].rtoi = xl_u[cs];
            if ( isunicode() ) {
                G[i].itol = NULL ;
                G[i].ltoi = NULL ;
            }
            else {
                G[i].itol = xl_tx[tcsl] ;
                G[i].ltoi = xl_u[tcsl] ;
            }
            G[i].itor = xl_tx[cs];
        }
        GL = &G[0];
        GR = &G[3];
    }
}

void
SNI_chcode( int state ) {
    int    x,y;
    USHORT w,h;
    USHORT ch;

    if ( sni_bitmode == 7 ) {
        if (!sni_chcode_7 || sni_chcode == state)
            return;

        sni_chcode = state;
        if ( sni_chcode ) {
            /* if the current National language is German then convert */
            /* NRC characters to US-ASCII characters in the current    */
            /* screen.
            */
            if ( G[0].designation == TX_GERMAN ) {
                w = VscrnGetWidth(VTERM);
                h = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                for (y = 0; y < h; y++) {
                    for ( x = 0 ; x < w; x++ ) {
                        ch = VscrnGetCell( VTERM, x, y )->c;
                        if ( !isunicode() )
                            ch = xl_u[tcsl](ch);
                        switch ( ch ) {
                        case 0xA7: ch = '@'; break;
                        case 0xC4: ch = '['; break;
                        case 0xD6: ch = '\\'; break;
                        case 0xDC: ch = ']'; break;
                        case 0xE4: ch = '{'; break;
                        case 0xF6: ch = '|'; break;
                        case 0xFC: ch = '}'; break;
                        case 0xDF: ch = '~'; break;
                        default:
                            continue;
                        }
                        VscrnGetCell( VTERM, x, y )->c = ch;
                    }
                }
                for ( x = 0 ; x < w; x++ ) {
                    ch = VscrnGetCell( VSTATUS, x, 0 )->c;
                    if ( !isunicode() )
                        ch = xl_u[tcsl](ch);
                    switch ( ch ) {
                    case 0xA7: ch = '@'; break;
                    case 0xC4: ch = '['; break;
                    case 0xD6: ch = '\\'; break;
                    case 0xDC: ch = ']'; break;
                    case 0xE4: ch = '{'; break;
                    case 0xF6: ch = '|'; break;
                    case 0xFC: ch = '}'; break;
                    case 0xDF: ch = '~'; break;
                    default:
                        continue;
                    }
                    VscrnGetCell( VSTATUS, x, 0 )->c = ch;
                }
                VscrnIsDirty(VTERM);
            }
        } else {
            /* if the current National language is German then convert */
            /* US-ASCII characters to NRC characters in the current    */
            /* screen.                                                 */
            if ( G[0].designation == TX_GERMAN ) {
                w = VscrnGetWidth(VTERM);
                h = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                for (y = 0; y < h; y++) {
                    for ( x = 0 ; x < w; x++ ) {
                        ch = VscrnGetCell( VTERM, x, y )->c;
                        if ( !isunicode() )
                            ch = xl_u[tcsl](ch);
                        switch ( ch ) {
                        case '@': ch = 0xA7; break;
                        case '[': ch = 0xC4; break;
                        case '\\': ch = 0xD6; break;
                        case ']': ch = 0xDC; break;
                        case '{': ch = 0xE4; break;
                        case '|': ch = 0xF6; break;
                        case '}': ch = 0xFC; break;
                        case '~': ch = 0xDF; break;
                        default:
                            continue;
                        }
                        if ( !isunicode() ) {
                            ch = xl_tx[tcsl](ch);
                        }
                        VscrnGetCell( VTERM, x, y )->c = ch;
                    }
                }
                for ( x = 0 ; x < w; x++ ) {
                    ch = VscrnGetCell( VSTATUS, x, 0 )->c;
                    if ( !isunicode() )
                        ch = xl_u[tcsl](ch);
                    switch ( ch ) {
                    case '@': ch = 0xA7; break;
                    case '[': ch = 0xC4; break;
                    case '\\': ch = 0xD6; break;
                    case ']': ch = 0xDC; break;
                    case '{': ch = 0xE4; break;
                    case '|': ch = 0xF6; break;
                    case '}': ch = 0xFC; break;
                    case '~': ch = 0xDF; break;
                    default:
                        continue;
                    }
                    if ( !isunicode() ) {
                        ch = xl_tx[tcsl](ch);
                    }
                    VscrnGetCell( VSTATUS, x, 0 )->c = ch;
                }
                VscrnIsDirty(VTERM);
            }
        }
    } else if ( sni_bitmode == 8 ) {
        if (!sni_chcode_8)
            return;

        sni_chcode = state;
    }
}

void
savecurpos(int vmode, int x) {          /* x: 0 = cursor only, 1 = all */
    int i ;
    saved[vmode] = TRUE;                        /* Remember they are saved */
    savedrow[vmode] = wherey[vmode];            /* Current row (absolute) */
    savedcol[vmode] = wherex[vmode];            /* Current column (absolute) */
    if (x) {
        savedattribute[vmode] = attribute;      /* Current PC video attributes */
        saveddefaultattribute[vmode] = defaultattribute ;
        savedunderlineattribute[vmode] = underlineattribute;
        saveditalicattribute[vmode] = italicattribute;
        savedreverseattribute[vmode]= reverseattribute;
        savedgraphicattribute[vmode]= graphicattribute;
        savedborderattribute[vmode]= borderattribute;
        savedattrib[vmode] = attrib;            /* Current DEC character attributes */
        saverelcursor[vmode] = relcursor;       /* Cursor addressing mode */
        savedwrap[vmode]     = tt_wrap;         /* Wrap mode */

        if ( x==1 ) {
            for (i=0; i<4; i++)
                savedG[vmode][i] = G[i] ;
            savedGL[vmode] = GL ;
            savedGR[vmode] = GR ;
            savedSSGL[vmode] = SSGL ;
        }
    }
}

void
restorecurpos(int vmode, int x) {
    int i ;
    if (saved[vmode] == FALSE) {                /* Nothing saved, home the cursor */
        lgotoxy(vmode, 1, relcursor ? margintop : 1);
    }
    else {
        lgotoxy(vmode, savedcol[vmode], savedrow[vmode]);/* Goto saved position */
        if (x) {
            attribute = savedattribute[vmode];  /* Restore saved attributes */
            defaultattribute=saveddefaultattribute[vmode];
            underlineattribute=savedunderlineattribute[vmode];
            italicattribute=saveditalicattribute[vmode];
            reverseattribute=savedreverseattribute[vmode];
            graphicattribute=savedgraphicattribute[vmode];
            borderattribute=savedborderattribute[vmode];
            attrib = savedattrib[vmode];
            relcursor = saverelcursor[vmode];   /* Restore cursor addressing mode */
            tt_wrap = savedwrap[vmode] ;       /* Restore wrap mode */

            if ( x==1 ) {                       /* Restore char sets */
                for (i=0; i<4; i++)
                    G[i] = savedG[vmode][i] ;
                GL = savedGL[vmode] ;
                GR = savedGR[vmode] ;
                SSGL = savedSSGL[vmode] ;
            }
        }
    }
}

/* DECSASD - Select Active Status Display */
void
setdecsasd( bool x )
{
    if ( x == SASD_STATUS && decssdt == SSDT_HOST_WRITABLE ) {
        if ( decsasd == SASD_TERMINAL ) {
            savecurpos(VTERM,2);
            if ( saved[VSTATUS] )
                restorecurpos(VSTATUS,2);
        }
        decsasd = x ;
    }
    else if (x == SASD_TERMINAL) {
        if ( decsasd == SASD_STATUS ) {
            savecurpos(VSTATUS,2);
            if ( saved[VTERM] )
                restorecurpos(VTERM,2);
        }
        decsasd = 0 ;
    }
}

/* DECSSDT - Select Status Display Type */
void
setdecssdt( int x )
{
    int i;
    switch ( x ) {
    case SSDT_BLANK:
        settermstatus(FALSE);
        break;
    case SSDT_INDICATOR:
        settermstatus(TRUE);
        break;
    case SSDT_HOST_WRITABLE:
        if ( decssdt == SSDT_INDICATOR ) {
            lgotoxy(VSTATUS,1,1);
            clrtoeoln(VSTATUS,SP);
            savecurpos(VSTATUS,2);
            for ( i=0;i<MAXTERMCOL;i++ )
                hoststatusline[i] = SP ;
            hoststatusline[MAXTERMCOL]=NUL;
        }
        settermstatus(TRUE);
        break;
    }
    decssdt = x ;
    VscrnIsDirty(VTERM);
}

void
setkeyclick(int on)
{
    keyclick = on;
#ifdef KUI
    KuiSetProperty(KUI_TERM_KEYCLICK, on, 0);
#endif /* KUI */
}

void                                    /* Reset the terminal emulator */
doreset(int x) {                        /* x = 0 (soft), nonzero (hard) */
    extern BYTE vmode ;
    extern int dgunix, dgunix_usr;
    int i;

    debug(F111,"doreset","x",x);

    tt_type_mode = tt_type ;

    attribute = defaultattribute = colornormal; /* Normal colors */
    underlineattribute = colorunderline ;
    reverseattribute = colorreverse;
    graphicattribute = colorgraphic;
    borderattribute  = colorborder;
    italicattribute  = coloritalic;

    saveddefaultattribute[VTERM] = colornormal; /* Default saved values */
    savedunderlineattribute[VTERM] = colorunderline ;
    saveditalicattribute[VTERM] = coloritalic;
    savedreverseattribute[VTERM] = colorreverse;
    savedgraphicattribute[VTERM] = colorgraphic;
    savedborderattribute[VTERM]  = colorborder;
    savedattribute[VTERM] = attribute;

    attrib.blinking = FALSE;            /* No blink */
    attrib.bold = FALSE;                /* No bold */
    attrib.invisible = FALSE;           /* Visible */
    attrib.underlined = FALSE;          /* No underline */
    attrib.reversed = FALSE;            /* No reverse video */
    attrib.unerasable = FALSE;          /* Erasable */
    attrib.graphic = FALSE ;            /* Not graphic character */
    attrib.dim = FALSE ;                /* No dim */
    attrib.wyseattr = FALSE ;
    attrib.italic = FALSE;              /* No italic */
    attrib.hyperlink = FALSE;
    attrib.linkid = 0;
    savedattrib[VTERM] = attrib;

    erasemode = user_erasemode;

    /* Restore DEC VT Graphic Set translation functions */
    for ( i = 0 ; i < 4 ; i++ )
    {
        G[i].designation = G[i].def_designation ;
        G[i].size = G[i].def_size ;
        G[i].c1 = G[i].def_c1 ;
        G[i].national = CSisNRC(G[i].designation) ;
        if ( tcs_transp ) {
            debug(F111,"doreset - ERROR","G(i).designation == TX_TRANSP",i);
            G[i].rtoi = NULL ;
            G[i].itol = NULL ;
            G[i].ltoi = NULL ;
            G[i].itor = NULL ;
        }
        else
        {
            G[i].rtoi = xl_u[G[i].designation];
            G[i].itol = xl_tx[tcsl] ;
            G[i].ltoi = xl_u[tcsl] ;
            G[i].itor = xl_tx[G[i].designation];
        }
        G[i].init = FALSE ;
    }
    GNOW = GL = &G[0] ;
    GR = ISLINUX(tt_type_mode) ? &G[1] : ISVT220(tt_type_mode) ? &G[2] : &G[1];
    SSGL = NULL ;
    decnrcm = decnrcm_usr;
    setdecsasd(SASD_TERMINAL);
    if ( IS97801(tt_type_mode) )
        setdecssdt( SSDT_HOST_WRITABLE );
    else if ( x )
        setdecssdt( tt_status_usr[VTERM] ? SSDT_INDICATOR : SSDT_BLANK );

    xprint = FALSE ;                    /* Turn off ctrl print */
    setaprint(FALSE);                   /* Turn off auto print */
    cprint = FALSE ;                    /* Turn off copy print */
    if ( !uprint ) {
        printeroff() ;                  /* Turn off the printer if it is on */
        printon = FALSE;                /* Printer is not on */
    }
    printregion = FALSE;                /* Printer extent is full screen */
    screenon = TRUE;                    /* The screen is turned on */
#ifdef CK_APC
    if ( !apcactive ) {
        apcactive = APC_INACTIVE;       /* An APC command is not active */
        apclength = 0;                  /* ... */
    }
    apcrecv = dcsrecv = oscrecv = pmrecv = pu1recv = pu2recv = c1strrecv = 0 ;
#endif /* CK_APC */

    vt52graphics = FALSE;               /* Get out of VT52 graphics mode */
    for ( i=0;i<VNUM;i++ )
        saved[i] = FALSE;               /* Nothing is saved */
    tnlm = tt_crd = FALSE;              /* We're not in newline mode */
    insertmode = FALSE;                 /* Not in character-insert mode */
    tt_arrow = TTK_NORM;                /* Arrow keypad to cursor mode */
    tt_keypad = TTK_NORM;               /* Auxilliary keypad to numeric mode */
    tt_shift_keypad = FALSE ;           /* Do not shift keypad values */
    tt_wrap = TRUE;                     /* (FALSE for real VT terminal!) */
    send_c1 = send_c1_usr;              /* Don't send C1 controls */
    keylock = FALSE;                    /* Keyboard is not locked */

    udkreset() ;                        /* Reset UDKs     */
    deccolm = FALSE;                    /* default column mode */
    tt_cols[VTERM] = tt_cols_usr ;
    if (tt_updmode == TTU_FAST ) /* set terminal scroll mode to original */
        JumpScroll();
    else
        SmoothScroll();
    if (vmode==VTERM)
        SetCols(VTERM) ;
#ifdef TCPSOCKET
#ifdef CK_NAWS
    if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0)
    {
        tn_snaws();
#ifdef RLOGCODE
        rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
        ssh_snaws();
#endif /* SSHBUILTIN */
    }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */
    ipadl25();

    decsace = FALSE;
    decscnm = FALSE;
    if (decscnm_usr)                    /* Reverse Screen Mode */
        flipscreen(VTERM);

    for (i = 1; i < VscrnGetWidth(VTERM); i++)  /* Tab settings every 8 spaces except QNX */
      htab[i] = (i % (ISQNX(tt_type_mode)?4:8)) == 1 ? 'T' : '0'; /* was "== 0" */
    relcursor = FALSE;                  /* Cursor position is absolute */

    /* Real terminal sets margins to (1,24) */
    setmargins(1, VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0));/* Scrolling region is whole screen */
    marginleft=1;
    marginright=VscrnGetWidth(VTERM);

    escstate = ES_NORMAL;               /* In case we're stuck in a string */
    setborder();                        /* Restore border color */

    if (x) {                            /* Now clear the screen and home the cursor*/
        if ( VscrnGetBufferSize(VTERM) > 0 && !VscrnIsClear(VTERM)) {
            clrscreen(VTERM,SP);
            lgotoxy(VTERM,1,1);       /* and home the cursor */
        }
    }
    VscrnSetDisplayHeight(VTERM,0);
    if (scrollflag[VTERM])              /* Were we rolled back? */
      scrollflag[VTERM] = 0;            /* Not any more ... */
    ipadl25();                          /* Put back regular status line */

    cursorena[VTERM] = tt_cursorena_usr;/* Cursor enabled? */
    cursoron[VTERM] = FALSE ;           /* Force newcursor to restore the cursor */
    tt_cursor = tt_cursor_usr ;
    setcursormode() ;

    if ( duplex_sav > -1 ) {            /* Restore user Echo setting */
        duplex = duplex_sav ;
        duplex_sav = -1 ;
    }
    deckbum = FALSE ;                   /* Keyboard in Typewriter mode */
    if ( savdecbkm )                    /* Restore Backspace mode to user default */
    {
        keymap[KEY_SCAN | 8] = savdecbkm ;
        savdecbkm = 0 ;
    }

    sco8bit = FALSE ;                   /* SCO ANSI 8th bit quoting off */
    scoBCS2 = FALSE ;                   /* SCO ANSI BCS2 mode */
    wyse8bit = FALSE ;
    crm = FALSE ;                       /* SCO ANSI c0 display off      */
    setkeyclick(FALSE);

    marginbell = FALSE ;
    marginbellcol = VscrnGetWidth(VTERM)-8 ;

    beepfreq = DEF_BEEP_FREQ ;
    beeptime = DEF_BEEP_TIME ;

    sni_pagemode = sni_pagemode_usr;    /* SNI 97801 AutoRoll */
    sni_scroll_mode = sni_scroll_mode_usr;/* SNI 97801 Roll */
    if ( IS97801(tt_type_mode) ) {
        SNI_chcode(sni_chcode_usr);     /* 97801 CH.CODE mode */
        sni_chcode_7 = TRUE;            /* 97801 CH.CODE key enabled 7-bit mode */
        sni_chcode_8 = TRUE;            /* 97801 CH.CODE key enabled 8-bit mode */
        SNI_bitmode((cmask == 0377) ? 8 : 7);
    }

    autoscroll = TRUE ;                 /* WYSE ASCII AutoScroll */
    protect = FALSE;                    /* WYSE ASCII Protect */
    writeprotect = FALSE ;              /* WYSE ASCII WriteProtect */
    WPattrib = defWPattrib;             /* WYSE ASCII WP Attribute */
    wysegraphics = FALSE ;
    wy_keymode = FALSE ;                /* WYSE ASCII Key Appl Mode */
    wy_enhanced = TRUE ;                /* WYSE ASCII Enhanced Mode On */
    wy_widthclr = FALSE ;               /* WYSE ASCII Width Clear Mode Off */
    wy_nullsuppress = TRUE;             /* WYSE ASCII Null Suppress On */
    if ( ISWYSE(tt_type_mode) ||
         ISTVI(tt_type_mode) ||
         ISHZL(tt_type_mode) ||
         ISDG200(tt_type_mode) ) {      /* WYSE ASCII Auto Page Mode */
        wy_autopage = TRUE;
    }
    else {
        wy_autopage = FALSE ;
    }
    if ( wy_monitor ) {                 /* WYSE ASCII Monitor Mode */
        setdebses(FALSE);
        wy_monitor = FALSE ;
    }
    hzgraphics = 0;                     /* Hazeltine Graphics Mode */

    switch ( tt_type_mode ) {           /* Do Attributes occupy a space in */
    case TT_WY30:
    case TT_WY50:
    case TT_TVI910:
    case TT_TVI925:
    case TT_TVI950:
        tt_hidattr = FALSE;
        break;
    case TT_HP2621:                     /* LINE or PAGE modes              */
    case TT_HPTERM:
    default:
        tt_hidattr = TRUE ;
        break;
    }

    if ( ISWY60(tt_type_mode) )         /* Set the proper Attribute Mode */
        attrmode = ATTR_PAGE_MODE | ATTR_CHAR_MODE ;
    else if ( ISWYSE(tt_type_mode) || ISTVI(tt_type_mode) )
        attrmode = ATTR_PAGE_MODE ;
    else if ( ISHP(tt_type_mode) )
        attrmode = ATTR_LINE_MODE ;
    else
        attrmode = ATTR_CHAR_MODE ;

    tt_kb_mode = KBM_EN ;               /* Turn off Special Keyboard Modes */

    tvi_ic = SP ;                       /* TVI Insert Char is space */
    wy_block = FALSE;                  /* TVI in Conversation mode */

    dgunix = dgunix_usr;

    Qsaved = FALSE;                     /* QANSI Saved Character sets */

    ba80_fkey_read = 0;                 /* We are not reading BA80 fkeys */
    vtnt_index = 0;
    vtnt_read  = VTNT_MIN_READ;

    dokverb(VTERM,K_ENDSCN);
    dokverb(VTERM,K_MARK_CANCEL);

#ifdef CKLEARN
    learnreset();
#endif /* CKLEARN */

    VscrnIsDirty(VTERM) ;
}


/*
  The flow of characters from the communication device to the screen is:

                           +---(debug)----+
                           |              |
  rdcomwrtscr --> cwrite --+--> vt100 --> wrtch
                                       |              |
                                       +--> vtescape--+

  rdcomwrtscr() reads character from communication device via ttinc() and:
   - converts 8-bit controls to 7-bit ESC sequences
   - handles TELNET negotiations
   - handles SO/SI (NOTE: this prevents
   - handles charset translation
   - handles newline-mode and cr-display
   - handles connection loss
   - passes all output chars to cwrite()

  cwrite()
   - handles debug output, direct to wrtch().
   - detects and parses escape-sequences:
     . builds up escape sequence in buffer
     . when complete, calls vtescape(), or does APC.
   - when not debugging & not in esc seq, sends all other chars to vt100().

  vt100()
   - handles ctrl chars in straightforward, modeless way (ENQ, tab, BS, beep)
   - *thinks* it handles SO/SI, but never gets called for this (???)
   - handles graphic chars via mode (G[], vtgraphics, etc) -> wrtch()
   - handles wraparound

  vtescape()
   - acts on escape sequences, changes state, moves cursor, etc.
*/

/*
 * RDCOMWRTSCR  --  Read from the communication device and write to the screen.
 * This function is executed by a separate thread.
 */

void
rdcomwrtscr(void * pArgList)
{
    int c=0, cx=0, tx=0;
    int prty = -2;
    int prtyboost = 0;

    PostRdComWrtScrThreadSem() ;
    setint();

    while ( !Shutdown )
    {

        /* Close printer if necessary */
        if ( printerclose_t ) {
            if ( printerclose_t < time(NULL) ) {
                printerclose();
            }
        }

        if ( !IsConnectMode() ||
#ifdef COMMENT
             what != W_CONNECT ||
#endif /* COMMENT */
             !term_io ||
             (ttyfd == -1
#ifdef CK_TAPI
               || (!tttapi && ttyfd == -2)
#endif /* CK_TAPI */
               ) &&
             !le_inbuf() ||
             holdscreen ) {
            /* debug(F110,"rdcomwrtscr","Waiting for CONNECT_MODE or HOLDSCREEN",0); */
            if (prty != -1 ) {
                prty = -1;      /* lie */
                SetThreadPrty(XYP_REG,0);
            }
            msleep(500);
            continue;
        }

        if ( priority != prty ) {
#ifdef NT
            SetThreadPrty(priority,isWin95() ? 3 : 11);
#else /* NT */
            SetThreadPrty(priority,3);
#endif /* NT */
            prty = priority;
            prtyboost = 0;
        }

        c = ttinc(-500);           /* Get a character from the host */
        if ( c < 0 ) {
            if ( ttyfd == -1
#ifdef CK_TAPI
                 || (!tttapi && ttyfd == -2)
#endif /* CK_TAPI */
                 ) {
                SetConnectMode(FALSE,CSX_HOSTDISC);
                strcpy(termessage, "Connection closed.\n");
            }
            else if (c == -2)
            {
                SetConnectMode(FALSE,CSX_HOSTDISC);
                strcpy(termessage, "Connection closed.\n");
                ttclos(0) ;
                /* link broken */
            }
            else if (c == -3)
            {
                SetConnectMode(FALSE,CSX_SESSION);
                strcpy(termessage, "Session Limit exceeded - closing connection.\n");
                ttclos(0) ;
                /* link broken */
            }
            else {
                if (prtyboost == 1) {
#ifdef NT
                    SetThreadPrty(priority,isWin95() ? 3 : 11);
#else /* NT */
                    SetThreadPrty(priority,3);
#endif /* NT */
                    prtyboost = 0;
                }
                msleep(0);
            }
            continue;
        }
        else {
            scriptwrtbuf(c);

            if (prtyboost == 0) {
#ifdef NT
                SetThreadPrty(priority,(isWin95() ? 3 : 11)+10);
#else /* NT */
                SetThreadPrty(priority,13);
#endif /* NT */
                prtyboost = 1;
            }
        }

#ifdef __DEBUG
        /* we want to be able to see things on a character */
        /* by character basis when debugging these threads */
        /* msleep(0) ; */
#endif /* __DEBUG */
    }
    PostRdComWrtScrThreadDownSem() ;
    ckThreadEnd(pArgList) ;
}

int
ltorxlat( int c, CHAR ** bytes )
{
    unsigned short xkey, xkey7;
    static CHAR mybyte;
    int count = 1;

    xkey = (unsigned short) c;

    if ( tt_kb_glgr ) {
        if (xkey <= 127)
        {
            if ( txrinfo[tcsl]->size != 94 )
                xkey = (*xl_u[TX_ASCII])(xkey);
            else
            {
                if (GL->ltoi)
                    xkey = (*GL->ltoi)(xkey);
            }
            if ( tt_utf8 ) {
                count = ucs2_to_utf8( key, bytes );
                return(count);
            }
            else if (GL->itor)
                xkey = (*GL->itor)(xkey);
        }
        else
        {
            if ( tt_kb_mode == KBM_HE )
            {
                xkey7 = xkey = xl_u[TX_CP862](xkey);
            }
            else if ( tt_kb_mode == KBM_RU )
            {
                xkey7 = xkey = xl_u[TX_CP866](xkey);
            }
            else {
                if (GL->ltoi)
                    xkey7 = (*GL->ltoi)(xkey);
                else
                    xkey7 = 0xFFFF;

                if (GR->ltoi)
                    xkey = (*GR->ltoi)(xkey);
            }

            if ( tt_utf8 ) {
                count = ucs2_to_utf8( xkey, bytes );
                return(count);
            }
            else {
                if ( GL->itor )
                    xkey7 = (*GL->itor)(xkey7);
                else
                    xkey7 = 0xFFFF;

                if ( GR->itor)
                    xkey = (*GR->itor)(xkey);
                else
                    xkey = 0xFFFF;

                if ( xkey7 != 0xFFFF )
                    xkey = xkey7;
            }
        }
    } else {
        /* Use Keyboard Character-set based upon  */
        /* state of DECNRCM for VT terminals or   */
        /* CH.CODE and bit mode for SNI terminals */

        if (xkey > 127) {
            if ( tt_kb_mode == KBM_HE )
            {
                xkey = xl_u[TX_CP862](xkey);
            }
            else if ( tt_kb_mode == KBM_RU )
            {
                xkey = xl_u[TX_CP866](xkey);
            }
            else if ( IS97801(tt_type_mode) ) {
                debug(F111,"ltorxlat()","xkey > 127",xkey);
                debug(F111,"ltorxlat()","tcsl",tcsl);
                xkey = xl_u[tcsl](xkey);
                debug(F111,"ltorxlat()","xl_u[tcsl](xkey)",xkey);
            } else
                xkey = xl_u[tcsl](xkey);
        }

        if ( tt_utf8 ) {
            count = ucs2_to_utf8( xkey, bytes );
            return(count);
        } else if ( ISVT100(tt_type_mode) && decnrcm ||
                    IS97801(tt_type_mode) && sni_bitmode == 7 && !sni_chcode) {
            xkey = xl_tx[dec_nrc](xkey);
            if ( IS97801(tt_type_mode) ) {
                debug(F111,"ltorxlat() sni=7 !ch.code","xl_tx[decnrc]",xkey);
            }
        } else if (IS97801(tt_type_mode) && sni_bitmode == 7 && sni_chcode) {
            if ( IS97801(tt_type_mode) )
                debug(F100,"ltorxlat() sni=7 ch.code","",0);
            if ( xkey > 127 ) {
                bytes = NULL;
                return(0);
            }
            /* otherwise, we use US-ASCII - no translation necessary */
        } else if ( cs_is_nrc(dec_kbd) ) {
            xkey = xl_tx[dec_kbd](xkey);
        } else if ( xkey > 127 ) {
            xkey = xl_tx[dec_kbd](xkey);
        }
    }
    if ( xkey == 0xFFFF ) {
        debug(F110,"ltorxlat()","xkey == 0xFFFF",0);
        bytes = NULL;
        return(0);
    }

    mybyte = (unsigned char) xkey;
    *bytes = &mybyte;
    return(1);
}

int
utorxlat( int c, CHAR ** bytes )
{
    unsigned short xkey, xkey7;
    static CHAR mybyte;
    int count = 1;

    xkey = (unsigned short) c;

    if ( tt_kb_glgr ) {
        if ( tt_utf8 ) {
            count = ucs2_to_utf8( key, bytes );
            return(count);
        }
        else if (GL->itor)
            xkey = (*GL->itor)(xkey);
    } else {
        /* Use Keyboard Character-set based upon  */
        /* state of DECNRCM for VT terminals or   */
        /* CH.CODE and bit mode for SNI terminals */

        if ( tt_utf8 ) {
            count = ucs2_to_utf8( xkey, bytes );
            return(count);
        } else if ( ISVT100(tt_type_mode) && decnrcm ||
                    IS97801(tt_type_mode) && sni_bitmode == 7 && !sni_chcode) {
            xkey = xl_tx[dec_nrc](xkey);
            if ( IS97801(tt_type_mode) ) {
                debug(F111,"ltorxlat() sni=7 !ch.code","xl_tx[decnrc]",xkey);
            }
        } else if (IS97801(tt_type_mode) && sni_bitmode == 7 && sni_chcode) {
            if ( IS97801(tt_type_mode) )
                debug(F100,"ltorxlat() sni=7 ch.code","",0);
            if ( xkey > 127 ) {
                bytes = NULL;
                return(0);
            }
            /* otherwise, we use US-ASCII - no translation necessary */
        } else if ( cs_is_nrc(dec_kbd) ) {
            xkey = xl_tx[dec_kbd](xkey);
        } else if ( xkey > 127 ) {
            xkey = xl_tx[dec_kbd](xkey);
        }
    }
    if ( xkey == 0xFFFF ) {
        debug(F110,"ltorxlat()","xkey == 0xFFFF",0);
        bytes = NULL;
        return(0);
    }

    mybyte = (unsigned char) xkey;
    *bytes = &mybyte;
    return(1);
}

int
rtolxlat( int c )
{
    int cx;
    /* Do not perform translations if we are VTNT */
    if ( tcs_transp || ISVTNT(tt_type_mode) )
        return(c);

    debug(F101,"rtolxlat c","",c);
    debug(F101,"rtolxlat tcsl","",tcsl);

    if ( ((ISANSI(tt_type_mode) || ISQANSI(tt_type_mode)) && sco8bit ||
           ISWY60(tt_type_mode) && wyse8bit)
         && c >= 32 )
        c |= 0200 ;             /* Shift 8th bit */

    if ( SSGL == NULL )
        GNOW = c > 127 ? GR : GL ;
    else {
        GNOW = c > 127 ? GR : SSGL ;
        SSGL = NULL ;
    }

    /* SNI 97801 has special translations if CH.CODE is ON */
    if ( IS97801(tt_type_mode) && sni_chcode ) {
        if ( sni_bitmode == 8 && c > 127 ) {
            /* 8-bit mode uses Latin-1 */
            cx = xl_u[TX_8859_1](c);
            if ( !isunicode() )
                cx = xl_tx[tcsl](cx);
            debug(F101,"rtolxlat return 1","",cx);
            return(cx);
        } else if (sni_bitmode == 7 && GNOW == &G[0] ){
            /* 7-bit mode uses dec-nrc */
            cx = xl_u[TX_ASCII](c);
            debug(F101,"rtolxlat return 2","",cx);
            return(cx);
        }
        /* otherwise, perform a normal translation */
    }

    if ( !debses && tcs_transp ) {
        if ( GNOW == GR && !GNOW->national ) {
            c |= 0200;
            literal_ch = TRUE ;
        } else
            c &= ~0200;

        c &= cmdmsk;                        /* Apply command mask. */
    } else {
        if (!debses &&                      /* Not DEBUG mode       */
             c >= 32 &&                     /* Not Control-0 char   */
             !tcs_transp)                   /* Not transparent set */
            c |= 0200;                      /* Shift char to 8-bit  */

        if ( GNOW->national )
            c &= ~0200 ;                    /* Shift to 7-bit */

        c &= cmdmsk;                        /* Apply command mask. */

        /* Translate character sets */
        cx = c ;                    /* Save the old value */
        if ( !(GNOW == GL && c < 32) ) {
            if (GNOW->rtoi)
                c = (*GNOW->rtoi)(c);
            if ( !isunicode() )
            {
                if (GNOW->itol )
                    c = (*GNOW->itol)(c);
            }
#ifdef NT
            else
            {
                if ( win95hsl && c >= 0x23BA && c <= 0x23BD )
                    c = tx_hslsub(c);
                else if ( c >= 0xE000 && c <= 0xF8FF )
                    c = tx_usub(c);
                if (win95lucida && c > 0x017f)
                    c = tx_lucidasub(c);
            }
#endif /* NT */
            if ( c < 0 )            /* Character could not be xlated */
                c = '?' ;           /* to local character set */
        }
        if ( cx >= SP && c < SP ||
             c >=128 && c <= 159)
            literal_ch = TRUE ;
        debug(F101,"rtolxlat return 3","",c);
    }
    return(c);
}

int
utolxlat( int c )
{
    c = xl_tx[tcsl](c);
    return(c);
}

/*
  S C R I P T W R T B U F
  This function performs equivalent functionality for [M]INPUT commands
  as RDCOMWRTSCR does for CONNECT sessions.  Must process incoming
  Telnet operations and perform character set translations.

  And since it does exactly the same thing that RDCOMWRTSCR has done for
  years, there is no point in duplicating the code.  So now RDCOMWRTSCR
  calls SCRIPTWRTBUF to do the hard work.
*/
int
scriptwrtbuf(unsigned short word)
/* scriptwrtbuf */ {
    int c, cx, tx;
#ifdef CK_TRIGGER
    extern char * tt_trigger[], * triggerval;
#endif /* CK_TRIGGER */

    debug(F111,"scriptwrtbuf","word",word);

    /* Close Printer if necessary */
    if ( printerclose_t ) {
        if ( printerclose_t < time(NULL) ) {
            printerclose();
        }
    }

pushed:
    if (f_pushed) {             /* Handle 8-bit controls */
        c = c_pushed;           /* by converting to ESC + 7-bit char */
        f_pushed = 0;
        f_popped = 1;
    }
    else
    {
        c = word;               /* Get a character from the host */

#ifdef TNCODE
        /* Handle TELNET negotiations here */
        if (network && IS_TELNET()) {
            static int got_cr = 0;
            switch ( c ) {
            case IAC:
                if ((tx = tn_doop((CHAR)(c & 0xff),duplex,netinc)) == 0) {
                    return(0);
                }
                else if (tx == -1) {    /* I/O error */
                    strcpy(termessage, "TELNET negotiation error.\n");
                    SetConnectMode(FALSE,CSX_TN_ERR);      /* Quit */
                    return(-1);
                }
                else if (tx == -2) { /* Connection failed. */
                    extern int ttyfd ;
                    SetConnectMode(FALSE,CSX_HOSTDISC);
                    strcpy(termessage, "Connection closed by peer.\n");
                    ttclos(0) ;
                    return(-2);
                }
                else if (tx == -3) { /* Connection failed. */
                    extern int ttyfd ;
                    SetConnectMode(FALSE,CSX_TN_POL);
                    strcpy(termessage, "Connection closed due to telnet policy.\n");
                    ttclos(0) ;
                    return(-2);
                }
                else if (tx == 1) {     /* ECHO change */
                    duplex = 1; /* Get next char */
                    return(1);
                }
                else if (tx == 2) {     /* ECHO change */
                    duplex = 0; /* Get next char */
                    return(2);
                }
                else if (tx == 3) {     /* Quoted IAC */
                    c = 255;    /* proceeed with it. */
                }
#ifdef IKS_OPTION
                else if (tx == 4) {     /* IKS State Change */
                    if ( TELOPT_SB(TELOPT_KERMIT).kermit.u_start &&
                         !tcp_incoming) {
                        SetConnectMode(FALSE,CSX_IKSD);
                    }
                    return(4);
                }
#endif /* IKS_OPTION */
                else if (tx == 6) {
                    strcpy(termessage, "Remote Logout.\n");
                    SetConnectMode(FALSE,CSX_HOSTDISC);      /* Quit */
                    return(6);
                }
                else return(0); /* Unknown, get next char */
                break;
            case CR:
                got_cr = 1;
                break;
            case NUL:
                if ( !TELOPT_U(TELOPT_BINARY) && got_cr ) {
                    got_cr = 0;
                    return(0);
                }
            default:
                got_cr = 0;
            }
            if (IsConnectMode() &&
                TELOPT_ME(TELOPT_ECHO) && tn_rem_echo) { /* I'm echo'ing incoming data */
                ttoc(c);
            }
        }
#endif /* TNCODE */

        /* Output to the session log */
        /* The session log should be a pure log of what the host */
        /* is sending minus the Telnet stuff.  This way the file */
        /* can be used as an input stream to the emulator for    */
        /* testing bugs.  This can't happen if we perform C1 to  */
        /* ESC C0 and Character-set conversions.                 */
        if (seslog)
            logchar((USHORT)c);

        /* The purpose of copy-print is to allow K95 to display */
        /* the terminal output on the K95 screen while at the   */
        /* same time piping the raw data through a terminal     */
        /* hooked to the printer device (possibly in bi-di mode */
        /* so we print the raw character here and make cprint   */
        /* supercede the xprint flag.                           */
        /* If the terminal type is Wyse don't print the DC4     */
        /* which is going to result in cprint being turned off. */
        if (cprint && !(ISWYSE(tt_type_mode) && c == DC4))
            prtchar(c);

        f_popped = 0;

        /* Handle the UTF8 conversion if we are in that mode */
        if ( tt_utf8 ) {
            USHORT * ucs2 = NULL;
            int rc = utf8_to_ucs2( (CHAR)(c & 0xFF), &ucs2 );
            if ( rc > 0 )
                return(0);
            else {
                if (rc < 0) {
                    c = 0xfffd;
                    f_pushed = 1;
                    c_pushed = *ucs2;
                } else
                    c = *ucs2;

                if (c == 0x2028 || c == 0x2029) { /* LS or PS */
                    c = CR;
                    f_pushed = 1;
                    c_pushed = LF;
                }
                if ( isunicode() ) {
                    if ( win95hsl && c >= 0x23BA && c <= 0x23BD )
                        c = tx_hslsub(c);
                    else if ( c >= 0xE000 && c <= 0xF8FF )
                        c = tx_usub(c);
                    if (win95lucida && c > 0x017f)
                        c = tx_lucidasub(c);
                } else {        
                    cx = c;
                    c = xl_tx[tcsl](cx);
                    if ( c < 0 )                /* Character could not be xlated */
                        c = '?' ;               /* to local character set */
                    if ( cx >= SP && c < SP ||
                         c >=128 && c <= 159)
                        literal_ch = TRUE ;
                }
            }
        }

        if ((ISVT220(tt_type_mode) ||
             ISANSI(tt_type_mode)) &&
             !xprint ) {                /* VT220 and above... */
                cx = tt_utf8 ? c : c & cmask & pmask;   /* C1 check must be masked */

            if (!tt_utf8 && ( GR->c1 ) &&
                 (cx > 127) && (cx < 160) /* It's a C1 character */
                 ) {
                f_pushed = 1;
                c_pushed = (c & 0x7F) | 0x40;
                c = ESC;
            }
        }
    }

    if (c >= 0) {                       /* Got character with no error? */
        if ( !xprint ) {
            if (!tt_utf8)
               c = c & cmask & pmask ;  /* Maybe strip 8th bit */
#ifndef NOXFER
            if ( (IsConnectMode() && autodl) ||
                 (!IsConnectMode() && inautodl) )
                autodown( c ) ;                 /* Download? */
#endif /* NOXFER */
            if (escstate == ES_NORMAL) {
                if ( !tt_utf8 )
                    c = rtolxlat(c);
            }
            else {
                c &= cmdmsk;            /* Apply command mask. */
            }

            if ((!debses) &&
                 ((tnlm &&              /* NEWLINE-MODE? */
                    (c == LF || c == FF || c == 11)) ||
                   (c == CR && tt_crd ) /* CR-DISPLAY CRLF ? */
                   )) {
                cwrite((USHORT) CR);    /* Yes, output CR */
                c = LF;                 /* and insert a linefeed */
            }
            cwrite((USHORT) c);
        } else {
            if (!GR->c1 && c >=128 && c <= 159) {
                literal_ch = TRUE;
                cwrite((USHORT) c);
                literal_ch = FALSE;
            } else
                cwrite((USHORT) c);
        }

#ifdef CK_TRIGGER
        if ( !xprint && tt_trigger[0] ) {
            int trigger = autoexitchk(c);
            if ( trigger > -1 ) {
                if (triggerval) /* Make a copy of the trigger */
                    free(triggerval);
                triggerval = NULL;
                triggerval = strdup(tt_trigger[trigger]);
                debug(F110,"scriptwrtbuf() triggerval",triggerval,0);
                ckmakmsg(termessage, sizeof(termessage),
                         "Trigger \"", triggerval,
                         "\" found.\n", NULL);
                SetConnectMode(FALSE,CSX_TRIGGER);
            }
        }
#endif /* CK_TRIGGER */
    }
    if (f_pushed)
        goto pushed ;

    debug(F100,"scriptwrtbuf returns","",0);
    return(0);
}

int
CSisNRC( int x )
{
    return cs_is_nrc(x);
}

/* Character-set final characters
 *
 *    A - British
 *    B - ASCII
 *    C - Finnish
 *    E - Norwegian/Dutch
 *    H - Swedish
 *    K - German
 *    Q - French Canadian
 *    R - French
 *    Y - Italian
 *    Z - Spanish
 *    0 - DEC Special Graphics
 *    4 - Dutch
 *    5 - Finnish
 *    6 - Norwegian/Dutch
 *    7 - Swedish
 *    < - DEC supplemental
 *    = - Swiss
 *
 */

int
isNRC( int x )
{
    switch ( x ) {
    case 'A':           /* FC_UKASCII TX_BRITISH */
    case 'B':           /* FC_USASCII TX_ASCII */
    case 'C': case '5': /* FC_FIASCII TX_FINNISH */
    case 'E': case '6': /* FC_NOASCII TX_NORWEGIAN */
    case 'H': case '7': /* FC_SWASCII TX_SWEDISH */
    case 'K':           /* FC_GEASCII TX_GERMAN */
    case 'Q':           /* FC_FCASCII TX_CN_FRENCH */
    case 'R': case 'f': /* FC_FRASCII TX_FRENCH */
    case 'Y':           /* FC_ITASCII TX_ITALIAN */
    case 'Z':           /* FC_SPASCII TX_SPANISH */
    case '4':           /* FC_DUASCII TX_DUTCH */
    case '=':           /* FC_CHASCII TX_SWISS */
    case '1':           /* DEC Alternate ROM - ASCII */
        return 1;
    default:
        return 0;
    }
}


/* See http://www.itscj.ipsj.or.jp/ISO-IR/ for Internation Char Set Registry */
unsigned char
charset( enum charsetsize size, unsigned short achar, struct _vtG * pG )
{
    unsigned char cs = TX_UNDEF ;
    unsigned char bchar ;

    switch ( size ) {
    case cs94:
        switch ( achar ) {
#ifdef SN97801_5XX
        case '@':       /* International (new) */
            break;
        case 'B':       /* International A (US-ASCII) */
            break;
        case 'K':       /* German character set (GR-ASCII) */
            break;
        case 'w':       /* Brackets character set (new) */
            break;
        case 'c':       /* FACET character set (new) */
            break;
        case 'v':       /* IBM character set (new) */
            break;
        case 'u':       /* EURO symbols (new) */
            break;
        case 't':       /* Mathematics symbols (new) */
            break;
        case 'y':       /* Blanks (new) */
            break;
        case 'x':       /* Load Area G2 (7-bit) or DRCS area (8-bit) if G0/G1 */
                        /* DRCS area always if G2/G3 */
            break;
#endif /* SN97801_5XX */
        case '@':  /* 97801 - Not quite US ASCII but close */
        case 'A':
            cs = TX_BRITISH ;
            break;
        case 'B':       /* Use the default value for G[0] */
            if (ISLINUX(tt_type_mode)) {
                if ( pG == &G[0] )
                    cs = TX_ASCII;
                else
                    cs = TX_8859_1 ;
            } else if ( !(ISVT100(tt_type_mode) && decnrcm) ) {
                /*
                From VT330/340 install guide page 81.:
                "When you select an NRC set in multinational mode,
                the NRC set replaces the ASCII set."
                */
#ifdef COMMENT
                cs = G[0].def_designation;
#else /* COMMENT */
                cs = dec_nrc;
#endif /* COMMENT */
            }
            else {
                cs = TX_ASCII ;
            }
            break;
        case '1':       /* DEC Alternate ROM */
            cs = TX_ASCII ;
            break;
        case 'C':
        case '5':
            cs = TX_FINNISH ;
            break;
        case 'E':
        case '6':
        case '`':
            cs = TX_NORWEGIAN ;
            break;
        case 'G':
        case 'H':
        case '7':
            cs = TX_SWEDISH ;
            break;
        case 'K':
            if ( ISLINUX(tt_type_mode) ) {       /* user defined */
                if (pG == &G[0]) {
                    cs = TX_ASCII ;
                } else {
                    cs = TX_CP850;               /* we will choose one */
                }
            } else
                cs = TX_GERMAN ;
            break;
        case 'Q':
        case '9':
            cs = TX_CN_FRENCH ;
            break;
        case 'R':
        case 'f':
            cs = TX_FRENCH ;
            break;
        case 'Y':
            cs = TX_ITALIAN ;
            break;
        case 'Z':
            cs = TX_SPANISH ;
            break;
        case '0':
            cs = TX_DECSPEC ;
            break;
        case '>':
        case '2':       /* DEC Alternate ROM - Special Graphics */
            cs = TX_DECTECH ;
            break;
        case '4':
            cs = TX_DUTCH ;
            break;
        case '<':
            if ( ISVT320(tt_type_mode) ) {
                /* DEC User Preferred Supplemental VT320 and higher */
                cs = dec_upss ;
            }
            else {
                cs = TX_DECMCS ;
            }
            break;
        case '=':
            cs = TX_SWISS ;
            break;
        case '%':
            bchar = (escnext<=esclast)?escbuffer[escnext++]:0;
            if (bchar == '5') {
                cs = TX_DECMCS ;
            }
            else if (bchar == '6') {
                cs = TX_PORTUGUESE ;
            }
#ifdef COMMENT
            else if (bchar == '0') {
                cs = DEC TURKISH ;
            }
            else if (bchar == '=') {
                cs = HEBREW NRCS ;
            }
            else if ( bchar == '2' )
                cs = TURKISH NRCS;
            else if ( bchar == '3' )
                cs = SCS NRCS;
#endif /* COMMENT */
            break;
        case 'L':
            cs = TX_PORTUGUESE ;
            break;
        case 'i':
            cs = TX_HUNGARIAN ;
            break;
        case 'J':
            cs = TX_J201R ;
            break;
        case 'I':
            cs = TX_J201K ;
            break;
        case '*':
            cs = TX_IBMC0GRPH;  /* QANSI/Linux */
            break;
        case 'U':               /* QANSI/Linux */
        case '?':               /* This is a MSK hack for Word Perfect */
            if (pG == &G[0]) {
                cs = TX_ASCII ;
            } else {
                cs = TX_CP437 ;
            }
            break;
        case 'c':       /* FACET character set (new) */
            if ( IS97801(tt_type_mode) ) {
                cs = TX_SNIFACET;
            }
            break;
        case 'e':       /* APL-ISO */
            cs = TX_APL1;
            break;
        case 'u':       /* EURO symbols (new) */
            if ( IS97801(tt_type_mode) ) {
                cs = TX_SNIEURO;
            }
            break;
        case 'v':       /* IBM character set (new) */
            if ( IS97801(tt_type_mode) ) {
                cs = TX_SNIIBM;
            }
            break;
        case 'w':       /* Brackets character set (new) */
            if ( IS97801(tt_type_mode) ) {
                cs = TX_SNIBRACK;
            }
            break;
        case ' ':  /* space */
            bchar = (escnext<=esclast)?escbuffer[escnext++]:0;
            switch (bchar) {
            case '@':   /* soft character set */
                /* not supported - do nothing */
                debug(F100,"charset - host tried to activate soft-character-set","",0);
                break;
            }
            break;
#ifdef COMMENT
        case '"':
            if ( bchar == '?' )
                cs = DEC GREEK;
            else if ( bchar == '4' )
                cs = DEC HEBREW;
            else if ( bchar == '>' )
                cs = GREEK NRCS;
            break;
        case '&':
            if ( bchar == '4' )
                cs = DEC CYRILLIC;
            else if ( bchar == '5' )
                cs = RUSSIAN NRC;
            break;
#endif /* COMMENT */

        }
        break;

    case cs96:
        switch ( achar ) {
#ifdef SN97801_5XX
        case 'A':       /* 8859-1 */
        case 'B':       /* 8859-2 */
        case 'C':       /* 8859-3 */
        case 'D':       /* 8859-4 */
        case 'F':       /* 8859-5 (different) */
        case '@':       /* 8859-7 (different) */
        case 'T':       /* 8859-9 (different) */
        case 'x':       /* DRCS area */
#endif /* SN97801_5XX */
        case '<':       /* DEC User-preferred Supplemental */
            cs = dec_upss ;
            break;

        case 'A':
            cs = TX_8859_1 ;
            break;
        case 'B':
            cs = TX_8859_2 ;
            break;
        case 'C':
            cs = TX_8859_3 ;
            break;
        case 'D':
            cs = TX_8859_4 ;
            break;
        case 'F':
            cs = IS97801(tt_type_mode) ? TX_8859_5 : TX_8859_7 ;
            break;
        case '@':
            cs = TX_8859_7 ;
            break;
        case 'G':
            cs = TX_8859_6 ;
            break;
        case 'H':
            cs = TX_8859_8 ;
            break;
        case 'L':
            cs = TX_8859_5 ;
            break;
        case 'T': /* SNI-97801 */
            if ( !IS97801(tt_type_mode) )
                break;

        case 'M': /* DEC VT3xx */
            cs = TX_8859_9 ;
            break;

        case 'V':
            cs = TX_8859_6 ;
            break;

        case '%':
            bchar = (escnext<=esclast)?escbuffer[escnext++]:0;
            if (bchar == '5') {
                cs = TX_DECMCS ;
            }
        case '*':
            cs = TX_IBMC0GRPH;  /* QANSI/Linux */
            break;
        case 'U':               /* QANSI/Linux */
            /* fall through */
        case '?':               /* This is a MSK hack for Word Perfect */
            if (pG == &G[0]) {
                cs = TX_ASCII ;
            } else {
                cs = TX_CP437 ;
            }
            break;
        case 'b':
            cs = TX_8859_15;    /* Latin 9 */
            break;

            /* Warning the following have been allocated officially */
            /* the values below are not official.                   */
        case 'd':               /* Hack alert: DGI */
            cs = TX_DGI ;
            break;
        case 'e':               /* Hack alert: DG PC Graphics */
            cs = TX_DGPCGRPH;
            break;
        case 'f':               /* Hack alert: DG Line Drawing */
            cs = TX_DGLDGRPH;
            break;
        case 'g':               /* Hack alert: DG Word Processing Graphics */
            cs = TX_DGWPGRPH;
            break;
        case 'h':               /* Hack alert: HP Roman-8 */
            cs = TX_HPR8;
            break;
        case 'i':               /* Hack alert: HP Math-8 */
            cs = TX_HPMATH;
            break;
        case 'j':               /* Hack alert: HP Line-8 */
            cs = TX_HPLINE;
            break;
        }
    }

    if ( cs <= MAXTXSETS ) {
        pG->designation = cs ;
        pG->size = size ;
        pG->c1 = cs_is_std(cs) ;
        pG->national = CSisNRC( cs ) ;
        pG->rtoi = xl_u[cs] ;
        pG->itol = xl_tx[tcsl] ;
        pG->ltoi = xl_u[tcsl] ;
        pG->itor = xl_tx[cs] ;
    }
    else if ( cs != TX_UNDEF ) {
        debug(F110,"charset - ERROR","cs != TX_UNDEF",0);
        pG->designation = cs ;
        pG->c1 = FALSE ;
        pG->national = FALSE;
        pG->rtoi = NULL ;
        pG->itol = NULL ;
        pG->itor = NULL ;
        pG->ltoi = NULL ;
    }
    return cs ;
}

void
resetcolors( int x )
{
    if ( !x )
        attribute = defaultattribute ;
    else {
        if ( decscnm ) {
            defaultattribute =
                byteswapcolors(colornormal);
            underlineattribute =
                byteswapcolors(colorunderline);
            italicattribute = 
                byteswapcolors(coloritalic);
            reverseattribute =
                byteswapcolors(colorreverse);
            graphicattribute =
                byteswapcolors(colorgraphic);
        }
        else {
            defaultattribute = colornormal ;
            underlineattribute = colorunderline;
            italicattribute = coloritalic;
            reverseattribute = colorreverse;
            graphicattribute = colorgraphic;
        }
        attribute = defaultattribute ;
        borderattribute = colorborder ;
    }
}

/*---------------------------------------------------------------------------*/
/* movetoscreen                                                              */
/*---------------------------------------------------------------------------*/
void
movetoscreen(char *source, int x, int y, int len) {
   /* x and y begin at 1 */

   int c,l=0 ;
   videoline * line = NULL ;

   line = VscrnGetLineFromTop(VTERM, y-1) ;
   while (l<len) {
      if ( x-1 == VscrnGetWidth(VTERM) ) {
         x=1 ;
         y++ ;
         line = VscrnGetLineFromTop(VTERM, y-1) ;
         }
      line->cells[x-1].c = source[l] ;
      x++ ;
      }
    VscrnIsDirty(VTERM);
}

void
os2debugoff() {                         /* Turn off debugging from outside */
    debug(F100,"os2debugoff","",0);
    attribute = defaultattribute;
}

void
os2bold() {                             /* Toggle boldness from the outside */
    tn_bold = 1 - tn_bold;
}

void                            /* Toggle session debugging */
flipdebug() {
    if (debses) {
        attribute = defaultattribute;   /* Back to normal coloring */
        setdebses(0);                     /* Turn off session debug flag */
    } else {
        setdebses(1);                     /* Turn on session debug flag */
    }
    if (!scrollflag[VTERM] && status_saved < 0)
      ipadl25();
}


#ifdef NETCONN
#ifdef TCPSOCKET
do_tn_cmd(CHAR x) {
    CHAR temp[3];

    if (network && IS_TELNET()) { /* TELNET */
        temp[0] = (CHAR) IAC;
        temp[1] = x;
        temp[2] = NUL;
        ttol((CHAR *)temp,2);

        if (tn_deb || debses || deblog) {
            extern char tn_msg[];
            ckmakmsg(tn_msg,TN_MSG_LEN,"TELNET SENT ",TELCMD(x),NULL,NULL);
            debug(F101,tn_msg,"",x);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
    } else
      bleep(BP_FAIL);
    return(0);
}
#endif /* TCPSOCKET */
#endif /* NETCONN */

#ifndef NOKVERBS
void
bookmarkset( int vmode )
{
    int mark = 0 ;
    CHAR x1;
    con_event evt ;

    save_status_line();
    escapestatus[vmode] = TRUE ;
    strcpy(exittext,"Cancel: Space"); /* Make special one */
    strcpy(usertext," SET MARK: [ ]"); /* with mini-echo-buffer */
    if (vik.help > 255 && keyname(vik.help))
        sprintf(helptext, "Help: %s", keyname(vik.help));       /* safe */
    VscrnIsDirty(vmode);  /* status line needs to be updated */

    do {
        evt = congev(vmode,-1) ;
        switch (evt.type) {
        case key:
#ifdef COMMENT
            x1 = mapkey(evt.key.scancode); /* Get value from keymap */
#else
            x1 = evt.key.scancode ;
#endif
            break;
        case kverb:
            x1 = evt.kverb.id & ~F_KVERB;
            break;
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
        case csi:
        case esc:
        default:
            x1 = 0 ;
        }
        if ( x1 == K_HELP )
            popuphelp(vmode,hlp_bookmark);
    } while ( x1 == K_HELP );
    if ((x1 < '0') || (x1 > '9')) {
        if (x1 != SP)
            bleep(BP_FAIL);
        goto bookmark_exit;
    }

    sprintf(usertext," SET MARK: [%c]", x1); /* Echo char */
    VscrnIsDirty(vmode);  /* status line needs to be updated */
    msleep(500);      /* let the user see it for a bit */
    mark = x1 - '0' ;
    VscrnSetBookmark( vmode, mark,
                      (tt_roll[vmode] && scrollflag[vmode]) ?
                      VscrnGetScrollTop(vmode) : VscrnGetTop(vmode) ) ;

  bookmark_exit:                        /* Common exit point */
    escapestatus[vmode] = FALSE ;
    restore_status_line();              /* Restore status line */
    return;
}

int
bookmarkget( int vmode )
{
   return VscrnGetBookmark( vmode, 0 ) ;
}

void
bookmarkreset( int vmode )
{
   VscrnSetBookmark( vmode, 0, -1 ) ;
}

void
bookmarkjump( int vmode )
{
    int mark = 0, bookmark ;
    CHAR x1;
    con_event evt ;

    save_status_line();
    escapestatus[vmode] = TRUE ;
    strcpy(exittext,"Cancel: Space"); /* Make special one */
    strcpy(usertext," GO MARK: [ ]"); /* with mini-echo-buffer */
    if (vik.help > 255 && keyname(vik.help))
        sprintf(helptext, "Help: %s", keyname(vik.help));
    VscrnIsDirty(vmode);  /* status line needs to be updated */

    do {
        evt = congev(vmode,-1) ;
        switch (evt.type) {
        case key:
#ifdef COMMENT
            x1 = mapkey(evt.key.scancode); /* Get value from keymap */
#else
            x1 = evt.key.scancode ;
#endif
            break;
        case kverb:
            x1 = evt.kverb.id & ~F_KVERB;
            break;
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
        case csi:
        case esc:
        default:
            x1 = 0 ;
        }
        if ( x1 == K_HELP )
            popuphelp(vmode,hlp_bookmark);
    } while ( x1 == K_HELP );
    if ((x1 < '0') || (x1 > '9')) {
        if (x1 != SP)
            bleep(BP_FAIL);
        goto bookmark_exit;
    }

    sprintf(usertext," GO MARK: [%c]", x1); /* Echo char */
    VscrnIsDirty(vmode);  /* status line needs to be updated */
    msleep(500);      /* let the user see it for a bit */
    mark = x1 - '0' ;
    bookmark = VscrnGetBookmark( vmode, mark ) ;

    if ( bookmark < 0 )
    {
        bleep(BP_FAIL);
        goto bookmark_exit;
    }

    if (!tt_roll[vmode]) {
        if ( VscrnSetTop(vmode, bookmark) < 0 )
            bleep(BP_WARN) ;
    }
    else {
        if ( VscrnSetScrollTop(vmode, bookmark) < 0 )
            bleep(BP_WARN);
    }

  bookmark_exit:                        /* Common exit point */
    restore_status_line();              /* Restore status line */
    escapestatus[vmode] = FALSE ;
    return;
}

void
gotojump( int vmode )
{
   int line = 0, negative = 0, maxval = 0 ;
   CHAR x1;
   con_event evt ;

   save_status_line();
   escapestatus[vmode] = TRUE ;

   do {
      maxval = (VscrnGetEnd(vmode) - VscrnGetBegin(vmode) - VscrnGetHeight(vmode)
                 + VscrnGetBufferSize(vmode))%VscrnGetBufferSize(vmode) ;

      if ( negative && line < -maxval )
      {
         line = -maxval ;
         bleep(BP_NOTE);
      }

      if ( !negative && line > (maxval+1) )
      {
         line = maxval+1 ;
         bleep(BP_NOTE);
      }

       strcpy(exittext,"Cancel: Space"); /* Make special one */
       sprintf(usertext," GOTO: [%10d]",line); /* with mini-echo-buffer */
       helptext[0] = '\0' ;
       if (vik.help > 255 && keyname(vik.help))
           sprintf(hostname, "Help: %s", keyname(vik.help));
       VscrnIsDirty(vmode);  /* status line needs to be updated */

       evt = congev(vmode,-1) ;
       switch (evt.type) {
       case key:
#ifdef COMMENT
           x1 = mapkey(evt.key.scancode); /* Get value from keymap */
#else
           x1 = evt.key.scancode ;
#endif
           break;
       case kverb:
           x1 = evt.kverb.id & ~F_KVERB;
           break;
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
       case csi:
       case esc:
       default:
           x1 = 0 ;
       }

       if ( x1 == K_HELP )
       {
           popuphelp(vmode,hlp_bookmark);
       }
       else if ( x1 == '-' )
       {
           negative = (negative ? FALSE : TRUE) ;
       }
       else if ( x1 >= '0' && x1 <= '9' )
       {
           line = line * 10 + (x1-'0') ;
       }
       else if ( x1 == 8 || x1 == 127 )
       {
           line = line / 10 ;
       }
       else if ( x1 == 13 )
       {
           break;
       }
       else
       {
           if ( x1 != ' ' )
               bleep(BP_FAIL);
           goto bookmark_exit;
       }

       if ( ( negative && line > 0 ) || ( !negative && line < 0 ) )
           line = -line ;

   } while ( TRUE );

    msleep(500);      /* let the user see it for a bit */

    if ( line <= 0 )
    {
        if (!tt_roll[vmode]) {
            if ( VscrnSetTop(vmode, VscrnGetEnd(vmode)-VscrnGetHeight(vmode)+line) < 0 )
                bleep(BP_WARN) ;
        }
        else {
            if ( VscrnSetScrollTop(vmode, VscrnGetEnd(vmode)-VscrnGetHeight(vmode)+line) < 0 )
                bleep(BP_WARN);
        }
    }
    else
    {
        if (!tt_roll[vmode]) {
            if ( VscrnSetTop(vmode, VscrnGetBegin(vmode)+line-1) < 0 )
                bleep(BP_WARN) ;
        }
        else {
            if ( VscrnSetScrollTop(vmode, VscrnGetBegin(vmode)+line-1) < 0 )
                bleep(BP_WARN);
        }
    }

  bookmark_exit:                        /* Common exit point */
    restore_status_line();              /* Restore status line */
    escapestatus[vmode] = FALSE ;
    return;
}

BOOL
search( BYTE vmode, BOOL forward, BOOL prompt )
{
#ifdef DCMDBUF
   extern int * inpcas;
#else
   extern int inpcas[] ;
#endif /* DCMDBUF */
   static char searchstring[63] = "" ;
   CHAR x1;
   con_event evt ;
   int line = 1 ;
   int found = 0 ;
   static int len = 0 ;
   static int row=-1, col=-1 ;

    if ( prompt )
    {
        save_status_line();
        escapestatus[vmode] = TRUE ;

        do {
            len = strlen( searchstring ) ;
            strcpy(exittext,"Cancel: ESC"); /* Special one with edit buffer */
            sprintf(usertext," %s: [%-16s]",forward ? "SEARCH>" : "<SEARCH",
                  len <= 16 ? searchstring : &searchstring[len-16] );
            helptext[0] = '\0' ;
            if (vik.help > 255 && keyname(vik.help))
                sprintf(hostname, "Help: %s", keyname(vik.help));
            VscrnIsDirty(vmode);  /* status line needs to be updated */

            evt = congev(vmode,-1) ;
            switch (evt.type) {
            case key:
#ifdef COMMENT
                x1 = mapkey(evt.key.scancode); /* Get value from keymap */
#else
                x1 = evt.key.scancode ;
#endif
                break;
            case kverb:
                x1 = evt.kverb.id & ~F_KVERB;
                break;
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
            case esc:
            case csi:
            default:
                x1 = 0 ;
            }

            if ( x1 == K_HELP )
            {
                popuphelp(vmode,hlp_search);
            }
            else if ( x1 >= ' ' && x1 <= 126 || x1 >= 128 && x1 <= 255 )
            {
                if ( len >= 62 ) {
                    bleep(BP_WARN);
                }
                else {
                    searchstring[len] = x1 ;
                    searchstring[len+1] = '\0' ;
                }
            }
            else if ( x1 == 8 || x1 == 127 )
            {
                searchstring[len-1] = '\0' ;
            }
            else if ( x1 == 13 )
            {
                break;
            }
            else if ( x1 == 21 )
            {
                searchstring[0] = '\0' ;
                len = 0 ;
            }
            else
            {
                if ( x1 != ESC )
                    bleep(BP_FAIL);
                goto search_exit;
            }
        } while ( TRUE );

        msleep(500);      /* let the user see it for a bit */
        restore_status_line();          /* Restore status line */
        escapestatus[vmode] = FALSE ;

        col = VscrnGetCurPos(vmode)->x ;
        row = (( markmodeflag[vmode] ? VscrnGetTop(vmode) : VscrnGetScrollTop(vmode) )
                + VscrnGetCurPos(vmode)->y)%VscrnGetBufferSize(vmode) ;
    }
    else
    {
        if ( row < 0 || col < 0 || !searchstring || !searchstring[0] )
        {
            bleep(BP_FAIL);
            goto search_exit ;
        }

        /* advance the cursor */
        if ( forward )
        {
            col++ ;
            if ( col == vscrn[vmode].lines[row%vscrn[vmode].linecount].width )
            {
                if ( row != VscrnGetEnd(vmode) )
                {
                    col = 0 ;
                    row++ ;
                }
                else
                {
                    col--;
                    goto search_exit;
                }
            }
            if ( row >= VscrnGetBufferSize( vmode ) )
                row = 0 ;
        }
        else
        {
            col-- ;
            if ( col < 0 )
            {
                if ( row != VscrnGetBegin(vmode) )
                {
                    row-- ;
                    col = vscrn[vmode].lines[row%vscrn[vmode].linecount].width - 1 ;
                }
                else
                {
                    col++;
                    goto search_exit;
                }
            }
            if ( row < 0 )
                row = VscrnGetBufferSize(vmode) -1 ;
        }
    }

    /* Now we have the string to search for */
    /* so lets search for it ???            */
    {
        char *s = searchstring ;
        int y=len,i;
        char *xx, *xp, *xq = (char *)0;
        CHAR c;

        i = 0 ;                         /* String pattern match position */

        if (!inpcas[cmdlvl] || !forward)
        {               /* INPUT CASE = IGNORE?  */
            xp = malloc(y+2);           /* Make a separate copy of the */
            if (!xp)
            {                   /* search string. */
                row = col = -1 ;
                return(found);
            }
            else
                xq = xp;                        /* Keep pointer to beginning. */

            if ( !forward )
            {
                xp = xp + len ;
                *xp = NUL ;
                xp--;
            }

            while (*s) {                        /* Yes, convert to lowercase */
                *xp = *s;
                if (!inpcas[cmdlvl] && isupper(*xp))
                    *xp = tolower(*xp);
                if ( forward )
                    xp++;
                else
                    xp--;
                s++;
            }
            if ( forward )
                *xp = NUL;                      /* Terminate it! */
            s = xq;                             /* Move search pointer to it. */
        }

        while (TRUE)
        {
            c = vscrn[vmode].lines[row].cells[col].c;                   /* Get next character */
            if (!inpcas[cmdlvl])
            {           /* Ignore alphabetic case? */
                if (isupper(c))
                    c = tolower(c); /* Yes */
            }
            debug(F000,"search char","",c);
            debug(F000,"compare char","",(CHAR) s[i]);
            if (c == s[i])
            {           /* Check for match */
                if ( i==0 )
                    VscrnUnmarkAll( vmode ) ;
                VscrnMark( vmode, row, col, col );
                i++;                    /* Got one, go to next character */
            }
            else
            {                   /* Don't have a match */
                int j;
                for (j = i; i > 0; )
                {       /* [jrs] search backwards for it  */
                    i--;
                    if (c == s[i])
                    {
                        if (!strncmp(s,&s[j-i],i))
                        {
                            /* This code assumes the matching sub pattern */
                            /* appears entirely on one line.  But there   */
                            /* are situations where it might be split.    */
                            /* What then? */
                            VscrnUnmarkAll(vmode);
                            if ( forward )
                                VscrnMark( vmode, row, col-i, col ) ;
                            else
                                VscrnMark( vmode, row, col, col+i ) ;
                            i++;
                            break;
                        }
                    }
                }
            }                           /* [jrs] or return to zero from -1 */
            if (s[i] == '\0')
            {           /* Matched all the way to end? */
                VscrnMark( vmode, row, col, col ) ;
                found = 1;                      /* Yes, */
                line = row ;
                break;                  /* done. */
            }

            /* check to see if we hit the begin or end of vscrn */
            if ( forward && row == VscrnGetEnd(vmode) && col == vscrn[vmode].lines[row%vscrn[vmode].linecount].width-1 ||
                 !forward && row == VscrnGetBegin(vmode) && col == 0 )
                break;    /* search string not found */

            /* advance the cursor */
            if ( forward )
            {
                col++ ;
                if ( col == vscrn[vmode].lines[row%vscrn[vmode].linecount].width )
                {
                    if ( row != VscrnGetEnd(vmode) )
                    {
                        col = 0 ;
                        row++ ;
                    }
                    else
                    {
                        col--;
                        break;
                    }
                }
                if ( row >= VscrnGetBufferSize( vmode ) )
                    row = 0 ;
            }
            else
            {
                col-- ;
                if ( col < 0 )
                {
                    if ( row != VscrnGetBegin(vmode) )
                    {
                        row-- ;
                        col = vscrn[vmode].lines[row%vscrn[vmode].linecount].width - 1 ;
                    }
                    else
                    {
                        col++;
                        break;
                    }
                }
                if ( row < 0 )
                    row = VscrnGetBufferSize(vmode) -1 ;
            }
        }

        if ((!inpcas[cmdlvl] || !forward) && xq)
            free(xq); /* Free this if it was malloc'd. */
    }

    /* okay, did we find it?  if so, go there */
    if ( found )
    {
        if ( row >= VscrnGetTop(vmode) && row <= VscrnGetEnd(vmode) ||
             row <= VscrnGetEnd(vmode)
             && (row+VscrnGetBufferSize(vmode)) > VscrnGetTop(vmode)
             && VscrnGetEnd(vmode) < VscrnGetTop(vmode)
             )
        {
            if ( tt_roll[vmode] )
                VscrnSetScrollTop(vmode, VscrnGetTop(vmode) ) ;
            VscrnSetCurPos( vmode, col,
                            (row - VscrnGetTop(vmode) +
                              VscrnGetHeight(vmode)-(tt_status[vmode]?1:0))
                            %(VscrnGetHeight(vmode)-(tt_status[vmode]?1:0)) ) ;
        }
        else
        {
            if(tt_roll[vmode])
                VscrnSetScrollTop( vmode, row ) ;
            else
                VscrnSetTop( vmode, row ) ;
            VscrnSetCurPos( vmode, col, 0 ) ;
        }
    }

  search_exit:                  /* Common exit point */
    if ( !found )
    {
        VscrnUnmarkAll(vmode);
        row = col = -1 ;             /* start next search from current cursor position */
    }

    return(found);
}

void
dokcompose( int mode, int ucs2 )
{
    int i, round=0;
    USHORT x[4]={SP,SP,SP,SP}, c=0;
    con_event evt;

    if (txrinfo[GL->designation]->family != AL_ROMAN ||
         txrinfo[GR->designation]->family != AL_ROMAN ) {
        bleep(BP_WARN);         /* Not Latin character-set */
        goto compose_exit;
    }
    save_status_line();
    escapestatus[mode] = TRUE ;
    strcpy(exittext,"Cancel: Space"); /* Make special one */
    if (vik.help > 255 && keyname(vik.help))
        sprintf(helptext, "Help: %s", keyname(vik.help));

    for (i=0; i < (ucs2 ? 4 : 2); i++)  {
        /* mini-echo-buffer */
        if ( ucs2 )
            sprintf(usertext," UNICODE: [%c%c%c%c]",x[0],x[1],x[2],x[3]);
        else
            sprintf(usertext," COMPOSE: [%c%c]",x[0],x[1]);
        VscrnIsDirty(mode);        /* Status line needs update */

        do {
            evt = congev(mode,-1) ;
            switch (evt.type) {
            case key:
                x[i] = evt.key.scancode ;
                break;
            case kverb:
                x[i] = evt.kverb.id & ~F_KVERB;
                break;
            case macro:
                if ( evt.macro.string )
                    free( evt.macro.string ) ;
                x[i] = SP;
            break;
            case literal:
                if ( evt.literal.string )
                    free( evt.literal.string ) ;
                x[i] = SP;
            break;
            case csi:
            case esc:
            default:
                x[i] = SP;
            }
            if ( x[i] == K_HELP )
                popuphelp(mode,ucs2 ? hlp_ucs2 : hlp_compose);
        } while ( x[i] == K_HELP );
        if ((x[i] <= SP) || (x[i] > 0x7E) || ucs2 && !isxdigit(x[i]) ) {
            if (x[i] != SP)
                bleep(BP_WARN);
            goto compose_exit;
        }
    }

    if ( ucs2 ) {
        char hexstr[5];

        sprintf(usertext," UNICODE: [%c%c%c%c]",x[0],x[1],x[2],x[3]);
        VscrnIsDirty(mode);        /* Status line needs update */

        for ( i=0;i<4;i++ )
            hexstr[i] = x[i];
        hexstr[5] = NUL;

        c = hextoulong(hexstr,4);
    } else {
        sprintf(usertext," COMPOSE: [%c%c]",x[0],x[1]);
        VscrnIsDirty(mode);        /* Status line needs update */

        if (tcsl == TX_CP852) {     /* East European */
            for (i = 0; i < nl2ktab; i++) /* Look up the 2-char sequence */
                if ((x[0] == l2ktab[i].c1) && (x[1] == l2ktab[i].c2))
                    break;
            if (i >= nl2ktab) {     /* Not found */
                bleep(BP_WARN);
                goto compose_exit;
            } else c = l2ktab[i].c3; /* Latin-2 Character we found */

            /* Convert to Unicode */
            c = (*xl_u[TX_8859_2])(c);
        } else {
            for (i = 0; i < nl1ktab; i++) /* Look up the 2-char sequence */
                if ((x[0] == l1ktab[i].c1) && (x[1] == l1ktab[i].c2))
                    break;
            if (i >= nl1ktab) {         /* Not found */
                bleep(BP_WARN);
                goto compose_exit;
            } else
                c = l1ktab[i].c3;    /* Latin-1 Character we found */

            /* Convert to Unicode */
            c = (*xl_u[TX_8859_1])(c);

            /* Convert c to local character set */
            c = (*xl_tx[tcsl])(c);
        }
    }

    /* We know have a UCS2 value in 'c'.  Output it */
    if ( mode == VTERM ) {
        /* Convert c to remote character set */
        if ( tt_utf8 ) {
            CHAR * bytes = NULL;
            int count,i;

            count = ucs2_to_utf8( c, &bytes );
            for ( i=0; i<count ; i++ )
                sendcharduplex(bytes[i],TRUE);   /* Send it */
        } else {
            c = (*xl_tx[tcsr])(c);
            sendcharduplex(c,TRUE);             /* Send it */
        }
    } else {
        /* Convert c to local character set */
        c = (*xl_tx[tcsl])(c);
        putkey(mode,c);
    }
    msleep(333);                /* Some time to look at minibuffer */

  compose_exit:                 /* Common exit point */
    restore_status_line();      /* Restore status line */
    escapestatus[mode] = FALSE ;
}

/* Set the Keyboard Mode (English, Russian, Hebrew, ...) */

void
set_kb_mode( int kb_mode ) {
    static int tcsl_sav = -1;
    int i,x;

    if ( tt_kb_mode == kb_mode )
        return;

    tt_kb_mode = kb_mode;

#ifdef COMMENT
    /* I added this code because I didn't think that Russian and Hebrew  */
    /* keyboard modes could work without it.  However, it turns out that */
    /* sendcharduplex() has a special check to convert from CP866 to the */
    /* remote character set if Russian keyboard mode is in use or from   */
    /* CP862 if Hebrew is in use.  Therefore, this code is not necessary */

    switch ( kb_mode ) {
    case KBM_RU:
        if ( tcsl_sav == -1 )
            tcsl_sav = tcsl;
        tcsl = TX_CP866;
        break;
    case KBM_HE:
        if ( tcsl_sav == -1 )
            tcsl_sav = tcsl;
        tcsl = TX_CP862;
        break;
    default:
        if ( tcsl_sav != -1 ) {
            tcsl = tcsl_sav;
            tcsl_sav = -1;
        }
    }

    for (i = 0; i < 4; i++) {
        x = G[i].designation;
        G[i].c1 = (x != tcsl) && cs_is_std(x);
        x = G[i].def_designation;
        G[i].def_c1 = (x != tcsl) && cs_is_std(x);

        /* initialize DEC Graphic Set Translation functions if necessary */
        G[i].rtoi = xl_u[G[i].designation];
        G[i].itol = xl_tx[tcsl] ;
        G[i].ltoi = xl_u[tcsl] ;
        G[i].itor = xl_tx[G[i].designation];
        G[i].init = FALSE ;
    }
#endif /* COMMENT */
}


/*  D O K V E R B  --  Execute a keyboard verb  */

void
dokverb(int mode, int k) {                        /* 'k' is the kverbs[] table index. */
    extern int activecmd ;
    int x;
    con_event evt ;
    char escbuf[10];                    /* For building key escape sequences */
/*
  Items are grouped according to function, and checked approximately
  in order of how frequently they are used.
*/
    k &= ~(F_KVERB);                    /* Clear KVERB flag */

    if (k >= K_ACT_MIN && k <= K_ACT_MAX) {

        switch (k) {                    /* Common Kermit actions first ... */
        case K_EXIT:  /* \Kexit: */
            if ( mode == VTERM ) {
                if ( markmodeflag[mode] != notmarking ) {
                    markmode(mode,k);
                }
                else if ( tt_escape )
                    SetConnectMode(FALSE,CSX_ESCAPE);/*   Exit from terminal emulator */
            }
            else if ((mode == VCMD) && !(what & W_XFER)) {
                if (
#ifndef NOSPL
                 cmdlvl == 0
#else
                 tlevel == -1
#endif /* NOSPL */
                   ) {
                    x = conect(1);
#ifdef KUI
                    KuiSetTerminalConnectButton(x);
#endif /* KUI */
                } else
                    bleep(BP_WARN);
            }
            return;

        case K_QUIT:  /* \Kquit: */
            if ( markmodeflag[mode] != notmarking ) {
                markmode(mode,k);
                /* return ; */
            }
            if ( mode == VTERM ) {
                quitnow = 1;
                strcpy(termessage, "Hangup and quit.\n");
                SetConnectMode(0,CSX_USERDISC);
            }
            else {
                ttclos(0);
                doexit(GOOD_EXIT,0);
            }
            return;

        case K_BREAK:                 /* \Kbreak */
            if (mode == VTERM && !kbdlocked()) {
                int iosav = term_io;
                term_io = FALSE;
                msleep(750);               /* Allow term thread to notice */
                ttsndb();
                term_io = iosav;
            }
            return;
        case K_DOS:                   /* \Kdos or \Kos2 */
            os2push(); 
            return;                   /*   Push to system */
        case K_RESET:                 /* \Kreset */
            if ( mode == VTERM )
              doreset(1); 
            return;         /*   Reset terminal emulator */
        case K_HELP: {        /* \Khelp */
              enum helpscreen x = hlp_normal ;
              if ( markmodeflag[mode] != notmarking ) {
                  markmode(mode,k);
                  return ;
              }
              if ( scrollstatus[mode] ) {
                  scrollback(mode,k);
                  return ;
              }
              while ( popuphelp(mode,x) == ( F_KVERB | K_HELP ) )
#ifdef OS2MOUSE
                  switch ( x ) {
                  case hlp_normal:
                      x = hlp_rollback;
                      break;
                  case hlp_rollback:
                      x = hlp_mouse;
                      break;
                  case hlp_mouse:
                      x = hlp_normal;
                      break;
                  }
#else /* OS2MOUSE */
            switch ( x ) {
            case hlp_normal:
                x = hlp_rollback;
                break;
            case hlp_rollback:
                x = hlp_normal;
                break;
            }
#endif /* OS2MOUSE */
              return;   /*   Pop-up help message */
          }
          case K_PRTCTRL:               /* \Kprtscn,\Kprtctrl */
            if ( mode == VTERM )
              if ( xprint ) {               /*   Toggle Printer Ctrl mode on/off */
                  xprint = FALSE;         /*   It's on, turn it off */
                  if ( !cprint && !aprint )
                    printeroff();
                  else
                    ipadl25();
              } else {                    /*   vice versa ... */
                  xprint = TRUE ;
                  printeron();
              }
            return;
        case K_PRTAUTO:               /* \Kprtauto */
            if ( mode == VTERM )
              if ( aprint ) {         /*   Toggle printer auto mode on/off */
                  setaprint(FALSE);   /*   It's on, turn it off */
                  if ( !cprint && !xprint )
                    printeroff();
                  else
                    ipadl25();
              } else {                    /*   vice versa ... */
                  setaprint(TRUE);
                  printeron();
              }
            return;
        case K_PRTCOPY:               /* \Kprtcopy */
            if ( mode == VTERM )
              if ( cprint ) {     /*   Toggle printer copy mode on/off */
                  cprint = FALSE;         /*   It's on, turn it off */
                  if ( !aprint && !xprint )
                    printeroff();
                  else
                    ipadl25();
              } else {                    /*   vice versa ... */
                  cprint = TRUE ;
                  printeron();
              }
            return;
        case K_DUMP:                  /* \Kdump   */
            if ( markmodeflag[mode] != notmarking ) {
                markmode(mode,k);
                return ;
            }
            if ( scrollflag[mode] ) {
                scrollback(mode,k);
                return ;
            }
            x = xprintff; xprintff = 0; /*   Print/Dump current screen */
            prtscreen(mode,1,VscrnGetHeight(mode)-(tt_status[VTERM]?1:0));
            xprintff = x; 
            return;
        case K_PRINTFF:               /* \KprintFF - Print Form Feed */
            prtchar(FF); /* Send formfeed */
            return;
        case K_HANGUP:                /* \Khangup */
            {
                debug( F110,"dokverb","K_HANGUP",0);
                DialerSend( OPT_KERMIT_HANGUP, 0 ) ;
#ifndef NODIAL
                if (mdmhup() < 1)
#endif /* NODIAL */
                    tthang();
            }
            return;
        case K_NULL:                  /* \Knull */
            if ( mode == VTERM ||
                 mode == VCMD && activecmd == XXOUT )
              if ( !kbdlocked() )
                sendcharduplex('\0',TRUE); 
            return; /*   Send a NUL */
          case K_LBREAK:                /* \Klbreak: */
            if ( mode == VTERM ||
                 mode == VCMD && activecmd == XXOUT )
              if ( !kbdlocked() )
                ttsndlb(); 
            return;          /*   Send a Long BREAK */
          case K_DEBUG:                 /* \Kdebug */
            flipdebug(); 
            return;        /*   Toggle debugging */
          case K_FLIPSCN:               /* \KflipScn */
            flipscreen(mode);
            VscrnIsDirty(mode);
            return;                     /* Toggle video */
        case K_HOLDSCRN:              /* \KholdScrn */
            if ( holdscreen ) {
                holdscreen = FALSE ;
                restore_status_line();      /* Restore status line */
                VscrnIsDirty(mode) ;
            }
            else {
                save_status_line();         /* Save current status line */
                sprintf(usertext,
                         "SCROLL-LOCK      Press %s to unlock",
                         keyname(vik.holdscrn) );
                helptext[0] = exittext[0] = hostname[0] = NUL;
                VscrnIsDirty(mode);        /* Status line needs to be updated */
                holdscreen = TRUE ;
            }
            return;
        case K_ANSWERBACK:
            if ( mode == VTERM ||
                 mode == VCMD && activecmd == XXOUT )
                sendcharsduplex(answerback,strlen(answerback), FALSE) ;
            return ;
        case K_IGNORE:                /* \Kignore */
            return;
#ifdef NETCONN
#ifdef TCPSOCKET
        case K_TN_SAK:
            if ( mode == VTERM ||
                 mode == VCMD && activecmd == XXOUT )
              if ( !kbdlocked() )
                do_tn_cmd((CHAR) TN_SAK);
            return;
        case K_TN_GA:                /* TELNET Go Ahead */
            if ( mode == VTERM ||
                 mode == VCMD && activecmd == XXOUT )
              if ( !kbdlocked() )
                do_tn_cmd((CHAR) TN_GA);
            return;
        case K_TN_AO:                /* TELNET Abort Output */
            if ( mode == VTERM ||
                 mode == VCMD && activecmd == XXOUT )
              if ( !kbdlocked() )
                do_tn_cmd((CHAR) TN_AO);
            return;
        case K_TN_EL:                /* TELNET Erase Line */
            if ( mode == VTERM ||
                 mode == VCMD && activecmd == XXOUT )
              if ( !kbdlocked() )
                do_tn_cmd((CHAR) TN_EL);
            return;
        case K_TN_EC:                /* TELNET Erase Character */
            if ( mode == VTERM ||
                 mode == VCMD && activecmd == XXOUT )
              if ( !kbdlocked() )
                do_tn_cmd((CHAR) TN_EC);
            return;
        case K_TN_AYT:                /* TELNET Are You There */
            if ( mode == VTERM ||
                 mode == VCMD && activecmd == XXOUT )
              if ( !kbdlocked() )
                do_tn_cmd((CHAR) TN_AYT);
            return;
        case K_TN_IP:                 /* TELNET Interrupt Process */
            if ( mode == VTERM ||
                 mode == VCMD && activecmd == XXOUT )
              if ( !kbdlocked() )
                do_tn_cmd((CHAR) TN_IP);
            return;
        case K_TN_LOGOUT:             /* TELNET LOGOUT */
            if ( mode == VTERM ||
                 mode == VCMD && activecmd == XXOUT )
                if ( !kbdlocked() ) {
                    tn_sopt(DO,TELOPT_LOGOUT);
                    TELOPT_UNANSWERED_DO(TELOPT_LOGOUT) = 1;
                }
            return;
        case K_TN_NAWS:               /* TELNET NAWS - Send Window Size */
#ifdef  CK_NAWS
            if ( mode == VTERM ||
                 mode == VCMD && activecmd == XXOUT )
                if ( !kbdlocked() ) {
                    TELOPT_SB(TELOPT_NAWS).naws.x = 0;
                    TELOPT_SB(TELOPT_NAWS).naws.y = 0;
                    tn_snaws();
#ifdef RLOGCODE
                    rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                    ssh_snaws();
#endif /* SSHBUILTIN */
                }
#endif /* CK_NAWS */
            return;
#endif /* TCPSOCKET */
#endif /* NETCONN */
        case K_PASTE:                 /* Paste clipboard to Host */
            if ( !kbdlocked() )
              CopyClipboardToKbdBuffer(mode) ;
            return;
        case K_CLRSCRN:
            VscrnScroll( mode, UPWARD, 0,
                        VscrnGetHeight(mode)-(tt_status[mode]?2:1),
                        VscrnGetHeight(mode)-(tt_status[mode]?1:0),
                        TRUE,
                        SP );
            cleartermscreen(mode) ;    /* Clear the terminal screen */
            VscrnIsDirty(mode);
            return;
        case K_CLRSCROLL:
            clearscrollback(mode);
            VscrnIsDirty(mode);
            return;
        case K_SESSION:
            if ( !seslog ) {
                if ( *sesfil )
                  setseslog(1);
                else {
#ifdef KUI
                    char logfile[MAX_PATH+1];
                    if ( !KuiDownloadDialog("Create Session Log ...","session.log",
                                            logfile,MAX_PATH+1) )
                        return;
                    setseslog(sesopn( logfile, 0));
#else /* KUI */
                    setseslog(sesopn( "session.log", 0 )); /* create new */
#endif /* KUI */
                }
                if ( !seslog )
                  bleep( BP_FAIL ) ;
            } else {
                setseslog(0);    /* session file.  Therefore, we can continue */
            }
            break;
        case K_LOGON:
            if ( !seslog ) {
                if ( *sesfil )
                  setseslog(1);
                else
                  setseslog(sesopn( "session.log", 0 )); /* create new */
                if ( !seslog )
                  bleep( BP_FAIL ) ;
            } else {
                bleep( BP_WARN ) ;
            }
            return ;
        case K_LOGOFF:
            if ( seslog ) {     /* Unlike CLOSE SESSION we do not close the  */
                setseslog(0);    /* session file.  Therefore, we can continue */
            } else {            /* to append to the file later on.           */
                bleep( BP_WARN ) ;
            }
            return ;

#ifdef PCTERM
        case K_PCTERM:
            setpcterm(!tt_pcterm);
            break;
#endif /* PCTERM */

        case K_AUTODOWN:
#ifndef NOXFER
            if ( !autodl )
                setautodl(1,0);
            else if ( autodl && !adl_ask )
                setautodl(1,1);
            else 
                setautodl(0,0);
#else /* NOXFER */
            bleep( BP_WARN ) ;
#endif /* NOXFER */
            break;

        case K_URLHILT:
            seturlhl(!tt_url_hilite);
            break;

        case K_BYTESIZE:
            if ( cmask == 0177 )
                setcmask(8);
            else
                setcmask(7);
            if (IS97801(tt_type_mode))
                SNI_bitmode((cmask == 0377) ? 8 : 7);
            break;

        case K_SET_BOOK:              /* Bookmark Kverbs */
            bookmarkset(mode);
            return ;

        case K_TERMTYPE:                /* Toggle Terminal Type */
            if ( mode == VTERM )
                  settermtype((tt_type+1)%(TT_MAX+1), 0);
            return ;

        case K_STATUS:          /* Toggle Status-Line Type */
            if ( mode == VTERM )
                setdecssdt((decssdt+1)%3);
            return ;


        case K_KEYCLICK:                /* Toggle Keyclick */
            setkeyclick(!keyclick);
            return ;

        case K_LOGDEBUG:                /* Toggle Debug Log File */
#ifdef DEBUG
            {
                extern int deblog ;
                extern char debfil[] ;
                char * tp ;

                if ( deblog ) {
#ifdef COMMENT
                    doclslog(LOGD);
#else
                    ztime(&tp);
                    debug(F110,"Debug log suspended at",tp,0);
                    deblog = FALSE ;
#endif
                }
                else {
                    if ( debfil && *debfil ) {
                        deblog = TRUE ;
                        ztime(&tp);
                        debug(F110,"Debug log resumed at",tp,0);
                    }
                    else
                        deblog = debopn("debug.log",0);
                }
            }
#endif /* DEBUG */
            return ;

        case K_FNKEYS:                  /* Display Function Key Labels */
            if ( mode == VTERM ) {
                fkeypopup(mode);
            }
            return;

        case K_CURSOR_URL:
            mouseurl(mode,vscrn[mode].cursor.y,vscrn[mode].cursor.x);
            break;

        default:                        /* None of the above */
            return;                     /* Ignore this key and return. */
        }
    }

    /* Capture all kverbs that must go to mark mode */

    if (k >= K_MARK_MIN && k <= K_MARK_MAX ||
        ( markmodeflag[mode] != notmarking &&
            (k >= K_ROLLMIN && k <= K_ROLLMAX ||
            k >= K_ARR_MIN && k <= K_ARR_MAX ))) {
        /* mark mode goes here */
        markmode(mode,k);
        return ;
    }
    if (k >= K_ROLLMIN && k <= K_ROLLMAX ) { /* Screen rollback... */
        scrollback(mode,k);
        return;
    }

    if ( k >= K_LFONE && k <= K_RTALL ) {       /* Horizontal Scrolling */
        switch ( k ) {
        case K_LFONE:
            if ( vscrn[mode].hscroll == 0 )
                bleep(BP_WARN);
            else if ( vscrn[mode].hscroll > 0 )
                vscrn[mode].hscroll -= 1 ;
            break;
        case K_LFPAGE:
            if ( vscrn[mode].hscroll == 0 )
                bleep(BP_WARN);
            else if ( vscrn[mode].hscroll > 8 )
                vscrn[mode].hscroll -= 8 ;
            else
                vscrn[mode].hscroll = 0 ;
            break;
        case K_LFALL:
            if ( vscrn[mode].hscroll == 0 )
                bleep(BP_WARN);
            else if ( vscrn[mode].hscroll > 0 )
                vscrn[mode].hscroll = 0 ;
            break;
        case K_RTONE:
            if ( vscrn[mode].hscroll == MAXTERMCOL-pwidth )
                bleep(BP_WARN);
            else if ( vscrn[mode].hscroll < MAXTERMCOL-pwidth )
                vscrn[mode].hscroll += 1 ;
            break;
        case K_RTPAGE:
            if ( vscrn[mode].hscroll == MAXTERMCOL-pwidth )
                bleep(BP_WARN);
            else if ( vscrn[mode].hscroll < MAXTERMCOL-pwidth-8 )
                vscrn[mode].hscroll += 8 ;
            else
                vscrn[mode].hscroll = MAXTERMCOL-pwidth ;
            break;
        case K_RTALL:
            if ( vscrn[mode].hscroll == MAXTERMCOL-pwidth )
                bleep(BP_WARN);
            else if ( vscrn[mode].hscroll < MAXTERMCOL-pwidth )
                vscrn[mode].hscroll = MAXTERMCOL-pwidth ;
            break;
        }
        VscrnIsDirty(mode);
        return;
    }

#ifdef OS2MOUSE
    if (k >= K_MOUSE_MIN && k <= K_MOUSE_MAX) {
        /* mouse verbs cannot be assigned to keys */
        bleep(BP_FAIL);
        return ;
    }
#endif /* OS2MOUSE */

    if ( !kbdlocked() ) {
        if ( mode == VTERM ||
             mode == VCMD && activecmd == XXOUT ) {
            if (k >= K_ARR_MIN && k <= K_ARR_MAX) {
                if ( ISDG200( tt_type_mode ) ) {
                    /* Data General */
                    extern int dgunix;

                    switch ( k ) {
                    case K_UPARR:
                        if ( dgunix ) {
                            if ( udkfkeys[K_DGUPARR-K_DGF01+25] )
                                sendcharsduplex(udkfkeys[K_DGUPARR - K_DGF01+25],
                                                 strlen(udkfkeys[K_DGUPARR - K_DGF01+25]),
                                                 TRUE);
                            else
                                sendcharsduplex("\036PA",3,TRUE);
                        }
                        else {
                            if ( udkfkeys[K_DGUPARR-K_DGF01] )
                                sendcharsduplex(udkfkeys[K_DGUPARR - K_DGF01],
                                                 strlen(udkfkeys[K_DGUPARR - K_DGF01]),
                                                 TRUE);
                            else
                            sendcharduplex(ETB,TRUE);
                        }
                        break;
                    case K_RTARR:
                        if ( dgunix ) {
                            if ( udkfkeys[K_DGRTARR-K_DGF01+25] )
                                sendcharsduplex(udkfkeys[K_DGRTARR - K_DGF01+25],
                                                 strlen(udkfkeys[K_DGRTARR - K_DGF01+25]),
                                                 TRUE);
                            else
                            sendcharsduplex("\036PC",3,TRUE);
                        }
                        else {
                            if ( udkfkeys[K_DGRTARR-K_DGF01] )
                                sendcharsduplex(udkfkeys[K_DGRTARR - K_DGF01],
                                                 strlen(udkfkeys[K_DGRTARR - K_DGF01]),
                                                 TRUE);
                            else
                            sendcharduplex(CAN,TRUE);
                        }
                        break;
                    case K_LFARR:
                        if ( dgunix ) {
                            if ( udkfkeys[K_DGLFARR-K_DGF01+25] )
                                sendcharsduplex(udkfkeys[K_DGLFARR - K_DGF01+25],
                                                 strlen(udkfkeys[K_DGLFARR - K_DGF01+25]),
                                                 TRUE);
                            else
                            sendcharsduplex("\036PD",3,TRUE);
                        }
                        else {
                            if ( udkfkeys[K_DGLFARR-K_DGF01] )
                                sendcharsduplex(udkfkeys[K_DGLFARR - K_DGF01],
                                                 strlen(udkfkeys[K_DGLFARR - K_DGF01]),
                                                 TRUE);
                            else
                            sendcharduplex(XEM,TRUE);
                        }
                        break;
                    case K_DNARR:
                        if ( dgunix ) {
                            if ( udkfkeys[K_DGDNARR-K_DGF01+25] )
                                sendcharsduplex(udkfkeys[K_DGDNARR - K_DGF01+25],
                                                 strlen(udkfkeys[K_DGDNARR - K_DGF01+25]),
                                                 TRUE);
                            else
                            sendcharsduplex("\036PB",3,TRUE);
                        }
                        else {
                            if ( udkfkeys[K_DGDNARR-K_DGF01] )
                                sendcharsduplex(udkfkeys[K_DGDNARR - K_DGF01],
                                                 strlen(udkfkeys[K_DGDNARR - K_DGF01]),
                                                 TRUE);
                            else
                            sendcharduplex(SUB,TRUE);
                        }
                        break;
                    }
                }
                else if ( ISQNX( tt_type_mode ) ) {
                    /* QNX */
                    char buf[3] ;
                    switch ( k ) {
                    case K_UPARR:
                        sprintf(buf,"\377\241");
                        break;
                    case K_RTARR:
                        sprintf(buf,"\377\246");
                        break;
                    case K_LFARR:
                        sprintf(buf,"\377\244");
                        break;
                    case K_DNARR:
                        sprintf(buf,"\377\251");
                        break;
                    }
                    sendcharsduplex(buf,2,TRUE);
                }
                else if ( ISHP( tt_type_mode ) ||
                          ISIBM31(tt_type_mode)) {
                    /* HP2621A || IBM 31xx */
                    char buf[3] ;
                    switch ( k ) {
                    case K_UPARR:
                        sprintf(buf,"%cA",ESC);
                        break;
                    case K_RTARR:
                        sprintf(buf,"%cC",ESC);
                        break;
                    case K_LFARR:
                        sprintf(buf,"%cD",ESC);
                        break;
                    case K_DNARR:
                        sprintf(buf,"%cB",ESC);
                        break;
                    }
                    sendcharsduplex(buf,2,TRUE);
                }
                else if ( ISHZL( tt_type_mode ) ) {
                    /* Hazeltine */
                    switch ( k ) {
                    case K_UPARR:
                        sendcharsduplex("~\x0C",2,TRUE);
                        break;
                    case K_RTARR:
                        sendcharduplex(DLE,TRUE);
                        break;
                    case K_LFARR:
                        sendcharduplex(BS,TRUE);
                        break;
                    case K_DNARR:
                        sendcharsduplex("~\x0B",2,TRUE);
                        break;
                    }
                }
                else if ( ISVC( tt_type_mode ) ) {
                    /* Victor Craig */
                    switch ( k ) {
                    case K_UPARR:
                        sendcharduplex(SUB,TRUE);
                        break;
                    case K_RTARR:
                        sendcharduplex(NAK,TRUE);
                        break;
                    case K_LFARR:
                        sendcharduplex(BS,TRUE);
                        break;
                    case K_DNARR:
                        sendcharduplex(LF,TRUE);
                        break;
                    }
                }
                else if ( ISWYSE( tt_type_mode ) ) {
                    switch ( k ) {
                    case K_UPARR:
                        if ( udkfkeys[K_WYUPARR-K_WYF01] )
                            sendcharsduplex(udkfkeys[K_WYUPARR - K_WYF01],
                                       strlen(udkfkeys[K_WYUPARR - K_WYF01]),
                                             TRUE);
                        else
                            sendcharduplex(VT,TRUE);
                        break;
                    case K_RTARR:
                        if ( udkfkeys[K_WYRTARR-K_WYF01] )
                            sendcharsduplex(udkfkeys[K_WYRTARR - K_WYF01],
                                       strlen(udkfkeys[K_WYRTARR - K_WYF01]),
                                             TRUE);
                        else
                            sendcharduplex(FF,TRUE);
                        break;
                    case K_LFARR:
                        if ( udkfkeys[K_WYLFARR-K_WYF01] )
                            sendcharsduplex(udkfkeys[K_WYLFARR - K_WYF01],
                                       strlen(udkfkeys[K_WYLFARR - K_WYF01]),
                                             TRUE);
                        else
                            sendcharduplex(BS,TRUE);
                        break;
                    case K_DNARR:
                        if ( udkfkeys[K_WYDNARR-K_WYF01] )
                            sendcharsduplex(udkfkeys[K_WYDNARR - K_WYF01],
                                       strlen(udkfkeys[K_WYDNARR - K_WYF01]),
                                             TRUE);
                        else
                            sendcharduplex(LF,TRUE);
                        break;
                    }
                }
                else if ( ISTVI( tt_type_mode ) ) {
                    switch ( k ) {
                    case K_UPARR:
                        if ( udkfkeys[K_TVIUPARR-K_TVIF01] )
                            sendcharsduplex(udkfkeys[K_TVIUPARR - K_TVIF01],
                                       strlen(udkfkeys[K_TVIUPARR - K_TVIF01]),
                                             TRUE);
                        else
                            sendcharduplex(DLE,TRUE);
                        break;
                    case K_RTARR:
                        if ( udkfkeys[K_TVIRTARR-K_TVIF01] )
                            sendcharsduplex(udkfkeys[K_TVIRTARR - K_TVIF01],
                                       strlen(udkfkeys[K_TVIRTARR - K_TVIF01]),
                                             TRUE);
                        else
                            sendcharduplex(ACK,TRUE);
                        break;
                    case K_LFARR:
                        if ( udkfkeys[K_TVILFARR-K_TVIF01] )
                            sendcharsduplex(udkfkeys[K_TVILFARR - K_TVIF01],
                                       strlen(udkfkeys[K_TVILFARR - K_TVIF01]),
                                             TRUE);
                        else
                            sendcharduplex(STX,TRUE);
                        break;
                    case K_DNARR:
                        if ( udkfkeys[K_TVIDNARR-K_TVIF01] )
                            sendcharsduplex(udkfkeys[K_TVIDNARR - K_TVIF01],
                                       strlen(udkfkeys[K_TVIDNARR - K_TVIF01]),
                                             TRUE);
                        else
                            sendcharduplex(SO,TRUE);
                        break;
                    }
                }
                else if ( IS97801(tt_type_mode) ) {
                    switch ( k ) {
                    case K_UPARR:
                        if ( udkfkeys[K_SNI_CURSOR_UP-K_SNI_F01] )
                            sendcharsduplex(udkfkeys[K_SNI_CURSOR_UP - K_SNI_F01],
                                       strlen(udkfkeys[K_SNI_CURSOR_UP - K_SNI_F01]),
                                             TRUE);
                        else
                            sendcharsduplex("\033[A",3,TRUE);
                        break;
                    case K_RTARR:
                        if ( udkfkeys[K_SNI_CURSOR_RIGHT-K_SNI_F01] )
                            sendcharsduplex(udkfkeys[K_SNI_CURSOR_RIGHT - K_SNI_F01],
                                       strlen(udkfkeys[K_SNI_CURSOR_RIGHT - K_SNI_F01]),
                                             TRUE);
                        else
                            sendcharsduplex("\033[C",3,TRUE);
                        break;
                    case K_LFARR:
                        if ( udkfkeys[K_SNI_CURSOR_LEFT-K_SNI_F01] )
                            sendcharsduplex(udkfkeys[K_SNI_CURSOR_LEFT - K_SNI_F01],
                                       strlen(udkfkeys[K_SNI_CURSOR_LEFT - K_SNI_F01]),
                                             TRUE);
                        else
                            sendcharsduplex("\033[D",3,TRUE);
                        break;
                    case K_DNARR:
                        if ( udkfkeys[K_SNI_CURSOR_DOWN-K_SNI_F01] )
                            sendcharsduplex(udkfkeys[K_SNI_CURSOR_DOWN - K_SNI_F01],
                                       strlen(udkfkeys[K_SNI_CURSOR_DOWN - K_SNI_F01]),
                                             TRUE);
                        else
                            sendcharsduplex("\033[B",3,TRUE);
                        break;
                    }
                }
                else {
                    /* DEC arrow (cursor) keys */
                    if (ISVT100(tt_type_mode) ||
                         ISANSI(tt_type_mode) ||
                         ISQANSI(tt_type_mode) ||
                         ISBEOS(tt_type_mode) ||
                        ISHFT(tt_type_mode))
                        sprintf(escbuf,"%c%c",( (tt_arrow == TTK_APPL) ? 'O' : '[' ),
                                 'A' + (k - K_UPARR));
                    else
                        sprintf(escbuf,"%c", 'A' + (k - K_UPARR));
                    sendescseq(escbuf);
                }
                return;
            }

            if (k >= K_PF1 && k <= K_PF4) { /* DEC PF keys */
                char *p = escbuf;
                if (ISVT100(tt_type_mode)) /* SS3 P..S for VT100 & above */
                  *p++ = 'O';
                *p++ = 'P' + (k - K_PF1);   /* or just ESC P..S for VT52 */
                *p = NUL;
                sendescseq(escbuf);
                return;
            }

            if (k >= K_KP0 && k <= K_KP9) { /* DEC numeric keypad keys */
                if (tt_keypad == TTK_NORM)  /* Keypad in numeric mode */
                  sendcharduplex((CHAR) ('0' + (CHAR) (k - K_KP0)),TRUE);
                else {                      /* Application mode */
                    int dup_sav = duplex;
                    duplex = 0;
                    sprintf(escbuf,
                        "%c%c",
                        (ISVT100(tt_type_mode) ? 'O' : '?'),
                        ('p' + (k - K_KP0))
                        );
                    sendescseq(escbuf);
                    duplex = dup_sav;
                }
                return;
            }

            if (k >= K_KPCOMA && k <= K_KPDOT) { /* DEC keypad punctuation keys */
                if (tt_keypad == TTK_NORM)  /* Keypad in numeric mode */
                  sendcharduplex((CHAR) (',' + (CHAR) (k - K_KPCOMA)),TRUE); /*  */
                else {                      /* Keypad in application mode */
                    int dup_sav = duplex;
                    duplex = 0;
                    sprintf(escbuf,
                             "%c%c",
                             (ISVT100(tt_type_mode) ? 'O' : '?'),
                             ('l' + (k - K_KPCOMA))
                             );
                    sendescseq(escbuf);
                    duplex = dup_sav;
                }
                return;
            }

            if (k == K_KPENTER) {           /* DEC keypad Enter key */
                if (tt_keypad == TTK_NORM) { /* Keypad in numeric mode */
                    sendcharduplex('\015',TRUE);  /* Send CR */
                    if (tnlm)
                      sendcharduplex('\012',TRUE); /* Newline mode, send LF too */
                } else {                    /* Keypad in application mode */
                    int dup_sav = duplex;
                    duplex = 0;
                    sprintf(escbuf,
                             "%c%M",
                             (ISVT100(tt_type_mode) ? 'O' : '?')
                             );
                    sendescseq(escbuf);
                    duplex = dup_sav;
                }
                return;
            }

            if (k >= K_DECFIND && k <= K_DECHOME) { /* DEC editing keypad keys */
                int dup_sav = duplex;
                duplex = 0;
                if (ISVT220(tt_type_mode)) /* VT220 and above */
                  sendescseq(ekeys[k - K_DECFIND]);
                else bleep(BP_WARN);
                duplex = dup_sav;
                return;
            }

            if (k >= K_DECF1 && k <= K_DECF20) { /* DEC top-rank function keys */
                if (ISVT220(tt_type_mode)) { /* VT220 and above */
                    int dup_sav = duplex;
                    duplex =0;
                    sendescseq(fkeys[k - K_DECF1]);
                    duplex = dup_sav;
                    return;
                } else {                    /* VT102 or lower */
                    switch (k) {
                    case K_DECF11:
                        sendcharduplex(ESC,TRUE); return;
                    case K_DECF12:
                        sendcharduplex(BS,TRUE); return;
                    case K_DECF13:
                        sendcharduplex(LF,TRUE); return;
                    default:
                        bleep(BP_WARN);
                        return;
                    }
                }
            }

            if (k >= K_UDKF1 && k <= K_UDKF20) { /* User-Defined Keys */
                if (ISVT220(tt_type_mode)) { /* VT220 and above */
                    if ( udkfkeys[k-K_UDKF1] )
                      sendcharsduplex(udkfkeys[k - K_UDKF1],
                                 strlen(udkfkeys[k - K_UDKF1]),TRUE);

                }
                else {
                    bleep( BP_WARN );
                }
                return;
            }

            if ( k >= K_ANSIF01 && k <= K_ANSIF61 ) { /* ANSI FN Keys */
                if ( ISUNIXCON(tt_type_mode) ) {
                    if ( udkfkeys[k-K_ANSIF01] )
                      sendcharsduplex(udkfkeys[k-K_ANSIF01],
                                 strlen(udkfkeys[k-K_ANSIF01]),TRUE) ;
                }
                else {
                    bleep( BP_WARN ) ;
                }
                return ;
            }

            if ( k >= K_WYF01 && k <= K_WYSSEND ) {    /* WYSE FN Keys */
                if ( ISWYSE(tt_type_mode) ) {
                    if ( udkfkeys[k-K_WYF01] )
                        sendcharsduplex(udkfkeys[k-K_WYF01],
                                   strlen(udkfkeys[k-K_WYF01]),TRUE);
                }
                else
                    bleep( BP_WARN ) ;
                return ;
            }

            if ( k >= K_TVIF01 && k <= K_TVIPRTSCN ||
                        k == K_TVISEND || k == K_TVISSEND) {    /* TVI FN Keys */
                if ( ISTVI(tt_type_mode) ) {
                    if ( udkfkeys[k-K_TVIF01] )
                        sendcharsduplex(udkfkeys[k-K_TVIF01],
                                   strlen(udkfkeys[k-K_TVIF01]),TRUE);
                }
                else
                    bleep( BP_WARN ) ;
                return ;
            }

            if ( k >= K_HPF01 && k <= K_HPENTER ) {     /* HP FN Keys */
                if ( ISHP(tt_type_mode) ) {
                    if ( udkfkeys[k-K_HPF01] )
                        sendcharsduplex(udkfkeys[k-K_HPF01],
                                   strlen(udkfkeys[k-K_HPF01]),TRUE);
                }
                else
                    bleep( BP_WARN ) ;
                return ;
            }

            if ( k >= K_DGF01 && k <= K_DGSHOME ) {     /* DG FN Keys */
                if ( ISDG200(tt_type_mode) ) {
                    extern int dgunix;
                    if ( k <= K_DGCSF15 || ! dgunix ) {
                        if ( udkfkeys[k-K_DGF01] )
                        sendcharsduplex(udkfkeys[k-K_DGF01],
                                   strlen(udkfkeys[k-K_DGF01]),TRUE);
                    }
                    else {
                        if ( udkfkeys[k-K_DGF01+25] )
                        sendcharsduplex(udkfkeys[k-K_DGF01+25],
                                   strlen(udkfkeys[k-K_DGF01+25]),TRUE);
                    }
                }
                else
                    bleep( BP_WARN );
                return;
            }

            if ( k >= K_SNI_MIN && k <= K_SNI_MAX ) {   /* SNI FN Keys */
                if ( IS97801(tt_type_mode) ) {
                    if ( k == K_SNI_CH_CODE ) {
                        if ( sni_bitmode == 8 && !sni_chcode_8 ||
                             sni_bitmode == 7 && !sni_chcode_7 )
                            bleep(BP_WARN);
                        else {
                            SNI_chcode(!sni_chcode);
                            if ( sni_chcode )
                                printf("SNI Ch.Code is ON\n");
                            else
                                printf("SNI Ch.Code is OFF\n");
                        }
                    } else {
                        if ( udkfkeys[k-K_SNI_MIN] )
                            sendcharsduplex(udkfkeys[k-K_SNI_MIN],
                                             strlen(udkfkeys[k-K_SNI_MIN]),TRUE);
                    }
                }
                else
                    bleep( BP_WARN );
                return;
            }

            if ( k >= K_BA80_MIN && k <= K_BA80_MAX ) { /* SNI FN Keys */
                if ( ISBA80(tt_type_mode) ) {
                    if ( udkfkeys[k-K_BA80_MIN] )
                        sendcharsduplex(udkfkeys[k-K_BA80_MIN],
                                   strlen(udkfkeys[k-K_BA80_MIN]),TRUE);
                }
                else
                    bleep( BP_WARN );
                return;
            }

            if ( k >= K_I31_MIN && k <= K_I31_MAX ) {   /* IBM 31xx FN Keys */
                if ( ISIBM31(tt_type_mode) ) {
                    if ( udkfkeys[k-K_I31_MIN] )
                        sendcharsduplex(udkfkeys[k-K_I31_MIN],
                                   strlen(udkfkeys[k-K_I31_MIN]),TRUE);
                }
                else
                    bleep( BP_WARN );
                return;
            }

            if ( k >= K_SUN_MIN && k <= K_SUN_MAX ) {   /* SUN Keys */
                if ( ISSUN(tt_type_mode) ) {
                    if ( udkfkeys[k-K_SUN_MIN] )
                        sendcharsduplex(udkfkeys[k-K_SUN_MIN],
                                   strlen(udkfkeys[k-K_SUN_MIN]),TRUE);
                }
                else
                    bleep( BP_WARN );
                return;
            }

            if ( k == K_EMACS_OVER && tt_kb_mode == KBM_EM )
                sendescseq("Xoverwrite\r") ;
        }

        if (k == K_COMPOSE) {           /* Compose key */
            dokcompose(mode, 0) ;
            return;
        }

        if (k == K_C_UNI16) {           /* Compose key */
            dokcompose(mode, 1) ;
            return;
        }

        if ( mode == VTERM ) {
            if (k == K_KB_HEB) {            /* Hebrew keyboard support... */
                if ( tt_kb_mode == KBM_HE )
                    set_kb_mode(KBM_EN) ;
                else
                    set_kb_mode(KBM_HE) ;
                ipadl25();                      /* Update the status line */
                return;
            }

            if (k == K_KB_RUS) {            /* Russian keyboard support... */
                if ( tt_kb_mode == KBM_RU )
                    set_kb_mode(KBM_EN) ;
                else
                    set_kb_mode(KBM_RU) ;
                ipadl25();                      /* Update the status line */
                return;
            }

            if ( k == K_KB_EMA ) {      /* Emacs keyboard support */
                if ( tt_kb_mode == KBM_EM )
                    set_kb_mode(KBM_EN) ;
                else
                    set_kb_mode(KBM_EM) ;
                ipadl25();                      /* Update the status line */
                return ;
            }

            if ( k == K_KB_WP ) {       /* WP5.1 keyboard support */
                if ( tt_kb_mode == KBM_WP )
                    set_kb_mode(KBM_EN);
                else
                    set_kb_mode(KBM_WP);
                ipadl25();                      /* Update the status line */
                return ;
            }
            if (k == K_KB_ENG) {            /* English too... */
                if ( tt_kb_mode != KBM_EN ) {
                    set_kb_mode(KBM_EN);
                    ipadl25();                  /* Update the status line */
                }
                return;
            }
        }

    } /* !kbdlocked() */
}
#endif /* NOKVERBS */

/*  S E N D K E Y D E F  --  Send key definition  */

void
sendkeydef(unsigned char *s, int no_xlate ) {
#ifndef NOKVERBS
    int x, y, brace = 0;
    char * p, * b;
#define K_BUFLEN 30
    char kbuf[K_BUFLEN + 1];            /* Key verb name buffer */
#define SEND_BUFLEN 255
#define sendbufd(x) { sendbuf[sendndx++] = x ; if (sendndx == SEND_BUFLEN) { sendcharsduplex(s,SEND_BUFLEN,no_xlate); sendndx = 0 ;}}
    char sendbuf[SEND_BUFLEN +1] ;
    int  sendndx = 0 ;
#endif /* NOKVERBS */

#ifdef NOKVERBS
    sendcharsduplex(s, strlen(s), no_xlate); /* Send the entire string literally. */
#else   /* \Kverb support ... */
    for ( ; *s; s++ ) {                 /* Go through the string */

        if (*s != CMDQ) {               /* Normal character */
            sendbufd(*s) ;
            continue;                   /* Get next, etc. */
        }
        b = s++;                        /* Get next character */
        if (!*s) {
            sendbufd(CMDQ) ;            /* Send the CMDQ char */
            goto exit_sendkeydef ;      /* and then exit */
        }
        if (*s == '{') {                /* Opening brace? */
            brace++;                    /* Remember */
            s++;                        /* Go to next character */
            if (!*s) {
                goto exit_sendkeydef ;
            }
        }
        if (*s == 'K' || *s == 'k') {   /* Have K verb? */
            s++;
            if (*s == '{') {                /* Opening brace? */
                brace++;                  /* Remember */
                s++;                        /* Go to next character */
                if (!*s) {
                    goto exit_sendkeydef ;
                }
            } else if (!*s) {
                goto exit_sendkeydef ;
            }

/*
  We assume that the verb name is {braced}, or it extends to the end of the
  string, s, or it ends with a space, control character, or backslash.
*/
            p = kbuf;                   /* Copy verb name into local buffer */
            x = 0;
            while ((x++ < K_BUFLEN) && (*s > SP) && (*s != CMDQ)) {
                if (brace && *s == '}') {
                    if (brace == 1)
                        break;
                    brace--;
                    s++;
                    continue;
                }
                *p++ = *s++;
            }
            if (*s && !brace)           /* If we broke because of \, etc, */
                s--;                    /*  back up so we get another look. */
            brace = 0;
            *p = NUL;                   /* Terminate. */
            p = kbuf;                   /* Point back to beginning */
            debug(F110,"sendkeydef kverb",p,0);
            y = xlookup(kverbs,p,nkverbs,&x); /* Look it up */
            debug(F101,"sendkeydef lookup",0,y);
            if (y > -1) {
                if ( sendndx ) {
                    sendcharsduplex( sendbuf, sendndx, no_xlate ) ;
                    sendndx = 0 ;
                }
                dokverb(VTERM,y) ;
#ifndef NOSPL
            } else {                    /* Is it a macro? */
                y = mxlook(mactab,p,nmac);
                if (y > -1) {
                    keymac = 1;         /* Flag for key macro active */
                    keymacx = y;        /* Key macro index */
                    keydefptr = s;      /* Where to resume next time */
                    debug(F111,"sendkeydef mxlook",keydefptr,y);
                    SetConnectMode(0,CSX_MACRO); /* Leave CONNECT mode */
                }
#endif /* NOSPL */
            }
        } else if (*s == 'V' || *s == 'v') { /* Have \v(xxx)? */
            s++;                        /* Next char */
            if (!*s)
                goto exit_sendkeydef ;
            if (*s != '(') {            /* Should be left paren */
                sendbufd(CMDQ) ;
                sendbufd(*s) ;
                continue;
            }
            b = s - 1;                  /* Save my place */
            s++;                        /* Point past paren */
            p = kbuf;                   /* Point to word buffer */
            x = 0;                      /* Copy verb into it */
            while ((x++ < K_BUFLEN) && (*s > SP) && (*s != ')'))
                *p++ = *s++;
            *p = NUL;
            if (*s != ')') {            /* Have terminating paren? */
                                        /* No, send this literally */
                sendbufd(CMDQ) ;
                s = b;
                continue;
            } else {
                p = kbuf;               /* Point back to beginning */
                debug(F110,"sendkeydef v(verb)",p,0);
                p = nvlook(p);          /* Look it up. */
                if (!p)                 /* Not found, */
                    continue;           /* continue silently. */
                while (*p) {            /* Send it. */
                    sendbufd(*p++) ;
                }
                if (!*s)
                    goto exit_sendkeydef ;
            }
        } else if (*s == CMDQ) {        /* A Quoted CMDQ */
            sendbufd(CMDQ);
        } else {                        /* Not \K, \{K, \V, or \\ */
                                        /* Back up and send backslash, */
            sendbufd(CMDQ) ;
            if (brace) {                /* and maybe a left brace, */
                sendbufd('{') ;
                brace = 0;
            }
            if (*s) /* and this char literally. */
                sendbufd(*s) ;
        }
        if (!*s) {
            keydefptr = NULL;
            return;
        }
    }
  exit_sendkeydef:
    if ( sendndx ) {
        sendcharsduplex( sendbuf, sendndx, no_xlate ) ;
        sendndx = 0 ;
    }
    keydefptr = NULL;
#undef sendbufd
#endif /* NOKVERBS */
}

void
scrollstatusline( void )
{
#ifndef NOKVERBS
    /* Build a special status line for scrollback mode */
    /* Usertext field updated dynamicly in line25()    */

    if ( markmodeflag[VTERM] == notmarking ) {
        if (vik.upscn > 255 && keyname(vik.upscn)) {
                strncpy(helptext, keyname(vik.upscn), 20);
            if (vik.dnscn > 255 && keyname(vik.dnscn)) {
                ckstrncat(helptext,"/",HLPTXTLEN);
                ckstrncat(helptext,keyname(vik.dnscn),HLPTXTLEN);
            }
        }
        else
            ckstrncpy(helptext, "\\KupScn,\\KdnScn",HLPTXTLEN);
        }
    else {
        sprintf(helptext, "%c,%c,%c,%c\0",27,24,25,26);
    }

    if ((vik.help > 255) &&
         (keyname(vik.help)) &&
         ((HSTNAMLEN - 6 - (int)strlen(keyname(vik.help))) > 0)) {
        sprintf(hostname, "Help: %s", keyname(vik.help));
    }
    else if (vik.homscn > 255 && keyname(vik.homscn)) {
            strcpy(hostname, keyname(vik.homscn));
        if (vik.endscn > 255 && keyname(vik.endscn)) {
            ckstrncat(hostname,"/",HSTNAMLEN);
            ckstrncat(hostname,keyname(vik.endscn),HSTNAMLEN);
        }
    }
    else
        ckstrncpy(hostname, "\\KhomScn,\\KendScn",HSTNAMLEN);

    helptext[0] = '\0';
    exittext[0] = '\0';
    /* filetext[0] = '\0'; */
#endif /* NOKVERBS */
}

/* Mark mode - this replaces most of the Mouse Functions */

void
markmode( BYTE vmode, int k )
{
#ifndef NOKVERBS
    static int rollstate[VNUM]={0,0,0,0},
               scrollstate[VNUM]={0,0,0,0};
    static KEY savekeys[8]={0,0,0,0,0,0,0,0};
    static MACRO savemacros[8]={0,0,0,0,0,0,0,0};
    con_event evt ;

    if ( k != K_MARK_START && markmodeflag[vmode] == notmarking )
    {
       switch( k )
       {
       case K_BACKSRCH:
          markmode( vmode, K_MARK_START ) ;
          if ( search( vmode, FALSE, TRUE ) )
          {
             k = K_MARK_START ;
             break;
          }
          else
          {
             markmode( vmode, K_MARK_CANCEL ) ;
             bleep( BP_FAIL ) ;
             return;
          }
       case K_BACKNEXT:
          markmode( vmode, K_MARK_START ) ;
          if ( search( vmode, FALSE, FALSE ) )
          {
             k = K_MARK_START ;
             break;
          }
          else
          {
             markmode( vmode, K_MARK_CANCEL ) ;
             bleep( BP_FAIL ) ;
             return;
          }
       case K_FWDSRCH:
          markmode( vmode, K_MARK_START ) ;
          if ( search( vmode, TRUE, TRUE ) )
          {
             k = K_MARK_START ;
             break;
          }
          else
          {
             markmode( vmode, K_MARK_CANCEL ) ;
             bleep( BP_FAIL ) ;
             return;
          }
       case K_FWDNEXT:
          markmode( vmode, K_MARK_START ) ;
          if ( search( vmode, TRUE, FALSE ) )
          {
             k = K_MARK_START ;
             break;
          }
          else
          {
             markmode( vmode, K_MARK_CANCEL ) ;
             bleep( BP_FAIL ) ;
             return;
          }

       case K_MARK_CANCEL:
           return;

       default:
          bleep( BP_WARN ) ;
          return ;
       }
    }

    if ( markmodeflag[vmode] == notmarking ) {
        /* Save Macro and Keymap settings for Arrow keys */
        savemacros[0] = macrotab[KEY_SCAN | KEY_ENHANCED | 38] ;     /* Up Arrow    Gray */
        savemacros[1] = macrotab[KEY_SCAN | 38               ] ;     /* Up Arrow    Numeric */
        savemacros[2] = macrotab[KEY_SCAN | KEY_ENHANCED | 37] ;     /* Left Arrow  Gray */
        savemacros[3] = macrotab[KEY_SCAN | 37               ] ;     /* Left Arrow  Numeric*/
        savemacros[4] = macrotab[KEY_SCAN | KEY_ENHANCED | 39] ;     /* Right Arrow Gray */
        savemacros[5] = macrotab[KEY_SCAN | 39               ] ;     /* Right Arrow Numeric */
        savemacros[6] = macrotab[KEY_SCAN | KEY_ENHANCED | 40] ;     /* Down Arrow  Gray */
        savemacros[7] = macrotab[KEY_SCAN | 40               ] ;     /* Down Arrow  Numeric */

        savekeys[0] = keymap[KEY_SCAN | KEY_ENHANCED | 38] ;         /* Up Arrow    Gray */
        savekeys[1] = keymap[KEY_SCAN | 38               ] ;         /* Up Arrow    Numeric */
        savekeys[2] = keymap[KEY_SCAN | KEY_ENHANCED | 37] ;         /* Left Arrow  Gray */
        savekeys[3] = keymap[KEY_SCAN | 37               ] ;         /* Left Arrow  Numeric*/
        savekeys[4] = keymap[KEY_SCAN | KEY_ENHANCED | 39] ;         /* Right Arrow Gray */
        savekeys[5] = keymap[KEY_SCAN | 39               ] ;         /* Right Arrow Numeric */
        savekeys[6] = keymap[KEY_SCAN | KEY_ENHANCED | 40] ;         /* Down Arrow  Gray */
        savekeys[7] = keymap[KEY_SCAN | 40               ] ;         /* Down Arrow  Numeric */

        macrotab[KEY_SCAN | KEY_ENHANCED | 38] = NULL ;              /* Up Arrow    Gray */
        macrotab[KEY_SCAN | 38               ] = NULL ;              /* Up Arrow    Numeric */
        macrotab[KEY_SCAN | KEY_ENHANCED | 37] = NULL ;              /* Left Arrow  Gray */
        macrotab[KEY_SCAN | 37               ] = NULL ;              /* Left Arrow  Numeric*/
        macrotab[KEY_SCAN | KEY_ENHANCED | 39] = NULL ;              /* Right Arrow Gray */
        macrotab[KEY_SCAN | 39               ] = NULL ;              /* Right Arrow Numeric */
        macrotab[KEY_SCAN | KEY_ENHANCED | 40] = NULL ;              /* Down Arrow  Gray */
        macrotab[KEY_SCAN | 40               ] = NULL ;              /* Down Arrow  Numeric */

        keymap[KEY_SCAN | KEY_ENHANCED | 38] = F_KVERB | K_UPARR;       /* Up Arrow    Gray */
        keymap[KEY_SCAN | 38               ] = F_KVERB | K_UPARR;       /* Up Arrow    Numeric */
        keymap[KEY_SCAN | KEY_ENHANCED | 37] = F_KVERB | K_LFARR;       /* Left Arrow  Gray */
        keymap[KEY_SCAN | 37               ] = F_KVERB | K_LFARR;       /* Left Arrow  Numeric*/
        keymap[KEY_SCAN | KEY_ENHANCED | 39] = F_KVERB | K_RTARR;       /* Right Arrow Gray */
        keymap[KEY_SCAN | 39               ] = F_KVERB | K_RTARR;       /* Right Arrow Numeric */
        keymap[KEY_SCAN | KEY_ENHANCED | 40] = F_KVERB | K_DNARR;       /* Down Arrow  Gray */
        keymap[KEY_SCAN | 40               ] = F_KVERB | K_DNARR;       /* Down Arrow  Numeric */

        rollstate[vmode] = tt_roll[vmode] ;
        scrollstate[vmode] = scrollflag[vmode] ;
        markmodeflag[vmode] = inmarkmode ;

        if (!scrollstate[vmode]) {
            scrollstatus[vmode] = TRUE ;
            }

        scrollstatusline() ;
        scrollflag[vmode] = TRUE ;
        if ( !scrollstate[vmode] )
            VscrnSetScrollTop( vmode, VscrnGetTop(vmode) ) ;
        VscrnIsDirty(vmode) ;
        return ;
        }  /* if (markmode[vmode] == notmarking) */

        switch (k &= ~(F_KVERB)) {
        case K_MARK_START:
            scrollstatusline();
            markstart(vmode) ;
            break;

        case K_MARK_CANCEL:
            markcancel(vmode) ;
            break;

        case K_MARK_COPYCLIP:
            markcopyclip(vmode,0) ;
            break;

        case K_MARK_COPYHOST:
            markcopyhost(vmode,0) ;
            break;

       case K_MARK_SELECT:
            markselect(vmode,0);
            break;

        case K_MARK_COPYCLIP_NOEOL:
            markcopyclip(vmode,1) ;
            break;

        case K_MARK_COPYHOST_NOEOL:
            markcopyhost(vmode,1) ;
            break;

       case K_MARK_SELECT_NOEOL:
            markselect(vmode,1);
            break;

        case K_DNONE:
        case K_DNARR:
            markdownone(vmode) ;
            break;

        case K_UPONE:
        case K_UPARR:
            markupone(vmode) ;
            break;

        case K_LFONE:
        case K_LFARR:
            markleftone(vmode) ;
            break;

        case K_RTONE:
        case K_RTARR:
            markrightone(vmode) ;
            break;

        case K_DNSCN:
            markdownscreen(vmode) ;
            break;

        case K_UPSCN:
            markupscreen(vmode) ;
            break;

        case K_HOMSCN:
            markhomescreen(vmode) ;
            break;

        case K_ENDSCN:
            markendscreen(vmode) ;
            break;

#ifdef IN_PROGRESS  /* They just are not finished */
        case K_LFPAGE:
            markleftpage(vmode) ;
            break;

        case K_RTPAGE:
            markrightpage(vmode) ;
            break;
#endif /* IN_PROGRESS */

        case K_DUMP:
            markprint(vmode, 0);
            break;

        case K_HELP:                    /* Help */
            while ( popuphelp( vmode, hlp_markmode ) == ( F_KVERB | K_HELP ) ) ;
            return;

        case K_EXIT:            /* Exit terminal mode */
            markcancel(vmode);
            putkey( vmode, vik.exit ) ;
            return ;

    case K_QUIT:            /* Quit Kermit */
            markcancel(vmode);
            putkey( vmode, vik.quit ) ;
            return ;

    case K_BACKSRCH:
           if (!search( vmode, FALSE, TRUE ) )
           {
              markcancel(vmode);
              bleep(BP_FAIL);
           }
           break;

    case K_BACKNEXT:
           if (!search( vmode, FALSE, FALSE ))
           {
              markcancel(vmode);
              bleep(BP_FAIL);
           }
           break;
    case K_FWDSRCH:
           if (!search( vmode, TRUE, TRUE ))
           {
              markcancel(vmode);
              bleep(BP_FAIL);
           }
           break;
    case K_FWDNEXT:
           if (!search( vmode, TRUE, FALSE ))
           {
              markcancel(vmode);
              bleep(BP_FAIL);
           }
           break;

        default:
            bleep(BP_WARN);
        }

    if ( markmodeflag[vmode] == notmarking ) {
        if (scrollstate[vmode]) {
            scrollstatusline() ;
            }
        else {
            scrollstatus[vmode] = FALSE ;
            ipadl25() ;   /* put back the normal status line */
            }

        scrollflag[vmode] = scrollstate[vmode] ;
        tt_roll[vmode] = rollstate[vmode] ;

        VscrnSetCurPos( vmode, wherex[vmode] -1, wherey[vmode]-1 ) ;

        /* Restore Macro and KeyMap settings for Arrow keys */

        macrotab[KEY_SCAN | KEY_ENHANCED | 38] = savemacros[0] ;     /* Up Arrow    Gray */
        macrotab[KEY_SCAN | 38               ] = savemacros[1] ;     /* Up Arrow    Numeric */
        macrotab[KEY_SCAN | KEY_ENHANCED | 37] = savemacros[2] ;     /* Left Arrow  Gray */
        macrotab[KEY_SCAN | 37               ] = savemacros[3] ;     /* Left Arrow  Numeric*/
        macrotab[KEY_SCAN | KEY_ENHANCED | 39] = savemacros[4] ;     /* Right Arrow Gray */
        macrotab[KEY_SCAN | 39               ] = savemacros[5] ;     /* Right Arrow Numeric */
        macrotab[KEY_SCAN | KEY_ENHANCED | 40] = savemacros[6] ;     /* Down Arrow  Gray */
        macrotab[KEY_SCAN | 40               ] = savemacros[7] ;     /* Down Arrow  Numeric */

        keymap[KEY_SCAN | KEY_ENHANCED | 38] = savekeys[0] ;         /* Up Arrow    Gray */
        keymap[KEY_SCAN | 38               ] = savekeys[1] ;         /* Up Arrow    Numeric */
        keymap[KEY_SCAN | KEY_ENHANCED | 37] = savekeys[2] ;         /* Left Arrow  Gray */
        keymap[KEY_SCAN | 37               ] = savekeys[3] ;         /* Left Arrow  Numeric*/
        keymap[KEY_SCAN | KEY_ENHANCED | 39] = savekeys[4] ;         /* Right Arrow Gray */
        keymap[KEY_SCAN | 39               ] = savekeys[5] ;         /* Right Arrow Numeric */
        keymap[KEY_SCAN | KEY_ENHANCED | 40] = savekeys[6] ;         /* Down Arrow  Gray */
        keymap[KEY_SCAN | 40               ] = savekeys[7] ;         /* Down Arrow  Numeric */
        } /* if ( markmodeflag[vmode] == notmarking ) */

    os2settitle(NULL,1);
    VscrnIsDirty(vmode);
#endif /* NOKVERBS */
}

/* Scrollback handler */
/*
  Totally rewritten for edit 190 to allow TERMINAL ROLL options and to
  use new keymap  - fdc.

  And again for 191 - jaltman.
*/

void
scrollback(BYTE vmode, int k) {                 /* Keycode */
#ifndef NOKVERBS
    con_event evt ;

    /* Initialization */

#ifdef DEBUG
    debug(F101,"scrollback key","",key);
    debug(F101,"scrollback tt_roll","",tt_roll[vmode]);
#endif /* DEBUG */

    if ( !tt_scroll ) {
        debug(F100,"scrollback disabled","",0);
        return;
    }
/*
  If TERMINAL ROLL OFF, we just process the argument key and return control of
  the keyboard to the user.  This lets the user type characters to the host
  while looking at rollback screens, as well as view new incoming characters
  on an old screen.  But it can result in some confusion by mixing new and old
  material in the rollback buffer.

  If TERMINAL ROLL ON, the user is held captive inside the rolled-back
  screens, allowed to type only rollback keys until reaching the bottom again,
  and no characters are read from the communication device while rolled back.
  This prevents transmission of characters to the host, and it prevents
  mixture of new and old material in the rollback buffer.

  This is not true in this code anymore.  We always fall through regardless
  of ROLL ON or OFF.
*/
    switch (k &= ~(F_KVERB)) {  /* Handle the key... */

    case K_GO_BOOK:       /* Scroll to Bookmark location */
       bookmarkjump(vmode);
       break;

    case K_GOTO:
       gotojump(vmode);
       break;

    case K_HOMSCN:              /* Scrolling UP (backwards) ... */
        if (!tt_roll[vmode]) {
            if ( VscrnSetTop(vmode, VscrnGetBegin(vmode)) < 0 )
              bleep(BP_WARN) ;
        }
        else {
            if ( VscrnSetScrollTop(vmode,VscrnGetBegin(vmode)) < 0 )
              bleep(BP_WARN);
        }
        break;

    case K_UPSCN:
        if (!tt_roll[vmode]) {
            if ( VscrnMoveTop(vmode,-(VscrnGetHeight(vmode)-(tt_status[vmode]?1:0))) < 0 )
              if ( VscrnSetTop(vmode,VscrnGetBegin(vmode)) < 0 )
                bleep(BP_WARN) ;
        }
        else {
            if ( VscrnMoveScrollTop(vmode,-(VscrnGetHeight(vmode)-(tt_status[vmode]?1:0))) < 0 )
              if ( VscrnSetScrollTop(vmode,VscrnGetBegin(vmode)) < 0 )
                bleep(BP_WARN);
        }
        break;

    case K_UPONE:
        if (!tt_roll[vmode]) {
            if ( VscrnMoveTop(vmode,-1) < 0 )
              bleep(BP_WARN) ;
        }
        else {
            if ( VscrnMoveScrollTop(vmode,-1) < 0 )
              bleep(BP_WARN);
        }
        break;

    case K_DNSCN:                       /* Go down */
        if (!tt_roll[vmode]) {
            if ( VscrnMoveTop(vmode,VscrnGetHeight(vmode)-(tt_status[vmode]?1:0)) < 0 )
              if ( VscrnSetTop(vmode,VscrnGetEnd(vmode) - VscrnGetHeight(vmode)+1) < 0 )
                bleep(BP_WARN);
        }
        else {
            if ( VscrnMoveScrollTop(vmode,VscrnGetHeight(vmode)-(tt_status[vmode]?1:0)) < 0 )
              if ( VscrnSetScrollTop(vmode,VscrnGetEnd(vmode)
                                      - (VscrnGetHeight(vmode)-(tt_status[vmode]?1:0))+1) < 0 )
                bleep(BP_WARN);
        }
        break;

    case K_DNONE:
        if (!tt_roll[vmode]) {
            if ( VscrnMoveTop(vmode,1) < 0 )
              bleep(BP_WARN);
        }
        else {
            if ( VscrnMoveScrollTop(vmode,1) < 0 )
              bleep(BP_WARN);
        }
        break;

    case K_ENDSCN:              /* Scroll to bottom */
        if (!tt_roll[vmode]) {
            if ( VscrnSetTop(vmode,VscrnGetEnd(vmode) - VscrnGetHeight(vmode)-(tt_status[vmode]?1:0) + 1) < 0 )
              bleep(BP_WARN);
        }
        else {
            if ( VscrnSetScrollTop(vmode,VscrnGetTop(vmode)) < 0 )
              bleep(BP_WARN);
        }
        break;

    case K_DUMP: {              /* Print/Dump current screen */
        int x;
        x = xprintff; xprintff = 0;
        prtscreen(vmode,1,VscrnGetHeight(vmode)-(tt_status[vmode]?1:0));
        xprintff = x;
        break;
    }

    case K_HELP: {              /* Help */
        int x = hlp_rollback;
        while ( popuphelp(vmode,x) == ( F_KVERB | K_HELP ) )
            switch ( x ) {
#ifdef OS2MOUSE
            case hlp_rollback:
                x = hlp_mouse;
                break;
            case hlp_mouse:
                x = hlp_rollback;
                break;
#else /* OS2MOUSE */
            case hlp_rollback:
                x = hlp_rollback;
                break;
#endif /* OS2MOUSE */
            }
        killcursor(vmode);              /* Turn it back off */
        break;
    }

    case K_MARK_START:
        markmode(vmode,K_MARK_START);
        break;

    case K_EXIT:
        putkey( vmode, vik.endscn ) ;
        putkey( vmode, vik.exit ) ;
        break ;

    case K_QUIT:
        putkey( vmode, vik.endscn ) ;
        putkey( vmode, vik.quit ) ;
        break ;

    default:                    /* Not a rollback key. */
        bleep(BP_WARN);
        break;

    } /* switch (k) */

    scrollflag[vmode] = tt_roll[vmode] && ( VscrnGetTop(vmode) != VscrnGetScrollTop(vmode) ) ;

    if ( !scrollstatus[vmode] ) {
        scrollstatus[vmode] = TRUE ;
        scrollstatusline() ;
    }

/*
  Get here after processing one keystroke and TERMINAL ROLL is OFF,
  or when TERMINAL ROLL is ON, only when we've reached the bottom.
  Wrong.
*/
    if ( !scrollflag[vmode] ) {
        if (tt_roll[vmode] ||
             (VscrnGetTop(vmode)+VscrnGetHeight(vmode)-(tt_status[vmode]?2:1))%VscrnGetBufferSize(vmode) == VscrnGetEnd(vmode))
        {
            scrollstatus[vmode] = FALSE ;
            ipadl25();                  /* Put back normal status line */
        }
    } /* if ( !scrollflag[vmode] ) */

    os2settitle(NULL,1);
    VscrnIsDirty(vmode) ;
#endif /* NOKVERBS */
}

void
vt100key(int key) {
#ifdef NOKVERBS
    char str[3];                        /* Not needed in simplified code */
    int prt, st;
#endif /* NOKVERBS */

    if ( keydefptr ) {                  /* First send any leftovers */
        sendkeydef( keydefptr,FALSE );  /* from previous session... */
        keydefptr = NULL;                   /* and then forget about it */
        return;
    }

    if (key < 0x100 && !kbdlocked()) {      /* If it's a regular key, */
        sendcharduplex((char) key,FALSE);/* just send it, */
        if (tt_pacing > 0)              /* taking care of pacing */
            msleep(tt_pacing);
        if (key == CR && tnlm) {        /* and newline mode */
            if (tt_pacing > 0)          /* and pacing */
                msleep(tt_pacing);
        }
        return;
    }

    /* vt100key is only called from within conkbdhandler() */
    /* and all events are handled there, so how would this */
    /* get here */
    if ( IS_CSI(key) ) {
        char buf[3] ;
        buf[0] = '[' ;
        buf[1] = key & 0xFF ;
        buf[2] = NUL ;
        sendescseq(buf) ;
    }
    else if ( IS_ESC(key) ) {
        char buf[2] ;
        buf[0] = key & 0xFF ;
        buf[1] = NUL ;
        sendescseq(buf) ;
    }
#ifndef NOKVERBS
    else if (IS_KVERB(key)) {        /* Specially precoded keyboard verb */
        dokverb(VTERM,key) ;          /* We already know its index */
    }
    else
        bleep(BP_WARN);                         /* Extended key with no definition. */
    return;                             /* Just beep & return. */
#endif /* NOKVERBS */
}

#ifdef CK_APC
/* ------------------------------------------------------------------ */
/* doosc - process OSC sequences (VT320 and higher)                   */
/*         sequence of apclength in apcbuf                            */
/* ------------------------------------------------------------------ */

void
doosc( void ) {
/* at current we only process two SET WINDOW TITLE and SET ICON TITLE */
/* So let's not implement a full parser                               */

    /* AIXTERM */
    /*   0 - Set Icon and Title */
    /*   1 - Set Icon only      */
    /*   2 - Set Title only     */

    switch ( apcbuf[0] ) {
    case '0': { /* XTERM */
        /* the rest of the apcbuffer is the Window Title */
        char wtitle[31] ;
        int i ;
        for ( i=0;i<=30 && i+2 < apclength; i++ )
            wtitle[i] = apcbuf[i+2] ;
        if ( i > 0 && apcbuf[i-1] == 0x07 ) {
            /* XTERMs may append a Beep indicator at the end */
            wtitle[i-1] = NUL ;
            bleep(BP_NOTE);
        }
        else
            wtitle[i] = NUL ;

        if (!os2settitle(wtitle,1)) {
            bleep(BP_FAIL);
            debug(F110,"doosc os2settitle fails",wtitle,0);
        }
        break;
    }
    case '2': /* DEC VTxxx */
        switch ( apcbuf[1] ) {
        case 'L':       /* SET ICON TITLE - DECSIN */
            /* the rest of the apcbuffer is the Icon Title */
            break;
        case '1': {     /* SET WINDOW TITLE - DECSWT */
            /* the rest of the apcbuffer is the Window Title */
            char wtitle[31] ;
            int i ;
            for ( i=0;i<=30 && i+3<=apclength; i++ )
                wtitle[i] = apcbuf[i+3] ;
            wtitle[i] = NUL ;

            if (!os2settitle(wtitle,1)) {
                bleep(BP_FAIL);
                debug(F110,"doosc os2settitle fails",wtitle,0);
            }
            break;
        }
        }
        break;
    }
    return;
}

/* dopu1 - process PU1 sequences (97801-5xx terminals) */
void
dopu1( void )
{
    debug(F111,"dopu1",apcbuf,apclength);
}

/* dopu2 - process PU2 sequences (97801-5xx terminals) */
void
dopu2( void )
{
    debug(F111,"dopu2",apcbuf,apclength);
    switch ( apcbuf[0] ) {
    case 'A':
        /* Load key assignment table */
        break;
    case 'B':
        /* Load one or more character generator addresses with a new */
        /* symbol ( ... = 3-byte address and 28/32-byte pattern in hex format ) */
        break;
    case 'C':
        if ( apcbuf[1] == NUL )
            ; /* Query load process for string key table */
        else
            ; /* Load string key table */
        break;
    case 'D':
        if ( apclength == 1 )
            ; /* Delete compose key table */
        else
            ; /* Load compose key table */
        break;
    case 'E':
        /* Load dead key table */
        break;
    case 'F':
        /* Load key assignment table */
        break;
    }
}

/* ------------------------------------------------------------------ */
/* dodcs - process DCS sequences (VT320 and higher)                   */
/*         sequence of apclength in apcbuf                            */
/* ------------------------------------------------------------------ */

void
dodcs( void )
{
    int pn[11];
    int i,k,l;
    int dcsnext = 0 ;
    char keydef[257] ; /* UDK defs can't be longer than 256 chars */
    char c, * p ;

    /*
    we haven't coded this yet
    but what would go here would be DECUDK, DECRSPS, DECRQSS, ....
    */
    debug( F111,"DCS string",apcbuf,apclength ) ;

    if ( debses )              /* If TERMINAL DEBUG ON */
      return ;                 /* don't do anything    */
    achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
    switch ( achar ) {
    case '$':  /* as in $q - DECRQSS */
        k = 0 ;
        goto LB4003;
    case '|':  /* DECUDK */
        pn[1] = pn[2] = 0;
        k = 2;
        goto LB4003;
    case ';':           /* As in DCS ; 7 m */
        pn[1] = 0;
        k = 1;
        goto LB4002;
    default:            /* Pn - got a number */
      LB4001:
        {               /* Esc [ Pn...Pn x   functions */
            pn[1] = 0 ;
            while (isdigit(achar)) {            /* Get number */
                pn[1] = (pn[1] * 10) + achar - 48;
                achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
            }
            k = 1;
          LB4002:
            while (achar == ';') { /* get Pn[k] */
                achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                k++;
                if (achar == '?')
                  achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                                pn[k] = 0 ;
                while (isdigit(achar)) {                /* Get number */
                    pn[k] = (pn[k] * 10) + achar - 48;
                    achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                }
            }
            pn[k + 1] = 1;
          LB4003:
            switch (achar) { /* Third level */
            case '$': {
                char decrpss[12] = "";
                achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                switch ( achar ) {
                case 'q':               /* DECRQSS */
                    /* The next set of characters are the D...D portion */
                    /* of the DECRQSS request */
                    achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                    switch ( achar ) {
                    case '}':           /* DECPRO */
                        if ( send_c1 )
                            sprintf(decrpss,"%c0$r}%c",_DCS,_ST8);
                        else
                            sprintf(decrpss,"%cP0$r}%c\\",ESC,ESC);
                        break;
                    case '$':
                        achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                        switch ( achar ) {
                        case '}':       /* DECSASD */
                            if ( send_c1 )
                                sprintf(decrpss,"%c0$r$}%c",_DCS,_ST8);
                            else
                                sprintf(decrpss,"%cP0$r$}%c\\",ESC,ESC);
                            break;
                        case '|':       /* DECSCPP */
                            if ( send_c1 )
                                sprintf(decrpss,"%c0$r$|%c",_DCS,_ST8);
                            else
                                sprintf(decrpss,"%cP0$r$|%c\\",ESC,ESC);
                            break;
                        case '~':       /* DECSSDT */
                            if ( send_c1 )
                                sprintf(decrpss,"%c0$r$~%c",_DCS,_ST8);
                            else
                                sprintf(decrpss,"%cP0$r$~%c\\",ESC,ESC);
                            break;
                        }
                        break;
                    case '"':
                        achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                        switch ( achar ) {
                        case 'q':       /* DECSCA */
                            if ( send_c1 )
                                sprintf(decrpss,"%c0$r\"q%c",_DCS,_ST8);
                            else
                                sprintf(decrpss,"%cP0$r\"q%c\\",ESC,ESC);
                            break;
                        case 'p':       /* DECSCL */
                            if ( send_c1 )
                                sprintf(decrpss,"%c0$r\"p%c",_DCS,_ST8);
                            else
                                sprintf(decrpss,"%cP0$r\"p%c\\",ESC,ESC);
                            break;
                        }
                        break;
                    case 't':           /* DECSLPP */
                        if ( send_c1 )
                            sprintf(decrpss,"%c0$rt%c",_DCS,_ST8);
                        else
                            sprintf(decrpss,"%cP0$rt%c\\",ESC,ESC);
                        break;
                    case 'r':           /* DECSTBM */
                        if ( send_c1 )
                            sprintf(decrpss,"%c0$rr%c",_DCS,_ST8);
                        else
                            sprintf(decrpss,"%cP0$rr%c\\",ESC,ESC);
                        break;
                    case '|':           /* DECTTC */
                        if ( send_c1 )
                            sprintf(decrpss,"%c0$r|%c",_DCS,_ST8);
                        else
                            sprintf(decrpss,"%cP0$r|%c",ESC,ESC);
                        break;
                    case '\'':
                        achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                        switch ( achar ) {
                        case 's':       /* DECTLTC */
                            if ( send_c1 )
                                sprintf(decrpss,"%c0$r\'s%c",_DCS,_ST8);
                            else
                                sprintf(decrpss,"%cP0$r\'s%c\\",ESC,ESC);
                            break;
                        }
                        break;
                    case 'm':           /* SGR */
                        if ( send_c1 )
                            sprintf(decrpss,"%c0$rm%c",_DCS,_ST8);
                        else
                            sprintf(decrpss,"%cP0$rm%c\\",ESC,ESC);
                        break;
                    case 's':           /* DECSLRM - Set Left and Right Margins */
                        if ( send_c1 )
                            sprintf(decrpss,"%c1$rs%c",_DCS,_ST8);
                        else
                            sprintf(decrpss,"%cP1$rs%c\\",ESC,ESC);
                        break;
                    case '*':
                        achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                        switch ( achar ) {
                        case 'x':       /* DECSACE - Select Attrib Change Extent */
                            if ( send_c1 )
                                sprintf(decrpss,"%c%d$r*x%c",_DCS,decsace?2:1,_ST8);
                            else
                                sprintf(decrpss,"%cP%d$r*x%c\\",ESC,decsace?2:1,ESC);
                            break;
                        case '|':       /* DECSNLS - Set Num Lines Per Screen */
                            if ( send_c1 )
                                sprintf(decrpss,"%c1$r*|%c",_DCS,_ST8);
                            else
                                sprintf(decrpss,"%cP1$r*|%c\\",ESC,ESC);
                            break;
                        }
                        break;
                    case '+':
                        achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                        switch ( achar ) {
                        case 'q':       /* DECELF - Enable Local Functions */
                            if ( send_c1 )
                                sprintf(decrpss,"%c1$r+q%c",_DCS,_ST8);
                            else
                                sprintf(decrpss,"%cP1$r+q%c\\",ESC,ESC);
                            break;
                        case 'r':       /* DECSMKR - Select modifier key reporting */
                            if ( send_c1 )
                                sprintf(decrpss,"%c1$r+r%c",_DCS,_ST8);
                            else
                                sprintf(decrpss,"%cP1$r+r%c\\",ESC,ESC);
                            break;
                        }
                        break;
                    case '=':
                        achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                        switch ( achar ) {
                        case '}':       /* DECLFKC - Local Function Key Control */
                            if ( send_c1 )
                                sprintf(decrpss,"%c1$r=}%c",_DCS,_ST8);
                            else
                                sprintf(decrpss,"%cP1$r=}%c\\",ESC,ESC);
                            break;
                        }
                        break;
                    }
                    if ( decrpss[0] ) {
                        if ( send_c1 )
                            sprintf(decrpss,"%c1$r%c",_DCS,_ST8);
                        else
                            sprintf(decrpss,"%cP1$r%c\\",ESC,ESC);
                    }
                    if (decrpss[0])
                        sendchars(decrpss,strlen(decrpss));
                    break;
                }
				break;
            } /* $ */
            case '|': {    /* DECUDK */
                int key = 0 ;
                /* pn[1] - Clear keys: 0 All, 1 One */
                /* pn[2] - Lock keys:  0 Lock, 1 no Lock */
                if ( k<2 )
                    pn[2] = 1 ;
                if ( k<1 )
                    pn[1] = 0 ;

                if ( udklock )
                    return;  /* do nothing if the UDKs are locked */

                if ( pn[1] == 0 ) { /* Clear all key defs */
                    udkreset() ;
                }

                while ( dcsnext < apclength ) {
                    /* look for patterns of XX/string; */

                    key=0;
                    /* which key to assign to? */
                    achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                    if ( achar == ';' )
                        achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;

                    while (isdigit(achar)) {                /* Get number */
                        key = (key * 10) + achar - 48;
                        achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                    }

                    /* Check for proper separator */
                    if ( achar != '/') {
                        /* if error, call cwrite() on rest of DCS         */
                        /* including ST.  Actually, in a real VT terminal */
                        /* we wouldn't even wait until this point to      */
                        /* determine if there was an error.  We would do  */
                        /* this as the data is received.                  */
                        while ( achar ) {
                            cwrite(achar);
                            achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                        }
                        cwrite(ESC);
                        cwrite('\\');
                        return ;
                    }

                    /* Get the key definition */
                    l = 0 ; /* characters in definition */
                    i = 0 ; /* which pass? */
                    while ( apcbuf[dcsnext] != ';' &&
                            dcsnext < apclength ) {
                        achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                        c = 0 ;
                        if ( isxdigit(achar) )
                        {
                            if ( isdigit(achar) )
                                c = achar - '0' ;
                            else if ( islower(achar) )
                                c = achar - 'a' + 10 ;
                            else
                                c = achar - 'A' + 10 ;
                        }   
                        if ( i ) {
                            keydef[l] = (keydef[l]<<4)|c ;
                            l++ ;
                            keydef[l] = '\0' ;
                            i = 0 ;
                        }
                        else {
                            keydef[l] = c ;
                            i = 1 ;
                        }
                    }   

                    if ( i == 1 ||
                         apcbuf[dcsnext] != ';' &&
                         dcsnext < apclength ){
                        /* if error, call cwrite() on rest of DCS         */
                        /* including ST.  Actually, in a real VT terminal */
                        /* we wouldn't even wait until this point to      */
                        /* determine if there was an error.  We would do  */
                        /* this as the data is received.                  */
                        while ( achar ) {
                            cwrite(achar);
                            achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                        }
                        cwrite(ESC);
                        cwrite('\\');
                        return ;
                    }

                    i = -1 ;
                    switch ( key ) {
                        /* DEC couldn't make it easy, could they */
                    case 11: i = 0 ; break ;
                    case 12: i = 1 ; break ;
                    case 13: i = 2 ; break ;
                    case 14: i = 3 ; break ;
                    case 15: i = 4 ; break ;
                    case 17: i = 5 ; break ;
                    case 18: i = 6 ; break ;
                    case 19: i = 7 ; break ;
                    case 20: i = 8 ; break ;
                    case 21: i = 9 ; break ;
                    case 23: i = 10 ; break ;
                    case 24: i = 11 ; break ;
                    case 25: i = 12 ; break ;
                    case 26: i = 13 ; break ;
                    case 28: i = 14 ; break ;
                    case 29: i = 15 ; break ;
                    case 31: i = 16 ; break ;
                    case 32: i = 17 ; break ;
                    case 33: i = 18 ; break ;
                    case 34: i = 19 ; break ;
                    }
                    if ( i>= 0 )
                    {
                        if ( udkfkeys[i] ) {
                            free( udkfkeys[i] ) ;
                            udkfkeys[i]=NULL;
                        }
                        if ( strlen(keydef) )
                            udkfkeys[i] = strdup(keydef) ;
                    }   
                    }   

                if ( !pn[2] )
                    udklock = TRUE ;
                break;
            }   /* | */
            case '+': {
                achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                switch ( achar ) {
                case 'm': {  /* URL or UNC attribute */
                    int  quote = 0;
                    char * link = NULL, *p;

                    if ( k < 1 )
                        pn[1] = 0;

                    switch ( pn[1] ) {
                    case 0:     /* End URL or UNC link */
                        debug(F100,"URL/UNC link attribute removed","",0);
                        if ( debses )
                            break;
                        attrib.hyperlink = FALSE;
                        attrib.linkid = 0;
                        break;
                    case 1:     /* Begin URL link */
                    case 2:     /* Begin UNC link */
                        link = (char *) malloc(apclength - dcsnext + 1);
                        if ( !link )
                            break;
                        
                        p = link;
                        achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                        quote = (achar == '"');
                        if ( quote )
                            achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;

                        while ( achar && (!quote || (quote && achar != '"') ))
                        {
                            *p++ = achar;
                             achar = (dcsnext<apclength)?apcbuf[dcsnext++]:0;
                        }
                        *p = '\0';
                        switch ( pn[1] ) {
                        case 1: 
                            debug(F110,"URL link assigned",link,0);
                            if ( debses ) {
                                free(link);
                                break;
                            }
                            attrib.hyperlink = TRUE;
                            attrib.linkid = hyperlink_add(HYPERLINK_URL, link);
                            break;
                        case 2:
                            debug(F110,"UNC link assigned",link,0);
                            if ( debses ) {
                                free(link);
                                break;
                            }
                            attrib.hyperlink = TRUE;
                            attrib.linkid = hyperlink_add(HYPERLINK_UNC, link);
                            break;
                        default:
                            debug(F110,"unknown link type",link,pn[1]);
                            free(link);
                        }
                        break;
                    }
                    break;
                }   /* m */
                }   /* achar */
                break;
            }  /* + */
            }   /* third level */
        }
    }
}
#endif /* CK_APC */

/* ------------------------------------------------------------------ */
/* cwrite - check and process escape sequence                         */
/* ------------------------------------------------------------------ */


void
debugses( unsigned char ch )
{
    static char old_c = 0;
    static int  deb_wrap = 0;

    if (debses && !f_pushed) {          /* Session debugging */
        unsigned char ch8 = f_popped ? ((ch ^ 0x40) | 0x80) : ch;
        int tt_wrap_sav = tt_wrap;
        tt_wrap = 0;

        deb_wrap = wherex[VTERM] >= VscrnGetWidth(VTERM);
        if (ch8 < 32 && !literal_ch) {  /* C0 Control character */
            attribute = colordebug;     /* Use underline color */
            if (decscnm)                /* Handle flipped screen */
              attribute = swapcolors(attribute);
            wrtch((char)(ch8 | 64));    /* Write corresponding letter */
        } else if (ch8 == 127) {        /* DEL */
            attribute = colordebug;     /* Use underline color */
            if (decscnm)                /* Handle flipped screen */
              attribute = swapcolors(attribute);
            wrtch('?');                 /* question mark */
        } else if (ch8 > 127 && ch8 < 160 && !literal_ch) { /* C1 control */
            attribute = colordebug;     /* Use underline color */
            ch8 = (ch8 ^ 0x40) & 0x7F ;
            if (!decscnm)               /* Handle flipped screen */
              attribute = swapcolors(attribute);
            wrtch(ch8);                 /* The character itself */
        } else if (escstate != ES_NORMAL) { /* Part of Esc sequence */
            attribute = swapcolors(defaultattribute); /* Reverse video */
            wrtch(ch8);
        } else {                        /* Regular character */
            attribute = defaultattribute; /* Normal colors */
            if (tn_bold) attribute ^= 8; /* (only for TELNET debugging...) */
            wrtch(ch8);
        }
        if (deb_wrap ||
             ch8 == LF && old_c == CR) { /* Break lines for readability */
            attribute = defaultattribute;
            wrtch(CR);
            wrtch(LF);
        }
        old_c = ch8;                    /* Remember this character */
                                        /* for CRLF matching. */
        tt_wrap = tt_wrap_sav;
    }
}

void
cwrite(unsigned short ch) {             /* Used by ckcnet.c for */
                                        /* TELNET options debug display. */
    static vt52esclen = 0 ;
/*
   Edit 190.
   New code, supporting APC, and integrating escape sequence state switching
   that was formerly done, but not used to good advantage, in chkaes().  The
   additional complexity comes from the fact that ESC is not used only to
   commence escape sequences, but also, in the case of APC, OSC, PM, etc, to
   terminate them.  New code also supports session debugging.

   Edit 192.
   New code supporting auto-download for Zmodem and Kermit protocols.
*/

    debugses( ch ) ;                    /* Session debugging */

#ifdef CKLEARN
    learnnet(ch);
#endif /* CKLEARN */

#ifndef NOXFER
    /* if Auto-download C0 conflicts are not being processed, stop now */
    if ( (!adl_kc0 && ch == stchr) || (!adl_zc0 && ch == CAN) )
         return;
#endif /* NOXFER */

#ifdef NOTERM
    if (!debses)
        wrtch(ch);
    return;
#else /* NOTERM */
    /* Unless we are parsing a terminal type based upon ANSI X3.64-1979 */
    /* this is as far as we go. */

    if (tt_type_mode == TT_NONE) {
        if (!debses)
            wrtch(ch);
        return;
    }

    if ( ISWYSE(tt_type_mode) )
    {
        wyseascii(ch) ;
        return ;
    }

    if ( ISDG200(tt_type_mode) )
    {
        dgascii(ch) ;
        return ;
    }

    if ( ISHP(tt_type_mode) )
    {
        hpascii(ch);
        return ;
    }

    if ( ISHZL(tt_type_mode) )
    {
        hzlascii(ch) ;
        return ;
    }

    if ( ISADM3A(tt_type_mode) || ISADM5(tt_type_mode) ) {
        admascii(ch);
        return;
    }

    if ( ISVC(tt_type_mode) )
    {
        vcascii(ch);
        return;
    }

    if ( ISIBM31(tt_type_mode) )
    {
        i31ascii(ch);
        return ;
    }

    if ( ISQNX(tt_type_mode) ) {
        qnxascii(ch);
        return;
    }

    if ( ISTVI(tt_type_mode) )
    {
        tviascii(ch);
        return ;
    }

    if ( ISBA80(tt_type_mode) && ba80_fkey_read ) {
        extern char fkeylabel[16][32];
        if ( ba80_fkey_read < 0 ) {
            int i=0;
            ba80_fkey_read = (ch - '0') * 10;
            ba80_fkey_ptr = NULL;
            for ( i=0;i<16;i++ )
                fkeylabel[i][0] = NUL;
        }
        else if ( ba80_fkey_read % 10 == 0 ) {
            if ( ba80_fkey_ptr )
                *ba80_fkey_ptr = NUL;
            if ( isdigit(ch) )
                ba80_fkey_ptr = &fkeylabel[ch-'1'][0];
            ba80_fkey_read--;
        } else {
            if ( ba80_fkey_ptr ) {
                *ba80_fkey_ptr = ch;
                ba80_fkey_ptr++;
            }
            ba80_fkey_read--;
        }
        return;
    }

    if ( ISVTNT(tt_type_mode) && !debses && !tn_deb) {
        /* Microsoft VTNT does not start using raw WinCon structs until after */
        /* the IAC SB TERMINAL-TYPE IS VTNT IAC SE has been sent to the host. */
        /* Until that time we just treat the data as ANSI X3.64 based.        */

        if ( ttnum == -1 )
            vt100(ch);
        else {
            PCONSOLE_SCREEN_BUFFER_INFO pScrnBufInf = NULL;
            PCOORD pCursor = NULL;
            PCOORD pBufCoord = NULL;
            PCOORD pBufSz = NULL;
            PSMALL_RECT pWriteRegion = NULL;
            PCHAR_INFO pCharInfo = NULL;

            if ( vtnt_index < vtnt_read && vtnt_index < VTNT_BUFSZ) {
                vtnt_buf[vtnt_index++] = ch;
                if ( vtnt_index == VTNT_MIN_READ ) {
                    int szchng = 0, h_vtnt, w_vtnt;

                    ckhexdump("VTNT MIN_READ",vtnt_buf,VTNT_MIN_READ);

                    /* Figure out how much more data we need to read */
                    pScrnBufInf = (PCONSOLE_SCREEN_BUFFER_INFO) vtnt_buf;
                    pCursor = (PCOORD) ((PCHAR) pScrnBufInf
                                      + sizeof(CONSOLE_SCREEN_BUFFER_INFO));
                    pBufCoord = (PCOORD) ((PCHAR) pCursor + sizeof(COORD));
                    pBufSz = (PCOORD) ((PCHAR) pBufCoord + sizeof(COORD));
                    pWriteRegion = (PSMALL_RECT) ((PCHAR) pBufSz
                                                   + sizeof(COORD));

#ifdef COMMENT
                    debug(F100,"VTNT Using pBufSz for Dimensions","",0);
                    w_vtnt = pBufSz->X;
                    h_vtnt = pBufSz->Y;
#else /* COMMENT */
#ifndef COMMENT2
                    debug(F100,"VTNT Using pScrnBufInf for Dimensions","",0);
                    w_vtnt = pScrnBufInf->dwSize.X;
                    h_vtnt = pScrnBufInf->dwSize.Y;
#else /* COMMENT2 */
                    debug(F100,"VTNT Using WriteRegion for Dimensions","",0);
                    w_vtnt = pWriteRegion->Right - pWriteRegion->Left + 1;
                    h_vtnt = pWriteRegion->Bottom - pWriteRegion->Top + 1;
#endif /* COMMENT2 */
#endif /* COMMENT */
                    vtnt_read = pBufSz->X * pBufSz->Y * sizeof(CHAR_INFO) + VTNT_MIN_READ;
                    debug(F111,"VTNT char_info bytes","vtnt_read",vtnt_read);

                    if ( pBufSz->X > pScrnBufInf->dwSize.X )
                        debug(F101,"VTNT WARNING BufSz->X > ScrnBufInf->Size.X","",
                               pBufSz->X);
                    if ( pBufSz->Y > pScrnBufInf->dwSize.Y )
                        debug(F101,"VTNT WARNING BufSz->Y > ScrnBufInf->Size.Y","",
                               pBufSz->Y);

                    debug(F101,"VTNT CONSOLE_SCREEN_BUFFER_INFO dwSize.X","",pScrnBufInf->dwSize.X);
                    debug(F101,"VTNT CONSOLE_SCREEN_BUFFER_INFO dwSize.Y","",pScrnBufInf->dwSize.Y);
                    debug(F101,"VTNT CONSOLE_SCREEN_BUFFER_INFO dwCursorPosition.X","",pScrnBufInf->dwCursorPosition.X);
                    debug(F101,"VTNT CONSOLE_SCREEN_BUFFER_INFO dwCursorPosition.Y","",pScrnBufInf->dwCursorPosition.Y);
                    debug(F101,"VTNT CONSOLE_SCREEN_BUFFER_INFO wAttributes","",pScrnBufInf->wAttributes);
                    debug(F101,"VTNT CONSOLE_SCREEN_BUFFER_INFO srWindow.Left","",pScrnBufInf->srWindow.Left);
                    debug(F101,"VTNT CONSOLE_SCREEN_BUFFER_INFO srWindow.Top","",pScrnBufInf->srWindow.Top);
                    debug(F101,"VTNT CONSOLE_SCREEN_BUFFER_INFO srWindow.Right","",pScrnBufInf->srWindow.Right);
                    debug(F101,"VTNT CONSOLE_SCREEN_BUFFER_INFO srWindow.Bottom","",pScrnBufInf->srWindow.Bottom);
                    debug(F101,"VTNT CONSOLE_SCREEN_BUFFER_INFO dwMaximumWindowSize.X","",pScrnBufInf->dwMaximumWindowSize.X);
                    debug(F101,"VTNT CONSOLE_SCREEN_BUFFER_INFO dwMaximumWindowSize.Y","",pScrnBufInf->dwMaximumWindowSize.Y);
                    debug(F101,"VTNT CURSOR X","",pCursor->X);
                    debug(F101,"VTNT CURSOR Y","",pCursor->Y);
                    debug(F101,"VTNT BUFFER COORD X","",pBufCoord->X);
                    debug(F101,"VTNT BUFFER COORD Y","",pBufCoord->Y);
                    debug(F101,"VTNT BUFFER SIZE X","",pBufSz->X);
                    debug(F101,"VTNT BUFFER SIZE Y","",pBufSz->Y);
                    debug(F101,"VTNT WRITE REGION Left","",  pWriteRegion->Left);
                    debug(F101,"VTNT WRITE REGION Top","",   pWriteRegion->Top);
                    debug(F101,"VTNT WRITE REGION Right","", pWriteRegion->Right);
                    debug(F101,"VTNT WRITE REGION Bottom","",pWriteRegion->Bottom);

#ifdef COMMENT
                    /* Compare screen dimensions and switch if they do not match */
                    if ( VscrnGetWidth(VTERM) != w_vtnt ) {
                        if ( w_vtnt > 0 && w_vtnt <= MAXTERMROW) {
                            debug(F101,"VTNT Width Changed to","",w_vtnt);
                            VscrnSetWidth(VTERM,w_vtnt);
                            tt_rows[VTERM] = w_vtnt;
#ifndef KUI
                            vt100screen.mi.col = w_vtnt;
#endif /* KUI */
                            szchng = 1;
                        }
                    }
                    if ( VscrnGetHeight(VTERM) != (h_vtnt + (tt_status[VTERM]?1:0)) ) {
                        if ( h_vtnt > 0 && h_vtnt <= MAXTERMCOL) {
                            debug(F101,"VTNT Height Changed to","",h_vtnt);
                            VscrnSetHeight(VTERM,h_vtnt + (tt_status[VTERM]?1:0));
                            tt_cols[VTERM] = h_vtnt;
#ifndef KUI
                            vt100screen.mi.row = h_vtnt + (tt_status[VTERM]?1:0);
#endif /* KUI */
                            szchng = 1;
                        }
                    }
                    if ( szchng )
                        RestoreTermMode();
#endif /* COMMENT */

                    if (vtnt_index == vtnt_read) {
                        /* In other words, the size of the char info is 0 */
                        VscrnSetCurPos(VTERM,
                                        pCursor->X,
                                        pCursor->Y);
                        VscrnIsDirty(VTERM);

                        vtnt_index = 0;
                        vtnt_read = VTNT_MIN_READ;
                        memset(vtnt_buf,0,VTNT_BUFSZ);
                    }
                }
                else if (vtnt_index == vtnt_read) {
                    /* We should now have the complete set of struct data */
                    /* now to process it  */
                    USHORT Row, Col;
                    viocell  vio={0,0};
                    vtattrib vta={0,0,0,0,0,0,0,0,0,0};

                    pScrnBufInf = (PCONSOLE_SCREEN_BUFFER_INFO) vtnt_buf;
                    pCursor = (PCOORD) ((PCHAR) pScrnBufInf
                                         + sizeof(CONSOLE_SCREEN_BUFFER_INFO));
                    pBufCoord = (PCOORD) ((PCHAR) pCursor + sizeof(COORD));
                    pBufSz = (PCOORD) ((PCHAR) pBufCoord + sizeof(COORD));
                    pWriteRegion = (PSMALL_RECT) ((PCHAR) pBufSz
                                                   + sizeof(COORD));
                    pCharInfo = (PCHAR_INFO) ((PCHAR) pWriteRegion
                                               + sizeof(SMALL_RECT));

                    ckhexdump("VTNT CharInfo",pCharInfo,vtnt_read-VTNT_MIN_READ);

                    Row = pWriteRegion->Top;
                    Col = pWriteRegion->Left;
                    while ( (PCHAR) pCharInfo < &vtnt_buf[vtnt_index] &&
                            (Row < pWriteRegion->Bottom ||
                            Row == pWriteRegion->Bottom &&
                              Col <= pWriteRegion->Right)) {
                        if ( isunicode() )
                        {
                            vio.c = pCharInfo->Char.UnicodeChar;
                        }
                        else {
                            if (GNOW->itol)
                                vio.c = (*GNOW->itol)(pCharInfo->Char.UnicodeChar);
                            else
                                vio.c = (pCharInfo->Char.UnicodeChar & 0xFF);
                        }
                        vio.a = (pCharInfo->Attributes & 0xFF);
                        VscrnWrtCell(VTERM,
                                      vio,
                                      vta,
                                      Row,
                                      Col);
                        if ( ++Col > pWriteRegion->Right ) {
                            Row++;
                            Col = pWriteRegion->Left;
                        }
                        pCharInfo++;
                    }
                    VscrnSetCurPos(VTERM,
                                    pCursor->X,
                                    pCursor->Y);
                    VscrnIsDirty(VTERM);

                    vtnt_index = 0;
                    vtnt_read = VTNT_MIN_READ;
                    memset(vtnt_buf,0,VTNT_BUFSZ);
                }
            }
        }
        return;
    }

/*
  Even if debugging, we still plow through the escape-sequence state table
  switcher, but we don't call any of the action routines.
  This lets us highlight text that appears inside an escape sequence.
*/

    if (escstate == ES_NORMAL) {        /* Not in an escape sequence */
        if (ch == ESC &&
             !literal_ch) {             /* This character is an Escape */
            escstate = ES_GOTESC;       /* Change state to GOTESC */
            esclast = 0;                /* Reset buffer pointer */
            escbuffer[0] = ESC;
        }
        else if ( ch == _CSI && !literal_ch) {
            escstate = ES_GOTCSI;       /* Escape sequence was restarted */
            escbuffer[0] = _CSI;         /* Save in case we have to replay it */
            esclast = 1;                /* Reset buffer pointer, but */
            escbuffer[1] = '[';         /* But translate for vtescape() */
        } else {                        /* Not an ESC, stay in NORMAL state */
          if (!debses)
            vt100(ch);          /* and send to vt100() for display. */
        }
        return;                 /* Return in either case. */
    }

/* We are in an escape sequence... */

    if (ch < SP || (ch == _CSI)) {                 /* Control character? */
        if ( xprint ) {                            /* During a transparent print the */
            int i;                                 /* only sequence we care about is */
            if ( !debses && escstate != ES_NORMAL ) { /* CSI 4 i.                    */
                switch ( escbuffer[0] ) {
                case ESC:
                    i = 0;                         /* ESC is beginning of C0 sequence */
                    break;
                case _CSI:
                    prtchar(_CSI);                    /* We really had a C1 sequence */
                    i = 2;
                    break;
                default:
                    prtchar(ESC);                    /* it must have been C0, and we made */
                    i = 1;                         /* a coding mistake */
                }
                for ( ; i <= esclast; i++ )        /* Print what we've got. */
                    prtchar(escbuffer[i]);
            }
            esclast  = 0;               /* Reset the buffer */
            escbuffer[0] = 0;           /* And clear the first char, just in case */
            escstate = ES_NORMAL;       /* Go back to normal */
            cwrite(ch);                 /* And then restart */
        } else if ( escstate == ES_GOTESC && ISANSI(tt_type_mode) && crm ) {
            if ( !debses )
                vt100(ch);
            escstate = ES_NORMAL;       /* Go back to normal */
        } else if (ch == CAN || ch == SUB) { /* These cancel an escape sequence  */
            /* According to the 97801-5xx manual.  SUB is different from CAN in  */
            /* that SUB outputs to the screen the characters received as part of */
            /* the escape sequence. However, the only noticeable difference on   */
            /* the VT320 is that a backwards question mark is printed whenever a */
            /* SUB is received.                                                  */
            escstate = ES_NORMAL;            /* Go back to normal. */
#ifdef CK_APC
            apclength = 0;
            apcbuf[0] = 0 ;
            apcactive = APC_INACTIVE ;
            apcrecv = FALSE ;
            dcsrecv = FALSE ;
            pmrecv  = FALSE ;
            oscrecv = FALSE ;
            pu1recv = FALSE ;
            pu2recv = FALSE ;
            c1strrecv = FALSE ;
#endif /* CK_APC */
        } else if (ch == BS) {  /* Erases previous */
            if ( escstate == ES_GOTCSI && esclast == 1 ) {
                escstate = ES_GOTESC ;
                esclast = 0 ;
            }
            else if ( escstate == ES_ESCSEQ && esclast == 1 ) {
                escstate = ES_GOTESC ;
                esclast = 0 ;
            }
            else if ( escstate == ES_GOTESC && esclast == 0 ) {
                escstate = ES_NORMAL ;
            }
            else if ( escstate == ES_TERMIN ) {
                escstate = ES_STRING ;
            }
            else if ( escstate == ES_STRING ) {
#ifdef CK_APC
                if ( apclength > 0 )
                    apclength-- ;
                else
#endif /* CK_APC */
                {
                    escstate = ES_GOTESC ;
                }
            }
            else if ( esclast > 0 ) {
                esclast--;              /* escape sequence char (really?) */
            }
        } else if (ch == ESC) {
            if ( escstate == ES_STRING )
                escstate = ES_TERMIN ;
            else {
                escstate = ES_GOTESC;   /* Escape sequence was restarted */
                esclast = 0;            /* Reset buffer pointer */
                escbuffer[0] = ESC;     /* Save in case we have to replay it */
            }
        } else if (ch == _CSI) {
            escstate = ES_GOTCSI;       /* Escape sequence was restarted */
            escbuffer[0] = _CSI;         /* Save in case we have to replay it */
            esclast = 1;                /* Reset buffer pointer, but */
            escbuffer[1] = '[';         /* But translate for vtescape() */
        } else if (ch != NUL) {
            wrtch(ch);
        }
        return;
    }
/*
  Put this character in the escape sequence buffer.
  But we don't put "strings" in this buffer; either absorb them silently
  (as with Operating System Command), or handle them specially (as with APC).
  Note that indexing starts at 1, not 0.
*/
    if ( escstate != ES_STRING && escstate != ES_TERMIN )
        if (esclast < ESCBUFLEN)
            escbuffer[++esclast] = ch;


   /* Note: vtescape() sets escstate back to ES_NORMAL. */

   if (ISVT52(tt_type_mode)) {          /* VT52 Emulation */
      switch (esclast) {
      case 1:                           /* First char after Esc */
          switch ( ch ) {
          case 'Y':
              vt52esclen = 3 ;
              break;
          case 'r':
          case 'x':
          case 'y':
              vt52esclen = 2 ;
              break;
          default:
              vtescape() ;
          }
          break;
      default:
          if ( esclast == vt52esclen )
              vtescape() ;              /* then it's a complete sequence */
          else if ( esclast > vt52esclen )
              escstate = ES_NORMAL;     /* Something unexpected, ignore */
          break;
      }
       return;                                /* Done with VT52 */
   }

   /* The rest of this routine handles ANSI/VT1xx/VT2xx/VT3xx/VT4xx/VT5xx emulation... */

    switch (escstate) {                 /* Enter esc sequence state switcher */
    case ES_GOTESC:                     /* GOTESC state, prev char was Esc */
        switch ( ch ) {
        case '[':                       /* Left bracket after ESC is CSI */
            escstate = ES_GOTCSI;       /* Change to GOTCSI state */
            break;
        case '_':                       /* Application Program Command (APC) */
            if ( !xprint ) {
                escstate = ES_STRING;   /* Enter STRING-absorption state */
                if ( !savefiletext[0] )
                    ckstrncpy(savefiletext,filetext,sizeof(savefiletext));
                ckstrncpy(filetext,"APC STRING",sizeof(filetext)) ;
                VscrnIsDirty(VTERM);    /* Update status line */
#ifdef CK_APC
                apcrecv = TRUE;         /* We are receiving an APC string */
                apclength = 0;          /* and reset APC buffer pointer */
#endif /* CK_APC */
            } else {
                vtescape();                               /* Go act on it. */
            }
        case 'P':                       /* Device Control String (DCS) Introducer */
            if ( !xprint ) {
                escstate = ES_STRING;   /* Enter STRING-absorption state */
#ifdef CK_APC
                dcsrecv = TRUE ;        /* Set DCS-Active flag */
                apclength = 0 ;         /* we use the same buffer as for APC */
#endif /* CK_APC */
                if ( !savefiletext[0] )
                    ckstrncpy(savefiletext,filetext,sizeof(savefiletext));
                ckstrncpy(filetext,"DCS STRING",sizeof(filetext)) ;
                VscrnIsDirty(VTERM);    /* Update status line */
            } else {
                vtescape();                               /* Go act on it. */
            }
            break;
        case 'Q':                       /* Private Use One (PU1) Introducer 97801-5xx */
            if ( !xprint && !ISANSI(tt_type_mode) ) {
                /* 
                 * SCOANSI used ESC Q to prefix keyboard assignments
                 * so do not enter the string state
                 */
                escstate = ES_STRING;   /* Enter STRING-absorption state */
#ifdef CK_APC
                pu1recv = TRUE ;        /* Set PU1-Active flag */
                apclength = 0 ;         /* we use the same buffer as for APC */
#endif /* CK_APC */
                if ( !savefiletext[0] )
                    ckstrncpy(savefiletext,filetext,sizeof(savefiletext));
                ckstrncpy(filetext,"PU1 STRING",sizeof(filetext)) ;
                VscrnIsDirty(VTERM);    /* Update status line */
            }
            else {
                vtescape();                               /* Go act on it. */
            }
            break;
        case 'R':                       /* Private Use Two (PU2) Introducer 97801-5xx */
            if ( !xprint ) {
                escstate = ES_STRING;   /* Enter STRING-absorption state */
#ifdef CK_APC
                pu2recv = TRUE ;        /* Set PU2-Active flag */
                apclength = 0 ;         /* we use the same buffer as for APC */
#endif /* CK_APC */
                if ( !savefiletext[0] )
                    ckstrncpy(savefiletext,filetext,sizeof(savefiletext));
                ckstrncpy(filetext,"PU2 STRING",sizeof(filetext)) ;
                VscrnIsDirty(VTERM);    /* Update status line */
            }
            else {
                vtescape();                               /* Go act on it. */
            }
            break;
        case 'X':                       /* Start-Of-String (SOS) Ignored */
            if ( !xprint ) {
                escstate = ES_STRING;   /* Enter STRING-absorption state */
#ifdef CK_APC
                c1strrecv = TRUE ;      /* Set Active flag */
                apclength = 0 ;         /* we use the same buffer as for APC */
#endif /* CK_APC */
                if ( !savefiletext[0] )
                    ckstrncpy(savefiletext,filetext,sizeof(savefiletext));
                ckstrncpy(filetext,"SOS STRING",sizeof(filetext));
                VscrnIsDirty(VTERM);    /* Update status line */
            }
            else {
                vtescape();             /* Go act on it. */
            }
            break;
        case 'Z':                       /* DECID */
            if ( ISVT52(tt_type_mode) ) {
                sendescseq(tt_info[TT_VT52].x_id);
            }
            else if ( ISVT220(tt_type) ) {
                sendescseq(tt_info[tt_type].x_id);
            }
            break;
        case '^':                       /* Privacy Message (PM) */
            if ( !xprint ) {
                escstate = ES_STRING;   /* Enter STRING-absorption state */
#ifdef CK_APC
                pmrecv = TRUE ;       /* Set PM-Active flag */
                apclength = 0 ;         /* we use the same buffer as for APC */
#endif /* CK_APC */
                if ( !savefiletext[0] )
                    ckstrncpy(savefiletext,filetext,sizeof(savefiletext));
                ckstrncpy(filetext,"PM STRING",sizeof(filetext)) ;
                VscrnIsDirty(VTERM);    /* Update status line */
                break;
            }
        case ']':                       /* Operating System Command (OSC) */
            if ( !xprint ) {
                escstate = ES_STRING;   /* Enter STRING-absorption state */
#ifdef CK_APC
                oscrecv = TRUE ;      /* Set OSC-Active flag */
                apclength = 0 ;         /* we use the same buffer as for APC */
#endif /* CK_APC */
                if ( !savefiletext[0] )
                    ckstrncpy(savefiletext,filetext,sizeof(savefiletext));
                ckstrncpy(filetext,"OSC STRING",sizeof(filetext)) ;
                VscrnIsDirty(VTERM);    /* Update status line */
                break;
            }
        default:
            if ((ch > 057) && (ch < 0177)) /* Or final char, '0' thru '~' */
                vtescape();                               /* Go act on it. */
            else
                escstate = ES_ESCSEQ;
         }
         break;

    case ES_ESCSEQ:                     /* ESCSEQ -- in an escape sequence */
        if (ch > 057 && ch < 0177)      /* Final character is '0' thru '~' */
          vtescape();                   /* Go handle it */
        break;

    case ES_GOTCSI:                     /* GOTCSI -- In a control sequence */
        if (ch > 077 && ch < 0177)      /* Final character is '@' thru '~' */
          vtescape();                   /* Go act on it. */
        break;

    case ES_STRING:                     /* Inside a string */
        if (ch == ESC)                  /* ESC may be 1st char of terminator */
          escstate = ES_TERMIN;         /* Change state to find out. */
#ifdef CK_APC
        else if ( ch == BEL && ISAIXTERM(tt_type_mode) && oscrecv ) {
            /* BEL terminates an OSC string in AIXTERM */
            escstate = ES_NORMAL;       /* If so, back to NORMAL */
            if (savefiletext[0]) {              /* Fix status line */
                ckstrncpy(filetext,savefiletext,sizeof(filetext));
                savefiletext[0] = NUL;
                VscrnIsDirty(VTERM);  /* status line needs to be updated */
            }
            apcbuf[apclength] = NUL; /* terminate it */
                                       /* process it */
            if (!debses)
                doosc() ;
            oscrecv = FALSE ;
        }
        else if (apcrecv || dcsrecv || oscrecv || pmrecv ||
                  pu1recv || pu2recv || c1strrecv) {
            if (apclength < apcbuflen)    /* If in APC string, */
              apcbuf[apclength++] = ch;   /* deposit this character */
            else {                        /* Buffer overrun */
                apcrecv = FALSE ;         /* Discard what we got */
                dcsrecv = FALSE ;
                oscrecv = FALSE ;
                pmrecv  = FALSE ;
                pu1recv = FALSE ;
                pu2recv = FALSE ;
                c1strrecv = FALSE ;
                apclength = 0;            /* and go back to normal */
                apcbuf[0] = 0;            /* Not pretty, but what else */
                escstate = ES_NORMAL ;
            }
        }
#endif /* CK_APC */
        break;                          /* Absorb all other characters. */

    case ES_TERMIN:                     /* May have a string terminator */
        if (ch == '\\') {               /* which must be backslash */
            escstate = ES_NORMAL;       /* If so, back to NORMAL */
            if (savefiletext[0]) {              /* Fix status line */
                ckstrncpy(filetext,savefiletext,sizeof(filetext));
                savefiletext[0] = NUL;
                VscrnIsDirty(VTERM);  /* status line needs to be updated */
            }
#ifdef CK_APC
            if (apcrecv && apclength > 0)/* If it was an APC string, */
            {
                apcbuf[apclength] = NUL; /* terminate it */
                if ((apcstatus & APC_ON) && !debses) {
                    apc_command(APC_REMOTE,apcbuf);
                    return;             /* with the apcactive flag still set */
                }
                else apcactive = APC_INACTIVE;
            }
            else if ( dcsrecv ) /* it was a DCS string, */
            {
                apcbuf[apclength] = NUL; /* terminate it */
                if ( tt_type_mode >= TT_VT320 && /* and if we are a VT320 */
                     tt_type_mode <= TT_WY370 )
                {                            /* process it */
                    if (!debses)
                      dodcs() ;
                }
                dcsrecv = FALSE ;
            }
            else if ( pu1recv ) /* it was a PU1 string, */
            {
                apcbuf[apclength] = NUL; /* terminate it */
                if ( IS97801(tt_type_mode) )
                {                            /* process it */
                    if (!debses)
                      dopu1() ;
                }
                pu1recv = FALSE ;
            }
            else if ( pu2recv ) /* it was a PU2 string, */
            {
                apcbuf[apclength] = NUL; /* terminate it */
                if ( IS97801(tt_type_mode) )
                {                            /* process it */
                    if (!debses)
                      dopu2() ;
                }
                pu2recv = FALSE ;
            }
            else if ( oscrecv ) /* it was a OSC string, */
            {
                apcbuf[apclength] = NUL; /* terminate it */
                if ( tt_type_mode >= TT_VT320 && /* and if we are a VT320 */
                     tt_type_mode <= TT_WY370 )
                {                            /* process it */
                    if (!debses)
                      doosc() ;
                }
                oscrecv = FALSE ;
            }
#endif /* CK_APC */
        } else {
#ifdef CK_APC
            if ( dcsrecv ) {
                dcsrecv = FALSE ;
                apcbuf[0] = NUL ;
                apclength = 0 ;
                escstate = ES_GOTESC ;
                if (savefiletext[0]) {          /* Fix status line */
                    ckstrncpy(filetext,savefiletext,sizeof(filetext));
                    savefiletext[0] = NUL;
                    VscrnIsDirty(VTERM);  /* status line needs to be updated */
                }
                cwrite(ch);
            }
            else {
#endif /* CK_APC */
                if ( ch >= SP )         /* Just a stray Esc character. */
                    escstate = ES_STRING;       /* Return to string absorption. */
#ifdef CK_APC
                if (apcrecv) {
                    if (apclength+1 < apcbuflen) { /* In APC string, */
                        apcbuf[apclength++] = ESC;   /* deposit the Esc character */
                        apcbuf[apclength++] = ch;    /* and this character too */
                    }
                }
            }
#endif /* CK_APC */
        }
    }
#endif /* NOTERM */
}

/*---------------------------------------------------------------------------*/
/* scrninit                                                                  */
/*---------------------------------------------------------------------------*/
void
scrninit() {
    if (deccolm)
      Set132Cols(VTERM) ;
    else
      SetCols(VTERM) ;

#ifdef TCPSOCKET
#ifdef CK_NAWS
   if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0)
   {
       tn_snaws();
#ifdef RLOGCODE
       rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
       ssh_snaws();
#endif /* SSHBUILTIN */
   }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */

    setborder();
    VscrnIsDirty(VTERM);
}

/*---------------------------------------------------------------------------*/
/* wrtch                                                                     */
/*---------------------------------------------------------------------------*/
void
wrtch(unsigned short ch) {
    viocell cell;
    vtattrib vta = {0,0,0,0,0,0,0,0,0,0};
    int vmode = decsasd == SASD_TERMINAL ? VTERM : VSTATUS ;
    extern int k95stdio,k95stdin,k95stdout;

    if (printon && xprint && !debses)   /* If transparent print active, */
      return;                           /*  we don't touch the screen. */

    if ( !IsConnectMode() && !interm )
        return;                         /* Don't touch screen if being */
                                        /* executed from an input cmd  */

    if ( k95stdout ) {
#ifdef putchar
#undef putchar
#endif
        putchar(ch);
        return;
    }

    prevchar = ch;                      /* save char for Repeat operations */

    if (ch >= SP || literal_ch || crm ||
         ((ISWYSE(tt_type_mode) || ISTVI(tt_type_mode)) &&
           (ch == STX || ch == ETX)))
    {                                   /* Treat as a Normal character */
        if (attrib.invisible)
          ch = SP;

        if ( isunicode() && crm && ch < SP ) {
            if ( ISANSI(tt_type_mode) )
                ch = (*xl_u[TX_IBMC0GRPH])(ch) ;
            else
                ch = (*xl_u[TX_C0PICT])(ch) ;
        }
        if ( (!tt_hidattr || ISHP(tt_type_mode)) &&
             (attrmode == ATTR_PAGE_MODE || attrmode == ATTR_LINE_MODE) )
        {
            /* Read the current attribute for wherex,wherey and */
            /* use it for the current location                  */
            vta = VscrnGetVtCharAttr( vmode,
                                      wherex[vmode]-1,
                                      wherey[vmode]-1 ) ;
            if ( !tt_hidattr )
                vta.wyseattr = FALSE ;
            cell.c = ch;
            cell.a = attribute;

            if ( (ISWYSE(tt_type_mode) ||
                   ISTVI(tt_type_mode) ||
                   ISHZL(tt_type_mode) )
                 && protect ) {
                /* If the current cursor is on a protected cell */
                /* then we must advance the cursor to the first */
                /* non-protected cell on the screen             */
                int x = wherex[vmode];
                int y = wherey[vmode];
                int width = VscrnGetWidth(vmode);

                while ( vta.unerasable ) {
                    if ( !vta.unerasable ) /* bug in MSVC 5.0 */
                        break;
                    x++;
                    if ( x > width ) {
                        y++;
                        if ( y >= marginbot ) {
                            x-- ;
                            y-- ;
                            return ;    /* Can't write this character */
                        }
                        x = 1 ;
                    }

                    lgotoxy(vmode,x,y); /* set wherex, wherey */

                    /* Retrieve the attributes of the new position */
                    vta = VscrnGetVtCharAttr( vmode,x-1,y-1 ) ;
                    if ( !tt_hidattr )
                        vta.wyseattr = FALSE ;
                }
            }

            if (insertmode) {
                VscrnScrollRt(vmode,
                               wherey[vmode] - 1,
                               wherex[vmode] - 1,
                               wherey[vmode] - 1,
                               VscrnGetWidth(vmode) - 1,
                               1, cell);
                /* VscrnScrollRt() doesn't apply the current attribute */
                VscrnWrtCell(vmode, cell, vta,
                          wherey[vmode] - 1,
                              wherex[vmode] - 1);
                if ( vta.wyseattr ) {
                    /* the attribute is only assigned at one place */
                    vta.wyseattr = FALSE;
                    VscrnWrtCell(vmode, cell, vta,
                                  wherey[vmode] - 1,
                                  wherex[vmode]);
                }
            }
            else {
                vta.unerasable = attrib.unerasable ;    /* use current write protect */

                VscrnWrtCell(vmode, cell, vta,
                          wherey[vmode] - 1,
                              wherex[vmode] - 1);
            }

            literal_ch = FALSE;
            /* don't wrap if autowrap is off */
            if ( tt_wrap || wherex[vmode] < VscrnGetWidth(vmode) ) {
                if (++wherex[vmode] > VscrnGetWidth(vmode)) {
                    if ( autoscroll && !protect || wherey[vmode] < marginbot ) {
                        wherex[vmode] = 1;
                        wrtch((char) LF);
                    }
                }
            }
        }
        else    /* We are in character attribute mode */
        {
            cell.c = ch;
            cell.a = attribute;

            if ( (ISWYSE(tt_type_mode) ||
                   ISTVI(tt_type_mode) ||
                   ISHZL(tt_type_mode) )
                 && protect ) {
                int width = VscrnGetWidth(vmode);
                /* If the current cursor is on a protected cell */
                /* then we must advance the cursor to the first */
                /* non-protected cell on the screen             */

                vta = VscrnGetVtCharAttr( vmode,
                                          wherex[vmode]-1,
                                          wherey[vmode]-1 ) ;

                while ( vta.unerasable ) {
                    if ( !vta.unerasable )  /* MSVC 5.0 bug */
                        break;
                    if ( ++wherex[vmode] > width ) {
                        if ( ++wherey[vmode] >= marginbot ) {
                            wherex[vmode]-- ;
                            wherey[vmode]-- ;
                            return ;    /* Can't write this character */
                        }
                        wherex[vmode] = 1 ;
                    }

                    lgotoxy(vmode,wherex[vmode],wherey[vmode]);
                }
            }

            if (insertmode) {
                VscrnScrollRt(vmode, wherey[vmode] - 1,
                               wherex[vmode] - 1, wherey[VTERM] - 1,
                               VscrnGetWidth(vmode) - 1, 1, cell);
                /* VscrnScrollRt() doesn't apply the current attribute */
                VscrnWrtCell(vmode, cell, attrib,
                          wherey[vmode] - 1,
                              wherex[vmode] - 1);
            }
            else {
                VscrnWrtCell(vmode, cell, attrib,
                          wherey[vmode] - 1, wherex[vmode] - 1);
            }

            literal_ch = FALSE;
            /* don't wrap if autowrap is off */
            if ( tt_wrap || wherex[vmode] < VscrnGetWidth(vmode) ) {
                if (++wherex[vmode] > VscrnGetWidth(vmode) && decsasd == SASD_TERMINAL) {
                    if ( IS97801(tt_type_mode) ) {
                        if ( !sni_pagemode ) {
                            wherex[vmode] = 1;
                            wrtch((char) LF);
                        }
                        else {  /* Page Mode */
                            lgotoxy(VTERM,1,margintop);
                        }

                    }
                    else if ( autoscroll && !protect || wherey[vmode] < marginbot ) {
                        wherex[vmode] = 1;
                        wrtch((char) LF);
                    }
                }
            }
        }

    }
    else
    {   /* Control character */
        
        switch (ch) {
        case LF:
          dolf:
            if ( printon && is_aprint() ) {
                prtline( wherey[VTERM], LF ) ;
            }
            if ( decsasd == SASD_TERMINAL ) {
                if (wherey[vmode] == marginbot) {
                    if ( IS97801(tt_type_mode) ) {
                        if ( !sni_pagemode )
                            VscrnScroll(vmode,UPWARD, margintop - 1, marginbot - 1, 1,
                                         (margintop == 1), SP ) ;
                        else /* Page Mode */
                            wherex[VTERM] = 1;
                    } else if ( autoscroll && !protect ) {
                        VscrnScroll(vmode,UPWARD, margintop - 1, marginbot - 1, 1,
                                     (margintop == 1), SP ) ;
                    } else if ( wherex[vmode] > VscrnGetWidth(vmode) )
                        wherex[vmode] = VscrnGetWidth(vmode);
                } else {
                    wherey[vmode]++;
                    if (wherey[vmode] >= VscrnGetHeight(vmode)+(tt_status[VTERM]?0:1)) {
                        if ( IS97801(tt_type_mode) ) {
                            if (wherey[vmode] == VscrnGetHeight(vmode)+(tt_status[VTERM]?0:1)
                                 && decssdt == SSDT_HOST_WRITABLE)
                                setdecsasd(SASD_STATUS);
                            wherey[vmode] = margintop;
                        } else
                            wherey[vmode] = VscrnGetHeight(vmode)+(tt_status[VTERM]?0:1)-1;
                    }
                }
            }
            break;
        case CR:
            if ( (IS97801(tt_type_mode) || ISHP(tt_type_mode)) &&
                 vmode == VTERM )
                wherex[vmode] = marginleft;
            else
                wherex[vmode] = 1;
            if ( !(ISANSI(tt_type_mode) || ISHFT(tt_type_mode)) )
                wrapit = FALSE;
            break;
        case BS:
            if ( (IS97801(tt_type_mode) || ISHP(tt_type_mode)) &&
                 vmode == VTERM ) {
                if (wherex[vmode] > marginleft)
                    wherex[vmode]--;
            }
            else if ( ISQNX(tt_type_mode) && vmode == VTERM ) {
                if ( wherex[vmode] > 1 )
                    wherex[vmode]--;
                else {
                    wherex[vmode] = VscrnGetWidth(vmode);
                    if ( wherey[vmode] > 1 ) {
                        wherey[vmode]--;
                    }
                    else {
                        wherey[vmode] = VscrnGetHeight(vmode)
                            -(tt_status[VTERM]?1:0) ;
                    }
                }
            }
            else {
                if (wherex[vmode] > 1)
                    wherex[vmode]--;
            }
            if ( !(ISANSI(tt_type_mode) || ISHFT(tt_type_mode)) )
                wrapit = FALSE;
            break;
        case FF:
            if ( printon && is_aprint() ) {
                prtline( wherey[VTERM], LF ) ;
            }
            if ( ISSUN(tt_type_mode) && vmode == VTERM) {
                clrscreen(VTERM,SP);
                lgotoxy(VTERM,1,1);       /* and home the cursor */
            } 
            else if ( (IS97801(tt_type_mode) || ISHP(tt_type_mode)) &&
                 vmode == VTERM ) {
                if (wherex[vmode] < marginright)
                    wherex[vmode]++;
            }
            else {
                goto dolf;
            }
            break;
        case BEL:
            bleep(BP_BEL);
            break;
        case NUL:                       /* represent it as a space */
            wrtch(SP);
            break;
        case VT:
            if ( printon && is_aprint() ) {
                prtline( wherey[VTERM], LF ) ;
            }
            if ( ISWYSE(tt_type_mode) ||
                 ISTVI(tt_type_mode) ||
                 ISHZL(tt_type_mode) ) 
            {
                if ( wherey[vmode] == margintop ) {
                    if ( autoscroll && !protect )
                        VscrnScroll(vmode,DOWNWARD, margintop - 1, marginbot - 1, 1,
                                     FALSE, SP ) ;
                } else {
                    cursorup(0);
                }
            } else if ( ISVT100(tt_type_mode) ) {
                goto dolf;
            }
            break;
        case XFS:
            if ( IS97801(tt_type_mode) ) {
                /* Abbreviated Cursor Position */
                /* <FS> <line + 0x20> <col + 0x20>, 0-based */
                debug(F110,"wrtch - 97801","Abbreviated Cursor Position - not implemented",0);
            }
            break;
        default:;                       /* Ignore */
        }
    }
    lgotoxy(vmode,wherex[vmode],wherey[vmode]);
    VscrnIsDirty(VTERM);  /* always mark the Terminal as requiring the update */
}

/*---------------------------------------------------------------------------*/
/* lgotoxy                                                                   */
/*---------------------------------------------------------------------------*/
void
lgotoxy(BYTE vmode, int x, int y) {
    if ( x < 1 ) x = 1;
    if ( y < 1 ) y = 1;

    if ( vmode == VTERM && decsasd == SASD_STATUS )
        vmode = VSTATUS ;

    if ( vmode == VSTATUS )
        y = 1 ;

    if ( vmode == VTERM && marginbell ) {
        position * cur = VscrnGetCurPos(VTERM) ;
        if ( cur->x+1 < marginbellcol && x >= marginbellcol )
            bleep(BP_BEL) ;
    }

    if ( vmode == VTERM &&
                (ISWYSE(tt_type_mode) ||
           ISTVI(tt_type_mode) ||
           ISHZL(tt_type_mode) )
         && protect ) {
        /* If the current cursor is on a protected cell */
        /* then we must advance the cursor to the first */
        /* non-protected cell on the screen             */
        vtattrib vta = VscrnGetVtCharAttr( VTERM, x-1, y-1 ) ;
        int width    = VscrnGetWidth(VTERM);
        while ( vta.unerasable ) {
            if ( !vta.unerasable )      /* bug in MSVC 5.0 */
                break;
            x++;
            if ( x > width ) {
                y++;
                if ( y > marginbot ) {
                    x=0 ;
                    y=margintop ;
                }
                x = 1 ;
            }
            /* Retrieve the attributes of the new position */
            vta = VscrnGetVtCharAttr( VTERM, x-1, y-1 ) ;
        }
    }

    wherex[vmode] = x;
    wherey[vmode] = y;

    wrapit = FALSE;

    if ( markmodeflag[vmode] == notmarking )
      VscrnSetCurPos( vmode, x - 1, y - 1 ) ;
#ifdef COMMENT
    debug(F111,"lgotoxy","vmode",vmode);
    debug(F111,"lgotoxy","x",x);
    debug(F111,"lgotoxy","y",y);
#endif /* COMMENT */

}

/*---------------------------------------------------------------------------*/
/* setmargins                                                                */
/*---------------------------------------------------------------------------*/
void
setmargins(int topmargin, int bottommargin) {
    margintop = topmargin;
    marginbot = bottommargin;
}

/*---------------------------------------------------------------------------*/
/* line25                                                                    */
/*---------------------------------------------------------------------------*/

#define STATUS_MAX 10
char *save_status[STATUS_MAX];
char *save_usertext[STATUS_MAX];
char *save_exittext[STATUS_MAX];
char *save_helptext[STATUS_MAX];
char *save_filetext[STATUS_MAX];
char *save_savefiletext[STATUS_MAX];
char *save_hostname[STATUS_MAX];

void
save_status_line() {
    if ((++status_saved >= (STATUS_MAX - 1)) || (status_saved < 0))
      return;

    if (!(save_status[status_saved] = malloc(MAXTERMCOL+1)))
      return;
    else
      strncpy(save_status[status_saved],statusline,MAXTERMCOL);

    if (!(save_usertext[status_saved] = malloc(MAXTERMCOL+1)))
      return;
    else
      strncpy(save_usertext[status_saved],usertext,MAXTERMCOL);

    if (!(save_exittext[status_saved] = malloc(MAXTERMCOL+1)))
      return;
    else
      strncpy(save_exittext[status_saved],exittext,MAXTERMCOL+1);

    if (!(save_helptext[status_saved] = malloc(MAXTERMCOL+1)))
      return;
    else
      strncpy(save_helptext[status_saved],helptext,MAXTERMCOL+1);

    if (!(save_filetext[status_saved] = malloc(MAXTERMCOL+1)))
      return;
    else
      strncpy(save_filetext[status_saved],filetext,MAXTERMCOL+1);

    if (!(save_savefiletext[status_saved] = malloc(MAXTERMCOL+1)))
      return;
    else
      strncpy(save_savefiletext[status_saved],savefiletext,MAXTERMCOL+1);

    if (!(save_hostname[status_saved] = malloc(MAXTERMCOL+1)))
      return;
    else
      strncpy(save_hostname[status_saved],hostname,MAXTERMCOL+1);
}

void
restore_status_line() {
    if ((status_saved >= (STATUS_MAX - 1)) || (status_saved < 0))
      return;

    if (save_status[status_saved]) {
        strncpy(statusline,save_status[status_saved],MAXTERMCOL);
        free(save_status[status_saved]);
    }

    if (save_usertext[status_saved]) {
        ckstrncpy(usertext,save_usertext[status_saved],sizeof(usertext));
        free(save_usertext[status_saved]);
    }

    if (save_exittext[status_saved]) {
        ckstrncpy(exittext,save_exittext[status_saved],sizeof(exittext));
        free(save_exittext[status_saved]);
    }

    if (save_helptext[status_saved]) {
        ckstrncpy(helptext,save_helptext[status_saved],sizeof(helptext));
        free(save_helptext[status_saved]);
    }

    if (save_filetext[status_saved]) {
        ckstrncpy(filetext,save_filetext[status_saved],sizeof(filetext));
        free(save_filetext[status_saved]);
    }

    if (save_savefiletext[status_saved]) {
        ckstrncpy(savefiletext,save_savefiletext[status_saved],sizeof(savefiletext));
        free(save_savefiletext[status_saved]);
    }

    if (save_hostname[status_saved]) {
        ckstrncpy(hostname,save_hostname[status_saved],sizeof(hostname));
        free(save_hostname[status_saved]);
    }

    status_saved--;
    VscrnIsDirty(VTERM);  /* status line needs to be updated */
}

void
strinsert(char *d, char *s) {
    while (*s)
      *d++ = *s++;
}

char *
line25(int vmode) {
    char *s = statusline, * mode = NULL;
    int  i;
    int numlines, linesleft ;
    int w = VscrnGetWidth(vmode);

    if ( scrollstatus[vmode] && !escapestatus[vmode] ) {
        /* we are in scrollback mode -- dynamicly update the status line */
        numlines = ( VscrnGetBegin(vmode) == 0 ) ?
            VscrnGetEnd(vmode) + 1 : VscrnGetBufferSize(vmode) ;

        if (tt_roll[vmode]) {
            linesleft = ( VscrnGetScrollTop(vmode) >= VscrnGetBegin(vmode) ) ?
                    VscrnGetScrollTop(vmode) - VscrnGetBegin(vmode) :
                    VscrnGetScrollTop(vmode) - VscrnGetBegin(vmode)
                        + VscrnGetBufferSize(vmode) ;
        }
        else {
            linesleft = ( VscrnGetTop(vmode) >= VscrnGetBegin(vmode) ) ?
                VscrnGetTop(vmode) - VscrnGetBegin(vmode) :
                    VscrnGetTop(vmode) - VscrnGetBegin(vmode)
                        + VscrnGetBufferSize(vmode) ;
        }

       switch (markmodeflag[vmode]) {
       case notmarking:
           mode = "SCROLLBACK" ;
           break;
       case inmarkmode:
           mode = "MARK MODE " ;
           break;
       case marking:
           mode = "MARKING   " ;
           break;
       }

        /* Do special status line. */
        switch ( vmode ) {
        case VCMD:
            sprintf(usertext, "%s Top Line is %d out of %d | Help: Alt-H", mode, linesleft+1, numlines );
            break;
        default:
            sprintf(usertext, "%s Top Line is %d out of %d ", mode, linesleft+1, numlines );
        }
    }

    switch ( vmode ) {
    case VTERM:
        /* build the status line */
        for (i = 0; i < MAXTERMCOL; i++)
            s[i] = ' ';
        if (usertext[0])
            strinsert(&s[01], usertext);    /* Leftmost item */
        if (helptext[0])
            strinsert(&s[18], helptext);
        if (exittext[0])
            strinsert(&s[32], exittext);
        i = strlen(filetext);               /* How much needed for last item */
        if (i > 0) {
            strinsert(&s[78 - i], filetext); /* Right-justify it */
            if (hostname[0]) {
                i = 31 - i;                 /* Space remaining for hostname */
                if ((int) strlen(hostname) > (i - 1)) { /* Too long? */
                    int j;
                    for (j = i; j > 0 && hostname[j] != ':'; j--) ;
                    if (j > 0) {            /* Cut off ":service" if any */
                        hostname[j] = '\0';
                    }
                    else {          /* Or else ... */
                        hostname[i - 3] = '.'; /* show ellipsis */
                        hostname[i - 2] = '.';
                        hostname[i - 1] = '.';
                        hostname[i] = '\0';
                    }
                }
            }
        }
        if (hostname[0])
            strinsert(&s[47], hostname);

#ifndef KUI
        s[0]=(vscrn[vmode].hscroll==0?0xFE:0x11);
        s[pwidth-1]=((pwidth!=w&&vscrn[vmode].hscroll<w-pwidth)?0x10:0xFE);
#endif /* KUI */
        break;

    case VCMD:
        for (i = 0; i < MAXTERMCOL; i++)
            s[i] = ' ';
        if ( scrollstatus[vmode] || escapestatus[vmode] )
            strinsert(&s[01],usertext);
        else
            strinsert(&s[01],
            "K95 Command Screen | Help: Alt-H | Terminal: CONNECT or Alt-X ");
        break;
    default:
        s = "";
    }
    return s ;
}

/* CHSTR  --  Make a printable string out of a character  */

char*
chstr(int c) {
    char s[8];
    char *cp = s;

    if (c < SP || c == DEL)
      sprintf(cp, "CTRL-%c", ctl(c));
    else
      sprintf(cp, "'%c'\n", c);
    cp = s;
    return (cp);
}

/* DOESC  --  Process an escape character argument  */

void
esc25(int h) {
    strcpy(usertext, " ESCAPE");
    if (vik.help > 255 && keyname(vik.help))
        sprintf(helptext, "Help: %s", keyname(vik.help));
    else
        strcpy(helptext, "No Help");
    strcpy(exittext, h ? "" : (esc_exit ? "Exit: c" : "Prompt: c"));
    strcpy(hostname, "" );
    strcpy(filetext, "" ) ;
    VscrnIsDirty(VTERM);  /* status line needs to be updated */
}

void
settermstatus( int y )
{
    if ( y != tt_status[VTERM] ) {
        /* might need to fixup the margins */
        if ( marginbot == VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0) )
            if ( y ) {
                marginbot-- ;
            }
            else {
                marginbot++ ;
            }
        tt_status[VTERM] = y;
        if ( y ){
            tt_szchng[VTERM] = 2 ;
            tt_rows[VTERM]--;
            VscrnInit( VTERM ) ;  /* Height set here */
#ifdef TCPSOCKET
#ifdef CK_NAWS
            if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0) {
                tn_snaws();
#ifdef RLOGCODE
                rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                ssh_snaws();
#endif /* SSHBUILTIN */
            }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */
        }
        else {
            tt_szchng[VTERM] = 1 ;
            tt_rows[VTERM]++;
            VscrnInit( VTERM ) ;  /* Height set here */
#ifdef TCPSOCKET
#ifdef CK_NAWS
            if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0){
                tn_snaws();
#ifdef RLOGCODE
                rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                ssh_snaws();
#endif /* SSHBUILTIN */
            }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */
        }
    }
}

/* ttmacro - allow users to specify a particular macro to be executed when */
/*           a terminal type is chosen.                                    */
void
ttmacro( int tt )
{
    extern int maclvl;                  /* Macro invocation level */
    extern CHAR sstate ;
#ifdef DCMDBUF
    extern struct cmdptr *cmdstk;
#else
    extern struct cmdptr cmdstk[];
#endif /* DCMDBUF */

    extern struct keytab ttyptab[] ;
    extern int nttyp ;
    char macroname[24] ;
    int i, m, l, z ;
    int szo = sizeof(struct keytab) ;
    int term_io_sav = term_io;
    term_io = 0;                        /* Disable Terminal Emulator I/O */

    /* Compute the macroname */
    for ( i=0;i<nttyp;i++ )
        if ( tt == ttyptab[i].kwval )
            break;
    sprintf( macroname, "tt_%s", ttyptab[i].kwd ) ;

    /* Lookup the macroname */

    l = maclvl ;
    m = mxlook(mactab, macroname, nmac);
    if (m > -1) {
        debug(F111,"zzstring mxlook",macroname,m);
        if ( IsConnectMode() ) {
            apc_command(APC_LOCAL,macroname);
        } else {
            if ((z = dodo(m,NULL,cmdstk[cmdlvl].ccflgs)) > 0) {
                while (maclvl > l) {                /* Keep going till done with it, */
                    debug(F101,"ttmacro loop maclvl 1","",maclvl);
                    sstate = (CHAR) parser(1);      /* parsing & executing each command, */
                    debug(F101,"ttmacro loop maclvl 2","",maclvl);
                    if (sstate) proto();    /* including protocol commands. */
                }
                debug(F101,"ttmacro loop exit maclvl","",maclvl);
            }
        }
    }
    term_io = term_io_sav;
}

void
settermtype( int x, int prompts )
{
    static int savresetcol = 0;
    static int savtcsr = -1, savtcsl = -1, savfcs = -1, savcp = -1;
    static int savcolor = 0;            /* Terminal color */
    static int savgrcol = 0;            /* Graphics color */
    static int savulcol = 0;            /* Underline color */
    static int savulatt = 0;            /* Underline attribute */
    static int savrvatt = 0;            /* Reverse attribute */
    static int savblatt = 0;            /* Blink attribute */
    static int savcmask = 0;            /* For saving terminal bytesize */
    static int savedGset[VNUM] = {FALSE,FALSE,FALSE,FALSE};
#ifndef KUI
    static int savstatus = TRUE ;
#else
    static int savstatus = FALSE ;
#endif
    extern int fcharset, pflag, initvik, SysInited ;
    int i,y ;

    tt_type_mode = tt_type = x;

    if (savcolor) {             /* Restore this stuff if we */
        colorreset = savresetcol ;
        colornormal = savcolor; /* were ANSI before... */
        colorgraphic = savgrcol;
        colorunderline = savulcol;

        trueblink     = savblatt ;
        truereverse   = savrvatt ;
        trueunderline = savulatt ;

        savcolor = 0;
        savgrcol = 0 ;
        savulcol = 0 ;
        scrninitialized[VTERM] = 0;
        tt_status_usr[VTERM] = savstatus ;
        settermstatus(tt_status_usr[VTERM]) ;
    }

    if (savcmask) {             /* Restore terminal bytesize */
        cmask = savcmask;
        savcmask = 0;
    }
    if (savtcsl > -1) {         /* Restore character sets */
        tcsl = savtcsl;
        tcsr = savtcsr;
        fcharset = savfcs;
        savtcsl = -1;
    }
    if ( savedGset[VTERM] ) {
        for ( i = 0 ; i < 4 ; i++ )
            G[i] = savedG[VTERM][i] ;
        savedGset[VTERM] = FALSE ;
    }
#ifdef COMMENT
    if (savcp > 0) {            /* Restore code page */
        os2setcp(savcp);
        savcp = -1;
    }
#endif /* COMMENT */

    if (ISANSI(tt_type) || ISLINUX(tt_type)) {
        if (parity && prompts) {
 printf("WARNING, ANSI terminal emulation works right only if PARITY is NONE.\n");
 printf("HELP SET PARITY for further information.\n");
        }
        if ( !savcolor ) {
            savcolor = colornormal;     /* Save coloration */
            savgrcol = colorgraphic ;
            savulcol = colorunderline ;

            savulatt = trueunderline ;
            savblatt = trueblink ;
            savrvatt = truereverse ;

            colornormal = 0x07;         /* Light gray on black */
            colorgraphic = 0x07;        /* Light gray on black */
            colorunderline = 0x47;      /* Light gray on Red */

#ifndef KUI
            trueunderline = FALSE ;     /* Simulate underline */
#endif /* KUI */

            savresetcol = colorreset;     /* Save Reset Color mode */

            if (ISLINUX(tt_type) )
                colorreset = FALSE ;
            else
                colorreset = TRUE ;     /* Turn Reset color mode on */
            scrninitialized[VTERM] = 0; /* To make it take effect */

            savstatus = tt_status_usr[VTERM] ;
            if ( ISUNIXCON(tt_type) ) {
                tt_status_usr[VTERM] = FALSE ;
                settermstatus( tt_status_usr[VTERM] ) ;
            }

            VscrnInit(VTERM);           /* Reinit the screen buffer */

            savcmask = cmask;           /* Go to 8 bits */
            cmask = 0xFF;

            savtcsl = tcsl;             /* Save terminal charset */
            savtcsr = tcsr;
            savfcs  = fcharset;

            if ( ISLINUX(tt_type) ) {
                G[0].def_designation = G[0].designation = TX_ASCII;
                G[0].init = TRUE;
                G[0].def_c1 = G[0].c1 = FALSE;
                G[0].size = cs94;
                G[0].national = FALSE;

                G[1].def_designation = G[1].designation = TX_8859_1;
                G[1].init = TRUE;
                G[1].def_c1 = G[1].c1 = FALSE;
                G[1].size = cs94;
                G[1].national = FALSE;

                G[2].def_designation = G[2].designation = TX_8859_1;
                G[2].init = TRUE;
                G[2].def_c1 = G[2].c1 = FALSE;
                G[2].size = cs94;
                G[2].national = FALSE;

                G[3].def_designation = G[3].designation = TX_DECSPEC;
                G[3].init = TRUE;
                G[3].def_c1 = G[3].c1 = FALSE;
                G[3].size = cs94;
                G[3].national = FALSE;
            } else {
                tcsr = tcsl = TX_ASCII;     /* Make them both the same */
                y = os2getcp();             /* Default is current code page */
                switch (y) {
#ifdef COMMENT
                    /* These do not support box drawing characters */
                case 1250: setremcharset(TX_CP1250,4); break;
                case 1251: setremcharset(TX_CP1251,4); break;
                case 1252: setremcharset(TX_CP1252,4); break;
                case 1253: setremcharset(TX_CP1253,4); break;
                case 1254: setremcharset(TX_CP1254,4); break;
                case 1257: setremcharset(TX_CP1257,4); break;
#endif /* COMMENT */
                case 850:  setremcharset(TX_CP850,4); break;
                case 852:  setremcharset(TX_CP852,4); break;
                case 857:  setremcharset(TX_CP857,4); break;
                case 862:  setremcharset(TX_CP862,4); break;
                case 866:  setremcharset(TX_CP866,4); break;
                case 869:  setremcharset(TX_CP869,4); break;
                case 437: 
                default:   setremcharset(TX_CP437,4); break;
                }
#ifdef COMMENT
                /* These should no longer be necessary with the above */
                if (!cs_is_nrc(tcsl)) {
                    G[0].def_designation = G[0].designation = TX_ASCII;
                    G[0].init = TRUE;
                    G[0].def_c1 = G[0].c1 = FALSE;
                    G[0].size = cs94;
                    G[0].national = FALSE;
                }
                for (i = cs_is_nrc(tcsl) ? 0 : 1; i < 4; i++) {
                    G[i].def_designation = G[i].designation = tcsl;
                    G[i].init = TRUE;
                    G[i].def_c1 = G[i].c1 = FALSE;
                    switch (cs_size(G[i].designation)) { /* 94, 96, or 128 */
                    case 128:
                    case 96:
                        G[i].size = G[i].def_size = cs96;
                        break;
                    case 94:
                        G[i].size = G[i].def_size = cs94;
                        break;
                    default:
                        G[i].size = G[i].def_size = csmb;
                        break;
                    }
                    G[i].national = cs_is_nrc(tcsl);
                }
#endif /* COMMENT */
            }
        }
    }
    else if ( ISAAA(tt_type) ) {
        savtcsl = tcsl;         /* Save terminal charset */
        savtcsr = tcsr;
        savfcs  = fcharset;

        /* Default Character-set is ASCII */
        for ( i = 0 ; i < 4 ; i++ ) {
            savedG[VTERM][i] = G[i] ;
        }
        savedGset[VTERM] = TRUE ;

        G[0].def_designation = G[0].designation = TX_ASCII;
        G[0].init = TRUE;
        G[0].def_c1 = G[0].c1 = FALSE;
        G[0].size = cs94;
        G[0].national = TRUE;

        G[1].def_designation = G[1].designation = TX_ASCII;
        G[1].init = TRUE;
        G[1].def_c1 = G[1].c1 = FALSE;
        G[1].size = cs94;
        G[1].national = TRUE;

        G[2].def_designation = G[2].designation = TX_ASCII;
        G[2].init = TRUE;
        G[2].def_c1 = G[2].c1 = FALSE;
        G[2].size = cs94;
        G[2].national = TRUE;

        G[3].def_designation = G[3].designation = TX_ASCII;
        G[3].init = TRUE;
        G[3].def_c1 = G[3].c1 = FALSE;
        G[3].size = cs94;
        G[3].national = TRUE;
    } 
    else if ( ISHP(tt_type) ) {
        savtcsl = tcsl;         /* Save terminal charset */
        savtcsr = tcsr;
        savfcs  = fcharset;

        /* Default Character-set is HP-ROMAN8 */
        G[0].designation = G[0].def_designation = TX_ASCII ;
        G[0].size = G[0].def_size = cs94 ;
        G[0].c1   = G[0].def_c1 = FALSE ;
        G[0].national = CSisNRC(G[0].designation) ;
        G[0].init = TRUE ;
        for ( i=1; i<4; i++ ) {
            G[i].designation = G[i].def_designation =
                (i == 1) ? TX_HPR8 : TX_HPLINE;
            G[i].size = G[i].def_size = cs96 ;
            G[i].c1   = G[i].def_c1 = TRUE ;
            G[i].init = TRUE ;
        }
        savedGset[VTERM] = TRUE ;
    }
    else if ( ISQNX(tt_type) ) {
        savcmask = cmask;               /* Go to 8 bits */
        cmask = 0xFF;

        savtcsl = tcsl;         /* Save terminal charset */
        savtcsr = tcsr;
        savfcs  = fcharset;

        /* Default Character-set is CP437 */
        for ( i = 0 ; i < 4 ; i++ ) {
            savedG[VTERM][i] = G[i] ;
        }
            G[0].designation = G[0].def_designation = TX_ASCII ;
            G[0].size = G[0].def_size = cs94 ;
            G[0].c1   = G[0].def_c1 = FALSE ;
            G[0].national = CSisNRC(G[0].designation) ;
            G[0].init = TRUE ;
            for ( i=1; i<4; i++ ) {
                G[i].designation = G[i].def_designation =
                    TX_CP437;
                G[i].size = G[i].def_size = cs96 ;
                G[i].c1   = G[i].def_c1 = TRUE ;
                G[i].init = TRUE ;
            }
            savedGset[VTERM] = TRUE ;
    }
    else if ( ISQANSI(tt_type) ) {
        if ( !savcolor ) {
            savcolor = colornormal;     /* Save coloration */
            savgrcol = colorgraphic ;
            savulcol = colorunderline ;

            savulatt = trueunderline ;
            savblatt = trueblink ;
            savrvatt = truereverse ;

            colornormal = 0x07;         /* Light gray on black */
            colorgraphic = 0x07;        /* Light gray on black */
            colorunderline = 0x47;      /* Light gray on Red */

#ifndef KUI
            trueunderline = FALSE ;     /* Simulate underline */
#endif /* KUI */

            savresetcol = colorreset;     /* Save Reset Color mode */

            colorreset = FALSE ;
            scrninitialized[VTERM] = 0; /* To make it take effect */

            savstatus = tt_status_usr[VTERM] ;
        }

        savcmask = cmask;               /* Go to 8 bits */
        cmask = 0xFF;

        savtcsl = tcsl;         /* Save terminal charset */
        savtcsr = tcsr;
        savfcs  = fcharset;

        /* Default Character-set is ASCII/CP437 */
        for ( i = 0 ; i < 4 ; i++ ) {
            savedG[VTERM][i] = G[i] ;
        }
        /* Set G0 */
        G[0].designation = G[0].def_designation = TX_ASCII ;
        G[0].size = G[0].def_size = cs94 ;
        G[0].c1   = G[0].def_c1 = FALSE ;
        G[0].national = CSisNRC(G[0].designation) ;
        G[0].init = TRUE ;

        /* Set G1 */
        G[1].designation = G[1].def_designation = TX_DECSPEC ;
        G[1].size = G[1].def_size = cs94 ;
        G[1].c1   = G[1].def_c1 = FALSE ;
        G[1].national = CSisNRC(G[1].designation) ;
        G[1].init = TRUE ;

        /* Set G2 */
        G[2].designation = G[2].def_designation = TX_8859_2 ;
        G[2].size = G[2].def_size = cs96 ;
        G[2].c1   = G[2].def_c1 = FALSE ;
        G[2].national = CSisNRC(G[2].designation) ;
        G[2].init = TRUE ;

        /* Set G3 */
        G[3].designation = G[3].def_designation = TX_DECSPEC ;
        G[3].size = G[3].def_size = cs94 ;
        G[3].c1   = G[3].def_c1 = FALSE ;
        G[3].national = CSisNRC(G[3].designation) ;
        G[3].init = TRUE ;
        savedGset[VTERM] = TRUE ;
    }

    GNOW = GL = &G[0] ;
    GR = ISLINUX(tt_type_mode) ? &G[1] : ISVT220(tt_type_mode) ? &G[2] : &G[1];
    SSGL = NULL ;

    if ( ISQNX(tt_type) ) {
        user_erasemode = TRUE;
    }
    else {
        user_erasemode = FALSE;
    }

    if ( IS97801(tt_type_mode) ) {
        dec_lang = VTL_GERMAN;
        dec_nrc = TX_GERMAN;
        dec_kbd = TX_8859_1;
        SNI_bitmode((cmask == 0377) ? 8 : 7);
    }

    if ( ISHFT(tt_type) || ISSCO(tt_type) )
        colorreset = FALSE;
    else
        colorreset = TRUE;

    updanswerbk() ;

    VscrnInit(VTERM);
    initvik = TRUE;     /* Tell doreset() to initialize the vik table */
    doreset(1);         /* Clear screen and home the cursor */

    ttmacro(x) ;        /* Execute any user defined terminal type macros */

#ifdef KUI
    KuiSetProperty( KUI_TERM_TYPE, (long) tt_type, (long) 0 ) ;
#endif /* KUI */
    ipadl25() ;
    VscrnIsDirty(VTERM);
    msleep(10);
}

unsigned char
ComputeColorFromAttr( int mode, unsigned char colorattr, USHORT vtattr )
{
    static unsigned char colorval= 0x00;
    static unsigned char _colorattr=0x00;
    static USHORT _vtattr=0x00;

    if ( _colorattr == colorattr && vtattr == _vtattr )
        goto done;

    colorval = _colorattr = colorattr;
    _vtattr = vtattr;

    if (vtattr == VT_CHAR_ATTR_NORMAL)
        goto done;

    if (!(vtattr & WY_CHAR_ATTR) || tt_hidattr)
    {
        if ( (ISWYSE(tt_type_mode) ||
              ISTVI(tt_type_mode) ||
              ISHZL(tt_type_mode))
            && (vtattr & VT_CHAR_ATTR_PROTECTED) ) {
            vtattr = VT_CHAR_ATTR_NORMAL |
           (WPattrib.bold       ? VT_CHAR_ATTR_BOLD      : 0) |
           (WPattrib.dim        ? VT_CHAR_ATTR_DIM       : 0) |
           (WPattrib.underlined ? VT_CHAR_ATTR_UNDERLINE : 0) |
           (WPattrib.blinking   ? VT_CHAR_ATTR_BLINK     : 0) |
           (WPattrib.italic     ? VT_CHAR_ATTR_ITALIC    : 0) |
           (WPattrib.reversed   ? VT_CHAR_ATTR_REVERSE   : 0) |
           (WPattrib.invisible  ? VT_CHAR_ATTR_INVISIBLE : 0) |
           (WPattrib.unerasable ? VT_CHAR_ATTR_PROTECTED : 0) |
           (WPattrib.graphic    ? VT_CHAR_ATTR_GRAPHIC   : 0) |
           (WPattrib.hyperlink  ? VT_CHAR_ATTR_HYPERLINK : 0) |
           (WPattrib.wyseattr   ? WY_CHAR_ATTR           : 0) ;
        }

        if (vtattr & VT_CHAR_ATTR_HYPERLINK)
            vtattr |= tt_url_hilite_attr;

        if ((vtattr & VT_CHAR_ATTR_UNDERLINE) &&
            !trueunderline /* underline simulated by color */ )
            colorval = underlineattribute ;
        else if ((vtattr & VT_CHAR_ATTR_REVERSE) &&
                 !truereverse /* reverse simulated by color */ )
            colorval = reverseattribute ;
        else if ((vtattr & VT_CHAR_ATTR_ITALIC) &&
                 !trueitalic /* italic simulated by color */ )
            colorval = italicattribute;
        else if ((vtattr & VT_CHAR_ATTR_GRAPHIC))
            /* a graphic character */
            colorval = graphicattribute ;
        else
            colorval = colorattr ;


        if ((vtattr & VT_CHAR_ATTR_BLINK) &&
            !trueblink /* blink simulated by BGI */
#ifndef KUI
            || (vtattr & VT_CHAR_ATTR_UNDERLINE) &&
            trueunderline /* underline simulated by BGI */
#endif /* KUI */
             )
        {
            if (decscnm) {
                if ( fgi )
                    colorval &= 0xF7 ;  /* Toggle FGI */
                else
                    colorval |= 0x08 ;
            } else {
                if (bgi)
                    colorval &= 0x7F ;  /* Toggle BGI */
                else
                    colorval |= 0x80 ;
            }
        }

        if ( vtattr & VT_CHAR_ATTR_BOLD ||
             ( vtattr & VT_CHAR_ATTR_DIM 
#ifdef KUI
               && !truedim
#endif /* KUI */
               )
             ) {
            if (decscnm) {
                if (bgi)
                    colorval &= 0x7F ;  /* Toggle BGI */
                else
                    colorval |= 0x80 ;
            } else {
                if ( fgi )
                    colorval &= 0xF7 ;  /* Toggle FGI */
                else
                    colorval |= 0x08 ;
            }
        }

        if ( vtattr & VT_CHAR_ATTR_REVERSE &&
            truereverse /* not being simulated */ )
            colorval = byteswapcolors(colorval);

        if ( vtattr & VT_CHAR_ATTR_INVISIBLE )
            colorval = (colorval&0xF0)|((colorval&0xF0)>>4) ;
    }

  done:
    if ( flipscrnflag[mode] )
        return byteswapcolors(colorval);
    else
        return colorval;
}

#ifndef NOTERM
/*  V T E S C A P E  --  Interpret a VT52/100/102/220/320 escape sequence  */
/*
  Called after an escape sequence has been received and collected
  into escbuffer[]. To be added:

  Soft character sets (DRCS)
  Request Presentation State Report (DECRQPSR)
  Cursor Information Report (DECCIR)
  Tab Stop Report (DECTABSR)
  Restore Presentation State (DECRSPS)
  Restore Terminal State (DECRSTS)
  many more VT320 specific stuff
*/

static int   pn[11]={0,0,0,0,0,0,0,0,0,0,0};
static bool  private=FALSE;
static bool  ansiext=FALSE;
static bool  zdsext=FALSE;
static bool  kermext=FALSE;
static vtattrib blankattrib={0,0,0,0,0,0,0,0,0,0};

void
vtcsi(void)
{
    unsigned short  j;
    unsigned short  k;
    unsigned short  l;
    unsigned char   blankcell[2];
    viocell         blankvcell ;
    int             i;
    char            tempstr[20];
    int             fore, back;
    int             tcs ;
    unsigned char   des ;

    if ( ISH19(tt_type_mode) ) {
        /* Hold Screen Mode On */
        /* They must have been drunk when they did this one */
        ;
    }
    else
    {
        achar = (escnext<=esclast)?escbuffer[escnext++]:0;
     LB2000:
        switch (achar) {        /* Second level */
        case 'A':               /* Cursor up one line */
            if ( IS97801(tt_type_mode) ) {
                /* ignored if outside scroll region */
                if ( wherey[VTERM] < margintop ||
                    wherey[VTERM] > marginbot )
                    break;
            }
            cursorup(0);
            if ( !ISANSI(tt_type_mode) || ISLINUX(tt_type_mode))
                wrapit = FALSE;
            break;
        case 'B':               /* Cursor down one line */
            if ( IS97801(tt_type_mode) ) {
                /* ignored if outside scroll region */
                if ( wherey[VTERM] < margintop ||
                    wherey[VTERM] > marginbot )
                    break;
            }
            cursordown(0);
            if ( !ISANSI(tt_type_mode) || ISLINUX(tt_type_mode) )
                wrapit = FALSE;
            break;
        case 'C':               /* Cursor forward, stay on same line */
            cursorright(0);
            break;
        case 'D':               /* Cursor back, stay on same line */
            cursorleft(0);
            break;
        case 'E':
            if ( ansiext && ISBA80(tt_type_mode) ) {
                /* Window Status Request */
                char buf[64];
                sprintf(buf,"%c[=D%3d:%3d;001:001;Lohn    D",
                         27,VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0),
                         VscrnGetWidth(VTERM));
                ttol(buf,strlen(buf));

            }
            else if ( private && ISAIXTERM(tt_type_mode) ) {
                /* Erase Status Line */
                clrscreen(VSTATUS,SP);
                break;
            }
            else {
                /* Cursor Next Line */
                cursornextline();
            }
            break;
        case 'F':
            if ( private && ISAIXTERM(tt_type_mode) ) {
                /* Return from Status Line */
                setdecssdt(SASD_TERMINAL);
                break;
            }
            else {
                /* Cursor Previous Line */
                cursorprevline();
            }
            break;
        case 'J': /* Erase from cursor to end of scrn */
            if (private)
                selclreoscr_escape(VTERM,SP);
            else {
                if ( IS97801(tt_type_mode) ) {
                    /* ignored if outside scroll region */
                    if ( wherey[VTERM] < margintop ||
                        wherey[VTERM] > marginbot )
                        break;
                }
                clreoscr_escape(VTERM,SP);
            }
            break;
        case 'V': /* Erase from cursor to end of region */
            if ( ISSCO(tt_type_mode) )
                clreoreg_escape(VTERM,SP);
            break;
        case 'K':
            if ( ansiext && ISBA80(tt_type_mode) ) {
                /* Begin Reading Function Labels */
                ba80_fkey_read = -1;
            }
                /* Erase from cursor to end of line */
            else if ( private == TRUE )
                selclrtoeoln(VTERM,SP);
            else
                clrtoeoln(VTERM,SP);
            break;
        case '?':               /* DEC private */
            private = TRUE;
            achar = (escnext<=esclast)?escbuffer[escnext++]:0;
            goto LB2000;
        case '=':               /* SCO,Unixware,ANSI,Nixdorf ext. */
            ansiext = TRUE ;
            achar = (escnext<=esclast)?escbuffer[escnext++]:0;
            goto LB2000;
        case '>':               /* Heath/Zenith/AnnArbor extension */
            zdsext = TRUE ;
            achar = (escnext<=esclast)?escbuffer[escnext++]:0;
            goto LB2000;
        case '<':               /* Kermit extension */
            kermext = TRUE ;
            achar = (escnext<=esclast)?escbuffer[escnext++]:0;
            goto LB2000;
        case 'H':
            if ( private && ISHFT(tt_type_mode) ) {
                /* Hide Status Line */
                setdecssdt( SSDT_INDICATOR );
                break;
            }
            /* (no break) Cursor Home */
        case 'f':
            if ( IS97801(tt_type_mode) && decsasd == SASD_STATUS )
                setdecsasd(SASD_TERMINAL);
            lgotoxy(VTERM, 1, relcursor ? margintop : 1);
            break;
        case 'g':
            if ( !ISSCO(tt_type_mode) ) {
                /* Tab Clear at this position */
                htab[wherex[VTERM]] = '0';
            }
            break;
        case 'm':       /* Normal Video - Exit all attribute modes */
            if (colorreset)
                resetcolors(0) ;

            attrib.blinking = FALSE;
            attrib.italic = FALSE;              /* No italic */
            attrib.bold = FALSE;
            attrib.invisible = FALSE;
            attrib.underlined = FALSE;
            attrib.reversed = FALSE;
            attrib.dim = FALSE ;
            attrib.graphic = FALSE ;
            attrib.wyseattr = FALSE ;
            attrib.hyperlink = FALSE;
            attrib.linkid = 0;

            sco8bit = FALSE ;

            if ( ISANSI(tt_type_mode) )
                crm = FALSE ;
            break;
        case 'M':
            if ( ansiext && ISBA80(tt_type_mode) ) {
                /* Goto Status Line */
                setdecssdt( SSDT_HOST_WRITABLE );
                setdecsasd( SASD_STATUS );
                break;
            }
            else {
                pn[1] = 1;
                k = 1;
                goto LB2003;
            }
            break;
        case 'r': 
#ifdef COMMENT
            /* Reset Margin */
            setmargins(1, VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0));
            if ( decsasd == SASD_STATUS )
                lgotoxy( VSTATUS, 1, 1 );
            else
                lgotoxy(VTERM, 1, 1);
#else /* COMMENT */
            if ( ISSUN(tt_type_mode) ) {
                pn[1] = 0;
                k = 1;
            } else {
                pn[1] = pn[2] = 1;
                k = 2;
            }
            goto LB2003;
#endif /* COMMENT */
            break;
        case 's': 
            if ( ISSUN(tt_type_mode) ) {
                doreset(1);
            } else 
            /* ANSI.SYS save cursor position */
            if ( ISANSI(tt_type_mode) ||
                IS97801(tt_type_mode))
                savecurpos(VTERM,0);
            break;
        case 'u': /* ANSI.SYS restore cursor position */
            if ( ISANSI(tt_type_mode) ||
                IS97801(tt_type_mode))
                restorecurpos(VTERM,0);
            break;
        case 'U': /* SCO ANSI Reset Initial Screen */
            if ( ISSCO(tt_type_mode) )
                doreset(1);   /* Hard Reset */
            break;
        case '!':
            achar = (escnext<=esclast)?escbuffer[escnext++]:0;
            switch (achar) {
            case 'p':
                doreset(0);   /* DECSTR - Soft Reset */
                break;
            }
            break;
        case '$':
            achar = (escnext<=esclast)?escbuffer[escnext++]:0;
            switch (achar) {
            case '}':
                /* DECSASD - Select Active Status Display */
                setdecsasd( SASD_TERMINAL );
                break;
            case '-':   /* VT320 */
            case '~':   /* WY370 */
                /* DECSSDT - Select Status Line Type */
                setdecssdt( SSDT_BLANK );
                break;
            }
            break;
        case 'S':
            if ( private && ISAIXTERM(tt_type_mode) ) {
                /* Show Status Line */
                setdecssdt( SSDT_HOST_WRITABLE );
                break;
            } else if ( ansiext && ISBA80(tt_type_mode) ) {
                /* Return from Status Line */
                setdecsasd(SASD_TERMINAL);
                break;
            }
            else {
                pn[1] = 1;
                k = 1;
                goto LB2003;
            }
            break;
        case 'a':
        case 'e':
        case 'T':
        case 'L':
        case '@':
        case 'P':
		case 'Z':
        case SP:
            pn[1] = 1;
            k = 1;
            goto LB2003;
        case 'c':
        case 'h':
        case 'l':
        case 'n':
        case 'x':
            pn[1] = 0;
            k = 1;
            goto LB2003;
        case ';':               /* As in ESC [ ; 7 m */
            pn[1] = 0;
            k = 1;
            goto LB2002;
        default:  {             /* Pn - got a number */
          LB2001:
            /* Esc [ Pn...Pn x   functions */
            pn[1] = pnumber(&achar);
            k = 1;
          LB2002:
            while (achar == ';') { /* get Pn[k] */
                achar = (escnext<=esclast)?
                    escbuffer[escnext++]:0;
                k++;
                /* If there is a '?' at this point it is a protocol */
                /* error.  We will skip over it since this appears  */
                /* to be a frequent mistake that people make when   */
                /* combining private parameters.                    */
                if (achar == '?')
                    achar = (escnext<=esclast)?
                        escbuffer[escnext++]:0;
                pn[k] = pnumber(&achar);
            }
            pn[k + 1] = 1;
          LB2003:
            switch (achar) { /* Third level */
            case '"':
                achar = (escnext<=esclast)?
                    escbuffer[escnext++]:0;
                switch (achar) {
                case 'p': /* COMPATIBILITY LEVEL (DECSCL) */
                    setdecsasd(SASD_TERMINAL);
                    switch ( pn[1] ) {
                    case 61:
                        switch ( tt_type ) {
                        case TT_VT100:
                            tt_type_mode = TT_VT100 ;
                            break;
                        case TT_VT102:
                        case TT_VT220:
                        case TT_VT220PC:
                        case TT_VT320:
                        case TT_VT320PC:
                        case TT_WY370:
                            tt_type_mode = TT_VT102 ;
                            break;
                        }
                        break;
                    case 62:
                        switch ( tt_type ) {
                        case TT_VT220:
                        case TT_VT220PC:
                        case TT_VT320:
                        case TT_VT320PC:
                        case TT_WY370:
                            tt_type_mode = tt_type;
                            break;
                        }
                        if ( k > 1 && pn[2] == 1 )
                            send_c1 = FALSE ; /* 7-bit cntrols */
                        else
                            send_c1 = TRUE ; /* 8-bit controls */
                        break;
                    case 63:
                        switch (tt_type) {
                        case TT_VT320:
                        case TT_VT320PC:
                        case TT_WY370:
                            tt_type_mode = tt_type ;
                            if ( k > 1 && pn[2] == 1 )
                                send_c1 = FALSE ; /* 7-bit */
                            else
                                send_c1 = TRUE ; /* 8-bit */
                        }
                        break;
                    }
                    ipadl25();
                    break;
                case 'q':
                    /* DECSCA - Select Character Attributes */
                    if ( ISVT220(tt_type_mode) ) {
                        switch (pn[1]) {
                        case 0:  /* Reset all attributes */
                        case 2:  /* Designate character erasable */
                            attrib.unerasable = FALSE ;
                            break;
                        case 1:  /* Designate character unerasable */
                            attrib.unerasable = TRUE ;
                            break;
                        default:
                            break;
                        }
                    }
                    break;
                case 'v':
                    if ( ISVT220(tt_type_mode) ) {
                        /* DECRQDE - Windows Reports */
                        char decrpde[32] ;
                        sprintf(decrpde,"[%d;%d;%d;%d;%d\"w",
                                 VscrnGetHeight(VTERM),
                                 VscrnGetWidth(VTERM),
                                 1,
                                 1,
                                 1);
                        sendescseq(decrpde);
                    }
                    break;
                default:
                    break;
                }
                break;
            case '$':
                achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                switch (achar) {
                case '}':
                    /* DECSASD - Select Active Status Display */
                    setdecsasd( pn[1] );
                    break;
                case '-': /* DEC VT320 */
                case '~': /* WYSE 370  */
                    /* DECSSDT - Select Status Line Type */
                    setdecssdt( pn[1] );
                    break;
                case '|':
                    /* DECSCPP - Set Columns Per Page */
                    tt_cols[VTERM] = pn[1];
                    VscrnSetWidth( VTERM, pn[1]);
#ifdef TCPSOCKET
                    if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0) {
                        tn_snaws();
#ifdef RLOGCODE
                        rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                        ssh_snaws();
#endif /* SSHBUILTIN */
                    }
#endif /* TCPSOCKET */
                    break;
                case 'p': {     /* DECRQM (from host) */
                    char buf[16] ;
                    if ( private ) {
                        switch ( pn[1] ) {
                        case 1: /* DECCKM */
                            pn[2] = (tt_arrow == TTK_APPL) ? 1 : 2 ;
                            break;
                        case 2: /* DECANM */
                            pn[2] = (tt_type_mode == tt_type_vt52) ?
                                1 : 2 ;
                            break;
                        case 3: /* DECCOLM */
                            pn[2] = deccolm ? 1 : 2 ;
                            break;
                        case 4: /* DECSCLM */
                            pn[2] = (updmode == TTU_SMOOTH) ? 1 : 2 ;
                            break;
                        case 5: /* DECSCNM */
                            pn[2] = decscnm ? 1 : 2 ;
                            break;
                        case 6: /* DECOM */
                            pn[2] = relcursor ? 1 : 2 ;
                            break;
                        case 7: /* DECAWM */
                            pn[2] = tt_wrap ? 1 : 2 ;
                            break;
                        case 8: /* DECARM */
                            pn[2] = 3 ; /* permanently set */
                            break;
                        case 18: /* DECPFF */
                            pn[2] = xprintff ? 1 : 2 ;
                            break;
                        case 19: /* DECPEX */
                            pn[2] = !printregion ? 1 : 2 ;
                            break;
                        case 25: /* DECTCEM */
                            pn[2] = cursorena[VTERM] ? 1 : 2 ;
                            break;
                        case 42: /* DECNRCM */
                            pn[2] = decnrcm ? 1 : 2 ;
                            break;
                        case 66: /* DECNKM */
                            pn[2] = tt_keypad == TTK_APPL ? 1 : 2 ;
                            break;
                        case 67: /* DECBKM */
                            pn[2] = keymap[KEY_SCAN|8] == BS ? 1 : 2 ;
                            break;
                        case 68: /* DECKBUM */
                            pn[2] = 3 ; /* permanently set */
                            break;
                        default:
                            pn[2] = 0 ; /* Unrecognized mode */
                            break;
                        }
                        sprintf(buf,"[?%d;%d$y",pn[1],pn[2]);
                    }
                    else {
                        switch ( pn[1] ) {
                        case 2: /* KAM */
                            pn[2] = keylock ? 1 : 2 ;
                            break;
                        case 3: /* CRM */
                            pn[2] = crm ? 1 : 2 ;
                            break;
                        case 4: /* IRM */
                            pn[2] = insertmode ? 1 : 2 ;
                            break;
                        case 10: /* HEM */
                            pn[2] = 4 ; /* permanently reset */
                            break;
                        case 12: /* SRM */
                            pn[2] = !duplex ? 1 : 2 ;
                            break;
                        case 20: /* LNM */
                            pn[2] = tt_crd && tnlm ? 1 : 2 ;
                            break;
                        default:
                            pn[2] = 0 ; /* unrecognized mode */
                            break;
                        }
                        sprintf(buf,"[%d;%d$y",pn[1],pn[2]);
                    }
                    sendescseq(buf);
                }
                case 'R':
                    if ( ISBA80(tt_type_mode) && ansiext ) {
                        /* Reset virtual window to the background */
                    } else if ( ISSCO(tt_type_mode)) {
                        /* Delete lines - Alternate form of (CSI P1 M) */
                        for (i = 1; i <= pn[1]; ++i) {
                            VscrnScroll(VTERM,
                                         UPWARD,
                                         wherey[ VTERM] - 1,
                                         marginbot - 1,
                                         1,
                                         FALSE,
                                         SP);
                        }
                        VscrnIsDirty(VTERM);
                    }
                    break;
                case 'r':       /* DECCARA - Change Attr in Rect Area */
                    if ( ISVT420(tt_type_mode) )
                    {
                        int w, h, x, y, z;
                        /*
                         * pn[1] - top-line border      default=1
                         * pn[2] - left-col border      default=1
                         * pn[3] - bottom-line border   default=last line
                         * pn[4] - right col border     default=last column
                         * pn[5] -> pn[k] attributes to change - default=0
                         *
                         *  0 - attributes off
                         *  1 - bold
                         *  4 - underline
                         *  5 - blink
                         *  7 - negative image
                         * 22 - no bold
                         * 24 - no underline
                         * 25 - no blink
                         * 27 - positive image
                         *
                         * decsace == FALSE, stream else rectangle
                         */
                        if ( k < 1 ) pn[1] = 1;
                        if ( k < 2 ) pn[2] = 1;
                        if ( k < 3 ) pn[3] = VscrnGetHeight(VTERM)
                             -(tt_status[VTERM]?1:0);
                        if ( k < 4 ) pn[4] = VscrnGetWidth(VTERM);
                        if ( k < 5 ) {
                            pn[5] = 0;
                            k = 5;
                        }

                        if ( pn[3] < pn[1] || pn[4] < pn[2] )
                            break;

                        w = pn[4] - pn[2] + 1;
                        h = pn[3] - pn[1] + 1;

                        if ( decsace ) {        /* rectangle */
                            for ( y=0; y<h; y++ ) {
                                videoline * line = VscrnGetLineFromTop(VTERM,pn[1]+y-1);
                                for ( x=0; x<w; x++ ) {
                                    for ( z=5; z<=k; z++ ) {
                                        USHORT a = line->vt_char_attrs[pn[2]+x-1];
                                        switch ( pn[z] ) {
                                        case 0:
                                            a = VT_CHAR_ATTR_NORMAL;
                                            break;
                                        case 1:
                                            a |= VT_CHAR_ATTR_BOLD;
                                            break;
                                        case 4:
                                            a |= VT_CHAR_ATTR_UNDERLINE;
                                            break;
                                        case 5:
                                            a |= VT_CHAR_ATTR_BLINK;
                                            break;
                                        case 7:
                                            a |= VT_CHAR_ATTR_REVERSE;
                                            break;
                                        case 22:
                                            a &= ~VT_CHAR_ATTR_BOLD;
                                            break;
                                        case 24:
                                            a &= ~VT_CHAR_ATTR_UNDERLINE;
                                            break;
                                        case 25:
                                            a &= ~VT_CHAR_ATTR_BLINK;
                                            break;
                                        case 27:
                                            a &= ~VT_CHAR_ATTR_REVERSE;
                                            break;
                                        }
                                        line->vt_char_attrs[pn[2]+x-1] = a;
                                    }
                                }
                            }
                        } else {                /* stream */
                            for ( y=0; y<h; y++ ) {
                                videoline * line = VscrnGetLineFromTop(VTERM,pn[1]+y-1);
                                for ( x = (y==0 ? pn[2] - 1 : 0);
                                      x < ((y==h-1) ? pn[4] : VscrnGetWidth(VTERM));
                                      x++ ) {
                                    for ( z=5; z<=k; z++ ) {
                                        USHORT a = line->vt_char_attrs[x];
                                        switch ( pn[z] ) {
                                        case 0:
                                            a = VT_CHAR_ATTR_NORMAL;
                                            break;
                                        case 1:
                                            a |= VT_CHAR_ATTR_BOLD;
                                            break;
                                        case 4:
                                            a |= VT_CHAR_ATTR_UNDERLINE;
                                            break;
                                        case 5:
                                            a |= VT_CHAR_ATTR_BLINK;
                                            break;
                                        case 7:
                                            a |= VT_CHAR_ATTR_REVERSE;
                                            break;
                                        case 22:
                                            a &= ~VT_CHAR_ATTR_BOLD;
                                            break;
                                        case 24:
                                            a &= ~VT_CHAR_ATTR_UNDERLINE;
                                            break;
                                        case 25:
                                            a &= ~VT_CHAR_ATTR_BLINK;
                                            break;
                                        case 27:
                                            a &= ~VT_CHAR_ATTR_REVERSE;
                                            break;
                                        }
                                        line->vt_char_attrs[x] = a;
                                    }
                                }
                            }
                        }
                        VscrnIsDirty(VTERM);
                    }
                    break;
                case 't':       /* DECRARA - Reverse Attr in Rect Area */
                    if ( ISVT420(tt_type_mode) )
                    {
                        int w, h, x, y, z;
                        /*
                         * pn[1] - top-line border      default=1
                         * pn[2] - left-col border      default=1
                         * pn[3] - bottom-line border   default=last line
                         * pn[4] - right col border     default=last column
                         * pn[5] -> pn[k] attributes to change - default=0
                         *
                         *  0 - reverse all attributes
                         *  1 - reverse bold
                         *  4 - reverse underline
                         *  5 - reverse blink
                         *  7 - reverse negative image
                         *
                         * decsace == FALSE, stream else rectangle
                         */

                        if ( k < 1 ) pn[1] = 1;
                        if ( k < 2 ) pn[2] = 1;
                        if ( k < 3 ) pn[3] = VscrnGetHeight(VTERM)
                             -(tt_status[VTERM]?1:0);
                        if ( k < 4 ) pn[4] = VscrnGetWidth(VTERM);
                        if ( k < 5 ) {
                            pn[5] = 0;
                            k = 5;
                        }

                        if ( pn[3] < pn[1] || pn[4] < pn[2] )
                            break;

                        w = pn[4] - pn[2] + 1;
                        h = pn[3] - pn[1] + 1;

                        if ( decsace ) {        /* rectangle */
                            for ( y=0; y<h; y++ ) {
                                videoline * line = VscrnGetLineFromTop(VTERM,pn[1]+y-1);
                                for ( x=0; x<w; x++ ) {
                                    for ( z=5; z<=k; z++ ) {
                                        USHORT a = line->vt_char_attrs[pn[2]+x-1];
                                        if (pn[z] == 0 || pn[z] == 1) {
                                            if ( a & VT_CHAR_ATTR_BOLD )
                                                a &= ~VT_CHAR_ATTR_BOLD;
                                            else
                                                a |= VT_CHAR_ATTR_BOLD;
                                        }
                                        if (pn[z] == 0 || pn[z] == 4) {
                                            if ( a & VT_CHAR_ATTR_UNDERLINE )
                                                a &= ~VT_CHAR_ATTR_UNDERLINE;
                                            else
                                                a |= VT_CHAR_ATTR_UNDERLINE;
                                        }
                                        if (pn[z] == 0 || pn[z] == 5) {
                                            if ( a & VT_CHAR_ATTR_BLINK )
                                                a &= ~VT_CHAR_ATTR_BLINK;
                                            else
                                                a |= VT_CHAR_ATTR_BLINK;
                                        }
                                        if (pn[z] == 0 || pn[z] == 7) {
                                            if ( a & VT_CHAR_ATTR_REVERSE )
                                                a &= ~VT_CHAR_ATTR_REVERSE;
                                            else
                                                a |= VT_CHAR_ATTR_REVERSE;
                                        }
                                        line->vt_char_attrs[pn[2]+x-1] = a;
                                    }
                                }
                            }
                        } else {                /* stream */
                            for ( y=0; y<h; y++ ) {
                                videoline * line = VscrnGetLineFromTop(VTERM,pn[1]+y-1);
                                for ( x = (y==0 ? pn[2] - 1 : 0);
                                      x < ((y==h-1) ? pn[4] : VscrnGetWidth(VTERM));
                                      x++ ) {
                                    for ( z=5; z<=k; z++ ) {
                                        USHORT a = line->vt_char_attrs[x];
                                        if (pn[z] == 0 || pn[z] == 1) {
                                            if ( a & VT_CHAR_ATTR_BOLD )
                                                a &= ~VT_CHAR_ATTR_BOLD;
                                            else
                                                a |= VT_CHAR_ATTR_BOLD;
                                        }
                                        if (pn[z] == 0 || pn[z] == 4) {
                                            if ( a & VT_CHAR_ATTR_UNDERLINE )
                                                a &= ~VT_CHAR_ATTR_UNDERLINE;
                                            else
                                                a |= VT_CHAR_ATTR_UNDERLINE;
                                        }
                                        if (pn[z] == 0 || pn[z] == 5) {
                                            if ( a & VT_CHAR_ATTR_BLINK )
                                                a &= ~VT_CHAR_ATTR_BLINK;
                                            else
                                                a |= VT_CHAR_ATTR_BLINK;
                                        }
                                        if (pn[z] == 0 || pn[z] == 7) {
                                            if ( a & VT_CHAR_ATTR_REVERSE )
                                                a &= ~VT_CHAR_ATTR_REVERSE;
                                            else
                                                a |= VT_CHAR_ATTR_REVERSE;
                                        }
                                        line->vt_char_attrs[x] = a;
                                    }
                                }
                            }
                        }
                        VscrnIsDirty(VTERM);
                    }
                    break;
                case 'v':       /* DECCRA - Copy Rect Area */
                    if ( ISVT420( tt_type_mode) )
                    {
                        USHORT * data = NULL;
                        int w, h, x, y;

                        /* Area to be copied:
                         * pn[1] - top-line border      default=1
                         * pn[2] - left-col border      default=1
                         * pn[3] - bottom-line border   default=last line
                         * pn[4] - right col border     default=last column
                         * pn[5] - page number          default=1
                         * Destination
                         * pn[6] - top-line border      default=1
                         * pn[7] - left-col border      default=1
                         * pn[8] - page number          default=1
                         *
                         * if pn[3] > pn[1] or pn[2] > pn[4] ignore
                         * coordinates are relative to DECOM setting
                         * not affected by page margins
                         * copy text, not attributes, destination attributes remain
                         * page becomes last available page if too large
                         * if destination is off page, clip off page data
                         */
                        if ( k < 1 ) pn[1] = 1;
                        if ( k < 2 ) pn[2] = 1;
                        if ( k < 3 ) pn[3] = VscrnGetHeight(VTERM)
                             -(tt_status[VTERM]?1:0);
                        if ( k < 4 ) pn[4] = VscrnGetWidth(VTERM);
                        if ( k < 5 ) pn[5] = 1;
                        if ( k < 6 ) pn[6] = 1;
                        if ( k < 7 ) pn[7] = 1;
                        if ( k < 8 ) pn[8] = 1;
                        k = 8;

                        if ( pn[3] < pn[1] || pn[4] < pn[2] )
                            break;

                        /* we ignore pages, since we only support one */

                        w = pn[4] - pn[2] + 1;
                        h = pn[3] - pn[1] + 1;
                        data = malloc(sizeof(USHORT) * w * h);
                        if ( !data )
                            break;
                        for ( y=0; y<h; y++ ) {
                            videoline * line = VscrnGetLineFromTop(VTERM,pn[1]+y-1);
                            for ( x=0; x<w; x++ ) {
                                data[y*w + x] = line->cells[pn[2]+x-1].c;
                            }
                        }

                        for ( y=0; y<h; y++ ) {
                            videoline * line = VscrnGetLineFromTop(VTERM,pn[6]+y-1);
                            for ( x=0; x<w && (pn[7]+x <= VscrnGetWidth(VTERM)); x++ ) {
                                line->cells[pn[7]+x-1].c = data[y*w + x];
                            }
                        }
                        free(data);
                        VscrnIsDirty(VTERM);
                    }
                    break;
                case 'x':       /* DECFRA - Fill Rect Area */
                    if ( ISVT420(tt_type_mode) ) {
                        /* pn[1] - fill char                 */
                        /* pn[2] - top-line border default=1 */
                        /* pn[3] - left-col border default=1 */
                        /* pn[4] - bot-line border default=Height */
                        /* pn[5] - Right border    default=Width */
                        if ( k < 5 || pn[5] > VscrnGetWidth(VTERM) ||
                             pn[5] < 1 )
                            pn[4] = VscrnGetWidth(VTERM);
                        if ( k < 4 || pn[4] > VscrnGetHeight(VTERM)
                             -(tt_status[VTERM]?1:0) || pn[4] < 1 )
                            pn[4] = VscrnGetHeight(VTERM)
                                -(tt_status[VTERM]?1:0);
                        if ( k < 3 || pn[3] < 1 )
                            pn[3] = 1 ;
                        if ( k < 2 || pn[2] < 1 )
                            pn[2] = 1 ;
                        if ( k < 1 )
                            pn[1] = SP ;
                        clrrect_escape( VTERM, pn[2], pn[3],
                                        pn[4], pn[5], pn[1] ) ;
                        VscrnIsDirty(VTERM);
                    }
                    break;
                case 'z':       /* DECERA - Erase Rect Area */
                    if ( ISVT420(tt_type_mode) ) {
                        /* pn[1] - top-line border default=1 */
                        /* pn[2] - left-col border default=1 */
                        /* pn[3] - bot-line border default=Height */
                        /* pn[4] - Right border    default=Width */
                        if ( k < 4 || pn[4] > VscrnGetWidth(VTERM) ||
                             pn[4] < 1 )
                            pn[4] = VscrnGetWidth(VTERM);
                        if ( k < 3 || pn[3] > VscrnGetHeight(VTERM)
                             -(tt_status[VTERM]?1:0) || pn[3] < 1 )
                            pn[3] = VscrnGetHeight(VTERM)
                                -(tt_status[VTERM]?1:0);
                        if ( k < 2 || pn[2] < 1 )
                            pn[2] = 1 ;
                        if ( k < 1 || pn[1] < 1 )
                            pn[1] = 1 ;
                        clrrect_escape( VTERM, pn[1], pn[2],
                                        pn[3], pn[4], SP ) ;
                        VscrnIsDirty(VTERM);
                    }
                    break;
                case '{':       /* DECSERA - Selective Erase Rect Area */
                    if ( ISVT420(tt_type_mode) ) {
                        /* pn[1] - top-line border default=1 */
                        /* pn[2] - left-col border default=1 */
                        /* pn[3] - bot-line border default=Height */
                        /* pn[4] - Right border    default=Width */
                        if ( k < 4 || pn[4] > VscrnGetWidth(VTERM) ||
                             pn[4] < 1 )
                            pn[4] = VscrnGetWidth(VTERM);
                        if ( k < 3 || pn[3] > VscrnGetHeight(VTERM)
                             -(tt_status[VTERM]?1:0) || pn[3] < 1 )
                            pn[3] = VscrnGetHeight(VTERM)
                                -(tt_status[VTERM]?1:0);
                        if ( k < 2 || pn[2] < 1 )
                            pn[2] = 1 ;
                        if ( k < 1 || pn[1] < 1 )
                            pn[1] = 1 ;
                        selclrrect_escape( VTERM, pn[1], pn[2],
                                        pn[3], pn[4], SP ) ;
                        VscrnIsDirty(VTERM);
                    }
                    break;
                }
                break;
            case '*':
                achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                switch (achar) {
                case 'x':       /* DECSACE - Select Attribute Change Extent */
                    if ( ISVT420(tt_type_mode) )
                    {
                        /*
                         * 0 - DECCARA or DECRARA affect the stream of character
                               positions that begins with the first position specified
                               in the DECCARA or DECRARA command, and ends with the
                               second character position specified. (default)
                         * 1 - Same as 0.
                         * 2 - DECCARA or DECRARA affect all characters positions in
                               the rectangular area.  The DECCARA or DECRARA command
                               specifies the top-left and bottom-right corners.
                         */
                        if ( pn[1] == 0 || pn[1] == 1 )
                            decsace = FALSE;
                        else if ( pn[1] == 2 )
                            decsace = TRUE;
                    }
                    break;
                }
                break;
            case '`':
                /* Horizontal Position Absolute (HPA) */
                /* moves active position to column pn[1] */
                if ( decsasd == SASD_STATUS )
                    lgotoxy( VSTATUS, pn[1], 1 );
                else
                    lgotoxy( VTERM, pn[1], wherey[VTERM] ) ;
                break;
            case 'A':
                /* ANSI - Set Border Color */
                if ( ansiext ) {
                    if (ISANSI(tt_type_mode)) {
                        /* pn[1] contains new color */
                        if ( !sgrcolors )
                            break;
                        borderattribute = sgrcols[pn[1]%10];
                        setborder();
                    }
                    else if ( ISBA80(tt_type_mode) ) {
                        switch ( pn[1] ) {
                        case 0:
                            /* Dead Key Mode Off */
                            break;
                        case 1:
                            /* Dead Key Mode On */
                            break;
                        case 2:
                            /* Block Start */
                            break;
                        case 3:
                            /* Block End */
                            break;
                        }
                    }
                }
                else { /* CUU - Cursor up Pn lines */
                    if ( IS97801(tt_type_mode) ) {
                        /* ignored if outside scroll region */
                        if ( wherey[VTERM] < margintop ||
                             wherey[VTERM] > marginbot )
                            break;
                    }
                    do {
                        cursorup(0);
                        if ( tt_type_mode != TT_ANSI &&
                             tt_type_mode != TT_SCOANSI &&
                             tt_type_mode != TT_AT386 )
                            wrapit = FALSE;
                        pn[1] = pn[1] - 1;
                    } while (!(pn[1] <= 0));
                }
                break;
            case 'a':
                /* Horizontal Position Relative */
                /* moves active position pn[1] characters */
                /* to the right */
                do {
                    cursorright(0);
                    pn[1] = pn[1] - 1;
                } while (pn[1] > 0);
                break;
            case 'B':
                if ( ansiext ) {
                    /* ANSI - Set Beep Freq/Duration */
                    if ( ISANSI( tt_type_mode ) ) {
                        beepfreq = pn[1] ; /* ??? this isn't correct */
                        beeptime = pn[2] * 100 ;
                    }
                    else if ( ISBA80(tt_type_mode) ) {
                        switch ( pn[1] ) {
                        case 0:
                            /* Cursor Off */
                            cursorena[VTERM] = TRUE;
                            break;
                        case 1:
                            /* Cursor On */
                            cursorena[VTERM] = FALSE;
                            break;
                        case 2:
                            /* Alternate Cursor */
                            tt_cursor = TTC_BLOCK ;
                            setcursormode();
                            cursorena[VTERM] = TRUE ;
                            break;
                        case 3:
                            /* Normal Cursor */
                            tt_cursor = TTC_ULINE ;
                            setcursormode();
                            cursorena[VTERM] = TRUE ;
                            break;
                        }
                    }
                }
                else {  /* CUD - Cursor down pn lines */
                    if ( IS97801(tt_type_mode) ) {
                        /* ignored if outside scroll region */
                        if ( wherey[VTERM] < margintop ||
                             wherey[VTERM] > marginbot )
                            break;
                    }
                    do {
                        cursordown(0);
                        if ( !ISANSI( tt_type_mode ) ||
                             ISLINUX(tt_type_mode))
                            wrapit = FALSE;
                        pn[1] = pn[1] - 1;
                    } while (!(pn[1] <= 0));
                }
                break;
            case 'b':
                /* QANSI - Repeat previous character Pn times */
                if ( ISQANSI(tt_type_mode) ) {
                    while ( pn[1] ) {
                        wrtch(prevchar);
                        pn[1] = pn[1] - 1;
                    }
                } else if ( ISSCO(tt_type_mode) ||
                            ISAAA(tt_type_mode)) {
                    /* HPR - Horizontal Position Relative */
                    while ( pn[2] ) {
                        wrtch(pn[1]);
                        pn[2] = pn[2] - 1;
                    }
                }
                break;
            case 'C':
                if ( ISBA80(tt_type_mode) && ansiext ) {
                    /* Close Virtual Window */
                    /* close the current virtual window */
                }
                /* ANSI - Set Cursor Height */
                if ( ISANSI( tt_type_mode ) &&
                     ansiext ) {
                    debug(F111,"ANSI-SetCursor","start",pn[1]);
                    debug(F111,"ANSI-SetCursor","end",pn[2]);
                    if ( pn[1] > pn[2] ) {
                        cursorena[VTERM] = FALSE ;
                    }
                    else {
                        if ( pn[2]-pn[1] <= 1 )
                            tt_cursor = TTC_ULINE ;
                        else if ( pn[2]-pn[1] < 10 )
                            tt_cursor = TTC_HALF ;
                        else
                            tt_cursor = TTC_BLOCK ;
                        setcursormode();
                        cursorena[VTERM] = TRUE ;
                    }
                }
                else  /* CUF - Cursor right pn chars */
                    do {
                        cursorright(0);
                        pn[1] = pn[1] - 1;
                    } while (pn[1] > 0);
                break;
            case 'c':
                if ( zdsext && ISVT220(tt_type) ) {
                    /* Secondary Device Attributes Report Request */
                    if (pn[1] == 0) {
                        sendescseq("[>24;0;0c");
                    }
                }
                else if ( ansiext && ISVT220(tt_type) ) {
                    /* Tertiary Device Attributes Report Request */
                    if (pn[1] == 0) {
                        char DECRPTUI[24] ;
                        sprintf(DECRPTUI,"P!|00000000%c\\",ESC);
                        sendescseq(DECRPTUI);
                    }
                }
                else if ( IS97801(tt_type_mode) ) {
                    /* Reset to Initial State */
                    if ( debses )
                        break;
                    doreset(1);
                }
                else if ( ansiext && ISSCO(tt_type_mode) ||
                          ISANSI( tt_type_mode ) ) {
                    /* Set Cursor Type */
                    /* pn[1] = 0 - underline */
                    /* pn[1] = 1 - block     */
                    /* pn[1] = 2 - no cursor */
                    debug(F111,"CSI c","SetCursorType",pn[1]);
                    switch ( pn[1] ) {
                    case 0:
                        tt_cursor = TTC_ULINE ;
                        cursorena[VTERM] = TRUE;
                        break;
                    case 1:
                        tt_cursor = TTC_BLOCK ;
                        cursorena[VTERM] = TRUE ;
                        break;
                    case 2:
                        cursorena[VTERM] = FALSE ;
                        break;
                    }
                    setcursormode() ;
                }
                else if ( private && ISLINUX(tt_type_mode) ) {
                    /* Set Cursor Type */
#ifdef COMMENT
  VGA-softcursor.txt, from the 2.2 kernel

  Software cursor for VGA    by Pavel Machek <pavel@atrey.karlin.mff.cuni.cz>
  =======================    and Martin Mares <mj@atrey.karlin.mff.cuni.cz>

     Linux now has some ability to manipulate cursor appearance. Normally, you
  can set the size of hardware cursor (and also work around some ugly bugs in
  those miserable Trident cards--see #define TRIDENT_GLITCH in drivers/video/
  vgacon.c). You can now play a few new tricks:  you can make your cursor look
  like a non-blinking red block, make it inverse background of the character it's
  over or to highlight that character and still choose whether the original
  hardware cursor should remain visible or not.  There may be other things I have
  never thought of.

     The cursor appearance is controlled by a "<ESC>[?1;2;3c" escape sequence
  where 1, 2 and 3 are parameters described below. If you omit any of them,
  they will default to zeroes.

     Parameter 1 specifies cursor size (0=default, 1=invisible, 2=underline, ...,
  8=full block) + 16 if you want the software cursor to be applied + 32 if you
  want to always change the background color + 64 if you dislike having the
  background the same as the foreground.  Highlights are ignored for the last two
  flags.

     The second parameter selects character attribute bits you want to change
  (by simply XORing them with the value of this parameter). On standard VGA,
  the high four bits specify background and the low four the foreground. In both
  groups, low three bits set color (as in normal color codes used by the console)
  and the most significant one turns on highlight (or sometimes blinking--it
  depends on the configuration of your VGA).

     The third parameter consists of character attribute bits you want to set.
  Bit setting takes place before bit toggling, so you can simply clear a bit by
  including it in both the set mask and the toggle mask.

  Examples:
  =========

  To get normal blinking underline, use: echo -e '\033[?2c'
  To get blinking block, use:            echo -e '\033[?6c'
  To get red non-blinking block, use:    echo -e '\033[?17;0;64c'
#endif /* COMMENT */
                    debug(F111,"CSI ? c","SetCursorType pn[1]",pn[1]);
                    debug(F111,"CSI ? c","SetCursorType pn[2]",pn[2]);
                    debug(F111,"CSI ? c","SetCursorType pn[3]",pn[3]);
                    switch ( pn[1] & 0x0F ) {
                    case 0:
                    case 2:
                        tt_cursor = TTC_ULINE ;
                        cursorena[VTERM] = TRUE;
                        break;
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        tt_cursor = TTC_HALF ;
                        cursorena[VTERM] = TRUE ;
                        break;
                    case 8:
                        tt_cursor = TTC_BLOCK ;
                        cursorena[VTERM] = TRUE ;
                        break;
                    case 1:
                        cursorena[VTERM] = FALSE ;
                        break;
                    }
                    setcursormode() ;
                    break;
                } else { /* DA - Device Attributes */
                    if (pn[1] == 0)
                        if (tt_type >= 0 &&
                             tt_type <= max_tt) {
                            sendescseq(tt_info[tt_type].x_id);
                        }
                    }
                    break;
            case 'D':
                /* ANSI - Turn on/off Background Intensity */
                if ( ISANSI( tt_type_mode ) &&
                     ansiext ) {
                    if ( !pn[1] && !bgi ||
                         pn[1] && bgi ) {
                        attribute |= 0x80 ;
                        defaultattribute |= 0x80 ;
                    }
                    else
                    {
                        attribute &= 0x7F ;
                        defaultattribute &= 0x7F ;
                    }
                }
                else  /* CUB - Cursor Left pn characters */
                    do {
                        cursorleft(0);
                        pn[1] = pn[1] - 1;
                    } while (pn[1] > 0);
                break;
            case 'd':
                /* VPA - Vertical Position Absolute */
                /* moves active position to row pn[1] */
                if ( decsasd == SASD_TERMINAL )
                    lgotoxy( VTERM, wherex[VTERM], pn[1] ) ;
                break;
            case 'E':
                if ( ISBA80(tt_type_mode) && ansiext ) {
                    /* Window Status Request */
                    char buf[64];
                    sprintf(buf,"%c[=D%3d:%3d;001:001;Lohn    D",
                             27,VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0),
                             VscrnGetWidth(VTERM));
                        ttol(buf,strlen(buf));
                }
                /* ANSI - TRUE BLINK vs Bold Background Intensity */
                if ( ISANSI( tt_type_mode ) &&
                     ansiext ) {
                    if ( !pn[1] )
                    {
                        /* use TRUE BLINK - default */
                        trueblink = TRUE ;
                    }
                    else
                    {
                        /* use Bold Background Intensity */
                        trueblink = FALSE ;
                    }
                }
                else {
                    /* CNL - Cursor next line */
                    /* moves active position pn[1] rows down */
                    do {
                        cursornextline();
                        pn[1] = pn[1] - 1;
                    } while (pn[1] > 0);
                }
                break;
            case 'e':
                /* VPR - Vertical Position Relative */
                /* moves active position pn[1] rows down */
                do {
                    cursordown(0);
                    pn[1] = pn[1] - 1;
                } while (pn[1] > 0);
                break;
            case 'F':
                if ( ISBA80(tt_type_mode) && ansiext ) {
                    /* Fetch virtual window from the background */
                }
                /* ANSI - Set Normal Foreground Color */
                if (( ISANSI( tt_type_mode ) ||
                      ISQANSI(tt_type_mode)) &&
                     ansiext ) {
                    /* pn[1] contains new color */
                    l = pn[1];
                    if (decscnm
#ifdef COMMENT
                         && !attrib.reversed ||
                         !decscnm && attrib.reversed
#endif
                         ) {
                        i = (defaultattribute & 0xF0);
                        attribute = defaultattribute = (i | (l << 4));

                    }
                    else {
                        i = (defaultattribute & 0xF0);
                        attribute = defaultattribute = (i | l);
                    }
                }
                else {
                    /* CPL - Cursor Previous Line */
                    /* moves active position pn[1] rows up */
                    /* in the first column */
                    do {
                        cursorprevline();
                        pn[1] = pn[1] - 1;
                    } while (pn[1] > 0);
                }
                break;
            case 'G':
                /* ANSI - Set Normal Background Color */
                if ( ansiext &&
                     (ISANSI( tt_type_mode ) || ISQANSI(tt_type_mode)) ) {
                    /* pn[1] contains new color */
                    l = pn[1];
                    if (!decscnm
#ifdef COMMENT
                         && !attrib.reversed ||
                         decscnm && attrib.reversed
#endif
                         ) {
                        i = (defaultattribute & 0x0F);
                        attribute = defaultattribute = (i | (l << 4));
                    }
                    else {
                        i = (defaultattribute & 0x0F);
                        attribute = defaultattribute = (i | l);
                    }
                }
                else {
                    /* CHA - Cursor Horizontal Absolute */
                    if ( ISHFT(tt_type_mode) ||
                         ISLINUX(tt_type_mode) ||
                         ISQANSI(tt_type_mode) ||
                         ISANSI(tt_type_mode)) {
                        if ( pn[1] < 1 || pn[1] > VscrnGetWidth(VTERM) )
                            break;
                        lgotoxy(VTERM,pn[1],wherey[VTERM]);
                    }
                }
                break;
            case 'H':
                /* ANSI - Set Reverse Foreground Color */
                if ( ISANSI( tt_type_mode ) &&
                     ansiext ) {
                    /* pn[1] contains new color */
                    l = pn[1];
                    i = (reverseattribute & 0xF0);
                    reverseattribute = (i | l);
                    break;
                }
                /* 'H' is also CUP - Direct cursor address for */
                /* !ansiext, so don't put a break here   */
            case 'f':   /* HVP - Direct cursor address */
                if ( IS97801(tt_type_mode) && decsasd == SASD_STATUS )
                    setdecsasd(SASD_TERMINAL);

                if (pn[1] == 0)
                    pn[1] = 1;
                if (relcursor)
                    pn[1] += margintop - 1;
                if (pn[1] > VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0))
                    pn[1] = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                if (pn[2] == 0)
                    pn[2] = 1;
                if (isdoublewidth(pn[1])) {
                    if (pn[2] > VscrnGetWidth(VTERM)/2)
                        pn[2] = VscrnGetWidth(VTERM)/2;
                    }
                    else if (pn[2] > VscrnGetWidth(VTERM))
                        pn[2] = VscrnGetWidth(VTERM);
                    wrapit = FALSE;

                /* SNI 97801 - If the cursor is addressed to the */
                /* status line when SSDT_HOST_WRITABLE, we must  */
                /* enter SASD_STATUS mode                        */
                if ( IS97801(tt_type_mode) ) {
                    if ( pn[1] == VscrnGetHeight(VTERM) &&
                         decsasd != SASD_STATUS ) {
                        if ( decssdt == SSDT_INDICATOR )
                            setdecssdt(SSDT_HOST_WRITABLE);
                        setdecsasd(SASD_STATUS);
                    }
                }

                if ( decsasd == SASD_STATUS )
                    lgotoxy( VSTATUS, pn[2], 1 );
                else
                    lgotoxy(VTERM, pn[2], pn[1]);
                break;
            case 'I':
                if ( ansiext && private ) {
                    if ( ISBA80(tt_type_mode) ) {
                        /* Screen Saver Off */
                    }
                }
                else if ( ansiext ) {
                    if ( ISBA80(tt_type_mode) ) {
                        /* Input */
                    }
                    /* ANSI - Set Reverse Background Color */
                    if ( ISANSI( tt_type_mode ) )
                    {
                        /* pn[1] contains new color */
                        l = pn[1];
                        i = (reverseattribute & 0x0F);
                        reverseattribute = (i | (l << 4));
                    }
                }
                else {
                    /* CHT - Cursor Horizontal Tab */
                    if ( k < 1 )
                        pn[1] = 1;
                    i = wherex[VTERM];
                    while ( pn[1] ) {
                        if (i < VscrnGetWidth(VTERM))
                        {
                            do {
                                i++;
                                cursorright(0);
                            } while ((htab[i] != 'T') &&
                                      (i <= VscrnGetWidth(VTERM)-1));
                        }
                        pn[1]--;
                    }
                    VscrnIsDirty(VTERM);
                }
                break;
            case 'g':
                /* ANSI - Display Graphic Character */
                if ( ISSCO(tt_type_mode) && !scoBCS2 ||
                     ISANSI( tt_type_mode ) &&
                     ansiext ) {
                    /* pn[1] contains a graphic Character
                    to display */
                    unsigned char a = attribute ;
                    int eight = sco8bit ;
                    int c = crm ;
                    struct _vtG * g = GR ;

                    attribute = graphicattribute ;
                    sco8bit = FALSE ;
                    crm = TRUE;
                    GR = &G[2];

                    wrtch( pn[1] ) ;

                    attribute = a ;
                    sco8bit = eight ;
                    crm = c;
                    GR = g;
                }
                else {
                    /* TBC - Tabulation Clear */
                    /* VT terminals only support 0 and 3 */
                    /* HFT (begin) */
                    /*  0 - clear horizontal tab stop at active position */
                    /*  1 - vertical tab at line indicated by cursor     */
                    /*  2 - horizontal tabs on line                      */
                    /*  3 - all horizontal tabs                          */
                    /*  4 - all vertical tabs                            */
                    /* HFT (end)                                         */
                    /* SCO BCS2 mode supports this functionality */
                    if (pn[1] == 3 || 
                        (pn[1] == 2 && !ISVT100(tt_type_mode)))
                    {
                        /* clear all tabs */
                        for (j = 1; j <=MAXTERMCOL; ++j)
                            htab[j] = '0';
                    } else if (pn[1] == 0) {
                        /* clear tab at current position */
                        htab[wherex[VTERM]] = '0';
                    }
                }
                break;
            case 'h':   /* Set Mode */
                for (j = 1; j <= k; ++j) {
                    if (private) {
                        switch (pn[j]) {        /* Field specs */
                        case 1: /* DECCKM  */
                            tt_arrow = TTK_APPL;
                            break;
                        case 2:
                            /* DECANM : ANSI/VT52 */
                            /* XTERM - Designate US_ASCII into G0-G3 */
                            if ( ISH19(tt_type) ) {
                                tt_type_mode = tt_type ;
                                vt52graphics = FALSE;
                            }
                            else if ( decsasd == SASD_TERMINAL ){
                                if ( ISVT52(tt_type_mode) ) {
                                    tt_type_mode = tt_type_vt52 ;
                                    vt52graphics = FALSE;
                                    ipadl25();
                                }
                            }
                            break;
                        case 3: /* DECCOLM = 132 Columns */
                            RequestScreenMutex(SEM_INDEFINITE_WAIT);
                            killcursor(VTERM);
                            deccolm = TRUE;
                            Set132Cols(VTERM);
#ifdef TCPSOCKET
#ifdef CK_NAWS
                            if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0) {
                                tn_snaws();
#ifdef RLOGCODE
                                rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                                ssh_snaws();
#endif /* SSHBUILTIN */
                            }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */
                            setborder();
                            newcursor(VTERM);
                            clrscreen(VTERM,SP);
                            lgotoxy(VTERM,1,1);       /* and home the cursor */
                            ipadl25();
                            ReleaseScreenMutex();
                            break;
                        case 4: /* DECSCLM - Smooth scroll */
                            SmoothScroll();
                            break;
                        case 5: /* DECSCNM - Reverse screen */
                            if (decscnm) /* Already reverse */
                                break;
                            else {
                                flipscreen(VTERM);
                                VscrnIsDirty(VTERM) ;
                            }
                            break;
                        case 6: /* DECOM - Relative origin */
                            relcursor = TRUE;
                            if ( decsasd == SASD_STATUS )
                                lgotoxy( VSTATUS, 1, 1 );
                            else
                                lgotoxy(VTERM, 1, margintop);
                            break;
                        case 7: /* DECAWM - Auto Wrap mode */
                            tt_wrap = TRUE;
                            break;
                        case 8: /* DECARM - Autorepeat */
                            break;
                        case 9: /* DECINLM - Interlace */
                            /* XTERM - Send Mouse X & Y on button press */
                            break;
                        case 10:        /* DECEDM - Block Mode On */
                            break;
                        case 18:  /* DECPFF - Print Form Feed */
                            xprintff = TRUE;
                            break;
                        case 19:        /* DECPEX - Printer extent */
                            printregion = FALSE; /* Full screen */
                            break;
                        case 25:   /* DECTCEM - Cursor On */
                            cursorena[VTERM] = TRUE;
                            break;
                        case 34:    /* DECRLM - Right to Left Mode */
                            debug(F111,"VT","DECRLM - Right to Left Mode",1);
                            break;
                        case 35: /* DECHEBM */
                            /* Enter Hebrew keyboard mode */
                            tt_kb_mode = KBM_HE ;
                            break;
                        case 36:    /* DECHEM - Hebrew Encoding Mode */
                            debug(F111,"VT","DECHEM - Hebrew Encoding Mode",1);
                            break;
                        case 38: /* DECTEK */
                            /* Enter Tektronix 4010/4014 mode */
                            settekmode();
                            break;
                        case 40: /* XTERM - Allow 80 - 132 Mode */
                            break;
                        case 41: /* XTERM - more(1) fix */
                            break;
                        case 42: { /* DECNRCM */
                            /* Use 7-bit NRC */
                            int tcs ;
                            decnrcm = TRUE ;
#ifdef COMMENT
                            if ( G[0].national )
                            {
                                for ( i = 1 ; i < 4 ; i++ ) {
                                    G[i].designation =
                                        G[0].def_designation ;
                                    G[i].size = G[0].def_size ;
                                    G[i].c1   = G[0].def_c1 ;
                                    G[i].national = G[0].national;
                                    G[i].rtoi = xl_u[G[0].def_designation];
                                    if ( isunicode() ) {
                                        G[i].itol = NULL ;
                                        G[i].ltoi = NULL ;
                                    }
                                    else {
                                        G[i].itol = xl_tx[tcsl] ;
                                        G[i].ltoi = xl_u[tcsl] ;
                                    }
                                    G[i].itor = xl_tx[G[0].def_designation] ;
                                }
                            }
                            else {
                                for ( i = 1 ; i < 4 ; i++ ) {
                                    G[i].designation = TX_ASCII;
                                    G[i].size = cs94 ;
                                    G[i].c1 = TRUE ;
                                    G[i].national = CSisNRC(G[i].designation);
                                    G[i].rtoi = xl_u[TX_ASCII];
                                    if ( isunicode() ) {
                                        G[i].itol = NULL ;
                                        G[i].ltoi = NULL ;
                                    }
                                    else {
                                        G[i].itol = xl_tx[tcsl] ;
                                        G[i].ltoi = xl_u[tcsl] ;
                                    }
                                    G[i].itor = xl_tx[TX_ASCII];
                                }
                            }
#else /* COMMENT */
                            for ( i = 1 ; i < 4 ; i++ ) {
                                G[i].designation = dec_nrc;
                                G[i].size = cs94 ;
                                G[i].c1 = TRUE ;
                                G[i].national = CSisNRC(dec_nrc);
                                G[i].rtoi = xl_u[dec_nrc];
                                if ( isunicode() ) {
                                    G[i].itol = NULL ;
                                    G[i].ltoi = NULL ;
                                }
                                else {
                                    G[i].itol = xl_tx[tcsl] ;
                                    G[i].ltoi = xl_u[tcsl] ;
                                }
                                G[i].itor = xl_tx[dec_nrc];
                            }
#endif /* COMMENT */
                        }
                            break;
                        case 43: /* DECGEPM */
                            /* Graphics Expanded Print mode */
                            break;
                        case 44: /* DECGPCM */
                            /* Graphics Print Color mode */
                            /* XTERM - Turn on Margin Bell */
                            break;
                        case 45: /* DECGPCS */
                            /* Graphics Print Color syntax = RGB */
                            /* XTERM - Reverse-wraparound mode */
                            break;
                        case 46: /* DECGPBM */
                            /* Graphics Print Background mode */
                            /* XTERM - Start Logging */
                            break;
                        case 47: /* DECGRCM */
                            /* Graphics Rotated Print mode */
                            /* XTERM - Use Alternate Screen Buffer */
                            break;
                        case 53: /* DEC131TM */
                            /* VT131 Transmit */
                            break;
                        case 60: /* DECHCCM */
                            /* Horizontal cursor coupling */
                            break;
                        case 61: /* DECVCCM */
                            /* Vertical cursor coupling */
                            break;
                        case 64: /* DECPCCM */
                            /* Page cursor coupling */
                            break;
                        case 66: /* DECNKM */
                            /* Numeric Keyboard - Application */
                            tt_keypad = TTK_APPL ;
                            break;
                        case 67: /* DECBKM */
                            /* Backarrow key sends BS */
                            if ( !savdecbkm )
                                savdecbkm = keymap[KEY_SCAN | 8];
                            keymap[KEY_SCAN | 8] = BS ;
                            break;
                        case 68: /* DECKBUM */
                            /* Keyboard Usage - Data Processing */
                            deckbum = 1;
                            break;
                        case 73: /* DECXRLM */
                            /* Transmit rate limiting */
                            break;
                        case 80:        /* WY161 - 161-column mode */
                            if ( ISWY370(tt_type_mode) )
                            {
                                tt_szchng[VTERM] = 1 ;
                                tt_cols[VTERM] = 161 ;
                                VscrnInit( VTERM ) ;  /* Height set here */
#ifdef TCPSOCKET
#ifdef CK_NAWS
                                if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0){
                                    tn_snaws();
#ifdef RLOGCODE
                                    rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                                    ssh_snaws();
#endif /* SSHBUILTIN */
                                }
#endif /* TCPSOCKET */
#endif /* CK_NAWS */
                            }
                            break;
                            case 83:    /* WY52 - 52 line mode */
                            if ( ISWY370(tt_type_mode)
                                 && tt_modechg == TVC_ENA) {
                                tt_szchng[VTERM] = 1 ;
                                tt_rows[VTERM] = 52 ;
                                VscrnInit( VTERM ) ;          /* Height set here */
#ifdef TCPSOCKET
#ifdef CK_NAWS
                                if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0){
                                    tn_snaws();
#ifdef RLOGCODE
                                    rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                                    ssh_snaws();
#endif /* SSHBUILTIN */
                                }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */
                            }
                            break;
                        case 84:        /* WYENAT */
                            if ( ISWY370(tt_type_mode) )
                                /* Enable separate assignment of */
                                /* attributes (SGR) to erasable  */
                                /* and nonerasable characters    */
                                ;
                            break;
                        case 85:        /* WYREPL */
                            if ( ISWY370(tt_type_mode) )
                                /* Set replacement character to */
                                /* current character background */
                                /* color */
                                ;
                            break;
                        case 1000:
                            /* XTERM - Send Mouse X&Y on button press and release */
                            break;
                        case 1001:
                            /* XTERM - Use Hilite Mouse Tracking */
                            break;
                        default:
                            break;
                        }
                    } else if ( zdsext ) {
                        switch (pn[j]) {
                        /* Zenith/Heath values */
                        case 1: /* Enable 25th line */
                            if ( !tt_status[VTERM] )
                                break;
                            settermstatus(FALSE);
                            break;
                        case 2: /* No Key Click */
                            setkeyclick(FALSE);
                            break;
                        case 3: /* Hold Screen Mode */
                            break;
                        case 4: /* Block Cursor */
                            tt_cursor = TTC_BLOCK ;
                            setcursormode() ;
                            break;
                        case 5: /* Cursor Off */
                            cursorena[VTERM] = FALSE ;
                            break;
                        case 6: /* Keypad Shifted */
                            tt_shift_keypad = TRUE ;
                            break;
                        case 7: /* Alternate keypad mode */
                            tt_keypad = TTK_APPL ;
                            break;
                        case 8: /* Auto Line Feed on receipt of CR */
                            tt_crd = TRUE ;
                            break;
                        case 9: /* Auto CR on receipt of LF */
                            tnlm = TRUE ;
                            break;

                        /* AnnArbor values */
                        case 25:        /* zMDM - Margin Bell Mode */
                            break;
                        case 26:        /* zKCM - Key Click Mode */
                            break;
                        case 27:        /* zKPCM - Key Pad Control Mode */
                            break;
                        case 28:        /* zKRM - Key Repeat Mode */
                            break;
                        case 29:        /* zRLM - Return Line Feed Mode */
                            break;
                        case 30:        /* zDBM - Destructive Backspace Mode */
                            break;
                        case 31:        /* zBKCM - Block Cursor Mode */
                            break;
                        case 32:        /* zBNCM - Blinking Cursor Mode */
                            break;
                        case 33:        /* zWFM - Wrap Forward Mode */
                            break;
                        case 34:        /* zWBM - Wrap Backward Mode */
                            break;
                        case 35:        /* zDDM - DEL character Display Mode */
                            break;
                        case 36:        /* zSPM - Scroll Page Mode */
                            break;
                        case 37:        /* zAXM - Auto Xoff/Xon Mode */
                            break;
                        case 38:        /* zAPM - Auto Pause Mode */
                            break;
                        case 39:        /* zSSM - Slow Scroll Mode */
                            break;
                        case 40:        /* zHDM - Half-Duplex Mode */
                            break;
                        case 41:        /* zTPDM - Transfer Pointer Display Mode */
                            break;
                        case 42:        /* zLTM - Line Transfer Mode */
                            break;
                        case 43:        /* zCSTM - Column Separator Transfer Mode */
                            break;
                        case 44:        /* zFSTM - Field Separator Transfer Mode */
                            break;
                        case 45:        /* zGRTM - Graphic Rendition Transfer Mode */
                            break;
                        case 46:        /* zAKDM - Auto Keyboard Disable Mode */
                            break;
                        case 47:        /* zFRM - Fast Repeat Mode */
                            break;
                        case 48:        /* zHAM - Hold in Area Mode */
                            break;
                        case 49:        /* zGAPM - Guarded Area Print Mode */
                            break;
                        case 51:        /* zACM - Alternate Cursor Mode */
                            break;
                        case 52:        /* zMKM - Meta Key Mode */
                            break;
                        case 53:        /* zCLIM - Caps Lock Invert Mode */
                            break;
                        case 54:        /* zINM - Ignore NUL Mode */
                            break;
                        case 55:        /* zCNM - CR New-Line Mode */
                            break;
                        case 56:        /* zICM - Invisible Cursor Mode */
                            break;
                        case 57:        /* zMMM - Meta Monitor Mode */
                            break;
                        case 59:        /* zIVM - Inverse Video Mode */
                            break;
                        case 61:        /* Z8RCM - 8-bit Remote Copy Mode */
                            break;
                        }
                    } else {      /* Not private */
                        if ( ISSCO(tt_type_mode) ) {
                            switch ( pn[j] ) {
                            case 2: /* Keyboard locked */
                                keylock = TRUE;
                                break;
                            case 3: /* 132 Columns */
                                RequestScreenMutex(SEM_INDEFINITE_WAIT);
                                killcursor(VTERM);
                                deccolm = TRUE;
                                Set132Cols(VTERM);
#ifdef TCPSOCKET
#ifdef CK_NAWS
                                if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0) {
                                    tn_snaws();
#ifdef RLOGCODE
                                    rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                                    ssh_snaws();
#endif /* SSHBUILTIN */
                                }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */
                                setborder();
                                newcursor(VTERM);
                                clrscreen(VTERM,SP);
                                lgotoxy(VTERM,1,1);       /* and home the cursor */
                                ipadl25();
                                ReleaseScreenMutex();
                                break;
                            case 4: /* Smooth scroll */
                                SmoothScroll();
                                break;

                            case 6: /* Relative origin */
                                relcursor = TRUE;
                                if ( decsasd == SASD_STATUS )
                                    lgotoxy( VSTATUS, 1, 1 );
                                else
                                    lgotoxy(VTERM, 1, margintop);
                                break;
                            case 7: /* Auto Wrap mode */
                                tt_wrap = TRUE;
                                break;
                            case 25:   /* Cursor On */
                            case 48:
                            case 1048:
                                cursorena[VTERM] = TRUE;
                                break;
                            case 30:    /* Scrollbars On */
                                break;
                            case 44:    /* Margin Bell On */
                                marginbell = TRUE;
                                break;
                            }
                        } else {
                            switch (pn[j]) {
                            case 2: /* Keyboard locked */
                                keylock = TRUE;
                                break;
                            case 3: /* CRM - Controls Mode On */
                                crm = TRUE ;
                                break;
                            case 4: /* ANSI insert mode */
                                if (ISVT102(tt_type_mode) ||
                                     ISANSI(tt_type_mode))
                                    insertmode = TRUE;
                                break;
                            case 6: /* Erasure Mode (ERM) Set */
                                erm = 1 ;
                                break;
                            case 12:        /* SRM - Send Receive ON */
                                if ( duplex_sav < 0 )
                                    duplex_sav = duplex ;
                                duplex = 0; /* Remote echo */
                                break;
                            case 13:        /* FEAM - Control execution off */
                                /* Display control codes */
                                break;
                            case 16:        /* TTM - Send through cursor position */
                                break;
                            case 20:
                                /* LNM - linefeed / newline mode */
                                tt_crd = tnlm = TRUE;
                                break;
                            case 30:        /* WYDSCM - Turn display off */
                                if ( ISWY370(tt_type_mode) ) {
                                    screenon = FALSE;
                                    if ( !savefiletext[0] )
                                        strcpy(savefiletext,filetext);
                                    strcpy(filetext,"SCREEN OFF") ;
                                    VscrnIsDirty(VTERM);
                                }
                                break;
                            case 31:        /* WYSTLINM - Display status line */
                                break;
                            case 32:        /* WYCTRSAVM - Screen Saver off */
                                break;
                            case 33:        /* WYSTCURM - Cursor steady */
                                break;
                            case 34:        /* WYULCURM - Underline cursor */
                                if ( ISWY370(tt_type_mode) ) {
                                    tt_cursor = TTC_ULINE ;
                                    setcursormode();
                                }
                                break;
                            case 35:        /* WYCLRM - Width change clear off */
                                break;
                            case 36:        /* WYDELKM - Set delete key to BS/DEL */
                                break;
                            case 37:        /* WYGATM - Send all characters */
                                break;
                            case 38:        /* WYTEXM - Send full page */
                                break;
                            case 40:        /* WYEXTDM - Extra data line */
                                break;
                            case 42:        /* WYASCII - Select Wyse 350 personality */
                                break;
                            case 50:    /* 97801-5XX - Compose key table on */
                                if ( IS97801(tt_type_mode) ) {
                                    break;
                                }
                                break;
                            case 51:    /* 97801-5XX - Dead key table on */
                                if ( IS97801(tt_type_mode) ) {
                                    break;
                                }
                                break;
                            case 53:    /* 97801-5XX - 132-character mode */
                                if ( IS97801(tt_type_mode) ) {
                                    RequestScreenMutex(SEM_INDEFINITE_WAIT);
                                    killcursor(VTERM);
                                    deccolm = TRUE;
                                    Set132Cols(VTERM);
#ifdef TCPSOCKET            
#ifdef CK_NAWS              
                                    if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0) {
                                        tn_snaws();
#ifdef RLOGCODE             
                                        rlog_naws();
#endif /* RLOGCODE */       
#ifdef SSHBUILTIN           
                                        ssh_snaws();
#endif /* SSHBUILTIN */     
                                    }
#endif /* CK_NAWS */        
#endif /* TCPSOCKET */      
                                    setborder();
                                    newcursor(VTERM);
                                    clrscreen(VTERM,SP);
                                    lgotoxy(VTERM,1,1);       /* and home the cursor */
                                    ipadl25();
                                    ReleaseScreenMutex();
                                }
                                break;
                            default:
                                break;
                            }
                        }
                    }
                } /* for */
                break;
            case 'l':
                if ( ansiext && private ) {
                    /* Insert Line Up */
                    VscrnScroll(VTERM,
                                 UPWARD,
                                 0,
                                 wherey[VTERM] - 1,
                                 1,
                                 FALSE,
                                 SP);
                    break;
                }
                else if (ansiext && ISSCO(tt_type_mode)) {
                    /* SCO - Clear and Home Cursor */
                    clrscreen(VTERM,SP);
                    lgotoxy(VTERM,1,1);
                    VscrnIsDirty(VTERM);
                }
                else {/* Reset Mode */
                    for (j = 1; j <= k; ++j) {/* Go thru all pn's */
                        if (private) {
                            switch ((pn[j])) {      /* Field specs */
                            case 1: /* DECCKM - Cursor key mode */
                                tt_arrow = TTK_NORM;
                                break;
                            case 2: /* DECANM : ANSI/VT52 */
                                if ( decsasd == SASD_TERMINAL ) {
                                    tt_type_vt52 = tt_type_mode ;
                                    tt_type_mode = TT_VT52;
                                    vt52graphics = FALSE;
                                    ipadl25();
                                }
                                break;
                            case 3: /* DECCOLM - 80 Columns */
                                RequestScreenMutex(SEM_INDEFINITE_WAIT);
                                killcursor(VTERM);
                                deccolm = FALSE;
                                Set80Cols(VTERM);
#ifdef TCPSOCKET
#ifdef CK_NAWS
                                if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0) {
                                    tn_snaws();
#ifdef RLOGCODE
                                    rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                                   ssh_snaws();
#endif /* SSHBUILTIN */
                               }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */
                               setborder();
                               newcursor(VTERM);
                               clrscreen(VTERM,SP);
                               lgotoxy(VTERM,1,1);       /* and home the cursor */
                               ipadl25();
                               ReleaseScreenMutex();
                               break;
                           case 4: /* DECSCLM - Jump scrolling */
                               JumpScroll() ;
                               break;
                           case 5: /* DECSCNM - Normal screen */
                               if (!decscnm) /* Already normal? */
                                   break;
                               else {
                                   flipscreen(VTERM);
                                   VscrnIsDirty(VTERM);
                               }
                               break;
                           case 6: /* DECOM - Absolute origin */
                               relcursor = FALSE;
                               if ( decsasd == SASD_STATUS )
                                   lgotoxy( VSTATUS, 1, 1 );
                               else
                                   lgotoxy(VTERM,1, 1);
                               break;
                           case 7: /* DECAWM - Auto wrap */
                               tt_wrap = FALSE;
                               break;
                           case 8: /* DECARM - Auto repeat */
                               break;
                           case 9: /* DECINLM - Interlace */
                               /* XTERM - Don't Send Mouse X&Y on button press */
                               break;
                           case 10:        /* DECEDM - Block mode off */
                               break;
                           case 18: /* DECPFF - Print Form Feed */
                               xprintff = FALSE;
                               break;
                           case 19:        /* DECPEX - Print extent */
                               printregion = TRUE; /* Region */
                               break;
                           case 25: /* DECTCEM - Cursor Off */
                               cursorena[VTERM] = FALSE;
                               break;
                           case 34:    /* DECRLM - Right to Left Mode */
                               debug(F111,"VT","DECRLM - Right to Left Mode",0);
                               break;
                           case 35: /* DECHEBM */
                               /* Enter English keyboard mode */
                               tt_kb_mode = KBM_EN ;
                               break;
                           case 36:    /* DECHEM - Hebrew Encoding Mode */
                               debug(F111,"VT","DECHEM - Hebrew Encoding Mode",0);
                               break;
                           case 38: /* DECTEK */
                               /* Exit Tektronix 4010/4014 mode */
                               resettekmode();
                               break;
                           case 40:
                               /* XTERM - Disallow 80 - 132 mode switch */
                               break;
                           case 41:
                               /* XTERM - No more(1) fix */
                               break;
                           case 42: { /* DECNRCM */
                               /* 8-bit multinational char set */
                               int tcs ;
                               decnrcm = FALSE ;
                               for ( i = 0 ; i < 4 ; i++ ) {
                                   G[i].designation = G[i].def_designation ;
                                   G[i].size = G[i].def_size ;
                                   G[i].c1   = G[i].def_c1 ;
                                   G[i].national = CSisNRC(G[i].designation);
                                   if ( G[i].def_designation == FC_TRANSP ) {
                                       G[i].rtoi = NULL ;
                                       G[i].itol = NULL ;
                                       G[i].itor = NULL ;
                                       G[i].ltoi = NULL;
                                   }
                                   else {
                                       G[i].rtoi = xl_u[G[i].def_designation];
                                       if ( isunicode() ) {
                                           G[i].itol = NULL ;
                                           G[i].ltoi = NULL;
                                       }
                                       else {
                                           G[i].itol = xl_tx[tcsl] ;
                                           G[i].ltoi = xl_u[tcsl] ;
                                       }
                                       G[i].itor = xl_tx[G[i].def_designation];
                                   }
                               }
                           }
                               break;
                           case 43: /* DECGEPM */
                               /* Graphics Normal Print mode */
                               break;
                           case 44: /* DECGPCM */
                               /* Graphics Print Monochrome mode */
                               /* XTERM - Turn off margin bell */
                               break;
                           case 45: /* DECGPCS */
                               /* Graphics Print Color syntax = HLS */
                               /* XTERM - No Reverse-wraparound mode */
                               break;
                           case 46: /* DECGPBM */
                               /* Graphics Print No-Background mode */
                               /* XTERM - Stop Logging */
                               break;
                           case 47: /* DECGRCM */
                               /* Graphics Unrotated Print mode */
                               /* XTERM - Use Normal Screen Buffer */
                               break;
                           case 53: /* DEC131TM */
                               /* VT131 Transmit */
                               break;
                           case 60: /* DECHCCM */
                               /* Horizontal cursor coupling */
                               break;
                           case 61: /* DECVCCM */
                               /* Vertical cursor coupling */
                               break;
                           case 64: /* DECPCCM */
                               /* Page cursor coupling */
                               break;
                           case 66: /* DECNKM */
                               /* Numeric Keyboard - Numeric */
                               tt_keypad = TTK_NORM;
                               break;
                           case 67: /* DECBKM */
                               /* Backarrow key sends DEL */
                               if ( !savdecbkm )
                                   savdecbkm = keymap[KEY_SCAN | 8];
                               keymap[KEY_SCAN | 8] = DEL ;
                               break;
                           case 68: /* DECKBUM */
                               /* Keyboard Usage - Typewriter mode */
                               deckbum = 0 ;
                               break;
                           case 73: /* DECXRLM */
                               /* Transmit rate limiting */
                               break;
                           case 80:        /* WY161 - 80-column mode */
                               if ( ISWY370(tt_type_mode) )
                               {
                                   tt_szchng[VTERM] = 1 ;
                                   tt_cols[VTERM] = 80 ;
                                   VscrnInit( VTERM ) ;  /* Height set here */
#ifdef TCPSOCKET
#ifdef CK_NAWS
                                   if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0){
                                       tn_snaws();
#ifdef RLOGCODE
                                       rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                                       ssh_snaws();
#endif /* SSHBUILTIN */
                                   }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */
                               }
                               break;
                           case 83:        /* WY52 - 24 line mode */
                               if ( ISWY370(tt_type_mode)
                                   && tt_modechg == TVC_ENA )
                               {
                                   tt_szchng[VTERM] = 1 ;
                                   tt_rows[VTERM] = 24 ;
                                   VscrnInit( VTERM ) ;  /* Height set here */
#ifdef TCPSOCKET
#ifdef CK_NAWS
                                   if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0){
                                       tn_snaws();
#ifdef RLOGCODE
                                       rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                                       ssh_snaws();
#endif /* SSHBUILTIN */
                                   }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */
                               }
                               break;
                           case 84:        /* WYENAT */
                               if ( ISWY370(tt_type_mode) )
                                   /* Enable attribute assignment (SGR) */
                                   /* to extend to both eraseable and   */
                                   /* nonerasable characters            */
                                   ;
                               break;
                           case 85:        /* WYREPL */
                               if ( ISWY370(tt_type_mode) )
                                   /* Set replacement character to */
                                   /* color map background color   */
                                   ;
                               break;
                           case 1000:
                               /* XTERM - Don't Send Mouse X&Y on button press and release */
                               break;
                           case 1001:
                               /* XTERM - Don't use Hilite Mouse Tracking */
                               break;
                           default:
                               break;
                           }
                        } else if ( zdsext ) {
                           switch (pn[j]) {
                           /* Zenith/Heath */
                           case 1: /* Disable 25th line */
                               if ( tt_status[VTERM] )
                                   break;
                               settermstatus(TRUE);
                               break;
                           case 2: /* Key Click */
                               setkeyclick(TRUE);
                               break;
                           case 3: /* Exit Hold Screen Mode */
                               break;
                           case 4: /* Underline Cursor */
                               tt_cursor = TTC_ULINE ;
                               setcursormode() ;
                               break;
                           case 5: /* Cursor On */
                               cursorena[VTERM] = TRUE ;
                               break;
                           case 6: /* Keypad Unshifted */
                               tt_shift_keypad = FALSE ;
                               break;
                           case 7: /* Exit Alternate keypad mode */
                               tt_keypad = TTK_NORM ;
                               break;
                           case 8: /* No Auto Line Feed on receipt of CR */
                               tt_crd = FALSE ;
                               break;
                           case 9: /* No Auto CR on receipt of LF */
                               tnlm = FALSE ;
                               break;

                               /* AnnArbor values */
                           case 25:        /* zMDM - Margin Bell Mode */
                               break;
                           case 26:        /* zKCM - Key Click Mode */
                               break;
                           case 27:        /* zKPCM - Key Pad Control Mode */
                               break;
                           case 28:        /* zKRM - Key Repeat Mode */
                               break;
                           case 29:        /* zRLM - Return Line Feed Mode */
                               break;
                           case 30:        /* zDBM - Destructive Backspace Mode */
                               break;
                           case 31:        /* zBKCM - Block Cursor Mode */
                               break;
                           case 32:        /* zBNCM - Blinking Cursor Mode */
                               break;
                           case 33:        /* zWFM - Wrap Forward Mode */
                               break;
                           case 34:        /* zWBM - Wrap Backward Mode */
                               break;
                           case 35:        /* zDDM - DEL character Display Mode */
                               break;
                           case 36:        /* zSPM - Scroll Page Mode */
                               break;
                           case 37:        /* zAXM - Auto Xoff/Xon Mode */
                               break;
                           case 38:        /* zAPM - Auto Pause Mode */
                               break;
                           case 39:        /* zSSM - Slow Scroll Mode */
                               break;
                           case 40:        /* zHDM - Half-Duplex Mode */
                               break;
                           case 41:        /* zTPDM - Transfer Pointer Display Mode */
                               break;
                           case 42:        /* zLTM - Line Transfer Mode */
                               break;
                           case 43:        /* zCSTM - Column Separator Transfer Mode */
                               break;
                           case 44:        /* zFSTM - Field Separator Transfer Mode */
                               break;
                           case 45:        /* zGRTM - Graphic Rendition Transfer Mode */
                               break;
                           case 46:        /* zAKDM - Auto Keyboard Disable Mode */
                               break;
                           case 47:        /* zFRM - Fast Repeat Mode */
                               break;
                           case 48:        /* zHAM - Hold in Area Mode */
                               break;
                           case 49:        /* zGAPM - Guarded Area Print Mode */
                               break;
                           case 51:        /* zACM - Alternate Cursor Mode */
                               break;
                           case 52:        /* zMKM - Meta Key Mode */
                               break;
                           case 53:        /* zCLIM - Caps Lock Invert Mode */
                               break;
                           case 54:        /* zINM - Ignore NUL Mode */
                               break;
                           case 55:        /* zCNM - CR New-Line Mode */
                               break;
                           case 56:        /* zICM - Invisible Cursor Mode */
                               break;
                           case 57:        /* zMMM - Meta Monitor Mode */
                               break;
                           case 59:        /* zIVM - Inverse Video Mode */
                               break;
                           case 61:        /* Z8RCM - 8-bit Remote Copy Mode */
                               break;
                           }
                       } else { /* Not private */
                           if ( ISSCO(tt_type_mode) ) {
                               switch ( pn[j] ) {
                               case 2: /* Keyboard unlocked */
                                   keylock = FALSE;
                                   break;
                               case 3: /* 80 Columns */
                                   RequestScreenMutex(SEM_INDEFINITE_WAIT);
                                   killcursor(VTERM);
                                   deccolm = FALSE;
                                   Set80Cols(VTERM);
#ifdef TCPSOCKET
#ifdef CK_NAWS
                                   if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0) {
                                       tn_snaws();
#ifdef RLOGCODE
                                       rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                                       ssh_snaws();
#endif /* SSHBUILTIN */
                                   }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */
                                   setborder();
                                   newcursor(VTERM);
                                   clrscreen(VTERM,SP);
                                   lgotoxy(VTERM,1,1);       /* and home the cursor */
                                   ipadl25();
                                   ReleaseScreenMutex();
                                   break;
                               case 4: /* Jump scroll */
                                   JumpScroll();
                                   break;
                               case 6: /* Absolute origin */
                                   relcursor = FALSE;
                                   if ( decsasd == SASD_STATUS )
                                       lgotoxy( VSTATUS, 1, 1 );
                                   else
                                       lgotoxy(VTERM,1, 1);
                                   break;
                               case 7: /* Auto wrap */
                                   tt_wrap = FALSE;
                                   break;
                               case 25:   /* Cursor Off */
                               case 48:
                               case 1048:
                                   cursorena[VTERM] = FALSE;
                                   break;
                               case 30:    /* Scrollbars Off */
                                   break;
                               case 44:    /* Margin Bell Off */
                                   marginbell = FALSE;
                                   break;
                               }
                           }
                           else switch (pn[j])
                           {
                           case 2: /* Keyboard unlocked */
                               keylock = FALSE;
                               break;
                           case 3: /* CRM - Controls Mode Off */
                               crm = FALSE ;
                               break;
                           case 4: /* ANSI insert mode */
                               if (ISVT102(tt_type_mode) ||
                                   ISANSI(tt_type_mode))
                                   insertmode = FALSE;
                               break;
                           case 6: /* Erasure Mode (ERM) Reset */
                               erm = 0 ;
                               break;
                           case 12: /* SRM - Send Receive OFF */
                               if ( duplex_sav < 0 )
                                   duplex_sav = duplex ;
                               duplex = 1; /* Local echo */
                               break;
                           case 13:        /* FEAM - Control execution on */
                               break;
                           case 16:        /* TTM - Send through EOL or EOS */
                               break;
                           case 20:
                               /* LNM - linefeed / newline mode */
                               tt_crd = tnlm = FALSE;
                               break;
                           case 30:        /* WYDSCM - Turn display on */
                               if ( ISWY370(tt_type_mode) ) {
                                   screenon = TRUE;
                                   if ( savefiletext[0] ) {
                                       strcpy(filetext,savefiletext);
                                       savefiletext[0] = NUL ;
                                       /* Update status line */
                                       VscrnIsDirty(VTERM);
                                   }
                               }
                               break;
                           case 31:        /* WYSTLINM - Turn off status line */
                               break;
                           case 32:        /* WYCTRSAVM - Screen Saver on */
                               break;
                           case 33:        /* WYSTCURM - Cursor blinking */
                               break;
                           case 34:        /* WYULCURM - Block cursor */
                               if ( ISWY370(tt_type_mode) ) {
                                   tt_cursor = TTC_BLOCK ;
                                   setcursormode() ;
                               }
                               break;
                           case 35:        /* WYCLRM - Width change clear on */
                               break;
                           case 36:        /* WYDELKM - Set delete key to DEL/CAN */
                               break;
                           case 37:        /* WHGATM - Send erasable characters */
                               break;
                           case 38:        /* WYTEXM - Send scrolling region */
                               break;
                           case 40:        /* WYEXTDM - Extra data line */
                               break;
                           case 50:    /* 97801-5XX - Compose key table off */
                               if ( IS97801(tt_type_mode) ) {
                                   /* Make compose key send Compose Key String */
                                   break;
                               }
                               break;
                           case 51:    /* 97801-5XX - Dead key table off */
                               if ( IS97801(tt_type_mode) ) {
                                   break;
                               }
                               break;
                           case 53:    /* 97801-5XX - 80-character mode */
                               if ( IS97801(tt_type_mode) ) {
                                   RequestScreenMutex(SEM_INDEFINITE_WAIT);
                                   killcursor(VTERM);
                                   deccolm = FALSE;
                                   Set80Cols(VTERM);
#ifdef TCPSOCKET
#ifdef CK_NAWS
                                   if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0) {
                                       tn_snaws();
#ifdef RLOGCODE
                                       rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                                       ssh_snaws();
#endif /* SSHBUILTIN */
                                   }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */
                                   setborder();
                                   newcursor(VTERM);
                                   clrscreen(VTERM,SP);
                                   lgotoxy(VTERM,1,1);       /* and home the cursor */
                                   ipadl25();
                                   ReleaseScreenMutex();
                               }
                               break;
                           default:
                               break;
                           }
                       }
                    }
                } /* for */
                break;
            case 'i':       /* Media Copy */
                /*
                  This code crashes the 16-bit version with Stack Overflow.
                  Print-Whole-Screen & Print-Cursor-Line support added in edit 190, fdc.
                */
                if (pn[1] == 0)  /* Print whole screen */
                    prtscreen(VTERM,
                               printregion ?
                               margintop :
                               1,
                               printregion ?
                               marginbot :
                               VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0)
                               );
                else if (pn[1] == 1 &&  /* Print cursor line */
                          /* Only if ESC [?1i */
                          private == TRUE)
                    prtscreen(VTERM,wherey[VTERM],wherey[VTERM]);
                /*
                Send Screen to Host computer
                #******************************************************************************#
                #                                                                              #
                #       Copyright (c) 1991-1993 Maximum Computer Technologies, Inc.            #
                #                           All Rights Reserved.                               #
                #       Version 1.2.0                                       09/15/93           #
                #                                                                              #
                #******************************************************************************#
                NAME        = ansi
                DESCRIPTION = Ansi standard console.
                ALIASES     = li|ansi|ansic|ansi80x25|scoansi|scoansic

                BEGIN_SCREEN_READ_CAPABILITIES

                Send_screen         = \E[2i
                End_send_screen     = \010
                Rcflowcntrloff      = NO
                Rsflowcntrloff      = NO
                Mustreposition      = NO
                Passthrough         = YES
                Mustpruneattribute  = NO

                END_SCREEN_READ_CAPABILITIES
                */
                else if ( pn[1] == 2 && !private ) {
                    int x,y,ch,
                    w=VscrnGetWidth(VTERM),
                    h=VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0),
                    xs=0,
                    ys=0,
                    xe=w-1,
                    ye=h-1;

                    /* Now send the data */
                    for ( y=ys;y<=ye; y++ ) {
                        for ( x=(y==ys)?xs:0 ; x <= (y==ye?xe:w-1) ; x++ ) {
                            ch = VscrnGetCell( VTERM, x, y )->c;
                            if ( tt_senddata ) {
                                    unsigned char * bytes;
                                int nbytes;
                                if ( isunicode() )
                                    nbytes = utorxlat(ch,&bytes);
                                else
                                    nbytes = utorxlat(ch,&bytes);
                                while ( nbytes-- > 0 )
                                    sendchar(*bytes++) ;
                            } else
                                sendchar(SP);
                            }   
                        if ( y < ye ) {
                            sendchar(CR);
                            sendchar(LF);
                        }
                    }
                    sendchar(BS);
                }
                /* For pn = 4 or 5, we should differentiate here between transparent print  */
                /* (private == FALSE) and autoprint (private == TRUE).  As presently coded, */
                /* we always do transparent print.  Autoprint means that the current screen */
                /* line is printed as soon as the cursor leaves it, i.e. when the terminal  */
                /* receives a linefeed, form feed, or vertical tab, or when it wraps.       */
                /* Edit 191.  Now we distinguish between auto-print and transparent print.  */
                else if (pn[1] == 4) {
                    if ( private == TRUE ) {
                        setaprint(FALSE);
                    } else {
                        xprint = FALSE;
                    }
                    if ( is_noprint() )
                        turnoffprinter = TRUE;
                } else if (pn[1] == 5) {
                    if ( private == TRUE ) {
                        setaprint(TRUE);
                    } else {
                        xprint = TRUE ;
                    }
                    if ( !printon )
                        turnonprinter = TRUE;
                    }   

                    /*  6 and 7 are in the VT125 manual.*/
                    /* These are really ANSI Media Copy */
                    /* sequences which should control   */
                    /* the flow of data from the printer */
                    /* port to the host */
                else if (pn[1] == 6) {
                    if ( private == FALSE ) {
                        /* Turn off screen display */
                        screenon = FALSE;
                        if ( !savefiletext[0] )
                            strcpy(savefiletext,filetext);
                        strcpy(filetext,"SCREEN OFF") ;
                        VscrnIsDirty(VTERM);
                    }
                } else if (pn[1] == 7) {
                    if ( private == FALSE ) {
                        /* Turn on screen display */
                        screenon = TRUE;
                        if ( savefiletext[0] ) {
                            strcpy(filetext,savefiletext);
                            savefiletext[0] = NUL ;
                            /* Update status line */
                            VscrnIsDirty(VTERM);
                        }
                    }
                }
                break;
            case 'k':       /* Keyclick */
            if ( pn[1] == 1 )
                    setkeyclick(FALSE);
                else if ( pn[1] == 0 )
                    setkeyclick(TRUE);
                break;
            case 'n':
                switch ( pn[1] ) {
                case 0:
                    /* SCO - Transmit Cursor Position */
                    if (ISSCO(tt_type_mode)) {
                        position * pos = VscrnGetCurPos(VTERM);
                        sprintf(tempstr,
                                 "%d %d\n",
                                 pos->y+1,
                                 pos->x+1
                                 );
                        sendchars(tempstr,strlen(tempstr));
                    }
                    break;
                case 5: /* Terminal Status Report */
                    /* Pages 103-104, VT220 manual */
                    sendescseq("[0n"); /* Terminal is OK */
                    break;
                case 6: {
                    /* Cursor position report (CPR/DECXCPR) */
                    /* Pages 103-104, VT220 manual */
                    position * pos = VscrnGetCurPos(VTERM);
                    if (private)
                        sprintf(tempstr,
                                "[%d;%d;%dR",
                                pos->y+1,
                                pos->x+1,
                                1
                                );
                    else
                        sprintf(tempstr,"[%d;%dR",
                                pos->y+1,
                                pos->x+1
                                );
                    sendescseq(tempstr);
                    break;
                }
#ifdef COMMENT
                case 7:
                    if ( private ) {
                        /* Request Time ("HH:MM:SS"<CR>) */
                        char response[32];
                        sprintf(response,"\"%2d:%2d:%2d\"\r",
                                0,0,0);
                        sendchars(response,strlen(response));
                    }
                    break;
                case 8: /* Request Date ("mmdddyy"<CR>) */
                    if ( private ) {
                        char response[32];
                        sprintf(response,"\"%2d%3s%2d\"\r",
                                1,"jan",80);
                        sendchars(response,strlen(response));
                    }
                    break;
#endif /* COMMENT */
                case 15: {
                    /* DECDSR Printer status */
                    sendescseq("[?10n"); /* Printer is ready */
                    break;
                }
                case 25: {
                    /*
                      DECDSR UDK status;
                      20=unlocked, 21=locked
                      */
                    if ( udklock )
                        sendescseq("[?21n");
                    else
                        sendescseq("[?20n");
                    break;
                }
                case 26: {
                    /*
                      DECDSR Keyboard language, UK or US
                      */
                    sendescseq((G[0].designation == TX_BRITISH) ?
                               "[?27;2n" :
                               "[?27;1n" );
                    break;
                }
                }
                break;
            case 'o': 
                if ( ISSCO(tt_type_mode) ) {
                    /* SCO - Transmit Margin Position */

                    switch ( pn[1] ) {
                    case '0': /* all margins */
                        sprintf(tempstr,
                                 "%d %d %d %d\n",
                                 margintop,
                                 marginbot,
                                 marginleft,
                                 marginright
                                 );
                        sendchars(tempstr,strlen(tempstr));
                        break;
                    case '1':
                        sprintf(tempstr,
                                 "%d\n",
                                 margintop
                                 );
                        sendchars(tempstr,strlen(tempstr));
                        break;
                    case '2':
                        sprintf(tempstr,
                                 "%d\n",
                                 marginbot
                                 );
                        sendchars(tempstr,strlen(tempstr));
                        break;
                    case '3':
                        sprintf(tempstr,
                                 "%d\n",
                                 marginleft
                                 );
                        sendchars(tempstr,strlen(tempstr));
                        break;
                    case '4':
                        sprintf(tempstr,
                                 "%d\n",
                                 marginright
                                 );
                        sendchars(tempstr,strlen(tempstr));
                        break;
                    }
                } else if (ISAAA(tt_type_mode)) {
                    /* DAQ - Define Area Qualification */

                }
                break;
            case 'p':   /* Proprietary */
                if ( ISH19(tt_type) ) {
                    /* Transmit Page to host */
                    break;
                } else if ( ISAAA(tt_type_mode) ) {
                    /* zSDP - Set Display Parameters Pn1;Pn2;...;Pn6 
                     *
                     * Pn1 - Active Memory Lines 
                     * Pn2 - Upper Host Area Lines
                     * Pn3 - Lower Host Area Lines
                     * Pn4 - Screen Lines 
                     * Pn5 - Screen Columns
                     * Pn6 - Active Memory Columns
                     */
                    if ( pn[1] <= pn[2] + pn[3] ||
                         pn[4] <= pn[2] + pn[3] + 1 ||
                         (k > 4) && (pn[6] <= pn[5]))
                        break;

		    /* Kermit is not going to do the right thing 
		     * Since we do not support memory pages different
		     * than the actual screen size, we ignore everything
		     * but the memory lines
		     */
#ifdef KUI
                    tt_linespacing[VTERM] = (CKFLOAT)pn[1] / (CKFLOAT)pn[4];
                    gui_resize_mode(0);
#endif /* KUI */
                    tt_szchng[VTERM] = 1 ;
#ifdef COMMENT
                    tt_rows[VTERM] = pn[4];
#else
                    tt_rows[VTERM] = pn[1];
                    marginbot = pn[4];
#endif /* COMMENT */
                    if (k > 4)
                        tt_cols[VTERM] = pn[5];
                    VscrnInit( VTERM ) ;  /* Height set here */
                    VscrnSetDisplayHeight(VTERM, pn[1] != pn[4] ? pn[4] : 0);
#ifdef TCPSOCKET
#ifdef CK_NAWS
                    if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0){
                        tn_snaws();
#ifdef RLOGCODE
                        rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                        ssh_snaws();
#endif /* SSHBUILTIN */
                    }
#endif /* TCPSOCKET */
#endif /* CK_NAWS */
                    break;
                } else if ( ISSUN(tt_type_mode) ) {
                    /* 
                    ESC[p       Black On White (SUNBOW)
                    Takes no parameters.  Sets the screen  mode  to  black-
                    on-white.   If  the  screen  mode  is already black-on-
                    white, has no effect.  In this mode spaces  display  as
                    solid  white,  other characters as black-on-white.  The
                    cursor is a solid black block.  Characters displayed in
                    negative  image  rendition  (see `Select Graphic Rendi-
                    tion' above) is white-on-black in this mode.   This  is
                    the initial setting of the screen mode on reset.
                    */
                }
                else if ( IS97801(tt_type_mode) ) {
                    switch ( pn[1] ) {
                    case 0: /* Blinking on, lines 1-25 or 1-24 */
                        break;
                    case 1: /* Blinking off, lines 1-25 or 1-24 */
                        break;
                    case 2: /* Blinking on, line 25 */
                        break;
                    case 3: /* Blinking off, line 25 */
                        break;
                    case 5: /* Request abbreviated cursor position */
                        {
                            /* Respond with <FS> <line + 0x20> <col + 0x20>, 0 based */
                            char buf[4];
                            buf[0] = XFS;
                            buf[1] = wherey[1] + SP - 1;
                            buf[2] = wherex[1] + SP - 1;
                            buf[3] = NUL;
                            sendchars(buf,3);
                        }
                        break;
                    case 6: /* Cursor not visible */
                        cursorena[VTERM] = FALSE;
                        break;
                    case 7: /* Activate cursor */
                        cursorena[VTERM] = TRUE;
                        break;
                    case 8: /* Display off */
                        /* This is supposed to fill the viewable      */
                        /* screen with blanks but continue processing */
                        /* additional chars into the display buffer   */
                        screenon = FALSE;
                        if ( !savefiletext[0] )
                            strcpy(savefiletext,filetext);
                        strcpy(filetext,"SCREEN OFF") ;
                        VscrnIsDirty(VTERM);
                        break;
                    case 9: /* Display on */
                        /* Re-enable viewing the display buffer with  */
                        /* real data.                                 */
                        screenon = TRUE;
                        if ( savefiletext[0] ) {
                            strcpy(filetext,savefiletext);
                            savefiletext[0] = NUL ;
                            /* Update status line */
                            VscrnIsDirty(VTERM);
                        }
                        break;
                    case 10: /* Single inverting block cursor */
                        tt_cursor = TTC_HALF;
                        setcursormode();
                        cursorena[VTERM] = TRUE;
                        break;
                    case 11: /* Activate block cursor */
                        tt_cursor = TTC_BLOCK;
                        setcursormode();
                        cursorena[VTERM] = TRUE;
                        break;
                    case 12: /* Activate underscore cursor */
                        tt_cursor = TTC_ULINE;
                        setcursormode();
                        cursorena[VTERM] = TRUE;
                        break;
                    case 13: /* Activate cursor blinking */
                        break;
                    case 14: /* Deactivate cursor blinking */
                        break;
                    }
                }
                break;
            case 'q':   /* Proprietary */
                if ( ISH19(tt_type) ) {
                    /* Transmit 25th line to host */
                    break;
                } else if ( ISSUN(tt_type_mode) ) {
                    /*
                    ESC[q       White On Black (SUNWOB)
                    Takes no parameters.  Sets the screen  mode  to  white-
                    on-black.   If  the  screen  mode  is already white-on-
                    black, has no effect.  In this mode spaces  display  as
                    solid  black,  other characters as white-on-black.  The
                    cursor is a solid white block.  Characters displayed in
                    negative  image  rendition  (see `Select Graphic Rendi-
                    tion' above) is black-on-white in this mode.  The  ini-
                    tial  setting of the screen mode on reset is the alter-
                    native mode, black on white.
                    */
                }
                else if ( IS97801(tt_type_mode) ) {
                    switch ( pn[1] ) {
                    case 0: /* CH.CODE LED off */
                        /* does not affect actual mode */
                        break;
                    case 6: /* CH.CODE LED on */
                        /* does not affect actual mode */
                        break;
                    }
                    break;
                }
                else {
                    /* Load LEDs */
                    for ( i=1 ; i<=k ; i++ ) {
                        switch ( pn[i] ) {
                        case 0:
                            decled[0] = decled[1] =
                                decled[2] = decled[3] = FALSE ;
                            break;
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                            decled[ pn[i]-1 ] = TRUE ;
                            break;
                        case 21:
                        case 22:
                        case 23:
                        case 24:
                            decled[ pn[i]-21 ] = FALSE ;
                            break;
                        }
                    }
                    ipadl25() ;
                }
                break;
            case 'x':   /* Proprietary */
                if ( ISWY370(tt_type_mode) ) {
                    /* Set Color and Attribute */
                    /* pn[1] - 64 color palette - fg */
                    /* pn[2] - 64 color palette - bg */
                    /* pn[3] - attribute             */
                    switch ((pn[3])) {   /* This one... */
                    case 0: /* Set all attributes to normal */
                        if (colorreset)
                            resetcolors(0);

                        attrib.blinking = FALSE;
                        attrib.italic = FALSE;              /* No italic */
                        attrib.bold = FALSE;
                        attrib.invisible = FALSE;
                        attrib.underlined = FALSE;
                        attrib.reversed = FALSE;
                        attrib.graphic = FALSE ;
                        attrib.dim = FALSE ;
                        attrib.wyseattr = FALSE ;
                        attrib.hyperlink = FALSE;
                        attrib.linkid = 0;

                        sco8bit = FALSE ;

                        if ( ISANSI(tt_type_mode) )
                            crm = FALSE ;
                        break;
                    case 1: /* Turn on BOLD */
                        /* No colors involved. */
                        if (attrib.bold) /* Already bold? */
                            break;
                        attrib.bold = TRUE; /* Remember */
                        break;
                    case 2: /* Turn on DIM (QANSI) */
                        /* No colors involved. */
                        if (attrib.dim) /* Already dim? */
                            break;
                        attrib.dim = TRUE; /* Remember */
                        break;
                    case 3: /* Turn on ITALIC (ANSI X3.64) */
                        if ( tt_type_mode == TT_AT386 &&
                             !trueitalic ) {
                            if ( colorreset )
                                resetcolors(0);
                            attribute = italicattribute;
                        }
                        else {
                            if (attrib.italic)
                                break;
                            attrib.italic = TRUE;
                        }
                        break;
                    case 4: /* Turn on UNDERLINE */
                        if ( tt_type_mode == TT_AT386 &&
                             !trueunderline ) {
                            if ( colorreset )
                                resetcolors(0);
                            attribute = underlineattribute;
                        }
                        else {
                            if (attrib.underlined)
                                break;
                            attrib.underlined = TRUE;
                        }
                        break;
                    case 5: /* Turn on Slow BLINK */
                        /* Simulated by background intensity) */
                        if ( attrib.blinking )
                            break;
                        attrib.blinking = TRUE;
                        break;
                    case 6: /* Turn on Fast BLINK */
                        if ( attrib.blinking )
                            break;
                        attrib.blinking = TRUE ;
                        break;
                    case 7: /* Turn on REVERSE VIDEO */
                        if ( tt_type_mode == TT_AT386 ) {
                            if ( colorreset )
                                resetcolors(0);
                            attribute = swapcolors(attribute);
                        }
                        else {
                            if (attrib.reversed)
                                break;
                            attrib.reversed = TRUE;
                        }
                        break;

                        /* 8 - 12 are ANSI X3.64 */
                    case 8: /* Turn on INVISIBLE */
                    case 9: /* Turn on INVISIBLE (QANSI) */
                        attrib.invisible = TRUE; /* see wrtch */
                        break;
                    }
                    if ( !sgrcolors )
                        break;
                    l = sgrcols[(decscnm?pn[2]:pn[1])%8];
                    i = (attribute & 0x8F);
                    attribute = (i | ((l << 4)));
                    l = sgrcols[(decscnm?pn[1]:pn[2])%8];
                    i = (attribute & 0xF8);
                    attribute = (i | l);
                    break;
                }

                /* DECREQTPARM Request Terminal Parameters */
                if (pn[1] > 1)
                    break;
                tempstr[0] = '[';
                tempstr[1] = (pn[1] == 0) ? '2' : '3';
                tempstr[2] = ';';
                tempstr[5] = '2'; /* Nbits = 7 */
                switch (parity) {
                case 0:
                    tempstr[3] = '1'; /* Parity = None */
                    /* Nbits depends on TERMINAL BYTESIZE */
                    if (cmask == 0xFF)
                        tempstr[5] = '1'; /* Nbits = 8 */
                    break;
                case 'e':
                    tempstr[3] = '5'; /* Parity = Even */
                    break;
                case 'o':
                    tempstr[3] = '4'; /* Parity = Odd */
                    break;
                case 'm':
                    tempstr[3] = '3'; /* Parity = Mark */
                    break;
                default:
                    tempstr[3] = '2'; /* Parity = Space */
                    break;
                }
                tempstr[4] = ';';
                switch (speed) {
                case 50:
                    i = 0;
                    break;
                case 75:
                    i = 8;
                    break;
                case 110:
                    i = 16;
                    break;
                case 133:
                    i = 14;
                    break;
                case 150:
                    i = 32;
                    break;
                case 200:
                    i = 40;
                    break;
                case 300:
                    i = 48;
                    break;
                case 600:
                    i = 56;
                    break;
                case 1200:
                    i = 64;
                    break;
                case 1800:
                    i = 72;
                    break;
                case 2000:
                    i = 80;
                    break;
                case 2400:
                    i = 88;
                    break;
                case 3600:
                    i = 96;
                    break;
                case 4800:
                    i = 104;
                    break;
                case 9600:
                    i = 112;
                    break;
                case 19200: /* Really the 19200 value */
                    i=120;
                    break;
                case 38400: /* but it's the highest one */
                    i=128;
                    break;
                case 57600:
                case 115200:
                case 230400:
                case -1: /*  Lie about networks too */
                    i = 128;
                    break;
                default: /* UNKNOWN speed - not true */
                    i = 128;
                    break;
                }
                sprintf(&tempstr[6], ";%d;%d;1;0x", i, i);
                sendescseq(tempstr);
                break;
            case '}':       /* (Why this?) */
            case 'm':
                if ( ansiext && private ) {
                    /* Delete Line Up */
                    VscrnScroll(VTERM,
                                DOWNWARD,
                                0,
                                wherey[VTERM] - 1,
                                1,
                                FALSE,
                                SP);
                    break;
                } else if ( ansiext && ISSCO(tt_type_mode) ) {
                    switch ( pn[1] ) {
                    case 0: /* top margin */
                        margintop = pn[2];
                        break;
                    case 1: /* bottom margin */
                        marginbot = pn[2];
                        break;
                    case 2: /* left margin */
                        marginleft = pn[2];
                        break;
                    case 3: /* right margin */
                        marginright = pn[2];
                        break;
                    }
                } else { /* Select Graphic Rendition (SGR) */
                    for (j = 1; j <= k; ++j) /* Go thru all Pn's */
                        switch ((pn[j])) {   /* This one... */
                        case 0: /* Set all attributes to normal */
                            if (colorreset)
                                resetcolors(0);

                            attrib.blinking = FALSE;
                            attrib.italic = FALSE;              /* No italic */
                            attrib.bold = FALSE;
                            attrib.invisible = FALSE;
                            attrib.underlined = FALSE;
                            attrib.reversed = FALSE;
                            attrib.graphic = FALSE ;
                            attrib.dim = FALSE ;
                            attrib.wyseattr = FALSE ;
                            attrib.hyperlink = FALSE;
                            attrib.linkid = 0;

                            sco8bit = FALSE ;

                            if ( ISANSI(tt_type_mode) )
                                crm = FALSE ;
                            break;
                        case 1:     /* Turn on BOLD */
                            /* No colors involved. */
                            if (attrib.bold) /* Already bold? */
                                break;
                            attrib.bold = TRUE; /* Remember */
                            break;
                        case 2:
                            if ( ISSCO(tt_type_mode) ) {
                                /* Select fg/bg iso colors */
                                if ( !sgrcolors )
                                    break;
                                if (decscnm) {
                                    i = (attribute & 0x88);
                                    attribute = (i | (((pn[j+1] & 0x07) << 4)) | (pn[j+2] & 0x07));
                                    defaultattribute = attribute;
                                    graphicattribute = attribute;
                                    reverseattribute = (i | (((pn[j+2] & 0x07) << 4)) | (pn[j+1] & 0x07));
                                    /* reverseattribute = attribute; */
                                } else {
                                    i = (attribute & 0x88);
                                    attribute = (i | (((pn[j+2] & 0x07) << 4)) | (pn[j+1] & 0x07));
                                    defaultattribute = attribute;
                                    graphicattribute = attribute;
                                    reverseattribute = (i | (((pn[j+1] & 0x07) << 4)) | (pn[j+2] & 0x07));
                                    /* reverseattribute = attribute; */
                                }
                                j += 2;
                            } else {
                                /* Turn on DIM (QANSI) */
                                /* No colors involved. */
                                if (attrib.dim) /* Already dim? */
                                    break;
                                attrib.dim = TRUE; /* Remember */
                            }
                            break;
                        case 3:
                            if ( ISSCO(tt_type_mode) ) {
                                /* Turn on Italic */
                                /* pn[j+1] determines blink or bold background*/
                                if (attrib.italic)
                                    break;
                                attrib.italic = TRUE;
                                j++;
                                break;
                            } else {
                                /* Turn on ITALIC (ANSI X3.64) */
                                if (attrib.italic)
                                    break;
                                attrib.italic = TRUE;
                            }
                            break;
                        case 4:     /* Turn on UNDERLINE */
                            if ( tt_type_mode == TT_AT386 &&
                                !trueunderline ) {
                                if ( colorreset )
                                    resetcolors(0);
                                attribute = underlineattribute;
                            }
                            else {
                                if (attrib.underlined)
                                    break;
                                attrib.underlined = TRUE;
                            }
                            break;
                        case 5:
                            if ( ISSCO(tt_type_mode) ) {
                                /* Turn on Blink Mode */
                                if ( attrib.blinking )
                                    break;
                                attrib.blinking = TRUE;
                            } else {
                                /* Turn on Slow BLINK */
                                /* Simulated by background intensity) */
                                if ( attrib.blinking )
                                    break;
                                attrib.blinking = TRUE;
                            }
                            break;
                        case 6:
                            if ( ISSCO(tt_type_mode) ) {
                                /* Turn off Blink Mode */
                                if ( !attrib.blinking )
                                    break;
                                attrib.blinking = FALSE;
                            } else {
                                /* Turn on Fast BLINK */
                                if ( attrib.blinking )
                                    break;
                                attrib.blinking = TRUE ;
                            }
                            break;
                        case 7: /* Turn on REVERSE VIDEO */
                            if ( tt_type_mode == TT_AT386 ) {
                                if ( colorreset )
                                    resetcolors(0);
                                attribute = swapcolors(attribute);
                            }
                            else {
                                if (attrib.reversed)
                                    break;
                                attrib.reversed = TRUE;
                            }
                            break;

                            /* 8 - 12 are ANSI X3.64 */
                        case 8: /* Turn on INVISIBLE */
                        case 9: /* Turn on INVISIBLE (QANSI) */
                            attrib.invisible = TRUE; /* see wrtch */
                            break;

                        case 10:  /* Select Primary font */
                            if (ISANSI(tt_type_mode) ||
                                ISBEOS(tt_type_mode))
                            {
                                sco8bit = FALSE;
                                crm = FALSE ;
                                attrib.graphic = FALSE ;
                                GR = &G[1];
                                if ( tt_type_mode == TT_AT386 && colorreset ||
                                     ISSCO(tt_type_mode) && scocompat )
                                    resetcolors(0);
                            } else if (ISLINUX(tt_type_mode) ||
                                       ISQANSI(tt_type_mode)) {
                                sco8bit = FALSE;
                                crm = FALSE;
                                if ( Qsaved ) {
                                    int i;
                                    Qsaved = FALSE;
                                    for ( i=0;i<4;i++ )
                                        G[i] = QsavedG[i];
                                }
                            }
                            break;

                        case 11:  /* Display GL Control Characters */
                            /* as Graphic Characters */
                            if ( ISANSI(tt_type_mode) )
                            {
                                sco8bit = FALSE ;
                                crm = TRUE;
                                GR = &G[2];
                                if ( tt_type_mode == TT_AT386 && colorreset ||
                                     ISSCO(tt_type_mode) && scocompat )
                                    resetcolors(0);
                                attribute = graphicattribute ;
                            }
                            else if (ISLINUX(tt_type_mode)) {
                                int i;
                                sco8bit = TRUE ;
                                crm = TRUE;
                                if ( !Qsaved ) {
                                    Qsaved = TRUE;
                                    for ( i=0;i<4;i++ )
                                        QsavedG[i] = G[i];
                                }
                                for ( i=1;i<4;i++ )
                                    charset(cs94,'U',&G[i]);
                            }
                            else if (ISQANSI(tt_type_mode)) {
                                int i;
                                sco8bit = TRUE ;
                                crm = TRUE;
                                if ( !Qsaved ) {
                                    Qsaved = TRUE;
                                    for ( i=0;i<4;i++ )
                                        QsavedG[i] = G[i];
                                }
                                for ( i=1;i<3;i++ )
                                    charset(cs94,'*',&G[i]);
                            }
                            break;

                        case 12:
                            if (ISLINUX(tt_type_mode)) {
                                /*
                                select null mapping (CP437),
                                set display control flag,
                                set toggle meta flag. (The toggle meta flag
                                causes the high bit of a byte to be toggled
                                before the mapping table translation is done.)
                                */
                                int i;
                                sco8bit = TRUE ;
                                crm = TRUE ;
                                attrib.graphic = FALSE ;
                                if ( !Qsaved ) {
                                    Qsaved = TRUE;
                                    for ( i=0;i<4;i++ )
                                        QsavedG[i] = G[i];
                                }
                                for ( i=1;i<4;i++ )
                                    charset(cs94,'U',&G[i]);
                                break;
                            }
                        case 13: /* IBM HFT */
                        case 14: /* IBM HFT */
                        case 15: /* IBM HFT */
                        case 16: /* IBM HFT */
                        case 17: /* IBM HFT */
                            /* Shift High Bit on before displaying */
                            /* Characters */
                            if ( ISANSI(tt_type_mode) ||
                                ISBEOS(tt_type_mode))
                            {
                                sco8bit = TRUE ;
                                crm = FALSE ;
                                attrib.graphic = FALSE ;
                                GR = &G[3];

                                if ( tt_type_mode == TT_AT386 &&
                                    colorreset )
                                    resetcolors(0);
                            } else if (ISQANSI(tt_type_mode)) {
                                int i;
                                sco8bit = TRUE ;
                                crm = FALSE;
                                if ( !Qsaved ) {
                                    Qsaved = TRUE;
                                    for ( i=0;i<4;i++ )
                                        QsavedG[i] = G[i];
                                }
                                for ( i=1;i<3;i++ )
                                    charset(cs94,'U',&G[i]);
                            }

                        case 21: /* Set Normal Intensity */
                            if (attrib.bold)
                                attrib.bold = FALSE;
                            if (attrib.dim)
                                attrib.dim = FALSE;
                            break;
                        case 22: /* Turn BOLD Off */
                            if (attrib.bold)
                                attrib.bold = FALSE;
                            if (attrib.dim)
                                attrib.dim = FALSE;
                            break;
                        case 23: /* Turn ITALIC off */
                            if (!attrib.italic)
                                break;
                            attrib.italic = FALSE;
                            break;

                        case 24: /* Turn UNDERLINE Off */
                            if (!attrib.underlined)
                                break;
                            attrib.underlined = FALSE;
                            /* Go back to normal coloring */
                            break;
                        case 25: /* Turn BLINK Off */
                            if (!attrib.blinking)
                                break;
                            attrib.blinking = FALSE;
                            break;
                        case 26: /* Turn BLINK On */
                            if ( ISSCO(tt_type_mode)) {
                                if (attrib.blinking)
                                    break;
                                attrib.blinking = TRUE;
                            } else {
                                if (!attrib.blinking)
                                    break;
                                attrib.blinking = FALSE;
                            }
                            break;
                        case 27: /* Turn REVERSE VIDEO Off */
                            if (!attrib.reversed)
                                break;
                            attrib.reversed = FALSE;
                            break;
                        case 28:/* Turn INVISIBLE Off */
                        case 29:/* QANSI */
                            if (!attrib.invisible)
                                break;
                            attrib.invisible = FALSE;
                            break;

                        case 30: /* Colors */
                        case 31:
                        case 32:
                        case 33:
                        case 34:
                        case 35:
                        case 36:
                        case 37:
                            /* Select foreground color */
                            if ( !sgrcolors )
                                break;
                            if ( 0 && ISQANSI(tt_type_mode) )
                                l = pn[j] - 30;
                            else
                                l = sgrcols[pn[j] - 30];
                            if (decscnm) {
                                i = (attribute & 0x8F);
                                attribute = (i | ((l << 4)));

                                if ( ISSCO(tt_type_mode) ) {
                                    /* set the default attribute as well */
                                    i = (defaultattribute & 0x8F);
                                    defaultattribute = (i | (l << 4));
                                    i = (graphicattribute & 0x8F);
                                    graphicattribute = (i | (l << 4));
#ifdef COMMENT
                                    i = (reverseattribute & 0x8F);
                                    reverseattribute = (i | (l << 4));
#else
                                    i = (reverseattribute & 0xF8);
                                    reverseattribute = (i | l);
#endif
                                }
                            } else {
                                i = (attribute & 0xF8);
                                attribute = (i | l);

                                if ( ISSCO(tt_type_mode) ) {
                                    /* set the default attribute as well */
                                    i = (defaultattribute & 0xF8);
                                    defaultattribute = (i | l);
                                    i = (graphicattribute & 0xF8);
                                    graphicattribute = (i | l);
#ifdef COMMENT
                                    i = (reverseattribute & 0xF8);
                                    reverseattribute = (i | l);
#else
                                    i = (reverseattribute & 0x8F);
                                    reverseattribute = (i | (l << 4));
#endif
                                }
                            }
                            break;
                        case 38:  /* enable underline option */
                            break;
                        case 39:  /* disable underline option */
                            /* Supported by SCO ANSI */
                            /* QANSI - restore fg color saved with */
                            /* CSI = Pn F                          */
                            if ( !sgrcolors )
                                break;

                            if (decscnm) {
                                l = (defaultattribute & 0x70);
                                i = (attribute & 0x8F);
                                attribute = (i | l);
                            } else {
                                l = defaultattribute & 0x07;
                                i = (attribute & 0xF8);
                                attribute = (i | l);
                            }
                            break;
                        case 40:
                        case 41:
                        case 42:
                        case 43:
                        case 44:
                        case 45:
                        case 46:
                        case 47:
                            /* Select background color */
                            if ( !sgrcolors )
                                break;

                            if ( 0 && ISQANSI(tt_type_mode) )
                                l = pn[j] - 40;
                            else
                                l = sgrcols[pn[j] - 40];
                            if (!decscnm) {
                                i = (attribute & 0x8F);
                                attribute = (i | ((l << 4)));

                                if ( tt_type_mode == TT_SCOANSI ) {
                                    /* set the default attribute as well */
                                    i = (defaultattribute & 0x8F);
                                    defaultattribute = (i | (l << 4));
                                    i = (graphicattribute & 0x8F);
                                    graphicattribute = (i | (l << 4));
#ifdef COMMENT
                                    i = (reverseattribute & 0x8F);
                                    reverseattribute = (i | (l << 4));
#else
                                    i = (reverseattribute & 0xF8);
                                    reverseattribute = (i | l);
#endif
                                }
                            } else {
                                i = (attribute & 0xF8);
                                attribute = (i | l);

                                if ( tt_type_mode == TT_SCOANSI ) {
                                    /* set the default attribute as well */
                                    i = (defaultattribute & 0xF8);
                                    defaultattribute = (i | l);
                                    i = (graphicattribute & 0xF8);
                                    graphicattribute = (i | l);
#ifdef COMMENT
                                    i = (reverseattribute & 0xF8);
                                    reverseattribute = (i | l);
#else
                                    i = (reverseattribute & 0x8F);
                                    reverseattribute = (i | (l << 4));
#endif
                                }
                            }
                            break;
                        case 49:
                            /* Supported by SCO ANSI */
                            /* QANSI - restore bg color saved with */
                            /* CSI = Pn G                          */
                            if ( !sgrcolors )
                                break;

                            if (!decscnm) {
                                l = defaultattribute & 0x70;
                                i = (attribute & 0x8F);
                                attribute = (i | l);
                            } else {
                                l = defaultattribute & 0x07;
                                i = (attribute & 0xF8);
                                attribute = (i | l);
                            }
                            break;
                        case 50:
                            if ( IS97801(tt_type_mode) ) {
                                /* Save Attributes */
                                /* When the "save attributes" command is issued,  */
                                /* all attributes then in effect for the entire   */
                                /* screen are stored.  These attributes will then */
                                /* apply to any new display data (without SGR com-*/
                                /* mand sequences) output to the screen.          */
                                /* The stored attributes can be returned to normal*/
                                /* by commands such as "erase in display with     */
                                /* blanks/NULs; clear attributes". */
                                break;
                            } else if ( ISSCO(tt_type_mode) ) {
                                /*
                                Reset to the original color pair. In backwards
                                compatibility mode, set the normal attribute to white
                                text on a black background, and reverse video to black
                                text on a white background.  In the default (new) mode,
                                set the current attribute according to the current mode
                                and the current values for reverse, normal or graphics
                                */
                            }
                            break;
                        case 51:
                            if ( ISSCO(tt_type_mode) ) {
                                /*
                                Resets all colors to the system default of gray for the
                                foreground and reverse background, and black for the
                                background and reverse foreground.  Only valid in new
                                mode.
                                */
                                break;
                            }
                        case 90: /* Colors */
                        case 91:
                        case 92:
                        case 93:
                        case 94:
                        case 95:
                        case 96:
                        case 97:
                            /* Select foreground color (8-bit high) */
                            if ( !sgrcolors )
                                break;
                            if ( 0 && ISQANSI(tt_type_mode) )
                                l = pn[j] - 90;
                            else
                                l = sgrcols[pn[j] - 90];
                            l += 8;     /* 8th bit high */
                            if (decscnm
#ifdef COMMENT
                                 && !attrib.reversed ||
                                 !decscnm && attrib.reversed
#endif
                                 ) {
                                i = (attribute & 0x8F);
                                attribute = (i | ((l << 4)));

                                if ( tt_type_mode == TT_SCOANSI ) {
                                    /* set the default attribute as well */
                                    i = (defaultattribute & 0x8F);
                                    defaultattribute = (i | (l << 4));
                                    i = (graphicattribute & 0x8F);
                                    graphicattribute = (i | (l << 4));
#ifdef COMMENT
                                    i = (reverseattribute & 0x8F);
                                    reverseattribute = (i | (l << 4));
#else
                                    i = (reverseattribute & 0xF8);
                                    reverseattribute = (i | l);
#endif
                                }
                                } else {
                                    i = (attribute & 0xF8);
                                    attribute = (i | l);

                                    if ( tt_type_mode == TT_SCOANSI ) {
                                        /* set the default attribute as well */
                                        i = (defaultattribute & 0xF8);
                                        defaultattribute = (i | l);
                                        i = (graphicattribute & 0xF8);
                                        graphicattribute = (i | l);
#ifdef COMMENT
                                        i = (reverseattribute & 0xF8);
                                        reverseattribute = (i | l);
#else
                                        i = (reverseattribute & 0x8F);
                                        reverseattribute = (i | (l << 4));
#endif
                                    }
                                }
                            break;
                        case 100:
                        case 101:
                        case 102:
                        case 103:
                        case 104:
                        case 105:
                        case 106:
                        case 107:
                            /* Select background color (8-bit high) */
                            if ( !sgrcolors )
                                break;

                            if ( 0 && ISQANSI(tt_type_mode) )
                                l = pn[j] - 100;
                            else
                                l = sgrcols[pn[j] - 100];
                            l += 8;     /* 8th bit high */
                            if (!decscnm
#ifdef COMMENT
                                 && !attrib.reversed ||
                                 decscnm && attrib.reversed
#endif
                                ) {
                                i = (attribute & 0x8F);
                                attribute = (i | ((l << 4)));

                                if ( tt_type_mode == TT_SCOANSI ) {
                                    /* set the default attribute as well */
                                    i = (defaultattribute & 0x8F);
                                    defaultattribute = (i | (l << 4));
                                    i = (graphicattribute & 0x8F);
                                    graphicattribute = (i | (l << 4));
#ifdef COMMENT
                                    i = (reverseattribute & 0x8F);
                                    reverseattribute = (i | (l << 4));
#else
                                    i = (reverseattribute & 0xF8);
                                    reverseattribute = (i | l);
#endif
                                }
                                } else {
                                    i = (attribute & 0xF8);
                                    attribute = (i | l);

                                    if ( tt_type_mode == TT_SCOANSI ) {
                                        /* set the default attribute as well */
                                        i = (defaultattribute & 0xF8);
                                        defaultattribute = (i | l);
                                        i = (graphicattribute & 0xF8);
                                        graphicattribute = (i | l);
#ifdef COMMENT
                                        i = (reverseattribute & 0xF8);
                                        reverseattribute = (i | l);
#else
                                        i = (reverseattribute & 0x8F);
                                        reverseattribute = (i | (l << 4));
#endif 
                                    }
                                }
                            break;
                        default:
                            break;
                        }
                }
                break;
            case 'r':   /* Proprietary */
                if ( ISH19(tt_type) ) {
                    /* Set Baud Rate to pn[1] */
                    break;
                }
                else if ( ISSUN(tt_type_mode) ) {
                    /* 
                    ESC[#r      Set scrolling (SUNSCRL)
                    Takes one parameter, #  (default  0).   Sets  to  #  an
                    internal  register  which determines how many lines the
                    screen scrolls up when a  line-feed  function  is  per-
                    formed with the cursor on the bottom line.  A parameter
                    of 2 or 3 introduces a small amount of ``jump'' when  a
                    scroll  occurs.   A  parameter  of 34 clears the screen
                    rather than scrolling.  The initial  setting  is  1  on
                    reset.

                    A parameter of zero initiates ``wrap mode'' instead  of
                    scrolling.   In  wrap mode, if a linefeed occurs on the
                    bottom line, the cursor  goes  to  the  same  character
                    position  in  the  top  line  of  the screen.  When any
                    linefeed occurs, the line that the cursor moves  to  is
                    cleared.   This  means  that  no scrolling ever occurs.
                    `ESC [ 1 r' exits back to scroll mode.

                    For more information, see the description of the  Line-
                    feed (CTRL-J) control function above.
                    */
                }
                else if ( ansiext && ISSCO(tt_type_mode) ) {
                    margintop = 1;
                    marginbot = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                    marginleft = 1;
                    marginright = VscrnGetWidth(VTERM);
                    break;
                } else if ( private ) {
                    /* Restore Modes */
                    if ( ISHFT(tt_type_mode) ) {
                        for (j = 1; j <= k; ++j) /* Go thru all Pn's */
                            switch ((pn[j])) {   /* This one... */
                            default:
                                break;
                            }
                    }
                }
                else {
                    /* Set margin (scrolling region) */
                    int h = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                    int w = VscrnGetWidth(VTERM);

                    if ((k < 2) || (pn[2] == 0))
                        pn[2] = h;

                    /* for SCO */
                    if ((k < 3) || (pn[3] == 0))
                        pn[3] = 1;

                    if ((k < 4) || (pn[4] == 0))
                        pn[4] = w;

                    if (pn[1] == 0)
                        pn[1] = 1;

                    /* The 97801 allows the bottom margin include the */
                    /* status line even when it is separated from the */
                    /* main display area.                             */
                    if ( IS97801(tt_type_mode) &&
                         decssdt == SSDT_HOST_WRITABLE )
                        h++;

                    if  ((pn[1] > 0) &&
                         (pn[1] < pn[2]) &&
                         (pn[2] <= h) &&
                         (pn[3] < pn[4]) &&
                         (pn[4] <= w))
                    {
                        setmargins(pn[1], pn[2]);
                        if ( ISSCO(tt_type_mode) ) {
                            marginleft = pn[3];
                            marginright = pn[4];
                            lgotoxy(VTERM, relcursor ? marginleft : 1,
                                     relcursor ? marginbot : 1);
                        } else if ( !IS97801(tt_type_mode) ) {
                            if ( decsasd == SASD_STATUS )
                                lgotoxy( VSTATUS, 1, 1 );
                            else
                                lgotoxy(VTERM, 1, relcursor ? margintop : 1);
                        }
                    }
                    else if (!ISSCO(tt_type_mode)) {
                        if ( pn[1] > 0 &&
                            pn[1] <= VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0) )
                            pn[2] = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0) ;
                        setmargins(pn[1], pn[2]);
                        if ( !IS97801(tt_type_mode) ) {
                            if ( decsasd == SASD_STATUS )
                                lgotoxy( VSTATUS, 1, 1 );
                            else
                                lgotoxy(VTERM, 1, relcursor ?
                                        margintop : 1);
                        }
                    }
                    break;
                }
                break;
            case 'J':
                if ( ansiext && private ) {
                    if ( ISBA80(tt_type_mode) ) {
                        /* Screen Saver On */
                    }
                }
                else if ( ansiext ) {
                    /* ANSI - Set Graphic Foreground Color */
                    if ( ISANSI(tt_type_mode) ) {
                        /* pn[1] contains new color */
                        l = pn[1];
                        i = (graphicattribute & 0xF0);
                        graphicattribute = (i | l);
                    }
                }
                else if ( private == TRUE ) {
                    /* Selective clear screen - DECSED */
                    if ( ISVT220(tt_type_mode) ) {
                        switch ((pn[1])) {
                            /* Selective clear to end of screen */
                        case 0:
                            selclreoscr_escape(VTERM,SP);
                            break;
                            /* Selective clear to beginning */
                        case 1:
                            selclrboscr_escape(VTERM,SP);
                            break;
                            /* Selective clear whole screen */
                        case 2:
                            {
                                /* Cursor does not move */
                                if ( decsasd == SASD_STATUS ) {
                                    selclrscreen(VSTATUS,SP);
                                }
                                else {
                                    selclrscreen(VTERM,SP);
                                }
                            }
                            break;
                        default:
                            break;
                        }
                    }
                } else {
                    /* Erase in Display (ED) */
                    /* ??? For the 97801, the called functions must be */
                    /* recoded to only clear the current scroll region */
                    /* with both vertical and horizontal boundaries.   */
                    if ( IS97801(tt_type_mode) ) {
                        /* ignored if outside scroll region */
                        if ( wherey[VTERM] < margintop ||
                            wherey[VTERM] > marginbot )
                            break;
                    }
                    switch ((pn[1])) {
                    case 0:
                        /* Clear to end of screen w/SP */
                        /* Clear Attributes */
                        if ( IS97801(tt_type_mode) )
                            clreoreg_escape(VTERM,SP);
                        else
                            clreoscr_escape(VTERM,SP);
                        break;
                    case 1:
                        /* Clear to beginning w/SP */
                        /* Clear Attributes */
                        if ( IS97801(tt_type_mode) )
                            clrboreg_escape(VTERM,SP);
                        else
                            clrboscr_escape(VTERM,SP);
                        break;
                    case 2:
                        /* Clear whole screen w/SP */
                        /* Clear Attributes */
                        /* DEC ... */
                        if ( !ISANSI(tt_type_mode) ) {
                            /* Cursor does not move */
                            if ( decsasd == SASD_STATUS ) {
                                clrscreen(VSTATUS,SP);
                            }
                            else {
                                if ( IS97801(tt_type_mode) )
                                    clrregion(VTERM,SP);
                                else
                                    clrscreen(VTERM,SP);
                            }
                        } else { /* ANSI homes the cursor */
                            clrscreen(VTERM,SP);
                            lgotoxy(VTERM,1,1);       /* and home the cursor */
                        }
                        break;
                    case 3:
                        /* Clear to end of screen w/NUL */
                        /* Clear Attributes */
                        if ( IS97801(tt_type_mode) ) {
                            clreoreg_escape(VTERM,NUL);
                        }
                        break;
                    case 4:
                        /* Clear to beginning w/NUL */
                        /* Clear Attributes */
                        if ( IS97801(tt_type_mode) ) {
                            clrboreg_escape(VTERM,NUL);
                        }
                        break;
                    case 5:
                        /* Clear whole screen w/NUL */
                        /* Clear Attributes */
                        /* DEC ... */
                        /* Cursor does not move */
                        if ( IS97801(tt_type_mode) ) {
                            if ( decsasd == SASD_STATUS ) {
                                clrscreen(VSTATUS,NUL);
                            }
                            else {
                                clrregion(VTERM,NUL);
                            }
                        }
                        break;
                    case 6:
                        /* Clear to end of screen w/SP */
                        /* Preset with the last valid attribute */
                        /* (if attributes are not stored) */
                        if ( IS97801(tt_type_mode) ) {
                            clreoreg_escape(VTERM,SP);
                        }
                        break;
                    case 7:
                        /* Clear to beginning w/SP */
                        /* Preset with the last valid attribute */
                        /* (if attributes are not stored) */
                        if ( IS97801(tt_type_mode) ) {
                            clrboreg_escape(VTERM,SP);
                        }
                        break;
                    case 8:
                        /* Clear whole screen w/SP */
                        /* Preset with the last valid attribute */
                        /* (if attributes are not stored) */
                        if ( IS97801(tt_type_mode) ) {
                            if ( decsasd == SASD_STATUS ) {
                                clrscreen(VSTATUS,SP);
                            }
                            else {
                                clrregion(VTERM,SP);
                            }
                        }
                        break;
                    case 9:
                        /* Clear to end of screen w/NUL */
                        /* Preset with the last valid attribute */
                        /* (if attributes are not stored) */
                        if ( IS97801(tt_type_mode) ) {
                            clreoreg_escape(VTERM,NUL);
                        }
                        break;
                    case 10:
                        /* Clear to beginning w/NUL */
                        /* Preset with the last valid attribute */
                        /* (if attributes are not stored) */
                        if ( IS97801(tt_type_mode) ) {
                            clrboreg_escape(VTERM,NUL);
                        }
                        break;
                    case 11:
                        /* Clear whole screen w/NUL */
                        /* Preset with the last valid attribute */
                        /* (if attributes are not stored) */
                        if ( IS97801(tt_type_mode) ) {
                            if ( decsasd == SASD_STATUS ) {
                                clrscreen(VSTATUS,NUL);
                            }
                            else {
                                clrregion(VTERM,NUL);
                            }
                        }
                        break;
                    case 12:
                        /* Clear to end of screen w/SP */
                        /* Preserve attributes */
                        if ( IS97801(tt_type_mode) ) {
                            clreoreg_escape(VTERM,SP);
                        }
                        break;
                    case 13:
                        /* Clear to beginning w/SP */
                        /* Preserve attributes */
                        if ( IS97801(tt_type_mode) ) {
                            clrboreg_escape(VTERM,SP);
                        }
                        break;
                    case 14:
                        /* Clear whole screen w/SP */
                        /* Preserve attributes */
                        if ( IS97801(tt_type_mode) ) {
                            if ( decsasd == SASD_STATUS ) {
                                clrscreen(VSTATUS,SP);
                            }
                            else {
                                clrregion(VTERM,SP);
                            }
                        }
                        break;
                    case 15:
                        /* Clear to end of screen w/NUL */
                        /* Preserve attributes */
                        if ( IS97801(tt_type_mode) ) {
                            clreoreg_escape(VTERM,NUL);
                        }
                        break;
                    case 16:
                        /* Clear to beginning w/NUL */
                        /* Preserve attributes */
                        if ( IS97801(tt_type_mode) ) {
                            clrboreg_escape(VTERM,NUL);
                        }
                        break;
                    case 17:
                        /* Clear whole screen w/NUL */
                        /* Preserve attributes */
                        if ( IS97801(tt_type_mode) ) {
                            if ( decsasd == SASD_STATUS ) {
                                clrscreen(VSTATUS,NUL);
                            }
                            else {
                                clrregion(VTERM,NUL);
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }
                break;
            case 'V':
                if (ISSCO(tt_type_mode)) {
                    /* Erase in Region (ER) */
                    switch ((pn[1])) {
                    case 0:
                        /* Clear to end of screen w/SP */
                        /* Clear Attributes */
                        clreoreg_escape(VTERM,SP);
                        break;
                    case 1:
                        /* Clear to beginning w/SP */
                        /* Clear Attributes */
                        clrboreg_escape(VTERM,SP);
                        break;
                    case 2:
                        /* Clear whole region w/SP */
                        /* Clear Attributes */
                        /* DEC ... */
                        clrregion(VTERM,SP);
                        break;
                    default:
                        break;
                    }
                }
                break;
            case 'v':   /* Proprietary */
                if ( ISAAA(tt_type_mode) ) {
                    /* zSRC - Start Remote Copy */
                    break;
                } else if ( IS97801(tt_type_mode) ) {
                    switch ( pn[1] ) {
                    case 5:
                        /* Switch within G0 (national/international, 7-bit mode) */
                        if ( sni_bitmode == 7 )
                            SNI_chcode(!sni_chcode);
                        break;
                    case 7:
                        /* Full screen mode off */
                        break;
                    case 8:
                        /* Fill scroll area with chracters (diagnostics) */
                        break;
                    case 9:
                        /* Output entire character generator */
                        break;
                    case 10:
                        /* Disable CH.CODE key (7-bit mode) */
                        sni_chcode_7 = 0;
                        break;
                    case 11:
                        /* Enable CH.CODE key (7-bit mode) */
                        sni_chcode_7 = 1;
                        break;
                    case 13: {
                        /* Query current code table */
                        /* 0 - SI: G0FEST active */
                        /* 1 - SI: G0 active     */
                        /* 2 - SO: G0FEST activ  */
                        /* 3 - SO: G0 active     */

                        char chtable[9];
                        char status;
                        if ( GL == &G[0] )
                            status = sni_chcode ? '1' : '0';
                        else
                            status = sni_chcode ? '3' : '2';
                        if ( send_c1 )
                            sprintf(chtable,"%c13v%c%c",_DCS,status,_ST8);
                        else
                            sprintf(chtable,"%cP13v%c%c\\",ESC,status,ESC);
                        sendchars(chtable,strlen(chtable));
                        break;
                    }
                    case 20:
                        /* Switch to alternative key assignment table (8-bit CH.CODE) */
                        if ( sni_bitmode == 8 )
                            SNI_chcode(!sni_chcode);
                        break;
                    case 21: {
                        /* Query current key assignment table (CH.CODE status, 8-bit mode) */
                        char chtable[9];
                        char status;
                        status = sni_chcode ? '1' : '0';
                        if ( send_c1 )
                            sprintf(chtable,"%c21v%c%c",_DCS,status,_ST8);
                        else
                            sprintf(chtable,"%cP21v%c%c\\",ESC,status,ESC);
                        sendchars(chtable,strlen(chtable));
                        break;
                    }
                    case 22:
                        /* Disable CH.CODE key (8-bit mode) */
                        sni_chcode_8 = 0;
                        break;
                    case 23:
                        /* Enable CH.CODE key (8-bit mode) */
                        sni_chcode_8 = 1;
                        break;
                    }
                }
                break;
            case 'w': /* Proprietary */
                if ( ISAAA(tt_type_mode) ) {
                    /* zSPF - Set Print Format Pn1;Pn2;Pn3
                     * Pn1 - Printed Lines (per page) 1 - 126
                     * Pn2 - Total Lines (per page) 1 - 126
                     * Pn3 - Left Margin 0 - 254
                     */
                } 
                else if ( ISHFT(tt_type_mode) ) {
                    /* vt raw keyboard input */
                    ;
                }
                else if ( IS97801(tt_type_mode) ) {
                    switch ( pn[1] ) {
                    case 0:     /* Transmit keylock switch status */
                        break;
                    case 1:     /* Reinitialize character generator */
                        break;
                    }
                }
                else if ( ISWY370(tt_type_mode) )
                {
                    /* All sorts of color controls */
                    switch ( pn[1] ) {
                        /* 0-47 - Defining Color Associations */
                    case 48:
                        /* Select Foreground Color */
                        break;
                    case 49:
                        /* Select Background Color */
                        /* Sets border color to Bg Color */
                        break;
                    case 50:
                        /* Restore Foreground and Background colors */
                        /* sets border color to bg color */
                        break;
                    case 51:
                        /* Select border color */
                        break;
                    case 52:
                        /* Select cursor color */
                        break;
                    case 54:
                        /* Select user status line attrs/colors */
                        break;
                    case 55:
                        /* Select system status line attrs/colors */
                        break;
                    case 56:
                        /* Select replacement chars attrs/colors */
                        break;
                    case 57:
                        /* Select nonerasable chars attrs/colors */
                        break;
                    case 60:
                        /* Restore default color index values */
                        break;
                    case 61:
                        /* Assign current char fg color */
                        /* Turns on color index mode    */
                        break;
                    case 62:
                        /* Assign current char bg color */
                        /* sets border color to bg color */
                        break;
                    case 63:
                        /* Turn color index mode on/off */
                        break;
                    case 64:
                        /* Change current nonerasable char */
                        /* foreground color                */
                        break;
                    case 65:
                        /* Change current nonerasable char */
                        /* background color                */
                        break;
                    case 66:
                        /* Redefine color index value */
                        break;
                    default:
                        break;
                    }
                }
                break;
            case 'K':
                if ( ansiext )
                {
                    /* ANSI - Set Graphic Background Color */
                    if ( ISANSI(tt_type_mode) ) {
                        /* pn[1] contains new color */
                        l = pn[1];
                        i = (graphicattribute & 0x0F);
                        graphicattribute = (i | (l << 4));
                    }
                    else if ( ISBA80(tt_type_mode) ) {
                        /* Begin Reading Function Labels */
                        ba80_fkey_read = -1;
                    }
                }
                else if ( private == TRUE ) {
                    /* Selective clear line - DECSEL */
                    if ( ISVT220(tt_type_mode) ) {
                        switch ((pn[1])) {
                            /* Selective clear to end of line; clear attributes */
                        case 0:
                            selclrtoeoln(VTERM,SP);
                            break;
                            /* Selective clear to beginning; clear attributes */
                        case 1:
                            selclrbol_escape(VTERM,SP);
                            break;
                            /* Selective clear whole cursor line; clear attributes */
                        case 2:
                            selclrline_escape(VTERM,SP);
                            break;
                        default:
                            break;
                        }
                    }
                } else {
                    /* Erase line (EL) */
                    switch ((pn[1])) {
                    case 0:
                        /* Clear to end of line; clear attributes */
                        clrtoeoln(VTERM,SP);
                        break;
                    case 1:
                        /* Clear to beginning; clear attributes */
                        clrbol_escape(VTERM,SP);
                        break;
                    case 2:
                        /* Clear whole cursor line; clear attributes */
                        clrline_escape(VTERM,SP);
                        break;
                    case 3:
                        /* SNI 97801 */
                        /* Erase with NULs to EOL */
                        /* Clear attributes */
                        clrtoeoln(VTERM,NUL);
                        break;
                    case 4:
                        /* SNI 97801 */
                        /* Erase with NULs to BOL */
                        /* Clear attributes */
                        clrbol_escape(VTERM,NUL);
                        break;
                    case 5:
                        /* SNI 97801 */
                        /* Erase with NULs entire line */
                        /* Clear attributes */
                        clrline_escape(VTERM,NUL);
                        break;
                    case 6:
                        /* SNI 97801 */
                        /* Erase with SP to EOL */
                        /* Preset with the last valid SGR attribute */
                        /* (if attributes are not stored) */
                        clrtoeoln(VTERM,SP);
                        break;
                    case 7:
                        /* SNI 97801 */
                        /* Erase with SP to BOL */
                        /* Preset with the last valid SGR attribute */
                        /* (if attributes are not stored) */
                        clrbol_escape(VTERM,SP);
                        break;
                    case 8:
                        /* SNI 97801 */
                        /* Erase with SP entire line */
                        /* Preset with the last valid SGR attribute */
                        /* (if attributes are not stored) */
                        clrline_escape(VTERM,SP);
                        break;
                    case 9:
                        /* SNI 97801 */
                        /* Erase with NUL to EOL */
                        /* Preset with the last valid SGR attribute */
                        /* (if attributes are not stored) */
                        clrtoeoln(VTERM,NUL);
                        break;
                    case 10:
                        /* SNI 97801 */
                        /* Erase with NUL to BOL */
                        /* Preset with the last valid SGR attribute */
                        /* (if attributes are not stored) */
                        clrbol_escape(VTERM,NUL);
                        break;
                    case 11:
                        /* SNI 97801 */
                        /* Erase with NUL entire line */
                        /* Preset with the last valid SGR attribute */
                        /* (if attributes are not stored) */
                        clrline_escape(VTERM,NUL);
                        break;
                    case 12:
                        /* SNI 97801 */
                        /* Erase with SP to EOL */
                        /* Preserve attributes */
                        clrtoeoln(VTERM,SP);
                        break;
                    case 13:
                        /* SNI 97801 */
                        /* Erase with SP to BOL */
                        /* Preserve attributes */
                        clrbol_escape(VTERM,SP);
                        break;
                    case 14:
                        /* SNI 97801 */
                        /* Erase with SP entire line */
                        /* Preserve attributes */
                        clrline_escape(VTERM,SP);
                        break;
                    case 15:
                        /* SNI 97801 */
                        /* Erase with NUL to EOL */
                        /* Preserve attributes */
                        clrtoeoln(VTERM,NUL);
                        break;
                    case 16:
                        /* SNI 97801 */
                        /* Erase with NUL to BOL */
                        /* Preserve attributes */
                        clrbol_escape(VTERM,NUL);
                        break;
                    case 17:
                        /* SNI 97801 */
                        /* Erase with NUL entire line */
                        /* Preserve attributes */
                        clrline_escape(VTERM,NUL);
                        break;
                    default:
                        break;
                    }
                }
                break;
            case 'L':
                if ( ISSCO(tt_type_mode) && ansiext ) {
                    /* SCO - Set Emulation Feature */
                    switch ( pn[1] ) {
                    case 0: /* New regions filled with the current attribute (default) */
                        break;
                    case 1: /* New regions filled with the normal attribute */
                        break;
                    case 2: /* Disable iBCS2 compliance (default) */
                        break;
                    case 3: /* Enable iBCS2 compliance */
                        break;
                    case 4: /* Disable ANSI SGR0 interpretation (default) */
                        break;
                    case 5: /* Enable ANSI SGR0 interpretation */
                        break;
                    case 6: /* Disable backwards compatibility mode (default) */
                        scocompat = FALSE;
                        break;
                    case 7: /* Enable backwards compatibility mode (resets margins too) */
                        scocompat = TRUE;
                        break;
                    case 8: /* Cursor motion bound by region (default) */
                        break;
                    case 9: /* Cursor motion unbound */
                        break;
                    case 10: /* Enable 8-bit keyboard meta mode (default) */
                        break;
                    case 11: /* Disable 8-bit keyboard meta mode */
                        break;
                    case 12: /* Disable debugging for this screen (default) */
                        break;
                    case 13: /* Enable debugging for this screen (do not use) */
                        break;
                    case 14: /* Disable global video loop debugging (default) */
                        break;
                    case 15: /* Enable global video loop debugging (do not use) */
                        break;
                    case 16: /* Enable M6845 frame buffer optimization (default) */
                        break;
                    case 17: /* Disable M6845 frame buffer optimization (debugging only) */
                        break;
                    case 20: /* Disable using ESC for meta (high) bit. */
                        break;
                    case 21: /* Enable using ESC for meta (high) bit. */
                        break;
                    default:
                        break;
                    }
                } else if ( ISANSI(tt_type_mode) && ansiext ) {
                    /* ANSI - Set Fill (Erase?) mode */
                    if ( !pn[1] )
                        /* use current color */
                        erasemode = FALSE ;
                    else
                        /* use normal color */
                        erasemode = TRUE ;
                }
                else {
                    if ( ISVT102(tt_type_mode) ||
                         ISANSI(tt_type_mode)) {
                        /* IL - Insert lines */
                        if ( IS97801(tt_type_mode) ) {
                            /* ignored if outside scroll region */
                            if ( wherey[VTERM] < margintop ||
                                 wherey[VTERM] > marginbot )
                                break;
                        }
                        for (i = 1; i <= pn[1]; ++i) {
                            VscrnScroll(VTERM,
                                         DOWNWARD,
                                         wherey[VTERM] - 1,
                                         marginbot - 1,
                                         1,
                                         FALSE,
                                         SP);
                        }
                    }
                }
                break;
            case 'M':
                if ( ansiext ) {
                    /* ANSI - Report Color Attributes */
                    if ( ISANSI(tt_type_mode) )
                    {
                        char buf[32] ;

                        switch ( pn[1] ) {
                        case 0: /* Normal */
                        case 2: /* Graphic - ??? */
                            sprintf(buf,"%d %d\n",defaultattribute&0x0F,
                                     (defaultattribute&0xF0)>>4);
                            break;
                        case 1: /* Reverse */
                            sprintf(buf,"%d %d\n",
                                     (defaultattribute&0xF0)>>4,
                                     defaultattribute&0x0F);
                            break;
                        default:
                            *buf = '\0';
                        }

                        if ( *buf )
                            sendcharsduplex( buf, strlen(buf), FALSE);
                    } else if ( ISBA80(tt_type_mode) ) {
                        /* Move to Status Line */
                        setdecssdt( SSDT_HOST_WRITABLE );
                        setdecsasd( SASD_STATUS );
                    }
                }
                else {
                    if ( ISVT102(tt_type_mode) ||
                         ISANSI(tt_type_mode)) {
                        /* DL - Delete lines */
                        for (i = 1; i <= pn[1]; ++i) {
                            VscrnScroll(VTERM,
                                         UPWARD,
                                         wherey[VTERM] - 1,
                                         marginbot - 1,
                                         1,
                                         FALSE,
                                         SP);
                        }
                    }
                }
                break;
            case '@':   /* (ICH) Insert characters */
                if ( ISVT102(tt_type_mode) ||
                     ISANSI(tt_type_mode) &&
                     private == FALSE &&
                     ansiext == FALSE ) {
                    blankvcell.c = SP;
                    blankvcell.a = attribute;
                    if (pn[1] > VscrnGetWidth(VTERM) + 1 -
                         wherex[VTERM])
                        pn[1] = VscrnGetWidth(VTERM) + 1 -
                            wherex[VTERM];
                    VscrnScrollRt(VTERM, wherey[VTERM] - 1,
                                   wherex[VTERM] - 1,
                                   wherey[VTERM] - 1,
                                   VscrnGetWidth(VTERM) - 1,
                                   pn[1],
                                   blankvcell
                                   );
                }
                break;
            case 'N':
                if ( ISHFT(tt_type_mode) || ISLINUX(tt_type_mode) || 
                     ISAAA(tt_type_mode)) {
                    /* EF - Erase Field */
                    int start, end, width;
                    blankvcell.c = ' ' ;
                    blankvcell.a = geterasecolor(VTERM) ;
                    start = end = wherex[VTERM];
                    width = VscrnGetWidth(VTERM);
                    if (start > 1) {
                        do {
                            start--;
                        } while ((htab[start] != 'T') &&
                                  (start >= 2));
                    }
                    if (end < width) {
                        do {
                            end++;
                        } while ((htab[end] != 'T') &&
                                  (end < width));
                    }
                    switch ( pn[1] ) {
                    case 0: /* Erase to End of Field (cursor to tab stop) */
                        for ( i = wherex[VTERM] ; i < end ; i++ ) {
                            VscrnWrtCell(VTERM,
                                          blankvcell,
                                          attrib,
                                          wherey[VTERM]-1,
                                          i);
                        }
                        break;
                    case 1: /* Erase From Field Start (tab stop to cursor) */
                        for ( i = start ; i <= wherex[VTERM] ; i++ ) {
                            VscrnWrtCell(VTERM,
                                          blankvcell,
                                          attrib,
                                          wherey[VTERM]-1,
                                          i);
                        }
                        break;
                    case 2: /* Erase All of Field (tab stop to tab stop) */
                        for ( i = start ; i <= end ; i++ ) {
                            VscrnWrtCell(VTERM,
                                          blankvcell,
                                          attrib,
                                          wherey[VTERM]-1,
                                          i);
                        }
                        break;
                    }
                }
                break;
            case 'O':
                if ( ISBA80(tt_type_mode) && ansiext ) {
                    /* Open Virtual Window */
                    /* Format is CSI = O Pa Pn1[:Pn2][;Pa Pn1[:Pn2]][...] O */
                    /* Pa = 1 - Set Height:Width of virtual window */
                    /* Pa = 2 - Set Row:Column of virtual window in relation to */
                    /*          physical window */
                    /* Pa = 4 - Name of the virtual Window (8 chars) */
                    /*          default are 24:80, name is "Lohn    " */
                }
                else if ( ISHFT(tt_type_mode) ||
                          ISLINUX(tt_type_mode) ||
                          ISAAA(tt_type_mode)) {
                    /* EA - Erase Area */
                    switch ( pn[1] ) {
                    case 0: /* Erase to end of area (line) */
                        clrtoeoln(VTERM,SP);
                        break;
                    case 1: /* Erase from area start (line) */
                        clrbol_escape(VTERM,SP);
                        break;
                    case 2: /* Erase all of area (line) */
                        clrline_escape(VTERM,SP);
                        break;
                    }
                }
                break;
            case 'P':   /* (DCH) DeleteChar */
                if ( (ISVT102(tt_type_mode) ||
                       ISANSI(tt_type_mode)) &&
                     private == FALSE &&
                     ansiext == FALSE ) {
                    blankvcell.c = SP;
                    blankvcell.a = geterasecolor(VTERM);
                    if (pn[1] > VscrnGetWidth(VTERM) + 1 -
                         wherex[VTERM])
                        pn[1] = VscrnGetWidth(VTERM) + 1 -
                            wherex[VTERM];
                    VscrnScrollLf(VTERM, wherey[VTERM] - 1,
                                   wherex[VTERM] - 1,
                                   wherey[VTERM] - 1,
                                   VscrnGetWidth(VTERM) - 1,
                                   pn[1],
                                   blankvcell
                                   ) ;
                }
                break;
            case 'Q':   /* SEE - Select Editing Extent */
                if ( ISAAA(tt_type_mode) ) {
                    switch ( pn[1] ) {
                    case 0:     /* Edit in Page (default) */
                        break;
                    case 1:     /* Edit in Line */
                        break;
                    case 2:     /* Edit in Field (between Tab Stops */
                        break;
                    case 3:     /* Edit in Qualified Area (see DAQ, SPA, EPA) */
                        break;
                    }
                }
                break;
            case 'R':   /* CPR - Cursor Position Report */
                /* Sent to host (see DSR), not processed by terminal */
                break;
            case 'S':
                if ( ansiext ) {
                    if (ISBA80(tt_type_mode)) {
                        /* Return from Status Line */
                        setdecsasd(SASD_TERMINAL);
                    }
                } else {
                    /* (SU) Scroll up-scrolls the characters up */
                    /* n lines.  The bottom n lines are    */
                    /* cleared to blanks (default: n=1). */
                    /* Also (RU) Roll up with SNI 97801 */
                    if ( IS97801(tt_type_mode) ) {
                        /* ignored if outside scroll region */
                        if ( wherey[VTERM] < margintop ||
                             wherey[VTERM] > marginbot )
                            break;
                    }
                    if ( pn[1] == 0 )
                        pn[1] = 1 ;
                    else if ( pn[1] > marginbot )
                        pn[1] = marginbot ;
                    
                    if ( ISVT320(tt_type_mode) || IS97801(tt_type_mode) ) {
                        if ( sni_scroll_mode ) {
                            VscrnScroll(VTERM, UPWARD,
                                         margintop-1, wherex[VTERM]-1,
                                         pn[1],
                                         margintop == 1,
                                         SP);
                        } else { /* Roll Mode */
                            VscrnScroll(VTERM, UPWARD,
                                         margintop-1, marginbot-1,
                                         pn[1],
                                         margintop == 1,
                                         SP);
                        }
                    }
                    /* No paged memory, so do nothing */
                    else if (ISHFT(tt_type_mode) || ISANSI(tt_type_mode)) {
                            VscrnScroll(VTERM, UPWARD,
                                         0, VscrnGetHeight(VTERM)-((tt_status)?2:1),
                                         pn[1], TRUE, SP);
                    }
                }
                break;
            case 's':   /* Proprietary */
                if ( private ) {
                    /* Save Modes */
                    if ( ISHFT(tt_type_mode) ) {
                        for (j = 1; j <= k; ++j) /* Go thru all Pn's */
                            switch ((pn[j])) {   /* This one... */
                            default:
                                break;
                            }
                    }
                } 
                else if ( ISAAA(tt_type_mode) ) {
                    /* zPSH - Push Line */
                }
                else if ( ISSUN(tt_type_mode) ) {
                     /*
                     ESC[s       Reset terminal emulator (SUNRESET)
                     Takes no parameters.   Resets  all  modes  to  default,
                     restores  current  font  from  PROM.  Screen and cursor
                     position are
                     */
                    doreset(1);
                }
                else if ( IS97801(tt_type_mode) ) {
                        switch ( pn[1] ) {
                        case 0: /* Repeat off */
                            break;
                        case 1: /* Repeat on */
                            break;
                        case 2: /* Clicker off */
                            setkeyclick(FALSE);
                            break;
                        case 3: /* Clicker on */
                            setkeyclick(TRUE);
                            break;
                        }
                }
                else if ( ISANSI(tt_type_mode) ||
                            IS97801(tt_type_mode) ||
                            ISSCO(tt_type_mode) ) {
                    /* Save Cursor Position */
                    savecurpos(VTERM,0);
                }
                break;
            case '^': /* (SD) Scroll Down (ECMA 48) */
                if ( pn[1] == 0 )
                    pn[1] = 1 ;
                else if ( pn[1] > VscrnGetHeight(VTERM)
                          -((tt_status)?2:1) - margintop )
                    pn[1] = VscrnGetHeight(VTERM)
                        -((tt_status)?2:1) - margintop ;
                if ( debses )
                    break;
                VscrnScroll(VTERM,
                             DOWNWARD,
                             0,
                             VscrnGetHeight(VTERM)
                             -(tt_status[VTERM]?2:1),
                             pn[1],
                             TRUE,
                             SP);
                break;
            case 'T':
                /* (SD) Scroll down-scrolls the characters down */
                /* n lines.  The top n lines are    */
                /* cleared to blanks (default: n=1). */
                /* Also (RD) Roll down with SNI 97801 */
                if ( debses )
                    break;

                if ( IS97801(tt_type_mode) ) {
                    /* ignored if outside scroll region */
                    if ( wherey[VTERM] < margintop ||
                         wherey[VTERM] > marginbot )
                        break;
                }

                if ( pn[1] == 0 )
                    pn[1] = 1 ;
                else if ( pn[1] > VscrnGetHeight(VTERM)
                          -((tt_status)?2:1) - margintop )
                    pn[1] = VscrnGetHeight(VTERM)
                        -((tt_status)?2:1) - margintop ;

                if ( private ) {
                    if ( ISAIXTERM(tt_type_mode) ) {
                        /* Go to column pn[1] of Status Line */
                        setdecssdt(SSDT_HOST_WRITABLE);
                        setdecsasd(SASD_STATUS);
                        lgotoxy(VSTATUS,pn[1],1);
                        break;
                    }
                }
                else if ( ISVT320(tt_type_mode) ||
                          IS97801(tt_type_mode) ) {
                    if ( sni_scroll_mode ) {
                        VscrnScroll(VTERM,
                                     DOWNWARD,
                                     wherex[VTERM]-1,
                                     marginbot-1,
                                     pn[1],
                                     FALSE,
                                     SP);
                    } else { /* Roll Mode */
                        VscrnScroll(VTERM,
                                     DOWNWARD,
                                     margintop-1,
                                     marginbot-1,
                                     pn[1],
                                     FALSE,
                                     SP);
                    }
                }
                /* We don't support paged memory so it does nothing */
                else if (ISHFT(tt_type_mode) || ISANSI(tt_type_mode)) {
                VscrnScroll(VTERM,
                             DOWNWARD,
                             0,
                             VscrnGetHeight(VTERM)
                             -(tt_status[VTERM]?2:1),
                             pn[1],
                             TRUE,
                             SP);
                }
                break;
            case 't':   /* Proprietary */
                if ( ISAAA(tt_type_mode) ) {
                    /* zPOP - Pop Line */
                    break;
                }
                else if ( IS97801(tt_type_mode) ) {
                     switch ( pn[1] ) {
                     case 0: /* Disable ID card reader */
                         break;
                     case 1: /* Enable ID Card reader */
                         break;
                     case 2: {
                         /* Query ID Card reader status */
                         /* @ - ID Card Reader Present */
                         /* A = ID Card Reader broken or missing */
                         char buf[8];
                         if ( debses )
                             break;
                         if ( send_c1 )
                             sprintf(buf,"%cPt%c%c",_DCS,'A',_ST8);
                         else
                             sprintf(buf,"%cPPt%c%c\\",ESC,'A',ESC);
                         sendchars(buf,strlen(buf));
                         break;
                     }
                     }
                }
                else {
                    switch ( pn[1] ) {
                    case 24:    /* DECSLPP - Set screen height */
                    case 25:
                    case 36:
                    case 41:
                    case 42:
                    case 48:
                    case 52:
                    case 53:
                    case 72:
                        if ( tt_modechg == TVC_ENA ) {
                            tt_szchng[VTERM] = 1 ;
                            tt_rows[VTERM] = pn[1] ;
                            VscrnInit( VTERM ) ;  /* Height set here */
#ifdef TCPSOCKET    
#ifdef CK_NAWS      
                                if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0){
                                    tn_snaws();
#ifdef RLOGCODE     
                                    rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                                    ssh_snaws();
#endif /* SSHBUILTIN */
                                }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */
                        }
                        break;

                        /* These are XTERM functions */
                    case 1: /* Deiconify (Restore window) */
#ifdef KUI
                        gui_win_run_mode(1);
#endif
                        break;
                    case 2: /* Iconify (Minimize window) */
#ifdef KUI
                        gui_win_run_mode(2);
#endif
                        break;
                    case 3: /* Position window (Y=Pn[2],X=Pn[3]) */
#ifdef KUI
                        if ( k < 2 ) {
                            pn[2] = 0;
                            pn[3] = 0;
                        } else if ( k < 3 )
                            pn[3] = 0;

                        gui_position(pn[3],pn[2]);
#endif /* KUI */
                        break;
                    case 4: /* Size window in pixels (Y=Pn[2],X=Pn[3]) */
                        /* 0 means leave that dimension alone */
                        if ( k < 2 ) {
                            pn[2] = 0;
                            pn[3] = 0;
                        } else if ( k < 3 )
                            pn[3] = 0;

                        if ( pn[2] == 0 && pn[3] == 0 )
                            break;
#ifdef KUI
                        gui_resize_pixels(pn[3], pn[2]);
#endif /* KUI */
                        break;
                    case 5: /* Raise Window */
                        break;
                    case 6: /* Lower Window */
                        break;
                    case 8: /* Size window in characters (Y=Pn[2],X=Pn[3]) */
                        /* 0 means leave that dimension alone */
                        if ( k < 2 || pn[2] == 0 && pn[3] == 0 )
                            break;
                        if ( k < 3 )
                            pn[3] = 0 ;
                        if ( pn[2] == 0 )
                            pn[2] = tt_rows[VTERM] ;
                        if ( pn[3] == 0 )
                            pn[3] = tt_cols[VTERM] ;
                        if ( tt_modechg == TVC_ENA ) {
                            tt_szchng[VTERM] = 1 ;
                            tt_rows[VTERM] = pn[2] ;
                            tt_cols[VTERM] = pn[3]%2 ? pn[3]+1 : pn[3] ;
                            VscrnInit( VTERM ) ; /* Size is set here */
                            msleep(50);
#ifdef TCPSOCKET    
#ifdef CK_NAWS      
                            if (TELOPT_ME(TELOPT_NAWS) && ttmdm < 0){
                                tn_snaws();
#ifdef RLOGCODE     
                                rlog_naws();
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
                                ssh_snaws();
#endif /* SSHBUILTIN */
                            }
#endif /* CK_NAWS */
#endif /* TCPSOCKET */
                        }
                        break;
                    case 20: /* Report Icon Label */
                        break;
                    case 21: /* Report Window Label */
                        break;
                    case 18: /* Report size of Window in chars */
                        break;
                    case 14: /* Report size of Window in pixels */
                        break;
                    case 13: /* Report position of Window in pixels */
                        break;
                    case 11: /* Report state of Window (normal/iconified) */
                        break;
                    }
                }
                break;
            case 'u':
                if ( ISAAA(tt_type_mode) ) {
                    /* zSTP - Set Transfer Point */
                    break;
                }
                else if ( IS97801(tt_type_mode) ) {
                    switch ( pn[1] ) {
                    case 0: /* 25-line mode on */
                        if ( debses )
                            break;
                        setdecssdt(SSDT_BLANK);
                        margintop = 1;
                        marginbot = VscrnGetHeight(VTERM);
                        break;
                    case 1: /* 24-line mode on */
                        if ( debses )
                            break;
                        setdecssdt(SSDT_HOST_WRITABLE);
                        margintop = 1;
                        marginbot = VscrnGetHeight(VTERM)-1;
                        break;
                    case 2: /* Clear character NUL */
                        break;
                    case 3: /* Clear character SP */
                        break;
                    case 4: /* Video timeout off */
                        break;
                    case 5: /* Video timeout on */
                        break;
                    case 8: /* Page mode on */
                        if ( debses )
                            break;
                        sni_pagemode = TRUE;
                        break;
                    case 9: /* Auto Roll mode on */
                        if ( debses )
                            break;
                        sni_pagemode = FALSE;
                        break;
                    case 10: /* Roll mode */
                        /* Only affects RD/SD - CSI Pn T */
                        if ( debses )
                            break;
                        sni_scroll_mode = FALSE;
                        break;
                    case 11: /* Scroll mode */
                        /* Only affects RD/SD - CSI Pn T */
                        if ( debses )
                            break;
                        sni_scroll_mode = TRUE;
                        break;
                    case 18: /* Reduced-intensity character display */
                        break;
                    case 19: /* Reduced-intensity background display */
                        break;
                    case 20: /* Dark background (white on black) */
                        if (!decscnm) /* Already normal? */
                            break;
                        else {
                            flipscreen(VTERM);
                            VscrnIsDirty(VTERM);
                        }
                        break;
                    case 21: /* Light background (black on white) */
                        if (decscnm) /* Already reverse */
                            break;
                        else {
                            flipscreen(VTERM);
                            VscrnIsDirty(VTERM) ;
                        }
                        break;
                    }
                }
                else if ( ISANSI(tt_type_mode) ||
                          ISHFT(tt_type_mode) ||
                          ISSCO(tt_type_mode) ) {
                    /* Restore Cursor Position */
                    restorecurpos(VTERM,0);
                    break;
                }
                break;
            case 'W':
                if ( ISHFT(tt_type_mode) ||
                     ISLINUX(tt_type_mode) ||
                     ISAAA(tt_type_mode)) {
                    /* CTC - Cursor Tab Stop Control */
                    do {
                        cursordown(0);
                        pn[1] = pn[1] - 1;
                    } while (pn[1] > 0);
                }
                break;
            case 'X':
                if (( ISVT220(tt_type_mode) ||
                      ISHFT(tt_type_mode) ||
                      ISLINUX(tt_type_mode) ||
                      ISANSI(tt_type_mode) || 
                      ISAAA(tt_type_mode)) &&
                     private == FALSE &&
                     ansiext == FALSE ) {
                    /* Erase characters (ECH) VT200 */
                    blankvcell.c = ' ' ;
                    blankvcell.a = geterasecolor(VTERM);
                    if ( pn[1] == 0 ) pn[1] = 1 ;
                    if ( pn[1] >
                         VscrnGetWidth(VTERM) + 1
                         - wherex[VTERM] )
                        pn[1] =
                            VscrnGetWidth(VTERM) + 1
                                - wherex[VTERM] ;
                    for ( i = 0 ; i < pn[1] ; i++ ) {
                        VscrnWrtCell(VTERM,
                                      blankvcell,
                                      blankattrib,
                                      wherey[VTERM]-1,
                                      wherex[VTERM]-1+i
                                      );
                    }
                }
                break;
            case 'Y':
                /* Cursor Vertical Tab */
                if ( ISHFT(tt_type_mode) ) {
                }
                break;
            case 'y':   /* Proprietary */
                if ( ISVIP(tt_type_mode) ) {
                    char vipstr[32] ;
                    sprintf(vipstr,"%c[8p  OT%c%c[y7813  P GC  A %c%c",
                             ESC,ETX,ESC,ETX,NUL);
                    sendchars(vipstr,28);
                }
                else if ( ISHFT(tt_type_mode) ) {
                    /* Virtual Terminal locator report */
                    ;
                }
                else if ( IS97801(tt_type_mode) ) {
                    switch ( pn[1] ) {
                    case 1:
                        /* Execute hardware test for terminal */
                        break;
                    case 2:
                        /* Transmit results of hardware test */
                        break;
                    case 3:
                        /* Activate system test and request result */
                        break;
                    case 4:
                        /* Request firmware version of terminal control */
                        break;
                    case 5:
                        /* Transmit firmware version of keyboard */
                        break;
                    case 11:
                        /* Read current key assignment table */
                        break;
                    case 12:
                        /* Query load process for character generator */
                        break;
                    case 19:
                        /* Transmit board code */
                        break;
                    case 20:
                        /* Activate 8-bit mode   */
                        /* US_ASCII to G0   (GL) */
                        /* Brackets to G1        */
                        /* Blanks   to G2        */
                        /* ISO 8859-1 to G3 (GR) */
                        SNI_bitmode(8);
                        sni_chcode_8 = TRUE;
                        break;
                    case 21:
                        /* Activate 7-bit mode */
                        /* US_ASCII to G0  (GL)  */
                        /* US_ASCII to G0  (GR)  */
                        SNI_bitmode(7);
                        sni_chcode_7 = TRUE;
                        break;
                    case 22: {
                        /* Query active mode */
                        char buf[9];
                        sprintf(buf,"%c22y%c%c",
                                 _DCS,
                                 decnrcm?'7':'8',
                                 _ST8);
                        ttol(buf,6);
                        break;
                    }
                    case 30:
                        /* Query load process for string key table */
                        break;
                    }
                }
                else if ( k == 2 && pn[1] == 2 ) { /* Confidence Tests */
                    if ( pn[2] & 0x01 )
                        /* Power-up self test */
                        ;
                    if ( pn[2] & 0x02 )
                        /* Data Loop Back test */
                        ;
                    if ( pn[2] & 0x04 )
                        /* EIA modem control test */
                        ;
                    if ( pn[2] & 0x08 )
                        /* Repeat selected test(s) indefinitely */
                        ;
                    }
                break;
            case 'Z':
                if ( ISANSI(tt_type_mode) &&
                     ansiext ) {
                    resetcolors(1);
                }
                else {
                    /* Cursor Backward Tabulation (CBT) */
                    /* moves active position back n tabstops */
                    if (k < 1) pn[1] = 1;
                    i = wherex[VTERM];
                    while (pn[1]) {
                        if (i > 1) {
                            do {
                                i--;
                                cursorleft(0);
                            } while ((htab[i] != 'T') && (i >= 2));
                        }
                        pn[1]--;
                    }
                    VscrnIsDirty(VTERM);
                }
                break;
            case 'z':   /* Proprietary */
                if ( ISH19(tt_type) ) {
                    doreset(1) ;
                }
                else if ( ISAAA(tt_type_mode) ) {
                    /* zSTE - Set Transfer End */
                }
                else if ( IS97801( tt_type_mode) ) {
                    /* Define left (pn[1]) and right (pn[2]) margins */
                    /* 1 <= pn[1] < pn[2] <= 80                      */
                    /* This sequence affects only the character that */
                    /* immediately follows this sequence.  That char */
                    /* is used to fill the scroll region using these */
                    /* borders, then the borders revert to 1,80.     */
                    int width = VscrnGetWidth(VTERM);
                    if ( debses )
                        break;
                    if ( pn[1] < 1 || pn[1] > width )
                        pn[1] = 1;
                    if ( pn[2] < 1 || pn[2] > width )
                        pn[2] = width;
                    if ( pn[1] > pn[2] )
                        break;
#ifdef COMMENT
                    /* Not implemented yet. (see above note.) */
                    marginleft = pn[1];
                    marginright = pn[2];
#endif /* COMMENT */
                    break;
                } else if ( ISSCO(tt_type_mode) ) {
                    if ( ansiext ) {
                        /* Clear All Tabs (CAT) */
                        for (j = 1; j <=MAXTERMCOL; ++j)
                            htab[j] = '0';
                    } else {
                        /* Screen Switch - pn[1] specifies screen to switch to */
                    }
                }
                break;
            case ',': {
                achar = (escnext<=esclast)?
                    escbuffer[escnext++]:0;
                switch ( achar ) {
                case 'p':   /* DECLTOD - Load Time of Day */
                    /* pn[1] = hr 00-23; default 8 */
                    /* pn[2] = mn 00-59; default 0 */
                    if ( k < 2 )
                        pn[2] = 0 ;
                    if ( k < 1 )
                        pn[1] = 8 ;
                    loadtod( pn[1], pn[2] ) ;
                    break;
                }
                break;
            }
            case '\'':
                achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                switch (achar) {
                case '~':
                    if ( ISVT420(tt_type_mode) ) {
                        /* DECDC - Delete Column */
                        viocell cell ;
                        cell.c = SP ;
                        cell.a = geterasecolor(VTERM) ;
                        if ( k < 1 || pn[0] == 0 )
                            pn[1] = 1;
                        else if ( pn[1] > VscrnGetWidth(VTERM)-1 )
                            pn[1] = VscrnGetWidth(VTERM)-1 ;

                        VscrnScrollLf( VTERM,
                                       relcursor ? margintop - 1 : 0, /* top row */
                                       wherex[VTERM], /* left col */
                                       relcursor ? marginbot - 1 :
                                       VscrnGetHeight(VTERM)
                                       -(tt_status[VTERM]?2:1), /* bot row */
                                       VscrnGetWidth(VTERM)-1,  /* right col */
                                       pn[1],
                                       cell);
                    }
                    break;
                case '}':
                    if ( ISVT420(tt_type_mode) ) {
                        /* DECIC - Insert Column */
                        viocell cell ;
                        cell.c = SP ;
                        cell.a = geterasecolor(VTERM) ;
                        if ( k < 1 || pn[0] == 0 )
                            pn[1] = 1;
                        else if ( pn[1] > VscrnGetWidth(VTERM)-1 )
                            pn[1] = VscrnGetWidth(VTERM)-1 ;

                        VscrnScrollRt( VTERM,
                                       relcursor ? margintop - 1 : 0, /* top row */
                                       wherex[VTERM], /* left col */
                                       relcursor ? marginbot - 1 :
                                       VscrnGetHeight(VTERM)
                                       -(tt_status[VTERM]?2:1), /* bot row */
                                       VscrnGetWidth(VTERM)-1,  /* right col */
                                       pn[1],
                                       cell);
                    }
                    break;
                }
                break;
            case SP:
            if ( ISSCO(tt_type_mode) ) {
                /* SCO ANSI violates the rules and uses SP as a final character */
                do {
                    cursorleft(0);
                    pn[1] -= 1;
                } while (pn[1] > 0);
                break;
            } else {
                achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                switch (achar) {
                case '@': {
                    /* Scroll Left - Pan Right */
                    viocell cell ;
                    cell.c = SP ;
                    cell.a = geterasecolor(VTERM) ;
                    if ( pn[1] == 0 )
                        pn[1] = 1 ;
                    else if ( pn[1] > VscrnGetWidth(VTERM)-1 )
                        pn[1] = VscrnGetWidth(VTERM)-1 ;
                    VscrnScrollLf( VTERM,
                                   0,
                                   0,
                                   VscrnGetHeight(VTERM)
                                   -(tt_status[VTERM]?2:1),
                                   VscrnGetWidth(VTERM)-1,
                                   pn[1],
                                   cell);
                    break;
                }
                case 'A': {
                    /* Scroll Right - Pan Left */
                    viocell cell ;
                    cell.c = SP ;
                    cell.a = geterasecolor(VTERM) ;
                    if ( pn[1] == 0 )
                        pn[1] = 1 ;
                    else if ( pn[1] > VscrnGetWidth(VTERM)-1 )
                        pn[1] = VscrnGetWidth(VTERM)-1 ;
                        VscrnScrollRt( VTERM,
                                       0,
                                       0,
                                       VscrnGetHeight(VTERM)
                                       -(tt_status[VTERM]?2:1),
                                       VscrnGetWidth(VTERM)-1,
                                       pn[1],
                                       cell);
                        break;
                }
                case 'q':       /* Set Cursor Type - VT520 */
                    switch ( pn[1] ) {
                    case 0:
                    case 1:
                        /* Blinking Block */
                        tt_cursor = TTC_BLOCK ;
                        break;
                    case 2:
                        /*  Steady Block */
                        tt_cursor = TTC_BLOCK ;
                        break;
                    case 3:
                        /* Blinking Underline */
                        tt_cursor = TTC_ULINE ;
                        break;
                    case 4:
                        /* Steady Underline */
                        tt_cursor = TTC_ULINE ;
                        break;
                    }
                    setcursormode() ;
                    cursorena[VTERM] = TRUE;
                    break;
                }
                break;
            } /* SP */
            case '~': {
                if ( kermext ) {
                    /* MS-DOS Kermit PRODUCT macro */
#ifdef CK_APC
                    sprintf(apcbuf,
                             "do product %d %d %d %d %d %d %d %d %d",
                             1<=k?pn[1]:0,
                             2<=k?pn[2]:0,
                             3<=k?pn[3]:0,
                             4<=k?pn[4]:0,
                             5<=k?pn[5]:0,
                             6<=k?pn[6]:0,
                             7<=k?pn[7]:0,
                             8<=k?pn[8]:0,
                             9<=k?pn[9]:0);
                    apc_command(APC_LOCAL,apcbuf);
#endif /* CK_APC */
                }
                else { /* DECTME */
                    switch ( pn[1] ) {
                    case 0:
                    case 1:
                        switch ( tt_type ) {
                        case TT_VT220:
                        case TT_VT220PC:
                        case TT_VT320:
                        case TT_VT320PC:
                        case TT_WY370:
                            tt_type_mode = tt_type;
                            break;
                        default:
                            tt_type_mode = TT_VT320 ;
                            break;
                        }
                        if ( k > 1 && pn[2] == 1 )
                            send_c1 = FALSE ; /* 7-bit cntrols */
                        else
                            send_c1 = TRUE ; /* 8-bit controls */
                        break;
                    case 2:
                        switch ( tt_type ) {
                        case TT_VT100:
                            tt_type_mode = TT_VT100 ;
                            break;
                        default:
                            tt_type_mode = TT_VT102 ;
                            break;
                        }
                        break;
                    case 3:
                        tt_type_mode = TT_VT52;
                        break;
                    case 4:   /* PC TERM */
                        break;
                    case 5:
                        tt_type_mode = TT_WY60 ;
                        break;
                    case 6:   /* WYSE PC TERM */
                        break;
                    case 7:
                        tt_type_mode = TT_WY50 ;
                        break;
                    case 8:   /* WYSE 150/120 */
                        break;
                    case 9:
                        tt_type_mode = TT_TVI950 ;
                        break;
                    case 10:
                        tt_type_mode = TT_TVI925 ;
                            break;
                    case 11:
                        tt_type_mode = TT_TVI910 ;
                        break;
                    case 12:  /* ADDS A2 */
                        break;
                    case 13:
                        tt_type_mode = TT_SCOANSI ;
                        break;
                    case 14:  /* WYSE 325 */
                    break;
                    }
                    /* status line needs to be updated */
                    VscrnIsDirty(VTERM);
                }
                break;
            }
            case '[': {
                if ( ISHFT(tt_type_mode) ) {
                    /* Select Reversed String */
                    switch ( pn[1] ) {
                    case 0: /* End Push */
                        break;
                    case 1: /* Start Push */
                        break;
                    }
                }
                break;
            }
            case ']': {
                if ( ISHFT(tt_type_mode) ) {
                    /* Select Alternate Presentation Variant */
                    for (j = 1; j <= k; ++j) { /* Go thru all Pn's */
                        switch ((pn[j])) {   /* This one... */
                        case 0:
                            /* Set Default Values for BIDI */
                            break;
                        case 1:
                            /* Set Arabic numeric shapes */
                            break;
                        case 2:
                            /* Set Hindi numeric shapes */
                            break;
                        case 3:
                            /* Set symmetric swapping mode */
                            /* for directional characters */
                            break;
                        case 5:
                            /* the following graphic character */
                            /* is presented in its isolated form */
                            /* (Arabic only) */
                            /* effects only the following char */
                            /* unless used with 21 and 22 */
                            break;
                        case 6:
                            /* the following graphic character */
                            /* is presented in it initial form */
                            /* (Arabic only) */
                            /* effects only the following char */
                            /* unless used with 21 and 22 */
                            break;
                        case 7:
                            /* the following graphic character */
                            /* is presented in it middle form */
                            /* (Arabic only) */
                            /* effects only the following char */
                            /* unless used with 21 and 22 */
                        case 8:
                            /* the following graphic character */
                            /* is presented in it final form */
                            /* (Arabic only) */
                            /* effects only the following char */
                            /* unless used with 21 and 22 */
                            break;
                        case 13:
                            /* set Special shaping mode */
                            break;
                        case 14:
                            /* set standard shaping mode */
                            break;
                        case 15:
                            /* set symmetric mode */
                            break;
                        case 18:
                            /* Passthru (everything) */
                            break;
                        case 19:
                            /* Passthru (everything but numbers) */
                            break;
                        case 20:
                            /* Contextual numbers (device dependent) */
                            break;
                        case 21:
                            /* lock 5,6,7,8 */
                            break;
                        case 22:
                            /* unlock */
                            break;
                        case 23:
                            /* set the nonull mode */
                            break;
                        case 24:
                            /* reset the nonull mode */
                            break;
                        default:
                            break;
                        }
                    }
                } else if ( ISLINUX(tt_type_mode) ) {
                    /*
                    ESC [ 1 ; n ]       Set color n as the underline color
                    ESC [ 2 ; n ]       Set color n as the dim color
                    ESC [ 8 ]           Make the current color pair the default attributes.
                    ESC [ 9 ; n ]       Set screen blank timeout to n minutes.
                    ESC [ 10 ; n ]      Set bell frequency in Hz.
                    ESC [ 11 ; n ]      Set bell duration in msec.
                    ESC [ 12 ; n ]      Bring specified console to the front.
                    ESC [ 13 ]          Unblank the screen.
                    ESC [ 14 ; n ]      Set the VESA powerdown interval in minutes.
                    */
                }
                break;
            }
            case '&': {
                achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                switch (achar) {
                case 'x': /* Enable Session */
                    break;
                }
                break;
            }
            default:
                debug(F111,"ANSI X3.64","unknown achar",achar);
                break;
            }       /* third level of left bracket */
        }   /* Esc [ Pn...Pn x   functions */
            break;
        }   /* Second level */

    } /* Left square bracket */
}

void
vtescape( void )
{
    unsigned short  j;
    unsigned short  k;
    unsigned short  l;
    unsigned char   blankcell[2];
    viocell         blankvcell ;
    int             i;
    char            tempstr[20];
    int             fore, back;
    int             tcs ;
    unsigned char   des ;

    /* Initialize default values */
    for (i=0;i<11;i++)
        pn[i]=0;
    private = ansiext = zdsext = kermext = FALSE;

    escstate = ES_NORMAL;               /* Set escape state back to normal */
    escnext = 1;                        /* Point to beginning of esc seq buf */
    if (debses)                         /* If TERMINAL DEBUG ON, */
      return;                           /* we don't actually do anything. */

    achar = (escnext<=esclast)?escbuffer[escnext++]:0;
    if ( !xprint /* && screenon */) {
        /* screen escape sequences  */
        switch (achar) {                /* First Level */
        case '[':      /* CSI */
            vtcsi();
            break;
        case SP:
              achar = (escnext<=esclast)?escbuffer[escnext++]:0;
              if (ISVT220(tt_type_mode)) {
                  switch (achar) {
                  case 'F':  /* S7C1T */
                      send_c1 = FALSE;
                      break;
                  case 'G':  /* S8C1T */
                      send_c1 = TRUE ;
                      break;
                  case '0':     /* WYID */
                      if ( tt_type == TT_WY370 )
                          sendchars("370\r",4);
                      break;
                  case 'L': /* Select ANSI Level 1 */
                      /* ASCII set designated as G0 */
                      /* ISO Latin-1 supplemental set designated as G1 */
                      /* G0 mapped into GL */
                      /* G1 mapped into GR */
                  case 'M': /* Select ANSI Level 2 */
                      /* ASCII set designated as G0 */
                      /* ISO Latin-1 supplemental set designated as G1 */
                      /* G0 mapped into GL */
                      /* G1 mapped into GR */
                  case 'N': /* Select ANSI Level 3 */
                      /* ASCII set designated as G0 */
                      /* G0 mapped into GL */
                      {
                          int tcs;
                          decnrcm = FALSE ;
                          /* we treat all of the conformance levels the same    */
                          /* ISO-4873 does not specify that lower levels cannot */
                          /* support the capabilities of the higher levels      */
                          for ( i = 0 ; i < 4 ; i++ ) {
                              G[i].designation = G[i].def_designation ;
                              G[i].size = G[i].def_size ;
                              G[i].c1   = G[i].def_c1 ;
                              G[i].national = CSisNRC(G[i].designation);
                              if ( G[i].def_designation == FC_TRANSP ) {
                                  G[i].rtoi = NULL ;
                                  G[i].itol = NULL ;
                                  G[i].itor = NULL ;
                                  G[i].ltoi = NULL;
                              }
                              else {
                                  G[i].rtoi = xl_u[G[i].def_designation];
                                  if ( isunicode() ) {
                                      G[i].itol = NULL ;
                                      G[i].ltoi = NULL;
                                  }
                                  else {
                                      G[i].itol = xl_tx[tcsl] ;
                                      G[i].ltoi = xl_u[tcsl] ;
                                  }
                                  G[i].itor = xl_tx[G[i].def_designation];
                              }
                          }
                      }
                      break;
                  case '}':     /* DECKBD */
                      /* Pn[1] specifies the keyboard Layout */
                      /*       0,1, or None - VT             */
                      /*       2            - enhanced PC    */
                      /* Pn[2] indicates the national keyboard language */
                      /*       0,1, or None - North American */
                      /*       2            - British */
                      /*       3            - Belgian (Flemish) */
                      /*       4            - French Canadian */
                      /*       5            - Danish */
                      /*       6            - Finnish */
                      /*       7            - German */
                      /*       8            - Dutch */
                      /*       9            - Italian */
                      /*       10           - Swiss French */
                      /*       11           - Swiss German */
                      /*       12           - Swedish */
                      /*       13           - Norwegian */
                      /*       14           - French */
                      /*       15           - Spanish */
                      /*       16           - Portuguese */
                      /*       19           - Hebrew */
                      /*       22           - Greek */
                      /*       28           - Canadian (English) */
                      /*       29           - Turkish Q/Turkish */
                      /*       30           - Turkish F */
                      /*       31           - Hungarian */
                      /*       33           - Slovak */
                      /*       34           - Czech */
                      /*       35           - Polish */
                      /*       36           - Romanian */
                      /*       38           - SCS */
                      /*       39           - Russian */
                      /*       40           - Latin American */
                      break;
                      if ( k<1 )
                          pn[1] = 1;
                      if ( k<2 )
                          pn[2] = 0;

                      /* We do not support the PC mode yet */
                      if ( pn[1] != 0 && pn[1] != 1 )
                          break;

                      switch ( pn[2] ) {
                      case VTL_NORTH_AM:  /* North American */
                          /* Multinational: Latin-1   */
                          /* National:      US_ASCII  */
                          dec_lang = pn[2];
                          dec_nrc = TX_ASCII;
                          dec_kbd = TX_8859_1;
                          break;
                      case VTL_BRITISH :
                          /* Multinational: Latin-1   */
                          /* National:      UK_ASCII  */
                          dec_lang = pn[2];
                          dec_nrc = TX_BRITISH;
                          dec_kbd = TX_8859_1;
                          break;
                      case VTL_FRENCH  :
                      case VTL_BELGIAN :
                      case VTL_CANADIAN:
                          /* Multinational: Latin-1   */
                          /* National:      FR_ASCII  */
                          dec_lang = pn[2];
                          dec_nrc = TX_FRENCH;
                          dec_kbd = TX_8859_1;
                          break;
                      case VTL_FR_CAN  :
                          /* Multinational: Latin-1   */
                          /* National:      FC_ASCII  */
                          dec_lang = pn[2];
                          dec_nrc = TX_CN_FRENCH;
                          dec_kbd = TX_8859_1;
                          break;
                      case VTL_DANISH  :
                      case VTL_NORWEGIA:
                          /* Multinational: Latin-1   */
                          /* National:      NO_ASCII  */
                          dec_lang = pn[2];
                          dec_nrc = TX_NORWEGIAN;
                          dec_kbd = TX_8859_1;
                          break;
                      case VTL_FINNISH :
                          /* Multinational: Latin-1   */
                          /* National:      FI_ASCII  */
                          dec_lang = pn[2];
                          dec_nrc = TX_FINNISH;
                          dec_kbd = TX_8859_1;
                          break;
                      case VTL_GERMAN  :
                          /* Multinational: Latin-1   */
                          /* National:      GR_ASCII  */
                          dec_lang = pn[2];
                          dec_nrc = TX_GERMAN;
                          dec_kbd = TX_8859_1;
                          break;
                      case VTL_DUTCH   :
                          /* Multinational: Latin-1   */
                          /* National:      DU_ASCII  */
                          dec_lang = pn[2];
                          dec_nrc = TX_DUTCH;
                          dec_kbd = TX_8859_1;
                          break;
                      case VTL_ITALIAN :
                          /* Multinational: Latin-1   */
                          /* National:      IT_ASCII  */
                          dec_lang = pn[2];
                          dec_nrc = TX_ITALIAN;
                          dec_kbd = TX_8859_1;
                          break;
                      case VTL_SW_FR   :
                      case VTL_SW_GR   :
                          /* Multinational: Latin-1   */
                          /* National:      CH_ASCII  */
                          dec_lang = pn[2];
                          dec_nrc = TX_SWISS;
                          dec_kbd = TX_8859_1;
                          break;
                      case VTL_SWEDISH :
                          /* Multinational: Latin-1   */
                          /* National:      SW_ASCII  */
                          dec_lang = pn[2];
                          dec_nrc = TX_SWEDISH;
                          dec_kbd = TX_8859_1;
                          break;
                      case VTL_SPANISH :
                          /* Multinational: Latin-1   */
                          /* National:      SP_ASCII  */
                          dec_lang = pn[2];
                          dec_nrc = TX_SPANISH;
                          dec_kbd = TX_8859_1;
                          break;
                      case VTL_PORTUGES:
                          /* Multinational: Latin-1   */
                          /* National:      Portugese ASCII  */
                          dec_lang = pn[2];
                          dec_nrc = TX_PORTUGUESE;
                          dec_kbd = TX_8859_1;
                          break;
                      case VTL_HEBREW  :
                          /* Multinational: Latin-Hebrew / DEC-Hebrew  */
                          /* National:      DEC 7-bit Hebrew  */
                          dec_lang = pn[2];
                          dec_nrc = TX_HE7;
                          dec_kbd = TX_8859_8;
                          break;
                      case VTL_GREEK   :
                          /* Multinational: Latin-Greek / DEC-Greek   */
                          /* National:      DEC Greek NRC             */
                          /* is ELOT927 equivalent to DEC Greek????   */
                          dec_lang = pn[2];
                          dec_nrc = TX_ELOT927;
                          dec_kbd = TX_8859_7;
                          break;
#ifdef COMMENT
                      case VTL_TURK_Q  :
                      case VTL_TURK_F  :
                          /* Multinational: Latin-Turkish / DEC-Turkish   */
                          /* National:      DEC 7-bit Turkish             */
                          break;
#endif /* COMMENT */
                      case VTL_HUNGARIA:
                          /* Multinational: Latin-2   */
                          /* National:      no national mode  */
                          dec_lang = pn[2];
                          dec_nrc = TX_HUNGARIAN;
                          dec_kbd = TX_8859_2;
                          break;
                      case VTL_SLOVAK  :
                      case VTL_CZECH   :
                      case VTL_POLISH  :
                      case VTL_ROMANIAN:
                          /* Multinational: Latin-2   */
                          /* National:      no national mode  */
                          dec_lang = pn[2];
                          dec_nrc = TX_ASCII;
                          dec_kbd = TX_8859_2;
                          break;
                      case VTL_RUSSIAN :
                          /* Multinational: Latin-Cyrillic / KOI-8   */
                          /* National:      DEC Russian NRC  */
                          dec_lang = pn[2];
                          dec_nrc = TX_KOI7;
                          dec_kbd = TX_8859_5;
                          break;
                      case VTL_LATIN_AM:
                          /* Multinational: not listed in table   */
                          /* National:      not listed in table  */
                          dec_lang = pn[2];
                          dec_nrc = TX_ASCII;
                          dec_kbd = TX_8859_1;
                          break;
#ifdef COMMENT
                      case VTL_SCS     :
                          /* Multinational: Latin-2   */
                          /* National:      SCS NRC   */
                          break;
#endif /* COMMENT */
                      }
                  }
              }
              break;
        case '\\':
              if ( ISH19(tt_type_mode) ) {
                  /* Exit Hold Screen Mode */
              }
              /* String Terminator */
              /*
                 Only here for debugging purposes; really handled in cwrite()
                 Assuming we are in a string; otherwise, we ignore it.
                 */
              break;
        case '@':
            if ( ISH19(tt_type_mode) ) {
                /* Enter Insert Character Mode */
                insertmode = TRUE ;
            }
            break;
        case '1':
                if ( ISH19(tt_type_mode) ) {
                    /* Enter Keypad Shift Mode */
                    tt_shift_keypad = TRUE ;
                    break;
                }
                else if ( ISVT100(tt_type_mode) ) {
                    /* Graphic Processor On */
                }
                break;
        case '2':
              if ( ISVT100(tt_type_mode) ) {
                    /* Graphic Processor Off */
              }
              break;
        case 5:
            /* Wy75 - Transmit Character at Cursor */
            if ( ISAAA(tt_type_mode) ) {
                /* AnnArbor: zTFC - Toggle Fast-Blink Cursor */
            }
            break;
        case '6':  
            if ( ISAAA(tt_type_mode) ) {
                /* AnnArbor: zTI - Toggle IRM */
            } else if (ISVT100(tt_type_mode)) {
                /* DECBI - Back Index */
                if ( wherex[VTERM] > 1 )
                    cursorleft(0);
                else {
                    blankvcell.c = SP;
                    blankvcell.a = geterasecolor(VTERM);
                    VscrnScrollRt(VTERM,
                               wherey[VTERM] - 1,
                               0,
                               wherey[VTERM] - 1,
                               VscrnGetWidth(VTERM) - 1,
                               1, blankvcell);
                }
            }
            break;
        case '7':
            /* AnnArbor - zSC - Save Cursor */
            /* Save cursor position and attributes */
            if ( ISANSI(tt_type_mode) || ISAAA(tt_type_mode) )
                savecurpos(VTERM,0);
            else
                savecurpos(VTERM,1);
            break;
        case '8':
            /* AnnArbor: - zRC - Restore Cursor */
            /* Restore Cursor Position and attributes */
            if ( ISANSI(tt_type_mode) || ISAAA(tt_type_mode) )
                restorecurpos(VTERM,0);
            else
                restorecurpos(VTERM, 1);
            break;
        case '9':  
              if ( ISAAA(tt_type_mode) ) {
                  /* zCGR - Change Graphic Rendition to Qualified Area */
		  /* Default area goes from Current Cursor position to End of Screen */
		  /* Lawrence A Deck - Raytheon 1 Nov 2002 */

		  int start_x, start_y, end_x, end_y, x, y;
		  int h = VscrnGetHeight(VTERM)-(tt_status[vmode]?1:0);
		  int w = VscrnGetWidth(VTERM);
		      
		  start_x = wherex[VTERM];
		  start_y = wherey[VTERM];
		  end_x = w;
		  end_y = h;

		  for ( x=start_x; 
			x < (start_y == end_y ? end_x : MAXTERMCOL); 
			x++ ) {
		      viocell * pcell = VscrnGetCell(VTERM,x-1,start_y-1);
		      pcell->a = attribute;
		      VscrnWrtCell( VTERM, *pcell, attrib,
				    start_y - 1,
				    x - 1);
		  }
		  if ( start_y != end_y ) {
		      for ( y=start_y+1; y<end_y; y++) {
			  for ( x=0; x<MAXTERMCOL; x++ ) {
			      viocell * pcell = VscrnGetCell(VTERM,x-1,y-1);
			      pcell->a = attribute;
			      VscrnWrtCell( VTERM, *pcell, attrib,
					    y - 1,
					    x - 1);
			  }
		      }
		      for ( x=0; 
			    x <= end_x; 
			    x++ ) {
			  viocell * pcell = VscrnGetCell(VTERM,x-1,end_y-1);
			  pcell->a = attribute;
			  VscrnWrtCell( VTERM, *pcell, attrib,
					end_y - 1,
					x - 1);
		      }
		  }
              } else if (ISVT100(tt_type_mode)) { 
                  /* DECFI - Forward Index */
                  if ( wherex[VTERM] < VscrnGetWidth(VTERM) )
                      cursorright(0);
                  else {
                      blankvcell.c = SP;
                      blankvcell.a = geterasecolor(VTERM);
                      VscrnScrollLf(VTERM,
                                 wherey[VTERM] - 1,
                                 0,
                                 wherey[VTERM] - 1,
                                 VscrnGetWidth(VTERM) - 1,
                                 1, blankvcell);
                  }
              }
	    VscrnIsDirty(VTERM);
            break;
        case ':':
            if ( ISAAA(tt_type_mode) ) {
                /* zGUA - Guard Unprotected Areas */
            }
            break;
        case '`':
            if ( ISVIP(tt_type_mode) ) {
                /* Honeywell Clear screen, cursor does not move */
                clrscreen(VTERM,SP);
            }
            else if ( IS97801(tt_type_mode) ||
                      ISHFT(tt_type_mode) || 
                      ISAAA(tt_type_mode) ) {
                /* DMI (Disable Manual Input */
                keylock = TRUE ;
                break;
            }
            break;
        case 'A':
            if ( ISVT52(tt_type_mode) ) /* VT52 control */
                cursorup(0);
            break;
        case 'a':
            if ( ISAAA(tt_type_mode) ) {
                /* INT - INTERRUPT */
            }
            break;
        case 'B':
            if ( ISVT52(tt_type_mode) ) /* VT52 control */
                cursordown(0);
            break;
        case 'b':
            if ( ISH19(tt_type_mode) ) {
                /* Erase Beginning of Display */
                clrboscr_escape(VTERM,SP);
            }
            else if ( IS97801(tt_type_mode) ||
                      ISHFT(tt_type_mode) ||
                      ISAAA(tt_type_mode)) {
                /* EMI (Enable Manual Input) */
                keylock = FALSE ;
                break;
            }
            break;
        case 'C':
            if ( ISVT52(tt_type_mode) ) /* VT52 control */
                cursorright(0);
            break;
        case 'c':
            /* Reset - RIS */
            doreset(1);
            break;
        case 'D':
            if ( ISVT52(tt_type_mode) ) /* VT52 control */
                cursorleft(0);
            else if ( ISVT100(tt_type_mode) ) {/* IND - Index */
                if (wherey[VTERM] == marginbot) {
                    VscrnScroll(VTERM,
                                 UPWARD,
                                 margintop - 1,
                                 marginbot - 1,
                                 1,
                                 (margintop==1),
                                 SP);
                } else
                    cursordown(0);
            }
            break;
        case 'E':
            if ( ISH19(tt_type_mode) ) {
                /* Clear Display (Shift Erase) */
                clrscreen(VTERM,SP);
                lgotoxy(VTERM,1,1);       /* and home the cursor */
            }
            else {                      /* NEL - Next Line */
                cursornextline();
            }
            break;
        case 'F':
            if ( ISVT52(tt_type_mode) ) /* VT52 control */
                vt52graphics = TRUE;
            else if ( ISVT220(tt_type_mode) ) {
                /* SSA - Start Selected Area */
                ;
            }
            break;
        case 'G':
            if ( ISVT52(tt_type_mode) ) /* VT52 control */
                vt52graphics = FALSE;
            else if ( ISVT220(tt_type_mode) ) {
                /* ESA - End selected area */
                ;
            }
            break;
        case 'H':
              if ( ISVT100(tt_type_mode) ) {
                  /* HTS - Set Tab Stop */
                  htab[wherex[VTERM]] = 'T';
              }
              else if ( ISVT52(tt_type_mode) )
                  /* Cursor Home */
                  lgotoxy(VTERM,1, 1);
              break;
        case 'I':                       
            if ( ISAAA(tt_type_mode) ) {
                /* HTJ - Horizontal Tab with Justify */
                ;
            } else if (ISVT52(tt_type_mode)) {
                /* Reverse Linefeed */
                if (margintop == wherey[VTERM])
                    VscrnScroll(VTERM,
                                 DOWNWARD,
                                 margintop - 1,
                                 marginbot - 1,
                                 1,
                                  FALSE,
                                  SP
                                  );
                else
                    cursorup(0);
            }
            break;
        case 'J':
            if (ISVT52(tt_type_mode))
                /* VT52 control */
                clreoscr_escape(VTERM,SP);
            break;
        case 'j':
            if ( ISH19(tt_type_mode) ) {
                /* H19 - Save Cursor Position */
                savecurpos(VTERM,0);
            }
            break;
        case 'K':
            if (ISVT52(tt_type_mode))
                /* VT52 control */
                clrtoeoln(VTERM,SP);
            break;
        case 'k':
            if (ISH19(tt_type_mode)) {
                /* H19 - Restore Cursor Position */
                restorecurpos(VTERM,0);
            }
            break;
        case 'L':
            if ( ISH19(tt_type_mode) ) {
                /* H19 - Insert Line */
                VscrnScroll(VTERM,
                             DOWNWARD,
                             wherey[VTERM] - 1,
                             marginbot - 1,
                             1,
                             FALSE,
                             SP);
            }
            break;
        case 'l':
              if ( ISH19(tt_type_mode) ) {
                  /* Erase Entire Line */
                  clrline_escape(VTERM,SP);
              } else if ( ISSCO(tt_type_mode) ) {
                  /* Lock Memory Area */
                  setmargins(wherey[VTERM],VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0));
                  lgotoxy(VTERM, relcursor ? marginleft : 1,
                           relcursor ? marginbot : 1);
              }
              break;
        case 'M':                       
            /* RI - Reverse Index, VT102 */
            if (ISVT100(tt_type_mode)) {
                if (margintop == wherey[VTERM])
                    VscrnScroll(VTERM,
                                 DOWNWARD,
                                 margintop - 1,
                                 marginbot - 1,
                                 1,
                                  FALSE,
                                  SP
                                  );
                else
                    cursorup(0);
            }
            else if ( ISH19(tt_type_mode) && achar == 'M' ) {
                /* H19 - Delete Line */
                VscrnScroll(VTERM,
                             UPWARD,
                             wherey[VTERM] - 1,
                             marginbot - 1,
                                 1,
                                 FALSE,
                                 SP);
            }
            break;
        case 'm':
              if ( ISSCO(tt_type_mode) ) {
                  /* Unlock Memory Area */
                  setmargins(1,VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0));
                  lgotoxy(VTERM, 1, 1);
              }
              break;
        case 'p':
            if ( ISH19(tt_type_mode) ) {
                /* Enter Reverse Video Mode */
                attrib.reversed = TRUE ;
            }
            break;
        case 'Q':               
            /* SCOANSI/AT386 Fn key assignments */
            /* Normally PU1 String.  SCOANSI break ANSI X.64-1979 */
            if ( ISANSI(tt_type_mode) )
            {
                int fn,quote, next, control=0 ;

                fn = ttinc(0) - '0' ;   /* get the Fn number */
                if ( fn < 0 ) {         /* illegal Fn */
					le_putchar(fn);
                    break;
				}

                if ( !savefiletext[0] )
                    strcpy(savefiletext,filetext);
                strcpy( filetext, "ANSI FN" ) ;
                VscrnIsDirty(VTERM) ;

                quote = ttinc(0) ;              /* get the Quote char */
#ifdef CK_APC
                apclength = 0 ;
                while ( (next = ttinc(0)) != quote &&
                        apclength < apcbuflen ) {
                    if ( control )
                    {
                        apcbuf[apclength++] = next-32 ;
                        control = 0 ;
                    }
                    else
                    {
                        if ( next == '^' )
                            control = 1 ;
                        else
                            apcbuf[apclength++] = next ;
                    }
				}
                apcbuf[apclength] = '\0' ;
#else /* CK_APC */
                while ( ttinc(0) != quote );
#endif /* CK_APC */
                if (savefiletext[0]) {          /* Fix status line */
                    strcpy(filetext,savefiletext);
                    savefiletext[0] = NUL;
                    VscrnIsDirty(VTERM);  /* status line needs to be updated */
                }

#ifdef CK_APC
#ifndef NOSETKEY
#ifndef NOKVERBS
                if ( next == quote )
                {
                    /* assign the string to the function key */
                    if ( udkfkeys[fn] ) {
                        free(udkfkeys[fn]) ;
                        udkfkeys[fn]=NULL;
                    }
                    if ( strlen(apcbuf) )
                        udkfkeys[fn] = strdup(apcbuf) ;
                }
#endif /* NOKVERBS */
#endif /* NOSETKEY */
#endif /* CK_APC */
            }
            break;
        case 'q':
            if ( ISH19(tt_type_mode) ) {
                /* Exit Reverse Video Mode */
                attrib.reversed = FALSE ;
            }
            break;
        case 'r':
            if ( ISH19(tt_type_mode) ) {
                /* Modify Baud Rate */
                /* we don't do this, just eat the next character */
                achar = (escnext<=esclast)?escbuffer[escnext++]:0;
            }
            break;
        case 'S':
            if ( ISAAA(tt_type_mode) ) {
                /* STS - Set Transmit State */
            }
            break;
/*
  VT52 printer controls.  For now, autoprint is treated like transparent
  print.  32-bit version only, to avoid stack overflows in 16-bit version.
  Edit 190, fdc.
*/
        case 'u':
                if ( ISH19(tt_type_mode) ) {
                    /* Exit Keypad Shift Mode */
                    tt_shift_keypad = FALSE ;
                    break;
                }
        case 'V':                       
            if (ISVT52(tt_type_mode)) {/* VT52 control */
                /* Print cursor line */
                prtscreen(VTERM,wherey[VTERM],wherey[VTERM]);
            } else if ( ISAAA(tt_type_mode) ) {
                /* SPA - Start Protected Area */
            }
            break;
        case 'v':
                if ( ISH19(tt_type_mode) ) {
                    /* Wrap Around at End of Line */
                    tt_wrap = TRUE ;
                }
                break;
        case 'W':                       
            if (ISVT52(tt_type_mode)) { /* VT52 control */
                /* Transparent print on */
                turnonprinter = xprint = TRUE;
            } else if ( ISAAA(tt_type_mode) ) {
                /* EPA - End Protected Area */
            }
            break;
        case 'w':
                if ( ISH19(tt_type_mode) ) {
                    /* Discard at End of Line */
                    tt_wrap = FALSE ;
                }
                break;
        case 'X':                       /* Transparent print off */
            if (ISVT52(tt_type_mode)) { /* VT52 control */
                turnoffprinter = TRUE;
                xprint = FALSE ;
            }
            break;
        case 'x':
            if ( ISH19(tt_type_mode) ) {
                /* Set Modes */
                achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                switch ( achar ) {
                case '1':       /* Enable 25th line */
                    if ( !tt_status[VTERM] )
                        break;
                    settermstatus(FALSE);
                    break;
                case '2':       /* No Key Click */
                    setkeyclick(FALSE);
                    break;
                case '3':       /* Hold Screen Mode */
                    break;
                case '4':       /* Block Cursor */
                    tt_cursor = TTC_BLOCK ;
                    setcursormode() ;
                    break;
                case '5':       /* Cursor Off */
                    cursorena[VTERM] = FALSE ;
                    break;
                case '6':       /* Keypad Shifted */
                    tt_shift_keypad = TRUE ;
                    break;
                case '7':       /* Alternate keypad mode */
                    tt_keypad = TTK_APPL ;
                    break;
                case '8':       /* Auto Line Feed on receipt of CR */
                    tt_crd = TRUE ;
                    break;
                case '9':       /* Auto CR on receipt of LF */
                    tnlm = TRUE ;
                    break;
                }
            }
            break;
        case 'Y':                       /* Direct cursor address */
            if (ISVT52(tt_type_mode)) { /* VT52 control */
                int row,column ;
                achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                row = achar - 31;
                achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                column = achar - 31;
                lgotoxy(VTERM,column, row);
            }
            break;
        case 'y':
            if ( ISH19(tt_type_mode) ) {
                /* Reset Modes */
                achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                switch ( achar ) {
                case '1':       /* Disable 25th line */
                    if ( tt_status[VTERM] )
                        break;
                    settermstatus(TRUE);
                    break;
                case '2':       /* Key Click */
                    setkeyclick(TRUE);
                    break;
                case '3':       /* Exit Hold Screen Mode */
                    break;
                case '4':       /* Underline Cursor */
                    tt_cursor = TTC_ULINE ;
                    setcursormode() ;
                    break;
                case '5':       /* Cursor On */
                    cursorena[VTERM] = TRUE ;
                    break;
                case '6':       /* Keypad Unshifted */
                    tt_shift_keypad = FALSE ;
                    break;
                case '7':       /* Exit Alternate keypad mode */
                    tt_keypad = TTK_NORM ;
                    break;
                case '8':       /* No Auto Line Feed on receipt of CR */
                    tt_crd = FALSE ;
                    break;
                case '9':       /* No Auto CR on receipt of LF */
                    tnlm = FALSE ;
                    break;
                }
            }
            break;
        case 'Z':                       /* DECID - Identify Terminal */
            if ( ISVT52(tt_type_mode) ) {
                sendescseq(tt_info[TT_VT52].x_id);
            }
            else if ( ISVT220(tt_type) ) {
                sendescseq(tt_info[tt_type].x_id);
            }
            else if (tt_type_mode >= 0 && tt_type_mode <= max_tt) {
                sendescseq(tt_info[tt_type_mode].x_id);
            }
            break;
        case 'z':
            if ( ISH19(tt_type_mode) ) {
                doreset(1) ;
            }
            break;
        case ']':
            if ( ISH19(tt_type_mode) ) {
                /* Transmit 25th line to host */
                ;
            }
            else if (ISVT52(tt_type_mode)) /* VT52 control */
                /* Print screen */
                prtscreen(VTERM,1,VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0));
            break;
        case '^':                       /* Autoprint on */
              if (ISVT52(tt_type_mode)) {/* VT52 control */
                  turnonprinter = TRUE;
                  setaprint(TRUE);
              }
            break;
        case '_':                       /* Autoprint off */
            if (ISVT52(tt_type_mode)) { /* VT52 control */
                turnoffprinter = TRUE;
                setaprint(FALSE);
            }
            break;
        case '#':
            if ( ISH19(tt_type_mode) ) {
                /* Transmit Page (lines 1-24) to Host */
                break;
            }
            /* Esc # sequence */
            achar = (escnext<=esclast)?escbuffer[escnext++]:0;
            switch (achar) {
            case '3': /* DECDHL */
                decdwl_escape(VT_LINE_ATTR_DOUBLE_WIDE |
                               VT_LINE_ATTR_DOUBLE_HIGH |
                                  VT_LINE_ATTR_UPPER_HALF );
                break;
            case '4': /* DECDHL */
                decdwl_escape(VT_LINE_ATTR_DOUBLE_WIDE |
                                  VT_LINE_ATTR_DOUBLE_HIGH |
                                  VT_LINE_ATTR_LOWER_HALF);
                break;
            case '5': /* DECSWL */
                decdwl_escape(VT_LINE_ATTR_NORMAL);
                break;
            case '6': /* DECDWL */
                decdwl_escape(VT_LINE_ATTR_DOUBLE_WIDE);
                break;
            case '7': /* Hardcopy (vt100) */
                break;
            case '8': /* Screen Alignment Display */
                {
                    videoline * line ;
                    viocell cell;
                    short x,y ;

                    cell.c = 'E';
                    cell.a = defaultattribute; /* was 0x07 */
                    /* Self Test */
                    /* 24 lines of MAXTERMCOL cols of cells */
                    for ( y=0 ; y < 24 ; y++ ) {
                        line = VscrnGetLineFromTop(VTERM, y) ;
                        for ( x=0 ; x < MAXTERMCOL ; x++ ) {
                            line->cells[x] = cell ;
                            line->vt_char_attrs[x] = VT_CHAR_ATTR_NORMAL;
                        }
                        line->vt_line_attr = VT_LINE_ATTR_NORMAL;
                    }
                    setmargins(1, VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0));
                    if ( decsasd == SASD_STATUS )
                        lgotoxy( VSTATUS, 1, 1 );
                    else
                        lgotoxy(VTERM,1, 1);
                    VscrnIsDirty(VTERM);
                }
                break;
            case ':':   /* WYDHL */
                decdwl_escape(VT_LINE_ATTR_DOUBLE_HIGH |
                               VT_LINE_ATTR_UPPER_HALF);
                break;
            case ';':   /* WYDHL */
                decdwl_escape(VT_LINE_ATTR_DOUBLE_HIGH |
                               VT_LINE_ATTR_LOWER_HALF);
                break;
            default:
                break;
            } /* Esc # sequence */
            break;

        case '=':                       /* Application mode */
            tt_keypad = TTK_APPL;
            break;
        case '>':                       /* Numeric mode */
            tt_keypad = TTK_NORM;
            break;
        case '<':                       /* Enter ANSI mode */
            /* VT52 control */
            if (ISVT52(tt_type_mode) && tt_type != TT_VT52 ) {
                if ( ISH19(tt_type) )
                    tt_type_mode = TT_VT100 ;
                else {
#ifdef COMMENT
                    tt_type_mode = tt_type_vt52;
#else /* COMMENT */
                    /* As per dickey@clark.net */
                    tt_type_mode = TT_VT102 ;
#endif /* COMMENT */
                }
                ipadl25();
            }
            break;

        case '(':                       /* Select 94 Character Set into G0 */
            achar = (escnext<=esclast)?escbuffer[escnext++]:0;
              if ( ISLINUX(tt_type_mode) ) {
#ifndef COMMENT
                  /* From man "console_codes" as of Red Hat 8.0
                   *  ESC (    Start sequence defining G0 character set
                   *  ESC ( B     Select default (ISO 8859-1 mapping)
                   *  ESC ( 0     Select vt100 graphics mapping
                   *  ESC ( U     Select null mapping - straight to character ROM
                   *  ESC ( K     Select user mapping - the map that is loaded by
                   *              the utility mapscrn(8).
                   *  ESC )    Start sequence defining G1
                   *           (followed by one of B, 0, U, K, as above).
                   */
                  charset( cs94, achar, &G[0]) ;
#else
                  /* Linux only has two 256 char character sets */
                  /* We simulate them by using G0 for ASCII and */
                  /* G2 as Linux-G0 and G3 as Linux-G1          */
                  charset( cs94, achar, &G[3]) ;
#endif /* COMMENT */
              } else {
                  if ( decnrcm && isNRC(achar))
                  {
                      for ( i=0; i<=3 ; i++ )
                          charset( cs94, achar, &G[i] ) ;
                  }
                  else
                  {
                      charset( cs94, achar, &G[0] ) ;
                  }
            }
            break;
        case ')':                       /* Select 94 Character Set into G1 */
            achar = (escnext<=esclast)?escbuffer[escnext++]:0;
              if ( ISLINUX(tt_type_mode) ) {
#ifndef COMMENT
                  /* From man "console_codes" as of Red Hat 8.0
                   *  ESC (    Start sequence defining G0 character set
                   *  ESC ( B     Select default (ISO 8859-1 mapping)
                   *  ESC ( 0     Select vt100 graphics mapping
                   *  ESC ( U     Select null mapping - straight to character ROM
                   *  ESC ( K     Select user mapping - the map that is loaded by
                   *              the utility mapscrn(8).
                   *  ESC )    Start sequence defining G1
                   *           (followed by one of B, 0, U, K, as above).
                   */
                  charset( cs94, achar, &G[0]) ;
#else /* COMMENT */
                  /* Linux only has two 256 char character sets */
                  /* We simulate them by using G0 for ASCII and */
                  /* G2 as Linux-G0 and G3 as Linux-G1          */
                  charset( cs94, achar, &G[3]) ;
#endif /* COMMENT */
              } else {
                  if ( decnrcm && isNRC(achar) )
                  {
                      for ( i=0; i<=3 ; i++ )
                          charset( cs94, achar, &G[i] ) ;
                  }
                  else
                  {
                      charset( cs94, achar, &G[1] ) ;
                  }
            }
            break;
        case '*':  /* Select 94 Character Set into G2 */
              if ( !ISLINUX(tt_type_mode) &&
                   (ISVT220(tt_type_mode) || ISANSI(tt_type_mode))) {
                  achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                  if ( decnrcm && isNRC(achar) )
                  {
                      for ( i=0; i<=3 ; i++ )
                          charset( cs94, achar, &G[i] ) ;
                  }
                  else
                  {
                      charset( cs94, achar, &G[2] ) ;
                  }
              }
              break;
        case '%':       /* Non-ISO 2022 character sets */
              achar = (escnext<=esclast)?escbuffer[escnext++]:0;
              switch ( achar ) {
              case '@':         /* Return to ISO 2022 mode */
                  tt_utf8 = 0;
                  break;
              case 'G':         /* UTF-8 with standard return */
                  tt_utf8 = 1;
                  break;
              }
              break;
        case '+':                       /* Select 94 Character Set into G3 */
              if ( !ISLINUX(tt_type_mode) &&
                   (ISVT220(tt_type_mode) || ISANSI(tt_type_mode))) {
                  achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                  if ( decnrcm && isNRC(achar) )
                  {
                      for ( i=0; i<=3 ; i++ )
                          charset( cs94, achar, &G[i] ) ;
                  }
                  else
                  {
                      charset( cs94, achar, &G[3] ) ;
                  }
              }
              break;

        case '-':                       /* Select 96 Character Set into G1 */
              achar = (escnext<=esclast)?escbuffer[escnext++]:0;
              charset( cs96, achar, &G[1] ) ;
              break;
        case '.':                       /* Select 96 Character Set into G2 */
              if ( ISVT220(tt_type_mode) || ISANSI(tt_type_mode)) {
                  achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                  charset( cs96, achar, &G[2] ) ;
              }
              break;
        case '/':                       /* Select 96 Character Set into G3 */
              if ( ISVT220(tt_type_mode) || ISANSI(tt_type_mode)) {
                  achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                  charset( cs96, achar, &G[3] ) ;
              }
              break;

        case '~':                       /* Lock shift G1, right */
              if ( ISVT220(tt_type_mode) || ISANSI(tt_type_mode)) {
                  GR = &G[1] ;
              }
              break;
        case '}':                       /* Lock shift G2, right */
              if (ISVT220(tt_type_mode) || ISANSI(tt_type_mode)) {
                  GR = &G[2] ;
              }
              else if ( ISH19(tt_type_mode) ) {
                  /* Keyboard Disabled */
                  keylock = TRUE ;
              }
              break;
        case 'n':
              if ( ISH19(tt_type_mode) ) {
                  /* Cursor Position Report */
                  char buf[12] ;
                  sprintf(buf,"%cY%c%c",ESC,wherey[VTERM]+31,wherex[VTERM]+31);
                  sendchars(buf,strlen(buf));
              }
              else if ( ISVT220(tt_type_mode)  || ISANSI(tt_type_mode)) {
                  /* Lock shift G2, left */
                  GL = &G[2] ;
              }
              break;
        case 'o':                       /* Lock shift G3, left */
              if (ISVT220(tt_type_mode) || ISANSI(tt_type_mode)) {
                  GL = &G[3] ;
              }
              else if ( ISH19(tt_type_mode) ) {
                  /* Erase Beginning of Line */
                  clrbol_escape(VTERM,SP);
              }
              break;
        case '|':                       /* Lock shift G3, right */
              if (ISVT220(tt_type_mode) || ISANSI(tt_type_mode)) {
                  GR = &G[3] ;
              }
              break;
        case 'N':                       /* Single shift G2, left */
              if (ISVT220(tt_type_mode) || ISANSI(tt_type_mode)) {
                  SSGL = &G[2] ;
              }
              else if ( ISH19(tt_type_mode) ) {
                  /* H19 - Delete Character */
                  blankvcell.c = SP;
                  blankvcell.a = geterasecolor(VTERM);
                  if (pn[1] > VscrnGetWidth(VTERM) + 1 -
                       wherex[VTERM])
                      pn[1] = VscrnGetWidth(VTERM) + 1 -
                          wherex[VTERM];
                  VscrnScrollLf(VTERM, wherey[VTERM] - 1,
                                 wherex[VTERM] - 1,
                                 wherey[VTERM] - 1,
                                 VscrnGetWidth(VTERM) - 1,
                                 pn[1],
                                 blankvcell
                                 ) ;

              }
              break;
        case 'O':                       /* Single shift G3, left */
              if (ISVT220(tt_type_mode) || ISANSI(tt_type_mode)) {
                  SSGL = &G[3] ;
              }
              else if ( ISH19(tt_type_mode) ) {
                  /* Exit Insert Character Mode */
                  insertmode = FALSE ;
              }
              break;

        case '{':
                if ( ISH19(tt_type_mode) ) {
                    /* Keyboard Enabled */
                    keylock = FALSE ;
                    break;
                }
                break;
        default:
            if (achar == 12) {  /* ESC Ctrl-L */
                if ( decsasd == SASD_STATUS ) {
                    lgotoxy( VSTATUS, 1, 1 );
                    clrscreen(VSTATUS,SP);
                }
                else {
                    lgotoxy(VTERM,1, 1);
                    clrscreen(VTERM,SP);
                }
            }
            break;
        } /* First Level Case  */
    }
    else {                              /* transparent print */
        switch (achar) {                /* First Level */
          case '[':                     /* CSI */
            {
                achar = (escnext<=esclast)?escbuffer[escnext++]:0;
              LB3000:
                switch (achar) {        /* Second level */
                  case '?':             /* DEC private */
                    private = TRUE;
                    achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                    goto LB3000;
                    break;
                  default:              /* Pn - got a number */
                  LB3001:
                    {                   /* Esc [ Pn...Pn x   functions */
                        pn[1] = pnumber(&achar);
                        k = 1;
                      LB3002:
                        while (achar == ';') { /* get Pn[k] */
                            achar = (escnext<=esclast)?escbuffer[escnext++]:0;
                            k++;
                            if (achar == '?')
                              achar =
                                (escnext<=esclast)?escbuffer[escnext++]:0;
                            pn[k] = pnumber(&achar);
                        }
                        pn[k + 1] = 1;
                      LB3003:
                        switch (achar) { /* Third level */
                          case 'i':     /* Media Copy */
                            switch (pn[1]) {
                            case  4:
                                if ( private == FALSE ) {
                                    xprint = FALSE;
                                }
                                if ( is_noprint() )
                                  turnoffprinter = TRUE;
                                break;
                            case 5:
                                if ( private == FALSE )
                                    return ; /* Don't print this */
                                break;
                            case 7:
                                if ( !xprint && private == FALSE) {
                                    /* Turn on screen display */
                                    screenon = TRUE;
                                    if ( savefiletext[0] ) {
                                        strcpy(filetext,savefiletext);
                                        savefiletext[0] = NUL ;
                                        /* Update status line */
                                        VscrnIsDirty(VTERM);
                                    }
                                }
                                break;
                            }
                        }
                    }
                    break;
                }
            }                   /* Left square bracket */
            break;
        } /* First Level Case  */
    } /* transparent print || screen off */

    /* Host said to turn off the printer. */

    if (turnoffprinter) {
        printeroff();
        turnoffprinter = FALSE;
    }

/* If printer is in User or Controller modes, print this escape sequence. */
/* but not if Auto print mode */

    if ( printon && (is_xprint() || is_uprint())) {
        switch(escbuffer[0]) {
        case _CSI:
            prtchar(_CSI);
            i = 2;
            break;
        case ESC:
        default:
            i=0;;
            break;
        }
        if (esclast > 0) {
            /* print esc sequence */
            for (; i <= esclast; ++i)
                prtchar(escbuffer[i]);
        }
    }
/*
  If we just got a "printer on" directive, turn on the printer now.
  This way, the "printer on" directive itself is not printed.
*/
    if (turnonprinter) {                /* Last command was "printer on" */
        printeron();
        turnonprinter = FALSE;
    }
    if ( xprint == FALSE )
      VscrnIsDirty(VTERM);
}

/* ================================================================== */
/* VT100  -  emulate a DEC VT terminal writing a character            */
/* ================================================================== */
void
vt100(unsigned short vtch) {
    int             i, j;
    viocell       cell ;
    int vmode = decsasd == SASD_TERMINAL ? VTERM : VSTATUS ;

#ifdef COMMENT
    debug(F101,"vt100","",vtch);
#endif /* COMMENT */

   if (tt_type_mode == TT_NONE) {
      wrtch(vtch);
      return;
   } else if (!xprint) {
      if (vtch < 32 &&                  /* Control Character */
          (!crm && !literal_ch)) {      /* no display of c0 control chars */
         achar = vtch;  /* Let the rest of this module see the value */
         switch (achar) {
         case FF:                       /* Formfeed */
             if ( ISSUN(tt_type_mode) ) {
                 clrscreen(vmode,SP);
                 lgotoxy(vmode,1,1);       /* and home the cursor */
                 VscrnIsDirty(vmode);
             }
             else if ( (IS97801(tt_type_mode) || ISHP(tt_type_mode)) ) {
                 if (wherex[vmode] < marginright)
                     wherex[vmode]++;
             }
             else if ( ISANSI(tt_type_mode) ) {
                 if ( ISAVATAR(tt_type_mode) )  /* Avatar */
                     attribute = defaultattribute ;
                 clrscreen(vmode,SP);
                 lgotoxy(vmode,1,1);       /* and home the cursor */
                 VscrnIsDirty(vmode);
             }
             else {
                 wrtch((char) LF);
             }
             break;
         case LF:                       /* Linefeed */
         case VT:                       /* Vertical tab */
             wrtch((char) LF);
             break;
         case CR:                       /* Carriage return */
             wrtch((char) achar);
             break;
         case SO:                       /* SO */
             /* If I read the Linux Console Codes doc right, this is the proper behavior */
             if ( ISLINUX(tt_type_mode) ) {
                 GR = &G[3];
             } else if ( !ISANSI(tt_type_mode) )       { /* "ANSI music" protection... */
                 GL = &G[1] ;
             }
             break;

         case SI:                       /* SI */
             /* ditto */
             if ( ISLINUX(tt_type_mode) ) {
                 GR = &G[2];
             } else if ( !ISANSI(tt_type_mode) )       { /* Ditto, gag... */
                 GL = &G[0] ;
             }
             break;

         case BS:                       /* BS */
             wrtch((CHAR) achar);
             break;

         case BEL:                      /* BEL */
             bleep(BP_BEL);
             break;

         case ENQ:                      /* ENQ */
             if ( ISVIP(tt_type_mode) ) {
                 sendescseq("7813  P GC  A\003");
             }
             else if (tt_answer)
                 sendchars(answerback,strlen(answerback)) ;
             break;
         case HT:               /* Horizontal tab */
             i = wherex[vmode];
             if (i < VscrnGetWidth(vmode))
             {
                 do {
                     i++;
                     cursorright(0);
                  } while ((htab[i] != 'T') &&
                            (i <= VscrnGetWidth(vmode)-1));
                  VscrnIsDirty(vmode);
             }
             break;
         case SYN:      /* Ctrl-V - AVATAR AVTCODE */
             if ( ISAVATAR(tt_type_mode) )
                 avatar() ;
             break;
         case XEM:       /* AVATAR - Repeat character */
             if ( ISAVATAR(tt_type_mode) ) {
                 int ch, count;
                 ch = avainc(0) ;
                 count = avainc(0) ;
                 if ( debses )
                     break;
                 while ( count-- )
                     wrtch(ch);
             }
             break;
         case NUL:
             if ( IS97801(tt_type_mode) )
                 wrtch(SP);
             break;
         default:               /* ignore it */
             break;
         }
            /* End of Control Character */
      } else {
         if (vtch != DEL) {             /* Normal character. */
            if (ISVT100(tt_type_mode)) {
                if ( vtch == 35 &&
                     GNOW->designation == TX_BRITISH &&
                     tcsr == TX_ASCII )
                {
                    /* UK ISO 646 character-set, # = Pound Sterling Sign */
                    vtch = 156;
                }
            } else {
                if (vt52graphics) {
                    if ( ISH19(tt_type) && (94 <= vtch) && (vtch <= 126)) {
                       literal_ch = TRUE ;
                        vtch = xlh19grph(vtch) ;
                    }
                    else if ((95 <= vtch) && (vtch <= 126)) {
                        literal_ch = TRUE;
                        vtch = xldecgrph(vtch);
                    }
                }
            }

            /* On the right margin? */
            if (wherex[vmode] != (isdoublewidth(wherey[vmode]) ?
                                   VscrnGetWidth(vmode)/2 :
                                   VscrnGetWidth(vmode)))
            {
                wrtch(vtch);    /* Not on right margin */
                wrapit = FALSE;
            } else {            /* On right margin. */
                if (wrapit) {   /* Time to wrap?  */
                    if (literal_ch) {
                        literal_ch = 0;
                        wrtch(CR);
                        wrtch(LF);
                        literal_ch = 1;
                    } else {
                        wrtch(CR);
                        wrtch(LF);
                    }
                    wrtch(vtch);        /* Now write the character */
                    wrapit = FALSE;     /* Remember wrapping is done */
                } else {                /* Not time to wrap */
                    cell.c = vtch;
                    cell.a = attribute ;
                    if (isdoublewidth(wherey[vmode]))
                        VscrnWrtCell(vmode, cell,attrib,wherey[vmode]-1,
                                      VscrnGetWidth(vmode)/2-1);
                    else
                        VscrnWrtCell(vmode, cell,attrib,wherey[vmode]-1,
                                      VscrnGetWidth(vmode)-1) ;
                    VscrnIsDirty(vmode);
                    literal_ch = FALSE;
                    if (tt_wrap) { /* If TERM WRAP ON */
                        if ( IS97801(tt_type_mode) ) {
                            if ( wherey[vmode] == marginbot ) {
                                if ( !sni_pagemode ) {
                                    wrtch(CR);
                                    wrtch(LF);
                                }
                                else {  /* Page Mode */
                                    lgotoxy(vmode,1,1);
                                }
                            }
                            else {
                                wrtch(CR);
                                wrtch(LF);
                            }
                        } else /* if ( !deccolm ) */ {
							/* 20050407 - I do not remember why there is a test
							 * to disable wrapping if 132 column mode is on.
							 * If there was such a exclusion it must be restricted
							 * to a particular model of terminal.  Which one?
							 */
                            if ( (ISLINUX(tt_type_mode) ||
                                  ISVT100(tt_type_mode) ||
                                  ISVT52(tt_type_mode)) &&
                                  !ISHFT(tt_type_mode))
                                wrapit = TRUE ; /* need to wrap next time */
                            else {
                                wrtch(CR);
                                wrtch(LF);
                            }
                        }
                    }
                }
            }
         }                      /* End Normal char */
      }
   }
   if (is_xprint() || (is_uprint() && (vtch != 27)))
       prtchar(vtch) ;
}

#ifdef CK_APC
void
apc_command(int type, char * cmd)
{
    extern struct mtab *mactab;             /* Macro table */
    extern int nmac;
#ifdef DCMDBUF
    extern struct cmdptr *cmdstk;
#else
    extern struct cmdptr cmdstk[];
#endif /* DCMDBUF */

    debug(F111,"apc_command",cmd,type);
    if ( cmd == NULL )
        return;
    if ( cmd != apcbuf )
        ckstrncpy(apcbuf,cmd,APCBUFLEN);
    apcactive = type;
    apclength = strlen(apcbuf) ;

    if (apcstatus & APC_NOINP) {
        debug(F110,"apc_command (no input) about to execute APC",apcbuf,0);
        domac("_apc_commands",apcbuf,cmdstk[cmdlvl].ccflgs|CF_APC);
        debug(F110,"apc_command finished executing APC",apcbuf,0);
        delmac("_apc_commands",1);
        cmini(ckxech);
        if ( apcactive == APC_INACTIVE
#ifndef NOXFER
             || (apcactive == APC_LOCAL && !xferstat && adl_err != 0)
#endif /* NOXFER */
             )
            SetConnectMode(FALSE,CSX_APC); /* exit from terminal emulator */
        return;
    }

    debug(F111,"apc_command","tt_async",tt_async);
    if ( tt_async ) {
        if (mlook(mactab,"_apc_commands",nmac) == -1) {
            int vmsave = vmode;
            SaveTermMode(wherex[VTERM],wherey[VTERM]);
            RestoreCmdMode();
            concooked();
            debug(F110,"apc_command about to execute APC",apcbuf,0);
            domac("_apc_commands",apcbuf,cmdstk[cmdlvl].ccflgs|CF_APC);
            debug(F110,"apc_command finished executing APC",apcbuf,0);
            delmac("_apc_commands",1);
            cmini(ckxech);
            conraw();
            RestoreTermMode();
            vmode = vmsave;
            VscrnIsDirty(VCMD);
            VscrnIsDirty(vmode);
            if ( apcactive == APC_INACTIVE
#ifndef NOXFER
                 || (apcactive == APC_LOCAL && !xferstat && adl_err != 0)
#endif /* NOXFER */
                 )
                SetConnectMode(FALSE,CSX_APC); /* exit from terminal emulator */
        }
        apcactive = APC_INACTIVE;
    } else {
        term_io = FALSE;                /* Disable Emulator I/O */
        SetConnectMode(FALSE,CSX_APC);  /* exit from terminal emulator */
    }
}
#endif /* CK_APC */
#endif /* NOTERM */
#endif /* NOLOCAL */
