/*  C K O H P . C  --  Hewlett-Packard Emulation  */

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
#include "ckokey.h"

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
extern vtattrib attrib ;
extern unsigned char attribute, colorstatus;
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

extern char fkeylabel[16][32];

static int attr_x=0,attr_y=0;
static int hpprt_term=-1;
static int hpprt_count=0;
static int hpprt_lf=0;
static int hpprt_len=0;

int
hpinc(void)
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
    if ( ch < SP && escstate == ES_ESCSEQ ) {
        /* HP ignores many control characters during escape sequences */
        switch ( ch ) {
        case NUL:
        case SOH:
        case STX:
        case ETX:
        case EOT:
        case ENQ:
        case ACK:
        case DLE:
        case DC1:
        case DC2:
        case DC3:
        case DC4:
        case NAK:
        case SYN:
        case ETB:
        case DEL:
            ch = hpinc();
            break;
        default:
            break;
        }
    }
    return ch;
}

void
hpattroff( void )
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
hpctrl( int ch )
{
    int i,j,x,y;

    switch ( ch ) {
    case ETX:
        debug(F100,"HPTERM Control: ETX","",0);
        if ( debses )
            break;
        break;
    case EOT:
        debug(F100,"HPTERM Control: EOT","",0);
        if ( debses )
            break;
        break;
    case ENQ:
        debug(F100,"HPTERM Control: ENQ","",0);
        if ( debses )
            break;
        sendchar(ACK);
        break;
    case ACK:
        debug(F100,"HPTERM Control: ACK","",0);
        if ( debses )
            break;
        break;
    case BEL:
        debug(F100,"HPTERM Control: BEL","",0);
        if ( debses )
            break;
        bleep(BP_BEL);
        break;
    case BS:
        debug(F100,"HPTERM Control: BS","",0);
        if ( debses )
            break;
        wrtch(BS);
        break;
    case HT:
        debug(F100,"HPTERM Control: HT","",0);
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
        debug(F100,"HPTERM Control: LF","",0);
        if ( debses )
            break;
        wrtch(LF);
        break;
    case VT:
        debug(F100,"HPTERM Control: VT","",0);
        if ( debses )
            break;
        break;
    case FF:
        debug(F100,"HPTERM Control: FF","",0);
        if ( debses )
            break;
        break;
    case CR:
        debug(F100,"HPTERM Control: CR","",0);
        if ( debses )
            break;
        wrtch(CR);
        break;
    case SO:    /* Use alternate character set */
        debug(F110,"HPTERM Control: SO","Use Alternate Character Set",0);
        if ( debses )
            break;
        GL = &G[2];
        GR = &G[3];
        break;
    case SI:    /* Use normal character set */
        debug(F110,"HPTERM Control: SI","Use Normal Character Set",0);
        if ( debses )
            break;
        GL = &G[0];
        GR = &G[1];
        break;
    case DLE:
        debug(F100,"HPTERM Control: DLE","",0);
        break;
    case DC1:
        debug(F100,"HPTERM Control: DC1","",0);
        break;
    case DC2:
        debug(F100,"HPTERM Control: DC2","",0);
        break;
    case DC3:
        debug(F100,"HPTERM Control: DC3","",0);
        break;
    case DC4:
        debug(F100,"HPTERM Control: DC4","",0);
        break;
    case NAK:
        debug(F100,"HPTERM Control: NAK","",0);
        break;
    case SYN:
        debug(F100,"HPTERM Control: SYN","",0);
        break;
    case ETB:
        debug(F100,"HPTERM Control: ETB","",0);
        break;
    case CAN:
        debug(F100,"HPTERM Control: CAN","",0);
        break;
    case XEM:
        debug(F100,"HPTERM Control: EM","",0);
        break;
    case SUB:
        debug(F100,"HPTERM Control: SUB","",0);
        break;
    case ESC:
        /* initiate escape sequence */
        debug(F100,"HPTERM Control: ESC","",0);
        escstate = ES_GOTESC ;
        break;
    case XRS:
        debug(F100,"HPTERM Control: RS","",0);
        break;
    case US:
        debug(F100,"HPTERM Control: US","",0);
        break;
    default:
        debug(F101,"HPTERM Control: <unknown>","",ch);
    }
}

/* Reads a parameter combination */
/* Returns -1 if error, 0 if this is this last parameter, */
/* and 1 if there are more parameters in the sequence.    */
/* num is set to the numeric value and alpha to the command code */

int
hpparam( int * num, int * alpha, int * relative )
{
    int ch=0;
    int neg=0;

    *num = 0;
    *alpha = NUL;
    *relative = 0;

    while((ch = hpinc()) >= 0) {
        if ( isdigit(ch) ) {
            *num *= 10;
            *num += (ch - '0');
        }
        else if ( islower(ch) ) {
            *alpha = ch;
            if ( neg ) *num *= -1;
            return(1);
        }
        else if ( isupper(ch) ) {
            *alpha = tolower(ch);
            if ( neg ) *num *= -1;
            return(0);
        }
        else if ( ch == '-' ) {
            neg = 1;
            *relative = 1;
        }
        else if ( ch == '+' ) {
            neg = 0;
            *relative = 1;
        }
        else if ( ch < SP ) {
            debug(F101,"HPTERM hpparam() got control","",ch);
        }
        else if ( ispunct(ch) ) {
            *alpha = ch;
            if ( neg ) *num *= -1;
            return(0);
        }
        else {
            debug(F101,"HPTERM hpparam() got <unknown>","",ch);
        }
    }
    return(ch);
}

