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

#ifdef TCPSOCKET
#ifdef CK_NAWS
extern int tn_snaws();
#endif /* CK_NAWS */
#endif /* TCPSOCKET */

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
CHAR tvi_send_end_of_text[3] = { CR, NUL, NUL };

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
    if ( isunicode() ) {
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
    int i,j,x,y;

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
    case CR:
        if ( debses )
            break;
        wrtch((char) CR);
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
            wrtch((CHAR)CR);
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
    int i,j,k,n,x,y,z;
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
                    int attr ;
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
                         buf[3] = CR ;
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
                    buf[2] = CR ;
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
                    int n = tviinc() ;
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
                                tviinc() != CR ) {
                            i++;
                        }
                    }
                    else {
                        while ( i < w &&
                                (tviuserline[i]=tviinc()) != CR ) {
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
                int dir = tviinc() ;
                int i = 0 ;
                int ctrlp = 0 ;

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


#ifdef COMMENT
Article 3037 of comp.terminals:
Path: utkcs2!darwin.sura.net!europa.asd.contel.com!uunet!comp.vuw.ac.nz
!canterbury.ac.nz!phys169
From: phys169@csc.canterbury.ac.nz
Newsgroups: comp.terminals
Subject: Televideo control code table (was Re: TVI950)
Message-ID: <1992Apr23.112004.4789@csc.canterbury.ac.nz>
Date: 23 Apr 92 11:20:03 +1200
References: <1992Apr18.060736.29498@noose.ecn.purdue.edu>
            <1992Apr22.174647.4783@csc.canterbury.ac.nz>
Summary: Control codes/Escape sequences for TVI905 to TVI950
Keywords: Programming TVI950, TVI910, TVI912, TVI920, TVI914, TVI924, TVI925
Expires: 22 May 92 11:23:06 +1200
Organization: University of Canterbury, Christchurch, New Zealand
Lines: 224

In article <1992Apr22.174647.4783@csc.canterbury.ac.nz>,
 phys169@csc.canterbury.ac.nz writes:
>
> Hope the following is useful... NOTE that the 914/924 columns are NOT complete,
> they take a heck of a lot of typing, but there's probably enough there to get
> the magnitude of the difference.  I might follow up with a more complete table
> some day.

Well, there were a few mistakes in it (like ESC B), as well as the 914 & 924
columns not being finished.  Here's an updated version, but I don't have good
references for the TVI925 & 905, so if anyone can post corrections that would
be very nice.  The only Televideo source for the 925 I had was a little glossy
titled "The Value Leaders", which seems *very* wrong in places.

I still might follow with tables for other popular terminals, like the Wyse and
Qume and DG series.  The VT series has already been done by others, of course,
but I might post my collection (including differences betweem other ANSI
implementations, such as Heath/Zenith, TVI970 and Data General).

If anyone wants to cut out the following and stick it on some anonymous ftp
site, that is fine with me, but please keep my e-mail address on it so I can
be told of any errors or omissions.

*** CONTROL CODES FOR:          TVI950  910  912  920  914  924  925  905

^E       ENQ (Send ID msg)                                   y
^G       Beep                       y    y    y    y    y    y    y    y
^H       Cursor Left                y    y    y    y    y    y    y    y
^I       Tab                        y    y    y    y    y    y    y    y
^J       Line Feed                  y    y    y    y    y    y    y    y
^K       Cursor Up                  y    y    y    y    y    y    y    y
^L       Cursor Right               y    y    y    y    y    y    y    y
^M       Carriage Return            y    y    y    y    y    y    y    y
^N       Disable ^S/^Q              y         y    y    y    y
^O       Enable ^S/^Q               y         y    y    y    y
^R       Enable P3<->P4             y    y              y    y
^T       Disable P3<->P4            y    y              y    y
^V       Cursor Down                y                   y    y    y
^X       Clear unprot field                             y    y
^Z       Clear Unprot.              y    y    y    y    y    y    y    y
^^       Home                       y    y    y    y    y    y    y    y
^_       NewLine                    y    y    y    y    y    y    y    y

ESC ^B   insert Ctrl-B (STX)                            y    y
ESC ^C   insert Ctrl-C (ETX)                            y    y
ESC ESC  Display "EC" symbol        y
ESC ! 1  Linelock this line         y
ESC ! 2  Clear all locks            y
ESC "    Unlock keyboard            y    y    y    y    y    y    y    y
ESC #    Lock keyboard              y    y    y    y    y    y    y    y
ESC $    Graphics mode on           y                   y    y
ESC %    Graphics mode off          y                   y    y
ESC &    Protect mode on            y         y    y    y    y    y    y
ESC '    Protect mode off           y         y    y    y    y    y    y
ESC (    W-Prot off & dim           y    dim  y    y    y    y    y    y
ESC )    W-Prot on & bold           y    bold y    y    y    y    y    y
ESC *    Clear all to nulls         y    y    y    y              y    ?
ESC * 0  Clear all to nulls                             y    y
ESC * 1  Clear all to spaces                            y    y
ESC * 2  Clear unprot to nulls                          y    y
ESC * 3  Clear unprot to spaces                         y    y
ESC +    Clear unprot to ic*        y    y    y    y
ESC ,    Clear all to dim sp        y         y    y
ESC -    Move to page/row/col       y                        y    y
ESC .    Toggle cursor on/off            y    y    y
ESC . 0  Invisible cursor           y                   y    y
ESC . 1  Blinking block cursor      y                   y    y
ESC . 2  Steady block cursor        y         y    y    y    y
ESC . 3  Blinking underline         y         y    y    y    y
ESC . 4  Steady underline           y         y    y    y    y
ESC /    Report Cursor p/r/c        y                        y    y
ESC 0 0  Program Shift-Send key     y
ESC 0 1  Program Send key           y
ESC 0 @  Program Home key                                    y
ESC 0 P  Program Send key                               y    y
ESC 0 p  Program shift-Send key                         y    y
ESC 0 s  Program shift-ENTER key                             y
ESC 1    Set tab stop               y    y    y    y
ESC 2    Clear tab stop             y    y    y    y
ESC 3    Clear all tab stops        y    y    y    y
ESC 4    Send unprot from BOL       y         y    y
ESC 5    Send unprot from BOP       y         y    y
ESC 6    Send all from BOL          y         y    y
ESC 7    Send all from BOP          y         y    y
ESC 8    Smooth scroll mode on      y
ESC 8 0  Jump scroll mode on                                 y
ESC 8 1  Smooth scroll mode on                               y
ESC 9    Jump scroll mode on        y
ESC :    Clear unprot to null       y                             ?    ?
ESC ;    Clear unprot to ic*        y         y    y              ?    ?
ESC <    Keyclick off               y                             y
ESC < 0  Keyclick off                                   y    y
ESC < 1  Keyclick on                                    y    y
ESC >    Keyclick on                y                             y
ESC =    Move to row/column         y    y    y    y    y    y    y    y
ESC ?    Report row/column          y    y    y    y    y    y    y    y
ESC @    Copy-print mode on         y    y              y    y    y
ESC A    Copy-print mode off        y    y              y    y    y
ESC B    Block mode on/conv off     y                   y    y    y
ESC C    Conversation mode on       y                   y    y    y
ESC D H  Half duplex, Block off     y                   y    y
ESC D F  Full duplex, Block off     y                   y    y
ESC E    Insert a line              y        y     y    y    y    y    y
ESC F    Display control char            y
ESC F    Define block of attr.                               y
ESC F    Load status msg            ?
ESC G 0  Reset video attributes     y    y              y    y    y    y
ESC G 1  Blank video attribute      y    y              y    y    y    y
ESC G 2  Blank video attribute      y    y              y    y    y    y
ESC G 4  Blank video attribute      y    y              y    y    y    y
ESC G 8  Blank video attribute      y    y              y    y    y    y
ESC H    Auto scroll on/off              y
ESC H    Define block graph. area                            y
ESC I    Back tab                   y    y              ?    ?    y    y
ESC J    Back page                  y                        y    y
ESC K    Next page                  y                        y    y
ESC L    Unformatted Print Page     y
ESC L    Send data to hidden cursor                          y
ESC M    Transmit terminal ID       y                   y    y
ESC N    PageEdit mode on           y
ESC N 0  PageEdit mode off                                   y
ESC N 1  PageEdit mode on                                    y
ESC O    PageEdit mode off          y
ESC P    Print page, show next      y                             y
ESC P 0  Print formatted page                           y    y
ESC P 1  Print form. unprot. page                       y    y
ESC P 3  Print formatted page                           y    y
ESC P 4  Print unformatted page                         y    y
ESC P 5  Print unform. unprot. page                     y    y
ESC P 7  Print unformatted page                         y    y
ESC Q    Insert a character         y         y    y    y    y    y    y
ESC R    Delete the line            y         y    y    y    y    y    y
ESC S    Send unprot message        y         y    y              y    y
ESC S 1  Send unprot from BOL                           y    y
ESC S 3  Send all from BOL                              y    y
ESC S 5  Send unprot from BOP                           y    y
ESC S 7  Send all from BOP                              y    y
ESC S 9  Send unprot text STX->ETX                      y    y
ESC S ;  Send text from STX to ETX                      y    y
ESC S ?  Send form                                      y    y
ESC T    Erase to EOL with ic*      y    y              y    y    y    y
ESC U    Monitor mode on            y    y    y    y    y    y    y    y
ESC V    Start self test                 y    y    y    y    y
ESC W    Delete a character         y         y    y    y    y    y    y
ESC X    Monitor mode off           y    y    y    y    y    y    y    y
ESC Y    Clear to EOP with ic*      y    y    y    y    y    y    ?
ESC Z 0  Report User line           y                   y    y    y
ESC Z 1  Report Status line         y                   y    y    y
ESC Z 2  Report Setup lines                                  y
ESC [    Move to row                     y
ESC ]    Move to column                  y
ESC ] 0  Program unshift. edit keys                          y
ESC ] 1  Program shifted edit keys                           y
ESC \ 1  24 lines/logical page      y                        y
ESC \ 2  48 lines/logical page      y                        y
ESC \ 3  96 lines/logical page      y                        y
ESC ^    Program answerback msg                              y
ESC ^    Start blink attribute                y    y
ESC _    Define scrolling region                             y
ESC _    Start blank attribute                y    y
ESC `    Buffered print mode on     y         y    y    y    y    ?
ESC a    Buffered print off         y         y    y    y    y    ?
ESC b    Screen black on white      y                   y    y
ESC d    Screen white on black      y                   y    y
ESC e    Load insert character      y
ESC f    Load user line             y                        y
ESC g    25th line is User line     y
ESC g    Assign log. attr. to field                          y
ESC h    25th line is Status        y
ESC i    Field tab                  y         y    y    y    y    ?
ESC j    Reverse linefeed           y
ESC j    Reverse video attr.                  y    y
ESC k    Normal background attr.              y    y
ESC k    Local edit mode            y         y    y
ESC k 0  Duplex edit mode                               y    y
ESC k 1  Local edit mode                                y    y
ESC l    Duplex Edit mode           y         y    y
ESC l    Start underline attr.                y    y
ESC m    End underline attr.                  y    y
ESC n 0  Screen On                                      y    y
ESC n 1  Screen Off                                     y    y
ESC o 0  Logical attribute mode on                           y
ESC o 1  Logical attribute mode on                           y
ESC p    Report NVRAM contents                               y
ESC q    End blink/blank                      y    y
ESC q    Insert mode on             y
ESC r    Insert mode off            y
ESC s    Send message all           y         y    y              y    y
ESC s    Define 25th line                                    y
ESC t    Erase to EOL, nulls        y         y    y    y    y    y    ?
ESC u    Monitor mode off           y         y    y              y    ?
ESC v    Autopage on                y                             y
ESC v 0  Autopage off                                        y
ESC v 1  Autopage on                                         y
ESC w    Autopage off               y                             y
ESC x 0  Set field delimiter        y                        y    ?
ESC x 1  Set line delimiter         y         y    y         y    ?
ESC x 2  Set start prot delim.      y                        y    ?
ESC x 3  Set end prot delimiter     y                        y    ?
ESC x 4  Set screen delimiter       y         y    y         y    ?
ESC y    Erase to EOP, nulls        y         y    y    y    y    ?    ?
ESC z    Call User PROM             y
ESC {    Set main (P3) port         y                             ?
ESC { 0  Set main (comp) port                                y
ESC { 1  Set printer port baud etc                           y
ESC |    Program function keys      y                   y    y
ESC }    Set printer (P4) port      y                        ?    ?
ESC ~ 0  Reset terminal                                 y    y
ESC ~ 1  Reset to factory def.                          y    y

Notes:
The TVI925 and 905 columns probably have a lot of mistakes, the other columns
      might have a few too! (hopefully not too many)
ESC G sets video attributes based on flags, so ESC G 6 would be reverse blink;
      on most oldish ADM/Wyse/TVI/QVT terminals this attribute takes up one
      character position on the screen; this is optional on some (e.g. ADM 12)
      On the TVI914/924 they never take a space. The 914/924 allows dim to be
      set (e.g. ESC G space is DIM, ESC G " is blink dim).
BOL = Beginning of line
BOP = Beginning of page
EOL = End of line
ic* = whatever insert character is set to, space by default. A TVI950 lets you
      define what the space charecter is, on most other terminals it is always
      a space (ASCII 32).

Mark Aitchison, University of Canterbury, New Zealand.
(usual disclaimers apply, plus a big E & O E).


TVI950.CHT

              Summary of TeleVideo, Inc. Model 950 ESC sequences
              ==================================================

+------------+------------+------------+------------+------------+------------+
|SPC         |0           |@           |P           |`           |p           |
|            |Load Send   |Extention   |Formatted   |Buff transp |            |
|            |key  ++     |print on    |page prnt on|print on    |            |
+------------+------------+------------+------------+------------+------------+
|!           |1           |A           |Q           |a           |q           |
|Line lock   |Set typwritr|Extention   |Character   |Buff transp |Insert      |
|1=on, 2=off |column tab  |print off   |insert      |print off   |mode on     |
+------------+------------+------------+------------+------------+------------+
|"           |2           |B           |R           |b           |r           |
|Keyboard    |Clear type- |Block       |Line        |Set reverse |Edit mode   |
|unlock      |writer tab  |mode on     |delete      |background  |(replace) on|
+------------+------------+------------+------------+------------+------------+
|#           |3           |C           |S           |c           |s           |
|Keyboard    |Clear all   |Ret to prev |Send unprot |Local       |Send whole  |
|lock        |tabs        |duplex mode |message  ++ |mode on     |msg w/prot++|
+------------+------------+------------+------------+------------+------------+
|$           |4           |D           |T           |d           |t           |
|Graphics    |Send unprot |Dplx mode on|Clear to EOL|Set normal  |Clear to EOL|
|mode on     |line to crsr|H=half F=ful|w/ insrt chr|background  |with nulls  |
+------------+------------+------------+------------+------------+------------+
|%           |5           |E           |U           |e           |u           |
|Graphics    |Send unprot |Line        |Monitor     |Load insert |Monitor     |
|mode off    |page to crsr|insert      |mode on     |character   |mode exit   |
+------------+------------+------------+------------+------------+------------+
|&           |6           |F           |V           |f           |v           |
|Protected   |Send all lin|            |            |Load user   |Auto page   |
|mode on     |to curs/prot|            |            |line        |on          |
+------------+------------+------------+------------+------------+------------+
|'           |7           |G <d>       |W           |g           |w           |
|Protected   |Send all pag|Set screen  |Character   |Display     |Auto page   |
|mode off    |to curs/prot|attrib  **  |delete      |user line   |off         |
+------------+------------+------------+------------+------------+------------+
|(           |8           |H           |X           |h           |x           |
|Half intens-|Smooth      |            |Monitor     |Display     |Set send    |
|ity off     |scroll on   |            |mode off    |status line |delimters++ |
+------------+------------+------------+------------+------------+------------+
|)           |9           |I           |Y           |i           |y           |
|Half intens-|Smooth      |Back        |Clr end page|Field       |Clr to end  |
|ity on      |scroll off  |tab         |w/ insrt chr|tab         |of page/null|
+------------+------------+------------+------------+------------+------------+
|*           |:           |J           |Z           |j           |z           |
|Clear page  |Clear unprot|Prev screen |Send line   |Rev line    |Execute user|
|to nulls    |page to null|page        |0=usr,1=stat|feed        |ROM program |
+------------+------------+------------+------------+------------+------------+
|+           |;           |K           |[           |k           |{           |
|Clear unprot|Clear unprot|Next screen |            |Edit keys   |Configure   |
|page/ins chr|page/ins chr|page        |            |local       |comp port ++|
+------------+------------+------------+------------+------------+------------+
|,           |<           |L           |\ <d>       |l           ||           |
|Clear page  |Keyclick    |Unformatted |Set page    |Edit keys   |Program     |
|to half int |off         |page prnt on|length  *** |duplex      |func key  ++|
+------------+------------+------------+------------+------------+------------+
|-           |=           |M           |]           |m           |}           |
|Pos cursor  |Pos cursor  |Send trminal|            |            |Configure   |
|page row col|row col     |ID string   |            |            |prtr port ++|
+------------+------------+------------+------------+------------+------------+
|. <d>       |>           |N           |^           |n           |~           |
|Set cursor  |Keyclick    |Page        |            |Screen      |            |
|attrib  *   |on          |mode on     |            |on          |            |
+------------+------------+------------+------------+------------+------------+
|/           |?           |O           |_           |o           |DEL         |
|Read cursor |Read cursor |Line        |            |Screen      |            |
|page row col|row col     |mode on     |            |off         |            |
+------------+------------+------------+------------+------------+------------+


  *  '0' =off, '1' =blink block, '2' =block, '3' =blink undrln, '4' =undrlin

 **  '0' to '?', if bit 0 set =blank, 1 =blink, 2 =reverse, 3 =underline

