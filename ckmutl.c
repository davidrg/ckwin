/* Version 0.8(35) - Jim Noble at Planning Research Corporation, June 1987. */
/* Ported to Megamax native Macintosh C compiler. */
/* Edit by Frank on 20 Jun 5:25pm */
/* In commdialog(), don't set parity on sio chip. */
/* Edit by Bill on Apr 30 00:11 */
/* use mygetitem instead of getitem, it doesn't convert from pascal to c */
/* Redo handapple */
/* Edit by Bill on Apr 21 17:27 */
/* Try to fix meaning of TURN and TURNCH in dialog box */
/* Remove version from dofiledialog dialog box */

/*
 * file ckmutl.c
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

#include <ctype.h>		/* For isdigit */
#include "ckcdeb.h"
#include "ckcker.h"

#define	__SEG__ KStuff
#include <controls.h>
#include <devices.h>
#include <dialogs.h>
#include <packages.h>
#include <serial.h>
#include <menus.h>
#include <Memory.h>
#include <resources.h>
#include <events.h>
#include <toolutils.h>
#include <segload.h>
	/* for ExitToShell (I don't know why it's in here...) */

#include "ckmdef.h"		/* General Mac defs */
#include "ckmres.h"		/* Resource file defs */
#include "ckmasm.h"		/* Assembler code */
#include "ckmcon.h"		/* defines, etc. for terminal emulator */

extern MenuHandle menus[];



/****************************************************************************/
/*  B L D L E N  --  Make length-encoded copy of string  */
/****************************************************************************/
char *
bldlen (str, dest)
char *str;
char *dest;
{
    int len;

    len = strlen (str);
    *dest = tochar (len);
    strcpy (dest + 1, str);
    return (dest + len + 1);
}				/* bldlen */



/****************************************************************************/
/*  S E T G E N  --  Construct a generic command  */
/****************************************************************************/
setgen (stor, type, arg1, arg2, arg3)
char *stor;
char type;
char *arg1;
char *arg2;
char *arg3;
{
    char *upstr, *cp;

    cp = stor;
    *cp++ = type;
    *cp = '\0';
    if (*arg1 != '\0') {
	upstr = bldlen (arg1, cp);
	if (*arg2 != '\0') {
	    upstr = bldlen (arg2, upstr);
	    if (*arg3 != '\0')
		bldlen (arg3, upstr);
	}
    }
    debug (F110, "setgen", stor, 0);
}				/* setgen */



/****************************************************************************/
/* printerr - display error message and number in standard error box. */
/****************************************************************************/
printerr (str, err)
char *str;
int err;
{
    int i;
    char error[10];

    if (str == NULL || str[0] == 0)
	str = "OOPS: printerr called with an empty string";

    if (err) {			/* Err=0 signals message only */
	for (i = 0; i < 10; error[i++] = '\0');	/* Make string null
						 * terminated */
	NumToString ((long) err, error);	/* Convert err number */
	ParamText (str, error, "", "");	/* Insert strings into error box */
    } else
	ParamText (str, "", "", "");

    if (!server)		/* Display alert only if not server mode */
	NoteAlert (ALERT_ERROR, NILPROC);
}				/* printerr */



/****************************************************************************/
/* fatal - close file, and exit to shell.	*/
/****************************************************************************/
fatal (str, err)
char *str;
int err;
{
    int i;
    char error[10];

    if (err) {			/* Err=0 signals message only */
	for (i = 0; i < 10; error[i++] = '\0');	/* Make string null
						 * terminated */
	NumToString ((long) err, error);	/* Convert err number */
	ParamText ("Fatal error:", str, error, "");	/* Insert strings into
							 * error box */
    } else
	ParamText ("Fatal error:", str, "", "");

    if (!server)		/* Display alert only if not server mode */
	StopAlert (ALERT_ERROR, NILPROC);

    /*
     * terminate NOW!  The heap might be scrambled, so don't try to clean
     * anything up.
     */
    ExitToShell ();
}				/* fatal */



/****************************************************************************/
/* SetStrText - Set the string value in the dialog's edit or static text
   item.	*/
/****************************************************************************/
SetStrText (item, s, dlg)
int item;
char *s;
DialogPtr dlg;
{
    short itemtype;
    Rect itemrect;
    Handle itemhdl;

    GetDItem (dlg, item, &itemtype, &itemhdl, &itemrect);
    if ((itemtype & editText) || (itemtype & statText))
	SetIText (itemhdl, s);
    else
	printerr ("Bad item in SetStrText: ", item);
}				/* SetStrText */



/****************************************************************************/
/* SetNumText - Set the numeric value in the dialog's edit or static text
   item.	*/
/****************************************************************************/
SetNumText (item, val, dlg)
int item;
int val;
DialogPtr dlg;
{
    char itembuf[10];

    NumToString ((long) val, itembuf);
    SetStrText (item, itembuf, dlg);
}				/* SetNumText */

/*PWP************************************************************************/
/* circleOK - Circle the default button for a dialog (since the dialog manager
 * doesn't do this for us. */
/****************************************************************************/
circleOK(dlg)
DialogPtr dlg;
{
    short itemtype;
    Rect itemrect;
    Handle itemhdl;
    GrafPtr oldport;

    GetDItem (dlg, 1, &itemtype, &itemhdl, &itemrect);
    if (itemtype != (ctrlItem | btnCtrl))
	return;		/* it wasn't a button */

    GetPort(&oldport);
    SetPort(dlg);
    /* PWP: might have to do a local-->global on the rect here */
    PenSize(3,3);	/* see IM 1, page 407 (dialog manager) */
    InsetRect(&itemrect,-4,-4);
    FrameRoundRect(&itemrect,16,16);
    SetPort(oldport);
}				/* SetStrText */


extern char *protv;		/* protocol version number */
extern char *versio;		/* C-Kermit version number */

