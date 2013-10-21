/*  C K O I 3 1 . C  --  IBM 31x1 Emulation  */

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
#include "ckcuni.h"
#include "ckuusr.h"
#include "ckocon.h"
#include "ckoi31.h"

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
extern char termessage[] ;
extern int autoscroll, protect ;
extern struct _vtG G[4];
extern struct _vtG *GL, *SSGL;          /* GL and single shift GL */
extern struct _vtG *GR;                 /* GR */
extern int tcsl;

int i31_monitor = FALSE ;
int i31_xprint  = 0;
int i31_lta     = CR;

int
i31inc(void)
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

/* i31rdctrl - is used to read Set Control command sequences */
/* returns a bit flag of which parameters were received      */
#define IBM_PA1 1
#define IBM_PA2 2
#define IBM_PA3 4
#define IBM_PA4 8
#define IBM_OP  16

#define GOT_PA1(x) (x & IBM_PA1)
#define GOT_PA2(x) (x & IBM_PA2)
#define GOT_PA3(x) (x & IBM_PA3)
#define GOT_PA4(x) (x & IBM_PA4)
#define GOT_OP(x)  (x & IBM_OP)

#define BIT1 0x01
#define BIT2 0x02
#define BIT3 0x04
#define BIT4 0x08
#define BIT5 0x10
#define BIT6 0x20
#define BIT7 0x40
#define BIT8 0x80

int
i31rdctrl( int m, int * pa1, int * pa2, int * pa3, int * pa4, int * op )
{
    int n=0;
    int *p=NULL;

    if ( m >= 1 ) {
        *pa1 = i31inc();
        if ( (*pa1) < 0 )
            return(n);
        n += IBM_PA1;

        if ( m >= 2 ) {
            if ( (*pa1 & 0x60) == 0x20 ) {
                *pa2 = i31inc();
                if ( (*pa2) < 0 )
                    return(n);
                n += IBM_PA2;
            }
            if ( m >= 3 ) {
                if ( (*pa2 & 0x60) == 0x20 ) {
                    *pa3 = i31inc();
                    if ( (*pa3) < 0 )
                        return(n);
                    n += IBM_PA3;
                }
                if ( m >= 4 ) {
                    if ( (*pa3 & 0x60) == 0x20 ) {
                        *pa4 = i31inc();
                        if ( (*pa4) < 0 )
                            return(n);
                        n += IBM_PA4;
                    }
                }

            }
        }

        /* Now take care of op */
        switch ( m ) {
        case 1:
            p = pa1;
            break;
        case 2:
            p = pa2;
            break;
        case 3:
            p = pa3;
            break;
        case 4:
            p = pa4;
            break;
        }
        if ( (*p & 0x60) == 0x20 ) {
            *op  = i31inc();
            if ( (*op) < 0 )
                return(n);
            n += IBM_OP;
        }
    }
    return(n);
}

