/* John A. Oberschelp for Emory University -- vt102 printer support 22 May 1989 */
/*                    Emory contact is Peter W. Day, ospwd@emoryu1.cc.emory.edu */ 
/* Paul Placeway 4/89    - fixed up things for profiling, minor changes */
/* Paul Placeway 3/28/88 - created by moving a bunch of junk out of ckmusr.c */
/*
 * file ckmini.c
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

#define	__SEG__ ckmini
#include <desk.h>
#include <files.h>
#include <windows.h>
#include <events.h>
#include <dialogs.h>
#include <fonts.h>
#include <menus.h>
#include <Memory.h>
#include <Resources.h>
#include <Traps.h>
#include <toolutils.h>
#include <devices.h>
#include <serial.h>
#include <textedit.h>
#include <segload.h>
#include <ctype.h>

extern	Handle	hPrintBuffer;					/*JAO*/
#include <printing.h>						/*JAO*/

/* here is what is different */
#ifndef __QUICKDRAW__
#include <QuickDraw.h>
#endif
#include <osutils.h>

/* PWP: put the #include for the script manager here! */

#ifdef PROFILE
#include <Perf.h>
TP2PerfGlobals ThePGlobals = nil;
#endif /* PROFILE */

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

Cursor *watchcurs;		/* the watch cursor */
Cursor *textcurs, *normcurs;

int quit = FALSE;

Boolean mcmdactive = TRUE;	/* Enable menu command keys */
Boolean fkeysactive = TRUE;	/* Enable FKEYs */

WindowPtr terminalWindow;	/* the terminal window */
extern WindowPtr remoteWindow;	/* the remote command window */

extern int dfloc;                       /* Default location: remote/local */
extern int dfprty;                      /* Default parity */
extern int dfflow;                      /* Default flow control */

extern int local;			/* running local or remote? */

extern Boolean have_multifinder; /* becomes true if we are running MF */
extern Boolean in_background;	/* becomes TRUE if have_multifinder and
				 * we have recieved a "suspend" event */

extern long mf_sleep_time;	/* this is the number of (60Hz) ticks to
				 * sleep before getting a nullEvent (to
				 * flash our cursor) (and input chars from
				 * the serial line)
				 */

extern Boolean have_128roms;	/* actually, a Mac + or better */

/* local variables */

Boolean have_fourone = FALSE;	/* true if we are running system 4.1 or
				 * better */
Boolean have_ctrl_key = FALSE; /* true if we have an ADB (SE or II) keyboard */

Boolean usingRAMdriver = FALSE;	/* true if using the RAM serial driver */

short takeFRefNum;		/* file reference number of the take file */

/****************************************************************************/
/****************************************************************************/
cmdini ()
{
    short vRefNum;
    Str255 volName;
    OSErr err;

    GetVol (&volName, &vRefNum);
    err = FSOpen ("Kermit Takefile", vRefNum, &takeFRefNum);
    /* try to open the take file */
    if (err == noErr) {
	tlevel = 1;
	getch ();		/* get first character of take file */
	gettoken ();
    };
}				/* cmdini */



/****************************************************************************/
/* return uppercase for a letter */
/****************************************************************************/
char
CAP (c)
char c;
{
    if (islower (c))
	return (_toupper (c));
    else
	return (c);
}				/* CAP */


#define TAK_SERV	1
#define TAK_QUIT	2
#define TAK_SEND	3
#define TAK_RECV	4
#define TAK_GET		5
#define TAK_INP		6
#define TAK_OUT		7
#define TAK_UNK 255

char *takecmdtab[] = {
    "SERVER",
    "QUIT",
    "SEND",
    "RECEIVE",
    "GET",
    "INPUT"
    "OUTPUT",
};

int taketoktab[] = {
    TAK_SERV
    TAK_QUIT
    TAK_SEND
    TAK_RECV
    TAK_GET
    TAK_INP
    TAK_OUT
};

#define NUMOFCMDS (sizeof (taketoktab)/sizeof(int))