extern int tsecs;		/* Seconds for transaction */
extern long filcnt,		/* Number of files in transaction */
    flci,			/* Characters from line, current file */
    flco,			/* Chars to line, current file  */
    tlci,			/* Chars from line in transaction */
    tlco,			/* Chars to line in transaction */
    ffc,			/* Chars to/from current file */
    tfc;			/* Chars to/from files in transaction */

/****************************************************************************/
/* aboutKermit - Display the about kermit dialog box, and some transaction */
/*     	      	 statistics */
/****************************************************************************/
aboutKermit ()
{
    Handle kversion;
    short itemhit;
    DialogPtr aboutDlg;
    THz curZone;

    aboutDlg = GetNewDialog (ABOUTID, NILPTR, (WindowPtr) - 1);
    circleOK(aboutDlg);
    
    kversion = GetResource (ApplCreator, 0);	/* get version information */
    HLock (kversion);		/* prevent movement */
    p2cstr (*kversion);		/* convert to C string */

    SetStrText (AB_VERS, *kversion, aboutDlg);
    SetStrText (AB_PROV, versio, aboutDlg);	/* was protv */

#ifdef COMMENT
/* take these out for the real version */
SetStrText (AB_COPY, "\rThis is a Beta Test copy of Kermit.  Please do not \
redistribute, and please get a real copy after July 31, 1989", aboutDlg);
SetStrText (AB_BUGS, "Bugs to: paul@cis.ohio-state.edu", aboutDlg);
#endif /* COMMENT */

    ModalDialog (NILPROC, &itemhit);
    DisposDialog (aboutDlg);

    c2pstr (*kversion);		/* convert back to Pascal string */
    HUnlock (kversion);		/* undo previous HLock */
    curZone = GetZone();		/* as per John Norstad's (Disinfectant) */
    SetZone(HandleZone(kversion));	/* "Toolbox Gotchas" */
    ReleaseResource(kversion);	/* no longer needed */
    SetZone(curZone);
}				/* aboutKermit */

/****************************************************************************/
/* show_stats -- show some statistics about the file transfer speed */
/****************************************************************************/
show_stats ()
{
    Handle kversion;
    short itemhit;
    DialogPtr statDlg;
    long lx;

    statDlg = GetNewDialog (STATBOXID, NILPTR, (WindowPtr) - 1);
    circleOK(statDlg);
    
    SetNumText (ST_TSEC, tsecs, statDlg);
    SetNumText (ST_TLCI, tlci, statDlg);
    SetNumText (ST_TLCO, tlco, statDlg);
    SetNumText (ST_TFC, tfc, statDlg);
    if (tsecs > 0)
	SetNumText (ST_EBAUD, ((tfc*10)/tsecs), statDlg);
    else
	SetStrText (ST_EBAUD, "n/a", statDlg);

    if (tsecs > 0) {
	lx = (tfc * 10l) / tsecs;
	SetNumText (ST_EBAUD, lx, statDlg);
	if (speed > 0) {
	    lx = (lx * 100l) / speed;
	    SetNumText (ST_EEFF, lx, statDlg);
	} else {
	    SetStrText (ST_EEFF, "n/a", statDlg);
	}
    } else {
	SetStrText (ST_EBAUD, "n/a", statDlg);
	SetStrText (ST_EEFF, "n/a", statDlg);
    }


    ModalDialog (NILPROC, &itemhit);
    DisposDialog (statDlg);
}				/* aboutKermit */


/****************************************************************************/
/* handapple - Handle the apple menu, either running a desk accessory */
/*    	       or calling a routine to display information about our */
/*    	       program.  Use the practice of */
/*    	       checking for available memory, and saving the GrafPort */
/*    	       described in the DA Manager's Guide. */
/****************************************************************************/
handapple (accitem)
int accitem;
{
    GrafPtr savePort;		/* Where to save current port */
    Handle acchdl;		/* holds ptr to accessory resource */
    char accname[255];		/* string holds accessory name */
    long accsize;		/* holds size of the acc + stack */

    if (accitem == 1) {		/* tell about Kermit? */
	aboutKermit ();		/* yes, "about kermit" please */
	return;			/* and return */
    }
    GetItem (menus[APPL_MENU], accitem, accname); /* get the pascal name */
    SetResLoad (FALSE);		/* don't load into memory */

    /* figure out acc size + heap */
    accsize = SizeResource (GetNamedResource ((ResType) 'DRVR', accname));
    acchdl = NewHandle (accsize);	/* try for a block this size */
    SetResLoad (TRUE);		/* reset flag for rsrc mgr */
    if (acchdl == NIL) {	/* able to get a chunk? */
	printerr ("Not enough memory for accessory. Requires: ",
		  (int) accsize);
	return;			/* failed */
    }
    DisposHandle (acchdl);	/* get rid of this handle */
    GetPort (&savePort);	/* save the current port */
    OpenDeskAcc (accname);	/* run desk accessory */
    SetPort (savePort);		/* and put back our port */
}				/* handapple */


typedef struct {
    unsigned kerval;
    int macval;
}   KMTBL;

KMTBL kerbaudtbl[] = {
    {300, baud300},
    {600, baud600},
    {1200, baud1200},
    {1800, baud1800},
    {2400, baud2400},
    {4800, baud4800},
    {7200, baud7200},
    {9600, baud9600},
    {19200, baud19200},    
    {38400, 2}		/* this is a guess */
    {57600, baud57600}
};

#define kerbaudlen ((sizeof (kerbaudtbl))/(sizeof (KMTBL)))

extern Boolean usingRAMdriver;	/* true if using the RAM serial driver */

#define PARITY_ETC	(noParity+stop10+data8)

