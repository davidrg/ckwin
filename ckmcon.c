/* Paul Placeway, Ohio State -- added option to flashing cursor, made */
/*  key macros use Pascal strings, so that a NUL (0x00) can be sent */
/* Enhanced by Clayton M. Elwell, Ohio State University 24 Nov 1987 -- */
/*  added insert character */
/* Matthias Aebi, ECOFIN Research and Consulting, Ltd., Oct 1987 -- */
/*  ported to MPW, changed the way keys work */
/* Version 0.8(35) - Jim Noble at Planning Research Corporation, June 1987. */
/* Ported to Megamax native Macintosh C compiler. */
/* From: DPVC@UORDBV.BITNET */
/* DPVC at U of R, Oct 1, add blinking cursor and mouse cursor movement */
/* DPVC at U of R, Sept. 26, fixed book-keeping for scrolling and inserting */
/*  characters and lines */
/* DPVC at U of R, Sept. 25, to fix cursor positioning off the screen, and */
/*  a few other, minor VT100 incompatibilities */
/* DPVC at the University of Rochester, Sept. 9, to add Block Cursor and */
/*  ability to do VT100 graphics characters */
/* By CAM2 and DPVC at the University of Rochester on Sept 6, */
/*  changed bolding from using TextStyle attributes to using a separate bold */
/*  font */
/* By Frank on June 20 - Add parity to all outbound chars using software */
/*  Also, ignore DEL (0177) characters on input. */
/* By Bill on May 29 - Add Key set translation */
/* By WBC3 on Apr 24 - Add ^^, ^@ and ^_.  Also use Pascal strings for */
/*  output in the terminal emulator */
/* By WBC3 on Apr 23 - Add query terminal and be more fastidious about */
/*  ignoring sequences we don't know about */
/* By WBC3 on Apr 22 - Fix tab stops to conform to the rest of the world! */
/* By Bill on Apr 21 - Fix immediate echo problems. */
/*  do less cursor_erase, cursor_draw stuff */

/*
 * FILE ckmcon.c
 *
 * Module of mackermit: contains code for the terminal simulation
 * routine.
 */

#include "ckcdeb.h"

#define	__SEG__	ckmcon
#include <quickdraw.h>
#include <files.h>
#include <events.h>
#include <windows.h>
#include <toolutils.h>
#include <osutils.h>
#include <ctype.h>

#include "ckmdef.h"
#include "ckmasm.h"		/* Assembler code */

#define MAXLIN      24
#define MAXCOL      80
#define LINEHEIGHT  12
#define CHARWIDTH    6
#define TOPMARGIN    3		/* Terminal display constants */
#define bottomMARGIN (LINEHEIGHT * MAXLIN + TOPMARGIN)
#define LEFTMARGIN   3
#define rightMARGIN  (CHARWIDTH * MAXCOL + LEFTMARGIN)
#define LINEADJ      3		/* Amount of char below base line */

/* Font Numbers (UoR Mod) to fix bolding problems */
/* These should be placed in the RESOURCE FORK of the executable */

#define VT100FONT  128		/* VT100 Terminal Font (not-bold) */
#define VT100BOLD  129		/* VT100 Bold Font */

/* Tab settings */

/*
#define NUMTABS 9
short tabstops[NUMTABS] = {8,16,24,32,40,48,56,64,72};
  *//* (UoR) remove old method of tab stops */

/* (UoR) do tapstops via an array: 0 means no tab, 1 means tab at that column */
short tabstops[MAXCOL + 1] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1
};

#define USA_SET  0		/* (UoR) VT100 character set numbers */
#define UK_SET   1
#define GRAF_SET 2

int topmargin = TOPMARGIN,	/* Edges of adjustable window */
    bottommargin = bottomMARGIN,
    textstyle = 0, 
    currentfont = VT100FONT,    /* (UoR) currently active font */
    graphicsinset[2] = {USA_SET, USA_SET}, /* (UoR) current character sets */
    current_set = 0,		/* (UoR) current chosen set */
    doinvert = FALSE,		/* Flag for inverted terminal mode */
    dounder = FALSE,		/* Flag for underlining (PWP) */
    screeninvert = FALSE,	/* (UoR) inverted screen flag */
    insert = FALSE,
    newline = FALSE,		/* (UoR) linefeed mode by default */
    autowrap = TRUE,		/* Autowrap on by default */
    relorigin = FALSE,		/* (UoR) relative origin off */
    autorepeat = TRUE,		/* (UoR) auto repeat flag */
    smoothscroll = FALSE,	/* do smooth scrolling (PWP: or not) */
    transparent = TRUE,		/* do not show control characters */
    blockcursor = TRUE,		/* show block or underline cursor */
    mouse_arrows = FALSE,	/* mouse down in screen does arrow keys */
    visible_bell = FALSE,	/* true if we do blink instead of bell */
    nat_chars = FALSE,		/* half transparent -- show undef. control
				 * chars */
    blinkcursor = TRUE;		/* true if we make the cursor blink */

char *querystring = "\033[?1;2c";	/* Answer we are a VT100 with AVO */
 /* (UoR) used to be VT102 */
char *reportstring = "\033[0n";	/* (UoR) report that we're OK */
char *noprinter = "\033[?13n";	/* (UoR) report no printer */

Rect ScreenRect;

 /*
  * (UoR) don't need scrollrect any more (use scroll_up and scroll_down), use
  * ScreenRect for mouse check
  */

/* Screen book keeping variables */

char scr[MAXLIN][MAXCOL + 1];	/* Characters on the screen */
short nxtlin[MAXLIN], toplin, botlin;	/* Linked list of lines */
int curlin, curcol, abslin;	/* Cursor position */
int savcol, savlin;		/* Cursor save variables */
int savsty, savfnt, savgrf, savmod, savset[2];	/* (UoR) cursor save
						 * variables */
int savund;			/* PWP for saved underlining */
int scrtop, scrbot;		/* Absolute scrolling region bounds */
int cursor_invert = FALSE,	/* (UoR) for flashing cursor */
    cur_drawn = FALSE;
long last_flash = 0;
int oldlin = -1;
int oldcol = 0;			/* (UoR) for last mouse position */

RgnHandle dummyRgn;		/* dummy region for ScrollRect */
				/* Initialized in mac_init */

# define CARETTIME 20		/* (UoR) ticks between flashes */

/* Stuff for escape character processing */

#define CF_OUTC 0		/* Just output the char */
#define CF_SESC 1		/* In a single char escape seq */
#define CF_MESC 2		/* In a multi char '[' escape seq */
#define CF_TOSS 3		/* Toss this char */
#define CF_GRF0 4		/* (UoR) for graphics sequence 0 */
#define CF_GRF1 5		/* (UoR) for graphics sequence 1 */

char prvchr, numone[6], numtwo[6], *numptr;
int num1, num2, charflg = CF_OUTC;
/* extern CSParam controlparam; */
extern unsigned char dopar ();

typedef int (*PFI) ();

/* Terminal function declarations. */

int 
tab (), back_space (), carriage_return (), line_feed (), bell (),
escape_seq (), text_mode (), clear_line (), erase_display (),
cursor_position (), cursor_up (), cursor_down (), cursor_right (),
cursor_left (), cursor_save (), cursor_restore (), set_scroll_region (),
reverse_line_feed (), dummy (), delete_char (), insert_mode (),
end_insert_mode (), insert_line (), delete_line (), query_terminal (),
multi_char (), toss_char (), insert_chars (),

 /* (UoR) for VT100 graphic character set */

graphic_G0 (), graphic_G1 (), control_N (), control_O (),

 /* (UoR) for other VT100 functions */

new_line (), request_report (), set_tab (), clear_tab ();


/* (UoR) constansts that point to the function definitions for arrow keys */
/*  Used by mouse cursor positioning function (Pascal strings) */

