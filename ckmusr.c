/* edit by PWP 3/27/88 -- Make the log session and transaction stuff a   */
/*  separate menu. */
/* edits by PWP -- Nov. 87..Mar. 88  Fixed several oversights, bugs, etc., */
/* added MultiFinder support */
/* Version 0.9(36) - Matthias Aebi, ECOFIN Research and Consulting, Ltd., */
/*  Oct 1987 ported to MPW C */
/* Version 0.8(35) - Jim Noble at Planning Research Corporation, June 1987. */
/* Ported to Megamax native Macintosh C compiler. */
/* DPVC at U of R, on Oct 1, to do blinking cursor and mouse cursor movement */
/* Edit by Bill, Jeff, and Howie on Jun 13 */
/* Add select window, drag */
/* Edit by WBC3 on Apr 29 */
/* Don't supply a second name for send as, otherwise file name translation */
/* will not occur.  Let user type it if they want. */
/* Edit by WBC3 on Apr 23 */
/* Make typein to the emulator only take effect when it's the "front window" */
/* Edit by WBC3 on Apr 23 */
/* Make only Command-. stop protocol rather then anything but Command-. */
/* Edit by WBC3 on Apr 22 */
/* Make debug and tlog only show up if DEBUG is defined */
/* Edit by Bill on Apr 21 17:51 */
/* In Screen depelete event queue instead of handling one event at */
/* a time, also don't loose events */

/*
 * file ckmusr.c
 *
 * Module of mackermit containing code for the menus and other MacIntosh
 * things.
 *
 */

/*
 Copyright (C) 1985, Trustees of Columbia University in the City of New York.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained.
*/

#include "ckcdeb.h"
#include "ckcker.h"

#define	__SEG__ ckmusr
#include <desk.h>
#include <files.h>
#include <windows.h>
#include <events.h>
#include <dialogs.h>
#include <fonts.h>
#include <menus.h>
#include <toolutils.h>
#include <serial.h>
#include <textedit.h>
#include <segload.h>
#include <ctype.h>
#include <environs.h>
#include <osutils.h>
/* PWP: put the #include for the script manager here! */

#include "ckmdef.h"		/* General Mac defs */
#include "ckmres.h"		/* Mac resource equates */
#include "ckmasm.h"		/* new A8 and A9 traps */

/* Global Variables */

MenuHandle menus[MAX_MENU + 1];	/* handle on our menus */

short innum;			/* Input driver refnum */
short outnum;			/* Output driver refnum */
int protocmd;			/* protocol file cmd, or -1 for */
 /* remote cmds, or 0 if not in */
 /* protocol */

char *mybuff;			/* Serial drivers new buffer */
CursHandle watchcurs;		/* the watch cursor */
SerShk controlparam;		/* To change serial driver paramaters */

int quit = FALSE;

Boolean mcmdactive = TRUE;	/* Enable menu command keys */
Boolean fkeysactive = TRUE;	/* Enable FKEYs */

WindowPtr terminalWindow;	/* the terminal window */
extern WindowPtr remoteWindow;	/* the remote command window */


/* local variables */

Boolean have_multifinder = FALSE; /* becomes true if we are running MF */
Boolean in_background = FALSE;	/* becomes TRUE if have_multifinder and
				 * we have recieved a "suspend" event */

long mf_sleep_time = 3L;	/* this is the number of (60Hz) ticks to
				 * sleep before getting a nullEvent (to
				 * flash our cursor) (and input chars from
				 * the serial line)
				 */

/****************************************************************************/
/*
 *  p a r s e r
 *
 *  Entry point for kermit.
 *  Establish a virtual terminal connection with the remote host and
 *  process mouse events, such as selecting items from the menu bars.
 *
 */