int
setserial (irefnum, orefnum, b, p)
int irefnum, orefnum, b, p;
{
    int err, mb, i;
    long *mbp;
    CntrlParam cpb;
    
    if ((b < 300) || (b > 57600))
    	return;		/* was a garbage value */

    if (usingRAMdriver) {
	mb = baud9600;
    } else {
	for (i = 0; i <= kerbaudlen; i++) {
	    if (kerbaudtbl[i].kerval == b) {
		mb = kerbaudtbl[i].macval;	/* set mac value for baud */
		goto found_speed;
	    }
	}
	printerr("Can't set serial port to", b);
	return (0);
      found_speed:
    }

/*
 * PWP: if the Mac does parity in hardware, then Even parity seems to
 * not work correctly, so we have Kermit do parity in
 * software.
 */

    err = SerReset (irefnum, mb + PARITY_ETC);	/* reset serial input port */
    if (err != noErr) {
	printerr ("setserial couldn't set port (input): ", err);
	return (0);
    }

    if (usingRAMdriver) {
	cpb.csCode = 13;			/* set baud rate */
	cpb.csParam[0] = (unsigned short) b;
	cpb.ioCRefNum = irefnum;
	err = PBControl (&cpb, FALSE);
	if (err != noErr) {
	    printerr ("setserial couldn't set port (input): ", err);
	    return (0);
	}
    }

    err = SerReset (orefnum, mb + PARITY_ETC);	/* reset serial output port */
    if (err != noErr) {
	printerr ("setserial couldn't set port (output): ", err);
	return (0);
    }

    if (usingRAMdriver) {
	cpb.csCode = 13;			/* set baud rate */
	cpb.csParam[0] = (unsigned short) b;
	cpb.ioCRefNum = orefnum;
	err = PBControl (&cpb, FALSE);
	if (err != noErr) {
	    printerr ("setserial couldn't set port (input): ", err);
	    return (0);
	}
    }

    speed = b;			/* set global kermit value */
    
    return (1);	/* we did it */
}				/* setserial */

typedef struct {
    int resval;
    unsigned serval;
}   RESSERTBL;

RESSERTBL commbaudtbl[] = {
    {CR_BAUD300, 300},
    {CR_BAUD600, 600},
    {CR_BAUD1200, 1200},
    {CR_BAUD1800, 1800},
    {CR_BAUD2400, 2400},
    {CR_BAUD3600, 3600},
    {CR_BAUD4800, 4800},
    {CR_BAUD7200, 7200},
    {CR_BAUD9600, 9600},
    {CR_BAUD14400, 14400},
    {CR_BAUD19200, 19200},
    {CR_BAUD28800, 28800},
    {CR_BAUD38400, 38400},
    {CR_BAUD57600, 57600},
#ifdef NOT_YET
    {CR_BAUD76800, 76800},
    {CR_BAUD115200, 115200},
#endif
    {0, 0}
};

RESSERTBL commparitytbl[] = {
    {CR_PARODD, KPARITY_ODD},
    {CR_PAREVEN, KPARITY_EVEN},
    {CR_PARMARK, KPARITY_MARK},
    {CR_PARSPACE, KPARITY_SPACE},
    {CR_PARNONE, KPARITY_NONE},
    {0, 0}
};



/****************************************************************************/
/* rshilite - hilite the radio control item matching the given */
/*    	  serial value, and disable all other control items in the */
/*    	  resource-serial table. */
/****************************************************************************/
rshilite (servalue, rstbl, dlg)
int servalue;
RESSERTBL rstbl[];
DialogPtr dlg;
{
    int i;
    short itemtype;
    Rect itembox;
    Handle itemhdl;

    for (i = 0; rstbl[i].resval != 0; i++) {
	GetDItem (dlg, rstbl[i].resval, &itemtype, &itemhdl, &itembox);

	if (itemtype != ctrlItem + radCtrl)
	    printerr ("rshilite called with non radio control: ",
		      rstbl[i].resval);

	if (rstbl[i].serval == servalue)
	    SetCtlValue ((ControlHandle) itemhdl, btnOn);
	else
	    SetCtlValue ((ControlHandle) itemhdl, btnOff);
    }
}				/* rshilite */



/****************************************************************************/
/****************************************************************************/
rsreference (rstbl, value, dlg)
RESSERTBL rstbl[];
DialogPtr dlg;
int value;
{
    int i;
    short itemtype;
    Rect itembox;
    Handle itemhdl;

    for (i = 0; rstbl[i].resval != 0; i++) {
	GetDItem (dlg, rstbl[i].resval, &itemtype, &itemhdl, &itembox);
	SetCRefCon ((ControlHandle) itemhdl, (long) value);
    }
}				/* rsreference */



/****************************************************************************/
/* rsserval - given a resource ID for a control and a resource-serial */
/*    	      table, return the serial value. */
/****************************************************************************/
int
rsserval (resvalue, rstbl)
int resvalue;
RESSERTBL rstbl[];
{
    int i;

    for (i = 0; rstbl[i].resval != 0; i++)
	if (rstbl[i].resval == resvalue)
	    return (rstbl[i].serval);

    fatal ("rsserval didn't find: ", resvalue);
    return (0);			/* never get here */
}				/* rsserval */



#define CREF_BAUD 1
#define CREF_PARITY 2

extern int drop_dtr;