# define UPARROW    "\003\033OA"
# define DOWNARROW  "\003\033OB"
# define leftARROW  "\003\033OD"
# define rightARROW "\003\033OC"


/* Terminal control character function command table. */

#define MINSINGCMDS 000
#define MAXSINGCMDS 037

PFI controltable[MAXSINGCMDS - MINSINGCMDS + 1] =
{
    dummy,			/* 0 */
    dummy,			/* 1 */
    dummy,			/* 2 */
    dummy,			/* 3 */
    dummy,			/* 4 */
    dummy,			/* 5 */
    dummy,			/* 6 */
    bell,			/* 7 */
    back_space,			/* 10 */
    tab,			/* 11 */
    line_feed,			/* 12 */
    line_feed,			/* 13 (Vertical tab) */
    line_feed,			/* 14 (Form feed) */
    carriage_return,		/* 15 */
    control_N,			/* 16 (graphic set 1) *//* (UoR) */
    control_O,			/* 17 (graphic set 0) *//* (UoR) */
    dummy,			/* 20 */
    dummy,			/* 21 */
    dummy,			/* 22 */
    dummy,			/* 23 */
    dummy,			/* 24 */
    dummy,			/* 25 */
    dummy,			/* 26 */
    dummy,			/* 27 */
    dummy,			/* 30 */
    dummy,			/* 31 */
    dummy,			/* 32 */
    escape_seq,			/* 33 (Escape) */
    dummy,			/* 34 */
    dummy,			/* 35 */
    dummy,			/* 36 */
    dummy			/* 37 */
};



#define MINSINGESCS 0040
#define MAXSINGESCS 0137

PFI singescapetable[MAXSINGESCS - MINSINGESCS + 1] =
{
    dummy,			/* 40 */
    dummy,			/* 41 */
    dummy,			/* 42 */
    toss_char,			/* 43 '#' */
    dummy,			/* 44 */
    dummy,			/* 45 */
    dummy,			/* 46 */
    dummy,			/* 47 */
    graphic_G0,			/* 50 '(' *//* (UoR) */
    graphic_G1,			/* 51 ')' *//* (UoR) */
    dummy,			/* 52 */
    dummy,			/* 53 */
    dummy,			/* 54 */
    dummy,			/* 55 */
    dummy,			/* 56 */
    dummy,			/* 57 */
    dummy,			/* 60 */
    dummy,			/* 61 */
    dummy,			/* 62 */
    dummy,			/* 63 */
    dummy,			/* 64 */
    dummy,			/* 65 */
    dummy,			/* 66 */
    cursor_save,		/* 67 '7' */
    cursor_restore,		/* 70 '8' */
    dummy,			/* 71 */
    dummy,			/* 72 */
    dummy,			/* 73 */
    dummy,			/* 74 '<' */
    dummy,			/* 75 '=' */
    dummy,			/* 76 '>' */
    dummy,			/* 77 */
    dummy,			/* 100 */
    dummy,			/* 101 */
    dummy,			/* 102 */
    dummy,			/* 103 */
    line_feed,			/* 104 'D' */
    new_line,			/* 105 'E' *//* (UoR) */
    dummy,			/* 106 */
    dummy,			/* 107 */
    set_tab,			/* 110 'H' *//* (UoR) */
    dummy,			/* 111 */
    dummy,			/* 112 */
    dummy,			/* 113 */
    dummy,			/* 114 */
    reverse_line_feed,		/* 115 'M' */
    toss_char,			/* 116 'N' *//* CME */
    toss_char,			/* 117 'O' *//* (UoR) ignore these */
    dummy,			/* 120 */
    dummy,			/* 121 */
    dummy,			/* 122 */
    dummy,			/* 123 */
    dummy,			/* 124 */
    dummy,			/* 125 */
    dummy,			/* 126 */
    dummy,			/* 127 */
    dummy,			/* 130 */
    dummy,			/* 131 */
    query_terminal,		/* 132 'Z' */
    multi_char,			/* 133 '[' */
    dummy,			/* 134 */
    dummy,			/* 135 */
    dummy,			/* 136 */
    dummy			/* 137 */
};




/* Terminal escape sequence function command table */

#define MINMULTESCS 0100
#define MAXMULTESCS 0177

PFI escapetable[MAXMULTESCS - MINMULTESCS + 1] =
{
    insert_chars,		/* 100 *//* CME */
    cursor_up,			/* 101 'A' */
    cursor_down,		/* 102 'B' */
    cursor_right,		/* 103 'C' */
    cursor_left,		/* 104 'D' */
    dummy,			/* 105 */
    dummy,			/* 106 */
    dummy,			/* 107 */
    cursor_position,		/* 110 'H' */
    dummy,			/* 111 */
    erase_display,		/* 112 'J' */
    clear_line,			/* 113 'K' */
    insert_line,		/* 114 'L' */
    delete_line,		/* 115 'M' */
    dummy,			/* 116 */
    dummy,			/* 117 */
    delete_char,		/* 120 'P' */
    dummy,			/* 121 */
    dummy,			/* 122 */
    dummy,			/* 123 */
    dummy,			/* 124 */
    dummy,			/* 125 */
    dummy,			/* 126 */
    dummy,			/* 127 */
    dummy,			/* 130 */
    dummy,			/* 131 */
    dummy,			/* 132 */
    dummy,			/* 133 */
    dummy,			/* 134 */
    dummy,			/* 135 */
    dummy,			/* 136 */
    dummy,			/* 137 */
    dummy,			/* 140 */
    dummy,			/* 141 */
    dummy,			/* 142 */
    query_terminal,		/* 143 'c' */
    dummy,			/* 144 */
    dummy,			/* 145 */
    cursor_position,		/* 146 'f' */
    clear_tab,			/* 147 'g' *//* (UoR) */
    insert_mode,		/* 150 'h' */
    dummy,			/* 151 */
    dummy,			/* 152 */
    dummy,			/* 153 */
    end_insert_mode,		/* 154 'l' */
    text_mode,			/* 155 'm' */
    request_report,		/* 156 'n' *//* (UoR) */
    dummy,			/* 157 */
    dummy,			/* 160 */
    dummy,			/* 161 */
    set_scroll_region,		/* 162 'r' */
    dummy,			/* 163 */
    dummy,			/* 164 */
    dummy,			/* 165 */
    dummy,			/* 166 */
    dummy,			/* 167 */
    dummy,			/* 170 */
    dummy,			/* 171 */
    dummy,			/* 172 */
    dummy,			/* 173 */
    dummy,			/* 174 */
    dummy,			/* 175 */
    dummy,			/* 176 */
    dummy			/* 177 */
};



/****************************************************************************/
/* Connect support routines */
/****************************************************************************/
consetup ()
{
    PenMode (patXor);
    flushio ();			/* Get rid of pending characters */

    init_term ();		/* Set up some terminal variables */
    TextFont (VT100FONT);	/* (UoR) Set initial font to VT100 */
    TextMode (srcXor);		/* (UoR) use XOR mode (for inverse) */
    TextFace (0);		/* PWP: be safe.  We allways stay like this */
    clear_screen ();		/* Clear the screen */
    home_cursor ();		/* Go to the upper left */
    cursor_save ();		/* Save this position */
    cursor_draw ();		/* (UoR) be sure to draw it */
}				/* consetup */



/****************************************************************************/
/* Input and process all the characters pending on the tty line */
/****************************************************************************/
inpchars ()
{
    int rdcnt;

    if ((rdcnt = ttchk ()) == 0)/* How many chars there? */
	return;			/* Ret if 0 */

    cursor_erase ();		/* remove cursor from screen */
    while (rdcnt-- > 0)		/* Output all those characters */
	printit (ttinc (0));
    flushbuf ();		/* Flush any remaining characters */
    cursor_draw ();		/* put it back */
}				/* inpchars */



/****************************************************************************/
/* writeps - write a pascal form string to the serial port.
 *
 */
