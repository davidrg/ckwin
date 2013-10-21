/*  C K O Q N X . C  --  QNX Emulation  */

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
#include "ckoqnx.h"

extern bool keyclick ;
extern int  cursorena[], keylock, duplex, duplex_sav, screenon ;
extern int  printon, aprint, cprint, uprint, xprint, seslog ;
extern int  insertmode, tnlm, decssdt ;
extern int  escstate, debses, decscnm, tt_cursor ;
extern int  tt_type, tt_type_mode, tt_max, tt_answer, tt_status[VNUM], tt_szchng[] ;
extern int  tt_cols[], tt_rows[], tt_wrap, tt_modechg ;
extern int  wherex[], wherey[], margintop, marginbot, marginleft, marginright ;
extern int  marginbell, marginbellcol ;
extern char answerback[], htab[] ;
extern struct tt_info_rec tt_info[] ;
extern vtattrib attrib, savedattrib[] ;
extern unsigned char attribute, defaultattribute;
extern unsigned char savedattribute[], saveddefaultattribute[];
extern char * udkfkeys[];
extern int tt_senddata;
extern struct _vtG G[4];
extern struct _vtG *GL,*GR;
extern int tt_hidattr;

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

int
qnxinc(void)
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
qnxattroff( void )
{
    attrib.blinking = FALSE;            /* No blink */
    attrib.bold = FALSE;                /* No bold */
    attrib.invisible = FALSE;           /* Visible */
    attrib.underlined = FALSE;          /* No underline */
    attrib.reversed = FALSE;            /* No reverse video */
    attrib.unerasable = FALSE;          /* Erasable */
    attrib.graphic = FALSE ;            /* Not graphic character */
    attrib.dim = FALSE ;                /* No dim */
}

void
qnxctrl( int ch )
{
    int i,j,x,y;

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
        /* wraps to previous line from left margin */
        /* wraps to bottom right from upper left */
        wrtch(BS);
        break;
    case HT:
        if ( debses )
            break;

        i = wherex[VTERM];
        if (i < marginright)
        {
            do {
                i++;
                cursorright(0);
            } while ((htab[i] != 'T') &&
                      (i <= marginright-1));
        }
        if ( i == marginright ) {
            wrtch(CR);
            wrtch(LF);
        }
        VscrnIsDirty(VTERM);
        break;
    case LF:
        if ( debses )
            break;
        wrtch(LF);
        break;
    case VT:
        if ( debses )
            break;
        break;
    case FF:
        if ( debses )
            break;
        /* clear screen to Fill color and home cursor */
        clrscreen(VTERM,SP);
        lgotoxy(VTERM,1,1);       /* and home the cursor */
        break;
    case CR:
        if ( debses )
            break;
        wrtch(CR);
        break;
    case SO:
        if ( debses )
            break;
        break;
    case SI:
        if ( debses )
            break;
        break;
    case DLE:
        if ( debses )
            break;
        break;
    case DC1:
        break;
    case DC2:
        if ( debses )
        break;
    case DC3:
        break;
    case DC4:
        if ( debses )
            break;
        break;
    case NAK:
        if ( debses )
            break;
        break;
    case SYN:
        if ( debses )
            break;
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
        break;
    case SUB:
        if ( debses )
            break;
        break;
    case ESC:
        /* initiate escape sequence */
        escstate = ES_GOTESC ;
        break;
    case XRS:
        if ( debses )
            break;
        break;
    case US:
        if ( debses )
            break;
        break;
    }
}