/****************************************************************************/
char
parser ()
{
    char menu_event ();
    char domouse ();
    char nextcmd ();
    char rstate = 0;
    EventRecord myevent;
    extern int oldlin;		/* used in mouse --> cursor stuff */

    if (tlevel > -1) {		/* if we are working under take-file */
	rstate = nextcmd ();	/* file control, get next command */
	if (!quit)
	    return (rstate);
    }
    InitCursor ();		/* back to normal cursor */
    protocmd = 0;		/* protocol not active */

    while (!(quit || rstate)) {	/* Until they want to quit */
	/* or return */
	if (!have_multifinder)	/* MF does this for us */
	    SystemTask ();	/* Update system things */

	inpchars ();		/* Handle all the pending port chars */

	if (blinkcursor)	/* PWP: save some time */
	    flash_cursor (terminalWindow);	/* (UoR) for flashing cursor */
	if (mouse_arrows || (oldlin >= 0))	/* PWP: save more time */
	    check_pointer (terminalWindow);	/* (UoR) check for mouse
						 * cursor pointer */

	if (have_multifinder)
	    /* task, task */
	    WaitNextEvent (everyEvent, &myevent, mf_sleep_time, NULL);
	else
	    GetNextEvent (everyEvent, &myevent);

	switch (myevent.what) {	/* events */
	  case keyDown:	/* Handle char input */
	  case autoKey:	/* both ways */
	    if ((myevent.modifiers & cmdKey) && (mcmdactive)) {
		rstate = menu_event (MenuKey (myevent.message & charCodeMask));
		HiliteMenu (0);	/* Done, so un-hilite */
	    } else if (FrontWindow () == terminalWindow)
		handle_char (&myevent);
	    break;

	  case updateEvt:	/* A window update event */
	    doupdate ((WindowPtr) myevent.message);	/* Handle update */
	    break;

	  case activateEvt:	/* (de)active a window */
	    doactivate ((WindowPtr) myevent.message, myevent.modifiers);
	    break;

	  case mouseDown:	/* Mouse event */
	    rstate = domouse (&myevent);
	    break;

	  case app4Evt:	/* really a suspend/resume event */
	    if (have_multifinder)
		in_background = !in_background;
	    break;
	}
    }

    if (quit)			/* want to exit the program? */
	doexit (0);		/* yes, finish up */

    SetCursor (*watchcurs);	/* set the watch */
    return (rstate);		/* and return with state */
}				/* parser */



/****************************************************************************/
/* miniparser - called during protocol to handle events.  Handles dialog, */
/*    	      	update, and keydown (the abort key) events.  Ignores */
/*    	      	all other events.  The dialog events are assumed to */
/*    	      	be for screen (status) display. */
/****************************************************************************/
miniparser (deplete)		/* deplete pending events */
{
    EventRecord ev;
    DialogPtr mydlg;
    short item;

    do {
	if (have_multifinder) {
	    /* task, task */
	    WaitNextEvent (everyEvent, &ev, mf_sleep_time, NULL);
	} else {
	    SystemTask ();	/* let the system move */
	    GetNextEvent (everyEvent, &ev);	/* get an event */
	}

	if (ev.what == nullEvent)
	    return;		/* depleted pending events */

	if (IsDialogEvent (&ev))/* meant for dialog manager? */
	    /* must be for screen */
	    if (DialogSelect (&ev, &mydlg, &item))
		scrmydlg (item);/* let him handle it */

	switch (ev.what) {	/* we may need to do something */
	  case updateEvt:
	    doupdate ((WindowPtr) ev.message);	/* handle updates */
	    break;

	  case keyDown:
	    if ((ev.modifiers & cmdKey) && ((ev.message & 0x7f) == '.'))
		if (CautionAlert (ALERT_ABORT, NILPROC) == OKBtn)
		    sstate = 'a';	/* move into abort state */
	    break;

	  case app4Evt:	/* really a suspend/resume event */
	    if (have_multifinder) {
		in_background = !in_background;
		/* PWP: do stuff for disabling the status window here */
	    }
	    break;

	}
    } while (deplete);		/* return now, or loop */
}				/* miniparser */



/****************************************************************************/
/* domouse  - handle mouse down events for different windows. */
/****************************************************************************/
char
domouse (evt)
EventRecord *evt;
{
    char state = 0;
    WindowPtr window;
    int evtwcode;

    evtwcode = FindWindow (&evt->where, &window);

    switch (evtwcode) {		/* Tell us where */
      case inMenuBar:		/* Looking at the menus? */
	check_pointer ((WindowPtr) NIL);	/* (UoR) mouse cursor off */
	state = menu_event (MenuSelect (&evt->where));
	HiliteMenu (0);		/* Done, so un-hilite */
	break;			/* All done */

      case inSysWindow:	/* Looking at system, like */
	SystemClick (evt, window);	/* a desk accessary */
	break;			/* Let the system handle it */

      case inContent:
	if (window != FrontWindow ())
	    SelectWindow (window);	/* make window current */
	else if (window == terminalWindow)
	    mouse_cursor_move (evt);	/* (UoR) check for mouse cursor move */
	else if (window == remoteWindow)
	    rcdmouse (evt);
	break;

      case inDrag:		/* Wanna drag? */
	DragWindow (window, &evt->where, &qd.screenBits.bounds);
	break;

      case inGoAway:
	if (TrackGoAway (remoteWindow, &evt->where))
	    rcmdwhide ();	/* put it away */
	break;

      case inGrow:
	growwindow (remoteWindow, &evt->where);
	break;
    }

    flash_cursor ((WindowPtr) NIL);	/* (UoR) reset timer for flashes */
    return (state);		/* return with new state or 0 */

}				/* domouse */



