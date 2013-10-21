/*  C K O A V A . C  --  AVATAR Emulation  */

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
#include "ckoava.h"

extern bool keyclick ;
extern int  cursorena[], keylock, duplex, duplex_sav, screenon ;
extern int  printon, aprint, uprint, cprint, xprint, seslog ;
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
avainc( int striphighbit )
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
    debugses(ch);
    if ( striphighbit )
        ch &= 0x7F ;
    ch = ch & pmask & cmask;
    if (printon && (is_xprint() || is_uprint()))
        prtchar(ch);
    return ch;
}

void
avatar(void)
{
    int i,j;
    int ch = avainc(1) ;

    switch ( ch ) {
        /* These are AVATAR/0 commands */

    case SOH: {
        /* 1 - set current attribute */
        int attr = avainc(1) ;
        if ( debses )
            break;
        attribute = attr ;
        attrib.blinking = FALSE ;
        insertmode = FALSE ;
        break;
    }
    case STX:
        /* 0 - turn blink on */
        if ( debses )
            break;
        attrib.blinking = TRUE ;
        insertmode = FALSE ;
        break;
    case ETX:
        /* 0 - move cursor up one line */
        if ( debses )
            break;
        cursorup(0);
        insertmode = FALSE ;
        break;
    case EOT:
        /* 0 - move cursor down one line */
        if ( debses )
            break;
        cursordown(0);
        insertmode = FALSE ;
        break;
    case ENQ: {
        /* 0 - move cursor left one column */
        if ( debses )
            break;
        cursorleft(0) ;
        insertmode = FALSE ;
        break;
    }
    case ACK:
        /* 0 - move cursor right one column */
        if ( debses )
            break;
        cursorright(0) ;
        insertmode = FALSE ;
        break;

    case BEL:
        /* 0 - clear to end of line */
        if ( debses )
            break;
        clrtoeoln(VTERM,SP);
        insertmode = FALSE ;
        break;
    case BS: {
        /* 2 - locate cursor position */
        int row, col ;
        row = avainc(1) ;
        col = avainc(1) ;
        if ( debses )
            break;
        lgotoxy( VTERM, col, row ) ;
        insertmode = FALSE ;
        break;
    }

        /* Now we start the AVATAR/0+ commands */
    case HT:
        /* 0 - turn insert mode on */
        if ( debses )
            break;
        insertmode = TRUE ;
        break;
    case LF: {
        /* 5 - scroll area up */
        int numlines, upper, left, lower, right ;
        viocell cell ;

        numlines = avainc(1) ;
        upper = avainc(1) ;
        left = avainc(1) ;
        lower = avainc(1) ;
        right = avainc(1) ;

        if ( debses )
            break;

        cell.c = SP ;
        cell.a = attribute ;

        VscrnScrollUp( VTERM, upper-1, left-1, lower-1, right-1,
                       numlines, cell ) ;
        break;
    }
    case VT: {
        /* 5 - scroll area down */
        int numlines, upper, left, lower, right ;
        viocell cell ;

        numlines = avainc(1) ;
        upper = avainc(1) ;
        left = avainc(1) ;
        lower = avainc(1) ;
        right = avainc(1) ;
        if ( debses )
            break;

        cell.c = SP ;
        cell.a = attribute ;

        VscrnScrollDn( VTERM, upper-1, left-1, lower-1, right-1,
                       numlines, cell ) ;
        break;
    }
    case FF: {
        /* 3 - clear area, set current attribute */
        int attr, lines, cols ;
        int x, y ;
        viocell cell ;

        attr = avainc(1) ;
        lines = avainc(1) ;
        cols = avainc(1) ;

        if ( debses )
            break;

        attribute = attr ;
        attrib.blinking = FALSE ;

        cell.c = SP ;
        cell.a = attribute ;

        for ( y=0 ; y < lines ; y++ )
            for ( x=0 ; x < cols ; x++ )
                VscrnWrtCell( VTERM, cell, attrib,
                              wherey[VTERM]-1+y,
                              wherex[VTERM]-1+x ) ;
        break;
    }
    case CR: {
        /* 4 - initialize area, set current attribute */
        int attr, ch, lines, cols ;
        int x, y ;
        viocell cell ;

        attr = avainc(0) ;
        ch = avainc(1) ;
        lines = avainc(1) ;
        cols = avainc(1) ;

        if ( debses )
            break;

        attribute = attr ;
        attrib.blinking = FALSE ;

        cell.c = ch ;
        cell.a = attribute ;

        for ( y=0 ; y < lines ; y++ )
            for ( x=0 ; x < cols ; x++ )
                VscrnWrtCell( VTERM, cell, attrib,
                              wherey[VTERM]-1+y,
                              wherex[VTERM]-1+x ) ;
        break;
    }
    case SO: {
        /* 0 - delete character, scroll eol left */
        viocell cell ;
        if ( debses )
            break;
        cell.c = SP ;
        cell.a = attribute ;
        VscrnScrollLf(VTERM, wherey[VTERM] - 1,
                       wherex[VTERM] - 1,
                       wherey[VTERM] - 1,
                       VscrnGetWidth(VTERM) - 1,
                       1, cell ) ;
        break;
    }

#ifdef AVATAR1
        /* And now AVATAR/1 commands */
    case SI:
        /* 0 - turn clockwise mode on */
        if ( debses )
            break;
        break;
    case DLE:
        /* ? - dummy call, NOP */
        if ( debses )
            break;
        break;
#endif /* AVATAR1 */
    case DC1: {
        /* 1 - query the driver */
        int query = avainc(1) ;
        if ( debses )
            break;
        if ( query == DC1 ) {
            sendchars("AVT0, Kermit ANSI-BBS Terminal\r",31);
        }
        break;
    }
#ifdef AVATAR1
    case DC2:
        /* 0 - reset the driver */
        if ( debses )
            break;
        break;
    case DC3: {
        /* 3 - sound a tone */
        int tone, octave, duration ;
        tone = avainc(1) ;
        octave = avainc(1);
        duration = avainc(1);

        if ( debses )
            break;
        break;
    }
    case DC4: {
        /* 1 - highlight cursor position */
        int attr = avainc(1);
        if ( debses )
            break;
        break;
    }
    case NAK: {
        /* 2 - hightlight a window */
        int window, attr ;
        window = avainc(1);
        attr = avainc(1);
        if ( debses )
            break;
        break;
    }
    case SYN: {
        /* 6 - define a window */
        int window, attr, c,d,e,f ;
        window = avainc(1);
        attr = avainc(1);
        c=avainc(1);
        d=avainc(1);
        e=avainc(1);
        f=avainc(1);
        if ( debses )
            break;
        break;
    }
    case ETB:
        /* 0 - flush input */
        if ( debses )
            break;
        break;
    case CAN: {
        /* 1 - switch to a window */
        int window = avainc(1);
        if ( debses )
            break;
        break;
    }
#endif /* AVATAR1 */

    case XEM: {
        /* * - repeat pattern */
        int ch, count;
        ch = avainc(1) ;
        if ( debses )
            break;
        count = avainc(1) ;
        while ( count-- )
            wrtch(ch);
        break;
    }

#ifdef AVATAR1
    case SUB:
        /* ? - reserved  */
        if ( debses )
            break;
        break;
#endif /* AVATAR1 */

    case ESC:
        /* ? - reserved for ANSI */
        escstate = ES_GOTESC ;
        break;

#ifdef AVATAR1
    case XFS:
        /* 0 - go to sleep */
        if ( debses )
            break;
        break;
    case XGS:
        /* 0 - wake up */
        if ( debses )
            break;
        break;
    case XRS:
        /* 0 - start vertical mode */
        if ( debses )
            break;
        break;
    case US:
        /* 0 - end vertical mode */
        if ( debses )
            break;
        break;
    case SP:
        /* 0 - reserved */
        break;
    case '!': {
        /* 4 - poke */
        int ch, attr, c, d ;
        ch = avainc(1);
        attr = avainc(1) ;
        c = avainc(1) ;
        d = avainc(1) ;
        if ( debses )
            break;
        break;
    }
    case '"':
        /* - 0 - do not wrap at eol (wrap off)         */
        tt_wrap = FALSE ;
        break;
    case '#':
        /* - 0 - switch direction at eol (wrap zigzag) */
        break;
    case '$':
        /* - 0 - wrap at eol (wrap on)                 */
        tt_wrap = TRUE ;
        break;
    case '%':
        /* - 0 - reverse LF action                     */
        break;
    case '&':
        /* - 0 - normalize LF action                   */
        break;
    case '\'':   {
        /* - 1 - set cursor type                       */
        int type = avainc(1);
        if ( debses )
            break;
        break;
    }
    case '(':
        /* - 0 - do not print in reverse               */
        break;
    case ')':
        /* - 0 - print in reverse                      */
        break;
    case '*':   {
        /* - 1 - make system pause                     */
        int duration = avainc(1) ;
        if ( debses )
            break;
        break;
    }
    case '+':
        /* - 0 - insert line                           */
        break;
    case ',':
        /* - 0 - insert column                         */
        break;
    case '-':
        /* - 0 - delete line                           */
        break;
    case '.':
        /* - 0 - delete column                         */
        break;
    case '/':  {
        /* - 1 - set/reset static mode                 */
        int mode = avainc(1) ;
        if ( debses )
            break;
        break;
    }
    case '0':   {
        /* - 1 - highlight eol                         */
        int attr = avainc(1) ;
        if ( debses )
            break;
        break;
    }
    case '1':   {
        /* - 1 - highlight bol                         */
        int attr = avainc(1) ;
        if ( debses )
            break;
        break;
    }
    case ':': {
        /* - 1 - keyboard mode                         */
        int mode = avainc(1) ;
        if ( debses )
            break;
        break;
    }
    case ';':
        /* - ? - <reserved>                            */
        break;
    case '<':    {
        /* - 5 - scroll area left                      */
        int a, b, c, d, e ;
        a = avainc(1);
        b = avainc(1);
        c = avainc(1);
        d = avainc(1);
        e = avainc(1);
        if ( debses )
            break;
        break;
    }
    case '=':    {
        /* - 1 - set parser mode (cooked or raw)       */
        int mode = avainc(1);
        if ( debses )
            break;
        break;
    }
    case '>':   {
        /* - 5 - scroll area right                     */
        int a, b, c, d, e ;
        a = avainc(1);
        b = avainc(1);
        c = avainc(1);
        d = avainc(1);
        e = avainc(1);
        if ( debses )
            break;
        break;
    }
    case '?':   {
        /* - 2 - peek                                  */
        int a, b ;
        a = avainc(1);
        b = avainc(1);
        if ( debses )
            break;
        break;
    }
#endif  /* AVATAR1 */
    }
}
#endif /* NOTERM */

