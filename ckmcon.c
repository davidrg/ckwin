/* John A. Oberschelp for Emory University -- vt102 printer support 22 May 1989 */
/*                    Emory contact is Peter W. Day, ospwd@emoryu1.cc.emory.edu */ 
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
 * Module of mackermit: contains code for the terminal emulation
 * routine.  PWP: This file contains the stuff to deal with parsing for
 * a vt??? terminal.  For Macintosh screen handling things, see
 * ckmco2.c.
 */

#include "ckcdeb.h"

#define	__SEG__	ckmcon
#include <quickdraw.h>
#include <controls.h>
#include <files.h>
#include <events.h>
#include <windows.h>
#include <dialogs.h>
#include <menus.h>
#include <toolutils.h>
#include <osutils.h>
#include <ctype.h>

#include "ckmdef.h"
#include "ckmasm.h"		/* Assembler code */
#include "ckmres.h"		/* kermit resources */

#include "ckmcon.h"		/* defines, etc. for terminal emulator */

RgnHandle dummyRgn;		/* dummy region for ScrollRect */

/* Tab settings */

/* (UoR) do tapstops via an array: 0 means no tab, 1 means tab at that column */
/* (PWP) Tabbing bug fixed by Eamonn McManus <emcmanus@csvax1.tcd.ie> */
short tabstops[MAXCOL + 1] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
       0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
       0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
       0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 
       0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1
};

extern int textstyle;		/* (UoR) current style */

int screensize = INIT_SCREENSIZE,	/* variable number of lines on screen */
    topmargin,			/* Edges of adjustable window */
    bottommargin,
    graphicsinset[4] = {ASCII_SET, ASCII_SET, LAT1_SET, LAT1_SET}, /* (UoR) current character sets */
    Gl_set = 0,			/* (UoR) current chosen set */
    Gr_set = 1,			/* (PWP) current chosen RH set */
    old_Gl_set = -1;		/* (PWP) set to come back to after single shift GL */
    ch_protect = FALSE,		/* Flag for protected characters (PWP) */
    screeninvert = FALSE,	/* (UoR) inverted screen flag */
    insert = FALSE,
    newline = FALSE,		/* (UoR) linefeed mode by default */
    autowrap = TRUE,		/* Autowrap on by default */
    relorigin = FALSE,		/* (UoR) relative origin off */
    autorepeat = TRUE,		/* (UoR) auto repeat flag */
    appl_mode = FALSE,		/* (PWP) keypad application mode */
    curskey_mode = FALSE,	/* (PWP) cursor key application mode */
    smoothscroll = FALSE,	/* do smooth scrolling (PWP: or not) */
    transparent = TRUE,		/* do not show control characters */
    blockcursor = TRUE,		/* show block or underline cursor */
    cursor_shown = TRUE,	/* (PWP) show the cursor */
    mouse_arrows = FALSE,	/* mouse down in screen does arrow keys */
    visible_bell = FALSE,	/* true if we do blink instead of bell */
    eightbit_disp = FALSE,	/* do 8 bit wide to the screen */
    nat_char_mode = FALSE,	/* true if we are doing 7 bit national
    				   character sets rather than ISO 8859 */
    nat_set = 0;		/* national char. set used */
    blinkcursor = TRUE;		/* true if we make the cursor blink */

char *querystring = ANS_VT100AVO;	/* Answer we are a VT100 with AVO */
 /* (UoR) used to be VT102 */
char *reportstring = "\033[0n";	/* (UoR) report that we're OK */
char *noprinter = "\033[?13n";	/* (UoR) report no printer */

extern Boolean have_128roms;	/* true if we are a Plus or better */

/* Screen book keeping variables */
/*
 * (PWP) Note that in order to support scroll back, we do things a bit
 * differently than before.  There is no linked list of lines (it doesn't
 * take all that much time to just copy the pointers around), and the
 * indexing in scr is done BACKWARDS (0 is the bottom line) since if we
 * shrink the window, we want to see the bottom most part.
 *
 * Doing this makes the handling of scrolling regions a bit tricky, but
 * it was so allready so this isn't much worse.
 */
 
ucharptr *scr, *scr_attrs;	    /* virtual screen, attributes pointer */
ucharptr real_scr[MAX_SCREENSIZE];   /* The real screen, including scrollback */
ucharptr real_attrs[MAX_SCREENSIZE]; /* the attributes of each character */

extern int display_topline;	/* top line actually displayed */
extern int display_totlines;	/* number of real lines in screen + scrollback */
int curlin, curcol;		/* Cursor position */
int savcol, savlin;		/* Cursor save variables */
int savsty, savfnt, savGl, savGr,
    savmod, savset[4];		/* (UoR) cursor save variables */
int savund;			/* PWP for saved underlining */
int scrtop, scrbot;		/* Absolute scrolling region bounds */

/*****************************************/
/* Stuff for escape character processing */

#define CF_OUTC	0		/* Just output the char */
#define CF_ESC	1		/* In a single char escape seq */
#define CF_CSI	2		/* In a multi char '[' escape seq */
#define CF_TOSS	3		/* Toss this char */
#define CF_GS_0	4   /* (UoR) for graphics sequence 0, allways 94 char set */
#define CF_GS_1	5		/* (UoR) GS 1, 94 char. set */
#define CF_GS_2	6		/* (PWP) GS 2, 94 char. set */
#define CF_GS_3	7		/* (PWP) GS 3, 94 char. set */
#define CF_T_ST	8		/* (PWP) Toss chars till ST */
#define CF_DCS	9		/* (PWP) In a device control string sequence */

int charflg = CF_OUTC;		/* state variable */

char intermedbuf[NUMBUFSIZ], *intermedptr;	/* intermed. char buffer */

char paramarg[MAX_ARGCOUNT][NUMBUFSIZ], *argptr;	/* (non-num) parameter buffer */
int numarg[MAX_ARGCOUNT], argcount;	/* numeric argument buffer */

char numbuf[NUMBUFSIZ], *numptr;   /* buffer for collecting numbers into numarg */


/*****************************************/

/* extern CSParam controlparam; */
extern unsigned char dopar ();

typedef void (*PFV) ();

/* Terminal function declarations. */

void 
tab (), back_space (), carriage_return (), line_feed (), bell (),

csi_seq (), dcs_seq(), toss_char (), toss_till_st(), norm_char(),
escape_seq (), string_term(),

vt320_mode(), vt52_mode(),

text_mode (), clear_line (), erase_display (),
cursor_position (), cursor_up (), cursor_down (), cursor_right (),
cursor_left (), cursor_save (), cursor_restore (), set_scroll_region (),
reverse_line_feed (), dummy (), delete_char (), insert_mode (),
end_insert_mode (), insert_line (), delete_line (), query_terminal (),
insert_char (), insert_chars (),

erase_char(), cursor_h_pos(), home_cursor(),

/* (PWP) stuff for line width and height */
line_dblh_top(), line_dblh_bot(), line_singw(), line_dblw(),

printer_control (),	/*JAO*/

 /* (UoR) for VT100 graphic character set */

graphic_G0 (), graphic_G1 (), graphic_G2 (), graphic_G3 (),
control_N (), control_O (), single_shift_2(), single_shift_3(),
lock_shift_2(), lock_shift_3(), lock_shift_3r(), lock_shift_2r(),
lock_shift_1r(),

set_appl (), reset_appl (), set_compat(),

set_charattr(), start_selected(), end_selected(),

/* PWP: random support routines */
set_mode (), reset_mode(), 
set_heath_mode (), reset_heath_mode(), 
position_report(), zeroline(),
relmove(), absmove(),

 /* (UoR) for other VT100 functions */

new_line (), request_report (), set_tab (), clear_tab ();

extern	int		to_printer;		/*JAO*/
extern	int		to_screen;		/*JAO*/
extern	int		printer_is_on_line_num;	/*JAO*/
extern	Handle	hPrintBuffer;			/*JAO*/
extern	long	lPrintBufferSize;		/*JAO*/
extern	long	lPrintBufferChars;		/*JAO*/
extern	long	lPrintBufferAt;			/*JAO*/

extern	DialogPtr	bufferingDialog;	/*JAO*/
extern	DialogPtr	overflowingDialog;	/*JAO*/

extern	MenuHandle menus[];	/* handle on our menus */  /*JAO*/

extern WindowPtr terminalWindow;	/* the terminal window */

#ifdef COMMENT	/* (PWP) this is done with a case statement below */

/* Terminal control character function command table. */

#define MINSINGCMDS 000
#define MAXSINGCMDS 037

PFV controltable[MAXSINGCMDS - MINSINGCMDS + 1] =
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
#endif /* COMMENT */