/****************************************************************************/
/* doupdate - handle update event on different windows, dispatch to */
/*    	      redraw routines */
/****************************************************************************/
doupdate (window)
WindowPtr window;
{
    GrafPtr savePort;

    GetPort (&savePort);
    SetPort (window);
    BeginUpdate (window);

    if (window == terminalWindow)
	term_redraw ();		/* terminal window */
    else if (window == remoteWindow)
	rcdupdate (window);	/* Redraw remote command window */

    EndUpdate (window);
    SetPort (savePort);
}				/* doupdate */



/****************************************************************************/
/* doactivate - activate a window */
/****************************************************************************/
doactivate (window, mod)
WindowPtr window;
int mod;
{
    HiliteWindow (window, ((mod & activeFlag) != 0));
    if (window == remoteWindow)
	rcdactivate (mod);
}				/* doactivate */



char genstr[100];

/****************************************************************************/
/****************************************************************************/
char
menu_event (menu_item)
long menu_item;
{
    short menu = HiWord (menu_item);	/* decompose arg */
    short item = LoWord (menu_item);
    char state = '\0';
    int remotedialog ();	/* returns boolean */

    switch (menu) {
      case APPL_MENU:		/* Mac system menu item */
	handapple (item);
	break;			/* all done */

      case FILE_MENU:
	switch (item) {		/* Find out which was selected */
	  case QUIT_FIL:	/* Want to quit program? */
	    quit = TRUE;	/* Yes... flag it */
	    break;
	  case XFER_FIL:
	    handlelaunch ();	/* Handle x-fer to application */
	    break;

	  case SAVE_FIL:	/* save settings */
	    savevals ();	/* save em */
	    break;

	  case LOAD_FIL:	/* load settings */
	    loadvals ();
	    break;

	  case SEND_FIL:	/* send a file: local, remote files */
	    if (!dosenddialog (&cmarg, &cmarg2))
		break;
	    nfils = -1;		/* Use cmarg, not cmlist */
	    state = 's';	/* return with send state */
	    break;

	  case RECV_FIL:	/* Ask for recv info -- */
	    initfilrecv ();	/* init recv flags */
	    state = 'v';	/* return with recv state */
	    break;

	  case GETS_FIL:	/* Get from server */
	    if (dogetfdialog (&cmarg))	/* remote-file. */
		state = 'r';	/* Say we want to get */
	    break;
	}

	if (state != '\0') {	/* going to enter protocol? */
	    protocmd = item;	/* yes, remember how */
	    scrcreate ();	/* create the status screen */
	}
	break;

      case EDIT_MENU:		/* PWP: good for DA editors */
	if (SystemEdit (item - 1))
	    break;
	SysBeep (3);
	break;

      case SETG_MENU:
	switch (item) {
	  case PROT_SETG:
	    protodialog ();
	    break;

	  case COMM_SETG:
	    commdialog ();	/* communications dialog */
	    break;

	  case FILE_SETG:
	    setfiledialog ();	/* do default file settings */
	    break;

	  case TERM_SETG:	/* do terminal emulation settings */
	    termsetdialog ();
	    break;

	  case SCRD_SETG:	/* fkeys  active / not active */
	    CheckItem (menus[SETG_MENU], SCRD_SETG,
		       (fkeysactive = !fkeysactive));
	    ScrDmpEnb = (fkeysactive) ? scrdmpenabled : scrdmpdisabled;
	    break;

	  case MCDM_SETG:	/* menu command keys active / not active */
	    CheckItem (menus[SETG_MENU], MCDM_SETG,
		       (mcmdactive = !mcmdactive));
	    break;

	  case KEYM_SETG:
	    keymacros ();
	    break;

	  case MODF_SETG:
	    keymoddialog ();
	    break;
	}
	break;

	/*
	 * return either 'g' for generic or 'c' for host with cmarg holding
	 * cmd
	 */

      case REMO_MENU:
	cmarg = genstr;		/* indicate cmd ok to proceed */
	switch (item) {
	  case RESP_REMO:	/* Want to toggle display */
	    togglercmdw ();	/* Do it */
	    cmarg = NILPTR;	/* Don't do anything else */
	    break;

	  case FIN_REMO:
	    setgen (genstr, 'F', "", "", "");	/* Finish */
	    break;

	  case BYE_REMO:
	    setgen (genstr, 'L', "", "", "");	/* Bye, logout */
	    break;

	  case SERV_REMO:
	    displa = 1;
	    scrcreate ();	/* create the status screen */
	    protocmd = item;
	    return ('x');

	  default:
	    if (!remotedialog (item, genstr))
		cmarg = NILPTR;	/* cancel issued, prevent it */
	    break;
	}

	if (cmarg == NILPTR)
	    break;		/* Gave up on this command? */
	protocmd = -1;		/* hey we're going to protocol! */
	if (item == HOST_REMO)	/* Remote host? */
	    state = 'c';	/* yes, return it to driver */
	else
	    state = 'g';	/* else generic */
	break;
	
      case LOG_MENU:
	switch(item) {
	  case SLOG_LOG:	/* session logging */
	    if (seslog) {
		closeslog ();
		seslog = 0;
	    } else {
		seslog = openslog ();
	    }
	    CheckItem (menus[LOG_MENU], SLOG_LOG, seslog);
	    if (seslog)
	    	EnableItem(menus[LOG_MENU], SDMP_LOG);
	    else
	    	DisableItem(menus[LOG_MENU], SDMP_LOG);
	    break;

	  case SDMP_LOG:	/* dump screen to session log */
	    if (seslog)	{	/* session logging active? */
		scrtolog ();	/* if the file is open, just do the dump */
	    } else {
		SysBeep(3);	/* can't dump screen without a log file. */
	    	DisableItem(menus[LOG_MENU], SDMP_LOG);
	    }
	    break;

#ifdef TLOG
	  case TLOG_LOG:	/* transaction logging */
	    if (tralog) {
		closetlog ();
		tralog = 0;
	    } else {
		tralog = opentlog ();
	    }
	    CheckItem (menus[LOG_MENU], TLOG_LOG, tralog);
	    break;
#endif TLOG
	}
	break;

    }
    return (state);		/* Don't go into Kermit protocol */
}				/* menu_event */


