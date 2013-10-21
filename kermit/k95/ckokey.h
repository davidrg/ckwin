/* C K O K E Y . H */

/*
  Authors: Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
             Columbia University Academic Information Systems, New York City.
           Jeffrey Altman (jaltman@secure-endpoints.com),
             Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

/* This file requires that ckuusr.h be included before it */

#ifndef CKOKEY_H
#define CKOKEY_H

#ifdef NT
#define CONTROL        (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)
#define SCROLLLOCK     SCROLLLOCK_ON
#define ALT            (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)
#define NUMLOCK        NUMLOCK_ON
#define LEFTSHIFT      SHIFT_PRESSED
#define RIGHTSHIFT     SHIFT_PRESSED
#define SHIFT          SHIFT_PRESSED
#define SHIFT_KEY_IN   SHIFT_PRESSED
#else /* not NT */
#ifndef SHIFT_KEY_IN
#define SHIFT_KEY_IN    KBDTRF_SHIFT_KEY_IN
#endif /* SHIFT_KEY_IN */

#ifndef CONTROL
#define CONTROL         KBDSTF_CONTROL
#define SCROLLLOCK      KBDSTF_SCROLLLOCK
#define NUMLOCK         KBDSTF_NUMLOCK
#define LEFTSHIFT       KBDSTF_LEFTSHIFT
#define RIGHTSHIFT      KBDSTF_RIGHTSHIFT
#define ALT             KBDSTF_ALT
#endif /* CONTROL */

#ifndef SCROLLLOCK_ON
#define SCROLLLOCK_ON   KBDSTF_SCROLLLOCK_ON
#endif
#ifndef NUMLOCK_ON
#define NUMLOCK_ON      KBDSTF_NUMLOCK_ON
#endif

#define SHIFT           (LEFTSHIFT | RIGHTSHIFT)
#define ENHANCED_KEY    0x02
#endif /* NT */

#define KBM_EN 0 /* English */
#define KBM_HE 1 /* Hebrew */
#define KBM_RU 2 /* Russian */
#define KBM_BR 3 /* Belorussian (not used) */
#define KBM_UR 4 /* Ukrainian (not used) */
#define KBM_EM 5 /* Emacs */
#define KBM_WP 6 /* Word Perfect 5.1 */

#ifndef NOKVERBS
#include "ckokvb.h"
#include "ckucmd.h"                     /* For struct keytab definition. */

typedef struct _vik_rec {               /* Very Important Keys - */
    KEY help;                           /* We look these up once */
    KEY exit;                           /* and remember where they are... */
    KEY quit;
    KEY upscn;
    KEY dnscn;
    KEY upone;
    KEY dnone;
    KEY homscn;
    KEY endscn;
    KEY lfone;
    KEY lfpg ;
    KEY lfall;
    KEY rtone;
    KEY rtpg ;
    KEY rtall;
    KEY hangup;
    KEY xbreak;
    KEY lbreak;
    KEY dump;
    KEY prtctrl;
    KEY prtauto;
    KEY os2;
    KEY printff;
    KEY flipscn;
    KEY debug;
    KEY reset;
    KEY compose;
    KEY ucs2;
    KEY markstart;
    KEY markcancel;
    KEY copyclip;
    KEY copyhost;
    KEY paste;
    KEY holdscrn;
#ifdef OS2MOUSE
    KEY mou_curpos;
    KEY mou_copyhost;
    KEY mou_copyclip;
    KEY mou_paste;
    KEY mou_dump;
    KEY mou_mark;
    KEY mou_url;
#endif /* OS2MOUSE */
    KEY backsrch;
    KEY fwdsrch;
    KEY backnext;
    KEY fwdnext;
    KEY setbook;
    KEY gobook;
    KEY xgoto;
    KEY fnkeys;
    KEY status;
    KEY ttype;
} vik_rec;
#endif /* NOKVERBS */

enum con_evt_type {
    key=1,
#ifdef OS2MOUSE
    mouse=2,
#endif /* OS2MOUSE */
#ifndef NOKVERBS
    kverb=3,
#endif /* NOKVERBS */
    macro=4,
    esc=5,
    csi=6,
    literal=7,
    error=8,
    noop=99
};

typedef struct _esc_event {
    KEY key;
} esc_event ;

typedef struct _csi_event {
    KEY key ;
} csi_event ;

typedef struct _key_event {
    KEY scancode ;
} key_event ;