void
qnxascii( int ch )
{
    int i,j,k,n,x,y,z;
    vtattrib attr={0,0,0,0,0,0,0,0,0,0,0} ;
    viocell blankvcell;

    if (printon && (is_xprint() || is_uprint()))
        prtchar(ch);

    if ( escstate == ES_GOTESC )/* Process character as part of an escstate sequence */
    {
        if ( ch <= SP ) {
            /* Print character as graphic */
            escstate = ES_NORMAL ;
            debug(F111,"QNX","Display character as graphic",ch);
            wrtch(ch);
        }
        else
        {
            escstate = ES_ESCSEQ ;
            switch ( ch ) {
            case '!': {
                /* Set the fill color used for blank lines when scrolling   */
                /* and for clearscreen when FF is received.                 */
                /* '0'=black, blue, green, cyan, red, magenta, brown, white */
                int fg = qnxinc();
                int bg = qnxinc();
                debug(F110,"QNX","Set the Fill Color",0);
                if ( debses )
                    break;
                if ( bg < '0' || bg >= '8' || fg < '0' || fg >= '8' )
                    break;
                defaultattribute = ((bg-'0')<<4) | (fg-'0');
                break;
            }
            case '"':
                break;
            case '#':
                break;
            case '$':
                break;
            case '%':
                break;
            case '&':
                break;
            case '\'':
                break;
            case '(':
                /* Turn on Reverse attribute */
                debug(F110,"QNX","Turn on Reverse attribute",0);
                if ( debses )
                    break;
                attrib.reversed = TRUE;
                break;
            case ')':
                /* Turn off Reverse Attribute */
                debug(F110,"QNX","Turn off Reverse attribute",0);
                if ( debses )
                    break;
                attrib.reversed = FALSE;
                break;
            case '*':
                break;
            case '+':
                break;
            case ',':
                break;
            case '-':
                break;
            case '.':
                break;
            case '/':
                break;
            case '0':
                break;
            case '1':
                break;
            case '2':
                break;
            case '3':
                break;
            case '4':
                break;
            case '5':
                break;
            case '6':
                break;
            case '7':
                break;
            case '8':
                break;
            case '9':
                break;
            case ':':
                break;
            case ';':
                break;
            case '<':
                /* Turn on Bold */
                debug(F110,"QNX","Turn on Bold",0);
                if ( debses )
                    break;
                attrib.bold = TRUE;
                break;
            case '=': {
                /* Position Cursor */
                int row = qnxinc() - 31;
                int col = qnxinc() - 31;
                debug(F110,"QNX","Address cursor to row col",0);
                if ( debses )
                    break;
                lgotoxy(VTERM,col,row);
                break;
            }
            case '>':
                /* Turn off Bold */
                debug(F110,"QNX","Turn off Bold",0);
                if ( debses )
                    break;
                attrib.bold = FALSE;
                break;
            case '?':
                break;
            case '@': {
                /* Define fg,bg colors of the display characters */
                int fg = qnxinc();
                int bg = qnxinc();
                debug(F110,"QNX","Set the Display Colors",0);
                if ( debses )
                    break;
                if ( fg < '0' || fg >= '8' || bg < '0' || bg >= '8' )
                    break;
                attribute = ((bg-'0')<<4) | (fg-'0');
                break;
            }
            case 'A':
                /* Cursor Up */
                debug(F110,"QNX","Cursor Up (no wrap)",0);
                if ( debses )
                    break;
                cursorup(0);
                break;
            case 'B':
                /* Cursor Down */
                debug(F110,"QNX","Cursor Down (no wrap)",0);
                if ( debses )
                    break;
                cursordown(0);
                break;
            case 'C':
                /* Cursor Right (no wrap)*/
                debug(F110,"QNX","Cursor Right (no wrap)",0);
                if ( debses )
                    break;
                cursorright(0);
                break;
            case 'D':
                /* Cursor Left (no wrap)*/
                debug(F110,"QNX","Cursor Left (no wrap)",0);
                if ( debses )
                    break;
                cursorleft(0);
                break;
            case 'E':
                /* Insert Line */
                debug(F110,"QNX","Insert Line",0);
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
            case 'F':
                /* Delete Line */
                debug(F110,"QNX","Delete Line",0);
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
            case 'G':
                break;
            case 'H':
                /* Cursor Home */
                debug(F110,"QNX","Home Cursor",0);
                if ( debses )
                    break;
                lgotoxy(VTERM,1,1);
                break;
            case 'I':
                /* Reverse Line Feed */
                debug(F110,"QNX","Reverse Line Feed",0);
                if ( debses )
                    break;
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
                break;
            case 'J':
                /* Clear Display with SP */
                /* use Fill Color */
                debug(F110,"QNX","Clear to EOS",0);
                clreoscr_escape(VTERM,SP);
                break;
            case 'K':
                /* Erase to End of Line with SP */
                /* use Fill Color */
                debug(F110,"QNX","Clear to EOL",0);
                if ( debses )
                    break;
                clrtoeoln(VTERM,SP);
                break;
            case 'L':
                break;
            case 'M':
                break;
            case 'N':
                break;
            case 'O':
                break;
            case 'P':
                break;
            case 'Q':
                break;
            case 'R':
                /* Restore Color and Attribute Information */
                debug(F110,"QNX","Restore Colors and Attributes",0);
                if ( debses )
                    break;
                attrib = savedattrib[VTERM];
                attribute = savedattribute[VTERM];
                defaultattribute = saveddefaultattribute[VTERM];
                break;
            case 'S':
                /* Save Color and Attribute Information   */
                /* save attribute, fill color, and attrib */
                debug(F110,"QNX","Save Colors and Attributes",0);
                if ( debses )
                    break;
                savedattrib[VTERM] = attrib;
                savedattribute[VTERM] = attribute;
                saveddefaultattribute[VTERM] = defaultattribute;
                break;
            case 'T':
                break;
            case 'U':
                break;
            case 'V':
                break;
            case 'W':
                /* From Relisys terminals */
                debug(F110,"QNX","Transparent Print On",0);
                if ( debses )
                    break;
                xprint = TRUE ;
                if ( !printon )
                    printeron() ;
                break;
            case 'X':
                /* From Relisys terminals */
                debug(F110,"QNX","Print Off",0);
                if ( debses )
                    break;
                xprint = cprint = FALSE ;
                setaprint(FALSE);
                if ( !uprint && !xprint && !cprint && !aprint && printon )
                    printeroff();
                break;
            case 'Y': {
                /* Position Cursor */
                int row = qnxinc() - 31;
                int col = qnxinc() - 31;
                debug(F110,"QNX","Address cursor to row col",0);
                if ( debses )
                    break;
                lgotoxy(VTERM,col,row);
                break;
            }
            case 'Z':
                break;
            case '[':
                /* Turn on underline */
                debug(F110,"QNX","Turn on Underline",0);
                if ( debses )
                    break;
                attrib.underlined = TRUE;
                break;
            case '\\':
                break;
            case ']':
                /* Turn off underline */
                debug(F110,"QNX","Turn off Underline",0);
                if ( debses )
                    break;
                attrib.underlined = FALSE;
                break;
            case '^':
                break;
            case '_':
                break;
            case '`':
                break;
            case 'a':
                /* Cursor Up with wrap */
                debug(F110,"QNX","Cursor Up (wrap)",0);
                if ( debses )
                    break;
                cursorup(1);
                break;
            case 'b':
                /* Cursor Down with wrap */
                debug(F110,"QNX","Cursor Down (wrap)",0);
                if ( debses )
                    break;
                cursordown(1);
                break;
            case 'c':
                /* Cursor Right with wrap */
                debug(F110,"QNX","Cursor Right (wrap)",0);
                if ( debses )
                    break;
                cursorright(1);
                break;
            case 'd':
                /* Cursor Left with wrap */
                debug(F110,"QNX","Cursor Left (wrap)",0);
                if ( debses )
                    break;
                cursorleft(1);
                break;
            case 'e':
                /* Inserts a space. */
                debug(F110,"QNX","Insert SP",0);
                if ( debses )
                    break;
                blankvcell.c = SP;
                blankvcell.a = attribute;
                VscrnScrollRt(VTERM, wherey[VTERM] - 1,
                               wherex[VTERM] - 1, wherey[VTERM] - 1,
                               VscrnGetWidth(VTERM) - 1, 1, blankvcell);
                break;
            case 'f':
                /* Deletes a character */
                debug(F110,"QNX","Delete a char",0);
                if ( debses )
                    break;
                blankvcell.c = SP;
                blankvcell.a = attribute;
                VscrnScrollLf(VTERM, wherey[VTERM] - 1,
                               wherex[VTERM] - 1,
                               wherey[VTERM] - 1,
                               VscrnGetWidth(VTERM) - 1,
                               1, blankvcell) ;
                break;
            case 'g': {
                /* Repeat character */
                int n = qnxinc() - SP;
                int c = rtolxlat(qnxinc());
                debug(F110,"QNX","Repeat character",0);
                if ( debses )
                    break;
                while ( n-- )
                    wrtch(c);
                break;
            }
            case 'h':
                /* Auto wrap on */
                debug(F110,"QNX","Auto wrap on",0);
                if ( debses )
                    break;
                tt_wrap = TRUE;
                break;
            case 'i':
                /* Auto wrap off */
                debug(F110,"QNX","Auto wrap off",0);
                if ( debses )
                    break;
                tt_wrap = FALSE;
                break;
            case 'j':
                break;
            case 'k':
                break;
            case 'l':
                break;
            case 'm':
                break;
            case 'n':
                break;
            case 'o':
                break;
            case 'p':
            case 'q':
            case 'r':
            case 's':
            case 't':
            case 'u':
            case 'v':
            case 'w':
            case 'x':
                break;
            case 'y': {
                /* set cursor type */
                int c = qnxinc();
                debug(F110,"QNX","Define cursor type",0);
                if ( debses )
                    break;
                switch ( c ) {
                case '0':
                    /* no cursor */
                    cursorena[VTERM] = FALSE ;
                    break;
                case '1':
                    /* line cursor */
                    tt_cursor = TTC_ULINE ;
                    setcursormode() ;
                    cursorena[VTERM] = TRUE ;
                    break;
                case '2':
                    /* block cursor */
                    tt_cursor = TTC_BLOCK ;
                    setcursormode() ;
                    cursorena[VTERM] = TRUE ;
                    break;
                }
                break;
            }
            case 'z':
                break;
            case '{':
                /* Turn blinking on */
                debug(F110,"QNX","Turn blinking on",0);
                if ( debses )
                    break;
                attrib.blinking = TRUE;
                break;
            case '|':
                break;
            case '}':
                /* Turn blinking off */
                debug(F110,"QNX","Turn blinking off",0);
                if ( debses )
                    break;
                attrib.blinking = FALSE;
                break;
            case '~':
                break;
            case DEL:
                break;
            }
            escstate = ES_NORMAL ;      /* Done parsing escstate sequence */
        }
    }
    else                /* Handle as a normal character */
    {
        if ( ch < SP || ch == DEL )
            qnxctrl(ch) ;
        else if ( !debses ) {
            /* Display the character */
            wrtch(ch);
        }
    }
    VscrnIsDirty(VTERM) ;
}
#endif /* NOTERM */
