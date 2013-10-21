/*  C K O A D M . C  --  ADM-3A Emulation  */

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
#include "ckoadm.h"
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

int
adminc(void)
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
admctrl( int ch )
{
    int i,j;

    if ( !xprint ) {
    switch ( ch ) {
    case SOH:
    case STX:
    case ETX:
    case EOT:
        debug(F111,"ADM-3A","unused Ctrl",ch);
        break;
    case ENQ: /* Answerback Message */
        if ( debses )
            break;
        sendcharsduplex(answerback,strlen(answerback), FALSE) ;
        break;
    case ACK:
        debug(F111,"ADM-3A","unused Ctrl",ch);
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
        debug(F111,"ADM-3A","unused Ctrl",ch);
        break;
    case LF:
        /* Linefeed */
        if ( debses )
            break;
        wrtch((CHAR)LF);
        break;
    case VT:
        if ( debses )
            break;
        cursorup(0);
        break;
    case FF:
        if ( debses )
            break;
        cursorright(0);
        break;
    case CR:
        if ( debses )
            break;
        wrtch((char) CR);
        break;
    case SO: /* lock keyboard */
        if ( debses )
            break;
        keylock = TRUE;
        break;
    case SI: /* unlock keyboard */
        if ( debses )
            break;
        keylock = FALSE;
        break;
    case DLE:
    case DC1:
    case DC2:
    case DC3:
    case DC4:
    case NAK:
    case SYN:
    case ETB:
    case CAN:
        debug(F111,"ADM-3A","unused Ctrl",ch);
        break;
    case ESC:
        debug(F111,"ADM-3A","ESC",ch);
        if ( debses )
            break;
        escstate = ES_GOTESC;
        break;
    case XEM:
        debug(F111,"ADM-3A","unused Ctrl",ch);
        break;
    case SUB: /* Clear Screen */
        if ( debses )
            break;
        cleartermscreen(VTERM);
        break;
    case XFS:
    case XGS:
        debug(F111,"ADM-3A","unused Ctrl",ch);
        break;
    case XRS: /* Home cursor */
        if ( debses )
            break;
        lgotoxy(VTERM,1,1);
        break;
    case US:
    default:
        debug(F111,"ADM-3A","unused Ctrl",ch);
    }
    } /* !xprint */
    else {
        switch ( ch ) {
        default:
            break; /* There are no print commands */
        }
    }
}

void
admascii( int ch )
{
    int i,j,k,n,x,y,z;
    vtattrib attr ;
    viocell blankvcell;

    if (printon && (is_xprint() || is_uprint()))
        prtchar(ch);

    if ( escstate == ES_GOTESC )/* Process character as part of an escstate sequence */
    {
        escstate = ES_ESCSEQ ;
        switch ( ch ) {
        case '=': {
            /* Absolute cursor position */
            int col, line ;
            line  = adminc();
#ifdef NETCONN
#ifdef TCPSOCKET
            if ( network && IS_TELNET() && !TELOPT_U(TELOPT_BINARY) && line == CR ) {
                /* Handle TELNET CR-NUL or CR-LF if necessary */
                int dummy = ttinc(0);
                debug(F111,"ADM-3A","Addr cursor in page found CR",dummy);
                if ( dummy != NUL )
                    ttpush = dummy;
            }
#endif /* TCPSOCKET */
#endif /* NETCONN */
            col = adminc();
#ifdef COMMENT
        /* Is this really necessary?  Who cares if the next character is a NUL now that */
        /* we know that the sequence is complete?                                       */
#ifdef NETCONN
#ifdef TCPSOCKET
            if ( network && IS_TELNET() && !TELOPT_U(TELOPT_BINARY) && col == CR ) {
                /* Handle TELNET CR-NUL or CR-LF if necessary */
                int dummy = ttinc(0);
                debug(F111,"ADM-3A","Addr cursor in page found CR",dummy);
                if ( dummy != NUL )
                    ttpush = dummy;
            }
#endif /* TCPSOCKET */
#endif /* NETCONN */
#endif /* COMMENT */
            if ( debses )
                break;
            lgotoxy(VTERM,(col-SP)+1,(line-SP)+1);
            break;
        }
		case 'T': /* ADM 5 - clear to end of line with SP */
			debug(F110,"ADM-5","Clear to End of Line with SP",0);
            if ( debses )
                break;
			clrtoeoln( VTERM, SP );
			break;
		case 'Y': /* ADM 5 - clear to end of page with SP */
			debug(F110,"ADM-5","Clear to Screen of Line with SP",0);
            if ( debses )
                break;
			clreoscr_escape( VTERM, SP );
			break;
		case '(': /* ADM 5 - restore standard intensity */
			debug(F110,"ADM-5","Restore Standard Intensity",0);
            if ( debses )
                break;
			break;
		case ')': /* ADM 5 - set reduced intensity */
			debug(F110,"ADM-5","Set Reduced Intensity",0);
            if ( debses )
                break;
			break;
		case 'G': /* ADM 5 - toggle reverse intensity */
			debug(F110,"ADM-5","Toggle Reverse Intensity",0);
            if ( debses )
                break;
			break;
        default:
            debug(F111,"ADM-3A","unknown ESC sequence",ch);
        }
        escstate = ES_NORMAL ;
    }
    else {            /* Handle as a normal character */
        if ( ch < SP )
            admctrl(ch) ;
        else if ( !debses && ch != DEL ) {
            wrtch(ch);
        }
    }
    VscrnIsDirty(VTERM) ;
}
#endif /* NOTERM */

