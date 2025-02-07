/*  C K O A D S . C  --  ADDS Regent Emulation  */

/*
  Author: David Goodwin <david@zx.net.nz>

  Copyright (C) 2024 David Goodwin
  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

/*
 * Currently only the Regent 20 and Regent 25 are implemented.
 *
 * Stubs for the Regent 200 are present, but not yet implemented. Many of this
 * terminals unique features (compared to the Regent 25) may be difficult to
 * implement. WYSE terminal emulation has protected/unprotected fields, but
 * I don't think Kermit 95 implements something like the Regent 200s Page Mode
 * for any terminal type, or the ability to transmit a filled out form.
 *
 * If support for ADDS ViewPoint terminals is ever implemented, the Wyse WY-60
 * supports switching to an ADDS ViewPoint A2 and VP-60 emulation mode - see
 * around line 4869 of ckowys.c
 */


#include "ckcdeb.h"
#ifndef NOTERM
#include "ckcker.h"     /* For logchar, autodown, autoexitchk */
#include "ckcasc.h"
#include "ckcuni.h"     /* for utf8_to_ucs2 */
#include "ckocon.h"     /* For is_xprint, is_uprint, prtchar, wrtch,
 *                         VscrnIsDirty, ES_GOTESC, VTERM, debugses,
 *                         cursorleft*/
#include "ckuusr.h"
#include "ckoads.h"
#ifdef NETCONN
#ifdef TCPSOCKET
#include "ckcnet.h"
extern int network, nettype, ttnproto, u_binary;
#endif /* TCPSOCKET */
#endif /* NETCONN */

extern int tt_type, tt_type_mode, tt_status[VNUM];
extern int keylock;
extern int printon, aprint, uprint, xprint, cprint, seslog ;
extern int escstate, debses;
extern int wherex[], wherey[];
extern int ttpush;
extern int autoscroll;
int store_ctrl = FALSE;
extern bool crm;

/* This is a direct copy from ckoadm.c, which is also identical to what's in
 * ckohzl.c and probably a bunch of others. Probably should be refactored out
 * to a common function */