/****************************************************************************/
/* commdialog - enter communications setup dialog. */
/****************************************************************************/
commdialog ()
{
    DialogPtr commdlg;
    long i;
    short itemhit;
    short itemtype;
    int dlgspeed;
    int dlgparity;
    int dlgflow;
    int dlgport, newinnum;
    int dlgdtr;
    Handle itemhdl;
    Rect itembox;

    commdlg = GetNewDialog (COMMBOXID, NILPTR, (WindowPtr) - 1);
    circleOK(commdlg);
    
    dlgspeed = speed;		/* initialize to current global */
    dlgparity = parity;		/* values of baud and parity */
    dlgflow = flow;
    if (innum == -6)
	dlgport = CR_PMODEM;
    else
	dlgport = CR_PPRINTER;
    if (!usingRAMdriver) {
	/* grey out all the speeds we can't set */
	GetDItem (commdlg, CR_BAUD3600, &itemtype, &itemhdl, &itembox);
	HiliteControl((ControlHandle) itemhdl, 255);	/* grey out */
	GetDItem (commdlg, CR_BAUD14400, &itemtype, &itemhdl, &itembox);
	HiliteControl((ControlHandle) itemhdl, 255);	/* grey out */
	GetDItem (commdlg, CR_BAUD28800, &itemtype, &itemhdl, &itembox);
	HiliteControl((ControlHandle) itemhdl, 255);	/* grey out */
	GetDItem (commdlg, CR_BAUD38400, &itemtype, &itemhdl, &itembox);
	HiliteControl((ControlHandle) itemhdl, 255);	/* grey out */
    }
    dlgdtr = drop_dtr;

    rshilite (dlgspeed, commbaudtbl, commdlg);	/* hilite our baud */
    rshilite (dlgparity, commparitytbl, commdlg); /* hilite our parity */
    SetCtlValue (getctlhdl (CR_XONXOFF, commdlg), /* hilite our flow */
		 dlgflow ? btnOn : btnOff);
    SetCtlValue (getctlhdl (dlgport, commdlg), btnOn);	/* hilite our port */
    SetCtlValue (getctlhdl (CR_DROPDTR, commdlg), /* hilite our DTR */
		 dlgdtr ? btnOn : btnOff);

/* for all baud and parity controls set the reference value for each
 * control to the resource-serial table address so we can manipulate
 * these controls easily during the dialog processing.
 */

    rsreference (commbaudtbl, CREF_BAUD, commdlg); /* setup control ref's */
    rsreference (commparitytbl, CREF_PARITY, commdlg);

    for (;;) {
	ModalDialog (NILPROC, &itemhit);

	switch (itemhit) {
	  case OKBtn:
	    drop_dtr = dlgdtr;
	    if (dlgport == CR_PMODEM) {
		newinnum = -6;
	    } else {
		newinnum = -8;
	    }
	    if (newinnum != innum) {
		dlgdtr = drop_dtr;
		drop_dtr = 1;	/* so we don't confuse AppleTalk */
		port_close();
		drop_dtr = dlgdtr;
		port_open(newinnum);
	    }
	    parity = dlgparity;
	    flow = dlgflow;
	    if (!setserial (innum, outnum, dlgspeed, KPARITY_NONE))
		break;	/* keep doing dialog */

	  case QuitBtn:
	    DisposDialog (commdlg);	/* finished with the dialog */
	    return;		/* so return */

	  case CR_XONXOFF:	/* do flow control */
	    dlgflow = !dlgflow;
	    SetCtlValue (getctlhdl (CR_XONXOFF, commdlg),
			 dlgflow ? btnOn : btnOff);
	    break;

	  case CR_DROPDTR:	/* drop DTR on exit */
	    dlgdtr = !dlgdtr;
	    SetCtlValue (getctlhdl (CR_DROPDTR, commdlg),
			 dlgdtr ? btnOn : btnOff);
	    break;

	  case CR_PMODEM:
	  case CR_IMODEM:
	    SetCtlValue (getctlhdl (dlgport, commdlg), btnOff);
	    dlgport = CR_PMODEM;
	    SetCtlValue (getctlhdl (dlgport, commdlg), btnOn);
	    break;

	  case CR_PPRINTER:
	  case CR_IPRINTER:
	    SetCtlValue (getctlhdl (dlgport, commdlg), btnOff);
	    dlgport = CR_PPRINTER;
	    SetCtlValue (getctlhdl (dlgport, commdlg), btnOn);
	    break;
	    	    
	  default:		/* default is radio button */
	    GetDItem (commdlg, itemhit, &itemtype, &itemhdl, &itembox);
	    i = GetCRefCon ((ControlHandle) itemhdl);

	    switch (i) {
	      case CREF_BAUD:
		dlgspeed = rsserval (itemhit, commbaudtbl);
		rshilite (dlgspeed, commbaudtbl, commdlg);
		break;

	      case CREF_PARITY:
		dlgparity = rsserval (itemhit, commparitytbl);
		rshilite (dlgparity, commparitytbl, commdlg);
		break;

	      default:
		printerr ("Item has no refcon: ", itemhit);
	    }
	}
    }
}				/* commdialog */



RESSERTBL protoblktbl[] = {	/* block check:  */
    {PR_BLK1, 1},		/* type 1 */
    {PR_BLK2, 2},		/* type 2 */
    {PR_BLK3, 3},		/* type 3 */
    {0, 0}
};

#define KTURN_NONE 0128		/* indicate no handshake */

RESSERTBL protohstbl[] = {	/* hand shake: */
    {PR_HSBELL, 7},		/* bell = 7 */
    {PR_HSCR, 15},		/* cr = 15 */
    {PR_HSESC, 33},		/* esc = 33  */
    {PR_HSLF, 12},		/* lf = 12 */
    {PR_HSNONE, KTURN_NONE},	/* none = 128 */
    {PR_HSXON, XON},		/* xon = 21 */
    {PR_HSXOFF, 23},		/* xoff = 23 */
    {0, 0}
};

RESSERTBL protointtbl[] = {	/* edit text integer items: */
    {PR_INPADN, 0},		/* inbound pad count edit text */
    {PR_OUTPADN, 1},		/* outbound pad count edit text */
    {PR_INTIMEO, 2},		/* inbound secs timeout edit text */
    {PR_OUTTIMEO, 3},		/* outbound secs timeout edit text */
    {PR_INPKTLEN, 4},		/* inbound packet length edit text */
    {PR_OUTPKTLEN, 5},		/* outbound packet length edit text */
    {0, 0}
};

int *protointcells[] = {	/* parallel to above table! */
    &mypadn,			/* inbound pad count */
    &npad,			/* outbound pad count */
    &timint,			/* inbound timeout in secs */
    &rtimo,			/* outbound timeout in secs */
    &urpsiz,			/* user requested inbound pkt length */
    &spsiz			/* inbound pkt length */
};

