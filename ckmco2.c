/*
 * FILE ckmco2.c
 *
 * Module of mackermit: contains code for dealing with the Mac side
 * of terminal emulation.
 */

#include "ckcdeb.h"

#define	__SEG__	ckmcon
#include <quickdraw.h>
#include <controls.h>
#include <devices.h>
#include <serial.h>
#include <files.h>
#include <events.h>
#include <windows.h>
#include <dialogs.h>
#include <menus.h>
#include <toolutils.h>
#include <osutils.h>
#include <resources.h>
#include <ctype.h>
#include <Script.h>
#include <Memory.h>

#include "ckmdef.h"
#include "ckmasm.h"		/* Assembler code */
#include "ckmres.h"		/* kermit resources */

#include "ckmcon.h"		/* defines, etc. for terminal emulator */

extern WindowPtr terminalWindow;	/* the terminal window */
Rect ScreenRect;
ControlHandle t_vscroll;

 /*
  * (UoR) don't need scrollrect any more (use scroll_up and scroll_down), use
  * ScreenRect for mouse check
  */

/* Screen book keeping variables */
extern int screensize,		/* variable number of lines on screen */
    topmargin,			/* Edges of adjustable window */
    bottommargin,
    graphicsinset[4],		/* (UoR) current character sets */
    Gl_set, 			/* (UoR) current chosen set */
    Gr_set,			/* (PWP) current chosen upper set */
    screeninvert,		/* (UoR) inverted screen flag */
    insert,
    newline,			/* (UoR) linefeed mode by default */
    autowrap,			/* Autowrap on by default */
    relorigin,			/* (UoR) relative origin off */
    autorepeat,			/* (UoR) auto repeat flag */
    appl_mode,			/* (PWP) application mode */
    curskey_mode,		/* (PWP) cursor key application mode */
    smoothscroll,		/* do smooth scrolling (PWP: or not) */
    transparent,		/* do not show control characters */
    blockcursor,		/* show block or underline cursor */
    cursor_shown,		/* (PWP) show the cursor */
    mouse_arrows,		/* mouse down in screen does arrow keys */
    visible_bell,		/* true if we do blink instead of bell */
    eightbit_disp,		/* do 8 bit wide to the screen */
    blinkcursor;		/* true if we make the cursor blink */

extern ucharptr *scr, *scr_attrs;		/* virtual screen pointer */
extern ucharptr real_scr[MAX_SCREENSIZE];	/* The real screen, including scrollback */
extern ucharptr real_attrs[MAX_SCREENSIZE];	/* the attributes of each character */

int display_topline;		/* top line actually displayed */
int display_totlines;		/* number of real lines in screen + scrollback */
extern int curlin, curcol;	/* Cursor position */
extern int savcol, savlin;		/* Cursor save variables */
extern int savsty, savfnt, savgrf, savmod, savset[2];	/* (UoR) cursor save
						 * variables */
extern int savund;			/* PWP for saved underlining */
extern int scrtop, scrbot;		/* Absolute scrolling region bounds */

int textstyle = 0;		/* (UoR) current style */
int current_style = 0;
int draw_sing_chars = FALSE;

int oldlin = -1;
int oldcol = 0;			/* (UoR) for last mouse position */

/* (PWP) variables for controling the selected text */
int from_lin = -1;
int from_col = 0;
int to_lin;
int to_col;
Boolean have_selection = 0;

#define ABS(a)	((a) < 0 ? -(a) : (a))

int cursor_invert = FALSE,	/* (UoR) for flashing cursor */
    cur_drawn = FALSE;
long last_flash = 0;

char **myclip_h;		/* internal clipboard */
int myclip_size;		/* size of above */

static int mousecurs_drawn = FALSE;	/* (PWP) is the mouse cursor drawn? */

static int in_front = 0;	/* PWP: true when we are the front window */

extern Boolean usingRAMdriver,
	       have_128roms;	/* true if we are a Plus or better */

RgnHandle dummyRgn;		/* dummy region for ScrollRect */
				/* Initialized in mac_init */

long MyCaretTime;		/* (UoR) ticks between flashes */

extern Cursor *textcurs, *normcurs, *watchcurs;	/* mouse cursor shapes */

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

/* keyboard handling stuff */

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


Boolean have_scriptmgr = FALSE;

long old_KCHR, old_SICN;	/* pointers to current system key script, icon */
long cur_KCHR;

/***** forward decls ********/

void cursor_draw(), cursor_erase();

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/* keyboard event handling routines                                         */
/****************************************************************************/


InitKeyStuff()
{
    have_scriptmgr = NGetTrapAddress(num_UnknownTrap, 1) !=
		     NGetTrapAddress(num_ScriptTrap, 1);
		     
    if (have_scriptmgr) {
	old_KCHR = GetScript( smRoman, smScriptKeys);
	old_SICN = GetScript( smRoman, smScriptIcon);
    }
    cur_KCHR = old_KCHR;

    UpdateOptKey(1);	/* get things set right initially */
}

UpdateOptKey(enable)
int enable;
{
    int i;
    int futzit = 0;
    
    if (enable) {
	for (i = 0; i < NUMOFMODS; i++) {
	/* shift what to look for into high byte */
	    if ((modtable[i].modbits) & (optionKey >> 4))	/* if Option is selected */
	        futzit = 1;
	}
    } else {	/* allways turn off when disabling window */
        futzit = 0;
    }
    
    (void) FutzOptKey(futzit);
}

FutzOptKey(enable)
int enable;
{
    int err;
    
    if (have_scriptmgr) {		/* if we are system 4.1 or later... */
	if (enable) {	/* no deadkeys */
	    if (cur_KCHR != old_KCHR)
		return (1);	/* we are allready fine */
	    if (GetEnvirons(smKeyScript) == smRoman) {  /* only if in roman script */
		/* set the key map */
		err = SetScript (smRoman, smScriptKeys, NODEAD_KCHR);
		if (err != noErr) {
		    printerr ("Trouble setting custom keymap (KCHR):", err);
		    return (0);
		}
		/* set the icon */
		err = SetScript (smRoman, smScriptIcon, NODEAD_SICN);
		if (err != noErr) {
		    printerr ("Trouble setting custom keymap icon (SICN):", err);
		    return (0);
		}
		KeyScript (smRoman);
		cur_KCHR = NODEAD_KCHR;
		return (1);	/* success! */
	    } else {
	        printerr("Can't disable Option key -- you have a non-US keyboard",0);
	        return (0);
	    }
	} else {	/* back to normal */
	    if (cur_KCHR == old_KCHR)
		return (1);	/* we are allready fine */
	    /* set the key map */
	    err = SetScript (smRoman, smScriptKeys, old_KCHR);
	    if (err != noErr) {
		printerr ("Trouble resetting default keymap (KCHR):", err);
		return (0);
	    }
	    /* set the icon */
	    err = SetScript (smRoman, smScriptIcon, old_SICN);
	    if (err != noErr) {
		printerr ("Trouble resetting default keymap icon (SICN):", err);
		return (0);
	    }
	    KeyScript (smRoman);
	    cur_KCHR = old_KCHR;
	    return (1);		/* success! */
	}
    } else {
	/* do something or other to do the old way */
	/* printerr("Kermit can't disable Option on old systems",0); */
    }
    return (0);
}