void
hpascii( int ch )
{
    int i=0,j=0,k=0,m=0,n=0,x=0,y=0,z=0;
    vtattrib attr={0,0,0,0,0,0,0,0,0,0,0} ;
    viocell blankvcell;
    char debbuf[256]="";

    if ( xprint ) {
        /* RECORD MODE */
        if ( (hpprt_term == ch &&
             (hpprt_lf || !(hpprt_count%256))) ||
             (hpprt_term == LF && ch == LF)) {
            /* we are done */
            xprint = FALSE;
            if ( !uprint && !xprint && !cprint && !aprint && printon )
                printeroff();
        }
        else {
            hpprt_count++;
            hpprt_lf = (ch == LF);
            if (printon && (is_xprint() || is_uprint()))
                prtchar(ch);

            if ( hpprt_len && hpprt_count >= hpprt_len ) {
                /* we are done */
                xprint = FALSE;
                if ( !uprint && !xprint && !cprint && !aprint && printon )
                    printeroff();
            }
        }
        return;
    }

    if (printon && (is_xprint() || is_uprint()))
        prtchar(ch);

    if ( escstate == ES_GOTESC )/* Process character as part of an escstate sequence */
    {
        if ( ch <= SP ) {
            /* Illegal sequence; ignore both characters */
            escstate = ES_NORMAL ;
            debug(F101,"HPTERM illegal escape sequence","",ch);
        }
        else
        {
            escstate = ES_ESCSEQ ;
            switch ( ch ) {
#ifdef COMMENT
            case '!':
                break;
            case '"':
                break;
            case '#':
                break;
#endif /* COMMENT */
            case '$': { /* HPTERM Kanji */
                int ch2;
                do {
                    if ((x = hpparam(&n,&ch2,&y)) < 0)
                        break;
                    switch ( ch2 ) {
                    case '@':   /* JIS Kanji Character Set */
                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                    case 'e':
                    case 'f':
                    case 'g':
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %d %c%s",
                                     ch, n, ch2, y ? " (relative)" : "");
                            debug(F110,"HPTERM Kanji Character Set selection (unsupported)",
                                   debbuf,0);
                        }
                        break;
                    default:
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %d %c%s",
                                     ch, n, ch2, y ? " (relative)" : "");
                            debug(F110,"HPTERM Unknown",
                                   debbuf,0);
                        }
                    }
                } while (x>0);
                break;
            }
            case '%': { /* HPTERM Text Direction */
                int ch2;
                do {
                    if ((x = hpparam(&n,&ch2,&y)) < 0)
                        break;
                    switch ( ch2 ) {
                    case 'r':   /* Latin Text Direction */
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %d %c%s",
                                     ch, n, ch2, y ? " (relative)" : "");
                            debug(F110,"HPTERM Text Direction: Latin (unsupported)",
                                   debbuf,0);
                            }
                        break;
                    case 's':   /* Arabic Text Direction */
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %d %c%s",
                                     ch, n, ch2, y ? " (relative)" : "");
                            debug(F110,"HPTERM Text Direction: Arabic (unsupported)",
                                   debbuf,0);
                            }
                        break;
                    default:
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %d %c%s",
                                     ch, n, ch2, y ? " (relative)" : "");
                            debug(F110,"HPTERM Unknown",
                                   debbuf,0);
                        }
                    }
                } while (x>0);
                break;
            }
            case '&': {
                /* Parameter Sequence */
                int ch2 = hpinc() ;
                switch ( ch2 ) {
                case 'a': {
                    /* Memory and Screen Addressing */
                    /* memory row number r column number C - Absolute Memory Addressing */
                    /* screen row number y column number C - Absolute Screen Addressing */
                    /* +/- memory row number r column number C - Relative Memory Addressing */
                    /* +/- screen row number y column number C - Relative Screen Addressing */
                    int row=-1,column=-1,relcol=FALSE,relrow=FALSE;
                    int ch3;

                    do {
                        if ((x = hpparam(&n,&ch3,&y)) < 0)
                            break;
                        switch ( ch3 ) {
                        case 'c':       /* column */
                            column = n;
                            relcol = y;
                            if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Memory and Screen Addressing: column",
                                   debbuf,0);
                            }
                            break;
                        case 'r':       /* memory row */
                            row = n;
                            relrow = y;
                            if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Memory Addressing: row",
                                   debbuf,0);
                            }
                            break;
                        case 'y':       /* screen row */
                            row = n;
                            relrow = y;
                            if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Screen Addressing: row",
                                   debbuf,0);
                            }
                            break;
                        default:
                            if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM unknown command",debbuf,0);
                            }
                        }
                    } while (x>0);

                    if ( x < 0 )
                        break;

                    if ( debses )
                        break;

                    if ( column < 0 && !relcol ) {
                        /* not assigned */
                        column = wherex[VTERM];
                    }
                    else if ( relcol ) {
                        column += wherex[VTERM];
                    }
                    else {
                        column += 1;
                    }
                    if ( row < 0 && !relrow ) {
                        /* not assigned */
                        row = wherey[VTERM];
                    } else if ( relrow ) {
                        row += wherey[VTERM];
                    } else {
                        row += 1;
                    }

                    if ( column > VscrnGetWidth(VTERM) )
                        column = VscrnGetWidth(VTERM);
                    else if ( column < 1 )
                        column = 1;

                    if ( row > VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0) )
                        row = VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0);
                    else if ( row < 1 )
                        row = 1;

                    lgotoxy(VTERM,column, row);
                    break;
                }
                case 'd': {
                    /* Display Enhancement */
                    /* A -> O - Enable Underline; @ - Disable Underline */
                    int ch3;

                    do {
                        if ((x = hpparam(&n,&ch3,&y)) < 0)
                            break;
                    switch ( ch3 ) {
                    case '@':
                        attr.blinking = FALSE;
                        attr.reversed = FALSE;
                        attr.underlined = FALSE;
                        attr.dim = FALSE;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Normal",
                                   debbuf,0);
                            }
                        break;
                    case 'a':
                        attr.blinking = TRUE ;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Blink",
                                   debbuf,0);
                        }
                        break;
                    case 'b':
                        attr.reversed = TRUE ;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Reverse",
                                   debbuf,0);
                        }
                        break;
                    case 'c':
                        attr.blinking = TRUE;
                        attr.reversed = TRUE;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Blink Reverse",
                                   debbuf,0);
                        }
                        break;
                    case 'd':
                        attr.underlined = TRUE;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Underline",
                                   debbuf,0);
                        }
                        break;
                    case 'e':
                        attr.blinking = TRUE;
                        attr.underlined = TRUE;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Blink Underline",
                                   debbuf,0);
                        }
                        break;
                    case 'f':
                        attr.reversed = TRUE;
                        attr.underlined = TRUE;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Reverse Underline",
                                   debbuf,0);
                        }
                        break;
                    case 'g':
                        attr.blinking = TRUE;
                        attr.reversed = TRUE;
                        attr.underlined = TRUE;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Blink Reverse Underline",
                                   debbuf,0);
                        }
                        break;
                    case 'h':
                        attr.dim = TRUE ;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Dim",
                                   debbuf,0);
                        }
                        break;
                    case 'i':
                        attr.blinking = TRUE;
                        attr.dim = TRUE;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Blink Dim",
                                   debbuf,0);
                        }
                        break;
                    case 'j':
                        attr.reversed = TRUE;
                        attr.dim = TRUE;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Reverse Dim",
                                   debbuf,0);
                        }
                        break;
                    case 'k':
                        attr.blinking = TRUE;
                        attr.reversed = TRUE;
                        attr.dim = TRUE;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Blink Reverse Dim",
                                   debbuf,0);
                        }
                        break;
                    case 'l':
                        attr.underlined = TRUE;
                        attr.dim = TRUE;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Underline Dim",
                                   debbuf,0);
                        }
                        break;
                    case 'm':
                        attr.blinking = TRUE;
                        attr.underlined = TRUE;
                        attr.dim = TRUE;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Blink Underline Dim",
                                   debbuf,0);
                        }
                        break;
                    case 'n':
                        attr.reversed = TRUE;
                        attr.underlined = TRUE;
                        attr.dim = TRUE;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Reverse Underline Dim",
                                   debbuf,0);
                        }
                        break;
                    case 'o':
                        attr.blinking = TRUE;
                        attr.reversed = TRUE;
                        attr.underlined = TRUE;
                        attr.dim = TRUE;
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM Display Enhancement: Blink Reverse Underline Dim",
                                   debbuf,0);
                        }
                        break;
                    default:
                        if ( deblog ) {
                        sprintf(debbuf,
                                 "ESC %c %c %d %c%s",
                                 ch, ch2, n, ch3, y ? " (relative)" : "");
                        debug(F110,"HPTERM unknown command",debbuf,0);
                        }
                    }
                    } while(x>0);
                    if ( x < 0 ) break;
                    if ( debses )
                        break;

                    attr_x = x = wherex[VTERM];
                    attr_y = y = wherey[VTERM];
                    if ( !tt_hidattr )
                        wrtch(SP) ;
                    ApplyLineAttribute( VTERM, x, y, attr );
                    attrib = attr;
                    break;
                }
                case 'f': {
                    /* Program Screen Labels */
                    /* (Use) a (key) k (label length) d (string length) | (label) (string) soft key */
                    /* label where: */
                    /* Use: 0 = normal; 1 = local ; 2 = remote (default) */
                    /* Key: 1 = F1 Function Key to 8 = F8 Function Key */
                    /* label = length 0 to 8 - number of characters in label (0 default) */
                    /* string length = 0 to 72 - number of characters in string (1 default) */
                    /* string length = 0 to 72 - number of diacritical characters in string */
                    /* <n> E - invoke softkey */
                    /* 0 k <length> D <icon-name> */
                    /* -1 k <length> D <window-name> */
                    int use=0, key=1, invoke=-2, llen=0, slen=0;
                    char label[80]="";
                    char string[160]="";
                    int ch3=0, keyi=0, i=0;     /* work variables */

                    do {
                        if ((x = hpparam(&n,&ch3,&y)) < 0)
                            break;
                        switch ( ch3 ) {
                        case 'a':       /* attribute */
                        case 'A':
                            use = n ;
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Program Screen Labels: attribute",
                                       debbuf,0);
                            }
                            break;
                        case 'e':       /* Invoke key */
                        case 'E':
                            invoke = n;
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Program Screen Labels: invoke key",
                                       debbuf,0);
                            }
                            break;
                        case 'k':       /* key */
                        case 'K':
                            key = n ;
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Program Screen Labels: key",
                                       debbuf,0);
                            }
                            break;
                        case 'd':       /* label-length */
                        case 'D':
                            llen = n ;
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Program Screen Labels: label length",
                                       debbuf,0);
                            }
                            break;
                        case 'l':       /* string-length */
                        case 'L':
                            slen = n ;
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Program Screen Labels: string length",
                                       debbuf,0);
                            }
                            break;
                        default:
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM unknown command",debbuf,0);
                        }
                    } while(x>0);
                    if ( x < 0 ) break;

                    /* Now read the strings if any */
                    if ( llen > 0 ) {
                        for ( i=0; i < llen && i < 79 ; i++ ) {
                            label[i] = hpinc();
                        }
                    }
                    label[i] = '\0';
                    debug(F110,"HPTERM Label",label,0);
                    if ( slen > 0 ) {
                        for ( i=0; i < slen && i < 159 ; i++ ) {
                            string[i] = hpinc();
                        }
                    }
                    string[i] = '\0';
                    debug(F110,"HPTERM String",string,0);

                    if ( debses )
                        break;
                    if ( use < 0 || use > 2 ||
                         use == 1 /* we don't support local mode */)
                        break;
                    if ( key < -1 || key > 16 )
                        break;

                    /* For when we support labels */
                    if ( llen == -1 && key > 0) {
                        /* undefine the label for this key */
                        strcpy( fkeylabel[key-1], "");
                    }
                    else if ( llen == 0 && key > 0) {
                        /* leave the label alone */;
                    }
                    else {
                        if ( key > 0 ) {
                        /* assign the first 16 chars as the new label  */
                        /* ignore labels for the Enter and Return keys */
                            ckstrncpy( fkeylabel[key-1], label, 32);
                        }
                        else if ( key == 0 ) {
                            /* Set Icon Label */;
                        }
                        else if ( key == -1 ) {
                            /* Set Window Title */
                            os2settitle(label,1);
                        }
                    }
                    switch ( key ) {
#ifndef NOKVERBS
                    case -1:
                        keyi = K_HPENTER - K_HPF01;
                        break;
                    case 0:
                        keyi = K_HPRETURN - K_HPF01;
                        break;
#endif /* NOKVERBS */
                    default:
                        keyi = key - 1;
                        break;
                    }
