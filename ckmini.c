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


extern Boolean have_multifinder; /* becomes true if we are running MF */
extern Boolean in_background;	/* becomes TRUE if have_multifinder and
				 * we have recieved a "suspend" event */

extern long mf_sleep_time;	/* this is the number of (60Hz) ticks to
				 * sleep before getting a nullEvent (to
				 * flash our cursor) (and input chars from
				 * the serial line)
				 */

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

    for (i = MIN_MENU; i <= MAX_MENU; i++) {	/* For all menus */
	menus[i] = GetMenu (i);	/* Fetch it from resource file */
	InsertMenu (menus[i], 0);	/* Put it on menu line */
    }

    AddResMenu (menus[APPL_MENU], 'DRVR');
    DrawMenuBar ();		/* Finish up by displaying the bar */

    /*
     * PWP: we do command keys by default ONLY on a keyboard that has a CTRL
     * key
     */
    CheckItem (menus[SETG_MENU], MCDM_SETG, have_ctrl_key);
    mcmdactive = have_ctrl_key;
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

    /* is WNE implemented? */
    if (theWorld.machineType < 0)
	return FALSE;		/* we don't know what kind of Mac this is. */

    /* "..., 1" 'cause these are tooltraps: */
    if (NGetTrapAddress (num_WaitNextEvent, 1) !=
	NGetTrapAddress (num_UnknownTrap, 1))
	return TRUE;

    return FALSE;
}

extern hmacrodefs macroshdl;	/* handle to the macro table */
extern modrec modtable[NUMOFMODS];	/* modifier records */
extern RgnHandle dummyRgn;	/* dummy region for ckmcon */
WindowRecord terminalWRec;	/* store window stuff here */


/****************************************************************************/
/* mac_init - Initialize the macintosh and any window, menu, or other */
/* resources we will be using. */
/****************************************************************************/
mac_init ()
{
    int err;
    int i;

    MaxApplZone ();		/* Make appl. heap big as can be */

    MoreMasters ();		/* Create some more master pointers */
    err = MemError ();
    if (err != noErr)
	printerr ("Unable to create masters", err);

    InitGraf (&qd.thePort);	/* Init the graf port */
    InitFonts ();		/* The fonts */
    InitWindows ();		/* The windows */

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

#ifdef COMMENT
    /******** N O T E : *********/
    /* PWP: this has not ever been tested.  I don't know if it will work */
    /****************************/

    if (have_fourone) {		/* if we are system 4.1 or later... */
	/* set the key map */
	err = SetScript (smRoman, smScriptKeys, NODEAD_KCHR);
	if (err)
	    printerr ("Trouble setting custom keymap (KCHR):", err);
	/* set the icon */
	err = SetScript (smRoman, smScriptIcon, NODEAD_KCHR);
	if (err)
	    printerr ("Trouble setting keymap icon (SICN):", err);
	KeyScript (smRoman);
    } else {
	/* do something or other to do the old way */
    }
#endif

    watchcurs = GetCursor (watchCursor);	/* the waiting cursor */

    mybuff = NewPtr ((long) MYBUFSIZE);	/* Allocate mybuff from the heap */
    if (mybuff == NIL)
	printerr ("Unable to allocate mybuff", 0);

    setup_menus ();		/* build our menus */
    ScrDmpEnb = scrdmpenabled;	/* enable FKEYs */

    initrcmdw ();		/* init remote cmd window */
    initfilset ();		/* init file settings */

    terminalWindow = GetNewWindow (TERMBOXID, &terminalWRec, (WindowPtr) - 1);
    SetPort (terminalWindow);

    TextFont (monaco);		/* Monaco font for non-proportional spacing */
    TextSize (9);

    FlushEvents (everyEvent, 0);/* clear click ahead */

    /* Set up IO drivers */
    innum = -6;
    outnum = -7;
    err = RAMSDOpen (sPortA);
    if (err != noErr) {
    	printerr("Can't open RAM serial driver; using the ROM driver without\
 flow control.",0);
	err = OpenDriver (".AIn", &innum);
	if (err != noErr)
	    fatal ("macinit could not OpenDriver .AIn: ", err);
	err = OpenDriver (".AOut", &outnum);
	if (err != noErr)
	    fatal ("macinit could not OpenDriver .AOut: ", err);
    	usingRAMdriver = FALSE;
    } else {
    	usingRAMdriver = TRUE;
    }

    parity = DEFPAR;
    setserial (innum, outnum, DSPEED, DEFPAR);	/* set speed parity */

    if (mybuff != NIL) {
	err = SerSetBuf (innum, mybuff, MYBUFSIZE);
			/* Make driver use larger buff */
	if (err)
	    printerr ("Trouble making IO buffer:", err);
    }
    
    /* PWP: .fXOn and .fInX TRUE as per Matthias' note */
    controlparam.fXOn = TRUE;	/* Specify handshake options */
    controlparam.fCTS = FALSE;
    controlparam.xOn = 17;
    controlparam.xOff = 19;
    controlparam.errs = FALSE;
    controlparam.evts = FALSE;
    controlparam.fInX = TRUE;

    err = SerHShake (outnum, &controlparam);
    if (err)
	printerr ("Trouble with output handshake: ", err);
    err = SerHShake (innum, &controlparam);
    if (err)
	printerr ("Trouble with input handshake: ", err);

    consetup ();		/* Set up for connecting */
    displa = TRUE;		/* Make everything goes to screen */

    /* init the macro table */
    macroshdl = (hmacrodefs) NewHandle (MacroBaseSize);
    (*macroshdl)->numOfMacros = 0;

    /* clear the prefix strings */
    for (i = 0; i < NUMOFMODS; i++)
	modtable[i].prefix[0] = '\0';

    loadkset ();		/* PWP: get our defaults for these */
    loadmset ();

    dummyRgn = NewRgn ();
}				/* mac_init */



