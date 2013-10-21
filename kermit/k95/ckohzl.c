/*  C K O H Z L . C  --  Hazeltine 1500 Emulation  */

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
#include "ckohzl.h"
#ifdef NETCONN
#ifdef TCPSOCKET
#include "ckcnet.h"
extern int network, nettype, ttnproto, u_binary;
#endif /* TCPSOCKET */
#endif /* NETCONN */

extern bool keyclick ;
extern int  cursorena[], keylock, duplex, duplex_sav, screenon ;
extern int  printon, aprint, uprint, xprint, cprint, seslog ;
extern int  insertmode, tnlm ;
extern int  escstate, debses, decscnm, tt_cursor ;
extern int  tt_type, tt_type_mode, tt_max, tt_answer, tt_status[VNUM], tt_szchng[] ;
extern int  tt_cols[], tt_rows[], tt_wrap ;
extern int  wherex[], wherey[], margintop, marginbot ;
extern int  marginbell, marginbellcol ;
extern char answerback[], htab[] ;
extern struct tt_info_rec tt_info[] ;
extern vtattrib attrib ;
extern unsigned char attribute;
extern int ttpush;
extern int autoscroll, protect ;

extern struct _vtG G[];
extern struct _vtG *GL, *GR;

int hzesc = 1;

int hzgraphics = 0;
static USHORT
xlhzgrph( CHAR c ) {
    USHORT cx = 0 ;
    extern struct _vtG *GNOW;
    extern int win95lucida;
    extern int win95hsl;

    if ( isunicode() ) {
        cx = (*xl_u[TX_HZ1500])(c) ;
        if ( win95hsl && cx >= 0x23BA && cx <= 0x23BD )
            cx = tx_hslsub(cx);
        else if ( cx >= 0xE000 && cx <= 0xF8FF )
            cx = tx_usub(cx);
        if (win95lucida && cx > 0x017f)
            cx = tx_lucidasub(cx);
    }
    else
    {
        cx = (*xl_u[TX_HZ1500])(c) ;
        if ( GNOW->itol )
            cx = (*GNOW->itol)(cx);
    }
    return(cx);
}


int
hzlinc(void)
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

void
hzlctrl( int ch )
{
    int i,j;

    if ( !xprint ) {
    switch ( ch ) {
    case SOH:
    case STX:
    case ETX:
    case EOT:
    case ENQ:
    case ACK:
        debug(F111,"Hazeltine 1500","unused Ctrl",ch);
        break;
    case BEL:
        if ( debses )
            break;
        bleep(BP_BEL);
        break;
    case BS:
        /* Cursor Left */
        if ( debses )
            break;
        cursorleft(0) ;
        break;
    case HT:
        debug(F111,"Hazeltine 1500","unused Ctrl",ch);
        break;
    case LF:
        /* Linefeed */
        if ( debses )
            break;
        wrtch((CHAR)LF);
        break;
    case VT:
    case FF:
        debug(F111,"Hazeltine 1500","unused Ctrl",ch);
        break;
    case CR:
        if ( debses )
            break;
        wrtch((char) CR);
        break;
    case SO:
    case SI:
        debug(F111,"Hazeltine 1500","unused Ctrl",ch);
        break;
    case DLE: {
        /* Cursor right */
        if ( debses )
            break;
        cursorright(0);
        break;
    }
    case DC1:
        debug(F111,"Hazeltine 1500","unused Ctrl",ch);
        break;
    case DC2:
        /* WY30 defines this as Copy Print */
        /* transparent printer with screen display */
        if ( debses )
            break;
        cprint = TRUE ;
        if ( !printon )
            printeron() ;
        break;
    case DC3:
        debug(F111,"Hazeltine 1500","unused Ctrl",ch);
        break;
    case DC4:
        /* Turn all print modes off */
        if ( debses )
            break;
        xprint = cprint = FALSE ;
        setaprint(FALSE);
        if ( !uprint && !xprint && !cprint && !aprint && printon )
            printeroff();
        break;
    case NAK:
    case SYN:
    case ETB:
        debug(F111,"Hazeltine 1500","unused Ctrl",ch);
        break;
    case CAN:
        /* Transparent print on */
        if ( debses )
            break;
        xprint = TRUE ;
        if ( !printon )
            printeron() ;
        break;
    case ESC:
        debug(F111,"Hazeltine 1500","ESC",ch);
        if ( debses )
            break;

        if ( hzesc || ISWYSE(tt_type) ) {
            debug(F110,"Hazeltine 1500","treating escape as tilde",0);
            escstate = ES_GOTTILDE;
        }
        break;
    case XEM:
    case SUB:
    case XFS:
    case XGS:
    case XRS:
    case US:
    default:
        debug(F111,"Hazeltine 1500","unused Ctrl",ch);
    }
    } /* !xprint */
    else {
        switch ( ch ) {
        case DC4:
            /* Turn all print modes off */
            if ( debses )
                break;
            xprint = cprint = FALSE ;
            setaprint(FALSE);
            if ( !uprint && !xprint && !cprint && !aprint && printon )
                printeroff();
            break;
        }
    }
}

