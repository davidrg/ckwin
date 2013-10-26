/* C K O K E Y -- Kermit Keyboard support for OS/2 and Win32 Systems */

#ifdef NT
char *ckyv = "Win32 Keyboard I/O, 8.0.203, 30 Oct 2003";
#else
char *ckyv = "OS/2 Keyboard I/O, 8.0.203, 30 Oct 2003";
#endif /* NT */
/*
  Author: Jeffrey Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#include "ckcdeb.h"                     /* Typedefs, debug formats, etc */
#include "ckcasc.h"                     /* ASCII character names */
#include "ckcker.h"                     /* Kermit definitions */
#include "ckuusr.h"                     /* Command definitions - needed for ckokey.h */
#include "ckctel.h"
#include <ctype.h>                      /* Character types */
#include <stdio.h>                      /* Standard i/o */
#include <io.h>                         /* File io function declarations */
#include <process.h>                    /* Process-control functions */
#include <string.h>                     /* String manipulation declarations */
#include <stdlib.h>                     /* Standard library declarations */
#include <signal.h>

/* OS/2 system header files & related stuff */

#ifdef NT
#include <windows.h>
#else /* not NT */
#define INCL_WIN
#define INCL_WINSWITCHLIST
#define INCL_ERRORS
#define INCL_KBD
#ifdef OS2MOUSE
#define INCL_MOU
#endif /* OS2MOUSE */
#define INCL_DOSMISC
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVICES
#define INCL_DOSNLS
#include <os2.h>        /* This pulls in a whole load of stuff */
#undef COMMENT
#endif /* NT */

#include "ckcxla.h"
#include "ckuxla.h"
#include "ckcuni.h"
#include "ckocon.h"
#include "ckokey.h"
#ifdef KUI
#include "ikui.h"
#endif /* KUI */

#define THRDSTKSIZ      32768
extern HKBD KbdHandle;
extern TID tidKbdHandler;
extern BOOL KbdActive;
#ifndef NOSETKEY
extern KEY *keymap;
#endif /* NOSETKEY */
extern int islocal ;
extern int k95stdio, k95stdin, k95stdout;
extern int ttyfd ;
#ifdef IKSD
extern int inserver;
#endif /* IKSD */

#ifndef NOSETKEY
/* Key names */
char *keynam[256];                      /* Names of keys, by scan code */
char *charnam[256];                     /* Names of chars, by US ASCII */
#endif /* NOSETKEY */
#ifndef NOKVERBS
vik_rec vik ;                           /* Very Important Keys */
int     initvik = TRUE ;                /* Need to init VIK table */
int os2gks = 1 ;                        /* Kverb processing active */
int mskkeys = 0 ;                       /* Perform MSK keycode xlation  */
static int kbdCallsKverb = 0;
#endif /* NOKVERBS */

extern bool keyclick ;
#ifndef NOTERM
extern int tt_type, tt_type_mode ;
extern int tt_kb_mode ;
#ifdef PCTERM
int tt_pcterm = 0;                      /* PCTERM keyboard mode */
VOID
setpcterm(int x)
{
    tt_pcterm = x;
    ipadl25();
#ifdef KUI
    KuiSetProperty(KUI_TERM_PCTERM,x,0);
#endif /* KUI */
}
extern int wy_block;
#endif /* PCTERM */
#endif /* NOTERM */

#ifdef NT
extern int win95altgr;
#endif /* NT */

#ifndef NOKVERBS
/*
  Table of keyboard verbs, associating name with symbol (value).
  Names are chosen for compatibility with MS-DOS Kermit.
  Keyword strings MUST be in lowercase and must be in alphabetical order!
*/
struct keytab kverbs[] = {
    "answerback", K_ANSWERBACK,  0,
    "ansif01",    K_ANSIF01,     0,
    "ansif02",    K_ANSIF02,      0,
    "ansif03",    K_ANSIF03,      0,
    "ansif04",    K_ANSIF04,      0,
    "ansif05",    K_ANSIF05,      0,
    "ansif06",    K_ANSIF06,      0,
    "ansif07",    K_ANSIF07,      0,
    "ansif08",    K_ANSIF08,      0,
    "ansif09",    K_ANSIF09,      0,
    "ansif10",    K_ANSIF10,      0,
    "ansif11",    K_ANSIF11,      0,
    "ansif12",    K_ANSIF12,      0,
    "ansif13",    K_ANSIF13,      0,
    "ansif14",    K_ANSIF14,      0,
    "ansif15",    K_ANSIF15,      0,
    "ansif16",    K_ANSIF16,      0,
    "ansif17",    K_ANSIF17,      0,
    "ansif18",    K_ANSIF18,      0,
    "ansif19",    K_ANSIF19,      0,
    "ansif20",    K_ANSIF20,      0,
    "ansif21",    K_ANSIF21,      0,
    "ansif22",    K_ANSIF22,      0,
    "ansif23",    K_ANSIF23,      0,
    "ansif24",    K_ANSIF24,      0,
    "ansif25",    K_ANSIF25,      0,
    "ansif26",    K_ANSIF26,      0,
    "ansif27",    K_ANSIF27,      0,
    "ansif28",    K_ANSIF28,      0,
    "ansif29",    K_ANSIF29,      0,
    "ansif30",    K_ANSIF30,      0,
    "ansif31",    K_ANSIF31,      0,
    "ansif32",    K_ANSIF32,      0,
    "ansif33",    K_ANSIF33,      0,
    "ansif34",    K_ANSIF34,      0,
    "ansif35",    K_ANSIF35,      0,
    "ansif36",    K_ANSIF36,      0,
    "ansif37",    K_ANSIF37,      0,
    "ansif38",    K_ANSIF38,      0,
    "ansif39",    K_ANSIF39,      0,
    "ansif40",    K_ANSIF40,      0,
    "ansif41",    K_ANSIF41,      0,
    "ansif42",    K_ANSIF42,      0,
    "ansif43",    K_ANSIF43,      0,
    "ansif44",    K_ANSIF44,      0,
    "ansif45",    K_ANSIF45,      0,
    "ansif46",    K_ANSIF46,      0,
    "ansif47",    K_ANSIF47,      0,
    "ansif48",    K_ANSIF48,      0,
    "ansif49",    K_ANSIF49,      0,
    "ansif50",    K_ANSIF50,      0,
    "ansif51",    K_ANSIF51,      0,
    "ansif52",    K_ANSIF52,      0,
    "ansif53",    K_ANSIF53,      0,
    "ansif54",    K_ANSIF54,      0,
    "ansif55",    K_ANSIF55,      0,
    "ansif56",    K_ANSIF56,      0,
    "ansif57",    K_ANSIF57,      0,
    "ansif58",    K_ANSIF58,      0,
    "ansif59",    K_ANSIF59,      0,
    "ansif60",    K_ANSIF60,      0,
    "ansif61",    K_ANSIF61,      0,

    "autodown",   K_AUTODOWN,     0,

    "ba80_attr",     K_BA80_ATTR    , 0,
    "ba80_c_key",    K_BA80_C_KEY   , 0,
    "ba80_clear",    K_BA80_CLEAR   , 0,
    "ba80_cmd",      K_BA80_CMD     , 0,
    "ba80_copy",     K_BA80_COPY    , 0,
    "ba80_del",      K_BA80_DEL , 0,
    "ba80_del_b",    K_BA80_DEL_B   , 0,
    "ba80_do",       K_BA80_DO      , 0,
    "ba80_end",      K_BA80_END , 0,
    "ba80_env",      K_BA80_ENV     , 0,
    "ba80_eop",      K_BA80_EOP     , 0,
    "ba80_erase",    K_BA80_ERASE   , 0,
    "ba80_fmt",      K_BA80_FMT     , 0,
    "ba80_help",     K_BA80_HELP, 0,
    "ba80_home",     K_BA80_HOME    , 0,
    "ba80_ins",      K_BA80_INS , 0,
    "ba80_ins_b",    K_BA80_INS_B   , 0,
    "ba80_mark",     K_BA80_MARK    , 0,
    "ba80_move",     K_BA80_MOVE    , 0,
    "ba80_pa01",     K_BA80_PA01, 0,
    "ba80_pa02",     K_BA80_PA02, 0,
    "ba80_pa03",     K_BA80_PA03, 0,
    "ba80_pa04",     K_BA80_PA04, 0,
    "ba80_pa05",     K_BA80_PA05, 0,
    "ba80_pa06",     K_BA80_PA06, 0,
    "ba80_pa07",     K_BA80_PA07, 0,
    "ba80_pa08",     K_BA80_PA08, 0,
    "ba80_pa09",     K_BA80_PA09, 0,
    "ba80_pa10",     K_BA80_PA10, 0,
    "ba80_pa11",     K_BA80_PA11, 0,
    "ba80_pa12",     K_BA80_PA12, 0,
    "ba80_pa13",     K_BA80_PA13, 0,
    "ba80_pa14",     K_BA80_PA14, 0,
    "ba80_pa15",     K_BA80_PA15, 0,
    "ba80_pa16",     K_BA80_PA16, 0,
    "ba80_pa17",     K_BA80_PA17, 0,
    "ba80_pa18",     K_BA80_PA18, 0,
    "ba80_pa19",     K_BA80_PA19, 0,
    "ba80_pa20",     K_BA80_PA20, 0,
    "ba80_pa21",     K_BA80_PA21, 0,
    "ba80_pa22",     K_BA80_PA22, 0,
    "ba80_pa23",     K_BA80_PA23, 0,
    "ba80_pa24",     K_BA80_PA24, 0,
    "ba80_pgdn",     K_BA80_PGDN    , 0,
    "ba80_pgup",     K_BA80_PGUP    , 0,
    "ba80_pick",     K_BA80_PICK    , 0,
    "ba80_print",    K_BA80_PRINT   , 0,
    "ba80_put",      K_BA80_PUT     , 0,
    "ba80_refresh",  K_BA80_REFRESH , 0,
    "ba80_reset",    K_BA80_RESET   , 0,
    "ba80_rubout",   K_BA80_RUBOUT  , 0,
    "ba80_save",     K_BA80_SAVE    , 0,
    "ba80_softkey1", K_BA80_SOFTKEY1, 0,
    "ba80_softkey2", K_BA80_SOFTKEY2, 0,
    "ba80_softkey3", K_BA80_SOFTKEY3, 0,
    "ba80_softkey4", K_BA80_SOFTKEY4, 0,
    "ba80_softkey5", K_BA80_SOFTKEY5, 0,
    "ba80_softkey6", K_BA80_SOFTKEY6, 0,
    "ba80_softkey7", K_BA80_SOFTKEY7, 0,
    "ba80_softkey8", K_BA80_SOFTKEY8, 0,
    "ba80_softkey9", K_BA80_SOFTKEY9, 0,
    "ba80_undo",     K_BA80_UNDO    , 0,

    "backnext",   K_BACKNEXT,    0,
    "backsearch", K_BACKSRCH,    0,
    "break",      K_BREAK,       0,
    "bytesize",   K_BYTESIZE,    0,
    "clearscreen",K_CLRSCRN,     0,
    "clearscrollback", K_CLRSCROLL, 0,
    "compose",    K_COMPOSE,     0,
    "debug",      K_DEBUG,       0,
    "decdo",      K_DECDO,       0,
    "decf01",     K_DECF1,       CM_INV,
    "decf02",     K_DECF2,       CM_INV,
    "decf03",     K_DECF3,       CM_INV,
    "decf04",     K_DECF4,       CM_INV,
    "decf05",     K_DECF5,       CM_INV,
    "decf06",     K_DECF6,       CM_INV,
    "decf07",     K_DECF7,       CM_INV,
    "decf08",     K_DECF8,       CM_INV,
    "decf09",     K_DECF9,       CM_INV,
    "decf1",      K_DECF1,       0,
    "decf10",     K_DECF10,      0,
    "decf11",     K_DECF11,      0,
    "decf12",     K_DECF12,      0,
    "decf13",     K_DECF13,      0,
    "decf14",     K_DECF14,      0,
    "decf15",     K_DECF15,      0,
    "decf16",     K_DECF16,      0,
    "decf17",     K_DECF17,      0,
    "decf18",     K_DECF18,      0,
    "decf19",     K_DECF19,      0,
    "decf2",      K_DECF2,       0,
    "decf20",     K_DECF20,      0,
    "decf3",      K_DECF3,       0,
    "decf4",      K_DECF4,       0,
    "decf5",      K_DECF5,       0,
    "decf6",      K_DECF6,       0,
    "decf7",      K_DECF7,       0,
    "decf8",      K_DECF8,       0,
    "decf9",      K_DECF9,       0,
    "decfind",    K_DECFIND,     0,
    "dechelp",    K_DECHELP,     0,
    "dechome",    K_DECHOME,     0,
    "decinsert",  K_DECINSERT,   0,
    "decnext",    K_DECNEXT,     0,
    "decprev",    K_DECPREV,     0,
    "decremove",  K_DECREMOVE,   0,
    "decselect",  K_DECSELECT,   0,
    "debuglog",   K_LOGDEBUG,    CM_INV,
    "dgbs",       K_DGBS, 0,
    "dgc1",       K_DGC1, 0,
    "dgc2",       K_DGC2, 0,
    "dgc3",        K_DGC3, 0,
    "dgc4",       K_DGC4, 0,
    "dgcf01",      K_DGCF01,       0,
    "dgcf02",      K_DGCF02,       0,
    "dgcf03",      K_DGCF03,       0,
    "dgcf04",      K_DGCF04,       0,
    "dgcf05",      K_DGCF05,       0,
    "dgcf06",      K_DGCF06,       0,
    "dgcf07",      K_DGCF07,       0,
    "dgcf08",      K_DGCF08,       0,
    "dgcf09",      K_DGCF09,       0,
    "dgcf10",      K_DGCF10,       0,
    "dgcf11",      K_DGCF11,       0,
    "dgcf12",      K_DGCF12,       0,
    "dgcf13",      K_DGCF13,       0,
    "dgcf14",      K_DGCF14,       0,
    "dgcf15",      K_DGCF15,       0,
    "dgcmdprint",   K_DGCMDPRINT, 0,
    "dgcsf01",      K_DGCSF01,       0,
    "dgcsf02",      K_DGCSF02,       0,
    "dgcsf03",      K_DGCSF03,       0,
    "dgcsf04",      K_DGCSF04,       0,
    "dgcsf05",      K_DGCSF05,       0,
    "dgcsf06",      K_DGCSF06,       0,
    "dgcsf07",      K_DGCSF07,       0,
    "dgcsf08",      K_DGCSF08,       0,
    "dgcsf09",      K_DGCSF09,       0,
    "dgcsf10",      K_DGCSF10,       0,
    "dgcsf11",      K_DGCSF11,       0,
    "dgcsf12",      K_DGCSF12,       0,
    "dgcsf13",      K_DGCSF13,       0,
    "dgcsf14",      K_DGCSF14,       0,
    "dgcsf15",      K_DGCSF15,       0,
    "dgdnarr",     K_DGDNARR, 0,
    "dgeraseeol",     K_DGERASEEOL, 0,
    "dgerasepage",     K_DGERASEPAGE, 0,
    "dgf01",      K_DGF01,       0,
    "dgf02",      K_DGF02,       0,
    "dgf03",      K_DGF03,       0,
    "dgf04",      K_DGF04,       0,
    "dgf05",      K_DGF05,       0,
    "dgf06",      K_DGF06,       0,
    "dgf07",      K_DGF07,       0,
    "dgf08",      K_DGF08,       0,
    "dgf09",      K_DGF09,       0,
    "dgf10",      K_DGF10,       0,
    "dgf11",      K_DGF11,       0,
    "dgf12",      K_DGF12,       0,
    "dgf13",      K_DGF13,       0,
    "dgf14",      K_DGF14,       0,
    "dgf15",      K_DGF15,       0,
    "dghome",     K_DGHOME,      0,
    "dglfarr",     K_DGLFARR, 0,
    "dgrtarr",     K_DGRTARR, 0,
    "dgsc1",       K_DGSC1, 0,
    "dgsc2",     K_DGSC2, 0,
    "dgsc3",        K_DGSC3, 0,
    "dgsc4",     K_DGSC4, 0,
    "dgscmdprint",   K_DGSCMDPRINT, 0,
    "dgsdnarr",     K_DGSDNARR, 0,
    "dgseraseeol",     K_DGSERASEEOL, 0,
    "dgserasepage",   K_DGSERASEPAGE, 0,
    "dgsf01",      K_DGSF01,       0,
    "dgsf02",      K_DGSF02,       0,
    "dgsf03",      K_DGSF03,       0,
    "dgsf04",      K_DGSF04,       0,
    "dgsf05",      K_DGSF05,       0,
    "dgsf06",      K_DGSF06,       0,
    "dgsf07",      K_DGSF07,       0,
    "dgsf08",      K_DGSF08,       0,
    "dgsf09",      K_DGSF09,       0,
    "dgsf10",      K_DGSF10,       0,
    "dgsf11",      K_DGSF11,       0,
    "dgsf12",      K_DGSF12,       0,
    "dgsf13",      K_DGSF13,       0,
    "dgsf14",      K_DGSF14,       0,
    "dgsf15",      K_DGSF15,       0,
    "dgshome",     K_DGSHOME, 0,
    "dgslfarr",    K_DGSLFARR, 0,
    "dgsrtarr",    K_DGSRTARR, 0,
    "dgsuparr",    K_DGSUPARR, 0,
    "dguparr",     K_DGUPARR, 0,
    "dnarr",      K_DNARR,       0,
    "dnone",      K_DNONE,       0,
    "dnscn",      K_DNSCN,       0,
    "dos",        K_DOS,         0,
    "dump",       K_DUMP,        0,
    "emacs_overwrite", K_EMACS_OVER, CM_INV,
    "endscn",     K_ENDSCN,      0,
    "exit",       K_EXIT,        0,
    "flipscn",    K_FLIPSCN,     0,
    "fnkeys",     K_FNKEYS,      0,
    "fwdnext",    K_FWDNEXT,     0,
    "fwdsearch",  K_FWDSRCH,     0,
    "gold",       K_GOLD,        0,
    "gobook",     K_GO_BOOK,     0,
    "goto",       K_GOTO,        0,
    "hangup",     K_HANGUP,      0,
    "help",       K_HELP,        0,
    "holdscrn",   K_HOLDSCRN,    0,
    "homscn",     K_HOMSCN,      0,
    "hpbacktab",  K_HPBACKTAB,   0,
    "hpenter",    K_HPENTER,     0,
    "hpf01",      K_HPF01,       0,
    "hpf02",      K_HPF02,       0,
    "hpf03",      K_HPF03,       0,
    "hpf04",      K_HPF04,       0,
    "hpf05",      K_HPF05,       0,
    "hpf06",      K_HPF06,       0,
    "hpf07",      K_HPF07,       0,
    "hpf08",      K_HPF08,       0,
    "hpf09",      K_HPF09,       0,
    "hpf1",       K_HPF01,       CM_INV,
    "hpf10",      K_HPF10,       0,
    "hpf11",      K_HPF11,       0,
    "hpf12",      K_HPF12,       0,
    "hpf13",      K_HPF13,       0,
    "hpf14",      K_HPF14,       0,
    "hpf15",      K_HPF15,       0,
    "hpf16",      K_HPF16,       0,
    "hpf2",      K_HPF02,        CM_INV,
    "hpf3",      K_HPF03,        CM_INV,
    "hpf4",      K_HPF04,        CM_INV,
    "hpf5",      K_HPF05,        CM_INV,
    "hpf6",      K_HPF06,        CM_INV,
    "hpf7",      K_HPF07,        CM_INV,
    "hpf8",      K_HPF08,        CM_INV,
    "hpf9",      K_HPF09,        CM_INV,
    "hpreturn",   K_HPRETURN,    0,

    "i31_cancel", K_I31_CANCEL         ,      0,
    "i31_clear", K_I31_CLEAR           ,      0,
    "i31_delete", K_I31_DELETE         ,      0,
    "i31_del_ln", K_I31_DEL_LN         ,      0,
    "i31_erase_eof", K_I31_ERASE_EOF   ,      0,
    "i31_erase_eop", K_I31_ERASE_EOP   ,      0,
    "i31_erase_inp", K_I31_ERASE_INP   ,      0,
    "i31_f01", K_I31_F01               ,      0,
    "i31_f02", K_I31_F02               ,      0,
    "i31_f03", K_I31_F03               ,      0,
    "i31_f04", K_I31_F04               ,      0,
    "i31_f05", K_I31_F05               ,      0,
    "i31_f06", K_I31_F06               ,      0,
    "i31_f07", K_I31_F07               ,      0,
    "i31_f08", K_I31_F08               ,      0,
    "i31_f09", K_I31_F09               ,      0,
    "i31_f10", K_I31_F10               ,      0,
    "i31_f11", K_I31_F11               ,      0,
    "i31_f12", K_I31_F12               ,      0,
    "i31_f13", K_I31_F13               ,      0,
    "i31_f14", K_I31_F14               ,      0,
    "i31_f15", K_I31_F15               ,      0,
    "i31_f16", K_I31_F16               ,      0,
    "i31_f17", K_I31_F17               ,      0,
    "i31_f18", K_I31_F18               ,      0,
    "i31_f19", K_I31_F19               ,      0,
    "i31_f20", K_I31_F20               ,      0,
    "i31_f21", K_I31_F21               ,      0,
    "i31_f22", K_I31_F22               ,      0,
    "i31_f23", K_I31_F23               ,      0,
    "i31_f24", K_I31_F24               ,      0,
    "i31_f25", K_I31_F25               ,      0,
    "i31_f26", K_I31_F26               ,      0,
    "i31_f27", K_I31_F27               ,      0,
    "i31_f28", K_I31_F28               ,      0,
    "i31_f29", K_I31_F29               ,      0,
    "i31_f30", K_I31_F30               ,      0,
    "i31_f31", K_I31_F31               ,      0,
    "i31_f32", K_I31_F32               ,      0,
    "i31_f33", K_I31_F33               ,      0,
    "i31_f34", K_I31_F34               ,      0,
    "i31_f35", K_I31_F35               ,      0,
    "i31_f36", K_I31_F36               ,      0,
    "i31_ins_char", K_I31_INSERT_CHAR  ,      0,
    "i31_ins_line", K_I31_INS_LN         ,    0,
    "i31_ins_space", K_I31_INSERT_SPACE,      0,
    "i31_jump", K_I31_JUMP             ,      0,
    "i31_pa1", K_I31_PA1               ,      0,
    "i31_pa2", K_I31_PA2               ,      0,
    "i31_pa3", K_I31_PA3               ,      0,
    "i31_prt_line", K_I31_PRINT_LINE     ,    0,
    "i31_prt_msg", K_I31_PRINT_MSG     ,      0,
    "i31_prt_shift", K_I31_PRINT_SHIFT ,      0,
    "i31_reset", K_I31_RESET           ,      0,
    "i31_send_line", K_I31_SEND_LINE   ,      0,
    "i31_send_msg", K_I31_SEND_MSG     ,      0,
    "i31_send_page", K_I31_SEND_PAGE   ,      0,

    "ignore",     K_IGNORE,      0,
    "kbemacs",    K_KB_EMA,      0,
    "kbenglish",  K_KB_ENG,      0,
    "kbhebrew",   K_KB_HEB,      0,
    "kbrussian",  K_KB_RUS,      0,
    "kbwp",       K_KB_WP,       0,
    "keyclick",   K_KEYCLICK,    0,
    "kp0",        K_KP0,         0,
    "kp1",        K_KP1,         0,
    "kp2",        K_KP2,         0,
    "kp3",        K_KP3,         0,
    "kp4",        K_KP4,         0,
    "kp5",        K_KP5,         0,
    "kp6",        K_KP6,         0,
    "kp7",        K_KP7,         0,
    "kp8",        K_KP8,         0,
    "kp9",        K_KP9,         0,
    "kpcoma",     K_KPCOMA,      0,
    "kpdot",      K_KPDOT,       0,
    "kpenter",    K_KPENTER,     0,
    "kpminus",    K_KPMINUS,     0,
    "lbreak",     K_LBREAK,      0,
    "lfall",      K_LFALL,       0,
    "lfarr",      K_LFARR,       0,
    "lfone",      K_LFONE,       0,
    "lfpage",     K_LFPAGE,      0,
    "logoff",     K_LOGOFF,      CM_INV,
    "logon",      K_LOGON,       CM_INV,
    "markcancel", K_MARK_CANCEL, 0,
    "markcopyclip", K_MARK_COPYCLIP, 0,
    "markcopyclip_noeol", K_MARK_COPYCLIP_NOEOL, 0,
    "markcopyhost", K_MARK_COPYHOST, 0,
    "markcopyhost_noeol", K_MARK_COPYHOST_NOEOL, 0,
    "markselect",   K_MARK_SELECT, CM_INV,
    "markselect_noeol",   K_MARK_SELECT_NOEOL, CM_INV,
    "markstart",  K_MARK_START,  0,
/*  "modeline",   K_MODELINE,    0, */
#ifdef OS2MOUSE
    "mousecurpos",K_MOUSE_CURPOS,0,
    "mousemark",  K_MOUSE_MARK,  0,
    "mouseurl",   K_MOUSE_URL,   0,
#endif /* OS2MOUSE */
/*  "nethold",    K_NETHOLD,     0, */
/*  "nextsession",K_NEXTSESS,    0, */
    "null",       K_NULL,        0,
    "os2",        K_DOS,         0,
    "paste",      K_PASTE,       0,
    "pcterm",     K_PCTERM,      CM_INV,
    "pf1",        K_PF1,         0,
    "pf2",        K_PF2,         0,
    "pf3",        K_PF3,         0,
    "pf4",        K_PF4,         0,
    "printff",    K_PRINTFF,     0,
    "prtauto",    K_PRTAUTO,     0,
    "prtcopy",    K_PRTCOPY,     0,
    "prtctrl",    K_PRTCTRL,     0,
    "prtscn",     K_DUMP,        CM_INV,
    "quit",       K_QUIT,        0,
    "reset",      K_RESET,       0,
    "rtall",      K_RTALL,       0,
    "rtarr",      K_RTARR,       0,
    "rtone",      K_RTONE,       0,
    "rtpage",     K_RTPAGE,      0,
    "session",    K_SESSION,     0,
    "setbook",    K_SET_BOOK,    0,

    "sni_00",        K_SNI_DOUBLE_0      ,0,
    "sni_c_00",        K_SNI_C_DOUBLE_0    ,0,
    "sni_c_ce",        K_SNI_C_CE        ,0,
    "sni_c_compose",   K_SNI_C_COMPOSE     ,0,
    "sni_c_del_char",  K_SNI_C_DELETE_CHAR ,0,
    "sni_c_del_line",  K_SNI_C_DELETE_LINE ,0,
    "sni_c_del_word",  K_SNI_C_DELETE_WORD ,0,
    "sni_c_dnarr",     K_SNI_C_CURSOR_DOWN ,0,
    "sni_c_end",       K_SNI_C_END         ,0,
    "sni_c_endmarke",  K_SNI_C_ENDMARKE    ,0,
    "sni_c_f01",       K_SNI_C_F01       ,0,
    "sni_c_f02",       K_SNI_C_F02       ,0,
    "sni_c_f03",       K_SNI_C_F03       ,0,
    "sni_c_f04",       K_SNI_C_F04       ,0,
    "sni_c_f05",       K_SNI_C_F05       ,0,
    "sni_c_f06",       K_SNI_C_F06       ,0,
    "sni_c_f07",       K_SNI_C_F07       ,0,
    "sni_c_f08",       K_SNI_C_F08       ,0,
    "sni_c_f09",       K_SNI_C_F09       ,0,
    "sni_c_f10",       K_SNI_C_F10       ,0,
    "sni_c_f11",       K_SNI_C_F11       ,0,
    "sni_c_f12",       K_SNI_C_F12       ,0,
    "sni_c_f13",       K_SNI_C_F13       ,0,
    "sni_c_f14",       K_SNI_C_F14       ,0,
    "sni_c_f15",       K_SNI_C_F15       ,0,
    "sni_c_f16",       K_SNI_C_F16       ,0,
    "sni_c_f17",       K_SNI_C_F17       ,0,
    "sni_c_f18",       K_SNI_C_F18       ,0,
    "sni_c_f19",       K_SNI_C_F19       ,0,
    "sni_c_f20",       K_SNI_C_F20       ,0,
    "sni_c_f21",       K_SNI_C_F21       ,0,
    "sni_c_f22",       K_SNI_C_F22       ,0,
    "sni_c_help",      K_SNI_C_HELP      ,0,
    "sni_c_home",      K_SNI_C_HOME        ,0,
    "sni_c_ins_char",  K_SNI_C_INSERT_CHAR ,0,
    "sni_c_ins_line",  K_SNI_C_INSERT_LINE ,0,
    "sni_c_ins_word",  K_SNI_C_INSERT_WORD ,0,
    "sni_c_left_tab",  K_SNI_C_LEFT_TAB  ,0,
    "sni_c_lfarr",     K_SNI_C_CURSOR_LEFT ,0,
    "sni_c_mode",      K_SNI_C_MODE      ,0,
    "sni_c_page",      K_SNI_C_PAGE        ,0,
    "sni_c_print",     K_SNI_C_PRINT     ,0,
    "sni_c_rtarr",     K_SNI_C_CURSOR_RIGHT,0,
    "sni_c_scroll_dn", K_SNI_C_SCROLL_DOWN ,0,
    "sni_c_scroll_up", K_SNI_C_SCROLL_UP ,0,
    "sni_c_start",     K_SNI_C_START     ,0,
    "sni_c_uparr",     K_SNI_C_CURSOR_UP   ,0,
    "sni_c_user1",     K_SNI_C_USER1       ,0,
    "sni_c_user2",     K_SNI_C_USER2       ,0,
    "sni_c_user3",     K_SNI_C_USER3       ,0,
    "sni_c_user4",     K_SNI_C_USER4       ,0,
    "sni_c_user5",     K_SNI_C_USER5       ,0,
    "sni_c_tab",       K_SNI_C_TAB       ,0,
    "sni_ce",        K_SNI_CE            ,0,
    "sni_ch_code",   K_SNI_CH_CODE       ,0,
    "sni_compose",   K_SNI_COMPOSE       ,0,
    "sni_del_char",  K_SNI_DELETE_CHAR   ,0,
    "sni_del_line",  K_SNI_DELETE_LINE   ,0,
    "sni_del_word",  K_SNI_DELETE_WORD   ,0,
    "sni_dnarr",     K_SNI_CURSOR_DOWN   ,0,
    "sni_end",       K_SNI_END           ,0,
    "sni_endmarke",  K_SNI_ENDMARKE      ,0,
    "sni_f01",       K_SNI_F01           ,0,
    "sni_f02",       K_SNI_F02           ,0,
    "sni_f03",       K_SNI_F03           ,0,
    "sni_f04",       K_SNI_F04           ,0,
    "sni_f05",       K_SNI_F05           ,0,
    "sni_f06",       K_SNI_F06           ,0,
    "sni_f07",       K_SNI_F07           ,0,
    "sni_f08",       K_SNI_F08           ,0,
    "sni_f09",       K_SNI_F09           ,0,
    "sni_f10",       K_SNI_F10           ,0,
    "sni_f11",       K_SNI_F11           ,0,
    "sni_f12",       K_SNI_F12           ,0,
    "sni_f13",       K_SNI_F13           ,0,
    "sni_f14",       K_SNI_F14           ,0,
    "sni_f15",       K_SNI_F15           ,0,
    "sni_f16",       K_SNI_F16           ,0,
    "sni_f17",       K_SNI_F17           ,0,
    "sni_f18",       K_SNI_F18           ,0,
    "sni_f19",       K_SNI_F19           ,0,
    "sni_f20",       K_SNI_F20           ,0,
    "sni_f21",       K_SNI_F21           ,0,
    "sni_f22",       K_SNI_F22           ,0,
    "sni_help",      K_SNI_HELP          ,0,
    "sni_home",      K_SNI_HOME          ,0,
    "sni_ins_char",  K_SNI_INSERT_CHAR   ,0,
    "sni_ins_line",  K_SNI_INSERT_LINE   ,0,
    "sni_ins_word",  K_SNI_INSERT_WORD   ,0,
    "sni_left_tab",  K_SNI_LEFT_TAB      ,0,
    "sni_lfarr",     K_SNI_CURSOR_LEFT   ,0,
    "sni_mode",      K_SNI_MODE          ,0,
    "sni_page",      K_SNI_PAGE          ,0,
    "sni_print",     K_SNI_PRINT         ,0,
    "sni_rtarr",     K_SNI_CURSOR_RIGHT  ,0,
    "sni_s_00",        K_SNI_S_DOUBLE_0    ,0,
    "sni_s_ce",        K_SNI_S_CE        ,0,
    "sni_s_compose",   K_SNI_S_COMPOSE     ,0,
    "sni_s_del_char",  K_SNI_S_DELETE_CHAR ,0,
    "sni_s_del_line",  K_SNI_S_DELETE_LINE ,0,
    "sni_s_del_word",  K_SNI_S_DELETE_WORD ,0,
    "sni_s_dnarr",     K_SNI_S_CURSOR_DOWN ,0,
    "sni_s_end",       K_SNI_S_END         ,0,
    "sni_s_endmarke",  K_SNI_S_ENDMARKE    ,0,
    "sni_s_f01",       K_SNI_S_F01       ,0,
    "sni_s_f02",       K_SNI_S_F02       ,0,
    "sni_s_f03",       K_SNI_S_F03       ,0,
    "sni_s_f04",       K_SNI_S_F04       ,0,
    "sni_s_f05",       K_SNI_S_F05       ,0,
    "sni_s_f06",       K_SNI_S_F06       ,0,
    "sni_s_f07",       K_SNI_S_F07       ,0,
    "sni_s_f08",       K_SNI_S_F08       ,0,
    "sni_s_f09",       K_SNI_S_F09       ,0,
    "sni_s_f10",       K_SNI_S_F10       ,0,
    "sni_s_f11",       K_SNI_S_F11       ,0,
    "sni_s_f12",       K_SNI_S_F12       ,0,
    "sni_s_f13",       K_SNI_S_F13       ,0,
    "sni_s_f14",       K_SNI_S_F14       ,0,
    "sni_s_f15",       K_SNI_S_F15       ,0,
    "sni_s_f16",       K_SNI_S_F16       ,0,
    "sni_s_f17",       K_SNI_S_F17       ,0,
    "sni_s_f18",       K_SNI_S_F18       ,0,
    "sni_s_f19",       K_SNI_S_F19       ,0,
    "sni_s_f20",       K_SNI_S_F20       ,0,
    "sni_s_f21",       K_SNI_S_F21       ,0,
    "sni_s_f22",       K_SNI_S_F22       ,0,
    "sni_s_help",      K_SNI_S_HELP      ,0,
    "sni_s_home",      K_SNI_S_HOME        ,0,
    "sni_s_ins_char",  K_SNI_S_INSERT_CHAR ,0,
    "sni_s_ins_line",  K_SNI_S_INSERT_LINE ,0,
    "sni_s_ins_word",  K_SNI_S_INSERT_WORD ,0,
    "sni_s_left_tab",  K_SNI_S_LEFT_TAB  ,0,
    "sni_s_lfarr",     K_SNI_S_CURSOR_LEFT ,0,
    "sni_s_mode",      K_SNI_S_MODE      ,0,
    "sni_s_page",      K_SNI_S_PAGE        ,0,
    "sni_s_print",     K_SNI_S_PRINT     ,0,
    "sni_s_rtarr",     K_SNI_S_CURSOR_RIGHT,0,
    "sni_s_scroll_dn", K_SNI_S_SCROLL_DOWN ,0,
    "sni_s_scroll_up", K_SNI_S_SCROLL_UP ,0,
    "sni_s_start",     K_SNI_S_START     ,0,
    "sni_s_uparr",     K_SNI_S_CURSOR_UP   ,0,
    "sni_s_user1",     K_SNI_S_USER1       ,0,
    "sni_s_user2",     K_SNI_S_USER2       ,0,
    "sni_s_user3",     K_SNI_S_USER3       ,0,
    "sni_s_user4",     K_SNI_S_USER4       ,0,
    "sni_s_user5",     K_SNI_S_USER5       ,0,
    "sni_s_tab",       K_SNI_S_TAB       ,0,
    "sni_scroll_dn", K_SNI_SCROLL_DOWN   ,0,
    "sni_scroll_up", K_SNI_SCROLL_UP     ,0,
    "sni_start",     K_SNI_START         ,0,
    "sni_tab",       K_SNI_TAB           ,0,
    "sni_uparr",     K_SNI_CURSOR_UP     ,0,
    "sni_user1",     K_SNI_USER1         ,0,
    "sni_user2",     K_SNI_USER2         ,0,
    "sni_user3",     K_SNI_USER3         ,0,
    "sni_user4",     K_SNI_USER4         ,0,
    "sni_user5",     K_SNI_USER5         ,0,
    "status",     K_STATUS,      0,

    "sunagain", K_SUN_AGAIN, 0,
    "suncopy",  K_SUN_COPY, 0,
    "suncut",   K_SUN_CUT, 0,
    "sunfind",  K_SUN_FIND, 0,
    "sunfront", K_SUN_FRONT, 0,
    "sunhelp",  K_SUN_HELP, 0,
    "sunopen",  K_SUN_OPEN, 0,
    "sunpaste", K_SUN_PASTE, 0,
    "sunprops", K_SUN_PROPS, 0,
    "sunstop", K_SUN_STOP, 0,
    "sunundo",  K_SUN_UNDO, 0,

    "termtype",   K_TERMTYPE,    0,
    "tn_ao",      K_TN_AO,      0,
    "tn_ayt",     K_TN_AYT,      0,
    "tn_ec",      K_TN_EC,       0,
    "tn_el",      K_TN_EL,       0,
    "tn_ga",      K_TN_GA,       CM_INV,
    "tn_ip",      K_TN_IP,       0,
    "tn_logout",  K_TN_LOGOUT,   0,
    "tn_naws",    K_TN_NAWS,     0,
    "tn_sak",     K_TN_SAK,      0,
    "tvibs",      K_TVIBS,       0,
    "tviclrln",   K_TVICLRLN,    0,
    "tviclrpg",   K_TVICLRPG,    0,
    "tvidelchar", K_TVIDELCHAR,  0,
    "tvidelln",   K_TVIDELLN,    0,
    "tvidnarr",   K_TVIDNARR,    0,
    "tvienter",   K_TVIENTER,    0,
    "tviesc",     K_TVIESC,      0,
    "tvif01",     K_TVIF01,      0,
    "tvif02",     K_TVIF02,      0,
    "tvif03",     K_TVIF03,      0,
    "tvif04",     K_TVIF04,      0,
    "tvif05",     K_TVIF05,      0,
    "tvif06",     K_TVIF06,      0,
    "tvif07",     K_TVIF07,      0,
    "tvif08",     K_TVIF08,      0,
    "tvif09",     K_TVIF09,      0,
    "tvif10",     K_TVIF10,      0,
    "tvif11",     K_TVIF11,      0,
    "tvif12",     K_TVIF12,      0,
    "tvif13",     K_TVIF13,      0,
    "tvif14",     K_TVIF14,      0,
    "tvif15",     K_TVIF15,      0,
    "tvif16",     K_TVIF16,      0,
    "tvihome",    K_TVIHOME,     0,
    "tviinschar", K_TVIINSCHAR,  0,
    "tviinsert",  K_TVIINSERT,   0,
    "tviinsln",   K_TVIINSLN,    0,
    "tvilfarr",   K_TVILFARR,    0,
    "tvipgnext",  K_TVIPGNEXT,   0,
    "tvipgprev",  K_TVIPGPREV,   0,
    "tviprtscn",  K_TVIPRTSCN,   0,
    "tvireplace", K_TVIREPLACE,  0,
    "tvireturn",  K_TVIRETURN,   0,
    "tvirtarr",   K_TVIRTARR,    0,
    "tvisbs",     K_TVISBS,      0,
    "tvisclrln",  K_TVISCLRLN,   0,
    "tvisclrpg",  K_TVISCLRPG,   0,
    "tvisdnarr",  K_TVISDNARR,   0,
    "tvisend",    K_TVISEND,     0,
    "tvisenter",  K_TVISENTER,   0,
    "tvisesc",    K_TVISESC,     0,
    "tvisf01",    K_TVISF01,     0,
    "tvisf02",    K_TVISF02,     0,
    "tvisf03",    K_TVISF03,     0,
    "tvisf04",    K_TVISF04,     0,
    "tvisf05",    K_TVISF05,     0,
    "tvisf06",    K_TVISF06,     0,
    "tvisf07",    K_TVISF07,     0,
    "tvisf08",    K_TVISF08,     0,
    "tvisf09",    K_TVISF09,     0,
    "tvisf10",    K_TVISF10,     0,
    "tvisf11",    K_TVISF11,     0,
    "tvisf12",    K_TVISF12,     0,
    "tvisf13",    K_TVISF13,     0,
    "tvisf14",    K_TVISF14,     0,
    "tvisf15",    K_TVISF15,     0,
    "tvisf16",    K_TVISF16,     0,
    "tvishome",   K_TVISHOME,    0,
    "tvislfarr",  K_TVISLFARR,   0,
    "tvisreturn", K_TVISRETURN,  0,
    "tvisrtarr",  K_TVISRTARR,   0,
    "tvissend",   K_TVISSEND,    0,
    "tvistab",    K_TVISTAB,     0,
    "tvisuparr",  K_TVISUPARR,   0,
    "tvitab",     K_TVITAB,      0,
    "tviuparr",   K_TVIUPARR,    0,
    "ucs2",       K_C_UNI16,     0,
    "udkf1",      K_UDKF1,       0,
    "udkf2",      K_UDKF2,       0,
    "udkf3",      K_UDKF3,       0,
    "udkf4",      K_UDKF4,       0,
    "udkf5",      K_UDKF5,       0,
    "udkdo",      K_UDKDO,       0,
    "udkf6",      K_UDKF6,       0,
    "udkf7",      K_UDKF7,       0,
    "udkf8",      K_UDKF8,       0,
    "udkf9",      K_UDKF9,       0,
    "udkf10",     K_UDKF10,      0,
    "udkf11",     K_UDKF11,      0,
    "udkf12",     K_UDKF12,      0,
    "udkf13",     K_UDKF13,      0,
    "udkf14",     K_UDKF14,      0,
    "udkf15",     K_UDKF15,      CM_INV,
    "udkf16",     K_UDKF16,      CM_INV,
    "udkf17",     K_UDKF17,      0,
    "udkf18",     K_UDKF18,      0,
    "udkf19",     K_UDKF19,      0,
    "udkf20",     K_UDKF20,      0,
    "udkhelp",    K_UDKHELP,     0,
    "uparr",      K_UPARR,       0,
    "upone",      K_UPONE,       0,
    "upscn",      K_UPSCN,       0,
    "url",        K_CURSOR_URL,  0,
    "url-highlight", K_URLHILT,  0,
    "wybs",       K_WYBS,        0,
    "wyclrln",    K_WYCLRLN,     0,
    "wyclrpg",    K_WYCLRPG,     0,
    "wydelchar",  K_WYDELCHAR,   0,
    "wydelln",    K_WYDELLN,     0,
    "wydnarr",    K_WYDNARR,     CM_INV,
    "wyenter",    K_WYENTER,     0,
    "wyesc",      K_WYESC,       0,
    "wyf01",      K_WYF01,       0,
    "wyf02",      K_WYF02,       0,
    "wyf03",      K_WYF03,       0,
    "wyf04",      K_WYF04,       0,
    "wyf05",      K_WYF05,       0,
    "wyf06",      K_WYF06,       0,
    "wyf07",      K_WYF07,       0,
    "wyf08",      K_WYF08,       0,
    "wyf09",      K_WYF09,       0,
    "wyf10",      K_WYF10,       0,
    "wyf11",      K_WYF11,       0,
    "wyf12",      K_WYF12,       0,
    "wyf13",      K_WYF13,       0,
    "wyf14",      K_WYF14,       0,
    "wyf15",      K_WYF15,       0,
    "wyf16",      K_WYF16,       0,
    "wyf17",      K_WYF17,       0,
    "wyf18",      K_WYF18,       0,
    "wyf19",      K_WYF19,       0,
    "wyf20",      K_WYF20,       0,
    "wyhome",     K_WYHOME,      0,
    "wyinschar",  K_WYINSCHAR,   0,
    "wyinsert",   K_WYINSERT,    0,
    "wyinsln",    K_WYINSLN,     0,
    "wylfarr",    K_WYLFARR,     CM_INV,
    "wypgnext",   K_WYPGNEXT,    0,
    "wypgprev",   K_WYPGPREV,    0,
    "wyprtscn",   K_WYPRTSCN,    0,
    "wyreplace",  K_WYREPLACE,   0,
    "wyreturn",   K_WYRETURN,    0,
    "wyrtarr",    K_WYRTARR,     CM_INV,
    "wysbs",      K_WYSBS,       0,
    "wysclrln",   K_WYSCLRLN,    0,
    "wysclrpg",   K_WYSCLRPG,    0,
    "wysdnarr",   K_WYSDNARR,    0,
    "wysend",     K_WYSEND,      0,
    "wysenter",   K_WYSENTER,    0,
    "wysesc",     K_WYSESC,      0,
    "wysf01",     K_WYSF01,      0,
    "wysf02",     K_WYSF02,      0,
    "wysf03",     K_WYSF03,      0,
    "wysf04",     K_WYSF04,      0,
    "wysf05",     K_WYSF05,      0,
    "wysf06",     K_WYSF06,      0,
    "wysf07",     K_WYSF07,      0,
    "wysf08",     K_WYSF08,      0,
    "wysf09",     K_WYSF09,      0,
    "wysf10",     K_WYSF10,      0,
    "wysf11",     K_WYSF11,      0,
    "wysf12",     K_WYSF12,      0,
    "wysf13",     K_WYSF13,      0,
    "wysf14",     K_WYSF14,      0,
    "wysf15",     K_WYSF15,      0,
    "wysf16",     K_WYSF16,      0,
    "wysf17",     K_WYSF17,      0,
    "wysf18",     K_WYSF18,      0,
    "wysf19",     K_WYSF19,      0,
    "wysf20",     K_WYSF20,      0,
    "wyshome",    K_WYSHOME,     0,
    "wyslfarr",   K_WYSLFARR,    0,
    "wysreturn",  K_WYSRETURN,   0,
    "wysrtarr",   K_WYSRTARR,    0,
    "wyssend",    K_WYSSEND,     0,
    "wystab",     K_WYSTAB,      0,
    "wysuparr",   K_WYSUPARR,    0,
    "wytab",      K_WYTAB,       0,
    "wyuparr",    K_WYUPARR,     CM_INV
};
int nkverbs = (sizeof(kverbs) / sizeof(struct keytab));
#endif /* NOKVERBS */

#ifndef NOLOCAL
void
clickkeys(void)
{
    if ( keyclick )
#ifdef NT
    Beep( 1600, 15 ) ;
#else /* NT */
    DosBeep( 1600, 15 ) ;
#endif /* NT */
}
#endif /* NOLOCAL */

USHORT
getshiftstate( void ) {
#ifdef NT
    /* ??? returns VK_SHIFT, VK_CONTROL, VK_MENU (ALT) */
    /* ??? these probably do not match OS/2 */
    BYTE keystate = 0;
    GetKeyboardState(&keystate);
    return keystate;
#else /* NT */
    KBDINFO k ;

    memset( &k, 0, sizeof(k) ) ;
    k.cb = sizeof(k) ;
    KbdGetStatus( &k, KbdHandle ) ;
    return k.fsState ;
#endif  /* NT */
}


/* Begin Keyboard Buffer Code
   This is a simple implementation of a circular queue with access
   protected by a Mutual Exclusion Semaphore
*/

#define KEY_BUF_SIZE (65536 / sizeof(con_event))
con_event Keystroke[VNUM][KEY_BUF_SIZE] ;
int start[VNUM]={0,0,0}, end[VNUM]={0,0,0} ;

void
keybufinit( void ) {
   int i,j ;

    CreateEventAvailSem( FALSE );
    CreateKeyStrokeMutex( TRUE ) ;
    CreateKeyboardMutex( FALSE ) ;
    for ( j=0 ; j<VNUM ; j++ )
    {
        for ( i = 0 ; i < KEY_BUF_SIZE ; i++ )
            memset(&Keystroke[j][i],0,sizeof(con_event)) ;
        start[j] = 0 ;
        end[j] = 0 ;
        ReleaseKeyStrokeMutex(j) ;
    }
}

void
keybufcleanup( void ) {
    CloseKeyStrokeMutex() ;
    CloseKeyboardMutex() ;
    CloseEventAvailSem() ;
}

int
keyinbuf( int kmode ) {
    int rc = 0, count = 0;

    rc = RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;

    if ( start[kmode] != end[kmode] )
    {
        int i = start[kmode] ;
        for ( ; i != end[kmode] ; i++ )
        {
            if ( i == KEY_BUF_SIZE )
              i = 0 ;
            if ( Keystroke[kmode][i].type == key ) {
              count++ ;
            }
        }
    }
    rc = ReleaseKeyStrokeMutex(kmode) ;
    return count;
}

int
charinbuf( int kmode ) {
    int rc = 0, count = 0;

    rc = RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;

    if ( start[kmode] != end[kmode] )
    {
        int i = start[kmode] ;
        for ( ; i != end[kmode] ; i++ )
        {
            if ( i == KEY_BUF_SIZE )
              i = 0 ;
            if ( Keystroke[kmode][i].type == key &&
                 Keystroke[kmode][i].key.scancode < 256 )
            {
                count++ ;
            } else if ( Keystroke[kmode][i].type == macro ) {
                count += strlen(Keystroke[kmode][i].macro.string);
            } else if ( Keystroke[kmode][i].type == literal ) {
                count += strlen(Keystroke[kmode][i].literal.string);
            }
        }
    }
    rc = ReleaseKeyStrokeMutex(kmode) ;
    return count;
}

int
evtinbuf( int kmode ) {
    int rc = 0, count = 0 ;

    debug(F111,"evtinbuf","kmode",kmode);
    rc = RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    debug(F111,"evtinbuf","RequestKeyStrokeMutex",rc);

    debug(F111,"evtinbuf","start[kmode]",start[kmode]);
    debug(F111,"evtinbuf","end[kmode]",end[kmode]);
    if ( start[kmode] != end[kmode] )
    {
        count = (end[kmode] - start[kmode] + KEY_BUF_SIZE)%KEY_BUF_SIZE;
    }
    rc = ReleaseKeyStrokeMutex(kmode) ;
    debug(F111,"evtinbuf","ReleaseKeyStrokeMutex",rc);
    debug(F111,"evtinbuf","count",count);
    return count ;
}

int
putkeystr( int kmode, char * s )
{
    char * p ;
    int rc = 0 ;
    for ( p=s; *p && !rc ; p++ )
      rc = putkey( kmode, *p ) ;
    return rc ;
}

int
putkey( int kmode, int k ) {
    extern MACRO * macrotab ;
    int rc = 0, count = 0 ;
    con_event evt ;

#ifndef NOSETKEY
    evt = mapkey(k) ;
#else /* NOSETKEY */
    evt.type = key ;
    evt.key.scancode = k;
#endif /* NOSETKEY */

    debug(F111,"putkey","evt.type",(int)evt.type);
#ifndef NOKVERBS
    if ( !VscrnIsPopup(kmode) &&
         os2gks && evt.type == kverb ) {
        putkverb(kmode, evt.kverb.id & ~F_KVERB);
    }
    else if ( os2gks && evt.type != error && evt.type != key) {
        putevent( kmode, evt ) ;
    }
    else
#endif /* NOKVERBS */
    {
        if (
#ifndef NOKVERBS
             !os2gks ||
#endif /* NOKVERBS */
             evt.type == error ) {
            evt.type = key ;
            evt.key.scancode = k ;
            debug(F111,"putkey()","evt.key.scancode",k);
        }
        RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
        while ( (start[kmode] - end[kmode] == 1) ||
                ( start[kmode] == 0 && end[kmode] == KEY_BUF_SIZE - 1 ) )
            /* Buffer is full */
        {
            ReleaseKeyStrokeMutex(kmode) ;
            if ( ++count > 4 ) {
                bleep(BP_WARN);
                return(-1);
            }
            msleep(250);
            RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
        }

        Keystroke[kmode][end[kmode]++] = evt ;
        if ( end[kmode] == KEY_BUF_SIZE )
            end[kmode] = 0 ;
        PostEventAvailSem(kmode)  ;
        ReleaseKeyStrokeMutex(kmode) ;
    }
    return rc ;
}

#ifndef NOKVERBS
struct kverb_call {
    int kmode;
    int km;
};

static void
kverb_thread(struct kverb_call *kc)
{
    if ( kc ) {
        kbdCallsKverb = 1;
        dokverb( kc->kmode, kc->km );
        kbdCallsKverb = 0;
        free(kc);
    }
}

/* 
 * K_DUMP will spawn a thread from dokverb()
 */

int
putkverb( int kmode, int k ) {
    int rc = 0 ;
    int count = 0;
    con_event evt ;
    int km = 0 ;
    extern enum markmodes markmodeflag[VNUM];
    extern int SysInited;

    evt.type = kverb ;
    evt.kverb.id = k | F_KVERB ;

    /* Handle Scroll and Mark events immediately */
    km = evt.kverb.id & ~F_KVERB;

    if (SysInited) {
        if (km >= K_ROLLMIN && km <= K_HOMSCN ||
             /* Can't let K_GO_BOOK or K_GOTO be executed in this thread */
             km >= K_MARK_START && km <= K_MARK_SELECT ||
             km >= K_ARR_MIN && km <= K_ARR_MAX && markmodeflag[kmode] != notmarking ||
             km >= K_LFONE && km <= K_RTALL ||
             km >= K_KB_ENG && km <= K_KB_WP ||
             km == K_LOGDEBUG ||
             km == K_DEBUG ||
             km == K_HOLDSCRN ||
             km == K_EXIT ||
             km == K_HANGUP ||
	     km == K_DUMP && markmodeflag[kmode] != notmarking ||
#ifdef OS2MOUSE
             km >= K_MOUSE_MIN && km <= K_MOUSE_MAX
#endif /* OS2MOUSE */
             )
        {
            kbdCallsKverb = 1;
            dokverb( kmode, km );
            kbdCallsKverb = 0;
            return 0;
        }
        else if ( km == K_QUIT ||
                  km == K_CURSOR_URL ||
                  km >= K_TN_AO && km <= K_TN_LOGOUT || km == K_TN_AYT ||
                  km == K_LOGON || km == K_LOGOFF || km == K_STATUS  ||
                  km == K_RESET || km == K_DUMP ||
                  km == K_TN_IP
                  )
        {
            struct kverb_call * kc = malloc(sizeof(struct kverb_call));
            if ( kc ) {
                kc->kmode = kmode;
                kc->km = km;
                _beginthread(&kverb_thread,
#ifndef NT
                              0,
#endif /* NT */
                              65536,
                              (void *)kc);
                return 0;
            }
        }
    }

    RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    while ( (start[kmode] - end[kmode] == 1) ||
            ( start[kmode] == 0 && end[kmode] == KEY_BUF_SIZE - 1 ) )
       /* Buffer is full */
    {
        ReleaseKeyStrokeMutex(kmode) ;
        if ( ++count > 4 ) {
            bleep(BP_WARN);
            return(-1);
        }
        msleep(250);
        RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    }

    Keystroke[kmode][end[kmode]++] = evt ;
    if ( end[kmode] == KEY_BUF_SIZE )
      end[kmode] = 0 ;
    PostEventAvailSem(kmode)  ;
    ReleaseKeyStrokeMutex(kmode) ;

    return rc ;
}
#endif /* NOKVERBS */
int
puterror( int kmode ) {
    int rc = 0, count = 0 ;
    con_event evt ;

    evt.type = error ;

    RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    while ( (start[kmode] - end[kmode] == 1) || ( start[kmode] == 0 && end[kmode] == KEY_BUF_SIZE - 1 ) )
       /* Buffer is full */
    {
        ReleaseKeyStrokeMutex(kmode) ;
        if ( ++count > 4 ) {
            bleep(BP_WARN);
            return(-1);
        }
        msleep(250);
        RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    }

    Keystroke[kmode][end[kmode]++] = evt ;
    if ( end[kmode] == KEY_BUF_SIZE )
      end[kmode] = 0 ;
    PostEventAvailSem(kmode)  ;
    ReleaseKeyStrokeMutex(kmode) ;

    return rc ;
}

int
putesc( int kmode, int k ) {
    int rc = 0, count = 0 ;
    con_event evt ;

    evt.type = esc ;
    evt.esc.key = k & ~F_ESC ;

    RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    while ( (start[kmode] - end[kmode] == 1) || ( start[kmode] == 0 && end[kmode] == KEY_BUF_SIZE - 1 ) )
       /* Buffer is full */
    {
        ReleaseKeyStrokeMutex(kmode) ;
        if ( ++count > 4 ) {
            bleep(BP_WARN);
            return(-1);
        }
        msleep(250);
        RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    }

    Keystroke[kmode][end[kmode]++] = evt ;
    if ( end[kmode] == KEY_BUF_SIZE )
      end[kmode] = 0 ;
    PostEventAvailSem(kmode)  ;
    ReleaseKeyStrokeMutex(kmode) ;

    return rc ;
}

int
putcsi( int kmode, int k ) {
    int rc = 0, count = 0 ;
    con_event evt ;

    evt.type = csi ;
    evt.csi.key = k & ~F_CSI ;

    RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    while ( (start[kmode] - end[kmode] == 1) || ( start[kmode] == 0 && end[kmode] == KEY_BUF_SIZE - 1 ) )
       /* Buffer is full */
    {
        ReleaseKeyStrokeMutex(kmode) ;
        if ( ++count > 4 ) {
            bleep(BP_WARN);
            return(-1);
        }
        msleep(250);
        RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    }

    Keystroke[kmode][end[kmode]++] = evt ;
    if ( end[kmode] == KEY_BUF_SIZE )
      end[kmode] = 0 ;
    PostEventAvailSem(kmode)  ;
    ReleaseKeyStrokeMutex(kmode) ;

    return rc ;
}

int
putmacro( int kmode, char * s ) {
    int rc = 0, count = 0 ;
    con_event evt ;

    evt.type = macro ;
    evt.macro.string = strdup(s) ;

    RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    while ( (start[kmode] - end[kmode] == 1) || ( start[kmode] == 0 && end[kmode] == KEY_BUF_SIZE - 1 ) )
      /* Buffer is full */
    {
        ReleaseKeyStrokeMutex(kmode) ;
        if ( ++count > 4 ) {
            bleep(BP_WARN);
            return(-1);
        }
        msleep(250);
        RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    }
    Keystroke[kmode][end[kmode]++] = evt ;
    if ( end[kmode] == KEY_BUF_SIZE )
      end[kmode] = 0 ;
    PostEventAvailSem(kmode)  ;
    ReleaseKeyStrokeMutex(kmode) ;

    return rc ;
}

int
putliteral( int kmode, char * s ) {
    int rc = 0, count = 0 ;
    con_event evt ;

    evt.type = literal ;
    evt.literal.string = strdup(s) ;

    RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    while ( (start[kmode] - end[kmode] == 1) || ( start[kmode] == 0 && end[kmode] == KEY_BUF_SIZE - 1 ) )
      /* Buffer is full */
    {
        ReleaseKeyStrokeMutex(kmode) ;
        if ( ++count > 4 ) {
            bleep(BP_WARN);
            return(-1);
        }
        msleep(250);
        RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    }
    Keystroke[kmode][end[kmode]++] = evt ;
    if ( end[kmode] == KEY_BUF_SIZE )
      end[kmode] = 0 ;
    PostEventAvailSem(kmode)  ;
    ReleaseKeyStrokeMutex(kmode) ;

    return rc ;
}
#ifdef OS2MOUSE
int
putclick( int kmode, char but, char alt, char ctrl, char shift, char dbl, char drag ) {
    int rc = 0, count = 0 ;
    con_event evt ;

    evt.type = mouse ;
    evt.mouse.button = but;
    evt.mouse.alt    = (alt != 0);
    evt.mouse.ctrl   = (ctrl != 0);
    evt.mouse.shift  = (shift != 0);
    evt.mouse.dbl    = (dbl != 0);
    evt.mouse.drag   = (drag != 0);

    RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    while ( (start[kmode] - end[kmode] == 1) || ( start[kmode] == 0 && end[kmode] == KEY_BUF_SIZE - 1 ) )
      /* Buffer is full */
    {
        ReleaseKeyStrokeMutex(kmode) ;
        if ( ++count > 4 ) {
            bleep(BP_WARN);
            return(-1);
        }
        msleep(250);
        RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    }
    Keystroke[kmode][end[kmode]++] = evt ;
    if ( end[kmode] == KEY_BUF_SIZE )
      end[kmode] = 0 ;
    PostEventAvailSem(kmode)  ;
    ReleaseKeyStrokeMutex(kmode) ;

    return rc ;
}
#endif /* OS2MOUSE */

int
putevent( int kmode, con_event e ) {
    int rc = 0;
    con_event evt ;

    switch ( e.type ) {
    case key:
        rc = putkey( kmode, e.key.scancode ) ;
        break;
#ifndef NOKVERBS
    case kverb:
        rc = putkverb( kmode, e.kverb.id ) ;
        break;
#endif /* NOKVERBS */
    case macro:
        rc = putmacro( kmode, e.macro.string ) ;
        break;
    case literal:
        rc = putliteral( kmode, e.literal.string ) ;
        break;
#ifdef OS2MOUSE
    case mouse:
        rc = putclick( kmode, e.mouse.button, e.mouse.alt, e.mouse.ctrl,
                       e.mouse.shift, e.mouse.dbl, e.mouse.drag ) ;
        break;
#endif /* OS2MOUSE */
    case csi:
        rc = putcsi( kmode, e.csi.key ) ;
        break;
    case esc:
        rc = putesc( kmode, e.esc.key ) ;
        break;
    case error:
        rc = puterror( kmode );
        break;
    };

    return rc ;
}

int
getevent( int kmode, con_event * evt ) {
    int rc = 0, fc = 0 ;
    ULONG PostCount ;

    fc = RequestKeyStrokeMutex( kmode, SEM_INDEFINITE_WAIT ) ;
    debug(F111,"getevent","RequestKeyStrokeMutex()",fc);
    if ( start[kmode] != end[kmode] ) {
        *evt = Keystroke[kmode][start[kmode]] ;
        memset(&Keystroke[kmode][start[kmode]],0,sizeof(con_event)) ;
        start[kmode]++ ;

        if ( start[kmode] == KEY_BUF_SIZE )
          start[kmode] = 0 ;

        if ( start[kmode] == end[kmode] ) {
            fc = ResetEventAvailSem(kmode) ;
            debug(F111,"getevent","ResetEventAvailSem()",fc);
        }
        rc++ ;
    }
    else
    {
        evt->type = error ;
    }
    fc = ReleaseKeyStrokeMutex(kmode) ;
    debug(F111,"getevent","ReleaseKeyStrokeMutex()",fc);
    return rc ;
}

#ifndef NOLOCAL
#ifndef KUI
/* Begin Keyboard Handler Thread Code */
int
KbdHandlerInit( void ) {
    int rc = 0 ;

    debug(F100,"KbdHandlerInit called","",0);
    KbdActive = 1 ;

    CreateKbdThreadSem( FALSE ) ;
    tidKbdHandler = (TID) ckThreadBegin( &KbdHandlerThread,
                                         THRDSTKSIZ, 0, FALSE, 0 ) ;
    if ( tidKbdHandler == (TID) -1 ) {
        printf( "Sorry, can't create KbdHandlerThread\n" ) ;
        rc = -1 ;
    }
    else {
        WaitAndResetKbdThreadSem( SEM_INDEFINITE_WAIT ) ;
    }
    return rc ;
}

int
KbdHandlerCleanup( void ) {
    APIRET rc=0 ;
    int n = 0;
    if ( !tidKbdHandler )
        return(0);

    debug(F100,"KbdHandlerCleanup called","",0);
        ResetKbdThreadSem();
    KbdActive = 0 ;
    while ( !kbdCallsKverb && !WaitAndResetKbdThreadSem( 1000 ) && n++ < 5) {
        debug(F111,"Waiting for KbdThreadSem","n",n) ;
    }
    tidKbdHandler = (TID) 0 ;   /* This will be closed by the thread */
    CloseKbdThreadSem() ;
    return 0 ;
}
#endif /* KUI */
#endif /* NOLOCAL */

/* KbdHandlerThread() needs to be updated to support non-local mode
    and alternative inputs (pipes) */

#ifdef NT
/* NT version of KbdHandlerThread handles all Keyboard, Mouse, and Windows
   resizing events */

#ifndef NOLOCAL
win32WindowEvent( int mode, WINDOW_BUFFER_SIZE_RECORD r )
{
    LONG sz ;
    extern int ttmdm, me_naws;
#ifdef TCPSOCKET
_PROTOTYP( int tn_snaws, (void) ) ;
#ifdef RLOGCODE
_PROTOTYP( int rlog_naws, (void) ) ;
#endif /* RLOGCODE */
#endif /* TCPSOCKET */
#ifndef COMMENT
     char buf[64] ;
     sprintf(buf, "win32 Window Event: (%d,%d)\n", r.dwSize.X, r.dwSize.Y ) ;
     debug(F100,buf,"",0) ;
#endif /* COMMENT */
     if ( VscrnGetWidth(mode) != r.dwSize.X || VscrnGetHeight(mode) != r.dwSize.Y ){
         sz = (VscrnGetEnd(mode) - VscrnGetTop(mode)
                + VscrnGetBufferSize(mode) + 1)%VscrnGetBufferSize(mode) ;
         VscrnSetWidth( mode, r.dwSize.X ) ;
         VscrnSetHeight( mode, r.dwSize.Y ) ;
         VscrnScroll( mode, UPWARD, 1, sz, sz, TRUE, SP ) ;
         cleartermscreen(mode);
#ifdef TCPSOCKET
         if (ttmdm < 0 && TELOPT_ME(TELOPT_NAWS)) {
             tn_snaws() ;
#ifdef RLOGCODE
             rlog_naws() ;
#endif /* RLOGCODE */
#ifdef SSHBUILTIN
             ssh_snaws();
#endif /* SSHBUILTIN */
         }
#endif /* TCPSOCKET */
     }
}
#endif /* NOLOCAL */

int
getKeycodeFromKeyRec( KEY_EVENT_RECORD * pkeyrec, WORD * buf, int chcount )
{
    int c= -1, k, km ;
    extern int ckconraw ;
    int keycount = 1 ;
    char ch;
    static char keystate[256] ;
#ifdef CKOUNI_IN
    WORD xbuf[8] ;
#else
    CHAR xbuf[8];
#endif
    KEY_EVENT_RECORD keyrec = *pkeyrec;

    /* The following are used in Win95 only to simulate the Keyboard Layout Hotkey */
    static int altdown = 0, altup = 0, ctrlup = 0, ctrldown = 0,
               shiftdown = 0, shiftup = 0;
    static int getlist = 1;
    static HKL hkllist[64];
    static int nhkl = 0;
    static int ihkl = 0;

    /* In case the caller doesn't need to know what the dead key values are */
    if ( buf == NULL || chcount == 0 ) {
        buf = (WORD *) xbuf;
        chcount = 8;
    }

    /* begin computation of keycode value */

#ifndef KUI
    debug(F111,"win32keyrec","keyrec.wVirtualKeyCode",keyrec.wVirtualKeyCode);
    debug(F111,"win32keyrec","keyrec.wVirtualScanCode",keyrec.wVirtualScanCode);
    debug(F111,"win32keyrec","keyrec.uChar.AsciiChar",keyrec.uChar.AsciiChar);
    debug(F111,"win32keyrec","keyrec.dwControlKeyState",keyrec.dwControlKeyState);

    if ( isWin95() ) {
        /* This code should work but ActivateKeyboardLayout() appears to have no */
        /* effect on Win95 Console programs.  So instead we use a combination of */
        /* GetKeyboardLayoutList(), MapVirtualKeyEx(), and ToAsciiEx() to do what*/
        /* CONAGENT.EXE should have down from the very beginning.  As a toggle   */
        /* we implement both Alt-Shift and Ctrl-Shift.  Ctrl-Alt-Shift is not a  */
        /* valid toggle.                                                         */

        if ( getlist ) {
            nhkl = GetKeyboardLayoutList(64,hkllist);

            memset(keystate,0,256);
            for ( ihkl = 0; ihkl < nhkl ; ihkl++ ) {
                keycount = ToAsciiEx( 65,
                                      30 | 0x80,
                                      keystate,
                                      buf,
                                      FALSE,
                                      hkllist[ihkl]);
                if ( keycount == 1 )
                    break;
            }
            if ( ihkl == nhkl ) {
                debug(F111,"FATAL ERROR: No Latin Keyboard Layouts are installed",
                       "number of layouts installed",nhkl);
                printf("FATAL ERROR: No ASCII compatible Input Locale is installed\r\n");
                printf("  on this computer.  Please install an Input Locale that supports\r\n");
                printf("  English, French, German, or one of the other Input languages\r\n");
                printf("  supported by the ISO Latin character sets.  Use:\r\n\n");
                printf("     Start Menu->Settings->Control Panel->Keyboard:Input Locales\r\n\n");
                bleep(BP_FAIL);
                sleep(5);
                doexit(BAD_EXIT,10);
            }
            getlist = 0;
        }

        if ( keyrec.bKeyDown ) {
            switch ( keyrec.wVirtualKeyCode ) {
            case VK_MENU:
                altdown = 1;
                break;
            case VK_SHIFT:
                shiftdown = 1;
                break;
            case VK_CONTROL:
                ctrldown = 1;
                break;
            default:
                altdown = altup = shiftdown = shiftup = ctrldown = ctrlup = 0;
            }
        } else {
            HKL prevhkl = 0;
            switch ( keyrec.wVirtualKeyCode ) {
            case VK_MENU:
                if ( altdown && shiftdown && shiftup) {
                    ihkl = (ihkl+1)%nhkl;
                    altdown = altup = shiftdown = shiftup = ctrldown = ctrlup = 0;
                } else {
                    altup = 1;
                }
                break;
            case VK_CONTROL:
                if ( ctrldown && shiftdown && shiftup) {
                    ihkl = (ihkl+1)%nhkl;
                    altdown = altup = shiftdown = shiftup = ctrldown = ctrlup = 0;
                } else {
                    ctrlup = 1;
                }
                break;
            case VK_SHIFT:
                if ( ((altdown && altup && !ctrldown) ||
                       (ctrldown && ctrlup && !altdown)) && shiftdown ) {
                    ihkl = (ihkl+1)%nhkl;
                    altdown = altup = shiftdown = shiftup = ctrldown = ctrlup = 0;
                } else {
                    shiftup = 1;
                }
                break;
            }
        }

        if ( 1 || keyrec.bKeyDown ) {
            memset(keystate,0,256);

            if ( keyrec.dwControlKeyState & CAPSLOCK_ON ) {
                debug(F110,"win32key","CAPSLOCK_ON",0);
                keystate[VK_CAPITAL] = 0x01 ;
            }
            if ( keyrec.dwControlKeyState & NUMLOCK_ON ) {
                debug(F110,"win32key","NUMLOCK_ON",0);
                keystate[VK_NUMLOCK] = 0x01 ;
            }
            if ( keyrec.dwControlKeyState & SCROLLLOCK_ON ) {
                debug(F110,"win32key","SCROLLLOCK_ON",0);
                keystate[VK_SCROLL] = 0x01 ;
            }
            if ( keyrec.dwControlKeyState & LEFT_CTRL_PRESSED ) {
                debug(F110,"win32key","LEFT_CTRL_PRESSED",0);
                keystate[VK_CONTROL] = 0x80 ;
                keystate[VK_LCONTROL] = 0x80 ;
            }
            if ( keyrec.dwControlKeyState & RIGHT_CTRL_PRESSED ) {
                debug(F110,"win32key","RIGHT_CTRL_PRESSED",0);
                keystate[VK_CONTROL] = 0x80 ;
                keystate[VK_RCONTROL] = 0x80 ;
            }
            if ( keyrec.dwControlKeyState & LEFT_ALT_PRESSED ) {
                keystate[VK_MENU] = 0x80 ;
                keystate[VK_LMENU] = 0x80 ;
                debug(F111,"LeftAltPressed","keyrec.wVirtualKeyCode",
                       keyrec.wVirtualKeyCode);
                debug(F111,"LeftAltPressed","keyrec.wVirtualScanCode",
                       keyrec.wVirtualScanCode);
                debug(F111,"LeftAltPressed","keyrec.uChar.AsciiChar",
                       keyrec.uChar.AsciiChar);
                debug(F111,"LeftAltPressed","keyrec.dwControlKeyState",
                       keyrec.dwControlKeyState);
            }
            if ( keyrec.dwControlKeyState & RIGHT_ALT_PRESSED ) {
                keystate[VK_MENU] = 0x80 ;
                keystate[VK_RMENU] = 0x80 ;
                debug(F111,"RightAltPressed","keyrec.wVirtualKeyCode",
                       keyrec.wVirtualKeyCode);
                debug(F111,"RightAltPressed","keyrec.wVirtualScanCode",
                       keyrec.wVirtualScanCode);
                debug(F111,"RightAltPressed","keyrec.uChar.AsciiChar",
                       keyrec.uChar.AsciiChar);
                debug(F111,"RightAltPressed","keyrec.dwControlKeyState",
                       keyrec.dwControlKeyState);
                if ( (isWin95() && win95altgr) ||
                     keyrec.dwControlKeyState & LEFT_CTRL_PRESSED ) {
                    debug(F110,"win32key","AltGr combination",0);
                    keystate[VK_CONTROL] = 0x80 ;
                    keyrec.dwControlKeyState |= RIGHT_ALT_PRESSED | LEFT_CTRL_PRESSED;

#ifdef COMMENT
                    /* Should the ENHANCED_KEY flag be set for Alt-Gr?  */
                    /* It is not on NT/2000.  Must think about this.    */
                    keyrec.dwControlKeyState |= ENHANCED_KEY ;
#endif /* COMMENT */
                }
            }
            if ( keyrec.dwControlKeyState & SHIFT_PRESSED ) {
                debug(F110,"win32key","SHIFT_PRESSED",0);
                keystate[VK_SHIFT] = 0x80 ;
                keystate[VK_LSHIFT] = 0x80 ;
                keystate[VK_RSHIFT] = 0x80 ;
            }

            /* There is a comment in the ToAsciiEx() docs that says that  */
            /* the high order bit of the Scan Code must be set if the key */
            /* is not currently being pressed. */
            if ( !keyrec.bKeyDown )
                keyrec.wVirtualScanCode |= 0x80;
#ifdef CKOUNI_IN
            keycount = ToUnicode( keyrec.wVirtualKeyCode,
                                  keyrec.wVirtualScanCode,
                                  keystate,
                                  buf,
                                  chcount,
                                  FALSE
                                  );
#else /* CKOUNI_IN */
            if ( nhkl > 0 ) {
                /* We must convert from the scancode to the KeyCode */
                /* which we can then use to generate an Ascii value */
                /* all using a specific Keyboard Layout             */
                UINT wVirtualKeyCode = MapVirtualKeyEx( keyrec.wVirtualScanCode,
                                                        1, hkllist[ihkl]);
                keycount = ToAsciiEx(wVirtualKeyCode,
                                      keyrec.wVirtualScanCode,
                                      keystate,
                                      buf,
                                      FALSE,
                                      hkllist[ihkl]);
            }
            else
                keycount = ToAscii( keyrec.wVirtualKeyCode,
                                    keyrec.wVirtualScanCode,
                                    keystate,
                                    buf,
                                    FALSE);
#endif /* CKOUNI_IN */
            debug(F111,"win32key","keycount",keycount);
            if ( keycount < 0 ) {
                debug(F111,"win32Key","DeadKey",buf[0]);
                return -1;
            }
        }

        if ( keycount > 0 ) {
            if ( buf[0] < 128 ) {
                debug(F111,"win32Key","buf[0]",buf[0]);
                keyrec.uChar.AsciiChar = buf[0] ;
            }
            else if ( buf[0] < 256 ) {
                char ch = (char) buf[0];
                debug(F111,"win32Key","buf[0]",ch);
                CharToOemBuff( &ch, &(keyrec.uChar.AsciiChar), 1);
                debug(F111,"win32Key","keyrec.uChar.AsciiChar",
                       keyrec.uChar.AsciiChar);
            }
            else
                debug(F111,"win32Key - UNICODE character","buf[0]",buf[0]);
        }
        else {
            debug(F110,"win32Key","No translation",0);
            /* this is a really ugly hack to handle the fact that */
            /* win95 generates the wrong keycode for DECIMAL      */
            /* it returns DELETE instead                          */
            if ( keycount == 0 &&
                 ( keyrec.uChar.AsciiChar == '.' ||
                   keyrec.uChar.AsciiChar == ',') &&
                 keyrec.wVirtualKeyCode == 46 &&
                 keyrec.wVirtualScanCode == 83 ) {
                debug( F111,"win32Key","is this DECIMAL?",
                       keyrec.uChar.AsciiChar);
                }
            else {
                keyrec.uChar.AsciiChar = 0 ;
            }
        }
    } /* end of Win95 processing */
#endif /* KUI */

    if ( keyrec.bKeyDown &&
         !(keyrec.wVirtualKeyCode == 0x0010 ||  /* SHIFT */
            keyrec.wVirtualKeyCode == 0x0011 || /* CTRL */
            keyrec.wVirtualKeyCode == 0x0012)   /* ALT */
         ){

        if (keyrec.uChar.AsciiChar || keyrec.wVirtualKeyCode) {

            if ( keyrec.uChar.AsciiChar == 0x0 ||
                 (keyrec.dwControlKeyState & CONTROL) &&
                 (keyrec.dwControlKeyState & ALT) ||
                 keyrec.dwControlKeyState & ENHANCED_KEY )
            {

                /* Then we must use a scan code */
                c = keyrec.wVirtualKeyCode | KEY_SCAN ;

                /* We don't check for SHIFT of ascii values because they */
                /* just generate other ASCII values */
                if ( keyrec.dwControlKeyState & SHIFT )
                  c |= KEY_SHIFT ;
            }
            else
            {
                /* We have an ascii value */
                c = keyrec.uChar.AsciiChar ;
                switch (c) {    /* Handle ambiguous keypad and space keys */
                case ESC:
                    if ( keyrec.dwControlKeyState & SHIFT )
                      c |= KEY_SHIFT ;
                    break;
                case '\t':
                    if ( keyrec.wVirtualKeyCode == VK_TAB )
                    {
                        c = keyrec.wVirtualKeyCode | KEY_SCAN |
                          ( keyrec.dwControlKeyState & ENHANCED_KEY ? KEY_ENHANCED : 0x0 ) |
                            ( keyrec.dwControlKeyState & SHIFT_PRESSED ? KEY_SHIFT : 0x0 ) ;
                    }
                    break;

                case DEL:
                    if ( keyrec.wVirtualKeyCode == VK_DELETE )        /* look at the scan code */
                    {
                        c = keyrec.wVirtualKeyCode | KEY_SCAN |
                          ( keyrec.dwControlKeyState & ENHANCED_KEY ? KEY_ENHANCED : 0x0 ) |
                            ( keyrec.dwControlKeyState & SHIFT_PRESSED ? KEY_SHIFT : 0x0 ) ;
                    }
                    break;
                case '\b':
                    if ( keyrec.wVirtualKeyCode == VK_BACK )
                    {
                        c = keyrec.wVirtualKeyCode | KEY_SCAN |
                          ( keyrec.dwControlKeyState & ENHANCED_KEY ? KEY_ENHANCED : 0x0 ) |
                            ( keyrec.dwControlKeyState & SHIFT_PRESSED ? KEY_SHIFT : 0x0 ) ;
                    }
                    break;
                case '+':
                    if ( keyrec.wVirtualKeyCode == VK_ADD )
                    {
                        c = keyrec.wVirtualKeyCode | KEY_SCAN |
                          ( keyrec.dwControlKeyState & ENHANCED_KEY ? KEY_ENHANCED : 0x0 ) |
                            ( keyrec.dwControlKeyState & SHIFT_PRESSED ? KEY_SHIFT : 0x0 ) ;
                    }
                    break;
                case    '-':
                    if ( keyrec.wVirtualKeyCode == VK_SUBTRACT )
                    {
                        c = keyrec.wVirtualKeyCode | KEY_SCAN |
                          ( keyrec.dwControlKeyState & ENHANCED_KEY ? KEY_ENHANCED : 0x0 ) |
                            ( keyrec.dwControlKeyState & SHIFT_PRESSED ? KEY_SHIFT : 0x0 ) ;
                    }
                    break;
                case '*':
                    if ( keyrec.wVirtualKeyCode == VK_MULTIPLY )
                    {
                        c = keyrec.wVirtualKeyCode | KEY_SCAN |
                          ( keyrec.dwControlKeyState & ENHANCED_KEY ? KEY_ENHANCED : 0x0 ) |
                            ( keyrec.dwControlKeyState & SHIFT_PRESSED ? KEY_SHIFT : 0x0 ) ;
                    }
                    break;
                case '/':
                    if ( keyrec.wVirtualKeyCode == VK_DIVIDE )
                    {
                        c = keyrec.wVirtualKeyCode | KEY_SCAN |
                          ( keyrec.dwControlKeyState & ENHANCED_KEY ? KEY_ENHANCED : 0x0 ) |
                            ( keyrec.dwControlKeyState & SHIFT_PRESSED ? KEY_SHIFT : 0x0 ) ;
                    }
                    break;
                case '\r':
                case '\n':
                    if ( keyrec.wVirtualKeyCode == VK_RETURN )
                    {
                        c = keyrec.wVirtualKeyCode | KEY_SCAN |
                          ( keyrec.dwControlKeyState & ENHANCED_KEY ? KEY_ENHANCED : 0x0 ) |
                            ( keyrec.dwControlKeyState & SHIFT_PRESSED ? KEY_SHIFT : 0x0 ) ;
                    }
                    break;
                case '.':
                case ',':
                    if ( keyrec.wVirtualKeyCode == VK_DECIMAL )
                    {
                        c = keyrec.wVirtualKeyCode | KEY_SCAN |
                          ( keyrec.dwControlKeyState & ENHANCED_KEY ? KEY_ENHANCED : 0x0 ) |
                            ( keyrec.dwControlKeyState & SHIFT_PRESSED ? KEY_SHIFT : 0x0 ) ;
                    }
                    else if ( keyrec.wVirtualScanCode == 83 && isWin95() )
                    {
                        /* Windows 95 reports the wrong KeyCode for the KeyPad Decimal */
                        c = VK_DECIMAL | KEY_SCAN |
                          ( keyrec.dwControlKeyState & ENHANCED_KEY ? KEY_ENHANCED : 0x0 ) |
                            ( keyrec.dwControlKeyState & SHIFT_PRESSED ? KEY_SHIFT : 0x0 ) ;
                    }
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    if ( keyrec.wVirtualKeyCode >= VK_NUMPAD0 &&
                         keyrec.wVirtualKeyCode <= VK_NUMPAD9 )
                    {
                        c = keyrec.wVirtualKeyCode | KEY_SCAN |
                          ( keyrec.dwControlKeyState & ENHANCED_KEY ? KEY_ENHANCED : 0x0 ) |
                            ( keyrec.dwControlKeyState & SHIFT_PRESSED ? KEY_SHIFT : 0x0 ) ;
                    }
                    break;
                default:
                    ;
                }       /* switch */
            }           /* ascii char */
            if ( keyrec.dwControlKeyState & ENHANCED_KEY )
              c |= KEY_ENHANCED ;

            if ( keyrec.dwControlKeyState & CONTROL )
              c |= KEY_CTRL ;

            if ( keyrec.dwControlKeyState & ALT )
              c |= KEY_ALT ;

        } /* if (real key) */
    }
/* end computation of keycode value */
    *pkeyrec = keyrec;
    return c;
}

void
win32KeyEvent( int mode, KEY_EVENT_RECORD keyrec )
{
    int c = -1, i;
    int keycount = 1 ;
#define CHCOUNT 8
#ifdef CKOUNI_IN
    WORD buf[CHCOUNT] ;
#else
    CHAR buf[CHCOUNT];
#endif

#ifdef COMMENT
    printf("\n");
    printf("keyrec.bKeyDown          0x%X\n",keyrec.bKeyDown);
    printf("keyrec.wRepeatCount      0x%X\n",keyrec.wRepeatCount);
    printf("keyrec.wVirtualKeyCode   0x%X\n",keyrec.wVirtualKeyCode);
    printf("keyrec.wVirtualScanCode  0x%X\n",keyrec.wVirtualScanCode);
    printf("keyrec.uChar.AsciiChar   0x%X\n",keyrec.uChar.AsciiChar);
    printf("keyrec.dwControlKeyState 0x%X\n",keyrec.dwControlKeyState);
    printf("\n");
#endif /* COMMENT */

#ifdef PCTERM
    if ( mode == VTERM ) {
        /* Ctrl-CAPSLOCK is the toggle for PCTERM */
        static int pcterm_ctrl_down = 0, pcterm_ctrl_up = 0,
                   pcterm_caps_down = 0, pcterm_caps_up = 0;

        switch ( keyrec.wVirtualKeyCode ) {
        case VK_CONTROL:
            if ( keyrec.bKeyDown ) {
                pcterm_ctrl_down = 1;
            } else {
                if ( pcterm_caps_down && pcterm_caps_up ) {
                    setpcterm(!tt_pcterm);
                    pcterm_ctrl_down = pcterm_ctrl_up = pcterm_caps_down =
                        pcterm_caps_up = 0;
                } else {
                    pcterm_ctrl_up = 1;
                }
            }
            break;
        case VK_CAPITAL:
            if ( keyrec.bKeyDown ) {
                pcterm_caps_down = 1;
            } else {
                if ( pcterm_ctrl_down && pcterm_ctrl_up ) {
                    setpcterm(!tt_pcterm);
                    pcterm_ctrl_down = pcterm_ctrl_up = pcterm_caps_down =
                        pcterm_caps_up = 0;
                } else {
                    pcterm_caps_up = 1;
                }
            }
            break;
        default:
            pcterm_ctrl_down = pcterm_ctrl_up = pcterm_caps_down =
                pcterm_caps_up = 0;
        }

        if ( tt_pcterm && !wy_block ) {
            char pre[16]="";
            char hex[16]="";
            char post[16]="";
            char msg[32]="";
            int i=0;

            if ( keyrec.dwControlKeyState & ENHANCED_KEY ) {
                if ( keyrec.dwControlKeyState & SHIFT_PRESSED) {
                    if ( keyrec.bKeyDown )
                        sprintf(pre,"%c%c",0xE0,0xAA);
                    else
                        sprintf(post,"%c%c",0xE0,0x2A);
                }
                sprintf(hex,"%c",0xE0);
                strcat(pre,hex);
            }

            for ( i=0 ; i<keyrec.wRepeatCount && i<15 ; i++ ) {
                if ( keyrec.bKeyDown )
                    hex[i] = keyrec.wVirtualScanCode;
                else
                    hex[i] = keyrec.wVirtualScanCode + 0x80;
            }
            hex[i] = '\0';
            sprintf(msg,"%s%s%s",pre,hex,post);
            ttol(msg,strlen(msg));
#ifndef NOLOCAL
            clickkeys();
#endif /* NOLOCAL */
            return;
        }
    }
#endif /* PCTERM */

    c = getKeycodeFromKeyRec(&keyrec, (WORD *)buf, CHCOUNT);

    if ( c >= 0 ) {
#ifdef NOSETKEY
        con_event evt;
        evt.type = key ;
        evt.key.scancode = c;
#else /* NOSETKEY */
        con_event evt = mapkey(c);
        if ( evt.type == error ) {
            /* map the key to the Ascii value */
            /* if no previous value has been assigned */
            if ( keymap != NULL && keyrec.uChar.AsciiChar != 0x00 && c < KMSIZE)
                keymap[c] = keyrec.uChar.AsciiChar;
        }
#endif /* NOSETKEY */
        if ( (c == 1027)
#ifndef NOLOCAL
             && !IsConnectMode()
#endif /* NOLOCAL */
             )  /* Ctrl-C */
            raise(SIGINT);
        else
            for ( i=0; i<keyrec.wRepeatCount ;i++ )
            {
#ifndef NOLOCAL
                clickkeys() ;
#endif /* NOLOCAL */
#ifdef COMMENT
/* It was a good try, but it is not safe.  There is a reason that we */
/* don't allow the thread that reads the keystrokes to process them  */
/* and that is because that means the thread can block waiting for   */
/* input if the command to be carried out requires it.               */
                if ( mode == VTERM &&
                     !VscrnIsPopup() &&
                     evt.type != kverb ||
                     (evt.type == kverb &&
                       evt.kverb.id != K_GO_BOOK &&
                       evt.kverb.id != K_GOTO &&
                       evt.kverb.id != K_HELP &&
                       evt.kverb.id != K_COMPOSE &&
                       evt.kverb.id != K_C_UNI16)) {
                    evt = mapkey(c);
                    if ( evt.type == error ) {
                        evt.type = key;
                        evt.key.scancode = c;
                    }
                    con2host(evt);
                }
                else
#endif /* COMMENT */
                    putkey( mode, c ) ;

#ifndef KUI
                if ( isWin95() && (keycount == 2) ) {
                    char k2 = (char) buf[1], k2x;
                    debug(F111,"win32Key","buf[1]",k2);
                    CharToOemBuff( &k2, &k2x, 1);
                    debug(F111,"win32Key","k2x",k2x);

#ifdef COMMENT  
/* It was a good try, but it is not safe.  There is a reason that we */
/* don't allow the thread that reads the keystrokes to process them  */
/* and that is because that means the thread can block waiting for   */
/* input if the command to be carried out requires it.               */
                    if ( mode == VTERM &&
                         !VscrnIsPopup() &&
                         evt.type != kverb ||
                         (evt.type == kverb &&
                           evt.kverb.id != K_GO_BOOK &&
                           evt.kverb.id != K_GOTO &&
                           evt.kverb.id != K_HELP &&
                           evt.kverb.id != K_COMPOSE &&
                           evt.kverb.id != K_C_UNI16)) {
                        evt = mapkey(k2x);
                        if ( evt.type == error ) {
                            evt.type = key;
                            evt.key.scancode = k2x;
                        }
                        con2host(evt);
                    }
                    else
#endif /* COMMENT */    
                        putkey( mode, k2x ) ;
                }   
#endif /* KUI */        
            }       
    } /* if (c >= 0 ) */
}

int win32ScrollUp = 0, win32ScrollDown = 0;

#ifndef NOLOCAL
#ifndef KUI
void
KbdHandlerThread( void * pArgList ) {
    INPUT_RECORD k;
    DWORD count = 0;
    int rc=0, c=0, i=0;
    extern int StartedFromDialer ;
    extern BYTE vmode ;
    DWORD saved_mode=0;

    memset( &k, 0, sizeof(k) ) ;
    setint();

    debug(F100,"KbdHandlerThread is born","",0);
#ifdef IKSD
    if ( !inserver ) {
#endif /* IKSD */
        if ( StartedFromDialer && isWin95() && !k95stdin ) {
            KbdHandle = CreateFile( "CONIN$", GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    0,
                                    0) ;
        } else
            KbdHandle = GetStdHandle( STD_INPUT_HANDLE ) ;

        GetConsoleMode( KbdHandle, &saved_mode ) ;
#ifdef IKSD
    }
#endif /* IKSD */

    rc = PostKbdThreadSem() ;
    ResetThreadPrty();
    SetThreadPrty(0,3);

    /* we must wait for the keymap to be initialized before starting */
    /* to process keyboard input */
    WaitKeyMapInitSem(SEM_INDEFINITE_WAIT) ;
    /* Do not reset as we don't want to wait each */
    /* time this thread restarts */

    while ( KbdActive ) {
        if ( k95stdin ) {
            c = 0;
            if ( (c = getchar()) != EOF ) {
                debug(F101,"getchar()","",c);
#ifndef NOLOCAL
                clickkeys();
#endif /* NOLOCAL */
                putkey(vmode, c);
            }
#ifndef NOKVERBS
            else {
                 putkverb(vmode,K_QUIT);
            }
#endif /* NOKVERBS */
        }
        else if ( islocal ) {
            while ( KbdActive ) {
                memset( &k, 0, sizeof(k) ) ;
                rc = RequestKeyboardMutex( SEM_INDEFINITE_WAIT ) ;
                count = 0 ;
                if ( WAIT_OBJECT_0 == WaitForSingleObject(KbdHandle,
                     (win32ScrollUp || win32ScrollDown) ? 50 : 1000) )
                {
#ifdef CKOUNI_IN
                    rc = ReadConsoleInputW( KbdHandle, &k, 1, &count ) ;
#else /* CKOUNI_IN */
                    rc = ReadConsoleInput( KbdHandle, &k, 1, &count ) ;
#endif /* CKOUNI_IN */
#ifdef COMMENT
                    /* If we were MS-DOS Kermit, we could do this */
                    if ( !rc && isWin95() ) {
                        if ( Win95QueryClose() ) {
                            Win95AcknowledgeClose() ;
                            putmacro(VCMD,"quit\r");
                        }
                    }
#endif /* COMMENT */

                    if ( !rc ) {
                        int lasterror = GetLastError();
#ifdef BETADEBUG
                        printf("Keyboard Wait succeeded, no data.  Error %d\n",lasterror);
#endif /* BETADEBUG */
                        debug(F111,"KbdHandlerThread - ReadConsoleInput failed","GetLastError()",
                               lasterror);
                    }
                }
#ifndef NOKVERBS
                else if ( win32ScrollUp ) {
                    putkverb(vmode, F_KVERB | K_UPONE);
                } else if ( win32ScrollDown ) {
                    putkverb(vmode, F_KVERB | K_DNONE);
                }
#endif /* NOKVERBS */
                rc = ReleaseKeyboardMutex() ;
                if (count)
                  break ;
            }

            if ( KbdActive ) {
                extern int ttnum;               /* from ckcnet.c */

                switch ( k.EventType ) {
                case KEY_EVENT:
#ifndef NOTERM
                    if ( ISVTNT(tt_type_mode) &&
                         ttnum != -1
#ifndef NOLOCAL
                         && IsConnectMode()
#endif /* NOLOCAL */
                         )
                        ttol((char *)&k,sizeof(INPUT_RECORD));
                    else
#endif /* NOTERM */
                        win32KeyEvent( vmode, k.Event.KeyEvent ) ;
                    break;
#ifdef OS2MOUSE
                case MOUSE_EVENT:
                    win32MouseEvent( vmode, k.Event.MouseEvent ) ;
                    break;
#endif /* OS2MOUSE */
#ifndef NOLOCAL
                case WINDOW_BUFFER_SIZE_EVENT:
                    win32WindowEvent( vmode, k.Event.WindowBufferSizeEvent ) ;
                    break;
#endif /* NOLOCAL */
                case MENU_EVENT:
                    debug(F111,"MENU_EVENT","k.Event.MenuEvent.dwCommandId",
                           k.Event.MenuEvent.dwCommandId);
                    break;
                case FOCUS_EVENT:
                    debug(F111,"FOCUS_EVENT","k.Event.FocusEvent.bSetFocus",
                           k.Event.FocusEvent.bSetFocus);
                    break;
                }
            } /* if ( KbdActive ) */
        }   /* if ( !islocal ) */
        else {
            c = 0;
            if (ttyfd > -1 && _read(ttyfd, &c, 1) == 1 ) {
                putkey(vmode, c);
            }
            else
              msleep(80);
        }
    }  /* while ( KbdActive ) */

#ifdef IKSD
    if ( !inserver ) {
#endif /* IKSD */
        SetConsoleMode( KbdHandle, saved_mode ) ;

        if ( StartedFromDialer && isWin95() && !k95stdin )
            CloseHandle(KbdHandle);
        KbdHandle = 0 ;
#ifdef IKSD
    }
#endif /* IKSD */

    PostKbdThreadSem() ;
    debug(F100,"KbdHandlerThread dies","",0);
    ckThreadEnd(pArgList);
}
#endif /* KUI */
#endif /* NOLOCAL */
#else /* not NT */
/* OS/2 version of the KbdHandlerThread only sees Keyboard events */
static int os2win32_XS[] =
{
    /* 782 - Backspace */  KEY_SCAN | KEY_SHIFT | 8,
    /* 783 - Tab */        KEY_SCAN | KEY_SHIFT | 9,
    0,0,0,0,0,0,0,0,0,0,
    0,0,
    /* 796 - Enter */      KEY_SCAN | KEY_SHIFT | 13,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    /* 837 - NumLock */    KEY_ENHANCED | KEY_SCAN | 144,
    /* 838 - ScrollLock */ KEY_SCAN | 145,
    /* 839 - Home */       KEY_SCAN | 36,
    /* 840 - Up */         KEY_SCAN | 38,
    /* 841 - PgUp */       KEY_SCAN | 33,
    0,
    /* 843 - Left */       KEY_SCAN | 37,
    /* 844 - Clear */      KEY_SCAN | 12,
    /* 845 - Right */      KEY_SCAN | 39,
    0,
    /* 847 - End */        KEY_SCAN | 35,
    /* 848 - Down */       KEY_SCAN | 40,
    /* 849 - PgDn */       KEY_SCAN | 34,
    /* 850 - Insert */     KEY_SCAN | 45,
    /* 851 - Delete */     KEY_SCAN | 46,
    /* 852 - F1  */ KEY_SCAN | KEY_SHIFT | 112,
    /* 853 - F2  */ KEY_SCAN | KEY_SHIFT | 113,
    /* 854 - F3  */ KEY_SCAN | KEY_SHIFT | 114,
    /* 855 - F4  */ KEY_SCAN | KEY_SHIFT | 115,
    /* 856 - F5  */ KEY_SCAN | KEY_SHIFT | 116,
    /* 857 - F6  */ KEY_SCAN | KEY_SHIFT | 117,
    /* 858 - F7  */ KEY_SCAN | KEY_SHIFT | 118,
    /* 859 - F8  */ KEY_SCAN | KEY_SHIFT | 119,
    /* 860 - F9  */ KEY_SCAN | KEY_SHIFT | 120,
    /* 861 - F10 */ KEY_SCAN | KEY_SHIFT | 121,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,
    /* 903 - F11 */ KEY_SCAN | KEY_SHIFT | 122,
    /* 904 - F12 */ KEY_SCAN | KEY_SHIFT | 123
};

static int os2win32_C[] =
{
    /* 1025 - A */   KEY_CTRL | 1 ,
    /* 1026 - B */   KEY_CTRL | 2 ,
    /* 1027 - C */   KEY_CTRL | 3 ,
    /* 1028 - D */   KEY_CTRL | 4 ,
    /* 1029 - E */   KEY_CTRL | 5 ,
    /* 1030 - F */   KEY_CTRL | 6 ,
    /* 1031 - G */   KEY_CTRL | 7 ,
    0,
    /* 1033 - I */   KEY_CTRL | 9 ,
    0,
    /* 1035 - K */   KEY_CTRL | 11 ,
    /* 1036 - L */   KEY_CTRL | 12 ,
    0,
    /* 1038 - N */   KEY_CTRL | 14 ,
    /* 1039 - O */   KEY_CTRL | 15 ,
    /* 1040 - P */   KEY_CTRL | 16 ,
    /* 1041 - Q */   KEY_CTRL | 17 ,
    /* 1042 - R */   KEY_CTRL | 18 ,
    /* 1043 - S */   KEY_CTRL | 19 ,
    /* 1044 - T */   KEY_CTRL | 20 ,
    /* 1045 - U */   KEY_CTRL | 21 ,
    /* 1046 - V */   KEY_CTRL | 22 ,
    /* 1047 - W */   KEY_CTRL | 23 ,
    /* 1048 - X */   KEY_CTRL | 24 ,
    /* 1049 - Y */   KEY_CTRL | 25 ,
    /* 1050 - Z */   KEY_CTRL | 26 ,
    0,
    /* 1052 - \ */   KEY_CTRL | 28 ,
    /* 1053 - ] */   KEY_CTRL | 29 ,
    /* 1054 - 6 */   KEY_CTRL | 30 ,
    /* 1055 - - */   KEY_CTRL | 31 ,
    /* 1056 - Space */ KEY_CTRL | SP
};

static int os2win32_XC[] =
{
    /* 1283 - 2 */   KEY_SCAN | KEY_CTRL | 50,
    0,0,0,0,0,0,0,0,0,0,
    /* 1294 - Backspace */  KEY_SCAN | KEY_CTRL | 8,
    0,0,0,0,0,0,0,0,0,0,
    0,
    /* 1306 - [ */      KEY_CTRL | 27,
    0,
    /* 1308 - Enter */      KEY_SCAN | KEY_CTRL | 13,
    0,0,0,0,0,0,
    /* 1315 - H */   KEY_CTRL | 8 ,
    /* 1316 - J */   KEY_CTRL | 10 ,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,
    /* 1330 - M */   KEY_CTRL | 13 ,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,
    /* 1374 - F1  */ KEY_SCAN | KEY_CTRL | 112,
    /* 1375 - F2  */ KEY_SCAN | KEY_CTRL | 113,
    /* 1376 - F3  */ KEY_SCAN | KEY_CTRL | 114,
    /* 1377 - F4  */ KEY_SCAN | KEY_CTRL | 115,
    /* 1378 - F5  */ KEY_SCAN | KEY_CTRL | 116,
    /* 1379 - F6  */ KEY_SCAN | KEY_CTRL | 117,
    /* 1380 - F7  */ KEY_SCAN | KEY_CTRL | 118,
    /* 1381 - F8  */ KEY_SCAN | KEY_CTRL | 119,
    /* 1382 - F9  */ KEY_SCAN | KEY_CTRL | 120,
    /* 1383 - F10 */ KEY_SCAN | KEY_CTRL | 121,
    0,0,0,0,0,0,0,0,0,0,
    0,
    /* 1395 - Kp4 */        KEY_SCAN | KEY_CTRL | 37,
    /* 1396 - Kp6 */        KEY_SCAN | KEY_CTRL | 39,
    /* 1397 - Kp1 */        KEY_SCAN | KEY_CTRL | 35,
    /* 1398 - Kp3 */        KEY_SCAN | KEY_CTRL | 34,
    /* 1399 - Kp7 */        KEY_SCAN | KEY_CTRL | 36,
    0,0,0,0,0,0,0,0,0,0,
    0,0,
    /* 1412 - Kp9 */        KEY_SCAN | KEY_CTRL | 33,
    0,0,0,0,
    /* 1417 - F11 */        KEY_SCAN | KEY_CTRL | 122,
    /* 1418 - F12 */        KEY_SCAN | KEY_CTRL | 123,
    0,0,
    /* 1421 - Kp8 */        KEY_SCAN | KEY_CTRL | 38,
    /* 1422 - Kp- */        KEY_SCAN | KEY_CTRL | 109,
    /* 1423 - Kp5 */        KEY_SCAN | KEY_CTRL | 12,
    /* 1424 - Kp+ */        KEY_SCAN | KEY_CTRL | 107,
    /* 1425 - Kp2 */        KEY_SCAN | KEY_CTRL | 40,
    /* 1426 - Kp0 */        KEY_SCAN | KEY_CTRL | 45,
    /* 1427 - Decimal */    KEY_SCAN | KEY_CTRL | 46,
    /* 1428 - Tab */        KEY_SCAN | KEY_CTRL | 9,
    0,
    /* 1430 - Kp* */        KEY_SCAN | KEY_CTRL | 106
};

static int os2win32_A[] =
{
    /* 1025 - A */   KEY_ALT | 65 ,
    /* 1026 - B */   KEY_ALT | 66 ,
    /* 1027 - C */   KEY_ALT | 67 ,
    /* 1028 - D */   KEY_ALT | 68 ,
    /* 1029 - E */   KEY_ALT | 69 ,
    /* 1030 - F */   KEY_ALT | 70 ,
    /* 1031 - G */   KEY_ALT | 71 ,
    0,
    /* 1033 - I */   KEY_ALT | 73 ,
    0,
    /* 1035 - K */   KEY_ALT | 75 ,
    /* 1036 - L */   KEY_ALT | 76 ,
    0,
    /* 1038 - N */   KEY_ALT | 78 ,
    /* 1039 - O */   KEY_ALT | 79 ,
    /* 1040 - P */   KEY_ALT | 80 ,
    /* 1041 - Q */   KEY_ALT | 81 ,
    /* 1042 - R */   KEY_ALT | 82 ,
    /* 1043 - S */   KEY_ALT | 83 ,
    /* 1044 - T */   KEY_ALT | 84 ,
    /* 1045 - U */   KEY_ALT | 85 ,
    /* 1046 - V */   KEY_ALT | 86 ,
    /* 1047 - W */   KEY_ALT | 87 ,
    /* 1048 - X */   KEY_ALT | 88 ,
    /* 1049 - Y */   KEY_ALT | 89 ,
    /* 1050 - Z */   KEY_ALT | 90 ,
    0,
    /* 1052 - \ */   KEY_ALT | 65 ,
    /* 1053 - [ */   KEY_ALT | 65 ,
    /* 1054 - 6 */   KEY_ALT | 54,
    /* 1055 - - */   KEY_ALT | 189,
    /* 1056 -   */   KEY_ALT | 32

};

static int os2win32_XA[] =
{
    /* 2318 - Backspace */ KEY_SCAN | KEY_ALT | 8,
    0,
    /* 2320 - Q     */ KEY_ALT | 'q',
    /* 2321 - W     */ KEY_ALT | 'w',
    /* 2322 - E     */ KEY_ALT | 'e',
    /* 2323 - R     */ KEY_ALT | 'r',
    /* 2324 - T     */ KEY_ALT | 't',
    /* 2325 - Y     */ KEY_ALT | 'y',
    /* 2326 - U     */ KEY_ALT | 'u',
    /* 2327 - I     */ KEY_ALT | 'i',
    /* 2328 - O     */ KEY_ALT | 'o',
    /* 2329 - P     */ KEY_ALT | 'p',
    /* 2330 - [     */ KEY_ALT | '[',
    /* 2331 - ]     */ KEY_ALT | ']',
    /* 2332 - Enter */ KEY_SCAN | KEY_ALT | 13,
    0,
    /* 2334 - A     */ KEY_ALT | 'a',
    /* 2335 - S     */ KEY_ALT | 's',
    /* 2336 - D     */ KEY_ALT | 'd',
    /* 2337 - F     */ KEY_ALT | 'f',
    /* 2338 - G     */ KEY_ALT | 'g',
    /* 2339 - H     */ KEY_ALT | 'h',
    /* 2340 - J     */ KEY_ALT | 'j',
    /* 2341 - K     */ KEY_ALT | 'k',
    /* 2342 - L     */ KEY_ALT | 'l',
    /* 2343 - ;     */ KEY_ALT | ';',
    /* 2344 - '     */ KEY_ALT | '\'',
    /* 2345 - `     */ KEY_ALT | '`',
    0,
    /* 2347 - \     */ KEY_ALT | '\\',
    /* 2348 - Z     */ KEY_ALT | 'z',
    /* 2349 - X     */ KEY_ALT | 'x',
    /* 2350 - C     */ KEY_ALT | 'c',
    /* 2351 - V     */ KEY_ALT | 'v',
    /* 2352 - B     */ KEY_ALT | 'b',
    /* 2353 - N     */ KEY_ALT | 'n',
    /* 2354 - M     */ KEY_ALT | 'm',
    /* 2355 - ,     */ KEY_ALT | ',',
    /* 2356 - .     */ KEY_ALT | '.',
    /* 2357 - /     */ KEY_ALT | '/',
    0,
    /* 2359 - Kp* */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 106 , /* ??? */
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    /* 2378 - Kp- */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 109 , /* ??? */
    0,0,0,
    /* 2382 - Kp+ */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 107 , /* ??? */
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,
    /* 2408 - F1     */ KEY_SCAN | KEY_ALT | 112,
    /* 2409 - F2     */ KEY_SCAN | KEY_ALT | 113,
    /* 2410 - F3     */ KEY_SCAN | KEY_ALT | 114,
    /* 2411 - F4     */ KEY_SCAN | KEY_ALT | 115,
    /* 2412 - F5     */ KEY_SCAN | KEY_ALT | 116,
    /* 2413 - F6     */ KEY_SCAN | KEY_ALT | 117,
    /* 2414 - F7     */ KEY_SCAN | KEY_ALT | 118,
    /* 2415 - F8     */ KEY_SCAN | KEY_ALT | 119,
    /* 2416 - F9     */ KEY_SCAN | KEY_ALT | 120,
    /* 2417 - F10    */ KEY_SCAN | KEY_ALT | 121,
    0,0,0,0,0,0,
    /* 2424 - 1     */ KEY_ALT | '1',
    /* 2425 - 2     */ KEY_ALT | '2',
    /* 2426 - 3     */ KEY_ALT | '3',
    /* 2427 - 4     */ KEY_ALT | '4',
    /* 2428 - 5     */ KEY_ALT | '5',
    /* 2429 - 6     */ KEY_ALT | '6',
    /* 2430 - 7     */ KEY_ALT | '7',
    /* 2431 - 8     */ KEY_ALT | '8',
    /* 2432 - 9     */ KEY_ALT | '9',
    /* 2433 - 0     */ KEY_ALT | '0',
    /* 2434 - -     */ KEY_ALT | '-',
    /* 2435 - =     */ KEY_ALT | '=',
    0,0,0,0,0,0,0,
    /* 2443 - F11    */ KEY_SCAN | KEY_ALT | 122,
    /* 2444 - F12    */ KEY_SCAN | KEY_ALT | 123,
    0,0,0,0,0,0,0,0,0,0,
    /* 2455 - Home   */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 36 ,
    /* 2456 - Up     */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 38 ,
    /* 2457 - PgUp   */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 33 ,
    0,
    /* 2459 - Left   */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 37 ,
    0,
    /* 2461 - Right  */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 39 ,
    0,
    /* 2463 - End    */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 35 ,
    /* 2464 - Down   */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 40 ,
    /* 2465 - PgDn   */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 34 ,
    /* 2466 - Insert */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 45 ,
    /* 2467 - Delete */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 46 ,
    /* 2468 - Kp/    */ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 111 , /* ??? */
    0,
    /* 2470 - KpEnter*/ KEY_SCAN | KEY_ENHANCED | KEY_ALT | 13
};


static int os2win32_XAS[] =
{
    /* 2830 - Backspace */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 8,
    0,
    /* 2832 - Q     */ KEY_ALT | 'Q',
    /* 2833 - W     */ KEY_ALT | 'W',
    /* 2834 - E     */ KEY_ALT | 'E',
    /* 2835 - R     */ KEY_ALT | 'R',
    /* 2836 - T     */ KEY_ALT | 'T',
    /* 2837 - Y     */ KEY_ALT | 'Y',
    /* 2838 - U     */ KEY_ALT | 'U',
    /* 2839 - I     */ KEY_ALT | 'I',
    /* 2840 - O     */ KEY_ALT | 'O',
    /* 2841 - P     */ KEY_ALT | 'P',
    /* 2842 - [     */ KEY_ALT | '{',
    /* 2843 - ]     */ KEY_ALT | '}',
    /* 2844 - Enter */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 13,
    0,
    /* 2846 - A     */ KEY_ALT | 'A',
    /* 2847 - S     */ KEY_ALT | 'S',
    /* 2848 - D     */ KEY_ALT | 'D',
    /* 2849 - F     */ KEY_ALT | 'F',
    /* 2850 - G     */ KEY_ALT | 'G',
    /* 2851 - H     */ KEY_ALT | 'H',
    /* 2852 - J     */ KEY_ALT | 'J',
    /* 2853 - K     */ KEY_ALT | 'K',
    /* 2854 - L     */ KEY_ALT | 'L',
    /* 2855 - ;     */ KEY_ALT | ':',
    /* 2856 - '     */ KEY_ALT | '"',
    /* 2857 - `     */ KEY_ALT | '~',
    0,
    /* 2859 - \     */ KEY_ALT | '|',
    /* 2860 - Z     */ KEY_ALT | 'Z',
    /* 2861 - X     */ KEY_ALT | 'X',
    /* 2862 - C     */ KEY_ALT | 'C',
    /* 2863 - V     */ KEY_ALT | 'V',
    /* 2864 - B     */ KEY_ALT | 'B',
    /* 2865 - N     */ KEY_ALT | 'N',
    /* 2866 - M     */ KEY_ALT | 'M',
    /* 2867 - ,     */ KEY_ALT | '<',
    /* 2868 - .     */ KEY_ALT | '>',
    /* 2869 - /     */ KEY_ALT | '?',
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    /* 2890 - Kp- */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 109 , /* ??? */
    0,0,0,
    /* 2894 - Kp+ */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 107 , /* ??? */
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,
    /* 2920 - F1     */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 112,
    /* 2921 - F2     */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 113,
    /* 2922 - F3     */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 114,
    /* 2923 - F4     */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 115,
    /* 2924 - F5     */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 116,
    /* 2925 - F6     */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 117,
    /* 2926 - F7     */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 118,
    /* 2927 - F8     */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 119,
    /* 2928 - F9     */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 120,
    /* 2929 - F10    */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 121,
    0,0,0,0,0,0,
    /* 2936 - 1     */ KEY_SHIFT | KEY_ALT | '1',
    /* 2937 - 2     */ KEY_SHIFT | KEY_ALT | '2',
    /* 2938 - 3     */ KEY_SHIFT | KEY_ALT | '3',
    /* 2939 - 4     */ KEY_SHIFT | KEY_ALT | '4',
    /* 2940 - 5     */ KEY_SHIFT | KEY_ALT | '5',
    /* 2941 - 6     */ KEY_SHIFT | KEY_ALT | '6',
    /* 2942 - 7     */ KEY_SHIFT | KEY_ALT | '7',
    /* 2943 - 8     */ KEY_SHIFT | KEY_ALT | '8',
    /* 2944 - 9     */ KEY_SHIFT | KEY_ALT | '9',
    /* 2945 - 0     */ KEY_SHIFT | KEY_ALT | '0',
    /* 2946 - -     */ KEY_SHIFT | KEY_ALT | '-',
    /* 2947 - =     */ KEY_SHIFT | KEY_ALT | '=',
    0,0,0,0,0,0,0,
    /* 2955 - F11    */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 122,
    /* 2956 - F12    */ KEY_SCAN | KEY_SHIFT | KEY_ALT | 123,
    0,0,0,0,0,0,0,0,0,0,
    /* 2967 - Home   */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 36 ,
    /* 2968 - Up     */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 38 ,
    /* 2969 - PgUp   */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 33 ,
    0,
    /* 2971 - Left   */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 37 ,
    0,
    /* 2973 - Right  */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 39 ,
    0,
    /* 2975 - End    */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 35 ,
    /* 2976 - Down   */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 40 ,
    /* 2977 - PgDn   */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 34 ,
    /* 2978 - Insert */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 45 ,
    /* 2979 - Delete */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 46 ,
    /* 2980 - Kp/    */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 111 , /* ??? */
    0,
    /* 2982 - KpEnter*/ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 13
};


static int os2win32_XCA[] =
{
    /* 3329 - ESC */       KEY_CTRL | KEY_ALT | 27,
    0,0,0,0,0,0,0,0,0,0,
    0,0,
    /* 3342 - Backspace */ KEY_SCAN | KEY_CTRL | KEY_ALT | 8,
    0,
    /* 3344 - Q     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'Q',
    /* 3345 - W     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'W',
    /* 3346 - E     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'E',
    /* 3347 - R     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'R',
    /* 3348 - T     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'T',
    /* 3349 - Y     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'Y',
    /* 3350 - U     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'U',
    /* 3351 - I     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'I',
    /* 3352 - O     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'O',
    /* 3353 - P     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'P',
    /* 3354 - [     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 219,
    /* 3355 - ]     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 221,
    /* 3356 - Enter */ KEY_SCAN | KEY_CTRL | KEY_ALT | 13,
    0,
    /* 3358 - A     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'A',
    /* 3359 - S     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'S',
    /* 3360 - D     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'D',
    /* 3361 - F     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'F',
    /* 3362 - G     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'G',
    /* 3363 - H     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'H',
    /* 3364 - J     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'J',
    /* 3365 - K     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'K',
    /* 3366 - L     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'L',
    /* 3367 - ;     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 186,
    /* 3368 - '     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 222,
    /* 3369 - `     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 192,
    0,
    /* 3371 - \     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 220,
    /* 3372 - Z     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'Z',
    /* 3373 - X     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'X',
    /* 3374 - C     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'C',
    /* 3375 - V     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'V',
    /* 3376 - B     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'B',
    /* 3377 - N     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'N',
    /* 3378 - M     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 'M',
    /* 3379 - ,     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 188,
    /* 3380 - .     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 190,
    /* 3381 - /     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 191,
    0,0,
    /* 3383 - Kp* */ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 106 , /* ??? */
    0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    /* 3402 - Kp- */ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 109 , /* ??? */
    0,0,0,
    /* 3406 - Kp+ */ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 107 , /* ??? */
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,
    /* 3432 - F1     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 112,
    /* 3433 - F2     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 113,
    /* 3434 - F3     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 114,
    /* 3435 - F4     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 115,
    /* 3436 - F5     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 116,
    /* 3437 - F6     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 117,
    /* 3438 - F7     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 118,
    /* 3439 - F8     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 119,
    /* 3440 - F9     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 120,
    /* 3441 - F10    */ KEY_SCAN | KEY_CTRL | KEY_ALT | 121,
    0,0,0,0,0,0,
    /* 3448 - 1     */ KEY_SCAN | KEY_CTRL | KEY_ALT | '1',
    /* 3449 - 2     */ KEY_SCAN | KEY_CTRL | KEY_ALT | '2',
    /* 3450 - 3     */ KEY_SCAN | KEY_CTRL | KEY_ALT | '3',
    /* 3451 - 4     */ KEY_SCAN | KEY_CTRL | KEY_ALT | '4',
    /* 3452 - 5     */ KEY_SCAN | KEY_CTRL | KEY_ALT | '5',
    /* 3453 - 6     */ KEY_SCAN | KEY_CTRL | KEY_ALT | '6',
    /* 3454 - 7     */ KEY_SCAN | KEY_CTRL | KEY_ALT | '7',
    /* 3455 - 8     */ KEY_SCAN | KEY_CTRL | KEY_ALT | '8',
    /* 3456 - 9     */ KEY_SCAN | KEY_CTRL | KEY_ALT | '9',
    /* 3457 - 0     */ KEY_SCAN | KEY_CTRL | KEY_ALT | '0',
    /* 3458 - -     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 189,
    /* 3459 - =     */ KEY_SCAN | KEY_CTRL | KEY_ALT | 187,
    0,0,0,0,0,0,0,
    /* 3467 - F11    */ KEY_SCAN | KEY_CTRL | KEY_ALT | 122,
    /* 3468 - F12    */ KEY_SCAN | KEY_CTRL | KEY_ALT | 123,
    0,0,0,0,0,0,0,0,0,0,
    0,
    /* 3480 - Up     */ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 38 ,
    /* 3481 - PgUp   */ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 33 ,
    0,
    /* 3483 - Left   */ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 37 ,
    0,
    /* 3485 - Right  */ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 39 ,
    0,
    /* 3487 - End    */ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 35 ,
    /* 3488 - Down   */ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 40 ,
    /* 3489 - PgDn   */ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 34 ,
    /* 3490 - Insert */ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 45 ,
    /* 3491 - Delete */ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 46 ,
    /* 3492 - Kp/    */ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 111 , /* ??? */
    0,
    /* 3494 - KpEnter*/ KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 13
};


static int os2win32_XCAS[] =
{
    /* 3841 - ESC */       KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 27,
    0,0,0,0,0,0,0,0,0,0,
    0,0,
    /* 3854 - Backspace */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 8,
    0,
    /* 3856 - Q     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'Q',
    /* 3857 - W     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'W',
    /* 3858 - E     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'E',
    /* 3859 - R     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'R',
    /* 3860 - T     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'T',
    /* 3861 - Y     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'Y',
    /* 3862 - U     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'U',
    /* 3863 - I     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'I',
    /* 3864 - O     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'O',
    /* 3865 - P     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'P',
    /* 3866 - [     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 219,
    /* 3867 - ]     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 221,
    /* 3868 - Enter */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 13,
    0,
    /* 3870 - A     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'A',
    /* 3871 - S     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'S',
    /* 3872 - D     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'D',
    /* 3873 - F     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'F',
    /* 3874 - G     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'G',
    /* 3875 - H     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'H',
    /* 3876 - J     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'J',
    /* 3877 - K     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'K',
    /* 3878 - L     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'L',
    /* 3879 - ;     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 186,
    /* 3880 - '     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 222,
    /* 3881 - `     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 192,
    0,
    /* 3883 - \     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 220,
    /* 3884 - Z     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'Z',
    /* 3885 - X     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'X',
    /* 3886 - C     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'C',
    /* 3887 - V     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'V',
    /* 3888 - B     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'B',
    /* 3889 - N     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'N',
    /* 3890 - M     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'M',
    /* 3891 - ,     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 186,
    /* 3892 - .     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 190,
    /* 3893 - /     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 191,
    0,0,
    /* 3896 - Kp* */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 106 ,
    0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    /* 3914 - Kp- */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 109 ,
    0,0,0,
    /* 3918 - Kp+ */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 107 ,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,
    /* 3944 - F1     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 112,
    /* 3945 - F2     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 113,
    /* 3946 - F3     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 114,
    /* 3947 - F4     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 115,
    /* 3948 - F5     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 116,
    /* 3949 - F6     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 117,
    /* 3950 - F7     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 118,
    /* 3951 - F8     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 119,
    /* 3952 - F9     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 120,
    /* 3953 - F10    */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 121,
    0,0,0,0,0,0,
    /* 3960 - 1     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | '1',
    /* 3961 - 2     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | '2',
    /* 3962 - 3     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | '3',
    /* 3963 - 4     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | '4',
    /* 3964 - 5     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | '5',
    /* 3965 - 6     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | '6',
    /* 3966 - 7     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | '7',
    /* 3967 - 8     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | '8',
    /* 3968 - 9     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | '9',
    /* 3969 - 0     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | '0',
    /* 3970 - -     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 189,
    /* 3971 - =     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 187,
    0,0,0,0,0,0,0,
    /* 3979 - F11    */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 122,
    /* 3980 - F12    */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 123,
    0,0,0,0,0,0,0,0,0,0,
    /* 3991 - Home   */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 36 ,
    /* 3992 - Up     */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 38 ,
    /* 3993 - PgUp   */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 33 ,
    0,
    /* 3995 - Left   */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 37 ,
    0,
    /* 3997 - Right  */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 39 ,
    0,
    /* 3999 - End    */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 35 ,
    /* 4000 - Down   */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 40 ,
    /* 4001 - PgDn   */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 34 ,
    /* 4002 - Insert */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 45 ,
    /* 4003 - Delete */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 46 ,
    /* 4004 - Kp/    */ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 111 , /* ??? */
    0,
    /* 4006 - KpEnter*/ KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 13
};

static int os2win32_E[] = {
    /* 4138 - Kp* */    KEY_SCAN | 106 ,
    /* 4139 - Kp+ */    KEY_SCAN | 107 ,
    0,
    /* 4141 - Kp- */    KEY_SCAN | 109 ,
    /* 4142 - Decimal */KEY_SCAN | 110,
    /* 4143 - Kp/    */ KEY_SCAN | KEY_ENHANCED |  111 ,
    /* 4144 - Kp0    */ KEY_SCAN | 96 ,
    /* 4145 - Kp1    */ KEY_SCAN | 97 ,
    /* 4146 - Kp2    */ KEY_SCAN | 98 ,
    /* 4147 - Kp3    */ KEY_SCAN | 99 ,
    /* 4148 - Kp4    */ KEY_SCAN | 100 ,
    /* 4149 - Kp5    */ KEY_SCAN | 101 ,
    /* 4150 - Kp6    */ KEY_SCAN | 102 ,
    /* 4151 - Kp7    */ KEY_SCAN | 103 ,
    /* 4152 - Kp8    */ KEY_SCAN | 104 ,
    /* 4153 - Kp9    */ KEY_SCAN | 105
};

static int os2win32_E1[] =
{
    /* 4423 - Home   */ KEY_SCAN | KEY_ENHANCED |  36 ,
    /* 4424 - Up     */ KEY_SCAN | KEY_ENHANCED |  38 ,
    /* 4425 - PgUp   */ KEY_SCAN | KEY_ENHANCED |  33 ,
    0,
    /* 4427 - Left   */ KEY_SCAN | KEY_ENHANCED |  37 ,
    0,
    /* 4429 - Right  */ KEY_SCAN | KEY_ENHANCED |  39 ,
    0,
    /* 4431 - End    */ KEY_SCAN | KEY_ENHANCED |  35 ,
    /* 4432 - Down   */ KEY_SCAN | KEY_ENHANCED |  40 ,
    /* 4433 - PgDn   */ KEY_SCAN | KEY_ENHANCED |  34 ,
    /* 4434 - Insert */ KEY_SCAN | KEY_ENHANCED |  45 ,
    /* 4435 - Delete */ KEY_SCAN | KEY_ENHANCED |  46
};

static int os2win32_E1A[] =
{
    /* 4651 - Kp+ */    KEY_SCAN | KEY_SHIFT | 107 ,
    0,
    /* 4653 - Kp- */    KEY_SCAN | KEY_SHIFT | 109 ,
    /* 4654 - Decimal */KEY_SCAN | KEY_SHIFT | 46,
    /* 4655 - Kp/    */ KEY_SCAN | KEY_SHIFT | KEY_ENHANCED | 111 ,
    /* 4656 - Kp0    */ KEY_SCAN | KEY_SHIFT | 45 ,
    /* 4657 - Kp1    */ KEY_SCAN | KEY_SHIFT | 35 ,
    /* 4658 - Kp2    */ KEY_SCAN | KEY_SHIFT | 40 ,
    /* 4659 - Kp3    */ KEY_SCAN | KEY_SHIFT | 34 ,
    /* 4660 - Kp4    */ KEY_SCAN | KEY_SHIFT | 37 ,
    /* 4661 - Kp5    */ KEY_SCAN | KEY_SHIFT | 12 ,
    /* 4662 - Kp6    */ KEY_SCAN | KEY_SHIFT | 39 ,
    /* 4663 - Kp7    */ KEY_SCAN | KEY_SHIFT | 36 ,
    /* 4664 - Kp8    */ KEY_SCAN | KEY_SHIFT | 38 ,
    /* 4665 - Kp9    */ KEY_SCAN | KEY_SHIFT | 33
};

static int os2win32_E1B[] =
{
    /* 4935 - Home   */ KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  36,
    /* 4936 - Up     */ KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  38 ,
    /* 4937 - PgUp   */ KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  33 ,
    0,
    /* 4939 - Left   */ KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  37 ,
    0,
    /* 4941 - Right  */ KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  39 ,
    0,
    /* 4943 - End    */ KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  35 ,
    /* 4944 - Down   */ KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  40 ,
    /* 4945 - PgDn   */ KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  34 ,
    /* 4946 - Insert */ KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  45 ,
    /* 4947 - Delete */ KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  46
};

static int os2win32_E2[] =
{
    /* 5491 - Left   */ KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  37 ,
    /* 5492 - Right  */ KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  39 ,
    /* 5493 - End    */ KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  35 ,
    /* 5494 - PgDn   */ KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  34 ,
    /* 5495 - Home   */ KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  36 ,
     0,0,0,0,0,0,0,0,0,0,
     0,0,
    /* 5508 - PgUp   */ KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  33 ,
    0,0,0,0,0,0,0,0,
    /* 5517 - Up     */ KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  38 ,
    0,0,0,
    /* 5521 - Down   */ KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  40 ,
    /* 5522 - Insert */ KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  45 ,
    /* 5523 - Delete */ KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  46
};

static int os2win32_E3[] =
{
    /* 6003 - Left   */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  37 ,
    /* 6004 - Right  */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  39 ,
    /* 6005 - End    */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  35 ,
    /* 6006 - PgDn   */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  34 ,
    /* 6007 - Home   */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  36 ,
     0,0,0,0,0,0,0,0,0,0,
     0,0,
    /* 6020 - PgUp   */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  33 ,
    0,0,0,0,0,0,0,0,
    /* 6029 - Up     */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  38 ,
    0,0,0,
    /* 6033 - Down   */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  40 ,
    /* 6034 - Insert */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  45 ,
    /* 6035 - Delete */ KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  46
};

static int
os2towin32( int c )
{
    if ( c < 256 )
      c ;
    else if ( c < 512 )  /* KEY_SCAN */
    {
        switch ( c ) {
        case 257: /* ESC */
            c = 27 ;
            break;
        case 315: /* F1 */
            c = KEY_SCAN | 112 ;
            break;
        case 316: /* F2 */
            c = KEY_SCAN | 113 ;
            break;
        case 317: /* F3 */
            c = KEY_SCAN | 114 ;
            break;
        case 318: /* F4 */
            c = KEY_SCAN | 115 ;
            break;
        case 319: /* F5 */
            c = KEY_SCAN | 116 ;
            break;
        case 320: /* F6 */
            c = KEY_SCAN | 117 ;
            break;
        case 321: /* F7 */
            c = KEY_SCAN | 118 ;
            break;
        case 322: /* F8 */
            c = KEY_SCAN | 119 ;
            break;
        case 323: /* F9 */
            c = KEY_SCAN | 120 ;
            break;
        case 324: /* F10 */
            c = KEY_SCAN | 121 ;
            break;
        case 389: /* F11 */
            c = KEY_SCAN | 122 ;
            break;
        case 390: /* F12 */
            c = KEY_SCAN | 123 ;
            break;
        case 325: /* NUMLOCK */
            c = KEY_SCAN | 144 ;
            break;
        case 326: /* SCROLLBACK */
            c = KEY_SCAN | 145 ;
            break;
        case 270: /* Backspace */
            c = KEY_SCAN | 8 ;
            break;
        case 271: /* Tab */
            c = KEY_SCAN | 9 ;
            break;
        case 284: /* Enter */
            c = KEY_SCAN | 13 ;
            break;
        case 327: /* Kp7 - NumLock Off */
            c = KEY_SCAN | 36;   /* Home */
            break;
        case 328: /* Kp8 - NumLock Off */
            c = KEY_SCAN | 38;  /* Up */
            break;
        case 329: /* Kp9 - NumLock Off */
            c = KEY_SCAN | 33;  /* PgUp */
            break;
        case 331: /* Kp4 - NumLock Off */
            c = KEY_SCAN | 37; /* Left */
            break;
        case 332: /* Kp5 - NumLock Off */
            c = KEY_SCAN | 12; /* Clear */
            break;
        case 333: /* Kp6 - NumLock Off */
            c = KEY_SCAN | 39; /* Right */
            break;
        case 335: /* Kp1 - NumLock Off */
            c = KEY_SCAN | 35; /* End */
            break;
        case 336: /* Kp2 - NumLock Off */
            c = KEY_SCAN | 40; /* Down */
            break;
        case 337: /* Kp3 - NumLock Off */
            c = KEY_SCAN | 34; /* PgDn */
            break;
        case 338: /* Kp0 - NumLock Off */
            c = KEY_SCAN | 45; /* Insert */
            break;
        case 339: /* Decimal - NumLock Off */
            c = KEY_SCAN | 46; /* Delete */
            break;
        case 493: /* RightMSWindows */
            c = KEY_SCAN | 92;
            break;
        default:
            c = 0 ;
        }
    }
    else if ( c >= 782 && c <= 904 )     /* KEY_SCAN | KEY_SHIFT */
    {
       c = os2win32_XS[c-782] ;
    }
    else if ( c== 963 )
    {
        c = KEY_SCAN | KEY_SHIFT | 92 ; /* RightMSWindows */
    }
    else if ( c >= 1025 && c <= 1056 )      /* KEY_CTRL */
    {
        c = os2win32_C[c-1025] ;
    }
    else if ( c >= 1283 && c <= 1430 )      /* KEY_SCAN | KEY_CTRL */
    {
        c = os2win32_XC[c-1283] ;
    }
    else if ( c== 1486 )
    {
       c = KEY_SCAN | KEY_CTRL | KEY_ENHANCED | 91 ; /* LeftMSWindows */
    }
    else if ( c== 1487 )
    {
        c = KEY_SCAN | KEY_CTRL | KEY_ENHANCED | 92 ; /* RightMSWindows */
    }
    else if ( c== 1516 )
    {
        c = KEY_SCAN | KEY_ENHANCED | 91 ; /* LeftMSWindows */
    }
    else if ( c >= 1792 && c < 2048 )    /* KEY_SCAN | KEY_SHIFT | KEY_CTRL */
    {
       switch ( c ) {
       case 1793: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 27 ; break ;
       case 1795: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 50 ; break ;
       case 1806: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 8 ; break ;
       case 1818: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 219 ; break ;
       case 1820: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 13 ; break ;
       case 1827: c = KEY_SHIFT | KEY_CTRL | 8 ; break ;
       case 1828: c = KEY_SHIFT | KEY_CTRL | 9 ; break ;
       case 1842: c = KEY_SHIFT | KEY_CTRL | 13 ; break ;
       case 1886: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 112 ; break ;
       case 1887: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 113 ; break ;
       case 1888: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 114 ; break ;
       case 1889: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 115 ; break ;
       case 1890: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 116 ; break ;
       case 1891: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 117 ; break ;
       case 1892: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 118 ; break ;
       case 1893: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 119 ; break ;
       case 1894: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 120 ; break ;
       case 1895: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 121 ; break ;

       case 1907: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 37 ; break ;
       case 1908: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 39 ; break ;
       case 1909: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 35 ; break ;
       case 1910: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 34 ; break ;
       case 1911: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 36 ; break ;

       case 1924: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 33 ; break ;
       case 1929: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 122 ; break ;
       case 1930: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 123 ; break ;
       case 1933: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 38 ; break ;
       case 1935: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 12 ; break ;
       case 1937: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 40 ; break ;
       case 1938: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 45 ; break ;
       case 1939: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 46 ; break ;
       case 1940: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 9 ; break ;

       case 1985: c = KEY_SCAN | KEY_SHIFT | 91 ; break ;
       case 1998: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 91 ; break ;
       case 1999: c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | 92 ; break ;

       default:
           c = 0 ;
       }
    }
    else if ( c == 2080 )
    {
        c = KEY_ALT | 32 ;
    }
    else if ( c == 2084 )
    {
        c = KEY_SCAN | KEY_CTRL | KEY_ALT | 52 ; /* AltGr-4 - DK */
    }
    else if ( c >= 2112 && c <= 2301 ) /* AltGr combinations */
    {
        switch ( c )
        {
        case 2112: c = KEY_SCAN | KEY_CTRL | KEY_ALT | 81 ; break ; /* Q */
            /* This is Ctrl-Alt-2 with the DK keyboard */
        case 2204: c = KEY_SCAN | KEY_CTRL | KEY_ALT | 51 ; break ; /* 3 - DK */
        case 2171: c = KEY_SCAN | KEY_CTRL | KEY_ALT | 55 ; break ; /* 7 */
        case 2139: c = KEY_SCAN | KEY_CTRL | KEY_ALT | 56 ; break ; /* 8 */
        case 2141: c = KEY_SCAN | KEY_CTRL | KEY_ALT | 57 ; break ; /* 9 */
        case 2172: c = KEY_SCAN | KEY_CTRL | KEY_ALT | 226; break ; /* OEM.102 */
        case 2173: c = KEY_SCAN | KEY_CTRL | KEY_ALT | 48 ; break ; /* 0 */
        case 2140: c = KEY_SCAN | KEY_CTRL | KEY_ALT | 219 ; break ; /* - */
        case 2174: c = KEY_SCAN | KEY_CTRL | KEY_ALT | 109 ; break ; /* ] */
        case 2301: c = KEY_SCAN | KEY_CTRL | KEY_ALT | 50 ; break ;  /* 2 */
        case 2300: c = KEY_SCAN | KEY_CTRL | KEY_ALT | 51 ; break ;  /* 3 */
        case 2278: c = KEY_SCAN | KEY_CTRL | KEY_ALT | 77 ; break ; /* M */
        default: c |= KEY_SCAN | KEY_CTRL | KEY_ALT ;
            /* at least it will be something */
        }
    }
    else if ( c >= 2318 && c <= 2470 ) /* KEY_SCAN | KEY_ALT */
    {
        c = os2win32_XA[c-2318] ;
    }
    else if ( c== 2523 )
    {
        c = KEY_SCAN | KEY_ALT | KEY_ENHANCED | 92 ; /* RightMSWindows */
    }
    else if ( c >= 2830 && c <= 2982 ) /* KEY_SCAN | KEY_SHIFT | KEY_ALT */
    {
        c = os2win32_XAS[c-2830];
    }
    else if ( c == 3104 )
    {
        c = KEY_CTRL | KEY_ALT | 32 ;
    }
    else if ( c== 3035 )
    {
        c = KEY_SCAN | KEY_SHIFT | KEY_ALT | KEY_ENHANCED | 92 ; /* RightMSWindows */
    }
    else if ( c >= 3329 && c <= 3494 ) /* KEY_SCAN | KEY_CTRL | KEY_ALT */
    {
       c = os2win32_XCA[c-3329];
    }
    else if ( c== 3546 )
    {
        c = KEY_SCAN | KEY_ALT | KEY_ENHANCED | 91 ; /* LeftMSWindows */
    }
    else if ( c== 3547 )
    {
        c = KEY_SCAN | KEY_CTRL | KEY_ALT | KEY_ENHANCED | 92 ; /* RightMSWindows */
    }
    else if ( c >= 3841 && c <= 4006 )      /* KEY_SCAN | KEY_SHIFT | KEY_ALT | KEY_CTRL */
    {
        c = os2win32_XCAS[c-3841];
    }
    else if ( c== 4058 )
    {
        c = KEY_SCAN | KEY_SHIFT | KEY_ALT | KEY_ENHANCED | 91 ; /* LeftMSWindows */
    }
    else if ( c== 4059 )
    {
        c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | KEY_ENHANCED | 92 ; /* RightMSWindows */
    }
    else if ( c >= 4138 && c <= 4153 )
    {
        c = os2win32_E[c-4138];
    }
    else if ( c >= 4423 && c <= 4435 )
    {
        c = os2win32_E1[c-4423];
    }
    else if ( c == 4576 )
    {
        /* KpEnter */ c = KEY_SCAN | KEY_ENHANCED | 13 ;
    }
    else if ( c >= 4651 && c <= 4665 )
    {
        c = os2win32_E1A[c-4651];
    }
    else if ( c >= 4935 && c <= 4947 )
    {
        c = os2win32_E1B[c-4935];
    }
    else if ( c == 5088 )
    {
        /* KpEnter */ c = KEY_SCAN | KEY_SHIFT | KEY_ENHANCED | 13 ;
    }
    else if ( c >= 5491 && c <= 5523 )
    {
        c = os2win32_E2[c-5491];
    }
    else if ( c == 5600 )
    {
        /* KpEnter */ c = KEY_SCAN | KEY_CTRL | KEY_ENHANCED | 13 ;
    }
    else if ( c >= 6003 && c <= 6035 )
    {
        c = os2win32_E3[c-6003];
    }
    else if ( c == 6112 )
    {
        /* KpEnter */ c = KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED | 13 ;
    }
    else
      c |= KEY_SCAN ;

    return c ;
}

_PROTOTYP( APIRET ConnectToPM, (void) );
_PROTOTYP( APIRET ReadFromPM, (char *, ULONG, ULONG *) );
_PROTOTYP( APIRET DisconnectFromPM, (void) );

#ifndef NOLOCAL
void
KbdHandlerThread( void * ArgList ) {
    KBDKEYINFO k ;
    int rc, c ;
    extern BYTE vmode ;
    extern int priority;
    char buffer[512] ;
    ULONG bytes;

    debug(F100,"KbdHandlerThread is born","",0);
    SetThreadPrty(XYP_REG,3);
    rc = PostKbdThreadSem() ;

    /* we must wait for the keymap to be initialized before starting */
    /* to process keyboard input */
    WaitKeyMapInitSem(SEM_INDEFINITE_WAIT) ;
    /* Do not reset as we don't want to wait each */
    /* time this thread restarts */

    while ( KbdActive ) {
        if ( k95stdin ) {
            c = 0;
            if ( (c = getchar()) != EOF ) {
                clickkeys();
                putkey(vmode, c);
            }
            else
                msleep(80);
        }
        else if ( islocal ) {
            c = -1 ;
            if ( !ReadFromPM( buffer, 512, &bytes ))
            {
#ifndef NOLOCAL
                if ( IsConnectMode() )
                    SetConnectMode( FALSE, CSX_INTERNAL ) ;
#endif /* NOLOCAL */
                if ( bytes )
                    putmacro( VCMD, buffer ) ;
            }

            memset( &k, 0, sizeof(k) ) ;
            RequestKeyboardMutex( SEM_INDEFINITE_WAIT ) ;
            rc = KbdCharIn(&k, IO_NOWAIT, KbdHandle ) ;
            ReleaseKeyboardMutex() ;
            if (!(k.fbStatus & (3 << 6)))
                /* && !(k.fbStatus & 1) --- blocks ScrollLock */
            {
                msleep( 100 ) ; /* a human can't type faster than this */
                continue;       /* top level while(KbdActive) */
            }

            if ( !KbdActive )
                continue;       /* top level while(KbdActive) */

#ifdef PCTERM
            Can't implement PCTERM because I don't have a clue
            as to how we can capture the KEY_UP events in an OS/2
            console application.  Using KbdSetStatus() with the
            KEYBOARD_SHIFT_REPORT flag allows us to see the UP/DOWN
            events for SHIFT, CTRL, ALT, CAPSLOCK, SCROLLLOCK, NUMLOCK,
            and maybe a few others but not for the keyboard in general
            and for those it does report it does not include the scancode.
#ifndef COMMENT
            if ( tt_pcterm ) {
                printf("k.chChar    0x%X\n",k.chChar   );
                printf("k.chScan    0x%X\n",k.chScan   );
                printf("k.fbStatus  0x%X\n",k.fbStatus );
                printf("k.bNlsShift 0x%X\n",k.bNlsShift);
                printf("k.fsState   0x%X\n",k.fsState  );
                printf("k.time      0x%X\n",k.time     );
            }
#endif /* COMMENT */

            if ( mode == VTERM ) {
                /* Ctrl-CAPSLOCK is the toggle for PCTERM */
                static int pcterm_ctrl_down = 0, pcterm_ctrl_up = 0,
                pcterm_caps_down = 0, pcterm_caps_up = 0;

                switch ( k.wVirtualKeyCode ) {
                case VK_CONTROL:
                    if ( k.bKeyDown ) {
                        pcterm_ctrl_down = 1;
                    } else {
                        if ( pcterm_caps_down && pcterm_caps_up ) {
                            setpcterm(!tt_pcterm);
                            pcterm_ctrl_down = pcterm_ctrl_up = pcterm_caps_down =
                                pcterm_caps_up = 0;
                        } else {
                            pcterm_ctrl_up = 1;
                        }
                    }
                    break;
                case VK_CAPITAL:
                    if ( k.bKeyDown ) {
                        pcterm_caps_down = 1;
                    } else {
                        if ( pcterm_ctrl_down && pcterm_ctrl_up ) {
                            setpcterm(!tt_pcterm);
                            pcterm_ctrl_down = pcterm_ctrl_up = pcterm_caps_down =
                                pcterm_caps_up = 0;
                        } else {
                            pcterm_caps_up = 1;
                        }
                    }
                    break;
                default:
                    pcterm_ctrl_down = pcterm_ctrl_up = pcterm_caps_down =
                        pcterm_caps_up = 0;
                }

                if ( tt_pcterm && !wy_block ) {
                    char pre[16]="";
                    char hex[16]="";
                    char post[16]="";
                    char msg[32]="";
                    int i=0;

                    if ( k.fbStatus & ENHANCED_KEY ) {
                        if ( k.fsStatus & SHIFT) {
                            if ( k.bKeyDown )
                                sprintf(pre,"%c%c",0xE0,0xAA);
                            else
                                sprintf(post,"%c%c",0xE0,0x2A);
                        }
                        sprintf(hex,"%c",0xE0);
                        strcat(pre,hex);
                    }

                    for ( i=0 ; i<k.wRepeatCount && i<15 ; i++ ) {
                        if ( k.bKeyDown )
                            hex[i] = k.wVirtualScanCode;
                        else
                            hex[i] = k.wVirtualScanCode + 0x80;
                    }
                    hex[i] = '\0';
                    sprintf(msg,"%s%s%s",pre,hex,post);
                    ttol(msg,strlen(msg));
                    return;
                }
            }
#endif /* PCTERM */


            if ( k.chChar || k.chScan ) {
                if ( ( k.fbStatus & ENHANCED_KEY ) &&
                     ( k.chChar == 0x0 || k.chChar == 0xE0 ) )
                {
                    /* Then we have an extended scan code */
                    c = k.chScan | KEY_SCAN ;

                    /* We don't check for SHIFT of normal characters because they */
                    /* just generate other ASCII values */

                    if ( k.fsState & SHIFT )
                        c |= KEY_SHIFT ;

                    if ( k.chChar == 0xE0 )
                        c |= KEY_ENHANCED ;
                }
                else
                {
                    /* We have a normal character */
                    c = k.chChar ;
                    switch (c) {        /* Handle ambiguous keypad and space keys */
                    case ESC:
                        c = k.chScan | KEY_SCAN         |
                            ( k.fsState & SHIFT ? KEY_SHIFT : 0x0 ) ;
                        break ;
                    case '\t':
                        if ( k.chScan == 0x0F )
                        {
                            c = k.chScan | KEY_SCAN |
                                ( ( k.fbStatus & ENHANCED_KEY && k.chChar == 0xE0 )
                                  ? KEY_ENHANCED : 0x0 ) |
                                      ( k.fsState & SHIFT ? KEY_SHIFT : 0x0 ) ;
                        }
                        break ;
                    case DEL:
                        c = k.chScan | KEY_SCAN |
                            ( ( k.fbStatus & ENHANCED_KEY && k.chChar == 0xE0 )
                              ? KEY_ENHANCED : 0x0 ) |
                                  ( k.fsState & SHIFT ? KEY_SHIFT : 0x0 ) ;
                        break;
                    case '\b':
                        c = k.chScan | KEY_SCAN |
                            ( ( k.fbStatus & ENHANCED_KEY && k.chChar == 0xE0 )
                              ? KEY_ENHANCED : 0x0 ) |
                                  ( k.fsState & SHIFT ? KEY_SHIFT : 0x0 ) ;
                        break;
                    case '+':
                        /* c |= k.chScan == 0x4E ? KEY_ENHANCED : 0 ; */
                        if ( k.chScan == 0x4E )
                        {
                            c |= KEY_ENHANCED |
                                ( k.fsState & SHIFT ? KEY_SHIFT : 0x0 ) ;
                        }
                        break;
                    case '-':
                        /* c |= k.chScan == 0x4A ? KEY_ENHANCED : 0; */
                        if ( k.chScan == 0x4A )
                        {
                            c |= KEY_ENHANCED |
                                ( k.fsState & SHIFT ? KEY_SHIFT : 0x0 ) ;
                        }
                        break;
                    case '*':
                        /* c |= k.chScan == 0x37 ? KEY_ENHANCED : 0; */
                        if ( k.chScan == 0x37 )
                        {
                            c |= KEY_ENHANCED |
                                ( k.fsState & SHIFT ? KEY_SHIFT : 0x0 ) ;
                        }
                        break;
                    case '/':
                        if ( k.chScan == 0xE0 )
                        {
                            c |= KEY_ENHANCED |
                                ( k.fsState & SHIFT ? KEY_SHIFT : 0x0 ) ;
                        }
                        break;
                    case '\r':
                    case '\n':
                        /* c |= k.chScan == 0xE0 ? KEY_ENHANCED : 0; */
                        c = k.chScan | KEY_SCAN ;
                        if ( k.chScan == 0xE0 ||
                             k.chScan == 166 )
                        {
                            c |= KEY_ENHANCED ;
                        }
                        c |= ( k.fsState & SHIFT ? KEY_SHIFT : 0x0 ) ;
                        break;
                    case '.':
                    case ',':
                        /* c |= k.chScan == 0x53 ? KEY_ENHANCED : 0; */
                        if ( k.chScan == 0x53 )
                        {
                            c |= KEY_ENHANCED |
                                ( k.fsState & SHIFT ? KEY_SHIFT : 0x0 ) ;
                        }
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        /* c |= k.chScan >= 0x47 ? KEY_ENHANCED : 0; */
                        if ( k.chScan >= 0x47 )
                        {
                            c |= KEY_ENHANCED |
                                ( k.fsState & SHIFT ? KEY_SHIFT : 0x0 ) ;
                        }
                        break;
                    }
                }

                if ( k.fsState & CONTROL )
                    c |= KEY_CTRL ;

                if ( k.fsState & ALT )
                    c |= KEY_ALT ;
            }
            else if ( (k.fbStatus & SHIFT_KEY_IN) && (k.fsState & NUMLOCK) ) {
                c = KEY_SCAN | 69 | ((k.fsState & NUMLOCK_ON) ? KEY_SHIFT : 0x0) ;
            }
            else if ( (k.fbStatus & SHIFT_KEY_IN) && (k.fsState & SCROLLLOCK) ) {
                c = KEY_SCAN | 70 | ((k.fsState & SCROLLLOCK_ON) ? KEY_SHIFT : 0x0) ;
            }

            debug(F111,"KbdThreadHandler","scancode",c);
            if ( c >= 0 ) {
                c = os2towin32(c);      /* Convert Scan Code                         */
                debug(F111,"KbdThreadHandler","os2towin32(c)",c);
                if ( c >= 256 &&            /* Not a pure ascii value                    */
                     k.chChar != 0x00 &&    /* these values are used when there          */
                     k.chChar != 0xE0 ) {   /* is no Ascii value associated with the key */
#ifndef NOSETKEY
                    if ( keymap != NULL && mapkey(c).type == error &&
                         c < KMSIZE )   /* Not equal to its default value            */
                        keymap[c] = k.chChar ;
#endif /* NOSETKEY */
                }
                debug(F101,"putkey","",c);
                clickkeys() ;
                putkey( vmode, c ) ;
            }
        }   /* if ( !islocal ) */
        else {
            c = 0;
            if ( read(ttyfd, &c, 1) == 1 ) {
                putkey(vmode,c);
            }
            else
                msleep(80);
        }
    }  /* while ( KbdActive ) */

    PostKbdThreadSem() ;
    debug(F100,"KbdHandlerThread dies","",0);
    ckThreadEnd(NULL);
}
#endif /* NOLOCAL */
#endif /* NT */

#ifndef NOSETKEY
/* Initialize key map */
void
keymapinit() {
    int i ;
#ifdef BETADEBUG
    printf("Initializing Default Keymaps\n");
#endif /* BETADEBUG */
    for ( i=0;i<=TT_MAX+4;i++ )
        defaultkeymap(i) ;
#ifdef BETADEBUG
    printf("Initialization complete!\n");
#endif /* BETADEBUG */

   PostKeyMapInitSem() ;
}

char *
keyname( ULONG key )
{
    static char namestr[80] ;
    ULONG c = 0 ;
    int len = 0;

    namestr[0] = '\0' ;
    if ( key & KEY_CTRL &&
         !(charnam[c] && !(key & KEY_SCAN)))
    {
        strcat( namestr, "Ctrl-" ) ;
        len += 5 ;
    }

    if ( key & KEY_ALT )
    {
        strcat( namestr, "Alt-" ) ;
        len += 4 ;
    }

    if ( key & KEY_SHIFT )
    {
        strcat( namestr, "Shift-" ) ;
        len += 6 ;
    }

    if ( key & KEY_ENHANCED )
    {
        if ( key & KEY_SCAN )
        {
            strcat( namestr, "Gray-" ) ;
            len += 5 ;
        }
        else
        {
            strcat( namestr, "Kp-" ) ;
            len += 3 ;
        }
    }

    c = key & 0xFF ;
    if ( key & KEY_SCAN )
    {
        char buf[32] ;
#ifdef NT
        strcat( namestr, keynam[c] == NULL ? _itoa(c, buf, 10) : keynam[c] ) ;
#else /* NT */
        strcat( namestr, keynam[c] == NULL ? itoa(c, buf, 10) : keynam[c] ) ;
#endif /* NT */
    }
    else /* A pure ASCII value */
    {
        if ( charnam[c] )
            strcat( namestr, charnam[c] ) ;
        else if ( c == 0x00 )
            return NULL ;
        else if ( c == 8 )
        {
            strcat( namestr, "BS" ) ;
            len = 2 ;
        }
        else if ( c == 9 )
        {
            strcat( namestr, "TAB" ) ;
            len = 3 ;
        }
        else if ( c == 10 )
        {
            strcat( namestr, "LF" ) ;
            len = 2 ;
        }
        else if ( c == 13 )
        {
            strcat( namestr, "CR" ) ;
            len = 2 ;
        }
        else if ( c == 27 )
        {
            strcat( namestr, "ESC" ) ;
            len = 3 ;
        }
        else if ( c < 32 )
        {
            namestr[len++] = '^' ;
            namestr[len++] = c + 64 ;
            namestr[len] = '\0' ;
        }
        else if ( c == 127 )
        {
            strcat( namestr, "DEL" ) ;
            len = 9 ;
        }
        else if ( c == 32 )
        {
            strcat( namestr, "SPACE" ) ;
            len = 11 ;
        }
        else
        {
            namestr[len++] = c ;
            namestr[len] = '\0' ;
        }
    }
    return *namestr ? namestr : NULL ;
}

VOID
keynaminit() {

/*  These names apply only to US keyboards.  */

    int i;

    for (i = 0; i < 256; i++) {
        keynam[i] = NULL ;  /* Should be NULL */
        charnam[i] = NULL ;
    }

    keynam[1] = "LMouse" ;
    keynam[2] = "RMouse" ;
    keynam[3] = "Control-Break" ;
    keynam[4] = "MMouse" ;

    keynam[8] = "Backspace" ;
    keynam[9] = "Tab" ;

    keynam[12] = "Clear" ;
    keynam[13] = "Enter" ;

    keynam[16] = "Shift" ;
    keynam[17] = "Control" ;
    keynam[18] = "Alt" ;
    keynam[19] = "Pause" ;
    keynam[20] = "CapsLock" ;

    keynam[27] = "Esc" ;

    keynam[32] = "Space" ;
    keynam[33] = "PageUp" ;
    keynam[34] = "PageDown" ;
    keynam[35] = "End" ;
    keynam[36] = "Home" ;
    keynam[37] = "LeftArrow" ;
    keynam[38] = "UpArrow" ;
    keynam[39] = "RightArrow" ;
    keynam[40] = "DownArrow" ;
    keynam[41] = "Select" ;
    keynam[42] = "Print";  /* Nokia */
    keynam[43] = "Execute" ;
    keynam[44] = "PrintScreen" ;
    keynam[45] = "Insert" ;
    keynam[46] = "Delete" ;
    keynam[47] = "Help" ;
    keynam[48] = "0" ;
    keynam[49] = "1" ;
    keynam[50] = "2" ;
    keynam[51] = "3" ;
    keynam[52] = "4" ;
    keynam[53] = "5" ;
    keynam[54] = "6" ;
    keynam[55] = "7" ;
    keynam[56] = "8" ;
    keynam[57] = "9" ;

    keynam[65] = "A" ;
    keynam[66] = "B" ;
    keynam[67] = "C" ;
    keynam[68] = "D" ;
    keynam[69] = "E" ;
    keynam[70] = "F" ;
    keynam[71] = "G" ;
    keynam[72] = "H" ;
    keynam[73] = "I" ;
    keynam[74] = "J" ;
    keynam[75] = "K" ;
    keynam[76] = "L" ;
    keynam[77] = "M" ;
    keynam[78] = "N" ;
    keynam[79] = "O" ;
    keynam[80] = "P" ;
    keynam[81] = "Q" ;
    keynam[82] = "R" ;
    keynam[83] = "S" ;
    keynam[84] = "T" ;
    keynam[85] = "U" ;
    keynam[86] = "V" ;
    keynam[87] = "W" ;
    keynam[88] = "X" ;
    keynam[89] = "Y" ;
    keynam[90] = "Z" ;

    keynam[91] = "LeftMSWindows" ;
    keynam[92] = "RightMSWindows" ;
    keynam[93] = "TaskList" ;

    keynam[96] = "Keypad-0" ;
    keynam[97] = "Keypad-1" ;
    keynam[98] = "Keypad-2" ;
    keynam[99] = "Keypad-3" ;
    keynam[100] = "Keypad-4" ;
    keynam[101] = "Keypad-5" ;
    keynam[102] = "Keypad-6" ;
    keynam[103] = "Keypad-7" ;
    keynam[104] = "Keypad-8" ;
    keynam[105] = "Keypad-9" ;
    keynam[106] = "Multiply" ;
    keynam[107] = "Add" ;
    keynam[108] = "Separator" ;
    keynam[109] = "Subtract" ;
    keynam[110] = "Decimal" ;
    keynam[111] = "Divide" ;
    keynam[112] = "F1" ;
    keynam[113] = "F2" ;
    keynam[114] = "F3" ;
    keynam[115] = "F4" ;
    keynam[116] = "F5" ;
    keynam[117] = "F6" ;
    keynam[118] = "F7" ;
    keynam[119] = "F8" ;
    keynam[120] = "F9" ;
    keynam[121] = "F10" ;
    keynam[122] = "F11" ;
    keynam[123] = "F12" ;
    keynam[124] = "F13" ;
    keynam[125] = "F14" ;
    keynam[126] = "F15" ;
    keynam[127] = "F16" ;
    keynam[128] = "F17" ;
    keynam[129] = "F18" ;
    keynam[130] = "F19" ;
    keynam[131] = "F20" ;
    keynam[132] = "F21" ;
    keynam[133] = "F22" ;
    keynam[134] = "F23" ;
    keynam[135] = "F24" ;

    keynam[144] = "NumLock" ;
    keynam[145] = "ScrollLock" ;

    keynam[166] = "Web Page Backward";
    keynam[167] = "Web Page Forward";

    keynam[186] = "OEM.US.SemiColon" ;
    keynam[187] = "OEM.US.Equal" ;
    keynam[188] = "OEM.US.Comma" ;
    keynam[189] = "OEM.US.Subtract" ;
    keynam[190] = "OEM.US.Period" ;
    keynam[191] = "OEM.US.Slash" ;
    keynam[192] = "OEM.US.Grave" ;

    keynam[219] = "OEM.US.LeftBracket" ;
    keynam[220] = "OEM.US.BackSlash" ;
    keynam[221] = "OEM.US.RightBracket" ;
    keynam[222] = "OEM.US.Quote" ;
    keynam[223] = "OEM_8";
    keynam[224] = "Olivetti.F17" ;
    keynam[225] = "Olivetti.F18" ;
    keynam[226] = "OEM.IBM.Enhanced.102";
    keynam[227] = "Olivetti.Help";
    keynam[228] = "Olivetti.00";

    keynam[230] = "Olivetti.Clear";

    keynam[233] = "OEM.Reset";  /* Nokia */
    keynam[234] = "OEM.Jump" ;  /* Nokia */
    keynam[235] = "OEM.PA1";    /* Nokia */
    keynam[236] = "OEM.PA2";    /* Nokia */
    keynam[237] = "OEM.PA3";    /* Nokia */
    keynam[238] = "OEM.WSCtrl"; /* Nokia */
    keynam[239] = "OEM.CUSel";  /* Nokia */
    keynam[240] = "OEM.Attn";   /* Nokia */
    keynam[241] = "OEM.Finnish";/* Nokia */
    keynam[242] = "OEM.Copy";   /* Nokia */
    keynam[243] = "OEM.Auto";   /* Nokia */
    keynam[244] = "OEM.Enlw";   /* Nokia */
    keynam[245] = "OEM.Backtab";/* Nokia */
    keynam[246] = "Attn";
    keynam[247] = "Crsel";
    keynam[248] = "Exsel";
    keynam[249] = "Ereof";
    keynam[250] = "Play";
    keynam[251] = "Zoom";
    keynam[252] = "Noname";
    keynam[253] = "PA1";
    keynam[254] = "OEM.Clear";

    /* Now complete the char name table */
    charnam[0] = "NUL" ;
    charnam[1] = "SOH" ;
    charnam[2] = "STX" ;
    charnam[3] = "ETX" ;
    charnam[4] = "EOT" ;
    charnam[5] = "ENQ" ;
    charnam[6] = "ACK" ;
    charnam[7] = "BEL" ;
    charnam[8] = "BS" ;
    charnam[9] = "HT" ;
    charnam[10] = "LF" ;
    charnam[11] = "VT" ;
    charnam[12] = "FF" ;
    charnam[13] = "CR" ;
    charnam[14] = "SO" ;
    charnam[15] = "SI" ;
    charnam[16] = "DLE" ;
    charnam[17] = "DC1" ;
    charnam[18] = "DC2" ;
    charnam[19] = "DC3" ;
    charnam[20] = "DC4" ;
    charnam[21] = "NAK" ;
    charnam[22] = "SYN" ;
    charnam[23] = "ETB" ;
    charnam[24] = "CAN" ;
    charnam[25] = "EM" ;
    charnam[26] = "SUB" ;
    charnam[27] = "ESC" ;
    charnam[28] = "FS" ;
    charnam[29] = "GS" ;
    charnam[30] = "RS" ;
    charnam[31] = "US" ;
    charnam[32] = "SP" ;
    charnam[127] = "DEL" ;
    charnam[132] = "IND" ;
    charnam[133] = "NEL" ;
    charnam[134] = "SSA" ;
    charnam[135] = "ESA" ;
    charnam[136] = "HTS" ;
    charnam[137] = "HTJ" ;
    charnam[138] = "VTS" ;
    charnam[139] = "PLD" ;
    charnam[140] = "PLU" ;
    charnam[141] = "RI" ;
    charnam[142] = "SS2" ;
    charnam[143] = "SS3" ;
    charnam[144] = "DCS" ;
    charnam[145] = "PU1" ;
    charnam[146] = "PU2" ;
    charnam[147] = "STS" ;
    charnam[148] = "CCH" ;
    charnam[149] = "MW" ;
    charnam[150] = "SPA" ;
    charnam[151] = "EPA" ;
    charnam[155] = "CSI" ;
    charnam[156] = "ST" ;
    charnam[157] = "OSC" ;
    charnam[158] = "PM" ;
    charnam[159] = "APC" ;
}

struct {
    int msk ;
    int ck ;
} mskcontab[] = {
    /* Control Keys */
/* Ctrl-A */ 1, 1025 ,
/* Ctrl-B */ 2, 1026 ,
/* Ctrl-C */ 3, 1027 ,
/* Ctrl-D */ 4, 1028 ,
/* Ctrl-E */ 5, 1029 ,
/* Ctrl-F */ 6, 1030 ,
/* Ctrl-G */ 7, 1031 ,
/* Ctrl-H */ 8, 1032 ,
/* Ctrl-I */ 9, 1033 ,
/* Ctrl-J */ 10, 1034 ,
/* Ctrl-K */ 11, 1035 ,
/* Ctrl-L */ 12, 1036 ,
/* Ctrl-M */ 13, 1037 ,
/* Ctrl-N */ 14, 1038 ,
/* Ctrl-O */ 15, 1039 ,
/* Ctrl-P */ 16, 1040 ,
/* Ctrl-Q */ 17, 1041 ,
/* Ctrl-R */ 18, 1042 ,
/* Ctrl-S */ 19, 1043 ,
/* Ctrl-T */ 20, 1044 ,
/* Ctrl-U */ 21, 1045 ,
/* Ctrl-V */ 22, 1046 ,
/* Ctrl-W */ 23, 1047 ,
/* Ctrl-X */ 24, 1048 ,
/* Ctrl-Y */ 25, 1049 ,
/* Ctrl-Z */ 26, 1050 ,
/* Ctrl-[ */ 27, 1051 ,
/* Ctrl-\ */ 28, 1052 ,
/* Ctrl-] */ 29, 1053 ,
/* Ctrl-^ */ 30, 1054 ,
/* Ctrl-_ */ 31, 1055 ,

/* Esc */   257, 27 ,
/* BS */    270, 264 ,
/* Tab */   271, 265 ,
/* Enter */ 284, 269 ,
/* Space */ 313, 32 ,

/* Fn */
/* F1 */ 315, 368 ,
/* F2 */ 316, 369 ,
/* F3 */ 317, 370 ,
/* F4 */ 318, 371 ,
/* F5 */ 319, 372 ,
/* F6 */ 320, 373 ,
/* F7 */ 321, 374 ,
/* F8 */ 322, 375 ,
/* F9 */ 323, 376 ,
/* F10*/ 324, 377 ,
/* F11*/ 389, 378 ,
/* F12*/ 390, 379 ,

    /* Shift */
/* Esc */   769, 539 ,
/* BS */    782, 776 ,
/* Tab */   783, 777 ,
/* Enter */ 796, 781 ,
/* Space */ 825, 32 ,

/* Shift-Fn */
/* F1 */ 852, 880 ,
/* F2 */ 853, 881 ,
/* F3 */ 854, 882 ,
/* F4 */ 855, 883 ,
/* F5 */ 856, 884 ,
/* F6 */ 857, 885 ,
/* F7 */ 858, 886 ,
/* F8 */ 859, 887 ,
/* F9 */ 860, 888 ,
/* F10*/ 861, 889 ,
/* F11*/ 903, 890 ,
/* F12*/ 904, 891 ,

/* Gray Keys */
4434, 4397 ,
4435, 4398 ,
4423, 4388 ,
4431, 4387 ,
4425, 4385 ,
4433, 4386 ,
4427, 4389 ,
4432, 4392 ,
4429, 4391 ,
4424, 4390 ,

/* Ctrl-Fn */
/* F1 */ 1374, 1392 ,
/* F2 */ 1375, 1393 ,
/* F3 */ 1376, 1394 ,
/* F4 */ 1377, 1395 ,
/* F5 */ 1378, 1396 ,
/* F6 */ 1379, 1397 ,
/* F7 */ 1380, 1398 ,
/* F8 */ 1381, 1399 ,
/* F9 */ 1382, 1400 ,
/* F10*/ 1383, 1401 ,
/* F11*/ 1417, 1402 ,
/* F12*/ 1418, 1403 ,

/* Alt-Fn */
/* F1 */ 2408, 2416 ,
/* F2 */ 2409, 2417 ,
/* F3 */ 2410, 2418 ,
/* F4 */ 2411, 2419 ,
/* F5 */ 2412, 2420 ,
/* F6 */ 2413, 2421 ,
/* F7 */ 2414, 2422 ,
/* F8 */ 2415, 2423 ,
/* F9 */ 2416, 2424 ,
/* F10*/ 2417, 2425 ,
/* F11*/ 2443, 2426 ,
/* F12*/ 2444, 2427 ,

/* Ctrl-Shift-Fn */
/* F1 */ 1886, 1904 ,
/* F2 */ 1887, 1905 ,
/* F3 */ 1888, 1906 ,
/* F4 */ 1889, 1907 ,
/* F5 */ 1890, 1908 ,
/* F6 */ 1891, 1909 ,
/* F7 */ 1892, 1910 ,
/* F8 */ 1893, 1911 ,
/* F9 */ 1894, 1912 ,
/* F10*/ 1895, 1913 ,
/* F11*/ 1929, 1914 ,
/* F12*/ 1930, 1915 ,

/* Alt-Shift-Fn */
/* F1 */ 2920, 2928 ,
/* F2 */ 2921, 2929 ,
/* F3 */ 2922, 2930 ,
/* F4 */ 2923, 2931 ,
/* F5 */ 2924, 2932 ,
/* F6 */ 2925, 2933 ,
/* F7 */ 2926, 2934 ,
/* F8 */ 2927, 2935 ,
/* F9 */ 2928, 2936 ,
/* F10*/ 2929, 2937 ,
/* F11*/ 2955, 2938 ,
/* F12*/ 2956, 2939 ,

/* Ctrl-Alt-Fn */
/* F1 */ 3432, 3440 ,
/* F2 */ 3433, 3441 ,
/* F3 */ 3434, 3442 ,
/* F4 */ 3435, 3443 ,
/* F5 */ 3436, 3444 ,
/* F6 */ 3437, 3445 ,
/* F7 */ 3438, 3446 ,
/* F8 */ 3439, 3447 ,
/* F9 */ 3440, 3448 ,
/* F10*/ 3441, 3449 ,
/* F11*/ 3467, 3450 ,
/* F12*/ 3468, 3451 ,

/* Ctrl-Alt-Shift-Fn */
/* F1 */ 3944, 3952 ,
/* F2 */ 3945, 3953 ,
/* F3 */ 3946, 3954 ,
/* F4 */ 3947, 3955 ,
/* F5 */ 3948, 3956 ,
/* F6 */ 3949, 3957 ,
/* F7 */ 3950, 3958 ,
/* F8 */ 3951, 3959 ,
/* F9 */ 3952, 3960 ,
/* F10*/ 3953, 3961 ,
/* F11*/ 3979, 3962 ,
/* F12*/ 3980, 3963 ,

/* Ctrl */
/* Tab */ 1428, 1289 ,
/* BS */  127, 1151 ,
/* Enter */ 1308, 1293,
/* Space */ 1337, 1056,
/* 2 */   1283, 1330,
/* 6 */   30, 1334,
/* - */   31, 1469,
/* \ */   28, 1052,

/* Alt */
/* BS */    2318, 2312 ,
/* Enter */ 2332, 2317 ,
/* Space */ 2361, 2080 ,

2345, 2144 ,
2424, 2097 ,
2425, 2098 ,
2426, 2099 ,
2427, 2100 ,
2428, 2101 ,
2429, 2102 ,
2430, 2103 ,
2431, 2104 ,
2432, 2105 ,
2433, 2096 ,
2434, 2093 ,
2435, 2109 ,

2320, 2161 ,

2334, 2145 ,


/* Ctrl-Shift */
1940, 1801 ,
1820, 1805 ,

/* Alt-Shift */
2830, 2824 ,

/* Ctrl-Alt-Shift */
4005, 3849 ,
3854, 3848 ,
3868, 3853 ,

/* Alt-Alphabet */
/* ' */ 2344, 2087,
/* , */ 2355, 2092,
/* - */ 2434, 2093,
/* . */ 2356, 2094,
/* / */ 2357, 2095,
/* 0 */ 2433, 2096,
/* 1 */ 2424, 2097,
/* 2 */ 2425, 2098,
/* 3 */ 2426, 2099,
/* 4 */ 2427, 2100,
/* 5 */ 2428, 2101,
/* 6 */ 2429, 2102,
/* 7 */ 2430, 2103,
/* 8 */ 2431, 2104,
/* 9 */ 2432, 2105,
/* ; */ 2343, 2107,
/* = */ 2435, 2109,
/* [ */ 2330, 2139,
/* \ */ 2347, 2140,
/* ] */ 2331, 2141,
/* ` */ 2345, 2144,
/* a */ 2334, 2145,
/* b */ 2352, 2146,
/* c */ 2350, 2147,
/* d */ 2336, 2148,
/* e */ 2322, 2149,
/* f */ 2337, 2150,
/* g */ 2338, 2151,
/* h */ 2339, 2152,
/* i */ 2327, 2153,
/* j */ 2340, 2154,
/* k */ 2341, 2155,
/* l */ 2342, 2156,
/* m */ 2354, 2157,
/* n */ 2353, 2158,
/* o */ 2328, 2159,
/* p */ 2329, 2160,
/* q */ 2320, 2161,
/* r */ 2323, 2162,
/* s */ 2335, 2163,
/* t */ 2324, 2164,
/* u */ 2326, 2165,
/* v */ 2351, 2166,
/* w */ 2321, 2167,
/* x */ 2349, 2168,
/* y */ 2325, 2169,
/* z */ 2348, 2170,
/* " */ 2856, 2082,
/* < */ 2867, 2108,
/* _ */ 2946, 2143,
/* > */ 2868, 2110,
/* ? */ 2869, 2111,
/* ) */ 2945, 2089,
/* ! */ 2936, 2081,
/* @ */ 2937, 2112,
/* # */ 2938, 2083,
/* $ */ 2939, 2084,
/* % */ 2940, 2085,
/* ^ */ 2941, 2142,
/* & */ 2942, 2086,
/* * */ 2943, 2090,
/* ( */ 2944, 2088,
/* : */ 2855, 2106,
/* + */ 2947, 2091,
/* { */ 2842, 2171,
/* | */ 2859, 2172,
/* } */ 2843, 2173,
/* ~ */ 2857, 2174,
/* A */ 2846, 2113,
/* B */ 2864, 2114,
/* C */ 2862, 2115,
/* D */ 2848, 2116,
/* E */ 2834, 2117,
/* F */ 2849, 2118,
/* G */ 2850, 2119,
/* H */ 2851, 2120,
/* I */ 2839, 2121,
/* J */ 2852, 2122,
/* K */ 2853, 2123,
/* L */ 2854, 2124,
/* M */ 2866, 2125,
/* N */ 2865, 2126,
/* O */ 2840, 2127,
/* P */ 2841, 2128,
/* Q */ 2832, 2129,
/* R */ 2835, 2130,
/* S */ 2847, 2131,
/* T */ 2836, 2132,
/* U */ 2838, 2133,
/* V */ 2863, 2134,
/* W */ 2833, 2135,
/* X */ 2861, 2136,
/* Y */ 2837, 2137,
/* Z */ 2860, 2138,

/* Ctrl-Alt Alphabet */
/* ' */ 3368, 3550,
/* , */ 3379, 3516,
/* - */ 3458, 3517,
/* . */ 3380, 3518,
/* / */ 3381, 3519,
/* 0 */ 3457, 3376,
/* 1 */ 3448, 3377,
/* 2 */ 3449, 3378,
/* 3 */ 3450, 3379,
/* 4 */ 3451, 3380,
/* 5 */ 3452, 3381,
/* 6 */ 3453, 3382,
/* 7 */ 3454, 3383,
/* 8 */ 3455, 3384,
/* 9 */ 3456, 3385,
/* ; */ 3367, 3514,
/* = */ 3459, 3515,
/* [ */ 3354, 3547,
/* \ */ 3371, 3548,
/* ] */ 3355, 3549,
/* ` */ 3369, 3520,
/* a */ 3358, 3393,
/* b */ 3376, 3394,
/* c */ 3374, 3395,
/* d */ 3360, 3396,
/* e */ 3346, 3397,
/* f */ 3361, 3398,
/* g */ 3362, 3399,
/* h */ 3363, 3400,
/* i */ 3351, 3401,
/* j */ 3364, 3402,
/* k */ 3365, 3403,
/* l */ 3366, 3404,
/* m */ 3378, 3405,
/* n */ 3377, 3406,
/* o */ 3352, 3407,
/* p */ 3353, 3408,
/* q */ 3344, 3409,
/* r */ 3347, 3410,
/* s */ 3359, 3411,
/* t */ 3348, 3412,
/* u */ 3350, 3413,
/* v */ 3375, 3414,
/* w */ 3345, 3415,
/* x */ 3373, 3416,
/* y */ 3349, 3417,
/* z */ 3372, 3418,
/* " */ 3880, 4062,
/* < */ 3891, 4028,
/* _ */ 3970, 4029,
/* > */ 3892, 4030,
/* ? */ 3893, 4031,
/* ) */ 3969, 3888,
/* ! */ 3960, 3889,
/* @ */ 3961, 3890,
/* # */ 3962, 3891,
/* $ */ 3963, 3892,
/* % */ 3964, 3893,
/* ^ */ 3965, 3894,
/* & */ 3966, 3895,
/* * */ 3967, 3896,
/* ( */ 3968, 3897,
/* : */ 3879, 4026,
/* + */ 3971, 4027,
/* { */ 3866, 4059,
/* | */ 3883, 4060,
/* } */ 3867, 4061,
/* ~ */ 3881, 4032,
/* A */ 3870, 3905,
/* B */ 3888, 3906,
/* C */ 3886, 3907,
/* D */ 3872, 3908,
/* E */ 3858, 3909,
/* F */ 3873, 3910,
/* G */ 3874, 3911,
/* H */ 3875, 3912,
/* I */ 3863, 3913,
/* J */ 3876, 3914,
/* K */ 3877, 3915,
/* L */ 3878, 3916,
/* M */ 3890, 3917,
/* N */ 3889, 3918,
/* O */ 3864, 3919,
/* P */ 3865, 3920,
/* Q */ 3856, 3921,
/* R */ 3859, 3922,
/* S */ 3871, 3923,
/* T */ 3860, 3924,
/* U */ 3862, 3925,
/* V */ 3887, 3926,
/* W */ 3857, 3927,
/* X */ 3885, 3928,
/* Y */ 3861, 3929,
/* Z */ 3884, 3930,

/* Keypad */
/* NumLock - no keycode on MSK */
/* Gray Divide - WinNT */       4399, 4463,
/* Gray Divide - Win95 */       4399, 4143,
/* Multiply */          311, 362,
/* Subtract */          330, 365,
/* Add */               334, 363,
/* Gray-Enter */        4365, 4365,
/* Insert */            338, 301,
/* Delete */            339, 302,
/* End */               335, 291,
/* DownArrow */         336, 296,
/* PgDn */              337, 290,
/* LeftArrow */         331, 293,
/* Clear */             332, 268,
/* RightArrow */        333, 295,
/* Home */              327, 292,
/* UpArrow */           328, 294,
/* PgUp */              329, 289,
/* Decimal */           850/* 339 */, 366,
/* Kp-0 */              851/* 338 */, 352,
/* Kp-1 */              847/* 335 */, 353,
/* Kp-2 */              848/* 336 */, 354,
/* Kp-3 */              849/* 337 */, 355,
/* Kp-4 */              843/* 331 */, 356,
/* Kp-5 */              844/* 332 */, 357,
/* kp-6 */              845/* 333 */, 358,
/* kp-7 */              839/* 327 */, 359,
/* kp-8 */              840/* 328 */, 360,
/* kp-9 */              841/* 329 */, 361,

/* Shift-Keypad */
/* NumLock - no keycode on MSK */
/* Gray Divide */       4911, 4975,
/* Multiply */          823, 874,
/* Subtract */          842, 877,
/* Add */               846, 875,
/* Gray-Enter */        4877, 4877,
/* Insert */            850, 813,
/* Delete */            851, 814,
/* End */               847, 803,
/* DownArrow */         848, 808,
/* PgDn */              849, 802,
/* LeftArrow */         843, 805,
/* Clear */             844, 780,
/* RightArrow */        845, 807,
/* Home */              839, 804,
/* UpArrow */           840, 806,
/* PgUp */              841, 801,

/* Ctrl-Keypad */
/* NumLock - no keycode on MSK */
/* Gray Divide */       1429, 5487,
/* Multiply */          1430, 1386,
/* Subtract */          1422, 1389,
/* Add */               1424, 1387,
/* Gray-Enter */        5386, 5389,
/* Insert */            1426, 1325,
/* Delete */            1427, 1326,
/* End */               1397, 1315,
/* DownArrow */         1425, 1320,
/* PgDn */              1398, 1314,
/* LeftArrow */         1395, 1317,
/* Clear */             1423, 1292,
/* RightArrow */        1396, 1319,
/* Home */              1399, 1316,
/* UpArrow */           1421, 1318,
/* PgUp */              1412, 1313,

/* Alt-Keypad */
/* Gray Divide */       2468, 6511,
/* Multiply */          2359, 2410,
/* Subtract */          2378, 2413,
/* Add */               2382, 2411,

/* Alt-Shift-Keypad */
/* Gray Divide */       2980, 7023,
/* Multiply */          2871, 2922,
/* Subtract */          2890, 2925,
/* Add */               2894, 2923,

/* Alt-Ctrl-Keypad */
/* Gray Divide */       3492, 7535,
/* Multiply */          3383, 3434,
/* Subtract */          3402, 3437,
/* Add */               3406, 3435,
/* Gray-Enter */        3494, 7437,

/* Alt-Ctrl-Shift-Keypad */
/* Gray Divide */       4004, 8047,
/* Multiply */          3895, 3946,
/* Subtract */          3914, 3949,
/* Add */               3918, 3947,
/* Gray-Enter */        4006, 7949,

/* Gray */
/* - Home   */ 4423, KEY_SCAN | KEY_ENHANCED |  36 ,
/* - Up     */ 4424, KEY_SCAN | KEY_ENHANCED |  38 ,
/* - PgUp   */ 4425, KEY_SCAN | KEY_ENHANCED |  33 ,
/* - Left   */ 4427, KEY_SCAN | KEY_ENHANCED |  37 ,
/* - Right  */ 4429, KEY_SCAN | KEY_ENHANCED |  39 ,
/* - End    */ 4431, KEY_SCAN | KEY_ENHANCED |  35 ,
/* - Down   */ 4432, KEY_SCAN | KEY_ENHANCED |  40 ,
/* - PgDn   */ 4433, KEY_SCAN | KEY_ENHANCED |  34 ,
/* - Insert */ 4434, KEY_SCAN | KEY_ENHANCED |  45 ,
/* - Delete */ 4435, KEY_SCAN | KEY_ENHANCED |  46 ,

/* Shift-Gray */
/* - Home   */ 4935, KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  36,
/* - Up     */ 4936, KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  38 ,
/* - PgUp   */ 4937, KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  33 ,
/* - Left   */ 4939, KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  37 ,
/* - Right  */ 4941, KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  39 ,
/* - End    */ 4943, KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  35 ,
/* - Down   */ 4944, KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  40 ,
/* - PgDn   */ 4945, KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  34 ,
/* - Insert */ 4946, KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  45 ,
/* - Delete */ 4947, KEY_SCAN | KEY_SHIFT | KEY_ENHANCED |  46 ,

/* Ctrl-Gray */
/* - Left   */ 5491, KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  37 ,
/* - Right  */ 5492, KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  39 ,
/* - End    */ 5493, KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  35 ,
/* - PgDn   */ 5494, KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  34 ,
/* - Home   */ 5495, KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  36 ,
/* - PgUp   */ 5508, KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  33 ,
/* - Up     */ 5517, KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  38 ,
/* - Down   */ 5521, KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  40 ,
/* - Insert */ 5522, KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  45 ,
/* - Delete */ 5523, KEY_SCAN | KEY_CTRL | KEY_ENHANCED |  46 ,

/* Ctrl-Shift-Gray */
/* - Left   */ 6003, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  37 ,
/* - Right  */ 6004, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  39 ,
/* - End    */ 6005, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  35 ,
/* - PgDn   */ 6006, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  34 ,
/* - Home   */ 6007, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  36 ,
/* - PgUp   */ 6020, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  33 ,
/* - Up     */ 6029, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  38 ,
/* - Down   */ 6033, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  40 ,
/* - Insert */ 6034, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  45 ,
/* - Delete */ 6035, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ENHANCED |  46 ,

/* Alt-Gray */
/* - Home   */ 2455, KEY_SCAN | KEY_ENHANCED | KEY_ALT | 36 ,
/* - Up     */ 2456, KEY_SCAN | KEY_ENHANCED | KEY_ALT | 38 ,
/* - PgUp   */ 2457, KEY_SCAN | KEY_ENHANCED | KEY_ALT | 33 ,
/* - Left   */ 2459, KEY_SCAN | KEY_ENHANCED | KEY_ALT | 37 ,
/* - Right  */ 2461, KEY_SCAN | KEY_ENHANCED | KEY_ALT | 39 ,
/* - End    */ 2463, KEY_SCAN | KEY_ENHANCED | KEY_ALT | 35 ,
/* - Down   */ 2464, KEY_SCAN | KEY_ENHANCED | KEY_ALT | 40 ,
/* - PgDn   */ 2465, KEY_SCAN | KEY_ENHANCED | KEY_ALT | 34 ,
/* - Insert */ 2466, KEY_SCAN | KEY_ENHANCED | KEY_ALT | 45 ,
/* - Delete */ 2467, KEY_SCAN | KEY_ENHANCED | KEY_ALT | 46 ,

/* Alt-Shift-Gray */
/* - Home   */ 2967, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 36 ,
/* - Up     */ 2968, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 38 ,
/* - PgUp   */ 2969, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 33 ,
/* - Left   */ 2971, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 37 ,
/* - Right  */ 2973, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 39 ,
/* - End    */ 2975, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 35 ,
/* - Down   */ 2976, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 40 ,
/* - PgDn   */ 2977, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 34 ,
/* - Insert */ 2978, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 45 ,
/* - Delete */ 2979, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_ALT | 46 ,

/* Ctrl-Alt-Gray */
/* - Up     */ 3480, KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 38 ,
/* - PgUp   */ 3481, KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 33 ,
/* - Left   */ 3483, KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 37 ,
/* - Right  */ 3485, KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 39 ,
/* - End    */ 3487, KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 35 ,
/* - Down   */ 3488, KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 40 ,
/* - PgDn   */ 3489, KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 34 ,
/* - Insert */ 3490, KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 45 ,
/* - Delete */ 3491, KEY_SCAN | KEY_ENHANCED | KEY_CTRL | KEY_ALT | 46 ,

/* Ctrl-Shift-Alt Gray */
/* - Home   */ 3991, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 36 ,
/* - Up     */ 3992, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 38 ,
/* - PgUp   */ 3993, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 33 ,
/* - Left   */ 3995, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 37 ,
/* - Right  */ 3997, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 39 ,
/* - End    */ 3999, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 35 ,
/* - Down   */ 4000, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 40 ,
/* - PgDn   */ 4001, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 34 ,
/* - Insert */ 4002, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 45 ,
/* - Delete */ 4003, KEY_SCAN | KEY_ENHANCED | KEY_SHIFT | KEY_CTRL | KEY_ALT | 46 ,

/* Windows Keys */
/* Left-Window */       4534,   4443,
/* Right-Window */      4535,   4444,
/* Task List */         4536,   4445,

/* Shift-Windows Keys */
/* Left-Window */       5058,   4955,
/* Right-Window */      5059,   4956,
/* Task List */         5060,   4957,

/* Ctrl-Windows Keys */
/* Left-Window */       5582,   5467,
/* Right-Window */      5583,   5468,
/* Task List */         5584,   5469,

/* Ctrl-Shift-Windows Keys */
/* Left-Window */       6094,   5979,
/* Right-Window */      6095,   5980,
/* Task List */         6096,   5981,

/* Alt-Windows Keys */
/* Left-Window */       6618,   6491,
/* Right-Window */      6619,   6492,
/* Task List */         6620,   6493,

/* Shift-Alt-Windows Keys */
/* Left-Window */       7130,   7003,
/* Right-Window */      7131,   7004,
/* Task List */         7132,   7005,

/* Ctrl-Alt-Windows Keys */
/* Left-Window */       7642,   7515,
/* Right-Window */      7643,   7516,
/* Task List */         7644,   7517,

/* Ctrl-Shift-Alt-Windows Keys */
/* Left-Window */       8154,   8027,
/* Right-Window */      8155,   8028,
/* Task List */         8156,   8029

};
int nmskcon = sizeof(mskcontab)/sizeof(int)/2 ;

int
msktock( int key )
{
    int i ;
    for ( i=0;i<nmskcon;i++ )
        if ( mskcontab[i].msk == key )
            return mskcontab[i].ck ;
    return key < 256 ? key : 0 ;
}

int
cktomsk( int key )
{
    int i ;
    for ( i=0;i<nmskcon;i++ )
        if ( mskcontab[i].ck == key )
            return mskcontab[i].msk ;
    return key < 256 ? key : 0;
}

struct keynode * ttkeymap[TT_MAX+5] ;

static con_event
mkkeyevt( KEY scancode ) {
    con_event evt ;

    if ( IS_KVERB(scancode) ) {
        evt.type = kverb ;
        evt.kverb.id = scancode ;
    }
    else if ( IS_CSI(scancode) ) {
        evt.type = csi ;
        evt.csi.key = scancode ;
    }
    else if ( IS_ESC(scancode) ) {
        evt.type = esc ;
        evt.esc.key = scancode ;
    }
    else {
        evt.type = key ;
        evt.key.scancode = scancode ;
    }
    return evt ;
}

static con_event
mkmacroevt( CHAR * string ) {
    con_event evt ;

    evt.type = macro ;
    evt.macro.string = strdup(string) ;
    return evt ;
}


static con_event
mkliteralevt( CHAR * string ) {
    con_event evt ;

    evt.type = literal ;
    evt.literal.string = strdup(string) ;
    return evt ;
}


int
defbasekm( int tt )
{
    if (
    insertkeymap( tt, KEY_CTRL | 32, mkkeyevt(F_KVERB | K_NULL)) ||     /* Ctrl-Space return NUL */
    insertkeymap( tt, KEY_SCAN | KEY_CTRL | KEY_SHIFT | 50, mkkeyevt(F_KVERB | K_NULL)) ||      /* Ctrl-@ return NUL */
    insertkeymap( tt, KEY_SCAN | KEY_CTRL | 50, mkkeyevt(F_KVERB | K_NULL)) ||    /* Ctrl-2 return NUL */

    insertkeymap( tt, 1471, mkkeyevt(29) ) ||
    insertkeymap( tt, 1846, mkkeyevt(30) ) ||
    insertkeymap( tt, 1981, mkkeyevt(31) ) ||
    insertkeymap( tt, 1469, mkkeyevt(31) ) ||
    insertkeymap( tt, 1983, mkkeyevt(31) ) ||
    insertkeymap( tt, 1334, mkkeyevt(30) ) ||
    insertkeymap( tt, 2011, mkkeyevt(27) ) ||
    insertkeymap( tt, 2012, mkkeyevt(28) ) ||
    insertkeymap( tt, 2013, mkkeyevt(29) ) ||

    insertkeymap( tt, KEY_SCAN | KEY_SHIFT | 9, mkkeyevt(8)) || /* Shift-Tab return Backtab return Backspace */
    insertkeymap( tt, KEY_SHIFT | 27, mkkeyevt(27)) ||    /* Shift-Escape return Escape */

    insertkeymap( tt, KEY_SCAN | 13, mkkeyevt(CR) ) ||               /* Enter sends CR */
    insertkeymap( tt, KEY_SCAN | 8, mkkeyevt(DEL) ) ||               /* Backspace sends DEL */
    insertkeymap( tt, KEY_SCAN | KEY_SHIFT | 8, mkkeyevt(DEL) ) ||   /* Shift-Backspace sends DEL */
    insertkeymap( tt, KEY_SCAN | KEY_ALT | 8, mkkeyevt(BS)) ||        /* Alt-Backspace return Backspace */
    insertkeymap( tt, KEY_SCAN | KEY_CTRL | 46, mkkeyevt(BS)) ||    /* Ctrl-Backspace return DEL for Kai Uwe */
    insertkeymap( tt, KEY_SCAN | 46,  mkkeyevt(DEL)) ||         /* Keypad Del sends DEL */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 46, mkkeyevt(DEL)) ||       /* Gray Delete send DEL */
    insertkeymap( tt, KEY_SCAN | KEY_SHIFT | KEY_ENHANCED | 46, mkkeyevt(DEL)) || /* Shift-Gray Delete return DEL */
    insertkeymap( tt, KEY_SCAN | KEY_CTRL | KEY_ENHANCED | 46, mkkeyevt(BS)) || /* Ctrl-Gray Delete return BS */

/* Arrow keys */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 38, mkkeyevt(F_KVERB | K_UPARR)) ||     /* Up Arrow    Gray */
    insertkeymap( tt, KEY_SCAN | 38               , mkkeyevt(F_KVERB | K_UPARR)) ||     /* Up Arrow    Numeric */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 37, mkkeyevt(F_KVERB | K_LFARR)) ||     /* Left Arrow  Gray */
    insertkeymap( tt, KEY_SCAN | 37               , mkkeyevt(F_KVERB | K_LFARR)) ||     /* Left Arrow  Numeric*/
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 39, mkkeyevt(F_KVERB | K_RTARR)) ||     /* Right Arrow Gray */
    insertkeymap( tt, KEY_SCAN | 39               , mkkeyevt(F_KVERB | K_RTARR)) ||     /* Right Arrow Numeric */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 40, mkkeyevt(F_KVERB | K_DNARR)) ||     /* Down Arrow  Gray */
    insertkeymap( tt, KEY_SCAN | 40               , mkkeyevt(F_KVERB | K_DNARR)) ||     /* Down Arrow  Numeric */

    /* Prep the keypad key values */
    insertkeymap( tt, 4365, mkkeyevt(CR) ) || /* Gray-Enter sends CR */
    insertkeymap( tt, 363, mkkeyevt('+') ) || /* Add sends '+' */
    insertkeymap( tt, 365, mkkeyevt('-') ) || /* Subtract sends '-' */
    insertkeymap( tt, 362, mkkeyevt('*') ) || /* Multiply sends '*' */
    insertkeymap( tt, 364, mkkeyevt('/') ) || /* Divide sends '/' */
    insertkeymap( tt, 4543, mkkeyevt('/')) || /* 95 Divide sends '/' */
    insertkeymap( tt, 4463, mkkeyevt('/')) || /* NT Divide sends '/' */
    insertkeymap( tt, KEY_SCAN | 352, mkkeyevt('0') ) || /* 0 sends '0' */
    insertkeymap( tt, KEY_SCAN | 366, mkkeyevt('.') ) || /* Decimal sends '.' */
    insertkeymap( tt, KEY_SCAN | 353, mkkeyevt('1') ) || /* 1 sends '1' */
    insertkeymap( tt, KEY_SCAN | 354, mkkeyevt('2') ) || /* 2 sends '2' */
    insertkeymap( tt, KEY_SCAN | 355, mkkeyevt('3') ) || /* 3 sends '3' */
    insertkeymap( tt, KEY_SCAN | 356, mkkeyevt('4') ) || /* 4 sends '4' */
    insertkeymap( tt, KEY_SCAN | 357, mkkeyevt('5') ) || /* 5 sends '5' */
    insertkeymap( tt, KEY_SCAN | 358, mkkeyevt('6') ) || /* 6 sends '6' */
    insertkeymap( tt, KEY_SCAN | 359, mkkeyevt('7') ) || /* 7 sends '7' */
    insertkeymap( tt, KEY_SCAN | 360, mkkeyevt('8') ) || /* 8 sends '8' */
    insertkeymap( tt, KEY_SCAN | 361, mkkeyevt('9') ) || /* 9 sends '9' */

    insertkeymap( tt, KEY_SCAN | KEY_CTRL | KEY_SHIFT | 13, mkkeyevt(F_KVERB | K_KEYCLICK) ) ||
    insertkeymap( tt, KEY_SCAN | KEY_CTRL | KEY_SHIFT | KEY_ALT | 'D', mkkeyevt(F_KVERB | K_LOGDEBUG) ) ||

        /* Screen scroll */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 33, mkkeyevt(F_KVERB | K_UPSCN)) ||               /* Page Up:        Up one screen */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | KEY_ALT | KEY_SHIFT | 33, mkkeyevt(F_KVERB | K_UPSCN)) ||  /* Page Up:        Up one screen */
/*    insertkeymap( tt, KEY_SCAN | 33, mkkeyevt(F_KVERB | K_UPSCN)) ||                            /* PgUp:           Up one screen */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | KEY_CTRL | 33, mkkeyevt(F_KVERB | K_UPONE)) ||  /* Ctrl Page Up:   Up one line */
/*    insertkeymap( tt, KEY_SCAN | KEY_CTRL | 33, mkkeyevt(F_KVERB | K_UPONE)) ||                     /* Ctrl PgUp:      Up one line */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 34, mkkeyevt(F_KVERB | K_DNSCN)) ||     /* Page Down:      Down one screen */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | KEY_ALT | KEY_SHIFT | 34, mkkeyevt(F_KVERB | K_DNSCN)) ||       /* Page Down:      Down one screen */
    insertkeymap( tt, KEY_SCAN | 34, mkkeyevt(F_KVERB | K_DNSCN)) ||    /* PgDn:           Down one screen */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | KEY_CTRL | 34, mkkeyevt(F_KVERB | K_DNONE)) ||  /* Ctrl Page Down: Down one line */
/*    insertkeymap( tt, KEY_SCAN | KEY_CTRL | 34, mkkeyevt(F_KVERB | K_DNONE)) ||       /* Ctrl PgDn:      Down one line */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 36, mkkeyevt(F_KVERB | K_HOMSCN)) ||    /* Gray Home:      Top of buffer */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | KEY_ALT | KEY_SHIFT | 36, mkkeyevt(F_KVERB | K_HOMSCN)) ||      /* Gray Home:      Top of buffer */
    insertkeymap( tt, KEY_SCAN | 36, mkkeyevt(F_KVERB | K_HOMSCN)) ||   /* Numeric Home:   Top of buffer */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 35, mkkeyevt(F_KVERB | K_ENDSCN)) ||    /* Gray End:       Bottom of buffer */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | KEY_ALT | KEY_SHIFT | 35, mkkeyevt(F_KVERB | K_ENDSCN)) ||      /* Gray End:       Bottom of buffer */
    insertkeymap( tt, KEY_SCAN | 35, mkkeyevt(F_KVERB | K_ENDSCN)) ||   /* Numeric End:    Bottom of buffer */

    /* Bookmarks, Goto, and Searching */
    insertkeymap( tt, KEY_ALT | KEY_SCAN | KEY_ENHANCED | 38, mkkeyevt(F_KVERB | K_GO_BOOK)) || /* Alt- Up Arrow    Gray */
    insertkeymap( tt, KEY_CTRL | KEY_SCAN | KEY_ENHANCED | 38, mkkeyevt(F_KVERB | K_GOTO)) ||       /* Ctrl- Up Arrow    Gray */
    insertkeymap( tt, KEY_ALT | KEY_SCAN | KEY_ENHANCED | 40, mkkeyevt(F_KVERB | K_SET_BOOK)) ||        /* Alt- Down Arrow  Gray */

    insertkeymap( tt, KEY_ALT  | KEY_SCAN | KEY_ENHANCED | 37, mkkeyevt(F_KVERB | K_BACKSRCH)) ||       /* Alt- Left Arrow  Gray */
    insertkeymap( tt, KEY_CTRL | KEY_SCAN | KEY_ENHANCED | 37, mkkeyevt(F_KVERB | K_BACKNEXT)) ||       /* Ctrl-Left Arrow  Gray */
    insertkeymap( tt, KEY_ALT  | KEY_SCAN | KEY_ENHANCED | 39, mkkeyevt(F_KVERB | K_FWDSRCH)) ||        /* Alt- Right Arrow Gray */
    insertkeymap( tt, KEY_CTRL | KEY_SCAN | KEY_ENHANCED | 39, mkkeyevt(F_KVERB | K_FWDNEXT)) ||        /* Ctrl-Right Arrow Gray */

    /* Horizontal Scrolling */
    insertkeymap( tt, KEY_SHIFT | KEY_SCAN | KEY_ENHANCED | 37, mkkeyevt(F_KVERB | K_LFONE)) || /* Shift-Left Arrow    Gray */
    insertkeymap( tt, KEY_ALT | KEY_SHIFT | KEY_SCAN | KEY_ENHANCED | 37, mkkeyevt(F_KVERB | K_LFPAGE)) ||          /* Alt-Shift-Left Arrow    Gray */
    insertkeymap( tt, KEY_ALT | KEY_CTRL | KEY_SHIFT | KEY_SCAN | KEY_ENHANCED | 37, mkkeyevt(F_KVERB | K_LFALL)) ||    /* Ctrl-Alt-Shift-Left Arrow  Gray */
    insertkeymap( tt, KEY_SHIFT | KEY_SCAN | KEY_ENHANCED | 39, mkkeyevt(F_KVERB | K_RTONE)) || /* Shift-Right Arrow    Gray */
    insertkeymap( tt, KEY_ALT | KEY_SHIFT | KEY_SCAN | KEY_ENHANCED | 39, mkkeyevt(F_KVERB | K_RTPAGE)) ||          /* Alt-Shift-Right Arrow    Gray */
    insertkeymap( tt, KEY_ALT | KEY_CTRL | KEY_SHIFT | KEY_SCAN | KEY_ENHANCED | 39, mkkeyevt(F_KVERB | K_RTALL)) ||    /* Ctrl-Alt-Shift-Right Arrow  Gray */

    /* The compose key */
    insertkeymap( tt, KEY_ALT | 67, mkkeyevt(F_KVERB | K_COMPOSE)) ||   /* Alt-C, Compose */
    insertkeymap( tt, KEY_ALT | 99, mkkeyevt(F_KVERB | K_COMPOSE)) ||   /* Alt-c, Compose */

    insertkeymap( tt, KEY_ALT | 78, mkkeyevt(F_KVERB | K_C_UNI16)) ||   /* Alt-N, Compose */
    insertkeymap( tt, KEY_ALT | 110, mkkeyevt(F_KVERB | K_C_UNI16)) ||  /* Alt-n, Compose */

    /* Kermit action verbs */
    /* Default mappings designed so key names are valid on all systems */
    /* Upper insertkeymap( tt, versions */
    insertkeymap( tt, KEY_ALT | 66, mkkeyevt(F_KVERB | K_BREAK)) ||     /* Alt-B, Send BREAK */
    insertkeymap( tt, KEY_ALT | 68, mkkeyevt(F_KVERB | K_DEBUG)) ||     /* Alt-D, Toggle debugging */
    insertkeymap( tt, KEY_ALT | 70, mkkeyevt(F_KVERB | K_FNKEYS)) ||    /* Alt-F, Display Fn Key labels */
    insertkeymap( tt, KEY_ALT | 72, mkkeyevt(F_KVERB | K_HELP)) ||      /* Alt-H, Help */
    insertkeymap( tt, KEY_ALT | 75, mkkeyevt(F_KVERB | K_KEYCLICK)) ||  /* Alt-K, Toggle Keyclick */
    insertkeymap( tt, KEY_ALT | 76, mkkeyevt(F_KVERB | K_LBREAK)) ||    /* Alt-L, Send Long BREAK */
    insertkeymap( tt, KEY_ALT | 79, mkkeyevt(F_KVERB | K_PRTAUTO)) ||   /* Alt-O, Toggle printing */
    insertkeymap( tt, KEY_ALT | 80, mkkeyevt(F_KVERB | K_DUMP)) ||      /* Alt-P, Print/Dump screen */
    insertkeymap( tt, KEY_ALT | 81, mkkeyevt(F_KVERB | K_QUIT)) ||      /* Alt-Q, Quit Kermit */
    insertkeymap( tt, KEY_ALT | 82, mkkeyevt(F_KVERB | K_RESET)) ||     /* Alt-R, Reset emulator */
    insertkeymap( tt, KEY_ALT | 83, mkkeyevt(F_KVERB | K_STATUS)) ||    /* Alt-S, Toggle Status Line */
    insertkeymap( tt, KEY_ALT | 84, mkkeyevt(F_KVERB | K_TERMTYPE)) ||  /* Alt-T, Toggle Terminal Type */
    insertkeymap( tt, KEY_ALT | 85, mkkeyevt(F_KVERB | K_HANGUP)) ||    /* Alt-U, Hangup connection */
    insertkeymap( tt, KEY_ALT | 86, mkkeyevt(F_KVERB | K_FLIPSCN)) ||   /* Alt-V, video toggle */
    insertkeymap( tt, KEY_ALT | 88, mkkeyevt(F_KVERB | K_EXIT)) ||              /* Alt-X, Exit emulator */

    /* Lower insertkeymap( tt, versions */
    insertkeymap( tt, KEY_ALT | 98, mkkeyevt(F_KVERB | K_BREAK)) ||     /* Alt-b, Send BREAK */
    insertkeymap( tt, KEY_ALT | 100, mkkeyevt(F_KVERB | K_DEBUG)) ||    /* Alt-d, Toggle debugging */
    insertkeymap( tt, KEY_ALT | 102, mkkeyevt(F_KVERB | K_FNKEYS)) ||   /* Alt-f, Display Fn Key labels */
    insertkeymap( tt, KEY_ALT | 104, mkkeyevt(F_KVERB | K_HELP)) ||        /* Alt-h, Help */
    insertkeymap( tt, KEY_ALT | 107, mkkeyevt(F_KVERB | K_KEYCLICK)) ||    /* Alt-k, Toggle Keyclick */
    insertkeymap( tt, KEY_ALT | 108, mkkeyevt(F_KVERB | K_LBREAK)) ||   /* Alt-l, Send Long BREAK */
    insertkeymap( tt, KEY_ALT | 111, mkkeyevt(F_KVERB | K_PRTAUTO)) ||  /* Alt-o, Toggle printing */
    insertkeymap( tt, KEY_ALT | 112, mkkeyevt(F_KVERB | K_DUMP)) ||     /* Alt-p, Print/Dump screen */
    insertkeymap( tt, KEY_ALT | 113, mkkeyevt(F_KVERB | K_QUIT)) ||     /* Alt-q, Quit Kermit */
    insertkeymap( tt, KEY_ALT | 114, mkkeyevt(F_KVERB | K_RESET)) ||    /* Alt-r, Reset emulator */
    insertkeymap( tt, KEY_ALT | 115, mkkeyevt(F_KVERB | K_STATUS)) ||      /* Alt-s, Toggle Status Line */
    insertkeymap( tt, KEY_ALT | 116, mkkeyevt(F_KVERB | K_TERMTYPE)) ||    /* Alt-t, Toggle Terminal Type */
    insertkeymap( tt, KEY_ALT | 117, mkkeyevt(F_KVERB | K_HANGUP)) ||   /* Alt-u, Hangup connection */
    insertkeymap( tt, KEY_ALT | 118, mkkeyevt(F_KVERB | K_FLIPSCN)) ||  /* Alt-v, video toggle */
    insertkeymap( tt, KEY_ALT | 120, mkkeyevt(F_KVERB | K_EXIT)) ||     /* Alt-x, Exit emulator */

    insertkeymap( tt, KEY_ALT | 61, mkkeyevt(F_KVERB | K_RESET)) ||     /* Alt-return, Reset emulator */
    insertkeymap( tt, KEY_SCAN | KEY_ALT | KEY_ENHANCED | 35, mkkeyevt(F_KVERB | K_PRINTFF)) || /* Alt-End, Print FF */
    insertkeymap( tt, KEY_SCAN | KEY_ALT | KEY_ENHANCED | 34, mkkeyevt(F_KVERB | K_DOS)) ||         /* Alt-Page Down, Push to OS/2 */
    insertkeymap( tt, KEY_SCAN | 145, mkkeyevt(F_KVERB | K_HOLDSCRN)) || /* ScrollLock, Hold Screen */
    insertkeymap( tt, KEY_SCAN | 20, mkkeyevt(F_KVERB | K_IGNORE)) ||   /* CapsLock, don't beep */

    insertkeymap( tt, KEY_ALT | 'E', mkkeyevt(F_KVERB | K_PRINTFF)) ||  /* Alt-E, Print FF */
    insertkeymap( tt, KEY_ALT | 'e', mkkeyevt(F_KVERB | K_PRINTFF)) ||  /* Alt-e, Print FF */
    insertkeymap( tt, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'E', mkkeyevt(F_KVERB | K_KB_EMA)) ||
    insertkeymap( tt, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'H', mkkeyevt(F_KVERB | K_KB_HEB )) ||
    insertkeymap( tt, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'N', mkkeyevt(F_KVERB | K_KB_ENG )) ||
    insertkeymap( tt, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'R', mkkeyevt(F_KVERB | K_KB_RUS )) ||
    insertkeymap( tt, KEY_SCAN | KEY_SHIFT | KEY_CTRL | KEY_ALT | 'W', mkkeyevt(F_KVERB | K_KB_WP )) ||

    /* Num-Lock */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 144, mkkeyevt(F_KVERB | K_IGNORE)) ||   /* WinNT */
    insertkeymap( tt, KEY_SCAN | 144, mkkeyevt(F_KVERB | K_IGNORE)) ||  /* Win95 */
    insertkeymap( tt, KEY_SCAN | KEY_SHIFT | KEY_ENHANCED | 144, mkkeyevt(F_KVERB | K_IGNORE)) ||       /* WinNT ThinkPad */
    insertkeymap( tt, KEY_SCAN | KEY_SHIFT | 144, mkkeyevt(F_KVERB | K_IGNORE)) ||      /* Win95 Thinkpad */

    /* Workaround the Win95 Shift-Gray-Multiply bug for Shift-Printscreen */
    insertkeymap( tt, KEY_SCAN | KEY_SHIFT | KEY_ENHANCED | 106, mkkeyevt(F_KVERB | K_IGNORE)) ||
    insertkeymap( tt, 4458, mkkeyevt(F_KVERB | K_DUMP)) ||          /* PrtScrn (Gray-Multiply) if not captured */

    /* These are seen in KUI only */
    insertkeymap( tt, 16, mkkeyevt(F_KVERB | K_IGNORE)) ||     /* Left & Right Shift */
    insertkeymap( tt, 17, mkkeyevt(F_KVERB | K_IGNORE)) ||     /* Left Ctrl */
    insertkeymap( tt, 18, mkkeyevt(F_KVERB | K_IGNORE)) ||     /* Left Alt */
    insertkeymap( tt, 4443, mkkeyevt(F_KVERB | K_IGNORE)) ||   /* Left MSWindows */
    insertkeymap( tt, 4444, mkkeyevt(F_KVERB | K_IGNORE)) ||   /* Right MSWindows */
    insertkeymap( tt, 4445, mkkeyevt(F_KVERB | K_IGNORE)) ||   /* TaskList */
    insertkeymap( tt, 4516, mkkeyevt(F_KVERB | K_IGNORE)) ||   /* Right Ctrl */
    insertkeymap( tt, 4514, mkkeyevt(F_KVERB | K_IGNORE)) ||   /* Right Alt */

    /* Mark Mode action verbs */
    insertkeymap( tt, 4909, mkkeyevt(F_KVERB | K_PASTE)) ||         /* Shift-Insert */
    insertkeymap( tt, 5421, mkkeyevt(F_KVERB | K_MARK_COPYCLIP)) || /* Ctrl-Insert  */
    insertkeymap( tt, 5933, mkkeyevt(F_KVERB | K_MARK_COPYHOST)) || /* Ctrl-Shift-Insert */
    insertkeymap( tt, 1392, mkkeyevt(F_KVERB | K_MARK_START)) ||    /* Ctrl-F1      */
    insertkeymap( tt, 1393, mkkeyevt(F_KVERB | K_MARK_CANCEL))      /* Ctrl-F2      */

    )
        return -1;
    return 0;
}

int
defwpkm( int tt )
{
    /* MS-DOS Kermit Defaults which are expected by Word Perfect */
    insertkeymap( tt, 372, mkkeyevt(F_KVERB | K_KP7));      /* F5   */
    insertkeymap( tt, 373, mkkeyevt(F_KVERB | K_KP8));      /* F6   */
    insertkeymap( tt, 374, mkkeyevt(F_KVERB | K_KP9));      /* F7   */
    insertkeymap( tt, 375, mkkeyevt(F_KVERB | K_KPMINUS));  /* F8   */
    insertkeymap( tt, 376, mkkeyevt(F_KVERB | K_KP4));      /* F9   */
    insertkeymap( tt, 377, mkkeyevt(F_KVERB | K_KP5));      /* F10  */
    insertkeymap( tt, 378, mkliteralevt("\033~A"));         /* F11  */
    insertkeymap( tt, 379, mkliteralevt("\033~B"));         /* F12  */
    insertkeymap( tt, 1293, mkliteralevt("\033)\015"));     /* Ctrl-Enter */

    /* The following keymaps were provided by Pete Hickey */
    insertkeymap( tt, 353, mkkeyevt('1'));
    insertkeymap( tt, 354, mkkeyevt('2'));
    insertkeymap( tt, 355, mkkeyevt('3'));
    insertkeymap( tt, 356, mkkeyevt('4'));
    insertkeymap( tt, 357, mkkeyevt('5'));
    insertkeymap( tt, 358, mkkeyevt('6'));
    insertkeymap( tt, 359, mkkeyevt('7'));
    insertkeymap( tt, 360, mkkeyevt('8'));
    insertkeymap( tt, 361, mkkeyevt('9'));
    insertkeymap( tt, 352, mkkeyevt('0'));
    insertkeymap( tt, 366, mkkeyevt('.'));

    insertkeymap( tt, 291, mkliteralevt("\033$4"));
    insertkeymap( tt, 296, mkkeyevt(F_KVERB | K_DNARR));
    insertkeymap( tt, 290, mkliteralevt("\033$5"));
    insertkeymap( tt, 293, mkkeyevt(F_KVERB | K_LFARR));
    insertkeymap( tt, 268, mkkeyevt(ESC));
    insertkeymap( tt, 295, mkkeyevt(F_KVERB | K_RTARR));
    insertkeymap( tt, 292, mkliteralevt("\033$1"));
    insertkeymap( tt, 294, mkkeyevt(F_KVERB | K_UPARR));
    insertkeymap( tt, 289, mkliteralevt("\033$2"));
    insertkeymap( tt, 301, mkliteralevt("\033$0"));
    insertkeymap( tt, 302, mkliteralevt("\033$3"));

    insertkeymap( tt, 4543, mkkeyevt('/'));
    insertkeymap( tt, 362, mkkeyevt('*'));
    insertkeymap( tt, 365, mkkeyevt('-'));
    insertkeymap( tt, 363, mkkeyevt('+'));
    insertkeymap( tt, 4365, mkkeyevt(CR));
    /* End of Pete Hickey additions */


    /* The following come from the old WPUNIX.INI MS-DOS Kermit file */
    insertkeymap( tt, 880, mkkeyevt(F_KVERB | K_KP6));
    insertkeymap( tt, 881, mkkeyevt(F_KVERB | K_KPCOMA));
    insertkeymap( tt, 882, mkkeyevt(F_KVERB | K_KP1));
    insertkeymap( tt, 883, mkkeyevt(F_KVERB | K_KP2));
    insertkeymap( tt, 884, mkkeyevt(F_KVERB | K_KP3));
    insertkeymap( tt, 885, mkkeyevt(F_KVERB | K_KPENTER));
    insertkeymap( tt, 886, mkkeyevt(F_KVERB | K_KP0));
    insertkeymap( tt, 887, mkkeyevt(F_KVERB | K_KPDOT));
    insertkeymap( tt, 888, mkliteralevt("\033!8"));
    insertkeymap( tt, 889, mkliteralevt("\033!9"));

    insertkeymap( tt, 1028, mkkeyevt(EOT));
    insertkeymap( tt, 1032, mkliteralevt("\033)h"));
    insertkeymap( tt, 1033, mkliteralevt("\033)i"));
    insertkeymap( tt, 1034, mkliteralevt("\033)\015"));
    insertkeymap( tt, 1037, mkliteralevt("\033)m"));
    insertkeymap( tt, 1041, mkliteralevt("\033)q"));
    insertkeymap( tt, 1043, mkliteralevt("\033)s"));
    insertkeymap( tt, 1151, mkliteralevt("\033)\008"));
    insertkeymap( tt, 1313, mkliteralevt("\033^2"));
    insertkeymap( tt, 1314, mkliteralevt("\033^5"));
    insertkeymap( tt, 1315, mkliteralevt("\033^4"));
    insertkeymap( tt, 1316, mkliteralevt("\033^1"));
    insertkeymap( tt, 1317, mkliteralevt("\033^L"));
    insertkeymap( tt, 1318, mkliteralevt("\033^U"));
    insertkeymap( tt, 1319, mkliteralevt("\033^R"));
    insertkeymap( tt, 1320, mkliteralevt("\033^D"));
    insertkeymap( tt, 1325, mkliteralevt("\033^0"));

    insertkeymap( tt, 1392, mkliteralevt("\033@0"));
    insertkeymap( tt, 1393, mkliteralevt("\033@1"));
    insertkeymap( tt, 1394, mkliteralevt("\033@2"));
    insertkeymap( tt, 1395, mkliteralevt("\033@3"));
    insertkeymap( tt, 1396, mkliteralevt("\033@4"));
    insertkeymap( tt, 1397, mkliteralevt("\033@5"));
    insertkeymap( tt, 1398, mkliteralevt("\033@6"));
    insertkeymap( tt, 1399, mkliteralevt("\033@7"));
    insertkeymap( tt, 1400, mkliteralevt("\033@8"));
    insertkeymap( tt, 1401, mkliteralevt("\033@9"));

    insertkeymap( tt, 2094, mkkeyevt(F_KVERB | K_EXIT));
    insertkeymap( tt, 2096, mkliteralevt("\033|9"));
    insertkeymap( tt, 2097, mkliteralevt("\033|0"));
    insertkeymap( tt, 2098, mkliteralevt("\033|1"));
    insertkeymap( tt, 2099, mkliteralevt("\033|2"));
    insertkeymap( tt, 2100, mkliteralevt("\033|3"));
    insertkeymap( tt, 2101, mkliteralevt("\033|4"));
    insertkeymap( tt, 2102, mkliteralevt("\033|5"));
    insertkeymap( tt, 2103, mkliteralevt("\033|6"));
    insertkeymap( tt, 2104, mkliteralevt("\033|7"));
    insertkeymap( tt, 2105, mkliteralevt("\033|8"));
    insertkeymap( tt, 2107, mkkeyevt(F_KVERB | K_DUMP));
    insertkeymap( tt, 2109, mkliteralevt("\033|="));
    insertkeymap( tt, 2145, mkliteralevt("\033|a"));
    insertkeymap( tt, 2146, mkliteralevt("\033|b"));
    insertkeymap( tt, 2147, mkliteralevt("\033|c"));
    insertkeymap( tt, 2148, mkliteralevt("\033|d"));
    insertkeymap( tt, 2149, mkliteralevt("\033|e"));
    insertkeymap( tt, 2150, mkliteralevt("\033|f"));
    insertkeymap( tt, 2151, mkliteralevt("\033|g"));
    insertkeymap( tt, 2152, mkliteralevt("\033|h"));
    insertkeymap( tt, 2153, mkliteralevt("\033|i"));
    insertkeymap( tt, 2154, mkliteralevt("\033|j"));
    insertkeymap( tt, 2155, mkliteralevt("\033|k"));
    insertkeymap( tt, 2156, mkliteralevt("\033|l"));
    insertkeymap( tt, 2157, mkliteralevt("\033|m"));
    insertkeymap( tt, 2158, mkliteralevt("\033|n"));
    insertkeymap( tt, 2159, mkliteralevt("\033|o"));
    insertkeymap( tt, 2160, mkliteralevt("\033|p"));
    insertkeymap( tt, 2161, mkliteralevt("\033|q"));
    insertkeymap( tt, 2162, mkliteralevt("\033|r"));
    insertkeymap( tt, 2163, mkliteralevt("\033|s"));
    insertkeymap( tt, 2164, mkliteralevt("\033|t"));
    insertkeymap( tt, 2165, mkliteralevt("\033|u"));
    insertkeymap( tt, 2166, mkliteralevt("\033|v"));
    insertkeymap( tt, 2167, mkliteralevt("\033|w"));
    insertkeymap( tt, 2168, mkliteralevt("\033|x"));
    insertkeymap( tt, 2169, mkliteralevt("\033|y"));
    insertkeymap( tt, 2170, mkliteralevt("\033|z"));

    insertkeymap( tt, 2416, mkliteralevt("\033#0"));
    insertkeymap( tt, 2417, mkliteralevt("\033#1"));
    insertkeymap( tt, 2418, mkliteralevt("\033#2"));
    insertkeymap( tt, 2419, mkliteralevt("\033#3"));
    insertkeymap( tt, 2420, mkliteralevt("\033#4"));
    insertkeymap( tt, 2421, mkliteralevt("\033#5"));
    insertkeymap( tt, 2422, mkliteralevt("\033#6"));
    insertkeymap( tt, 2423, mkliteralevt("\033#7"));
    insertkeymap( tt, 2424, mkliteralevt("\033#8"));
    insertkeymap( tt, 2425, mkliteralevt("\033#9"));

    insertkeymap( tt, 4385, mkliteralevt("\033$2"));
    insertkeymap( tt, 4386, mkliteralevt("\033$5"));
    insertkeymap( tt, 4387, mkliteralevt("\033$4"));
    insertkeymap( tt, 4388, mkliteralevt("\033$1"));
    insertkeymap( tt, 4397, mkliteralevt("\033$0"));
    insertkeymap( tt, 4398, mkliteralevt("\033$3"));

    insertkeymap( tt, 4909, mkliteralevt("\033%0"));
    insertkeymap( tt, 4910, mkliteralevt("\033%3"));

    insertkeymap( tt, 5389, mkliteralevt("\033)\015"));
    insertkeymap( tt, 5409, mkliteralevt("\033^2"));
    insertkeymap( tt, 5410, mkliteralevt("\033^5"));
    insertkeymap( tt, 5411, mkliteralevt("\033^4"));
    insertkeymap( tt, 5412, mkliteralevt("\033^1"));
    insertkeymap( tt, 5413, mkliteralevt("\033^L"));
    insertkeymap( tt, 5414, mkliteralevt("\033^U"));
    insertkeymap( tt, 5415, mkliteralevt("\033^R"));
    insertkeymap( tt, 5416, mkliteralevt("\033^D"));
    insertkeymap( tt, 5421, mkliteralevt("\033^0"));

    insertkeymap( tt, 6436, mkliteralevt("\033&1"));
    insertkeymap( tt, 6437, mkliteralevt("\033&L"));
    insertkeymap( tt, 6438, mkliteralevt("\033&U"));
    insertkeymap( tt, 6439, mkliteralevt("\033&R"));
    if(insertkeymap( tt, 6440, mkliteralevt("\033&D")))
        return(-1);
    return(0);
}

int
defemacskm( int tt )
{
    if (
        /* Alt key = Meta key */
        insertkeymap( tt, 2416, mkkeyevt(F_KVERB | K_HELP )) || /* Alt-F1 Help                     */
        insertkeymap( tt, 2419, mkkeyevt(F_KVERB | K_EXIT )) || /* Alt-F4 return to prompt              */
        insertkeymap( tt, 2145, mkkeyevt(F_ESC | 'a'      )) || /* Alt-a sends Meta-a (i.e. ESC a)      */
        insertkeymap( tt, 2146, mkkeyevt(F_ESC | 'b'      )) || /* Alt-b sends Meta-b (ESC b)   */
        insertkeymap( tt, 2147, mkkeyevt(F_ESC | 'c'      )) || /* Alt-c etc etc...             */
        insertkeymap( tt, 2148, mkkeyevt(F_ESC | 'd'      )) || /* Alt-d                                */
        insertkeymap( tt, 2149, mkkeyevt(F_ESC | 'e'      )) || /* Alt-e                                */
        insertkeymap( tt, 2150, mkkeyevt(F_ESC | 'f'      )) || /* Alt-f                                */
        insertkeymap( tt, 2151, mkkeyevt(F_ESC | 'g'      )) || /* Alt-g                                */
        insertkeymap( tt, 2152, mkkeyevt(F_ESC | 'h'      )) || /* Alt-h                                */
        insertkeymap( tt, 2153, mkkeyevt(F_ESC | 'i'      )) || /* Alt-i                                */
        insertkeymap( tt, 2154, mkkeyevt(F_ESC | 'j'      )) || /* Alt-j                                */
        insertkeymap( tt, 2155, mkkeyevt(F_ESC | 'k'      )) || /* Alt-k                                */
        insertkeymap( tt, 2156, mkkeyevt(F_ESC | 'l'      )) || /* Alt-l                                */
        insertkeymap( tt, 2157, mkkeyevt(F_ESC | 'm'      )) || /* Alt-m                                */
        insertkeymap( tt, 2158, mkkeyevt(F_ESC | 'n'      )) || /* Alt-n                                */
        insertkeymap( tt, 2159, mkkeyevt(F_ESC | 'o'      )) || /* Alt-o                                */
        insertkeymap( tt, 2160, mkkeyevt(F_ESC | 'p'      )) || /* Alt-p                                */
        insertkeymap( tt, 2161, mkkeyevt(F_ESC | 'q'      )) || /* Alt-q                                */
        insertkeymap( tt, 2162, mkkeyevt(F_ESC | 'r'      )) || /* Alt-r                                */
        insertkeymap( tt, 2163, mkkeyevt(F_ESC | 's'      )) || /* Alt-s                                */
        insertkeymap( tt, 2164, mkkeyevt(F_ESC | 't'      )) || /* Alt-t                                */
        insertkeymap( tt, 2165, mkkeyevt(F_ESC | 'u'      )) || /* Alt-u                                */
        insertkeymap( tt, 2166, mkkeyevt(F_ESC | 'v'      )) || /* Alt-v                                */
        insertkeymap( tt, 2167, mkkeyevt(F_ESC | 'w'      )) || /* Alt-w                                */
        insertkeymap( tt, 2168, mkkeyevt(F_ESC | 'x'      )) || /* Alt-x                                */
        insertkeymap( tt, 2169, mkkeyevt(F_ESC | 'y'      )) || /* Alt-y                                */
        insertkeymap( tt, 2170, mkkeyevt(F_ESC | 'z'      )) || /* Alt-z                                */

         insertkeymap( tt, 2096, mkkeyevt( F_ESC | '0' )) || /* Alt-0 */
         insertkeymap( tt, 2097, mkkeyevt( F_ESC | '1' )) || /* Alt-1 */
         insertkeymap( tt, 2098, mkkeyevt( F_ESC | '2' )) || /* Alt-2 */
         insertkeymap( tt, 2099, mkkeyevt( F_ESC | '3' )) || /* Alt-3 */
         insertkeymap( tt, 2100, mkkeyevt( F_ESC | '4' )) || /* Alt-4 */
         insertkeymap( tt, 2101, mkkeyevt( F_ESC | '5' )) || /* Alt-5 */
         insertkeymap( tt, 2102, mkkeyevt( F_ESC | '6' )) || /* Alt-6 */
         insertkeymap( tt, 2103, mkkeyevt( F_ESC | '7' )) || /* Alt-7 */
         insertkeymap( tt, 2104, mkkeyevt( F_ESC | '8' )) || /* Alt-8 */
         insertkeymap( tt, 2105, mkkeyevt( F_ESC | '9' )) || /* Alt-9 */

        insertkeymap( tt, 2107, mkkeyevt(F_ESC | 59       )) || /* Alt-) ||                             */
        insertkeymap( tt, 2087, mkkeyevt(F_ESC | 39       )) || /* Alt-'                                */
        insertkeymap( tt, 2139, mkkeyevt(F_ESC | '['      )) || /* Alt-[                                */
        insertkeymap( tt, 2141, mkkeyevt(F_ESC | ']'      )) || /* Alt-]                                */
        insertkeymap( tt, 2140, mkkeyevt(F_ESC | 92       )) || /* Alt-\                                */
        insertkeymap( tt, 2092, mkkeyevt(F_ESC | ','      )) || /* Alt-< (unshifted)            */
        insertkeymap( tt, 2094, mkkeyevt(F_ESC | '.'      )) || /* Alt-> (unshifted)            */
        insertkeymap( tt, 2144, mkkeyevt(F_ESC | '`'      )) || /* Alt-`                        */
        insertkeymap( tt, 2093, mkkeyevt(F_ESC | '-'      )) || /* Alt-- */

        insertkeymap( tt, 2113, mkkeyevt(F_ESC | 'A'      )) || /* Alt-A sends Meta-a (i.e. ESC a)      */
        insertkeymap( tt, 2114, mkkeyevt(F_ESC | 'B'      )) || /* Alt-B sends Meta-b (ESC b)   */
        insertkeymap( tt, 2115, mkkeyevt(F_ESC | 'C'      )) || /* Alt-C etc etc...             */
        insertkeymap( tt, 2116, mkkeyevt(F_ESC | 'D'      )) || /* Alt-D                                */
        insertkeymap( tt, 2117, mkkeyevt(F_ESC | 'E'      )) || /* Alt-E                                */
        insertkeymap( tt, 2118, mkkeyevt(F_ESC | 'F'      )) || /* Alt-F                                */
        insertkeymap( tt, 2119, mkkeyevt(F_ESC | 'G'      )) || /* Alt-G                                */
        insertkeymap( tt, 2120, mkkeyevt(F_ESC | 'H'      )) || /* Alt-H                                */
        insertkeymap( tt, 2121, mkkeyevt(F_ESC | 'I'      )) || /* Alt-I                                */
        insertkeymap( tt, 2122, mkkeyevt(F_ESC | 'J'      )) || /* Alt-J                                */
        insertkeymap( tt, 2123, mkkeyevt(F_ESC | 'K'      )) || /* Alt-K                                */
        insertkeymap( tt, 2124, mkkeyevt(F_ESC | 'L'      )) || /* Alt-L                                */
        insertkeymap( tt, 2125, mkkeyevt(F_ESC | 'M'      )) || /* Alt-M                                */
        insertkeymap( tt, 2126, mkkeyevt(F_ESC | 'N'      )) || /* Alt-N                                */
        insertkeymap( tt, 2127, mkkeyevt(F_ESC | 'O'      )) || /* Alt-O                                */
        insertkeymap( tt, 2128, mkkeyevt(F_ESC | 'P'      )) || /* Alt-P                                */
        insertkeymap( tt, 2129, mkkeyevt(F_ESC | 'Q'      )) || /* Alt-Q                                */
        insertkeymap( tt, 2130, mkkeyevt(F_ESC | 'R'      )) || /* Alt-R                                */
        insertkeymap( tt, 2131, mkkeyevt(F_ESC | 'S'      )) || /* Alt-S                                */
        insertkeymap( tt, 2132, mkkeyevt(F_ESC | 'T'      )) || /* Alt-T                                */
        insertkeymap( tt, 2133, mkkeyevt(F_ESC | 'U'      )) || /* Alt-U                                */
        insertkeymap( tt, 2134, mkkeyevt(F_ESC | 'V'      )) || /* Alt-V                                */
        insertkeymap( tt, 2135, mkkeyevt(F_ESC | 'W'      )) || /* Alt-W                                */
        insertkeymap( tt, 2136, mkkeyevt(F_ESC | 'X'      )) || /* Alt-X                                */
        insertkeymap( tt, 2137, mkkeyevt(F_ESC | 'Y'      )) || /* Alt-Y                                */
        insertkeymap( tt, 2138, mkkeyevt(F_ESC | 'Z'      )) || /* Alt-Z                                */

        insertkeymap( tt, 2082, mkkeyevt(F_ESC | '"'      )) || /* Alt-"                                */
        insertkeymap( tt, 2171, mkkeyevt(F_ESC | '{'     )) || /* Alt-{                         */
        insertkeymap( tt, 2173, mkkeyevt(F_ESC | '}'     )) || /* Alt-}                         */
        insertkeymap( tt, 2172, mkkeyevt(F_ESC | '|'      )) || /* Alt-\                                */
        insertkeymap( tt, 2108, mkkeyevt(F_ESC | '<'      )) || /* Alt-< (shifted)                      */
        insertkeymap( tt, 2110, mkkeyevt(F_ESC | '>'      )) || /* Alt-> (shifted)                      */
        insertkeymap( tt, 2174, mkkeyevt(F_ESC | '~'      )) || /* Alt-~ */
        insertkeymap( tt, 2143, mkkeyevt(F_ESC | '_'      )) || /* Alt-_ */
         insertkeymap( tt, 2106, mkkeyevt(F_ESC | ':'      )) || /* Alt-: */
         insertkeymap( tt, 2107, mkkeyevt(F_ESC | ';'      )) || /* Alt-; */

        insertkeymap( tt, 2081, mkkeyevt(F_ESC | '!'      )) || /* Alt-!                                */
        insertkeymap( tt, 2112, mkkeyevt(F_ESC | '@'      )) || /* Alt-@                                */
        insertkeymap( tt, 2083, mkkeyevt(F_ESC | '#'      )) || /* Alt-#                                */
        insertkeymap( tt, 2084, mkkeyevt(F_ESC | '$'      )) || /* Alt-$                                */
        insertkeymap( tt, 2085, mkkeyevt(F_ESC | '%'      )) || /* Alt-%                                */
        insertkeymap( tt, 2142, mkkeyevt(F_ESC | '^'      )) || /* Alt-^                                */
        insertkeymap( tt, 2086, mkkeyevt(F_ESC | '&'      )) || /* Alt-&                                */
        insertkeymap( tt, 2090, mkkeyevt(F_ESC | '*'      )) || /* Alt-*                                */
        insertkeymap( tt, 2088, mkkeyevt(F_ESC | '('      )) || /* Alt-(                                */
        insertkeymap( tt, 2089, mkkeyevt(F_ESC | ')'      )) || /* Alt-)                                */
         insertkeymap( tt, 2091, mkkeyevt(F_ESC | '+'      )) || /* Alt-+                               */
         insertkeymap( tt, 2095, mkkeyevt(F_ESC | '/'      )) || /* Alt-/                               */
         insertkeymap( tt, 2109, mkkeyevt(F_ESC | '='      )) || /* Alt-=                               */
         insertkeymap( tt, 2111, mkkeyevt(F_ESC | '?'      )) || /* Alt-?                               */
         insertkeymap( tt, 2350, mkkeyevt(F_ESC | DEL      )) || /* Alt-DEL sends Meta-DEL */

            /* Ctrl-Alt = Ctrl-Meta */


        insertkeymap( tt, 4030, mkkeyevt(F_ESC | SP    )) || /* Ctrl-Alt-SP sends Meta-SP */
        insertkeymap( tt, 3393, mkkeyevt(F_ESC | 1     )) || /* Ctrl-Alt-a sends Meta-Ctrl-a (i.e. Esc ^A) */
        insertkeymap( tt, 3394, mkkeyevt(F_ESC | 2     )) || /* Ctrl-Alt-b sends Meta-Ctrl-b (Esc ^B)   */
        insertkeymap( tt, 3395, mkkeyevt(F_ESC | 3     )) || /* Ctrl-Alt-c etc etc...                   */
        insertkeymap( tt, 3396, mkkeyevt(F_ESC | 4     )) || /* Ctrl-Alt-d                                      */
        insertkeymap( tt, 3397, mkkeyevt(F_ESC | 5     )) || /* Ctrl-Alt-e                                      */
        insertkeymap( tt, 3398, mkkeyevt(F_ESC | 6     )) || /* Ctrl-Alt-f                                      */
        insertkeymap( tt, 3399, mkkeyevt(F_ESC | 7     )) || /* Ctrl-Alt-g                                      */
        insertkeymap( tt, 3400, mkkeyevt(F_ESC | 8     )) || /* Ctrl-Alt-h                                      */
        insertkeymap( tt, 3401, mkkeyevt(F_ESC | 9     )) || /* Ctrl-Alt-i                                      */
        insertkeymap( tt, 3402, mkkeyevt(F_ESC | 10    )) || /* Ctrl-Alt-j                                      */
        insertkeymap( tt, 3403, mkkeyevt(F_ESC | 11    )) || /* Ctrl-Alt-k                                      */
        insertkeymap( tt, 3404, mkkeyevt(F_ESC | 12    )) || /* Ctrl-Alt-l                                      */
        insertkeymap( tt, 3405, mkkeyevt(F_ESC | 13    )) || /* Ctrl-Alt-m                                      */
        insertkeymap( tt, 3406, mkkeyevt(F_ESC | 14    )) || /* Ctrl-Alt-n                                      */
        insertkeymap( tt, 3407, mkkeyevt(F_ESC | 15    )) || /* Ctrl-Alt-o                                      */
        insertkeymap( tt, 3408, mkkeyevt(F_ESC | 16    )) || /* Ctrl-Alt-p                                      */
        insertkeymap( tt, 3409, mkkeyevt(F_ESC | 17    )) || /* Ctrl-Alt-q                                      */
        insertkeymap( tt, 3410, mkkeyevt(F_ESC | 18    )) || /* Ctrl-Alt-r                                      */
        insertkeymap( tt, 3411, mkkeyevt(F_ESC | 19    )) || /* Ctrl-Alt-s                                      */
        insertkeymap( tt, 3412, mkkeyevt(F_ESC | 20    )) || /* Ctrl-Alt-t                                      */
        insertkeymap( tt, 3413, mkkeyevt(F_ESC | 21    )) || /* Ctrl-Alt-u                                      */
        insertkeymap( tt, 3414, mkkeyevt(F_ESC | 22    )) || /* Ctrl-Alt-v                                      */
        insertkeymap( tt, 3415, mkkeyevt(F_ESC | 23    )) || /* Ctrl-Alt-w                                      */
        insertkeymap( tt, 3416, mkkeyevt(F_ESC | 24    )) || /* Ctrl-Alt-x                                      */
        insertkeymap( tt, 3417, mkkeyevt(F_ESC | 25    )) || /* Ctrl-Alt-y                                      */
        insertkeymap( tt, 3418, mkkeyevt(F_ESC | 26    )) || /* Ctrl-Alt-z                                      */
        insertkeymap( tt, 3547, mkkeyevt(F_ESC | 27    )) || /* Ctrl-Alt-[                                      */
        insertkeymap( tt, 3548, mkkeyevt(F_ESC | 28    )) || /* Ctrl-Alt-\                                      */
        insertkeymap( tt, 3549, mkkeyevt(F_ESC | 29    )) || /* Ctrl-Alt-]                                      */
        insertkeymap( tt, 3894, mkkeyevt(F_ESC | 30    )) || /* Ctrl-Alt-^                                      */
        insertkeymap( tt, 4029, mkkeyevt(F_ESC | 31    )) || /* Ctrl-Alt-_                                      */

        insertkeymap( tt, 2312, mkkeyevt(F_ESC | BS    )) || /* Alt-Backspace */
         insertkeymap( tt, 2609, mkkeyevt(F_ESC | '!'   )) || /* Alt-! */
         insertkeymap( tt, 2610, mkkeyevt(F_ESC | '@'   )) || /* Alt-@ */
         insertkeymap( tt, 2611, mkkeyevt(F_ESC | '#'   )) || /* Alt-# */
         insertkeymap( tt, 2612, mkkeyevt(F_ESC | '$'   )) || /* Alt-$ */
         insertkeymap( tt, 2613, mkkeyevt(F_ESC | '%'   )) || /* Alt-% */
         insertkeymap( tt, 2614, mkkeyevt(F_ESC | '^'   )) || /* Alt-^ */
         insertkeymap( tt, 2615, mkkeyevt(F_ESC | '&'   )) || /* Alt-& */
         insertkeymap( tt, 2616, mkkeyevt(F_ESC | '*'   )) || /* Alt-* */
         insertkeymap( tt, 2617, mkkeyevt(F_ESC | '('   )) || /* Alt-( */
         insertkeymap( tt, 2608, mkkeyevt(F_ESC | ')'   )) || /* Alt-) */
        insertkeymap( tt, 6446, mkkeyevt(F_ESC | DEL   )) || /* Alt-Gray-Del */

        insertkeymap( tt, 4397, mkkeyevt(F_KVERB | K_EMACS_OVER )) || /* Insert key toggles insert / overwrite mode */

        insertkeymap( tt, 4388, mkkeyevt(F_ESC | '<'  )) || /* Home         Home key goes to top screen line      */
        insertkeymap( tt, 4385, mkkeyevt(F_ESC | 'v'  )) || /* Page up      Page up goes to previous screen       */
        insertkeymap( tt, 4398, mkkeyevt(127          )) || /* Delete       Delete key sends Delete            */
        insertkeymap( tt, 4387, mkkeyevt(F_ESC | '>'  )) || /* End          End key goes to bottom screen line    */
        insertkeymap( tt, 4386, mkkeyevt(22           )) || /* Page down    Page down goes to next screen              */
        insertkeymap( tt, 4392, mkkeyevt(14           )) || /* Down arrow   Down arrow goes down one line              */
        insertkeymap( tt, 4389, mkkeyevt(02           )) || /* Left arrow   Left arrow goes left one character    */
        insertkeymap( tt, 4391, mkkeyevt(06           )) || /* Right arrow  Right arrow goes right one character  */
        insertkeymap( tt, 4390, mkkeyevt(16           )) /* Up arrow     Up arrow goes up one line             */
         )
        return -1;
    return 0;
}

int
defrussiankm(int tt) {                      /* Russian keyboard mode (CP866) */

/*  Corresponds to Soviet Union PC keyboard layout. */

    if (
    insertkeymap( tt,  35, mkkeyevt(252)) ||                  /* # = Number sign */
    insertkeymap( tt,  36, mkkeyevt( 37)) ||                  /* $ = %           */
    insertkeymap( tt,  44, mkkeyevt(161)) ||                  /* , = be          */
    insertkeymap( tt,  46, mkkeyevt(238)) ||                  /* . = yu          */
    insertkeymap( tt,  47, mkkeyevt( 46)) ||                  /* / = .           */
    insertkeymap( tt,  58, mkkeyevt(157)) ||                  /* : = E           */
    insertkeymap( tt,  59, mkkeyevt(237)) ||                  /* ) || = e           */
    insertkeymap( tt,  60, mkkeyevt(129)) ||                  /* < = Be          */
    insertkeymap( tt,  62, mkkeyevt(158)) ||                  /* > = Yu          */
    insertkeymap( tt,  63, mkkeyevt( 44)) ||                  /* ? = ,           */
    insertkeymap( tt,  64, mkkeyevt( 34)) ||                  /* @ = "           */
    insertkeymap( tt,  65, mkkeyevt(148)) ||                  /* A = Ef          */
    insertkeymap( tt,  66, mkkeyevt(136)) ||                  /* B = I           */
    insertkeymap( tt,  67, mkkeyevt(145)) ||                  /* C = Es          */
    insertkeymap( tt,  68, mkkeyevt(130)) ||                  /* D = Ve          */
    insertkeymap( tt,  69, mkkeyevt(147)) ||                  /* E = U           */
    insertkeymap( tt,  70, mkkeyevt(128)) ||                  /* F = A           */
    insertkeymap( tt,  71, mkkeyevt(143)) ||                  /* G = Pe          */
    insertkeymap( tt,  72, mkkeyevt(144)) ||                  /* H = Er          */
    insertkeymap( tt,  73, mkkeyevt(152)) ||                  /* I = Sha         */
    insertkeymap( tt,  74, mkkeyevt(142)) ||                  /* J = O           */
    insertkeymap( tt,  75, mkkeyevt(139)) ||                  /* K = El          */
    insertkeymap( tt,  76, mkkeyevt(132)) ||                  /* L = De          */
    insertkeymap( tt,  77, mkkeyevt(156)) ||                  /* M = Soft Sign   */
    insertkeymap( tt,  78, mkkeyevt(146)) ||                  /* N = Te          */
    insertkeymap( tt,  79, mkkeyevt(153)) ||                  /* O = Shcha       */
    insertkeymap( tt,  80, mkkeyevt(135)) ||                  /* P = Ze          */
    insertkeymap( tt,  81, mkkeyevt(137)) ||                  /* Q = I-kratkoye  */
    insertkeymap( tt,  82, mkkeyevt(138)) ||                  /* R = Ka          */
    insertkeymap( tt,  83, mkkeyevt(155)) ||                  /* S = Yeri        */
    insertkeymap( tt,  84, mkkeyevt(133)) ||                  /* T = Ie          */
    insertkeymap( tt,  85, mkkeyevt(131)) ||                  /* U = Ghe         */
    insertkeymap( tt,  86, mkkeyevt(140)) ||                  /* V = Em          */
    insertkeymap( tt,  87, mkkeyevt(150)) ||                  /* W = Tse         */
    insertkeymap( tt,  88, mkkeyevt(151)) ||                  /* X = Che         */
    insertkeymap( tt,  89, mkkeyevt(141)) ||                  /* Y = En          */
    insertkeymap( tt,  90, mkkeyevt(159)) ||                  /* Z = Ya          */
    insertkeymap( tt,  91, mkkeyevt(229)) ||                  /* [ = ha          */
    insertkeymap( tt,  92, mkkeyevt( 47)) ||                  /* \ = /           */
    insertkeymap( tt,  93, mkkeyevt(234)) ||                  /* , hard sign   */
    insertkeymap( tt,  94, mkkeyevt( 58)) ||                  /* ^ = :           */
    insertkeymap( tt,  96, mkkeyevt(241)) ||                  /* ` = io          */
    insertkeymap( tt,  97, mkkeyevt(228)) ||                  /* a = ef          */
    insertkeymap( tt,  98, mkkeyevt(168)) ||                  /* b = i           */
    insertkeymap( tt,  99, mkkeyevt(225)) ||                  /* c = es          */
    insertkeymap( tt, 100, mkkeyevt(162)) ||                  /* d = ve          */
    insertkeymap( tt, 101, mkkeyevt(227)) ||                  /* e = u           */
    insertkeymap( tt, 102, mkkeyevt(160)) ||                  /* f = a           */
    insertkeymap( tt, 103, mkkeyevt(175)) ||                  /* g = pe          */
    insertkeymap( tt, 104, mkkeyevt(224)) ||                  /* h = er          */
    insertkeymap( tt, 105, mkkeyevt(232)) ||                  /* i = sha         */
    insertkeymap( tt, 106, mkkeyevt(174)) ||                  /* j = o           */
    insertkeymap( tt, 107, mkkeyevt(171)) ||                  /* k = el          */
    insertkeymap( tt, 108, mkkeyevt(164)) ||                  /* l = de          */
    insertkeymap( tt, 109, mkkeyevt(236)) ||                  /* m = soft sign   */
    insertkeymap( tt, 110, mkkeyevt(226)) ||                  /* n = te          */
    insertkeymap( tt, 111, mkkeyevt(233)) ||                  /* o = shcha       */
    insertkeymap( tt, 112, mkkeyevt(167)) ||                  /* p = ze          */
    insertkeymap( tt, 113, mkkeyevt(169)) ||                  /* q = i-kratkoye  */
    insertkeymap( tt, 114, mkkeyevt(170)) ||                  /* r = ka          */
    insertkeymap( tt, 115, mkkeyevt(235)) ||                  /* s = yeri        */
    insertkeymap( tt, 116, mkkeyevt(165)) ||                  /* t = ie          */
    insertkeymap( tt, 117, mkkeyevt(163)) ||                  /* u = ghe         */
    insertkeymap( tt, 118, mkkeyevt(172)) ||                  /* v = em          */
    insertkeymap( tt, 119, mkkeyevt(230)) ||                  /* w = tse         */
    insertkeymap( tt, 120, mkkeyevt(231)) ||                  /* x = che         */
    insertkeymap( tt, 121, mkkeyevt(173)) ||                  /* y = en          */
    insertkeymap( tt, 122, mkkeyevt(239)) ||                  /* z = ya          */
    insertkeymap( tt, 123, mkkeyevt(149)) ||                  /* { = Ha          */
    insertkeymap( tt, 125, mkkeyevt(154)) ||                  /* } = Hard Sign   */
    insertkeymap( tt, 126, mkkeyevt(240))                     /* ~ = Io          */
         )
        return -1;
    return 0;
}


/*
  Hebrew keyboard-mode support functions.  These are needed when using a
  non-Hebrew keyboard to enter Hebrew letters during CONNECT mode.  When
  the keyboard is in "English" mode, the main keypad acts normally.  When it
  is in Hebrew mode, the lowerinsertkeymap( tt, letters (and some punctuation marks) are
  mapped to Hebrew letters.  Keyboard mode is set via the keyboard verbs
  \KkbHebrew and \KkbHebrew, and also by DECHEBM escape sequences
  (CSI ? 35 h / l) from the host (handled in vtescape()).  Both methods
  invoke these routines.
*/
int
defhebrewkm(int tt) {                   /* Hebrew keyboard mode (CP862) */

/*
  This is the WordPerfect / Windows / Microsoft Word mapping.  If different
  mappings are needed, we can add them via a parameter to this function,
  settable by a SET TERMINAL HEBREW-KEYMAP command.  Note that we don't change
  the keymap by executing a macro (like MS-DOS Kermit's KEYBOARDS/KEYBOARDR
  macros) since that causes disconcerting and lengthy screen flashes as Kermit
  pops back and forth between CONNECT and command mode.
*/
    if (
    insertkeymap( tt,  39, mkkeyevt( 44)) ||                    /* ' (Apostrophe) = , (Comma) */
    insertkeymap( tt,  44, mkkeyevt(154)) ||                    /* , (Comma) = Taw */
    insertkeymap( tt,  46, mkkeyevt(149)) ||                    /* . (Period) = Final Zade */
    insertkeymap( tt,  47, mkkeyevt( 46)) ||                    /* / (Slash) = . (Period) */
    insertkeymap( tt,  59, mkkeyevt(147)) ||                    /* ) || (Semicolon) = Terminal Pe */
    insertkeymap( tt,  97, mkkeyevt(153)) ||                    /* a = Shin */
    insertkeymap( tt,  98, mkkeyevt(144)) ||                    /* b = Nun */
    insertkeymap( tt,  99, mkkeyevt(129)) ||                    /* c = Bet */
    insertkeymap( tt, 100, mkkeyevt(130)) ||                    /* d = Gimel */
    insertkeymap( tt, 101, mkkeyevt(151)) ||                    /* e = Qoph */
    insertkeymap( tt, 102, mkkeyevt(139)) ||                    /* f = Kaph */
    insertkeymap( tt, 103, mkkeyevt(146)) ||                    /* g = Ayin */
    insertkeymap( tt, 104, mkkeyevt(137)) ||                    /* h = Yod */
    insertkeymap( tt, 105, mkkeyevt(143)) ||                    /* i = Final Nun */
    insertkeymap( tt, 106, mkkeyevt(135)) ||                    /* j = Chet */
    insertkeymap( tt, 107, mkkeyevt(140)) ||                    /* k = Lamed */
    insertkeymap( tt, 108, mkkeyevt(138)) ||                    /* l = Final Kaph */
    insertkeymap( tt, 109, mkkeyevt(150)) ||                    /* m = Zade */
    insertkeymap( tt, 110, mkkeyevt(142)) ||                    /* n = Mem */
    insertkeymap( tt, 111, mkkeyevt(141)) ||                    /* o = Final Mem */
    insertkeymap( tt, 112, mkkeyevt(148)) ||                    /* p = Pe */
    insertkeymap( tt, 113, mkkeyevt( 47)) ||                    /* q = / (slash) */
    insertkeymap( tt, 114, mkkeyevt(152)) ||                    /* r = Resh */
    insertkeymap( tt, 115, mkkeyevt(131)) ||                    /* s = Dalet */
    insertkeymap( tt, 116, mkkeyevt(128)) ||                    /* t = Aleph */
    insertkeymap( tt, 117, mkkeyevt(133)) ||                    /* u = Waw */
    insertkeymap( tt, 118, mkkeyevt(132)) ||                    /* v = He */
    insertkeymap( tt, 119, mkkeyevt( 39)) ||                    /* w = ' (Apostrophe) */
    insertkeymap( tt, 120, mkkeyevt(145)) ||                    /* x = Samech */
    insertkeymap( tt, 121, mkkeyevt(136)) ||                    /* y = Tet */
    insertkeymap( tt, 122, mkkeyevt(134))                       /* z = Zain */
         )
        return -1;
    return 0;
}

int
defvt100km( int tt )
{

    if ( defbasekm( tt ) ||
    /* Ctrl characters */

    /* Arrow keys */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 38, mkkeyevt(F_KVERB | K_UPARR)) ||     /* Up Arrow    Gray */
    insertkeymap( tt, KEY_SCAN | 38               , mkkeyevt(F_KVERB | K_UPARR)) ||     /* Up Arrow    Numeric */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 37, mkkeyevt(F_KVERB | K_LFARR)) ||     /* Left Arrow  Gray */
    insertkeymap( tt, KEY_SCAN | 37               , mkkeyevt(F_KVERB | K_LFARR)) ||     /* Left Arrow  Numeric*/
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 39, mkkeyevt(F_KVERB | K_RTARR)) ||     /* Right Arrow Gray */
    insertkeymap( tt, KEY_SCAN | 39               , mkkeyevt(F_KVERB | K_RTARR)) ||     /* Right Arrow Numeric */
    insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 40, mkkeyevt(F_KVERB | K_DNARR)) ||     /* Down Arrow  Gray */
    insertkeymap( tt, KEY_SCAN | 40               , mkkeyevt(F_KVERB | K_DNARR)) ||     /* Down Arrow  Numeric */

    /* Function keys */
    insertkeymap( tt, KEY_SCAN | 112, mkkeyevt(F_KVERB | K_PF1)) ||     /* F1 return DEC PF1 */
    insertkeymap( tt, KEY_SCAN | 113, mkkeyevt(F_KVERB | K_PF2)) ||     /* F2 return DEC PF2 */
    insertkeymap( tt, KEY_SCAN | 114, mkkeyevt(F_KVERB | K_PF3)) ||     /* F3 return DEC PF3 */
    insertkeymap( tt, KEY_SCAN | 115, mkkeyevt(F_KVERB | K_PF4)) ||     /* F4 return DEC PF4 */

    insertkeymap( tt, KEY_ALT | 49, mkkeyevt(F_KVERB | K_KP1)) ||       /* Alt-1, Top Rank return DEC Keypad 1 */
    insertkeymap( tt, KEY_ALT | 50, mkkeyevt(F_KVERB | K_KP2)) ||       /* Alt-2, Top Rank return DEC Keypad 2 */
    insertkeymap( tt, KEY_ALT | 51, mkkeyevt(F_KVERB | K_KP3)) ||       /* Alt-3, Top Rank return DEC Keypad 3 */
    insertkeymap( tt, KEY_ALT | 52, mkkeyevt(F_KVERB | K_KP4)) ||       /* Alt-4, Top Rank return DEC Keypad 4 */
    insertkeymap( tt, KEY_ALT | 53, mkkeyevt(F_KVERB | K_KP5)) ||       /* Alt-5, Top Rank return DEC Keypad 5 */
    insertkeymap( tt, KEY_ALT | 54, mkkeyevt(F_KVERB | K_KP6)) ||       /* Alt-6, Top Rank return DEC Keypad 6 */
    insertkeymap( tt, KEY_ALT | 55, mkkeyevt(F_KVERB | K_KP7)) ||       /* Alt-7, Top Rank return DEC Keypad 7 */
    insertkeymap( tt, KEY_ALT | 56, mkkeyevt(F_KVERB | K_KP8)) ||       /* Alt-8, Top Rank return DEC Keypad 8 */
    insertkeymap( tt, KEY_ALT | 57, mkkeyevt(F_KVERB | K_KP9)) ||       /* Alt-9, Top Rank return DEC Keypad 9 */
    insertkeymap( tt, KEY_ALT | 48, mkkeyevt(F_KVERB | K_KP0)) ||       /* Alt-0, Top Rank return DEC Keypad 0 */

    insertkeymap( tt, KEY_ALT | 45, mkkeyevt(F_KVERB | K_KPMINUS)) ||/* Alt -, Top Rank return DEC Keypad - */
    insertkeymap( tt, KEY_ALT | 44, mkkeyevt(F_KVERB | K_KPCOMA)) ||    /* Alt-Comma return DEC Keypad comma   */
    insertkeymap( tt, KEY_ALT | 46, mkkeyevt(F_KVERB | K_KPDOT)) ||     /* Alt-Period return DEC Keypad period */
    insertkeymap( tt, KEY_SCAN | KEY_SHIFT | KEY_CTRL | 13,
                    mkkeyevt(F_KVERB | K_KPENTER)) || /* Ctrl-Shift-Enter  return DEC Keypad Enter  */

    /* Scan  Kverb                              DEC Key            PC Key           PC Key Group */
    insertkeymap( tt,  400, mkkeyevt(F_KVERB | K_PF1 )) ||      /* PF1 (Gold) 95      Num Lock         Numeric keypad */
    insertkeymap( tt, 4496, mkkeyevt(F_KVERB | K_PF1 )) ||      /* PF1 (Gold) NT      Num Lock         Numeric keypad */
    insertkeymap( tt, 4143, mkkeyevt(F_KVERB | K_PF2 )) ||      /* PF2        95      Keypad /         Numeric keypad */
    insertkeymap( tt, 4543, mkkeyevt(F_KVERB | K_PF2 )) ||      /* PF2        95      Keypad /         Numeric keypad */
    insertkeymap( tt, 4463, mkkeyevt(F_KVERB | K_PF2 )) ||      /* PF2        NT      Keypad /         Numeric keypad */
    insertkeymap( tt,  362, mkkeyevt(F_KVERB | K_PF3 )) ||      /* PF3                Keypad *         Numeric keypad */
    insertkeymap( tt,  365, mkkeyevt(F_KVERB | K_PF4 )) ||      /* PF4                Keypad -         Numeric keypad */

    /*
    * DEC auxiliary keypad digit keys (two each, for Num Lock On and Off).
    */
    insertkeymap( tt, 352 , mkkeyevt(F_KVERB | K_KP0 )) || /* KP 0               Keypad 0         Numeric keypad */
    insertkeymap( tt, 301 , mkkeyevt(F_KVERB | K_KP0 )) || /* KP 0               Keypad 0         Numeric keypad */
    insertkeymap( tt, 353 , mkkeyevt(F_KVERB | K_KP1 )) || /* KP 1               Keypad 1         Numeric keypad */
    insertkeymap( tt, 291 , mkkeyevt(F_KVERB | K_KP1 )) || /* KP 1               Keypad 1         Numeric keypad */
    insertkeymap( tt, 354 , mkkeyevt(F_KVERB | K_KP2 )) || /* KP 2               Keypad 2         Numeric keypad */
    insertkeymap( tt, 296 , mkkeyevt(F_KVERB | K_KP2 )) || /* KP 2               Keypad 2         Numeric keypad */
    insertkeymap( tt, 355 , mkkeyevt(F_KVERB | K_KP3 )) || /* KP 3               Keypad 3         Numeric keypad */
    insertkeymap( tt, 290 , mkkeyevt(F_KVERB | K_KP3 )) || /* KP 3               Keypad 3         Numeric keypad */
    insertkeymap( tt, 356 , mkkeyevt(F_KVERB | K_KP4 )) || /* KP 4               Keypad 4         Numeric keypad */
    insertkeymap( tt, 293 , mkkeyevt(F_KVERB | K_KP4 )) || /* KP 4               Keypad 4         Numeric keypad */
    insertkeymap( tt, 357 , mkkeyevt(F_KVERB | K_KP5 )) || /* KP 5               Keypad 5         Numeric keypad */
    insertkeymap( tt, 268 , mkkeyevt(F_KVERB | K_KP5 )) || /* KP 5               Keypad 5         Numeric keypad */
    insertkeymap( tt, 358 , mkkeyevt(F_KVERB | K_KP6 )) || /* KP 6               Keypad 6         Numeric keypad */
    insertkeymap( tt, 295 , mkkeyevt(F_KVERB | K_KP6 )) || /* KP 6               Keypad 6         Numeric keypad */
    insertkeymap( tt, 359 , mkkeyevt(F_KVERB | K_KP7 )) || /* KP 7               Keypad 7         Numeric keypad */
    insertkeymap( tt, 292 , mkkeyevt(F_KVERB | K_KP7 )) || /* KP 7               Keypad 7         Numeric keypad */
    insertkeymap( tt, 360 , mkkeyevt(F_KVERB | K_KP8 )) || /* KP 8               Keypad 8         Numeric keypad */
    insertkeymap( tt, 294 , mkkeyevt(F_KVERB | K_KP8 )) || /* KP 8               Keypad 8         Numeric keypad */
    insertkeymap( tt, 361 , mkkeyevt(F_KVERB | K_KP9 )) || /* KP 9               Keypad 9         Numeric keypad */
    insertkeymap( tt, 289 , mkkeyevt(F_KVERB | K_KP9 )) || /* KP 9               Keypad 9         Numeric keypad */
    /*
    ) || DEC auxiliary keypad punctuation and other keys.  Note that DEC keypads have
    ) || four keys on the right, whereas IBM keypads have only three.  Thus the
    ) || need for an Alt-key modifier on one of them.  Also note that while it might
    ) || seem to make more sense to map DEC Keypad Minus to IBM Keypad Minus, most
    ) || DEC users are accustomed to having PF1-PF4 across the top keypad row.
    ) ||
    ) ||   NOTE: In Windows 95, KP period with Num Lock on does not
    ) ||   return a unique scan code.
    */
    insertkeymap( tt, 366 , mkkeyevt(F_KVERB | K_KPDOT   )) || /* KP Period          Keypad Period    Numeric keypad */
    insertkeymap( tt, 302 , mkkeyevt(F_KVERB | K_KPDOT   )) || /* KP Period          Keypad Period    Numeric keypad */
    insertkeymap( tt, 4365, mkkeyevt(F_KVERB | K_KPENTER )) || /* KP Enter           Keypad Enter     Numeric keypad */
    insertkeymap( tt, 2411, mkkeyevt(F_KVERB | K_KPMINUS )) || /* KP Minus           Alt-Keypad-Plus  Numeric keypad */
    insertkeymap( tt, 363 , mkkeyevt(F_KVERB | K_KPCOMA  ))    /* KP Comma           Keypad Plus      Numeric keypad */
         )
        return -1;
    return 0;
}

int
defvt200km( int tt )
{
    if ( defvt100km( tt ) ||
#ifdef COMMENT
        /* Scan  Kverb                          DEC Key            PC Key           PC Key Group */
    insertkeymap( tt,  400, mkkeyevt(F_KVERB | K_PF1 )) ||      /* PF1 (Gold) 95      Num Lock         Numeric keypad */
    insertkeymap( tt, 4496, mkkeyevt(F_KVERB | K_PF1 )) ||      /* PF1 (Gold) NT      Num Lock         Numeric keypad */
    insertkeymap( tt, 4143, mkkeyevt(F_KVERB | K_PF2 )) ||      /* PF2        95      Keypad /         Numeric keypad */
    insertkeymap( tt, 4543, mkkeyevt(F_KVERB | K_PF2 )) ||      /* PF2        95      Keypad /         Numeric keypad */
    insertkeymap( tt, 4463, mkkeyevt(F_KVERB | K_PF2 )) ||      /* PF2        NT      Keypad /         Numeric keypad */
    insertkeymap( tt,  362, mkkeyevt(F_KVERB | K_PF3 )) ||      /* PF3                Keypad *         Numeric keypad */
    insertkeymap( tt,  365, mkkeyevt(F_KVERB | K_PF4 )) ||      /* PF4                Keypad -         Numeric keypad */

        /*
        * DEC PF1-PF4 also assigned to PC F1-F4.
        */
    insertkeymap( tt, 368 , mkkeyevt(F_KVERB | K_PF1 )) || /* PF1 (Gold)         F1               Top Rank F key */
    insertkeymap( tt, 369 , mkkeyevt(F_KVERB | K_PF2 )) || /* PF2                F2               Top Rank F key */
    insertkeymap( tt, 370 , mkkeyevt(F_KVERB | K_PF3 )) || /* PF3                F3               Top Rank F key */
    insertkeymap( tt, 371 , mkkeyevt(F_KVERB | K_PF4 )) || /* PF4                F4               Top Rank F key */
    /*
    * DEC auxiliary keypad digit keys (two each, for Num Lock On and Off).
    */
    insertkeymap( tt, 352 , mkkeyevt(F_KVERB | K_KP0 )) || /* KP 0               Keypad 0         Numeric keypad */
    insertkeymap( tt, 301 , mkkeyevt(F_KVERB | K_KP0 )) || /* KP 0               Keypad 0         Numeric keypad */
    insertkeymap( tt, 353 , mkkeyevt(F_KVERB | K_KP1 )) || /* KP 1               Keypad 1         Numeric keypad */
    insertkeymap( tt, 291 , mkkeyevt(F_KVERB | K_KP1 )) || /* KP 1               Keypad 1         Numeric keypad */
    insertkeymap( tt, 354 , mkkeyevt(F_KVERB | K_KP2 )) || /* KP 2               Keypad 2         Numeric keypad */
    insertkeymap( tt, 296 , mkkeyevt(F_KVERB | K_KP2 )) || /* KP 2               Keypad 2         Numeric keypad */
    insertkeymap( tt, 355 , mkkeyevt(F_KVERB | K_KP3 )) || /* KP 3               Keypad 3         Numeric keypad */
    insertkeymap( tt, 290 , mkkeyevt(F_KVERB | K_KP3 )) || /* KP 3               Keypad 3         Numeric keypad */
    insertkeymap( tt, 356 , mkkeyevt(F_KVERB | K_KP4 )) || /* KP 4               Keypad 4         Numeric keypad */
    insertkeymap( tt, 293 , mkkeyevt(F_KVERB | K_KP4 )) || /* KP 4               Keypad 4         Numeric keypad */
    insertkeymap( tt, 357 , mkkeyevt(F_KVERB | K_KP5 )) || /* KP 5               Keypad 5         Numeric keypad */
    insertkeymap( tt, 268 , mkkeyevt(F_KVERB | K_KP5 )) || /* KP 5               Keypad 5         Numeric keypad */
    insertkeymap( tt, 358 , mkkeyevt(F_KVERB | K_KP6 )) || /* KP 6               Keypad 6         Numeric keypad */
    insertkeymap( tt, 295 , mkkeyevt(F_KVERB | K_KP6 )) || /* KP 6               Keypad 6         Numeric keypad */
    insertkeymap( tt, 359 , mkkeyevt(F_KVERB | K_KP7 )) || /* KP 7               Keypad 7         Numeric keypad */
    insertkeymap( tt, 292 , mkkeyevt(F_KVERB | K_KP7 )) || /* KP 7               Keypad 7         Numeric keypad */
    insertkeymap( tt, 360 , mkkeyevt(F_KVERB | K_KP8 )) || /* KP 8               Keypad 8         Numeric keypad */
    insertkeymap( tt, 294 , mkkeyevt(F_KVERB | K_KP8 )) || /* KP 8               Keypad 8         Numeric keypad */
    insertkeymap( tt, 361 , mkkeyevt(F_KVERB | K_KP9 )) || /* KP 9               Keypad 9         Numeric keypad */
    insertkeymap( tt, 289 , mkkeyevt(F_KVERB | K_KP9 )) || /* KP 9               Keypad 9         Numeric keypad */
    /*
    ) || DEC auxiliary keypad punctuation and other keys.  Note that DEC keypads have
    ) || four keys on the right, whereas IBM keypads have only three.  Thus the
    ) || need for an Alt-key modifier on one of them.  Also note that while it might
    ) || seem to make more sense to map DEC Keypad Minus to IBM Keypad Minus, most
    ) || DEC users are accustomed to having PF1-PF4 across the top keypad row.
    ) ||
    ) ||   NOTE: In Windows 95, KP period with Num Lock on does not
    ) ||   return a unique scan code.
    */
    insertkeymap( tt, 366 , mkkeyevt(F_KVERB | K_KPDOT   )) || /* KP Period          Keypad Period    Numeric keypad */
    insertkeymap( tt, 302 , mkkeyevt(F_KVERB | K_KPDOT   )) || /* KP Period          Keypad Period    Numeric keypad */
    insertkeymap( tt, 4365, mkkeyevt(F_KVERB | K_KPENTER )) || /* KP Enter           Keypad Enter     Numeric keypad */
    insertkeymap( tt, 2411, mkkeyevt(F_KVERB | K_KPMINUS )) || /* KP Minus           Alt-Keypad-Plus  Numeric keypad */
    insertkeymap( tt, 363 , mkkeyevt(F_KVERB | K_KPCOMA  )) || /* KP Comma           Keypad Plus      Numeric keypad */
#endif /* COMMENT */
        /*
        ) || DEC Cursor keys
        */
    insertkeymap( tt, 4390, mkkeyevt(F_KVERB | K_UPARR )) || /* Up Arrow           Up Arrow         Cursor keypad */
    insertkeymap( tt, 4392, mkkeyevt(F_KVERB | K_DNARR )) || /* Down Arrow         Down Arrow       Cursor keypad */
    insertkeymap( tt, 4391, mkkeyevt(F_KVERB | K_RTARR )) || /* Right Arrow        Right Arrow      Cursor keypad */
    insertkeymap( tt, 4389, mkkeyevt(F_KVERB | K_LFARR )) || /* Left Arrow         Left Arrow       Cursor keypad */
        /*
        ) || DEC Editing keys, VT220 and above.
        */
    insertkeymap( tt, 6445, mkkeyevt(F_KVERB | K_DECFIND   )) || /* Find               Alt-Insert      Gray keypad */
    insertkeymap( tt, 6436, mkkeyevt(F_KVERB | K_DECINSERT )) || /* Insert             Alt-Home        Gray keypad */
    insertkeymap( tt, 6433, mkkeyevt(F_KVERB | K_DECREMOVE )) || /* Remove             Alt-Page-Up     Gray keypad */
    insertkeymap( tt, 6446, mkkeyevt(F_KVERB | K_DECSELECT )) || /* Select             Alt-Delete      Gray keypad */
    insertkeymap( tt, 6435, mkkeyevt(F_KVERB | K_DECPREV   )) || /* Previous Screen    Alt-End         Gray keypad */
    insertkeymap( tt, 6434, mkkeyevt(F_KVERB | K_DECNEXT   )) || /* Next Screen        Alt-Page-Down   Gray keypad */

    insertkeymap( tt, 378 , mkkeyevt(F_KVERB | K_DECHELP   )) || /* Help = F15         F11 (Can't use Print Screen, */
    insertkeymap( tt, 379 , mkkeyevt(F_KVERB | K_DECDO     )) || /* Do = F16           F12  Scroll Lock, or Pause)       */
    /*
    ) || DEC Function keys, VT220 and above.
    ) ||
    ) ||   DEC F5-F10 are on PC F5-F10.
    ) ||   DEC F11-F10 are on PC Alt-F1 through Alt-F10.
    ) ||   UDK F1-F10 are on PC Shift-F1 - ShiftF10
    ) ||   UDK F11-F10 are on PC Alt-Shift-F1 through Alt-Shift-F10.
    */
    insertkeymap( tt, 372 , mkkeyevt(F_KVERB | K_BREAK  )) || /* F5 /Break          F5               Top Rank F key */
    insertkeymap( tt, 373 , mkkeyevt(F_KVERB | K_DECF6  )) || /* F6                 F6               Top Rank F key */
    insertkeymap( tt, 374 , mkkeyevt(F_KVERB | K_DECF7  )) || /* F7                 F7               Top Rank F key */
    insertkeymap( tt, 375 , mkkeyevt(F_KVERB | K_DECF8  )) || /* F8                 F8               Top Rank F key */
    insertkeymap( tt, 376 , mkkeyevt(F_KVERB | K_DECF9  )) || /* F9                 F9               Top Rank F key */
    insertkeymap( tt, 377 , mkkeyevt(F_KVERB | K_DECF10 )) || /* F10                F10              Top Rank F key */

    insertkeymap( tt, 2416, mkkeyevt(F_KVERB | K_DECF11 )) || /* F11 / Esc          Alt-F1           Top Rank F key */
    insertkeymap( tt, 2417, mkkeyevt(F_KVERB | K_DECF12 )) || /* F12 / BS           Alt-F2           Top Rank F key */
    insertkeymap( tt, 2418, mkkeyevt(F_KVERB | K_DECF13 )) || /* F13 / LF           Alt-F3           Top Rank F key */
    insertkeymap( tt, 2419, mkkeyevt(F_KVERB | K_DECF14 )) || /* F14                Alt-F4           Top Rank F key */
    insertkeymap( tt, 2420, mkkeyevt(F_KVERB | K_DECF15 )) || /* F15 / Help         Alt-F5           Top Rank F key */
    insertkeymap( tt, 2421, mkkeyevt(F_KVERB | K_DECF16 )) || /* F16 / Do           Alt-F6           Top Rank F key */
    insertkeymap( tt, 2422, mkkeyevt(F_KVERB | K_DECF17 )) || /* F17                Alt-F7           Top Rank F key */
    insertkeymap( tt, 2423, mkkeyevt(F_KVERB | K_DECF18 )) || /* F18                Alt-F8           Top Rank F key */
    insertkeymap( tt, 2424, mkkeyevt(F_KVERB | K_DECF19 )) || /* F19                Alt-F9           Top Rank F key */
    insertkeymap( tt, 2425, mkkeyevt(F_KVERB | K_DECF20 )) || /* F20                Alt-F10          Top Rank F key */

    insertkeymap( tt, 4466, mkkeyevt(F_KVERB | K_DECF13 )) || /* F13 / LF           LK450 F13        Top Rank F key */
    insertkeymap( tt, 4467, mkkeyevt(F_KVERB | K_DECF14 )) || /* F14                LK450 F14        Top Rank F key */
    insertkeymap( tt, 4468, mkkeyevt(F_KVERB | K_DECF15 )) || /* F15 / Help         LK450 F15        Top Rank F key */
    insertkeymap( tt, 4469, mkkeyevt(F_KVERB | K_DECF16 )) || /* F16 / Do           LK450 F16        Top Rank F key */
    insertkeymap( tt, 4470, mkkeyevt(F_KVERB | K_DECF17 )) || /* F17                LK450 F17        Top Rank F key */
    insertkeymap( tt, 4471, mkkeyevt(F_KVERB | K_DECF18 )) || /* F18                LK450 F18        Top Rank F key */
    insertkeymap( tt, 4472, mkkeyevt(F_KVERB | K_DECF19 )) || /* F19                LK450 F19        Top Rank F key */
    insertkeymap( tt, 4473, mkkeyevt(F_KVERB | K_DECF20 )) || /* F20                LK450 F20        Top Rank F key */

    insertkeymap( tt, 4476, mkkeyevt(F_KVERB | K_DECF13 )) || /* F13 / LF           LK450 F13        Top Rank F key */
    insertkeymap( tt, 4477, mkkeyevt(F_KVERB | K_DECF14 )) || /* F14                LK450 F14        Top Rank F key */
    insertkeymap( tt, 4478, mkkeyevt(F_KVERB | K_DECF15 )) || /* F15 / Help         LK450 F15        Top Rank F key */
    insertkeymap( tt, 4479, mkkeyevt(F_KVERB | K_DECF16 )) || /* F16 / Do           LK450 F16        Top Rank F key */
    insertkeymap( tt, 4480, mkkeyevt(F_KVERB | K_DECF17 )) || /* F17                LK450 F17        Top Rank F key */
    insertkeymap( tt, 4481, mkkeyevt(F_KVERB | K_DECF18 )) || /* F18                LK450 F18        Top Rank F key */
    insertkeymap( tt, 4482, mkkeyevt(F_KVERB | K_DECF19 )) || /* F19                LK450 F19        Top Rank F key */
    insertkeymap( tt, 4483, mkkeyevt(F_KVERB | K_DECF20 )) || /* F20                LK450 F20        Top Rank F key */

    insertkeymap( tt, 880 , mkkeyevt(F_KVERB | K_UDKF1  )) || /* F1                 Shift-F1         Top Rank F key */
    insertkeymap( tt, 881 , mkkeyevt(F_KVERB | K_UDKF2  )) || /* F2                 Shift-F2         Top Rank F key */
    insertkeymap( tt, 882 , mkkeyevt(F_KVERB | K_UDKF3  )) || /* F3                 Shift-F3         Top Rank F key */
    insertkeymap( tt, 883 , mkkeyevt(F_KVERB | K_UDKF4  )) || /* F4                 Shift-F4         Top Rank F key */
    insertkeymap( tt, 884 , mkkeyevt(F_KVERB | K_UDKF5  )) || /* F5                 Shift-F5         Top Rank F key */

    insertkeymap( tt, 885 , mkkeyevt(F_KVERB | K_UDKF6  )) || /* F6                 Shift-F6         Top Rank F key */
    insertkeymap( tt, 886 , mkkeyevt(F_KVERB | K_UDKF7  )) || /* F7                 Shift-F7         Top Rank F key */
    insertkeymap( tt, 887 , mkkeyevt(F_KVERB | K_UDKF8  )) || /* F8                 Shift-F8         Top Rank F key */
    insertkeymap( tt, 888 , mkkeyevt(F_KVERB | K_UDKF9  )) || /* F9                 Shift-F9         Top Rank F key */
    insertkeymap( tt, 889 , mkkeyevt(F_KVERB | K_UDKF10 )) || /* F10                Shift-F10        Top Rank F key */

    insertkeymap( tt, 2928, mkkeyevt(F_KVERB | K_UDKF11 )) || /* F11 / Esc          Alt-Shift-F1     Top Rank F key */
    insertkeymap( tt, 2929, mkkeyevt(F_KVERB | K_UDKF12 )) || /* F12 / BS           Alt-Shift-F2     Top Rank F key */
    insertkeymap( tt, 2930, mkkeyevt(F_KVERB | K_UDKF13 )) || /* F13 / LF           Alt-Shift-F3     Top Rank F key */
    insertkeymap( tt, 2931, mkkeyevt(F_KVERB | K_UDKF14 )) || /* F14                Alt-Shift-F4     Top Rank F key */
    insertkeymap( tt, 2932, mkkeyevt(F_KVERB | K_UDKF15 )) || /* F15 / Help         Alt-Shift-F5     Top Rank F key */
    insertkeymap( tt, 2933, mkkeyevt(F_KVERB | K_UDKF16 )) || /* F16 / Do           Alt-Shift-F6     Top Rank F key */
    insertkeymap( tt, 2934, mkkeyevt(F_KVERB | K_UDKF17 )) || /* F17                Alt-Shift-F7     Top Rank F key */
    insertkeymap( tt, 2935, mkkeyevt(F_KVERB | K_UDKF18 )) || /* F18                Alt-Shift-F8     Top Rank F key */
    insertkeymap( tt, 2936, mkkeyevt(F_KVERB | K_UDKF19 )) || /* F19                Alt-Shift-F9     Top Rank F key */
    insertkeymap( tt, 2937, mkkeyevt(F_KVERB | K_UDKF20 )) || /* F20                Alt-Shift-F10    Top Rank F key */

    insertkeymap( tt, 4978, mkkeyevt(F_KVERB | K_UDKF13 )) || /* F13 / LF           LK450 Shift-F13  Top Rank F key */
    insertkeymap( tt, 4979, mkkeyevt(F_KVERB | K_UDKF14 )) || /* F14                LK450 Shift-F14  Top Rank F key */
    insertkeymap( tt, 4980, mkkeyevt(F_KVERB | K_UDKF15 )) || /* F15 / Help         LK450 Shift-F15  Top Rank F key */
    insertkeymap( tt, 4981, mkkeyevt(F_KVERB | K_UDKF16 )) || /* F16 / Do           LK450 Shift-F16  Top Rank F key */
    insertkeymap( tt, 4982, mkkeyevt(F_KVERB | K_UDKF17 )) || /* F17                LK450 Shift-F17  Top Rank F key */
    insertkeymap( tt, 4983, mkkeyevt(F_KVERB | K_UDKF18 )) || /* F18                LK450 Shift-F18  Top Rank F key */
    insertkeymap( tt, 4984, mkkeyevt(F_KVERB | K_UDKF19 )) || /* F19                LK450 Shift-F19  Top Rank F key */
    insertkeymap( tt, 4985, mkkeyevt(F_KVERB | K_UDKF20 )) || /* F20                LK450 Shift-F20  Top Rank F key */

    insertkeymap( tt, 4988, mkkeyevt(F_KVERB | K_UDKF13 )) || /* F13 / LF           LK450 Shift-F13  Top Rank F key */
    insertkeymap( tt, 4989, mkkeyevt(F_KVERB | K_UDKF14 )) || /* F14                LK450 Shift-F14  Top Rank F key */
    insertkeymap( tt, 4990, mkkeyevt(F_KVERB | K_UDKF15 )) || /* F15 / Help         LK450 Shift-F15  Top Rank F key */
    insertkeymap( tt, 4991, mkkeyevt(F_KVERB | K_UDKF16 )) || /* F16 / Do           LK450 Shift-F16  Top Rank F key */
    insertkeymap( tt, 4992, mkkeyevt(F_KVERB | K_UDKF17 )) || /* F17                LK450 Shift-F17  Top Rank F key */
    insertkeymap( tt, 4993, mkkeyevt(F_KVERB | K_UDKF18 )) || /* F18                LK450 Shift-F18  Top Rank F key */
    insertkeymap( tt, 4994, mkkeyevt(F_KVERB | K_UDKF19 )) || /* F19                LK450 Shift-F19  Top Rank F key */
    insertkeymap( tt, 4995, mkkeyevt(F_KVERB | K_UDKF20 )) || /* F20                LK450 Shift-F20  Top Rank F key */

    insertkeymap( tt, 4361, mkkeyevt(F_KVERB | K_COMPOSE )) || /* Compose           LK450 Compose */
    insertkeymap( tt, 4459, mkkeyevt(F_KVERB | K_KPMINUS ))    /* Kp-Minus          LK450 Minus   */
        )
        return -1;
    return 0;
}

int
defvtpckm( int tt )
{
    if ( defvt200km( tt ) ||
         insertkeymap( tt, 368 , mkkeyevt(F_KVERB | K_DECF1  )) || /* F1 */
         insertkeymap( tt, 369 , mkkeyevt(F_KVERB | K_DECF2  )) || /* F2 */
         insertkeymap( tt, 370 , mkkeyevt(F_KVERB | K_DECF3  )) || /* F3 */
         insertkeymap( tt, 371 , mkkeyevt(F_KVERB | K_DECF4  )) || /* F4 */
         insertkeymap( tt, 372 , mkkeyevt(F_KVERB | K_DECF5  )) || /* F5 */
         insertkeymap( tt, 4397, mkkeyevt(F_KVERB | K_DECINSERT )) ||
         insertkeymap( tt, 4388, mkkeyevt(F_KVERB | K_DECHOME )) ||
         insertkeymap( tt, 4386, mkkeyevt(F_KVERB | K_DECNEXT )) ||
         insertkeymap( tt, 4385, mkkeyevt(F_KVERB | K_DECPREV )) ||
         insertkeymap( tt, 4387, mkkeyevt(F_KVERB | K_DECFIND ))
         )
       return(-1);

    return(0);
}

int
defdgkm( int tt )
{
    if ( defbasekm(tt) ||
        /* Scan  Kverb                     DG Key           PC Key           PC Key Group */

        /*
        DG Cursor keys
        */
         insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 38, mkkeyevt(F_KVERB | K_UPARR)) ||        /* Up Arrow    Gray */
         insertkeymap( tt, KEY_SCAN | 38               , mkkeyevt(F_KVERB | K_UPARR)) ||        /* Up Arrow    Numeric */
         insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 37, mkkeyevt(F_KVERB | K_LFARR)) ||        /* Left Arrow  Gray */
         insertkeymap( tt, KEY_SCAN | 37               , mkkeyevt(F_KVERB | K_LFARR)) ||        /* Left Arrow  Numeric*/
         insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 39, mkkeyevt(F_KVERB | K_RTARR)) ||        /* Right Arrow Gray */
         insertkeymap( tt, KEY_SCAN | 39               , mkkeyevt(F_KVERB | K_RTARR)) ||        /* Right Arrow Numeric */
         insertkeymap( tt, KEY_SCAN | KEY_ENHANCED | 40, mkkeyevt(F_KVERB | K_DNARR)) ||        /* Down Arrow  Gray */
         insertkeymap( tt, KEY_SCAN | 40               , mkkeyevt(F_KVERB | K_DNARR)) ||        /* Down Arrow  Numeric */
         /*
        DG Cursor keys (shifted)
        */
         insertkeymap( tt, 4902, mkkeyevt(F_KVERB | K_DGSUPARR )) || /* Up Arrow     Shift-Up Arrow    Cursor keypad */
         insertkeymap( tt, 4904, mkkeyevt(F_KVERB | K_DGSDNARR )) || /* Down Arrow   Shift-Down Arrow  Cursor keypad */
         insertkeymap( tt, 4903, mkkeyevt(F_KVERB | K_DGSRTARR )) || /* Right Arrow  Shift-Right Arrow Cursor keypad */
         insertkeymap( tt, 4901, mkkeyevt(F_KVERB | K_DGSLFARR )) || /* Left Arrow   Shift-Left Arrow  Cursor keypad */
         insertkeymap( tt, 808, mkkeyevt(F_KVERB | K_DGSDNARR)) ||           /* Shift-DnArr        */
         insertkeymap( tt, 806, mkkeyevt(F_KVERB | K_DGSUPARR)) ||              /* Shift-UpArr  */
         insertkeymap( tt, 807, mkkeyevt(F_KVERB | K_DGSRTARR)) ||           /* Shift-RtArr     */
         insertkeymap( tt, 805, mkkeyevt(F_KVERB | K_DGSLFARR)) ||             /* Shift-LfArr   */
         /*
        ) || DG Function keys.
        ) ||
        ) ||   DG F1-F10 are on PC F1-F10.
        ) ||   DG F11-F15 are on PC Alt-F1 through Alt-F6.
        ) ||   DG Shift F1-F10 are on PC Shift-F1 - Shift-F10
        ) ||   DG Shift F11-F15 are on PC Alt-Shift-F1 through Alt-Shift-F15.
        ) ||   DG Control F1-F10 are on PC Ctrl-F1 - Ctrl-F10
        ) ||   DG Control F11-F15 are on PC Ctrl-Alt-F1 through Ctrl-Alt-F15.
        ) ||   DG Control Shift F1-F10 are on PC Ctrl-Shift-F1 - Ctrl-Shift-F10
        ) ||   DG Control F11-F15 are on PC Ctrl-Alt-Shift-F1 through Ctrl-Alt-Shift-F15.
        */
    insertkeymap( tt, 368 , mkkeyevt(F_KVERB | K_DGF01 )) || /* F1               F1               Top Rank F key */
    insertkeymap( tt, 369 , mkkeyevt(F_KVERB | K_DGF02 )) || /* F2               F2               Top Rank F key */
    insertkeymap( tt, 370 , mkkeyevt(F_KVERB | K_DGF03 )) || /* F3               F3               Top Rank F key */
    insertkeymap( tt, 371 , mkkeyevt(F_KVERB | K_DGF04 )) || /* F4               F4               Top Rank F key */
    insertkeymap( tt, 372 , mkkeyevt(F_KVERB | K_DGF05 )) || /* F5               F5               Top Rank F key */
    insertkeymap( tt, 373 , mkkeyevt(F_KVERB | K_DGF06 )) || /* F6               F6               Top Rank F key */
    insertkeymap( tt, 374 , mkkeyevt(F_KVERB | K_DGF07 )) || /* F7               F7               Top Rank F key */
    insertkeymap( tt, 375 , mkkeyevt(F_KVERB | K_DGF08 )) || /* F8               F8               Top Rank F key */
    insertkeymap( tt, 376 , mkkeyevt(F_KVERB | K_DGF09 )) || /* F9               F9               Top Rank F key */
    insertkeymap( tt, 377 , mkkeyevt(F_KVERB | K_DGF10 )) || /* F10              F10              Top Rank F key */
    insertkeymap( tt, 378 , mkkeyevt(F_KVERB | K_DGF11 )) || /* F11              F11              Top Rank F key */
    insertkeymap( tt, 379 , mkkeyevt(F_KVERB | K_DGF12 )) || /* F12              F12              Top Rank F key */

    insertkeymap( tt, 2416, mkkeyevt(F_KVERB | K_DGF11 )) || /* F11               Alt-F1           Top Rank F key */
    insertkeymap( tt, 2417, mkkeyevt(F_KVERB | K_DGF12 )) || /* F12               Alt-F2           Top Rank F key */
    insertkeymap( tt, 2418, mkkeyevt(F_KVERB | K_DGF13 )) || /* F13               Alt-F3           Top Rank F key */
    insertkeymap( tt, 2419, mkkeyevt(F_KVERB | K_DGF14 )) || /* F14               Alt-F4           Top Rank F key */
    insertkeymap( tt, 2420, mkkeyevt(F_KVERB | K_DGF15 )) || /* F15               Alt-F5           Top Rank F key */

    insertkeymap( tt, 4466, mkkeyevt(F_KVERB | K_DGF13 )) || /* F13               LK450 F13        Top Rank F key */
    insertkeymap( tt, 4467, mkkeyevt(F_KVERB | K_DGF14 )) || /* F14               LK450 F14        Top Rank F key */
    insertkeymap( tt, 4468, mkkeyevt(F_KVERB | K_DGF15 )) || /* F15               LK450 F15        Top Rank F key */

    insertkeymap( tt, 4479, mkkeyevt(F_KVERB | K_DGF13 )) || /* F13               LK450 F13        Top Rank F key */
    insertkeymap( tt, 4480, mkkeyevt(F_KVERB | K_DGF14 )) || /* F14               LK450 F14        Top Rank F key */
    insertkeymap( tt, 4481, mkkeyevt(F_KVERB | K_DGF15 )) || /* F15               LK450 F15        Top Rank F key */

    insertkeymap( tt, 880 , mkkeyevt(F_KVERB | K_DGSF01 )) || /* Shift-F1         Shift-F1         Top Rank F key */
    insertkeymap( tt, 881 , mkkeyevt(F_KVERB | K_DGSF02 )) || /* Shift-F2         Shift-F2         Top Rank F key */
    insertkeymap( tt, 882 , mkkeyevt(F_KVERB | K_DGSF03 )) || /* Shift-F3         Shift-F3         Top Rank F key */
    insertkeymap( tt, 883 , mkkeyevt(F_KVERB | K_DGSF04 )) || /* Shift-F4         Shift-F4         Top Rank F key */
    insertkeymap( tt, 884 , mkkeyevt(F_KVERB | K_DGSF05 )) || /* Shift-F5         Shift-F5         Top Rank F key */
    insertkeymap( tt, 885 , mkkeyevt(F_KVERB | K_DGSF06 )) || /* Shift-F6         Shift-F6         Top Rank F key */
    insertkeymap( tt, 886 , mkkeyevt(F_KVERB | K_DGSF07 )) || /* Shift-F7         Shift-F7         Top Rank F key */
    insertkeymap( tt, 887 , mkkeyevt(F_KVERB | K_DGSF08 )) || /* Shift-F8         Shift-F8         Top Rank F key */
    insertkeymap( tt, 888 , mkkeyevt(F_KVERB | K_DGSF09 )) || /* Shift-F9         Shift-F9         Top Rank F key */
    insertkeymap( tt, 889 , mkkeyevt(F_KVERB | K_DGSF10 )) || /* Shift-F10        Shift-F10        Top Rank F key */
    insertkeymap( tt, 890 , mkkeyevt(F_KVERB | K_DGSF11 )) || /* Shift-F11        Shift-F11        Top Rank F key */
    insertkeymap( tt, 891 , mkkeyevt(F_KVERB | K_DGSF12 )) || /* Shift-F12        Shift-F12        Top Rank F key */

    insertkeymap( tt, 2928, mkkeyevt(F_KVERB | K_DGSF11 )) || /* Shift-F11         Alt-Shift-F1     Top Rank F key */
    insertkeymap( tt, 2929, mkkeyevt(F_KVERB | K_DGSF12 )) || /* Shift-F12         Alt-Shift-F2     Top Rank F key */
    insertkeymap( tt, 2930, mkkeyevt(F_KVERB | K_DGSF13 )) || /* Shift-F13         Alt-Shift-F3     Top Rank F key */
    insertkeymap( tt, 2931, mkkeyevt(F_KVERB | K_DGSF14 )) || /* Shift-F14         Alt-Shift-F4     Top Rank F key */
    insertkeymap( tt, 2932, mkkeyevt(F_KVERB | K_DGSF15 )) || /* Shift-F15         Alt-Shift-F5     Top Rank F key */

    insertkeymap( tt, 4978, mkkeyevt(F_KVERB | K_DGSF13 )) || /* Shift-F13         LK450 Shift-F13  Top Rank F key */
    insertkeymap( tt, 4979, mkkeyevt(F_KVERB | K_DGSF14 )) || /* Shift-F14         LK450 Shift-F14  Top Rank F key */
    insertkeymap( tt, 4980, mkkeyevt(F_KVERB | K_DGSF15 )) || /* Shift-F15         LK450 Shift-F15  Top Rank F key */

    insertkeymap( tt, 4988, mkkeyevt(F_KVERB | K_DGSF13 )) || /* Shift-F13         LK450 Shift-F13  Top Rank F key */
    insertkeymap( tt, 4989, mkkeyevt(F_KVERB | K_DGSF14 )) || /* Shift-F14         LK450 Shift-F14  Top Rank F key */
    insertkeymap( tt, 4990, mkkeyevt(F_KVERB | K_DGSF15 )) || /* Shift-F15         LK450 Shift-F15  Top Rank F key */

    insertkeymap( tt, 1392, mkkeyevt(F_KVERB | K_DGCF01 )) || /* Ctrl-F1          Ctrl-F1           Top Rank F key */
    insertkeymap( tt, 1393, mkkeyevt(F_KVERB | K_DGCF02 )) || /* Ctrl-F2          Ctrl-F2           Top Rank F key */
    insertkeymap( tt, 1394, mkkeyevt(F_KVERB | K_DGCF03 )) || /* Ctrl-F3          Ctrl-F3           Top Rank F key */
    insertkeymap( tt, 1395, mkkeyevt(F_KVERB | K_DGCF04 )) || /* Ctrl-F4          Ctrl-F4           Top Rank F key */
    insertkeymap( tt, 1396, mkkeyevt(F_KVERB | K_DGCF05 )) || /* Ctrl-F5          Ctrl-F5           Top Rank F key */
    insertkeymap( tt, 1397, mkkeyevt(F_KVERB | K_DGCF06 )) || /* Ctrl-F6          Ctrl-F6           Top Rank F key */
    insertkeymap( tt, 1398, mkkeyevt(F_KVERB | K_DGCF07 )) || /* Ctrl-F7          Ctrl-F7           Top Rank F key */
    insertkeymap( tt, 1399, mkkeyevt(F_KVERB | K_DGCF08 )) || /* Ctrl-F8          Ctrl-F8           Top Rank F key */
    insertkeymap( tt, 1400, mkkeyevt(F_KVERB | K_DGCF09 )) || /* Ctrl-F9          Ctrl-F9           Top Rank F key */
    insertkeymap( tt, 1401, mkkeyevt(F_KVERB | K_DGCF10 )) || /* Ctrl-F10         Ctrl-F10          Top Rank F key */
    insertkeymap( tt, 1402, mkkeyevt(F_KVERB | K_DGCF11 )) || /* Ctrl-F11         Ctrl-F11          Top Rank F key */
    insertkeymap( tt, 1403, mkkeyevt(F_KVERB | K_DGCF12 )) || /* Ctrl-F12         Ctrl-F12          Top Rank F key */

    insertkeymap( tt, 3440, mkkeyevt(F_KVERB | K_DGCF11)) || /* Ctrl-F11          Ctrl-Alt-F1       Top Rank F key */
    insertkeymap( tt, 3441, mkkeyevt(F_KVERB | K_DGCF12)) || /* Ctrl-F12          Ctrl-Alt-F2       Top Rank F key */
    insertkeymap( tt, 3442, mkkeyevt(F_KVERB | K_DGCF13)) || /* Ctrl-F13          Ctrl-Alt-F3       Top Rank F key */
    insertkeymap( tt, 3443, mkkeyevt(F_KVERB | K_DGCF14)) || /* Ctrl-F14          Ctrl-Alt-F4       Top Rank F key */
    insertkeymap( tt, 3444, mkkeyevt(F_KVERB | K_DGCF15)) || /* Ctrl-F15          Ctrl-Alt-F5       Top Rank F key */

    insertkeymap( tt, 5490, mkkeyevt(F_KVERB | K_DGCF13)) || /* Ctrl-F13          LK450 Ctrl-F13    Top Rank F key */
    insertkeymap( tt, 5491, mkkeyevt(F_KVERB | K_DGCF14)) || /* Ctrl-F14          LK450 Ctrl-F14    Top Rank F key */
    insertkeymap( tt, 5492, mkkeyevt(F_KVERB | K_DGCF15)) || /* Ctrl-F15          LK450 Ctrl-F15    Top Rank F key */

    insertkeymap( tt, 5500, mkkeyevt(F_KVERB | K_DGCF13)) || /* Ctrl-F13          LK450 Ctrl-F13    Top Rank F key */
    insertkeymap( tt, 5501, mkkeyevt(F_KVERB | K_DGCF14)) || /* Ctrl-F14          LK450 Ctrl-F14    Top Rank F key */
    insertkeymap( tt, 5502, mkkeyevt(F_KVERB | K_DGCF15)) || /* Ctrl-F15          LK450 Ctrl-F15    Top Rank F key */

    insertkeymap( tt, 1904, mkkeyevt(F_KVERB | K_DGCSF01)) || /* Ctrl-Shift-F1    Ctrl-Shift-F1     Top Rank F key */
    insertkeymap( tt, 1905, mkkeyevt(F_KVERB | K_DGCSF02)) || /* Ctrl-Shift-F2    Ctrl-Shift-F2     Top Rank F key */
    insertkeymap( tt, 1906, mkkeyevt(F_KVERB | K_DGCSF03)) || /* Ctrl-Shift-F3    Ctrl-Shift-F3     Top Rank F key */
    insertkeymap( tt, 1907, mkkeyevt(F_KVERB | K_DGCSF04)) || /* Ctrl-Shift-F4    Ctrl-Shift-F4     Top Rank F key */
    insertkeymap( tt, 1908, mkkeyevt(F_KVERB | K_DGCSF05)) || /* Ctrl-Shift-F5    Ctrl-Shift-F5     Top Rank F key */
    insertkeymap( tt, 1909, mkkeyevt(F_KVERB | K_DGCSF06)) || /* Ctrl-Shift-F6    Ctrl-Shift-F6     Top Rank F key */
    insertkeymap( tt, 1910, mkkeyevt(F_KVERB | K_DGCSF07)) || /* Ctrl-Shift-F7    Ctrl-Shift-F7     Top Rank F key */
    insertkeymap( tt, 1911, mkkeyevt(F_KVERB | K_DGCSF08)) || /* Ctrl-Shift-F8    Ctrl-Shift-F8     Top Rank F key */
    insertkeymap( tt, 1912, mkkeyevt(F_KVERB | K_DGCSF09)) || /* Ctrl-Shift-F9    Ctrl-Shift-F9     Top Rank F key */
    insertkeymap( tt, 1913, mkkeyevt(F_KVERB | K_DGCSF10)) || /* Ctrl-Shift-F10   Ctrl-Shift-F10    Top Rank F key */
    insertkeymap( tt, 1914, mkkeyevt(F_KVERB | K_DGCSF11)) || /* Ctrl-Shift-F11   Ctrl-Shift-F11    Top Rank F key */
    insertkeymap( tt, 1915, mkkeyevt(F_KVERB | K_DGCSF12)) || /* Ctrl-Shift-F12   Ctrl-Shift-F12    Top Rank F key */

    insertkeymap( tt, 3952, mkkeyevt(F_KVERB | K_DGCSF11)) || /* Ctrl-Shift-F11    Ctrl-Alt-Shift-F1 Top Rank F key */
    insertkeymap( tt, 3953, mkkeyevt(F_KVERB | K_DGCSF12)) || /* Ctrl-Shift-F12    Ctrl-Alt-Shift-F2 Top Rank F key */
    insertkeymap( tt, 3954, mkkeyevt(F_KVERB | K_DGCSF13)) || /* Ctrl-Shift-F13    Ctrl-Alt-Shift-F3 Top Rank F key */
    insertkeymap( tt, 3955, mkkeyevt(F_KVERB | K_DGCSF14)) || /* Ctrl-Shift-F14    Ctrl-Alt-Shift-F4 Top Rank F key */
    insertkeymap( tt, 3956, mkkeyevt(F_KVERB | K_DGCSF15)) ||  /* Ctrl-Shift-F15    Ctrl-Alt-Shift-F5 Top Rank F key */

    insertkeymap( tt, 6002, mkkeyevt(F_KVERB | K_DGCSF13)) || /* Ctrl-Shift-F13    LK450 Ctrl-Shift-F13 Top Rank F key */
    insertkeymap( tt, 6003, mkkeyevt(F_KVERB | K_DGCSF14)) || /* Ctrl-Shift-F14    LK450 Ctrl-Shift-F14 Top Rank F key */
    insertkeymap( tt, 6004, mkkeyevt(F_KVERB | K_DGCSF15)) ||  /* Ctrl-Shift-F15    LK450 Ctrl-Shift-F15 Top Rank F key */

    insertkeymap( tt, 6012, mkkeyevt(F_KVERB | K_DGCSF13)) || /* Ctrl-Shift-F13    LK450 Ctrl-Shift-F13 Top Rank F key */
    insertkeymap( tt, 6013, mkkeyevt(F_KVERB | K_DGCSF14)) || /* Ctrl-Shift-F14    LK450 Ctrl-Shift-F14 Top Rank F key */
    insertkeymap( tt, 6014, mkkeyevt(F_KVERB | K_DGCSF15)) ||  /* Ctrl-Shift-F15    LK450 Ctrl-Shift-F15 Top Rank F key */

    insertkeymap( tt, 269,  mkkeyevt(LF )) ||   /* Enter */
    insertkeymap( tt, 264,  mkkeyevt(F_KVERB | K_DGBS)) ||      /* Backspace */
    insertkeymap( tt, 776,  mkkeyevt(F_KVERB | K_DGBS )) ||     /* Shift-Backspace */
    insertkeymap( tt, 265,  mkkeyevt(HT)) ||    /* Tab                  Tab */
    insertkeymap( tt, 4365, mkkeyevt(LF)) ||    /* Keypad-Enter         Gray-Enter */

    insertkeymap( tt, 4397, mkkeyevt(F_KVERB | K_DGERASEPAGE)) ||       /* ErasePage    Gray-Insert */
    insertkeymap( tt, 4388, mkkeyevt(F_KVERB | K_DGC1)) ||              /* C1           Gray-Home */
    insertkeymap( tt, 4385, mkkeyevt(F_KVERB | K_DGC2)) ||              /* C2           Gray-PgUp */
    insertkeymap( tt, 4398, mkkeyevt(F_KVERB | K_DGERASEEOL)) ||        /* EraseEOL     Gray-Delete */
    insertkeymap( tt, 4387, mkkeyevt(F_KVERB | K_DGC3)) ||              /* C3           Gray-End */
    insertkeymap( tt, 4386, mkkeyevt(F_KVERB | K_DGC4)) ||              /* C4           Gray-PgDn */

    insertkeymap( tt, 4909, mkkeyevt(F_KVERB | K_DGSERASEPAGE)) ||      /* Shift-ErasePage    Gray-Shift-Insert */
    insertkeymap( tt, 4900, mkkeyevt(F_KVERB | K_DGSC1)) ||             /* Shift-C1           Gray-Shift-Home */
    insertkeymap( tt, 4897, mkkeyevt(F_KVERB | K_DGSC2)) ||             /* Shift-C2           Gray-Shift-PgUp */
    insertkeymap( tt, 4910, mkkeyevt(F_KVERB | K_DGSERASEEOL)) ||       /* Shift-EraseEOL     Gray-Shift-Delete */
    insertkeymap( tt, 4899, mkkeyevt(F_KVERB | K_DGSC3)) ||             /* Shift-C3           Gray-Shift-End */
    insertkeymap( tt, 4898, mkkeyevt(F_KVERB | K_DGSC4)) ||             /* Shift-C4           Gray-Shift-PgDn */

    insertkeymap( tt, 301, mkkeyevt(F_KVERB | K_DGERASEPAGE)) ||        /* ErasePage    Insert */
    insertkeymap( tt, 292, mkkeyevt(F_KVERB | K_DGC1)) ||               /* C1           Home */
    insertkeymap( tt, 289, mkkeyevt(F_KVERB | K_DGC2)) ||               /* C2           PgUp */
    insertkeymap( tt, 302, mkkeyevt(F_KVERB | K_DGERASEEOL)) ||         /* EraseEOL     Delete */
    insertkeymap( tt, 291, mkkeyevt(F_KVERB | K_DGC3)) ||               /* C3           End */
    insertkeymap( tt, 290, mkkeyevt(F_KVERB | K_DGC4)) ||               /* C4           PgDn */
    insertkeymap( tt, 268, mkkeyevt(F_KVERB | K_DGHOME)) ||             /* Home         Clear */

    insertkeymap( tt, 813, mkkeyevt(F_KVERB | K_DGSERASEPAGE)) ||       /* Shift-ErasePage    Gray-Shift-Insert */
    insertkeymap( tt, 804, mkkeyevt(F_KVERB | K_DGSC1)) ||              /* Shift-C1           Gray-Shift-Home */
    insertkeymap( tt, 801, mkkeyevt(F_KVERB | K_DGSC2)) ||              /* Shift-C2           Gray-Shift-PgUp */
    insertkeymap( tt, 814, mkkeyevt(F_KVERB | K_DGSERASEEOL)) ||        /* Shift-EraseEOL     Gray-Shift-Delete */
    insertkeymap( tt, 803, mkkeyevt(F_KVERB | K_DGSC3)) ||              /* Shift-C3           Gray-Shift-End */
    insertkeymap( tt, 802, mkkeyevt(F_KVERB | K_DGSC4)) ||              /* Shift-C4           Gray-Shift-PgDn */
    insertkeymap( tt, 780, mkkeyevt(F_KVERB | K_DGSHOME))             /* Shift-Home     Shift-Clear */
         )
        return -1;
    return 0;
}

int
defvckm( int tt )
{
    if ( defbasekm(tt) ||
        /* Scan  Kverb                     VC Key           PC Key           PC Key Group */

        /*
        VC Cursor keys
        */
    insertkeymap( tt, 4390, mkkeyevt(F_KVERB | K_UPARR )) || /* Up Arrow           Up Arrow         Cursor keypad */
    insertkeymap( tt, 4392, mkkeyevt(F_KVERB | K_DNARR )) || /* Down Arrow         Down Arrow       Cursor keypad */
    insertkeymap( tt, 4391, mkkeyevt(F_KVERB | K_RTARR )) || /* Right Arrow        Right Arrow      Cursor keypad */
    insertkeymap( tt, 4389, mkkeyevt(F_KVERB | K_LFARR )) || /* Left Arrow         Left Arrow       Cursor keypad */
        /*
        VC Function keys.

          VC F1-F10 are on PC F1-F10.
        */
    insertkeymap( tt, 368 , mkkeyevt(SOH)) || /* F1               F1               Top Rank F key */
    insertkeymap( tt, 369 , mkkeyevt(STX)) || /* F2               F2               Top Rank F key */
    insertkeymap( tt, 370 , mkkeyevt(ETX)) || /* F3               F3               Top Rank F key */
    insertkeymap( tt, 371 , mkkeyevt(EOT)) || /* F4               F4               Top Rank F key */
    insertkeymap( tt, 372 , mkkeyevt(ENQ)) || /* F5               F5               Top Rank F key */
    insertkeymap( tt, 373 , mkkeyevt(ACK)) || /* F6               F6               Top Rank F key */
    insertkeymap( tt, 374 , mkkeyevt(DC2)) || /* F7               F7               Top Rank F key */
    insertkeymap( tt, 375 , mkkeyevt(DC4)) || /* F8               F8               Top Rank F key */
    insertkeymap( tt, 376 , mkkeyevt(XFS)) || /* F9               F9               Top Rank F key */
    insertkeymap( tt, 377 , mkkeyevt(XGS)) || /* F10              F10              Top Rank F key */
    insertkeymap( tt, 378 , mkkeyevt(XRS)) || /* F11              F11              Top Rank F key */
    insertkeymap( tt, 379 , mkkeyevt(US))  || /* F12              F12              Top Rank F key */

    insertkeymap( tt, 264, mkkeyevt(BS )) ||            /* Backspace */
    insertkeymap( tt, 777, mkkeyevt(F_ESC | HT )) ||    /* Shift-Tab */
    insertkeymap( tt, 265, mkkeyevt(HT )) ||            /* Tab */
    insertkeymap( tt, 4398, mkkeyevt(F_ESC | 'd' )) ||  /* Del */
    insertkeymap( tt, 4397, mkkeyevt(F_ESC | 'i' ))     /* Ins          Gray-Insert */
         )
        return -1;
    return 0;
}

int
defhpkm( int tt )
{
    if ( defbasekm(tt) ||
        /* Scan  Kverb                     HP Key           PC Key           PC Key Group */

        /*
        HP Cursor keys
        */
    insertkeymap( tt, 4390, mkkeyevt(F_KVERB | K_UPARR )) || /* Up Arrow           Up Arrow         Cursor keypad */
    insertkeymap( tt, 4392, mkkeyevt(F_KVERB | K_DNARR )) || /* Down Arrow         Down Arrow       Cursor keypad */
    insertkeymap( tt, 4391, mkkeyevt(F_KVERB | K_RTARR )) || /* Right Arrow        Right Arrow      Cursor keypad */
    insertkeymap( tt, 4389, mkkeyevt(F_KVERB | K_LFARR )) || /* Left Arrow         Left Arrow       Cursor keypad */
        /*
        HP Function keys.

          HP F1-F8 are on PC F1-F8.
        */
    insertkeymap( tt, 368 , mkkeyevt(F_KVERB | K_HPF01)) || /* F1               F1               Top Rank F key */
    insertkeymap( tt, 369 , mkkeyevt(F_KVERB | K_HPF02)) || /* F2               F2               Top Rank F key */
    insertkeymap( tt, 370 , mkkeyevt(F_KVERB | K_HPF03)) || /* F3               F3               Top Rank F key */
    insertkeymap( tt, 371 , mkkeyevt(F_KVERB | K_HPF04)) || /* F4               F4               Top Rank F key */
    insertkeymap( tt, 372 , mkkeyevt(F_KVERB | K_HPF05)) || /* F5               F5               Top Rank F key */
    insertkeymap( tt, 373 , mkkeyevt(F_KVERB | K_HPF06)) || /* F6               F6               Top Rank F key */
    insertkeymap( tt, 374 , mkkeyevt(F_KVERB | K_HPF07)) || /* F7               F7               Top Rank F key */
    insertkeymap( tt, 375 , mkkeyevt(F_KVERB | K_HPF08)) || /* F8               F8               Top Rank F key */
    insertkeymap( tt, 376 , mkkeyevt(F_KVERB | K_HPF09)) || /* F1               F1               Top Rank F key */
    insertkeymap( tt, 377 , mkkeyevt(F_KVERB | K_HPF10)) || /* F2               F2               Top Rank F key */
    insertkeymap( tt, 378 , mkkeyevt(F_KVERB | K_HPF11)) || /* F3               F3               Top Rank F key */
    insertkeymap( tt, 379 , mkkeyevt(F_KVERB | K_HPF12)) || /* F4               F4               Top Rank F key */
    insertkeymap( tt, 880 , mkkeyevt(F_KVERB | K_HPF09)) || /* F1               F1               Top Rank F key */
    insertkeymap( tt, 881 , mkkeyevt(F_KVERB | K_HPF10)) || /* F2               F2               Top Rank F key */
    insertkeymap( tt, 882 , mkkeyevt(F_KVERB | K_HPF11)) || /* F3               F3               Top Rank F key */
    insertkeymap( tt, 883 , mkkeyevt(F_KVERB | K_HPF12)) || /* F4               F4               Top Rank F key */
    insertkeymap( tt, 884 , mkkeyevt(F_KVERB | K_HPF13)) || /* F5               F5               Top Rank F key */
    insertkeymap( tt, 885 , mkkeyevt(F_KVERB | K_HPF14)) || /* F6               F6               Top Rank F key */
    insertkeymap( tt, 886 , mkkeyevt(F_KVERB | K_HPF15)) || /* F7               F7               Top Rank F key */
    insertkeymap( tt, 887 , mkkeyevt(F_KVERB | K_HPF16)) || /* F8               F8               Top Rank F key */

    insertkeymap( tt, 269,  mkkeyevt(F_KVERB | K_HPRETURN))   || /* Return */
    insertkeymap( tt, 4365, mkkeyevt(F_KVERB | K_HPENTER))    || /* Kp Enter */
    insertkeymap( tt, 777,  mkkeyevt(F_KVERB | K_HPBACKTAB))  || /* Back Tab */
    insertkeymap( tt, 264,  mkkeyevt(BS )) ||                /* Backspace */
    insertkeymap( tt, 265,  mkkeyevt(HT )) ||                /* Tab */
    insertkeymap( tt, 4398, mkkeyevt(DEL))                   /* Del */
         )
        return -1;
    return 0;
}

int
defbetermkm( int tt )
{
    defbasekm(tt);

    insertkeymap( tt, 368 , mkliteralevt("\033[11~")); /* F1   */
    insertkeymap( tt, 369 , mkliteralevt("\033[12~")); /* F2   */
    insertkeymap( tt, 370 , mkliteralevt("\033[13~")); /* F3   */
    insertkeymap( tt, 371 , mkliteralevt("\033[14~")); /* F4   */
    insertkeymap( tt, 372 , mkliteralevt("\033[15~")); /* F5   */
    insertkeymap( tt, 373 , mkliteralevt("\033[16~")); /* F6   */
    insertkeymap( tt, 374 , mkliteralevt("\033[17~")); /* F7   */
    insertkeymap( tt, 375 , mkliteralevt("\033[18~")); /* F8   */
    insertkeymap( tt, 376 , mkliteralevt("\033[19~")); /* F9   */
    insertkeymap( tt, 377 , mkliteralevt("\033[20~")); /* F10  */
    insertkeymap( tt, 378 , mkliteralevt("\033[21~")); /* F11  */
    insertkeymap( tt, 379 , mkliteralevt("\033[22~")); /* F12  */
    insertkeymap( tt, 264,  mkkeyevt(BS )); /* Backspace */
    insertkeymap( tt, 265,  mkkeyevt(HT )); /* Tab */
    insertkeymap( tt, 777,  mkkeyevt(HT));  /* Back Tab */
    insertkeymap( tt, 4398, mkkeyevt(DEL)); /* Del */

    insertkeymap( tt, 4388, mkliteralevt("\033[1~"));  /* home         */
    insertkeymap( tt, 4385, mkliteralevt("\033[5~")); /* page up               */
    insertkeymap( tt, 4387, mkliteralevt("\033[4~"));  /* end          */
    insertkeymap( tt, 4386, mkliteralevt("\033[6~")); /* page down             */
    insertkeymap( tt, 4397, mkliteralevt("\033[2~"));  /* insert               */

    insertkeymap( tt, 268,  mkliteralevt("\033[G"));  /* Clear key */
    insertkeymap( tt, 301, mkkeyevt(ENQ));
    insertkeymap( tt, 302, mkkeyevt(DEL));
    insertkeymap( tt, 291, mkkeyevt(EOT));
    insertkeymap( tt, 296, mkkeyevt(US));
    insertkeymap( tt, 290, mkkeyevt(FF));
    insertkeymap( tt, 293, mkkeyevt(XFS));
    insertkeymap( tt, 295, mkkeyevt(XGS));
    insertkeymap( tt, 292, mkkeyevt(SOH));
    insertkeymap( tt, 294, mkkeyevt(XRS));
    insertkeymap( tt, 289, mkkeyevt(VT));

    return 0;
}

int
defhzlkm( int tt )
{
    if ( defbasekm(tt) ||
        /* Scan  Kverb                     VC Key           PC Key           PC Key Group */

        /*
        VC Cursor keys
        */
    insertkeymap( tt, 4390, mkkeyevt(F_KVERB | K_UPARR )) || /* Up Arrow           Up Arrow         Cursor keypad */
    insertkeymap( tt, 4392, mkkeyevt(F_KVERB | K_DNARR )) || /* Down Arrow         Down Arrow       Cursor keypad */
    insertkeymap( tt, 4391, mkkeyevt(F_KVERB | K_RTARR )) || /* Right Arrow        Right Arrow      Cursor keypad */
    insertkeymap( tt, 4389, mkkeyevt(F_KVERB | K_LFARR )) || /* Left Arrow         Left Arrow       Cursor keypad */
        /*
        VC Function keys.

          VC F1-F10 are on PC F1-F10.
        */
    insertkeymap( tt, 368 , mkkeyevt(SOH)) || /* F1               F1               Top Rank F key */
    insertkeymap( tt, 369 , mkkeyevt(STX)) || /* F2               F2               Top Rank F key */
    insertkeymap( tt, 370 , mkkeyevt(ETX)) || /* F3               F3               Top Rank F key */
    insertkeymap( tt, 371 , mkkeyevt(EOT)) || /* F4               F4               Top Rank F key */
    insertkeymap( tt, 372 , mkkeyevt(ENQ)) || /* F5               F5               Top Rank F key */
    insertkeymap( tt, 373 , mkkeyevt(ACK)) || /* F6               F6               Top Rank F key */
    insertkeymap( tt, 374 , mkkeyevt(DC2)) || /* F7               F7               Top Rank F key */
    insertkeymap( tt, 375 , mkkeyevt(DC4)) || /* F8               F8               Top Rank F key */
    insertkeymap( tt, 376 , mkkeyevt(XFS)) || /* F9               F9               Top Rank F key */
    insertkeymap( tt, 377 , mkkeyevt(XGS)) || /* F10              F10              Top Rank F key */
    insertkeymap( tt, 378 , mkkeyevt(XRS)) || /* F11              F11              Top Rank F key */
    insertkeymap( tt, 379 , mkkeyevt(US))  || /* F12              F12              Top Rank F key */

    insertkeymap( tt, 264,  mkkeyevt(BS)) ||            /* Backspace */
    insertkeymap( tt, 777,  mkliteralevt("~\x09")) ||   /* Shift-Tab */
    insertkeymap( tt, 265,  mkkeyevt(HT)) ||            /* Tab */
    insertkeymap( tt, 4398, mkkeyevt(STX)) ||           /* Del */
    insertkeymap( tt, 4397, mkkeyevt(NAK)) ||           /* Ins          Gray-Insert */
    insertkeymap( tt, 4388, mkliteralevt("~\x12")) ||   /* Home                 Gray-Home */
    insertkeymap( tt, 4387, mkliteralevt("~\x0F")) ||   /* End          Gray-End */
    insertkeymap( tt, 4385, mkkeyevt(ENQ)) ||           /* PgUp                 Gray-PgUp */
    insertkeymap( tt, 4386, mkkeyevt(SOH)) ||           /* PgDn                 Gray-PgDn */

    insertkeymap( tt, 302, mkkeyevt(DEL)) ||            /* KpDel                Kp-Del */
    insertkeymap( tt, 301, mkkeyevt(EOT))               /* KpIns                Kp-Ins */
         )
        return -1;
    return 0;
}

int
defwyseasciikm( int tt )
{
    if ( defbasekm(tt) ||
         /*
        ; WYSE Cursor keys
        */
    insertkeymap( tt, 4390, mkkeyevt(F_KVERB | K_UPARR )) || /* Up Arrow           Up Arrow         Cursor keypad */
    insertkeymap( tt, 4392, mkkeyevt(F_KVERB | K_DNARR )) || /* Down Arrow         Down Arrow       Cursor keypad */
    insertkeymap( tt, 4391, mkkeyevt(F_KVERB | K_RTARR )) || /* Right Arrow        Right Arrow      Cursor keypad */
    insertkeymap( tt, 4389, mkkeyevt(F_KVERB | K_LFARR )) || /* Left Arrow         Left Arrow       Cursor keypad */

    insertkeymap( tt, 294, mkkeyevt(F_KVERB | K_UPARR )) || /* Kp-Up Arrow     */
    insertkeymap( tt, 296, mkkeyevt(F_KVERB | K_DNARR )) || /* Kp-Down Arrow   */
    insertkeymap( tt, 295, mkkeyevt(F_KVERB | K_RTARR )) || /* Kp-Right Arrow  */
    insertkeymap( tt, 293, mkkeyevt(F_KVERB | K_LFARR )) || /* Kp-Left Arrow   */

        /*
        ) || WYSE Function keys, WY30, WY50 and WY60 and above.
        ) ||
        ) ||   WYSE F1-F10 are on PC F1-F10.
        ) ||   WYSE F11-F16 are on PC Alt-F1 through Alt-F6.
        ) ||   WYSE Shift F1-F10 are on PC Shift-F1 - ShiftF10
        ) ||   WYSE Shift F11-F16 are on PC Alt-Shift-F1 through Alt-Shift-F16.
        */
    insertkeymap( tt, 368 , mkkeyevt(F_KVERB | K_WYF01  )) || /* F1                 F1              Top Rank F key */
    insertkeymap( tt, 369 , mkkeyevt(F_KVERB | K_WYF02  )) || /* F2                 F2              Top Rank F key */
    insertkeymap( tt, 370 , mkkeyevt(F_KVERB | K_WYF03  )) || /* F3                 F3              Top Rank F key */
    insertkeymap( tt, 371 , mkkeyevt(F_KVERB | K_WYF04  )) || /* F4                 F4              Top Rank F key */
    insertkeymap( tt, 372 , mkkeyevt(F_KVERB | K_WYF05  )) || /* F5                 F5              Top Rank F key */
    insertkeymap( tt, 373 , mkkeyevt(F_KVERB | K_WYF06  )) || /* F6                 F6              Top Rank F key */
    insertkeymap( tt, 374 , mkkeyevt(F_KVERB | K_WYF07  )) || /* F7                 F7              Top Rank F key */
    insertkeymap( tt, 375 , mkkeyevt(F_KVERB | K_WYF08  )) || /* F8                 F8              Top Rank F key */
    insertkeymap( tt, 376 , mkkeyevt(F_KVERB | K_WYF09  )) || /* F9                 F9              Top Rank F key */
    insertkeymap( tt, 377 , mkkeyevt(F_KVERB | K_WYF10  )) || /* F10                F10             Top Rank F key */
    insertkeymap( tt, 378 , mkkeyevt(F_KVERB | K_WYF11 )) || /* F11                 F11             Top Rank F key */
    insertkeymap( tt, 379 , mkkeyevt(F_KVERB | K_WYF12 )) || /* F12                 F12             Top Rank F key */

    insertkeymap( tt, 2416, mkkeyevt(F_KVERB | K_WYF11 )) || /* F11                Alt-F1           Top Rank F key */
    insertkeymap( tt, 2417, mkkeyevt(F_KVERB | K_WYF12 )) || /* F12                Alt-F2           Top Rank F key */
    insertkeymap( tt, 2418, mkkeyevt(F_KVERB | K_WYF13 )) || /* F13                Alt-F3           Top Rank F key */
    insertkeymap( tt, 2419, mkkeyevt(F_KVERB | K_WYF14 )) || /* F14                Alt-F4           Top Rank F key */
    insertkeymap( tt, 2420, mkkeyevt(F_KVERB | K_WYF15 )) || /* F15                Alt-F5           Top Rank F key */
    insertkeymap( tt, 2421, mkkeyevt(F_KVERB | K_WYF16 )) || /* F16                Alt-F6           Top Rank F key */

    insertkeymap( tt, 4466, mkkeyevt(F_KVERB | K_WYF13 )) || /* F13                LK450 F13        Top Rank F key */
    insertkeymap( tt, 4467, mkkeyevt(F_KVERB | K_WYF14 )) || /* F14                LK450 F14        Top Rank F key */
    insertkeymap( tt, 4468, mkkeyevt(F_KVERB | K_WYF15 )) || /* F15                LK450 F15        Top Rank F key */
    insertkeymap( tt, 4469, mkkeyevt(F_KVERB | K_WYF16 )) || /* F16                LK450 F16        Top Rank F key */

    insertkeymap( tt, 4476, mkkeyevt(F_KVERB | K_WYF13 )) || /* F13                LK450 F13        Top Rank F key */
    insertkeymap( tt, 4477, mkkeyevt(F_KVERB | K_WYF14 )) || /* F14                LK450 F14        Top Rank F key */
    insertkeymap( tt, 4478, mkkeyevt(F_KVERB | K_WYF15 )) || /* F15                LK450 F15        Top Rank F key */
    insertkeymap( tt, 4479, mkkeyevt(F_KVERB | K_WYF16 )) || /* F16                LK450 F16        Top Rank F key */

    insertkeymap( tt, 880 , mkkeyevt(F_KVERB | K_WYSF01  )) || /* Shift-F1         Shift-F1         Top Rank F key */
    insertkeymap( tt, 881 , mkkeyevt(F_KVERB | K_WYSF02  )) || /* Shift-F2         Shift-F2         Top Rank F key */
    insertkeymap( tt, 882 , mkkeyevt(F_KVERB | K_WYSF03  )) || /* Shift-F3         Shift-F3         Top Rank F key */
    insertkeymap( tt, 883 , mkkeyevt(F_KVERB | K_WYSF04  )) || /* Shift-F4         Shift-F4         Top Rank F key */
    insertkeymap( tt, 884 , mkkeyevt(F_KVERB | K_WYSF05  )) || /* Shift-F5         Shift-F5         Top Rank F key */
    insertkeymap( tt, 885 , mkkeyevt(F_KVERB | K_WYSF06  )) || /* Shift-F6         Shift-F6         Top Rank F key */
    insertkeymap( tt, 886 , mkkeyevt(F_KVERB | K_WYSF07  )) || /* Shift-F7         Shift-F7         Top Rank F key */
    insertkeymap( tt, 887 , mkkeyevt(F_KVERB | K_WYSF08  )) || /* Shift-F8         Shift-F8         Top Rank F key */
    insertkeymap( tt, 888 , mkkeyevt(F_KVERB | K_WYSF09  )) || /* Shift-F9         Shift-F9         Top Rank F key */
    insertkeymap( tt, 889 , mkkeyevt(F_KVERB | K_WYSF10  )) || /* Shift-F10        Shift-F10        Top Rank F key */
    insertkeymap( tt, 890 , mkkeyevt(F_KVERB | K_WYSF11  )) || /* Shift-F11        Shift-F11     Top Rank F key */
    insertkeymap( tt, 891 , mkkeyevt(F_KVERB | K_WYSF12  )) || /* Shift-F12        Shift-F12     Top Rank F key */

    insertkeymap( tt, 2928, mkkeyevt(F_KVERB | K_WYSF11 )) || /* Shift-F11         Alt-Shift-F1     Top Rank F key */
    insertkeymap( tt, 2929, mkkeyevt(F_KVERB | K_WYSF12 )) || /* Shift-F12         Alt-Shift-F2     Top Rank F key */
    insertkeymap( tt, 2930, mkkeyevt(F_KVERB | K_WYSF13 )) || /* Shift-F13         Alt-Shift-F3     Top Rank F key */
    insertkeymap( tt, 2931, mkkeyevt(F_KVERB | K_WYSF14 )) || /* Shift-F14         Alt-Shift-F4     Top Rank F key */
    insertkeymap( tt, 2932, mkkeyevt(F_KVERB | K_WYSF15 )) || /* Shift-F15         Alt-Shift-F5     Top Rank F key */
    insertkeymap( tt, 2933, mkkeyevt(F_KVERB | K_WYSF16 )) || /* Shift-F16         Alt-Shift-F6     Top Rank F key */

    insertkeymap( tt, 4978, mkkeyevt(F_KVERB | K_WYSF13 )) || /* Shift-F13         LK450 Shift-F13  Top Rank F key */
    insertkeymap( tt, 4979, mkkeyevt(F_KVERB | K_WYSF14 )) || /* Shift-F14         LK450 Shift-F14  Top Rank F key */
    insertkeymap( tt, 4980, mkkeyevt(F_KVERB | K_WYSF15 )) || /* Shift-F15         LK450 Shift-F15  Top Rank F key */
    insertkeymap( tt, 4981, mkkeyevt(F_KVERB | K_WYSF16 )) || /* Shift-F16         LK450 Shift-F16  Top Rank F key */

    insertkeymap( tt, 4988, mkkeyevt(F_KVERB | K_WYSF13 )) || /* Shift-F13         LK450 Shift-F13  Top Rank F key */
    insertkeymap( tt, 4989, mkkeyevt(F_KVERB | K_WYSF14 )) || /* Shift-F14         LK450 Shift-F14  Top Rank F key */
    insertkeymap( tt, 4990, mkkeyevt(F_KVERB | K_WYSF15 )) || /* Shift-F15         LK450 Shift-F15  Top Rank F key */
    insertkeymap( tt, 4991, mkkeyevt(F_KVERB | K_WYSF16 )) || /* Shift-F16         LK450 Shift-F16  Top Rank F key */

    insertkeymap( tt, 264,  mkkeyevt(F_KVERB | K_WYBS    )) ||  /* Backspace */
    insertkeymap( tt, 776,  mkkeyevt(F_KVERB | K_WYSBS   )) ||  /* Shift-Backspace */
    insertkeymap( tt, 27,   mkkeyevt(F_KVERB | K_WYESC   )) ||  /* Esc */
    insertkeymap( tt, 539,  mkkeyevt(F_KVERB | K_WYSESC   )) ||  /* Shift-Esc */
    insertkeymap( tt, 302,  mkkeyevt(DEL )) ||              /* Kp-Del */
    insertkeymap( tt, 4398, mkkeyevt(F_KVERB | K_WYDELCHAR )) ||        /* Gray-Del */
    insertkeymap( tt, 4910, mkkeyevt(F_KVERB | K_WYDELLN )) || /* Shift-Gray-Del */
    insertkeymap( tt, 4387, mkkeyevt(F_KVERB | K_WYCLRLN )) || /* Gray-End */
    insertkeymap( tt, 291,  mkkeyevt(F_KVERB | K_WYCLRLN )) || /* End */
    insertkeymap( tt, 4899, mkkeyevt(F_KVERB | K_WYCLRPG )) || /* Shift-Gray-End */
    insertkeymap( tt, 4365, mkkeyevt(F_KVERB | K_WYENTER )) || /* Gray-Enter */
    insertkeymap( tt, 4877, mkkeyevt(F_KVERB | K_WYSENTER )) || /* Shift-Gray-Enter */
    insertkeymap( tt, 269,  mkkeyevt(F_KVERB | K_WYRETURN )) || /* Enter */
    insertkeymap( tt, 781,  mkkeyevt(F_KVERB | K_WYSRETURN )) || /* Shift-Enter */
    insertkeymap( tt, 292,  mkkeyevt(F_KVERB | K_WYHOME )) ||  /* Kp-Home */
    insertkeymap( tt, 804,  mkkeyevt(F_KVERB | K_WYSHOME )) || /* Shift-Kp-Home */
    insertkeymap( tt, 4388, mkkeyevt(F_KVERB | K_WYHOME )) ||  /* Gray-Home */
    insertkeymap( tt, 4900, mkkeyevt(F_KVERB | K_WYSHOME )) || /* Shift-Gray-Home */
    insertkeymap( tt, 301,  mkkeyevt(F_KVERB | K_WYREPLACE )) || /* Kp-Ins */
    insertkeymap( tt, 813,  mkkeyevt(F_KVERB | K_WYINSERT )) ||  /* Shift-Kp-Ins */
    insertkeymap( tt, 4397, mkkeyevt(F_KVERB | K_WYINSERT )) || /* Gray-Insert */
    insertkeymap( tt, 4909, mkkeyevt(F_KVERB | K_WYREPLACE )) ||  /* Shift-Gray-Insert */
    insertkeymap( tt, 290,  mkkeyevt(F_KVERB | K_WYPGNEXT )) ||  /* Kp-PgDn */
    insertkeymap( tt, 4386, mkkeyevt(F_KVERB | K_WYPGNEXT )) ||  /* Gray-PgDn */
    insertkeymap( tt, 289,  mkkeyevt(F_KVERB | K_WYPGPREV )) ||  /* Kp-PgUp */
    insertkeymap( tt, 4385, mkkeyevt(F_KVERB | K_WYPGPREV )) ||  /* Kp-PgUp */
    insertkeymap( tt, 265,  mkkeyevt(F_KVERB | K_WYTAB )) ||     /* Tab */
    insertkeymap( tt, 777,  mkkeyevt(F_KVERB | K_WYSTAB ))    /* Shift-Tab */
    )
        return -1;
    return 0;
}

int
deftviasciikm(int tt)
{
    if ( defbasekm(tt) ||
        /*
        ; Televideo Cursor keys
        */
    insertkeymap( tt, 4390, mkkeyevt(F_KVERB | K_UPARR )) || /* Up Arrow       */
    insertkeymap( tt, 4392, mkkeyevt(F_KVERB | K_DNARR )) || /* Down Arrow     */
    insertkeymap( tt, 4391, mkkeyevt(F_KVERB | K_RTARR )) || /* Right Arrow    */
    insertkeymap( tt, 4389, mkkeyevt(F_KVERB | K_LFARR )) || /* Left Arrow     */

    insertkeymap( tt, 4902, mkkeyevt(F_ESC | 'j' )) ||  /* Shift-Gray-UpArrow */
    insertkeymap( tt, 4904, mkkeyevt(LF )) ||           /* Shift-Gray-DnArrow */
    insertkeymap( tt, 4903, mkkeyevt(FF )) ||           /* Shift-Gray-RightArrow */
    insertkeymap( tt, 4901, mkkeyevt(BS )) ||           /* Shift-Gray-LeftArrow */

    insertkeymap( tt, 294, mkkeyevt(F_KVERB | K_UPARR )) || /* Kp-Up Arrow     */
    insertkeymap( tt, 296, mkkeyevt(F_KVERB | K_DNARR )) || /* Kp-Down Arrow   */
    insertkeymap( tt, 295, mkkeyevt(F_KVERB | K_RTARR )) || /* Kp-Right Arrow  */
    insertkeymap( tt, 293, mkkeyevt(F_KVERB | K_LFARR )) || /* Kp-Left Arrow   */

    insertkeymap( tt, 806, mkkeyevt(F_ESC | 'j' )) ||   /* Shift-Kp-UpArrow */
    insertkeymap( tt, 808, mkkeyevt(LF )) ||            /* Shift-Kp-DnArrow */
    insertkeymap( tt, 807, mkkeyevt(FF )) ||            /* Shift-Kp-RightArrow */
    insertkeymap( tt, 805, mkkeyevt(BS )) ||            /* Shift-Kp-LeftArrow */

        /*
        ) || Televideo Function keys
        ) ||
        ) ||   Televideo F1-F10 are on PC F1-F10.
        ) ||   Televideo F11-F16 are on PC Alt-F1 through Alt-F6.
        ) ||   Televideo Shift F1-F10 are on PC Shift-F1 - ShiftF10
        ) ||   Televideo Shift F11-F16 are on PC Alt-Shift-F1 through Alt-Shift-F16.
        */
    insertkeymap( tt, 368 , mkkeyevt(F_KVERB | K_TVIF01  )) || /* F1                 F1              Top Rank F key */
    insertkeymap( tt, 369 , mkkeyevt(F_KVERB | K_TVIF02  )) || /* F2                 F2              Top Rank F key */
    insertkeymap( tt, 370 , mkkeyevt(F_KVERB | K_TVIF03  )) || /* F3                 F3              Top Rank F key */
    insertkeymap( tt, 371 , mkkeyevt(F_KVERB | K_TVIF04  )) || /* F4                 F4              Top Rank F key */
    insertkeymap( tt, 372 , mkkeyevt(F_KVERB | K_TVIF05  )) || /* F5                 F5              Top Rank F key */
    insertkeymap( tt, 373 , mkkeyevt(F_KVERB | K_TVIF06  )) || /* F6                 F6              Top Rank F key */
    insertkeymap( tt, 374 , mkkeyevt(F_KVERB | K_TVIF07  )) || /* F7                 F7              Top Rank F key */
    insertkeymap( tt, 375 , mkkeyevt(F_KVERB | K_TVIF08  )) || /* F8                 F8              Top Rank F key */
    insertkeymap( tt, 376 , mkkeyevt(F_KVERB | K_TVIF09  )) || /* F9                 F9              Top Rank F key */
    insertkeymap( tt, 377 , mkkeyevt(F_KVERB | K_TVIF10  )) || /* F10                F10             Top Rank F key */
    insertkeymap( tt, 378 , mkkeyevt(F_KVERB | K_TVIF11 )) || /* F11                 F11             Top Rank F key */
    insertkeymap( tt, 379 , mkkeyevt(F_KVERB | K_TVIF12 )) || /* F12                 F12             Top Rank F key */

    insertkeymap( tt, 2416, mkkeyevt(F_KVERB | K_TVIF11 )) || /* F11                Alt-F1           Top Rank F key */
    insertkeymap( tt, 2417, mkkeyevt(F_KVERB | K_TVIF12 )) || /* F12                Alt-F2           Top Rank F key */
    insertkeymap( tt, 2418, mkkeyevt(F_KVERB | K_TVIF13 )) || /* F13                Alt-F3           Top Rank F key */
    insertkeymap( tt, 2419, mkkeyevt(F_KVERB | K_TVIF14 )) || /* F14                Alt-F4           Top Rank F key */
    insertkeymap( tt, 2420, mkkeyevt(F_KVERB | K_TVIF15 )) || /* F15                Alt-F5           Top Rank F key */
    insertkeymap( tt, 2421, mkkeyevt(F_KVERB | K_TVIF16 )) || /* F16                Alt-F6           Top Rank F key */

    insertkeymap( tt, 4466, mkkeyevt(F_KVERB | K_TVIF13 )) || /* F13                LK450 F13        Top Rank F key */
    insertkeymap( tt, 4467, mkkeyevt(F_KVERB | K_TVIF14 )) || /* F14                LK450 F14        Top Rank F key */
    insertkeymap( tt, 4468, mkkeyevt(F_KVERB | K_TVIF15 )) || /* F15                LK450 F15        Top Rank F key */
    insertkeymap( tt, 4469, mkkeyevt(F_KVERB | K_TVIF16 )) || /* F16                LK450 F16        Top Rank F key */

    insertkeymap( tt, 4476, mkkeyevt(F_KVERB | K_TVIF13 )) || /* F13                LK450 F13        Top Rank F key */
    insertkeymap( tt, 4477, mkkeyevt(F_KVERB | K_TVIF14 )) || /* F14                LK450 F14        Top Rank F key */
    insertkeymap( tt, 4478, mkkeyevt(F_KVERB | K_TVIF15 )) || /* F15                LK450 F15        Top Rank F key */
    insertkeymap( tt, 4479, mkkeyevt(F_KVERB | K_TVIF16 )) || /* F16                LK450 F16        Top Rank F key */

    insertkeymap( tt, 880 , mkkeyevt(F_KVERB | K_TVISF01  )) || /* Shift-F1         Shift-F1         Top Rank F key */
    insertkeymap( tt, 881 , mkkeyevt(F_KVERB | K_TVISF02  )) || /* Shift-F2         Shift-F2         Top Rank F key */
    insertkeymap( tt, 882 , mkkeyevt(F_KVERB | K_TVISF03  )) || /* Shift-F3         Shift-F3         Top Rank F key */
    insertkeymap( tt, 883 , mkkeyevt(F_KVERB | K_TVISF04  )) || /* Shift-F4         Shift-F4         Top Rank F key */
    insertkeymap( tt, 884 , mkkeyevt(F_KVERB | K_TVISF05  )) || /* Shift-F5         Shift-F5         Top Rank F key */
    insertkeymap( tt, 885 , mkkeyevt(F_KVERB | K_TVISF06  )) || /* Shift-F6         Shift-F6         Top Rank F key */
    insertkeymap( tt, 886 , mkkeyevt(F_KVERB | K_TVISF07  )) || /* Shift-F7         Shift-F7         Top Rank F key */
    insertkeymap( tt, 887 , mkkeyevt(F_KVERB | K_TVISF08  )) || /* Shift-F8         Shift-F8         Top Rank F key */
    insertkeymap( tt, 888 , mkkeyevt(F_KVERB | K_TVISF09  )) || /* Shift-F9         Shift-F9         Top Rank F key */
    insertkeymap( tt, 889 , mkkeyevt(F_KVERB | K_TVISF10  )) || /* Shift-F10        Shift-F10        Top Rank F key */
    insertkeymap( tt, 890 , mkkeyevt(F_KVERB | K_TVISF11  )) || /* Shift-F11        Shift-F11     Top Rank F key */
    insertkeymap( tt, 891 , mkkeyevt(F_KVERB | K_TVISF12  )) || /* Shift-F12        Shift-F12     Top Rank F key */

    insertkeymap( tt, 2928, mkkeyevt(F_KVERB | K_TVISF11 )) || /* Shift-F11         Alt-Shift-F1     Top Rank F key */
    insertkeymap( tt, 2929, mkkeyevt(F_KVERB | K_TVISF12 )) || /* Shift-F12         Alt-Shift-F2     Top Rank F key */
    insertkeymap( tt, 2930, mkkeyevt(F_KVERB | K_TVISF13 )) || /* Shift-F13         Alt-Shift-F3     Top Rank F key */
    insertkeymap( tt, 2931, mkkeyevt(F_KVERB | K_TVISF14 )) || /* Shift-F14         Alt-Shift-F4     Top Rank F key */
    insertkeymap( tt, 2932, mkkeyevt(F_KVERB | K_TVISF15 )) || /* Shift-F15         Alt-Shift-F5     Top Rank F key */
    insertkeymap( tt, 2933, mkkeyevt(F_KVERB | K_TVISF16 )) || /* Shift-F16         Alt-Shift-F6     Top Rank F key */

    insertkeymap( tt, 4978, mkkeyevt(F_KVERB | K_TVISF13 )) || /* Shift-F13         LK450 Shift-F13  Top Rank F key */
    insertkeymap( tt, 4979, mkkeyevt(F_KVERB | K_TVISF14 )) || /* Shift-F14         LK450 Shift-F14  Top Rank F key */
    insertkeymap( tt, 4980, mkkeyevt(F_KVERB | K_TVISF15 )) || /* Shift-F15         LK450 Shift-F15  Top Rank F key */
    insertkeymap( tt, 4981, mkkeyevt(F_KVERB | K_TVISF16 )) || /* Shift-F16         LK450 Shift-F16  Top Rank F key */

    insertkeymap( tt, 4988, mkkeyevt(F_KVERB | K_TVISF13 )) || /* Shift-F13         LK450 Shift-F13  Top Rank F key */
    insertkeymap( tt, 4989, mkkeyevt(F_KVERB | K_TVISF14 )) || /* Shift-F14         LK450 Shift-F14  Top Rank F key */
    insertkeymap( tt, 4990, mkkeyevt(F_KVERB | K_TVISF15 )) || /* Shift-F15         LK450 Shift-F15  Top Rank F key */
    insertkeymap( tt, 4991, mkkeyevt(F_KVERB | K_TVISF16 )) || /* Shift-F16         LK450 Shift-F16  Top Rank F key */

    insertkeymap( tt, 264,  mkkeyevt(F_KVERB | K_TVIBS   )) ||  /* Backspace */
    insertkeymap( tt, 302,  mkkeyevt(DEL )) ||              /* Kp-Del */
    insertkeymap( tt, 4398, mkkeyevt(F_KVERB | K_TVIDELCHAR )) ||       /* Gray-Del */
    insertkeymap( tt, 4910, mkkeyevt(F_KVERB | K_TVIDELLN )) || /* Shift-Gray-Del */
    insertkeymap( tt, 4387, mkkeyevt(F_KVERB | K_TVICLRLN )) || /* Gray-End */
    insertkeymap( tt, 291,  mkkeyevt(F_KVERB | K_TVICLRLN )) || /* End */
    insertkeymap( tt, 4899, mkkeyevt(F_KVERB | K_TVICLRPG )) || /* Shift-Gray-End */
    insertkeymap( tt, 4365, mkkeyevt(F_KVERB | K_TVIENTER )) || /* Gray-Enter */
    insertkeymap( tt, 269,  mkkeyevt(F_KVERB | K_TVIRETURN )) || /* Enter */
    insertkeymap( tt, 292,  mkkeyevt(F_KVERB | K_TVIHOME )) ||  /* Kp-Home */
    insertkeymap( tt, 804,  mkkeyevt(F_KVERB | K_TVISHOME )) || /* Shift-Kp-Home */
    insertkeymap( tt, 4388, mkkeyevt(F_KVERB | K_TVIHOME )) ||  /* Gray-Home */
    insertkeymap( tt, 4900, mkkeyevt(F_KVERB | K_TVISHOME )) || /* Shift-Gray-Home */
    insertkeymap( tt, 301,  mkkeyevt(F_KVERB | K_TVIREPLACE )) || /* Kp-Ins */
    insertkeymap( tt, 813,  mkkeyevt(F_KVERB | K_TVIINSERT )) ||  /* Shift-Kp-Ins */
    insertkeymap( tt, 4397, mkkeyevt(F_KVERB | K_TVIREPLACE )) || /* Gray-Insert */
    insertkeymap( tt, 4909, mkkeyevt(F_KVERB | K_TVIINSERT )) ||  /* Shift-Gray-Insert */
    insertkeymap( tt, 290,  mkkeyevt(F_KVERB | K_TVIPGNEXT )) ||  /* Kp-PgDn */
    insertkeymap( tt, 4386, mkkeyevt(F_KVERB | K_TVIPGNEXT )) ||  /* Gray-PgDn */
    insertkeymap( tt, 289,  mkkeyevt(F_KVERB | K_TVIPGPREV )) ||  /* Kp-PgUp */
    insertkeymap( tt, 4385, mkkeyevt(F_KVERB | K_TVIPGPREV )) ||  /* Kp-PgUp */
    insertkeymap( tt, 265,  mkkeyevt(F_KVERB | K_TVITAB )) ||     /* Tab */
    insertkeymap( tt, 777,  mkkeyevt(F_KVERB | K_TVISTAB ))    /* Shift-Tab */
    )
        return -1;

    if ( tt == TT_TVI950 ) {
        insertkeymap( tt, 379 , mkkeyevt(F_KVERB | K_TVISEND ));
        insertkeymap( tt, 2929, mkkeyevt(F_KVERB | K_TVISSEND ));
    }
    return 0;
}

int
deflinuxkm( int tt )
{
    if ( defbasekm(tt) ||
    insertkeymap( tt, 368, mkkeyevt(F_KVERB | K_ANSIF01 )) || /* f1                    */
    insertkeymap( tt, 369, mkkeyevt(F_KVERB | K_ANSIF02 )) || /* f2                    */
    insertkeymap( tt, 370, mkkeyevt(F_KVERB | K_ANSIF03 )) || /* f3                    */
    insertkeymap( tt, 371, mkkeyevt(F_KVERB | K_ANSIF04 )) || /* f4                    */
    insertkeymap( tt, 372, mkkeyevt(F_KVERB | K_ANSIF05 )) || /* f5                    */
    insertkeymap( tt, 373, mkkeyevt(F_KVERB | K_ANSIF06 )) || /* f6                    */
    insertkeymap( tt, 374, mkkeyevt(F_KVERB | K_ANSIF07 )) || /* f7                    */
    insertkeymap( tt, 375, mkkeyevt(F_KVERB | K_ANSIF08 )) || /* f8                    */
    insertkeymap( tt, 376, mkkeyevt(F_KVERB | K_ANSIF09 )) || /* f9                    */
    insertkeymap( tt, 377, mkkeyevt(F_KVERB | K_ANSIF10 )) || /* f10           */
    insertkeymap( tt, 378, mkkeyevt(F_KVERB | K_ANSIF11 )) || /* f11           */
    insertkeymap( tt, 379, mkkeyevt(F_KVERB | K_ANSIF12 )) || /* f12           */

    insertkeymap( tt, 880, mkkeyevt(F_KVERB | K_ANSIF13 )) || /* shift f1              */
    insertkeymap( tt, 881, mkkeyevt(F_KVERB | K_ANSIF14 )) || /* shift f2              */
    insertkeymap( tt, 882, mkkeyevt(F_KVERB | K_ANSIF15 )) || /* shift f3              */
    insertkeymap( tt, 883, mkkeyevt(F_KVERB | K_ANSIF16 )) || /* shift f4              */
    insertkeymap( tt, 884, mkkeyevt(F_KVERB | K_ANSIF17 )) || /* shift f5              */
    insertkeymap( tt, 885, mkkeyevt(F_KVERB | K_ANSIF18 )) || /* shift f6              */
    insertkeymap( tt, 886, mkkeyevt(F_KVERB | K_ANSIF19 )) || /* shift f7              */
    insertkeymap( tt, 887, mkkeyevt(F_KVERB | K_ANSIF20 )) || /* shift f8              */
    insertkeymap( tt, 888, mkkeyevt(F_KVERB | K_ANSIF21 )) || /* shift f9              */
    insertkeymap( tt, 889, mkkeyevt(F_KVERB | K_ANSIF22 )) || /* shift f10        */
    insertkeymap( tt, 890, mkkeyevt(F_KVERB | K_ANSIF23 )) || /* shift f11        */
    insertkeymap( tt, 891, mkkeyevt(F_KVERB | K_ANSIF24 )) || /* shift f12        */

    insertkeymap( tt, 1392, mkkeyevt(F_KVERB | K_ANSIF25 )) || /* control f1      */
    insertkeymap( tt, 1393, mkkeyevt(F_KVERB | K_ANSIF26 )) || /* control f2      */
    insertkeymap( tt, 1394, mkkeyevt(F_KVERB | K_ANSIF27 )) || /* control f3      */
    insertkeymap( tt, 1395, mkkeyevt(F_KVERB | K_ANSIF28 )) || /* control f4      */
    insertkeymap( tt, 1396, mkkeyevt(F_KVERB | K_ANSIF29 )) || /* control f5      */
    insertkeymap( tt, 1397, mkkeyevt(F_KVERB | K_ANSIF30 )) || /* control f6      */
    insertkeymap( tt, 1398, mkkeyevt(F_KVERB | K_ANSIF31 )) || /* control f7      */
    insertkeymap( tt, 1399, mkkeyevt(F_KVERB | K_ANSIF32 )) || /* control f8      */
    insertkeymap( tt, 1400, mkkeyevt(F_KVERB | K_ANSIF33 )) || /* control f9      */
    insertkeymap( tt, 1401, mkkeyevt(F_KVERB | K_ANSIF34 )) || /* control f10       */
    insertkeymap( tt, 1402, mkkeyevt(F_KVERB | K_ANSIF35 )) || /* control f11       */
    insertkeymap( tt, 1403, mkkeyevt(F_KVERB | K_ANSIF36 )) || /* control f12       */

    insertkeymap( tt, 1904, mkkeyevt(F_KVERB | K_ANSIF37 )) || /* control shift f1  */
    insertkeymap( tt, 1905, mkkeyevt(F_KVERB | K_ANSIF38 )) || /* control shift f2  */
    insertkeymap( tt, 1906, mkkeyevt(F_KVERB | K_ANSIF39 )) || /* control shift f3  */
    insertkeymap( tt, 1907, mkkeyevt(F_KVERB | K_ANSIF40 )) || /* control shift f4  */
    insertkeymap( tt, 1908, mkkeyevt(F_KVERB | K_ANSIF41 )) || /* control shift f5  */
    insertkeymap( tt, 1909, mkkeyevt(F_KVERB | K_ANSIF42 )) || /* control shift f6  */
    insertkeymap( tt, 1910, mkkeyevt(F_KVERB | K_ANSIF43 )) || /* control shift f7  */
    insertkeymap( tt, 1911, mkkeyevt(F_KVERB | K_ANSIF44 )) || /* control shift f8  */
    insertkeymap( tt, 1912, mkkeyevt(F_KVERB | K_ANSIF45 )) || /* control shift f9  */
    insertkeymap( tt, 1913, mkkeyevt(F_KVERB | K_ANSIF46 )) || /* control shift f10 */
    insertkeymap( tt, 1914, mkkeyevt(F_KVERB | K_ANSIF47 )) || /* control shift f11 */
    insertkeymap( tt, 1915, mkkeyevt(F_KVERB | K_ANSIF48 )) || /* control shift f12 */

    insertkeymap( tt, 264, mkkeyevt(8)) ||                  /* backspace               */

    insertkeymap( tt, 777, mkliteralevt("\x1B[Z"))          || /* shift tab            */

    insertkeymap( tt, 4388, mkkeyevt(F_KVERB | K_ANSIF49 )) || /* home         */
    insertkeymap( tt, 4385, mkkeyevt(F_KVERB | K_ANSIF51 )) || /* page up              */
    insertkeymap( tt, 4387, mkkeyevt(F_KVERB | K_ANSIF57 )) || /* end          */
    insertkeymap( tt, 4386, mkkeyevt(F_KVERB | K_ANSIF59 )) || /* page down            */
    insertkeymap( tt, 4397, mkkeyevt(F_KVERB | K_ANSIF60 )) || /* insert               */
    insertkeymap( tt, 4398, mkkeyevt(F_KVERB | K_ANSIF61 )) || /* delete               */

    insertkeymap( tt, 4390, mkkeyevt(F_KVERB | K_ANSIF50 )) || /* Up Arrow           Up Arrow         Cursor keypad */
    insertkeymap( tt, 4392, mkkeyevt(F_KVERB | K_ANSIF58 )) || /* Down Arrow         Down Arrow       Cursor keypad */
    insertkeymap( tt, 4391, mkkeyevt(F_KVERB | K_ANSIF55 )) || /* Right Arrow        Right Arrow      Cursor keypad */
    insertkeymap( tt, 4389, mkkeyevt(F_KVERB | K_ANSIF53 )) || /* Left Arrow         Left Arrow       Cursor keypad */

    insertkeymap( tt, 301 , mkkeyevt(F_KVERB | K_ANSIF60 )) || /* KP 0               Keypad 0         Numeric keypad */
    insertkeymap( tt, 291 , mkkeyevt(F_KVERB | K_ANSIF57 )) || /* KP 1               Keypad 1         Numeric keypad */
    insertkeymap( tt, 296 , mkkeyevt(F_KVERB | K_ANSIF58 )) || /* KP 2               Keypad 2         Numeric keypad */
    insertkeymap( tt, 290 , mkkeyevt(F_KVERB | K_ANSIF59 )) || /* KP 3               Keypad 3         Numeric keypad */
    insertkeymap( tt, 293 , mkkeyevt(F_KVERB | K_ANSIF53 )) || /* KP 4               Keypad 4         Numeric keypad */
    insertkeymap( tt, 268 , mkkeyevt(F_KVERB | K_ANSIF54 )) || /* KP 5               Keypad 5         Numeric keypad */
    insertkeymap( tt, 295 , mkkeyevt(F_KVERB | K_ANSIF55 )) || /* KP 6               Keypad 6         Numeric keypad */
    insertkeymap( tt, 292 , mkkeyevt(F_KVERB | K_ANSIF49 )) || /* KP 7               Keypad 7         Numeric keypad */
    insertkeymap( tt, 294 , mkkeyevt(F_KVERB | K_ANSIF50 )) || /* KP 8               Keypad 8         Numeric keypad */
    insertkeymap( tt, 289 , mkkeyevt(F_KVERB | K_ANSIF51 )) || /* KP 9               Keypad 9         Numeric keypad */

    insertkeymap( tt, 302 , mkkeyevt(F_KVERB | K_ANSIF61 )) || /* keypad delete */
    insertkeymap( tt, 877 , mkkeyevt(F_KVERB | K_ANSIF52 )) || /* Shift-keypad minus  */
    insertkeymap( tt, 875 , mkkeyevt(F_KVERB | K_ANSIF56 )) || /* Shift-keypad add    */
    insertkeymap( tt, 1901, mkkeyevt(F_KVERB | K_ANSIF52 )) || /* Ctrl-Shift-keypad minus  */
    insertkeymap( tt, 1899, mkkeyevt(F_KVERB | K_ANSIF56 ))    /* Ctrl-Shift-keypad add    */
    )
        return -1;
    return 0;
}

int
defannarbor( int tt )
{
    defbasekm(tt);
    return 0;
}

int
defscoansikm( int tt )
{
    if ( defbasekm(tt) ||
    insertkeymap( tt, 368, mkkeyevt(F_KVERB | K_ANSIF01 )) || /* f1                    */
    insertkeymap( tt, 369, mkkeyevt(F_KVERB | K_ANSIF02 )) || /* f2                    */
    insertkeymap( tt, 370, mkkeyevt(F_KVERB | K_ANSIF03 )) || /* f3                    */
    insertkeymap( tt, 371, mkkeyevt(F_KVERB | K_ANSIF04 )) || /* f4                    */
    insertkeymap( tt, 372, mkkeyevt(F_KVERB | K_ANSIF05 )) || /* f5                    */
    insertkeymap( tt, 373, mkkeyevt(F_KVERB | K_ANSIF06 )) || /* f6                    */
    insertkeymap( tt, 374, mkkeyevt(F_KVERB | K_ANSIF07 )) || /* f7                    */
    insertkeymap( tt, 375, mkkeyevt(F_KVERB | K_ANSIF08 )) || /* f8                    */
    insertkeymap( tt, 376, mkkeyevt(F_KVERB | K_ANSIF09 )) || /* f9                    */
    insertkeymap( tt, 377, mkkeyevt(F_KVERB | K_ANSIF10 )) || /* f10           */
    insertkeymap( tt, 378, mkkeyevt(F_KVERB | K_ANSIF11 )) || /* f11           */
    insertkeymap( tt, 379, mkkeyevt(F_KVERB | K_ANSIF12 )) || /* f12           */

    insertkeymap( tt, 880, mkkeyevt(F_KVERB | K_ANSIF13 )) || /* shift f1              */
    insertkeymap( tt, 881, mkkeyevt(F_KVERB | K_ANSIF14 )) || /* shift f2              */
    insertkeymap( tt, 882, mkkeyevt(F_KVERB | K_ANSIF15 )) || /* shift f3              */
    insertkeymap( tt, 883, mkkeyevt(F_KVERB | K_ANSIF16 )) || /* shift f4              */
    insertkeymap( tt, 884, mkkeyevt(F_KVERB | K_ANSIF17 )) || /* shift f5              */
    insertkeymap( tt, 885, mkkeyevt(F_KVERB | K_ANSIF18 )) || /* shift f6              */
    insertkeymap( tt, 886, mkkeyevt(F_KVERB | K_ANSIF19 )) || /* shift f7              */
    insertkeymap( tt, 887, mkkeyevt(F_KVERB | K_ANSIF20 )) || /* shift f8              */
    insertkeymap( tt, 888, mkkeyevt(F_KVERB | K_ANSIF21 )) || /* shift f9              */
    insertkeymap( tt, 889, mkkeyevt(F_KVERB | K_ANSIF22 )) || /* shift f10        */
    insertkeymap( tt, 890, mkkeyevt(F_KVERB | K_ANSIF23 )) || /* shift f11        */
    insertkeymap( tt, 891, mkkeyevt(F_KVERB | K_ANSIF24 )) || /* shift f12        */

    insertkeymap( tt, 1392, mkkeyevt(F_KVERB | K_ANSIF25 )) || /* control f1      */
    insertkeymap( tt, 1393, mkkeyevt(F_KVERB | K_ANSIF26 )) || /* control f2      */
    insertkeymap( tt, 1394, mkkeyevt(F_KVERB | K_ANSIF27 )) || /* control f3      */
    insertkeymap( tt, 1395, mkkeyevt(F_KVERB | K_ANSIF28 )) || /* control f4      */
    insertkeymap( tt, 1396, mkkeyevt(F_KVERB | K_ANSIF29 )) || /* control f5      */
    insertkeymap( tt, 1397, mkkeyevt(F_KVERB | K_ANSIF30 )) || /* control f6      */
    insertkeymap( tt, 1398, mkkeyevt(F_KVERB | K_ANSIF31 )) || /* control f7      */
    insertkeymap( tt, 1399, mkkeyevt(F_KVERB | K_ANSIF32 )) || /* control f8      */
    insertkeymap( tt, 1400, mkkeyevt(F_KVERB | K_ANSIF33 )) || /* control f9      */
    insertkeymap( tt, 1401, mkkeyevt(F_KVERB | K_ANSIF34 )) || /* control f10       */
    insertkeymap( tt, 1402, mkkeyevt(F_KVERB | K_ANSIF35 )) || /* control f11       */
    insertkeymap( tt, 1403, mkkeyevt(F_KVERB | K_ANSIF36 )) || /* control f12       */

    insertkeymap( tt, 1904, mkkeyevt(F_KVERB | K_ANSIF37 )) || /* control shift f1  */
    insertkeymap( tt, 1905, mkkeyevt(F_KVERB | K_ANSIF38 )) || /* control shift f2  */
    insertkeymap( tt, 1906, mkkeyevt(F_KVERB | K_ANSIF39 )) || /* control shift f3  */
    insertkeymap( tt, 1907, mkkeyevt(F_KVERB | K_ANSIF40 )) || /* control shift f4  */
    insertkeymap( tt, 1908, mkkeyevt(F_KVERB | K_ANSIF41 )) || /* control shift f5  */
    insertkeymap( tt, 1909, mkkeyevt(F_KVERB | K_ANSIF42 )) || /* control shift f6  */
    insertkeymap( tt, 1910, mkkeyevt(F_KVERB | K_ANSIF43 )) || /* control shift f7  */
    insertkeymap( tt, 1911, mkkeyevt(F_KVERB | K_ANSIF44 )) || /* control shift f8  */
    insertkeymap( tt, 1912, mkkeyevt(F_KVERB | K_ANSIF45 )) || /* control shift f9  */
    insertkeymap( tt, 1913, mkkeyevt(F_KVERB | K_ANSIF46 )) || /* control shift f10 */
    insertkeymap( tt, 1914, mkkeyevt(F_KVERB | K_ANSIF47 )) || /* control shift f11 */
    insertkeymap( tt, 1915, mkkeyevt(F_KVERB | K_ANSIF48 )) || /* control shift f12 */

    insertkeymap( tt, 264, mkkeyevt(8                    )) || /* backspace            */

    insertkeymap( tt, 777, mkkeyevt(F_KVERB | K_ANSIF14  )) || /* shift tab            */

    insertkeymap( tt, 4388, mkkeyevt(F_KVERB | K_ANSIF49 )) || /* home         */
    insertkeymap( tt, 4385, mkkeyevt(F_KVERB | K_ANSIF51 )) || /* page up              */
    insertkeymap( tt, 4387, mkkeyevt(F_KVERB | K_ANSIF57 )) || /* end          */
    insertkeymap( tt, 4386, mkkeyevt(F_KVERB | K_ANSIF59 )) || /* page down            */
    insertkeymap( tt, 4397, mkkeyevt(F_KVERB | K_ANSIF60 )) || /* insert               */
    insertkeymap( tt, 4398, mkkeyevt(DEL                 )) || /* delete               */

    insertkeymap( tt, 4390, mkkeyevt(F_KVERB | K_ANSIF50 )) || /* Up Arrow           Up Arrow         Cursor keypad */
    insertkeymap( tt, 4392, mkkeyevt(F_KVERB | K_ANSIF58 )) || /* Down Arrow         Down Arrow       Cursor keypad */
    insertkeymap( tt, 4391, mkkeyevt(F_KVERB | K_ANSIF55 )) || /* Right Arrow        Right Arrow      Cursor keypad */
    insertkeymap( tt, 4389, mkkeyevt(F_KVERB | K_ANSIF53 )) || /* Left Arrow         Left Arrow       Cursor keypad */

    insertkeymap( tt, 301 , mkkeyevt(F_KVERB | K_ANSIF60 )) || /* KP 0               Keypad 0         Numeric keypad */
    insertkeymap( tt, 291 , mkkeyevt(F_KVERB | K_ANSIF57 )) || /* KP 1               Keypad 1         Numeric keypad */
    insertkeymap( tt, 296 , mkkeyevt(F_KVERB | K_ANSIF58 )) || /* KP 2               Keypad 2         Numeric keypad */
    insertkeymap( tt, 290 , mkkeyevt(F_KVERB | K_ANSIF59 )) || /* KP 3               Keypad 3         Numeric keypad */
    insertkeymap( tt, 293 , mkkeyevt(F_KVERB | K_ANSIF53 )) || /* KP 4               Keypad 4         Numeric keypad */
    insertkeymap( tt, 268 , mkkeyevt(F_KVERB | K_ANSIF54 )) || /* KP 5               Keypad 5         Numeric keypad */
    insertkeymap( tt, 295 , mkkeyevt(F_KVERB | K_ANSIF55 )) || /* KP 6               Keypad 6         Numeric keypad */
    insertkeymap( tt, 292 , mkkeyevt(F_KVERB | K_ANSIF49 )) || /* KP 7               Keypad 7         Numeric keypad */
    insertkeymap( tt, 294 , mkkeyevt(F_KVERB | K_ANSIF50 )) || /* KP 8               Keypad 8         Numeric keypad */
    insertkeymap( tt, 289 , mkkeyevt(F_KVERB | K_ANSIF51 )) || /* KP 9               Keypad 9         Numeric keypad */

    insertkeymap( tt, 302 , mkkeyevt(DEL                 )) || /* keypad delete */
    insertkeymap( tt, 877 , mkkeyevt(F_KVERB | K_ANSIF52 )) || /* Shift-keypad minus  */
    insertkeymap( tt, 875 , mkkeyevt(F_KVERB | K_ANSIF56 )) || /* Shift-keypad add    */
    insertkeymap( tt, 1901, mkkeyevt(F_KVERB | K_ANSIF52 )) || /* Ctrl-Shift-keypad minus  */
    insertkeymap( tt, 1899, mkkeyevt(F_KVERB | K_ANSIF56 ))    /* Ctrl-Shift-keypad add    */
    )
        return -1;
    return 0;
}

int
defat386km( int tt )
{
    if ( defbasekm(tt) ||
    insertkeymap( tt, 368, mkkeyevt(F_KVERB | K_ANSIF01 )) || /* f1                    */
    insertkeymap( tt, 369, mkkeyevt(F_KVERB | K_ANSIF02 )) || /* f2                    */
    insertkeymap( tt, 370, mkkeyevt(F_KVERB | K_ANSIF03 )) || /* f3                    */
    insertkeymap( tt, 371, mkkeyevt(F_KVERB | K_ANSIF04 )) || /* f4                    */
    insertkeymap( tt, 372, mkkeyevt(F_KVERB | K_ANSIF05 )) || /* f5                    */
    insertkeymap( tt, 373, mkkeyevt(F_KVERB | K_ANSIF06 )) || /* f6                    */
    insertkeymap( tt, 374, mkkeyevt(F_KVERB | K_ANSIF07 )) || /* f7                    */
    insertkeymap( tt, 375, mkkeyevt(F_KVERB | K_ANSIF08 )) || /* f8                    */
    insertkeymap( tt, 376, mkkeyevt(F_KVERB | K_ANSIF09 )) || /* f9                    */
    insertkeymap( tt, 377, mkkeyevt(F_KVERB | K_ANSIF10 )) || /* f10           */
    insertkeymap( tt, 378, mkkeyevt(F_KVERB | K_ANSIF11 )) || /* f11           */
    insertkeymap( tt, 379, mkkeyevt(F_KVERB | K_ANSIF12 )) || /* f12           */

    insertkeymap( tt, 880, mkkeyevt(F_KVERB | K_ANSIF13 )) || /* shift f1              */
    insertkeymap( tt, 881, mkkeyevt(F_KVERB | K_ANSIF14 )) || /* shift f2              */
    insertkeymap( tt, 882, mkkeyevt(F_KVERB | K_ANSIF15 )) || /* shift f3              */
    insertkeymap( tt, 883, mkkeyevt(F_KVERB | K_ANSIF16 )) || /* shift f4              */
    insertkeymap( tt, 884, mkkeyevt(F_KVERB | K_ANSIF17 )) || /* shift f5              */
    insertkeymap( tt, 885, mkkeyevt(F_KVERB | K_ANSIF18 )) || /* shift f6              */
    insertkeymap( tt, 886, mkkeyevt(F_KVERB | K_ANSIF19 )) || /* shift f7              */
    insertkeymap( tt, 887, mkkeyevt(F_KVERB | K_ANSIF20 )) || /* shift f8              */
    insertkeymap( tt, 888, mkkeyevt(F_KVERB | K_ANSIF21 )) || /* shift f9              */
    insertkeymap( tt, 889, mkkeyevt(F_KVERB | K_ANSIF22 )) || /* shift f10        */
    insertkeymap( tt, 890, mkkeyevt(F_KVERB | K_ANSIF23 )) || /* shift f11        */
    insertkeymap( tt, 891, mkkeyevt(F_KVERB | K_ANSIF24 )) || /* shift f12        */

    insertkeymap( tt, 1392, mkkeyevt(F_KVERB | K_ANSIF25 )) || /* control f1      */
    insertkeymap( tt, 1393, mkkeyevt(F_KVERB | K_ANSIF26 )) || /* control f2      */
    insertkeymap( tt, 1394, mkkeyevt(F_KVERB | K_ANSIF27 )) || /* control f3      */
    insertkeymap( tt, 1395, mkkeyevt(F_KVERB | K_ANSIF28 )) || /* control f4      */
    insertkeymap( tt, 1396, mkkeyevt(F_KVERB | K_ANSIF29 )) || /* control f5      */
    insertkeymap( tt, 1397, mkkeyevt(F_KVERB | K_ANSIF30 )) || /* control f6      */
    insertkeymap( tt, 1398, mkkeyevt(F_KVERB | K_ANSIF31 )) || /* control f7      */
    insertkeymap( tt, 1399, mkkeyevt(F_KVERB | K_ANSIF32 )) || /* control f8      */
    insertkeymap( tt, 1400, mkkeyevt(F_KVERB | K_ANSIF33 )) || /* control f9      */
    insertkeymap( tt, 1401, mkkeyevt(F_KVERB | K_ANSIF34 )) || /* control f10       */
    insertkeymap( tt, 1402, mkkeyevt(F_KVERB | K_ANSIF35 )) || /* control f11       */
    insertkeymap( tt, 1403, mkkeyevt(F_KVERB | K_ANSIF36 )) || /* control f12       */

    insertkeymap( tt, 1904, mkkeyevt(F_KVERB | K_ANSIF37 )) || /* control shift f1  */
    insertkeymap( tt, 1905, mkkeyevt(F_KVERB | K_ANSIF38 )) || /* control shift f2  */
    insertkeymap( tt, 1906, mkkeyevt(F_KVERB | K_ANSIF39 )) || /* control shift f3  */
    insertkeymap( tt, 1907, mkkeyevt(F_KVERB | K_ANSIF40 )) || /* control shift f4  */
    insertkeymap( tt, 1908, mkkeyevt(F_KVERB | K_ANSIF41 )) || /* control shift f5  */
    insertkeymap( tt, 1909, mkkeyevt(F_KVERB | K_ANSIF42 )) || /* control shift f6  */
    insertkeymap( tt, 1910, mkkeyevt(F_KVERB | K_ANSIF43 )) || /* control shift f7  */
    insertkeymap( tt, 1911, mkkeyevt(F_KVERB | K_ANSIF44 )) || /* control shift f8  */
    insertkeymap( tt, 1912, mkkeyevt(F_KVERB | K_ANSIF45 )) || /* control shift f9  */
    insertkeymap( tt, 1913, mkkeyevt(F_KVERB | K_ANSIF46 )) || /* control shift f10 */
    insertkeymap( tt, 1914, mkkeyevt(F_KVERB | K_ANSIF47 )) || /* control shift f11 */
    insertkeymap( tt, 1915, mkkeyevt(F_KVERB | K_ANSIF48 )) || /* control shift f12 */

    insertkeymap( tt, 264, mkkeyevt(8                   )) || /* backspace             */

#ifdef COMMENT
    insertkeymap( tt, 777, mkkeyevt(F_CSI |  'z'        )) || /* shift tab             */
#else
        /* As per Pim Zandbergen <pim@cti-software.nl> */
    insertkeymap( tt, 777, mkkeyevt(29 )) ||            /* shift tab */
#endif /* COMMENT */

    insertkeymap( tt, 4388, mkkeyevt(F_KVERB | K_ANSIF49 )) || /* home         */
    insertkeymap( tt, 4385, mkkeyevt(F_KVERB | K_ANSIF51 )) || /* page up              */
    insertkeymap( tt, 4387, mkkeyevt(F_KVERB | K_ANSIF57 )) || /* end          */
    insertkeymap( tt, 4386, mkkeyevt(F_KVERB | K_ANSIF59 )) || /* page down            */
    insertkeymap( tt, 4397, mkkeyevt(F_KVERB | K_ANSIF60 )) || /* insert               */
    insertkeymap( tt, 4398, mkkeyevt(DEL                  )) || /* delete              */

    insertkeymap( tt, 4390, mkkeyevt(F_KVERB | K_ANSIF50 )) || /* Up Arrow           Up Arrow         Cursor keypad */
    insertkeymap( tt, 4392, mkkeyevt(F_KVERB | K_ANSIF58 )) || /* Down Arrow         Down Arrow       Cursor keypad */
    insertkeymap( tt, 4391, mkkeyevt(F_KVERB | K_ANSIF55 )) || /* Right Arrow        Right Arrow      Cursor keypad */
    insertkeymap( tt, 4389, mkkeyevt(F_KVERB | K_ANSIF53 )) || /* Left Arrow         Left Arrow       Cursor keypad */

    insertkeymap( tt, 301 , mkkeyevt(F_KVERB | K_ANSIF60 )) || /* KP 0               Keypad 0         Numeric keypad */
    insertkeymap( tt, 291 , mkkeyevt(F_KVERB | K_ANSIF57 )) || /* KP 1               Keypad 1         Numeric keypad */
    insertkeymap( tt, 296 , mkkeyevt(F_KVERB | K_ANSIF58 )) || /* KP 2               Keypad 2         Numeric keypad */
    insertkeymap( tt, 290 , mkkeyevt(F_KVERB | K_ANSIF59 )) || /* KP 3               Keypad 3         Numeric keypad */
    insertkeymap( tt, 293 , mkkeyevt(F_KVERB | K_ANSIF53 )) || /* KP 4               Keypad 4         Numeric keypad */
    insertkeymap( tt, 268 , mkkeyevt(F_KVERB | K_ANSIF54 )) || /* KP 5               Keypad 5         Numeric keypad */
    insertkeymap( tt, 295 , mkkeyevt(F_KVERB | K_ANSIF55 )) || /* KP 6               Keypad 6         Numeric keypad */
    insertkeymap( tt, 292 , mkkeyevt(F_KVERB | K_ANSIF49 )) || /* KP 7               Keypad 7         Numeric keypad */
    insertkeymap( tt, 294 , mkkeyevt(F_KVERB | K_ANSIF50 )) || /* KP 8               Keypad 8         Numeric keypad */
    insertkeymap( tt, 289 , mkkeyevt(F_KVERB | K_ANSIF51 )) || /* KP 9               Keypad 9         Numeric keypad */

    insertkeymap( tt, 302, mkkeyevt(DEL          )) || /* keypad delete */
    insertkeymap( tt, 877 , mkkeyevt(F_KVERB | K_ANSIF52 )) || /* Shift-keypad minus  */
    insertkeymap( tt, 875 , mkkeyevt(F_KVERB | K_ANSIF56 )) || /* Shift-keypad add    */
    insertkeymap( tt, 1901, mkkeyevt(F_KVERB | K_ANSIF52 )) || /* Ctrl-Shift-keypad minus  */
    insertkeymap( tt, 1899, mkkeyevt(F_KVERB | K_ANSIF56 ))    /* Ctrl-Shift-keypad add    */
         )
        return -1;
    return 0;
}

int
defqnxkm( int tt )
{
    defbasekm(tt);

    insertkeymap( tt, 264, mkkeyevt(DEL));             /* Backspace */

    insertkeymap( tt, 777, mkliteralevt("\377\200")) ; /* Shift-Tab                    */
    insertkeymap( tt, 368, mkliteralevt("\377\201")) ; /* f1                   */
    insertkeymap( tt, 369, mkliteralevt("\377\202")) ; /* f2                   */
    insertkeymap( tt, 370, mkliteralevt("\377\203")) ; /* f3                   */
    insertkeymap( tt, 371, mkliteralevt("\377\204")) ; /* f4                   */
    insertkeymap( tt, 372, mkliteralevt("\377\205")) ; /* f5                   */
    insertkeymap( tt, 373, mkliteralevt("\377\206")) ; /* f6                   */
    insertkeymap( tt, 374, mkliteralevt("\377\207")) ; /* f7                   */
    insertkeymap( tt, 375, mkliteralevt("\377\210")) ; /* f8                   */
    insertkeymap( tt, 376, mkliteralevt("\377\211")) ; /* f9                   */
    insertkeymap( tt, 377, mkliteralevt("\377\212")) ; /* f10          */

    insertkeymap( tt, 880, mkliteralevt("\377\213")) ; /* shift f1             */
    insertkeymap( tt, 881, mkliteralevt("\377\214")) ; /* shift f2             */
    insertkeymap( tt, 882, mkliteralevt("\377\215")) ; /* shift f3             */
    insertkeymap( tt, 883, mkliteralevt("\377\216")) ; /* shift f4             */
    insertkeymap( tt, 884, mkliteralevt("\377\217")) ; /* shift f5             */
    insertkeymap( tt, 885, mkliteralevt("\377\220")) ; /* shift f6             */
    insertkeymap( tt, 886, mkliteralevt("\377\221")) ; /* shift f7             */
    insertkeymap( tt, 887, mkliteralevt("\377\222")) ; /* shift f8             */
    insertkeymap( tt, 888, mkliteralevt("\377\223")) ; /* shift f9             */
    insertkeymap( tt, 889, mkliteralevt("\377\224")) ; /* shift f10       */

    insertkeymap( tt, 1392, mkliteralevt("\377\225" )) ; /* control f1    */
    insertkeymap( tt, 1393, mkliteralevt("\377\226" )) ; /* control f2    */
    insertkeymap( tt, 1394, mkliteralevt("\377\227" )) ; /* control f3    */
    insertkeymap( tt, 1395, mkliteralevt("\377\230" )) ; /* control f4    */
    insertkeymap( tt, 1396, mkliteralevt("\377\231" )) ; /* control f5    */
    insertkeymap( tt, 1397, mkliteralevt("\377\232" )) ; /* control f6    */
    insertkeymap( tt, 1398, mkliteralevt("\377\233" )) ; /* control f7    */
    insertkeymap( tt, 1399, mkliteralevt("\377\234" )) ; /* control f8    */
    insertkeymap( tt, 1400, mkliteralevt("\377\235" )) ; /* control f9    */
    insertkeymap( tt, 1401, mkliteralevt("\377\236" )) ; /* control f10       */
    insertkeymap( tt, 1289, mkliteralevt("\377\237")) ; /* Ctrl-Tab  */

    insertkeymap( tt, 292, mkliteralevt("\377\240")) ; /* Home */
/*    insertkeymap( tt, 294, mkliteralevt("\377\241")) ; /* Up */
    insertkeymap( tt, 289, mkliteralevt("\377\242")) ; /* PgUp */
    insertkeymap( tt, 365, mkliteralevt("\377\243")) ; /* Minus */
/*    insertkeymap( tt, 293, mkliteralevt("\377\244")) ; /* Left */
    insertkeymap( tt, 268, mkliteralevt("\377\245")) ; /* Blank */
/*    insertkeymap( tt, 295, mkliteralevt("\377\246")) ; /* Right */
    insertkeymap( tt, 363, mkliteralevt("\377\247")) ; /* Plus */
    insertkeymap( tt, 291, mkliteralevt("\377\250")) ; /* End */
/*    insertkeymap( tt, 296, mkliteralevt("\377\251")) ; /* Down */
    insertkeymap( tt, 290, mkliteralevt("\377\252")) ; /* PgDn */
    insertkeymap( tt, 301, mkliteralevt("\377\253")) ; /* Ins */
    insertkeymap( tt, 302, mkliteralevt("\377\254")) ; /* Del */
    insertkeymap( tt, 298, mkliteralevt("\377\255")) ; /* PrtScrn */
    insertkeymap( tt, 378, mkliteralevt("\377\256")) ; /* f11          */
    insertkeymap( tt, 379, mkliteralevt("\377\257")) ; /* f12          */

    insertkeymap( tt, 1316, mkliteralevt("\377\260")) ; /* Ctrl-Home */
    insertkeymap( tt, 1318, mkliteralevt("\377\261")) ; /* Ctrl-Up */
    insertkeymap( tt, 1313, mkliteralevt("\377\262")) ; /* Ctrl-PgUp */
    insertkeymap( tt, 1389, mkliteralevt("\377\263")) ; /* Ctrl-Minus */
    insertkeymap( tt, 1317, mkliteralevt("\377\264")) ; /* Ctrl-Left */
    insertkeymap( tt, 1292, mkliteralevt("\377\265")) ; /* Ctrl-Blank */
    insertkeymap( tt, 1319, mkliteralevt("\377\266")) ; /* Ctrl-Right */
    insertkeymap( tt, 1387, mkliteralevt("\377\267")) ; /* Ctrl-Plus */
    insertkeymap( tt, 1315, mkliteralevt("\377\270")) ; /* Ctrl-End */
    insertkeymap( tt, 1320, mkliteralevt("\377\271")) ; /* Ctrl-Down */
    insertkeymap( tt, 1314, mkliteralevt("\377\272")) ; /* Ctrl-PgDn */
    insertkeymap( tt, 1325, mkliteralevt("\377\273")) ; /* Ctrl-Ins */
    insertkeymap( tt, 1326, mkliteralevt("\377\274")) ; /* Ctrl-Del */
    insertkeymap( tt, 1322, mkliteralevt("\377\275")) ; /* Ctrl-PrtScrn */
    insertkeymap( tt, 1402, mkliteralevt("\377\276")) ; /* Ctrl-f11            */
    insertkeymap( tt, 1403, mkliteralevt("\377\277")) ; /* Ctrl-f12            */

    insertkeymap( tt, 2340, mkliteralevt("\377\300")) ; /* Alt-Home */
    insertkeymap( tt, 2342, mkliteralevt("\377\301")) ; /* Alt-Up */
    insertkeymap( tt, 2337, mkliteralevt("\377\302")) ; /* Alt-PgUp */
    insertkeymap( tt, 2413, mkliteralevt("\377\303")) ; /* Alt-Minus */
    insertkeymap( tt, 2341, mkliteralevt("\377\304")) ; /* Alt-Left */
    insertkeymap( tt, 2316, mkliteralevt("\377\305")) ; /* Alt-Blank */
    insertkeymap( tt, 2343, mkliteralevt("\377\306")) ; /* Alt-Right */
    insertkeymap( tt, 2411, mkliteralevt("\377\307")) ; /* Alt-Plus */
    insertkeymap( tt, 2339, mkliteralevt("\377\310")) ; /* Alt-End */
    insertkeymap( tt, 2344, mkliteralevt("\377\311")) ; /* Alt-Down */
    insertkeymap( tt, 2338, mkliteralevt("\377\312")) ; /* Alt-PgDn */
    insertkeymap( tt, 2349, mkliteralevt("\377\313")) ; /* Alt-Ins */
    insertkeymap( tt, 2350, mkliteralevt("\377\314")) ; /* Alt-Del */
    insertkeymap( tt, 2410, mkliteralevt("\377\315")) ; /* Alt-* */
    insertkeymap( tt, 2426, mkliteralevt("\377\316")) ; /* Alt-f11             */
    insertkeymap( tt, 2427, mkliteralevt("\377\317")) ; /* Alt-f12             */


    insertkeymap( tt, 1293, mkliteralevt("\377\320" )) ; /* ctrl enter  */
    insertkeymap( tt, 2416, mkliteralevt("\377\321" )) ; /* alt f1  */
    insertkeymap( tt, 2417, mkliteralevt("\377\322" )) ; /* alt f2  */
    insertkeymap( tt, 2418, mkliteralevt("\377\323" )) ; /* alt f3  */
    insertkeymap( tt, 2419, mkliteralevt("\377\324" )) ; /* alt f4  */
    insertkeymap( tt, 2420, mkliteralevt("\377\325" )) ; /* alt f5  */
    insertkeymap( tt, 2421, mkliteralevt("\377\326" )) ; /* alt f6  */
    insertkeymap( tt, 2422, mkliteralevt("\377\327" )) ; /* alt f7  */
    insertkeymap( tt, 2423, mkliteralevt("\377\330" )) ; /* alt f8  */
    insertkeymap( tt, 2424, mkliteralevt("\377\331" )) ; /* alt f9  */
    insertkeymap( tt, 2425, mkliteralevt("\377\332" )) ; /* alt f10 */
    insertkeymap( tt,  890, mkliteralevt("\377\333" )) ; /* shift f11     */
    insertkeymap( tt,  891, mkliteralevt("\377\334" )) ; /* shift f12     */
    insertkeymap( tt, 2346, mkliteralevt("\377\335" )) ; /* SysRq (Alt-PrtScrn)  */
    insertkeymap( tt, 1151, mkliteralevt("\377\336" )) ; /* Ctrl-BS  */
    insertkeymap( tt, 2312, mkliteralevt("\377\337" )) ; /* Alt-BS  */

    insertkeymap( tt, 781, mkliteralevt("\377\340")) ; /* Shift-Enter */
    insertkeymap( tt, 2145, mkliteralevt("\377\341")) ; /* Alt-a */
    insertkeymap( tt, 2146, mkliteralevt("\377\342")) ; /* Alt-b */
    insertkeymap( tt, 2147, mkliteralevt("\377\343")) ; /* Alt-c */
    insertkeymap( tt, 2148, mkliteralevt("\377\344")) ; /* Alt-d */
    insertkeymap( tt, 2149, mkliteralevt("\377\345")) ; /* Alt-e */
    insertkeymap( tt, 2150, mkliteralevt("\377\346")) ; /* Alt-f */
    insertkeymap( tt, 2151, mkliteralevt("\377\347")) ; /* Alt-g */
    insertkeymap( tt, 2152, mkliteralevt("\377\350")) ; /* Alt-h */
    insertkeymap( tt, 2153, mkliteralevt("\377\351")) ; /* Alt-i */
    insertkeymap( tt, 2154, mkliteralevt("\377\352")) ; /* Alt-j */
    insertkeymap( tt, 2155, mkliteralevt("\377\353")) ; /* Alt-k */
    insertkeymap( tt, 2156, mkliteralevt("\377\354")) ; /* Alt-l */
    insertkeymap( tt, 2157, mkliteralevt("\377\355")) ; /* Alt-m */
    insertkeymap( tt, 2158, mkliteralevt("\377\356")) ; /* Alt-n */
    insertkeymap( tt, 2159, mkliteralevt("\377\357")) ; /* Alt-o */

    insertkeymap( tt, 2160, mkliteralevt("\377\360")) ; /* Alt-p */
    insertkeymap( tt, 2161, mkliteralevt("\377\361")) ; /* Alt-q */
    insertkeymap( tt, 2162, mkliteralevt("\377\362")) ; /* Alt-r */
    insertkeymap( tt, 2163, mkliteralevt("\377\363")) ; /* Alt-s */
    insertkeymap( tt, 2164, mkliteralevt("\377\364")) ; /* Alt-t */
    insertkeymap( tt, 2165, mkliteralevt("\377\365")) ; /* Alt-u */
    insertkeymap( tt, 2166, mkliteralevt("\377\366")) ; /* Alt-v */
    insertkeymap( tt, 2167, mkliteralevt("\377\367")) ; /* Alt-w */
    insertkeymap( tt, 2168, mkliteralevt("\377\370")) ; /* Alt-x */
    insertkeymap( tt, 2169, mkliteralevt("\377\371")) ; /* Alt-y */
    insertkeymap( tt, 2170, mkliteralevt("\377\372")) ; /* Alt-z */
    insertkeymap( tt, 2317, mkliteralevt("\377\373")) ; /* Alt-Enter */
    insertkeymap( tt, 776, mkliteralevt("\377\374")) ; /* Shift-BS */

    insertkeymap( tt, 5389, mkliteralevt("\377\320" )) ; /* gray-ctrl enter  */
    insertkeymap( tt, 4877, mkliteralevt("\377\340")) ; /* gray-Shift-Enter */
    insertkeymap( tt, 6413, mkliteralevt("\377\373")) ; /* gray-Alt-Enter */

    insertkeymap( tt, 4388, mkliteralevt("\377\240")) ; /* Gray-Home */
/*    insertkeymap( tt, 4390, mkliteralevt("\377\241")) ; /* Gray-Up */
    insertkeymap( tt, 4385, mkliteralevt("\377\242")) ; /* Gray-PgUp */
/*    insertkeymap( tt, 4389, mkliteralevt("\377\244")) ; /* Gray-Left */
/*    insertkeymap( tt, 4391, mkliteralevt("\377\246")) ; /* Gray-Right */
    insertkeymap( tt, 4387, mkliteralevt("\377\250")) ; /* Gray-End */
/*    insertkeymap( tt, 4392, mkliteralevt("\377\251")) ; /* Gray-Down */
    insertkeymap( tt, 4386, mkliteralevt("\377\252")) ; /* Gray-PgDn */
    insertkeymap( tt, 4397, mkliteralevt("\377\253")) ; /* Gray-Ins */
    insertkeymap( tt, 4398, mkliteralevt("\377\254")) ; /* Gray-Del */

    insertkeymap( tt, 5412, mkliteralevt("\377\260")) ; /* Gray-Ctrl-Home */
    insertkeymap( tt, 5414, mkliteralevt("\377\261")) ; /* Gray-Ctrl-Up */
    insertkeymap( tt, 5409, mkliteralevt("\377\262")) ; /* Gray-Ctrl-PgUp */
    insertkeymap( tt, 5413, mkliteralevt("\377\264")) ; /* Gray-Ctrl-Left */
    insertkeymap( tt, 5415, mkliteralevt("\377\266")) ; /* Gray-Ctrl-Right */
    insertkeymap( tt, 5411, mkliteralevt("\377\270")) ; /* Gray-Ctrl-End */
    insertkeymap( tt, 5416, mkliteralevt("\377\271")) ; /* Gray-Ctrl-Down */
    insertkeymap( tt, 5410, mkliteralevt("\377\272")) ; /* Gray-Ctrl-PgDn */
    insertkeymap( tt, 5421, mkliteralevt("\377\273")) ; /* Gray-Ctrl-Ins */
    insertkeymap( tt, 5422, mkliteralevt("\377\274")) ; /* Gray-Ctrl-Del */

    insertkeymap( tt, 6436, mkliteralevt("\377\300")) ; /* Gray-Alt-Home */
    insertkeymap( tt, 6438, mkliteralevt("\377\301")) ; /* Gray-Alt-Up */
    insertkeymap( tt, 6433, mkliteralevt("\377\302")) ; /* Gray-Alt-PgUp */
    insertkeymap( tt, 6437, mkliteralevt("\377\304")) ; /* Gray-Alt-Left */
    insertkeymap( tt, 6439, mkliteralevt("\377\306")) ; /* Gray-Alt-Right */
    insertkeymap( tt, 6435, mkliteralevt("\377\310")) ; /* Gray-Alt-End */
    insertkeymap( tt, 6440, mkliteralevt("\377\311")) ; /* Gray-Alt-Down */
    insertkeymap( tt, 6434, mkliteralevt("\377\312")) ; /* Gray-Alt-PgDn */
    insertkeymap( tt, 6445, mkliteralevt("\377\313")) ; /* Gray-Alt-Ins */
    insertkeymap( tt, 6446, mkliteralevt("\377\314")) ; /* Gray-Alt-Del */

    /* insertkeymap( tt, 875,  mkliteralevt("\377\247"));  /* Shift-Gray-Add */
    return 0;
}

int
defaixkm( int tt )
{
    defbasekm(tt);

    insertkeymap( tt, 264, mkkeyevt(BS));             /* Backspace             */
    insertkeymap( tt, 2312, mkliteralevt("\033[071q")); /* Alt-Backspace         */
    insertkeymap( tt, 1151, mkkeyevt(DEL));           /* Ctrl-Backspace        */

    insertkeymap( tt, 1289, mkliteralevt("\033[072q")); /* Ctrl-Tab              */
    insertkeymap( tt, 777, mkliteralevt("\033[Z")) ;    /* Shift-Tab           */

    insertkeymap( tt, 2317, mkliteralevt("\033[100q")) ; /* Alt-Enter            */

    insertkeymap( tt, 368, mkliteralevt("\033[001q")) ; /* f1                  */
    insertkeymap( tt, 369, mkliteralevt("\033[002q")) ; /* f2                  */
    insertkeymap( tt, 370, mkliteralevt("\033[003q")) ; /* f3                  */
    insertkeymap( tt, 371, mkliteralevt("\033[004q")) ; /* f4                  */
    insertkeymap( tt, 372, mkliteralevt("\033[005q")) ; /* f5                  */
    insertkeymap( tt, 373, mkliteralevt("\033[006q")) ; /* f6                  */
    insertkeymap( tt, 374, mkliteralevt("\033[007q")) ; /* f7                  */
    insertkeymap( tt, 375, mkliteralevt("\033[008q")) ; /* f8                  */
    insertkeymap( tt, 376, mkliteralevt("\033[009q")) ; /* f9                  */
    insertkeymap( tt, 377, mkliteralevt("\033[010q")) ; /* f10         */
    insertkeymap( tt, 378, mkliteralevt("\033[011q")) ; /* f11                 */
    insertkeymap( tt, 379, mkliteralevt("\033[012q")) ; /* f12         */

    insertkeymap( tt, 880, mkliteralevt("\033[013q")) ; /* shift f1            */
    insertkeymap( tt, 881, mkliteralevt("\033[014q")) ; /* shift f2            */
    insertkeymap( tt, 882, mkliteralevt("\033[015q")) ; /* shift f3            */
    insertkeymap( tt, 883, mkliteralevt("\033[016q")) ; /* shift f4            */
    insertkeymap( tt, 884, mkliteralevt("\033[017q")) ; /* shift f5            */
    insertkeymap( tt, 885, mkliteralevt("\033[018q")) ; /* shift f6            */
    insertkeymap( tt, 886, mkliteralevt("\033[019q")) ; /* shift f7            */
    insertkeymap( tt, 887, mkliteralevt("\033[020q")) ; /* shift f8            */
    insertkeymap( tt, 888, mkliteralevt("\033[021q")) ; /* shift f9            */
    insertkeymap( tt, 889, mkliteralevt("\033[022q")) ; /* shift f10      */
    insertkeymap( tt, 890, mkliteralevt("\033[023q")) ; /* shift f11           */
    insertkeymap( tt, 891, mkliteralevt("\033[024q")) ; /* shift f12      */

    insertkeymap( tt, 1392, mkliteralevt("\033[025q" )) ; /* control f1   */
    insertkeymap( tt, 1393, mkliteralevt("\033[026q" )) ; /* control f2   */
    insertkeymap( tt, 1394, mkliteralevt("\033[027q" )) ; /* control f3   */
    insertkeymap( tt, 1395, mkliteralevt("\033[028q" )) ; /* control f4   */
    insertkeymap( tt, 1396, mkliteralevt("\033[029q" )) ; /* control f5   */
    insertkeymap( tt, 1397, mkliteralevt("\033[030q" )) ; /* control f6   */
    insertkeymap( tt, 1398, mkliteralevt("\033[031q" )) ; /* control f7   */
    insertkeymap( tt, 1399, mkliteralevt("\033[032q" )) ; /* control f8   */
    insertkeymap( tt, 1400, mkliteralevt("\033[033q" )) ; /* control f9   */
    insertkeymap( tt, 1401, mkliteralevt("\033[034q" )) ; /* control f10       */
    insertkeymap( tt, 1402, mkliteralevt("\033[035q" )) ; /* control f11          */
    insertkeymap( tt, 1403, mkliteralevt("\033[036q" )) ; /* control f12       */

    insertkeymap( tt, 2416, mkliteralevt("\033[037q" )) ; /* alt f1       */
    insertkeymap( tt, 2417, mkliteralevt("\033[038q" )) ; /* alt f2       */
    insertkeymap( tt, 2418, mkliteralevt("\033[039q" )) ; /* alt f3       */
    insertkeymap( tt, 2419, mkliteralevt("\033[040q" )) ; /* alt f4       */
    insertkeymap( tt, 2420, mkliteralevt("\033[041q" )) ; /* alt f5       */
    insertkeymap( tt, 2421, mkliteralevt("\033[042q" )) ; /* alt f6       */
    insertkeymap( tt, 2422, mkliteralevt("\033[043q" )) ; /* alt f7       */
    insertkeymap( tt, 2423, mkliteralevt("\033[044q" )) ; /* alt f8       */
    insertkeymap( tt, 2424, mkliteralevt("\033[045q" )) ; /* alt f9       */
    insertkeymap( tt, 2425, mkliteralevt("\033[046q" )) ; /* alt f10       */
    insertkeymap( tt, 2426, mkliteralevt("\033[047q" )) ; /* alt f11      */
    insertkeymap( tt, 2427, mkliteralevt("\033[048q" )) ; /* alt f12       */

    insertkeymap( tt, 4388, mkliteralevt("\033[H")) ; /* Gray-Home */
    /* insertkeymap( tt, 4390, mkliteralevt("\033[A")) ; /* Gray-Up */
    insertkeymap( tt, 4385, mkliteralevt("\033[150q")) ; /* Gray-PgUp */
    /* insertkeymap( tt, 4389, mkliteralevt("\033[D")) ; /* Gray-Left */
    /* insertkeymap( tt, 4391, mkliteralevt("\033[C")) ; /* Gray-Right */
    insertkeymap( tt, 4387, mkliteralevt("\033[146q")) ; /* Gray-End */
    /* insertkeymap( tt, 4392, mkliteralevt("\033[B")) ; /* Gray-Down */
    insertkeymap( tt, 4386, mkliteralevt("\033[154q")) ; /* Gray-PgDn */
    insertkeymap( tt, 4397, mkliteralevt("\033[139q")) ; /* Gray-Ins */
    insertkeymap( tt, 4398, mkliteralevt("\033[P")) ; /* Gray-Del */

    insertkeymap( tt, 4900, mkliteralevt("\033[143q")) ; /* Gray-Shift-Home */
    insertkeymap( tt, 4902, mkliteralevt("\033[161q")) ; /* Gray-Shift-Up */
    insertkeymap( tt, 4897, mkliteralevt("\033[151q")) ; /* Gray-Shift-PgUp */
    insertkeymap( tt, 4901, mkliteralevt("\033[158q")) ; /* Gray-Shift-Left */
    insertkeymap( tt, 4903, mkliteralevt("\033[167q")) ; /* Gray-Shift-Right */
    insertkeymap( tt, 4899, mkliteralevt("\033[147q")) ; /* Gray-Shift-End */
    insertkeymap( tt, 4904, mkliteralevt("\033[164q")) ; /* Gray-Shift-Down */
    insertkeymap( tt, 4898, mkliteralevt("\033[155q")) ; /* Gray-Shift-PgDn */
    insertkeymap( tt, 4909, mkliteralevt("\033[139q")) ; /* Gray-Shift-Ins */
    insertkeymap( tt, 4910, mkliteralevt("\033[P")) ; /* Gray-Shift-Del */

    insertkeymap( tt, 5412, mkliteralevt("\033[144q")) ; /* Gray-Ctrl-Home */
    insertkeymap( tt, 5414, mkliteralevt("\033[162q")) ; /* Gray-Ctrl-Up */
    insertkeymap( tt, 5409, mkliteralevt("\033[152q")) ; /* Gray-Ctrl-PgUp */
    insertkeymap( tt, 5413, mkliteralevt("\033[159q")) ; /* Gray-Ctrl-Left */
    insertkeymap( tt, 5415, mkliteralevt("\033[168q")) ; /* Gray-Ctrl-Right */
    insertkeymap( tt, 5411, mkliteralevt("\033[148q")) ; /* Gray-Ctrl-End */
    insertkeymap( tt, 5416, mkliteralevt("\033[165q")) ; /* Gray-Ctrl-Down */
    insertkeymap( tt, 5410, mkliteralevt("\033[156q")) ; /* Gray-Ctrl-PgDn */
    insertkeymap( tt, 5421, mkliteralevt("\033[140q")) ; /* Gray-Ctrl-Ins */
    insertkeymap( tt, 5422, mkliteralevt("\033[142q")) ; /* Gray-Ctrl-Del */

    insertkeymap( tt, 6436, mkliteralevt("\033[145q")) ; /* Gray-Alt-Home */
    insertkeymap( tt, 6438, mkliteralevt("\033[162q")) ; /* Gray-Alt-Up */
    insertkeymap( tt, 6433, mkliteralevt("\033[153q")) ; /* Gray-Alt-PgUp */
    insertkeymap( tt, 6437, mkliteralevt("\033[160q")) ; /* Gray-Alt-Left */
    insertkeymap( tt, 6439, mkliteralevt("\033[169q")) ; /* Gray-Alt-Right */
    insertkeymap( tt, 6435, mkliteralevt("\033[149q")) ; /* Gray-Alt-End */
    insertkeymap( tt, 6440, mkliteralevt("\033[165q")) ; /* Gray-Alt-Down */
    insertkeymap( tt, 6434, mkliteralevt("\033[157q")) ; /* Gray-Alt-PgDn */
    insertkeymap( tt, 6445, mkliteralevt("\033[141q")) ; /* Gray-Alt-Ins */
    insertkeymap( tt, 6446, mkliteralevt("\033[M")) ; /* Gray-Alt-Del */

    insertkeymap( tt, 298, mkliteralevt("\033[209q")) ; /* PrtScrn */
    insertkeymap( tt, 1322, mkliteralevt("\033[211q")) ; /* Ctrl-PrtScrn */
    insertkeymap( tt, 2346, mkliteralevt("\033[212q" )) ; /* SysRq (Alt-PrtScrn)  */

    insertkeymap( tt, 1316, mkliteralevt("\033[172q")) ; /* Ctrl-Home */
    insertkeymap( tt, 1318, mkliteralevt("\033[182q")) ; /* Ctrl-Up */
    insertkeymap( tt, 1313, mkliteralevt("\033[190q")) ; /* Ctrl-PgUp */
    insertkeymap( tt, 1389, mkliteralevt("\033[198q")) ; /* Ctrl-Minus */
    insertkeymap( tt, 1317, mkliteralevt("\033[174q")) ; /* Ctrl-Left */
    insertkeymap( tt, 1292, mkliteralevt("\033[184q")) ; /* Ctrl-Blank */
    insertkeymap( tt, 1319, mkliteralevt("\033[192q")) ; /* Ctrl-Right */
    insertkeymap( tt, 1387, mkliteralevt("\033[200q")) ; /* Ctrl-Plus */
    insertkeymap( tt, 1315, mkliteralevt("\033[176q")) ; /* Ctrl-End */
    insertkeymap( tt, 1320, mkliteralevt("\033[186q")) ; /* Ctrl-Down */
    insertkeymap( tt, 1314, mkliteralevt("\033[194q")) ; /* Ctrl-PgDn */
    insertkeymap( tt, 1325, mkliteralevt("\033[178q")) ; /* Ctrl-Ins */
    insertkeymap( tt, 1326, mkliteralevt("\033[196q")) ; /* Ctrl-Del */
    insertkeymap( tt, 1386, mkliteralevt("\033[187q")) ; /* Ctrl-Multiply */
    insertkeymap( tt, 5487, mkliteralevt("\033[179q")) ; /* Ctrl-Divide */

    insertkeymap( tt, 2413, mkliteralevt("\033[199q")) ; /* Alt-Minus */
    insertkeymap( tt, 2411, mkliteralevt("\033[201q")) ; /* Alt-Plus */
    insertkeymap( tt, 2350, mkliteralevt("\033[197q")) ; /* Alt-Del */
    insertkeymap( tt, 2410, mkliteralevt("\033[188q")) ; /* Alt-Multiply */
    insertkeymap( tt, 6511, mkliteralevt("\033[180q")) ; /* Alt-Divide         */

    insertkeymap( tt, 1329, mkliteralevt("\033[049q")) ; /* Ctrl-1 */
    insertkeymap( tt, 1331, mkliteralevt("\033[050q")) ; /* Ctrl-3 */
    insertkeymap( tt, 1332, mkliteralevt("\033[051q")) ; /* Ctrl-4 */
    insertkeymap( tt, 1333, mkliteralevt("\033[052q")) ; /* Ctrl-5 */
    insertkeymap( tt, 1335, mkliteralevt("\033[053q")) ; /* Ctrl-7 */
    insertkeymap( tt, 1336, mkliteralevt("\033[054q")) ; /* Ctrl-8 */
    insertkeymap( tt, 1337, mkliteralevt("\033[055q")) ; /* Ctrl-9 */
    insertkeymap( tt, 1328, mkliteralevt("\033[056q")) ; /* Ctrl-0 */
    insertkeymap( tt, 1472, mkliteralevt("\033[057q")) ; /* Ctrl-` */

    insertkeymap( tt, 2097, mkliteralevt("\033[058q")) ; /* Alt-1 */
    insertkeymap( tt, 2098, mkliteralevt("\033[059q")) ; /* Alt-2 */
    insertkeymap( tt, 2099, mkliteralevt("\033[060q")) ; /* Alt-3 */
    insertkeymap( tt, 2100, mkliteralevt("\033[061q")) ; /* Alt-4 */
    insertkeymap( tt, 2101, mkliteralevt("\033[062q")) ; /* Alt-5 */
    insertkeymap( tt, 2102, mkliteralevt("\033[063q")) ; /* Alt-6 */
    insertkeymap( tt, 2103, mkliteralevt("\033[064q")) ; /* Alt-7 */
    insertkeymap( tt, 2104, mkliteralevt("\033[065q")) ; /* Alt-8 */
    insertkeymap( tt, 2105, mkliteralevt("\033[066q")) ; /* Alt-9 */
    insertkeymap( tt, 2096, mkliteralevt("\033[067q")) ; /* Alt-0 */

    insertkeymap( tt, 2161, mkliteralevt("\033[074q")) ; /* Alt-q */
    insertkeymap( tt, 2167, mkliteralevt("\033[075q")) ; /* Alt-w */
    insertkeymap( tt, 2149, mkliteralevt("\033[076q")) ; /* Alt-e */
    insertkeymap( tt, 2162, mkliteralevt("\033[077q")) ; /* Alt-r */
    insertkeymap( tt, 2164, mkliteralevt("\033[078q")) ; /* Alt-t */
    insertkeymap( tt, 2169, mkliteralevt("\033[079q")) ; /* Alt-y */
    insertkeymap( tt, 2165, mkliteralevt("\033[080q")) ; /* Alt-u */
    insertkeymap( tt, 2153, mkliteralevt("\033[081q")) ; /* Alt-i */
    insertkeymap( tt, 2159, mkliteralevt("\033[082q")) ; /* Alt-o */
    insertkeymap( tt, 2160, mkliteralevt("\033[083q")) ; /* Alt-p */

    insertkeymap( tt, 2145, mkliteralevt("\033[087q")) ; /* Alt-a */
    insertkeymap( tt, 2163, mkliteralevt("\033[088q")) ; /* Alt-s */
    insertkeymap( tt, 2148, mkliteralevt("\033[089q")) ; /* Alt-d */
    insertkeymap( tt, 2150, mkliteralevt("\033[090q")) ; /* Alt-f */
    insertkeymap( tt, 2151, mkliteralevt("\033[091q")) ; /* Alt-g */
    insertkeymap( tt, 2152, mkliteralevt("\033[092q")) ; /* Alt-h */
    insertkeymap( tt, 2154, mkliteralevt("\033[093q")) ; /* Alt-j */
    insertkeymap( tt, 2155, mkliteralevt("\033[094q")) ; /* Alt-k */
    insertkeymap( tt, 2156, mkliteralevt("\033[095q")) ; /* Alt-l */

    insertkeymap( tt, 2170, mkliteralevt("\033[101q")) ; /* Alt-z */
    insertkeymap( tt, 2168, mkliteralevt("\033[102q")) ; /* Alt-x */
    insertkeymap( tt, 2147, mkliteralevt("\033[103q")) ; /* Alt-c */
    insertkeymap( tt, 2166, mkliteralevt("\033[104q")) ; /* Alt-v */
    insertkeymap( tt, 2146, mkliteralevt("\033[105q")) ; /* Alt-b */
    insertkeymap( tt, 2158, mkliteralevt("\033[106q")) ; /* Alt-n */
    insertkeymap( tt, 2157, mkliteralevt("\033[107q")) ; /* Alt-m */

    return 0;
}


int
defsunkm( int tt )
{
    defbasekm(tt);

    /* The only ones that I am sure of are F1-F9 from the termcap entry */



    insertkeymap( tt, 264, mkkeyevt(BS));             /* Backspace             */

    insertkeymap( tt, 368, mkliteralevt("\033[224z")) ; /* f1          */
    insertkeymap( tt, 369, mkliteralevt("\033[225z")) ; /* f2          */
    insertkeymap( tt, 370, mkliteralevt("\033[226z")) ; /* f3          */
    insertkeymap( tt, 371, mkliteralevt("\033[227z")) ; /* f4          */
    insertkeymap( tt, 372, mkliteralevt("\033[228z")) ; /* f5          */
    insertkeymap( tt, 373, mkliteralevt("\033[229z")) ; /* f6          */
    insertkeymap( tt, 374, mkliteralevt("\033[230z")) ; /* f7          */
    insertkeymap( tt, 375, mkliteralevt("\033[231z")) ; /* f8          */
    insertkeymap( tt, 376, mkliteralevt("\033[232z")) ; /* f9          */
    insertkeymap( tt, 377, mkliteralevt("\033[233z")) ; /* f10         */
    insertkeymap( tt, 378, mkliteralevt("\033[234z")) ; /* f11         */
    insertkeymap( tt, 379, mkliteralevt("\033[235z")) ; /* f12         */

    insertkeymap( tt, 880, mkliteralevt("\033[224z")) ; /* shift f1            */
    insertkeymap( tt, 881, mkliteralevt("\033[225z")) ; /* shift f2            */
    insertkeymap( tt, 882, mkliteralevt("\033[226z")) ; /* shift f3            */
    insertkeymap( tt, 883, mkliteralevt("\033[227z")) ; /* shift f4            */
    insertkeymap( tt, 884, mkliteralevt("\033[228z")) ; /* shift f5            */
    insertkeymap( tt, 885, mkliteralevt("\033[229z")) ; /* shift f6            */
    insertkeymap( tt, 886, mkliteralevt("\033[230z")) ; /* shift f7            */
    insertkeymap( tt, 887, mkliteralevt("\033[231z")) ; /* shift f8            */
    insertkeymap( tt, 888, mkliteralevt("\033[232z")) ; /* shift f9            */
    insertkeymap( tt, 889, mkliteralevt("\033[233z")) ; /* shift f10      */
    insertkeymap( tt, 890, mkliteralevt("\033[234z")) ; /* shift f11           */
    insertkeymap( tt, 891, mkliteralevt("\033[235z")) ; /* shift f12      */

    insertkeymap( tt, 1392, mkliteralevt("\033[224z" )) ; /* control f1   */
    insertkeymap( tt, 1393, mkliteralevt("\033[225z" )) ; /* control f2   */
    insertkeymap( tt, 1394, mkliteralevt("\033[226z" )) ; /* control f3   */
    insertkeymap( tt, 1395, mkliteralevt("\033[227z" )) ; /* control f4   */
    insertkeymap( tt, 1396, mkliteralevt("\033[228z" )) ; /* control f5   */
    insertkeymap( tt, 1397, mkliteralevt("\033[229z" )) ; /* control f6   */
    insertkeymap( tt, 1398, mkliteralevt("\033[230z" )) ; /* control f7   */
    insertkeymap( tt, 1399, mkliteralevt("\033[231z" )) ; /* control f8   */
    insertkeymap( tt, 1400, mkliteralevt("\033[232z" )) ; /* control f9   */
    insertkeymap( tt, 1401, mkliteralevt("\033[233z" )) ; /* control f10       */
    insertkeymap( tt, 1402, mkliteralevt("\033[234z" )) ; /* control f11          */
    insertkeymap( tt, 1403, mkliteralevt("\033[235z" )) ; /* control f12       */

    insertkeymap( tt, 2416, mkliteralevt("\033[224z" )) ; /* alt f1       */
    insertkeymap( tt, 2417, mkliteralevt("\033[225z" )) ; /* alt f2       */
    insertkeymap( tt, 2418, mkliteralevt("\033[226z" )) ; /* alt f3       */
    insertkeymap( tt, 2419, mkliteralevt("\033[227z" )) ; /* alt f4       */
    insertkeymap( tt, 2420, mkliteralevt("\033[228z" )) ; /* alt f5       */
    insertkeymap( tt, 2421, mkliteralevt("\033[229z" )) ; /* alt f6       */
    insertkeymap( tt, 2422, mkliteralevt("\033[230z" )) ; /* alt f7       */
    insertkeymap( tt, 2423, mkliteralevt("\033[231z" )) ; /* alt f8       */
    insertkeymap( tt, 2424, mkliteralevt("\033[232z" )) ; /* alt f9       */
    insertkeymap( tt, 2425, mkliteralevt("\033[233z" )) ; /* alt f10       */
    insertkeymap( tt, 2426, mkliteralevt("\033[234z" )) ; /* alt f11      */
    insertkeymap( tt, 2427, mkliteralevt("\033[235z" )) ; /* alt f12       */

    insertkeymap( tt, 292, mkliteralevt("\033[214z")) ; /* Home */
    /* insertkeymap( tt, 294, mkliteralevt("\033[A")) ;    /* Up */
    insertkeymap( tt, 289, mkliteralevt("\033[216z")) ; /* PgUp */
    insertkeymap( tt, 365, mkliteralevt("\033[254z")) ; /* Minus */
    /* insertkeymap( tt, 293, mkliteralevt("\033[D")) ;    /* Left */
    insertkeymap( tt, 268, mkliteralevt("\033[218z")) ; /* Blank */
    /* insertkeymap( tt, 295, mkliteralevt("\033[C")) ;    /* Right */
    insertkeymap( tt, 363, mkliteralevt("\033[253z")) ; /* Plus */
    insertkeymap( tt, 291, mkliteralevt("\033[220z")) ; /* End */
    /* insertkeymap( tt, 296, mkliteralevt("\033[B")) ;    /* Down */
    insertkeymap( tt, 290, mkliteralevt("\033[222z")) ; /* PgDn */
    insertkeymap( tt, 301, mkliteralevt("\033[247z")) ; /* Ins */
    insertkeymap( tt, 302, mkliteralevt("\033[249z")) ; /* Del */
    insertkeymap( tt, 362, mkliteralevt("\033[213z")) ; /* Multiply */
    insertkeymap( tt, 4463, mkliteralevt("\033[212z")) ; /* Divide - NT*/
    insertkeymap( tt, 4143, mkliteralevt("\033[212z")) ; /* Divide - 95*/

    insertkeymap( tt, 4388, mkliteralevt("\033[214z")) ; /* Gray-Home */
    /* insertkeymap( tt, 4390, mkliteralevt("\033[A")) ; /* Gray-Up */
    insertkeymap( tt, 4385, mkliteralevt("\033[216z")) ; /* Gray-PgUp */
    /* insertkeymap( tt, 4389, mkliteralevt("\033[D")) ; /* Gray-Left */
    /* insertkeymap( tt, 4391, mkliteralevt("\033[C")) ; /* Gray-Right */
    insertkeymap( tt, 4387, mkliteralevt("\033[220z")) ; /* Gray-End */
    /* insertkeymap( tt, 4392, mkliteralevt("\033[B")) ; /* Gray-Down */
    insertkeymap( tt, 4386, mkliteralevt("\033[222z")) ; /* Gray-PgDn */
    insertkeymap( tt, 4397, mkliteralevt("\033[247z")) ; /* Gray-Ins */
    insertkeymap( tt, 4398, mkkeyevt(DEL)) ;             /* Gray-Del */
    insertkeymap( tt, 4365, mkkeyevt(LF)) ;              /* Gray-Enter */

    insertkeymap( tt, 4900, mkliteralevt("\033[214z")) ; /* Gray-Shift-Home */
    insertkeymap( tt, 4902, mkliteralevt("\033[A")) ;    /* Gray-Shift-Up */
    insertkeymap( tt, 4897, mkliteralevt("\033[216z")) ; /* Gray-Shift-PgUp */
    insertkeymap( tt, 4901, mkliteralevt("\033[D")) ;    /* Gray-Shift-Left */
    insertkeymap( tt, 4903, mkliteralevt("\033[C")) ;    /* Gray-Shift-Right */
    insertkeymap( tt, 4899, mkliteralevt("\033[220z")) ; /* Gray-Shift-End */
    insertkeymap( tt, 4904, mkliteralevt("\033[B")) ;    /* Gray-Shift-Down */
    insertkeymap( tt, 4898, mkliteralevt("\033[222z")) ; /* Gray-Shift-PgDn */
    insertkeymap( tt, 4909, mkliteralevt("\033[247z")) ; /* Gray-Shift-Ins */
    insertkeymap( tt, 4910, mkkeyevt(DEL)) ;             /* Gray-Shift-Del */

    insertkeymap( tt, 5412, mkliteralevt("\033[214z")) ; /* Gray-Ctrl-Home */
    insertkeymap( tt, 5414, mkliteralevt("\033[A")) ;    /* Gray-Ctrl-Up */
    insertkeymap( tt, 5409, mkliteralevt("\033[216z")) ; /* Gray-Ctrl-PgUp */
    insertkeymap( tt, 5413, mkliteralevt("\033[D")) ;    /* Gray-Ctrl-Left */
    insertkeymap( tt, 5415, mkliteralevt("\033[C")) ;    /* Gray-Ctrl-Right */
    insertkeymap( tt, 5411, mkliteralevt("\033[220z")) ; /* Gray-Ctrl-End */
    insertkeymap( tt, 5416, mkliteralevt("\033[B")) ;    /* Gray-Ctrl-Down */
    insertkeymap( tt, 5410, mkliteralevt("\033[222z")) ; /* Gray-Ctrl-PgDn */
    insertkeymap( tt, 5421, mkliteralevt("\033[247z")) ; /* Gray-Ctrl-Ins */
    insertkeymap( tt, 5422, mkkeyevt(DEL)) ;             /* Gray-Ctrl-Del */

    insertkeymap( tt, 6436, mkliteralevt("\033[214z")) ; /* Gray-Alt-Home */
    insertkeymap( tt, 6438, mkliteralevt("\033[A")) ;    /* Gray-Alt-Up */
    insertkeymap( tt, 6433, mkliteralevt("\033[216z")) ; /* Gray-Alt-PgUp */
    insertkeymap( tt, 6437, mkliteralevt("\033[D")) ;    /* Gray-Alt-Left */
    insertkeymap( tt, 6439, mkliteralevt("\033[C")) ;    /* Gray-Alt-Right */
    insertkeymap( tt, 6435, mkliteralevt("\033[220z")) ; /* Gray-Alt-End */
    insertkeymap( tt, 6440, mkliteralevt("\033[B")) ;    /* Gray-Alt-Down */
    insertkeymap( tt, 6434, mkliteralevt("\033[222z")) ; /* Gray-Alt-PgDn */
    insertkeymap( tt, 6445, mkliteralevt("\033[247z")) ; /* Gray-Alt-Ins */
    insertkeymap( tt, 6446, mkkeyevt(DEL)) ;             /* Gray-Alt-Del */

    insertkeymap( tt, 298, mkliteralevt("\033[209z")) ; /* PrtScrn */
    insertkeymap( tt, 1322, mkliteralevt("\033[209z")) ; /* Ctrl-PrtScrn */
    insertkeymap( tt, 2346, mkliteralevt("\033[209z" )) ; /* SysRq (Alt-PrtScrn)  */

    insertkeymap( tt, 401, mkliteralevt("\033[210z"));  /* Scroll-Lock */
    insertkeymap( tt, 275, mkliteralevt("\033[208z"));  /* Pause */

    insertkeymap( tt, 1316, mkliteralevt("\033[214z")) ; /* Ctrl-Home */
    insertkeymap( tt, 1318, mkliteralevt("\033[A")) ;    /* Ctrl-Up */
    insertkeymap( tt, 1313, mkliteralevt("\033[216z")) ; /* Ctrl-PgUp */
    insertkeymap( tt, 1389, mkliteralevt("\033[254z")) ; /* Ctrl-Minus */
    insertkeymap( tt, 1317, mkliteralevt("\033[D")) ;    /* Ctrl-Left */
    insertkeymap( tt, 1292, mkliteralevt("\033[218z")) ; /* Ctrl-Blank */
    insertkeymap( tt, 1319, mkliteralevt("\033[C")) ;    /* Ctrl-Right */
    insertkeymap( tt, 1387, mkliteralevt("\033[253z")) ; /* Ctrl-Plus */
    insertkeymap( tt, 1315, mkliteralevt("\033[220z")) ; /* Ctrl-End */
    insertkeymap( tt, 1320, mkliteralevt("\033[B")) ;    /* Ctrl-Down */
    insertkeymap( tt, 1314, mkliteralevt("\033[222z")) ; /* Ctrl-PgDn */
    insertkeymap( tt, 1325, mkliteralevt("\033[247z")) ; /* Ctrl-Ins */
    insertkeymap( tt, 1326, mkliteralevt("\033[249z")) ; /* Ctrl-Del */
    insertkeymap( tt, 1386, mkliteralevt("\033[213z")) ; /* Ctrl-Multiply */
    insertkeymap( tt, 5487, mkliteralevt("\033[212z")) ; /* Ctrl-Divide */

    insertkeymap( tt, 2413, mkliteralevt("\033[199q")) ; /* Alt-Minus */
    insertkeymap( tt, 2411, mkliteralevt("\033[201q")) ; /* Alt-Plus */
    insertkeymap( tt, 2350, mkliteralevt("\033[197q")) ; /* Alt-Del */
    insertkeymap( tt, 2410, mkliteralevt("\033[188q")) ; /* Alt-Multiply */
    insertkeymap( tt, 6511, mkliteralevt("\033[180q")) ; /* Alt-Divide         */

    insertkeymap( tt, 1329, mkkeyevt(NUL)) ;             /* Ctrl-2 */
    insertkeymap( tt, 1334, mkkeyevt(XRS)) ;             /* Ctrl-6 */
    insertkeymap( tt, 1469, mkkeyevt(US)) ;              /* Ctrl-- */
    insertkeymap( tt, 1471, mkkeyevt(US)) ;              /* Ctrl-? */
    insertkeymap( tt, 1472, mkkeyevt(XFS)) ;             /* Ctrl-` */

    return 0;
}

int
defqansikm( int tt )
{
    defbasekm(tt);

    insertkeymap( tt, 264, mkkeyevt(DEL));           /* Backspace              */

    insertkeymap( tt, 777, mkliteralevt("\033[Z")) ;    /* Shift-Tab           */
    insertkeymap( tt, 368, mkliteralevt("\033OP")) ; /* f1                     */
    insertkeymap( tt, 369, mkliteralevt("\033OQ")) ; /* f2                     */
    insertkeymap( tt, 370, mkliteralevt("\033OR")) ; /* f3                     */
    insertkeymap( tt, 371, mkliteralevt("\033OS")) ; /* f4                     */
    insertkeymap( tt, 372, mkliteralevt("\033OT")) ; /* f5                     */
    insertkeymap( tt, 373, mkliteralevt("\033OU")) ; /* f6                     */
    insertkeymap( tt, 374, mkliteralevt("\033OV")) ; /* f7                     */
    insertkeymap( tt, 375, mkliteralevt("\033OW")) ; /* f8                     */
    insertkeymap( tt, 376, mkliteralevt("\033OX")) ; /* f9                     */
    insertkeymap( tt, 377, mkliteralevt("\033OY")) ; /* f10            */

#ifdef COMMENT
    /* do not know the sequences for Ctrl,Alt,Shift function keys */
    insertkeymap( tt, 880, mkliteralevt("\033")) ; /* shift f1         */
    insertkeymap( tt, 881, mkliteralevt("\033")) ; /* shift f2         */
    insertkeymap( tt, 882, mkliteralevt("\033")) ; /* shift f3         */
    insertkeymap( tt, 883, mkliteralevt("\033")) ; /* shift f4         */
    insertkeymap( tt, 884, mkliteralevt("\033")) ; /* shift f5         */
    insertkeymap( tt, 885, mkliteralevt("\033")) ; /* shift f6         */
    insertkeymap( tt, 886, mkliteralevt("\033")) ; /* shift f7         */
    insertkeymap( tt, 887, mkliteralevt("\033")) ; /* shift f8         */
    insertkeymap( tt, 888, mkliteralevt("\033")) ; /* shift f9         */
    insertkeymap( tt, 889, mkliteralevt("\033")) ; /* shift f10   */

    insertkeymap( tt, 1392, mkliteralevt("\033" )) ; /* control f1        */
    insertkeymap( tt, 1393, mkliteralevt("\033" )) ; /* control f2        */
    insertkeymap( tt, 1394, mkliteralevt("\033" )) ; /* control f3        */
    insertkeymap( tt, 1395, mkliteralevt("\033" )) ; /* control f4        */
    insertkeymap( tt, 1396, mkliteralevt("\033" )) ; /* control f5        */
    insertkeymap( tt, 1397, mkliteralevt("\033" )) ; /* control f6        */
    insertkeymap( tt, 1398, mkliteralevt("\033" )) ; /* control f7        */
    insertkeymap( tt, 1399, mkliteralevt("\033" )) ; /* control f8        */
    insertkeymap( tt, 1400, mkliteralevt("\033" )) ; /* control f9        */
    insertkeymap( tt, 1401, mkliteralevt("\033" )) ; /* control f10       */
    insertkeymap( tt, 1289, mkliteralevt("\033")) ; /* Ctrl-Tab  */
#endif /* COMMENT */

    insertkeymap( tt, 292, mkliteralevt("\033[H")) ; /* Home */
/*    insertkeymap( tt, 294, mkliteralevt("\033[A")) ; /* Up */
    insertkeymap( tt, 289, mkliteralevt("\033[V")) ; /* PgUp */
/*    insertkeymap( tt, 365, mkliteralevt("\033")) ; /* Minus */
/*    insertkeymap( tt, 293, mkliteralevt("\033[D")) ; /* Left */
/*    insertkeymap( tt, 268, mkliteralevt("\033")) ; /* Blank */
/*    insertkeymap( tt, 295, mkliteralevt("\033[C")) ; /* Right */
/*    insertkeymap( tt, 363, mkliteralevt("\033")) ; /* Plus */
/*    insertkeymap( tt, 291, mkliteralevt("\033")) ; /* End */
/*    insertkeymap( tt, 296, mkliteralevt("\033\251")) ; /* Down */
    insertkeymap( tt, 290, mkliteralevt("\033[U")) ; /* PgDn */
    insertkeymap( tt, 301, mkliteralevt("\033[@")) ; /* Ins */
    insertkeymap( tt, 302, mkliteralevt("\033[P")) ; /* Del */

#ifdef COMMENT
    /* do not know the values to send */
    insertkeymap( tt, 298, mkliteralevt("\033")) ; /* PrtScrn */
    insertkeymap( tt, 378, mkliteralevt("\033\256")) ; /* f11          */
    insertkeymap( tt, 379, mkliteralevt("\033\257")) ; /* f12          */


    insertkeymap( tt, 1316, mkliteralevt("\033\260")) ; /* Ctrl-Home */
#endif /* COMMENT */

    insertkeymap( tt, 1318, mkliteralevt("\033[a")) ; /* Ctrl-Up */

#ifdef COMMENT
    insertkeymap( tt, 1313, mkliteralevt("\033\262")) ; /* Ctrl-PgUp */
    insertkeymap( tt, 1389, mkliteralevt("\033\263")) ; /* Ctrl-Minus */
    insertkeymap( tt, 1317, mkliteralevt("\033\264")) ; /* Ctrl-Left */
    insertkeymap( tt, 1292, mkliteralevt("\033\265")) ; /* Ctrl-Blank */
    insertkeymap( tt, 1319, mkliteralevt("\033\266")) ; /* Ctrl-Right */
    insertkeymap( tt, 1387, mkliteralevt("\033\267")) ; /* Ctrl-Plus */
    insertkeymap( tt, 1315, mkliteralevt("\033\270")) ; /* Ctrl-End */
#endif /* COMMENT */

    insertkeymap( tt, 1320, mkliteralevt("\033[b")) ; /* Ctrl-Down */

#ifdef COMMENT
    insertkeymap( tt, 1314, mkliteralevt("\033\272")) ; /* Ctrl-PgDn */
    insertkeymap( tt, 1325, mkliteralevt("\033\273")) ; /* Ctrl-Ins */
    insertkeymap( tt, 1326, mkliteralevt("\033\274")) ; /* Ctrl-Del */
    insertkeymap( tt, 1322, mkliteralevt("\033\275")) ; /* Ctrl-PrtScrn */
    insertkeymap( tt, 1402, mkliteralevt("\033\276")) ; /* Ctrl-f11            */
    insertkeymap( tt, 1403, mkliteralevt("\033\277")) ; /* Ctrl-f12            */

    insertkeymap( tt, 2340, mkliteralevt("\033\300")) ; /* Alt-Home */
    insertkeymap( tt, 2342, mkliteralevt("\033\301")) ; /* Alt-Up */
    insertkeymap( tt, 2337, mkliteralevt("\033\302")) ; /* Alt-PgUp */
    insertkeymap( tt, 2413, mkliteralevt("\033\303")) ; /* Alt-Minus */
    insertkeymap( tt, 2341, mkliteralevt("\033\304")) ; /* Alt-Left */
    insertkeymap( tt, 2316, mkliteralevt("\033\305")) ; /* Alt-Blank */
    insertkeymap( tt, 2343, mkliteralevt("\033\306")) ; /* Alt-Right */
    insertkeymap( tt, 2411, mkliteralevt("\033\307")) ; /* Alt-Plus */
    insertkeymap( tt, 2339, mkliteralevt("\033\310")) ; /* Alt-End */
    insertkeymap( tt, 2344, mkliteralevt("\033\311")) ; /* Alt-Down */
    insertkeymap( tt, 2338, mkliteralevt("\033\312")) ; /* Alt-PgDn */
    insertkeymap( tt, 2349, mkliteralevt("\033\313")) ; /* Alt-Ins */
    insertkeymap( tt, 2350, mkliteralevt("\033\314")) ; /* Alt-Del */
    insertkeymap( tt, 2410, mkliteralevt("\033\315")) ; /* Alt-* */
    insertkeymap( tt, 2426, mkliteralevt("\033\316")) ; /* Alt-f11             */
    insertkeymap( tt, 2427, mkliteralevt("\033\317")) ; /* Alt-f12             */


    insertkeymap( tt, 1293, mkliteralevt("\033\320" )) ; /* ctrl enter  */
    insertkeymap( tt, 2416, mkliteralevt("\033\321" )) ; /* alt f1  */
    insertkeymap( tt, 2417, mkliteralevt("\033\322" )) ; /* alt f2  */
    insertkeymap( tt, 2418, mkliteralevt("\033\323" )) ; /* alt f3  */
    insertkeymap( tt, 2419, mkliteralevt("\033\324" )) ; /* alt f4  */
    insertkeymap( tt, 2420, mkliteralevt("\033\325" )) ; /* alt f5  */
    insertkeymap( tt, 2421, mkliteralevt("\033\326" )) ; /* alt f6  */
    insertkeymap( tt, 2422, mkliteralevt("\033\327" )) ; /* alt f7  */
    insertkeymap( tt, 2423, mkliteralevt("\033\330" )) ; /* alt f8  */
    insertkeymap( tt, 2424, mkliteralevt("\033\331" )) ; /* alt f9  */
    insertkeymap( tt, 2425, mkliteralevt("\033\332" )) ; /* alt f10 */
    insertkeymap( tt,  890, mkliteralevt("\033\333" )) ; /* shift f11     */
    insertkeymap( tt,  891, mkliteralevt("\033\334" )) ; /* shift f12     */
    insertkeymap( tt, 2346, mkliteralevt("\033\335" )) ; /* SysRq (Alt-PrtScrn)  */
    insertkeymap( tt, 1151, mkliteralevt("\033\336" )) ; /* Ctrl-BS  */
    insertkeymap( tt, 2312, mkliteralevt("\033\337" )) ; /* Alt-BS  */

    insertkeymap( tt, 781, mkliteralevt("\033\340")) ; /* Shift-Enter */
#endif /* COMMENT */

    insertkeymap( tt, 2145, mkliteralevt("\033Na")) ; /* Alt-a */
    insertkeymap( tt, 2146, mkliteralevt("\033Nb")) ; /* Alt-b */
    insertkeymap( tt, 2147, mkliteralevt("\033Nc")) ; /* Alt-c */
    insertkeymap( tt, 2148, mkliteralevt("\033Nd")) ; /* Alt-d */
    insertkeymap( tt, 2149, mkliteralevt("\033Ne")) ; /* Alt-e */
    insertkeymap( tt, 2150, mkliteralevt("\033Nf")) ; /* Alt-f */
    insertkeymap( tt, 2151, mkliteralevt("\033Ng")) ; /* Alt-g */
    insertkeymap( tt, 2152, mkliteralevt("\033Nh")) ; /* Alt-h */
    insertkeymap( tt, 2153, mkliteralevt("\033Ni")) ; /* Alt-i */
    insertkeymap( tt, 2154, mkliteralevt("\033Nj")) ; /* Alt-j */
    insertkeymap( tt, 2155, mkliteralevt("\033Nk")) ; /* Alt-k */
    insertkeymap( tt, 2156, mkliteralevt("\033Nl")) ; /* Alt-l */
    insertkeymap( tt, 2157, mkliteralevt("\033Nm")) ; /* Alt-m */
    insertkeymap( tt, 2158, mkliteralevt("\033Nn")) ; /* Alt-n */
    insertkeymap( tt, 2159, mkliteralevt("\033No")) ; /* Alt-o */

    insertkeymap( tt, 2160, mkliteralevt("\033Np")) ; /* Alt-p */
    insertkeymap( tt, 2161, mkliteralevt("\033Nq")) ; /* Alt-q */
    insertkeymap( tt, 2162, mkliteralevt("\033Nr")) ; /* Alt-r */
    insertkeymap( tt, 2163, mkliteralevt("\033Ns")) ; /* Alt-s */
    insertkeymap( tt, 2164, mkliteralevt("\033Nt")) ; /* Alt-t */
    insertkeymap( tt, 2165, mkliteralevt("\033Nu")) ; /* Alt-u */
    insertkeymap( tt, 2166, mkliteralevt("\033Nv")) ; /* Alt-v */
    insertkeymap( tt, 2167, mkliteralevt("\033Nw")) ; /* Alt-w */
    insertkeymap( tt, 2168, mkliteralevt("\033Nx")) ; /* Alt-x */
    insertkeymap( tt, 2169, mkliteralevt("\033Ny")) ; /* Alt-y */
    insertkeymap( tt, 2170, mkliteralevt("\033Nz")) ; /* Alt-z */

#ifdef COMMENT
    insertkeymap( tt, 2317, mkliteralevt("\033\373")) ; /* Alt-Enter */
#endif
    insertkeymap( tt, 776, mkliteralevt("\033[Z")) ; /* Shift-BS */

#ifdef COMMENT
    insertkeymap( tt, 5389, mkliteralevt("\033\320" )) ; /* gray-ctrl enter  */
    insertkeymap( tt, 4877, mkliteralevt("\033\340")) ; /* gray-Shift-Enter */
    insertkeymap( tt, 6413, mkliteralevt("\033\373")) ; /* gray-Alt-Enter */
#endif /* COMMENT */

    insertkeymap( tt, 4388, mkliteralevt("\033[H")) ; /* Gray-Home */
/*    insertkeymap( tt, 4390, mkliteralevt("\033[A")) ; /* Gray-Up */
    insertkeymap( tt, 4385, mkliteralevt("\033[V")) ; /* Gray-PgUp */
/*    insertkeymap( tt, 4389, mkliteralevt("\033[D")) ; /* Gray-Left */
/*    insertkeymap( tt, 4391, mkliteralevt("\033[C")) ; /* Gray-Right */
/*    insertkeymap( tt, 4387, mkliteralevt("\033")) ; /* Gray-End */
/*    insertkeymap( tt, 4392, mkliteralevt("\033[B")) ; /* Gray-Down */
    insertkeymap( tt, 4386, mkliteralevt("\033[U")) ; /* Gray-PgDn */
    insertkeymap( tt, 4397, mkliteralevt("\033[@")) ; /* Gray-Ins */
    insertkeymap( tt, 4398, mkliteralevt("\033[P")) ; /* Gray-Del */

/*    insertkeymap( tt, 5412, mkliteralevt("\033\260")) ; /* Gray-Ctrl-Home */
    insertkeymap( tt, 5414, mkliteralevt("\033[a")) ; /* Gray-Ctrl-Up */
#ifdef COMMENT
    insertkeymap( tt, 5409, mkliteralevt("\033\262")) ; /* Gray-Ctrl-PgUp */
    insertkeymap( tt, 5413, mkliteralevt("\033\264")) ; /* Gray-Ctrl-Left */
    insertkeymap( tt, 5415, mkliteralevt("\033\266")) ; /* Gray-Ctrl-Right */
    insertkeymap( tt, 5411, mkliteralevt("\033\270")) ; /* Gray-Ctrl-End */
#endif
    insertkeymap( tt, 5416, mkliteralevt("\033[b")) ; /* Gray-Ctrl-Down */
#ifdef COMMENT
    insertkeymap( tt, 5410, mkliteralevt("\033\272")) ; /* Gray-Ctrl-PgDn */
    insertkeymap( tt, 5421, mkliteralevt("\033\273")) ; /* Gray-Ctrl-Ins */
    insertkeymap( tt, 5422, mkliteralevt("\033\274")) ; /* Gray-Ctrl-Del */

    insertkeymap( tt, 6436, mkliteralevt("\033\300")) ; /* Gray-Alt-Home */
    insertkeymap( tt, 6438, mkliteralevt("\033\301")) ; /* Gray-Alt-Up */
    insertkeymap( tt, 6433, mkliteralevt("\033\302")) ; /* Gray-Alt-PgUp */
    insertkeymap( tt, 6437, mkliteralevt("\033\304")) ; /* Gray-Alt-Left */
    insertkeymap( tt, 6439, mkliteralevt("\033\306")) ; /* Gray-Alt-Right */
    insertkeymap( tt, 6435, mkliteralevt("\033\310")) ; /* Gray-Alt-End */
    insertkeymap( tt, 6440, mkliteralevt("\033\311")) ; /* Gray-Alt-Down */
    insertkeymap( tt, 6434, mkliteralevt("\033\312")) ; /* Gray-Alt-PgDn */
    insertkeymap( tt, 6445, mkliteralevt("\033\313")) ; /* Gray-Alt-Ins */
    insertkeymap( tt, 6446, mkliteralevt("\033\314")) ; /* Gray-Alt-Del */
#endif /* COMMENT */
    return 0;
}

int
defba80km( int tt )
{
    defbasekm(tt);
    insertkeymap( tt, 368 , mkkeyevt(F_KVERB | K_BA80_PA01 )); /* F1               F1     */
    insertkeymap( tt, 369 , mkkeyevt(F_KVERB | K_BA80_PA02 )); /* F2               F2     */
    insertkeymap( tt, 370 , mkkeyevt(F_KVERB | K_BA80_PA03 )); /* F3               F3     */
    insertkeymap( tt, 371 , mkkeyevt(F_KVERB | K_BA80_PA04 )); /* F4               F4     */
    insertkeymap( tt, 372 , mkkeyevt(F_KVERB | K_BA80_PA05 )); /* F5               F5     */
    insertkeymap( tt, 373 , mkkeyevt(F_KVERB | K_BA80_PA06 )); /* F6               F6     */
    insertkeymap( tt, 374 , mkkeyevt(F_KVERB | K_BA80_PA07 )); /* F7               F7     */
    insertkeymap( tt, 375 , mkkeyevt(F_KVERB | K_BA80_PA08 )); /* F8               F8     */
    insertkeymap( tt, 376 , mkkeyevt(F_KVERB | K_BA80_PA09 )); /* F9               F9     */
    insertkeymap( tt, 377 , mkkeyevt(F_KVERB | K_BA80_PA10 )); /* F10              F10    */
    insertkeymap( tt, 378 , mkkeyevt(F_KVERB | K_BA80_PA11 )); /* F11              F11    */
    insertkeymap( tt, 379 , mkkeyevt(F_KVERB | K_BA80_PA12 )); /* F12              F12    */

    insertkeymap( tt, 2416, mkkeyevt(F_KVERB | K_BA80_PA13 )); /* F13               Alt-F13 */
    insertkeymap( tt, 2417, mkkeyevt(F_KVERB | K_BA80_PA14 )); /* F14               Alt-F14 */
    insertkeymap( tt, 2418, mkkeyevt(F_KVERB | K_BA80_PA15 )); /* F15               Alt-F15 */
    insertkeymap( tt, 2419, mkkeyevt(F_KVERB | K_BA80_PA16 )); /* F16               Alt-F16 */
    insertkeymap( tt, 2420, mkkeyevt(F_KVERB | K_BA80_PA17 )); /* F17               Alt-F17 */
    insertkeymap( tt, 2421, mkkeyevt(F_KVERB | K_BA80_PA18 )); /* F18               Alt-F18 */
    insertkeymap( tt, 2422, mkkeyevt(F_KVERB | K_BA80_PA19 )); /* F19               Alt-F19 */
    insertkeymap( tt, 2423, mkkeyevt(F_KVERB | K_BA80_PA20 )); /* F20               Alt-F20 */
    insertkeymap( tt, 2424, mkkeyevt(F_KVERB | K_BA80_PA21 )); /* F21               Alt-F21 */
    insertkeymap( tt, 2425, mkkeyevt(F_KVERB | K_BA80_PA22 )); /* F22               Alt-F22 */
    insertkeymap( tt, 2426, mkkeyevt(F_KVERB | K_BA80_PA23 )); /* F23               Alt-F23 */
    insertkeymap( tt, 2427, mkkeyevt(F_KVERB | K_BA80_PA24 )); /* F24               Alt-F24 */

    insertkeymap( tt, 2097, mkkeyevt(F_KVERB | K_BA80_SOFTKEY1 )); /* Softkey1         Alt-1 */
    insertkeymap( tt, 2098, mkkeyevt(F_KVERB | K_BA80_SOFTKEY2 )); /* Softkey2         Alt-2 */
    insertkeymap( tt, 2099, mkkeyevt(F_KVERB | K_BA80_SOFTKEY3 )); /* Softkey3         Alt-3 */
    insertkeymap( tt, 2100, mkkeyevt(F_KVERB | K_BA80_SOFTKEY4 )); /* Softkey4         Alt-4 */
    insertkeymap( tt, 2101, mkkeyevt(F_KVERB | K_BA80_SOFTKEY5 )); /* Softkey5         Alt-5 */
    insertkeymap( tt, 2102, mkkeyevt(F_KVERB | K_BA80_SOFTKEY6 )); /* Softkey6         Alt-6 */
    insertkeymap( tt, 2103, mkkeyevt(F_KVERB | K_BA80_SOFTKEY7 )); /* Softkey7         Alt-7 */
    insertkeymap( tt, 2104, mkkeyevt(F_KVERB | K_BA80_SOFTKEY8 )); /* Softkey8         Alt-8 */
    insertkeymap( tt, 2105, mkkeyevt(F_KVERB | K_BA80_SOFTKEY9 )); /* Softkey9         Alt-9 */

    insertkeymap( tt, 1025, mkliteralevt("\033"));
    insertkeymap( tt, 1026, mkliteralevt("\034"));
    insertkeymap( tt, 1027, mkliteralevt("\035"));
    insertkeymap( tt, 1028, mkliteralevt("\036"));
    insertkeymap( tt, 1029, mkliteralevt("\037"));
    insertkeymap( tt, 1030, mkliteralevt("\177"));
    insertkeymap( tt, 1031, mkkeyevt(F_KVERB | K_NULL));

    insertkeymap( tt, 4388,   mkkeyevt(F_KVERB | K_BA80_HOME)) ; /* Gray-Home */
/*    insertkeymap( tt, 4390, mkkeyevt(F_KVERB | K_BA80_)) ; /* Gray-Up */
    insertkeymap( tt, 4385,   mkkeyevt(F_KVERB | K_BA80_PGUP)) ; /* Gray-PgUp */
/*    insertkeymap( tt, 4389, mkkeyevt(F_KVERB | K_BA80_)) ; /* Gray-Left */
/*    insertkeymap( tt, 4391, mkkeyevt(F_KVERB | K_BA80_)) ; /* Gray-Right */
    insertkeymap( tt, 4387, mkkeyevt(F_KVERB | K_BA80_END)) ; /* Gray-End  */
/*    insertkeymap( tt, 4392, mkkeyevt(F_KVERB | K_BA80_)) ; /* Gray-Down */
    insertkeymap( tt, 4386,   mkkeyevt(F_KVERB | K_BA80_PGDN)) ; /* Gray-PgDn */
    insertkeymap( tt, 4397,   mkkeyevt(F_KVERB | K_BA80_INS)) ; /* Gray-Ins */
    insertkeymap( tt, 4398,   mkkeyevt(F_KVERB | K_BA80_DEL)) ; /* Gray-Del */

    return 0;
}

int
defsnikm( int tt )
{
    defbasekm(tt);

    insertkeymap( tt, 368 , mkkeyevt(F_KVERB | K_SNI_F01 )); /* F1     F1     */
    insertkeymap( tt, 369 , mkkeyevt(F_KVERB | K_SNI_F02 )); /* F2     F2     */
    insertkeymap( tt, 370 , mkkeyevt(F_KVERB | K_SNI_F03 )); /* F3     F3     */
    insertkeymap( tt, 371 , mkkeyevt(F_KVERB | K_SNI_F04 )); /* F4     F4     */
    insertkeymap( tt, 372 , mkkeyevt(F_KVERB | K_SNI_F05 )); /* F5     F5     */
    insertkeymap( tt, 373 , mkkeyevt(F_KVERB | K_SNI_F06 )); /* F6     F6     */
    insertkeymap( tt, 374 , mkkeyevt(F_KVERB | K_SNI_F07 )); /* F7     F7     */
    insertkeymap( tt, 375 , mkkeyevt(F_KVERB | K_SNI_F08 )); /* F8     F8     */
    insertkeymap( tt, 376 , mkkeyevt(F_KVERB | K_SNI_F09 )); /* F9     F9     */
    insertkeymap( tt, 377 , mkkeyevt(F_KVERB | K_SNI_F10 )); /* F10    F10    */
    insertkeymap( tt, 378 , mkkeyevt(F_KVERB | K_SNI_F11 )); /* F11    F11    */
    insertkeymap( tt, 379 , mkkeyevt(F_KVERB | K_SNI_F12 )); /* F12    F12    */

    insertkeymap( tt, 880 , mkkeyevt(F_KVERB | K_SNI_S_F01  )) || /* F1   Shift-F1  */
    insertkeymap( tt, 881 , mkkeyevt(F_KVERB | K_SNI_S_F02  )) || /* F2   Shift-F2  */
    insertkeymap( tt, 882 , mkkeyevt(F_KVERB | K_SNI_S_F03  )) || /* F3   Shift-F3  */
    insertkeymap( tt, 883 , mkkeyevt(F_KVERB | K_SNI_S_F04  )) || /* F4   Shift-F4  */
    insertkeymap( tt, 884 , mkkeyevt(F_KVERB | K_SNI_S_F05  )) || /* F5   Shift-F5  */
    insertkeymap( tt, 885 , mkkeyevt(F_KVERB | K_SNI_S_F06  )) || /* F6   Shift-F6  */
    insertkeymap( tt, 886 , mkkeyevt(F_KVERB | K_SNI_S_F07  )) || /* F7   Shift-F7  */
    insertkeymap( tt, 887 , mkkeyevt(F_KVERB | K_SNI_S_F08  )) || /* F8   Shift-F8  */
    insertkeymap( tt, 888 , mkkeyevt(F_KVERB | K_SNI_S_F09  )) || /* F9   Shift-F9  */
    insertkeymap( tt, 889 , mkkeyevt(F_KVERB | K_SNI_S_F10 ))  || /* F10  Shift-F10 */
    insertkeymap( tt, 890 , mkkeyevt(F_KVERB | K_SNI_S_F11  )) || /* F11  Shift-F9  */
    insertkeymap( tt, 891 , mkkeyevt(F_KVERB | K_SNI_S_F12 ))  || /* F12  Shift-F10 */

    insertkeymap( tt, 1392, mkkeyevt(F_KVERB | K_SNI_F11 ));
    insertkeymap( tt, 1393, mkkeyevt(F_KVERB | K_SNI_F12 ));
    insertkeymap( tt, 1394, mkkeyevt(F_KVERB | K_SNI_F13 ));
    insertkeymap( tt, 1395, mkkeyevt(F_KVERB | K_SNI_F14 ));
    insertkeymap( tt, 1396, mkkeyevt(F_KVERB | K_SNI_F15 ));
    insertkeymap( tt, 1397, mkkeyevt(F_KVERB | K_SNI_F16 ));
    insertkeymap( tt, 1398, mkkeyevt(F_KVERB | K_SNI_F17 ));
    insertkeymap( tt, 1399, mkkeyevt(F_KVERB | K_SNI_F18 ));
    insertkeymap( tt, 1400, mkkeyevt(F_KVERB | K_SNI_F19 ));
    insertkeymap( tt, 1401, mkkeyevt(F_KVERB | K_SNI_F20 ));

    insertkeymap( tt, 1904 , mkkeyevt(F_KVERB | K_SNI_S_F11  )) || /* F1   Ctrl-Shift-F1  */
    insertkeymap( tt, 1905 , mkkeyevt(F_KVERB | K_SNI_S_F12  )) || /* F2   Ctrl-Shift-F2  */
    insertkeymap( tt, 1906 , mkkeyevt(F_KVERB | K_SNI_S_F13  )) || /* F3   Ctrl-Shift-F3  */
    insertkeymap( tt, 1907 , mkkeyevt(F_KVERB | K_SNI_S_F14  )) || /* F4   Ctrl-Shift-F4  */
    insertkeymap( tt, 1908 , mkkeyevt(F_KVERB | K_SNI_S_F15  )) || /* F5   Ctrl-Shift-F5  */
    insertkeymap( tt, 1909 , mkkeyevt(F_KVERB | K_SNI_S_F16  )) || /* F6   Ctrl-Shift-F6  */
    insertkeymap( tt, 1910 , mkkeyevt(F_KVERB | K_SNI_S_F17  )) || /* F7   Ctrl-Shift-F7  */
    insertkeymap( tt, 1911 , mkkeyevt(F_KVERB | K_SNI_S_F18  )) || /* F8   Ctrl-Shift-F8  */
    insertkeymap( tt, 1912 , mkkeyevt(F_KVERB | K_SNI_S_F19  )) || /* F9   Ctrl-Shift-F9  */
    insertkeymap( tt, 1913 , mkkeyevt(F_KVERB | K_SNI_S_F20 ))  || /* F10  Ctrl-Shift-F10 */
    insertkeymap( tt, 1914 , mkkeyevt(F_KVERB | K_SNI_S_F19  )) || /* F9   Ctrl-Shift-F11  */
    insertkeymap( tt, 1915 , mkkeyevt(F_KVERB | K_SNI_S_F20 ))  || /* F10  Ctrl-Shift-F12 */

    insertkeymap( tt, 2097 , mkkeyevt(F_KVERB | K_SNI_F21  )) ||
    insertkeymap( tt, 2098 , mkkeyevt(F_KVERB | K_SNI_F22 ))  ||
    insertkeymap( tt, 2081 , mkkeyevt(F_KVERB | K_SNI_S_F21  )) ||
    insertkeymap( tt, 2082 , mkkeyevt(F_KVERB | K_SNI_S_F22 ))  ||

    insertkeymap( tt, 2928, mkkeyevt(F_KVERB | K_SNI_C_F01  )) || /* F1   Shift-F1  */
    insertkeymap( tt, 2929, mkkeyevt(F_KVERB | K_SNI_C_F02  )) || /* F2   Shift-F2  */
    insertkeymap( tt, 2930, mkkeyevt(F_KVERB | K_SNI_C_F03  )) || /* F3   Shift-F3  */
    insertkeymap( tt, 2931, mkkeyevt(F_KVERB | K_SNI_C_F04  )) || /* F4   Shift-F4  */
    insertkeymap( tt, 2932, mkkeyevt(F_KVERB | K_SNI_C_F05  )) || /* F5   Shift-F5  */
    insertkeymap( tt, 2933, mkkeyevt(F_KVERB | K_SNI_C_F06  )) || /* F6   Shift-F6  */
    insertkeymap( tt, 2934, mkkeyevt(F_KVERB | K_SNI_C_F07  )) || /* F7   Shift-F7  */
    insertkeymap( tt, 2935, mkkeyevt(F_KVERB | K_SNI_C_F08  )) || /* F8   Shift-F8  */
    insertkeymap( tt, 2936, mkkeyevt(F_KVERB | K_SNI_C_F09  )) || /* F9   Shift-F9  */
    insertkeymap( tt, 2937, mkkeyevt(F_KVERB | K_SNI_C_F10 ))  || /* F10  Shift-F10 */
    insertkeymap( tt, 2938, mkkeyevt(F_KVERB | K_SNI_C_F11  )) || /* F11  Shift-F9  */
    insertkeymap( tt, 2939, mkkeyevt(F_KVERB | K_SNI_C_F12 ))  || /* F12  Shift-F10 */

    insertkeymap( tt, 3889 , mkkeyevt(F_KVERB | K_SNI_C_F21  )) ||
    insertkeymap( tt, 3890 , mkkeyevt(F_KVERB | K_SNI_C_F22 ))  ||

    insertkeymap( tt, 3952, mkkeyevt(F_KVERB | K_SNI_C_F11  )) ||
    insertkeymap( tt, 3953, mkkeyevt(F_KVERB | K_SNI_C_F12  )) ||
    insertkeymap( tt, 3954, mkkeyevt(F_KVERB | K_SNI_C_F13  )) ||
    insertkeymap( tt, 3955, mkkeyevt(F_KVERB | K_SNI_C_F14  )) ||
    insertkeymap( tt, 3956, mkkeyevt(F_KVERB | K_SNI_C_F15  )) ||
    insertkeymap( tt, 3957, mkkeyevt(F_KVERB | K_SNI_C_F16  )) ||
    insertkeymap( tt, 3958, mkkeyevt(F_KVERB | K_SNI_C_F17  )) ||
    insertkeymap( tt, 3959, mkkeyevt(F_KVERB | K_SNI_C_F18  )) ||
    insertkeymap( tt, 3960, mkkeyevt(F_KVERB | K_SNI_C_F19  )) ||
    insertkeymap( tt, 3961, mkkeyevt(F_KVERB | K_SNI_C_F20 ))  ||

    insertkeymap( tt, 2099, mkkeyevt(F_KVERB | K_SNI_USER1  )) ||
    insertkeymap( tt, 2100, mkkeyevt(F_KVERB | K_SNI_USER2  )) ||
    insertkeymap( tt, 2101, mkkeyevt(F_KVERB | K_SNI_USER3  )) ||
    insertkeymap( tt, 2102, mkkeyevt(F_KVERB | K_SNI_USER4  )) ||
    insertkeymap( tt, 2103, mkkeyevt(F_KVERB | K_SNI_USER5  )) ||

    insertkeymap( tt, 4388, mkkeyevt(F_KVERB | K_SNI_HOME)) ; /* Gray-Home */
    insertkeymap( tt, 4385, mkkeyevt(F_KVERB | K_SNI_SCROLL_UP)) ; /* Gray-PgUp */
    insertkeymap( tt, 4387, mkkeyevt(F_KVERB | K_SNI_END)) ; /* Gray-End  */
    insertkeymap( tt, 4386, mkkeyevt(F_KVERB | K_SNI_SCROLL_DOWN)) ; /* Gray-PgDn */
    insertkeymap( tt, 4397, mkkeyevt(F_KVERB | K_SNI_INSERT_CHAR)) ; /* Gray-Ins */
    insertkeymap( tt, 4398, mkliteralevt("\177"));

    insertkeymap( tt, 4900, mkkeyevt(F_KVERB | K_SNI_HOME)) ; /* Gray-Shift-Home */
    insertkeymap( tt, 4897, mkkeyevt(F_KVERB | K_SNI_SCROLL_UP)) ; /* Gray-Shift-PgUp */
    insertkeymap( tt, 4899, mkkeyevt(F_KVERB | K_SNI_END)) ; /* Gray-Shift-End  */
    insertkeymap( tt, 4898, mkkeyevt(F_KVERB | K_SNI_SCROLL_DOWN)) ; /* Gray-Shift-PgDn */
    insertkeymap( tt, 4909, mkkeyevt(F_KVERB | K_SNI_INSERT_CHAR)) ; /* Gray-Shift-Ins */
    insertkeymap( tt, 4910, mkkeyevt(F_KVERB | K_SNI_DELETE_CHAR)) ; /* Gray-Shift-Del */

    insertkeymap( tt,  264, mkkeyevt(BS));
    insertkeymap( tt,  265, mkkeyevt(F_KVERB | K_SNI_TAB));

    insertkeymap( tt, 6445, mkkeyevt(F_KVERB | K_SNI_INSERT_LINE));
    insertkeymap( tt, 6446, mkkeyevt(F_KVERB | K_SNI_DELETE_LINE));
    insertkeymap( tt, 5421, mkkeyevt(F_KVERB | K_SNI_INSERT_WORD));
    insertkeymap( tt, 5422, mkkeyevt(F_KVERB | K_SNI_DELETE_WORD));
    insertkeymap( tt, 6436, mkkeyevt(F_KVERB | K_SNI_CE));
    insertkeymap( tt, 6433, mkkeyevt(F_KVERB | K_SNI_CH_CODE));
    insertkeymap( tt, 6435, mkkeyevt(F_KVERB | K_SNI_ENDMARKE));

    insertkeymap( tt, 1043, mkkeyevt(F_KVERB | K_SNI_START));
    insertkeymap( tt, 2163, mkkeyevt(F_KVERB | K_SNI_START));
    insertkeymap( tt, 1040, mkkeyevt(F_KVERB | K_SNI_PRINT));
    insertkeymap( tt, 2160, mkkeyevt(F_KVERB | K_SNI_PRINT));
    insertkeymap( tt, 1032, mkkeyevt(F_KVERB | K_SNI_HELP));
    insertkeymap( tt, 2152, mkkeyevt(F_KVERB | K_SNI_HELP));
    insertkeymap( tt, 1028, mkkeyevt(F_KVERB | K_SNI_MODE));
    insertkeymap( tt, 2148, mkkeyevt(F_KVERB | K_SNI_MODE));
    return 0;
}

int
defi31km( int tt )
{
    defbasekm(tt);
    insertkeymap( tt, 368 , mkkeyevt(F_KVERB | K_I31_F01 )); /* F1   F1     */
    insertkeymap( tt, 369 , mkkeyevt(F_KVERB | K_I31_F02 )); /* F2   F2     */
    insertkeymap( tt, 370 , mkkeyevt(F_KVERB | K_I31_F03 )); /* F3   F3     */
    insertkeymap( tt, 371 , mkkeyevt(F_KVERB | K_I31_F04 )); /* F4   F4     */
    insertkeymap( tt, 372 , mkkeyevt(F_KVERB | K_I31_F05 )); /* F5   F5     */
    insertkeymap( tt, 373 , mkkeyevt(F_KVERB | K_I31_F06 )); /* F6   F6     */
    insertkeymap( tt, 374 , mkkeyevt(F_KVERB | K_I31_F07 )); /* F7   F7     */
    insertkeymap( tt, 375 , mkkeyevt(F_KVERB | K_I31_F08 )); /* F8   F8     */
    insertkeymap( tt, 376 , mkkeyevt(F_KVERB | K_I31_F09 )); /* F9   F9     */
    insertkeymap( tt, 377 , mkkeyevt(F_KVERB | K_I31_F10 )); /* F10  F10    */
    insertkeymap( tt, 378 , mkkeyevt(F_KVERB | K_I31_F11 )); /* F11  F11    */
    insertkeymap( tt, 379 , mkkeyevt(F_KVERB | K_I31_F12 )); /* F12  F12    */

    insertkeymap( tt, 2416, mkkeyevt(F_KVERB | K_I31_F13 )); /* F13   Alt-F1  */
    insertkeymap( tt, 2417, mkkeyevt(F_KVERB | K_I31_F14 )); /* F14   Alt-F2  */
    insertkeymap( tt, 2418, mkkeyevt(F_KVERB | K_I31_F15 )); /* F15   Alt-F3  */
    insertkeymap( tt, 2419, mkkeyevt(F_KVERB | K_I31_F16 )); /* F16   Alt-F4  */
    insertkeymap( tt, 2420, mkkeyevt(F_KVERB | K_I31_F17 )); /* F17   Alt-F5  */
    insertkeymap( tt, 2421, mkkeyevt(F_KVERB | K_I31_F18 )); /* F18   Alt-F6  */
    insertkeymap( tt, 2422, mkkeyevt(F_KVERB | K_I31_F19 )); /* F19   Alt-F7  */
    insertkeymap( tt, 2423, mkkeyevt(F_KVERB | K_I31_F20 )); /* F20   Alt-F8  */
    insertkeymap( tt, 2424, mkkeyevt(F_KVERB | K_I31_F21 )); /* F21   Alt-F9  */
    insertkeymap( tt, 2425, mkkeyevt(F_KVERB | K_I31_F22 )); /* F22   Alt-F10 */
    insertkeymap( tt, 2426, mkkeyevt(F_KVERB | K_I31_F23 )); /* F23   Alt-F11 */
    insertkeymap( tt, 2427, mkkeyevt(F_KVERB | K_I31_F24 )); /* F24   Alt-F12 */

    insertkeymap( tt, 2928, mkkeyevt(F_KVERB | K_I31_F25 )); /* F13   Alt-Shift-F1  */
    insertkeymap( tt, 2929, mkkeyevt(F_KVERB | K_I31_F26 )); /* F14   Alt-Shift-F2  */
    insertkeymap( tt, 2930, mkkeyevt(F_KVERB | K_I31_F27 )); /* F15   Alt-Shift-F3  */
    insertkeymap( tt, 2931, mkkeyevt(F_KVERB | K_I31_F28 )); /* F16   Alt-Shift-F4  */
    insertkeymap( tt, 2932, mkkeyevt(F_KVERB | K_I31_F29 )); /* F17   Alt-Shift-F5  */
    insertkeymap( tt, 2933, mkkeyevt(F_KVERB | K_I31_F30 )); /* F18   Alt-Shift-F6  */
    insertkeymap( tt, 2934, mkkeyevt(F_KVERB | K_I31_F31 )); /* F19   Alt-Shift-F7  */
    insertkeymap( tt, 2935, mkkeyevt(F_KVERB | K_I31_F32 )); /* F20   Alt-Shift-F8  */
    insertkeymap( tt, 2936, mkkeyevt(F_KVERB | K_I31_F33 )); /* F21   Alt-Shift-F9  */
    insertkeymap( tt, 2937, mkkeyevt(F_KVERB | K_I31_F34 )); /* F22   Alt-Shift-F10 */
    insertkeymap( tt, 2938, mkkeyevt(F_KVERB | K_I31_F35 )); /* F23   Alt-Shift-F11 */
    insertkeymap( tt, 2939, mkkeyevt(F_KVERB | K_I31_F36 )); /* F24   Alt-Shift-F12 */

    insertkeymap( tt, 2097, mkkeyevt(F_KVERB | K_I31_PA1 )); /* Softkey1         Alt-1 */
    insertkeymap( tt, 2098, mkkeyevt(F_KVERB | K_I31_PA2 )); /* Softkey2         Alt-2 */
    insertkeymap( tt, 2099, mkkeyevt(F_KVERB | K_I31_PA3 )); /* Softkey3         Alt-3 */

    insertkeymap( tt, 4388,   mkkeyevt(F_KVERB | K_I31_HOME)) ; /* Gray-Home */
/*    insertkeymap( tt, 4390, mkkeyevt(F_KVERB | K_I31_)) ; /* Gray-Up */
    insertkeymap( tt, 4385,   mkkeyevt(F_KVERB | K_I31_CLEAR)) ; /* Gray-PgUp */
/*    insertkeymap( tt, 4389, mkkeyevt(F_KVERB | K_I31_)) ; /* Gray-Left */
/*    insertkeymap( tt, 4391, mkkeyevt(F_KVERB | K_I31_)) ; /* Gray-Right */
    insertkeymap( tt, 4387, mkkeyevt(F_KVERB | K_I31_BACK_TAB)) ; /* Gray-End  */
/*    insertkeymap( tt, 4392, mkkeyevt(F_KVERB | K_I31_)) ; /* Gray-Down */
    insertkeymap( tt, 4386,   mkkeyevt(F_KVERB | K_I31_ERASE_EOF)) ; /* Gray-PgDn */
    insertkeymap( tt, 4397,   mkkeyevt(F_KVERB | K_I31_INSERT_CHAR)) ; /* Gray-Ins */
    insertkeymap( tt, 4398,   mkkeyevt(F_KVERB | K_I31_DELETE)) ; /* Gray-Del */

    return 0;
}

int
insertkmtolist( struct keynode ** plist, int key, con_event def )
{
    if ( *plist == NULL ) {
        *plist = (struct keynode *) malloc( sizeof(struct keynode) ) ;
        if ( *plist == NULL )
            return -1;
        (*plist)->key = key ;
        (*plist)->def = def ;
        (*plist)->next = NULL ;
        return 0;
    }
    else if ( (*plist)->key == key ) {
        (*plist)->def = def ;
        return 0;
    }
    else if ( (*plist)->key > key ) {
        struct keynode * pnode = (struct keynode *) malloc( sizeof(struct keynode) ) ;
		if (pnode) {
			pnode->key = key ;
			pnode->def = def ;
			pnode->next = *plist ;
			*plist = pnode ;
			return 0;
		}
		return -1;
    }
    else
        return insertkmtolist( &((*plist)->next), key, def );
}

int
insertkeymap( int terminal, int key, con_event def )
{
    return insertkmtolist( &(ttkeymap[terminal]), key, def );
}

int
deletekeymap( int terminal, int key )
{
    struct keynode ** plist = &(ttkeymap[terminal]) ;
    while ( *plist ) {
        if ( (*plist)->key == key ) {
            struct keynode * pnode = *plist ;
            *plist = (*plist)->next ;
            if ( pnode->def.type == macro ) {
                free( pnode->def.macro.string );
            }
            free(pnode) ;
            return 0;
        }

        plist = &((*plist)->next) ;
    }
    return -1;
}

int
clearkeymap( int terminal )
{
    struct keynode ** plist = &(ttkeymap[terminal]) ;
    while ( *plist ) {
        struct keynode * pnode = *plist ;
        *plist = (*plist)->next ;
        free(pnode) ;
    }
    return 0;
}

int
findkeymap( int terminal, int key, con_event * def )
{
    struct keynode * list = ttkeymap[terminal] ;
    if ( !def ) {
        return -2;
    }

    while ( list ) {
        if ( list->key == key ) {
            *def= list->def ;
            return 0;
        }
        else if ( list->key < key )
            list = list->next ;
        else
            break;
    }
    def->type = error ;
    return -1;
}

int
defaultkeymap( int terminal ) {
    if ( clearkeymap( terminal ) )
        return -1;

    switch ( terminal ) {
    case WPKM:
        return defwpkm(terminal) ;
    case EMACSKM:
        return defemacskm(terminal) ;
    case HEBREWKM:
        return defhebrewkm(terminal);
    case RUSSIANKM:
        return defrussiankm(terminal);
    case TT_VC4404:
        return defvckm(terminal);
    case TT_HPTERM:
    case TT_HP2621:
        return defhpkm(terminal);
    case TT_HZL1500:
        return defhzlkm(terminal);
    case TT_DG200:
    case TT_DG210:
    case TT_DG217:
        return defdgkm( terminal ) ;
    case TT_WY30:
    case TT_WY50:
    case TT_WY60:
    case TT_WY160:
        return defwyseasciikm( terminal ) ;
    case TT_BA80:
        return defba80km( terminal );
    case TT_HFT:
    case TT_AIXTERM:
        return defaixkm( terminal );
    case TT_SUN:
        return defsunkm( terminal );
    case TT_VT52:
    case TT_H19:
    case TT_ANSI:
    case TT_VT100:
    case TT_VT102:
    case TT_VIP7809:
        return defvt100km(terminal);
    case TT_BEOS:
        return defbetermkm( terminal );
    case TT_LINUX:
        return deflinuxkm( terminal );
    case TT_SCOANSI:
        return defscoansikm( terminal );
    case TT_AT386:
        return defat386km( terminal );
    case TT_QANSI:
        return defqansikm( terminal );
    case TT_AAA:
        return defannarbor( terminal );
    case TT_VT220:
    case TT_VT320:
    case TT_WY370:
        return defvt200km( terminal );
    case TT_VT220PC:
    case TT_VT320PC:
        return defvtpckm( terminal );
    case TT_97801:
        return defsnikm( terminal );
    case TT_TVI910:
    case TT_TVI925:
    case TT_TVI950:
        return deftviasciikm(terminal);
    case TT_QNX:        /* haven't defined this yet */
        return defqnxkm(terminal);
    case TT_IBM31:
        return defi31km(terminal);
    case TT_NONE:
    default:
        return defbasekm(terminal);
    }
    return 0;
}

con_event
mapkey( unsigned int c )
{
    extern MACRO * macrotab ;
    extern BYTE vmode ;
    int rc = -1;
    con_event event ;

    event.type = error;

    if ( vmode == VTERM && tt_kb_mode != KBM_EN ) {
        switch ( tt_kb_mode ) {
        case KBM_EM:
            rc = findkeymap(EMACSKM,c, &event);
            break;
        case KBM_HE:
            rc = findkeymap(HEBREWKM,c,&event);
            break;
        case KBM_RU:
            rc = findkeymap(RUSSIANKM,c,&event);
            break;
        case KBM_WP:
            rc = findkeymap(WPKM,c,&event);
            break;
        }
    }
    if ( rc == 0 )
        return(event);
    else if ( keymap && c < KMSIZE && keymap[c] != c ) {
        int km = keymap[c] ;
        if ( IS_KVERB(km) ) {
            event.type = kverb ;
            event.kverb.id = km & ~F_KVERB ;
        }
        else if ( IS_CSI(km) ) {
            event.type = csi ;
            event.csi.key = km & ~F_CSI ;
        }
        else if ( IS_ESC(km) ) {
            event.type = esc ;
            event.esc.key = km & ~F_ESC ;
        }
        else {
            event.type = key ;
            event.key.scancode = km;
        }
        rc = 0 ;        /* success */
    }
    else if ( macrotab && c < KMSIZE && macrotab[c] ) {
        event.type = macro ;
        event.macro.string = macrotab[c];
        rc = 0;         /* success */
    }
    else {
        rc = findkeymap( tt_type, c, &event ) ;
    }
    if ( rc < 0 )
        event.type = error ;

    return ( event );
}

con_event
maptermkey( int c, int emulation )
{
    extern MACRO * macrotab ;
    extern BYTE vmode ;
    int rc = -1;
    con_event event ;

    rc = findkeymap( emulation, c, &event ) ;
    if ( rc < 0 )
        event.type = error ;

    return ( event );
}
#endif /* NOSETKEY */

#ifndef NOKVERBS
void
vikinit( void )
{
    /* Initialize Very Important Keys structure */
    int i,button,event;

    if ( !initvik )
        return ;

    vik.help = 0;
    vik.exit = 0;
    vik.quit = 0;
    vik.upscn = 0;
    vik.dnscn = 0;
    vik.upone = 0;
    vik.dnone = 0;
    vik.homscn = 0;
    vik.endscn = 0;
    vik.lfone = 0;
    vik.lfpg  = 0;
    vik.lfall = 0;
    vik.rtone = 0;
    vik.rtpg  = 0;
    vik.rtall = 0;
    vik.hangup = 0;
    vik.xbreak = 0;
    vik.lbreak = 0;
    vik.dump = 0;
    vik.prtctrl = 0;
    vik.prtauto = 0;
    vik.os2 = 0;
    vik.printff = 0;
    vik.flipscn = 0;
    vik.debug = 0;
    vik.reset = 0;
    vik.compose = 0;
    vik.ucs2 = 0;
    vik.markstart = 0 ;
    vik.markcancel = 0 ;
    vik.copyclip = 0 ;
    vik.copyhost = 0 ;
    vik.paste = 0 ;
    vik.holdscrn = 0 ;
#ifdef OS2MOUSE
    vik.mou_curpos   = 0;
    vik.mou_copyhost = 0 ;
    vik.mou_copyclip = 0 ;
    vik.mou_paste    = 0 ;
    vik.mou_dump     = 0 ;
    vik.mou_mark     = 0 ;
#endif /* OS2MOUSE */
    vik.backsrch = 0;
    vik.backnext = 0;
    vik.fwdsrch  = 0;
    vik.fwdnext  = 0;
    vik.setbook  = 0;
    vik.gobook   = 0;
    vik.xgoto    = 0;
    vik.fnkeys   = 0;
    vik.status   = 0;
    vik.ttype    = 0;

    for (i = 256; i < KMSIZE; i++) {
        int km = 0;
        con_event evt = mapkey(i) ;
        if ( evt.type != kverb )
            continue;
        km = evt.kverb.id & ~F_KVERB ;
        if (!vik.help && (km == K_HELP))
          vik.help = i;
        else if (!vik.exit && (km == K_EXIT))
          vik.exit = i;
        else if (!vik.quit && (km == K_QUIT))
          vik.quit = i;
        else if (!vik.upscn && km == K_UPSCN)
          vik.upscn = i;
        else if (!vik.dnscn && km == K_DNSCN)
          vik.dnscn = i;
        else if (!vik.upone && km == K_UPONE)
          vik.upone = i;
        else if (!vik.dnone && km == K_DNONE)
          vik.dnone = i;
        else if (!vik.homscn && km == K_HOMSCN)
          vik.homscn = i;
        else if (!vik.endscn && km == K_ENDSCN)
          vik.endscn = i;
        else if (!vik.lfone && km == K_LFONE)
          vik.lfone = i;
        else if (!vik.lfpg && km == K_LFPAGE)
          vik.lfpg = i;
        else if (!vik.lfall && km == K_LFALL)
          vik.lfall = i;
        else if (!vik.rtone && km == K_RTONE)
          vik.rtone = i;
        else if (!vik.rtpg && km == K_RTPAGE)
          vik.rtpg = i;
        else if (!vik.rtall && km == K_RTALL)
          vik.rtall = i;
        else if (!vik.hangup && (km == K_HANGUP))
          vik.hangup = i;
        else if (!vik.xbreak && (km == K_BREAK))
          vik.xbreak = i;
        else if (!vik.lbreak && (km == K_LBREAK))
          vik.lbreak = i;
        else if (!vik.dump && (km == K_DUMP))
          vik.dump = i;
        else if (!vik.flipscn && (km == K_FLIPSCN))
          vik.flipscn = i;
        else if (!vik.debug && (km == K_DEBUG))
          vik.debug = i;
/*
  By default, Alt-= and Alt-r both get \Kreset, but we want Alt-r to show
  on the help screen...

  Too bad, it makes the screen look weird.
*/
        else if (!vik.reset && (km == K_RESET))
          vik.reset = i;
        else if (!vik.os2 && (km == K_DOS))
          vik.os2 = i;
        else if (!vik.prtctrl && (km == K_PRTCTRL))
          vik.prtctrl = i;
        else if (!vik.prtauto && (km == K_PRTAUTO))
          vik.prtauto = i;
        else if (!vik.printff && (km == K_PRINTFF))
          vik.printff = i;
        else if (!vik.compose && (km == K_COMPOSE))
          vik.compose = i;
        else if (!vik.ucs2 && (km == K_C_UNI16))
          vik.ucs2 = i;
        else if (!vik.markcancel && (km == K_MARK_CANCEL))
          vik.markcancel = i ;
        else if (!vik.markstart && (km == K_MARK_START))
          vik.markstart = i ;
        else if (!vik.copyclip && (km == K_MARK_COPYCLIP))
          vik.copyclip = i ;
        else if (!vik.copyhost && (km == K_MARK_COPYHOST))
          vik.copyhost = i ;
        else if (!vik.paste && (km == K_PASTE))
          vik.paste = i ;
        else if (!vik.holdscrn && (km == K_HOLDSCRN))
          vik.holdscrn = i ;
        else if (!vik.backsrch && (km == K_BACKSRCH))
          vik.backsrch = i ;
        else if (!vik.backnext && (km == K_BACKNEXT))
          vik.backnext = i ;
        else if (!vik.fwdsrch && (km == K_FWDSRCH))
          vik.fwdsrch = i ;
        else if (!vik.fwdnext && (km == K_FWDNEXT))
          vik.fwdnext = i ;
        else if (!vik.setbook && (km == K_SET_BOOK))
          vik.setbook = i ;
        else if (!vik.gobook && (km == K_GO_BOOK))
          vik.gobook = i ;
        else if (!vik.xgoto && (km == K_GOTO))
          vik.xgoto = i ;
        else if (!vik.fnkeys && (km == K_FNKEYS))
          vik.fnkeys = i ;
        else if (!vik.status && (km == K_STATUS))
          vik.status = i ;
        else if (!vik.ttype && (km == K_TERMTYPE))
          vik.ttype = i ;
    }
#ifdef OS2MOUSE
    for ( button = 0 ; button < MMBUTTONMAX ; button++ )
      for ( event = 0 ; event < MMEVENTSIZE ; event++ )
        if ( mousemap[button][event].type == kverb ) {
            switch ( mousemap[button][event].kverb.id ) {
              case F_KVERB | K_MOUSE_CURPOS :
                vik.mou_curpos = (button << 5) | event ;
                break;
              case F_KVERB | K_MARK_COPYCLIP :
                vik.mou_copyclip = (button << 5) | event ;
                break;
              case F_KVERB | K_MARK_COPYHOST :
                vik.mou_copyhost = (button << 5) | event ;
                break;
              case F_KVERB | K_PASTE :
                vik.mou_paste = (button << 5) | event ;
                break;
              case F_KVERB | K_DUMP :
                vik.mou_dump = (button << 5) | event ;
                break;
              case F_KVERB | K_MOUSE_MARK :
                vik.mou_mark = (button << 5) | event ;
            case F_KVERB | K_MOUSE_URL :
                vik.mou_url = (button << 5) | event ;
                break;
            }
        }
#endif /* OS2MOUSE */
    initvik = FALSE ;
}
#endif /* NOKVERBS */

void
SetCapsLock( BOOL bState )
{
#ifdef NT
    /* doesn't work for Windows 95 */
    BYTE keyState[256];

    GetKeyboardState((LPBYTE)&keyState);
    if( (bState && !(keyState[VK_CAPITAL] & 1)) ||
        (!bState && (keyState[VK_CAPITAL] & 1)) )
    {
        // Simulate a key press
        keybd_event( VK_CAPITAL,
                     0x45,
                     KEYEVENTF_EXTENDEDKEY | 0,
                     0 );

        // Simulate a key release
        keybd_event( VK_CAPITAL,
                     0x45,
                     KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
                     0);
   }
#endif
}

void
SetNumLock( BOOL bState )
{
#ifdef NT
    /* doesn't work for Windows 95 */
    BYTE keyState[256];

    GetKeyboardState((LPBYTE)&keyState);
    if( (bState && !(keyState[VK_NUMLOCK] & 1)) ||
        (!bState && (keyState[VK_NUMLOCK] & 1)) )
    {
        // Simulate a key press
        keybd_event( VK_NUMLOCK,
                     0x45,
                     KEYEVENTF_EXTENDEDKEY | 0,
                     0 );

        // Simulate a key release
        keybd_event( VK_NUMLOCK,
                     0x45,
                     KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
                     0);
   }
#endif
}

void
SetScrollLock( BOOL bState )
{
#ifdef NT
   BYTE keyState[256];

   GetKeyboardState((LPBYTE)&keyState);
   if( (bState && !(keyState[VK_SCROLL] & 1)) ||
       (!bState && (keyState[VK_SCROLL] & 1)) )
   {
   // Simulate a key press
      keybd_event( VK_SCROLL,
                   0x45,
                   KEYEVENTF_EXTENDEDKEY | 0,
                   0 );

   // Simulate a key release
      keybd_event( VK_SCROLL,
                   0x45,
                   KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
                   0);
   }
#endif /* NT */
}

#ifdef printf
#undef printf
#endif /* printf */

#ifndef NOLOCAL
int
OS2WaitForKey( void ) {
    printf("\n<Press a Key to continue>");
    congev( IsConnectMode() ? VTERM : VCMD, -1 );
    return(0);
}
#endif /* NOLOCAL */