/****************************************************************************/
writeps (s)
char *s;
{
    long wcnt, w2;
    int err;
    char *s2;

    w2 = wcnt = *s++;		/* get count */

    for (s2 = s; w2 > 0; w2--, s2++)	/* add parity */
	*s2 = dopar (*s2);

    err = FSWrite (outnum, &wcnt, s);	/* write the characters */
    if (err != noErr)
	printerr ("Bad FSWrite in writeps: ", err);

    return;
}				/* writeps */



/****************************************************************************/
/*
 * (UoR)
 *
 * Print a string to the screen (used to echo function and meta strings
 * in duplex mode).
 *
 */
/****************************************************************************/
printps (s)
char *s;
{
    long w2;
    char *s2;

    cursor_erase ();

    w2 = *s++;			/* get count */
    for (s2 = s; w2 > 0; w2--, s2++)
	printit (*s2);		/* print it out, and perform special
				 * functions */

    flushbuf ();

    cursor_draw ();
    return;
}				/* printps */



/****************************************************************************/
/* return the ASCII character which is generated by the keyCode specified */
/* with no modifiers pressed */
/****************************************************************************/
unsigned char
DeModifyChar (keyCode)
short keyCode;
{
    ProcHandle KeyTrans;
    long c;

    if (keyCode > 64)
	KeyTrans = 0x2A2;	/* keypad decode */
    else
	KeyTrans = 0x29E;	/* keyboard decode */

    SaveRegs ();		/* save all registers */
    AllRegs ();

    /* setup regs for procedure call */
    loadD1 ((long) 0);		/* no modifiers */
    loadD2 ((long) keyCode);	/* set the keycode */
    loadA0 (*KeyTrans);		/* load the content of Key1Trans to A0 */

    /* run the translation routine */
    execute ();			/* call the Key1Trans procedure */

    /* move the result from reg D0 to c */
    loadA0 (&c);		/* set destination address */
    pushD0 ();			/* move register D0 to stack */
    poptoA0 ();			/* load the stacktop to (A0) */

    RestoreRegs ();		/* restore all registers */
    AllRegs ();

    return (c);
}				/* DeModifyChar */



unsigned char obuf[2] = {1, 0};	/* single char output buffer */

/****************************************************************************/
/* send a character to the line if it is in ASCII range. Do local echo if */
/* necessary */
/****************************************************************************/
OutputChar (c)
unsigned char c;
{

    /*
     * PWP: NO 7 bit masking!!!  If we do this, then I can't use Emacs, and
     * the European users will be VERY unhappy, 'cause they won't be able to
     * send all of their characters.
     */

#ifdef COMMENT_OUT
    if (c > 127) {		/* reject non-ASCII characters */
	SysBeep (1);
	return;
    }
#endif				/* COMMENT_OUT */

    obuf[1] = c;		/* store character */
    writeps (obuf);		/* and write it out */

    if (duplex != 0) {
	cursor_erase ();	/* remove from screen */
	printit ((char) c);	/* Echo the char to the screen */
	flushbuf ();		/* flush the character */
	cursor_draw ();		/* put it back */
    }
}				/* OutputChar */



extern char keytable[512];	/* the key redefintion flag table */
extern modrec modtable[NUMOFMODS];	/* modifier records */

#define myKeyCodeMask	0x7F00
#define keyModifierMask	0x1F00
#define ctrlCodeMask	0x1F
#define metaOrBits	0x80

#define UnmodMask	0x80		/* action bits */
#define CapsMask	0x40
#define CtrlMask	0x20
#define MetaMask	0x10

/****************************************************************************/
/* Process a character received from the keyboard */
/****************************************************************************/
handle_char (evt)
EventRecord *evt;
{
    short i;
    short len;
    short theCode;
    short modCode;
    short theModBits;
    char flags;
    char tmpstr[256];
    unsigned char c;

    /* (UoR) check for auto repeated keys */
    if ((autorepeat == FALSE) && (evt->what == autoKey))
	return;

    ObscureCursor ();		/* PWP: hide the cursor until next move */

    modCode = evt->modifiers & keyModifierMask;
    theCode = ((evt->message & myKeyCodeMask) >> 8) + (modCode >> 1);
    
    /* check for a special code for this key */
    if (BitTst (&keytable, theCode)) {
	GetMacro (theCode, &flags, tmpstr);	/* get the macrostring */

	if (flags) {		/* check special flags */
	    if (flags & shortBreak) {	/* short break ? */
		sendbreak (5);
		return;
	    }
	    if (flags & longBreak) {	/* long break ? */
		sendbreak (70);
		return;
	    }
	}
	/* send key macro string */

	/*
	 * PWP: note, we DON'T have to convert it to a Pascal string, 'cause
	 * the macros are now stored as Pascal strings
	 */
	writeps (tmpstr);	/* send it to the line */
	if (duplex != 0)
	    printps (tmpstr);	/* echo it locally */
	return;
    }
    for (i = 0; i < NUMOFMODS; i++) {
	/* shift what to look for into high byte */
	theModBits = modtable[i].modbits << 4;
	len = strlen (modtable[i].prefix);

	if ((theModBits || len) &&
	    ((theModBits & modCode) == (theModBits & keyModifierMask))) {
	    /* send prefix if there is one */
	    if (len) {
		/* PWP: these are saved as Pascal strings now */
		BlockMove (modtable[i].prefix, tmpstr, (modtable[i].prefix[0] + 1));
		writeps (tmpstr);	/* send it to the line */
		if (duplex != 0)
		    printps (tmpstr);	/* echo it locally */
	    }

	    /*
	     * get the unmodified ASCII code if the unmodify action bit is
	     * active
	     */
	    if (theModBits & UnmodMask)
		c = DeModifyChar ((evt->message & myKeyCodeMask) >> 8);
	    else
		c = evt->message & charCodeMask;	/* otherwise get the
							 * standard char */

	    /* make an uppercase character if the caps action bit is active */
	    if ((theModBits & CapsMask) && islower (c))
		c = _toupper (c);

	    /* make a control character if the control action bit is active */
	    if (theModBits & CtrlMask)
		c &= ctrlCodeMask;

	    /* PWP: for Meta characters (yes, I use Emacs) */
	    if (theModBits & MetaMask)
		c |= metaOrBits;

	    OutputChar (c);
	    return;
	}			/* if */
    }				/* for */

    /* get the ASCII code and send it */
    OutputChar (evt->message & charCodeMask);
}				/* handle_char */



char outbuf[MAXCOL + 1];
int outcnt = 0, outcol;

/****************************************************************************/
/* flushbuf() -- draw all the buffered characters on the screen */
/****************************************************************************/
flushbuf ()
{
    Rect r;

    if (outcnt == 0)
	return;			/* Nothing to flush */

    /* Erase a hole large enough for outcnt chars */
    makerect (&r, abslin, outcol, 1, outcnt);

    EraseRect (&r);		/* (UoR) Use InvertRect instead of fillRect */
    DrawText (outbuf, 0, outcnt);	/* Output the string */

    if (doinvert)		/* PWP: moved this... */
	InvertRect (&r);

    if (dounder) {		/* PWP: do underlining */
	makertou (&r);		/* make into an underline */
	InvertRect (&r);
    }
    outcnt = 0;			/* Say no more chars to output */
}				/* flushbuf */



/****************************************************************************/
/* save a character in the buffer */
/****************************************************************************/
buf_char (c)
char c;
{
    if (outcnt == 0)
	outcol = curcol;	/* No chars in buffer, init column */
    outbuf[outcnt++] = c;	/* Put in the buffer to output later */
}				/* buf_char */



/****************************************************************************/
/*
 *  Printit:
 *      Draws character and updates buffer
 */
