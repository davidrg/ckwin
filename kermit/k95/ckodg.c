/*  C K O D G . C  --  Data General Emulation  */

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
#include "ckcxla.h"
#include "ckuxla.h"
#include "ckcuni.h"
#include "ckocon.h"
#include "ckodg.h"
#ifdef NETCONN
#ifdef TCPSOCKET
#include "ckcnet.h"
extern int network, nettype, ttnproto, u_binary;
#endif /* TCPSOCKET */
#endif /* NETCONN */

extern bool keyclick ;
extern int  cursorena[], keylock, duplex, duplex_sav, screenon ;
extern int  printon, aprint, uprint, cprint, xprint, seslog ;
extern int  insertmode, tnlm ;
extern int  escstate, debses, decscnm, tt_cursor;
extern int  tt_type, tt_type_mode, tt_type_vt52, tt_max, tt_answer, tt_status[VNUM], tt_szchng[] ;
extern int  tt_cols[], tt_rows[], tt_wrap ;
extern int  wherex[], wherey[], margintop, marginbot ;
extern int  marginbell, marginbellcol, parity, cmask ;
extern int  wy_monitor;
extern char answerback[], htab[] ;
extern struct tt_info_rec tt_info[] ;
extern vtattrib attrib ;
extern unsigned char attribute;
extern int autoscroll, protect ;
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR);  /* Character set xlate */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR);  /* functions. */
extern struct csinfo fcsinfo[]; /* File character set info */
extern int tcsr, tcsl;          /* Terminal character sets, remote & local. */
extern struct _vtG G[4];
extern struct _vtG *GL, *SSGL ;  /* GL and single shift GL */
extern struct _vtG *GR;          /* GR */
extern struct _vtG *GNOW;

int    dgunix = FALSE ;         /* DG Unix mode */
int    dgunix_usr = FALSE;

static unsigned char escchar=XRS;/* Remember which was used to enter GOTESC */

unsigned char
charset( enum charsetsize size, unsigned short achar, struct _vtG * pG );

extern int ttpush;
int
dginc(void)
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

    if ( !xprint )
        ch = ch & pmask & cmask;
    debugses(ch);
    if (printon && (is_xprint() || is_uprint()))
        prtchar(ch);

    return ch;
}

static char
int2unixhex( int n )
{
    char c;
    switch ( n ) {
    case 0:     c = '0'; break;
    case 1:     c = '1'; break;
    case 2:     c = '2'; break;
    case 3:     c = '3'; break;
    case 4:     c = '4'; break;
    case 5:     c = '5'; break;
    case 6:     c = '6'; break;
    case 7:     c = '7'; break;
    case 8:     c = '8'; break;
    case 9:     c = '9'; break;
    case 10:    c = 'A'; break;
    case 11:    c = 'B'; break;
    case 12:    c = 'C'; break;
    case 13:    c = 'D'; break;
    case 14:    c = 'E'; break;
    case 15:    c = 'F'; break;
    default:    c = '0';
    }
    return c;
}

static int
unixhex2int( char c )
{
    int n=-1;
    switch ( c ) {
    case '0': n = 0; break;
    case '1': n = 1; break;
    case '2': n = 2; break;
    case '3': n = 3; break;
    case '4': n = 4; break;
    case '5': n = 5; break;
    case '6': n = 6; break;
    case '7': n = 7; break;
    case '8': n = 8; break;
    case '9': n = 9; break;
    case 'A': case 'a': n = 10; break;
    case 'B': case 'b': n = 11; break;
    case 'C': case 'c': n = 12; break;
    case 'D': case 'd': n = 13; break;
    case 'E': case 'e': n = 14; break;
    case 'F': case 'f': n = 15; break;
    }
    return n;
}

int
dgcmd2int( char arg1, char arg2, char arg3 )
{
    int n = 0;

    if ( dgunix ) {
        n += unixhex2int(arg1) * 256;
        n += unixhex2int(arg2) * 16;
        n += unixhex2int(arg3);
    }
    else {
        n += (arg1 & 0x0F) * 256;
        n += (arg2 & 0x0F) * 16;
        n += (arg3 & 0x0F);
    }
    return n;
}

void
dgint2cmd( int n, char * arg1, char * arg2, char * arg3 )
{
    if ( dgunix ) {
        n %= 4096;                              /* max DG-Hex value */
        *arg1 = int2unixhex(n/256);
        n %= 256 ;
        *arg2 = int2unixhex(n/16);
        n %= 16 ;
        *arg3 = int2unixhex(n);
    }
    else {
        n %= 4096;                              /* max DG-Hex value */
        *arg1 = '0' + n / 256;
        n %= 256 ;
        *arg2 = '0' + n / 16;
        n %= 16 ;
        *arg3 = '0' + n;
    }
}

int
dgloc2int( char arg1, char arg2, char arg3 )
{
    int n = 0 ;

    if ( dgunix ) {
        n += unixhex2int(arg1) * 256;
        n += unixhex2int(arg2) * 16;
        n += unixhex2int(arg3);
    }
    else {
        n += (arg1 - '@')%32 * 1024;
        n += (arg2 - '@')%32 * 32;
        n += (arg3 - '@')%32 ;
    }
    return n;
}

void
dgint2loc( int n, char * arg1, char * arg2, char * arg3 )
{
    if ( dgunix ) {
        n %= 3072;                              /* max DG-Location value */
        *arg1 = int2unixhex(n/256);
        n %= 256 ;
        *arg2 = int2unixhex(n/16);
        n %= 16 ;
        *arg3 = int2unixhex(n);
    }
    else {
        n %= 3072;                              /* max DG-Location value */
        *arg1 = '@' + n / 1024;
        n %= 1024 ;
        *arg2 = '@' + n / 32;
        n %= 32 ;
        *arg3 = '@' + n;
    }
}