RESSERTBL protochrtbl[] = {	/* edit text character items: */
    {PR_INPADC, 0},		/* inbound pad chr edit text */
    {PR_OUTPADC, 1},		/* outbound pad chr */
    {PR_INEOP, 2},		/* inbound end of packet edit text */
    {PR_OUTEOP, 3},		/* outbound end of packet edit text */
    {PR_INSOP, 4},		/* inbound start of pkt edit text */
    {PR_OUTSOP, 5},		/* outbound start of pkt edit text */
    {0, 0}
};

char *protochrcells[] = {	/* parallel to above table! */
    &mypadc,			/* inbound pad char */
    &padch,			/* outbound pad char */
    &eol,			/* inbound end of packet char */
    &seol,			/* outbound end of pkt char */
    &stchr,			/* inbound start of pkt char */
    &mystch			/* outbound start of packet char */
};

/****************************************************************************/
/* etgetcc - Set edit text paramter from dialog window to an ASCII */
/*    	     control character value.  Returns FALSE if value is */
/*    	     illegal, warning has been displayed. */
/****************************************************************************/
int
etgetcc (item, dlg, chrcell)
DialogPtr dlg;
int item;
char *chrcell;
{
    short itemtype;
    int i;
    long rslt;
    Rect itemrect;
    Handle itemhdl;
    char itembuf[256], c;

    GetDItem (dlg, item, &itemtype, &itemhdl, &itemrect);
    if (itemtype != editText) {
	printerr ("etsetcc item not editText: ", item);
	return (FALSE);
    }
    GetIText (itemhdl, itembuf);
    /* p2cstr(itembuf);			/* convert to c string */
    for (i = 0; (c = itembuf[i]) != 0; i++)	/* check for digits */
	if (!isdigit (c)) {
	    printerr ("Field contains a non numeric, code ", c);
	    return (FALSE);
	}
    StringToNum (itembuf, &rslt);
    if ((rslt > 037) && (rslt != 0177)) {
	printerr ("Not in ASCII control range: ", (int) rslt);
	return (FALSE);
    }
    *chrcell = (char) rslt;
    return (TRUE);
}				/* etgetcc */



/****************************************************************************/
/* etgetnum - Set edit text paramet from dialog to numeric cell. */
/*    	      Returns FALSE if value is non numeric or not in range */
/*    	      in which case an error message is printed. */
/****************************************************************************/
int
etgetnum (item, dlg, intcell)
DialogPtr dlg;
int item;
int *intcell;
{
    short itemtype;
    int i;
    long rslt;
    Rect itemrect;
    Handle itemhdl;
    char itembuf[256];
    char c;

    GetDItem (dlg, item, &itemtype, &itemhdl, &itemrect);
    if (itemtype != editText) {
	printerr ("etsetnum item not editText: ", item);
	return (FALSE);
    }
    GetIText (itemhdl, itembuf);

    for (i = 0; (c = itembuf[i]) != 0; i++)	/* check for digits */
	if (!isdigit (c)) {
	    printerr ("Field contains a non numeric, code ", c);
	    return (FALSE);
	}
    StringToNum (itembuf, &rslt);

    /* check inbound packet length */
    /* MAXRP-8 is for the rest of the header (MARK, SEQ, TYPE, etc.) */
    if ((item == PR_INPKTLEN) && (rslt > MAXRP-8)) {
	printerr ("Sorry, the maximum receive packet length is ", MAXRP-8);
	return (FALSE);
    }
    /* check outbound packet length */
    if ((item == PR_OUTPKTLEN) && (rslt > MAXSP)) {
	printerr ("Sorry, the maximum send packet length is ", MAXSP);
	return (FALSE);
    }
    *intcell = (int) rslt;
    return (TRUE);
}				/* etgetnum */



#define PREF_BLKCHK 1
#define PREF_HNDSHK 2