/****************************************************************************/
printit (c)
char c;
{
    PFI funp, lookup ();
    long lnum1, lnum2;

    c &= 0177;

    if (c != 0) {		/* (UoR) ignore null characters */

	switch (charflg) {
	  case CF_OUTC:	/* Just output the char */
	    MDrawChar (c);
	    break;

	  case CF_SESC:	/* In a single char escape seq */
	    charflg = CF_OUTC;	/* Reset flag to simple outputting */
	    if (funp = lookup (c, singescapetable, MINSINGESCS, MAXSINGESCS))
		(*funp) ();	/* Do escape sequence function */
	    break;

	  case CF_GRF0:	/* (UoR) process graphic characters */
	  case CF_GRF1:
	    switch (c) {
	      case 'A':
		graphicsinset[charflg - CF_GRF0] = UK_SET;
		break;
	      case 'B':
	      case '1':
		graphicsinset[charflg - CF_GRF0] = USA_SET;
		break;

	      case '0':
	      case '2':
		graphicsinset[charflg - CF_GRF0] = GRAF_SET;
		break;
	    }
	    charflg = CF_OUTC;	/* Reset flag for next character */
	    break;

	  case CF_MESC:	/* Multichar escape sequence */
	    if (c >= 0x20 && c < 0x40) {	/* Deal with the modifiers */
		if (c >= '<' && c <= '?') {
		    prvchr = c;	/* Handle priv char */
		} else if ((numptr == numone || numptr == numtwo) &&
			 (c == '0' || c == '-' || c == '+')) {
		    /* if at start of sequence */
		    if (c == '-')	/* then we only record leading - */
		    	*numptr++ = c;
		} else if (c >= '0' && c <= '9') {  /* PWP: was also '+' or '-' */
		    *numptr++ = c;	/* Add the char to the num */
		} else if (c == ';') {
		    *numptr = '\0';	/* Terminate the string */
		    numptr = numtwo;	/* Go to next number */
		} else {
		    charflg = CF_OUTC;	/* (UoR) */
		}
	    } else if (c >= 0x40) {		/* End of sequence */
	    /* PWP: according to VTTEST, we ignore control characters here */
		if (funp = lookup (c, escapetable, MINMULTESCS, MAXMULTESCS)) {
		    *numptr = '\0';	/* Terminate the string */
		    StringToNum (numone, &lnum1);	/* Translate the numbers */
		    StringToNum (numtwo, &lnum2);
		    num1 = (int) lnum1;
		    num2 = (int) lnum2;
		    (*funp) ();	/* Do the escape sequence function */
		}
		charflg = CF_OUTC;	/* Back to simple outputting */
	    }
	    break;

	  case CF_TOSS:	/* Ignore this char */
	    charflg = CF_OUTC;	/* Reset flag */
	    break;
	}
    }
}				/* printit */



/****************************************************************************/
/*
 * Routine makerect
 *
 * Make a rectangle in r starting on line lin and column col extending
 * numlin lines and numcol characters.
 *
 */
/****************************************************************************/
makerect (r, lin, col, numlin, numcol)
Rect *r;
int lin;
int col;
int numlin;
int numcol;
{
    r->top = lin * LINEHEIGHT + TOPMARGIN;
    r->left = col * CHARWIDTH + LEFTMARGIN;
    r->bottom = r->top + numlin * LINEHEIGHT;
    r->right = r->left + numcol * CHARWIDTH;
}				/* makerect */

/* Make rect r (made by makerect()) into the right shape
	 for underlining */
makertou (r)
Rect *r;
{
    r->top = r->bottom - 1;
}

/****************************************************************************/
/*
 *   Lookup:
 *      Lookup a given character in the apropriate character table, and
 *      return a pointer to the appropriate function, if it exists.
 */
/****************************************************************************/
PFI
lookup (index, table, min, max)
char index;
PFI table[];
int min;
int max;
{
    if (index > max || index < min)
	return ((PFI) NULL);	/* Don't index out of range */
    return (table[index - min]);
}				/* lookup */



/****************************************************************************/
/*
 *   Flushio:
 *      Initialize some communications constants, and clear screen and
 *      character buffers. */
/****************************************************************************/
flushio ()
{
    int err;

    err = KillIO (-6);
    if (err)
	printerr ("Bad input clear", err);
    err = KillIO (-7);
    if (err)
	printerr ("Bad ouput clear", err);
}				/* flushio */



/****************************************************************************/
/* sendbreak - sends a break across the communictions line.
 *
 * The argument is in units of approximately 0.05 seconds (or 50
 * milliseconds).  To send a break of duration 250 milliseconds the
 * argument would be 5; a break of duration 3.5 seconds would be (umm,
 * lets see now) 70.
 *
 */
/****************************************************************************/
sendbreak (msunit)
{
    long finalticks;

/* delay wants 1/60th units.  We have 3/60 (50 ms.) units, convert */

    msunit = msunit * 3;

    SerSetBrk (outnum);		/* start breaking */
    Delay ((long) msunit, &finalticks);	/* delay */
    SerClrBrk (outnum);		/* stop breaking */
}				/* sendbreak */



/****************************************************************************/
/* draw a characer on the screen (or buffer it) */
/****************************************************************************/
MDrawChar (chr)
char chr;
{
    PFI funp;

    /* If it's a control char, do the apropriate function. */

    if ((chr < ' ') && (transparent)) {	/* Is it a control character */
	flushbuf ();
	if (funp = lookup (chr, controltable, MINSINGCMDS, MAXSINGCMDS)) {
	    if (funp != dummy) {/* PWP */
		(*funp) ();
		return;
	    }
	}
	if (!nat_chars)		/* PWP: half transparent -- show undefd
				 * controls */
	    return;
	if (chr == '\0')	/* PWP: never show nulls */
	    return;
    }
    if (chr < 0177) {		/* Don't do Mac graphic characters */
	switch (graphicsinset[current_set]) {
	  case GRAF_SET:	/* Do VT100 graphics (offset to character in
				 * VT100 font) */
	    if ((chr >= '_') && (chr <= '~'))
		chr += 128;
	    break;

	  case UK_SET:		/* Use pound symbol from VT100 font */
	    if (chr == '#')
		chr = 0375;	/* VT100 pound symbol = 0375 */
	    break;
	}

	if (curcol >= MAXCOL) {	/* Are we about to wrap around? */
	    if (autowrap) {	/* If autowrap indicated wrap */
		flushbuf ();
		if (newline == FALSE)
		    carriage_return ();
		line_feed ();
	    } else {
		flushbuf ();	/* (UoR) make sure last char is shown */
		back_space ();	/* Otherwise just overwrite */
	    }
	}
	if (insert) {		/* Insert mode? */
	    insert_char ();	/* Open hole for char if requested */
	    erase_char ();	/* Erase the old char */
	    DrawChar (chr);
	} else
	    buf_char (chr);	/* Otherwise just buffer the char */

	scr[curlin][curcol++] = chr;
    }
}				/* MDrawChar */



/****************************************************************************/
/*
 *      Control character functions:
 *              Each of the following allow the mac to simulate
 *              the behavior of a terminal when given the proper
 *              control character.
 */
/****************************************************************************/


back_space ()
{
    if (curcol > 0)
	relmove (-1, 0);
}				/* back_space */



erase_char ()
{
    Rect r;

    scr[curlin][curcol] = ' ';	/* Erase char for update */
    makerect (&r, abslin, curcol, 1, 1);	/* One char by one line */

    EraseRect (&r);		/* (UoR) use InvertRect instead of FillRect */
    if (doinvert)
	InvertRect (&r);
}				/* erase_char */



tab ()
{
    int i;

/*    for (i=0; i<NUMTABS; i++)
    {
        if (tabstops[i] > curcol)
        {
            absmove(tabstops[i],abslin);
            return;
        }
     *  }*/
    /* (UoR) remove old method of tabbing */

    /* (UoR) find next tabstop */
    for (i = curcol + 1; (i < MAXCOL) && (tabstops[i] == 0); i++);
    absmove (i, abslin);
}				/* tab */