#ifndef NOSETKEY
                    if ( slen == -1 ) {
                        /* undefine the string for this key */;
                        if ( udkfkeys[keyi] ){
                            free( udkfkeys[keyi] ) ;
                            udkfkeys[keyi]=NULL;
                        }
                    }
                    else if ( slen == 0 )
                        /* leave the string alone */;
                    else {
                        /* assign the first 80 chars as the new string */
                        /* except for Enter and Return which use 8 chars */
                        if ( udkfkeys[keyi] ){
                            free( udkfkeys[keyi] ) ;
                            udkfkeys[keyi]=NULL;
                        }
                        if ( strlen(string) )
                            udkfkeys[keyi] = strdup(string) ;
                    }
#endif /* NOSETKEY */
                    /* Now handle invocation */
                    if ( invoke > -2 ) {
                        switch ( invoke ) {
#ifndef NOKVERBS
                        case -1:
                            keyi = K_HPENTER - K_HPF01;
                            break;
                        case 0:
                            keyi = K_HPRETURN - K_HPF01;
                            break;
#endif /* NOKVERBS */
                        default:
                            keyi = key - 1;
                            break;
                        }
#ifndef NOSETKEY
                        if ( udkfkeys[keyi] )
                            sendcharsduplex(udkfkeys[keyi],
                                             strlen(udkfkeys[keyi]),TRUE);
#endif /* NOSETKEY */
                    }
                    break;
                }
                case 'j': {
                    /* Screen Labels Control */
                    int ch3;
                    do {
                        if ((x = hpparam(&n,&ch3,&y)) < 0)
                            break;
                        switch ( ch3 ) {
                        case '@':       /* Clear Screen Labels */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Screen Labels Control: Clear Labels",
                                       debbuf,0);
                            }
                            break;
                        case 'a':       /* Display Screen Labels */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Screen Labels Control: Display Labels (unsupported)",
                                       debbuf,0);
                            }
                            break;
                        case 'b':       /* Display F1-F8 Screen Labels */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Screen Labels Control: Display F1-F8 Labels (unsupported)",
                                       debbuf,0);
                            }
                            break;
                        case 'r':       /* Unlock labels */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Screen Labels Control: Unlock Labels (unsupported)",
                                       debbuf,0);
                            }
                            break;
                        case 's':       /* Lock current labels */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Screen Labels Control: Lock Labels (unsupported)",
                                       debbuf,0);
                            }
                            break;
                        case 'c':       /* Clear Status Line ??? */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Screen Labels Control: Clear Status Line ??? (unsupported)",
                                       debbuf,0);
                            }
                            break;
                        case 'd':       /* ??? */
                            /* <ESC>&j<p>D have seen <p> = 1 and 2 */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Screen Labels Control: ??? (unsupported)",
                                       debbuf,0);
                            }
                            break;
                        case 'l': {  /* Program Status Line */
                            /* <ESC>&j<p>L */
                            char buf[132];
                            int width = VscrnGetWidth(VTERM);
                            int i = 0;

                            if ( debses ) {
                                while ( i < n ) {
                                    hpinc();
                                    i++;
                                }
                            }
                            else {
                                while ( i < n ) {
                                    buf[i]=hpinc();
                                    i++;
                                }
                                buf[i] = NUL ;
                            }
                            while ( i < width )
                                buf[i++] = SP ;
                            buf[i]=NUL ;

                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s: %s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "",
                                         buf);
                                debug(F110,"HPTERM Screen Labels Control: Program Status Line",
                                       debbuf,0);
                            }
                            if ( debses )
                                break;

                            setdecssdt(SSDT_HOST_WRITABLE);
                            if ( decssdt == SSDT_HOST_WRITABLE ) {
                                VscrnSetWidth(VSTATUS,VscrnGetWidth(VTERM));
                                VscrnWrtCharStrAtt( VSTATUS, buf,
                                                    strlen(buf),
                                                    1, 1, &colorstatus ) ;
                            }
                            break;
                        }
                        default:
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM unknown command",debbuf,0);
                        }
                    } while (x>0);
                    if ( x < 0 || debses )
                        break;
                    break;
                }
                case 'k': {
                    /* <ESC>&k0a0b0C */
                    int ch3=0;
                    do {
                        if ((x = hpparam(&n,&ch3,&y)) < 0)
                            break;
                        if ( debses )
                            continue;
                        switch ( ch3 ) {
                        case 'o':
                        case 'O':
                            /* Keycode mode */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Keycode Mode (unsupported)",
                                       debbuf,0);
                            }
                            break;
                        case 'p':
                        case 'P':
                            /* Enables/disables CAPS mode */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM CAPS mode (unsupported)",
                                       debbuf,0);
                            }
                            break;
                        case '\\':
                            /* Change ANSI/HP mode */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM ANSI/HP mode (unsupported)",
                                       debbuf,0);
                            }

                            break;
                        default:
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM unknown command",debbuf,0);
                        }
                    } while (x>0);
                    break;
                }
                case 'p': {
                    /* Printer Control - HP2621P only */
                    /*
                       Esc & p 11 C
                       Enable data logging from the bottom of memory
                       (print current entry line)

                       Esc & p 12 C
                       Enable data logging from the top of memory
                       (print line as it rolls off top of memory)

                       Esc & p 13 C
                       Disable data logging (top or bottom, whichever is enabled)

                       Esc & p M
                       Print all data in memory from cursor line to bottom

                       Esc & p F
                       Print all data on screen from cursor line to end of screen

                       Esc & p B
                       Print cursor line
                    */
                    int action=0,device=0;
                    int ch3=0;
                    hpprt_term = -1;
                    hpprt_count = 0;
                    hpprt_lf = 0;
                    hpprt_len = 0;

                    do {
                        if ((x = hpparam(&n,&ch3,&z)) < 0)
                            break;
                        switch ( ch3 ) {
                        case '^':       /* Status of device */
                            device = n;
                            action = '^';
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Printing: Status of Device",
                                       debbuf,0);
                            }
                            debug(F111,"HP Print","device",device);
                            debug(F111,"HP Print","action",action);
                            break;
                        case 'd':       /* Device number */
                        case 'D':
                        case 'u':       /* same, from Reflection manual */
                        case 'U':
                            device = n;     /* we ignore this anyway */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Printing: Device Number (unsupported)",
                                       debbuf,0);
                            }
                            debug(F111,"HP Print","device",device);
                            break;
                        case 'p':       /* Termination character */
                        case 'P':
                            hpprt_term = n;
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Printing: Termination character",
                                       debbuf,0);
                            }
                            debug(F111,"HP Print","terminator",hpprt_term);
                            break;
                        case 'b':       /* Action */
                        case 'B':
                            action = 'B';
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Printing: Action",
                                       debbuf,0);
                            }
                            debug(F111,"HP Print","action",action);
                            break;
                        case 'f':       /* Action */
                        case 'F':
                            action = 'F';
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Printing: Action",
                                       debbuf,0);
                            }
                            debug(F111,"HP Print","action",action);
                            break;
                        case 'm':       /* Action */
                        case 'M':
                            action = 'M';
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Printing: Action",
                                       debbuf,0);
                            }
                            debug(F111,"HP Print","action",action);
                            break;
                        case 'c':       /* Action */
                        case 'C':
                            action = n;
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Printing: Action",
                                       debbuf,0);
                            }
                            debug(F111,"HP Print","action",action);
                            break;
                        case 'w':       /* Print ASCII string */
                        case 'W':
                            if ( n == 0 ) {
                                hpprt_term = LF;
                                hpprt_len = 256;
                                debug(F111,"HP Print","terminator",LF);
                                debug(F111,"HP Print","length",256);
                            }
                            else {
                                hpprt_len = n;
                                debug(F111,"HP Print","length",n);
                            }
                            action = 20;
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Printing: Print String",
                                       debbuf,0);
                            }
                            debug(F111,"HP Print","action",action);
                            break;
                        default:
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM unknown command",debbuf,0);
                        }
                    } while (x>0);
                    debug(F111,"HP Print","Final",ch3);

                    if ( debses )
                        break;

                    switch ( action ) {
                    case 1: {     /* Skip <x> lines */
                        int i;
                        for ( i=0;i<hpprt_term;i++ )
                            prtchar(LF);
                        break;
                    }
                    case 0:     /* Generate 1 form feed */
                    case 2:     /* Generate 1 form feed */
                    case 3:     /* Generate 1 form feed */
                    case 4:     /* Generate 1 form feed */
                    case 5:     /* Generate 1 form feed */
                    case 6:     /* Generate 1 form feed */
                    case 7:     /* Generate 1 form feed */
                    case 8:     /* Generate 1 form feed */
                    case 9:     /* Generate 1 form feed */
                    case 10:    /* Generate 1 form feed */
                        prtchar(FF);
                        break;
                    case 11:    /* Turns on log bottom mode */
                        setaprint(TRUE);
                        if ( !printon )
                            printeron();
                        break;
                    case 12:    /* Turns on log top mode */
                    case 13:    /* Turns off either log mode */
                        setaprint(FALSE);
                        if ( !uprint && !xprint && !cprint && !aprint && printon )
                            printeroff();
                        break;
                    case 14:    /* Print normal characters */
                    case 15:    /* Print expanded characters */
                    case 16:    /* Print compressed characters */
                    case 17:    /* Turns on report mode */
                    case 18:    /* Turns on metric report mode */
                    case 19:    /* Turns off any report mode */
                        break;
                    case 20:    /* Enables Record Mode */
                        xprint = TRUE ;
                        if ( !printon )
                            printeron() ;
                        break;
                    case 'B':    /* Print cursor line */
                        prtline(wherey[VTERM],LF);
                        break;
                    case 'F':    /* Copy from cursor to End of Screen */
                    case 'M':    /* Copy from cursor to End of Memory */
                        prtscreen(VTERM,wherey[VTERM],VscrnGetHeight(VTERM));
                        break;
                    case '^': {  /* Send device status */
                        char response[8];
                        response[0] = ESC;
                        response[1] = '\\';
                        response[2] = 'p';
                        response[3] = '0' + device;
                        response[4] = 48;       /* Printer Error */
                        response[5] = 56;       /* Printer Status */
                        response[6] = 49;       /* Printer Present */
                        response[7] = NUL;
                        debug(F110,"HP Print Status",response,0);
                        sendchars(response,7);
                        break;
                    }
                    default:
                        break;
                    }
                    break;
                }
                case 'q': {
                    /* Configuration Lock */
                    /*
                       0 L - Unlock Configuration
                       1 L - Lock Configuration
                    */
                    int ch3;
                    do {
                        if ((x = hpparam(&n,&ch3,&y)) < 0)
                            break;
                        if ( debses )
                            continue;
                        switch ( ch3 ) {
                        case 'l':       /* configuration lock */
                            switch ( n ) {
                            case 0:
                                /* unlock */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Configuration: Unlock (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 1:
                                /* lock */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Configuration: Lock (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            }
                            break;
                        default:
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM unknown command",debbuf,0);
                        }
                    } while (x>0);
                    if ( x < 0 )
                        break;
                    break;
                }
                case 'r': {     /* Roll */
                    /* <n> d <n> l <n> r <n> u */
                    int ch3;
                    do {
                        if ((x = hpparam(&n,&ch3,&y)) < 0)
                            break;
                        if ( debses )
                            continue;
                        switch ( ch3 ) {
                        case 'd':       /* roll down n */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Roll: Down (unsupported)",
                                       debbuf,0);
                            }
                            break;
                        case 'l':       /* roll left n */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Roll: Left (unsupported)",
                                       debbuf,0);
                            }
                            break;
                        case 'r':       /* roll right n */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Roll: Right (unsupported)",
                                       debbuf,0);
                            }
                            break;
                        case 'u':       /* roll up n */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Roll: Up (unsupported)",
                                       debbuf,0);
                            }
                            break;
                        default:
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM unknown command",debbuf,0);
                        }
                    } while (x>0);
                    if ( x < 0 )
                        break;
                    break;
                }
                case 's': {
                    /* looks like it can handle multiple inputs of the form */
                    /* <ESC>&s0a0b1c0d0g0H */
                    /* <ESC>&s0b0c0D <ESC>&s0A */

                    int ch3=0;
                    do {
                        if ((x = hpparam(&n,&ch3,&y)) < 0)
                            break;
                        if ( debses )
                            continue;
                        switch ( ch3 ) {
                        case 'a':       /* HPTerm keypad controls */
                        case 'A':       /* Strap A: transmit function keys */
                            switch ( n ) {
                            case 0:
                                /* turn off keypad */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Strap A: Keypad off (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 1:
                                /* turn on keypad */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Strap A: Keypad On (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            }
                            break;
                        case 'b':
                        case 'B':       /* Strap B: Space Overwrite */
                            switch ( n ) {
                            case 0:
                                /* turn off */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Strap B: Space Overwrite Off (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 1:
                                /* turn on  */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Strap B: Space Overwrite On (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            }
                            break;
                        case 'c':
                        case 'C':
                            /* Strap C: Inhibit word wrap */
                            switch ( n ) {
                            case 0:
                                /* turn off wrap */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Strap C: Word Wrap Off",
                                           debbuf,0);
                                }
                                if ( debses )
                                    break;
                                tt_wrap = TRUE;
                                break;
                            case 1:
                                /* turn on wrap */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Strap C: Word Wrap On",
                                           debbuf,0);
                                }
                                if ( debses )
                                    break;
                                tt_wrap = FALSE;
                                break;
                            }
                            break;
                        case 'd':
                        case 'D':
                            /* Strap D: Page/Line */
                            switch ( n ) {
                            case 0:
                                /* Line at a time */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Strap D: Block-Mode is Line at a time (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 1:
                                /* Page at a time */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Strap D: Block-Mode is Page at a time (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            }
                            break;

                            /* Straps G and H work in tandem.  */
                            /* G0 H0 - Data Transfers use DC1/DC2 Handshake.  */
                            /*   Other transfers triggered by receipt of DC1  */
                            /* G0 H1 - Data is sent when the host receives a  */
                            /*   DC2.  All other transfers are triggered by   */
                            /*   receipt of DC1.                              */
                            /* G1 H0 - All block transfers require a DC1/DC2  */
                            /*   handshake.                                   */
                            /* G1 H1 - No DC1/DC2 handshake is required for   */
                            /*    any block transfer.                         */

                        case 'g':
                        case 'G':
                            /* Strap G: DC2 Handshaking */
                            switch ( n ) {
                            case 0: /* Off */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Strap G: Off (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 1: /* On */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Strap G: On (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            }
                            break;
                        case 'h':
                        case 'H':
                            /* Strap H: Inhibit DC2 */
                            switch ( n ) {
                            case 0: /* Off */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Strap H: Off (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 1: /* On */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %c %d %c%s",
                                             ch, ch2, n, ch3, y ? " (relative)" : "");
                                    debug(F110,"HPTERM Strap H: On (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            }
                            break;
                        default:
                            if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM unknown command",debbuf,0);
                            }
                        }
                    } while (x>0);
                    break;
                }
                case 'v': {     /* Color sequences */
                    int ch3;
                    do {
                        if ((x = hpparam(&n,&ch3,&y)) < 0)
                            break;
                        if ( debses )
                            continue;
                        switch ( ch3 ) {
                        default:
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM unknown command",debbuf,0);
                        }
                    } while (x>0);
                }
                default:
                    sprintf(debbuf,"ESC %c %c", ch, ch2);
                    debug(F110,"HPTERM unknown command",debbuf,0);
                }
                break;
            }
#ifdef COMMENT
            case '\'':
                break;
#endif /* COMMENT */
            case '(': { /* Select Specific Primary Character Set */
                int ch2;
                int len=0;
                do {
                    if ((x = hpparam(&n,&ch2,&y)) < 0)
                        break;
                    switch ( ch2 ) {
                    case '@':   /* Select Primary Character Set */
                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                    case 'y':
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %d %c%s",
                                     ch, n, ch2, y ? " (relative)" : "");
                            debug(F110,"HPTERM Select Primary Character Set (unsupported)",
                                   debbuf,0);
                            }
                        break;
                    case 's': { /* Define Primary Char Set Attributes */
                        int ch3;
                        do {
                            if ((x = hpparam(&m,&ch3,&z)) < 0)
                                break;
                            switch ( ch3 ) {
                            case 'b':   /* Stroke Weight */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Primary Char Set Attributes: Stroke Weight (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'h':   /* Pitch */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Primary Char Set Attributes: Pitch (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'n':   /* Font name */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Primary Char Set Attributes: Font name (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'p':   /* Fixed or Proportional Spacing */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Primary Char Set Attributes: Fixed or Proportional Spacing (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'q':   /* Density */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Primary Char Set Attributes: Density (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 's':   /* Style */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Primary Char Set Attributes: Style (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 't':   /* Typeface */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Primary Char Set Attributes: Typeface (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'u':   /* Superior or inferior style */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Primary Char Set Attributes: Superior or Inferior (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'v':   /* Height in points */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Primary Char Set Attributes: Height (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'w':   /* Length of download characters */
                                len = m;
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Primary Char Set Attributes: Length of download (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            default:
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Unknown",
                                           debbuf,0);
                                }
                            }
                        } while (x>0);
                        break;
                    }
                    default:
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %d %c%s",
                                     ch, n, ch2, y ? " (relative)" : "");
                            debug(F110,"HPTERM Unknown",
                                   debbuf,0);
                        }
                    }
                } while (x>0);
                /* If there is download data, swallow it */
                if ( len ) {
                    while ( len-- )
                        hpinc();
                }
                break;
            }
            case ')': { /* Select Specific Secondary Character Set */
                int ch2;
                int len=0;
                do {
                    if ((x = hpparam(&n,&ch2,&y)) < 0)
                        break;
                    switch ( ch2 ) {
                    case '@':   /* Select Secondary Character Set */
                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                    case 'y':
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %d %c%s",
                                     ch, n, ch2, y ? " (relative)" : "");
                            debug(F110,"HPTERM Select Secondary Character Set (unsupported)",
                                   debbuf,0);
                            }
                        break;
                    case 'f': { /* Reserved */
                        int ch3;
                        do {
                            if ((x = hpparam(&m,&ch3,&z)) < 0)
                                break;
                            switch ( ch3 ) {
                            case 'h':   /* reserved */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Secondary Char Set Attributes: Reserved (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            default:
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Unknown",
                                           debbuf,0);
                                }
                            }
                        } while (x>0);
                        break;
                    }
                    case 's': { /* Define Secondary Char Set Attributes */
                        int ch3;
                        do {
                            if ((x = hpparam(&m,&ch3,&z)) < 0)
                                break;
                            switch ( ch3 ) {
                            case 'b':   /* Stroke Weight */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Secondary Char Set Attributes: Stroke Weight (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'h':   /* Pitch */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Secondary Char Set Attributes: Pitch (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'n':   /* Font name */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Secondary Char Set Attributes: Font name (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'p':   /* Fixed or Proportional Spacing */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Secondary Char Set Attributes: Fixed or Proportional Spacing (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'q':   /* Density */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Secondary Char Set Attributes: Density (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 's':   /* Style */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Secondary Char Set Attributes: Style (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 't':   /* Typeface */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Secondary Char Set Attributes: Typeface (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'u':   /* Superior or inferior style */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Secondary Char Set Attributes: Superior or Inferior (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'v':   /* Height in points */
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Secondary Char Set Attributes: Height (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            case 'w':   /* Length of download characters */
                                len = m;
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Set Secondary Char Set Attributes: Length of download (unsupported)",
                                           debbuf,0);
                                }
                                break;
                            default:
                                if ( deblog ) {
                                    sprintf(debbuf,
                                             "ESC %c %d %c %d %c%s",
                                             ch, n, ch2, m, ch3,
                                             z ? " (relative)" : "");
                                    debug(F110,"HPTERM Unknown",
                                           debbuf,0);
                                }
                            }
                        } while (x>0);
                        break;
                    }
                    default:
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %d %c%s",
                                     ch, n, ch2, y ? " (relative)" : "");
                            debug(F110,"HPTERM Unknown",
                                   debbuf,0);
                        }
                    }
                } while (x>0);
                /* If there is download data, swallow it */
                if ( len ) {
                    while ( len-- )
                        hpinc();
                }
                break;
            }
            case '*': {
                /* ESC * d Q   Cursor On */
                /* ESC * d R   Cursor Off */
                /* ESC * s ^   Identify */
                /* ESC * s 1 ^ Identify */
                int ch2, ch3;
                ch2 = hpinc() ;
                if ( debses )
                    break;
                switch ( ch2 ) {
                case 'd':
                    do {
                        if ((x = hpparam(&n,&ch3,&y)) < 0)
                            break;
                        if ( debses )
                            continue;
                        switch ( ch3 ) {
                        case 'q':       /* Cursor On */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Cursor On",
                                       debbuf,0);
                            }
                            if ( debses )
                                break;
                            cursorena[VTERM] = TRUE;
                            break;
                        case 'r':       /* Cursor Off */
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Cursor Off",
                                       debbuf,0);
                            }
                            if ( debses )
                                break;
                            cursorena[VTERM] = FALSE ;
                            break;
                        default:
                            if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM unknown command",debbuf,0);
                            }
                            break;
                        }
                    } while (x>0);
                    break;
                case 's':
                    do {
                        if ((x = hpparam(&n,&ch3,&y)) < 0)
                            break;
                        if ( debses )
                            continue;
                        switch ( ch3 ) {
                        case '^':
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %c %d %c%s",
                                         ch, ch2, n, ch3, y ? " (relative)" : "");
                                debug(F110,"HPTERM Identify Terminal",
                                       debbuf,0);
                            }
                            if ( debses )
                                break;
                            sendchars( tt_info[tt_type].x_id,
                                       strlen(tt_info[tt_type].x_id) ) ;
                            break;
                        default:
                            if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %c %d %c%s",
                                     ch, ch2, n, ch3, y ? " (relative)" : "");
                            debug(F110,"HPTERM unknown command",debbuf,0);
                            }
                            break;
                        }
                    } while (x>0);
                    break;
                default:
                    /* ignore everything we don't understand */
                    if ( deblog ) {
                        sprintf(debbuf,"ESC %c %c", ch, ch2);
                        debug(F110,"HPTERM unknown command",debbuf,0);
                    }
                }
                break;
            }