/****************************************************************************/
/* protodialog - enter protocol setup dialog. */
/****************************************************************************/
protodialog ()
{
    DialogPtr protoDialog;
    long i;
    short itemhit;
    short itemtype;
    int dlgval;
    int dlgint;
    int dlgbctr;
    int dlgturnch;
    Handle itemhdl;
    Rect itembox;
    char dlgchr;

    protoDialog = GetNewDialog (PROTOBOXID, NILPTR, (WindowPtr) - 1);
    circleOK(protoDialog);
    
    dlgbctr = bctr;		/* init local block check */
    dlgturnch = turnch;		/* handshake */
    if (!turn)			/* no turn character? */
	dlgturnch = KTURN_NONE;	/* indicate none */

    /* for all button controls set the reference value */
    rsreference (protohstbl, PREF_HNDSHK, protoDialog);
    rsreference (protoblktbl, PREF_BLKCHK, protoDialog);

    /* for each button controls, hilite the current setting */
    rshilite (dlgturnch, protohstbl, protoDialog); /* hilite our hand shake */
    rshilite (dlgbctr, protoblktbl, protoDialog); /* hilite our block check */

    /* for each edit text item, set current value in edit text */
    for (i = 0; protochrtbl[i].resval != 0; i++)
	SetNumText (protochrtbl[i].resval,
		    (int) *protochrcells[i], protoDialog);

    for (i = 0; protointtbl[i].resval != 0; i++)
	SetNumText (protointtbl[i].resval,
		    *protointcells[i], protoDialog);

    for (;;) {
	ModalDialog (NILPROC, &itemhit);

	switch (itemhit) {
	  case OKBtn:		/* finish up */
	    turnch = dlgturnch;	/* set global handshake */
	    turn = (turnch == KTURN_NONE) ? FALSE : TRUE; /* set turn */
	    bctr = dlgbctr;	/* set block type check */

	    for (i = 0; protochrtbl[i].resval != 0; i++) /* set chr vals */
		if (!etgetcc (protochrtbl[i].resval, protoDialog,
			      protochrcells[i]))
		    fatal ("During OKBtn etgetcc failed!", (int) i);

	    for (i = 0; protointtbl[i].resval != 0; i++) /* set int vals */
		if (!etgetnum (protointtbl[i].resval, protoDialog,
			       protointcells[i]))
		    fatal ("During OKBtn etgetnum failed!", (int) i);

	  case QuitBtn:	/* fall in from above */
	    DisposDialog (protoDialog);	/* finished with the dialog */
	    return;		/* return */

	  case PR_INPADC:
	  case PR_OUTPADC:
	  case PR_INEOP:
	  case PR_OUTEOP:
	  case PR_INSOP:
	  case PR_OUTSOP:
	    if (!etgetcc (itemhit, protoDialog, &dlgchr)) {
		/* get back tbl idx */
		dlgval = rsserval (itemhit, protochrtbl);
		dlgval = (int) *protochrcells[dlgval];	/* now cell value */
		SetNumText (itemhit, dlgval, protoDialog); /* and reset it */
		SelIText(protoDialog, itemhit, 0, 32767);
	    }
	    break;

	  case PR_INPADN:
	  case PR_OUTPADN:
	  case PR_INTIMEO:
	  case PR_OUTTIMEO:
	  case PR_INPKTLEN:
	  case PR_OUTPKTLEN:
	    if (!etgetnum (itemhit, protoDialog, &dlgint)) {
		/* get back tbl idx */
		dlgval = rsserval (itemhit, protointtbl);
		dlgval = *protointcells[dlgval];	/* now cell value */
		SetNumText (itemhit, dlgval, protoDialog); /* and reset it */
		SelIText(protoDialog, itemhit, 0, 32767);
	    }
	    break;

	  default:
	    GetDItem (protoDialog, itemhit, &itemtype, &itemhdl, &itembox);

	    if (itemtype != ctrlItem + radCtrl)	/* must be radio button */
		fatal ("Not radio button", itemhit);

	    i = GetCRefCon ((ControlHandle) itemhdl);

	    if (i == PREF_BLKCHK) {
		dlgbctr = rsserval (itemhit, protoblktbl);
		rshilite (dlgbctr, protoblktbl, protoDialog);

	    } else if (i == PREF_HNDSHK) {
		dlgturnch = rsserval (itemhit, protohstbl);
		rshilite (dlgturnch, protohstbl, protoDialog);

	    } else
		printerr ("radio item has bad refcon: ", itemhit);

	    break;		/* all done with radio buttons */
	}
    }
}				/* protodialog */



#define TR_FIRST TR_AUTOWRAP
#define TR_LAST TR_BLINKC

extern int screensize;	/* number of screen lines */

/****************************************************************************/
/****************************************************************************/
termsetdialog ()
{
    DialogPtr termdlg;
    short itemhit;
    short i;
    int dlgint, t_numlines;
    Boolean ts[TR_LAST + 1];

    termdlg = GetNewDialog (TERMINALBOXID, NILPTR, (WindowPtr) - 1);
    circleOK(termdlg);
    
    ts[TR_AUTOWRAP] = autowrap;
    ts[TR_AUTOREPT] = autorepeat;
    ts[TR_INVERT] = screeninvert;
    ts[TR_SMOOTH] = smoothscroll;
    ts[TR_AUTOLF] = newline;
    ts[TR_LOCLECHO] = duplex;
    ts[TR_TRANSP] = !transparent;	/* PWP */
    ts[TR_BLOCKC] = blockcursor;
    ts[TR_MOUSE] = mouse_arrows;
    ts[TR_VISBELL] = visible_bell;
    ts[TR_EIGHTBIT] = eightbit_disp;
    ts[TR_BLINKC] = blinkcursor;
    t_numlines = screensize;

    /* set the ctl values according to the flags */
    for (i = TR_FIRST; i <= TR_LAST; i++)
	SetCtlValue (getctlhdl (i, termdlg), (ts[i]) ? btnOn : btnOff);

    SetNumText(TR_NUMLINES,t_numlines,termdlg);
    SelIText(termdlg, TR_NUMLINES, 0, 32767);
    
    for (;;) {
	ModalDialog (NILPROC, &itemhit);

	switch (itemhit) {
	  case OKBtn:		/* finish up */
	    autowrap = ts[TR_AUTOWRAP];
	    autorepeat = ts[TR_AUTOREPT];
	    smoothscroll = ts[TR_SMOOTH];
	    newline = ts[TR_AUTOLF];
	    duplex = ts[TR_LOCLECHO];
	    transparent = !ts[TR_TRANSP];
	    visible_bell = ts[TR_VISBELL];
	    eightbit_disp = ts[TR_EIGHTBIT];
	    if ((blockcursor != ts[TR_BLOCKC]) ||
		(blinkcursor != ts[TR_BLINKC]) ||
		(ts[TR_MOUSE])) {
		cursor_erase ();
		blockcursor = ts[TR_BLOCKC];
		blinkcursor = ts[TR_BLINKC];
		mouse_arrows = ts[TR_MOUSE];
		cursor_draw ();
	    }
	    if (screeninvert != ts[TR_INVERT])
		invert_term ();
	    grow_term_to(t_numlines);

	  case QuitBtn:	/* fall in from above */
	    DisposDialog (termdlg);	/* finished with the dialog */
	    return;		/* return */

	  case TR_RESETBTN:
	    term_reset();		/* reset the terminal emulator */
	    break;
	    
	  case TR_AUTOWRAP:
	  case TR_AUTOREPT:
	  case TR_INVERT:
	  case TR_SMOOTH:
	  case TR_AUTOLF:
	  case TR_LOCLECHO:
	  case TR_TRANSP:
	  case TR_BLOCKC:
	  case TR_MOUSE:
	  case TR_VISBELL:
	  case TR_EIGHTBIT:
	  case TR_BLINKC:
	    ts[itemhit] = !ts[itemhit];
	    SetCtlValue (getctlhdl (itemhit, termdlg),
	    		 (ts[itemhit]) ? btnOn : btnOff);
	    break;

	  case TR_NUMLINES:
	    if (!etgetnum (TR_NUMLINES, termdlg, &dlgint)) {
		SetNumText (TR_NUMLINES, t_numlines, termdlg);
		SelIText(termdlg, TR_NUMLINES, 0, 32767);
	    } else {
		if (dlgint > MAX_SCREENSIZE) {
		    printerr ("Sorry, the maximum screen length is",
		      MAX_SCREENSIZE);
		    SetNumText (TR_NUMLINES, t_numlines, termdlg);
		    SelIText(termdlg, TR_NUMLINES, 0, 32767);
		} else {
		    t_numlines = dlgint;
		}
	    }
	    break;
	}
    }
}				/* termsetdialog */