line_feed ()
{
    if (newline)
	absmove (0, abslin);	/* (UoR) perform newline function */

    if (curlin == scrbot)
	scroll_up (scrtop, curlin);	/* (UoR) scroll lines up */
    else
	relmove (0, 1);
}				/* line_feed */



reverse_line_feed ()
{
    if (curlin == scrtop)
	scroll_down (curlin, scrbot);	/* (UoR) scroll down in region */
    else
	relmove (0, -1);
}				/* reverse_line_feed */



carriage_return ()
{
    if (newline)
	line_feed ();		/* (UoR) perform newline function */
    else
	absmove (0, abslin);
}				/* carriage_return */



new_line ()
{
    carriage_return ();
    line_feed ();
}				/* new_line */



clear_screen ()
{
    register int i;
    Rect r;

    makerect (&r, 0, 0, MAXLIN, MAXCOL);	/* The whole screen */
    EraseRect (&r);

    for (i = 0; i < MAXLIN; i++)
	zeroline (i);		/* Clear up the update records */
}				/* clear_screen */



home_cursor ()
{
    if (relorigin)
	absmove (0, fndabs (scrtop));
    else
	absmove (0, 0);		/* (UoR) correct for relative origin */
}				/* home_cursor */



/****************************************************************************/
/* PWP -- like waitasec(), but don't get any characters.  Used for 
   visable bell. */
/****************************************************************************/
waitnoinput ()
{
    long end_time;

    end_time = TickCount () + 2;/* pause for 1/30th second */
    while (TickCount () < end_time);
}				/* waitnoinput */



bell ()
{
    Rect r;

    if (visible_bell) {
	makerect (&r, 0, 0, MAXLIN, MAXCOL);	/* The whole screen */
	InvertRect (&r);
	waitnoinput ();		/* sleep for a bit (1/30 sec) */
	InvertRect (&r);
    } else {
	SysBeep (3);
    }
}				/* bell */



escape_seq ()
{
    charflg = CF_SESC;		/* Say we are in an escape sequence */
}				/* escape_seq */



graphic_G0 ()			/* (UoR) do VT100 graphic characters */
{
    charflg = CF_GRF0;
}				/* graphic_G0 */



graphic_G1 ()
{
    charflg = CF_GRF1;
}				/* graphic_G1 */



control_N ()
{
    current_set = 1;		/* set to graphics set 1 */
}				/* control_N */



control_O ()
{
    current_set = 0;		/* set to graphics set 0 */
}				/* control_O */



clear_line ()
{
    int i;
    Rect r;

    switch (num1) {
      case 0:			/* Clear:  here to the right */
	makerect (&r, abslin, curcol, 1, MAXCOL - curcol);
	for (i = curcol; i < MAXCOL; i++)
	    scr[curlin][i] = ' ';
	break;

      case 1:			/* Clear:  left to here */
	makerect (&r, abslin, 0, 1, curcol + 1);
	for (i = 0; i <= curcol; i++)
	    scr[curlin][i] = ' ';
	break;

      case 2:			/* Clear:  entire line */
	makerect (&r, abslin, 0, 1, MAXCOL);
	zeroline (curlin);
	break;
    }
    EraseRect (&r);
}				/* clear_line */



erase_display ()
{
    int i;
    Rect r;

    switch (num1) {
      case 0:
	clear_line ();		/* Same num1 causes correct clear */
	makerect (&r, abslin + 1, 0, MAXLIN - abslin - 1, MAXCOL);
				/* (UoR) -1 added */
	EraseRect (&r);
	for (i = abslin + 1; i < MAXLIN; i++)
	    zeroline (fndrel (i));
	break;

      case 1:
	clear_line ();		/* Same num1 causes correct clear */
	makerect (&r, 0, 0, abslin, MAXCOL);
	EraseRect (&r);
	for (i = 0; i < abslin; i++)
	    zeroline (fndrel (i));
	break;

      case 2:
	clear_screen ();
	break;
    }
}				/* erase_display */


/****************************************************************************/
/**** All cursor moves need to check that they don't go beyond the margins */
/****************************************************************************/

cursor_right ()
{
    if (num1 == 0)
	num1 = 1;
    relmove (num1, 0);
}				/* cursor_right */



cursor_left ()
{
    if (num1 == 0)
	num1 = 1;
    relmove (-num1, 0);
}				/* cursor_left */



cursor_up ()
{
    int abstop;			/* (UoR) check that we don't pass scrtop */

    abstop = fndabs (scrtop);
    if (num1 == 0)
	num1 = 1;
    if ((abslin >= abstop) && (abslin - num1 < abstop))
	absmove (curcol, abstop);
    else
	relmove (0, -num1);
}				/* cursor_up */



cursor_down ()
{
    int absbot;			/* (UoR) check that we don't pass scrbot */

    absbot = fndabs (scrbot);
    if (num1 == 0)
	num1 = 1;
    if ((abslin <= absbot) && (abslin + num1 > absbot))
	absmove (curcol, absbot);
    else
	relmove (0, num1);
}				/* cursor_down */



cursor_position ()
{
/*    if (--num1 < 0) num1 = 0;
     *    if (--num2 < 0) num2 = 0;  *//* This is taken care of by absmove */

    if (relorigin)
	absmove (--num2, fndabs (scrtop) + num1 - 1);	/* (UoR) */
    else
	absmove (--num2, --num1);	/* (UoR) moved "--" here from prev
					 * lines */
}				/* cursor_position */



cursor_save ()
{
    savcol = curcol;		/* Save the current line and column */
    savlin = abslin;

    /* savsty = textstyle; *//* (UoR) additions */
    savfnt = currentfont;
    savmod = doinvert;
    savund = dounder;		/* PWP */
    savgrf = current_set;
    savset[0] = graphicsinset[0];
    savset[1] = graphicsinset[1];
}				/* cursor_save */



cursor_restore ()
{
    absmove (savcol, savlin);	/* Move to the old cursor position */

    textstyle = savsty;		/* (UoR) additions */
    currentfont = savfnt;
    doinvert = savmod;
    TextFont (currentfont);
    /* TextFace(textstyle); */
    dounder = savund;		/* PWP */
    current_set = savgrf;
    graphicsinset[0] = savset[0];
    graphicsinset[1] = savset[1];
}				/* cursor_restore */



cursor_rect (line, col, r)
Rect *r;
{
    makerect (r, line, col, 1, 1);	/* Get character rectangle */
    if (blockcursor) {
	/* r->left--;  *//* make r a little wider (PWP: or not) */
	/* r->top--;	 */
    } else
	r->top = r->bottom - 1;
}				/* cursor_rect */



cursor_draw ()
{
    /*    Line(CHARWIDTH,0);*//* Draw cursor */

    Rect r;

    if (cursor_invert == FALSE) {
	cursor_rect (abslin, curcol, &r);
	InvertRect (&r);
    }
    if (oldlin >= 0) {		/* (UoR) replace mouse cursor */
	cursor_rect (oldlin, oldcol, &r);
	PenMode (patXor);
	FrameRect (&r);
	PenMode (patCopy);
    }
    cursor_invert = TRUE;
    cur_drawn = TRUE;
}				/* cursor_draw */



cursor_erase ()
{
    /*    Line(-CHARWIDTH,0);*//* Erase cursor */

    Rect r;

    if (cursor_invert) {
	cursor_rect (abslin, curcol, &r);
	InvertRect (&r);
    }
    if (oldlin >= 0) {		/* (UoR) remove mouse cursor */
	makerect (&r, oldlin, oldcol, 1, 1);
	cursor_rect (oldlin, oldcol, &r);
	PenMode (patXor);
	FrameRect (&r);
	PenMode (patCopy);
    }
    cursor_invert = FALSE;
    cur_drawn = FALSE;
}				/* cursor_erase */