#define MIN_ESC		0060
#define MAX_ESC		0177

PFV esc_table[MAX_ESC - MIN_ESC + 1] =
{
    dummy,			/* 60 '0' */
    dummy,			/* 61 '1' */
    dummy,			/* 62 '2' */
    line_dblh_top,		/* 63 '3' */
    line_dblh_bot,		/* 64 '4' */
    line_singw,			/* 65 '5' */
    line_dblw,			/* 66 '6' */
    cursor_save,		/* 67 '7' */
    cursor_restore,		/* 70 '8' */
    dummy,			/* 71 '9' */
    dummy,			/* 72 */
    dummy,			/* 73 */
    dummy,			/* 74 '<' */
    set_appl,			/* 75 '=' */
    reset_appl,			/* 76 '>' */
    dummy,			/* 77 */
    dummy,			/* 100 */
    dummy,			/* 101 */
    dummy,			/* 102 */
    dummy,			/* 103 */
    line_feed,			/* 104 'D' */
    new_line,			/* 105 'E' *//* (UoR) */
    start_selected,		/* 106 'F' */
    end_selected,		/* 107 'G' */
    set_tab,			/* 110 'H' *//* (UoR) */
    dummy,			/* 111 */
    dummy,			/* 112 */
    dummy,			/* 113 */
    dummy,			/* 114 */
    reverse_line_feed,		/* 115 'M' */
    single_shift_2,		/* 116 'N' */
    single_shift_3,		/* 117 'O' */
    dcs_seq,			/* 120 'P' */
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
    csi_seq,			/* 133 '[' */
    string_term,		/* 134 '\' */
    toss_till_st,		/* 135 ']' */
    toss_till_st,		/* 136 '^' */
    toss_till_st,		/* 137 '_' */
    dummy,			/* 140 */
    dummy,			/* 141 */
    dummy,			/* 142 */
    term_reset,			/* 143 'c' */
    dummy,			/* 144 */
    dummy,			/* 145 */
    dummy,			/* 146 'f' */
    dummy,			/* 147 'g' */
    dummy,			/* 150 'h' */
    dummy,			/* 151 'i' */
    dummy,			/* 152 */
    dummy,			/* 153 */
    dummy,			/* 154 'l' */
    dummy,			/* 155 'm' */
    lock_shift_2,		/* 156 'n' */
    lock_shift_3,		/* 157 */
    dummy,			/* 160 'p' */
    dummy,			/* 161 'q' */
    dummy,			/* 162 'r' */
    dummy,			/* 163 */
    dummy,			/* 164 */
    dummy,			/* 165 */
    dummy,			/* 166 */
    dummy,			/* 167 */
    dummy,			/* 170 */
    dummy,			/* 171 */
    dummy,			/* 172 */
    dummy,			/* 173 */
    lock_shift_3r,		/* 174 '|' */
    lock_shift_2r,		/* 175 '}' */
    lock_shift_1r,		/* 176 '~' */
    dummy			/* 177 */
};




/* Terminal escape sequence function command table */

#define MIN_CSI 0100
#define MAX_CSI 0177