void
i31ctrl( int ch )
{
    int i,j;

    if ( !cprint && xprint ) {
        switch ( ch ) {
        case DLE: {
            int nextch = i31inc() ;
            switch ( nextch ) {
            case DC2:       /* Begin Transparent Print */
                if ( debses )
                    break;
                i31_xprint++;
                if ( i31_xprint > 0 ) {
                    xprint = TRUE;
                    if ( !printon )
                        printeron();
                }
                /* Now that we have increased our counter, pass it on */
                prtchar(ch);
                prtchar(nextch);
                break;
            case DC4:       /* End Transparent Print */
                if ( debses )
                    break;
                if ( i31_xprint > 0 )
                    i31_xprint--;
                if ( !i31_xprint ) {
                    xprint = FALSE ;
                    if ( !uprint && !xprint && !cprint && !aprint && printon )
                        printeroff();
                }
                if ( i31_xprint > 0 ) {
                    /* we are not done yet, pass it on */
                    prtchar(ch);
                    prtchar(nextch) ;
                }
                break;
            default:
                prtchar(ch);
                prtchar(nextch) ;
                break;
            }
            break;
        }
        default:
            prtchar(ch);
            break;
        }
    } else {
        switch ( ch ) {
        case SOH:
            break;
        case STX:
            break;
        case ETX:
            break;
        case EOT:
            break;
        case ENQ: {
            break;
        }
        case ACK:
            break;
        case BEL:       /* Sound bell */
            if ( debses )
                break;
            bleep(BP_BEL);
            break;
        case BS:
            /* cursor left (backspace) */
            if ( debses )
                break;
            cursorleft(0) ;
            break;
        case HT:
            /* Tabulate cursor */
            if ( debses )
                break;
            i = wherex[VTERM];
            if (i < VscrnGetWidth(VTERM))
            {
                do {
                    i++;
                    cursorright(0);
                } while ((htab[i] != 'T') &&
                          (i <= VscrnGetWidth(VTERM)-1));
                VscrnIsDirty(VTERM);
            }
            break;
        case LF:
            /* cursor down; scroll */
            if ( debses )
                break;
            wrtch(LF);
            break;
        case VT:
            break;
        case FF:
            break;
        case CR:
            /* cursor to start of line */
            if ( debses )
                break;
            wrtch(CR);
            break;
        case SO:
            /* 3161- Select primary character set */
            if ( debses )
                break;
            GL = &G[0] ;
            break;
        case SI:
            /* 3161 - Select secondary character set */
            if ( debses )
                break;
            GL = &G[1] ;
            break;
        case DLE: {
            int nextch = i31inc() ;
            switch ( nextch ) {
            case STX:
                if ( debses || i31_monitor )
                    break;
                i31_monitor = TRUE ;
                setdebses(TRUE);
                break;
            case ETX:
                if ( !i31_monitor )
                    break;
                i31_monitor = FALSE ;
                setdebses(FALSE);
                break;
            case EOT:
                if ( debses )
                    break;
                strcpy( termessage, "Disconnected by host" ) ;
                ttclos(0);
                break;
            case DC2:       /* Begin Transparent Print */
                if ( debses )
                    break;
                i31_xprint++;
                if ( i31_xprint > 0 ) {
                    xprint = TRUE;
                    if ( !printon )
                        printeron();
                }
                break;
            case DC4:       /* End Transparent Print */
                if ( debses )
                    break;
                if ( i31_xprint > 0 )
                    i31_xprint--;
                if ( !i31_xprint ) {
                    xprint = FALSE ;
                    if ( !uprint && !xprint && !cprint && !aprint && printon )
                        printeroff();
                }
                break;
            default:
                i31ascii(nextch) ;
                break;
            }
            break;
        }
        case DC1:
            break;
        case DC2:
            break;
        case DC3:
            break;
        case DC4:
            break;
        case NAK:
            break;
        case SYN:
            break;
        case ETB:
            break;
        case CAN:
            break;
        case XEM:
            break;
        case SUB:
            break;
        case ESC:
            /* initiate escape sequence */
            escstate = ES_GOTESC ;
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
    } /* xprint */
}

void
i31ascii( int ch )
{
    int i,j,k,n,x,y,z;
    vtattrib attr ;
    viocell blankvcell;
    int ch2=0, ch3=0, pa=0, pa1=0, pa2=0, pa3=0, pa4=0, op=0;
    char response[32]="";

    if ( escstate == ES_GOTESC )/* Process character as part of an escstate sequence */
    {
        /* should never be able to get here if xprint is active */

        if ( ch < SP ) {
            escstate = ES_NORMAL ;
            i31ctrl(ch) ;
        }
        else
        {
            escstate = ES_ESCSEQ ;
            switch ( ch ) {
            case '!': {
                ch2 = i31inc() ;
                switch ( ch2 ) {
                case '6':
                    /* 3161 - Send terminal ID */
                    if ( debses )
                        break;
                    /* send ESC ! 6 pa ESC 6 LTA */
                    break;
                case '7':
                    /* 3161 - Read Control 2 */
                    /* Machine Status of the 3151 */
                    if ( debses )
                        break;
                    break;
                case '8':
                    /* 3161 - Send cursor line */
                    if ( debses )
                        break;
                    break;
                case '9': {
                    /* 3161 - Set Control 2 */
                    /* Machine Status of the 3151 */
                    if ( debses )
                        break;
                    n = i31rdctrl( 2, &pa1, &pa2, &pa3, &pa4, &op );
                    if ( debses || n == 0 )
                        break;

                    if ( GOT_OP(n) ) {
                        if ( (op & BIT7|BIT6) == 0x60 )
                        switch ( (op & 0x1F) ) {
                        case 0x00:      /* Replacement (default) */
                            break;
                        case 0x01:      /* Logical OR */
                            break;
                        case 0x02:      /* Logical AND */
                            break;
                        case 0x1F:      /* Interpreted as the DEL character */
                            break;
                        }
                    }

                    /* The following lines must be merged in to the above */
                    /* switch to enable the replace|and|or functionality  */
                    if ( GOT_PA1(n) ) {
                        /* Forcing Insert Line */
                        if ( pa1 & 0x10 )
                            ;   /* Enabled */
                        else
                            ;   /* Disabled (default) */

                        /* Enter Key */
                        if ( pa1 & 0x08 )
                            ;   /* Works as the Return key (default) */
                        else
                            ;   /* Works as the Send key */

                        /* CRT Saver */
                        if ( pa1 & 0x03 )
                            ;   /* 15 minutes */
                        else
                            ;   /* No saver (default) */
                    }
                    if ( GOT_PA2(n) ) {
                        /* Forcing Insert Character */
                        if ( pa2 & 0x10 )
                            ;   /* Enabled */
                        else
                            ;   /* Disabled (default) */

                        /* Set Field Attribute (pa2 & 0x08) */
                        /* should always be 1               */

                        /* Operator Initiated Transparent Mode */
                        if ( pa2 & 0x03 )
                            ;   /* Disabled */
                        else
                            ;   /* Enabled (default) */

                        /* New Line */
                        if ( pa2 & 0x02 )
                            ;   /* Return key generates CR (default) */
                        else
                            ;   /* Return key generates CR/LF */

                    }
                    break;
                }
                case 'L':
                    /* 3161 - Clear all */
                    if ( debses )
                        break;
                    clrscreen( VTERM, NUL );
                    lgotoxy(VTERM,1,1);       /* and home the cursor */
                    break;
                case 'M':
                    /* Reverse Index command */
                    if ( debses )
                        break;
                    break;
                case 'q':
                    /* 3161 - Select active partition */
                    /* Affects the viewport which local operator */
                    /* commands affect.  Cursor moves to active partition */
                    pa = i31inc();
                    if ( debses )
                        break;
                    switch ( pa ) {
                    case 'A':   /* viewport 1 */
                        break;
                    case 'B':   /* viewport 2 */
                        break;
                    case 'C':   /* viewport 3 */
                        break;
                    }
                    break;
                case 's':
                    /* 3161 - Reset keyboard and clear modified */
                    /*        data tag                          */
                    if ( debses )
                        break;
                    break;
                case 'S':
                    /* Reset Keyboard Lock and MDT Bit Command */
                    if ( debses )
                        break;
                    break;
                case '=': {
                    /* 3161 - Program function key definition */
                    int fn=0,fnx=0,ff=0;
                    char fp[128]="";
                    fn = i31inc();
                    if ( fn & (BIT5|BIT4|BIT3|BIT2|BIT1) == 0x00 )
                        fnx = i31inc();
                    ff = i31inc();
                    i = 0;
                    fp[i] = i31inc();
                    do {
                        fp[++i] = i31inc();
                    } while ( fp[i-1] != ESC && fp[i] != '=' && i<65 );
                    fp[i-1] = NUL;
                    if ( debses )
                        break;
                    /* The function key to be assigned is equal to      */
                    /* (fn & (BIT5|BIT4|BIT3|BIT2|BIT1)) ||             */
                    /* (32 + (fnx & (BIT5|BIT4|BIT3|BIT2|BIT1)))        */
                    /* (ff & BIT1) specifies whether the string is      */
                    /* for local display (1) or remote transmission (0) */
                    break;
                }
                default:
                    escstate = ES_NORMAL ;
                    i31ascii(ch);
                }
                break;
            }
            case SP: {
                ch2 = i31inc() ;
                switch ( ch2 ) {
                case ':':
                    /* Begin Outbound Trace mode */
                    if ( debses )
                        break;
                    cprint = TRUE;
                    if ( !printon )
                        printeron();
                    break;
                case ';':
                    /* End Outbound Trace mode */
                    if ( debses || !cprint )
                        break;
                    cprint = FALSE;
                    if ( !uprint && !xprint && !cprint && !aprint && printon )
                        printeroff();
                    break;
                case '1':
                    /* 3161 - Clear all tab stops */
                    if ( debses )
                        break;
                    for (j = 1; j <=MAXTERMCOL; ++j)
                        htab[j] = '0';
                    break;
                case '6':
                    /* 3161 - Send model */
                    if ( debses )
                        break;
                    /* send ESC SP 6 pa LTA */
                    break;
                case '7':
                    /* 3161 - Read Control 1 (6-15) */
                    /* Machine and Operating Modes */
                    if ( debses )
                        break;
                    pa1 = 0x81; /* 3151 and Character */
                    sprintf(response,"%c 7%c%c",
                             ESC,pa1,i31_lta);
                    ttol(response,strlen(response));
                    break;
                case '8':
                    /* 3161 - Send message */
                    if ( debses )
                        break;
                    break;
                case '9':
                    /* 3161 - Set Control 1 (6-15) */
                    /* Machine and Operating Modes */
                    n = i31rdctrl( 2, &pa1, &pa2, &pa3, &pa4, &op );
                    if ( debses || n == 0 )
                        break;

                    if ( GOT_OP(n) ) {
                        if ( (op & BIT7|BIT6) == 0x60 )
                        switch ( (op & 0x1F) ) {
                        case 0x00:      /* Replacement (default) */
                            break;
                        case 0x01:      /* Logical OR */
                            break;
                        case 0x02:      /* Logical AND */
                            break;
                        case 0x1F:      /* Interpreted as the DEL character */
                            break;
                        }
                    }
                    /* The following lines must be merged in to the above */
                    /* switch to enable the replace|and|or functionality  */
                    if ( GOT_PA1(n) ) {
                        /* Machine Mode */
                        switch ( (pa1 & 0x1C) ) {
                        case 0x00:  /* IBM 3151 (default) */
                            break;
                        case 0x04:  /* IBM 3101 */
                            break;
                        case 0x08:  /* ADM-3A */
                            break;
                        case 0x0C:  /* ADM-5 */
                            break;
                        case 0x10:  /* ADDS VP A2 */
                            break;
                        case 0x14:  /* HZ-1500 */
                            break;
                        case 0x18:  /* TVI-910+ */
                            break;
                        case 0x1C:  /* use pa2 */
                            if ( GOT_PA2(n) ) {
                                switch ( (pa2 & 0x0F) ) {
                                case 0x00:      /* TVI-925E */
                                    break;
                                case 0x01:      /* TVI-920/912 */
                                    break;
                                }
                            }
                            break;
                        }

                        /* Operating Mode */
                        switch ( (pa1 & 0x03) ) {
                        case 0x00:  /* Echo */
                            break;
                        case 0x01:  /* Character */
                            break;
                        case 0x02:  /* Block (default) */
                            break;
                        case 0x03:  /* Reserved */
                            break;
                        }
                    }
                    break;
                case 'M':
                    /* Index command */
                    if ( debses )
                        break;
                    break;
                case 'q':
                    /* 3161 - Select host partition */
                    /* Specifies partition that host data is written to */
                    pa = i31inc();
                    if ( debses )
                        break;
                    switch ( pa ) {
                    case 'A':   /* viewport 1 */
                        break;
                    case 'B':   /* viewport 2 */
                        break;
                    case 'C':   /* viewport 3 */
                        break;
                    }
                    break;
                case 'r': {
                    /* 3161 - Create Viewport */
                    char cmd[16]="";
                    ch3 = i31inc();
                    switch ( ch3 ) {
                    case '!':   /* one viewport */
                        for ( i=0;i<5;i++ )
                            cmd[i] = i31inc();
                        cmd[i] = NUL;
                        break;
                    case '"':   /* two viewports */
                        for ( i=0;i<10;i++ )
                            cmd[i] = i31inc();
                        cmd[i] = NUL;
                        break;
                    case '#':   /* three viewports */
                        for ( i=0;i<15;i++ )
                            cmd[i] = i31inc();
                        cmd[i] = NUL;
                        break;
                    }
                    if ( debses )
                        break;
                    switch ( ch3 ) {
                    case '!':   /* one viewport */
                        if ( !strcmp(cmd,"! 8\"P") ) {
                            ;   /* 24 lines, 80 cols */
                        }
                        else if ( !strcmp(cmd,"! 9\"P" ) ) {
                            ;   /* 25 lines, 80 cols */
                        }
                        else if ( !strcmp(cmd,"! 8$D" ) ) {
                            ;   /* 24 lines, 132 cols */
                        }
                        else if ( !strcmp(cmd,"! 9$D" ) ) {
                            ;   /* 25 lines, 132 cols */
                        }
                        break;
                    case '"':   /* two viewports */
                        if ( cmd[3] == '"' && cmd[4] == '0' &&
                             cmd[8] == '"' && cmd[9] == 'P') {
                            /* 80 columns */
                            /* height of viewport 1 is (cmd[2]-'!'+1) */
                            /* height of viewport 2 is (cmd[7]-'!'+1) */
                            /* sum of heights must equal 24 or 25 */
                        }
                        else if ( cmd[3] == '$' && cmd[4] == '$' &&
                                  cmd[8] == '$' && cmd[9] == 'D') {
                            /* 132 columns */
                            /* height of viewport 1 is (cmd[2]-'!'+1) */
                            /* height of viewport 2 is (cmd[7]-'!'+1) */
                            /* sum of heights must equal 24 or 25 */
                        }
                        break;
                    case '#':   /* three viewports */
                        if ( cmd[3] == '"' && cmd[4] == '0' &&
                             cmd[8] == '"' && cmd[9] == '0' &&
                             cmd[13] == '"' && cmd[14] == 'P') {
                            /* 80 columns */
                            /* height of viewport 1 is (cmd[2]-'!'+1) */
                            /* height of viewport 2 is (cmd[7]-'!'+1) */
                            /* height of viewport 3 is (cmd[12]-'!'+1) */
                            /* sum of heights must equal 24 or 25 */
                        }
                        else if ( cmd[3] == '$' && cmd[4] == '$' &&
                                  cmd[8] == '$' && cmd[9] == '$' &&
                                  cmd[13] == '$' && cmd[14] == 'D') {
                            /* 132 columns */
                            /* height of viewport 1 is (cmd[2]-'!'+1) */
                            /* height of viewport 2 is (cmd[7]-'!'+1) */
                            /* height of viewport 3 is (cmd[12]-'!'+1) */
                            /* sum of heights must equal 24 or 25 */
                        }
                        break;
                    }
                    break;
                }
                case 'S':
                    /* 3161 - Reset to initial state */
                    if ( debses )
                        break;
                    doreset(1) ;
                    break;
                case 't':
                    /* 3161 - Default all function keys */
                    if ( debses )
                        break;
                    udkreset() ;
                    break;
                case 'W':
                    /* 3161 - Print screen */
                    if ( debses )
                        break;
                    break;
                case 'Z':
                    /* 3161 - Reset buffer address mode to cursor addr mode */
                    if ( debses )
                        break;
                    break;
                default:
                    escstate = ES_NORMAL ;
                    i31ascii(ch);
                }
                break;
            }
            case '"': {
                ch2 = i31inc() ;
                switch ( ch2 ) {
                case ':':
                    /* Enable Write Null */
                    if ( debses )
                        break;
                    break;
                case ';':
                    /* Disable Write Null */
                    if ( debses )
                        break;
                    break;
                case '6':
                    /* Extended Read Model Command */
                    if ( debses )
                        break;
                    /* send ESC " 6 pa1 pa2 pa3 pa4 pa5 LTA */
                    break;
                case '7':
                    /* 3161 - Read Control 3 */
                    /* Display functions of the 3151 */
                    if ( debses )
                        break;
                    break;
                case '9':
                    /* 3161 - Set Control 3 */
                    /* Display functions of the 3151 */
                    n = i31rdctrl( 2, &pa1, &pa2, &pa3, &pa4, &op );
                    if ( debses || n == 0 )
                        break;

                    if ( GOT_OP(n) ) {
                        if ( (op & BIT7|BIT6) == 0x60 )
                        switch ( (op & 0x1F) ) {
                        case 0x00:      /* Replacement (default) */
                            break;
                        case 0x01:      /* Logical OR */
                            break;
                        case 0x02:      /* Logical AND */
                            break;
                        case 0x1F:      /* Interpreted as the DEL character */
                            break;
                        }
                    }

                    /* The following lines must be merged in to the above */
                    /* switch to enable the replace|and|or functionality  */
                    if ( GOT_PA1(n) ) {
                        /* Scroll (temporary) */
                        switch ( pa1 & 0x18 ) {
                        case 0x00:      /* No scroll */
                            break;
                        case 0x08:      /* Jump scroll (default) */
                            break;
                        case 0x10:      /* Smooth scroll */
                            break;
                        case 0x11:      /* Reserved */
                            break;
                        }

                        /* Line Wrap */
                        if ( pa1 & 0x04 )
                            ;   /* On (default) */
                        else
                            ;   /* Off */

                        /* Auto LF */
                        if ( pa1 & 0x02 )
                            ;   /* On (default) */
                        else
                            ;   /* Off */

                        /* ASCII LF Character */
                        if ( pa1 & 0x01 )
                            ;   /* New line */
                        else
                            ;   /* Line feed (default) */
                    }

                    if ( GOT_PA2(n) ) {
                        /* Scroll (permanently) */
                        switch ( pa2 & 0x18 ) {
                        case 0x00:      /* No scroll */
                            break;
                        case 0x08:      /* Jump scroll (default) */
                            break;
                        case 0x10:      /* Smooth scroll */
                            break;
                        case 0x11:      /* Reserved */
                            break;
                        }

                        /* Insert Character */
                        if ( pa2 & 0x04 )
                            ;   /* Inserts a space character */
                        else
                            ;   /* Sets insert mode (default) */

                        /* Tab */
                        if ( pa2 & 0x02 )
                            ;   /* Column Tab */
                        else
                            ;   /* Field Tab (default) */

                        /* Return Key */
                        if ( pa1 & 0x01 )
                            ;   /* New Line */
                        else
                            ;   /* Field (default) */

                    }
                    break;
                case 'A':
                    /* 3161 - Jump partition */
                    /* Selects next viewport as active partition */
                    if ( debses )
                        break;
                    break;
                default:
                    escstate = ES_NORMAL ;
                    i31ascii(ch);
                }
                break;
            }
            case '#': {
                ch2 = i31inc() ;
                switch ( ch2 ) {
                case ';':
                    /* Display Host Message stored in OIA buffer */
                    if ( debses )
                        break;
                    break;
                case ':':
                    /* Display Machine Status */
                    if ( debses )
                        break;
                    break;
                case '7':
                    /* 3161 - Read Control 4 */
                    /* Send parameters of the 3151 */
                    if ( debses )
                        break;
                    break;
                case '8':
                    /* 3161 - Send all */
                    if ( debses )
                        break;
                    break;
                case '9':
                    /* 3161 - Set Control 4 */
                    /* Send parameters of the 3151 */
                    n = i31rdctrl( 1, &pa1, &pa2, &pa3, &pa4, &op );

                    if ( debses || n == 0 )
                        break;

                    if ( GOT_OP(n) ) {
                        if ( (op & BIT7|BIT6) == 0x60 )
                        switch ( (op & 0x1F) ) {
                        case 0x00:      /* Replacement (default) */
                            break;
                        case 0x01:      /* Logical OR */
                            break;
                        case 0x02:      /* Logical AND */
                            break;
                        case 0x1F:      /* Interpreted as the DEL character */
                            break;
                        }
                    }

                    /* The following lines must be merged in to the above */
                    /* switch to enable the replace|and|or functionality  */
                    if ( GOT_PA1(n) ) {
                        /* Send Line */
                        if ( pa1 & 0x08 )
                            ;   /* Send key works as Send Line key */
                                /* and Send Line key works as Send key */
                        else
                            ;   /* Send key works as Send key and */
                                /* Send Line key works as Send Line key */
                                /* (default) */

                        /* Send Null Suppress */
                        if ( pa1 & 0x04 )
                            ;   /* On (default) */
                        else
                            ;   /* Off */

                        /* Lock Keyboard and Keep MDT Bit */
                        if ( pa1 & 0x02 )
                            ;   /* On */
                        else
                            ;   /* Off (default) */

                        /* Send Data Format */
                        if ( pa1 & 0x01 )
                            ;   /* AID LTA */
                        else
                            ;   /* Text LTA (default) */
                    }
                    break;
                default:
                    escstate = ES_NORMAL ;
                    i31ascii(ch);
                }
                break;
            }
            case '$': {
                ch2 = i31inc() ;
                switch ( ch2 ) {
                case '7':
                    /* 3161 - Read Control 5 */
                    /* Communication values on the main port */
                    if ( debses )
                        break;
                    break;
                case '9':
                    /* 3161 - Set Control 5 */
                    /* Communication values on the main port */
                    n = i31rdctrl( 4, &pa1, &pa2, &pa3, &pa4, &op );

                    if ( debses || n == 0 )
                        break;

                    if ( GOT_OP(n) ) {
                        if ( (op & BIT7|BIT6) == 0x60 )
                        switch ( (op & 0x1F) ) {
                        case 0x00:      /* Replacement (default) */
                            break;
                        case 0x01:      /* Logical OR */
                            break;
                        case 0x02:      /* Logical AND */
                            break;
                        case 0x1F:      /* Interpreted as the DEL character */
                            break;
                        }
                    }

                    /* The following lines must be merged in to the above */
                    /* switch to enable the replace|and|or functionality  */
                    if ( GOT_PA1(n) ) {
                        /* Stop Bits */
                        if ( pa1 & 0x10 )
                            ;   /* 2 */
                        else
                            ;   /* 1 (default) */

                        /* Line Speed */
                        switch ( pa1 & 0x0F ) {
                        case 0x00:      /* 50 or 38400 */
                            if ( pa3 & 0x04 )
                                ;       /* 38400 */
                            else
                                ;       /* 50 */
                            break;
                        case 0x01:      /* 75 bps */
                            break;
                        case 0x02:      /* 110 bps */
                            break;
                        case 0x03:      /* 134.5 bps */
                            break;
                        case 0x04:      /* 150 bps */
                            break;
                        case 0x05:      /* 200 bps */
                            break;
                        case 0x06:      /* 300 bps */
                            break;
                        case 0x07:      /* 600 bps */
                            break;
                        case 0x08:      /* 1200 bps */
                            break;
                        case 0x09:      /* 1800 bps */
                            break;
                        case 0x0A:      /* 2400 bps */
                            break;
                        case 0x0B:      /* 3600 bps */
                            break;
                        case 0x0C:      /* 4800 bps */
                            break;
                        case 0x0D:      /* Reserved */
                            break;
                        case 0x0E:      /* 9600 bps */
                            break;
                        case 0x0F:      /* 19200 bps */
                            break;
                        }
                    }
                    if ( GOT_PA2(n) ) {
                        /* Interface */
                        if ( pa2 & 0x10 )
                            ;   /* EIA RS-232C (always) */
                        else
                            ;   /* illegal value */

                        /* Word Length */
                        if ( pa2 & 0x08 )
                            ;   /* 8-bit */
                        else
                            ;   /* 7-bit (default) */

                        /* Parity */
                        switch ( pa2 & 0x07 ) {
                        case 0x00:      /* None */
                            break;
                        case 0x01:      /* Space */
                            break;
                        case 0x02:      /* Mark */
                            break;
                        case 0x03:      /* Odd (default) */
                            break;
                        case 0x04:      /* Even */
                            break;
                        }
                    }

                    if ( GOT_PA3(n) ) {
                        /* no name */
                        if ( pa3 & 0x10 )
                            ;   /* 100 ms response delay is returned in */
                                /* response to the Read Control 5 command */
                        else
                            ;   /* Reserved */

                        /* Reserved */
                        if ( pa3 & 0x08 )
                            ;   /* illegal value */
                        else
                            ;   /* Reserved (always) */

                        /* Line Speed Extension (used in pa1 above) */
                        if ( pa3 & 0x04 )
                            ;   /* Extended */
                        else
                            ;   /* Not extended (default) */

                        /* Line Control */
                        switch ( pa3 & 0x03 ) {
                        case 0x00:      /* CRTS */
                            break;
                        case 0x01:      /* PRTS (default) */
                            break;
                        case 0x02:      /* IPRTS */
                            break;
                        case 0x03:      /* Reserved */
                            break;
                        }
                    }

                    if ( GOT_PA4(n) ) {
                        /* Reserved */
                        if ( pa4 & 0x10 )
                            ;   /* Enable outbound pacing is returned in */
                                /* response to the Read Control 5 command */
                        else
                            ;   /* Reserved */

                        /* Reserved */
                        if ( pa4 & 0x08 )
                            ;   /* Enable inbound pacing is returned in */
                                /* response to the Read Control 5 command */
                        else
                            ;   /* Reserved */

                        /* Break Signal */
                        if ( pa4 & 0x04 )
                            ;   /* 500 ms (default) */
                        else
                            ;   /* 170 ms */

                        /* Turnaround Character */
                        switch ( pa4 & 0x03 ) {
                        case 0x00:      /* ETX (default) */
                            break;
                        case 0x01:      /* CR */
                            break;
                        case 0x02:      /* EOT */
                            break;
                        case 0x03:      /* DC3 (also disable inbound/outbound */
                                        /* pacing) */
                            break;
                        }
                    }
                    break;
                default:
                    escstate = ES_NORMAL ;
                    i31ascii(ch);
                }
                break;
            }
            case '%': {
                ch2 = i31inc() ;
                switch ( ch2 ) {
                case ':':
                    /* Enable Read Unprotected Field */
                    if ( debses )
                        break;
                    break;
                case ';':
                    /* Disable Read Unprotected Field */
                    if ( debses )
                        break;
                    break;
                case '7':
                    /* 3161 - Read Control 6 */
                    /* Communication values on the auxiliary port */
                    if ( debses )
                        break;
                    break;
                case '9':
                    /* 3161 - Set Control 6 */
                    /* Communication values on the auxiliary port */
                    n = i31rdctrl( 4, &pa1, &pa2, &pa3, &pa4, &op );

                    if ( debses || n == 0 )
                        break;

                    if ( GOT_OP(n) ) {
                        if ( (op & BIT7|BIT6) == 0x60 )
                        switch ( (op & 0x1F) ) {
                        case 0x00:      /* Replacement (default) */
                            break;
                        case 0x01:      /* Logical OR */
                            break;
                        case 0x02:      /* Logical AND */
                            break;
                        case 0x1F:      /* Interpreted as the DEL character */
                            break;
                        }
                    }

                    /* The following lines must be merged in to the above */
                    /* switch to enable the replace|and|or functionality  */
                    if ( GOT_PA1(n) ) {
                        /* Stop Bits */
                        if ( pa1 & 0x10 )
                            ;   /* 2 */
                        else
                            ;   /* 1 (default) */

                        /* Line Speed */
                        switch ( pa1 & 0x0F ) {
                        case 0x00:      /* 50 bps */
                            break;
                        case 0x01:      /* 75 bps */
                            break;
                        case 0x02:      /* 110 bps */
                            break;
                        case 0x03:      /* 134.5 bps */
                            break;
                        case 0x04:      /* 150 bps */
                            break;
                        case 0x05:      /* 200 bps */
                            break;
                        case 0x06:      /* 300 bps */
                            break;
                        case 0x07:      /* 600 bps */
                            break;
                        case 0x08:      /* 1200 bps */
                            break;
                        case 0x09:      /* 1800 bps */
                            break;
                        case 0x0A:      /* 2400 bps */
                            break;
                        case 0x0B:      /* 3600 bps */
                            break;
                        case 0x0C:      /* 4800 bps */
                            break;
                        case 0x0D:      /* Reserved */
                            break;
                        case 0x0E:      /* 9600 bps */
                            break;
                        case 0x0F:      /* 19200 bps */
                            break;
                        }
                    }
                    if ( GOT_PA2(n) ) {
                        /* Interface */
                        if ( pa2 & 0x10 )
                            ;   /* EIA RS-232C (always) */
                        else
                            ;   /* illegal value */

                        /* Word Length */
                        if ( pa2 & 0x08 )
                            ;   /* 8-bit */
                        else
                            ;   /* 7-bit (default) */

                        /* Parity */
                        switch ( pa2 & 0x07 ) {
                        case 0x00:      /* None */
                            break;
                        case 0x01:      /* Space */
                            break;
                        case 0x02:      /* Mark */
                            break;
                        case 0x03:      /* Odd (default) */
                            break;
                        case 0x04:      /* Even */
                            break;
                        }
                    }

                    if ( GOT_PA3(n) ) {
                        /* Reserved */
                        if ( pa3 & 0x10 )
                            ;   /* Reserved */
                        else
                            ;   /* Reserved (always) */

                        /* Reserved */
                        if ( pa3 & 0x08 )
                            ;   /* illegal value */
                        else
                            ;   /* Reserved (always) */

                        /* Reserved */
                        if ( pa3 & 0x04 )
                            ;   /* Reserved */
                        else
                            ;   /* Reserved (always) */

                        if ( pa3 & 0x02 )
                            ;   /* Enable outbound pacing is returned in */
                                /* response to the Read Control 6 command */
                        else
                            ;   /* Reserved */

                        /* Reserved */
                        if ( pa3 & 0x01 )
                            ;   /* Enable inbound pacing is returned in */
                                /* response to the Read Control 6 command */
                        else
                            ;   /* Reserved */

                    }

                    if ( GOT_PA4(n) ) {
                        /* Reserved */
                        /* Bits 5 and 4 are always 0 */

                        /* Inbound Pass-through */
                        if ( pa4 & 0x04 )
                            ;   /* Enabled */
                        else
                            ;   /* Disabled (default) */

                        /* ... */
                        switch ( pa4 & 0x03 ) {
                        case 0x00:      /* Reserved */
                            break;
                        case 0x01:      /* Enable outbound trace is returned */
                                        /* in response to the Read Control 6 */
                                        /* command.                          */
                            break;
                        }
                    }
                    break;

                default:
                    escstate = ES_NORMAL ;
                    i31ascii(ch);
                }
                break;
            }
            case '&': {
                ch2 = i31inc() ;
                switch ( ch2 ) {
                case ':':
                    /* Enable Host Protect */
                    if ( debses )
                        break;
                    break;
                case ';':
                    /* Disable Host Protect */
                    if ( debses )
                        break;
                    break;
                case '7':
                    /* 3161 - Read Control 7 */
                    /* Information used for print operations */
                    if ( debses )
                        break;
                    break;
                case '9':
                    /* 3161 - Set Control 7 */
                    /* Information used for print operations */
                    n = i31rdctrl( 3, &pa1, &pa2, &pa3, &pa4, &op );

                    if ( debses || n == 0 )
                        break;

                    if ( GOT_OP(n) ) {
                        if ( (op & BIT7|BIT6) == 0x60 )
                        switch ( (op & 0x1F) ) {
                        case 0x00:      /* Replacement (default) */
                            break;
                        case 0x01:      /* Logical OR */
                            break;
                        case 0x02:      /* Logical AND */
                            break;
                        case 0x1F:      /* Interpreted as the DEL character */
                            break;
                        }
                    }

                    /* The following lines must be merged in to the above */
                    /* switch to enable the replace|and|or functionality  */
                    if ( GOT_PA1(n) ) {
                        /* Bit 5 Reserved */

                        switch ( pa1 & 0x0C ) {
                        case 0x00:      /* Print screen is returned in */
                                        /* response to the Read Control 7 */
                                        /* command. */
                            break;
                        }

                        switch ( pa1 & 0x03 ) {
                        case 0x00:      /* No time fill character is returned */
                                        /* in response to the Read Control 7 */
                                        /* command. */
                            break;
                        }
                    }

                    if ( GOT_PA2(n) ) {
                        if ( pa2 & 0x10 )
                            ;   /* undefined */
                        else
                            ;   /* CR/LF for the line end character is returned */
                                /* in response to the Read Control 7 command */

                        if ( pa2 & 0x08 )
                            ;   /* Print data null suppression is returned in */
                                /* response to the Read Control 7 command */
                        else
                            ;   /* Reserved */

                        if ( pa2 & 0x04 )
                            ;   /* Print EOL on is returned in response to the */
                                /* Read Control 7 command */
                        else
                            ;   /* Reserved */

                        /* Characters */
                        switch ( pa2 & 0x03 ) {
                        case 0x00:      /* National (default) */
                            break;
                        case 0x01:      /* Reserved */
                        case 0x02:      /* Reserved */
                            break;
                        case 0x03:      /* All */
                            break;
                        }
                    }

                    if ( GOT_PA3(n) ) {
                        /* Bit 5 Reserved - always 0 */

                        switch ( pa3 & 0x0F ) {
                        case 0x00:      /* Reserved */
                            break;
                        case 0x05:      /* Is the response to the Read */
                                        /* Control 7 command that the DTR */
                                        /* signal (+) is being monitored. */
                            break;
                        }
                    }
                    break;

                default:
                    escstate = ES_NORMAL ;
                    i31ascii(ch);
                }
                break;
            }
            case '+': {
                ch2 = i31inc() ;
                switch ( ch2 ) {
                case ':':
                    /* Enable Partition Line Separator */
                    if ( debses )
                        break;
                    break;
                case ';':
                    /* Disable Partition Line Separator */
                    if ( debses )
                        break;
                    break;
                default:
                    escstate = ES_NORMAL ;
                    i31ascii(ch);
                }
                break;
            }
            case '*': {
                ch2 = i31inc() ;
                switch ( ch2 ) {
                case ':':
                    /* Enable OIA Divider Line */
                    if ( debses )
                        break;
                    break;
                case ';':
                    /* Disable OIA Divider Line */
                    if ( debses )
                        break;
                    break;
                default:
                    escstate = ES_NORMAL ;
                    i31ascii(ch);
                }
                break;
            }
            case '\'': {
                ch2 = i31inc() ;
                switch ( ch2 ) {
                case ':':
                    /* Enable Default Field Attribute */
                    if ( debses )
                        break;
                    break;
                case ';':
                    /* Disable Default Field Attribute */
                    if ( debses )
                        break;
                    break;
                default:
                    escstate = ES_NORMAL ;
                    i31ascii(ch);
                }
                break;
            }
            case ',': {
                ch2 = i31inc() ;
                switch ( ch2 ) {
                case ':':
                    /* Enable Field Attribute Visible Renditions */
                    if ( debses )
                        break;
                    break;
                case ';':
                    /* Disable Field Attribute Visible Renditions */
                    if ( debses )
                        break;
                    break;
                default:
                    escstate = ES_NORMAL ;
                    i31ascii(ch);
                }
                break;
            }
            case '=': {
                /* Write Host Message to the OIA buffer */
                if ( debses )
                    break;
                /* read message until another <ESC> = is received */
                /* message may be Terminal Width in length.       */
                break;
            }
            case ';':   /* Unlock keyboard */
                if ( debses )
                    break;
                keylock = FALSE ;
                break;
            case ':':   /* Lock keyboard */
                if ( debses )
                    break;
                keylock = TRUE ;
                break;
            case '<': {
                /* 3161 - Select Character Set G0 */
                struct _vtG * pG;
                int cs = -1;

                pa = i31inc();
                if ( debses )
                    break;

                pG = &G[0];

                switch ( pa ) {
                case '@':       /* US ASCII */
                    cs = TX_ASCII;
                    break;
                case 'A':       /* Special Graphics */
                    cs = TX_WYSE60G_1;
                    break;
                case 'B':
                    break;
                }

                if ( cs > -1 ) {
                    pG->designation = cs ;
                    pG->size = cs96;
                    pG->c1 = cs_is_std(cs) ;
                    pG->national = CSisNRC( cs ) ;
                    pG->rtoi = xl_u[cs] ;
                    pG->itol = xl_tx[tcsl] ;
                    pG->ltoi = xl_u[tcsl] ;
                    pG->itor = xl_tx[cs] ;
                }
                break;
            }
            case '>': {
                /* 3161 - Select Character Set G1 */
                struct _vtG * pG;
                int cs = -1;

                pa = i31inc();
                if ( debses )
                    break;

                pG = &G[1];

                switch ( pa ) {
                case '@':       /* US ASCII */
                    cs = TX_ASCII;
                    break;
                case 'A':       /* Special Graphics */
                    cs = TX_WYSE60G_1;
                    break;
                case 'B':       /* unknown - used after Special Graphics */
                    break;
                }

                if ( cs > -1 ) {
                    pG->designation = cs ;
                    pG->size = cs96;
                    pG->c1 = cs_is_std(cs) ;
                    pG->national = CSisNRC( cs ) ;
                    pG->rtoi = xl_u[cs] ;
                    pG->itol = xl_tx[tcsl] ;
                    pG->ltoi = xl_u[tcsl] ;
                    pG->itor = xl_tx[cs] ;
                }
                break;
            }
            case '0':
                /* Set tab stop */
                if ( debses )
                    break;
                htab[wherex[VTERM]] = 'T';
                break;
            case '1':
                /* Clear tab stop */
                if ( debses )
                    break;
                htab[wherex[VTERM]] = '0';
                break;
            case '2':
                /* 3101-2x and 3161 - Backtab */
                if ( debses )
                    break;
                i = wherex[VTERM];
                if (i > 1) {
                    do {
                        i--;
                        cursorleft(0);
                    } while ((htab[i] != 'T') &&
                              (i >= 2));
                    VscrnIsDirty(VTERM);
                }
                break;
            case '3':
                /* 3101-2x and 3161 - Set field attribute */
                n = i31rdctrl( 2, &pa1, &pa2, &pa3, &pa4, &op );
                if ( debses || n == 0)
                    break;
                /* field attributes take up a character position */
                if ( !GOT_OP(n) )
                    op = BIT7|BIT6;     /* Replacement */

                if ( (op & (BIT7|BIT6)) == 0x60 ) {
                    switch ( (op & (BIT5|BIT4|BIT3|BIT2|BIT1)) ) {
                    case 0x00:      /* Replacement (default) */
                        if ( GOT_PA1(n) ) {
                            /* Normal or Non-display */
                            if ( pa1 & BIT5 )
                                attrib.invisible=TRUE;   /* Non-display */
                            else
                                attrib.invisible=FALSE;   /* Normal (default) */

                            /* Intensity */
                            if ( pa1 & BIT4 )
                                attrib.bold=TRUE;    /* High Intensity */
                            else
                                attrib.bold=FALSE;   /* Normal (default) */

                            /* Blink */
                            if ( pa1 & BIT3 )
                                attrib.blinking=TRUE;   /* Blink On */
                            else
                                attrib.blinking=FALSE;  /* Blink off (default) */

                            /* Underline */
                            if ( pa1 & BIT2 )
                                attrib.underlined=TRUE;  /* Underline On */
                            else
                                attrib.underlined=FALSE;  /* Underline Off (default) */

                            /* Reverse */
                            if ( pa1 & BIT1 )
                                attrib.reversed=TRUE;   /* Reverse */
                            else
                                attrib.reversed=FALSE;  /* Normal (default) */
                        }

                        if ( GOT_PA2(n) ) {
                            /* Numeric */
                            if ( pa2 & BIT4 )
                                ;   /* Yes */
                            else
                                ;   /* No (default) */

                            /* Protected */
                            if ( pa2 & BIT2 )
                                attrib.unerasable=TRUE;   /* Protected */
                            else
                                attrib.unerasable=FALSE;  /* Unprotected (default) */

                            /* Modified Data Tag */
                            if ( pa2 & BIT1 )
                                ;   /* Modified */
                            else
                                ;   /* Not modified (default) */
                            }

                        break;
                    case 0x01:      /* Logical OR */
                        if ( GOT_PA1(n) ) {
                            /* Normal or Non-display */
                            if ( pa1 & BIT5 )
                                attrib.invisible=TRUE;   /* Non-display */

                            /* Intensity */
                            if ( pa1 & BIT4 )
                                attrib.bold=TRUE;    /* High Intensity */

                            /* Blink */
                                if ( pa1 & BIT3 )
                                    attrib.blinking=TRUE;   /* Blink On */

                            /* Underline */
                            if ( pa1 & BIT2 )
                                attrib.underlined=TRUE;  /* Underline On */

                            /* Reverse */
                            if ( pa1 & BIT1 )
                                attrib.reversed=TRUE;   /* Reverse */
                        }

                        if ( GOT_PA2(n) ) {
                            /* Numeric */
                            if ( pa2 & BIT4 )
                                ;   /* Yes */

                            /* Protected */
                            if ( pa2 & BIT2 )
                                attrib.unerasable=TRUE;   /* Protected */

                            /* Modified Data Tag */
                            if ( pa2 & BIT1 )
                                ;   /* Modified */
                        }

                        break;
                    case 0x02:      /* Logical AND */
                        if ( GOT_PA1(n) ) {
                            /* Normal or Non-display */
                            if ( !(pa1 & BIT5) )
                                attrib.invisible=FALSE;   /* Normal (default) */

                            /* Intensity */
                            if ( !(pa1 & BIT4) )
                                attrib.bold=FALSE;   /* Normal (default) */

                            /* Blink */
                            if ( !(pa1 & BIT3) )
                                attrib.blinking=FALSE;  /* Blink off (default) */

                            /* Underline */
                            if ( !(pa1 & BIT2) )
                                attrib.underlined=FALSE;  /* Underline Off (default) */

                            /* Reverse */
                            if ( !(pa1 & BIT1) )
                                attrib.reversed=FALSE;  /* Normal (default) */
                        }

                        if ( GOT_PA2(n) ) {
                            /* Numeric */
                            if ( !(pa2 & BIT4) )
                                ;   /* No (default) */

                            /* Protected */
                            if ( !(pa2 & BIT2) )
                                attrib.unerasable=FALSE;  /* Unprotected (default) */

                            /* Modified Data Tag */
                            if ( !(pa2 & BIT1) )
                                ;   /* Not modified (default) */
                        }
                        break;
                    case 0x1F:      /* Interpreted as the DEL character */
                        wrtch(DEL);
                        break;
                    }
                }
                break;
            case '4':
                /* 3161 - Set character attribute */
                n = i31rdctrl( 1, &pa1, &pa2, &pa3, &pa4, &op );
                if ( debses || n == 0 )
                    break;

                /* Character attributes do not take up a position */
                if ( !GOT_OP(n) )
                    op = BIT7|BIT6;

                if ( (op & (BIT7|BIT6)) == 0x60 ) {
                    switch ( (op & (BIT5|BIT4|BIT3|BIT2|BIT1)) ) {
                    case 0x00:      /* Replacement (default) */
                        if ( GOT_PA1(n) ) {
                            /* Normal or Non-display */
                            if ( pa1 & BIT5 )
                                attrib.invisible=TRUE;   /* Non-display */
                            else
                                attrib.invisible=FALSE;   /* Normal (default) */

                            /* Intensity */
                            if ( pa1 & BIT4 )
                                attrib.bold=TRUE;    /* High Intensity */
                            else
                                attrib.bold=FALSE;   /* Normal (default) */

                            /* Blink */
                            if ( pa1 & BIT3 )
                                attrib.blinking=TRUE;   /* Blink On */
                            else
                                attrib.blinking=FALSE;  /* Blink off (default) */

                            /* Underline */
                            if ( pa1 & BIT2 )
                                attrib.underlined=TRUE;  /* Underline On */
                            else
                                attrib.underlined=FALSE;  /* Underline Off (default) */

                            /* Reverse */
                            if ( pa1 & BIT1 )
                                attrib.reversed=TRUE;   /* Reverse */
                            else
                                attrib.reversed=FALSE;  /* Normal (default) */
                        }

                        if ( GOT_PA2(n) ) {
                            /* Numeric */
                            if ( pa2 & BIT4 )
                                ;   /* Yes */
                            else
                                ;   /* No (default) */

                            /* Protected */
                            if ( pa2 & BIT2 )
                                attrib.unerasable=TRUE;   /* Protected */
                            else
                                attrib.unerasable=FALSE;  /* Unprotected (default) */

                            /* Modified Data Tag */
                            if ( pa2 & BIT1 )
                                ;   /* Modified */
                            else
                                ;   /* Not modified (default) */
                        }

                        break;
                    case 0x01:      /* Logical OR */
                        if ( GOT_PA1(n) ) {
                            /* Normal or Non-display */
                            if ( pa1 & BIT5 )
                                attrib.invisible=TRUE;   /* Non-display */

                            /* Intensity */
                            if ( pa1 & BIT4 )
                                attrib.bold=TRUE;    /* High Intensity */

                            /* Blink */
                            if ( pa1 & BIT3 )
                                attrib.blinking=TRUE;   /* Blink On */

                            /* Underline */
                            if ( pa1 & BIT2 )
                                attrib.underlined=TRUE;  /* Underline On */

                            /* Reverse */
                            if ( pa1 & BIT1 )
                                attrib.reversed=TRUE;   /* Reverse */
                        }

                        if ( GOT_PA2(n) ) {
                            /* Numeric */
                            if ( pa2 & BIT4 )
                                ;   /* Yes */

                            /* Protected */
                            if ( pa2 & BIT2 )
                                attrib.unerasable=TRUE;   /* Protected */

                            /* Modified Data Tag */
                            if ( pa2 & BIT1 )
                                ;   /* Modified */
                        }

                        break;
                    case 0x02:      /* Logical AND */
                        if ( GOT_PA1(n) ) {
                            /* Normal or Non-display */
                            if ( !(pa1 & BIT5) )
                                attrib.invisible=FALSE;   /* Normal (default) */

                            /* Intensity */
                            if ( !(pa1 & BIT4) )
                                attrib.bold=FALSE;   /* Normal (default) */

                            /* Blink */
                            if ( !(pa1 & BIT3) )
                                attrib.blinking=FALSE;  /* Blink off (default) */

                            /* Underline */
                            if ( !(pa1 & BIT2) )
                                attrib.underlined=FALSE;  /* Underline Off (default) */

                            /* Reverse */
                            if ( !(pa1 & BIT1) )
                                attrib.reversed=FALSE;  /* Normal (default) */
                        }

                        if ( GOT_PA2(n) ) {
                            /* Numeric */
                            if ( !(pa2 & BIT4) )
                                ;   /* No (default) */

                            /* Protected */
                            if ( !(pa2 & BIT2) )
                                attrib.unerasable=FALSE;  /* Unprotected (default) */

                            /* Modified Data Tag */
                            if ( !(pa2 & BIT1) )
                                ;   /* Not modified (default) */
                        }
                        break;
                    case 0x1F:      /* Interpreted as the DEL character */
                        wrtch(DEL);
                        break;
                    }
                }
                break;
            case '5':
                /* 3161 - Read cursor address in 80-column current page */
                if ( debses )
                    break;
                /* send Set Cursor Address response */
                break;
            case '6':   {
                /* 3101 - Read status */
                /* 3161 - Report terminal status */
                if ( debses )
                    break;
                /* send ESC 6 pa1 pa2 LTA */
                break;
            }
            case '7':   /* 3101 - Read setup switch */
                if ( debses )
                    break;
                break;
            case '8':   /* 3101-2x and 3161 - Read Page (buffer) */
                if ( debses )
                    break;
                break;
            case '9':   /* 3101 - Set control */
                if ( debses )
                    break;
                break;
            case 'A':
                /* Cursor Up; no scroll */
                if ( debses )
                    break;
                cursorup(0) ;
                break;
            case 'B':
                /* Cursor Down; no scroll */
                if ( debses )
                    break;
                cursordown(0) ;
                break;
            case 'C':
                /* Curosr Right */
                if ( debses )
                    break;
                cursorright(0) ;
                break;
            case 'D':
                /* Cursor Left (backspace) */
                if ( debses )
                    break;
                cursorleft(0) ;
                break;
            case 'E':
                /* 3101-2x and 3161 - Write Send Mark */
                if ( debses )
                    break;
                /* places a send mark at the current cursor/buffer   */
                /* address location depending on mode.  Replaces any */
                /* previous Send Mark with NUL.                      */
                break;
            case 'H':
                /* Cursor Home Command */
                /* and change to cursor addr mode from buffer addr mode */
                if ( debses )
                    break;
                lgotoxy(VTERM,1,1);
                break;
            case 'I':
                /* 3101-1x - Clear line to nulls from cursor */
                /* 3101-2x and 3161 - Clear unprotected line */
                /*                    to nulls from cursor  */
                if ( debses )
                    break;
                clrtoeoln( VTERM, NUL );
                break;
            case 'J':
                /* Clear unprotected page to spaces from cursor */
                if ( debses )
                    break;
                clreoscr_escape( VTERM, NUL );
                break;
            case 'K':
                /* Clear input */
                if ( debses )
                    break;
                clrscreen(VTERM,NUL);
                lgotoxy(VTERM,1,1);       /* and home the cursor */
                break;
            case 'L':
                /* 3101-2x and 3161 - Clear page to nulls */
                if ( debses )
                    break;
                clrscreen( VTERM, NUL );
                lgotoxy(VTERM,1,1);       /* and home the cursor */
                break;
            case 'M':
                /* Cursor to start of next line */
                if ( debses )
                    break;
                wrtch(CR);
                wrtch(LF);
                break;
            case 'N':
                /* 3101-2x and 3161 - Insert line of nulls */
                if ( debses )
                    break;
                VscrnScroll(VTERM, DOWNWARD, wherey[VTERM] - 1,
                             marginbot - 1, 1, FALSE, NUL);
                break;
            case 'O':
                /* 3101-2x and 3161 - Delete cursor line */
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
            case 'P': {
                /* Insert Character Command */
                pa = i31inc();
                if ( debses )
                    break;

                break;
            }
            case 'Q': {
                /* 3101-2x and 3161 - Delete cursor character */
                viocell blankvcell ;
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
            }
            case 'S':
                /* Cancel Command */
                if ( debses )
                    break;
                break;
            case 't': {
                /* 3161 - Default function key */
                int fn=0,fnx=0;
                fn = i31inc();
                if ( !(fn&BIT7) && (fn&BIT6) )
                    fnx = i31inc();
                if ( debses )
                    break;
                break;
            }
            case 'U':
                /* 3101 - Print line */
                if ( debses )
                    break;
                break;
            case 'V':
                /* 3101 - Print message */
                if ( debses )
                    break;
                break;
            case 'W':
                /* 3101 - Print formatted page */
                /* 3161 - Print viewport */
                if ( debses )
                    break;
                break;
            case 'X': {
                /* Set buffer address */
                int pr, pc ;
                pr = i31inc();
                pc = i31inc();
                if ( debses )
                    break;

                break;
            }
            case 'x': {
                /* Extended Set buffer address */
                int prh, prl, pch, pcl ;
                prh = i31inc();
                prl = i31inc();
                pch = i31inc();
                pcl = i31inc();
                if ( debses )
                    break;

                break;
            }
            case 'Y': {
                /* Address cursor in 80-column current page */
                int pr, pc ;
                pr = i31inc();
                pc = i31inc();
                if ( debses )
                    break;

                lgotoxy( VTERM, pc-31, pr-31 ) ;
                break;
            }
            case 'y': {
                /* Extended Set cursor address */
                int prh, prl, pch, pcl ;
                prh = i31inc();
                prl = i31inc();
                pch = i31inc();
                pcl = i31inc();
                if ( debses )
                    break;
                lgotoxy( VTERM, (pch-32)*32+pcl-31,
                         (prh-32)*32+prl-31-SP ) ;
                break;
            }
            case 'Z':
                /* Insert cursor */
                if ( debses )
                    break;
                break;
            case ')': {
                ch2 = i31inc();
                switch ( ch2 ) {
                case ':':
                    /* Enable Print Key Attention command */
                    if ( debses )
                        break;
                    break;
                case ';':
                    /* Disable Print Key Attention command (default) */
                    if ( debses )
                        break;
                    break;
                }
                break;
            }
            case '(': {
                ch2 = i31inc();
                switch ( ch2 ) {
                case ':':
                    /* Enable Reset Key Attention command */
                    if ( debses )
                        break;
                    break;
                case ';':
                    /* Disable Reset Key Attention command (default) */
                    if ( debses )
                        break;
                    break;
                }
                break;
            }
            }
            escstate = ES_NORMAL ;      /* Done parsing escstate sequence */
        }
    }
    else                /* Handle as a normal character */
    {
        if ( ch < SP )
            i31ctrl(ch) ;
        else if ( !debses ) {
            if (printon && is_xprint())
                prtchar(ch);
            else {
                if (printon && is_uprint())
                    prtchar(ch);
                wrtch(ch);
            }
        }
    }
    VscrnIsDirty(VTERM) ;
}
#endif /* NOTERM */