#ifdef OS2MOUSE
typedef struct _ck_mouse_event {
    unsigned button : 2 ;
    unsigned alt    : 1 ;
    unsigned ctrl   : 1 ;
    unsigned shift  : 1 ;
    unsigned dbl    : 1 ;
    unsigned drag   : 1 ;
    position pos ;
} ck_mouse_event ;
#endif /* OS2MOUSE */
#ifndef NOKVERBS
typedef struct _kverb_event {
    USHORT id ;
} kverb_event;
#endif /* NOKVERBS */

typedef struct _macro_event {
    CHAR * string ;
} macro_event ;

typedef struct _literal_event {
    CHAR * string ;
} literal_event ;

typedef struct _con_event {
    enum con_evt_type type ;
    union {
        key_event   key ;
#ifdef OS2MOUSE
        ck_mouse_event mouse ;
#endif /* OS2MOUSE */
#ifndef NOKVERBS
        kverb_event kverb ;
#endif /* NOKVERBS */
        macro_event macro ;
        esc_event esc ;
        csi_event csi ;
        literal_event literal ;
        } ;
} con_event ;

struct keynode {
    int              key ;
    con_event        def ;
    struct keynode * next ;
};

#ifdef OS2MOUSE
#define MMB1      XYM_B1
#define MMB2      XYM_B2
#define MMB3      XYM_B3
#define MMBUTTONMAX  3

#define MMALT     XYM_ALT
#define MMCTRL    XYM_CTRL
#define MMSHIFT   XYM_SHIFT
#define MMCLICK   XYM_C1
#define MMDBL     XYM_C2
#define MMDRAG    XYM_DRAG
#define MMEVENTSIZE (MMDRAG * 2)
#define MMSIZE (MMBUTTONMAX * MMEVENTSIZE)               /* Mouse Map Size */

extern con_event mousemap[MMBUTTONMAX][MMSIZE] ;
#endif /* OS2MOUSE */

#define KEY_SCAN      0x0100
#define KEY_SHIFT     0x0200
#define KEY_CTRL      0x0400
#define KEY_ALT       0x0800
#define KEY_ENHANCED  0x1000

#define EMACSKM    (TT_MAX+1)
#define HEBREWKM   (TT_MAX+2)
#define RUSSIANKM  (TT_MAX+3)
#define WPKM       (TT_MAX+4)

_PROTOTYP( void keymapinit, (void) );
_PROTOTYP( void keynaminit, (void) );
_PROTOTYP( void vikinit, ( void ) ) ;
_PROTOTYP( char * keyname, (unsigned long) );
_PROTOTYP( void mousemapinit, (int,int) );
_PROTOTYP( char * mousename, (int,int) ) ;
_PROTOTYP( int keyinbuf, (int) ) ;
_PROTOTYP( int evtinbuf, (int) ) ;
_PROTOTYP( int putkey, (int,int) ) ;
_PROTOTYP( int putesc, (int,int) ) ;
_PROTOTYP( int putcsi, (int,int) ) ;
_PROTOTYP( int putkeystr, (int,char *) ) ;
_PROTOTYP( int putkverb, (int,int) ) ;
_PROTOTYP( int putmacro, (int,char *) ) ;
_PROTOTYP( int putliteral, (int,char *) ) ;
_PROTOTYP( int puterror, (int) ) ;
_PROTOTYP( int putevent, (int,con_event) ) ;
_PROTOTYP( int getevent, (int,con_event *) ) ;
_PROTOTYP( con_event congev, (int,int) ) ;
_PROTOTYP( void keybufinit, (void) ) ;
_PROTOTYP( void keybufcleanup, (void) ) ;
_PROTOTYP( int KbdHandlerInit, (void) ) ;
_PROTOTYP( int KbdHandlerCleanup, ( void ) ) ;
_PROTOTYP( void KbdHandlerThread, ( void * ) ) ;

_PROTOTYP( int insertkeymap, ( int, int, con_event ) ) ;
_PROTOTYP( int deletekeymap, ( int, int ) ) ;
_PROTOTYP( int clearkeymap, ( int ) ) ;
_PROTOTYP( int findkeymap, ( int, int, con_event * ) ) ;
_PROTOTYP( int defaultkeymap, ( int ) ) ;
_PROTOTYP( con_event mapkey, ( unsigned int ) ) ;
_PROTOTYP( con_event maptermkey, ( int, int ) ) ;
#ifdef NT
_PROTOTYP( void win32KeyEvent, (int, KEY_EVENT_RECORD) );
_PROTOTYP( int  getKeycodeFromKeyRec, (KEY_EVENT_RECORD *, WORD *, int));
#endif
#endif /* CKOKEY_H */