PFV csi_table[MAX_CSI - MIN_CSI + 1] =
{
    insert_chars,		/* 100 *//* CME */
    cursor_up,			/* 101 'A' */
    cursor_down,		/* 102 'B' */
    cursor_right,		/* 103 'C' */
    cursor_left,		/* 104 'D' */
    dummy,			/* 105 */
    cursor_h_pos,		/* 106 'F' */
    dummy,			/* 107 */
    cursor_position,		/* 110 'H' (PWP) */
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
    printer_control,		/* 151 'i' *//*JAO*/
    dummy,			/* 152 */
    dummy,			/* 153 */
    end_insert_mode,		/* 154 'l' */
    text_mode,			/* 155 'm' */
    request_report,		/* 156 'n' *//* (UoR) */
    dummy,			/* 157 */
    set_compat,			/* 160 'p' (PWP) */
    set_charattr,		/* 161 'q' (PWP) */
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

#ifdef COMMENT		/* not done yet */

#define MINVT52ESCS 0040
#define MAXVT52ESCS 0137

PFV vt52table[MAXVT52ESCS - MINVT52ESCS + 1] =
{
    dummy,			/* 40 */
    dummy,			/* 41 */
    dummy,			/* 42 */
    dummy,			/* 43 '#' */
    dummy,			/* 44 */
    dummy,			/* 45 */
    dummy,			/* 46 */
    dummy,			/* 47 */
    dummy,			/* 50 '(' */
    dummy,			/* 51 ')' */
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
    vt320_mode,			/* 74 '<' */
    set_appl,			/* 75 '=' */
    reset_appl,			/* 76 '>' */
    dummy,			/* 77 */
    dummy,			/* 100 */
    cursor_up,			/* 101 'A' */
    cursor_down,		/* 102 'B' */
    cursor_right,		/* 103 'C' */
    cursor_left,		/* 104 'D' */
    clear_screen,		/* 105 'E' */
    h19_graph_mode,		/* 106 */
    end_h19graph_mode,		/* 107 */
    home_cursor,		/* 110 'H' */
    reverse_line_feed,		/* 111 'I' */
    clear_eop,			/* 112 */
    clear_eol,			/* 113 */
    insert_line,		/* 114 */
    delete_line,		/* 115 'M' */
    delete_char,		/* 116 'N' */
    end_insert_mode,		/* 117 'O' */
    dummy,			/* 120 */
    dummy,			/* 121 */
    dummy,			/* 122 */
    dummy,			/* 123 */
    dummy,			/* 124 */
    dummy,			/* 125 */
    print_cur_line,		/* 126 'V' */
    start_printing,		/* 127 'W' */
    end_printing,		/* 130 'X' */
    h19_cursor_position,	/* 131 'Y' */
    query_terminal,		/* 132 'Z' */
    csi_seq,			/* 133 '[' */
    dummy,			/* 134 */
    dummy,			/* 135 */
    dummy,			/* 136 */
    dummy			/* 137 */
};

#endif /* COMMENT */

static unsigned char char_map[128];	/* holds the current translation tbl */

static unsigned char nat_remaps[13][12] = {
/* USA (ASCII) proper */
  {  '#',  '@',  '[', '\\',  ']',  '^',  '_',  '`',  '{',  '|',  '}',  '~' },
/* UK */
  { 0xA3,  '@',  '[', '\\',  ']',  '^',  '_',  '`',  '{',  '|',  '}',  '~' },
/* Dutch */
  { 0xA3, 0xBE, 0xFF, 0xBD,  '|',  '^',  '_',  '`', 0xA8,  'f', 0xBC, 0xB4 },
/* Finnish */
  {  '#',  '@', 0xC4, 0xD6, 0xC5, 0xDC,  '_', 0xE9, 0xE4, 0xF6, 0xE5, 0xFC },
/* French */
  { 0xA3, 0xE0, 0xB0, 0xE7, 0xA7,  '^',  '_',  '`', 0xE9, 0xF9, 0xE8, 0xA8 },
/* French Canadian */
  {  '#', 0xE0, 0xE2, 0xE7, 0xEA, 0xEE,  '_', 0xF4, 0xE9, 0xF9, 0xE8, 0xFB },
/* German */
  {  '#', 0xA7, 0xC4, 0xD6, 0xDC,  '^',  '_',  '`', 0xE4, 0xF6, 0xFC, 0xDF },
/* Italian */
  { 0xA3, 0xA7, 0xB0, 0xE7, 0xE9,  '^',  '_', 0xF9, 0xE0, 0xF2, 0xE8, 0xEC },
/* Norwegian/Danish */
  {  '#',  '@', 0xC6, 0xD8, 0xC5,  '^',  '_',  '`', 0xE6, 0xF8, 0xE5,  '~' },
/* Portuguese */
  {  '#',  '@', 0xC3, 0xC7, 0xD5,  '^',  '_',  '`', 0xE3, 0xE7, 0xF5,  '~' },
/* Spanish */
  { 0xA3, 0xA7, 0xA1, 0xD1, 0xBF,  '^',  '_',  '`',  '`', 0xB0, 0xF1, 0xE7 },
/* Swedish */
  {  '#', 0xC9, 0xC4, 0xD6, 0xC5, 0xDC,  '_', 0xE9, 0xE4, 0xF6, 0xE5, 0xFC },
/* Swiss */
  { 0xF9, 0xE0, 0xE9, 0xE7, 0xEA, 0xEE, 0xE8, 0xF4, 0xE4, 0xF6, 0xFC, 0xFB }
};


static struct {
    unsigned char fnum;
    unsigned char coffset;
    unsigned char lbound;
    unsigned char hbound;
} vt_to_fmap[] = {
	{0,	0,	' ',	127},	/* US ASCII */
	{0,	0,	0,	0},	/* unassigned */
	{0,	32	'`',	127},	/* DEC VT100 graphics */
	{1,	0	' ',	127},	/* DEC VT300 technical font */
	{0,	128	' ',	127},	/* DEC International font (almost 8859/1 */
	{0,	128	' ',	128},	/* ISO Latin 1 */
	{2,	0	' ',	128},	/* ISO Latin 2 */
	{2,	128	' ',	128},	/* ISO Latin 3 */
	{3,	0	' ',	128},	/* ISO Latin 4 */
	{4,	0	' ',	128},	/* ISO Latin/Cyrilic */
	{4,	128	' ',	128},	/* ISO Latin/Arabic */
	{5,	0	' ',	128},	/* ISO Latin/Greek */
	{5,	128	' ',	128},	/* ISO Latin/Hebrew */
	{3,	128	' ',	128}	/* ISO Latin 5 */
};


/****************************************************************************/
/* Input and process all the characters pending on the tty line */
/****************************************************************************/
inpchars ()
{
    register int rdcnt;
    register char *cp, *maxp;
    static char buf[128];
    
if (curlin < 0 || curlin >= screensize)
    fatal("inpchars(): curlin out of bounds:", curlin);
if (curcol < 0 || curcol > MAXCOL)	/* PWP: can be == when autowraping */
    fatal("inpchars(): curcol out of bounds:", curcol);
if (display_topline < (screensize - MAX_SCREENSIZE) || display_topline > 0)
    fatal("inpchars(): display_topline out of bounds:", display_topline);


    if ((rdcnt = ttinm(buf, sizeof(buf))) > 0) {
	cursor_erase ();		/* remove cursor from screen */
	screen_to_bottom ();	/* slide the visible region to active area */
	
    	maxp = buf + rdcnt;
	if (parity)
	    for (cp = buf; cp < maxp; cp++)  /* Output all those characters */
		printit(*cp & 0177);		/* strip off parity bits */
	else
	    for (cp = buf; cp < maxp; cp++)  /* Output all those characters */
		printit(*cp);

	if (!ttchk()) {
	    flushbuf ();		/* Flush any remaining characters */
	    cursor_draw ();		/* put it back */
	    update_vscroll();
	}
    }
}				/* inpchars */


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
/*
 *   Lookup:
 *      Lookup a given character in the apropriate character table, and
 *      return a pointer to the appropriate function, if it exists.
 */
/****************************************************************************/
PFV
lookup (index, table, min, max)
unsigned char index;
PFV table[];
int min;
int max;
{
    if (index > max || index < min)
	return ((PFV) NULL);	/* Don't index out of range */
    return (table[index - min]);
}				/* lookup */

/* tie off the current numeric argument */

void
end_numarg()
{
    long lnum = 0;	/* can't be register */

    if (argcount < MAX_ARGCOUNT) {
	if (argptr > paramarg[argcount] + NUMBUFSIZ - 1)
	    argptr = paramarg[argcount] + NUMBUFSIZ - 1;	/* bounds */
	*argptr = '\0';			/* tie off paramter argument */

	if (numptr > numbuf + NUMBUFSIZ - 1)
	    numptr = numbuf + NUMBUFSIZ - 1;	/* bounds */
	*numptr = '\0';		/* tie off number string */
	if (numptr > numbuf) {	/* if we had a number collected */
	    StringToNum (numbuf, &lnum);	/* Translate the numbers */
	    if (lnum < 0) lnum = 0;
	    if (lnum > 9999) lnum = 9999;	/* ANSI says between 0 and 9999 */
	} else {
	    lnum = 0;
	}
	numarg[argcount++] = lnum;
    }
    argptr = paramarg[argcount];	/* Go to next number */
    numptr = numbuf;			/* reset buffer */
}


/****************************************************************************/
/*
 *  Printit:
 *      Draws character and updates buffer
 */
/****************************************************************************/
printit (c)
register unsigned char c;
{
    register PFV funp;
    PFV lookup ();
    int char_set = ASCII_SET;

    if (!eightbit_disp)
	c &= 0177;

    if (c != 0) {		/* (UoR) ignore null characters */

	/* (PWP) Acording to vttest, vt100 series terminals will
	   do control characters in the middle of an escape sequence.
	   ick!. */
	/* If it's a control char, do the apropriate function. */

	if ((c & 0x60) == 0) {	/* Is it a control or meta-control character? */
	    if (transparent) {
		flushbuf ();
		switch (c) {		/* PWP: faster way to go about all this */
		  case 015:		/* CR */
		    if (!newline) {
			absmove (0, curlin);
			return;
		    }
		    /* (UoR) perform newline function */
		    /* PWP: by falling through into next cases */
	    
		  case 012:		/* Line Feed */
		  case 013:		/* Vertical Tab */
		  case 014:		/* Form Feed */
		  case 0x84:		/* Index */
		  case 0x85:		/* Next Line */
		    /* PWP: inline version of line_feed(); */
		    if (to_printer) {				/*JAO*/
			(*hPrintBuffer)[lPrintBufferAt++] = 13;
			if (lPrintBufferAt == lPrintBufferSize)
			    lPrintBufferAt = 0L;
			lPrintBufferChars++;
			if (lPrintBufferChars == lPrintBufferSize) {
			    overflowingDialog = GetNewDialog(OVERFLOWINGBOXID, NILPTR, (WindowPtr) - 1);
			    DrawDialog(overflowingDialog);
			}
		    }
		    
		    if (!to_screen) return;

		    /* (PWP) A better place to log the session */
		    if (seslog)			/* if logging is active then */
			slog (scr[curlin], MAXCOL);	/* write line to session log */

		    if ((newline && (c != 0x84)) || (c == 0x85))
			absmove (0, curlin);	/* (UoR) perform newline function */

		    if (curlin == scrbot) {
			scroll_screen (scrtop, curlin, -1);	/* (UoR) scroll lines up */
		    } else {
			if (curlin >= scrtop)	/* (PWP) if within scrolling region */
			    relmove (0, 1);
		    }
		    return;
	    
		  case 07:
		    bell();
		    return;
	    
		  case 010:
		    back_space();
		    return;
	    
		  case 011:
		    tab();
		    return;
	    
		  case 016:
		    control_N();	/* 16 (graphic set 1) *//* (UoR) */
		    return;
	    
		  case 017:
		    control_O();	/* 17 (graphic set 0) *//* (UoR) */
		    return;
	    
	          case 005:		/* 5 (send answerback message) */
		  case 021:		/* DC1 (XON) */
		  case 023:		/* DC3 (XOFF) */
		    return;		/* we ignore this */
		    
		  case 030:		/* CAN (^X: cancel current esc sequence */
		  case 032:		/* SUB (^Z: treated as CAN) */
		    norm_char();	/* back to normal input mode */
		    return;
		    
		  case 033:
		    escape_seq();	/* 33 (Escape) */
		    return;
	    
		  case 0:
		    return;		/* never draw NULs */

		  case 0x88:		/* Set hors. tab */
		    set_tab();
		    return;
		    
		  case 0x8d:		/* rev. index (scrolls) */
		    reverse_line_feed();
		    return;
		    
		  case 0x8e:		/* single shift G2 to GL */
		    single_shift_2();
		    return;
		    
		  case 0x8f:		/* single shift G3 to GL */
		    single_shift_3();
		    return;
		    
		  case 0x90:		/* device control string intro */
		    dcs_seq();
		    return;
		    
		  case 0x9b:		/* Control Seq. intro (like ESC [ ) */
		    csi_seq();
		    return;
		    
		  case 0x9c:		/* String Terminator (ST) */
		    norm_char();	/* Reset flag for next character */
		    return;
		    
		  case 0x9d:		/* Operating System Command (through ST) */
		  case 0x9e:		/* Privacy Message (through ST) */
		  case 0x9f:		/* Applications Prog. Command (through ST) */
		    toss_till_st();	/* toss them till ST */
		    return;
		    
		  default:
		    return;
		}
	    } else {
		MDrawChar(c);
		flushbuf ();
		if (c == '\012')
		    new_line();
		return;
	    }
	}	/* end if control */

	switch (charflg) {
	  case CF_OUTC:	/* Just output the char */
	    MDrawChar (c);
	    break;

	  case CF_ESC:	/* In a single char escape seq */
if ((intermedptr < intermedbuf) || (intermedptr > &intermedbuf[NUMBUFSIZ - 1])) {
    printerr ("intermedptr out of range:", intermedptr);
    intermedptr = intermedbuf;
}
/*
 * (PWP) Quoting from the DEC VT300 manual:
 *
 *	ESC	I		    F
 *	1/11	2/0 to 2/15	    3/0 to 7/14
 *		(Zero or more	    (One character)
 *		characters)
 */
	    /* (PWP) A hack for now, until I rework the char set handling */
    	    if (c == '(')
		graphic_G0();
	    else if (c == ')' || c == '-')
		graphic_G1();
	    else if (c == '*' || c == '.')
		graphic_G2();
	    else if (c == '+' || c == '/')
		graphic_G3();
	    else 
	    
 	    if (c >= 0x20 && c <= 0x2F) {
		if (intermedptr < &intermedbuf[NUMBUFSIZ - 1])
		    *intermedptr++ = c;	/* Add the char to the num */
	    } else if (c >= 0x30) {
		charflg = CF_OUTC;	/* Reset flag to simple outputting */
		*intermedptr = '\0';
		if (funp = lookup (c, esc_table, MIN_ESC, MAX_ESC))
		    (*funp) ();	/* Do escape sequence function */
	    }
	    break;

	  case CF_GS_0:	/* (UoR) process graphic characters */
	  case CF_GS_1:
	  case CF_GS_2:	/* PWP: for vt200 mode */
	  case CF_GS_3:
	    if (c >= 0x20 && c < 0x30) {	/* Deal with the modifiers */
		if (intermedptr < &intermedbuf[NUMBUFSIZ - 1])
		    *intermedptr++ = c;	/* Add the char to the num */
	    } else {
		*intermedptr = '\0';
		set_char_set(c);	/* (this uses charflg to select which set) */    
		charflg = CF_OUTC;	/* Reset flag for next character */
	    }
	    break;

	  case CF_CSI:	/* Multichar escape sequence */
	  case CF_DCS:	/* device control string sequence */
if ((intermedptr < intermedbuf) || (intermedptr > &intermedbuf[NUMBUFSIZ - 1])) {
    printerr ("intermedptr out of range:", intermedptr);
    intermedptr = intermedbuf;
}
if ((argptr < paramarg[argcount]) || (argptr > &paramarg[argcount][NUMBUFSIZ - 1])) {
    printerr ("argptr out of range:", argptr);
    argptr = paramarg[argcount];
}
if ((numptr < numbuf) || (numptr > &numbuf[NUMBUFSIZ - 1])) {
    printerr ("numptr out of range:", numptr);
    numptr = numbuf;
}
/*
 * (PWP) Also quoting from the DEC VT300 manual (orignal NOTE: in italics):
 *
 *	CSI	P...P		I...I		F
 *	ESC [	3/0 to 3/15	2/0 to 2/15	4/0 to 7/14
 *
 *	NOTE: All parameters muyst be positive decimal integers.
 *	Do not use a decimal point in a parameter -- the termial will
 *	ignore the command.
 *
 *	If the first character in a parameter string is the ? (3/15)
 *	character, it indicates that DEC private parameters follow.
 *	The terminal interprets private parameters according to ANSI X3.64
 *	and ISO 6429.
 */
	    if (c >= 0x30 && c < 0x40) {	/* Deal with the modifiers */
 		if ((c == '0') && (numptr == numbuf)) {
		    /* ignore the leading zero */
		} else if (c >= '0' && c <= '9') {  /* PWP: was also '+' or '-' */
		    if (numptr < &numbuf[NUMBUFSIZ - 1])
			*numptr++ = c;	/* Add the char to the num */
		} else if (c == ';') {
		    end_numarg();
		} else {
		    if (argptr < &paramarg[argcount][NUMBUFSIZ - 1])
			*argptr++ = c;	/* Add the char to the parameter list */
		}
	    } else if (c >= 0x20 && c < 0x30) {		/* Intermediate chars */
		/* (PWP) intermeadiate characters go in the intermedbuf[] */
		
		end_numarg();	/* tie off numeric argument */

		if (intermedptr < &intermedbuf[NUMBUFSIZ - 1])
		    *intermedptr++ = c;	/* Add the char to the num */
		    
	    } else if (c >= 0x40) {		/* End of sequence */
		charflg = CF_OUTC;	/* Back to simple outputting */
		if (funp = lookup (c, csi_table, MIN_CSI, MAX_CSI)) {
		    if (intermedptr == intermedbuf)	/* if we didn't just do this */
			end_numarg();	/* tie off numeric argument */
		    *intermedptr = '\0';	/* tie off intermediate */
		    (*funp) ();	/* Do the escape sequence function */
		}
	    }
	    break;

	  case CF_TOSS:	/* Ignore this char */
	    charflg = CF_OUTC;	/* Reset flag */
	    break;
	    
	  case CF_T_ST:		/* (PWP) toss till String Terminator */
	    break;
	}
    }
}				/* printit */

set_char_set(c)
unsigned char c;
{
    int ninetysix = 0;		/* are we talking about a 96 char. set */
    int set = charflg - CF_GS_0;	/* which slot are we talking about? */
    
    if (set > 3) {
	set -= 4;
	ninetysix = 1;
    }
    
    if (!nat_char_mode) {	/* if doing 8859 international sets */
        if (!ninetysix) {	/* 94 character set */
	    switch(c) {
	      case '1':		/* ALT ROM set (we claim ASCII) */
		graphicsinset[set] = ASCII_SET;
		break;

	      case '5':
		if (intermedbuf[0] == '%')		/* DEC supplimental graphic */
		    graphicsinset[set] = GRAF_SET;	/* for now */
		break;

	      case '0':
	      case '2':
		graphicsinset[set] = GRAF_SET;
		break;

	      case '>':
		graphicsinset[set] = TECH_SET;
		break;

	      case 'A':
		graphicsinset[set] = ASCII_SET;
	        break;

	      case '<':		/* DEC user-prefered supplemental set */
		graphicsinset[set] = GRAF_SET;
		break;

	      case 'B':		/* Allways ASCII half of an ISO set */
		graphicsinset[set] = ASCII_SET;
	        break;

	    } /* end switch(c) */
	} else {		/* 96 character set */
	    switch(c) {
	      case '1':		/* ALT ROM set (we claim ASCII) */
		graphicsinset[set] = ASCII_SET;
		break;

	      case '<':		/* DEC user-prefered supplemental set */
		graphicsinset[set] = GRAF_SET;
		break;

	      case 'A':
		graphicsinset[set] = LAT1_SET;
		break;

#ifdef COMMENT
	      case 'B':
		graphicsinset[set] = LAT2_SET;
		break;

	      case 'C':
		graphicsinset[set] = LAT3_SET;
		break;

	      case 'D':
		graphicsinset[set] = LAT4_SET;
		break;

	      case 'L':
		graphicsinset[set] = LATCYR_SET;
		break;

	      case '*':	/* we don't know what this should be yet */
		graphicsinset[set] = LATARAB_SET;
		break;

	      case 'F':
		graphicsinset[set] = LATGREEK_SET;
		break;

	      case 'H':
		graphicsinset[set] = LATHEBREW_SET;
		break;

	      case 'M':
		graphicsinset[set] = LAT5_SET;
		break;
#endif
	    } /* end switch(c) */
	} /* end if (ninetysix) */

    } else {			/* if in national character set mode */
	switch (c) {

 /* the first set of these don't actually change the national mapping */

	  case '0':
	  case '2':
	    graphicsinset[set] = GRAF_SET;
	    return;	/* don't change national mapping */

	  case '>':
	    graphicsinset[set] = TECH_SET;
	    return;	/* don't change national mapping */

 /* the rest of these do change the national mapping */

	  case 'B':
	  case '1':
	    nat_set = USA_NAT;
	    break;

	  case 'A':
	    nat_set = UK_NAT;
	    break;
		
	  case '4':
	    nat_set = DUTCH_NAT;
	    break;
		
	  case 'C':
	  case '5':
	    nat_set = FINNISH_NAT;
	    break;

	  case 'R':
	    nat_set = FRENCH_NAT;
	    break;
		
	  case '9':
	  case 'Q':
	    nat_set = FRENCHCAN_NAT;
	    break;
		
	  case 'K':
	    nat_set = GERMAN_NAT;
	    break;
		
	  case 'Y':
	    nat_set = ITALIAN_NAT;
	    break;

	  case '6':
	    if (intermedbuf[0] == '%') {
		nat_set = PORTUGUESE_NAT;
		break;
	    }
	    /* else fall through to norwegian */
	  case '`':
	  case 'E':
	    nat_set = NORWEGIAN_NAT;
	    break;				/* also Danish */

	  case 'Z':
	    nat_set = SPANISH_NAT;
	    break;

	  case '7':
	  case 'H':
	    nat_set = SWEDISH_NAT;
	    break;

	  case '=':
	    nat_set = SWISS_NAT;
	    break;
	} /* end switch(c) */
	graphicsinset[set] = ASCII_SET;
	set_char_map();
    }
}

set_char_map()
{
    register int i;
    
    for (i = 0; i < 128; i++)	/* reset the character remapping map */
	    char_map[i] = (unsigned char) i;
    if (nat_char_mode) {
	char_map['#'] = nat_remaps[nat_set][0];	/* set the values for the national map */
	char_map['@'] = nat_remaps[nat_set][1];
	char_map['['] = nat_remaps[nat_set][2];
	char_map['\\'] = nat_remaps[nat_set][3];
	char_map[']'] = nat_remaps[nat_set][4];
	char_map['^'] = nat_remaps[nat_set][5];
	char_map['_'] = nat_remaps[nat_set][6];
	char_map['`'] = nat_remaps[nat_set][7];
	char_map['{'] = nat_remaps[nat_set][8];
	char_map['|'] = nat_remaps[nat_set][9];
	char_map['}'] = nat_remaps[nat_set][10];
	char_map['~'] = nat_remaps[nat_set][11];
    }
}

/****************************************************************************/
/* draw a characer on the screen (or buffer it) */
/****************************************************************************/
MDrawChar (chr)
register unsigned char chr;
{
    register PFV funp;
    extern char outbuf[MAXCOL + 1];
    extern int outcnt, outcol;
    register int cset = Gl_set;

    if (chr & 0x80)		/* if a right-side (meta) character */
	cset = Gr_set;		/*  then we are in the GR character set */
    chr &= 0177;		/* trim to 7 bits */
    
    if (old_Gl_set >= 0) {	/* are we doing a single shift? */
	Gl_set = old_Gl_set;
	old_Gl_set = -1;
    }
    
    if ((nat_char_mode) && (graphicsinset[cset] == ASCII_SET)) {
	chr = char_map[chr];
	textstyle &= STY_STY;	/* reset to ASCII */
	set_style(textstyle);
    } else {
	if  ((chr >= vt_to_fmap[graphicsinset[cset]].lbound) &&
	     (chr <= vt_to_fmap[graphicsinset[cset]].hbound)) {
	    chr += vt_to_fmap[graphicsinset[cset]].coffset;
	    textstyle = (textstyle & STY_STY) |
			(vt_to_fmap[graphicsinset[cset]].fnum) << 4;
	} else {
	    textstyle &= STY_STY;	/* reset to ASCII */
	}
	set_style(textstyle);
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
	/* erase_char (); */	/* Erase the old char */
	MOVETOCHAR (curcol, curlin - display_topline);
	DrawChar (chr & 0377);
    } else {	/* Otherwise just buffer the char */
	/* PWP: an inline version of: buf_char (chr); */
	if (outcnt == 0)
	    outcol = curcol;	/* No chars in buffer, init column */
	outbuf[outcnt++] = chr;	/* Put in the buffer to output later */
    }
	
    scr[curlin][curcol] = chr;
    scr_attrs[curlin][curcol] = (unsigned char) textstyle;
    curcol++;
}				/* MDrawChar */



/****************************************************************************/
/*
 *      Control character functions:
 *              Each of the following allow the mac to simulate
 *              the behavior of a terminal when given the proper
 *              control character.
 */
/****************************************************************************/

void
back_space ()
{
    if (curcol > 0)
	relmove (-1, 0);
}				/* back_space */


void
erase_char ()
{
    Rect r;

    scr[curlin][curcol] = ' ';	/* Erase char for update */
    scr_attrs[curlin][curcol] = 0;	/* no attributes */
    makerect (&r, curlin, curcol, 1, 1);	/* One char by one line */

    EraseRect (&r);		/* (UoR) use InvertRect instead of FillRect */
    if (textstyle | VT_INVERT)
	InvertRect (&r);
}				/* erase_char */


void
tab ()
{
    int i;

    /* (UoR) find next tabstop */
    for (i = curcol + 1; (i < MAXCOL) && (tabstops[i] == 0); i++);
    absmove (i, curlin);
}				/* tab */


/* PWP: if you change this, also change MDrawChar() above */
void
line_feed ()
{
    if (to_printer) {				/*JAO*/
	(*hPrintBuffer)[lPrintBufferAt++] = 13;
	if (lPrintBufferAt == lPrintBufferSize)
	    lPrintBufferAt = 0L;
	lPrintBufferChars++;
	if (lPrintBufferChars == lPrintBufferSize) {
	    overflowingDialog = GetNewDialog(OVERFLOWINGBOXID, NILPTR, (WindowPtr) - 1);
	    DrawDialog(overflowingDialog);
	}
    }
    
    if (newline)
	absmove (0, curlin);	/* (UoR) perform newline function */

    /* (PWP) A better place to log the session */
    if (seslog)			/* if logging is active then */
	slog (scr[curlin], MAXCOL);	/* write line to session log */

    if (curlin == scrbot) {
	scroll_screen (scrtop, curlin, -1);	/* (UoR) scroll lines up */
    } else {
	if (curlin >= scrtop)	/* (PWP) if within scrolling region */
	    relmove (0, 1);
    }
}				/* line_feed */


void
reverse_line_feed ()
{
    if (curlin == scrtop) {
	scroll_screen (curlin, scrbot, 1);	/* (UoR) scroll down in region */
    } else {
	if (curlin <= scrbot)	/* (PWP) if within scrolling region */
	    relmove (0, -1);
    }
}				/* reverse_line_feed */



/* PWP: if you change this, also change MDrawChar() above */
void
carriage_return ()
{
    if (newline)
	line_feed ();		/* (UoR) perform newline function */
    else
	absmove (0, curlin);
}				/* carriage_return */


void
new_line ()
{
    carriage_return ();
    line_feed ();
}				/* new_line */

void
clear_screen ()
{
    register int i;
    Rect r;

    makerect (&r, 0, 0, screensize, MAXCOL);	/* The whole screen */
    EraseRect (&r);

    for (i = 0; i < screensize; i++)	/* (PWP) clear from bottom up */
	zeroline (i);	/* Clear up the update records */
}				/* clear_screen */

void
push_clear_screen()
{
    register int i, j, tlin;
    char *savedline, *savedattr;  /* temporary to hold screen line pointer */

    display_totlines += screensize;
    if (display_totlines > MAX_SCREENSIZE)
	display_totlines = MAX_SCREENSIZE;	/* bounds */

    tlin = screensize - display_totlines;	/* top of saved buffer */
    
    /* save cleared lines on scrollback buffer */
    for (j = 0; j < screensize; j++) {
	/* scroll screen buffer by one line */
 	savedline = scr[tlin];
	savedattr = scr_attrs[tlin];
	for (i = tlin+1; i <= botlin; i++) {
    	    scr[i-1] = scr[i];
    	    scr_attrs[i-1] = scr_attrs[i];
	}
	scr[botlin] = savedline;
	scr_attrs[botlin] = savedattr;
    }
    clear_screen();
}

void
vt_align()
{
    register int l, c;
    
    for (l = 0; l < screensize; l++) {
	absmove (0, l);
	if (outcnt == 0)
	    outcol = curcol;	/* No chars in buffer, init column */
	for (c = 0; c < MAXCOL; c++) {
	    scr[l][c] = 'E';
	    scr_attrs[l][c] = 0;
	    outbuf[outcnt++] = 'E';	/* Put in the buffer to output later */
	}
	flushbuf ();
    }
    absmove (0, 0);
}


void
home_cursor ()
{
    if (relorigin)
	absmove (0, scrtop);
    else
	absmove (0, 0);		/* (UoR) correct for relative origin */
}				/* home_cursor */


void
bell ()
{
    Rect r;

    if (visible_bell) {
	makerect (&r, 0, 0, screensize, MAXCOL);	/* The whole screen */
	InvertRect (&r);
	waitnoinput ();		/* sleep for a bit (1/30 sec) */
	InvertRect (&r);
    } else {
	SysBeep (3);
    }
}				/* bell */


/****************************************************************************/
/* does nothing. */
/****************************************************************************/
void
dummy ()
{
}				/* dummy */

void
toss_char ()
{
    charflg = CF_TOSS;
}				/* toss_char */

void
toss_till_st ()
{
    charflg = CF_T_ST;
}

void
norm_char ()
{
    charflg = CF_OUTC;
}

void
escape_seq ()
{
    intermedbuf[0] = '\0';		/* Initialize the numbers to zero */
    intermedptr = intermedbuf;		/* Place to put the next number */
    charflg = CF_ESC;		/* Say we are in an escape sequence */
}				/* escape_seq */

void
csi_seq ()
{
    argcount = 0;		/* no arguments yet */
    numarg[0] = 0;		/* Initialize the numbers to zero */

    paramarg[0][0] = '\0';
    argptr = paramarg[0];		/* Place to put the next number */

    numbuf[0] = '\0';		/* init number buffer */
    numptr = numbuf;
    
    intermedbuf[0] = '\0';		/* Initialize the numbers to zero */
    intermedptr = intermedbuf;		/* Place to put the next number */

    charflg = CF_CSI;		/* Say we are in a ESC [ sequence */
}				/* csi_seq */

void
dcs_seq ()
{
    csi_seq();		/* these are like CSI commands, but have strings too */
    charflg = CF_DCS;		/* Say we are in a device control sequence */
}

void
string_term()
{
    charflg = CF_OUTC;
}

void
vt320_mode()
{
}

void
vt52_mode()
{
}

void
graphic_G0 ()			/* (UoR) do VT100 graphic characters */
{
    charflg = CF_GS_0;
}				/* graphic_G0 */


void
graphic_G1 ()
{
    charflg = CF_GS_1;
}				/* graphic_G1 */


void
graphic_G2 ()
{
    charflg = CF_GS_2;
}				/* graphic_G1 */


void
graphic_G3 ()
{
    charflg = CF_GS_3;
}				/* graphic_G1 */


void
control_N ()		/* shift out */
{
    Gl_set = 1;		/* set to graphics set 1 */
}				/* control_N */


void
control_O ()		/* shift in */
{
    Gl_set = 0;		/* set to graphics set 0 */
}				/* control_O */


void
single_shift_2()
{
    old_Gl_set = Gl_set;
    Gl_set = 2;
}

void
single_shift_3()
{
    old_Gl_set = Gl_set;
    Gl_set = 3;
}

void
lock_shift_2()
{
    Gl_set = 2;
}

void
lock_shift_3()
{
    Gl_set = 3;
}

void
lock_shift_3r()
{
    Gr_set = 3;
}

void
lock_shift_2r()
{
    Gr_set = 2;
}

void
lock_shift_1r()
{
    Gr_set = 1;
}

void
h19_graph_mode()
{
}

void
end_h19_graph_mode()
{
}

void
set_appl ()
{
    appl_mode = TRUE;		/* applications keypad mode */
}


void
reset_appl ()
{
    appl_mode = FALSE;		/* normal keypad mode */
}

void
set_compat()			/* set vtNNN level of compatibility */
{
    if (argcount < 2) numarg[1] = 0;
    if (argcount < 1) numarg[0] = 0;
    if (intermedbuf[0] == '"') {
	switch (numarg[0]) {
	  case 61:		/* vt100 mode */
	    break;
	    
	  case 62:		/* vt200 mode */
	    switch (numarg[1]) {
	      case 0:
		break;
		
	      case 1:
		break;
		
	      case 2:
		break;
	    }
	}
    }
}

void
set_charattr()
{
    if (intermedbuf[0] == '"') {
    	switch (numarg[0]) {
	  case 0:	/* all attributes off */
	    ch_protect = FALSE;
	    break;
	  
	  case 1:
	    ch_protect = TRUE;
	    break;
	    
	  case 2:
	    ch_protect = FALSE;
	    break;
	}
    }
}

void
clear_line ()
{
    int i;
    Rect r;

    maybe_nuke_selection (curlin, curlin);
    switch (numarg[0]) {
      case 0:			/* Clear:  here to the right */
	makerect (&r, curlin, curcol, 1, MAXCOL - curcol);
	for (i = curcol; i < MAXCOL; i++) {
	    scr[curlin][i] = ' ';
	    scr_attrs[curlin][i] = 0;
	}
	break;

      case 1:			/* Clear:  left to here */
	makerect (&r, curlin, 0, 1, curcol + 1);
	for (i = 0; i <= curcol; i++) {
	    scr[curlin][i] = ' ';
	    scr_attrs[curlin][i] = 0;
	}
	break;

      case 2:			/* Clear:  entire line */
	makerect (&r, curlin, 0, 1, MAXCOL);
	zeroline (curlin);
	break;
    }
    EraseRect (&r);
}				/* clear_line */


void
erase_display ()
{
    int i;
    Rect r;

    switch (numarg[0]) {
      case 0:		/* clear from here to end */
	maybe_nuke_selection (curlin, botlin);
	if ((curlin == toplin) && (curcol == 0)) {
	    push_clear_screen ();	/* save lines in scrollback buffer */
	} else {
	    clear_line ();		/* Same numarg[0] causes correct clear */
	    makerect (&r, curlin + 1, 0, screensize - curlin - 1, MAXCOL);
				/* (UoR) -1 added */
	    EraseRect (&r);
	    for (i = curlin + 1; i <= botlin; i++)
		zeroline (i);
	}
	break;

      case 1:		/* clear from beginning to here */
	maybe_nuke_selection (toplin, curlin);
	clear_line ();		/* Same numarg[0] causes correct clear */
	makerect (&r, 0, 0, curlin, MAXCOL);
	EraseRect (&r);
	for (i = toplin; i < curlin; i++)
	    zeroline (i);
	break;

      case 2:		/* clear everything */
	maybe_nuke_selection (toplin, botlin);
	push_clear_screen ();	/* save lines in scrollback buffer */
	break;
    }
}				/* erase_display */


/****************************************************************************/
/**** All cursor moves need to check that they don't go beyond the margins */
/****************************************************************************/
void
cursor_right ()
{
    if (numarg[0] == 0) numarg[0] = 1;
    relmove (numarg[0], 0);
}				/* cursor_right */


void
cursor_left ()
{
    if (numarg[0] == 0) numarg[0] = 1;
    relmove (-numarg[0], 0);
}				/* cursor_left */


void
cursor_up ()
{
    int abstop;			/* (UoR) check that we don't pass scrtop */

    abstop = scrtop;
    if (numarg[0] == 0) numarg[0] = 1;
    if ((curlin >= abstop) && (curlin - numarg[0] < abstop))
	absmove (curcol, abstop);
    else
	relmove (0, -numarg[0]);
}				/* cursor_up */


void
cursor_down ()
{
    int absbot;			/* (UoR) check that we don't pass scrbot */

    absbot = scrbot;
    if (numarg[0] == 0) numarg[0] = 1;
    if ((curlin <= absbot) && (curlin + numarg[0] > absbot))
	absmove (curcol, absbot);
    else
	relmove (0, numarg[0]);
}				/* cursor_down */


void
cursor_position ()
{
    if (argcount < 2) numarg[1] = 1;
    if (argcount < 1) numarg[0] = 1;
    if (relorigin)
	absmove (--numarg[1], scrtop + numarg[0] - 1);	/* (UoR) */
    else
	absmove (--numarg[1], --numarg[0]);	/* (UoR) moved "--" here from prev
					 * lines */
}				/* cursor_position */

void
cursor_h_pos ()
{
    absmove (--numarg[0], curlin);
}				/* cursor_h_pos */

void
cursor_save ()		/* ESC 7 */
{
    savcol = curcol;		/* Save the current line and column */
    savlin = curlin;

    savsty = textstyle;		/* (UoR) additions */
    savGl = Gl_set;
    savGr = Gr_set;
    savset[0] = graphicsinset[0];
    savset[1] = graphicsinset[1];
    savset[2] = graphicsinset[2];
    savset[3] = graphicsinset[3];
}				/* cursor_save */


void
cursor_restore ()	/* ESC 8 */
{
    if (intermedbuf[0] == '#') {
	vt_align();
	return;
    }
    
    absmove (savcol, savlin);	/* Move to the old cursor position */

    textstyle = savsty;		/* (UoR) additions */
    set_style(textstyle);	/* new text face */
    Gl_set = savGl;
    Gr_set = savGr;
    graphicsinset[0] = savset[0];
    graphicsinset[1] = savset[1];
    graphicsinset[2] = savset[2];
    graphicsinset[3] = savset[3];
}				/* cursor_restore */

void
set_scroll_region ()
{
    if (argcount < 2) numarg[1] = 0;
    if (--numarg[0] < 0)
	numarg[0] = 0;		/* Make top of line (prev line) */
    if (numarg[1] == 0)
	numarg[1] = screensize;		/* Zero means entire screen */

    if (numarg[0] < numarg[1] - 1) {	/* (UoR) make sure region is legal */
	topmargin = (numarg[0] * LINEHEIGHT) + TOPMARGIN;
	bottommargin = (numarg[1] * LINEHEIGHT) + TOPMARGIN;

	scrtop = numarg[0];
	scrbot = numarg[1] - 1;

	home_cursor ();		/* We're supposed to home it! */
    }
}				/* set_scroll_region */



/****************************************************************************/
/* aka Select Graphic Rendition */
/****************************************************************************/
void
text_mode ()
{
    int i;
    
    if (argcount == 0) {
	argcount = 1;
	numarg[0] = 0;
    }
    
    for (i = 0; i < argcount; i++) {
	switch (numarg[i]) {
	  case 0:			/* primary rendition */
	    textstyle = 0;
	    break;

	  case 1:			/* bold or increased intensity */
	    textstyle |= VT_BOLD;
	    break;

	  case 4:		/* underscore */
	    textstyle |= VT_UNDER;    /* (PWP) */
	    break;

	  case 2:		/* faint or decreased intensity or
				 *  secondary color */
	  case 3:		/* italic */
	  case 5:		/* slow blink (< 150/sec); (UoR) blink is
				 * inverse */
	  case 6:		/* fast blink (>= 150/sec) */
	    textstyle |= VT_BLINK;
	    break;
	    
	  case 7:		/* reverse image */
	    textstyle |= VT_INVERT;    /* (PWP) */
	    break;

	  case 21:		/* ??? */
	  case 22:		/* normal intensity */
	    textstyle &= ~(VT_BOLD);    /* (PWP) */
	    break;

	  case 24:		/* not underlined */
	    textstyle &= ~(VT_UNDER);    /* (PWP) */
	    break;

	  case 25:		/* not blinking */
	    textstyle &= ~(VT_BLINK);    /* (PWP) */
	    break;
	    
	  case 27:		/* not reversed */
	    textstyle &= ~(VT_INVERT);    /* (PWP) */
	    break;
	}
	set_style(textstyle);	/* new text face */
    }
}				/* text_mode */


void
line_dblh_top()		/* double height line, top half */
{
}

void
line_dblh_bot()		/* double height line, bottom half */
{
}

void
line_singw()		/* single width line */
{
}

void
line_dblw()		/* double width line */
{
}

void
start_selected()
{
}

void
end_selected()
{
}

/****************************************************************************/
/*
 * (UoR)
 *
 * Insert and Delete lines (replacements for originals, which have
 *   which have been deleted)
 *
 */
/****************************************************************************/
void
insert_line ()
{
    int i, absbot;

    absbot = scrbot;

    if ((curlin >= scrtop) && (curlin <= absbot)) {
	if (numarg[0] == 0)
	    numarg[0] = 1;
	if (numarg[0] > absbot - curlin + 1)
	    numarg[0] = absbot - curlin + 1;

	for (i = 0; i < numarg[0]; i++)
	    scroll_screen (curlin, scrbot, 1);
    }
}				/* insert_line */


void
delete_line ()
{
    int i, absbot;

    absbot = scrbot;

    if ((curlin >= scrtop) && (curlin <= absbot)) {
	if (numarg[0] == 0)
	    numarg[0] = 1;
	if (numarg[0] > absbot - curlin + 1)
	    numarg[0] = absbot - curlin + 1;

	for (i = 0; i < numarg[0]; i++)
	    scroll_screen (curlin, scrbot, -1);
    }
}				/* delete_line */


void
delete_char ()
{
    int i;
    Rect r;

    if (numarg[0] == 0)
	numarg[0] = 1;

    makerect (&r, curlin, curcol, 1, MAXCOL - curcol);

    if (numarg[0] > MAXCOL - curcol - 1)
	numarg[0] = MAXCOL - curcol - 1;

    /* Scroll them out */
    ScrollRect (&r, -CHARWIDTH * numarg[0], 0, dummyRgn);

    /* Shift them down *//* (UoR) used to assign using abscol */
    for (i = curcol; i < MAXCOL - numarg[0]; i++) {
	scr[curlin][i] = scr[curlin][i + numarg[0]];
	scr_attrs[curlin][i] = scr_attrs[curlin][i + numarg[0]];
    }
    
    /* Fill in holes with spaces */
    while (i < MAXCOL) {
	scr[curlin][i++] = ' ';
	scr_attrs[curlin][i++] = 0;
    }
}				/* delete_char */



/****************************************************************************/
/* CME */
/****************************************************************************/
void
insert_chars ()
{
    int i;
    Rect r;

    if (numarg[0] == 0)
	numarg[0] = 1;

    makerect (&r, curlin, curcol, 1, MAXCOL - curcol);

    if (numarg[0] > MAXCOL - curcol - 1)
	numarg[0] = MAXCOL - curcol - 1;

    /* Scroll them out */
    ScrollRect (&r, CHARWIDTH * numarg[0], 0, dummyRgn);

    /* Shift them up *//* (UoR) used to assign using abscol */
    for (i = MAXCOL - 1; i >= curcol + numarg[0]; i--) {
	scr[curlin][i] = scr[curlin][i - numarg[0]];
	scr_attrs[curlin][i] = scr_attrs[curlin][i - numarg[0]];
    }
    
    /* Fill in holes with spaces */
    while (i > curcol) {
	scr[curlin][--i] = ' ';
	scr_attrs[curlin][--i] = 0;
    }
}				/* delete_char */


void
insert_char ()
{
    int i;
    Rect r;

    makerect (&r, curlin, curcol, 1, MAXCOL - curcol);
    ScrollRect (&r, CHARWIDTH, 0, dummyRgn);

    /* Shift em up *//* (UoR) used to assign ...[i-1]=...[i] */
    /* (UoR) used to assign using abscol */

    for (i = MAXCOL - 1; i > curcol; i--) {
	scr[curlin][i] = scr[curlin][i - 1];
	scr_attrs[curlin][i] = scr_attrs[curlin][i - 1];
    }
    
    scr[curlin][curcol] = ' ';
    scr_attrs[curlin][curcol] = 0;
}				/* insert_char */


void
insert_mode ()
{
    int i;

    if (argcount < 1) {
	numarg[0] = 0;
	argcount = 1;
    }
    for (i = 0; i < argcount; i++) {	/* handle multiple arguments */
	if (paramarg[i][0] == '?') {
	    set_mode (numarg[i]);		/* (UoR) do some of these calls */
	} else if (paramarg[i][0] == '>') {	/* (PWP) Heath ANSI stuff */
	    set_heath_mode(numarg[i]);
	} else {
	    switch (numarg[i]) {
	      case 20:		/* newline mode */
		newline = TRUE;
		break;

	      case 12:		/* send/recieve */
		break;
	
	      case 4:		/* insert mode */
		insert = TRUE;
		break;
	    }
	}
    }
}				/* insert_mode */


void
end_insert_mode ()
{
    int i;
    
    if (argcount < 1) {
	numarg[0] = 0;
	argcount = 1;
    }
    for (i = 0; i < argcount; i++) {	/* handle multiple arguments */
	if (paramarg[i][0] == '?') {
	    reset_mode (numarg[i]);		/* (UoR) do some of these calls */
	} else if (paramarg[i][0] == '>') {	/* (PWP) Heath ANSI stuff */
	    reset_heath_mode(numarg[i]);
	} else {
	    switch (numarg[i]) {
	      case 20:		/* newline mode */
		newline = FALSE;
		break;

	      case 12:		/* send/recieve */
		break;
	
	      case 4:		/* insert mode */
		insert = FALSE;
		break;
	    }
	}
    }
}				/* end_insert_mode */

/****************************************************************************/

void
printer_control ()				/*JAO*/
{
    extern char *NewHandle();
    
    switch (numarg[0]) {	/* "ESC [ ? # i" */
      case 5: /*Start printing*/
	if (hPrintBuffer == 0L) {
	    lPrintBufferSize = FreeMem();
	    do {
		lPrintBufferSize = (lPrintBufferSize >> 1)
			 + (lPrintBufferSize >> 2);
		hPrintBuffer = NewHandle(lPrintBufferSize);
	    } while (hPrintBuffer == NIL);
	    lPrintBufferSize = (lPrintBufferSize * 2) / 3;
	    SetHandleSize(hPrintBuffer, lPrintBufferSize);
	    lPrintBufferAt = lPrintBufferChars = 0L;
	} else {
	    (*hPrintBuffer)[lPrintBufferAt++] = 14;
	    if (lPrintBufferAt == lPrintBufferSize)
	        lPrintBufferAt = 0L;
	    lPrintBufferChars++;
	}
	bufferingDialog = GetNewDialog(BUFFERINGBOXID, NILPTR, (WindowPtr) - 1);
	DrawDialog(bufferingDialog);
	overflowingDialog = 0L;
	if (lPrintBufferChars == lPrintBufferSize) {			/*JAOtemp*/
	    overflowingDialog = GetNewDialog(OVERFLOWINGBOXID, NILPTR, (WindowPtr) - 1);
	    DrawDialog(overflowingDialog);				/*JAOtemp*/
	}								/*JAOtemp*/
			
	to_printer = TRUE;
	if (paramarg[0][0] == '?')
	    to_screen  = TRUE;
	else
	    to_screen  = TRUE; /*FALSE;*/
	break;

      case 4: /*Stop printing*/
	if (overflowingDialog != 0L) {
	    DisposDialog(overflowingDialog);
	}
	DisposDialog(bufferingDialog);
	to_printer = FALSE;
	to_screen  = TRUE;
	EnableItem(menus[PRNT_MENU], 0);
	DrawMenuBar();
	break;
    }
}	/* end of printer_control */		/*JAO*/


void
invert_term ()
{
    if (screeninvert)
	reset_mode (5);
    else
	set_mode (5);
}				/* invert_term */

void
set_mode (arg)	/* we get here by ESC [ ? <numarg[0]> h */
int arg;
{
    Rect r;

    switch (arg) {
      case 1:		/* cursor key mode */
	curskey_mode = TRUE;
	break;
	
      case 2:		/* keyboard lock */
	break;		/* we never do keyboard lock */
	
      case 3:		/* 132 column */
	break;
	
      case 4:		/* smooth scroll */
        smoothscroll = TRUE;
	break;
	
      case 5:		/* reverse screen */
	if (screeninvert == FALSE) {
	    BackPat (qd.black);	/* (UoR) use black background */
	    makerect (&r, 0, 0, screensize, MAXCOL);
	    InvertRect (&r);
	    screeninvert = TRUE;
	}
	break;

      case 6:		/* relative origin mode */
	relorigin = TRUE;
	home_cursor ();
	break;

      case 7:		/* auto wrap */
	autowrap = TRUE;
	break;

      case 8:		/* auto repeat */
	autorepeat = TRUE;
	break;

      case 18:		/* print form feed */
	break;

      case 19:		/* print extent */
	break;

      case 25:		/* text cursor enable */
	break;

      case 38:		/* Tektronics mode */
	break;

      case 42:		/* international character set */
	nat_char_mode = TRUE;
	set_char_map();
	break;

      case 43:		/* graphics expanded print */
	break;

      case 44:		/* graphics print color */
	break;

      case 45:		/* graphics print color syntax */
	break;

      case 46:		/* graphics print background */
	break;

      case 47:		/* graphics rotated print */
	break;
    }
}				/* set_mode */


void
reset_mode (arg)	/* we get here by ESC [ ? <numarg[0]> l */
int arg;
{
    Rect r;

    switch (arg) {
      case 1:		/* cursor key mode */
	curskey_mode = FALSE;
	break;
	
      case 2:		/* keyboard lock */
	break;		/* we never do keyboard lock */
	
      case 3:		/* 132 column */
	break;
	
      case 4:		/* smooth scroll */
        smoothscroll = FALSE;
	break;
	
      case 5:		/* reverse screen */
	if (screeninvert) {
	    BackPat (qd.white);
	    makerect (&r, 0, 0, screensize, MAXCOL);
	    InvertRect (&r);
	    screeninvert = FALSE;
	}
	break;

      case 6:		/* relative origin mode */
	relorigin = FALSE;
	home_cursor ();
	break;

      case 7:		/* auto wrap */
	autowrap = FALSE;
	break;

      case 8:		/* auto repeat */
	autorepeat = FALSE;
	break;

      case 18:		/* print form feed */
	break;

      case 19:		/* print extent */
	break;

      case 25:		/* text cursor enable */
	break;

      case 38:		/* Tektronics mode */
	break;

      case 42:		/* international character set */
	nat_char_mode = FALSE;
	set_char_map();    /* reset the display translate table */
	break;

      case 43:		/* graphics expanded print */
	break;

      case 44:		/* graphics print color */
	break;

      case 45:		/* graphics print color syntax */
	break;

      case 46:		/* graphics print background */
	break;

      case 47:		/* graphics rotated print */
	break;
    }
}				/* reset_mode */

void
set_heath_mode (arg)	/* (PWP) we get here by ESC [ > <numarg[0]> h */
int arg;
{
    Rect r;

    switch (arg) {
      case 1:		/* enable status (25th) line */
      case 2:		/* disable key click */
      case 3:		/* enter hold screen mode (we never do this) */
	break;

      case 4:		/* block cursor */
	blockcursor = TRUE;
	break;

      case 5:		/* show cursor */
	cursor_shown = TRUE;
	break;

      case 6:		/* keypad shifted */
      case 7:		/* enter alternate keypad mode */
      case 8:		/* auto LF on CR */
      case 9:		/* auto CR on LF */
	break;
    }
}

void
reset_heath_mode (arg)	/* (PWP) we get here by ESC [ > <numarg[0]> h */
int arg;
{
    Rect r;

    switch (arg) {
      case 1:		/* disable status (25th) line */
      case 2:		/* enable key click */
      case 3:		/* exit hold screen mode (we never do this) */
	break;

      case 4:		/* underline cursor */
	blockcursor = FALSE;
	break;

      case 5:		/* hide cursor */
	cursor_shown = FALSE;
	break;

      case 6:		/* keypad shifted */
      case 7:		/* enter alternate keypad mode */
      case 8:		/* auto LF on CR */
      case 9:		/* auto CR on LF */
	break;
    }
}

void
set_tab ()
{
    tabstops[curcol] = 1;
}				/* set_tab */


void
clear_tab ()
{
    int i;

    switch (numarg[0]) {
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
void
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


void
query_terminal ()
{
    writereply (querystring);
}				/* query_terminal */



/****************************************************************************/
/* (UoR) reports */
/****************************************************************************/
void
request_report ()
{
    switch (numarg[0]) {
      case 5:			/* (UoR) report that we're OK */
	writereply (reportstring);
	break;

      case 6:			/* (UoR) reprt the cursor position */
	position_report ();
	break;

      case 15:			/* (UoR) report printer status */
	if (paramarg[0][0] == '?')
	    writereply (noprinter);
	break;
    }
}				/* request_report */


void
position_report ()
{
    int i;
    char buf[9];
    char *report;

    i = 0;
    buf[i++] = '\033';
    buf[i++] = '[';
    if (curlin > 9)
	buf[i++] = '0' + (curlin + 1) / 10;
    buf[i++] = '0' + (curlin + 1) % 10;
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
/* Routine zeroline
 *
 * Zero (set to space) all the characters in absolute line lin.
 *
 */
/****************************************************************************/
void
zeroline (lin)
int lin;
{
    register int i;
    Rect r;

    for (i = 0; i < MAXCOL; i++) {
	scr[lin][i] = ' ';
	scr_attrs[lin][i] = 0;
    }
}				/* zeroline */



/****************************************************************************/
/* Move a relative number of lines and chars.  Both can be negative. */
/****************************************************************************/
void
relmove (hor, ver)
{
    absmove (curcol + hor, curlin + ver);	/* (UoR) use absmove, which
						 * checks */
    /* for cursor moving off screen */
}				/* relmove */



/****************************************************************************/
/* Move to absolute position hor char and ver line. */
/****************************************************************************/
void
absmove (hor, ver)
{
    if (hor > MAXCOL - 1)
	hor = MAXCOL - 1;	/* (UoR) make sure its on the screen */
    if (hor < 0)
	hor = 0;
    if (ver > screensize - 1)
	ver = screensize - 1;
    if (ver < 0)
	ver = 0;
    if (relorigin) {
	if (ver < scrtop)
	    ver = scrtop;
	if (ver > scrbot)
	    ver = scrbot;
    }
    /* MOVETOCHAR (hor, ver - display_topline); */
    curcol = hor;
    curlin = ver;
}				/* absmove */



/****************************************************************************/
/* dump the whole screen to the session log file */
/****************************************************************************/
void
scrtolog ()
{
    int lin, i;

    lin = toplin;
    for (i = 0; i < screensize; i++) {
	slog (scr[lin], MAXCOL);/* write this line to session log */
	lin++;		/* lin = nxtlin[lin]; */
    }
}				/* scrtolog */

void
scrlasttolog ()
{
    slog (scr[screensize-1], MAXCOL);	/* write last line to session log */
}