void
dgctrl( int ch )
{
    int i,j;


    if ( !xprint ) {
        if (printon && is_uprint())
            prtchar(ch);
        switch ( ch ) {
        case SOH:
            /* Print all unprotected characters */
            debug(F110,"Data General","SOH",0);
            prtscreen( VTERM, wherey[VTERM], VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0));
            sendchar(ACK);
            break;
        case STX:
            /* Reverse off */
            debug(F110,"Data General","STX",0);
            if ( debses )
                break;
            attrib.reversed = FALSE ;
            break;
        case ETX:
            debug(F110,"Data General","ETX",0);
            /* Blink Enable */
            break;
        case EOT:
            debug(F110,"Data General","EOT",0);
            /* Blink Disable */
            break;
        case ENQ: {
            /* Report cursor address */
            debug(F110,"Data General","ENQ",0);
            if ( debses || dgunix )
                break;
            sendchar(US);
            sendchar(wherex[VTERM]-1);
            sendchar(wherey[VTERM]-1);
            break;
        }
        case ACK:
            debug(F110,"Data General","ACK",0);
            break;
        case BEL:
            debug(F110,"Data General","BEL",0);
            if ( debses )
                break;
            bleep(BP_BEL);
            break;
        case BS:
            debug(F110,"Data General","BS",0);
            if ( !dgunix ) {
                /* Home Cursor */
                if ( debses )
                    break;
                lgotoxy( VTERM, 1, 1 ) ;
            } else {
                /* Backspace */
                if ( debses )
                    break;
                if ( wherex[VTERM] > 1 )
                    cursorleft(0);
            }
            break;
        case HT:
            debug(F110,"Data General","HT",0);
            if ( debses )
                break;
            i = wherex[VTERM];
            if (i < VscrnGetWidth(VTERM))
            {
                do {
                    i++;
                    cursorright(0);
                } while ( (dgunix ? (wherex[VTERM]%8) : ((htab[i] != 'T'))) &&
                            (i <= VscrnGetWidth(VTERM)-1));
                VscrnIsDirty(VTERM);
            }
            break;
        case LF:
            /* Cursor to start on next line */
            debug(F110,"Data General","LF",0);
            if ( debses )
                break;
            if ( dgunix )
            {
                wrtch((CHAR)LF);
            }
            else if ( autoscroll ||
                      wherey[VTERM] < VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0) ) {
                wrtch((CHAR)CR);
                wrtch((CHAR)LF);
            }
            else { /* Home Cursor */
                lgotoxy( VTERM, 1, 1 ) ;
            }
            break;
        case VT: /* Erase Field */
            debug(F110,"Data General","VT",0);
            if ( debses || dgunix )
                break;
            clrtoeoln( VTERM, SP ) ;
            break;
        case FF:
            debug(F110,"Data General","FF",0);
            if ( debses || dgunix )
                break;
            clrscreen(VTERM,SP);
            lgotoxy(VTERM,1,1);       /* and home the cursor */
            attrib.blinking = FALSE;
            attrib.underlined = FALSE;
            attrib.reversed = FALSE;
            attrib.dim = FALSE;
            break;
        case CR:
            debug(F110,"Data General","CR",0);
            if ( debses )
                break;
            wrtch((char) CR);
            break;
        case SO:
            debug(F110,"Data General","SO",0);
            if ( debses )
                break;
            if ( dgunix )
                GL = &G[1];
            else
                attrib.blinking = TRUE ;
            break;
        case SI:
            debug(F110,"Data General","SI",0);
            if ( debses )
                break;
            if ( dgunix )
                GL = &G[0];
            else
                attrib.blinking = FALSE ;
            break;
        case DLE: {
            /* Address cursor in current page */
            int col, line ;
            debug(F110,"Data General","DLE",0);
            col  = dginc();
#ifdef NETCONN
#ifdef TCPSOCKET
            if ( network && IS_TELNET() && !TELOPT_U(TELOPT_BINARY) && col == CR ) {
                /* Handle TELNET CR-NUL or CR-LF if necessary */
                int dummy = ttinc(0);
                debug(F111,"Data General","Addr cursor in page found CR",dummy);
                if ( dummy != NUL )
                    ttpush = dummy;
            }
#endif /* TCPSOCKET */
#endif /* NETCONN */
            line = dginc();
#ifdef COMMENT
#ifdef NETCONN
#ifdef TCPSOCKET
            if ( network && IS_TELNET() && !TELOPT_U(TELOPT_BINARY) && line == CR ) {
                /* Handle TELNET CR-NUL or CR-LF if necessary */
                int dummy = ttinc(0);
                debug(F111,"Data General","Addr cursor in page found CR",dummy);
                if ( dummy != NUL )
                    ttpush = dummy;
            }
#endif /* TCPSOCKET */
#endif /* NETCONN */
#endif /* COMMENT */
            if ( debses )
                break;

            if ( col == 127 )
                col = wherex[VTERM] - 1;
            if ( line == 127 )
                line = wherey[VTERM] - 1;

            lgotoxy(VTERM,col+1,line+1);
            break;
        }
        case DC1: {
            /* Print - current line to end of screen */
            /* Send Ctrl-F ACK when print is complete */
            debug(F110,"Data General","DC1",0);
            if ( debses || dgunix )
                break;
            debug(F110,"Data General","Print current line to end of screen",0);
            prtscreen( VTERM, wherey[VTERM], VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0));
            sendchar(ACK);
            break;
        }
        case DC2:
            debug(F110,"Data General","DC2",0);
            if ( debses )
                break;
            autoscroll = TRUE ;
            break;
        case DC3:
            debug(F110,"Data General","DC3",0);
            if ( debses || dgunix )
                break;
            autoscroll = FALSE ;
            break;
        case DC4:
            debug(F110,"Data General","DC4",0);
            if ( debses )
                break;
            attrib.underlined = TRUE ;
            break;
        case NAK:
            debug(F110,"Data General","NAK",0);
            if ( debses )
                break;
            attrib.underlined = FALSE ;
            break;
        case SYN:
            debug(F110,"Data General","SYN",0);
            if ( debses )
                break;
            attrib.reversed = TRUE ;
            break;
        case ETB:
            debug(F110,"Data General","ETB",0);
            if ( debses || dgunix )
                break;
            cursorup(0);
            break;
        case CAN:
            debug(F110,"Data General","CAN",0);
            if ( debses || dgunix )
                break;
            cursorright(0);
            break;
        case XEM:
            debug(F110,"Data General","EM",0);
            if ( debses || dgunix )
                break;
            cursorleft(0);
            break;
        case SUB:
            debug(F110,"Data General","SUB",0);
            if ( debses || dgunix )
                break;
            cursordown(0);
            break;
        case ESC:
            /* We will treat RS and ESC to be equivalent for our purposes            */
            /* initiate escape sequence */
            debug(F110,"Data General","ESC",0);
            escstate = ES_GOTESC ;
            escchar  = ESC;
            break;
        case XFS:
            debug(F110,"Data General","FS",0);
            if ( debses )
                break;
            attrib.dim = TRUE ;
            break;
        case XGS:
            debug(F110,"Data General","GS",0);
            if ( debses )
                break;
            attrib.dim = FALSE ;
            break;
        case XRS:
            /* This appears to be the beginning of escape character on the DG series */
            /* We will treat RS and ESC to be equivalent for our purposes            */
            /* initiate escape sequence */
            debug(F110,"Data General","RS",0);
            escstate = ES_GOTESC ;
            escchar  = XRS;
            break;
        case US:
            debug(F110,"Data General","US",0);
            break;
        }
    }
    else {      /* xprint */
        switch ( ch ) {
        case ESC:
            /* We will treat RS and ESC to be equivalent for our purposes            */
            /* initiate escape sequence */
            escstate = ES_GOTESC ;
            escchar  = ESC;
            break;
        case XRS:
            /* This appears to be the beginning of escape character on the DG series */
            /* We will treat RS and ESC to be equivalent for our purposes            */
            /* initiate escape sequence */
            escstate = ES_GOTESC ;
            escchar  = XRS;
            break;
        default:
            if (printon && is_uprint())
                prtchar(ch);
        }
    }
}

