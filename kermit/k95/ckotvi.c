/*  C K O T V I . C  --  Televideo Emulation  */

/*
  Author: Jeffrey Altman <jaltman@secure-endpoints.com>,
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#include "ckcdeb.h"
#ifndef NOTERM
#ifdef NT
#include <windows.h>
#else /* NT */
/* so that we get SEM_INDEFINITE_WAIT from the watcom OS/2 headers */
#define INCL_DOSSEMAPHORES
#include <os2.h>
#undef COMMENT
#endif /* NT */

#include "ckcker.h"
#include "ckcasc.h"
#include "ckuusr.h"
#include "ckcuni.h"
#include "ckocon.h"
#include "ckotvi.h"
#include "ckctel.h"
#include "ckokvb.h"

extern bool keyclick ;
extern int  cursorena[], keylock, duplex, duplex_sav, screenon ;
extern int  printon, aprint, cprint, uprint, xprint, seslog ;
extern int  insertmode, tnlm, decssdt ;
extern int  escstate, debses, decscnm, tt_cursor, ttmdm ;
extern int  tt_type, tt_type_mode, tt_max, tt_answer, tt_status[VNUM], tt_szchng[] ;
extern int  tt_cols[], tt_rows[], tt_wrap, tt_modechg ;
extern int  wherex[], wherey[], margintop, marginbot ;
extern int  marginbell, marginbellcol ;
extern char answerback[], htab[] ;
extern struct tt_info_rec tt_info[] ;
extern vtattrib attrib ;
extern unsigned char attribute, colorstatus;
extern char * udkfkeys[];
extern int tt_senddata ;
extern int tt_hidattr;

void selclrscreen(BYTE, CHAR);  /* ckoco3.c */
void setkeyclick(int);          /* ckoco3.c */
void clrscreen(BYTE, CHAR);     /* ckoco3.c */
void setborder();               /* ckocon.c */
USHORT tx_lucidasub(USHORT);    /* ckcuni.c */
USHORT tx_usub(USHORT);         /* ckcuni.c */
USHORT tx_hslsub(USHORT);       /* ckcuni.c */

#ifdef CK_NAWS                          /* Negotiate About Window Size */
#ifdef TCPSOCKET
extern int tn_snaws();
#endif /* TCPSOCKET */

#ifdef RLOGCODE
_PROTOTYP( int rlog_naws, (void) );     /* ckcnet.c */
#endif /* RLOGCODE */
#endif /* CK_NAWS */

extern int protect ;
extern int autoscroll ;
extern int writeprotect ;
extern int linelock ;
extern int attrmode ;
extern int wy_keymode ;
extern int wy_enhanced ;
extern int wy_widthclr ;
extern int wy_autopage ;
extern int wy_nullsuppress ;
extern int wy_monitor ;
extern int wy_block;
extern vtattrib WPattrib ;
extern tt_sac;
extern bool xprintff; /* Print formfeed */

int tvi_ic = SP;
CHAR tvi_send_field[3] = { XFS, NUL, NUL };
CHAR tvi_send_line[3] = { US, NUL, NUL };
CHAR tvi_send_start_protected_field[3] = { ESC, ')', NUL };
CHAR tvi_send_end_protected_field[3] = { ESC, '(', NUL } ;
CHAR tvi_send_end_of_text[3] = { CK_CR, NUL, NUL };

int tvigraphics = FALSE;

static char tviuserline[MAXTERMCOL+1] ;


/* Display Field                n  */
#define WY_FIELD_APPL           0       /* Application Display Area   */
#define WY_FIELD_FNKEY          1       /* Function Key Labeling Line */
#define WY_FIELD_LOCAL_MSG      2       /* Local Message Field        */
#define WY_FIELD_HOST_MSG       3       /* Host Message Field         */

_PROTOTYP(VOID wyse_tab, (VOID));
_PROTOTYP(VOID wyse_backtab, (VOID));

int
tviinc(void)
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
xltvigrph( CHAR c ) {
    extern struct _vtG *GNOW;
    extern int win95lucida, win95hsl;

    USHORT cx = 0;
    if ( ck_isunicode() ) {
        cx = (*xl_u[TX_TVIGRAPH])(c);
        if ( win95hsl && cx >= 0x23BA && cx <= 0x23BD )
            cx = tx_hslsub(cx);
        else if ( cx >= 0xE000 && cx <= 0xF8FF )
            cx = tx_usub(cx);
        if (win95lucida && cx > 0x017f)
            cx = tx_lucidasub(cx);
    }
    else
    {
        cx = (*xl_u[TX_TVIGRAPH])(c) ;
        if ( win95hsl && cx >= 0x23BA && cx <= 0x23BD )
            cx = tx_hslsub(cx);
        else if ( cx >= 0xE000 && cx <= 0xF8FF )
            cx = tx_usub(cx);
        if ( GNOW->itol )
            cx = (*GNOW->itol)(cx);
    }
    return(cx);
}

void
tvictrl( int ch )
{
    int x,y;

    if ( !xprint ) {
    switch ( ch ) {
    case ETX:
        if ( debses )
            break;
        break;
    case EOT:
        if ( debses )
            break;
        break;
    case ENQ:
        if ( debses )
            break;
        if (tt_answer)
            sendchars(answerback,strlen(answerback)) ;
        sendchar(ACK);
        break;
    case ACK:
        if ( debses )
            break;
        break;
    case BEL:
        if ( debses )
            break;
        bleep(BP_BEL);
        break;
    case BS:
        if ( debses )
            break;
        cursorleft(0) ;
        break;
    case HT:
        /* TVI 950 Tab behaves differently depending on state of */
        /* protect/non-protect mode and autopage mode */

        if ( debses )
            break;

        wyse_tab();
        break;
    case LF:
        if ( debses )
            break;
        if ( autoscroll && !protect || wherey[VTERM] < marginbot )  {
            wrtch((char) LF);
        }
        break;
    case VT:
        if ( debses )
            break;
        cursorup(0) ;
        break;
    case FF:
        if ( debses )
            break;
        cursorright(0) ;
        break;
    case CK_CR:
        if ( debses )
            break;
        wrtch((char) CK_CR);
        break;
    case SO:
        if ( debses )
            break;
        /* Set Host port Receive to DTR */
        break;
    case SI:
        if ( debses )
            break;
        /* Set Host port Receive to XON/XOFF or XPC */
        break;
    case DLE:
        /* Store next, used by load function key only */
        if ( debses )
            break;
        break;
    case DC1:
        /* Enable transmission */
        break;
    case DC2:
        /* Turn on Bi-directional port */
        if ( debses )
        break;
    case DC3:
        /* Disable transmission */
        break;
    case DC4:
        /* Turn off Bi-directional port */
        if ( debses )
            break;
        break;
    case NAK:
        if ( debses )
            break;
        break;
    case SYN:
        /* Cursordown - no scroll */
        if ( debses )
            break;
        cursordown(0) ;
        break;
    case ETB:
        if ( debses )
            break;
        break;
    case CAN:
        if ( debses )
            break;
        break;
    case XEM:
        /* End Message - used by load function key only */
        break;
    case SUB:
        /* Clear unprotected page to Insert Character */
        if ( debses )
            break;
        /* Clear unprotected page to Insert Character */
        selclrscreen( VTERM, tvi_ic );
        lgotoxy(VTERM,1,1);       /* and home the cursor */
        if ( writeprotect ) {
            int xs = VscrnGetWidth(VTERM);
            int ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
            vtattrib vta = {0,0,0,0,0,1,0,0,0,0,0}; /* Protected */
            for ( y = 0 ; y < ys ; y++ )
                for ( x = 0 ; x < xs ; x++ ) {
                    vta = VscrnGetVtCharAttr( VTERM, x, y ) ;
                    vta.unerasable = TRUE ;
                    VscrnSetVtCharAttr( VTERM, x, y, vta ) ;
                }
        }
        break;
    case ESC:
        /* initiate escape sequence */
        escstate = ES_GOTESC ;
        break;
    case XRS:
        /* Home cursor */
        if ( debses )
            break;
        lgotoxy(VTERM,1,1) ;
        break;
    case US:
        /* Cursor to start on next line */
        if ( debses )
            break;
        if (autoscroll && !protect) {
            wrtch((CHAR)CK_CR);
            wrtch((CHAR)LF);
        }
        break;
    }
    }
    else { /* xprint */
        switch ( ch ) {
        case ESC:
            /* initiate escape sequence */
            escstate = ES_GOTESC ;
            break;
        }
    }
}