#ifdef COMMENT
            case '+':
                break;
            case ',':   /* Status responses from Kanji Terminals */
                break;
#endif /* COMMENT */
            case '-':
            case '.':
            case '/': {
                int ch2;
                int len=0;
                do {
                    if ((x = hpparam(&n,&ch2,&y)) < 0)
                        break;
                    if ( x == 0 ) {
                        if ( deblog ) {
                            sprintf(debbuf,
                                     "ESC %c %d %c%s",
                                     ch, n, ch2, y ? " (relative)" : "");
                            debug(F110,"HPTERM Select Primary Character Set (unsupported)",
                                   debbuf,0);
                        }
                    }
                    else {
                        int ch3;
                        do {
                            if ((x = hpparam(&m,&ch3,&z)) < 0)
                                break;
                            if ( deblog ) {
                                sprintf(debbuf,
                                         "ESC %c %d %c %d %c%s",
                                         ch, n, ch2, m, ch3,
                                         z ? " (relative)" : "");
                                debug(F110,"HPTERM Unknown",
                                       debbuf,0);
                            }
                        } while (x>0);
                    }
                } while (x>0);
                break;
            }
            case '0':
                /* Print All */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Print All",
                           debbuf,0);
                }
                if ( debses )
                    break;
                prtscreen(VTERM,1,VscrnGetHeight(VTERM));
                break;
            case '1':
                /* Sets a tab stop */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Set Tab Stop",
                           debbuf,0);
                }
                if ( debses )
                    break;
                htab[wherex[VTERM]] = 'T';
                break;
            case '2':
                /* Clears a tab stop */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Clear Tab Stop",
                           debbuf,0);
                }
                if ( debses )
                    break;
                htab[wherex[VTERM]] = '0';
                break;
            case '3':
                /* Clears all tab stops */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Clear All Tab Stops",
                           debbuf,0);
                }
                if ( debses )
                    break;
                for (j = 1; j <=MAXTERMCOL; ++j)
                    htab[j] = '0';
                break;
            case '4':
                /* Set Left Margin */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Set Left Margin",
                           debbuf,0);
                }
                if ( debses )
                    break;
                marginleft = wherex[VTERM];
                break;
            case '5':
                /* Set Right Margin */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Set Right Margin",
                           debbuf,0);
                }
                if ( debses )
                    break;
                marginright = wherex[VTERM];
                break;