/****************************************************************************/
/* return the ASCII character which is generated by the keyCode specified */
/* with no modifiers pressed */
/****************************************************************************/
unsigned char
DeModifyChar (keyCode, modifiers)
long keyCode, modifiers;
{
    long c;
    long mystate, retval;
    short s_keycode;
    Handle kchr_h;
    THz curZone;

#ifdef COMMENT
    ProcHandle KeyTrans;

    if (keyCode > 64)
	KeyTrans = (ProcHandle) 0x2A2;	/* keypad decode */
    else
	KeyTrans = (ProcHandle) 0x29E;	/* keyboard decode */

    SaveRegs ();		/* save all registers */
    AllRegs ();

    /* setup regs for procedure call */
    /* loadD1 ((long) modifiers); */		/* no modifiers */
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

#endif /* COMMENT */

    if (have_scriptmgr) {		/* if we are system 4.1 or later... */
        mystate = 0;
	
	kchr_h = GetResource('KCHR', cur_KCHR);
	if (kchr_h == NIL) {
	    printerr("DeModifyChar: couldn't get KCHR address",0);
	    return(0);
	}
	LoadResource(kchr_h);
	HLock(kchr_h);
	
	s_keycode = (modifiers & 0xff00) | (keyCode & 0xff);

	c = KeyTrans(*kchr_h, s_keycode, &mystate);
	HUnlock(kchr_h);
	curZone = GetZone();		/* as per John Norstad's (Disinfectant) */
	SetZone(HandleZone(kchr_h));	/* "Toolbox Gotchas" */
	ReleaseResource(kchr_h);
	SetZone(curZone);
    }    
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

    obuf[1] = c;		/* store character */
    writeps (obuf);		/* and write it out */

    if (duplex != 0) {
	cursor_erase ();	/* remove from screen */
	printit ((char) c);	/* Echo the char to the screen */
	flushbuf ();		/* flush the character */
	cursor_draw ();		/* put it back */
    }
}				/* OutputChar */

#ifdef COMMENT
/****************************************************************************/
/* Bittest returns the setting of an element in a Pascal PACKED ARRAY [0..n]
   OF Boolean such as the KeyMap argument returned by GetKey
/****************************************************************************/
Boolean
bittest (bitmap, bitnum)
char bitmap[];
int bitnum;
{
    return (0x01 & (bitmap[bitnum / 8] >> (bitnum % 8)));
}				/* bittest */

/* PWP: or, as a macro, */
#define bittest(bitmap,bitnum)	(0x01 & (bitmap[bitnum / 8] >> (bitnum % 8)))