/****************************************************************************/
/****************************************************************************/
herald ()
{
}				/* herald */



/****************************************************************************/
/****************************************************************************/
conect ()
{
	/* we already ARE connected */
}				/* conect */


/****************************************************************************/
/****************************************************************************/
cmdlin ()
{
    return (0);			/* nothing parsed */
}				/* cmdlin */



/****************************************************************************/
/****************************************************************************/
chkint ()
{
}				/* chkint */



#ifdef DEBUG
/****************************************************************************/
/****************************************************************************/
debug (flags, str, str2, num)
int flags;
int num;
char *str;
char *str2;
{
    char numbuf[10];

    /* if (!debflg) return; */

    switch (flags) {
      default:
	NumToString ((long) num, numbuf);
	ParamText (str, str2, numbuf, "");
	CautionAlert (ALERT_DEBUG, NILPROC);
	break;
    }
}				/* debug */

#endif



/****************************************************************************/
/* sleep - called during protocol for a dismiss.  Keep machine running */
/*    	   with calls to the miniparser during this period. */
/****************************************************************************/
sleep (secs)
{
    long finalticks;		/* tickscount for exit */

    finalticks = TickCount () + (60 * secs);	/* TickCount for exit  */
    while (finalticks > TickCount ()) {	/* keep the machine running by */
	miniparser (TRUE);	/* deplete the Q */
	if (sstate == 'a')	/* if in abort state, forget */
	    return;		/* this wait */
    }
}				/* sleep */



/****************************************************************************/
/****************************************************************************/
ermsg (msg)			/* Print error message */
char *msg;
{
    screen (SCR_EM, 0, 0l, msg);/* Put the error on the screen */
    tlog (F110, "Error -", msg, 0l);
}				/* ermsg */



/****************************************************************************/
/****************************************************************************/
intmsg (n)
long n;
{
    return;
}				/* intmsg */