#ifdef COMMENT
            case '6':
                break;
            case '7':
                break;
            case '8':
                break;
#endif /* COMMENT */
            case '9':
                /* Clear margins */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Clear Margins",
                           debbuf,0);
                }
                if ( debses )
                    break;
                margintop = 1;
                marginbot = VscrnGetHeight(VTERM);
                marginleft =1;
                marginright=VscrnGetWidth(VTERM);
                break;
#ifdef COMMENT
            case ':':
                break;
            case ';':
                break;
#endif /* COMMENT */
            case '<': {
                /* <n> <alpha>    Change Base Font (G1) */
                /* 8U - Roman 8 */
                /* 8K - Kana 8  */
                /* 8M - Math/Special Symbols 8 */
                /* 0L - Line Draw 8 */
                char n, alpha;

                n = hpinc();
                alpha = hpinc();

                if ( deblog ) {
                    sprintf(debbuf, "ESC %c %c %c", ch, n, alpha);
                    debug(F110,"HPTERM Change Base Font (G1)",
                           debbuf,0);
                }

                if ( debses )
                    break;
                if ( n == 8 )
                    n = '8';
                if ( n == 0 )
                    n = '0';

                if ( n == '8' ) {
                    switch ( alpha ) {
                    case 'U':
                        charset( cs94, 'B', &G[0] );
                        charset( cs96, 'h', &G[1] );
                        break;
                    case 'K':
                        charset( cs96, 'h', &G[0] );
                        charset( cs96, 'h', &G[1] );
                        break;
                    case 'M':
                        charset( cs96, 'i', &G[0] );
                        charset( cs96, 'i', &G[1] );
                        break;
                    }
                }
                else if ( n == '0' && alpha == 'L' ) {
                    charset( cs96, 'j', &G[0] );
                    charset( cs96, 'j', &G[1] );
                }
                break;
            }