#endif /* COMMENT */


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
    if (BitTst (keytable, theCode)) {
	GetMacro (theCode, &flags, tmpstr);	/* get the macrostring */

	if (flags) {		/* check special flags */
	    switch (flags) {
	      case shortBreak:
		sendbreak (5);
		return;

	      case longBreak:
		sendbreak (70);
		return;

	      case leftArrowKey:
		do_arrow (leftARROW);
		return;

	      case rightArrowKey:
		do_arrow (rightARROW);
		return;

	      case upArrowKey:
		do_arrow (UPARROW);
		return;

	      case downArrowKey:
		do_arrow (DOWNARROW);
		return;
		
	      case keycomma:
	      case keyminus:
	      case keyperiod:
	      /* there is no keyslash */
	      case key0:
	      case key1:
	      case key2:
	      case key3:
	      case key4:
	      case key5:
	      case key6:
	      case key7:
	      case key8:
	      case key9:
		do_keypad(flags - keycomma);
		return;
		
	      case keypf1:
	      case keypf2:
	      case keypf3:
	      case keypf4:
		do_pfkey(flags - keypf1);
		return;
		
	      case keyenter:
		do_keyenter();
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
		c = DeModifyChar ((long) ((evt->message & myKeyCodeMask) >> 8),
				  (long) (modCode & shiftKey));
			 /* PWP: we pass through the shiftedness of this key */
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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/* general rectangle routines                                               */
/****************************************************************************/



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

/* (PWP) do what makerect does, then invert the rect */
invertchars (lin, col, numlin, numcol)
int lin;
int col;
int numlin;
int numcol;
{
    Rect r;
    r.top = lin * LINEHEIGHT + TOPMARGIN;
    r.left = col * CHARWIDTH + LEFTMARGIN;
    r.bottom = r.top + numlin * LINEHEIGHT;
    r.right = r.left + numcol * CHARWIDTH;
    InvertRect (&r);
}

/* Make rect r (made by makerect()) into the right shape
	 for underlining */
makertou (r)
Rect *r;
{
    r->top = r->bottom - 1;
}

/****************************************************************************/
/* Connect support routines */
/****************************************************************************/
consetup ()
{
    PenMode (srcCopy);		/* (PWP) was patXor */
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

void
term_reset ()
{
    PenMode (srcCopy);		/* (PWP) was patXor */
    flushio ();			/* Get rid of pending characters */
    screen_to_bottom ();	/* slide the visible region to active area */

    graphicsinset[0] = ASCII_SET;
    graphicsinset[1] = GRAF_SET;
    norm_char();
    Gl_set = 0;
    Gr_set = 1;
    textstyle = 0;
    current_style = 0;
    draw_sing_chars = 0;
    screeninvert = FALSE;	/* (UoR) inverted screen flag */
    insert = FALSE;
    newline = FALSE;		/* (UoR) linefeed mode by default */
    autowrap = TRUE;		/* Autowrap on by default */
    relorigin = FALSE;		/* (UoR) relative origin off */
    autorepeat = TRUE;		/* (UoR) auto repeat flag */
    appl_mode = FALSE;		/* (PWP) keypad application mode */
    curskey_mode = FALSE;	/* (PWP) cursor key application mode */
    smoothscroll = FALSE;	/* do smooth scrolling (PWP: or not) */
    transparent = TRUE;		/* do not show control characters */
    blockcursor = TRUE;		/* show block or underline cursor */
    cursor_shown = TRUE;	/* (PWP) show the cursor */
    mouse_arrows = FALSE;	/* mouse down in screen does arrow keys */
    visible_bell = FALSE;	/* true if we do blink instead of bell */
    eightbit_disp = FALSE;	/* default to 7 bits */
    blinkcursor = TRUE;		/* true if we make the cursor blink */

    have_selection = FALSE;	/* (PWP) we have no selected text */
    
    TextFont (VT100FONT);	/* (UoR) Set initial font to VT100 */
    TextMode (srcXor);		/* (UoR) use XOR mode (for inverse) */
    TextFace (0);		/* PWP: be safe.  We allways stay like this */
    clear_screen ();		/* Clear the screen */
    home_cursor ();		/* Go to the upper left */
    cursor_save ();		/* Save this position */
    cursor_draw ();		/* (UoR) be sure to draw it */
}				/* consetup */

/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/*
 *   Flushio:
 *      Initialize some communications constants, and clear screen and
 *      character buffers. */
/****************************************************************************/
flushio ()
{
    int err;

    err = KillIO (innum);
    if (err)
	printerr ("Bad input clear", err);
    err = KillIO (outnum);
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
int msunit;
{
    long finalticks;

/* delay wants 1/60th units.  We have 3/60 (50 ms.) units, convert */

    msunit = msunit * 3;

    SerSetBrk (outnum);		/* start breaking */
    Delay ((long) msunit, &finalticks);	/* delay */
    SerClrBrk (outnum);		/* stop breaking */
}				/* sendbreak */

/****************************************************************************/
/* toggledtr - Turn DTR off, wait a bit, turn it back on.
 *
 * the argument is in the same units as sendbreak (see above).
 */
/****************************************************************************/
toggle_dtr (msunit)
int msunit;
{
    long finalticks;
    CntrlParam cpb;
    int err;

    if (usingRAMdriver) {
	/* delay wants 1/60th units.  We have 3/60 (50 ms.) units, convert */

	msunit = msunit * 3;

	cpb.csCode = 18;		/* lower DTR */
	cpb.ioCRefNum = outnum;
	err = PBControl (&cpb, FALSE);
	if (err != noErr)
	    printerr ("toggle_dtr() trouble lowering DTR: ", err);

    	Delay ((long) msunit, &finalticks);	/* delay */

	cpb.csCode = 17;		/* raise DTR */
	cpb.ioCRefNum = outnum;
	err = PBControl (&cpb, FALSE);
	if (err != noErr)
	    printerr ("toggle_dtr() trouble raising DTR: ", err);
   }
}				/* sendbreak */

/****************************************************************************/
/* do_xon - xon the output port and send an xon (control-Q) character        */
/****************************************************************************/
do_xon ()
{
    CntrlParam cpb;
    int err;

    if (usingRAMdriver) {
	cpb.csCode = 22;		/* clear XOFF for my output */
	cpb.ioCRefNum = outnum;
	err = PBControl (&cpb, FALSE);
	if (err != noErr)
	    printerr ("do_xon() trouble unblocking output port: ", err);

	cpb.csCode = 24;		/* unconditionally send XON */
	cpb.ioCRefNum = outnum;
	err = PBControl (&cpb, FALSE);
	if (err != noErr)
	    printerr ("do_xon() trouble sending XON: ", err);
   } else {
   	OutputChar ('\021');	/* XON */
   }
}				/* sendbreak */

/*************************************************/
/* cursor drawing stuff                          */
/*************************************************/

Boolean
cursor_rect (line, col, r)
Rect *r;
{
    if (line - display_topline >= screensize)	/* if cursor not on screen */
    	return FALSE;
	
    makerect (r, line - display_topline, col, 1, 1);	/* Get character rectangle */
    if (!blockcursor)
	r->top = r->bottom - 1;
    return TRUE;
}				/* cursor_rect */

void
cursor_draw ()
{
    /*    Line(CHARWIDTH,0);*//* Draw cursor */

    Rect r;

    /* if (cur_drawn) return; */
    if (!cursor_shown) return;		/* (PWP) not if we are hiding cursor */
    
    if (!cursor_invert) {
	if (cursor_rect (curlin, curcol, &r)) {
	    if (in_front) {
		InvertRect (&r);
	    } else {
		PenMode (patXor);
		FrameRect (&r);
		PenMode (patCopy);
	    }
	}
    }

    if ((oldlin >= 0) && (!mousecurs_drawn)) {	/* (UoR) replace mouse cursor */
	makerect (&r, oldlin, oldcol, 1, 1);
	PenMode (patXor);
	FrameRect (&r);
	PenMode (patCopy);
	mousecurs_drawn = TRUE;
    }
    
    cursor_invert = TRUE;
    cur_drawn = TRUE;
}				/* cursor_draw */


void
cursor_erase ()
{
    /*    Line(-CHARWIDTH,0);*//* Erase cursor */

    Rect r;

    /* if (!cur_drawn) return; */

    if (cursor_invert) {
	if (cursor_rect (curlin, curcol, &r)) {
	    if (in_front) {
		InvertRect (&r);
	    } else {
		PenMode (patXor);
		FrameRect (&r);
		PenMode (patCopy);
	    }
	}
    }
    
    if ((oldlin >= 0) && (mousecurs_drawn)) {	/* (UoR) remove mouse cursor */
	makerect (&r, oldlin, oldcol, 1, 1);
	PenMode (patXor);
	FrameRect (&r);
	PenMode (patCopy);
 	mousecurs_drawn = FALSE;
   }

    cursor_invert = FALSE;
    cur_drawn = FALSE;
}				/* cursor_erase */

void
flash_cursor (theWindow)
WindowPtr theWindow;
{
    register long tc;
    Rect r;

    if (theWindow == (WindowPtr) NIL) {
	last_flash = TickCount ();
	return;
    }

    tc = TickCount ();
    if (((tc - last_flash) > MyCaretTime) ||
	(tc - last_flash) < 0L) {
	last_flash = tc;

	if (cur_drawn) {
	    if (cursor_rect (curlin, curcol, &r)) {
		if (blinkcursor && in_front) {	/* PWP: only blink if asked for */
		    InvertRect (&r);
		    cursor_invert = !cursor_invert;
		} else if (!cursor_invert) {	/* make sure that the cursor
						 * shows up */
		    if (in_front) {
			InvertRect (&r);
		    } else {
			PenMode (patXor);
			FrameRect (&r);
			PenMode (patCopy);
		    }
		    cursor_invert = TRUE;
		}
	    }
	}
    }
}				/* flash_cursor */


/****************************************************************************/
/* PWP -- like waitasec(), but don't get any characters.  Used for 
   visable bell. */
/****************************************************************************/
waitnoinput ()
{
    long ticks = 2, end_time;

    Delay (ticks, &end_time);	/* pause for 1/30th second */
}				/* waitnoinput */


/****************************************************************************/
/* (UoR) get any characters, and pause for a while */
/****************************************************************************/
waitasec ()
{
    waitnoinput();
    inpchars ();
}				/* waitasec */


/****************************************************************************/
/* updateCursor -- taken from NCSA Telnet for the Macintosh, v 2.2   */
/****************************************************************************/
void
updateCursor(force, myfrontwindow)
int force;
WindowPeek myfrontwindow;
{
    static Point lastPoint;
    static int optwasdown = 0;
    static Cursor *lastCursor=0L;	/* what we set the cursor to last */
    Cursor *thisCursor;
    int optDown;
    char allthekeys[16];	/* Someplace to put the keymap */
    int newlin, newcol;
    Point MousePt;
    Rect r;

    /* SetPort(myfrontwindow); */
    GetMouse(&MousePt);


    if ( (!force) && (MousePt == lastPoint) && (optDown == optwasdown))
	return;

    if (force)
	lastCursor=0L;
	
    if (protocmd != 0) {	/* if doing a transfer */
	thisCursor = watchcurs;
    } else if (((myfrontwindow == terminalWindow) ||
	    ((myfrontwindow == NIL) &&
	     (FrontWindow() == terminalWindow))) &&
	   PtInRect (&MousePt, &ScreenRect)) {

	GetKeys(allthekeys);
	optDown = allthekeys[7] & 4;	/* should be symbolic */

	if (mouse_arrows || optDown) {
	    newlin = (MousePt.v - TOPMARGIN) / LINEHEIGHT;
	    newcol = (MousePt.h - LEFTMARGIN + CHARWIDTH/2) / CHARWIDTH;

	    if ((oldlin != newlin) || (oldcol != newcol)) {
		PenMode (patXor);	/* For FrameRect calls */
		if (oldlin >= 0) {	/* if old rectangle */
		    if (mousecurs_drawn) {
			makerect (&r, oldlin, oldcol, 1, 1);
			FrameRect (&r);
		    }
		} else {		/* else if drawing for the first time */
		    HideCursor ();
		}
		
		makerect (&r, newlin, newcol, 1, 1);
		FrameRect (&r);
		PenMode (patCopy);	/* reset to normal pen mode */

		oldlin = newlin;
		oldcol = newcol;
 		mousecurs_drawn = TRUE;
	    }
	    lastPoint=MousePt;
	    optwasdown=optDown;
	    
	    return;
	} else {			
	    thisCursor = textcurs;
	}
    } else {
	thisCursor  = normcurs;		/* default cursor shape */
    }
	
    if  (lastCursor!= thisCursor) {
	SetCursor(thisCursor);
	lastCursor = thisCursor;
    }
    lastPoint=MousePt;
    optwasdown=optDown;

    if (oldlin >= 0) {		/* if we hade drawn a movement outline */
	if (mousecurs_drawn) {
	    PenMode (patXor);	/* For FrameRect calls */
	    makerect (&r, oldlin, oldcol, 1, 1);
	    FrameRect (&r);
	}
	
	oldlin = -1;
 	mousecurs_drawn = FALSE;
	ShowCursor ();
	PenMode (patCopy);	/* reset to normal pen mode */
    }
}


/****************************************************************************/
/* Put characters onto the actual screen                                    */
/****************************************************************************/

char outbuf[MAXCOL + 1];
int outcnt = 0, outcol;

static int to_mac_style[] = {
	normal,	underline, italic, underline|italic,
	bold, bold|underline, bold|italic, bold|underline|italic
};

/****************************************************************************/
/* flushbuf() -- draw all the buffered characters on the screen */
/****************************************************************************/
flushbuf ()
{
    register int i;
    Rect r;

    if (outcnt == 0)
	return;			/* Nothing to flush */

    if (to_printer) {					/*JAO*/
	for (i = 0; i < outcnt; i++) {
	    (*hPrintBuffer)[lPrintBufferAt++] = outbuf[i];
	    if (lPrintBufferAt == lPrintBufferSize)
		lPrintBufferAt = 0L;
	    lPrintBufferChars++;
	    if (lPrintBufferChars == lPrintBufferSize) {
		overflowingDialog = GetNewDialog(OVERFLOWINGBOXID, NILPTR, (WindowPtr) - 1);
		DrawDialog(overflowingDialog);
	    }
	}
    }

    if (!to_screen) return;													/*JAO*/

    makerect (&r, curlin, outcol, 1, outcnt);
    EraseRect (&r);

    if (draw_sing_chars) {
	for (i = 0; i < outcnt; i++) {
	    MOVETOCHAR(outcol + i, curlin - display_topline);
	    DrawChar(outbuf[i] & 0377);
	}
    } else {	/* non-blinking */
	MOVETOCHAR(outcol, curlin - display_topline);
	DrawText (outbuf, 0, outcnt);	/* Output the string */
    }
    
    if (textstyle & VT_INVERT) {
	InvertRect (&r);
    }

    outcnt = 0;			/* Say no more chars to output */
}				/* flushbuf */

/****************************************************************************/
/* set_style(style) - set the correct stuff for displaying chars in style   */
/****************************************************************************/
int
set_style(style)
int style;
{
    int m_sty, m_font;
    static int o_sty = 0, o_font = 0;
    Handle theRsrc;
    
    if (style == current_style) return;
    current_style = style;
    
    m_sty = to_mac_style[style & STY_MSTY];
    m_font = ((style & STY_FONT) >> 3) + VT100FONT;
    draw_sing_chars = style & VT_BLINK;
    
    if (!have_128roms && (m_sty & bold)) {	/* if on an old mac and bolding */
	if (m_font = VT100FONT) {
	    m_font = VT100BOLD;
	    m_sty &= ~bold;
	} else {
	    draw_sing_chars = 1;
	}
    }

    if (m_font != o_font) {
	TextFont (m_font);		/* new font */
	o_font = m_font;
    }
    if (m_sty != o_sty) {
	TextFace (m_sty);		/* new text face */
	o_sty = m_sty;
    }
}

/****************************************************************************/
/****************************************************************************/
void
draw_line_attrs(lin, v)
register int lin, v;
{
    register int o, i, sty, j;
    Rect r;
    
    if ((v < 0) || (v > screensize))
	printerr("draw_line_attrs, v out of range:", v);

    sty = scr_attrs[lin][0];
    o = 0;
    i = 0;
    do {
    	if ((scr_attrs[lin][i] != sty) || (i == MAXCOL)) {	/* if this style != current style */
	    makerect (&r, v, o, 1, i-o);
	    EraseRect (&r);

	    set_style(sty);
	    if (draw_sing_chars) {
		for (j = o; j < i; j++) {
		    MOVETOCHAR(j, v);
		    DrawChar(scr[lin][j] & 0377);
		}
	    } else {	/* non-blinking */
		MOVETOCHAR(o, v);
		DrawText (scr[lin], o, i-o);	/* Output this part */
	    }
	    if (sty & VT_INVERT) {
		InvertRect (&r);
	    }
	    o = i;	/* now left extent == current */
	    sty = scr_attrs[lin][i];	/* new current style */
	}
	i++;
    } while (i <= MAXCOL);
}

/****************************************************************************/
/****************************************************************************/
void
scroll_term()
{
    register int new_topline, delta, lin, i;
    int fl, fc, tl, tc;
    Rect r;			/* cannot be register */

    new_topline = screensize - display_totlines + GetCtlValue (t_vscroll);
    if ((new_topline > 0) || (new_topline <  screensize - MAX_SCREENSIZE)) {
    	printerr("BUG: in scroll_term(), new_topline out of range:", new_topline);
	return;
    }
    if ((delta = (display_topline - new_topline)) == 0)
	return;		/* we didn't move */

    makerect (&r, 0, 0, screensize, MAXCOL);
    
    if ((delta >= screensize) || (-delta >= screensize)) {   /* if whole screen */
    	EraseRect(&r);
	
    	lin = new_topline;			/* new top line */
	for (i = 0; i < screensize; i++) {
	    draw_line_attrs(lin, i);
	    lin++;
	}
	display_topline = new_topline;

	if (have_selection)
	    invert_text(from_lin, from_col, to_lin, to_col);
	    
	return;	/* we are done */
    }

    /* if we get here, we are not doing the whole screen */
    ScrollRect (&r, 0, delta * LINEHEIGHT, dummyRgn);

    if (delta > 0) {	/* scrolling down (pushing top arrow) */
    	lin = new_topline;			/* new top line */
	for (i = 0; (i < delta) && (i < screensize); i++) {
	    draw_line_attrs(lin, i);
	    lin++;
	}
	display_topline = new_topline;

	if (have_selection &&
	    (from_lin < display_topline + delta) &&
	    (to_lin >= display_topline)) {
	    if (from_lin < display_topline) {
		fl = display_topline;
		fc = 0;
	    } else {
		fl = from_lin;
		fc = from_col;
	    }
	    if (to_lin >= display_topline + delta) {
		tl = display_topline + delta - 1;
		tc = MAXCOL;
	    } else {
		tl = to_lin;
		tc = to_col;
	    }
	    invert_text(fl, fc, tl, tc);
	}
    } else {		/* scrolling up (pushing bottom arrow) */
    	lin = display_topline + screensize;	/* one past old bottom line */
	/*********** PWP: delta is negative here ****************/
	i = screensize + delta;
	if (i < 0) i = 0;	/* bounds */
	while (i < screensize)
	    draw_line_attrs(lin++, i++);
	display_topline = new_topline;

	if (have_selection &&
	    (from_lin < (display_topline + screensize)) &&
	    (to_lin >= (display_topline + screensize + delta))) {
	    if (from_lin < display_topline + screensize + delta) {
		fl = display_topline + screensize + delta;
		fc = 0;
	    } else {
		fl = from_lin;
		fc = from_col;
	    }
	    if (to_lin >= display_topline + screensize) {
		tl = display_topline + screensize - 1;
		tc = MAXCOL;
	    } else {
		tl = to_lin;
		tc = to_col;
	    }
	    invert_text(fl, fc, tl, tc);
	}
    }
}

void pascal
doscroll (WHICHCONTROL, THECODE)
ControlHandle WHICHCONTROL;
short THECODE;
{
    register int amount = 0, val, max;

    if (THECODE == inUpButton)
	amount = -1;
    if (THECODE == inDownButton)
	amount = 1;
    if (amount == 0)
	return;
    val = GetCtlValue (WHICHCONTROL) + amount;
    max = GetCtlMax (WHICHCONTROL);
    if ((val < 0) || (val > max))
	return;
    SetCtlValue (WHICHCONTROL, val);
    scroll_term ();
}				/* doscroll */


/****************************************************************************/
/* we move the displayed region to the bottom when we recieve characters */
/****************************************************************************/

screen_to_bottom()
{
    if (display_topline != toplin) {
	SetCtlValue (t_vscroll, display_totlines - screensize);
	scroll_term ();
    }
}

/****************************************************************************/
/* update_vscroll - adjust the scaling of the vertical scroll bar, or  */
/*    	      	    disable it if we havn't saved anything back yet */
/****************************************************************************/
update_vscroll ()
{
    if (in_front && display_totlines > screensize) {
	SetCtlMax (t_vscroll, display_totlines - screensize);
	SetCtlValue (t_vscroll, display_totlines - screensize);
	HiliteControl (t_vscroll, 0);
    } else {
	HiliteControl (t_vscroll, 255);
    }
}				/* sizescrollbars */

/****************************************************************************/
/****************************************************************************/
t_pagescroll (code, amount, ctrlh)
ControlHandle ctrlh;
{
    Point myPt;
    register int val, max;

    max = GetCtlMax (ctrlh);
    val = GetCtlValue (ctrlh);
    
    do {
	GetMouse (&myPt);
	if (TestControl (ctrlh, &myPt) != code)
	    continue;
	
	val += amount;
	if (val < 0)
	    val = 0;
	if (val > max)
	    val = max;
	SetCtlValue (ctrlh,  val);
	scroll_term ();
    } while (StillDown ());
}				/* pagescroll */


termmouse(evt)
EventRecord *evt;
{
    int actrlcode;
    long ticks;
    ControlHandle acontrol;
    GrafPtr savePort;
    
    GetPort (&savePort);	/* save the current port */
    SetPort (terminalWindow);

    GlobalToLocal (&evt->where);/* convert to local */
    if (mouse_arrows || (evt->modifiers & optionKey)) {
	if (PtInRect (&evt->where, &ScreenRect)) {	/* In terminal content? */
	    mouse_cursor_move(evt);	    
	    SetPort (savePort);		/* restore previous port */
	    return;			/* yes, do mouse stuff */
	}
    }
    cursor_erase();
    actrlcode = FindControl (&evt->where, terminalWindow, &acontrol);
    switch (actrlcode) {	
      case inUpButton:
      case inDownButton:
	(void) TrackControl (acontrol, &evt->where, doscroll);
	break;

      case inPageUp:
	t_pagescroll (actrlcode, -(screensize/2), acontrol);
	break;

      case inPageDown:
	t_pagescroll (actrlcode, (screensize/2), acontrol);
	break;

      case inThumb:
	(void) TrackControl (acontrol, &evt->where, (ProcPtr) NIL);
	scroll_term ();
	break;
	
      case 0:		/* in the window content itself */
	mouse_region_select(evt);
	break;
    }
    /* MOVETOCHAR(curcol, curlin - display_topline); */
    cursor_draw();
    SetPort (savePort);		/* restore previous port */
}

do_arrow(dir)	/* dir is 'A' (up), 'B' (down), 'C' (right), or 'D' (left) */
unsigned char dir;
{
    OutputChar('\033');		/* ESC */
    if (curskey_mode)
    	OutputChar('O');	/* SS3 */
    else
    	OutputChar('[');	/* CSI */
    OutputChar(dir);
}

do_keypad (n)	/* char to send is n + ',' */
int n;
{
    if (appl_mode) {
    	OutputChar('\033');		/* ESC */
    	OutputChar('O');	/* SS3 */
	OutputChar((unsigned char) n + 'l');
    } else {
    	OutputChar((unsigned char) n + ',');	/* normal digit or glyph */
    }
}

do_pfkey(n)	/* pf1 == 0 ... pf4 == 3 */
int n;
{
    OutputChar('\033');		/* ESC */
    OutputChar('O');	/* SS3 */
    OutputChar((unsigned char) n + 'P');
}

do_keyenter()
{
    if (appl_mode) {
    	OutputChar('\033');		/* ESC */
    	OutputChar('O');	/* SS3 */
	OutputChar('M');
    } else {
    	OutputChar('\015');
    }
}

mouse_cursor_move (evt)
EventRecord *evt;
{
    int mouselin;
    int mousecol;
    int tempcol;
    int templin;
    int i;
    Point MousePt;

    MousePt = evt->where;
    mouselin = (MousePt.v - TOPMARGIN) / LINEHEIGHT;
    mousecol = (MousePt.h - LEFTMARGIN + CHARWIDTH/2) / CHARWIDTH;
    tempcol = curcol;
    templin = curlin;

    if (mousecol < tempcol)
	for (i = tempcol; i > mousecol; i--) {
	    do_arrow (leftARROW);
	    waitasec ();
	    /* If tabs are used, we may go too far, so end loop */
	    if (curcol <= mousecol)
		i = mousecol;
	}

    if (mouselin < templin)
	for (i = templin; i > mouselin; i--) {
	    do_arrow (UPARROW);
	    waitasec ();
	}

    else if (mouselin > templin)
	for (i = templin; i < mouselin; i++) {
	    do_arrow (DOWNARROW);
	    waitasec ();
	}

    if (curlin == mouselin)
	tempcol = curcol;	/* for short lines */

    if (tempcol < mousecol)
	for (i = tempcol; i < mousecol; i++) {
	    do_arrow (rightARROW);
	    waitasec ();
	    /* If tabs are used, we may go too far, so end loop */
	    if (curcol >= mousecol)
		i = mousecol;
	}
}				/* mouse_cursor_move */

invert_text(from_lin, from_col, to_lin, to_col)
{
    Rect r;
    int t;
    
    if (from_lin > to_lin) {		/* make from < to */
    	t = to_lin;
	to_lin = from_lin;
	from_lin = t;
    	t = to_col;
	to_col = from_col;
	from_col = t;
    }
    
    from_lin -= display_topline;	/* convert to screen coords */
    if (from_lin < 0) {
	from_lin = 0;
	from_col = 0;
    }
    if (from_lin >= screensize)
	return;
    to_lin -= display_topline;		/* convert to screen coords */
    if (to_lin < 0)
	return;
    if (to_lin >= screensize) {
	to_lin = screensize-1;
	to_col = MAXCOL;
    }

    if (from_lin == to_lin) {	/* if only one line */
	if (from_col > to_col) {
    	    t = to_col;
	    to_col = from_col;
	    from_col = t;
	}
	if (from_col != to_col)	/* then invert the characters in between */
	    invertchars(from_lin, from_col, 1, to_col - from_col);
    } else {
	if (from_col < MAXCOL)
	    invertchars(from_lin, from_col, 1, MAXCOL - from_col);
	t = to_lin - from_lin - 1;
	if (t > 0)
	    invertchars(from_lin+1, 0, t, MAXCOL);
	if (to_col > 0)
	    invertchars(to_lin, 0, 1, to_col);
    }
}
    
mouse_region_select (evt)
EventRecord *evt;
{
    int mouselin;
    int mousecol;
    int i, shift, sval, smax;
    Point MousePt;

    /* if no selection, then a shift drag is just a drag */
    if (have_selection)
	shift = (evt->modifiers) & shiftKey;
    else
	shift = 0;
	
    /* if not adding to region, remove old one */
    if (!shift && have_selection)
    	invert_text(from_lin, from_col, to_lin, to_col);

    MousePt = evt->where;
    mouselin = (MousePt.v - TOPMARGIN) / LINEHEIGHT + display_topline;
    if (mouselin < display_topline)
	mouselin = display_topline;
    if (mouselin >= display_topline + screensize)
	mouselin = display_topline + screensize-1;
    mousecol = (MousePt.h - LEFTMARGIN + CHARWIDTH/2) / CHARWIDTH;
    if (mousecol < 0) mousecol = 0;
    if (mousecol > MAXCOL) mousecol = MAXCOL;
    
    if (shift) {
	/* swap from_* and to_* if closer to from */
	if (ABS((MAXCOL * from_lin + from_col) - (MAXCOL * mouselin + mousecol)) <
	    ABS((MAXCOL * to_lin + to_col) - (MAXCOL * mouselin + mousecol))) {
    	    i = to_lin;
	    to_lin = from_lin;
	    from_lin = i;
    	    i = to_col;
	    to_col = from_col;
	    from_col = i;
	}
    } else {
	from_lin = mouselin;
	from_col = mousecol;
	to_lin = mouselin;
	to_col = mousecol;
    }
    
    while (StillDown()) {
	GetMouse(&MousePt);
	mouselin = (MousePt.v - TOPMARGIN) / LINEHEIGHT + display_topline;
	mousecol = (MousePt.h - LEFTMARGIN + CHARWIDTH/2) / CHARWIDTH;
	if (mouselin < display_topline) {
	    sval = GetCtlValue (t_vscroll) - 1;
	    smax = GetCtlMax (t_vscroll);
	    if ((sval >= 0) && (sval <= smax)) {
		SetCtlValue (t_vscroll, sval);
		scroll_term ();
	    }
	    mouselin = display_topline;
	    mousecol = 0;
	} else if (mouselin >= display_topline + screensize) {
	    sval = GetCtlValue (t_vscroll) + 1;
	    smax = GetCtlMax (t_vscroll);
	    if ((sval >= 0) && (sval <= smax)) {
		SetCtlValue (t_vscroll, sval);
		scroll_term ();
	    }
	    mouselin = display_topline + screensize-1;
	    mousecol = MAXCOL;
	} else if (mousecol < 0) {
	    mousecol = 0;
	} else if (mousecol > MAXCOL) {
	    mousecol = MAXCOL;
	}
	if ((mousecol != to_col) || (mouselin != to_lin)) {
	    invert_text(to_lin, to_col, mouselin, mousecol);
	    to_lin = mouselin;
	    to_col = mousecol;
	}
    }
    
    if (from_lin > to_lin) {		/* make from < to */
	int t;
	
    	t = to_lin;
	to_lin = from_lin;
	from_lin = t;
    	t = to_col;
	to_col = from_col;
	from_col = t;
    } else if (from_lin == to_lin) {
    	if (from_col > to_col) {
	    int t;
	    
	    t = to_col;
	    to_col = from_col;
	    from_col = t;
	}
    }
    
    if ((from_lin != to_lin) || (from_col != to_col))
	have_selection = TRUE;
    else
	have_selection = FALSE;
}

/* (PWP) if the selection is within [tlin,blin], then remove it */

maybe_nuke_selection(tlin, blin)
int tlin, blin;
{
    if (have_selection &&
	((from_lin < tlin) || (to_lin > blin)) &&
    	((to_lin > tlin)   || (from_lin < blin))) {
	have_selection = FALSE;
	invert_text(from_lin, from_col, to_lin, to_col);
    }
}

/* copy the current selction to the (internal) clipboard */
scr_copy()
{
    int lin, i, rcol;
    long sz;
    char *dp;
    
    if (myclip_h == NIL) {
	printerr("scr_copy: clip handle not allocated", 0);
	return;
    }
    
    if (have_selection) {
	/****** find out how big the text to copy is ******/
    	if (from_lin == to_lin) {
	    sz = to_col - from_col + 1;
	} else {
	    for (rcol = MAXCOL; rcol > from_col; rcol--)	/* first */
	    	if (scr[from_lin][rcol-1] != ' ')
		    break;
	    sz = rcol - from_col + 1;	/* chars plus one for the newline */
	    for (lin = from_lin+1; lin < to_lin; lin++) {	/* in between */
		for (rcol = MAXCOL; rcol > 0; rcol--)
		    if (scr[lin][rcol-1] != ' ')
			break;
		sz += rcol + 1;	/* chars plus one for the newline */
	    }
	    for (rcol = to_col; rcol > 0; rcol--)		/* last */
	    	if (scr[to_lin][rcol-1] != ' ')
		    break;
	    sz += rcol;		/* chars */
	    if (to_col >= MAXCOL)
	    	sz++;
	}

	/****** allocate and lock a buffer for the text ******/
	if (sz > GetHandleSize ((Handle) myclip_h)) {
	    HUnlock((Handle) myclip_h);
	    SetHandleSize((Handle) myclip_h, sz);
	}
	HLock((Handle) myclip_h);
	dp = *myclip_h;
	
	/****** copy the characters over to the clip ******/
    	if (from_lin == to_lin) {
	    for (i = from_col; i < to_col; i++)
	    	*dp++ = scr[from_lin][i];
	} else {
	    /* trim off spaces */
	    for (rcol = MAXCOL; rcol > from_col; rcol--)	/* first */
		if (scr[from_lin][rcol-1] != ' ')
		    break;
	    for (i = from_col; i < rcol; i++)
		*dp++ = scr[from_lin][i];
	    *dp++ = '\r';
	    for (lin = from_lin+1; lin < to_lin; lin++) {	/* in between */
		for (rcol = MAXCOL; rcol > 0; rcol--)
		    if (scr[lin][rcol-1] != ' ')
			break;
		for (i = 0; i < rcol; i++)
		    *dp++ = scr[lin][i];
		*dp++ = '\r';
	    }
	    for (rcol = to_col; rcol > 0; rcol--)		/* last */
		if (scr[to_lin][rcol-1] != ' ')
		    break;
	    for (i = 0; i < rcol; i++)
		*dp++ = scr[to_lin][i];
	    if (to_col >= MAXCOL)
		*dp++ = '\r';
	}
	myclip_size = (dp - *myclip_h);

	/****** check to make sure we didn't overflow the clipboard ******/
	if (myclip_size > sz)
	    fatal ("Overflow! myclip_size - sz ==", myclip_size - sz);

	/****** We are done.  Unlock the handle ******/
	HUnlock((Handle) myclip_h);
    } else {
	SysBeep(3);
    }
}

/* paste the clipboard into the terminal, by "typing" it in */
scr_paste()
{
    char *cp, *endp;
    
    if (myclip_size > 0) {
	HLock((Handle) myclip_h);
	cp = *myclip_h;
	endp = cp + myclip_size;
	for (; cp < endp; cp++) {
	    OutputChar(*cp);
	    if (*cp == '\r')
		waitasec ();
	}
    } else {
	SysBeep(3);
    }
}


/****************************************************************************/
/****************************************************************************/

#ifdef COMMENT
show_inval_rgn(w)
WindowPeek w;
{
    RgnHandle r = NewRgn();

    CopyRgn (w->updateRgn, r);
    OffsetRgn(r,			/* convert to local grafport coords */
	      (((w->port).portBits).bounds).left,
	      (((w->port).portBits).bounds).top);
    FillRgn(r, qd.black);
    DisposeRgn(r);
}
#endif /* COMMENT */

/****************************************************************************/
/*
 * (UoR)
 *
 * Scroll lines within the scroll region upwards from line tlin
 * to line blin (lines are assumed to be in the region)
 *
 * (PWP) scroll_screen is the combination of scroll_up and scroll_down.
 *       dir is the number of lines to scroll, <0 if up, >0 if down.
 *	 (actually, right now only -1 and 1 are handled.)
 */
/****************************************************************************/
void
scroll_screen (tlin, blin, dir)		/* these are in scr[][] cordinates */
register int tlin;
register int blin;
register int dir;
{
    register int i, now;
    int abstop, absbot;
    char *savedline, *savedattr;  /* temporary to hold screen line pointer */
    Rect r, opened_r;		/* cannot be register */
    Point save_pt;
    GrafPtr currWindow;		/* cannot be register */
    RgnHandle newupdateRgn;

    abstop = tlin;	/* really tlin - display_topline, but we are at bottom */
    absbot = blin;	/* so display_topline == 0 */
    makerect (&r, abstop, 0, absbot - abstop + 1, MAXCOL);
    if (dir < 0)
	makerect (&opened_r, absbot, 0, 1, MAXCOL);
    else
	makerect (&opened_r, abstop, 0, 1, MAXCOL);

    /* (PWP) if our selected region overlaps, but is not enclosed by the region
       we want to scroll, then remove it, because the region no longer contains
       what the user thought it did. */
    if (have_selection && (tlin != toplin) && (blin != botlin) &&
	((from_lin < tlin) || (to_lin > blin)) &&
    	((to_lin > tlin)   || (from_lin < blin))) {
	have_selection = FALSE;
	invert_text(from_lin, from_col, to_lin, to_col);
    }
    if (!in_front) {
	/* compensate update region for scrolling */
	GetPort (&currWindow);
	/* scroll the old updateRgn */
	OffsetRgn (((WindowPeek) currWindow)->updateRgn, 0, -LINEHEIGHT);
    }
    
    /* do the scrolling */
    newupdateRgn = NewRgn();
    if (smoothscroll && in_front) {
	for (i = 1; i <= LINEHEIGHT; i += 1) {
	    /* PWP: wait for a vertical reblank (in a sneaky way) */
	    now = TickCount ();
	    while (TickCount () == now)
		/* wait... */ ;
	    if (dir < 0)
		ScrollRect (&r, 0, -1, newupdateRgn);
	    else
		ScrollRect (&r, 0, 1, newupdateRgn);
	}
    } else {
	ScrollRect (&r, 0, dir * LINEHEIGHT, newupdateRgn);
	if (!in_front) {
	    InvalRgn(newupdateRgn);
	    ValidRect(&opened_r);
	}
    }
    DisposeRgn(newupdateRgn);
    
    if (dir < 0) {
	/* adjust the internal character buffers */
	if ((tlin == toplin) && (blin == botlin)) {	/* if whole screen */
	    display_totlines++;
	    if (display_totlines > MAX_SCREENSIZE)
		display_totlines = MAX_SCREENSIZE;	/* bounds */
    	    tlin = screensize - display_totlines;	/* top of saved buffer */
	}
 	savedline = scr[tlin];
	savedattr = scr_attrs[tlin];
	for (i = tlin+1; i <= blin; i++) {
    	    scr[i-1] = scr[i];
    	    scr_attrs[i-1] = scr_attrs[i];
	}
	scr[blin] = savedline;
	scr_attrs[blin] = savedattr;
    
	zeroline (blin);		/* clear the line */

	/* adjust selection */
	if (have_selection && (from_lin >= tlin) && (to_lin <= blin)) {
	    from_lin--;
	    to_lin--;
	}
    } else {
        /* adjust the internal buffers */
	savedline = scr[blin];
	savedattr = scr_attrs[blin];
	for (i = blin-1; i >= tlin; i--) {
    	    scr[i+1] = scr[i];
    	    scr_attrs[i+1] = scr_attrs[i];
	}
	scr[tlin] = savedline;
	scr_attrs[tlin] = savedattr;

	zeroline (tlin);

	/* adjust selection */
	if (have_selection && (from_lin >= tlin) && (to_lin <= blin)) {
	    from_lin++;
	    to_lin++;
	}
    }
}				/* scroll_up */

/****************************************************************************/
/* redraw the terminal screen (we got a redraw event) */
/****************************************************************************/
term_redraw ()
{
    int i, lin;
    Rect r;

    BackPat(qd.white);
    PenPat(qd.black);

    r = terminalWindow->portRect;	/* copy the window size */
    /* r.right -= 15;	*/		/* subtract control */
    /* makerect (&r, 0, 0, screensize, MAXCOL); */   /* PWP: clear the screen first */
    EraseRect (&r);

    DrawGrowIcon (terminalWindow);
    DrawControls (terminalWindow);
    /* erase the bottom scroll line */
    PenMode(patBic);
    MoveTo(0, (terminalWindow->portRect).bottom - 15);
    LineTo((terminalWindow->portRect).right - 16,
    	(terminalWindow->portRect).bottom - 15);
    PenNormal();

    /* update_vscroll(); */
    /* SetCtlValue (t_vscroll, GetCtlValue (t_vscroll)); */

    lin = display_topline;
    for (i = 0; i < screensize; i++) {
	draw_line_attrs(lin, i);
	lin++;
    }

    if (have_selection)
    	invert_text(from_lin, from_col, to_lin, to_col);

    if (cur_drawn && cursor_invert) {	/* (UoR) only if cursor is showing */
	cursor_invert = FALSE;	/* (UoR) make sure we draw it */
	cursor_draw ();		/* redraw cursor */
	last_flash = TickCount ();	/* (UoR) reset timer */
    }

}				/* term_redraw */

term_activate(hilite)
int hilite;
{
    cursor_erase ();		/* remove cursor from screen */
    in_front = hilite;
    if (in_front) {
	HiliteControl (t_vscroll, 0);
	UpdateOptKey(1);
    } else {
	HiliteControl (t_vscroll, 255);
	UpdateOptKey(0);
    }
    cursor_draw ();
}


/****************************************************************************/
/* initalize the terminal emulator. */
/****************************************************************************/
init_term ()
{
    register int i, j;
    char *cp;
    
    topmargin = TOPMARGIN;	/* Edges of adjustable window */
    bottommargin = bottomMARGIN;
    
    if ((cp = (char *)NewPtr(((long)(MAXCOL+1) * (long) MAX_SCREENSIZE))) == NIL)
	fatal("Could not allocate screen buffer", 0);
	
    for (i = 0; i < MAX_SCREENSIZE; i++) {
    	real_scr[i] = cp + (i * (MAXCOL+1));	/* divvy up screen buffer */

	for (j = 0; j < MAXCOL; j++)
	    real_scr[i][j] = ' ';
	real_scr[i][MAXCOL] = '\0';	/* Terminate the lines as strings */

    	if ((real_attrs[i] = (char *)NewPtr((long)(MAXCOL+1))) == NIL)
	    fatal("Could not allocate screen attribute line", i);

	for (j = 0; j < MAXCOL; j++)
	    real_attrs[i][j] = 0;
	real_attrs[i][MAXCOL] = '\0';	/* Terminate the lines as strings */
    }
    scr = &real_scr[MAX_SCREENSIZE - screensize];
    if (scr[0] == NIL)
	fatal("init_term: scr assignment botched for [0]", 0);
    if (scr[screensize-1] == NIL)
	fatal("init_term: scr assignment botched for [screensize-1]", 0);

    scr_attrs = &real_attrs[MAX_SCREENSIZE - screensize];
    if (scr_attrs[0] == NIL)
	fatal("init_term: scr assignment botched for [0]", 0);
    if (scr_attrs[screensize-1] == NIL)
	fatal("init_term: scr assignment botched for [screensize-1]", 0);
    
    scrtop = toplin;		/* Scrolling region equals all */
    scrbot = botlin;
    display_topline = toplin;	/* init display w/elevator at bottom */
    display_totlines = screensize;
    makerect (&ScreenRect, 0, 0, screensize, MAXCOL);
    /* (UoR) full screen rectangle */
    
    SizeWindow(terminalWindow,
    	rightMARGIN + 1 + 16,     /* add extra to side for asthetics */
	bottomMARGIN + TOPMARGIN,     /* add extra to bottom for asthetics */
	FALSE);
    /* PWP: make the window match it's real size */
    
    DrawGrowIcon (terminalWindow);
    
    /* erase the bottom scroll line */
    PenMode(patBic);
    MoveTo(0, (terminalWindow->portRect).bottom - 15);
    LineTo((terminalWindow->portRect).right - 16,
    	(terminalWindow->portRect).bottom - 15);
    PenNormal();
    
    t_vscroll = GetNewControl (RCMDVSCROLL, terminalWindow);
    sizevscroll();

    InitKeyStuff();		/* find the original KCHR keymaps */

    /* ClipRect(&ScreenRect); */
}				/* init_term */

/****************************************************************************/
/* sizevscroll - called when window is created and after a window grow */
/*    	      	    sequence to resize the scroll window's bars. */
/****************************************************************************/
sizevscroll ()
{
    register Rect *r;

    r = &terminalWindow->portRect;/* window size */
    HideControl (t_vscroll);

    MoveControl (t_vscroll, r->right - 15, r->top - 1);
    SizeControl (t_vscroll, 16, r->bottom - r->top - 13);

    SetCtlMin (t_vscroll, 0);
    update_vscroll();
    ShowControl (t_vscroll);
}				/* sizescrollbars */

/****************************************************************************/
/* grow_term_to(size) -- change the size of the terminal window to size.
   this is called by growterm() (see below) and the terminal settings dialog
   handler (termsetdialog()).
/****************************************************************************/
grow_term_to (size)
int size;
{
    long gr;
    int height;
    int width;
    char *savedline;
    int i, j, d;
    GrafPtr savePort;

    if ((size < 1) || (size > MAX_SCREENSIZE))
       size = 24;	/* the default case */
    
    if (size > display_totlines) {	/* if getting bigger than we were */
    	for (i = screensize - display_totlines, j = screensize - size;
	     i < screensize; i++, j++) {
	    savedline = scr[i];		/* save used line */
	    scr[i] = scr[j];		/* copy empty line to old line */
	    scr[j] = savedline;		/* copy old line to new line */
	    savedline = scr_attrs[i];	/* do the same for attribute lines */
	    scr_attrs[i] = scr_attrs[j];
	    scr_attrs[j] = savedline;
	}
	display_totlines = size;
    } else {
	curlin += size - screensize;	/* adjust cursor row to match stretch */
	if (curlin < 0)
	    curlin = 0;
	if (curlin > size-1)
	    curlin = size-1;
    }
    
    screensize = size;
    if (screensize > MAX_SCREENSIZE)
    	screensize = MAX_SCREENSIZE;		/* bounds check */

    scr = &real_scr[MAX_SCREENSIZE - screensize];
    scr_attrs = &real_attrs[MAX_SCREENSIZE - screensize];
    
    bottommargin = bottomMARGIN;	/* this changes */
    
    scrtop = toplin;		/* Scrolling region equals all */
    scrbot = botlin;
    display_topline = 0;	/* re-init display w/elevator at bottom */
    makerect (&ScreenRect, 0, 0, screensize, MAXCOL);
    /* (UoR) full screen rectangle */
    
    SizeWindow(terminalWindow,
    	rightMARGIN + 1 + 16,     /* add extra to side for asthetics */
	bottomMARGIN + TOPMARGIN,     /* add extra to bottom for asthetics */
	FALSE);
    /* PWP: make the window match it's real size */
    sizevscroll ();	/* size the scroll bars */

    /* ClipRect(&ScreenRect); */

    GetPort (&savePort);
    SetPort (terminalWindow);
    InvalRect (&terminalWindow->portRect);	/* invalidate whole window rectangle */
    SetPort (savePort);
}				/* growwindow */

/****************************************************************************/
/****************************************************************************/
growterm (p)
Point *p;
{
    long gr;
    int height;
    int width;
    int size;
    Rect growRect;
    GrafPtr savePort;

    growRect = qd.screenBits.bounds;
    growRect.top = 50;		/* minimal horizontal size */
    growRect.left = rightMARGIN + 18;	/* minimal vertical size */
    growRect.right = rightMARGIN + 18;	/* minimal vertical size */

    gr = GrowWindow (terminalWindow, p, &growRect);

    if (gr == 0)
	return;
    height = HiWord (gr);
    width = LoWord (gr);

    size = (height - (2 * TOPMARGIN)) / LINEHEIGHT;
    if (size > MAX_SCREENSIZE)
    	screensize = MAX_SCREENSIZE;		/* bounds check */
    if (size < 1)
    	size = 1;

    grow_term_to(size);
}				/* growterm */
