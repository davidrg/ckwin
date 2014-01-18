/*  C K O W Y S . C  --  Wyse Emulation */

/*
  Author: Jeffrey Altman <jaltman@secure-endpoints.com>,
          Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

/*
   the WYSE 160 manual describes additional keys that we don't support yet
*/
/*    Prg               KeyName            */

/*      #    ESC W      Delete             */
/*      (    ESC R      Shift Delete       */
/*      X    ESC P      Shift Print Screen */
/*      :    ESC J      PgUp               */
/*      ;               Shift PgUp         */
/*      \    ESC T      End                */
/*      ]    ESC Y      Shift End          */

#ifdef NT
#include <windows.h>
#else /* NT */
#include <os2.h>
#undef COMMENT
#endif /* NT */

#include "ckcdeb.h"
#ifndef NOTERM
#include "ckcker.h"
#include "ckcasc.h"
#include "ckuusr.h"
#include "ckcuni.h"
#include "ckocon.h"
#include "ckokey.h"
#include "ckowys.h"
#include "ckctel.h"

extern bool keyclick ;
extern int  cursorena[], keylock, duplex, duplex_sav, screenon ;
extern int  printon, aprint, cprint, uprint, xprint, seslog ;
extern int  insertmode, tnlm, ttmdm, decssdt, cmask;
extern int  escstate, debses, decscnm, tt_cursor ;
#ifdef PCTERM
extern int tt_pcterm;
#endif /* PCTERM */
extern int  tt_type, tt_type_mode, tt_max, tt_answer, tt_status[VNUM], tt_szchng[] ;
extern int  tt_modechg ;
extern int  tt_cols[], tt_rows[], tt_wrap ;
extern int  wherex[], wherey[], margintop, marginbot ;
extern int  marginbell, marginbellcol ;
extern char answerback[], htab[] ;
extern struct tt_info_rec tt_info[] ;
extern vtattrib attrib ;
extern unsigned char attribute, colorstatus;
extern char * udkfkeys[];
extern int tt_senddata ;
extern int tt_hidattr;
extern int tt_sac;
extern bool xprintff; /* Print formfeed */
extern int tcsl;

extern struct _vtG G[];
extern struct _vtG *GL, *GR;

#ifdef TCPSOCKET
#ifdef CK_NAWS
extern int tn_snaws();
#endif /* CK_NAWS */
#endif /* TCPSOCKET */

int protect = FALSE ;
int autoscroll = TRUE ;
int writeprotect = FALSE ;
int linelock = FALSE ;
int wysegraphics = FALSE ;
int wyse8bit = FALSE;
int attrmode = ATTR_CHAR_MODE ;
int wy_keymode = FALSE ;
int wy_enhanced = TRUE ;
int wy_widthclr = FALSE ;
int wy_autopage = FALSE ;
int wy_nullsuppress = TRUE;
int wy_monitor  = FALSE ;
int wy_blockend = EOB_US_CR ;
int wy_block = FALSE;                           /* Wyse/TVI Block Mode */

static char wy_labelmsg[MAXTERMCOL+1] ;
static char wy_labelline[MAXTERMCOL+1] ;
static char wy_shiftlabelline[MAXTERMCOL+1] ;

extern char fkeylabel[16][32];

vtattrib WPattrib   ={0,0,0,0,0,1,0,1,0,0,0} ;/* Dim, and Protected */
vtattrib defWPattrib={0,0,0,0,0,1,0,1,0,0,0} ;/* Dim, and Protected */

int
wyinc(void)
{
    extern int pmask, cmask;
    extern int tt_utf8;
    int ch;

  loop:
    ch = ttinc(0);
    if ( ch < 0 )
        return ch;

    if ( seslog )
        logchar(ch);

    /* Handle the UTF8 conversion if we are in that mode */
    if ( tt_utf8 ) {
        USHORT * ucs2 = NULL;
        int rc = utf8_to_ucs2( (CHAR)(ch & 0xFF), &ucs2 );
        if ( rc > 0 )
            goto loop;
        else if ( rc < 0 )
            ch = 0xfffd;
        else
            ch = *ucs2;
    }

    if ( !xprint ) {
#ifndef NOXFER
        autodown(ch);
#endif /* NOXFER */
        autoexitchk(ch);
    }
    ch = ch & pmask & cmask;
    debugses(ch);
    if (printon && (is_xprint() || is_uprint()))
        prtchar(ch);
    return ch;
}

USHORT
xlwygrph( CHAR c ) {
    USHORT cx = 0 ;
    extern struct _vtG *GNOW;
    extern int win95lucida, win95hsl;

    if ( isunicode() ) {
        cx = (*xl_u[TX_WYSE60G_N])(c) ;
        if ( win95hsl && cx >= 0x23BA && cx <= 0x23BD )
            cx = tx_hslsub(cx);
        else if ( cx >= 0xE000 && cx <= 0xF8FF )
            cx = tx_usub(cx);
        if (win95lucida && cx > 0x017f)
            cx = tx_lucidasub(cx);
    }
    else
    {
        cx = (*xl_u[TX_WYSE60G_N])(c) ;
        if ( GNOW->itol )
            cx = (*GNOW->itol)(cx);
    }
    return(cx);
}

void
wyse_tab( VOID )
{
    int i;

    if ( !protect ) {               /* Typewriter tabs */
        i = wherex[VTERM];
        if (i < VscrnGetWidth(VTERM))
        {
            do {
                i++;
                cursorright(0);
            } while ((htab[i] != 'T') &&
                      (i <= VscrnGetWidth(VTERM)-1));
        }
    } else {                        /* Field tabs */
        int w = VscrnGetWidth(VTERM);
        int h = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
        int x = wherex[VTERM]-1;
        int y = wherey[VTERM]-1;
        int field_found = 0;
        vtattrib vta = {0,0,0,0,0,1,0,0,0,0,0}; /* Protected */

        for ( ; y < h ; y++,x=0 ) {
            for (  ; x < w ; x++ ) {
                vta = VscrnGetVtCharAttr( VTERM, x, y ) ;
                if ( !field_found ) {
                    if (vta.unerasable)
                        field_found = 1;
                } else {
                    if ( !vta.unerasable )
                        goto nextstep;
                }
            }
        }

      nextstep:
        if ( y == h ) {
            y = 0;
            x = 0;
            for ( ; y <= wherey[VTERM]-1 ; y++, x=0 ) {
                for ( ; y == wherey[VTERM]-1 ? x < wherex[VTERM]-1 : x < w ;
                      x++ ) {
                    vta = VscrnGetVtCharAttr( VTERM, x, y ) ;
                    if ( !field_found ) {
                        if (vta.unerasable)
                            field_found = 1;
                    } else {
                        if ( !vta.unerasable )
                            goto nextstep2;
                    }
                }
            }
        }

nextstep2:
        if ( field_found )
            lgotoxy(VTERM,x+1,y+1);
    }
    VscrnIsDirty(VTERM);

}

void
wyse_backtab( VOID )
{
    int i;

    if ( !protect ) {       /* Typewriter tabs */
        i = wherex[VTERM];
        if (i > 1) {
            do {
                i--;
                cursorleft(0);
            } while ((htab[i] != 'T') &&
                      (i >= 2));

            /*
            if the current cell is a protected cell, we must move
            the current cursor to the first unprotected position.
            */

            VscrnIsDirty(VTERM);
        }
    } else {    /* Field tabs */
        /* This code, because it uses Vscrn...() functions, uses 0-based
        * offsets instead of 1-based offsets as in where[xy] vars.
        */

        int w = VscrnGetWidth(VTERM);
        int h = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
        int x = wherex[VTERM]-1;
        int y = wherey[VTERM]-1;
        int field_found = 0;
        vtattrib vta = {0,0,0,0,0,1,0,0,0,0,0}; /* Protected */

        /* First, we must find the beginning of the current field that we are on. */
        /* Then we must find the end of the previous field. */
        /* And finally we must find the beginning of that field and position the cursor. */

        for ( ; y >= 0 ; y--,x=w-1 ) {
            for (  ; x >= 0 ; x-- ) {
                vta = VscrnGetVtCharAttr( VTERM, x, y );
                if ( field_found == 1 ) {
                    if (!vta.unerasable)
                        field_found = 2;
                } else {
                    if (vta.unerasable) {
                        if ( field_found == 2 ) {
                            field_found = 3;
                            goto nextstep1;
                        }
                        else
                            field_found = 1;
                    }
                }
            }
        }

      nextstep1:
        if ( y == -1 ) {
            y = h-1;
            x = w-1;
            for ( ; y >= wherey[VTERM]-1 ; y--, x=w-1 ) {
                for ( ; y == wherey[VTERM]-1 ? (x > wherex[VTERM]-1) : (x >= 0);
                      x-- ) {
                    vta = VscrnGetVtCharAttr( VTERM, x, y ) ;
                    if ( field_found == 1 ) {
                        if (!vta.unerasable)
                            field_found = 2;
                    } else {
                        if (vta.unerasable) {
                            if ( field_found == 2 ) {
                                field_found = 3;
                                goto nextstep2;
                            } else
                                field_found = 1;
                        }
                    }
                }
            }
        }

      nextstep2:
        if ( field_found == 3 ) {
            x++;
            if ( x == w ) {
                x = 0;
                y++;
                if ( y == h )
                    y = 0;
            }
            lgotoxy(VTERM,x+1,y+1);
            VscrnIsDirty(VTERM);
        }
    }
}

void
wysectrl( int ch )
{
    int i,j;

    if ( !xprint ) {
    switch ( ch ) {
    case ETX:
        debug(F110,"Wyse Ctrl","keylock off",0);
        if ( debses )
            break;
        if ( wy_enhanced )
            keylock = FALSE ;
        break;
    case EOT:
        debug(F110,"Wyse Ctrl","keylock on",0);
        if ( debses )
            break;
        if ( wy_enhanced )
            keylock = TRUE ;
        break;
    case ENQ:
        debug(F110,"Wyse Ctrl","Request Answerback String",0);
        if ( debses )
            break;
        if (tt_answer)
            sendchars(answerback,strlen(answerback)) ;
        break;
    case ACK:
        debug(F110,"Wyse Ctrl","Cursor Right",0);
        if ( debses )
            break;
        if ( wy_enhanced )
            cursorright(0);
        break;
    case BEL:
        debug(F110,"Wyse Ctrl","Ring Bell",0);
        if ( debses )
            break;
        bleep(BP_BEL);
        break;
    case BS:
        debug(F110,"Wyse Ctrl","Cursor Left",0);
        if ( debses )
            break;
        cursorleft(0) ;
        break;
    case HT:
        debug(F110,"Wyse Ctrl","Horizontal Tab",0);
        if ( debses )
            break;
        wyse_tab();
        break;
    case LF:
        debug(F110,"Wyse Ctrl","Line Feed",0);
        if ( debses )
            break;
        if ( autoscroll && !protect || wherey[VTERM] < marginbot )
            wrtch((char) LF);
        break;
    case VT:
        debug(F110,"Wyse Ctrl","Cursor Up",0);
        if ( debses )
            break;
        cursorup(0) ;
        break;
    case FF:
        debug(F110,"Wyse Ctrl","Cursor Right",0);
        if ( debses )
            break;
        cursorright(0) ;
        break;
    case CR:
        debug(F110,"Wyse Ctrl","Carriage Return",0);
        if ( debses )
            break;
        wrtch((char) CR);
        break;
    case SO:
        debug(F110,"Wyse Ctrl","keylock off",0);
        if ( debses )
            break;
        keylock = FALSE ;
        break;
    case SI:
        debug(F110,"Wyse Ctrl","keylock on",0);
        if ( debses )
            break;
        keylock = TRUE ;
        break;
    case DLE:
        if ( !ISWY60(tt_type_mode) ) {
            if ( wy_enhanced ) {
                /* Address cursor to column */
                int col = wyinc() ;
                debug(F111,"Wyse Ctrl","Address cursor to column",col);

                if ( debses )
                    break;
                lgotoxy(VTERM,col-31,wherey[VTERM]);
            }
        } else {
            CHAR ch;
            ch = wyinc() ;

            debug(F111,"Wyse Ctrl","Print single char (not implemented)",ch);

            if ( debses )
                break;
        }
        break;
    case DC1:
        /* Enable transmission */
        debug(F110,"Wyse Ctrl","Enable transmission",0);
        break;
    case DC2:
        /* WY30 defines this as Copy Print */
        /* transparent printer with screen display */
        debug(F110,"Wyse Ctrl","Copy Print On",0);
        if ( debses )
            break;
        cprint = TRUE ;
        if ( !printon )
            printeron() ;
        break;
    case DC3:
        /* Disable transmission */
        debug(F110,"Wyse Ctrl","Disable Transmission",0);
        break;
    case DC4:
        /* Turn all print modes off */
        debug(F110,"Wyse Ctrl","Print Off",0);
        if ( debses )
            break;
        xprint = cprint = FALSE ;
        setaprint(FALSE);
        if ( !uprint && !xprint && !cprint && !aprint && printon )
            printeroff();
        break;
    case NAK:
        debug(F110,"Wyse Ctrl","Cursor Left",0);
        if ( debses )
            break;
        if ( wy_enhanced )
            cursorleft(0) ;
        break;
    case SYN:
        /* Cursordown - no scroll */
        debug(F110,"Wyse Ctrl","Cursor Down",0);
        if ( debses )
            break;
        cursordown(0) ;
        break;
    case ETB:
        /* Cursor Off */
        debug(F110,"Wyse Ctrl","Cursor Off",0);
        if ( debses )
            break;
        if ( wy_enhanced )
            cursorena[VTERM] = FALSE;
        break;
    case CAN:
        /* Transparent print on */
        debug(F110,"Wyse Ctrl","Transparent Print On",0);
        if ( debses )
            break;
        if ( wy_enhanced ) {
            xprint = TRUE ;
            if ( !printon )
                printeron() ;
        }
        break;
    case SUB:
        /* Clear unprotected page to spaces */
        debug(F110,"Wyse Ctrl","Clear unprotected page to spaces",0);
        if ( debses )
            break;
        if ( protect )
            selclrscreen( VTERM, SP ) ;
        else
            clrscreen( VTERM, SP ) ;
        lgotoxy(VTERM,1,1);
        break;
    case ESC:
        debug(F110,"Wyse Ctrl","Initiate Escape Sequence",0);
        /* initiate escape sequence */
        escstate = ES_GOTESC ;
        break;
    case XRS:
        /* Home cursor */
        debug(F110,"Wyse Ctrl","Home Cursor",0);
        if ( debses )
            break;
        lgotoxy(VTERM,1,1) ;
        break;
    case US:
        /* Cursor to start on next line */
        debug(F110,"Wyse Ctrl","Cursor to Next Line",0);
        if ( debses )
            break;
        if (autoscroll && !protect) {
            wrtch((CHAR)CR);
            wrtch((CHAR)LF);
        }
        break;
    }
    }
    else {      /* xprint */
        switch ( ch ) {
        case DC4:
            /* Turn all print modes off */
            debug(F110,"Wyse Ctrl","Print Off",0);
            if ( debses )
                break;
            xprint = cprint = FALSE ;
            setaprint(FALSE);
            if ( !uprint && !xprint && !cprint && !aprint && printon )
                printeroff();
            break;
        case ESC:
            /* initiate escape sequence */
            debug(F110,"Wyse Ctrl","Initiate Escape Sequence",0);
            escstate = ES_GOTESC ;
            break;
        }
    }
}

vtattrib
wysecharattr( int ch )
{
    vtattrib a ;

    a.blinking = FALSE;                 /* No blink */
    a.bold = FALSE;                     /* No bold */
    a.dim = FALSE;                      /* No dim */
    a.invisible = FALSE;                /* Visible */
    a.underlined = FALSE;               /* No underline */
    a.reversed = FALSE;                 /* No reverse video */
    a.unerasable = attrib.unerasable ;  /* Erasable */
    a.graphic = FALSE ;                 /* Not graphic character */
    a.wyseattr = FALSE ;                /* WYSE attribute */

    debug(F111,"Wyse","Character Attribute",ch);

    switch ( ch ) {
    case '0':
        /* Normal */
        break;
    case '1':
        /* Blank (no display) */
        a.invisible = TRUE ;
        break;
    case '2':
        /* Blink */
        a.blinking = TRUE ;
        break;
    case '3':
        /* Blank */
        a.invisible = TRUE ;
        break;
    case '4':
        /* Reverse */
        a.reversed = TRUE ;
        break;
    case '5':
        /* Reverse and Blank */
        a.reversed = TRUE ;
        a.invisible = TRUE ;
        break;
    case '6':
        /* Reverse and Blink */
        a.reversed = TRUE ;
        a.blinking = TRUE ;
        break;
    case '7':
        /* Reverse, blink?, and blank */
        a.reversed = TRUE ;
        a.blinking = TRUE ;
        a.invisible = TRUE ;
        break;
    case '8':
        /* Underscore */
        a.underlined = TRUE ;
        break;
    case '9':
        /* Underscore and blank */
        a.underlined = TRUE ;
        a.invisible = TRUE ;
        break;
    case ':':
      /* Underscore and blink */
        a.underlined = TRUE ;
        a.blinking = TRUE ;
        break;
    case ';':
      /* Underscore, blink, and blank */
        a.underlined = TRUE ;
        a.blinking = TRUE ;
        a.invisible = TRUE ;
        break;
    case '<':
      /* Underscore and Reverse */
        a.underlined = TRUE ;
        a.reversed = TRUE ;
        break;
    case '=':
      /* Underscore, Reverse, and blank */
        a.underlined = TRUE ;
        a.reversed = TRUE ;
        a.invisible = TRUE;
        break;
    case '>':
      /* Underscore, Reverse, and blink */
        a.underlined = TRUE ;
        a.reversed = TRUE ;
        a.blinking = TRUE ;
        break;
    case '?':
      /* Underscore, Reverse, Blink, and Blank */
        a.underlined = TRUE ;
        a.reversed = TRUE ;
        a.blinking = TRUE ;
        a.invisible = TRUE ;
        break;
    case 'p':
      /* Dim */
        a.dim = TRUE ;
        break;
    case 'q':
      /* Dim and Blank */
        a.dim = TRUE ;
        a.invisible = TRUE ;
        break;
    case 'r':
      /* Dim and Blink */
        a.dim = TRUE ;
        a.blinking = TRUE ;
        break;
    case 's':
      /* Dim, Blink?, and Blank */
        a.dim = TRUE ;
        a.blinking = TRUE ;
        a.invisible = TRUE ;
        break;
    case 't':
      /* Dim and Reverse */
        a.dim = TRUE ;
        a.reversed = TRUE ;
        break;
    case 'u':
      /* Dim, Reverse, and Blank */
        a.dim = TRUE ;
        a.reversed = TRUE ;
        a.invisible = TRUE ;
        break;
    case 'v':
      /* Dim, Reverse, and Blink */
        a.dim = TRUE ;
        a.reversed = TRUE ;
        a.blinking = TRUE ;
        break;
    case 'w':
      /* Dim, Reverse, Blink?, and Blank */
        a.dim = TRUE ;
        a.reversed = TRUE ;
        a.blinking = TRUE ;
        a.invisible = TRUE ;
        break;
    case 'x':
      /* Dim and Underscore */
        a.dim = TRUE ;
        a.underlined = TRUE ;
        break;
    case 'y':
      /* Dim, Underscore, and Blank */
        a.dim = TRUE ;
        a.underlined = TRUE ;
        a.invisible = TRUE ;
        break;
    case 'z':
      /* Dim, Underscore, and Blink */
        a.dim = TRUE ;
        a.underlined = TRUE ;
        a.blinking = TRUE ;
        break;
    case '{':
      /* Dim, Underscore, Blink, and Blank */
        a.dim = TRUE ;
        a.underlined = TRUE ;
        a.blinking = TRUE ;
        a.invisible = TRUE ;
        break;
    case '|':
      /* Dim, Underscore, and Reverse */
        a.dim = TRUE ;
        a.underlined = TRUE ;
        a.reversed = TRUE ;
        break;
    case '}':
      /* Dim, Underscore, Reverse, and Blank */
        a.dim = TRUE ;
        a.underlined = TRUE ;
        a.reversed = TRUE ;
        a.invisible = TRUE ;
        break;
    case '~':
      /* Dim, Underscore, Reverse, and Blink */
        a.dim = TRUE ;
        a.underlined = TRUE ;
        a.reversed = TRUE ;
        a.blinking = TRUE ;
        break;
    }
    return a;
}