void
dgascii( int ch )
{
    int i,j,k,n,x,y,z;
    vtattrib attr ;
    viocell blankvcell;
    char arg1, arg2, arg3, arg4, arg5;

    if ( escstate == ES_GOTESC )/* Process character as part of an escstate sequence */
    {
        if ( ch < SP ) {
            escstate = ES_NORMAL ;
            dgctrl(ch) ;
        }
        else
        {
            escstate = ES_ESCSEQ ;
            if ( !xprint ) {

                /* We don't print escape sequences */

            switch ( ch ) {
            case '"':
                /* Unlock Keyboard */
                debug(F110,"Data General","Unlock Keyboard",0);
                if ( debses )
                    break;
                keylock = FALSE ;
                break;
            case '#':
                /* Lock Keyboard */
                debug(F110,"Data General","Lock Keyboard",0);
                if ( debses )
                    break;
                keylock = TRUE ;
                break;
            case 'A': 
                /* DG411 - Set Foreground Color */
                debug(F110,"Data General","Set Foreground Color",0);
                arg1 = dginc();
                if ( debses )
                    break;
                break;
            case 'C': {
                /* Send Terminal ID */
                /* RS o # <m> <x> <y>                    */
                /* <m> = model: d217 '5'; d413/d463 '6'  */
                /* <x> = bits  01TC PRRR                 */
                /*      T - 0:self test passed; 1:errors */
                /*      C - 0:7bit; 1:8bit               */
                /*      P - 0:no printer; 1:printer      */
                /*      R - 3bit revision number         */
                /* <y> = bits  01GK LLLL                 */
                /*      G - 0:no graphics; 1:graphics    */
                /*      K - 0:no keyboard; 1:keyboard    */
                /*      L - keyboard mode (1001 - US)    */
                char response[7];
                if ( debses )
                    break;
                response[0] = XRS;
                response[1] = 'o';
                response[2] = '#';

                switch ( tt_type_mode ) {
                case TT_DG200:
                    response[3] = '!';
                    break;
                case TT_DG210:
                    response[3] = '(';
                    break;
                case TT_DG217:
                    response[3] = '5';
                    break;
#ifdef COMMENT
                case TT_DG413:
                case TT_DG463:
                    response[3] = '6';
                    break;
#endif /* COMMENT */
                }

                if ( cmask == 0177 )
                    response[4] = 0x48 | 0x03; /* 7-bit; printer */
                else
                    response[4] = 0x58 | 0x03;  /* 8-bit; printer */
                response[5] = 0x59;     /* keyboard; US */
                response[6] = '\0';
                debug(F110,"Data General Read Model ID",response,0);
                sendchars(response,6);
                break;
            }
            case 'D':
                debug(F110,"Data General","Reverse On",0);
                if ( debses )
                    break;
                attrib.reversed = TRUE ;
                break;
            case 'E':
                debug(F110,"Data General","Reverse Off",0);
                if ( debses )
                    break;
                attrib.reversed = FALSE ;
                break;
            case 'F': {
                ch = dginc() ;
                switch ( ch ) {
                case '?': {
                    ch = dginc() ;
                    if ( debses )
                        break;
                    switch ( ch ) {
                    case ':':   /* Print Screen */
                        debug(F110,"Data General","Print Screen",0);
                        prtscreen( VTERM, 1, VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0));
                        sendchar(ACK);
                        break;
                    case '0':
                        debug(F110,"Data General 4xx","Simulprint Off",0);
                        break;
                    case '1':
                        debug(F110,"Data General 4xx","Simulprint On",0);
                        break;
                    case '2':
                        debug(F110,"Data General","Xprint Off",0);
                        xprint = FALSE ;
                        if ( !uprint && !xprint && !aprint && printon )
                            printeroff();
                        sendchar(ACK);
                        break;
                    case '3':
                        debug(F110,"Data General","Xprint On",0);
                        xprint = TRUE;
                        printeron();
                        break;
                    case '5':
                        debug(F110,"Data General 4xx","Window Bit Dump",0);
                        break;
                    case '6':
                        debug(F110,"Data General 4xx","Form Bit Dump",0);
                        break;
                    case '7':   /* VT220 AutoPrint off */
                        debug(F110,"Data General","Aprint Off",0);
                        setaprint(FALSE);
                        if ( !uprint && !xprint && !aprint && printon )
                            printeroff();
                        sendchar(ACK);
                    case '8':   /* VT220 AutoPrint on */
                        debug(F110,"Data General","Aprint On",0);
                        setaprint(TRUE);
                        printeron();
                        break;
                    case '9':   /* DG UNIX - Print Window */
                        if ( dgunix ) {
                            /* Print - current line to end of screen */
                            /* Send Ctrl-F ACK when print is complete */
                            debug(F110,"Data General",
                                   "Print current line to end of screen",0);
                            prtscreen( VTERM, wherey[VTERM],
                                       VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0));
                            sendchar(ACK);
                        }
                        break;
                    }
                    break;
                }
                case ';': {
                    /* Data Trap Mode */
                    int cmd = dginc();

                    debug(F111,"Data General","Data Trap Mode",cmd);

                    switch ( cmd ) {
                    case 'T':
                        if ( wy_monitor )
                            break;
                        setdebses(FALSE);
                        break;
                    case 'H':
                    case 'O':
                    case DEL:
                        if ( debses || wy_monitor )
                            break;
                        setdebses(TRUE);
                        wy_monitor = TRUE ;
                        break;
                    }
                }
                case '~': {
                    /* Switch Emulations */
                    int emulation = 0 ;
                    char response[5] = "";

                    arg1 = '0';
                    arg2 = dginc();
                    arg3 = dginc();

                    emulation = dgcmd2int( arg1, arg2, arg3 );

                    debug(F111,"Data General","Switch Emulation",emulation);

                    if ( debses )
                        break;
                    response[0] = XRS;
                    response[1] = 'o';
                    response[2] = '~';

                    switch ( emulation ) {
                    case 0x00:  /* DG Native mode */
                        response[3] = '1';
                        break;
                    case 0x08:  /* VT52 mode */
                        tt_type_vt52 = tt_type_mode;
                        tt_type_mode = TT_VT52;
                        response[3] = '1';
                        ipadl25();
                        break;
                    case 0x09:  /* VT100 mode */
                        tt_type_mode = TT_VT102;
                        response[3] = '1';
                        ipadl25();
                        break;
                    case 0x0C:  /* VT320 mode */
                        tt_type_mode = TT_VT320;
                        response[3] = '1';
                        ipadl25();
                        break;
                    case 0x10:  /* Tektronix 4010 */
                        response[3] = '0';
                        ipadl25();
                        break;
                    }
                    response[4] = '\0';
                    debug(F110,"Data General Switch Emulation Mode",response,0);
                    sendchars( response, 4 );
                    break;
                }
                case '{': {
                    /* Set Model ID */
                    int model = 0 ;
                    int graphics = 0;

                    arg1 = '0';
                    arg2 = dginc();
                    arg3 = dginc();
                    model = dgcmd2int( arg1, arg2, arg3 );

                    arg2 = '0';
                    arg3 = dginc();
                    graphics = dgcmd2int( arg1, arg2, arg3 );

                    debug(F110,"Data General Set Model ID","not supported",0);
                    if ( debses )
                        break;
                    break;
                }
                case '7': {
                    /* Select Printer National Character Set */
                    int lang;

                    arg1 = '0';
                    arg2 = dginc();
                    arg3 = dginc();
                    lang = dgcmd2int( arg1, arg2, arg3 );
                    debug(F101,"Data General Select Printer National CS","",lang);

                    if ( debses )
                        break;

                    break;
                }
                case '8':
                    debug(F110,"Data General 4xx","Display Character Generator Contents",0);
                    if ( debses )
                        break;
                    break;
                case '9':
                    debug(F110,"Data General 4xx","Fill Screen with Grid",0);
                    if ( debses )
                        break;
                    break;
                case '<':
                    debug(F110,"Data General 4xx","DG411 - Perform Uart Loopback Test",0);
                    if ( debses )
                        break;
                    break;
                case '>':
                    debug(F110,"Data General 4xx","Fill Screen with Character",0);
                    arg1 = dginc();
                    if ( debses )
                        break;
                    break;
                case '@':
                    debug(F110,"Data General 4xx","Select ANSI mode",0);
                    break;
                case 'A':
                    debug(F110,"Data General 4xx","Reset",0);
                    if ( debses )
                        break;
                    doreset(1);
                    break;
                case 'B':
                    debug(F110,"Data General 4xx","Set Windows",0);
                    /* <row count><normal>...<row count><normal>
                     * how do we know when the sequence ends?
                     */
                    arg1 = dginc();     
                    break;
                case 'C': {
                    /* Scroll Left - Pan Right */
                    viocell cell ;
                    debug(F110,"Data General 4xx","Scroll Left",0);
                    arg1 = dginc(); /* Column count */

                    if ( debses )
                        break;

                    cell.c = SP ;
                    cell.a = geterasecolor(VTERM) ;
                    if ( arg1 == 0 )
                        arg1 = 1 ;
                    else if ( arg1 > VscrnGetWidth(VTERM)-1 )
                        arg1 = VscrnGetWidth(VTERM)-1 ;
                    VscrnScrollLf( VTERM,
                                   0,
                                   0,
                                   VscrnGetHeight(VTERM)
                                   -(tt_status[VTERM]?2:1),
                                   VscrnGetWidth(VTERM)-1,
                                   arg1,
                                   cell);
                    break;
                }
                case 'D': {
                    /* Scroll Right - Pan Left */
                    viocell cell ;
                    debug(F110,"Data General 4xx","Scroll Right",0);
                    arg1 = dginc(); /* Column count */

                    if ( debses )
                        break;

                    cell.c = SP ;
                    cell.a = geterasecolor(VTERM) ;
                    if ( arg1 == 0 )
                        arg1 = 1 ;
                    else if ( arg1 > VscrnGetWidth(VTERM)-1 )
                        arg1 = VscrnGetWidth(VTERM)-1 ;
                        VscrnScrollRt( VTERM,
                                       0,
                                       0,
                                       VscrnGetHeight(VTERM)
                                       -(tt_status[VTERM]?2:1),
                                       VscrnGetWidth(VTERM)-1,
                                       arg1,
                                       cell);
                    break; 
                }
                case 'E':
                    debug(F110,"Data General 4xx","Erase Screen",0);
                    if ( debses )
                        break;
                    clrscreen(VTERM, SP);
                    break;
                case 'F':
                    /* D413 Clear Unprotected (cursor to EOS) */
                    debug(F110,"Data General 4xx","Erase Unprotected",0);
                    if ( debses )
                        break;
                    if ( protect )
                        selclreoscr_escape(VTERM, SP);
                    else
                        clreoscr_escape(VTERM, SP);
                    break;
                case 'G':
                    debug(F110,"Data General 4xx","Screen Home",0);
                    break;
                case 'H':
                    debug(F110,"Data General 4xx","Insert Line",0);
                    break;
                case 'I':
                    debug(F110,"Data General 4xx","Delete Line",0);
                    break;
                case 'J':
                    debug(F110,"Data General 4xx","Select Normal Spacing",0);
                    break;
                case 'K':
                    debug(F110,"Data General 4xx","Select Compressed Spacing",0);
                    break;
                case 'L':
                    debug(F110,"Data General 4xx","Protect On",0);
                    if ( debses )
                        break;
                    attrib.unerasable = TRUE;
                    break;
                case 'M':
                    debug(F110,"Data General 4xx","Protect Off",0);
                    if ( debses )
                        break;
                    attrib.unerasable = FALSE;
                    break;
                case 'N': {
                    /* D413 Change Attributes <nnn> <n> <n> */
                    int chars = 0 ;
                    int toset = 0 ;
                    int toreset = 0;

                    debug(F110,"Data General 4xx","Change Attributes",0);

                    arg1 = dginc();
                    arg2 = dginc();
                    arg3 = dginc();
                    chars = dgcmd2int( arg1, arg2, arg3 );

                    arg1 = '0';
                    arg2 = '0';
                    arg3 = dginc();
                    toset = dgcmd2int( arg1, arg2, arg3 );

                    arg3 = dginc();
                    toreset = dgcmd2int( arg1, arg2, arg3 );

                    if ( debses )
                        break;
                    break;
                }
                case 'O':
                    debug(F110,"Data General 4xx","Read Horizontal Scroll Offset",0);
                    break;
                case 'P': {
                    /* Write Screen Address <nn> <nn> */
                    int col=0, row=0;
                    arg1 = '0';
                    arg2 = dginc();
                    arg3 = dginc();
                    col = dgcmd2int( arg1, arg2, arg3 );

                    arg2 = dginc();
                    arg3 = dginc();
                    row = dgcmd2int( arg1, arg2, arg3 );

                    debug(F110,"Data General","Write Screen Address",0);

                    if ( debses )
                        break;

                    if ( col == 255 )
                        col = wherex[VTERM] - 1 ;
                    if ( row == 255 )
                        row = wherey[VTERM] - 1 ;

                    lgotoxy( VTERM, col+1, row+1 );
                    break;
                }
                case 'Q': {
                    /* Set Cursor Type - DG411/461 */
                    arg1 = dginc();

                    debug(F111,"Data General","Set Cursor Type",arg1-'0');

                    if ( debses )
                        break;
                    switch ( arg1 ) {
                    case '0':
                        cursorena[VTERM] = FALSE;
                        break;
                    case '5':
                        cursorena[VTERM] = TRUE;
                        break;
                    case '1': /* blink */
                    case '4': /* no blink */
                        tt_cursor = TTC_ULINE;
                        setcursormode();
                        cursorena[VTERM] = TRUE;
                        break;
                    case '2': /* no blink */
                    case '3': /* blink */
                        tt_cursor = TTC_BLOCK;
                        setcursormode();
                        cursorena[VTERM] = TRUE;
                        break;
                    }
                    break;
                }
                case 'R':
                    debug(F110,"Data General 4xx","Define Character",0);
                    arg1 = dginc(); /* character */
                    arg2 = dginc(); /* row 0 */
                    arg3 = dginc(); /* row 9 */
                    break;
                case 'S': {
                    /* Select Character Set <nn> (p 2-27) */
                    int cs ;
                    arg1 = '0' ;
                    arg2 = dginc();
                    arg3 = dginc();
                    if ( debses )
                        break;
                    cs = dgcmd2int( arg1, arg2, arg3 );
                    debug( F111, "Data General Switch to Character Set",
                           (GL==&G[0])?"G0":"G1",cs);
                    switch ( cs ) {
                    case 0:     /* Keyboard language */
                    case 1:     /* US ASCII */
                        charset( cs94, 'B', GL );
                        break;
                    case 2:     /* UK ASCII */
                        charset( cs94, 'A', GL );
                        break;
                    case 3:     /* French */
                        charset( cs94, 'R', GL );
                        break;
                    case 4:     /* German */
                        charset( cs94, 'K', GL );
                        break;
                    case 5:     /* Swedish/Finnish */
                        charset( cs94, 'C', GL );
                        break;
                    case 6:     /* Spanish */
                        charset( cs94, 'Y', GL );
                        break;
                    case 7:     /* Danish/Nowegian */
                        charset( cs94, 'E', GL );
                        break;
                    case 8:     /* Swiss */
                        charset( cs94, '=', GL );
                        break;
                    case 9:     /* Kata Kana (G0) */
                        charset( cs94, 'B', GL );
                        break;
                    case 14:    /* DG International */
                        charset( cs96, 'd', GL );
                        break;
                    case 15:    /* Kata Kana (G1) */
                        charset( cs94, 'I', GL );
                        break;
                    case 16:    /* DG Word Processing, Greek, Math */
                        charset( cs96, 'g', GL );
                        break;
                    case 17:    /* DG Line Drawing */
                        charset( cs96, 'f', GL );
                        break;
                    case 19:    /* DG Special Graphics */
                        charset( cs96, 'e', GL );
                        break;
                    case 20:    /* DEC Multinational */
                        charset( cs94, '<', GL );
                        break;
                    case 21:    /* DEC Special Graphics */
                        charset( cs94, '0', GL );
                        break;
                    case 29:    /* Low PC Term */
                        charset( cs94, 'B', GL );
                        break;
                    case 30:    /* High PC Term */
                        charset( cs94, '?', GL );
                        break;
                    case 31:    /* ISO 8859/1.2 */
                        charset( cs96, 'A', GL );
                        break;
                    default:
                        if ( cs >= 32 || cs <= 45 )
                            ; /* Downloadable Character Sets */
                    }
                    break;
                }
                case 'T':
                    debug(F110,"Data General 4xx","Set Scroll Rate",0);
                    break;
                case 'U': {
                    /* Select 7/8 Bit Operation */
                    int bits = 0 ;

                    arg1 = '0' ;
                    arg2 = '0' ;
                    arg3 = dginc();
                    bits = dgcmd2int( arg1, arg2, arg3 );

                    debug(F110,"Data General","Select 7/8 Bit Operation",0);

                    if ( debses )
                        break;

                    if ( bits ) {
                        /* 8-bits */
                        setcmask(8);
                        parity = 0;
                    }
                    else {
                        /* 7-bits */
                        setcmask(7);
                    }
                    break;
                }
                case 'V':
                    debug(F110,"Data General 4xx","Protect Enable",0);
                    if ( debses )
                        break;
                  protect = TRUE;
                    break;
                case 'W':
                    debug(F110,"Data General 4xx","Protect Disable",0);
                    if ( debses )
                        break;
                  protect = FALSE;
                    break;
                case 'X':
                    debug(F110,"Data General 4xx","Set Margins",0);
                    arg1 = dginc(); /* left */
                    arg2 = dginc(); /* right */
                    break;
                case 'Y':
                    debug(F110,"Data General 4xx","Set Alternate Margins",0);
                    arg1 = dginc(); /* row */
                    arg2 = dginc(); /* left */
                    arg3 = dginc(); /* right */
                    break;
                case 'Z':
                    debug(F110,"Data General 4xx","Restore Normal Margins",0);
                    break;
                case '[':
                    debug(F110,"Data General 4xx","Insert Line Between Margins",0);
                    break;
                case '\\':
                    debug(F110,"Data General 4xx","Delete Line Between Margins",0);
                    break;
                case ']':
                    debug(F110,"Data General 4xx","Horizontal Scroll Disable",0);
                    break;
                case '^':
                    debug(F110,"Data General 4xx","Horizontal Scroll Enable",0);
                    break;
                case '_':
                    debug(F110,"Data General 4xx","Show Columns",0);
                    arg1 = dginc(); /* left */
                    arg2 = dginc(); /* right */
                    break;
                case '`':
                    if ( debses )
                        break;
                    debug(F110,"Data General","Xprint On",0);
                    xprint = TRUE;
                    printeron();
                    break;
                case 'a':
                    if ( debses )
                        break;
                    debug(F110,"Data General","Xprint Off",0);
                    xprint = FALSE ;
                    if ( !uprint && !xprint && !aprint && printon )
                        printeroff();
                    sendchar(ACK);
                    break;
                case 'b':
                    debug(F110,"Data General 4xx","Read Screen Address",0);
                    break;
                case 'd':
                    debug(F110,"Data General 4xx","Read Characters Remaining",0);
                    break;
                case 'e':
                    debug(F110,"Data General 4xx","Initialize Draw",0);
                    arg1 = dginc(); /* set */
                    arg2 = dginc(); /* number */
                    break;
                case 'f': {
                    /* Set Keyboard Language */
                    int lang=0;

                    arg1='0';
                    arg2='0';
                    arg3=dginc();
                    lang = dgcmd2int( arg1, arg2, arg3 );

                    debug(F111,"Data General","Set Keyboard Language",lang);

                    if ( debses )
                        break;

                    for ( i = 0 ; i < 4 ; i++ )
                    {
                        if ( i == 0 ) {
                            switch ( lang ) {
                            case 0:     /* Default (national) */
                                G[i].designation = G[i].def_designation ;
                                G[i].size = G[i].def_size ;
                                G[i].c1 = G[i].def_c1 ;
                                break;
                            case 1:     /* DG International */
                            case 2:     /* Latin 1 */
                                G[i].designation = TX_ASCII ;
                                G[i].size = cs94 ;
                                G[i].c1 = TRUE ;
                                break;
                            }
                        }
                        else {
                            switch ( lang ) {
                            case 0:     /* Default (national) */
                                G[i].designation = G[i].def_designation ;
                                G[i].size = G[i].def_size ;
                                G[i].c1 = G[i].def_c1 ;
                                break;
                            case 1:     /* DG International */
                                G[i].designation = TX_DGI ;
                                G[i].size = cs96 ;
                                G[i].c1 = TRUE ;
                                break;
                            case 2:     /* Latin 1 */
                                G[i].designation = TX_8859_1 ;
                                G[i].size = cs96 ;
                                G[i].c1 = TRUE ;
                                break;
                            }
                        }
                        G[i].national = CSisNRC(G[i].designation) ;
                        if ( G[i].designation == TX_TRANSP ) {
                            debug(F111,"Data General - ERROR","G(i).designation == TX_TRANSP",i);
                            G[i].rtoi = NULL ;
                            G[i].itol = NULL ;
                            G[i].ltoi = NULL ;
                            G[i].itor = NULL ;
                        }
                        else
                        {
                            G[i].rtoi = xl_u[G[i].designation];
                            G[i].itol = xl_tx[tcsl] ;
                            G[i].ltoi = xl_u[tcsl] ;
                            G[i].itor = xl_tx[G[i].designation];
                        }
                        G[i].init = FALSE ;
                    }
                    GNOW = GL = &G[0] ;
                    GR = &G[1] ;
                    SSGL = NULL ;
                    break;
                }
                case 'h':
                    debug(F110,"Data General 4xx","Push",0);
                    break;
                case 'i':
                    debug(F110,"Data General 4xx","Pop",0);
                    break;
                case 'm': {
                    ch = dginc() ;
                    switch ( ch ) {
                    case '0':
                        debug(F110,"Data General 4xx","Read Cursor Contents",0);
                        break;
                    }
                    break;
                }
                case 'r': {
                    /* Set Clock Time */
                    int status=0, hh = 0, mm = 0;

                    arg1='0';
                    arg2='0';
                    arg3=dginc();
                    status = dgcmd2int( arg1, arg2, arg3 );

                    if ( status == 2 || status == 3 ) {
                        dginc();        /* unused clock position */
                        dginc();
                        dginc();
                        dginc();

                        arg2=dginc();
                        arg3=dginc();
                        hh = 10 * (arg2-'0') + (arg3-'0');

                        if ( dginc() != ':' )
                            debug(F100,"Data General Set Time - Invalid spacing character","",0);

                        arg2=dginc();
                        arg3=dginc();
                        mm = 10 * (arg2-'0') + (arg3-'0');
                    }

                    debug(F110,"Data General","Set Clock Time",0);

                    if ( debses )
                        break;
                    loadtod( hh, mm );
                    break;
                }
                case 't': {
                    /* Report Screen Size */
                    /* Responds <036> o < <nn> <nn> <nn> <nn> <status> */
                    char response[13] ;
                    if ( debses )
                        break;
                    response[0] = XRS;
                    response[1] = 'o';
                    response[2] = '<';

                    dgint2loc( VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0),&arg1,&arg2,&arg3);
                    response[3] = arg2;
                    response[4] = arg3;
                    response[7] = arg2;
                    response[8] = arg3;

                    dgint2loc( VscrnGetWidth(VTERM),&arg1,&arg2,&arg3);
                    response[5] = arg2;
                    response[6] = arg3;
                    response[9] = arg2;
                    response[10] = arg3;

                    response[11] = 'p'; /* one host; screen save not enabled */
                    response[12] = '\0';

                    debug(F110,"Data General Report Screen Size",response,0);
                    sendchars( response, 12 );
                    break;
                }
                case 'v': {
                    /* Read Window Contents <nn> <nn> <nn> <nn> */
                    int r1=0, c1=0, r2=0, c2=0;
                    arg1 = '0';
                    arg2 = dginc();
                    arg3 = dginc();
                    r1 = dgcmd2int( arg1, arg2, arg3 );

                    arg2 = dginc();
                    arg3 = dginc();
                    c1 = dgcmd2int( arg1, arg2, arg3 );

                    arg2 = dginc();
                    arg3 = dginc();
                    r2 = dgcmd2int( arg1, arg2, arg3 );

                    arg2 = dginc();
                    arg3 = dginc();
                    c2 = dgcmd2int( arg1, arg2, arg3 );

                    debug(F110,"Data General","Read Window Contents",0);

                    if ( debses )
                        break;

                    /* response with text bounded by rectangle <r1,c1>,<r2,c2>      */
                    /* each line separated with CR-LF.                              */
                    /* Terminate with ACK if configured to do so                    */
                    /* Characters from screen are to be translated using G0,G1 sets */
                    break;
                }
                case 'w': {
                    /* Read New Model ID */

                    char response[21] = "" ;

                    if ( debses )
                        break;
                    response[0] = XRS ;
                    response[1] = 'o';
                    response[2] = 'w';
                    switch ( tt_type_mode ) {
                    case TT_DG200:
                    case TT_DG210:
                    case TT_DG217:
                        response[3] = '1';      /* D200 series terminal */
                        break;
#ifdef COMMENT
                    case TT_DG413:
                        response[3] = '3';
                        break;
#endif /* COMMENT */
                    }

                    response[4] = '0';
                    response[5] = '1';

                    response[6] = '0';
                    response[7] = '0';

                    response[8] = 'D';
                    switch ( tt_type_mode ) {
                    case TT_DG200:
                        response[9] =  '2';
                        response[10] = '0';
                        response[11] = '0';
                        break;
                    case TT_DG210:
                        response[9] =  '2';
                        response[10] = '1';
                        response[11] = '0';
                        break;
                    case TT_DG217:
                        response[9] =  '2';
                        response[10] = '1';
                        response[11] = '7';
                        break;
#ifdef COMMENT
                    case TT_DG413:
                        response[9] =  '4';
                        response[10] = '1';
                        response[11] = '3';
                        break;
#endif
                    }
                    response[12] = response[13] = response[14] = response[15] = SP;
                    response[16] = response[17] = response[18] = response[19] = SP;
                    response[20] = '\0';
                    debug(F110,"Data General Read new Model ID",response,0);
                    sendchars(response,20);
                    break;
                }
                case 'x': {
                    /* Printer Pass Back to Host */
                    int mode;
                    char response[5];
                    arg1=arg2='0';
                    arg3=dginc();
                    mode=dgcmd2int(arg1, arg2, arg3);

                    if ( debses )
                        break;
                    response[0] = XRS;
                    response[1] = 'R';
                    response[2] = 'x';
                    response[3] = '0';
                    response[4] = '\0';
                    debug(F110,"Data General Printer Pass Back to Host",response,0);
                    sendchars(response,4);
                    break;
                }
                case 'z': {
                    /* Set 25th Line Mode <n> */
                    /* 0 - Status Line */
                    /* 1 - Message <nn> <Text> */
                    /* 2 - Extra screen row */
                    /* 3 - disabled */
                    int mode=0 ;
                    int length=0;
                    char message[128]="" ;
                    arg1 = '0';
                    arg2 = '0';
                    arg3 = dginc();
                    mode = dgcmd2int( arg1, arg2, arg3 );
                    if ( mode == 1 ) {
                        arg2 = dginc();
                        arg3 = dginc();
                        length = dgcmd2int( arg1, arg2, arg3 );
                        for ( i=0;i<length;i++ )
                            message[i] = dginc();
                        message[i] = '\0';
                    }

                    debug(F110,"Data General","Set 25th Line Mode",0);

                    if ( debses )
                        break;
                    break;
                }
                } /* F */
                break;
            }
            case 'G': {
                ch = dginc() ;
                switch ( ch ) {
                case 'F':
                    debug(F110,"Data General 4xx","Arc",0);
                    arg1 = dginc(); /* x */
                    arg2 = dginc(); /* y */
                    arg3 = dginc(); /* radius */
                    arg4 = dginc(); /* start */
                    arg5 = dginc(); /* end */
                    break;
                case 'G':
                    debug(F110,"Data General 4xx","Bar",0);
                    arg1 = dginc(); /* x */
                    arg2 = dginc(); /* y */
                    arg3 = dginc(); /* width */
                    arg4 = dginc(); /* height */
                    arg5 = dginc(); /* color */
                    break;
                case '8':
                    debug(F110,"Data General 4xx","Line",0);
                    do {
                        arg1 = dginc(); /* loc list */
                    } while (arg1 != NUL);
                    break;
                case ':':
                    debug(F110,"Data General 4xx","Polygon Fill",0);
                    do {
                        arg1 = dginc(); /* loc list */
                    } while (arg1 != NUL);
                    break;
                case '>':
                    ch = dginc();
                    switch ( ch ) {
                    case '|':
                        debug(F110,"Data General 4xx","Write Cursor Location",0);
                        arg1 = dginc(); /* x */
                        arg2 = dginc(); /* y */
                        break;
                    }
                    break;
                case '?':
                    ch = dginc();
                    switch ( ch ) {
                    case '|':
                        debug(F110,"Data General 4xx","Read Cursor Location",0);
                        break;
                    }
                    break;
                case '@':
                    debug(F110,"Data General 4xx","Read Cursor Attributes",0);
                    break;
                case 'A':
                    debug(F110,"Data General 4xx","Cursor Reset",0);
                    break;
                case 'B':
                    debug(F110,"Data General 4xx","Cursor On",0);
                    break;
                case 'C':
                    debug(F110,"Data General 4xx","Cursor Off",0);
                    break;
                case 'H':
                    debug(F110,"Data General 4xx","Cursor Track",0);
                    arg1 = dginc(); /* device */
                    break;
                case 'p':
                    ch = dginc();
                    switch ( ch ) {
                    case '1':
                        debug(F110,"Data General 4xx","Set Pattern",0);
                        arg1 = dginc(); /* offset */
                        do {
                            arg2 = dginc(); /* pattern definition */
                        } while (arg2 != NUL);
                        break;
                    }
                    break;
                }
                break;
            } /* G */
            case 'H':
                debug(F110,"Data General 4xx","Scroll Up",0);
                VscrnScroll( VTERM, 
                             UPWARD,
                             margintop-1, 
                             wherex[VTERM]-1,
                             1,
                             margintop == 1,
                             SP);
                break; 
            case 'I':
                debug(F110,"Data General 4xx","Scroll Down",0);
                VscrnScroll( VTERM,
                             DOWNWARD,
                             wherex[VTERM]-1,
                             marginbot-1,
                             1,
                             FALSE,
                             SP);
                break;
            case 'J':
                debug(F110,"Data General 4xx","Insert Character",0);
                break;
            case 'K':
                debug(F110,"Data General 4xx","Delete Character",0);
                break;
            case 'L':
                debug(F110,"Data General 4xx","Line",0);
                do {
                    arg1 = dginc(); /* loc list */
                } while (arg1 != NUL);
                break;
            case 'N':   /* Shift-Out */
                debug(F110,"Data General","Shift-Out",0);

                if ( debses )
                    break;
                GL = &G[1];
                break;
            case 'O':   /* Shift-In */
                debug(F110,"Data General","Shift-In",0);

                if ( debses )
                    break;
                GL = &G[0];
                break;
            case 'P': {
                ch = dginc() ;
                switch ( ch ) {
                case '@': {
                    /* Unix Mode 0:exit 1:enter */
                    int mode=0;
                    arg1='0';
                    arg2='0';
                    arg3=dginc();
                    mode = dgcmd2int(arg1, arg2, arg3);

                    debug(F111,"Data General","Unide Mode",mode-'0');

                    if ( debses )
                        break;
                    dgunix = mode;
                    break;
                }
                case 'A': { /* DG UNIX - Cursor Up */
                    debug(F110,"Data General","Unix Cursor Up",0);
                    if ( debses || !dgunix )
                        break;
                    cursorup(0);
                    break;
                }
                case 'B': { /* DG UNIX - Cursor Down */
                    debug(F110,"Data General","Unix Cursor Down",0);
                    if ( debses || !dgunix )
                        break;
                    cursordown(0);
                    break;
                }
                case 'C': { /* DG UNIX - Cursor Right */
                    debug(F110,"Data General","Unix Cursor Right",0);
                    if ( debses || !dgunix )
                        break;
                    cursorright(0);
                    break;
                }
                case 'D': { /* DG UNIX - Cursor Left */
                    debug(F110,"Data General","Unix Cursor Left",0);
                    if ( debses || !dgunix )
                        break;
                    cursorleft(0);
                    break;
                }
                case 'E': { /* DG UNIX - Erase Field */
                    debug(F110,"Data General","Unix Erase Field",0);
                    if ( debses || !dgunix )
                        break;
                    clrtoeoln( VTERM, SP ) ;
                    break;
                }
                case 'F': { /* DG UNIX - Window Home */
                    debug(F110,"Data General","Unix Window Home",0);
                    if ( debses || !dgunix )
                        break;
                    lgotoxy( VTERM, 1, 1 ) ;
                    break;
                }
                case 'G': { /* DG UNIX - Roll Disable */
                    debug(F110,"Data General","Unix Roll Disable",0);
                    if ( debses || !dgunix )
                        break;
                    autoscroll = FALSE ;
                    break;
                }
                case 'H': { /* DG UNIX - Erase Window */
                    debug(F110,"Data General","Unix Erase Window",0);
                    if ( debses || !dgunix )
                        break;
                    clrscreen(VTERM,SP);
                    lgotoxy(VTERM,1,1);       /* and home the cursor */
                    attrib.blinking = FALSE;
                    attrib.underlined = FALSE;
                    attrib.reversed = FALSE;
                    attrib.dim = FALSE;
                    break;
                }
                case 'I': { /* DG UNIX - Blink On */
                    debug(F110,"Data General","Unix Blink On",0);
                    if ( debses || !dgunix )
                        break;
                    attrib.blinking = TRUE ;
                    break;
                }
                case 'J': { /* DG UNIX - Blink Off */
                    debug(F110,"Data General","Unix Blink Off",0);
                    if ( debses || !dgunix )
                        break;
                    attrib.blinking = FALSE ;
                    break;
                }
                }
            } /* P */
            }
            } else {      /* xprint */
                switch ( ch ) {
                case 'F': {
                    ch = dginc() ;
                    switch ( ch ) {
                    case '?': {
                        ch = dginc() ;
                        if ( debses )
                            break;
                        switch ( ch ) {
                        case '2':
                            debug(F110,"Data General","Xprint Off",0);
                            xprint = FALSE ;
                            if ( !uprint && !xprint && !aprint && printon )
                                printeroff();
                            sendchar(ACK);
                            break;
                        default:
                            if (printon && (is_xprint() || is_uprint())) {
                                prtchar(escchar);
                                prtchar('F');
                                prtchar('?');
                                prtchar(ch);
                            }
                        }
                        break;
                    }
                    case 'a':
                        if ( debses )
                            break;
                        debug(F110,"Data General","Xprint Off",0);
                        xprint = FALSE ;
                        if ( !uprint && !xprint && !aprint && printon )
                            printeroff();
                        sendchar(ACK);
                        break;
                    default:
                        if ( !cprint && printon ) {
                            prtchar(escchar);
                            prtchar('F');
                            prtchar(ch);
                        }
                    }
                }
                default:
                    if (printon && (is_xprint() || is_uprint())) {
                        prtchar(escchar);
                        prtchar(ch);
                    }
                }
            }
            escstate = ES_NORMAL ;      /* Done parsing escstate sequence */
        }
    }
    else                /* Handle as a normal character */
    {
        if ( ch < SP ) {
            dgctrl(ch) ;
        }
        else if ( !debses ) {
            wrtch(ch);
            if (printon && (is_xprint() || is_uprint()))
                prtchar(ch);
        }
    }
    VscrnIsDirty(VTERM) ;
}
#endif /* NOTERM */