flash_cursor (theWindow)
WindowPtr theWindow;
{
    register long tc;
    Rect r;

    if (theWindow == (WindowPtr) NIL) {
	last_flash = TickCount ();
	return;
    }

#ifdef COMMENT			/* PWP: this eats up gobbs of time */
    if (FrontWindow () != theWindow) {
	last_flash = TickCount ();
	return;
    }
#endif

    tc = TickCount ();
    if (((tc - last_flash) > CARETTIME) ||
	(tc - last_flash) < 0) {
	last_flash = tc;

	if (cur_drawn) {
	    cursor_rect (abslin, curcol, &r);
	    if (blinkcursor) {	/* PWP: only blink if asked for */
		InvertRect (&r);
		if (cursor_invert)
		    cursor_invert = FALSE;
		else
		    cursor_invert = TRUE;
	    } else if (!cursor_invert) {	/* make sure that the cursor
						 * shows up */
		InvertRect (&r);
		cursor_invert = TRUE;
	    }
	}
    }
}				/* flash_cursor */



/****************************************************************************/
/* Bittest returns the setting of an element in a Pascal PACKED ARRAY [0..n]
   OF Boolean such as the KeyMap argument returned by GetKeys(). */
/****************************************************************************/
Boolean
bittest (bitmap, bitnum)
char bitmap[];
int bitnum;
{
    return (0x01 & (bitmap[bitnum / 8] >> (bitnum % 8)));
}				/* bittest */



/****************************************************************************/
/* check to see if the cursor is in the window and is going to send a mouse-
   arrow keys combination */
/****************************************************************************/
check_pointer (theWindow)
WindowPtr theWindow;
{
    Boolean mouse_in_window;
    int newlin;
    int newcol;
    Point MousePt;
    Rect r;

    /*
     * PWP: NOTE!!! since the common case is to do nothing, we do NOT want to
     * have the PenMode() calls outside of the tests. We will be only doing
     * one or the other anyway, so it won't slow us down any.
     */

    GetMouse (&MousePt);
    mouse_in_window = PtInRect (&MousePt, &ScreenRect);

    newlin = (MousePt.v - TOPMARGIN) / LINEHEIGHT;
    newcol = (MousePt.h - LEFTMARGIN) / CHARWIDTH;


    if ((FrontWindow () == theWindow) && (mouse_in_window) &&
	(cur_drawn) && (mouse_arrows) && Button ()) {
	PenMode (patXor);	/* For FrameRect calls */
	if ((oldlin != newlin) || (oldcol != newcol)) {
	    if (oldlin >= 0) {
		cursor_rect (oldlin, oldcol, &r);
		FrameRect (&r);
	    } else
		HideCursor ();

	    cursor_rect (newlin, newcol, &r);
	    FrameRect (&r);

	    oldlin = newlin;
	    oldcol = newcol;
	}
	PenMode (patCopy);	/* reset to normal pen mode */
    }
    if (oldlin >= 0) {		/* optomize: easy tests first */
	if ((FrontWindow () != theWindow) || (!mouse_in_window) ||
	    (!cur_drawn) || (!mouse_arrows) || (!Button ())) {
	    PenMode (patXor);	/* For FrameRect calls */
	    cursor_rect (oldlin, oldcol, &r);
	    FrameRect (&r);

	    oldlin = -1;
	    InitCursor ();	/* show the arrow cursor immediately */
	    /* (reset crsr level) */
	    PenMode (patCopy);	/* reset to normal pen mode */
	}
    }
}				/* check_pointer */


mouse_cursor_move (evt)
EventRecord *evt;
{
    int mouselin;
    int mousecol;
    int tempcol;
    int templin;
    int i;
    Point MousePt;

    if (!mouse_arrows)		/* PWP: make this an option */
	return;

    MousePt = evt->where;
    GlobalToLocal (&MousePt);
    mouselin = (MousePt.v - TOPMARGIN) / LINEHEIGHT;
    mousecol = (MousePt.h - LEFTMARGIN) / CHARWIDTH;
    tempcol = curcol;
    templin = abslin;

    if (mousecol < tempcol)
	for (i = tempcol; i > mousecol; i--) {
	    writeps (leftARROW);
	    waitasec ();
	    /* If tabs are used, we may go too far, so end loop */
	    if (curcol <= mousecol)
		i = mousecol;
	}

    if (mouselin < templin)
	for (i = templin; i > mouselin; i--) {
	    writeps (UPARROW);
	    waitasec ();
	}

    else if (mouselin > templin)
	for (i = templin; i < mouselin; i++) {
	    writeps (DOWNARROW);
	    waitasec ();
	}

    if (abslin == mouselin)
	tempcol = curcol;	/* for short lines */

    if (tempcol < mousecol)
	for (i = tempcol; i < mousecol; i++) {
	    writeps (rightARROW);
	    waitasec ();
	    /* If tabs are used, we may go too far, so end loop */
	    if (curcol >= mousecol)
		i = mousecol;
	}
}				/* mouse_cursor_move */



/****************************************************************************/
/* (UoR) get any characters, and pause for a while */
/****************************************************************************/
waitasec ()
{
    long end_time;

    end_time = TickCount () + 2;/* pause for 1/30th second */
    while (TickCount () < end_time);
    inpchars ();
}				/* waitasec */



set_scroll_region ()
{
    if (--num1 < 0)
	num1 = 0;		/* Make top of line (prev line) */
    if (num2 == 0)
	num2 = 24;		/* Zero means entire screen */

    if (num1 < num2 - 1) {	/* (UoR) make sure region is legal */
	topmargin = (num1 * LINEHEIGHT) + TOPMARGIN;
	bottommargin = (num2 * LINEHEIGHT) + TOPMARGIN;

	scrtop = fndrel (num1);
	scrbot = fndrel (num2 - 1);

	home_cursor ();		/* We're supposed to home it! */
    }
}				/* set_scroll_region */



/****************************************************************************/
/* aka Select Graphic Rendition */
/****************************************************************************/
text_mode ()
{
another:
    switch (num1) {
      case 0:			/* primary rendition */
	doinvert = FALSE;
	TextFont (VT100FONT);	/* (Uor) Use plain font */
	currentfont = VT100FONT;
	dounder = FALSE;	/* textstyle=0; TextFace(0); */
	break;

      case 1:			/* bold or increased intensity */
	TextFont (VT100BOLD);	/* use bold font instead */
	currentfont = VT100BOLD;
	break;

      case 2:			/* faint or decreased intensity or secondary
				 * color */
      case 3:			/* italic */
      case 4:			/* underscore */
	/* textstyle+=underline; *//* use = not += (avoid roll-over) */
	/* textstyle = underline;    /* (UoR) */
	/* TextFace(textstyle); */
	dounder = TRUE;
	break;

      case 5:			/* slow blink (< 150/sec); (UoR) blink is
				 * inverse */
      case 6:			/* fast blink (>= 150/sec) */
      case 7:			/* reverse image */
	doinvert = TRUE;
	break;

      case 21:
      case 22:
	TextFont (VT100FONT);	/* reset to plain font */
	currentfont = VT100FONT;
	break;

      case 24:
	/* TextFace(0);        /* just reset to plain style */
	/* since bolding is done via a separate font */
	/* textstyle = 0; */
	dounder = FALSE;
	break;

      case 25:
      case 27:
	doinvert = FALSE;
	break;
    }
    if (num2 != 0) {		/* Check for funny VAXTPU syntax. */
	num1 = num2;
	num2 = 0;
	goto another;
    }
}				/* text_mode */



/****************************************************************************/
/*
 * (UoR)
 *
 * Insert and Delete lines (replacements for originals, which have
 *   which have been deleted)
 *
 */
/****************************************************************************/
insert_line ()
{
    int i, absbot;

    absbot = fndabs (scrbot);

    if ((abslin >= fndabs (scrtop)) && (abslin <= absbot)) {
	if (num1 == 0)
	    num1 = 1;
	if (num1 > absbot - abslin + 1)
	    num1 = absbot - abslin + 1;

	for (i = 0; i < num1; i++)
	    scroll_down (curlin, scrbot);
    }
}				/* insert_line */