#ifdef COMMENT
            case '=':
                break;
#endif /* COMMENT */
            case '>': {
                /* <n> <alpha>    Change Alternate Font (G2) */
                /* 8U - Roman 8 */
                /* 8K - Kana 8  */
                /* 8M - Math/Special Symbols 8 */
                /* 0L - Line Draw 8 */
                char n, alpha;
                n = hpinc();
                alpha = hpinc();

                if ( deblog ) {
                    sprintf(debbuf, "ESC %c %c %c", ch, n, alpha);
                    debug(F110,"HPTERM Change Alternate Font (G2)",
                           debbuf,0);
                }

                if ( debses )
                    break;
                if ( n == 8 )
                    n = '8';
                if ( n == 0 )
                    n = '0';

                if ( n == '8' ) {
                    switch ( alpha ) {
                    case 'U':
                        charset( cs94, 'B', &G[2] );
                        charset( cs96, 'h', &G[3] );
                        break;
                    case 'K':
                        charset( cs96, 'h', &G[2] );
                        charset( cs96, 'h', &G[3] );
                        break;
                    case 'M':
                        charset( cs96, 'i', &G[2] );
                        charset( cs96, 'i', &G[3] );
                        break;
                    }
                }
                else if ( n == '0' && alpha == 'L' ) {
                    charset( cs96, 'j', &G[2] );
                    charset( cs96, 'j', &G[3] );
                }
                break;
            }