***  '1' =24 lines, '2' =48 lines, '3' =96 lines

 ++  See manual for additional details


                    TVI 950 Switch Setting Reference Charts
                    =======================================


                                    TABLE 1:

     S1     1     2     3     4     5     6     7     8     9    10
         +-----------------------+-----+-----+-----------------------+
         | Computer Baud Rate    |Data |Stop | Printer Baud Rate     |
         |                       |Bits |Bits |                       |
  +------+-----------------------+-----+-----+-----------------------+
  |  Up  |        See            |  7  |  2  |        See            |
  +------+-----------------------+-----+-----+-----------------------+
  | Down |      TABLE 2          |  8  |  1  |      TABLE 2          |
  +------+-----------------------+-----+-----+-----------------------+


     S2     1     2     3     4     5     6     7     8     9    10
         +-----+-----+-----------------+-----+-----------+-----+-----+
         |Edit |Cursr|    Parity       |Video|Transmiss'n| Hz  |Click|
  +------+-----+-----+-----------------+-----+-----------+-----+-----+
  |  Up  | Dplx|Blink|      See        |GonBk|   See     | 60  | Off |
  +------+-----+-----+-----------------+-----+-----------+-----+-----+
  | Down |Local|St'dy|    TABLE 3      |BkonG|  CHART    | 50  | On  |
  +------+-----+-----+-----------------+-----+-----------+-----+-----+


                                   TABLE 2:

            +-----------+-----+-----+-----+-----+-----------+
            | Display   |  1  |  2  |  3  |  4  |   Baud    |
            +-----------+-----+-----+-----+-----+           |
            | Printer   |  7  |  8  |  9  | 10  |   Rate    |
            +-----------+-----+-----+-----+-----+-----------+
                        |  D  |  D  |  D  |  D  |   9600    |
                        |  U  |  D  |  D  |  D  |     50    |
                        |  D  |  U  |  D  |  D  |     75    |
                        |  U  |  U  |  D  |  D  |    110    |
                        |  D  |  D  |  U  |  D  |    135    |
                        |  U  |  D  |  U  |  D  |    150    |
                        |  D  |  U  |  U  |  D  |    300    |
                        |  U  |  U  |  U  |  D  |    600    |
                        |  D  |  D  |  D  |  U  |   1200    |
                        |  U  |  D  |  D  |  U  |   1800    |
                        |  D  |  U  |  D  |  U  |   2400    |
                        |  U  |  U  |  D  |  U  |   3600    |
                        |  D  |  D  |  U  |  U  |   4800    |
                        |  U  |  D  |  U  |  U  |   7200    |
                        |  D  |  U  |  U  |  U  |   9600    |
                        |  U  |  U  |  U  |  U  |  19200    |
                        +-----+-----+-----+-----+-----------+




                                   TABLE 3:

                        +-----+-----+-----+-----------+
                        |  3  |  4  |  5  |   Parity  |
                        +-----+-----+-----+-----------+
                        |  X  |  X  |  D  |    None   |
                        |  D  |  D  |  U  |     Odd   |
                        |  D  |  U  |  U  |    Even   |
                        |  U  |  D  |  U  |    Mark   |
                        |  U  |  U  |  U  |   Space   |
                        +-----+-----+-----+-----------+

                                X = don't care




                                    CHART:

                        +-----+-----+-----------------+
                        |  7  |  8  | Communication   |
                        +-----+-----+-----------------+
                        |  D  |  D  |  Half Duplex    |
                        |  D  |  U  |  Full Duplex    |
                        |  U  |  D  |     Block       |
                        |  U  |  U  |     Local       |
                        +-----+-----+-----------------+


------------------------------------------------------------------------------

#endif /* COMMENT */
#endif /* NOTERM */