/****************************************************************************/
/* mac_cleanup() - called before leaving this program to clean up any */
/*    	      	   dangling Mac stuff. */
/* Called by doexit, transfer and zkself. */
/****************************************************************************/
mac_cleanup ()
{
    int err;

    ScrDmpEnb = scrdmpenabled;	/* re-enabled screen dumping */
    
    /* PWP: some day, these calls will make it into the real code... */\
    err = KillIO(innum);	/* Kill off IO drivers */
    if (err != noErr)
    	printerr("trouble KillIO-ing serial input driver:",err);
    err = KillIO(outnum);	/* Kill off IO drivers */
    if (err != noErr)
    	printerr("trouble KillIO-ing serial output driver:",err);

    err = SerSetBuf (innum, NULL, 0);	/* Make driver default buffer */
    if (err != noErr)
    	printerr("trouble resetting serial IO buffer:",err);

    if (usingRAMdriver) {
	err = RAMSDClose (sPortA);
	/* PWP: I don't know why this returns -1 on a Mac II */
	if ((err != noErr) && (err != -1))
    	    printerr("trouble closing RAM serial driver:",err);
    } else { /* close the old ROM drivers */
	err = CloseDriver (innum);
	if (err != noErr)
    	    printerr("trouble closing serial input driver:",err);
#ifdef COMMENT
For some reason or other, doing this close on a 64k ROM machine will cause
the mouse to freeze.  Since the input driver is the only one that really
matters, we just close it.

	err = CloseDriver (outnum);
	if (err != noErr)
    	    printerr("trouble closing serial output driver:",err);
#endif COMMENT
    }

#ifdef TLOG
    if (tralog)			/* close transaction log if necessary */
	closetlog ();
#endif TLOG

    if (seslog)			/* close session log if necessary */
	closeslog ();

    DisposeMacros ();		/* dipose all macro strings */
    DisposHandle ((Handle) macroshdl);	/* release the macro table */

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