#ifdef COMMENT
            case '?':
                break;
#endif /* COMMENT */
            case '@':
                /* Delay 1 second */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Delay 1 Second",
                           debbuf,0);
                }
                if ( debses )
                    break;
                sleep(1);
                break;
            case 'A':
                /* Cursor Up */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Cursor Up",
                           debbuf,0);
                }
                if ( debses )
                    break;

                cursorup(0);
                break;
            case 'B':
                /* Cursor Down */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Cursor Down",
                           debbuf,0);
                }
                if ( debses )
                    break;

                cursordown(0);
                break;
            case 'C':
                /* Cursor Right */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Cursor Right",
                           debbuf,0);
                }
                if ( debses )
                    break;

                cursorright(0);
                break;
            case 'D':
                /* Cursor Left */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Cursor Left",
                           debbuf,0);
                }
                if ( debses )
                    break;

                cursorleft(0);
                break;
            case 'E':
                /* Reset Terminal */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Reset Terminal",
                           debbuf,0);
                }
                if ( debses )
                    break;
                doreset(1);
                break;
            case 'F':
                /* Cursor Home Down */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Home Down",
                           debbuf,0);
                }
                if ( debses )
                    break;

                lgotoxy(VTERM, 1, VscrnGetHeight(VTERM));
                break;
            case 'G':
                /* Cursor Return */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Cursor Return",
                           debbuf,0);
                }
                if ( debses )
                    break;

                wrtch(CR);
                break;
            case 'H':
                /* Cursor Home Up */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Cursor Home Up",
                           debbuf,0);
                }
                if ( debses )
                    break;

                lgotoxy(VTERM,1,1);
                break;
            case 'I':
                /* Horizontal Tab */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Horizontal Tab",
                           debbuf,0);
                }
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
            case 'J':
                /* Clear Display */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Clear Display",
                           debbuf,0);
                }

                if ( debses )
                    break;
                clreoscr_escape(VTERM,SP);
                break;
            case 'K':
                /* Erase to End of Line */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Erase to End of Line",
                           debbuf,0);
                }
                if ( debses )
                    break;
                clrtoeoln(VTERM,SP);
                break;
            case 'L':
                /* Insert Line */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Insert Line",
                           debbuf,0);
                }
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
            case 'M':
                /* Delete Line */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Delete Line",
                           debbuf,0);
                }
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
#ifdef COMMENT
            case 'N':
                break;
            case 'O':
                break;
#endif /* COMMENT */
            case 'P':
                /* Delete Char */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Delete Char",
                           debbuf,0);
                }
                if ( debses )
                    break;
                blankvcell.c = SP;
                blankvcell.a = geterasecolor(VTERM);
                VscrnScrollLf(VTERM, wherey[VTERM] - 1,
                               wherex[VTERM] - 1,
                               wherey[VTERM] - 1,
                               VscrnGetWidth(VTERM) - 1,
                               1, blankvcell) ;

                break;
            case 'Q':
                /* Inserts Character On */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Inserts Character On",
                           debbuf,0);
                }
                if ( debses )
                    break;
                insertmode = TRUE ;
                break;
            case 'R':
                /* Insert Character Off */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Inserts Character Off",
                           debbuf,0);
                }
                if ( debses )
                    break;
                insertmode = FALSE ;
                break;
            case 'S':
                /* Roll Up */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Roll Up",
                           debbuf,0);
                }
                if ( debses )
                    break;
                VscrnScroll(VTERM,
                             UPWARD,
                             0,
                             VscrnGetHeight(VTERM)
                             -(tt_status[VTERM]?2:1),
                             1,
                             TRUE,
                             SP);
                break;
            case 'T':
                /* Roll Down */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Roll Down",
                           debbuf,0);
                }
                if ( debses )
                    break;
                VscrnScroll(VTERM,
                             DOWNWARD,
                             0,
                             VscrnGetHeight(VTERM)
                             -(tt_status[VTERM]?2:1),
                             1,
                             TRUE,
                             SP);
                break;
            case 'U':
                /* Next Page */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Next Page (unsupported)",
                           debbuf,0);
                }
                break;
            case 'V':
                /* Previous Page */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Previous Page (unsupported)",
                           debbuf,0);
                }
                break;