USHORT
wyselineattr( int ch )
{
    USHORT lineattr = 0 ;

    debug(F111,"Wyse","Line Attribute",ch);

    switch ( ch ) {
    case '@':
        lineattr = VT_LINE_ATTR_NORMAL ;
        break;
    case 'A':
        lineattr = VT_LINE_ATTR_DOUBLE_WIDE ;
        break;
    case 'B':
        lineattr = VT_LINE_ATTR_DOUBLE_HIGH | VT_LINE_ATTR_UPPER_HALF ;
        break;
    case 'C':
        lineattr = VT_LINE_ATTR_DOUBLE_HIGH | VT_LINE_ATTR_LOWER_HALF ;
        break;
    case 'D':
        lineattr = VT_LINE_ATTR_DOUBLE_HIGH | VT_LINE_ATTR_UPPER_HALF
            | VT_LINE_ATTR_DOUBLE_WIDE ;
        break;
    case 'E':
        lineattr = VT_LINE_ATTR_DOUBLE_HIGH | VT_LINE_ATTR_LOWER_HALF
            | VT_LINE_ATTR_DOUBLE_WIDE ;
        break;
    }

    return lineattr ;
}

USHORT
wyselinebg( int ch )
{
    USHORT linebg = 0;

    debug(F111,"Wyse","Line Background",ch);

    switch ( ch ) {
    case 'G':
        linebg = WY_LINE_ATTR_BG_NORMAL ;
        break;
    case 'H':
        linebg = WY_LINE_ATTR_BG_BOLD ;
        break;
    case 'I':
        linebg = WY_LINE_ATTR_BG_INVISIBLE ;
        break;
    case 'J':
        linebg = WY_LINE_ATTR_BG_DIM ;
        break;
    }
    return linebg;
}

void
ApplyPageAttribute( int vmode, int x, int y, vtattrib vta )
{
    vtattrib oldvta, prevvta ;
    int rc ;

    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;
    prevvta = VscrnGetVtCharAttr( vmode, x-1, y-1 ) ;

    if ( !tt_hidattr  || ISHP(tt_type_mode) )
        vta.wyseattr = TRUE ;
    VscrnSetVtCharAttr( vmode, x-1, y-1, vta ) ;
    vta.wyseattr = FALSE ;

    oldvta = prevvta ;
    while ( TRUE ) {
        if ( ++x > VscrnGetWidth(vmode) ) {
            if ( ++y > marginbot ) {
                break  ;                /* we are done */
            }
            x = 1 ;
        }

        /* Retrieve the attributes of the new position */
        oldvta = VscrnGetVtCharAttr( vmode, x-1, y-1 ) ;
        if ( (!tt_hidattr || ISHP(tt_type_mode)) && oldvta.wyseattr)
            break;
        if ( oldvta.unerasable && protect)
            continue;
        if ( tt_hidattr &&
             (oldvta.reversed != prevvta.reversed ||
               oldvta.blinking != prevvta.blinking ||
               oldvta.underlined != prevvta.underlined ||
               oldvta.bold != prevvta.bold ||
               oldvta.invisible != prevvta.invisible ||
               oldvta.dim != prevvta.dim))
            break;
        vta.unerasable = oldvta.unerasable;
        VscrnSetVtCharAttr( vmode, x-1, y-1, vta ) ;
    }
    ReleaseVscrnMutex( vmode ) ;
}

void
ApplyLineAttribute( int vmode, int x, int y, vtattrib vta )
{
    vtattrib oldvta, prevvta ;

    RequestVscrnMutex( vmode, SEM_INDEFINITE_WAIT ) ;
    prevvta = VscrnGetVtCharAttr( vmode, x-1, y-1 ) ;

    if ( !tt_hidattr || ISHP(tt_type_mode) )
        vta.wyseattr = TRUE ;
    VscrnSetVtCharAttr( vmode, x-1, y-1, vta ) ;
    vta.wyseattr = FALSE ;

    oldvta = prevvta ;
    for ( x++ ; x <= VscrnGetWidth(vmode); x++ ) {
        /* Retrieve the attributes of the new position */
        oldvta = VscrnGetVtCharAttr( vmode, x-1, y-1 ) ;
        if ( (!tt_hidattr || ISHP(tt_type_mode)) && oldvta.wyseattr)
            break;
        if ( oldvta.unerasable )
            continue;
        if ( tt_hidattr &&
             (oldvta.reversed != prevvta.reversed ||
               oldvta.blinking != prevvta.blinking ||
               oldvta.underlined != prevvta.underlined ||
               oldvta.bold != prevvta.bold ||
               oldvta.invisible != prevvta.invisible ||
               oldvta.dim != prevvta.dim))
            break;
        vta.unerasable = oldvta.unerasable;
        VscrnSetVtCharAttr( vmode, x-1, y-1, vta ) ;
    }
    ReleaseVscrnMutex( vmode ) ;
}

char *
wysedefkey( int key )
{
    debug(F111,"Wyse","Define Key to Default",key);
#ifndef NOKVERBS
    switch ( key + K_WYF01 ) {
    case K_WYF01: return strdup("\01@\r");    /* F1 */
    case K_WYF02: return strdup("\01A\r");
    case K_WYF03: return strdup("\01B\r");
    case K_WYF04: return strdup("\01C\r");
    case K_WYF05: return strdup("\01D\r");
    case K_WYF06: return strdup("\01E\r");
    case K_WYF07: return strdup("\01F\r");
    case K_WYF08: return strdup("\01G\r");
    case K_WYF09: return strdup("\01H\r");
    case K_WYF10: return strdup("\01I\r");
    case K_WYF11: return strdup("\01J\r");
    case K_WYF12: return strdup("\01K\r");
    case K_WYF13: return strdup("\01L\r");
    case K_WYF14: return strdup("\01M\r");
    case K_WYF15: return strdup("\01N\r");
    case K_WYF16: return strdup("\01O\r");
    case K_WYF17: return strdup("\01P\r");
    case K_WYF18: return strdup("\01Q\r");
    case K_WYF19: return strdup("\01R\r");
    case K_WYF20: return strdup("\01S\r");   /* F20 */
    case K_WYSF01: return strdup("\01`\r");   /* Shift-F1 */
    case K_WYSF02: return strdup("\01a\r");
    case K_WYSF03: return strdup("\01b\r");
    case K_WYSF04: return strdup("\01c\r");
    case K_WYSF05: return strdup("\01d\r");
    case K_WYSF06: return strdup("\01e\r");
    case K_WYSF07: return strdup("\01f\r");
    case K_WYSF08: return strdup("\01g\r");
    case K_WYSF09: return strdup("\01h\r");
    case K_WYSF10: return strdup("\01i\r");
    case K_WYSF11: return strdup("\01j\r");
    case K_WYSF12: return strdup("\01k\r");
    case K_WYSF13: return strdup("\01l\r");
    case K_WYSF14: return strdup("\01m\r");
    case K_WYSF15: return strdup("\01n\r");
    case K_WYSF16: return strdup("\01o\r");
    case K_WYSF17: return strdup("\01p\r");
    case K_WYSF18: return strdup("\01q\r");
    case K_WYSF19: return strdup("\01r\r");
    case K_WYSF20: return strdup("\01s\r");   /* Shift-F20 */
    case K_WYBS     : return strdup("\x08");      /* Backspace */
    case K_WYCLRLN  : return strdup("\x1BT");     /* Clr Line */
    case K_WYSCLRLN : return strdup("\x1Bt");     /* Shift Clr Line */
    case K_WYCLRPG  : return strdup("\x1BY");     /* Clr Page */
    case K_WYSCLRPG : return strdup("\x1By");     /* Shift Clr Page */
    case K_WYDELCHAR: return strdup("\x1BW");     /* Del Char */
    case K_WYDELLN  : return strdup("\x1BR");     /* Del Line */
    case K_WYENTER  : return strdup("\x0D");      /* Enter */
    case K_WYESC    : return strdup("\x1B");      /* Escape */
    case K_WYHOME   : return strdup("\x1E");      /* Home */
    case K_WYSHOME  : return strdup("\x1B{");     /* Shift Home */
    case K_WYINSERT : return strdup("\x1Bq");     /* Insert */
    case K_WYINSCHAR: return strdup("\x1BQ");     /* Ins Char */
    case K_WYINSLN  : return strdup("\x1B\x45");  /* Ins Line */
    case K_WYPGNEXT : return strdup("\x1BK");     /* Page Next*/
    case K_WYPGPREV : return strdup("\x1BJ");     /* Page Prev*/
    case K_WYREPLACE: return strdup("\x1Br");     /* Replace */
    case K_WYRETURN: return strdup("\x0D");       /* Return */
    case K_WYTAB   : return strdup("\x09");       /* Tab */
    case K_WYSTAB  : return strdup("\x1BI");      /* Shift Tab */
    case K_WYPRTSCN: return strdup("\x1BP");      /* Print Screen */
    case K_WYSESC   : return NULL;                /* Shift Esc */
    case K_WYSBS    : return strdup("\x08");      /* Shift BS */
    case K_WYSENTER : return NULL ;               /* Shift Enter */
    case K_WYSRETURN: return NULL ;               /* Shift Return */
    case K_WYUPARR : return strdup("\x0b");       /* Up Arrow - VT */
    case K_WYDNARR : return strdup("\x0a");       /* Dn Arrow - LF */
    case K_WYLFARR : return strdup("\x08");       /* Left Arrow - BS */
    case K_WYRTARR : return strdup("\x0c");       /* Right Arrow - NL */
    case K_WYSUPARR: return NULL ;                /* Shift Up Arrow */
    case K_WYSDNARR: return NULL ;                /* Shift Dn Arrow */
    case K_WYSLFARR: return NULL ;                /* Shift Left Arrow */
    case K_WYSRTARR: return NULL ;                /* Shift Right Arrow */
    case K_WYSEND : return strdup("\x1B\x37");    /* Send */
    case K_WYSSEND: return NULL ;                 /* Shift Send */
    default:
        debug(F111,"Wyse","Key undefined",key);
        return NULL ;
    }
#else
    return(NULL);
#endif /* NOKVERBS */
}