delete_line ()
{
    int i, absbot;

    absbot = fndabs (scrbot);

    if ((abslin >= fndabs (scrtop)) && (abslin <= absbot)) {
	if (num1 == 0)
	    num1 = 1;
	if (num1 > absbot - abslin + 1)
	    num1 = absbot - abslin + 1;

	for (i = 0; i < num1; i++)
	    scroll_up (curlin, scrbot);
    }
}				/* delete_line */



delete_char ()
{
    int i;
    Rect r;

    if (num1 == 0)
	num1 = 1;

    makerect (&r, abslin, curcol, 1, MAXCOL - curcol);

    if (num1 > MAXCOL - curcol - 1)
	num1 = MAXCOL - curcol - 1;

    /* Scroll them out */
    ScrollRect (&r, -CHARWIDTH * num1, 0, dummyRgn);

    /* Shift them down *//* (UoR) used to assign using abscol */
    for (i = curcol; i < MAXCOL - num1; i++)
	scr[curlin][i] = scr[curlin][i + num1];

    /* Fill in holes with spaces */
    while (i < MAXCOL)
	scr[curlin][i++] = ' ';
}				/* delete_char */



/****************************************************************************/
/* CME */
/****************************************************************************/
insert_chars ()
{
    int i;
    Rect r;

    if (num1 == 0)
	num1 = 1;

    makerect (&r, abslin, curcol, 1, MAXCOL - curcol);

    if (num1 > MAXCOL - curcol - 1)
	num1 = MAXCOL - curcol - 1;

    /* Scroll them out */
    ScrollRect (&r, CHARWIDTH * num1, 0, dummyRgn);

    /* Shift them up *//* (UoR) used to assign using abscol */
    for (i = MAXCOL - 1; i >= curcol + num1; i--)
	scr[curlin][i] = scr[curlin][i - num1];

    /* Fill in holes with spaces */
    while (i > curcol)
	scr[curlin][--i] = ' ';
}				/* delete_char */



insert_char ()
{
    int i;
    Rect r;

    makerect (&r, abslin, curcol, 1, MAXCOL - curcol);
    ScrollRect (&r, CHARWIDTH, 0, dummyRgn);

    /* Shift em up *//* (UoR) used to assign ...[i-1]=...[i] */
    /* (UoR) used to assign using abscol */

    for (i = MAXCOL - 1; i > curcol; i--)
	scr[curlin][i] = scr[curlin][i - 1];

    scr[curlin][curcol] = ' ';
}				/* insert_char */



insert_mode ()
{
    if (prvchr == '?')
	set_mode ();		/* (UoR) do some of these calls */

    switch (num1) {
      case 20:
	newline = TRUE;
	break;

      case 4:
	insert = TRUE;
	break;
    }
}				/* insert_mode */



end_insert_mode ()
{
    if (prvchr == '?')
	reset_mode ();		/* (UoR) do some of these calls */

    switch (num1) {
      case 20:
	newline = FALSE;
	break;

      case 4:
	insert = FALSE;
	break;
    }
}				/* end_insert_mode */



invert_term ()
{
    num1 = 5;
    if (screeninvert)
	reset_mode ();
    else
	set_mode ();
}				/* invert_term */



set_mode ()
{
    Rect r;

    switch (num1) {
      case 5:
	if (screeninvert == FALSE) {
	    BackPat (qd.black);	/* (UoR) use black background */
	    makerect (&r, 0, 0, MAXLIN, MAXCOL);
	    InvertRect (&r);
	    screeninvert = TRUE;
	}
	break;

      case 6:
	relorigin = TRUE;
	home_cursor ();
	break;

      case 7:
	autowrap = TRUE;
	break;

      case 8:
	autorepeat = TRUE;
	break;
    }
}				/* set_mode */



reset_mode ()
{
    Rect r;

    switch (num1) {
      case 5:
	if (screeninvert) {
	    BackPat (qd.white);
	    makerect (&r, 0, 0, MAXLIN, MAXCOL);
	    InvertRect (&r);
	    screeninvert = FALSE;
	}
	break;

      case 6:
	relorigin = FALSE;
	home_cursor ();
	break;

      case 7:
	autowrap = FALSE;
	break;

      case 8:
	autorepeat = FALSE;
	break;
    }
}				/* reset_mode */



set_tab ()
{
    tabstops[curcol] = 1;
}				/* set_tab */



clear_tab ()
{
    int i;

    switch (num1) {
      case 0:
	tabstops[curcol] = 0;
	break;

      case 3:
	for (i = 0; i < MAXCOL; i++)
	    tabstops[i] = 0;
	break;
    }
}				/* clear_tab */



/****************************************************************************/
/* (UoR) use for respoding to information requests */
/****************************************************************************/
writereply (s)
char *s;
{
    long wrcnt, w2;
    int err;
    char *s2;

    w2 = wrcnt = strlen (s);	/* How long is the string? */
    for (s2 = s; w2 > 0; w2--, s2++)	/* add parity */
	*s2 = dopar (*s2);
    err = FSWrite (outnum, &wrcnt, s);	/* Respond to the query */
    if (err)
	printerr ("Bad Writeout:", err);
}				/* writereply */



query_terminal ()
{
    writereply (querystring);
}				/* query_terminal */



/****************************************************************************/
/* (UoR) reports */
/****************************************************************************/
request_report ()
{
    switch (num1) {
      case 5:			/* (UoR) report that we're OK */
	writereply (reportstring);
	break;

      case 6:			/* (UoR) reprt the cursor position */
	position_report ();
	break;

      case 15:			/* (UoR) report printer status */
	if (prvchr == '?')
	    writereply (noprinter);
	break;
    }
}				/* request_report */



position_report ()
{
    int i;
    char buf[9];
    char *report;

    i = 0;
    buf[i++] = '\033';
    buf[i++] = '[';
    if (abslin > 9)
	buf[i++] = '0' + (abslin + 1) / 10;
    buf[i++] = '0' + (abslin + 1) % 10;
    buf[i++] = ';';
    if (curcol > 9)
	buf[i++] = '0' + (curcol + 1) / 10;
    buf[i++] = '0' + (curcol + 1) % 10;
    buf[i++] = 'R';
    buf[i] = '\0';
    report = buf;
    writereply (report);
}				/* position_report */



/****************************************************************************/
/* does nothing. */
/****************************************************************************/
dummy ()
{
}				/* dummy */



multi_char ()
{
    numone[0] = numtwo[0] = '0';/* Initialize the numbers to zero */
    numone[1] = numtwo[1] = '\0';
    numptr = numone;		/* Place to put the next number */
    prvchr = '\0';		/* No priv char yet */
    charflg = CF_MESC;		/* Say we are in a ESC [ swequence */
}				/* multi_char */



toss_char ()
{
    charflg = CF_TOSS;
}				/* toss_char */



/****************************************************************************/
/* Routine zeroline
 *
 * Zero (set to space) all the characters in relative line lin.
 *
 */
/****************************************************************************/
zeroline (lin)
int lin;
{
    register int i;
    Rect r;

    for (i = 0; i < MAXCOL; i++)
	scr[lin][i] = ' ';
}				/* zeroline */



/****************************************************************************/
/* Move a relative number of lines and chars.  Both can be negative. */
/****************************************************************************/
relmove (hor, ver)
{
    absmove (curcol + hor, abslin + ver);	/* (UoR) use absmove, which
						 * checks */
    /* for cursor moving off screen */
}				/* relmove */