/****************************************************************************/
/* return the token number for a specific take command */
/****************************************************************************/
int
findcmd (cmd)
char *cmd;
{
    int k;

    for (k = 0; k < NUMOFCMDS; k++)
	if (strcmp (takecmdtab[k], cmd) == 0)
	    return (taketoktab[k]);	/* and return ID */
    return (TAK_UNK);		/* else unknown */
}				/* findcmd */



char ch;

/****************************************************************************/
/****************************************************************************/
getch ()
{
    long count;

    count = 1;
    if (FSRead (takeFRefNum, &count, &ch) != noErr)
	ch = '\0';
}				/* getch */



#define TOK_CMD		1	/* command id in 'theCmd' */
#define TOK_STR		2	/* string in 'theString' */
#define TOK_NUM		3	/* number in 'theNumber' */
#define TOK_ID		4	/* identifier token */
#define TOK_EOF		5	/* end of file token */
#define TOK_SLS		6	/* ',' */
#define TOK_DOT		7	/* '.' */
#define TOK_UNK	255		/* unknown token */

int token;
int theCmd;
char theString[256];
long theNumber;

/****************************************************************************/
/****************************************************************************/
gettoken ()
{
    int cmdid;
    char *c;
    char buffer[30];
    Boolean comment;

    while ((ch <= ' ') || (ch == '/')) {
	if (ch <= ' ')		/* skip all characters <= blank */
	    if (ch == '\0') {	/* except eof character */
		token = TOK_EOF;
		return;
	    } else
		getch ();

	if (ch == '/') {	/* slash / comment */
	    getch ();

	    if (ch != '*') {
		token = TOK_SLS;
		return;
	    }
	    getch ();
	    comment = TRUE;
	    while (comment) {
		if (ch == '\0') {
		    token = TOK_EOF;
		    return;
		}
		if (ch == '*') {
		    getch ();
		    if (ch == '/') {
			comment = FALSE;
			getch ();
		    }
		} else
		    getch ();
	    }			/* while (comment) */

	}			/* if (ch == '/') */
    }				/* while ((ch <= ' ') || (ch == '/')) */

    if (ch == '"') {		/* string */
	token = TOK_STR;
	c = theString;
	getch ();

	while (TRUE) {
	    if (ch == '"')
		ch = '\0';

	    if (ch == '\\') {
		getch ();
		if (ch == 'n')
		    ch = '\n';
		else if (ch == 'b')
		    ch = '\b';
		else if (ch == 't')
		    ch = '\t';
	    }
	    if ((c - theString) < (sizeof (theString) - 1))
		*c++ = ch;

	    if (ch == '\0') {
		*c = '\0';
		getch ();
		return;
	    } else
		getch ();
	}
    }				/* TOK_STR */
    if ((ch >= '0') && (ch <= '9')) {	/* number */
	token = TOK_NUM;
	theNumber = 0;
	getch ();
	return;
    }				/* TOK_NUM */
    ch = CAP (ch);
    if ((ch >= 'A') && (ch <= 'Z')) {	/* command / identifier */
	c = buffer;
	while ((((ch >= 'A') && (ch <= 'Z')) ||	/* get the whole string */
		((ch >= '0') && (ch <= '9'))) &&
	       ((c - buffer) < (sizeof (buffer) - 1))) {
	    *c++ = ch;
	    getch ();
	    ch = CAP (ch);
	}
	*c = '\0';		/* end the buffer with \0 */

	cmdid = findcmd (buffer);	/* check for command */
	if (cmdid != TAK_UNK) {
	    token = TOK_CMD;
	    theCmd = cmdid;	/* return the command id if true */
	} else
	    token = TOK_ID;	/* return the identifier id if true */

	return;
    }				/* TOK_CMD / TOK_ID */
    switch (ch) {
      case '.':		/* dot */
	token = TOK_DOT;
	break;

      default:			/* unknown character */
	token = TOK_UNK;
    }
}				/* gettoken */