#ifdef COMMENT
            case 'W':
                break;
            case 'X':
                break;
#endif /* COMMENT */
            case 'Y':
                /* Display Functions On */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Display Functions On",
                           debbuf,0);
                }
                if ( debses || wy_monitor )
                    break;
                setdebses(TRUE);
                wy_monitor = TRUE ;
                break;
            case 'Z':
                /* Display Functions Off */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Display Functions Off",
                           debbuf,0);
                }
                if ( !wy_monitor )
                    break;
                setdebses(FALSE);
                wy_monitor = FALSE ;
                break;
#ifdef COMMENT
            case '[':
                break;
            case '\\':
                break;
            case ']':
                break;
#endif /* COMMENT */
            case '^': {
                /* Term Primary Status */
                /* Respond with ESC \ terminal primary status bytes (Apdx C) */
                char buf[11];
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Primary Status Request",
                           debbuf,0);
                }
                if ( debses )
                    break;
                buf[0] = ESC ;
                buf[1] = '\\';
                buf[2] = '4';
                buf[3] = '4';
                buf[4] = '0';
                buf[5] = '8';
                buf[6] = '0';
                buf[7] = '2';
                buf[8] = '0';
                buf[9] =  CR;
                buf[10] = NUL;
                sendchars(buf,10);
                break;
            }
#ifdef COMMENT
            case '_':
                break;
#endif /* COMMENT */
            case '`':
                /* Relative Cursor Sense */
                /* Respond with ESC & a column number x screen row Y */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Relative Cursor Request",
                           debbuf,0);
                }
                if ( !debses )
                {
                    char buf[20] ;
                    sprintf(buf,"%c&a%03dx%03dY\r",ESC,wherex[VTERM]-1,wherey[VTERM]-1);
                    sendchars(buf,strlen(buf));
                }
                break;
            case 'a':
                /* Absolute Cursor Sense */
                /* Respond with ESC & a column number c memory row R */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Absolute Cursor Request",
                           debbuf,0);
                }
                if ( !debses )
                {
                    char buf[20] ;
                    sprintf(buf,"%c&a%03dc%03dR\r",ESC,wherex[VTERM]-1,wherey[VTERM]-1);
                    sendchars(buf,strlen(buf));
                }
                break;
            case 'b':
                /* Enable keyboard */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Enable Keyboard",
                           debbuf,0);
                }
                if ( debses )
                    break;
                keylock = FALSE ;
                break;
            case 'c':
                /* Disable keyboard */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Disable Keyboard",
                           debbuf,0);
                }
                if ( debses )
                    break;
                keylock = TRUE ;
                break;
            case 'd':
                /* Enter (Send current line) */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Send Current Line",
                           debbuf,0);
                }
                if ( !debses )
                {
                    int x;
                    for ( x=wherex[VTERM]-1;x<VscrnGetWidth(VTERM);x++ ) {
                        if ( !VscrnGetVtCharAttr(VTERM, x, wherey[VTERM]-1).unerasable ) {
                            unsigned short ch = VscrnGetCell( VTERM, x, wherey[VTERM]-1 )->c ;
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
                    sendchar(CR);
                }
                break;
#ifdef COMMENT
            case 'e':
                break;
#endif /* COMMENT */
            case 'f':
                /* Modem Disconnect */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Modem Disconnect (unsupported)",
                           debbuf,0);
                }

                break;
#ifdef COMMENT
            case 'g':
                break;
#endif /* COMMENT */
            case 'h':
                /* Home Cursor Up */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Home Cursor Up",
                           debbuf,0);
                }
                if ( debses )
                    break;

                lgotoxy(VTERM,1,1);
                break;
            case 'i':
                /* Back Tab */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Back Tab",
                           debbuf,0);
                }
                if ( debses )
                    break;
                i = wherex[VTERM];
                if ( i == 1 && wherey[VTERM] != 1 ) {
                    wherex[VTERM] = VscrnGetWidth(VTERM);
                    cursorup(0);
                }
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
                }
                VscrnIsDirty(VTERM);
                break;
#ifdef COMMENT
            case 'j':
                break;
            case 'k':
                break;
            case 'l':   /* Lock Memory - HP Term ??? */
                break;
            case 'm':   /* Unlock Memory - HP Term ??? */
                break;
            case 'n':
                break;
            case 'o':
                break;
#endif /* COMMENT */
            case 'p':
            case 'q':
            case 'r':
            case 's':
            case 't':
            case 'u':
            case 'v':
            case 'w': {
                /* set Softkey default values */
                char string[3];
                int keyi;

                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Set Softkey Default Value",
                           debbuf,0);
                }
                if ( debses )
                    break;
                sprintf(string,"%c%c",ESC,ch);
                keyi = ch - 'p';
                if ( udkfkeys[keyi] ){
                    free( udkfkeys[keyi] ) ;
                    udkfkeys[keyi]=NULL;
                }
                if ( strlen(string) )
                    udkfkeys[keyi] = strdup(string) ;
                break;
            }
#ifdef COMMENT
            case 'x':
                break;
            case 'y':
                break;
#endif /* COMMENT */
            case 'z':
                /* Terminal Self Test */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Terminal Self Test (unsupported)",
                           debbuf,0);
                }
                break;
#ifdef COMMENT
            case '{':
                break;
            case '|':
                break;
            case '}':
                break;
#endif /* COMMENT */
            case '~':
                /* Secondary Terminal Status report */
                if ( deblog ) {
                    sprintf(debbuf, "ESC %c", ch);
                    debug(F110,"HPTERM Secondary Status Report (unsupported)",
                           debbuf,0);
                }
                break;
#ifdef COMMENT
            case DEL:
                break;
#endif /* COMMENT */
            default:
                sprintf(debbuf,
                         "ESC %c",ch);
                debug(F110,"HPTERM unknown command",debbuf,0);
            }
            escstate = ES_NORMAL ;      /* Done parsing escstate sequence */
        }
    }
    else                /* Handle as a normal character */
    {
        if ( ch < SP || ch == DEL )
            hpctrl(ch) ;
        else if ( !debses) {
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
            else {
                vtattrib vta = VscrnGetVtCharAttr( VTERM, wherex[VTERM]-1, wherey[VTERM]-1 ) ;
                if ( vta.wyseattr ) {
                    attrib.wyseattr = TRUE;
                    wrtch(ch);
                    attrib.wyseattr = FALSE;
                }
                else {
                    wrtch(ch);
                }
            }
        }
    }
    VscrnIsDirty(VTERM) ;
    if ( attr_y != wherey[VTERM] ) {
        hpattroff();
        attr_y = wherey[VTERM];
    }
}
#endif /* NOTERM */