vtattrib
wysecharattr( int ch ) ;

USHORT
wyselineattr( int ch ) ;

USHORT
wyselinebg( int ch )  ;

void
ApplyPageAttribute( int vmode, int x, int y, vtattrib vta )  ;

void
ApplyLineAttribute( int vmode, int x, int y, vtattrib vta )  ;

void
tviascii( int ch )
{
    int j,x,y;
    vtattrib attr ;
    viocell blankvcell;

    if (printon && (is_xprint() || is_uprint()))
        prtchar(ch);

    if ( escstate == ES_GOTESC )/* Process character as part of an escstate sequence */
    {
        if ( ch < SP ) {
            escstate = ES_NORMAL ;
            tvictrl(ch) ;
        }
        else
        {
            escstate = ES_ESCSEQ ;
            if ( !xprint ) {
            switch ( ch ) {
            case SP: {
                /* Time of Day */
                int mode, ampm, hr, min ;
                mode = tviinc() ;
                switch ( mode ) {
                case '1':       /* Load a/p hr min */
                    ampm = tviinc();
                    hr = tviinc() ;
                    min = tviinc() ;
                    if ( debses )
                        break;
                    break;
                case '2': {     /* Send */
                    if ( debses )
                        break;
                    break;
                }
                }
                break;
            }
            case '!':
                if ( ISTVI950(tt_type_mode) )
                {
                    ch = tviinc();
                    switch ( ch ) {
                    case '1':
                        /* TVI950 - Line lock mode on */
                        /* applies only to current line ?? */
                        if ( debses )
                            break;
                        linelock = TRUE ;
                        break;
                    case '2':
                        /* TVI950 - Line lock mode off */
                        /* Clears all line locks ?? */
                        if ( debses )
                            break;
                        linelock = FALSE ;
                        break;
                }
                }
                else {
                    /* clear unprotected page to display attribute */
                    ch = tviinc() ;
                }
                break;
            case '"':
                /* Unlocks the keyboard */
                if ( !debses )
                    keylock = FALSE ;
                break;
            case '#':
                /* Locks the keyboard */
                if ( !debses )
                    keylock = TRUE ;
                break;
            case '$':
                if ( debses )
                    break;
                tvigraphics = TRUE ;
                break;
            case '%':
                if ( debses )
                    break;
                tvigraphics = FALSE ;
                break;
            case '&':
                /* Turns the protect submode on and prevents the auto */
                /* scroll operation */
                /* not TVI 910+ */
                if ( ISTVI910(tt_type_mode) || debses )
                    break;
                protect = TRUE ;
                break;
            case '\'':
                /* Turns the protect submode off and allows the auto */
                /* scroll operation */
                if ( !debses )
                    protect = FALSE ;
                break;
            case '(':
                /* Turns the write protect submode off */
                if ( debses || !writeprotect )
                    break;
                writeprotect = FALSE ;
                attrib.unerasable = FALSE;
                break;
            case ')':
                /* Turns the write protect submode on */
                if ( debses || writeprotect )
                    break;
                writeprotect = TRUE ;
                attrib.unerasable = TRUE ;
                break;
            case '*':
                /* Clears the screen to nulls. */
                /* The protect submode is turned off */
                if ( debses )
                    break;
                protect = FALSE ;
                writeprotect = FALSE ;
                clrscreen( VTERM, NUL ) ;
                lgotoxy(VTERM,1,1);       /* and home the cursor */
                break;
            case '+':
                /* 955 mode: Clear all to replacement characters         */
                /* (reset protect and write protect modes)               */
                /* 950 mode: Clear unprotected to replacement characters */
                if ( debses )
                    break;
                selclrscreen( VTERM, tvi_ic );
                lgotoxy(VTERM,1,1);       /* and home the cursor */
                if ( writeprotect ) {
                    int xs = VscrnGetWidth(VTERM);
                    int ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                    vtattrib vta = {0,0,0,0,0,1,0,0,0,0,0}; /* Protected */
                    for ( y = 0 ; y < ys ; y++ )
                        for ( x = 0 ; x < xs ; x++ ) {
                            vta = VscrnGetVtCharAttr( VTERM, x, y ) ;
                            vta.unerasable = TRUE ;
                            VscrnSetVtCharAttr( VTERM, x, y, vta ) ;
                        }
                    }
                break;
            case ',': {
                /* 955: Clear unprotected to write protected spaces      */
                /* (reset protect mode)                                  */
                /* 950: Clear unprotected page to write-protected spaces */
                /* (do not reset protect mode)                           */
                int xs = VscrnGetWidth(VTERM);
                int ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                vtattrib vta = {0,0,0,0,0,1,0,0,0,0,0}; /* Protected */

                if ( debses )
                    break;
                clrscreen( VTERM, SP );
                lgotoxy(VTERM,1,1);       /* and home the cursor */
                for ( y = 0 ; y < ys ; y++ )
                    for ( x = 0 ; x < xs ; x++ ) {
                        vta = VscrnGetVtCharAttr( VTERM, x, y ) ;
                        vta.unerasable = TRUE ;
                        VscrnSetVtCharAttr( VTERM, x, y, vta ) ;
                    }
                break;
            }
            case '-': {
                /* Moves the cursor to a specified text segment. */
                /* This has a multiple code sequence of:         */
                /* ESC - prc                                     */
                /* where p = the page                            */
                /*       r = the row code                        */
                /*       c = the column code                     */
                /* TVI950 */
                int p,r,c ;
                p = tviinc();

                r = tviinc()-31;

                c = tviinc();
                if ( c == '~' )
                    c = tviinc() - 31 + 80 ;
                else
                    c -= 31 ;

                if ( debses )
                    break;
                if ( p == WY_FIELD_APPL )
                    lgotoxy(VTERM,c,r);
                break;
            }
            case '.': {
                if ( ISTVI910(tt_type_mode) )
                {
                    /* TVI910 - Toggle Cursor On/Off */
                    if ( debses )
                        break;
                    cursorena[VTERM] = !cursorena[VTERM] ;
                }
                else if ( ISTVI950(tt_type_mode) ||
                          ISTVI925(tt_type_mode) ) {
                    /* TVI950 - Set Cursor Type */
                    int n = tviinc() ;
                    if ( debses )
                        break;
                    switch ( n ) {
                    case '0':
                        /* Cursor display off */
                        cursorena[VTERM] = FALSE ;
                        break;
                    case '1':
                        /* Blinking block cursor (default) */
                        tt_cursor = TTC_BLOCK ;
                        setcursormode() ;
                        cursorena[VTERM] = TRUE ;
                        break;
                    case '2':
                        /* Steady block cursor */
                        tt_cursor = TTC_BLOCK ;
                        setcursormode() ;
                        cursorena[VTERM] = TRUE ;
                        break;
                    case '3':
                        /* Blinking line cursor */
                        tt_cursor = TTC_ULINE ;
                        setcursormode() ;
                        cursorena[VTERM] = TRUE ;
                        break;
                    case '4':
                        /* Steady line cursor */
                        tt_cursor = TTC_ULINE ;
                        setcursormode() ;
                        cursorena[VTERM] = TRUE ;
                        break;
                    }
                }
                break;
            }
            case '/': {
                if ( debses )
                    break;

                if ( ISTVI925(tt_type_mode) ||
                     ISTVI950(tt_type_mode) ) {
                         /* Transmits the active text segment number and */
                         /* cursor address. */
                         char buf[5] ;

                         buf[0] = 0;
                         buf[1] = wherey[VTERM]+31 ;
                         buf[2] = wherex[VTERM]+31 ;
                         buf[3] = CK_CR ;
                         buf[4] = NUL ;

                         sendchars( buf, 4 ) ;
                     }
                break;
            }
            case '0': {
                /* 950 - Program the SEND key (unsupported)   */
                /* 955 - ESC 0 Ps P1 P2 P3                    */
                /*       Reprogram any individual editing key */
                if ( ISTVI950(tt_type_mode) ) {
                    int p1, p2, k=-1;

                    if ((p1 = tviinc()) < 0)
                        break;
                    if ((p2 = tviinc()) < 0)
                        break;
                    if ( debses )
                        break;
                    switch ( p1 ) {
                    case '0':   /* Shift Send */
                        k = K_TVISSEND;
                        break;
                    case '1':   /* Send */
                        k = K_TVISEND;
                        break;

                    }
                    if ( k < 0 )
                        break;
                    if ( udkfkeys[k-K_TVIF01] ) {
                        free(udkfkeys[k-K_TVIF01]);
                        udkfkeys[k-K_TVIF01] = NULL;
                    }
                    switch ( p2 ) {
                    case '4':
                        udkfkeys[k-K_TVIF01] = strdup("\0334");
                        break;
                    case '5':
                        udkfkeys[k-K_TVIF01] = strdup("\0335");
                        break;
                    case '6':
                        udkfkeys[k-K_TVIF01] = strdup("\0336");
                        break;
                    case '7':
                        udkfkeys[k-K_TVIF01] = strdup("\0337");
                        break;
                    case 'S':
                        udkfkeys[k-K_TVIF01] = strdup("\033S");
                        break;
                    case 's':
                        udkfkeys[k-K_TVIF01] = strdup("\033s");
                        break;
                    }
                }
                break;
            }
            case '1':
                /* Sets a tab stop */
                if ( debses )
                    break;
                if ( ISTVI910(tt_type_mode) ||
                     ISTVI950(tt_type_mode) )
                    htab[wherex[VTERM]] = 'T';
                break;
            case '2':
                /* Clears a tab stop */
                if ( debses )
                    break;
                if ( ISTVI910(tt_type_mode) ||
                     ISTVI950(tt_type_mode) )
                    htab[wherex[VTERM]] = '0';
                break;
            case '3':
                /* Clears all tab stops */
                if ( debses )
                    break;
                if ( ISTVI910(tt_type_mode) ||
                     ISTVI950(tt_type_mode) )
                    for (j = 1; j <=MAXTERMCOL; ++j)
                        htab[j] = '0';
                break;
            case '4': {  /* Send line unprotected */
                int x, fs=1 ;

                debug(F110,"TVI Escape","Send all unprotected chars from BOL",0);
                if ( debses )
                    break;

                for ( x=0;x<=wherex[VTERM]-1;x++ ) {
                    if ( !VscrnGetVtCharAttr(VTERM, x, wherey[VTERM]-1).unerasable ) {
                        int ch = VscrnGetCell( VTERM, x, wherey[VTERM]-1 )->c ;
                        if ( ch || !wy_nullsuppress ) {
                            if ( tt_senddata ) {
                                unsigned char * bytes;
                                int nbytes;
                                if ( ck_isunicode() )
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
                        /* Send field delimeter */
                        if ( tvi_send_field[0] )
                            sendchars(tvi_send_field,strlen(tvi_send_field));
                        fs = 1 ;
                    }
                }
                /* Send End of Text delimeter */
                if ( tvi_send_end_of_text[0] )
                    sendchars(tvi_send_end_of_text,strlen(tvi_send_end_of_text));
                break;
            }
            case '5': {  /* Send page unprotected */
                int x,y,fs=1 ;

                debug(F110,"TVI Escape","Send all unprotected chars from BOS",0);
                if ( debses )
                    break;

                for ( y=0;y<=wherey[VTERM]-1; y++ ) {
                    for ( x=0;x<=wherex[VTERM]-1;x++ )
                        if ( !VscrnGetVtCharAttr(VTERM, x, y).unerasable ) {
                            int ch = VscrnGetCell( VTERM, x, y )->c ;
                            if ( ch || !wy_nullsuppress ) {
                                if ( tt_senddata ) {
                                    unsigned char * bytes;
                                    int nbytes;
                                    if ( ck_isunicode() )
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
                            /* Send field delimeter */
                            if ( tvi_send_field[0] )
                                sendchars(tvi_send_field,strlen(tvi_send_field));
                            fs = 1 ;
                        }
                    if ( y < wherey[VTERM]-1 ) {
                        /* Send end of line delimeter */
                        if ( tvi_send_line[0] )
                            sendchars(tvi_send_line,strlen(tvi_send_line));
                    }
                    /* Send End of Text delimeter */
                    if ( tvi_send_end_of_text[0] )
                        sendchars(tvi_send_end_of_text,strlen(tvi_send_end_of_text));
                }
                break;
            }
            case '6': {  /* Send line all */
                int x, fs=0, ch=0 ;

                debug(F110,"TVI Escape","Send all chars from BOL",0);
                if ( debses )
                    break;

                for ( x=0;x<=wherex[VTERM]-1;x++ ) {
                    if ( !VscrnGetVtCharAttr(VTERM, x, wherey[VTERM]-1).unerasable ) {
                        if ( fs ) {
                            if ( tvi_send_end_protected_field[0] )
                                sendchars(tvi_send_end_protected_field,
                                           strlen(tvi_send_end_protected_field));
                            fs = 0;
                        }
                    }
                    else if ( !fs ) {
                        if ( tvi_send_start_protected_field[0] )
                            sendchars(tvi_send_start_protected_field,
                                       strlen(tvi_send_start_protected_field));
                        fs = 1 ;
                    }
                    ch = VscrnGetCell( VTERM, x, wherey[VTERM]-1 )->c ;
                    if ( ch  || !wy_nullsuppress ) {
                        if ( tt_senddata ) {
                            unsigned char * bytes;
                            int nbytes;
                            if ( ck_isunicode() )
                                nbytes = utorxlat(ch,&bytes);
                            else
                                nbytes = utorxlat(ch,&bytes);
                            while ( nbytes-- > 0 )
                                sendchar(*bytes++) ;
                        } else
                            sendchar(SP);
                    }
                }
                /* Send End of Text delimeter */
                if ( tvi_send_end_of_text[0] )
                    sendchars(tvi_send_end_of_text,strlen(tvi_send_end_of_text));
                break;
            }
            case '7': {  /* Send page all */
                int x,y,fs=0,ch=0;

                debug(F110,"TVI Escape","Send all chars from BOS",0);
                if ( debses )
                    break;

                for ( y=0;y<=wherey[VTERM]-1; y++ ) {
                    for ( x=0;x<=wherex[VTERM]-1;x++ ) {
                        if ( !VscrnGetVtCharAttr(VTERM, x, y).unerasable ) {
                            if ( fs ) {
                                if ( tvi_send_end_protected_field[0] )
                                    sendchars(tvi_send_end_protected_field,
                                               strlen(tvi_send_end_protected_field));
                                fs = 0;
                            }
                        }
                        else if ( !fs ) {
                            if ( tvi_send_start_protected_field[0] )
                                sendchars(tvi_send_start_protected_field,
                                           strlen(tvi_send_start_protected_field));
                            fs = 1 ;
                        }
                        ch = VscrnGetCell( VTERM, x, y )->c;
                        if ( ch || !wy_nullsuppress ) {
                            if ( tt_senddata ) {
                                unsigned char * bytes;
                                int nbytes;
                                if ( ck_isunicode() )
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
                        /* Send end of line delimeter */
                        if ( tvi_send_line[0] )
                            sendchars(tvi_send_line,strlen(tvi_send_line));
                    }
                }
                /* Send End of Text delimeter */
                if ( tvi_send_end_of_text[0] )
                    sendchars(tvi_send_end_of_text,strlen(tvi_send_end_of_text));
                break;
            }
            case '8':
                /* Smooth Scroll On */
                if ( debses )
                    break;
                if ( ISTVI950(tt_type_mode) )
                    SmoothScroll() ;
                break;
            case '9':
                /* Smooth Scroll Off */
                if ( debses )
                    break;
                if ( ISTVI950(tt_type_mode) )
                    JumpScroll() ;
                break;
            case ':':
                if ( debses )
                    break;
                if ( ISTVI925(tt_type_mode) ||
                     ISTVI950(tt_type_mode) ) {
                    /* Clear unprotected page to NULs */
                    selclrscreen( VTERM, NUL );
                    lgotoxy(VTERM,1,1);       /* and home the cursor */
                    if ( writeprotect ) {
                        int xs = VscrnGetWidth(VTERM);
                        int ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                        vtattrib vta = {0,0,0,0,0,1,0,0,0,0,0}; /* Protected */
                        for ( y = 0 ; y < ys ; y++ )
                            for ( x = 0 ; x < xs ; x++ ) {
                                vta = VscrnGetVtCharAttr( VTERM, x, y ) ;
                                vta.unerasable = TRUE ;
                                VscrnSetVtCharAttr( VTERM, x, y, vta ) ;
                        }
                    }
                }
                break;
            case ';':
                if ( debses )
                    break;
                if ( ISTVI925(tt_type_mode) ||
                     ISTVI950(tt_type_mode) ) {
                    /* Clear unprotected page to Insert Character */
                    selclrscreen( VTERM, tvi_ic );
                    lgotoxy(VTERM,1,1);       /* and home the cursor */
                    if ( writeprotect ) {
                        int xs = VscrnGetWidth(VTERM);
                        int ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                        vtattrib vta = {0,0,0,0,0,1,0,0,0,0,0}; /* Protected */
                        for ( y = 0 ; y < ys ; y++ )
                            for ( x = 0 ; x < xs ; x++ ) {
                                vta = VscrnGetVtCharAttr( VTERM, x, y ) ;
                                vta.unerasable = TRUE ;
                                VscrnSetVtCharAttr( VTERM, x, y, vta ) ;
                        }
                    }
                }
                break;
            case '<':
                /* Turn off keyclick */
                if ( debses )
                    break;
                if ( ISTVI925(tt_type_mode) ||
                     ISTVI950(tt_type_mode) )
                    setkeyclick(FALSE);
                break;
            case '=': {
                /* Moves the cursor to a specified row and column for */
                /* an 80-column screen.  This has a format of:        */
                /*    ESC = rc where                                  */
                /* r = the row code; and c = the column code          */
                int r,c ;
                r = tviinc() ;

                if ( r < SP ) {
                    escstate = ES_NORMAL ;
                    tviascii(r) ;
                    return ;
                }
                c = tviinc() ;

                if ( c < SP ) {
                    escstate = ES_NORMAL ;
                    tviascii(c) ;
                    return ;
                }
                if ( debses )
                    break;
                if ( /* VscrnGetWidth(VTERM) == 80 && */ c > 31 && r > 31 )
                     lgotoxy(VTERM,c-31,r-31);
                else if ( VscrnGetWidth(VTERM) == 132 && c > 31 && r > 31 )
                     lgotoxy(VTERM,c-31,r-31);
                break;
            }
            case '>':
                /* Turn on keyclick */
                if ( debses )
                    break;
                if ( ISTVI925(tt_type_mode) ||
                     ISTVI950(tt_type_mode) )
                    setkeyclick(TRUE);
                break;
            case '?':
                /* Transmits the cursor address for the active text    */
                /* segment of an 80-column screen only.  The format is */
                /*    rc CR where r = the row code; and c = the column */
                /* code.                                               */
                if ( debses )
                    break;
                if ( 1 /* VscrnGetWidth(VTERM) == 80 */ ) {
                    char buf[4] ;
                    buf[0] = (char)(wherey[VTERM]+31) ;
                    buf[1] = (char)(wherex[VTERM]+31) ;
                    buf[2] = CK_CR ;
                    buf[3] = NUL ;
                    sendchars(buf,3) ;
                }
                break;
            case '@':
                /* Copy print mode on */
                if ( debses )
                    break;
                cprint = TRUE ;
                if ( !printon )
                    printeron() ;
                break;
            case 'A': {
                /* Turn copy-print mode off */
                if ( debses )
                    break;
                cprint = FALSE ;
                if ( !cprint && !uprint && !xprint && !aprint && printon )
                    printeroff();
                break;
            }
            case 'B':
                /* Places the terminal in the block mode.                  */
                if ( debses )
                    break;
                if ( ISTVI950(tt_type_mode) )
                    wy_block = TRUE;
                break;
            case 'C':
                /* Conversation mode; block mode off */
                if ( debses )
                    break;
                if ( ISTVI950(tt_type_mode) )
                    wy_block = FALSE;
                break;
            case 'D': {
                /* Selects the full duplex or half-duplex conversation   */
                /* modes.  This has the multiple code sequence:          */
                /*   ESC D x where x = F full duplex mode; H half duplex */
                int n = tviinc();

                if ( debses )
                    break;
                switch ( n ) {
                case 'F':
                    if ( duplex_sav < 0 )
                        duplex_sav = duplex ;
                    duplex = FALSE ;
                    break;
                case 'H':
                    if ( duplex_sav < 0 )
                        duplex_sav = duplex ;
                    duplex = TRUE ;
                    break;
                }
                break;
            }
            case 'E':
                /* Inserts a row of spaces */
                if ( debses )
                    break;
                if ( ISTVI925(tt_type_mode) ||
                     ISTVI950(tt_type_mode) ) {
                    if ( !protect )
                        VscrnScroll(VTERM, DOWNWARD, wherey[VTERM] - 1,
                                     marginbot - 1, 1, FALSE, SP);
                }
                break;
            case 'F':
                if ( debses )
                    break;
                if ( ISTVI910(tt_type_mode) ) {
                    /* Display control character */
                    /*int n =*/ tviinc() ;
                }
                else {
                    /* TVI 955 */
                    /* 0 - Attributes occupy a space */
                    /* 1 - Attributes do not occupy a space */
                    int n = tviinc() ;
                    switch ( n ) {
                    case '0':
                        tt_hidattr = FALSE ;
                        break;
                    case '1':
                        tt_hidattr = TRUE ;
                        break;
                    default:
                        break;
                    }
                }
                break;
            case 'G': {
                /* Sets a video attribute within the application display  */
                /* area.  The attribute occupies a space.  This has a     */
                /* multiple code sequence of:   ESC G ATTR where:         */
                /*    ATTR = attribute code.                              */
                /*
                950: Designating an embedded attribute when the cursor
                is in a protected field changes the attribute at the cursor
                position and destroys the character in that position.

                955: Designating an embedded attribute when the cursor
                is in a protected field moves the cursor to the next
                unprotected position and then changes the attribute.
                */

                int n = tviinc() ;

                if ( debses )
                    break;
                switch ( n ) {
                case SP:
                    wrtch(SP) ;
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
                    else        /* Character Mode */
                        attrib = attr ;
                }
                }
                break;
            }
            case 'H': {
                if ( ISTVI910(tt_type_mode) ) {
                    /* Auto scroll on/off */
                    ;
                }
                else /* if ( ISTVI925(tt_type_mode) ) */ {
                    /* Draw a box - 955 feature */
                    int w,h;
                    w = tviinc();
                    if ( w == '~' ) {
                        w = tviinc() - 31 + 80;
                        h = tviinc() - 31 ;
                    }
                    else {
                        w -= 31 ;
                        h = tviinc() - 31;
                    }
                    if ( debses )
                        break;
                    boxrect_escape( VTERM, wherey[VTERM]+h,
                                    wherex[VTERM]+w ) ;
                }
                break;
            }
            case 'I':
                /* Moves the cursor left to the previous tab stop */
                if ( debses )
                    break;
                wyse_backtab();
                break;
            case 'J':
                /* Display previous page */
                break;
            case 'K':
                /* Display next page */
                break;
            case 'L':
                /* Sends all characters unformatted to the auxiliary port. */
                /* Attribute codes are sent as spaces.  Row-end            */
                /* sequences are not sent.                                 */
                /* TVI950  */
                break;
            case 'M':
                if ( debses )
                    break;
                if ( ISTVI910(tt_type_mode) ) {
                    /* Send current cursor character to the host */
                    break;
                }
                else {
                    /* Reports the terminal identification to the */
                    /* Host computer */
                    sendchars( tt_info[tt_type].x_id,
                               strlen(tt_info[tt_type].x_id) ) ;
                    /* 955 sends "955 R.0,m\r" in 955 mode */
                    /* and "R.0,m\r" when in 950 mode.     */
                }
                break;
            case 'N':
                /* TVI950 - Page edit mode on */
                break;
            case 'O':
                /* TVI950 - Page edit mode off (default) */
                break;
            case 'P':
                /* Sends all protected and unprotected characters to the  */
                /* auxiliary port, regardless of the mode setting.        */
                /* Send ACK when done */
                /* 925, 950 */
                /* 914, 924 use an extra parameter to choose various modes */
                if ( debses )
                    break;

                if ( ISTVI925(tt_type_mode) || ISTVI950(tt_type_mode) ) {
                    int x;
                    x = xprintff; xprintff = 0; /*   Print/Dump current screen */
                    prtscreen(VTERM,1,VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0));
                    xprintff = x;
                    ttoc(ACK);
                }
                break;
            case 'Q':
                /* Inserts a space. */
                if ( debses )
                    break;
                if ( ISTVI925(tt_type_mode) ||
                     ISTVI950(tt_type_mode) ) {
                    blankvcell.c = SP;
                    blankvcell.a = geterasecolor(VTERM);
                    VscrnScrollRt(VTERM, wherey[VTERM] - 1,
                               wherex[VTERM] - 1, wherey[VTERM] - 1,
                               VscrnGetWidth(VTERM) - 1, 1, blankvcell);
                }
                break;
            case 'R':
                /* Deletes a row. */
                if ( debses )
                    break;
                if ( (ISTVI925(tt_type_mode) ||
                     ISTVI950(tt_type_mode)) &&
                     !protect ) {
                    VscrnScroll(VTERM,
                             UPWARD,
                             wherey[VTERM] - 1,
                             marginbot - 1,
                             1,
                             FALSE,
                             SP);
                }
                break;
            case 'S': {
                /* TVI950 - Sends a message unprotected */
                /* FIX THIS --- this is code from Wyse, modify for TVI */
                if ( ISTVI950(tt_type_mode) ) {
                    int x,y,ch,
                    w=VscrnGetWidth(VTERM),
                    h=VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0),
                    xs=-1,
                    ys=-1,
                    xe=w-1,
                    ye=h-1,
                    curx=0,
                    cury=0,
                    fs=1 ;

                    debug(F110,"TVI Escape","Send a message unprotected",0);
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
                                curx = xe = x-1 ;
                                cury = ye = y ;
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
                                        if ( ck_isunicode() )
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
                                /* Send field delimeter */
                                if ( tvi_send_field[0] )
                                    sendchars(tvi_send_field,strlen(tvi_send_field));
                                fs = 1 ;
                            }
                        if ( y < ye ) {
                            /* Send End of Line delimeter */
                            if ( tvi_send_field[0] )
                                sendchars(tvi_send_line,strlen(tvi_send_line));
                        }
                    }
                    /* Send End of Text delimeter */
                    if ( tvi_send_end_of_text[0] )
                        sendchars(tvi_send_end_of_text,strlen(tvi_send_end_of_text));

                    lgotoxy(VTERM,curx+1,cury+1);   /* Reposition cursor */
                }
#ifdef COMMENT
                else if ( ISTVI924() ) {
                    int n = tviinc();
                    if ( debses || !tt_senddata )
                        break;
                    switch ( n ) {
                    case '1':
                    case '2':
                    case '3':
                    case '5':
                    case '6':
                    case '7':
                    case '9':
                    case ':':
                    case ';':
                    case '?':
                        ;
                    }
                }
#endif /* COMMENT */
                break;
            }
            case 'T':
                /* clrtoeol with insert character */
                if ( debses )
                    break;
                selclrtoeoln( VTERM, tvi_ic );
                break;
            case 'U':
                /* Turns the monitor submode on */
                if ( debses || wy_monitor )
                    break;
                setdebses(TRUE);
                wy_monitor = TRUE ;
                break;
            case 'V':
                if ( debses )
                    break;
                    /* Start self test */
                    ;
#ifdef COMMENT
                /* This code is from the Wyse terminals documentation   */
                /* But it appears to be completely wrong.  The sequence */
                /* appears in the EFS, Inc. logs, but I can't figure    */
                /* out what it is supposed to do.                       */

                /* According to the Televideo 955 manual, this sequence */
                /* is defined as start self-test for all TVI teminals.  */
                else
                {
                    /* Clear cursor column */
                    viocell cell = {SP,geterasecolor(VTERM)};
                    int ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                    vtattrib vta = attrib;
                    x  = wherex[VTERM]-1 ;
                    vta.unerasable = TRUE ;
                    for ( y=wherey[VTERM]-1 ; y<ys ; y++ )
                        VscrnWrtCell( VTERM, cell, vta, y, x ) ;
                    cursorright(0);
                }
#endif /* COMMENT */
                break;
            case 'W':
                /* Deletes a character */
                if ( debses )
                    break;
                if ( ISTVI925(tt_type_mode) ||
                     ISTVI950(tt_type_mode) ) {
                    blankvcell.c = SP;
                    blankvcell.a = geterasecolor(VTERM);
                    VscrnScrollLf(VTERM, wherey[VTERM] - 1,
                               wherex[VTERM] - 1,
                               wherey[VTERM] - 1,
                               VscrnGetWidth(VTERM) - 1,
                               1, blankvcell) ;
                }
                break;
            case 'X':
                /* Turns the monitor submode off */
                if ( !wy_monitor )
                    break;
                setdebses(FALSE);
                wy_monitor = FALSE ;
                break;
            case 'Y':
                if ( debses )
                    break;
                {
                    /* clear unprotected page to space characters */
                    /* from cursor */
                    if ( ISTVI950(tt_type_mode) )
                        selclreoscr_escape( VTERM, tvi_ic ) ;
                    else {
                        clreoscr_escape( VTERM, tvi_ic ) ;
                    }
                }
                break;
            case 'Z': {
                /* When n = 0, transmit user line to host */
                /* When n = 1, transmit status line to host */
                /* 950 */
                int n = tviinc() ;

                /* The EFS logs contain a sequence ESC Z b 3       */
                /* which results in the '3' being printed to the   */
                /* screen in our emulation but not in the Wyse 60  */
                /* tvi emulations.                                 */
                /* It also has ESC Z @ 0 and ESC Z @ 1 so I think  */
                /* the 925 has an extended family of commands here */
                switch ( n ) {
                case 0: /* Transmit User line to host */
                    if ( ISTVI950(tt_type_mode) )
                        ;
                    break;
                case 1: /* Transmit Status line to host */
                    if ( ISTVI950(tt_type_mode) )
                        ;
                    break;
                case '@': {     /* Change width? */
                    /* This is not defined for any TVI terminal according  */
                    /* to the TVI 965 manual Appendix C.  However, we will */
                    /* leave it in until someone tells us that it gets in  */
                    /* the way.                                            */
                    int m = tviinc() ;
                    switch ( m ) {
                    case '0':   /* 80 cols; turn off write protect ?? */
                        /* writeprotect = FALSE ; */
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
                    case '1':   /* 132 cols; turn off write protect ?? */
                        /* writeprotect = FALSE ; */
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
                    }
                    break;
                }
#ifdef COMMENT
                case 'b': /* Have no idea */
                    /* This case comes from the EFS, Inc. logs but is not  */
                    /* defined for any Televideo terminal according to the */
                    /* TVI 965 manual Appendix C.                          */
                    if ( ISTVI925(tt_type_mode) )
                        n == tviinc() ;
                    break;
#endif /* COMMENT */
                }
                break;
            }
            case '[':
                if ( ISTVI950(tt_type_mode) ) {
                    /* TVI950 - Address cursor to row */
                    int row = tviinc() ;

                    if ( debses )
                        break;
                    lgotoxy(VTERM,wherex[VTERM],row-31);
                }
                else {
                    /* 955 - Command sequence introducer (ANSI format) */
                    ;
                }
                break;
            case '\\': {
                /* Set page size */
                int n = tviinc() ;
                switch ( n ) {
                case '1':
                    /* 24 lines */
                    break;
                case '2':
                    /* 48 lines */
                    break;
                case '3':
                    /* 96 lines */
                    break;
                }
                break;
            }
            case ']':
                if ( ISTVI950(tt_type_mode) ) {
                    /* TVI950 - Address cursor to column */
                    int col = tviinc() ;

                    if ( debses )
                        break;
                    lgotoxy(VTERM,col-31,wherey[VTERM]);
                }
                else {
                    /* 955 - Reprogram all editing keys */
                    ;
                }
                break;
            case '^': {
                char msg[17];
                int  ch2, n=0 ;
                ch2 = tviinc() ;

                switch ( ch2 ) {
                case '0':
                    if (tt_answer)
                        sendchars(answerback,strlen(answerback)) ;
                    break;
                case '1': {
                    /* Program the Answerback message (not allowed) */
                    for ( n=0; n<16 ; n++ ) {
                        msg[n] = tviinc() ;
                        if ( msg[n] == DLE ) { /* Quote next */
                            msg[n] = tviinc() ;
                        }
                        if ( msg[n] == XEM ) {
                            msg[n] = NUL ;
                            break;
                        }
                    }
                    break;
                }
                }
            }
            case '_':
                break;
            case '`': {
                /* Transparent print on */
                if ( debses )
                    break;
                xprint = TRUE ;
                if ( !printon )
                    printeron() ;
                break;
            }
            case 'a':
                /* Turn transparent print mode off */
                if ( debses )
                    break;
                xprint = FALSE ;
                if ( !cprint && !uprint && !xprint && !aprint && printon )
                    printeroff();
                break;
            case 'b':
                /* Reverse Background */
                if ( debses )
                    break;
                if (decscnm) /* Already reverse */
                    break;
                else {
                    flipscreen(VTERM);
                    VscrnIsDirty(VTERM) ;
                }
                break;
            case 'c':
                /* TVI925,TVI910+ - Block mode on */
                /* TVI950         - Set Local Mode */
                break;
            case 'd':
                /* Normal background */
                if ( debses )
                    break;
                if (!decscnm) /* Already normal? */
                    break;
                else {
                    flipscreen(VTERM);
                    VscrnIsDirty(VTERM);
                }
                break;
            case 'e':
                if ( debses )
                    break;
                if ( ISTVI950(tt_type_mode) ) {
                    int n = tviinc() ;
                    tvi_ic = n ;
                }
                break;
            case 'f': {
                /* TVI 925 */
                /* TVI 950 */
                /* Load user line with upto 80 characters */
                int i=0,
                w = VscrnGetWidth(VTERM);

                if ( ISTVI925(tt_type_mode) ||
                     ISTVI950(tt_type_mode) ) {
                    if (debses) {
                        while ( i < w &&
                                tviinc() != CK_CR ) {
                            i++;
                        }
                    }
                    else {
                        while ( i < w &&
                                (tviuserline[i]=tviinc()) != CK_CR ) {
                            i++;
                        }
                        while ( i < MAXTERMCOL )
                            tviuserline[i++] = SP ;
                        tviuserline[i]=NUL ;
                        if ( decssdt == SSDT_HOST_WRITABLE ) {
                            VscrnSetWidth(VSTATUS,VscrnGetWidth(VTERM));
                            VscrnWrtCharStrAtt( VSTATUS, tviuserline, strlen(tviuserline),
                                                1, 1, &colorstatus ) ;
                        }
                    }
                }
                break;
            }
            case 'g':
                if ( debses )
                    break;
                /* 925/950 - Turn on user line on line 25 */
                if ( ISTVI925(tt_type_mode) || ISTVI950(tt_type_mode) ) {
                    setdecssdt(SSDT_HOST_WRITABLE);
                    VscrnSetWidth(VSTATUS,VscrnGetWidth(VTERM));
                    VscrnWrtCharStrAtt( VSTATUS, tviuserline, strlen(tviuserline),
                                        1, 1, &colorstatus ) ;
                }
                break;
            case 'h':
                if ( debses )
                    break;
                /* 925/950 - Turn on status line on line 25 */
                if ( ISTVI925(tt_type_mode) || ISTVI950(tt_type_mode) )
                    setdecssdt(SSDT_INDICATOR);
                break;
            case 'i':
                /* Moves the cursor to the next tab stop on the right.    */
                /* Same as Horizontal Tab control character               */
                wyse_tab();
                break;
            case 'j':
                /* Moves the cursor up one row and begins scrolling at    */
                /* top row.                                               */
                if ( debses )
                    break;
                cursorup(0) ;
                break;
            case 'k':
                /* Turns the local edit submode on; duplex off */
                /* Not the same as ECHO mode */
                break;
            case 'l':
                /* Turns the duplex edit submode on; local off */
                /* Not the same as ECHO mode */
                break;
            case 'm':
                /* 955 - Enable optional graphics firmware */
                break;
            case 'n':
                /* Turn on screen display */
                if ( debses )
                    break;
                screenon = TRUE ;
                break;
            case 'o':
                /* Turn off screen display */
                if ( debses )
                    break;
                screenon = FALSE ;
                break;
            case 'p':
                /* 955 - Ps define the page print terminator */
                break;
            case 'q':
                /* TVI950 - Turns the insert submode on; replace off */
                if ( debses )
                    break;
                if ( ISTVI950(tt_type_mode) )
                    insertmode = TRUE ;
                break;
            case 'r':
                /* TVI950 - Turns the insert submode off; replace on */
                if ( debses )
                    break;
                if ( ISTVI950(tt_type_mode) )
                    insertmode = FALSE ;
                break;
            case 's': {
                /* TVI950 - Sends a message (entire block) */
                if ( ISTVI950(tt_type_mode) ) {
                    int x,y,ch,
                    w=VscrnGetWidth(VTERM),
                    h=VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0),
                    xs=-1,
                    ys=-1,
                    xe=w-1,
                    ye=h-1,
                    curx=0,
                    cury=0,
                    fs=0 ;

                    debug(F110,"TVI Escape","Send a message (entire block)",0);
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
                                curx = xe = x-1 ;
                                cury = ye = y ;
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
                                    if ( tvi_send_end_protected_field[0] )
                                        sendchars(tvi_send_end_protected_field,
                                                   strlen(tvi_send_end_protected_field));
                                    fs = 0;
                                }
                            }
                            else if ( !fs ) {
                                if ( tvi_send_start_protected_field[0] )
                                    sendchars(tvi_send_start_protected_field,
                                               strlen(tvi_send_start_protected_field));
                                fs = 1 ;
                            }
                            ch = VscrnGetCell( VTERM, x, y )->c;
                            if ( ch || !wy_nullsuppress ) {
                                if ( tt_senddata ) {
                                    unsigned char * bytes;
                                    int nbytes;
                                    if ( ck_isunicode() )
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
                            /* Send End of Line delimeter */
                            if ( tvi_send_field[0] )
                                sendchars(tvi_send_line,strlen(tvi_send_line));
                        }
                    }
                    /* Send End of Text delimeter */
                    if ( tvi_send_end_of_text[0] )
                        sendchars(tvi_send_end_of_text,strlen(tvi_send_end_of_text));

                    lgotoxy(VTERM,curx+1,cury+1);   /* Reposition cursor */
                }
#ifdef COMMENT
                else if ( ISTVI924 ) {
                    int n = tviinc();
                    if ( debses || !tt_senddata )
                        break;
                    switch ( n ) {
                    case '1':
                    case '2':
                    case '3':
                    case '5':
                    case '6':
                    case '7':
                    case '9':
                    case ':':
                    case ';':
                    case '?':
                        ;
                    }
                }
#endif /* COMMENT */
                break;
            }
            case 't':
                /* Erases all (unprotected) characters from the current   */
                /* cursor location to the end of the row and replaces     */
                /* them with nulls.                                       */
                if ( debses )
                    break;
                if ( protect )
                    selclrtoeoln( VTERM, NUL ) ;
                else
                    clrtoeoln( VTERM, NUL );
                break;
            case 'u':
                /* Turns the monitor submode off.                         */
                if ( !wy_monitor )
                    break;
                setdebses(FALSE);
                wy_monitor = FALSE ;
                break;
            case 'v':
                /* Turn on autopage mode */
                if ( debses )
                    break;
                wy_autopage = TRUE ;
                break;
            case 'w':
                /* Turn off autopage mode */
                if ( debses )
                    break;
                wy_autopage = FALSE ;
                break;
            case 'x': {
                /* TVI950 - Define SEND delimiters (12-33) */
                /* ESC x Dlmtr Char1 Char2            */
                int n, p1, p2 ;
                if ((n = tviinc()) < 0)
                    break;
                if ((p1 = tviinc()) < 0 )
                    break;
                if ((p2 = tviinc()) < 0)
                    break;
                if ( debses )
                    break;
                switch ( n ) {
                case '0':
                    tvi_send_field[0] = (CHAR)(p1 & 0xFF);
                    tvi_send_field[1] = (CHAR)(p2 & 0xFF);
                    break;
                case '1':
                    tvi_send_line[0] = (CHAR)(p1 & 0xFF);
                    tvi_send_line[1] = (CHAR)(p2 & 0xFF);
                    break;
                case '2':
                    tvi_send_start_protected_field[0] = (CHAR)(p1 & 0xFF);
                    tvi_send_start_protected_field[1] = (CHAR)(p2 & 0xFF);
                    break;
                case '3':
                    tvi_send_end_protected_field[0] = (CHAR)(p1 & 0xFF);
                    tvi_send_end_protected_field[1] = (CHAR)(p2 & 0xFF);
                    break;
                case '4':
                    tvi_send_end_of_text[0] = (CHAR)(p1 & 0xFF);
                    tvi_send_end_of_text[1] = (CHAR)(p2 & 0xFF);
                    break;
                }
                break;
            }
            case 'y':
                /* Erases all characters from the current cursor location   */
                /* to the end of the active segment and replaces them with  */
                /* nulls. (unprotected)                                     */
                if ( debses )
                    break;
                if ( ISTVI950(tt_type_mode) && protect )
                    selclreoscr_escape( VTERM, NUL ) ;
                else
                    clreoscr_escape( VTERM, NUL ) ;
                break;
            case 'z':
                /* 950 - Execute User PROM program */
                break;
            case '{': {
                /* TVI950 - Set Comm1 parameters */
                /* Baud Stop Parity Word */
                int baud, stop, parity, word ;
                baud = tviinc() ;
                stop = tviinc() ;
                parity = tviinc() ;
                word = tviinc() ;
                if ( debses )
                    break;
                break;
            }
            case '|': {
                /* TVI 950 */
                /* ESC | key dir sequence Ctrl-Y =            */
                /*    Program key direction and definition    */
                /* The 955 uses different values for <key>    */
                /* in 955 and 950 modes.                      */
                char keydef[256] = "" ;
                int key = tviinc() ;
                int i = 0 ;
                int ctrlp = 0 ;
                /*int dir =*/ tviinc() ;

                while ( (keydef[i]=tviinc()) != XEM && !ctrlp )
                    if ( keydef[i] != DLE || ctrlp ) {
                        i++;
                        ctrlp = 0 ;
                    }
                    else {
                        ctrlp = 1 ;
                    }
                keydef[i] = NUL ;

                if ( debses )
                        break;
                /* we only allow redefinition of function keys */
                if ( key >= '1' && key <= ';' ) {
                    if ( udkfkeys[key-'1'] ) {
                        free( udkfkeys[key-'1'] ) ;
                        udkfkeys[key-'1']=NULL;
                    }
                    if ( strlen(keydef) )
                        udkfkeys[key-'1'] = strdup( keydef ) ;
                }
                else if ( key >= '<' && key <= 'F' ) {
                    if ( udkfkeys[key-'<'+16] ) {
                        free( udkfkeys[key-'<'+16] ) ;
                        udkfkeys[key-'<'+16]=NULL;
                    }
                    if ( strlen(keydef) )
                        udkfkeys[key-'<'+16] = strdup( keydef ) ;
                }
                break;
            }
            case '}': {
                /* TVI950 - Set Comm2 parameters */
                /* Baud Stop Parity Word */
                int baud, stop, parity, word ;
                baud = tviinc() ;
                stop = tviinc() ;
                parity = tviinc() ;
                word = tviinc() ;
                if ( debses )
                    break;
                break;
            }
            case '~': {
                int n = tviinc() ;
                if ( debses )
                    break;
                switch ( n ) {
                case '0':
                    /* 955 - Reset terminal to factory defaults */
                    break;
                case '1':
                    /* 955 - Reset terminal to non-volatile memory defaults */
                    break;
                case '2':
                    /* 955 - Reset function keys to factory defaults */
                    break;
                case '3':
                    /* 955 - Reset editing keys to factory defaults */
                    break;
                }
            }
                break;
            case DEL:
                break;
            }
            }
            else { /* xprint */
                switch ( ch ) {
                case 'a':
                    /* Turn transparent print mode off */
                    if ( debses )
                        break;
                    xprint = FALSE ;
                    if ( !cprint && !uprint && !xprint && !aprint && printon )
                        printeroff();
                    break;
                }
            }
            escstate = ES_NORMAL ;      /* Done parsing escstate sequence */
        }
    }
    else                /* Handle as a normal character */
    {
        if ( ch < SP )
            tvictrl(ch) ;
        else if ( !debses && !xprint ) {
            if ( tvigraphics )
                ch = xltvigrph( ch ) ;

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
                        if ( --x < 0 ) {
                            if ( --y >= 0 )
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