/****************************************************************************/
/* Move to absolute position hor char and ver line. */
/****************************************************************************/
absmove (hor, ver)
{
    if (hor > MAXCOL - 1)
	hor = MAXCOL - 1;	/* (UoR) make sure its on the screen */
    if (hor < 0)
	hor = 0;
    if (ver > MAXLIN - 1)
	ver = MAXLIN - 1;
    if (ver < 0)
	ver = 0;
    if (relorigin) {
	if (ver < fndabs (scrtop))
	    ver = fndabs (scrtop);
	if (ver > fndabs (scrbot))
	    ver = fndabs (scrbot);
    }
    MoveTo (hor * CHARWIDTH + LEFTMARGIN, (ver + 1) * LINEHEIGHT + TOPMARGIN - LINEADJ);
    curcol = hor;
    abslin = ver;
    curlin = fndrel (ver);
}				/* absmove */



/****************************************************************************/
/* dump the whole screen to the session log file */
/****************************************************************************/
scrtolog ()
{
    int lin, i;

    lin = toplin;
    for (i = 0; i < MAXLIN; i++) {
	slog (scr[lin], MAXCOL);/* write this line to session log */
	lin = nxtlin[lin];
    }
}				/* scrtolog */



/****************************************************************************/
/*
 * (UoR)
 *
 * Scroll lines within the scroll region upwards from line tlin
 * to line blin (lines are assumed to be in the region)
 *
 */
/****************************************************************************/
scroll_up (tlin, blin)
int tlin;
int blin;
{
    register int i, now;
    int newtop;
    int abstop;
    Rect r;
    GrafPtr currWindow;

    abstop = fndabs (tlin);
    makerect (&r, abstop, 0, fndabs (blin) - abstop + 1, MAXCOL);

    ObscureCursor ();

    /* compensate update region for scrolling */
    GetPort (&currWindow);
    OffsetRgn ((WindowPeek) currWindow->updateRgn, 0, -LINEHEIGHT);

    /* do the scrolling */
    if (smoothscroll) {
	for (i = 1; i <= LINEHEIGHT; i += 2) {
	    /* PWP: wait for a vertical reblank (in a sneaky way) */
	    now = TickCount ();
	    while (TickCount () == now)
		 /* wait... */ ;
	    ScrollRect (&r, 0, -2, dummyRgn);
	}
    } else {
	ScrollRect (&r, 0, -LINEHEIGHT, dummyRgn);
    }

    if (tlin == blin) {		/* if only one line, just clear it */
	zeroline (blin);
	return;
    }
    newtop = nxtlin[tlin];	/* new top line */

    if (tlin == scrtop)
	scrtop = newtop;	/* reset scrtop, if needed */
    if (tlin == toplin)
	toplin = newtop;	/* reset toplin, if needed */
    else
	nxtlin[fndprv (tlin)] = newtop;	/* else de-link tlin line */

    nxtlin[tlin] = nxtlin[blin];/* link tlin in after blin */
    nxtlin[blin] = tlin;

    if (blin == scrbot) {	/* reset pointers to bottom of regions */
	if (blin == botlin)
	    botlin = tlin;
	scrbot = tlin;
    }
    if (seslog)			/* if logging is active then */
	slog (scr[tlin], MAXCOL);	/* write line to session log */
    zeroline (tlin);		/* clear the line */
    curlin = fndrel (abslin);
}				/* scroll_up */



/****************************************************************************/
/* (UoR) Scroll lines in scroll region down from line tlin to 	*/
/* line blin (lines are assumed to be within the scroll region)	*/
/****************************************************************************/
scroll_down (tlin, blin)
int tlin;
int blin;
{
    int i;
    int abstop;
    int newbot;
    Rect r;
    GrafPtr currWindow;

    abstop = fndabs (tlin);
    makerect (&r, abstop, 0, fndabs (blin) - abstop + 1, MAXCOL);

    ObscureCursor ();

    /* compensate update region for scrolling */
    GetPort (&currWindow);
    OffsetRgn ((WindowPeek) currWindow->updateRgn, 0, LINEHEIGHT);

    /* do the scrolling */

    if (smoothscroll)
	for (i = 1; i <= LINEHEIGHT; i += 2)
	    ScrollRect (&r, 0, 2, dummyRgn);
    else
	ScrollRect (&r, 0, LINEHEIGHT, dummyRgn);

    if (tlin == blin) {		/* if only one line, just clear it */
	zeroline (tlin);
	return;
    }
    newbot = fndprv (blin);	/* new bottom line */

    if (tlin == scrtop)
	scrtop = blin;		/* reset scrtop, if needed */
    if (tlin == toplin)
	toplin = blin;		/* reset toplin, if needed */
    else
	nxtlin[fndprv (tlin)] = blin;	/* else de-link top line */

    nxtlin[newbot] = nxtlin[blin];	/* link blin above tlin */
    nxtlin[blin] = tlin;

    if (blin == scrbot) {	/* reset bottom of region pointers */
	if (blin == botlin)
	    botlin = newbot;
	scrbot = newbot;
    }
    zeroline (blin);
    curlin = fndrel (abslin);
}				/* scroll_down */



/****************************************************************************/
/*
 * Find the relative line number given the absolute one.
 *
 */
/****************************************************************************/
fndrel (linum)
int linum;
{
    register int i, lin;

    lin = toplin;
    for (i = 0; i < linum; i++)
	lin = nxtlin[lin];
    return (lin);
}				/* fndrel */



/****************************************************************************/
/*
 * Find the absolute line number given the relative one.
 *
 */
/****************************************************************************/
fndabs (linum)
int linum;
{
    int i, lin;

    lin = toplin;
    i = 0;
    while (lin != linum) {
	i++;
	lin = nxtlin[lin];
    }
    return (i);
}				/* fndabs */



/****************************************************************************/
/*
 * Find the previous relative line number from relative line linum.
 *
 */
/****************************************************************************/
fndprv (linum)
int linum;
{
    int lin;

    lin = toplin;
    while (nxtlin[lin] != linum)
	lin = nxtlin[lin];
    return (lin);
}				/* fndprv */



/****************************************************************************/
/* redraw the terminal screen (we got a redraw event) */
/****************************************************************************/
term_redraw ()
{
    int i, lin;
    Rect r;

    makerect (&r, 0, 0, MAXLIN, MAXCOL);    /* PWP: clear the screen first */
    EraseRect (&r);

    lin = toplin;
    for (i = 0; i < MAXLIN; i++) {
	MoveTo (LEFTMARGIN, (i + 1) * LINEHEIGHT + TOPMARGIN - LINEADJ);
	DrawString (scr[lin]);
#ifdef COMMENT
	if (screeninvert) {	/* (UoR) refresh screen in invert mode */
	    makerect (&r, i, 0, 1, MAXCOL);
	    InvertRect (&r);	/* PWP: was Erase, but that does no good */
	}
#endif
	lin = nxtlin[lin];
    }

    MoveTo (curcol * CHARWIDTH + LEFTMARGIN,
	    (abslin + 1) * LINEHEIGHT + TOPMARGIN - LINEADJ);

    if (cur_drawn && cursor_invert) {	/* (UoR) only if cursor is showing */
	cursor_invert = FALSE;	/* (UoR) make sure we draw it */
	cursor_draw ();		/* redraw cursor */
	last_flash = TickCount ();	/* (UoR) reset timer */
    }
}				/* term_redraw */



/****************************************************************************/
/* initalize the terminal emulator. */
/****************************************************************************/
init_term ()
{
    int i;

    for (i = 0; i < MAXLIN; i++) {
	nxtlin[i] = i + 1;	/* Tie together the linked list */
	scr[i][MAXCOL] = '\0';	/* Terminate the lines as strings */
    }
    toplin = 0;			/* Initialize the top and bottom ptr */
    botlin = MAXLIN - 1;
    scrtop = toplin;		/* Scrolling region equals all */
    scrbot = botlin;
    nxtlin[botlin] = -1;	/* Indicate this is the end */
    makerect (&ScreenRect, 0, 0, MAXLIN, MAXCOL);
    /* (UoR) full screen rectangle */
}				/* init_term */
