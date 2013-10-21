/*  C K O V C . C  --  Volker Craig VC4404/404 Emulation  */

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
#include "ckocon.h"
#include "ckovc.h"

extern bool keyclick ;
extern int  cursorena[], keylock, duplex, duplex_sav, screenon ;
extern int  printon, aprint, cprint, uprint, xprint, seslog ;
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

extern int autoscroll, protect ;

int
vcinc(void)
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
vcctrl( int ch )
{
    int i,j;

    switch ( ch ) {
    case SOH:
        break;
    case STX:
        break;
    case ETX:
        break;
    case EOT:
        break;
    case ENQ:
        break;
    case ACK:
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
        cursorleft(0);
        break;
    case HT:
        break;
    case LF:
        /* Cursor Down */
        if ( debses )
            break;
        wrtch(LF);
        break;
    case VT:
        break;
    case FF:
        break;
    case CR:
        if ( debses )
            break;
        wrtch(CR);
        break;
    case SO:
        /* DIM */
        if ( debses )
            break;
        attrib.bold = FALSE ;
        break;
    case SI:
        /* BRIGHT */
        if ( debses )
            break;
        attrib.bold = TRUE ;
        break;
    case DLE: {
        /* Address cursor in current page */
        int col, line ;
        line = vcinc();
        col  = vcinc();
        if ( debses )
            break;
        lgotoxy(VTERM,col-SP+1,line-SP+1);
        break;
    }
    case DC1:
        break;
    case DC2:
        /* Peripheral Port ON */
        if ( debses )
            break;
        cprint = TRUE ;
        if ( !printon )
            printeron() ;
        break;
    case DC3:
        break;
    case DC4:
        /* Peripheral Port OFF */
        if ( debses )
            break;
        cprint = FALSE ;
        if ( !cprint && !uprint && !xprint && !aprint && printon )
            printeroff();
        break;
    case NAK:
        /* Cursor Right */
        if ( debses )
            break;
        cursorright(0);
        break;
    case SYN:
        /* Erase to EOL */
        if ( debses )
            break;
        clrtoeoln(VTERM,SP);
        break;
    case ETB:
        /* Erase to EOS */
        if ( debses )
            break;
        clreoscr_escape(VTERM,SP);
        break;
    case CAN:
        /* Clear */
        if ( debses )
            break;
        cleartermscreen(VTERM);
        break;
    case XEM:
        /* Cursor Home */
        if ( debses )
            break;
        lgotoxy(VTERM,1,1);
        break;
    case SUB:
        /* Cursor Up */
        if ( debses )
            break;
        cursorup(0);
        break;
    case ESC:
        break;
    case XFS:
        break;
    case XGS:
        break;
    case XRS:
        break;
    case US:
        break;
    }
}

void
vcascii( int ch )
{
    int i,j,k,n,x,y,z;
    vtattrib attr ;
    viocell blankvcell;

    if (printon && (is_xprint() || is_uprint()))
        prtchar(ch);

    if ( ch < SP )
        vcctrl(ch) ;
    else if ( !debses ) {
        wrtch(ch);
    }
    VscrnIsDirty(VTERM) ;
}
#endif /* NOTERM */