int
addsinc(void) {
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
addsctrl( int ch ) {
    if ( !xprint ) {
        switch (ch) {
            case SOH: { /* Send cursor home */
                int h;
                if (debses)
                    break;

                /*
                 * The manual says the home position varies depending on
                 * the auto-scroll switch setting.
                 *
                 * If auto-scroll is ON, the home position is the bottom
                 * left corner.
                 *
                 * If auto-scroll is OFF, the home position is the top
                 * left corner.
                 */

                if (autoscroll) {
                    h = VscrnGetHeight(VTERM) - (tt_status[VTERM] ? 1 : 0);
                    lgotoxy(VTERM, 1, h);
                } else {
                    lgotoxy(VTERM,1,1);
                }
                break;
            }
            case STX:
                if ( debses )
                    break;
                keylock = FALSE;
                break;

            case ETX:
                debug(F111, "ADDS Regent", "unused Ctrl", ch);
                break;

            case EOT:
                if ( debses )
                    break;

                /* TODO: Regent 200 - The manual doesn't say EOT locks the
                 * keyboard - instead it uses EM. The Regent 200 might use EOT
                 * for something else, or not use it at all.*/

                keylock = TRUE;
                break;

            case ENQ:
                debug(F111, "ADDS Regent", "unused Ctrl", ch);
                break;

            case ACK:   /* Cursor Right */
                if ( debses )
                    break;
                cursorright(0);       /* TODO: Check this wraps to the beginning of the next line */
                break;

            case BEL:   /* Terminal bell */
                if ( debses )
                    break;
                bleep(BP_BEL);
                break;

            case BS:    /* Cursor Left */
                if ( debses )
                    break;
                cursorleft(0);
                break;

            case HT:
                /* TODO: Regent 200: Tab
                 * Move to the first position in the next unprotected field */
                break;

            case LF:    /* Cursor Down */
                if ( debses )
                    break;
                cursordown(0);

                break;

            case VT: {    /* Move cursor to vertical position */
                int line;
                line = addsinc();
                if (debses)
                    break;

                line &= 0x1F;

                lgotoxy(VTERM, wherex[VTERM], line);
                break;
            }
            case FF:    /* Clear screen and home cursor */
                if ( debses )
                    break;
                clrscreen( VTERM, SP ) ;
                lgotoxy(VTERM,1,1);       /* and home the cursor */
                break;

            case CK_CR:
                if ( debses )
                    break;
                wrtch((char) CK_CR);
                break;

            case SO:
            case SI:
                debug(F111,"ADDS Regent","unused Ctrl",ch);
                break;

            case DLE: {  /* Move cursor to horizontal position */
                int col, group;
                col = addsinc();
                if (debses)
                    break;

                /* Only care about the lower 7 bits */
                col &= 0x7F;

                /* The three most significant are a group number */
                group = col >> 4;

                /* And the remaining is the position in the group, BCD */
                col &= 0xF;

                if (col < 10) {
                    col += group * 10;
                    lgotoxy(VTERM, col, wherey[VTERM]);
                }
                break;
            }
            case DC1:
                debug(F111,"ADDS Regent","unused Ctrl",ch);
                break;

            case DC2:   /* AUX port ON */
                debug(F110, "ADDS Regent", "AUX Port ON", NULL);
                /* TODO: All incoming data until we see a DC4 should be sent to
                 * the printer (transparent print?) */
                break;

            case DC3:
                debug(F111,"ADDS Regent","unused Ctrl",ch);
                break;

            case DC4:   /* AUX port OFF */
                debug(F110, "ADDS Regent", "AUX Port OFF", NULL);
                /* TODO: The DC4 should be sent to the printer, then we should */
                break;

            case NAK:   /* Cursor Back */
                if ( debses )
                    break;
                cursorleft(0);

            case SYN:
            case ETB:
            case CAN:
                debug(F111,"ADDS Regent","unused Ctrl",ch);
                break;
            case ESC:
                debug(F111,"ADDS Regent","ESC",ch);
                if ( debses )
                    break;
                escstate = ES_GOTESC;
                break;

            case XEM:
                if (ISREGENT200(tt_type_mode)) {
                    if ( debses )
                        break;
                    keylock = TRUE;
                }
                break;

            case SUB:   /* Cursor Up */
                if ( debses )
                    break;
                cursorup(0);
                break;

            case XFS:
            case XGS:
            case XRS:
            case US:
            default:
                debug(F111,"ADDS Regent","unused Ctrl",ch);
        }
    } /* xprint */
    else {
        switch ( ch ) {
            case ESC:
                debug(F111,"ADDS Regent","ESC",ch);
                if ( debses )
                    break;
                escstate = ES_GOTESC ;
                break;
            default:
                break; /* There are no other print commands */
        }
    }
}

void
addsascii( int ch ) {
    if (printon && (is_xprint() || is_uprint())) {
        /*
         * TODO: We should probably try to avoid sending the ESC,4 (disable
         *  transparent print) to the printer somehow.
         */
        prtchar(ch);
    }

    if ( escstate == ES_GOTESC ) /* Process character as part of an escstate sequence */
    {
        escstate = ES_ESCSEQ ;
        if (!xprint) {
            switch (ch) {
                case 'G':
                    if (ISREGENT200(tt_type_mode)) {
                        /* TODO: Erase Variable
                         *   replace all unprotected data on the screen with
                         *   spaces, and homes the cursor
                         */
                    }
                    break;
                case 'K':   /* Erase to end of current line */
                    if (debses)
                        break;

                    /* TODO: Regent 200: If in a formatted screen, erases to the
                     * end of the current field
                     */

                    clreol_escape(VTERM, SP);
                    break;
                case 'k':   /* Erase to the end of the screen */
                    if (debses)
                        break;
                    clreoscr_escape(VTERM, SP);
                    break;
                case 'R':
                    /* TODO: Regent 200
                     * Switch to Form Generation Mode.
                     */
                    break;
                case 's':
                    /* TODO: Regent 200
                     * Resets the terminal to Conversational Mode aborting any
                     * active buffered transmission or print local function and
                     * replies with a status message. The screen is not affected.
                     */
                    break;
                case 'X':
                    /* TODO: Regent 200 - Print Local
                     * Conversational, message and page modes
                     * print all variable and print-only data. Attributes and
                     * protected data are output as spaces.
                     */
                    break;
                case 'x':
                    /* TODO: Regent 200 - Print Local
                     * print all data, both protected and variable. In Forms mode,
                     * all data including attribute information is sent.
                     */
                    break;
                case 'Y': {
                    int col = 0, line = 0;
                    line = addsinc();
#ifdef NETCONN
#ifdef TCPSOCKET
                    if ( network && IS_TELNET() && !TELOPT_U(TELOPT_BINARY) && col == CK_CR ) {
                    /* Handle TELNET CR-NUL or CR-LF if necessary */
                    int dummy = ttinc(0);
                    debug(F111,"ADDS Regent 25","Addr cursor in page found CR",dummy);
                    if ( dummy != NUL )
                        ttpush = dummy;
                }
#endif /* TCPSOCKET */
#endif /* NETCONN */
                    col = addsinc();
                    if (debses)
                        break;

                    /* SP == 1, ! == 2, " == 3, etc */
                    line -= US;
                    col -= US;

                    debug(F111, "ADDS Regent 25", "Go to column", col);
                    debug(F111, "ADDS Regent 25", "Go to line", line);

                    lgotoxy(VTERM, col, line);


                    break;
                }
                case 'Z':   /* Store control character */
                    if (debses)
                        break;
                    store_ctrl = TRUE;
                    crm = TRUE;
                    break;
                case '0':
                    /* TODO: Regent 200 - Back Tab
                     * Move cursor to the first position of the previous unprotected
                     * field */
                    break;
                case '1':
                    /* TODO: Regent 200 - switch to line drawing character set
                     * Receiving any valid control code exits.
                     * See page 4-18 for the available symbols
                     */
                    break;
                case '2':
                    /* TODO: Regent 200 - exit line drawing character set
                     * This should be effectively a no-op as any control code
                     * should exit line drawing mode.
                     */
                    break;
                case '3':   /* Enable print transparent */
                    if (debses)
                        break;
                    xprint = TRUE;
                    if (!printon)
                        printeron();
                    break;
#ifdef COMMENT
                    /* This is handled down the bottom of the function */
                case '4':   /* Disable print transparent */
                    if (debses)
                        break;
                    xprint = FALSE;
                    if (!cprint && !uprint && !xprint && !aprint && printon)
                        printeroff();
                    break;
#endif /* COMMENT */
                case '5':   /* Keyboard Lock */
                    if (debses)
                        break;
                    keylock = TRUE;
                    break;
                case '6':   /* Keyboard unlock */
                    if (debses)
                        break;
                    keylock = FALSE;
                    break;
                case NUL: {
                    /* TODO: Regent 200 - Field Attribute follows
                     * (form generation mode only)
                     * One byte will follow indicating the field characteristic */
                    int attr;
                    attr = addsinc();
                    if (debses)
                        break;
                    /* TODO: apply the attribute */
                    break;
                }
                case DC1:
                    /* TODO: Regent 200: Transmit */
                    break;
                case ENQ:
                    /* TODO: Regent 200: Transmit status
                     * Ctrl+X should also transmit it.
                     * Byte 10 is the data under the cursor - it should only
                     * be transmitted if send-data is enabled, otherwise it
                     * this field should be sent as SP.
                     * Auto-line-feed can come from the Kermit 95 setting.
                     * */
                    break;
            }
        }
        else { /* xprint */
            switch (ch) {
                case '4':   /* Disable print transparent */
                    if (debses)
                        break;
                    xprint = FALSE;
                    if (!cprint && !uprint && !xprint && !aprint && printon)
                        printeroff();
                    break;
            }
        }
        escstate = ES_NORMAL ;
    }
    else {            /* Handle as a normal character */
        if ( ch < SP && !store_ctrl) {
            addsctrl(ch);
        } else if ( !debses && ch != DEL && !xprint ) {
            wrtch(ch);

            if (store_ctrl) {
                crm = FALSE;
                store_ctrl = FALSE;
            }
        }
    }
    VscrnIsDirty(VTERM) ;
}

#endif /* NOTERM */