#define CS_AFIRST	CS_USA
#define CS_ALAST	CS_SWISS
#define CS_BFIRST	CS_ISOLATIN1
#define CS_BLAST	CS_ISOLATIN5

extern int screensize;	/* number of screen lines */
extern int nat_set;	/* current national character set */
extern int nat_char_mode;	/* FALSE if doing ISO 8859 stuff */

/****************************************************************************/
/****************************************************************************/
charsetdialog ()
{
    DialogPtr chardlg;
    short itemhit;
    short i;
    int dlg_g0 = CS_USA, dlg_g1 = CS_VTGRAPH;

    chardlg = GetNewDialog (CHARBOXID, NILPTR, (WindowPtr) - 1);
    circleOK(chardlg);
    
    if (nat_char_mode)
        dlg_g0 = nat_set + CS_USA - USA_NAT;
    else
	dlg_g0 = CS_USA;	/* Should handle GRAF_SET here too */

    switch (graphicsinset[1]) {
      case LAT1_SET:
      case DECINTL_SET:
	dlg_g1 = CS_ISOLATIN1;
	break;

      case LAT2_SET:
	dlg_g1 = CS_ISOLATIN2;
	break;

      case LAT3_SET:
	dlg_g1 = CS_ISOLATIN3;
	break;

      case LAT4_SET:
	dlg_g1 = CS_ISOLATIN4;
	break;

      case LATCYR_SET:
	dlg_g1 = CS_ISOCYRILLIC;
	break;

      case LATARAB_SET:
	dlg_g1 = CS_ISOARABIC;
	break;

      case LATGREEK_SET:
	dlg_g1 = CS_ISOGREEK;
	break;

      case LATHEBREW_SET:
	dlg_g1 = CS_ISOHEBREW;
	break;

      case LAT5_SET:
	dlg_g1 = CS_ISOLATIN5;
	break;

      case GRAF_SET:
	dlg_g1 = CS_VTGRAPH;
	break;

      case TECH_SET:
	dlg_g1 = CS_VTTECH;
	break;

    }

    for (i = CS_AFIRST; i <= CS_ALAST; i++)
	SetCtlValue (getctlhdl (i, chardlg), btnOff);
    SetCtlValue (getctlhdl (dlg_g0, chardlg), btnOn);
    for (i = CS_BFIRST; i <= CS_BLAST; i++)
	SetCtlValue (getctlhdl (i, chardlg), btnOff);
    SetCtlValue (getctlhdl (dlg_g1, chardlg), btnOn);
    
    for (;;) {

	ModalDialog (NILPROC, &itemhit);

	switch (itemhit) {
	  case OKBtn:		/* finish up */
	    switch (dlg_g0) {
	      case CS_USA:
		nat_set = USA_NAT;
		graphicsinset[0] = ASCII_SET;
		/* but we leave the setting of nat_char_mode as is */
		break;

	      case CS_UK:
	      case CS_DUTCH:
	      case CS_FINNISH:
	      case CS_FRENCH:
	      case CS_FRCAN:
	      case CS_GERMAN:
	      case CS_ITALIAN:
	      case CS_NORDAN:
	      case CS_PORTUGUESE:
	      case CS_SPANISH:
	      case CS_SWEDISH:
	      case CS_SWISS:
		graphicsinset[0] = ASCII_SET;
		nat_set = dlg_g0 - CS_USA + USA_NAT;
		nat_char_mode = TRUE;
		break;
	    }
	    switch (dlg_g1) {
	      case CS_ISOLATIN1:
		graphicsinset[1] = LAT1_SET;
		nat_char_mode = FALSE;
		break;

	      case CS_ISOLATIN2:
		graphicsinset[1] = LAT2_SET;
		nat_char_mode = FALSE;
		break;

	      case CS_ISOLATIN3:
		graphicsinset[1] = LAT3_SET;
		nat_char_mode = FALSE;
		break;

	      case CS_ISOLATIN4:
		graphicsinset[1] = LAT4_SET;
		nat_char_mode = FALSE;
		break;

	      case CS_ISOCYRILLIC:
		graphicsinset[1] = LATCYR_SET;
		nat_char_mode = FALSE;
		break;

	      case CS_ISOARABIC:
		graphicsinset[1] = LATARAB_SET;
		nat_char_mode = FALSE;
		break;

	      case CS_ISOGREEK:
		graphicsinset[1] = LATGREEK_SET;
		nat_char_mode = FALSE;
		break;

	      case CS_ISOHEBREW:
		graphicsinset[1] = LATHEBREW_SET;
		nat_char_mode = FALSE;
		break;

	      case CS_ISOLATIN5:
		graphicsinset[1] = LAT5_SET;
		nat_char_mode = FALSE;
		break;

	      case CS_VTGRAPH:
		graphicsinset[1] = GRAF_SET;
		break;

	      case CS_VTTECH:
		graphicsinset[1] = TECH_SET;
		break;
	    }
	    set_char_map();    /* change the display translate table */
	    /* fall through */
	    
	  case QuitBtn:	/* fall in from above */
	    DisposDialog (chardlg);	/* finished with the dialog */
	    return;		/* return */

	  case CS_UK:
	  case CS_DUTCH:
	  case CS_FINNISH:
	  case CS_FRENCH:
	  case CS_FRCAN:
	  case CS_GERMAN:
	  case CS_ITALIAN:
	  case CS_NORDAN:
	  case CS_PORTUGUESE:
	  case CS_SPANISH:
	  case CS_SWEDISH:
	  case CS_SWISS:
	    /* national char mode implies not ISO 8859 */
	    SetCtlValue (getctlhdl (dlg_g1, chardlg), btnOff);
	    dlg_g1 = CS_VTGRAPH;
	    SetCtlValue (getctlhdl (dlg_g1, chardlg), btnOn);
	    /* fall through */
	  case CS_USA:
	    SetCtlValue (getctlhdl (dlg_g0, chardlg), btnOff);
	    dlg_g0 = itemhit;
	    SetCtlValue (getctlhdl (dlg_g0, chardlg), btnOn);
	    break;
	    
	  case CS_ISOLATIN1:
	  case CS_ISOLATIN2:
	  case CS_ISOLATIN3:
	  case CS_ISOLATIN4:
	  case CS_ISOCYRILLIC:
	  case CS_ISOARABIC:
	  case CS_ISOGREEK:
	  case CS_ISOHEBREW:
	  case CS_ISOLATIN5:
	    /* 8859 says that the lower half (G0) is straight ASCII */
	    SetCtlValue (getctlhdl (dlg_g0, chardlg), btnOff);
	    dlg_g0 = CS_USA;
	    SetCtlValue (getctlhdl (dlg_g0, chardlg), btnOn);
	    /* fall through */
	  case CS_VTGRAPH:
	  case CS_VTTECH:
	    SetCtlValue (getctlhdl (dlg_g1, chardlg), btnOff);
	    dlg_g1 = itemhit;
	    SetCtlValue (getctlhdl (dlg_g1, chardlg), btnOn);
	    break;

	}
    }
}				/* charsetdialog */