void
hzlascii( int ch )
{
    int i,j,k,n,x,y,z;
    vtattrib attr ;
    viocell blankvcell;

    if (printon && (is_xprint() || is_uprint()))
        prtchar(ch);

    if ( escstate == ES_GOTTILDE )/* Process character as part of an escstate sequence */
    {
        escstate = ES_TILDESEQ ;
        switch ( ch ) {
        case VT:
            /* Cursor down */
            if ( debses )
                break;
            cursordown(0);
            break;
        case FF:
            /* Cursor Up */
            if ( debses )
                break;
            cursorup(0);
            break;
        case SI:
            /* Erase to EOL */
            if ( debses )
                break;
            clrtoeoln(VTERM,SP);
            break;
        case ENQ: {
            /* Read cursor address in absolute position */
            char buf[4] ;
            if ( debses )
                break;
            buf[0] = wherex[VTERM] < SP ? wherex[VTERM] + '`' : wherex[VTERM] ;
            buf[1] = wherey[VTERM] < SP ? wherey[VTERM] + '`' : wherey[VTERM] ;
            buf[2] = CR ;
            buf[3] = NUL ;
            sendchars(buf,3);
            break;
        }
        case DC1: {
            /* Absolute cursor position */
            int col, line ;
            col  = hzlinc();
#ifdef NETCONN
#ifdef TCPSOCKET
            if ( network && IS_TELNET() && !TELOPT_U(TELOPT_BINARY) && col == CR ) {
                /* Handle TELNET CR-NUL or CR-LF if necessary */
                int dummy = ttinc(0);
                debug(F111,"Hazeltine 1500","Addr cursor in page found CR",dummy);
                if ( dummy != NUL )
                    ttpush = dummy;
            }
#endif /* TCPSOCKET */
#endif /* NETCONN */
            line = hzlinc();
#ifdef COMMENT
        /* Is this really necessary?  Who cares if the next character is a NUL now that */
        /* we know that the sequence is complete?                                       */
#ifdef NETCONN
#ifdef TCPSOCKET
            if ( network && IS_TELNET() && !TELOPT_U(TELOPT_BINARY) && line == CR ) {
                /* Handle TELNET CR-NUL or CR-LF if necessary */
                int dummy = ttinc(0);
                debug(F111,"Hazeltine 1500","Addr cursor in page found CR",dummy);
                if ( dummy != NUL )
                    ttpush = dummy;
            }
#endif /* TCPSOCKET */
#endif /* NETCONN */
#endif /* COMMENT */
            if ( debses )
                break;
            lgotoxy(VTERM,(col>='`'?col-'`':col)+1,(line>='`'?line-'`':line)+1);
            break;
        }
        case DC2:
            /* Home cursor */
            if ( debses )
                break;
            lgotoxy(VTERM,1,1);
            break;
        case DC3:
            /* Delete Line */
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
        case ETB: {
            /* Home Cursor and Clear Screen to Protected Spaces */
            int xs = VscrnGetWidth(VTERM);
            int ys = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
            vtattrib vta = {0,0,0,0,0,1,0,0,0,0,0}; /* Protected */
            if ( debses )
                break;
            clrscreen( VTERM, SP ) ;
            lgotoxy(VTERM,1,1);       /* and home the cursor */
            for ( y = 0 ; y < ys ; y++ )
                for ( x = 0 ; x < xs ; x++ )
                    VscrnSetVtCharAttr( VTERM, x, y, vta ) ;
            protect = FALSE ;
            insertmode = FALSE ;
            break;
        }
        case CAN:
            /* Erase unprotected to EOS */
            if ( debses )
                break;
            selclreoscr_escape(VTERM,SP);
            break;
        case SUB:
            /* Insert Line */
            if ( debses )
                break;
            VscrnScroll(VTERM,
                         DOWNWARD,
                         wherey[VTERM] - 1,
                         marginbot - 1,
                         1,
                         FALSE,
                         SP);
            break;
        case XFS:
            /* Home Cursor and Clear Screen to spaces */
            if ( debses )
                break;
            cleartermscreen(VTERM);
            break;
        case XGS:
            /* Home Cursor and Clear Unprotected Screen to Spaces - JEFFA */
            if ( debses )
                break;
            selclrscreen(VTERM,SP);
            lgotoxy(VTERM,1,1);       /* and home the cursor */
            break;
        case 'F':
            /* Enter graphics mode */
            if ( debses )
                break;
            hzgraphics = 1;
            break;
        case 'G':
            /* Exit graphics mode */
            if ( debses )
                break;
            hzgraphics = 0;
            break;
        case ACK:
            /* Key Lock */
            if ( debses )
                break;
            keylock = TRUE ;
            break;
        case NAK:
            /* Unlock Keys */
            if ( debses )
                break;
            keylock = FALSE ;
            break;
        case US:
            /* Write-protect mode off */
            if ( debses )
                break;
            attrib.unerasable = FALSE;
            break;
        case XEM:
            /* Write-protect mode on */
            if ( debses )
                break;
            attrib.unerasable = TRUE;
            break;
        case '1':
            /* Enable Keyclick */
            if ( debses )
                break;
            setkeyclick(TRUE);
            break;
        case '2':
            /* Disable Keyclick */
            if ( debses )
                break;
            setkeyclick(FALSE);
            break;
        case '~':
            /* Tilde Tilde does not change the escape state */
            return;
        default:
            debug(F111,"Hazeltine 1500","unknown ESC sequence",ch);
            /* if the original terminal type is Wyse, then see if it is */
            /* one of the enhanced wyse escape sequences.               */
            if ( ISWYSE(tt_type) ) {
                debug(F110,"Hazeltine 1500","attempting Wyse ESC sequence",0);
                escstate = ES_GOTESC;
                wyseascii(ch);
            }
        }
        escstate = ES_NORMAL ;
    } else if ( escstate == ES_GOTESC ) {
        wyseascii(ch);
    } else {            /* Handle as a normal character */
        if ( ch == '~' )
            escstate = ES_GOTTILDE ;
        else if ( ch < SP )
            hzlctrl(ch) ;
        else if ( !debses && ch != DEL ) {
            if ( hzgraphics )
                ch = xlhzgrph( ch ) ;
            wrtch(ch);
        }
    }
    VscrnIsDirty(VTERM) ;
}
#endif /* NOTERM */