/****************************************************************************/
/****************************************************************************/
char
nextcmd ()
{
    if (token == TOK_CMD) {
	switch (theCmd) {
	    case TAK_SERV:
	    displa = TRUE;
	    scrcreate ();	/* create the packet display dialog */
	    protocmd = SERV_REMO;	/* run the mac as server */
	    gettoken ();
	    return ('x');

	  case TAK_QUIT:
	    quit = TRUE;
	    FSClose (takeFRefNum);
	    return (0);

	  case TAK_SEND:	/* send a file: local, remote files */
	    gettoken ();
	    if (token != TOK_STR)
		return (0);

	    strcpy (filargs.fillcl, theString);	/* file to send */

	    gettoken ();
	    if (token == TOK_STR) {	/* send as */
		strcpy (filargs.filrem, theString);
		gettoken ();
	    } else
		zltor (filargs.fillcl, filargs.filrem);

	    cmarg = filargs.fillcl;
	    cmarg2 = filargs.filrem;

	    nfils = -1;		/* Use cmarg, not cmlist */
	    protocmd = SEND_FIL;
	    scrcreate ();
	    return ('s');	/* return with send state */

	  case TAK_RECV:
	    initfilrecv ();	/* init recv flags */
	    protocmd = RECV_FIL;
	    scrcreate ();
	    gettoken ();
	    return ('v');	/* return with recv state */

	  case TAK_GET:	/* Get from server */
	    gettoken ();
	    if (token != TOK_STR)
		return (0);

	    strcpy (cmarg, theString);
	    protocmd = GETS_FIL;
	    scrcreate ();
	    gettoken ();
	    return ('r');

	  default:
	    return (0);
	}			/* switch (theCmd) */

    } else {
	tlevel = -1;		/* no more commands */
	FSClose (takeFRefNum);
	return (0);
    }

}				/* nextcmd */


/****************************************************************************/
/* init_menus - create the menu bar. */
/****************************************************************************/
setup_menus ()
{
    int i;
    static int menus_are_drawn = 0;
    THz curZone;

    if (!menus_are_drawn) {	/* if the first time through */
	/*
	 * PWP: we do command keys by default ONLY on a keyboard that has a CTRL
	 * key
	 */
	mcmdactive = have_ctrl_key;
	
	/* setup Apple menu */
	if ((menus[APPL_MENU] = GetMenu (APPL_MENU)) == NIL)
	    printerr("Couldn't get MENU", APPL_MENU);
	else
	    AddResMenu (menus[APPL_MENU], 'DRVR');
    } else {
    	ClearMenuBar();		/* remove all menus from the list */
    }
    
    InsertMenu (menus[APPL_MENU], 0);	/* Put Apple Menu on menu line */

    for (i = MIN_MENU; i <= MAX_MENU; i++) {	/* For all menus */
        if (menus_are_drawn && menus[i]) {
	    curZone = GetZone();		/* as per John Norstad's (Disinfectant) */
	    SetZone(HandleZone(menus[i]));	/* "Toolbox Gotchas" */
	    ReleaseResource(menus[i]);		/* free old resource */
	    SetZone(curZone);
	}
    	if (mcmdactive) {
	    if ((menus[i] = GetMenu (i)) == NIL)	/* Fetch it from resource file */
		printerr("Couldn't get MENU", i);
	} else {
	    if ((menus[i] = GetMenu (i+32)) == NIL) {	/* try to get w/o clover */
	        printerr("Couldn't get MENU", i+32);
		menus[i] = GetMenu (i);	/* Fetch normal from resource file */
	    }
	}
	InsertMenu (menus[i], 0);	/* Put it on menu line */
    }

    DrawMenuBar ();		/* Finish up by displaying the bar */

    CheckItem (menus[SETG_MENU], MCDM_SETG, mcmdactive);
    menus_are_drawn = 1;
}				/* setup_menus */

