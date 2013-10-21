/* C K O T E K . C - Tektronix Emulation */

/*
  Author: Jeffrey E Altman <jaltman@secure-endpoints.com>
            Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of
  New York.  All rights reserved.  This copyright notice must not be removed,
  altered, or obscured.
*/

#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#ifndef NOTERM
#include "ckcker.h"             /* Kermit definitions */
#include "ckcasc.h"             /* ASCII character symbols */
#include "ckcxla.h"             /* Character set translation */
#include "ckcnet.h"             /* Network support */
#include "ckuusr.h"             /* For terminal type definitions, etc. */

#include <ctype.h>              /* Character types */
#include <io.h>                 /* File io function declarations */
#include <process.h>            /* Process-control function declarations */
#include <stdlib.h>             /* Standard library declarations */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/*
   if the following is defined, then Reverse Screen mode is handled not
   by flipping the Vscrn mode but instead by reversing the attribute
*/
#define OLDDECSCNM
#define DECLED

#ifdef NT
#include <windows.h>
#else /* NT */

#ifdef OS2MOUSE
#define INCL_MOU
#endif /* OS2MOUSE */

#define INCL_NOPM
#define INCL_VIO
#define INCL_ERRORS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVIOCTL
#define INCL_WINCLIPBOARD
#include <os2.h>
#undef COMMENT                /* COMMENT is defined in os2.h */
#endif /* NT */

#include "ckocon.h"             /* definitions common to console routines */
#include "ckokey.h"
#include "ckotek.h"

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

int tekmode = FALSE ;

void
settekmode( void )
{
   tekmode = TRUE ;
   debug(F100,"Entering Tek mode","",0);
}

void
resettekmode( void )
{
   tekmode = FALSE ;
   debug(F100,"Exiting Tek mode","",0);
}

void
tekescape( void )
{

}

int
tekinc(void)
{
    extern int cmask, pmask;
    int ch = ttinc(0);
    if ( !xprint ) {
#ifndef NOXFER
        autodown(ch);
#endif /* NOXFER */
        autoexitchk(ch);
    }
    if ( seslog )
        logchar(ch);
    ch = ch & pmask & cmask;
    debugses(ch);
    if (printon && (is_xprint() || is_uprint()))
        prtchar(ch);
    return ch;
}

void
tekctrl( int ch )
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
        /* Cursor Left 8 dots, can be destructive */
        if ( debses )
            break;
        break;
    case HT:
        /* treated as a single space */
        break;
    case LF:
        /* Cursor Down 8 dots */
        if ( debses )
            break;
        break;
    case VT:
        /* Cursor Down 8 dots */
        break;
    case FF:
        /* Erase screen, Home cursor */
        break;
    case CR:
        /* move cursor to column 1 */
        if ( debses )
            break;
        break;
    case SO:
        break;
    case SI:
        break;
    case DLE:
        break;
    case DC1:
        /* XON flow control */
        break;
    case DC2:
        break;
    case DC3:
        /* XOFF flow control */
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
        /* Return to text terminal mode if in sub-Tek mode */
        /* else ignored if regular Tek terminal */
        break;
    case XEM:
        break;
    case SUB:
        /* Same as CAN */
        break;
    case ESC:
        /* Start ESCAPE sequence, cancel any others */
        break;
    case XFS:
        /* Enter point plotting mode */
#ifdef COMMENT
        Draw a dot at the coordinate.  Point plotting
        mode.  Like GS but does not join end points with lines.
        Exit drawing upon reception of CR,LF,RS,US,FS,CAN.
#endif /* COMMENT */
        break;
    case XGS:
        /* Enter line drawing mode */
#ifdef COMMENT
        The first move will be with beam off (a moveto command), subsequent
        coordinates will be reached with the beam on (a drawto command).
        Note: this is also Kermit's Connect mode escape character so beware
        if typing GS by hand; SET ESCAPE to something else before the test.
        Exit drawing upon reception of CR,LF,RS,US,FS,CAN.
#endif /* COMMENT */
        break;
    case XRS:
        /* Enter incremental line drawing mode */
#ifdef COMMENT
        RS space        move with pen up (invisible)
        RS P            move with pen down (write dots)
        RS <letter>
        letter  motion                  letter  motion
          A     right (East)              B     left (West)
          B     right and up (NE)         J     left and down (SW)
          D     up (North)                H     down (South)
          F     left and up (NW)          I     right and down (SE)
        Exit drawing upon reception of CR,LF,RS,US,FS,CAN.
  Example: RS <space> J J J  means move three Tek positions left and down
        (three south west steps) with the pen up (move invisibly).
#endif /* COMMENT */
        break;
    case US:
        /* Enter Tek text mode (leave line/point drawing) */
        break;
    }
}

void
tekascii( int ch )
{
    int i,j,k,n,x,y,z;
    vtattrib attr ;
    viocell blankvcell;

    if (printon && (is_xprint() || is_uprint()))
        prtchar(ch);

    if ( ch < SP )
        tekctrl(ch) ;
    else if ( !debses ) {
        wrtch(ch);
    }
    VscrnIsDirty(VTERM) ;
}
#endif /* NOTERM */