/****************************************************************************/
/****************************************************************************/
setradpair (rid1, rid2, bool, dlg)
DialogPtr dlg;
{
    SetCtlValue (getctlhdl (rid1, dlg), bool ? btnOn : btnOff);
    SetCtlValue (getctlhdl (rid2, dlg), bool ? btnOff : btnOn);
}				/* setradpair */



/****************************************************************************/
/* setfiledialog - enter file settings dialog. */
/****************************************************************************/
setfiledialog ()
{
    DialogPtr setfdlg;
    short item = RADITM_FIRST - 1;

    setfdlg = GetNewDialog (FILEBOXID, NILPTR, (WindowPtr) - 1);
    circleOK(setfdlg);
    filargs.filflg = filargs.fildflg;	/* use current defaults */

    for (;;) {
	setfilflgs (item, setfdlg);	/* keep flags upto date */

	setradpair (FSET_ATTEND, FSET_UNATTEND,
		    (filargs.filflg & FIL_DODLG), setfdlg);
	setradpair (FSET_SUPERSEDE, FSET_NEWNAMES,
		    (filargs.filflg & FIL_OKILL), setfdlg);
	SetCtlValue (getctlhdl (FSET_KEEP, setfdlg),
		     (keep) ? btnOn : btnOff);
	SetCtlValue (getctlhdl (FSET_XMITTOO, setfdlg),
		     (sendusercvdef) ? btnOn : btnOff);

	ModalDialog (NILPROC, &item);
	switch (item) {
	  case ok:
	    filargs.fildflg = filargs.filflg;	/* set default flags */

	  case cancel:
	    DisposDialog (setfdlg);	/* all done with dialog */
	    return;		/* can go home... */

	  case FSET_ATTEND:
	  case FSET_UNATTEND:
	    filargs.filflg ^= FIL_DODLG;	/* toggle flag */
	    break;

	  case FSET_SUPERSEDE:
	  case FSET_NEWNAMES:
	    filargs.filflg ^= FIL_OKILL;	/* toggle flag */
	    break;

	  case FSET_KEEP:
	    keep = !keep;
	    break;
	    
	  case FSET_XMITTOO:
	    sendusercvdef = !sendusercvdef;
	    break;
	}
    }
}				/* setfiledialog */



struct launchparams {
    char *lnamep;
    short config;
}   lparams;
char lfile[64];



/****************************************************************************/
/* handlelaunch - Handle transfer to another application. */
/*                Called when "launch" selected from menu bar. */
/****************************************************************************/
handlelaunch ()
{
    OSType appltype = 'APPL';
    Point where;
    SFReply sfr;

    SetPt (&where, 75, 115);
    SFGetFile (&where, "", NILPROC, 1, &appltype, NILPROC, &sfr);

    if (!sfr.good)		/* hit cancel, return now */
	return;

    p2cstr (&sfr.fName);

    mac_cleanup ();		/* about to leave, leave clean! */

    strcpy (lfile, &sfr.fName);
    lparams.lnamep = lfile;
    lparams.config = 0;
    SetVol ((char *) 0, sfr.vRefNum);	/* allow any drive */
    c2pstr (lparams.lnamep);

    loadA0 (&lparams);
    Launch ();
}				/* handlelaunch */



/****************************************************************************/
/****************************************************************************/
fstats ()
{
    tfc += ffc;
    tlog(F100," end of file","",0l);
    tlog(F101,"  file characters        ","",ffc);
    tlog(F101,"  communication line in  ","",flci);
    tlog(F101,"  communication line out ","",flco);
}				/* fstats */



/****************************************************************************/
/****************************************************************************/
tstats ()
{
}				/* tstats */



/****************************************************************************/
/****************************************************************************/
rdebu (c)
int c;
{
}				/* rdebu */



/****************************************************************************/
/****************************************************************************/
sdebu (l)
int l;
{
}				/* sdebu */