Boolean
IsWNEImplemented ()
{
    int err;
    SysEnvRec theWorld;
#define FOURONEVERSION	1040	/* version == (short) (4.1 * 256.) */

    /*
     * (from Mac Tech Note #158) We need ot call SysEnvirons to make sure
     * that WaitNextEvent is implemented.  If we are running on 64K ROMs, and
     * RAM HFS is running (trap 0xA060), then GetTrapAddress(0x60) will
     * return a value different from the unimplemented trap since trap 60 is
     * implemented for HFS and the 64K ROM version of GetTrapAddress doesn't
     * differentiate between OS and Tool traps.
     */

    /* These are both toolbox traps, hence the 1 */
    err = SysEnvirons (1, &theWorld);	/* we have the glue, so machineType
					 * will allways be filled in */

    /* to see if we can use the script manager */
    have_fourone = (theWorld.systemVersion >= FOURONEVERSION);

    /* let's hope that Apple gets sane about the CTRL key... */
    have_ctrl_key = (theWorld.keyBoardType == envAExtendKbd) ||
	(theWorld.keyBoardType == envStandADBKbd);

    have_128roms = !((theWorld.machineType == envMac) ||
    		     (theWorld.machineType == envXL) ||
    		     (theWorld.machineType == envMachUnknown));
    
    /* is WNE implemented? */
    if (theWorld.machineType < 0)
	return FALSE;		/* we don't know what kind of Mac this is. */

    /* "..., 1" 'cause these are tooltraps: */
    /* 6.0.2 bug fixed by RWR <CES00661%UDACSVM.BITNET@cunyvm.cuny.edu> */
    
    if ((NGetTrapAddress (num_WaitNextEvent, 1) !=
	 NGetTrapAddress (num_UnknownTrap, 1)) &&	/* RWR  */
	(NGetTrapAddress (num_JugglDispatch, 1) !=	/* RWR  */
	 NGetTrapAddress (num_UnknownTrap, 1)))		/* RWR  */
	return TRUE;

    return FALSE;
}

extern hmacrodefs macroshdl;	/* handle to the macro table */
extern modrec modtable[NUMOFMODS];	/* modifier records */
extern RgnHandle dummyRgn;	/* dummy region for ckmcon */
WindowRecord terminalWRec;	/* store window stuff here */
extern char **myclip_h;		/* internal clipboard */
extern int myclip_size;		/* size of above */
extern long MyCaretTime;	/* ticks between flashes for cursor */
extern char **myclip_h;		/* internal terminal clipboard */