void
wyseascii( int ch )
{
    int i,j,k,n,x,y,z;
    vtattrib attr ;
    viocell blankvcell;

    if (printon && (is_xprint() || is_uprint()))
        prtchar(ch);

    if ( escstate == ES_GOTESC )/* Process character as part of an escstate sequence */
    {
        if ( ch < SP ) {
            escstate = ES_NORMAL ;
            wysectrl(ch) ;
        }
        else
        {
            escstate = ES_ESCSEQ ;
            if ( !xprint ) {
            switch ( ch ) {
            case SP:
                /* Reports the terminal identification to the */
                /* Host computer */
                debug(F110,"Wyse Escape","Terminal ID Request",0);
                if ( !debses )
                    sendchars( tt_info[tt_type].x_id,
                               strlen(tt_info[tt_type].x_id) ) ;
                break;
            case '!':
                if ( !ISWY60(tt_type_mode) ) {
                    /* Writes all unprotected character positions with   */
                    /* a specified attribute code.  This has a format of */
                    /* ESC ! ATTR where ATTR = attribute code.           */
                    /* This is not supported by the Wyse 60              */
                    int xs = VscrnGetWidth(VTERM);
                    int ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                    vtattrib vta ;
                    debug(F110,"Wyse Escape","Write Unprotected with Attribute",0);
                    ch = wyinc();
                    switch ( ch ) {
                    case SP: {
                        if ( debses )
                             break;
                        if ( protect )
                            selclrscreen( VTERM, SP ) ;
                        else
                            clrscreen( VTERM, SP );
                        lgotoxy(VTERM,1,1);       /* and home the cursor */
                        break;
                    }
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
                    case ':':
                    case ';':
                    case '<':
                    case '=':
                    case '>':
                    case '?':
                    case 'p':
                    case 'q':
                    case 'r':
                    case 's':
                    case 't':
                    case 'u':
                    case 'v':
                    case 'w':
                    case 'x':
                    case 'y':
                    case 'z':
                    case '{':
                    case '|':
                    case '}':
                    case '~':
                    {
                        attr = wysecharattr( ch ) ;
                        if ( debses )
                            break;

                        for ( y = 0 ; y < ys ; y++ )
                            for ( x = 0 ; x < xs ; x++ ) {
                                vta = VscrnGetVtCharAttr( VTERM, x, y ) ;
                                if ( !protect || !vta.unerasable )
                                    VscrnSetVtCharAttr( VTERM, x, y, attr ) ;
                            }
                        break;
                    }
                    }
                }
                break;
            case '"':
                /* Unlocks the keyboard */
                debug(F110,"Wyse Escape","keylock off",0);
                if ( !debses )
                    keylock = FALSE ;
                break;
            case '#':
                /* Locks the keyboard */
                debug(F110,"Wyse Escape","keylock on",0);
                if ( !debses )
                    keylock = TRUE ;
                break;
            case '$':
                debug(F110,"Wyse Escape","$",0);
                break;
            case '%':
                debug(F110,"Wyse Escape","%",0);
                break;
            case '&':
                /* Turns the protect submode on and prevents the auto */
                /* scroll operation */
                debug(F110,"Wyse Escape","Protect On",0);
                if ( !debses )
                    protect = TRUE ;
                break;
            case '\'':
                /* Turns the protect submode off and allows the auto */
                /* scroll operation */
                debug(F110,"Wyse Escape","Protect Off",0);
                if ( !debses )
                    protect = FALSE ;
                break;
            case '(':
                /* Turns the write protect submode off */
                debug(F110,"Wyse Escape","Write Protect Off",0);
                if ( debses || !writeprotect )
                    break;
                writeprotect = FALSE ;
                attrib.unerasable = FALSE;
                break;
            case ')':
                /* Turns the write protect submode on */
                debug(F110,"Wyse Escape","Write Protect On",0);
                if ( debses || writeprotect )
                    break;
                writeprotect = TRUE ;
                attrib.unerasable = TRUE ;
                break;
            case '*':
                /* Clears the screen to nulls. */
                /* The protect submode is turned off */
                debug(F110,"Wyse Escape","Clear screen to NUL; Protect Off; Home Cursor",0);
                if ( debses )
                    break;
                protect = FALSE ;
                writeprotect = FALSE ;
                insertmode = FALSE ;
                clrscreen( VTERM, NUL ) ;
                lgotoxy(VTERM,1,1);       /* and home the cursor */
                break;
            case '+':
                /* Clears the screen to spaces. */
                /* The protect submode is turned off */
                debug(F110,"Wyse Escape","Clear screen to SP; Protect Off; Home Cursor",0);
                if ( debses )
                    break;
                attrib.unerasable = FALSE;
                protect = FALSE ;
                writeprotect = FALSE ;
                insertmode = FALSE ;
                clrscreen( VTERM, SP );
                lgotoxy(VTERM,1,1);       /* and home the cursor */
                break;
            case ',': {
                /* Clears the screen to protected spaces. */
                /* The protect submode is turned off */
                /* Home Cursor */
                int xs = VscrnGetWidth(VTERM);
                int ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                vtattrib vta = {0,0,0,0,0,1,0,0,0,0,0}; /* Protected */
                debug(F110,"Wyse Escape","Clear screen to Protected SP; Protect Off; Home Cursor",0);
                if ( debses )
                    break;
                clrscreen( VTERM, SP ) ;
                lgotoxy(VTERM,1,1);       /* and home the cursor */
                for ( y = 0 ; y < ys ; y++ )
                    for ( x = 0 ; x < xs ; x++ )
                        VscrnSetVtCharAttr( VTERM, x, y, vta ) ;
                protect = FALSE ;
                writeprotect = FALSE ;
                insertmode = FALSE ;
                break;
            }
            case '-': {
                /* Moves the cursor to a specified text segment. */
                /* This has a multiple code sequence of:         */
                /* ESC - nrc                                     */
                /* where n = the text segment number, 0 or 1     */
                /*       r = the row code                        */
                /*       c = the column code                     */
                int n,r,c ;
                debug(F110,"Wyse Escape","Move cursor to text segment",0);
                n = wyinc();

                r = wyinc();

                c = wyinc();

                if ( debses )
                    break;
                if ( n == WY_FIELD_APPL )
                    lgotoxy(VTERM,c-31,r-31);
                break;
            }
            case '.': {
                /* Clears all unprotected character positions with    */
                /* a specified character code.  This has a format of: */
                /*   ESC . CODE where CODE = the character hex value  */
                /* Use protected characters if write-protect mode is on */
                int xs = VscrnGetWidth(VTERM);
                int ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                vtattrib vta = {0,0,0,0,0,1,0,0,0,0,0}; /* Protected */
                int C1, C2, C ;
                debug(F110,"Wyse Escape","Clear unprotected chars with code",0);
                C1 = wyinc() ;
                C2 = wyinc() ;

                if ( debses )
                    break;
                C = unhex(C1) * 16 + unhex(C2) ;
                if ( protect )
                    selclrscreen(VTERM, C) ;
                else
                    clrscreen( VTERM, C ) ;
                lgotoxy(VTERM,1,1);       /* and home the cursor */
                if ( writeprotect ) {
                    for ( y = 0 ; y < ys ; y++ )
                        for ( x = 0 ; x < xs ; x++ ) {
                            vta = VscrnGetVtCharAttr( VTERM, x, y ) ;
                            vta.unerasable = TRUE ;
                            VscrnSetVtCharAttr( VTERM, x, y, vta ) ;
                        }
                }
                protect = FALSE ;
                break;
            }
            case '/': {
                /* Transmits the active text segment number and cursor */
                /* address. */
                char buf[5] ;

                debug(F110,"Wyse Escape","Request Active Text Segment and Cursor Position",0);
                if ( debses )
                    break;

                buf[0] = 0;
                buf[1] = wherey[VTERM]+31 ;
                buf[2] = wherex[VTERM]+31 ;
                buf[3] = CR ;
                buf[4] = NUL ;

                sendchars( buf, 4 ) ;
                break;
            }
            case '0':
                /* Clears all tab settings */
                debug(F110,"Wyse Escape","Clear all tab settings",0);
                if ( debses )
                    break;
                for (j = 1; j <=MAXTERMCOL; ++j)
                    htab[j] = '0';
                break;
            case '1':
                /* Sets a tab stop */
                debug(F110,"Wyse Escape","Set tab stop",0);
                if ( debses )
                    break;
                htab[wherex[VTERM]] = 'T';
                break;
            case '2':
                /* Clears a tab stop */
                debug(F110,"Wyse Escape","Clear tab stop",0);
                if ( debses )
                    break;
                htab[wherex[VTERM]] = '0';
                break;
            case '3':
                /* Clears all tab stops - WY30 */
                debug(F110,"Wyse Escape","Clear all tab stops",0);
                if ( debses )
                    break;
                for (j = 1; j <=MAXTERMCOL; ++j)
                    htab[j] = '0';
                break;
            case '4': {
                /* Sends all unprotected characters from the start-of-row */
                /* to the host computer. */
                int x, fs=1 ;

                debug(F110,"Wyse Escape","Send all unprotected chars from BOL",0);
                if ( debses )
                    break;

                for ( x=0;x<=wherex[VTERM]-1;x++ ) {
                    if ( !VscrnGetVtCharAttr(VTERM, x, wherey[VTERM]-1).unerasable ) {
                        int ch = VscrnGetCell( VTERM, x, wherey[VTERM]-1 )->c ;
                        if ( ch || !wy_nullsuppress ) {
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
                        fs = 0 ;
                    }
                    else if ( !fs ) {
                        sendchar(XFS);
                        fs = 1 ;
                    }
                }
                switch ( wy_blockend ) {
                case EOB_CRLF_ETX:
                    sendchar(ETX);
                    break;
                case EOB_US_CR:
                    sendchar(CR);
                    break;
                }
                break;
            }
            case '5': {
                /* Sends all unprotected characters from the start-of-text */
                /* to the host computer */
                int x,y,fs=1 ;

                debug(F110,"Wyse Escape","Send all unprotected chars from BOS",0);
                if ( debses )
                    break;

                for ( y=0;y<=wherey[VTERM]-1; y++ ) {
                    int xe;
                    if (y == wherey[VTERM]-1)
                        xe = wherex[VTERM]-1;
                    else
                        xe = VscrnGetWidth(VTERM)-1;
                    for ( x=0;x<=xe;x++ )
                        if ( !VscrnGetVtCharAttr(VTERM, x, y).unerasable ) {
                            int ch = VscrnGetCell( VTERM, x, y )->c ;
                            if ( ch || !wy_nullsuppress ) {
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
                            fs = 0 ;
                        }
                        else if ( !fs ) {
                            sendchar(XFS);
                            fs = 1 ;
                        }
                    if ( y < wherey[VTERM]-1 ) {
                        switch ( wy_blockend ) {
                        case EOB_CRLF_ETX:
                            sendchar(CR);
                            sendchar(LF);
                            break;
                        case EOB_US_CR:
                            sendchar(US);
                            break;
                        }
                    }
                }
                switch ( wy_blockend ) {
                case EOB_CRLF_ETX:
                    sendchar(ETX);
                    break;
                case EOB_US_CR:
                    sendchar(CR);
                    break;
                }
                break;
            }
            case '6': {
                /* Sends all characters from the start-of-row to the host */
                /* computer */
                int x, fs=0, ch=0 ;

                debug(F110,"Wyse Escape","Send all chars from BOL",0);
                if ( debses )
                    break;

                for ( x=0;x<=wherex[VTERM];x++ ) {
                    if ( !VscrnGetVtCharAttr(VTERM, x, wherey[VTERM]-1).unerasable ) {
                        if ( fs ) {
                            sendchars("\x1b(",2);
                            fs = 0;
                        }
                    }
                    else if ( !fs ) {
                        sendchars("\x1b)",2);
                        fs = 1 ;
                    }
                    ch = VscrnGetCell( VTERM, x, wherey[VTERM]-1 )->c ;
                    if ( ch  || !wy_nullsuppress ) {
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
                }
                switch ( wy_blockend ) {
                case EOB_CRLF_ETX:
                    sendchar(ETX);
                    break;
                case EOB_US_CR:
                    sendchar(CR);
                    break;
                }
                break;
            }
            case '7': {
                /* Sends all characters from the start-of-text to the host */
                /* computer */
                int x,y,fs=0,ch=0;

                debug(F110,"Wyse Escape","Send all chars from BOS",0);
                if ( debses )
                    break;

                for ( y=0;y<=wherey[VTERM]-1; y++ ) {
                    int xe;
                    if (y == wherey[VTERM]-1)
                        xe = wherex[VTERM]-1;
                    else
                        xe = VscrnGetWidth(VTERM)-1;
                    for ( x=0;x<=xe;x++ ) {
                        if ( !VscrnGetVtCharAttr(VTERM, x, y).unerasable ) {
                            if ( fs ) {
                                sendchars("\x1b(",2);
                                fs = 0;
                            }
                        }
                        else if ( !fs ) {
                            sendchars("\x1b)",2);
                            fs = 1 ;
                        }
                        ch = VscrnGetCell( VTERM, x, y )->c;
                        if ( ch || !wy_nullsuppress ) {
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
                    }
                    if ( y < wherey[VTERM]-1 ) {
                        switch ( wy_blockend ) {
                        case EOB_CRLF_ETX:
                            sendchar(CR);
                            sendchar(LF);
                            break;
                        case EOB_US_CR:
                            sendchar(US);
                            break;
                        }
                    }
                }
                switch ( wy_blockend ) {
                case EOB_CRLF_ETX:
                    sendchar(ETX);
                    break;
                case EOB_US_CR:
                    sendchar(CR);
                    break;
                }
                break;
            }
            case '8':
                /* Enters a start-of-message character (STX) */
                debug(F110,"Wyse Escape","Enter Start of message Char",0);
                if ( debses )
                    break;
                wrtch(STX);
                break;
            case '9':
                /* Enters an end-of-message character (ETX) */
                debug(F110,"Wyse Escape","Enter End of message Char",0);
                if ( debses )
                    break;
                wrtch(ETX);
                break;
            case ':':
                /* Clears all unprotected characters to nulls */
                /* Home Cursor */
                debug(F110,"Wyse Escape","Clear all unprotected chars to NUL; Home Cursor",0);
                if ( debses )
                    break;
                if ( protect )
                    selclrscreen(VTERM,NUL);
                else
                    clrscreen( VTERM, NUL ) ;
                lgotoxy(VTERM,1,1);
                break;
            case ';':
                /* Clears all unprotected characters to spaces */
                /* Home Cursor */
                debug(F110,"Wyse Escape","Clear all unprotected chars to SP; Home Cursor",0);
                if ( debses )
                    break;
                if ( protect )
                    selclrscreen(VTERM,SP);
                else
                    clrscreen( VTERM, SP );
                lgotoxy(VTERM,1,1);
                break;
            case '<':
                /* Turn off keyclick - WY30 */
                debug(F110,"Wyse Escape","keyclick Off",0);
                if ( debses )
                    break;
                if ( wy_enhanced )
                    setkeyclick(FALSE);
                break;
            case '=': {
                /* Moves the cursor to a specified row and column for */
                /* an 80-column screen.  This has a format of:        */
                /*    ESC = rc where                                  */
                /* r = the row code; and c = the column code          */
                int r,c ;

                debug(F110,"Wyse Escape","Move cursor to row and column (80)",0);
                r = wyinc() ;

                if ( r < SP ) {
                    escstate = ES_NORMAL ;
                    wyseascii(r) ;
                    return ;
                }
                c = wyinc() ;

                if ( c < SP ) {
                    escstate = ES_NORMAL ;
                    wyseascii(c) ;
                    return ;
                }
                if ( debses )
                    break;
                if ( /* VscrnGetWidth(VTERM) == 80 && */ c > 31 && r > 31 )
                     lgotoxy(VTERM,c-31,r-31);
                else {
                        break;
                }
                break;
            }
            case '>':
                /* Turn on keyclick - WY30 */
                debug(F110,"Wyse Escape","keyclick on ",0);
                if ( debses )
                    break;
                if ( wy_enhanced )
                    setkeyclick(TRUE);
                break;
            case '?':
                /* Transmits the cursor address for the active text    */
                /* segment of an 80-column screen only.  The format is */
                /*    rc CR where r = the row code; and c = the column */
                /* code.                                               */
                debug(F110,"Wyse Escape","Request Cursor Position (80)",0);
                if ( debses )
                    break;
                if ( 1 /* VscrnGetWidth(VTERM) == 80 */ ) {
                    char buf[4] ;
                    buf[0] = (char)(wherey[VTERM]+31) ;
                    buf[1] = (char)(wherex[VTERM]+31) ;
                    buf[2] = CR ;
                    buf[3] = NUL ;
                    sendchars(buf,3) ;
                }
                break;
            case '@':
                /* Sends all unprotected characters from the start-of-text */
                /* to the auxiliary port.  Each row is terminated with:    */
                /*   CR LF NULL                                            */
                debug(F110,"Wyse Escape","Send all unprotected chars from BOS to AUX",0);
                break;
            case 'A': {
                /* Sets a video attribute for a specific message field or  */
                /* the entire application display area.  This has a        */
                /* multiple code sequence of ESC A n ATTR where:           */
                /*   n = field code; ATTR = attribute code                 */
                int field = wyinc() ;
                int n = wyinc();

                debug(F110,"Wyse Escape",
                       "Set attribute for message field or entire display",0);
                if ( debses )
                    break;
                if ( n == SP ) {
                    if ( field == WY_FIELD_APPL ) {
                        wrtch(tt_sac);
                    }
                }
                else {
                    attr = wysecharattr( n ) ;
                    if ( field == WY_FIELD_APPL ) {
                        if ( attrmode == ATTR_PAGE_MODE ) {
                            int x = wherex[VTERM];
                            int y = wherey[VTERM];
                            if ( !tt_hidattr )
                                wrtch(tt_sac) ;
                            ApplyPageAttribute( VTERM, x, y, attr ) ;
                        }
                        else if ( attrmode == ATTR_LINE_MODE ) {
                            int x = wherex[VTERM];
                            int y = wherey[VTERM];
                            if ( !tt_hidattr )
                                wrtch(tt_sac) ;
                            ApplyLineAttribute( VTERM, x, y, attr ) ;
                        }
                        /* set attribute for additional chars */
                        attrib = attr ;
                    }
                }
                break;
            }
            case 'B':
                /* Places the terminal in the block mode.                  */
                debug(F110,"Wyse Escape","Enter Block Mode",0);
                if ( debses )
                    break;
                wy_block = TRUE;
                break;
            case 'C':
                /* Places ther terminal in a conversational mode           */
                debug(F110,"Wyse Escape","Enter Conversational Mode",0);
                if ( debses )
                    break;
                wy_block = FALSE;
                break;
            case 'D': {
                /* Selects the full duplex or half-duplex conversation   */
                /* modes.  This has the multiple code sequence:          */
                /*   ESC D x where x = F full duplex mode; H half duplex */
                int n = wyinc();

                if ( debses )
                    break;
                switch ( n ) {
                case 'F':
                    debug(F110,"Wyse Escape","Full Duplex Mode",0);
                    if ( duplex_sav < 0 )
                        duplex_sav = duplex ;
                    duplex = FALSE ;
                    break;
                case 'H':
                    debug(F110,"Wyse Escape","Half Duplex Mode",0);
                    if ( duplex_sav < 0 )
                        duplex_sav = duplex ;
                    duplex = TRUE ;
                    break;
                }
                break;
            }
            case 'E':
                /* Inserts a row of spaces */
                debug(F110,"Wyse Escape","Insert row of spaces",0);
                if ( debses )
                    break;
                if ( !protect )
                    VscrnScroll(VTERM, DOWNWARD, wherey[VTERM] - 1,
                                 marginbot - 1, 1, FALSE, SP);
                break;
            case 'F': {
                /* Enters a message in the host message field.  This has     */
                /* a format of:  ESC F aaaa CR where                         */
                /*    aaaa = a character string of up to 46 characters       */
                /*           for an 80-column screen or up to 100            */
                /*           characters for a 132-column screen              */
                /* This is also known as the Title Bar.  So set window title */
                char buf[102] = "" ;
                int  n = 0 ;

                debug(F110,"Wyse Escape","Enter message into message field",0);

                for ( ; n < ((VscrnGetWidth(VTERM)<=80)?46:100); n++ )
                {
                    buf[n] = wyinc();
                    if ( buf[n] == CR )
                        break;
                }
                buf[n] = '\0';

                if ( debses )
                    break;
                /* assign buf to host message field */
                os2settitle(buf,1);
                break;
            }
            case 'G': {
                /* Sets a video attribute within the application display  */
                /* area.  The attribute occupies a space.  This has a     */
                /* multiple code sequence of:   ESC G ATTR where:         */
                /*    ATTR = attribute code.                              */
                int n = wyinc() ;

                debug(F110,"Wyse Escape","Set attribute within display area",0);
                if ( debses )
                    break;
                switch ( n ) {
                case SP:
                    if ( tt_hidattr ) {
                        n = '0';        /* Same as normal */
                    }
                    else {
                        wrtch(tt_sac) ;
                        break;
                    }
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
                case ':':
                case ';':
                case '<':
                case '=':
                case '>':
                case '?':
                case 'p':
                case 'q':
                case 'r':
                case 's':
                case 't':
                case 'u':
                case 'v':
                case 'w':
                case 'x':
                case 'y':
                case 'z':
                case '{':
                case '|':
                case '}':
                case '~':{
                    attr = wysecharattr( n ) ;
                    if ( attrmode == ATTR_PAGE_MODE ) {
                        int x = wherex[VTERM];
                        int y = wherey[VTERM];
                        if ( !tt_hidattr )
                            wrtch(tt_sac) ;
                        ApplyPageAttribute( VTERM, x, y, attr ) ;
                    }
                    else if ( attrmode == ATTR_LINE_MODE ) {
                        int x = wherex[VTERM];
                        int y = wherey[VTERM];
                        if ( !tt_hidattr )
                            wrtch(tt_sac) ;
                        ApplyLineAttribute( VTERM, x, y, attr ) ;
                    }
                    attrib = attr ;
                    break;
                }
                case '@':
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E': {
                    wyselineattr( n ) ;
                    if ( !tt_hidattr &&
                         (attrmode == ATTR_PAGE_MODE ||
                           attrmode == ATTR_LINE_MODE) )
                        wrtch(tt_sac) ;
                    break;
                }
                case 'G':
                case 'H':
                case 'I':
                case 'J': {
                    wyselinebg( n ) ;
                    if ( !tt_hidattr &&
                         (attrmode == ATTR_PAGE_MODE ||
                           attrmode == ATTR_LINE_MODE) )
                        wrtch(tt_sac) ;
                    break;
                }
                }
                break;
            }
            case 'H': {
                /* Enters a graphic character at the cursor locations.    */
                /* This has a multiple code sequence of ESC H x where     */
                /*    x = the graphic character code                      */
                /* ESC H STX (CTRL B) turns on the graphic submode        */
                /* ESC H ETX (CTRL C) turns off the graphic submode       */
                int x = wyinc(), gch = NUL ;

                switch ( x ) {
                case 2:
                    debug(F110,"Wyse Escape","Graphic Chars On",0);
                    if ( debses )
                        break;
                    wysegraphics = TRUE ;
                    break;
                case 3:
                    debug(F110,"Wyse Escape","Graphic Chars Off",0);
                    if ( debses )
                        break;
                    wysegraphics = FALSE ;
                    break;
                default:
                    debug(F110,"Wyse Escape","Enter Graphic Char",0);
                    if ( debses )
                        break;
                    gch = xlwygrph(x);
                    if ( gch )
                        wrtch( gch ) ;
                    else {
                        escstate = ES_NORMAL ;
                        wyseascii(x);
                        return ;
                    }
                }
                break;
            }
            case 'I':
                /* Moves the cursor left to the previous tab stop */
                debug(F110,"Wyse Escape","Reverse tab",0);
                if ( debses )
                    break;
                wyse_backtab();
                break;
            case 'J':
                /* Activates the alternate text segment */
                debug(F110,"Wyse Escape","Activate alternate text segment",0);
                break;
            case 'K':
                /* Activates the alternate text segment.  Same as ESC J */
                debug(F110,"Wyse Escape","Activate alternate text segment",0);
                break;
            case 'L':
                /* Sends all characters unformatted to the auxiliary port. */
                /* Attribute codes are sent as spaces.  Row-end            */
                /* sequences are not sent.  Same as ESC p.                 */
                debug(F110,"Wyse Escape","Send all chars unformatted to AUX ",0);
                break;
            case 'M':
                /* Cause the terminal to send the character at the cursor  */
                /* position to the host computer.                          */
                debug(F110,"Wyse Escape","Send current char to host",0);
                if ( debses )
                    break;
                if ( tt_senddata ) {
                    unsigned char * bytes;
                    int nbytes;
                    unsigned short ch = VscrnGetCell( VTERM, wherex[VTERM]-1,
                                       wherey[VTERM]-1 )->c;
                    if ( isunicode() )
                        nbytes = utorxlat(ch,&bytes);
                    else
                        nbytes = utorxlat(ch,&bytes);
                    while ( nbytes-- > 0 )
                        sendchar(*bytes++) ;
                } else
                    sendchar(SP);
                break;
            case 'N':
                /* turns the no scroll submode on */
                debug(F110,"Wyse Escape","No Scroll on",0);
                if ( debses )
                    break;
                autoscroll = FALSE ;
                break;
            case 'O':
                /* Turns the no scroll submode off */
                debug(F110,"Wyse Escape","No Scroll off",0);
                if ( debses )
                    break;
                autoscroll = TRUE ;
                break;
            case 'P': {
                /* Sends all protected and unprotected characters to the  */
                /* auxiliary port, regardless of the mode setting.        */
                int x;

                debug(F110,"Wyse Escape","Send all chars to AUX",0);
                if ( debses )
                    break;

                x = xprintff; xprintff = 0; /*   Print/Dump current screen */
                prtscreen(VTERM,1,VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0));
                xprintff = x;
                if ( wy_enhanced )
                    ttoc(ACK);
                break;
            }
            case 'Q':
                /* Inserts a space. */
                debug(F110,"Wyse Escape","Insert SP",0);
                if ( debses )
                    break;
                blankvcell.c = SP;
                blankvcell.a = geterasecolor(VTERM);
                VscrnScrollRt(VTERM, wherey[VTERM] - 1,
                               wherex[VTERM] - 1, wherey[VTERM] - 1,
                               VscrnGetWidth(VTERM) - 1, 1, blankvcell);
                break;
            case 'R':
                /* Deletes a row. */
                debug(F110,"Wyse Escape","Delete a row",0);
                if ( debses )
                    break;
                VscrnScroll(VTERM,
                             UPWARD,
                             wherey[VTERM] - 1,
                             marginbot - 1,
                             1,
                             FALSE,
                             SP);
                break;
            case 'S': {
                /* Sends a message unprotected */
                int x,y,ch,
                w=VscrnGetWidth(VTERM),
                h=VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0),
                xs=-1,
                ys=-1,
                xe=w-1,
                ye=h-1,
                fs=1 ;

                debug(F110,"Wyse Escape","Send a message unprotected",0);
                if ( debses )
                    break;

                /* First figure out if we have markers, STX then ETX */
                for ( y=0;y<h;y++ )
                    for ( x=0;x<w;x++ ) {
                        ch = VscrnGetCell( VTERM, x, y )->c ;
                        if ( ch == STX && xs < 0 && ys < 0 ) {
                            xs = x+1 ;
                            ys = y ;
                        }
                        else if ( ch == ETX && xs >= 0 && ys >= 0 ) {
                            xe = x-1 ;
                            ye = y ;
                            break;
                        }
                    }
                if ( xs < 0 || ys < 0 ) {
                    xs = 0 ;
                    ys = 0 ;
                }

                /* Now send the data */
                for ( y=ys;y<=ye; y++ ) {
                    for ( x=(y==ys)?xs:0 ; x <= (y==ye?xe:w-1) ; x++ )
                        if ( !VscrnGetVtCharAttr(VTERM, x, y).unerasable ) {
                            ch = VscrnGetCell( VTERM, x, y )->c;
                            if ( ch || !wy_nullsuppress ) {
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
                            fs = 0 ;
                        }
                        else if ( !fs ) {
                            sendchar(XFS);
                            fs = 1 ;
                        }
                    if ( y < ye ) {
                        switch ( wy_blockend ) {
                        case EOB_CRLF_ETX:
                            sendchar(CR);
                            sendchar(LF);
                            break;
                        case EOB_US_CR:
                            sendchar(US);
                            break;
                        }
                    }
                }
                switch ( wy_blockend ) {
                case EOB_CRLF_ETX:
                    sendchar(ETX);
                    break;
                case EOB_US_CR:
                    sendchar(CR);
                    break;
                }
                break;
            }
            case 'T':
                /* Clear unprotected line to spaces from cursor - WY60 */
                debug(F110,"Wyse Escape","Clear unprotected line to space from cursor",0);
                if ( debses )
                    break;
                if ( protect )
                    selclrtoeoln( VTERM, SP ) ;
                else
                    clrtoeoln( VTERM, SP ) ;
                break;
            case 'U':
                /* Turns the monitor submode on */
                debug(F110,"Wyse Escape","Monitor mode on",0);
                if ( debses || wy_monitor )
                    break;
                setdebses(TRUE);
                wy_monitor = TRUE ;
                break;
            case 'V':
                if ( ISWY60(tt_type_mode) ) {
                    /* Clear cursor column - wy60 */
                    viocell cell = {SP,geterasecolor(VTERM)};
                    int ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                    vtattrib vta = attrib;

                    debug(F110,"Wyse Escape","Clear cursor column",0);
                    if ( debses )
                        break;

                    x  = wherex[VTERM]-1 ;
                    vta.unerasable = TRUE ;
                    for ( y=wherey[VTERM]-1 ; y<ys ; y++ )
                        VscrnWrtCell( VTERM, cell, vta, y, x ) ;
                    cursorright(0);
                }
                else {
                    /* Sets a protected column */
                    int ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                    viocell cell = {SP,geterasecolor(VTERM)};
                    vtattrib vta ;

                    debug(F110,"Wyse Escape","Sets a protected column",0);
                    if ( debses )
                        break;

                    x  = wherex[VTERM]-1 ;
                    for ( y=wherey[VTERM]-1 ; y<ys ; y++ ) {
                        vta = VscrnGetVtCharAttr( VTERM, x, y ) ;
                        vta.unerasable = TRUE ;
                        VscrnWrtCell(VTERM, cell,
                                      writeprotect ? WPattrib : vta,
                                      y, x);
                    }
                    cursorright(0);
                }
                break;
            case 'W': {
                /* Deletes a character or attribute */
                vtattrib vta;
                debug(F110,"Wyse Escape","Deletes a char or attribute",0);
                if ( debses )
                    break;
                vta = VscrnGetVtCharAttr( VTERM,
                                          wherex[VTERM]-1,
                                          wherey[VTERM]-1 ) ;
                blankvcell.c = SP;
                blankvcell.a = geterasecolor(VTERM);
                VscrnScrollLf(VTERM, wherey[VTERM] - 1,
                               wherex[VTERM] - 1,
                               wherey[VTERM] - 1,
                               VscrnGetWidth(VTERM) - 1,
                               1, blankvcell) ;
                if ( ( attrmode == ATTR_PAGE_MODE ||
                       attrmode == ATTR_LINE_MODE ) &&
                     vta.wyseattr == TRUE ) {
                    /* BUG */
                    /* If we are in Page or Line modes and the character */
                    /* being deleted is an Attribute, then we need to    */
                    /* correct the attributes for characters to the right*/
                    /* and down. */
                    vta.wyseattr = FALSE;
                    VscrnSetVtCharAttr( VTERM, wherex[VTERM]-1, wherey[VTERM]-1, vta );

                    if ( attrmode == ATTR_LINE_MODE ) {
                        /* Scan backward to beginning of line to find previous */
                        /* cell which is an attribute.  Then reapply it.       */
                        int x;
                        for ( x=wherex[VTERM]-1 ; x >= 1 ; x-- ) {
                            vta = VscrnGetVtCharAttr( VTERM, x-1, wherey[VTERM]-1 ) ;
                            if ( vta.wyseattr ) {
                                ApplyLineAttribute( VTERM, x, wherey[VTERM], vta );
                                return;
                            }
                        }
                        ApplyLineAttribute( VTERM, 1, wherey[VTERM], vta );
                    }
                    else if ( attrmode == ATTR_PAGE_MODE ) {
                        /* Scan backward to beginning of screen to find previous */
                        /* cell which is an attribute.  Then reapply it.         */
                        int x,y;
                                                x = wherex[VTERM]-1;
                        for ( y=wherey[VTERM]-1 ; y >= 1 ; y-- ) {
                            for ( ; x >= 1 ; x-- ) {
                                vta = VscrnGetVtCharAttr( VTERM, x-1, y-1 ) ;
                                if ( vta.wyseattr ) {
                                    ApplyPageAttribute( VTERM, x, y, vta );
                                    return;
                                }
                            }
                            x = VscrnGetWidth( VTERM );
                        }
                        ApplyPageAttribute( VTERM, 1, 1, vta );
                    }
                }
                break;
            }
            case 'X':
                /* Turns the monitor submode off */
                debug(F110,"Wyse Escape","Monitor mode off",0);
                if ( !wy_monitor )
                    break;
                setdebses(FALSE);
                wy_monitor = FALSE ;
                break;
            case 'Y':
                if ( ISWY60(tt_type_mode) ) {
                    /* clear unprotected page to space characters */
                    /* from cursor */
                    debug(F110,"Wyse Escape","Clear unprotected page to SP from cursor",0);
                    if ( debses )
                        break;
                    if ( protect )
                        selclreoscr_escape( VTERM, SP ) ;
                    else
                        clreoscr_escape( VTERM, SP ) ;
                }
                else {
                    /* Erases all characters to the end of the active text */
                    /* segment and replaces them with spaces. */
                    debug(F110,"Wyse Escape","Clear all chars to end of segment with SP",0);
                    if ( debses )
                        break;
                    clreoscr_escape( VTERM, SP ) ;
                }
                /* ??? we really need to figure out what the last */
                /* assigned attribute was so that we know what    */
                /* to assign to these cleared cells               */
                break;
            case 'Z': {
                /* ESC Z dir key sequence DEL =            */
                /*    Program key direction and definition */
                /* ESC Z ~ key =                           */
                /*    Read key direction and definition    */
                char keydef[256] = "" ;
                int dir = wyinc() ;
                int key = wyinc() ;
                int i=0,j=0 ;
                int keyi = -1 ;

                if ( dir == '~' ) {
                    /* Report Key Definition to host */
                    debug(F110,"Wyse Escape","Report Key Definition to Host",0);
                    if ( debses )
                        break;
#ifndef NOSETKEY
                    keydef[i++] = '1' ; /* Direction = remote */
                    keydef[i++] = key ; /* Key */
                    keydef[i] = NUL;    /* So we can use strcat */

                    /* actual definition goes here */
                    if ( key >= '@' && key <= 'O' ) {
                        keyi = key-'@';
                    }
#ifndef NOKVERBS
                    else if ( key >= '`' && key <= 'o' ) {
                        keyi = key-'`'+K_WYSF01-K_WYF01 ;
                    }
                    else switch (key) {
                    case '"':   /* Backspace */
                        keyi = K_WYBS-K_WYF01 ;
                        break;
                    case '}':   /* Clear Line */
                        keyi = K_WYCLRLN-K_WYF01;
                        break;
                    case 'z':   /* Shift Clear Line - Clear Screen */
                        keyi = K_WYSCLRLN-K_WYF01;
                        break;
                        /* Clear Page */
                        /* Shift Clear Page */
                    case '5':   /* Del Char */
                        keyi = K_WYDELCHAR-K_WYF01;
                        break;
                    case '6':   /* Del Line */
                        keyi = K_WYDELLN-K_WYF01;
                        break;
                    case 's':   /* Enter */
                        keyi = K_WYENTER-K_WYF01;
                        break;
                    case SP:    /* Escape */
                        keyi = K_WYESC-K_WYF01;
                        break;
                    case '*':   /* Home */
                        keyi = K_WYHOME-K_WYF01;
                        break;
                    case '/':   /* Shift Home */
                        keyi = K_WYSHOME-K_WYF01;
                        break;
                    case '8':   /* Shift Replace (Insert) */
                        keyi = K_WYINSERT-K_WYF01;
                        break;
                    case 'q':   /* Insert Char */
                        keyi = K_WYINSCHAR-K_WYF01;
                        break;
                    case 'p':   /* Insert Line */
                        keyi = K_WYINSLN-K_WYF01;
                        break;
                    case 'r':   /* Page Next */
                        keyi = K_WYPGNEXT-K_WYF01;
                        break;
                    case 'w':   /* Page Prev */
                        keyi = K_WYPGPREV-K_WYF01;
                        break;
                    case '7':   /* Replace */
                        keyi = K_WYREPLACE-K_WYF01;
                        break;
                    case '$':   /* Return */
                        keyi = K_WYRETURN-K_WYF01;
                        break;
                    case '!':   /* Tab */
                        keyi = K_WYTAB-K_WYF01;
                        break;
                    case '&':   /* Shift Tab */
                        keyi = K_WYSTAB-K_WYF01;
                        break;
                    case 'R':   /* Print Screen */
                        keyi = K_WYPRTSCN-K_WYF01;
                        break;
                    case '%':   /* Shift Esc */
                        keyi = K_WYSESC-K_WYF01;
                        break;
                    case '\'':  /* Shift Backspace */
                        keyi = K_WYSBS-K_WYF01;
                        break;
                    case '4':   /* Shift Enter */
                        keyi = K_WYSENTER-K_WYF01;
                        break;
                    case ')':   /* Shift Return */
                        keyi = K_WYSRETURN-K_WYF01;
                        break;
                    case '+':   /* Up arrow */
                        keyi = K_WYUPARR-K_WYF01;
                        break;
                    case ',':   /* Dn arrow */
                        keyi = K_WYDNARR-K_WYF01;
                        break;
                    case '-':   /* Left arrow */
                        keyi = K_WYLFARR-K_WYF01;
                        break;
                    case '.':   /* Right arrow */
                        keyi = K_WYRTARR-K_WYF01;
                        break;
                    case '0':   /* Shift Up arrow */
                        keyi = K_WYSUPARR-K_WYF01;
                        break;
                    case '1':   /* Shift Dn arrow */
                        keyi = K_WYSDNARR-K_WYF01;
                        break;
                    case '2':   /* Shift Left arrow */
                        keyi = K_WYSLFARR-K_WYF01;
                        break;
                    case '3':   /* Shift Right arrow */
                        keyi = K_WYSRTARR-K_WYF01;
                        break;
                    case 'u':   /* Send */
                        keyi = K_WYSEND-K_WYF01;
                        break;
                    case 't':   /* Shift Send */
                        keyi = K_WYSSEND-K_WYF01;
                        break;
                    case 'Q':   /* F17 */
                        keyi = K_WYF17-K_WYF01;
                        break;
                    case 'W':   /* Shift F17 */
                        keyi = K_WYSF17-K_WYF01;
                        break;
                    case 'S':   /* F18 */
                        keyi = K_WYF18-K_WYF01;
                        break;
                    case 'Y':   /* Shift F18 */
                        keyi = K_WYSF18-K_WYF01;
                        break;
                    case 'T':   /* F19 */
                        keyi = K_WYF19-K_WYF01;
                        break;
                    case 'Z':   /* Shift F19 */
                        keyi = K_WYSF19-K_WYF01;
                        break;
                    case 'P':   /* F20 */
                        keyi = K_WYF20-K_WYF01;
                        break;
                    case 'V':   /* Shift F20 */
                        keyi = K_WYSF20-K_WYF01;
                        break;

                    case '#':   /* Del */
                    case '(':   /* Shift Del */
                    case 'X':   /* Shift Print Screen */
                    case ':':   /* PgUp */
                    case ';':   /* Shift PgUp */
                    case '\\':  /* End */
                    case ']':   /* Shift End */
                    default:
                        debug(F111,"Wyse (ESC Z ~)","key",key);
                        break;
                    }
#endif /* NOKVERBS */
                    if ( keyi >= 0 ) {
                        if ( udkfkeys[keyi] != NULL ) {
                            strcat( keydef, udkfkeys[keyi] );
                            i += strlen(udkfkeys[keyi]);
                        }
                    }

                    keydef[i++] = DEL ; /* End of report */
                    keydef[i] = NUL ;   /* For debugging */
                    sendchars(keydef,i);
#endif /* NOSETKEY */
                    break;
                }
                else {
                    debug(F110,"Wyse Escape","Assign Key Definition",0);
                    i = 0 ;
                    while ( (keydef[i]=wyinc()) != DEL )
                        i++;
                    keydef[i] = NUL ;

                    if ( debses )
                        break;
                    if ( key >= '@' && key <= 'O' ) {
                        keyi = key-'@';
                    }
#ifndef NOKVERBS
                    else if ( key >= '`' && key <= 'o' ) {
                        keyi = key-'`'+(K_WYSF01-K_WYF01);
                    }
                    else switch (key) {
                    case '"':   /* Backspace */
                        keyi = K_WYBS-K_WYF01 ;
                        break;
                    case '}':   /* Clear Line */
                        keyi = K_WYCLRLN-K_WYF01;
                        break;
                    case 'z':   /* Shift Clear Line - Clear Screen */
                        keyi = K_WYSCLRLN-K_WYF01;
                        break;
                        /* Clear Page */
                        /* Shift Clear Page */
                    case '5':   /* Del Char */
                        keyi = K_WYDELCHAR-K_WYF01;
                        break;
                    case '6':   /* Del Line */
                        keyi = K_WYDELLN-K_WYF01;
                        break;
                    case 's':   /* Enter */
                        keyi = K_WYENTER-K_WYF01;
                        break;
                    case SP:    /* Escape */
                        keyi = K_WYESC-K_WYF01;
                        break;
                    case '*':   /* Home */
                        keyi = K_WYHOME-K_WYF01;
                        break;
                    case '/':   /* Shift Home */
                        keyi = K_WYSHOME-K_WYF01;
                        break;
                    case '8':   /* Shift Replace (Insert) */
                        keyi = K_WYINSERT-K_WYF01;
                        break;
                    case 'q':   /* Insert Char */
                        keyi = K_WYINSCHAR-K_WYF01;
                        break;
                    case 'p':   /* Insert Line */
                        keyi = K_WYINSLN-K_WYF01;
                        break;
                    case 'r':   /* Page Next */
                        keyi = K_WYPGNEXT-K_WYF01;
                        break;
                    case 'w':   /* Page Prev */
                        keyi = K_WYPGPREV-K_WYF01;
                        break;
                    case '7':   /* Replace */
                        keyi = K_WYREPLACE-K_WYF01;
                        break;
                    case '$':   /* Return */
                        keyi = K_WYRETURN-K_WYF01;
                        break;
                    case '!':   /* Tab */
                        keyi = K_WYTAB-K_WYF01;
                        break;
                    case '&':   /* Shift Tab */
                        keyi = K_WYSTAB-K_WYF01;
                        break;
                    case 'R':   /* Print Screen */
                        keyi = K_WYPRTSCN-K_WYF01;
                        break;
                    case '%':   /* Shift Esc */
                        keyi = K_WYSESC-K_WYF01;
                        break;
                    case '\'':  /* Shift Backspace */
                        keyi = K_WYSBS-K_WYF01;
                        break;
                    case '4':   /* Shift Enter */
                        keyi = K_WYSENTER-K_WYF01;
                        break;
                    case ')':   /* Shift Return */
                        keyi = K_WYSRETURN-K_WYF01;
                        break;
                    case '+':   /* Up arrow */
                        keyi = K_WYUPARR-K_WYF01;
                        break;
                    case ',':   /* Dn arrow */
                        keyi = K_WYDNARR-K_WYF01;
                        break;
                    case '-':   /* Left arrow */
                        keyi = K_WYLFARR-K_WYF01;
                        break;
                    case '.':   /* Right arrow */
                        keyi = K_WYRTARR-K_WYF01;
                        break;
                    case '0':   /* Shift Up arrow */
                        keyi = K_WYSUPARR-K_WYF01;
                        break;
                    case '1':   /* Shift Dn arrow */
                        keyi = K_WYSDNARR-K_WYF01;
                        break;
                    case '2':   /* Shift Left arrow */
                        keyi = K_WYSLFARR-K_WYF01;
                        break;
                    case '3':   /* Shift Right arrow */
                        keyi = K_WYSRTARR-K_WYF01;
                        break;
                    case 'Q':   /* F17 */
                        keyi = K_WYF17-K_WYF01;
                        break;
                    case 'W':   /* Shift F17 */
                        keyi = K_WYSF17-K_WYF01;
                        break;
                    case 'S':   /* F18 */
                        keyi = K_WYF18-K_WYF01;
                        break;
                    case 'Y':   /* Shift F18 */
                        keyi = K_WYSF18-K_WYF01;
                        break;
                    case 'T':   /* F19 */
                        keyi = K_WYF19-K_WYF01;
                        break;
                    case 'Z':   /* Shift F19 */
                        keyi = K_WYSF19-K_WYF01;
                        break;
                    case 'P':   /* F20 */
                        keyi = K_WYF20-K_WYF01;
                        break;
                    case 'V':   /* Shift F20 */
                        keyi = K_WYSF20-K_WYF01;
                        break;
                    case 'u':   /* Send */
                        keyi = K_WYSEND-K_WYF01;
                        break;
                    case 't':   /* Shift Send */
                        keyi = K_WYSSEND-K_WYF01;
                        break;

                    case '#':   /* Del */
                    case '(':   /* Shift Del */
                    case 'X':   /* Shift Print Screen */
                    case ':':   /* PgUp */
                    case ';':   /* Shift PgUp */
                    case '\\':  /* End */
                    case ']':   /* Shift End */
                    default:
                        debug(F111,"Wyse (ESC Z)","unimplemented key",key);
                        break;
                    }
#endif /* NOKVERBS */
#ifndef NOSETKEY
                    if ( keyi >= 0 ) {
                        if ( udkfkeys[keyi] ){
                            free( udkfkeys[keyi] ) ;
                            udkfkeys[keyi]=NULL;
                        }
                        if ( strlen(keydef) ) {
                            char buf[80];
                            udkfkeys[keyi] = strdup( keydef ) ;
                            sprintf(buf,"Wyse [Define Key]=%d",keyi);
                            ckhexdump(buf,keydef,strlen(keydef));
                        } else {
                            udkfkeys[keyi] = wysedefkey( keyi );
                        }
                    }
#endif /* NOSETKEY */
                }
                break;
            }
            case '[': {
                /* Address cursor to specific row */
                int line = wyinc();

                debug(F110,"Wyse Escape","Address cursor to row",0);
                if ( debses )
                    break;
                lgotoxy(VTERM,wherex[VTERM],line-31);
                break;
            }
            case '\\':
                debug(F110,"Wyse Escape","\\",0);
                break;
            case ']':
                /* Activates text segment 0 - Upper Window */
                debug(F110,"Wyse Escape","Activate Text Segment 0",0);
                break;
            case '^':
                /* ESC ^ 0 - Restore normal screen */
                /* ESC ^ 1 - Reverse screen */
                {
                    int n = wyinc() ;
                    if ( debses )
                        break;
                    switch ( n ) {
                    case '0':
                        /* Restore normal screen */
                        debug(F110,"Wyse Escape","Restore Normal screen",0);
                        if (!decscnm) /* Already normal? */
                            break;
                        else {
                            flipscreen(VTERM);
                            VscrnIsDirty(VTERM);
                        }
                        break;
                    case '1':
                        /* Reverse screen */
                        debug(F110,"Wyse Escape","Reverse screen",0);
                        if (decscnm) /* Already reverse */
                            break;
                        else {
                            flipscreen(VTERM);
                            VscrnIsDirty(VTERM) ;
                        }
                        break;
                    }
                }
                break;
            case '_': {
                /* Address cursor to column */
                int col = wyinc() ;

                debug(F110,"Wyse Escape","Address cursor to col",0);
                if ( debses )
                    break;
                lgotoxy(VTERM,col-31,wherey[VTERM]);
                break;
            }
            case '`': {
                /* Sets the screen features.  This mode has the following */
                /* multiple code sequence:  ESC ' n  where:               */
                /*   n = screen feature code                              */
                int n = wyinc() ;
                switch ( n ) {
                case '0':
                    /* Cursor display off */
                    debug(F110,"Wyse Escape","Cursor display off",0);
                    if ( debses )
                        break;
                    cursorena[VTERM] = FALSE ;
                    break;
                case '1':
                    /* Cursor display on (default) */
                    debug(F110,"Wyse Escape","Cursor display on",0);
                    if ( debses )
                        break;
                    cursorena[VTERM] = TRUE ;
                    break;
                case '2':
                    /* Steady block cursor */
                    debug(F110,"Wyse Escape","Steady block cursor",0);
                    if ( debses )
                        break;
                    tt_cursor = TTC_BLOCK ;
                    setcursormode() ;
                    cursorena[VTERM] = TRUE ;
                    break;
                case '3':
                    /* Blinking line cursor */
                    debug(F110,"Wyse Escape","Blinking line cursor",0);
                    if ( debses )
                        break;
                    tt_cursor = TTC_ULINE ;
                    setcursormode() ;
                    cursorena[VTERM] = TRUE ;
                    break;
                case '4':
                    /* Steady line cursor */
                    debug(F110,"Wyse Escape","Steady line cursor",0);
                    if ( debses )
                        break;
                    tt_cursor = TTC_ULINE ;
                    setcursormode() ;
                    cursorena[VTERM] = TRUE ;
                    break;
                case '5':
                    /* Blinking block cursor (default) */
                    debug(F110,"Wyse Escape","Blinking block cursor",0);
                    if ( debses )
                        break;
                    tt_cursor = TTC_BLOCK ;
                    setcursormode() ;
                    cursorena[VTERM] = TRUE ;
                    break;

                    /* These attributes are used only in Write-Protect Mode */
                case '6':
                    /* Reverse protect character */
                    debug(F110,"Wyse Escape","Write Protect Attribute: Reverse",0);
                    if ( debses )
                        break;
                    WPattrib.blinking = FALSE;          /* No blink */
                    WPattrib.bold = FALSE;              /* No bold */
                    WPattrib.invisible = FALSE;         /* Visible */
                    WPattrib.underlined = FALSE;        /* No underline */
                    WPattrib.reversed = TRUE;           /* Reverse video */
                    WPattrib.unerasable = TRUE;         /* Write-Protected */
                    WPattrib.graphic = FALSE ;          /* Not graphic character */
                    WPattrib.dim = FALSE ;              /* No dim */
                    break;
                case '7':
                    /* Dim protect character (default) */
                    debug(F110,"Wyse Escape","Write Protect Attribute: Dim",0);
                    if ( debses )
                        break;
                    WPattrib.blinking = FALSE;          /* No blink */
                    WPattrib.bold = FALSE;              /* No bold */
                    WPattrib.invisible = FALSE;         /* Visible */
                    WPattrib.underlined = FALSE;        /* No underline */
                    WPattrib.reversed = FALSE;          /* No reverse video */
                    WPattrib.unerasable = TRUE;         /* Write-Protected */
                    WPattrib.graphic = FALSE ;          /* Not graphic character */
                    WPattrib.dim = TRUE ;               /* Dim */
                    break;
                case 'A':
                    /* Normal protect character */
                    debug(F110,"Wyse Escape","Write Protect Attribute: Normal",0);
                    if ( debses )
                        break;
                    WPattrib.blinking = FALSE;          /* No blink */
                    WPattrib.bold = FALSE;              /* No bold */
                    WPattrib.invisible = FALSE;         /* Visible */
                    WPattrib.underlined = FALSE;        /* No underline */
                    WPattrib.reversed = FALSE;          /* No reverse video */
                    WPattrib.unerasable = TRUE;         /* Write-Protected */
                    WPattrib.graphic = FALSE ;          /* Not graphic character */
                    WPattrib.dim = FALSE ;              /* No dim */
                    break;
                case 'B':
                    if ( ISWY60(tt_type_mode) ) {
                        /* Blink on */
                        debug(F110,"Wyse Escape","Write Protect Attribute: Blink",0);
                        if ( debses )
                            break;
                        WPattrib.blinking = TRUE ;
                    }
                    else if ( ISWY30( tt_type_mode ) ) {
                        /* Reverse and Dim */
                        debug(F110,"Wyse Escape","Write Protect Attribute: Reverse and Dim",0);
                        if ( debses )
                            break;
                        WPattrib.blinking = FALSE;      /* No blink */
                        WPattrib.bold = FALSE;          /* No bold */
                        WPattrib.invisible = FALSE;     /* Visible */
                        WPattrib.underlined = FALSE;    /* No underline */
                        WPattrib.reversed = TRUE;       /* reverse video */
                        WPattrib.unerasable = TRUE;     /* Write-Protected */
                        WPattrib.graphic = FALSE ;      /* Not graphic character */
                        WPattrib.dim = TRUE ;           /* dim */
                    }
                    break;
                case 'C':
                    if ( ISWY60(tt_type_mode) ) {
                        /* Invisible on */
                        debug(F110,"Wyse Escape","Write Protect Attribute: Invisible",0);
                        if ( debses )
                            break;
                        WPattrib.invisible = TRUE ;
                    }
                    else if ( ISWY30( tt_type_mode ) ) {
                        /* Double-wide and normal */
                        debug(F110,"Wyse Escape","Write Protect Attribute: Double Wide and Normal",0);
                        if ( debses )
                            break;
                        WPattrib.blinking = FALSE;      /* No blink */
                        WPattrib.bold = FALSE;          /* No bold */
                        WPattrib.invisible = FALSE;     /* Visible */
                        WPattrib.underlined = FALSE;    /* No underline */
                        WPattrib.reversed = FALSE;      /* No reverse video */
                        WPattrib.unerasable = TRUE;     /* Write-Protected */
                        WPattrib.graphic = FALSE ;      /* Not graphic character */
                        WPattrib.dim = FALSE ;          /* No dim */
                    }
                    break;
                case 'D':
                    if ( ISWY30( tt_type_mode ) ) {
                        /* Double-wide and reverse */
                        debug(F110,"Wyse Escape","Write Protect Attribute: Double Wide and Reverse",0);
                        if ( debses )
                            break;
                        WPattrib.blinking = FALSE;      /* No blink */
                        WPattrib.bold = FALSE;          /* No bold */
                        WPattrib.invisible = FALSE;     /* Visible */
                        WPattrib.underlined = FALSE;    /* No underline */
                        WPattrib.reversed = TRUE;       /* reverse video */
                        WPattrib.unerasable = TRUE;     /* Write-Protected */
                        WPattrib.graphic = FALSE ;      /* Not graphic character */
                        WPattrib.dim = FALSE ;          /* No dim */
                    }
                    break;
                case 'E':
                    if ( ISWY60(tt_type_mode) ) {
                        /* Underline on */
                        debug(F110,"Wyse Escape","Write Protect Attribute: Underline",0);
                        if ( debses )
                            break;
                        WPattrib.underlined = TRUE ;
                    }
                    else if ( ISWY30( tt_type_mode ) ) {
                        /* Double-wide and dim */
                        debug(F110,"Wyse Escape","Write Protect Attribute: Double Wide and Dim",0);
                        if ( debses )
                            break;
                        WPattrib.blinking = FALSE;      /* No blink */
                        WPattrib.bold = FALSE;          /* No bold */
                        WPattrib.invisible = FALSE;     /* Visible */
                        WPattrib.underlined = FALSE;    /* No underline */
                        WPattrib.reversed = FALSE;      /* No reverse video */
                        WPattrib.unerasable = TRUE;     /* Write-Protected */
                        WPattrib.graphic = FALSE ;      /* Not graphic character */
                        WPattrib.dim = TRUE ;           /* dim */
                    }
                    break;
                case 'F':
                    if ( ISWY60(tt_type_mode) ) {
                        /* Reverse on */
                        debug(F110,"Wyse Escape","Write Protect Attribute: Reverse",0);
                        if ( debses )
                            break;
                        WPattrib.reversed = TRUE ;
                    }
                    else if ( ISWY30( tt_type_mode ) ) {
                        /* Double-wide, reverse, and dim */
                        debug(F110,"Wyse Escape","Write Protect Attribute: Double Wide, Reverse and Dim",0);
                        if ( debses )
                            break;
                        WPattrib.blinking = FALSE;      /* No blink */
                        WPattrib.bold = FALSE;          /* No bold */
                        WPattrib.invisible = FALSE;     /* Visible */
                        WPattrib.underlined = FALSE;    /* No underline */
                        WPattrib.reversed = TRUE;       /* reverse video */
                        WPattrib.unerasable = TRUE;     /* Write-Protected */
                        WPattrib.graphic = FALSE ;      /* Not graphic character */
                        WPattrib.dim = TRUE ;           /* dim */
                    }
                    break;
                case 'G':
                    if ( ISWY60( tt_type_mode ) ) {
                        /* Dim on */
                        debug(F110,"Wyse Escape","Write Protect Attribute: Dim",0);
                        if ( debses )
                            break;
                        WPattrib.blinking = FALSE;      /* No blink */
                        WPattrib.bold = FALSE;          /* No bold */
                        WPattrib.invisible = FALSE;     /* Visible */
                        WPattrib.underlined = FALSE;    /* No underline */
                        WPattrib.reversed = FALSE;      /* No reverse video */
                        WPattrib.unerasable = TRUE;     /* Write-Protected */
                        WPattrib.graphic = FALSE ;      /* Not graphic character */
                        WPattrib.dim = TRUE ;           /* dim */
                    }
                    break;
                case 'H':
                    if ( ISWY60( tt_type_mode ) ) {
                        /* Line lock mode on */
                        debug(F110,"Wyse Escape","Line lock mode on",0);
                        if ( debses )
                            break;
                        linelock = TRUE ;
                    }
                    break;
                case 'I':
                    if ( ISWY60( tt_type_mode ) ) {
                        /* Line lock mode off */
                        debug(F110,"Wyse Escape","Line lock mode off",0);
                        if ( debses )
                            break;
                        linelock = FALSE ;
                    }
                    break;
                case 'J':
                    if ( ISWY60( tt_type_mode ) ) {
                        /* Set margin bell at cursor position */
                        debug(F110,"Wyse Escape","Set margin bell cursor position",0);
                        if ( debses )
                            break;
                        marginbellcol = wherex[VTERM] ;
                        marginbell = TRUE ;
                    }
                    else if ( ISWY30( tt_type_mode ) ) {
                        /* Underline */
                        debug(F110,"Wyse Escape","Write Protect Attribute: Underline",0);
                        if ( debses )
                            break;
                        WPattrib.blinking = FALSE;      /* No blink */
                        WPattrib.bold = FALSE;          /* No bold */
                        WPattrib.invisible = FALSE;     /* Visible */
                        WPattrib.underlined = TRUE;     /* underline */
                        WPattrib.reversed = TRUE;       /* reverse video */
                        WPattrib.unerasable = TRUE;     /* Write-Protected */
                        WPattrib.graphic = FALSE ;      /* Not graphic character */
                        WPattrib.dim = FALSE ;          /* No dim */
                    }
                    break;
                case 'K':
                    if ( ISWY30( tt_type_mode ) ) {
                        /* Underline and Dim */
                        debug(F110,"Wyse Escape","Write Protect Attribute: Underline and Dim",0);
                        if ( debses )
                            break;
                        WPattrib.blinking = FALSE;      /* No blink */
                        WPattrib.bold = FALSE;          /* No bold */
                        WPattrib.invisible = FALSE;     /* Visible */
                        WPattrib.underlined = TRUE;     /* underline */
                        WPattrib.reversed = FALSE;      /* No reverse video */
                        WPattrib.unerasable = TRUE;     /* Write-Protected */
                        WPattrib.graphic = FALSE ;      /* Not graphic character */
                        WPattrib.dim = TRUE;            /* dim */
                    }
                    break;
                case 'L':
                    if ( ISWY30( tt_type_mode ) ) {
                        /* Underline and Double-wide */
                        debug(F110,"Wyse Escape","Write Protect Attribute: Double-wide and Underline",0);
                        if ( debses )
                            break;
                        WPattrib.blinking = FALSE;      /* No blink */
                        WPattrib.bold = FALSE;          /* No bold */
                        WPattrib.invisible = FALSE;     /* Visible */
                        WPattrib.underlined = TRUE;     /* underline */
                        WPattrib.reversed = FALSE;      /* No reverse video */
                        WPattrib.unerasable = TRUE;     /* Write-Protected */
                        WPattrib.graphic = FALSE ;      /* Not graphic character */
                        WPattrib.dim = FALSE ;          /* No dim */
                    }
                    break;
                case '8':
                    /* Screen display off */
                    debug(F110,"Wyse Escape","Screen display off",0);
                    if ( debses )
                        break;
                    screenon = FALSE ;
                    break;
                case '9':
                    /* Screen display on (default) */
                    debug(F110,"Wyse Escape","Screen display on",0);
                    if ( debses )
                        break;
                    screenon = TRUE ;
                    break;

                case ':':
                    /* 80-column mode */
                    debug(F110,"Wyse Escape","80-column mode",0);
                    if ( debses )
                        break;
                    RequestScreenMutex(SEM_INDEFINITE_WAIT);
                    killcursor(VTERM);
                    Set80Cols(VTERM);
#ifdef TCPSOCKET
#ifdef CK_NAWS
                    if (ttmdm < 0 && TELOPT_ME(TELOPT_NAWS)) {
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
                    ipadl25();
                    ReleaseScreenMutex();
                    break;

                case ';':
                    /* 132-column mode (default) */
                    debug(F110,"Wyse Escape","132-column mode",0);
                    if ( debses )
                        break;
                    RequestScreenMutex(SEM_INDEFINITE_WAIT);
                    killcursor(VTERM);
                    Set132Cols(VTERM);
#ifdef TCPSOCKET
#ifdef CK_NAWS
                    if (ttmdm < 0 && TELOPT_ME(TELOPT_NAWS)) {
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
                    ipadl25();
                    ReleaseScreenMutex();
                    break;

                case '<':
                    /* Smooth scroll @ 1 row per second */
                    debug(F110,"Wyse Escape","Smooth scroll @ 1 row/second",0);
                    if ( debses )
                        break;
                    SmoothScroll() ;
                    break;
                case '=':
                    /* Smooth scroll @ 2 rows per second */
                    debug(F110,"Wyse Escape","Smooth scroll @ 2 row/second",0);
                    if ( debses )
                        break;
                    SmoothScroll() ;
                    break;
                case '>':
                    /* Smooth scroll @ 4 rows per second */
                    debug(F110,"Wyse Escape","Smooth scroll @ 4 row/second",0);
                    if ( debses )
                        break;
                    SmoothScroll() ;
                    break;
                case '?':
                    /* Smooth scroll @ 8 rows per second */
                    debug(F110,"Wyse Escape","Smooth scroll @ 8 row/second",0);
                    if ( debses )
                        break;
                    SmoothScroll() ;
                    break;
                case '@':
                    /* Jump scroll (default) */
                    debug(F110,"Wyse Escape","Jump scroll",0);
                    if ( debses )
                        break;
                    JumpScroll() ;
                    break;

                case 'a':
                    /* WY60 - Extended status line on */
                    /* We use the status line for the label line */
                    debug(F110,"Wyse Escape","Extended Status Line On",0);
                    break;
                case 'b':
                    /* WY60 - Standard status line on */
                    debug(F110,"Wyse Escape","Standard Status Line On",0);
                    break;
                case 'c':
                    /* WY60 - Status line display off */
                    debug(F110,"Wyse Escape","Status Line Off",0);
                    break;
                }
                break;
            }
            case 'a': {
                /* Moves the cursor to a specified row and column for an  */
                /* 80- or a 132-column screen.  This has a format of:     */
                /*    ESC a rr R ccc C  where:                            */
                /*   rr  = the ASCII encoded decimal value of the row     */
                /*   R   = ASCII R                                        */
                /*   ccc = the ASCII encoded decimal value of the column  */
                /*   C   = ASCII C                                        */
                /* For example:  ESC a 1 R 1 C positions the cursor at    */
                /* true home.                                             */
                int n, c=0, r=0 ;

                debug(F110,"Wyse Escape","Move cursor to row and column",0);

                n = wyinc();

                while ( isdigit(n) ) {
                    r = r * 10 + n - '0' ;
                    n = wyinc() ;
                }
                if ( n != 'R' ) {
                    escstate = ES_NORMAL ;
                    wyseascii( n ) ;
                    return ;
                }

                n = wyinc();

                while ( isdigit(n) ) {
                    c = c * 10 + n - '0' ;
                    n = wyinc() ;
                }
                if ( n != 'C' ) {
                    escstate = ES_NORMAL ;
                    wyseascii( n ) ;
                    return ;
                }

                if ( debses )
                    break;
                lgotoxy( VTERM, c, r ) ;
                break;
            }
            case 'b': {
                /* Transmits the cursor address to the host computer for  */
                /* the active text segment.  The format is:               */
                /*   rr R ccc C   where:                                  */
                /*   rr  = the ASCII encoded decimal value of the row     */
                /*   R   = ASCII R                                        */
                /*   ccc = the ASCII encoded decimal value of the column  */
                /*   C   = ASCII C                                        */
                char buf[12] ;
                debug(F110,"Wyse Escape","Transmit cursor address to the host",0);

                if ( debses )
                    break;
                sprintf(buf,"%dR%dC",wherey[VTERM],wherex[VTERM]);
                break;
            }
            case 'c':
                if ( ISWY30(tt_type_mode) ) {
                    /* Activate Upper Data Segment */
                    debug(F110,"Wyse Escape","Activate Upper Data Segment",0);
                    if ( debses )
                        break;
                }
                else if ( ISWY60(tt_type_mode) )
                {
                    int n = wyinc() ;

                    switch ( n ) {
                    case '0': {
                        /* Set MODEM port operating parameters */
                        /* baud stop parity word               */
                        int baud, stop, parity, word ;
                        debug(F110,"Wyse Escape","Set Modem Port parameters",0);
                        baud = wyinc();
                        stop = wyinc();
                        parity = wyinc();
                        word = wyinc();
                        break;
                    }
                    case '1': {
                        /* Set AUX port operating parameters */
                        /* baud stop parity word             */
                        int baud, stop, parity, word ;
                        debug(F110,"Wyse Escape","Set AUX port parameters",0);
                        baud = wyinc();
                        stop = wyinc();
                        parity = wyinc();
                        word = wyinc();
                        break;
                    }
                    case '2': {
                        /* Set MODEM port receive handshaking */
                        /* hndshk */
                        int hndshk = wyinc();
                        debug(F110,"Wyse Escape","Set Modem port RX handshaking",0);
                        break;
                    }
                    case '3': {
                        /* Set AUX port receive handshaking */
                        /* hndshk */
                        int hndshk = wyinc();
                        debug(F110,"Wyse Escape","Set AUX port RX handshaking",0);
                        break;
                    }
                    case '4': {
                        /* Set MODEM port transmit handshaking */
                        /* hndshk */
                        int hndshk = wyinc();
                        debug(F110,"Wyse Escape","Set Modem port TX handshaking",0);
                        break;
                    }
                    case '5': {
                        /* Set AUX port transmit handshaking */
                        /* hndshk */
                        int hndshk = wyinc();
                        debug(F110,"Wyse Escape","Set AUX port TX handshaking",0);
                        break;
                    }
                    case '6': {
                        /* Set maximum data transmission speed */
                        /* maxspd */
                        int maxspd = wyinc();
                        debug(F110,"Wyse Escape","Set Max Data TX speed",0);
                        break;
                    }
                    case '7': {
                        /* Set maximum function key transmission speed */
                        /* max */
                        int max = wyinc() ;
                        debug(F110,"Wyse Escape","Set Max Function Key TX speed",0);
                        break;
                    }
                    case '8': {
                        /* Load time of day */
                        /* hh mm */
                        int hh, mm ;
                        debug(F110,"Wyse Escape","Load Time of Day",0);
                        hh = wyinc();
                        mm = wyinc();
                        loadtod( hh, mm ) ;
                        break;
                    }
                    case ';': {
                        /* Program answerback message */
                        /* answer CTRL-Y */
                        int n ;
                        debug(F110,"Wyse Escape","Program Answerback message",0);
                        while ( (n=wyinc()) != 25 )
                            ;
                        if ( debses )
                            break;
                        break;
                    }
                    case '<':
                        /* Send answerback message */
                        debug(F110,"Wyse Escape","Send Answerback message",0);
                        if ( debses )
                            break;
                        if (tt_answer)
                            sendchars(answerback,strlen(answerback)) ;
                        break;
                    case '\\': {
                        /* 325 or 160 - Select Bell Tone */
                        /* 0 off, 1 Low pitch, 2,3 High pitch */
                        int tone = wyinc() ;
                        debug(F110,"Wyse Escape","Select Bell Tone",0);
                        if ( debses )
                            break;
                        break;
                    }
                    case '=':
                        /* Conceal answerback message */
                        debug(F110,"Wyse Escape","Conceal Answerback message",0);
                        break;
                    case '?': {
                        /* Clear font back */
                        /* bank */
                        int bank = wyinc();
                        struct _vtG * pG;

                        debug(F110,"Wyse Escape","Clear font bank",0);

                        if ( debses )
                            break;

                        pG = &G[bank];
                        pG->designation = TX_UNDEF;
                        pG->c1 = FALSE ;
                        pG->national = FALSE;
                        pG->rtoi = NULL ;
                        pG->itol = NULL ;
                        pG->itor = NULL ;
                        pG->ltoi = NULL ;
                        break;
                    }
                    case '@': {
                        /* Load font bank */
                        /* bank set */
                        int bank, set ;
                        unsigned char cs = TX_UNDEF ;
                        struct _vtG * pG;
                        debug(F110,"Wyse Escape","Load font bank",0);
                        bank = wyinc();
                        set = wyinc() ;

                        /* bank is 0 (G0) to 3 (G3) */
                        /* Set is:
                           @ - Native mode
                           A - Multinational (CP437)
                           B - Standard ASCII
                           C - Graphics 1
                           D - PC Equivalent
                           E - Graphics 2
                           F - Graphics 3
                           G - Standard ANSI
                           ` - 44-line native
                           a - 44-line multinational
                           b - 44-line PC
                           c - 44-line Standard ASCII
                           d - 44-line Standard ANSI
                        */
                        if ( debses )
                            break;

                        pG = &G[bank];
                        switch ( set ) {
                        case '@':
                        case 'a':
                            cs = TX_WYSE60G_N;
                            break;
                        case 'A':
                            cs = TX_CP437;
                            break;
                        case 'B':
                        case 'c':
                            cs = TX_ASCII;
                            break;
                        case 'C':
                            cs = TX_WYSE60G_1;
                            break;
                        case 'D':
                        case 'b':
                            cs = TX_IBMC0GRPH;
                            break;
                        case 'E':
                            cs = TX_WYSE60G_2;
                            break;
                        case 'F':
                            cs = TX_WYSE60G_3;
                            break;
                        case 'G':
                        case 'd':
                            cs = TX_CP1252; /* This is wrong */
                            break;
                        }

                        if ( cs <= MAXTXSETS ) {
                            pG->designation = cs ;
                            pG->size = cs96;
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
                        break;
                    }
                    case 'A': {
                        /* Define and load character */
                        /* bank pp bb ... bb CTRL-Y  */
                        int bank, pp, bb, cnt = 0 ;
                        debug(F110,"Wyse Escape","Define and load character",0);
                        bank = wyinc() ;
                        pp = wyinc() ;
                        do {
                            bb = wyinc() ;
                            cnt++;
                        } while ( cnt < 32 && bb != XEM );
                        break;
                    }
                    case 'B': {
                        /* Define primary character set */
                        /* bank */
                        int bank = wyinc() ;
                        debug(F110,"Wyse Escape","Define primary char set",0);
                        /* This should point GL at a bank */
                        if ( debses )
                            break;
                        if ( bank >= '0' && bank <= '3' )
                            GL = &G[bank-'0'];
                        break;
                    }
                    case 'C': {
                        /* Define secondary character set */
                        /* bank */
                        int bank = wyinc() ;
                        debug(F110,"Wyse Escape","Define secondary char set",0);
                        /* This should point GR at a bank */
                        if ( debses )
                            break;
                        if ( bank >= '0' && bank <= '3' )
                            GR = &G[bank-'0'];
                        break;
                    }
                    case 'D':
                        /* Select primary character set */
                        debug(F110,"Wyse Escape","Select primary char set",0);
                        if ( debses )
                            break;
                        wyse8bit = FALSE;
                        break;
                    case 'E':
                        /* Select secondary character set */
                        debug(F110,"Wyse Escape","Select secondary char set",0);
                        if ( debses )
                            break;
                        wyse8bit = TRUE;
                        break;
                    case 'F': {
                        /* Clear unprotected rectangle in 80-col page */
                        /* line col char */
                        /* Clear unprotected rectangle in 132-col page */
                        /* line ~ col char */
                        /* Is ignored if a line is locked */
                        int line, col, fillchar ;
                        debug(F110,"Wyse Escape","Clear unprotected rectangle",0);

                        line = wyinc() ;

                        col = wyinc();

                        if ( col == '~' ) {
                            col = wyinc() ;
                        }

                        fillchar = wyinc() ;

                        if ( debses )
                            break;
                        if ( protect )
                            selclrrect_escape( VTERM, wherey[VTERM], wherex[VTERM],
                                               line-31, col-31, fillchar ) ;
                        else
                            clrrect_escape( VTERM, wherey[VTERM], wherex[VTERM],
                                            line-31, col-31, fillchar ) ;
                        break;
                    }
                    case 'G': {
                        /* Box rectangle in 80-col page */
                        /* line col */
                        /* Box rectangle in 132-col page */
                        /* line ~ col */
                        /* Is ignored if a line is locked */
                        int line, col ;
                        debug(F110,"Wyse Escape","Box rectangle",0);
                        line = wyinc() ;

                        col = wyinc();

                        if ( col == '~' ) {
                            col = wyinc() ;
                        }

                        if ( debses )
                            break;
                        boxrect_escape( VTERM, line-31, col-31 ) ;
                        break;
                    }
                    case 'H': {
                        /* Clear entire rectangle in 80-col page */
                        /* line col char */
                        /* Clear entire rectangle in 132-col page */
                        /* line ~ col char */
                        /* Is ignored if a line is locked */
                        int line, col, fillchar ;
                        debug(F110,"Wyse Escape","Clear entire rectangle",0);
                        line = wyinc() ;

                        col = wyinc();

                        if ( col == '~' ) {
                            col = wyinc() ;
                        }

                        fillchar = wyinc() ;

                        if ( debses )
                            break;
                        clrrect_escape( VTERM, wherey[VTERM], wherex[VTERM],
                                        line-31, col-31, fillchar ) ;
                        break;
                    }
                    case 'I':
                        /* Clear unprotected col to specific character */
                        /* char */
                        {
                            int n = wyinc();
                            debug(F110,"Wyse Escape","Clear unprotected col to char",0);
                            if ( debses )
                                break;
                            if ( protect )
                                selclrcol_escape( VTERM, n ) ;
                            else
                                clrcol_escape( VTERM, n ) ;
                        }
                        break;
                    case 'J':
                        /* Delete cursor column */
                        if ( debses )
                            break;
                        blankvcell.c = SP ;
                        blankvcell.a = geterasecolor(VTERM) ;
                        VscrnScrollRt( VTERM,
                                       0, wherex[VTERM]-1,
                                       VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0),
                                       VscrnGetWidth(VTERM)-1,
                                       1, blankvcell ) ;
                        break;
                    case 'K':
                        /* Clear unprotected column to nulls */
                        debug(F110,"Wyse Escape","Clear unprotected col to NUL",0);
                        if ( debses )
                            break;
                        if (protect)
                            selclrcol_escape( VTERM, NUL ) ;
                        else
                            clrcol_escape( VTERM, NUL ) ;
                        break;
                    case 'L':
                        /* Clear unprotected to end of line with nulls */
                        debug(F110,"Wyse Escape","Clear unprotected to EOL to NUL",0);
                        if ( debses )
                            break;
                        if (protect)
                            selclrtoeoln( VTERM, NUL ) ;
                        else
                            clrtoeoln( VTERM, NUL ) ;
                        break;
                    case 'M':
                        /* Insert column of nulls */
                        debug(F110,"Wyse Escape","Insert col of NUL",0);
                        if ( debses )
                            break;
                        blankvcell.c = SP ;
                        blankvcell.a = geterasecolor(VTERM) ;
                        VscrnScrollRt( VTERM,
                                       0, wherex[VTERM]-1,
                                       VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0),
                                       VscrnGetWidth(VTERM)-1,
                                       1, blankvcell ) ;
                        break;
                    case 'N': {
                        /* Box rectangle to right of cursor */
                        /* width height */
                        int w, h ;
                        debug(F110,"Wyse Escape","Box rectangle to right of cursor",0);
                        w = wyinc() ;

                        h = wyinc() ;

                        if ( debses )
                            break;
                        boxrect_escape( VTERM, wherey[VTERM]+h-31,
                                        wherex[VTERM]+w-31 ) ;
                        break;
                    }
                    case 'O':
                        /* Clear unprotected to end of line with spaces */
                        debug(F110,"Wyse Escape","Clear unprotected to EOL to SP",0);
                        if ( debses )
                            break;
                        if ( protect )
                            selclrtoeoln( VTERM, SP ) ;
                        else
                            clrtoeoln( VTERM, SP ) ;
                        break;
                    case 'P':
                        /* Clear unprotected page foreground to spaces */
                        debug(F110,"Wyse Escape","Clear unprotected page foreground to SP",0);
                        if ( debses )
                            break;
                        if ( protect )
                            selclreoscr_escape( VTERM, SP ) ;
                        else
                            clreoscr_escape( VTERM, SP ) ;
                        /* ??? we really need to figure out what the last */
                        /* assigned attribute was so that we know what    */
                        /* to assign to these cleared cells               */
                        break;
                    case 'Q':
                        /* Clear unprotected page foreground to nulls */
                        debug(F110,"Wyse Escape","Clear unprotected page foreground to NUL",0);
                        if ( debses )
                            break;
                        if ( protect )
                            selclreoscr_escape( VTERM, NUL ) ;
                        else
                            clreoscr_escape( VTERM, NUL ) ;
                        /* ??? we really need to figure out what the last */
                        /* assigned attribute was so that we know what    */
                        /* to assign to these cleared cells               */
                        break;
                    case 'R':
                        /* Clear unprotected line foreground to spaces */
                        debug(F110,"Wyse Escape","Clear unprotected line foreground to SP",0);
                        if ( debses )
                            break;
                        if ( protect )
                            selclrtoeoln( VTERM, SP ) ;
                        else
                            clrtoeoln( VTERM, SP ) ;
                        break;
                    case 'S':
                        /* Clear unprotected line foreground to nulls */
                        debug(F110,"Wyse Escape","Clear unprotected line foreground to NUL",0);
                        if ( debses )
                            break;
                        if ( protect )
                            selclrtoeoln( VTERM, NUL ) ;
                        else
                            clrtoeoln( VTERM, NUL ) ;
                        break;
                    case 'T':
                        debug(F110,"Wyse Escape","cT",0);
                        break;
                    case 'U':
                        /* Clear all programmable keys */
                        debug(F110,"Wyse Escape","Clear all programmable keys",0);
                        if ( debses )
                            break;
                        udkreset() ;
                        break;
                    }
                }
                break;
            case 'd':
                if ( ISWY30(tt_type_mode ) ) {
                    /* Moves the cursor to a specified row and column for an  */
                    /* 80- or a 132-column screen.  This has a format of:     */
                    /*    ESC a rr R ccc C  where:                            */
                    /*   rr  = the ASCII encoded decimal value of the row     */
                    /*   R   = ASCII R                                        */
                    /*   ccc = the ASCII encoded decimal value of the column  */
                    /*   C   = ASCII C                                        */
                    /* For example:  ESC a 1 R 1 C positions the cursor at    */
                    /* true home.                                             */
                    int n, c=0, r=0 ;

                    debug(F110,"Wyse Escape","Move cursor to row and column",0);
                    n = wyinc();

                    while ( isdigit(n) ) {
                        r = r * 10 + n - '0' ;
                        n = wyinc() ;
                    }
                    if ( n != 'R' ) {
                        escstate = ES_NORMAL ;
                        wyseascii( n ) ;
                        return ;
                    }

                    n = wyinc();

                    while ( isdigit(n) ) {
                        c = c * 10 + n - '0' ;
                        n = wyinc() ;
                    }
                    if ( n != 'C' ) {
                        escstate = ES_NORMAL ;
                        wyseascii( n ) ;
                        return ;
                    }

                    if ( debses )
                        break;
                    lgotoxy( VTERM, c, r ) ;
                }
                else if ( ISWY60(tt_type_mode) )
                {
                    int n = wyinc();

                    switch ( n ) {
                    case SP:
                        /* Secondary receive mode off */
                        debug(F110,"Wyse Escape","Secondary RX mode off",0);
                        if ( debses )
                            break;
                        break;
                    case '!':
                        /* Secondary receive mode on */
                        debug(F110,"Wyse Escape","Secondary RX mode on",0);
                        if ( debses )
                            break;
                        break;
                    case '#':
                        /* Transparent print on */
                        debug(F110,"Wyse Escape","Transparent Print on",0);
                        if ( debses )
                            break;
                        xprint = TRUE ;
                        printeron();
                        break;
                    case '$':
                        /* Bidirectional mode off */
                        debug(F110,"Wyse Escape","Bidirectional mode off",0);
                        if ( debses )
                            break;
                        cprint = FALSE;
                        printeroff();
                        break;
                    case '%':
                        /* Bidirectional mode on */
                        debug(F110,"Wyse Escape","Bidirectional mode on",0);
                        if ( debses )
                            break;
                        cprint = TRUE;
                        printeron();
                        break;
                    case '&':
                        /* Begin print/send at top of screen */
                        debug(F110,"Wyse Escape","Begin print/send at top of screen",0);
                        if ( debses )
                            break;
                        break;
                    case '\'':
                        /* Begin print/send at top of page */
                        debug(F110,"Wyse Escape","Begin print/send at top of page",0);
                        if ( debses )
                            break;
                        break;
                    case '*':
                        /* Autopage mode off */
                        debug(F110,"Wyse Escape","Autopage mode off",0);
                        if ( debses )
                            break;
                        wy_autopage = FALSE ;
                        break;
                    case '+':
                        /* Autopage mode on */
                        debug(F110,"Wyse Escape","Autopage mode on",0);
                        if ( debses )
                            break;
                        wy_autopage = TRUE ;
                        break;
                    case '.':
                        /* End-of-line wrap mode off */
                        debug(F110,"Wyse Escape","EOL wrap mode off",0);
                        if ( debses )
                            break;
                        tt_wrap = FALSE ;
                        break;
                    case '/':
                        /* End-of-line wrap mode on */
                        debug(F110,"Wyse Escape","EOL wrap mode on",0);
                        if ( debses )
                            break;
                        tt_wrap = TRUE ;
                        break;
                    case '|':
                        /* Map the reverse attribute (default) - WY160 */
                        debug(F110,"Wyse Escape","Map the reverse attribute",0);
                        if ( debses )
                            break;
                        break;
                    case '{':
                        /* Map the blank attribute - WY160 */
                        debug(F110,"Wyse Escape","Map the blank attribute",0);
                        if ( debses )
                            break;
                        break;
                    }
                }
                break;
            case 'e':
                if ( ISWY30(tt_type_mode) || ISWY60(tt_type_mode) ) {
                int mode = wyinc() ;

                    switch ( mode ) {
                    case SP:
                        /* Answerback mode off */
                        debug(F110,"Wyse Escape","Answerback mode off",0);
                        if ( debses )
                            break;
                        tt_answer = FALSE ;
                        break;
                    case '!':
                        /* Answerback mode on */
                        debug(F110,"Wyse Escape","Answerback mode on",0);
                        if ( debses )
                            break;
                        tt_answer = TRUE ;
                        break;
                    case '"':
                        /* Page edit mode off */
                        debug(F110,"Wyse Escape","Page edit mode off",0);
                        if ( debses )
                            break;
                        break;
                    case '#':
                        /* Page edit mode on */
                        debug(F110,"Wyse Escape","Page edit mode on",0);
                        if ( debses )
                            break;
                        break;
                    case '$':
                        /* Keyclick on */
                        debug(F110,"Wyse Escape","keyclick on",0);
                        if ( debses )
                            break;
                        setkeyclick(TRUE);
                        break;
                    case '%':
                        /* Keyclick off */
                        debug(F110,"Wyse Escape","keyclick off",0);
                        if ( debses )
                            break;
                        setkeyclick(FALSE);
                        break;
                    case '&':
                        /* CAPSLOCK on */
                        debug(F110,"Wyse Escape","Capslock on",0);
                        if ( debses )
                            break;
                        break;
                    case '\'':
                        /* CAPSLOCK off */
                        debug(F110,"Wyse Escape","Capslock off",0);
                        if ( debses )
                            break;
                        break;
                    case '(':
                        /* Display 24 data lines */
                        debug(F110,"Wyse Escape","24 data lines",0);
                        if ( debses )
                            break;
                        settermstatus(TRUE);
                        break;
                    case ')':
                        /* Display 25 data lines */
                        debug(F110,"Wyse Escape","25 data lines",0);
                        if ( debses )
                            break;
                        settermstatus(FALSE);
                        break;
                    case '*':
                        /* Display 42 data lines */
                        debug(F110,"Wyse Escape","42 data lines",0);
                        if ( debses )
                            break;
                        settermstatus(TRUE);
                        break;
                    case '+':
                        /* Display 43 data lines */
                        debug(F110,"Wyse Escape","43 data lines",0);
                        if ( debses )
                            break;
                        settermstatus(FALSE);
                        break;
                    case ',':
                        /* Key repeat off */
                        debug(F110,"Wyse Escape","Key repeat off",0);
                        if ( debses )
                            break;
                        break;
                    case '-':
                        /* Key repeat on */
                        debug(F110,"Wyse Escape","Key repeat on",0);
                        if ( debses )
                            break;
                        break;
                    case '.':
                        /* Width change clear mode off */
                        debug(F110,"Wyse Escape","Width change clear mode off",0);
                        if ( debses )
                            break;
                        wy_widthclr = FALSE ;
                        break;
                    case '/':
                        /* Width change clear mode on */
                        debug(F110,"Wyse Escape","Width change clear mode on",0);
                        if ( debses )
                            break;
                        wy_widthclr = TRUE ;
                        break;
                    case '{':
                        /* Dim intensity on */
                        debug(F110,"Wyse Escape","Dim intensity on",0);
                        if ( debses )
                            break;
                        attrib.dim = TRUE ;
                        break;
                    case '|':
                        /* Dim intensity off */
                        debug(F110,"Wyse Escape","Dim intensity off",0);
                        if ( debses )
                            break;
                        attrib.dim = FALSE ;
                        break;
                    case '0':
                        /* Character attribute mode off */
                        debug(F110,"Wyse Escape","Character attribute mode off",0);
                        if ( debses )
                            break;
                        if ( attrmode & ATTR_CHAR_MODE )
                            attrmode ^= ATTR_CHAR_MODE ;
                        break;
                    case '1':
                        /* Character attribute mode on */
                        debug(F110,"Wyse Escape","Character attribute mode on",0);
                        if ( debses )
                            break;
                        if ( attrmode & ATTR_CHAR_MODE )
                            break;
                        attrmode ^= ATTR_CHAR_MODE ;
                        break;
                    case '2':
                        /* Page attribute mode on */
                        debug(F110,"Wyse Escape","Page attribute mode on",0);
                        if ( debses )
                            break;
                        attrmode = ATTR_PAGE_MODE ;
                        break;
                    case '3':
                        /* Line attribute mode on */
                        debug(F110,"Wyse Escape","Line attribute mode on",0);
                        if ( debses )
                            break;
                        attrmode = ATTR_LINE_MODE;
                        break;
                    case '4':
                        /* Received CR mode off */
                        debug(F110,"Wyse Escape","Received CR mode off",0);
                        if ( debses )
                            break;
                        tnlm = FALSE ;
                        break;
                    case '5':
                        /* Received CR mode on */
                        debug(F110,"Wyse Escape","Received CR mode on",0);
                        if ( debses )
                            break;
                        tnlm = TRUE ;
                        break;
                    case '6':
                        /* ACK mode off */
                        debug(F110,"Wyse Escape","ACK mode off",0);
                        if ( debses )
                            break;
                        break;
                    case '7':
                        /* ACK mode on */
                        debug(F110,"Wyse Escape","ACK mode on",0);
                        if ( debses )
                            break;
                        break;
                    case '8':
                        /* Select MODEM port for data communications */
                        debug(F110,"Wyse Escape","Select Modem port for data comm",0);
                        if ( debses )
                            break;
                        break;
                    case '9':
                        /* Select AUX port for data communications */
                        debug(F110,"Wyse Escape","Select AUX port for data comm",0);
                        if ( debses )
                            break;
                        break;
                    case ':':
                        /* Don't initialize tabs */
                        debug(F110,"Wyse Escape","Don't initialize tabs",0);
                        if ( debses )
                            break;
                        break;
                    case ';':
                        /* Initialize tabs */
                        debug(F110,"Wyse Escape","Initialize tabs",0);
                        if ( debses )
                            break;
                        break;
                    case '?':
                        /* Select 10x13 character cell - WY30 */
                        debug(F110,"Wyse Escape","10x3 character cell",0);
                        if ( debses )
                            break;
                        break;
                    case '@':
                        /* 325 or 160 - NumLock off */
                        debug(F110,"Wyse Escape","Numlock off",0);
                        if ( debses )
                            break;
                        break;
                    case 'A':
                        /* 325 or 160 - NumLock on */
                        debug(F110,"Wyse Escape","Numlock on",0);
                        if ( debses )
                            break;
                        break;
                    case 'B':
                        /* 325 or 160 - Scroll Lock off */
                        debug(F110,"Wyse Escape","Scroll Lock off",0);
                        if ( debses )
                            break;
                        break;
                    case 'C':
                        /* 325 or 160 - Scroll Lock on */
                        debug(F110,"Wyse Escape","Scroll Lock on",0);
                        if ( debses )
                            break;
                        break;
                    case 'F':
                        /* Economy 80-col mode off */
                        debug(F110,"Wyse Escape","Economy 80-col mode off",0);
                        if ( debses )
                            break;
                        break;
                    case 'G':
                        /* Economy 80-col mode on */
                        debug(F110,"Wyse Escape","Economy 80-col mode on",0);
                        if ( debses )
                            break;
                        break;
                    case 'H':
                        /* 325 or 160 - ASCII Key codes */
                        debug(F110,"Wyse Escape","ASCII Key codes",0);
                        if ( debses )
                            break;
                        break;
                    case 'I':
                        /* 325 or 160 - PC Scan codes */
                        debug(F110,"Wyse Escape","PC Scan codes",0);
                        if ( debses )
                            break;
                        break;
                    case 'J':
                        /* Don't save function key labels */
                        debug(F110,"Wyse Escape","Don't save function labels",0);
                        if ( debses )
                            break;
                        break;
                    case 'K':
                        /* Save function key labels */
                        debug(F110,"Wyse Escape","Save function labels",0);
                        if ( debses )
                            break;
                        break;
                    case 'L':
                        /* Margin bell off */
                        debug(F110,"Wyse Escape","Margin bell off",0);
                        if ( debses )
                            break;
                        marginbell = FALSE ;
                        break;
                    case 'M':
                        /* Margin bell on */
                        debug(F110,"Wyse Escape","Margin bell on",0);
                        if ( debses )
                            break;
                        marginbell = TRUE ;
                        break;
                    case 'N':
                        /* Automatic font loading off */
                        debug(F110,"Wyse Escape","Auto font loading off",0);
                        if ( debses )
                            break;
                        break;
                    case 'O':
                        /* Automatic font loading on */
                        debug(F110,"Wyse Escape","Auto font loading on",0);
                        if ( debses )
                            break;
                        break;
                    case 'P':
                        /* Screen saver off */
                        debug(F110,"Wyse Escape","Screen saver off",0);
                        if ( debses )
                            break;
                        break;
                    case 'Q':
                        /* Screen saver on */
                        debug(F110,"Wyse Escape","Screen saver on",0);
                        if ( debses )
                            break;
                        break;
                    case 'R':
                        /* Turn on nulls suppress */
                        debug(F110,"Wyse Escape","NUL suppress on",0);
                        if ( debses )
                            break;
                        wy_nullsuppress = TRUE ;
                        break;
                    case 'S':
                        /* Turn off nulls suppress */
                        debug(F110,"Wyse Escape","NUL suppress off",0);
                        if ( debses )
                            break;
                        wy_nullsuppress = FALSE ;
                        break;
                    case 'T':
                        /* Define CAPS LOCK key as CAPS LOCK */
                        debug(F110,"Wyse Escape","CapsLock is CapsLock",0);
                        if ( debses )
                            break;
                        break;
                    case 'U':
                        /* Define CAPS LOCK key as REV */
                        debug(F110,"Wyse Escape","CapsLock is REV",0);
                        if ( debses )
                            break;
                        break;
                    case 'V':
                        /* Define CAPS LOCK key as SHIFT */
                        debug(F110,"Wyse Escape","CapsLock is SHIFT",0);
                        if ( debses )
                            break;
                        break;
                    case 'x':
                        /* Turn off multiple page mode (default) WY30 */
                        debug(F110,"Wyse Escape","Multiple page mode off",0);
                        if ( debses )
                            break;
                        break;
                    case 'y':
                        /* Turn on multiple page mode WY30 */
                        debug(F110,"Wyse Escape","Multiple page mode on",0);
                        if ( debses )
                            break;
                        break;
                    case 'z':
                        /* Select 10x15 character cell WY30 */
                        debug(F110,"Wyse Escape","10x15 char cell",0);
                        if ( debses )
                            break;
                        break;
                    }
                }
                break;
            case 'f': {
                /* Program and display computer message on label line */
                /* Also known as the Status Line */
                int width = VscrnGetWidth(VTERM);
                int i = 0;

                debug(F110,"Wyse Escape","Program and display computer message on label line",0);
                if ( debses ) {
                    while ( i < width && wyinc() != CR ) {
                        i++;
                    }
                }
                else {
                    while ( i < width &&
                            (wy_labelmsg[i]=wyinc()) != CR ) {
                        i++;
                    }
                    wy_labelmsg[i] = NUL ;
                }
                while ( i < width )
                    wy_labelmsg[i++] = SP ;
                wy_labelmsg[i]=NUL ;
                if ( debses )
                    break;
                setdecssdt(SSDT_HOST_WRITABLE);
                if ( decssdt == SSDT_HOST_WRITABLE ) {
                    VscrnSetWidth(VSTATUS,VscrnGetWidth(VTERM));
                    VscrnWrtCharStrAtt( VSTATUS, wy_labelmsg,
                                        strlen(wy_labelmsg),
                                        1, 1, &colorstatus ) ;
                }
                break;
            }
            case 'g':
                /* Turn on shifted and unshifted label line display */
                /* (enhanced mode) WY30 */
                debug(F110,"Wyse Escape","Label Line Display on",0);
                if ( debses )
                    break;
                setdecssdt(SSDT_HOST_WRITABLE);
                break;
            case 'h':
                /* Turn off shifted and unshifted label line display */
                /* (enhanced mode) WY30 */
                debug(F110,"Wyse Escape","Label Line Display off",0);
                if ( debses )
                    break;
                setdecssdt(SSDT_INDICATOR);
                break;
            case 'i':
                /* Moves the cursor to the next tab stop on the right.    */
                debug(F110,"Wyse Escape","Horizontal Tab",0);
                if ( debses )
                    break;
                wyse_tab();
                break;
            case 'j':
                /* Moves the cursor up one row and begins scrolling at    */
                /* top row.                                               */
                debug(F110,"Wyse Escape","Cursor Up",0);
                if ( debses )
                    break;
                cursorup(0) ;
                break;
            case 'k':
                /* Turns the local edit submode on; duplex off */
                /* Not the same as ECHO mode */
                debug(F110,"Wyse Escape","Local Edit mode on; Duplex off",0);
                break;
            case 'l':
                /* Turns the duplex edit submode on; local off */
                /* Not the same as ECHO mode */
                debug(F110,"Wyse Escape","Local Edit mode off; Duplex on",0);
                break;
            case 'm':
                debug(F110,"Wyse Escape","m",0);
                break;
            case 'n':
                /* Turn on screen display (WY30) Enhanced Mode */
                debug(F110,"Wyse Escape","Screen Display on",0);
                if ( debses )
                    break;
                if ( wy_enhanced )
                    screenon = TRUE ;
                break;
            case 'o':
                /* Turn off screen display (WY30) Enhanced Mode */
                debug(F110,"Wyse Escape","Screen Display off",0);
                if ( debses )
                    break;
                if ( wy_enhanced )
                    screenon = FALSE ;
                break;
            case 'p':
                /* Sends all characters unformatted to the auxiliary port. */
                /* Attributes codes are sent as spaces.  Row-end sequences */
                /* are not sent.  The action is the same as ESC L.         */
                debug(F110,"Wyse Escape","Sends all chars unformatted to AUX",0);
                break;
            case 'q':
                /* Turns the insert submode on; replace off */
                debug(F110,"Wyse Escape","Insert submode on; replace off",0);
                if ( debses )
                    break;
                insertmode = TRUE ;
                break;
            case 'r':
                /* Turns the insert submode off; replace on */
                debug(F110,"Wyse Escape","Insert submode off; replace on",0);
                if ( debses )
                    break;
                insertmode = FALSE ;
                break;
            case 's': {
                /* Sends a message (entire block) */
                int x,y,ch,
                w=VscrnGetWidth(VTERM),
                h=VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0),
                xs=-1,
                ys=-1,
                xe=w-1,
                ye=h-1,
                fs=0 ;

                debug(F110,"Wyse Escape","Send a message (entire block)",0);
                if ( debses )
                    break;

                /* First figure out if we have markers, STX then ETX */
                for ( y=0;y<h;y++ )
                    for ( x=0;x<w;x++ ) {
                        ch = VscrnGetCell( VTERM, x, y )->c ;
                        if ( ch == STX && xs < 0 && ys < 0 ) {
                            xs = x+1 ;
                            ys = y ;
                        }
                        else if ( ch == ETX && xs >= 0 && ys >= 0 ) {
                            xe = x-1 ;
                            ye = y ;
                            break;
                        }
                    }
                if ( xs < 0 || ys < 0 ) {
                    xs = 0 ;
                    ys = 0 ;
                }

                /* Now send the data */
                for ( y=ys;y<=ye; y++ ) {
                    for ( x=(y==ys)?xs:0 ; x <= (y==ye?xe:w-1) ; x++ ) {
                        if ( !VscrnGetVtCharAttr(VTERM, x, y).unerasable ) {
                            if ( fs ) {
                                sendchars("\x1b(",2);
                                fs = 0;
                            }
                        }
                        else if ( !fs ) {
                            sendchars("\x1b)",2);
                            fs = 1 ;
                        }
                        ch = VscrnGetCell( VTERM, x, y )->c;
                        if ( ch || !wy_nullsuppress ) {
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
                    }
                    if ( y < ye ) {
                        switch ( wy_blockend ) {
                        case EOB_CRLF_ETX:
                            sendchar(CR);
                            sendchar(LF);
                            break;
                        case EOB_US_CR:
                            sendchar(US);
                            break;
                        }
                    }
                }
                switch ( wy_blockend ) {
                case EOB_CRLF_ETX:
                    sendchar(ETX);
                    break;
                case EOB_US_CR:
                    sendchar(CR);
                    break;
                }
                break;
            }
            case 't':
                /* Erases all (unprotected) characters from the current   */
                /* cursor location to the end of the row and replaces     */
                /* them with nulls.                                       */
                debug(F110,"Wyse Escape","Erase all unprotected chars from cursor to EOL with NUL",0);

                if ( debses )
                    break;
                if (protect)
                    selclrtoeoln( VTERM, NUL ) ;
                else
                    clrtoeoln( VTERM, NUL ) ;
                break;
            case 'u':
                /* Turns the monitor submode off.  See ESC X.             */
                debug(F110,"Wyse Escape","Monitor mode off",0);
                if ( !wy_monitor )
                    break;
                setdebses(FALSE);
                wy_monitor = FALSE ;
                break;
            case 'v':
                /* Turn on autopage mode */
                debug(F110,"Wyse Escape","Autopage mode on",0);
                if ( debses )
                    break;
                wy_autopage = TRUE ;
                break;
            case 'w':
                if ( ISWY30(tt_type_mode) || ISWY60(tt_type_mode) ) {
                    int n = wyinc() ;

                    switch ( n ) {
                    case '0':
                        /* Display page 0 */
                        debug(F110,"Wyse Escape","Display page 0",0);
                        break;
                    case '1':
                        /* Display page 1 */
                        debug(F110,"Wyse Escape","Display page 1",0);
                        break;
                    case '2':
                        /* Display page 2 (economy mode only) */
                        debug(F110,"Wyse Escape","Display page 2",0);
                        break;
                    case 'B':
                        /* Display previous page */
                        debug(F110,"Wyse Escape","Display previous page",0);
                        break;
                    case 'C':
                        /* Display next page */
                        debug(F110,"Wyse Escape","Display next page",0);
                        break;
                    case 'E':
                        /* Roll window up in page */
                        debug(F110,"Wyse Escape","Roll window up in page",0);
                        break;
                    case 'F':
                        /* Roll window down in page */
                        debug(F110,"Wyse Escape","Roll window down in page",0);
                        break;
                    case 'G':
                        /* Divide memory into pages (1xlines)    */
                        /* Length of page equal to the number of */
                        /* data lines (default)                  */
                        debug(F110,"Wyse Escape","Divide memory into pages (1x)",0);
                        break;
                    case 'H':
                        /* Divide memory into pages (2xlines)    */
                        /* Length of page equal to double the    */
                        /* number of data lines                  */
                        debug(F110,"Wyse Escape","Divide memory into pages (2x)",0);
                        break;
                    case 'I':
                        /* Divide memory into pages (4xlines)    */
                        /* Length of page equal to Four times    */
                        /* the number of data lines              */
                        debug(F110,"Wyse Escape","Divide memory into pages (4x)",0);
                        break;
                    case 'J':
                        /* Divide memory into pages                   */
                        /* One page contains the number of data lines */
                        /* The other contains the rest of the memory  */
                        debug(F110,"Wyse Escape","Divide memory into pages (split)",0);
                        break;
                    case '@':{
                        /* page line col */
                        /* Address cursor in specific 80-col page */
                        debug(F110,"Wyse Escape","Address cursor page line col",0);
                        z = wyinc();
                        y = wyinc() ;
                        x = wyinc() ;

                        if ( debses )
                            break;
                        if ( /* VscrnGetWidth(VTERM) == 80 && */ x > 31 && y > 31 )
                            lgotoxy(VTERM, x-31, y-31 ) ;
                        break;
                    }
                    case '`':
                        /* Read 80-col page number and cursor address */
                        debug(F110,"Wyse Escape","Report cursor page line col",0);
                        if ( debses )
                            break;
                        if ( 1 /* VscrnGetWidth(VTERM) == 80 */ ) {
                            char buf[4] ;
                            buf[0] = '0' ;
                            buf[1] = (char)(wherey[VTERM]+31) ;
                            buf[2] = (char)(wherex[VTERM]+31) ;
                            buf[3] = NUL ;
                            sendchars(buf,3) ;
                        }
                        break;
                    }
                }
                else {  /* Wyse 50 */
                    /* Turn off autopage mode */
                    debug(F110,"Wyse Escape","Autopage off",0);
                    if ( debses )
                        break;
                    wy_autopage = FALSE ;
                }
                break;
            case 'x':
                /* Changes the screen display format.  The sequences are:   */
                /*    ESC x 0 for a full screen, 24 rows by 80 or 132 cols. */
                /*    ESC x 1 HSR for a horizontal split screen. (two page max) */
                /*    ESC x 3 HSR for a horizontal split screen (wy60)      */
                /* where                                                    */
                /*    HSR = row code for the row number 2 to 24 on which    */
                /*          the lower text segment starts.                  */
                /* these calls clear the pages.                             */
                {
                    int n = wyinc() ;

                    switch ( n ) {
                    case '0':
                        /* Redefine screen as one window and clear pages */
                        debug(F110,"Wyse Escape","Redefine screen as 1 window and clear pages",0);
                        if ( debses )
                            break;
                        writeprotect = FALSE ;
                        protect = FALSE ;
                        attrib.unerasable = FALSE ;
                        clrscreen(VTERM, SP) ;
                        lgotoxy(VTERM,1,1);       /* and home the cursor */
                        break;
                    case '1':
                        /* line */
                        /* Split screen and clear pages (two max) */
                        debug(F110,"Wyse Escape","Split screen and clear pages",0);
                        if ( debses )
                            break;
                        clrscreen( VTERM, SP ) ;
                        lgotoxy(VTERM,1,1);       /* and home the cursor */
                        /* if we support this, send ACK to host */
                        break;
                    case '3':
                        /* line */
                        /* Split screen and clear pages - WY60 */
                        debug(F110,"Wyse Escape","Split screen and clear pages",0);
                        if ( debses )
                            break;
                        break;
                    case '@':
                        /* Redefine screen as one window - no clear - WY60 */
                        debug(F110,"Wyse Escape","Redefine screen as one window - no clear",0);
                        if ( debses )
                            break;
                        break;
                    case 'A':
                        /* line */
                        /* Split screen (two max) - WY60 */
                        debug(F110,"Wyse Escape","Split screen",0);
                        if ( debses )
                            break;
                        break;
                    case 'C':
                        /* line */
                        /* Split screen - WY60 */
                        debug(F110,"Wyse Escape","Split Screen",0);
                        if ( debses )
                            break;
                        break;
                    case 'P':
                        /* Lower Horizontal Split - Wy60 */
                        debug(F110,"Wyse Escape","Lower horizontal split",0);
                        if ( debses )
                            break;
                        break;
                    case 'R':
                        /* Raise Horizontal Split - WY60 */
                        debug(F110,"Wyse Escape","Raise horizontal split",0);
                        if ( debses )
                            break;
                        break;
                    }
                }
                break;
            case 'y':
                /* Erases all characters from the current cursor location   */
                /* to the end of the active segment and replaces them with  */
                /* nulls. (unprotected)                                     */
                debug(F110,"Wyse Escape","Erase all chars from cursor to end of segment with NUL",0);
                if ( debses )
                    break;
                if ( protect )
                    selclreoscr_escape( VTERM, NUL ) ;
                else
                    clreoscr_escape( VTERM, NUL ) ;
                break;
            case 'z':  {
                /* Enters a message into a selected function key label field */
                /* or programs a user-defined sequence for a function key    */
                /* (maximum of eight label fields, shiftable to 16 for an    */
                /* 80-column screen; maximum of 16 label fields, shiftable   */
                /* to 32 for a 132-column screen.  The message format is:    */
                /*   ESC z n aaaa CR   where:                                */
                /*   n = field code                                          */
                /*  aaaa = a character string of up to eight characters for  */
                /*         an 80-column screen or up to seven characters for */
                /*         an 132-column screen.                             */
                /*   ESC z n CR clears a particular function key label       */
                /*   ESC z DEL turns off the shifted function key labeling   */
                /*         line.                                             */
                /* The function key program format is:                       */
                /*   ESC z value SEQ DEL  where:                             */
                /*   value = the default value code                          */
                /*   SEQ   = the program sequence up to eight bytes          */

                /* WY60 adds: */
                /* ESC z ( text CR  Program/display unshifted label line     */
                /* ESC z ) text CR  Program shifted label line               */
                /* ESC z ( CR       Clear unshifted label line               */
                /* ESC z ) CR       Clear shifted label line                 */
                /* ESC z P CR       Display shifted label line               */
                char keydef[256] = "" ;
                int key = wyinc() ;
                int i = 0 ;

                if ( key == DEL ) {
                        /* Turn off shifted function key labeling */
                    debug(F110,"Wyse Escape","Turn off shifted function key labeling",0);
                    break ;
                }

                if ( !((key >= '@' && key <= 'O') ||
                        (key >= '`' && key <= 'o') ||
                        (key >= '0' && key <= '7') ||
                        (key >= 'P' && key <= '_'))) {
                    /* we don't support anything other than */
                    /* key programming yet. */
                    debug(F110,"Wyse Escape","Program key definition or label",0);
                    if ( debses )
                        break;
                    while ( (keydef[i]=wyinc()) != DEL &&
                                (keydef[i] != CR))
                        i++;
                    keydef[i] = NUL ;

                    break;
                }
                else if ( ISWY60(tt_type_mode) &&
                          key == '(' ) {
                    /* ESC z ( text CR  Program/display unshifted label line */
                    /* ESC z ( CR       Clear unshifted label line           */
                    int i = 0;

                    debug(F110,"Wyse Escape","Program/display/clear unshifted label line",0);

                    if ( debses ) {
                        while ( i < MAXTERMCOL &&
                                wyinc() != CR ) {
                            i++;
                        }
                    }
                    else {
                        while ( i < MAXTERMCOL &&
                                (wy_labelline[i]=wyinc()) != CR ) {
                            i++;
                        }
                        wy_labelline[i] = NUL ;
                    }
                }
                else if ( ISWY60(tt_type_mode) &&
                          key == ')' ) {
                    /* ESC z ) text CR  Program shifted label line */
                    /* ESC z ) CR       Clear shifted label line   */
                    int i = 0;

                    debug(F110,"Wyse Escape","Program/display/clear shifted label line",0);
                    if ( debses ) {
                        while ( i < MAXTERMCOL &&
                                wyinc() != CR ) {
                            i++;
                        }
                    }
                    else {
                        while ( i < MAXTERMCOL &&
                                (wy_shiftlabelline[i]=wyinc()) != CR ) {
                            i++;
                        }
                        wy_shiftlabelline[i] = NUL ;
                    }
                }
                else if ( ISWY60(tt_type_mode) &&
                          key == 'P' && wyinc() == CR ) {
                    /* ESC z P CR       Display shifted label line */
                    debug(F110,"Wyse Escape","Display Shifted label line",0);
                    if ( debses )
                        break;
                }
                else {
                    int label = ( (key >= '0' && key <= '7') ||
                              (key >= 'P' && key <= '_') );

                    while ( i < 255 &&
                            (((keydef[i]=wyinc()) != DEL) || label) &&
                            ((keydef[i] != CR) || !label) )
                        i++;
                    keydef[i] = NUL ;

                    if ( debses ) {
                        debug(F110,"Wyse Escape","Program key or label",0);
                        break;
                    }

                    if ( keydef[0] == '~' ) {
                        /* we only allow redefinition of function keys */
                        debug(F110,"Wyse Escape","Program key or label tilde",0);
                    }
                    else if ( label ) {
                        /* Function key labels */
                        ckstrncpy( fkeylabel[key-'0'], keydef, 32 );
                        debug(F111,"Wyse [Label]",fkeylabel[key-'0'],key-'0');
                    }
                    else {
                        int keyi=0;
                        if ( key >= '@' && key <= 'O' ) {
                            keyi = key-'@';
                        }
                        else if ( key >= '`' && key <= 'o' ) {
                            keyi = key-'`'+(K_WYSF01-K_WYF01);
                        }
#ifndef NOKVERBS
                        else switch (key) {
                        case '"':       /* Backspace */
                            keyi = K_WYBS-K_WYF01 ;
                            break;
                        case '}':       /* Clear Line */
                            keyi = K_WYCLRLN-K_WYF01;
                            break;
                        case 'z':       /* Shift Clear Line - Clear Screen */
                            keyi = K_WYSCLRLN-K_WYF01;
                            break;
                            /* Clear Page */
                            /* Shift Clear Page */
                        case '5':       /* Del Char */
                            keyi = K_WYDELCHAR-K_WYF01;
                            break;
                        case '6':       /* Del Line */
                            keyi = K_WYDELLN-K_WYF01;
                            break;
                        case 's':       /* Enter */
                            keyi = K_WYENTER-K_WYF01;
                            break;
                        case SP:        /* Escape */
                            keyi = K_WYESC-K_WYF01;
                            break;
                        case '*':       /* Home */
                            keyi = K_WYHOME-K_WYF01;
                            break;
                        case '/':       /* Shift Home */
                            keyi = K_WYSHOME-K_WYF01;
                            break;
                        case '8':       /* Shift Replace (Insert) */
                            keyi = K_WYINSERT-K_WYF01;
                            break;
                        case 'q':       /* Insert Char */
                            keyi = K_WYINSCHAR-K_WYF01;
                            break;
                        case 'p':       /* Insert Line */
                            keyi = K_WYINSLN-K_WYF01;
                            break;
                        case 'r':       /* Page Next */
                            keyi = K_WYPGNEXT-K_WYF01;
                            break;
                        case 'w':       /* Page Prev */
                            keyi = K_WYPGPREV-K_WYF01;
                            break;
                        case '7':       /* Replace */
                            keyi = K_WYREPLACE-K_WYF01;
                            break;
                        case '$':       /* Return */
                            keyi = K_WYRETURN-K_WYF01;
                            break;
                        case '!':       /* Tab */
                            keyi = K_WYTAB-K_WYF01;
                            break;
                        case '&':       /* Shift Tab */
                            keyi = K_WYSTAB-K_WYF01;
                            break;
                        case 'R':       /* Print Screen */
                            keyi = K_WYPRTSCN-K_WYF01;
                            break;
                        case '%':       /* Shift Esc */
                            keyi = K_WYSESC-K_WYF01;
                            break;
                        case '\'':      /* Shift Backspace */
                            keyi = K_WYSBS-K_WYF01;
                            break;
                        case '4':       /* Shift Enter */
                            keyi = K_WYSENTER-K_WYF01;
                            break;
                        case ')':       /* Shift Return */
                            keyi = K_WYSRETURN-K_WYF01;
                            break;
                        case '+':       /* Up arrow */
                            keyi = K_WYUPARR-K_WYF01;
                            break;
                        case ',':       /* Dn arrow */
                            keyi = K_WYDNARR-K_WYF01;
                            break;
                        case '-':       /* Left arrow */
                            keyi = K_WYLFARR-K_WYF01;
                            break;
                        case '.':       /* Right arrow */
                            keyi = K_WYRTARR-K_WYF01;
                            break;
                        case '0':       /* Shift Up arrow */
                            keyi = K_WYSUPARR-K_WYF01;
                            break;
                        case '1':       /* Shift Dn arrow */
                            keyi = K_WYSDNARR-K_WYF01;
                            break;
                        case '2':       /* Shift Left arrow */
                            keyi = K_WYSLFARR-K_WYF01;
                            break;
                        case '3':       /* Shift Right arrow */
                            keyi = K_WYSRTARR-K_WYF01;
                            break;
                        case 'u':       /* Send */
                            keyi = K_WYSEND-K_WYF01;
                            break;
                        case 't':       /* Shift Send */
                            keyi = K_WYSSEND-K_WYF01;
                            break;
                        case 'Q':       /* F17 */
                            keyi = K_WYF17-K_WYF01;
                            break;
                        case 'W':       /* Shift F17 */
                            keyi = K_WYSF17-K_WYF01;
                            break;
                        case 'S':       /* F18 */
                            keyi = K_WYF18-K_WYF01;
                            break;
                        case 'Y':       /* Shift F18 */
                            keyi = K_WYSF18-K_WYF01;
                            break;
                        case 'T':       /* F19 */
                            keyi = K_WYF19-K_WYF01;
                            break;
                        case 'Z':       /* Shift F19 */
                            keyi = K_WYSF19-K_WYF01;
                            break;
                        case 'P':       /* F20 */
                            keyi = K_WYF20-K_WYF01;
                            break;
                        case 'V':       /* Shift F20 */
                            keyi = K_WYSF20-K_WYF01;
                            break;

                        case '#':       /* Del */
                        case '(':       /* Shift Del */
                        case 'X':       /* Shift Print Screen */
                        case ':':       /* PgUp */
                        case ';':       /* Shift PgUp */
                        case '\\':      /* End */
                        case ']':       /* Shift End */
                        default:
                            debug(F111,"Wyse (ESC z)","key",key);
                            break;
                        }
#endif /* NOKVERBS */
#ifndef NOSETKEY
                        if ( keyi >= 0 ) {
                            if ( udkfkeys[keyi] ){
                                free( udkfkeys[keyi] ) ;
                                udkfkeys[keyi]=NULL;
                            }
                            if ( strlen(keydef) ) {
                                char buf[80];
                                udkfkeys[keyi] = strdup( keydef ) ;
                                sprintf(buf,"Wyse [Define Key]=%d",keyi);
                                ckhexdump(buf,keydef,strlen(keydef));
                            } else {
                                udkfkeys[keyi] = wysedefkey( keyi );
                            }
                        }
#endif /* NOSETKEY */
                    }
                }
                break;
            }
            case '{':
                /* Moves the cursor to the home position of the text         */
                /* segment.                                                  */
                debug(F110,"Wyse Escape","Home cursor in active segment",0);
                if ( debses )
                    break;
                lgotoxy(VTERM,1,1) ;
                break;
            case '|': {
                int n = wyinc() ;

                switch ( n ) {
                case '0':
                    /* Redefine screen as one window and clear pages */
                    debug(F110,"Wyse Escape","Redefine screen as 1 window and clear pages",0);
                    if ( debses )
                        break;
                    if ( wy_enhanced ) {
                        writeprotect = FALSE ;
                        protect = FALSE ;
                        attrib.unerasable = FALSE ;
                        clrscreen(VTERM, SP) ;
                        lgotoxy(VTERM,1,1);       /* and home the cursor */
                    }
                    break;
                case '1': {
                    /* Split screen horizontally and clear screen */
                    int line = wyinc();

                    debug(F110,"Wyse Escape","Split screen horizontally and clear screen",0);
                    if ( debses )
                        break;
                    if ( wy_enhanced ) {
                        clrscreen( VTERM, SP ) ;
                        lgotoxy(VTERM,1,1);       /* and home the cursor */
                        /* if we support this, send ACK to host */
                    }
                    break;
                }
                }
                break;
            }
            case '}':
                /* Activates text segment 1 (lower window) */
                debug(F110,"Wyse Escape","Activate text segment 1",0);
                break;
            case '~':
                if ( ISWY60(tt_type_mode) || ISWY60(tt_type) )
                {
                    int n = wyinc() ;
                    switch ( n ) {
                    case SP:
                        /* Enhanced mode off */
                        debug(F110,"Wyse Escape","Enhanced Mode Off",0);
                        if ( debses )
                            break;
                        wy_enhanced = FALSE ;
                        break;
                    case '!':
                        /* Enhanced mode on */
                        debug(F110,"Wyse Escape","Enhanced Mode On",0);
                        if ( debses )
                            break;
                        wy_enhanced = TRUE ;
                        break;
                    case '.':
                        /* Wyseword mode off */
                        debug(F110,"Wyse Escape","Wyseword Mode Off",0);
                        if ( debses )
                            break;
                        break;
                    case '/':
                        /* Wyseword mode on */
                        debug(F110,"Wyse Escape","Wyseword Mode On",0);
                        if ( debses )
                            break;
                        break;
                    case '2':
                        /* Application key mode off */
                        debug(F110,"Wyse Escape","Application Key Mode Off",0);
                        if ( debses )
                            break;
                        wy_keymode = FALSE ;
                        break;
                    case '3':
                        /* Application key mode on */
                        debug(F110,"Wyse Escape","Application Key Mode On",0);
                        if ( debses )
                            break;
                        wy_keymode = TRUE ;
                        break;
                    case '"':
                        /* WY50, WY50+, WY100 mode */
                        debug(F110,"Wyse Escape","Terminal Type WY30,50,50+,100",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        if ( ISWY30(tt_type) )
                            tt_type_mode = TT_WY30 ;
                        else
                            tt_type_mode = TT_WY50 ;
                        ipadl25();
                        break;
                    case '#':
                        /* TVI 910+ mode */
                        debug(F110,"Wyse Escape","Terminal Type TVI910+",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        tt_type_mode = TT_TVI910 ;
                        ipadl25();
                        break;
                    case '$':
                        /* TVI 925 mode */
                        debug(F110,"Wyse Escape","Terminal Type TVI925",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        tt_type_mode = TT_TVI925 ;
                        ipadl25();
                        break;
                    case '%':
                        /* ADDS VP A2 mode */
                        debug(F110,"Wyse Escape","Terminal Type ADDS VP A2",0);
                        if ( debses )
                            break;
                        break;
                    case '&':
                        /* HZ 1500 mode */
                        debug(F110,"Wyse Escape","Terminal Type HZ1500",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        tt_type_mode = TT_HZL1500 ;
                        ipadl25();
                        break;
                    case '\'':
                        /* TVI 912/920 mode */
                        debug(F110,"Wyse Escape","Terminal Type TVI912/920",0);
                        if ( debses )
                            break;
                        break;
                    case '(':
                        /* TVI 950 mode */
                        debug(F110,"Wyse Escape","Terminal Type TVI950",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        tt_type_mode = TT_TVI950 ;
                        ipadl25();
                        break;
                    case ')':
                        /* DG200 mode */
                        debug(F110,"Wyse Escape","Terminal Type DG200",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        tt_type_mode = TT_DG200 ;
                        ipadl25();
                        break;
                    case '*':
                        /* IBM 3101-1X mode */
                        debug(F110,"Wyse Escape","Terminal Type IBM 3101",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        tt_type_mode = TT_IBM31;
                        ipadl25();
                        break;
                    case '+':
                        /* ADM 31 mode */
                        debug(F110,"Wyse Escape","Terminal Type ADM 31",0);
                        if ( debses )
                            break;
                        break;
                    case '`':
                        /* TVI 955 mode */
                        debug(F110,"Wyse Escape","Terminal Type TVI 955",0);
                        if ( debses )
                            break;
                        break;
                    case '-':
                        /* WY75 mode */
                        debug(F110,"Wyse Escape","Terminal Type WY75",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        tt_type_mode = TT_VT102 ;
                        ipadl25();
                        break;
                    case '4':
                        /* WY60 mode */
                        debug(F110,"Wyse Escape","Terminal Type WY60",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        tt_type_mode = TT_WY60 ;
                        ipadl25();
                        break;
                    case '5':
                        /* PC Term mode */
                        debug(F110,"Wyse Escape","Terminal Type Wyse PC Term",0);
                        if ( debses )
                            break;
                        tt_type_mode = TT_WY60;
#ifdef PCTERM
                        setpcterm(TRUE);
#endif /* PCTERM */
                        ipadl25();
                        break;
                    case '6':
                        /* VT52 mode */
                        debug(F110,"Wyse Escape","Terminal Type VT52",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        tt_type_mode = TT_VT52 ;
                        ipadl25();
                        break;
                    case '7':
                        /* IBM 3101-2X mode */
                        debug(F110,"Wyse Escape","Terminal Type IBM 3101-2X",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        tt_type_mode = TT_IBM31;
                        ipadl25();
                        break;
                    case '8':
                        /* ADDS VP-60 mode */
                        debug(F110,"Wyse Escape","Terminal Type ADDS VP-60",0);
                        if ( debses )
                            break;
                        break;
                    case '9':
                        /* IBM 3161 mode */
                        debug(F110,"Wyse Escape","Terminal Type IBM 3161",0);
                        if ( debses )
                            break;
                        break;
                    case ':':
                        /* DG 210 mode */
                        debug(F110,"Wyse Escape","Terminal Type DG210",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        tt_type_mode = TT_DG210 ;
                        ipadl25();
                        break;
                    case ';':
                        /* VT100 mode */
                        debug(F110,"Wyse Escape","Terminal Type VT100",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        tt_type_mode = TT_VT102 ;
                        ipadl25();
                        break;
                    case 'A':
                        /* Esprit III mode */
                        debug(F110,"Wyse Escape","Terminal Type Esprit III",0);
                        if ( debses )
                            break;
                        break;
                    case 'B':
                        /* WY370/VT320 (7-bit) */
                        debug(F110,"Wyse Escape","Terminal Type WY370 (7-bit)",0);
                        if ( debses )
                            break;
                        tt_type_mode = TT_WY370 ;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        setcmask(7);
                        ipadl25();
                        break;
                    case 'C':
                        /* WY370/VT320 (8-bit) */
                        debug(F110,"Wyse Escape","Terminal Type WY370 (8-bit)",0);
                        if ( debses )
                            break;
#ifdef PCTERM
                        setpcterm(FALSE);
#endif /* PCTERM */
                        tt_type_mode = TT_WY370 ;
                        setcmask(8);
                        ipadl25();
                        break;
                    case '>':
                        /* Tek 4010/4014 mode */
                        debug(F110,"Wyse Escape","Terminal Type Tek 4010/4014",0);
                        if ( debses )
                            break;
                        ipadl25();
                        break;
                    }
                }
                break;
            case DEL:
                break;
            }
            }
            escstate = ES_NORMAL ;      /* Done parsing escstate sequence */
        }
    }
    else                /* Handle as a normal character */
    {
        if ( ch < SP )
            wysectrl(ch) ;
        else if ( !debses && !xprint ) {
            if ( wysegraphics && ch < 128 )
                ch = xlwygrph( ch ) ;

            /* Display the character */
            if ( !tt_hidattr &&
                 (attrmode == ATTR_PAGE_MODE || attrmode == ATTR_LINE_MODE) ) {
                vtattrib vta = VscrnGetVtCharAttr( VTERM, wherex[VTERM]-1, wherey[VTERM]-1 ) ;
                if ( vta.wyseattr ) {
                    int x=wherex[VTERM]-1 ;
                    int y=wherey[VTERM]-1 ;
                    /* we are about to overwrite an attribute */
                    /* this results in the previous attribute being used for the entire */
                    /* this and subsequent cells */
                    vta.wyseattr = FALSE ;
                    VscrnSetVtCharAttr( VTERM, x, y, vta ) ;

                    /* now find previous attribute */
                    while ( !vta.wyseattr ) {
                        if ( vta.wyseattr )     /* bug in MSVC 5.0 */
                            break;
                        x--;
                        if ( x < 0 ) {
                            y--;
                            if ( y >= 0 )
                                x = VscrnGetWidth(VTERM) ;
                            else {
                                x = y = 0 ;
                                break;  /* nowhere left to go */
                            }
                        }
                        vta = VscrnGetVtCharAttr( VTERM, x, y ) ;
                    }

                    if ( attrmode == ATTR_PAGE_MODE )
                        ApplyPageAttribute( VTERM, x+1, y+1, vta ) ;
                    else
                        ApplyLineAttribute( VTERM, x+1, y+1, vta ) ;
                }
            }
            if ( ch != DEL )
                wrtch(ch);
        }
    }
    VscrnIsDirty(VTERM) ;
}
#endif /* NOTERM */