/****************************************************************************/
/* mac_init - Initialize the macintosh and any window, menu, or other */
/* resources we will be using. */
/****************************************************************************/
mac_init ()
{
    int err;
    int i;
    CursHandle cursh;

    MaxApplZone ();		/* Make appl. heap big as can be */

    MoreMasters ();		/* Create some more master pointers */
    MoreMasters ();		/* Create some more master pointers */
    err = MemError ();
    if (err != noErr)
	printerr ("Unable to create masters", err);

    InitGraf (&qd.thePort);	/* Init the graf port */
    InitFonts ();		/* The fonts */
    InitWindows ();		/* The windows */

/* Debugger(); */
    /*
     * PWP: we MUST call IsWNEImplemented() BEFORE using have_fourone, or
     * have_ctrl_key (in InitMenus() )
     */
    have_multifinder = IsWNEImplemented ();	/* See Above. */

    InitMenus ();
    TEInit ();			/* Init text edit */
    InitDialogs (NILPROC);	/* The dialog manager */
    InitCursor ();		/* start with a nice cursor */
    SetEventMask (everyEvent - keyUpMask);

    dummyRgn = NewRgn ();

    normcurs = &qd.arrow;
    if ((cursh = GetCursor (watchCursor)) != NIL) {
	HLock(cursh);
	watchcurs = *cursh;		/* the waiting cursor */
    } else {
    	watchcurs = &qd.arrow;
    }
    if ((cursh = GetCursor (iBeamCursor)) != NIL) {
	HLock(cursh);
	textcurs = *cursh;		/* the text body cursor */
    } else {
    	textcurs = &qd.arrow;
    }

    MyCaretTime = GetCaretTime();
    if (MyCaretTime < 3 || MyCaretTime > 300)
	MyCaretTime = 20L;
    
    setup_menus ();		/* build our menus */
    ScrDmpEnb = scrdmpenabled;	/* enable FKEYs */

    inittiobufs();		/* init terminal I/O buffers */

    initrcmdw ();		/* init remote cmd window */
    initfilset ();		/* init file settings */

    terminalWindow = GetNewWindow (TERMBOXID, &terminalWRec, (WindowPtr) - 1);
    SetPort (terminalWindow);

    TextFont (monaco);		/* Monaco font for non-proportional spacing */
    TextSize (9);

    FlushEvents (everyEvent, 0);	/* clear click ahead */

    port_open(-6);	/* open Modem port by default */
    
    parity = DEFPAR;
    if (!setserial (innum, outnum, DSPEED, DEFPAR))	/* set speed parity */
	fatal("Couldn't set serial port to default speed",0);
	
    ttres();			/* (PWP) set up flow control for interactive use */
    
    consetup ();		/* Set up for connecting */
    displa = TRUE;		/* Make everything goes to screen */

    /* init (internal) clipboard */
    myclip_h = (char **) NewHandle (32);
    myclip_size = 0;
    
    /* init the macro table */
    macroshdl = (hmacrodefs) NewHandle (MacroBaseSize);
    (*macroshdl)->numOfMacros = 0;

    /* clear the prefix strings */
    for (i = 0; i < NUMOFMODS; i++)
	modtable[i].prefix[0] = '\0';

    loadkset ();		/* PWP: get our defaults for these */
    loadmset ();

    /* Frank changed main() to call init and then set flow, parity, etc.
       so we make sure they will be set right (again) after we return. */
    dfloc = local;                      /* And whether it's local or remote. */
    dfprty = parity;                    /* Set initial parity, */
    dfflow = flow;                      /* and flow control. */


#ifdef PROFILE
    if (!INITPERF(&ThePGlobals, 1, 8, FALSE, TRUE, "\pCODE", 0, "\pROMII",
    	          FALSE, 0, 0, 0))
	fatal("Could not start profiling", 0);
    (void) PerfControl(ThePGlobals, TRUE);
#endif /* PROFILE */
}				/* mac_init */


/****************************************************************************/
/* mac_cleanup() - called before leaving this program to clean up any */
/*    	      	   dangling Mac stuff. */
/* Called by doexit, transfer and zkself. */
/****************************************************************************/
mac_cleanup ()
{
    ScrDmpEnb = scrdmpenabled;	/* re-enabled screen dumping */
    
    FutzOptKey(0);		/* reset old Mac key map */
    
#ifdef PROFILE
    if (PERFDUMP(ThePGlobals, "\pPerform.out", true, 80))
	fatal("Could not dump profiling output", 0);
    (void) TermPerf (ThePGlobals);
#endif /* PROFILE */

    port_close();		/* close the serial port down */

#ifdef TLOG
    if (tralog)			/* close transaction log if necessary */
	closetlog ();
#endif TLOG

    if (seslog)			/* close session log if necessary */
	closeslog ();

    DisposeMacros ();		/* dipose all macro strings */
    if (macroshdl)
	DisposHandle ((Handle) macroshdl);	/* release the macro table */
    macroshdl = 0L;
    
    if (myclip_h)
	DisposHandle(myclip_h);											/*JAO*/
    myclip_h = 0L;													/*JAO*/

    if (hPrintBuffer)
	DisposHandle(hPrintBuffer);											/*JAO*/
    hPrintBuffer = 0L;													/*JAO*/

    DisposeRgn (dummyRgn);
}				/* mac_cleanup */



/****************************************************************************/
/* doexit(status) - exit to shell.  Perhaps we should check for abnormal */
/*    	      	    status codes... */
/****************************************************************************/
doexit (status)
{
    mac_cleanup ();		/* make things tidy */
    ExitToShell ();
}				/* doexit */
