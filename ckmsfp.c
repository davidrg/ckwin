/* Version 0.8(35) - Jim Noble at Planning Research Corporation, June 1987. */
/* Ported to Megamax native Macintosh C compiler. */
/* Edit by Bill on Wed May 15, 15:48 */
/* change name of rtol to sfprtol, make it the common file rtol rtn */
/* either use RSRC & BINA if .RSRC extension, or use current defaults */
/* Edit by Bill on Wed May 15, 15:48 */
/* in initfilerecv make sure kermit flag binary is set */

/*
 * file ckmsfp.c
 *
 * Module of MacKermit containing standard file package calls:
 *
 *    dosenddialog() - Send file...
 *    dorecvdialog() - Receive file...
 *    dogetfdialog() - Get file from server...
 *
 * Bill Schilit, May, 1984
 *
 * Copyright (C) 1985, Trustees of Columbia University in the City of
 * New York.  Permission is granted to any individual or institution to
 * use, copy, or redistribute this software so long as it is not sold
 * for profit, provided this copyright notice is retained.
 *
 */

#include "ckcdeb.h"
#include "ckcker.h"		/* Kermit definitions */

#define	__SEG__ ckmsfp
#include <ctype.h>

#include <files.h>
#include <errors.h>
#include <controls.h>
#include <dialogs.h>
#include <packages.h>
#include <events.h>

#include "ckmdef.h"		/* Common Mac module definitions */
#include "ckmres.h"		/* resource defs */

FILINF filargs;			/* is global */

int radflgs[] = {FIL_DATA, FIL_RSRC, FIL_TEXT, FIL_BINA};
int radnotflgs[] = {FIL_RSRC, FIL_DATA, FIL_BINA, FIL_TEXT};

/* Globals used by SEND dialog */

SFReply sfr;			/* holds file info */
Boolean sendselflg;		/* TRUE means file was selected */
Boolean sendasflg;		/* TRUE means AS field is active */

int sendusercvdef = FALSE;	/* use rec. file mode defaults as send defaults */

/****************************************************************************/
/* gethdl - return a control handle given a resource ID */
/****************************************************************************/
Handle
gethdl (item, dp)
DialogPtr dp;
{
    short itype;
    Rect ibox;
    Handle ihdl;

    GetDItem (dp, item, &itype, &ihdl, &ibox);
    return (ihdl);
}				/* gethdl */



/****************************************************************************/
/* setfilflgs - Manage the filflg word and radio controls.
 *
 * Flags will be changed when the resource ID of item hit (passed to
 * this routine) is one of the radio items, or alternately routines can
 * modify the filflg word itself and the radio items will be updated
 * accordingly.
 *
 * N.B. Each dialog using these flags has them defined with the same
 * DITL item numbers.
 *
 */
/****************************************************************************/
setfilflgs (item, dlg)
DialogPtr dlg;
{
    ControlHandle ctlhdl;
    int i;

    switch(item) {
	case RADITM_MACB:	/* MacBinary mode */
	filargs.filflg &= ~FIL_TEXT;
	filargs.filflg |= FIL_BINA | FIL_RSRC | FIL_DATA;
	break;
	
	case RADITM_TEXT:	/* Text mode */
	if ((filargs.filflg & (FIL_RSRC | FIL_DATA)) == (FIL_RSRC | FIL_DATA))
	    filargs.filflg &= ~FIL_RSRC;
	filargs.filflg &= ~FIL_BINA;
	filargs.filflg |= FIL_TEXT;
	break;
	
	case RADITM_BINA:	/* Binary mode */
	if ((filargs.filflg & (FIL_RSRC | FIL_DATA)) == (FIL_RSRC | FIL_DATA))
	    filargs.filflg &= ~FIL_RSRC;
	filargs.filflg &= ~FIL_TEXT;
	filargs.filflg |= FIL_BINA;
	break;
	
	case RADITM_DATA:	/* Data fork */
	filargs.filflg &= ~FIL_RSRC;
	filargs.filflg |= FIL_DATA;
	break;
	
	case RADITM_RSRC:	/* Resource fork */
	filargs.filflg &= ~FIL_DATA;
	filargs.filflg |= FIL_RSRC;
	break;

    }
    /* if MacBinary mode */
    if ((filargs.filflg & (FIL_RSRC | FIL_DATA)) == (FIL_RSRC | FIL_DATA)) {
	SetCtlValue (getctlhdl(RADITM_MACB, dlg), btnOn);
	SetCtlValue (getctlhdl(RADITM_TEXT, dlg), btnOff);
	SetCtlValue (getctlhdl(RADITM_BINA, dlg), btnOff);

	SetCtlValue (getctlhdl(RADITM_DATA, dlg), btnOn);
	SetCtlValue (getctlhdl(RADITM_RSRC, dlg), btnOn);
	HiliteControl(getctlhdl(RADITM_DATA, dlg), 255);  /* disable it */
	HiliteControl(getctlhdl(RADITM_RSRC, dlg), 255);  /* disable it */
    } else {	/* not MacBinary mode */
	SetCtlValue (getctlhdl(RADITM_MACB, dlg), btnOff);
	HiliteControl(getctlhdl(RADITM_DATA, dlg), 0);  /* enable it */
	HiliteControl(getctlhdl(RADITM_RSRC, dlg), 0);  /* enable it */
	for (i = RADITM_FIRST; i <= RADITM_LAST; i++) {	/* update all */
	    ctlhdl = getctlhdl (i, dlg);    /* get a handle on his radio item */
	    SetCtlValue (ctlhdl,
	       (filargs.filflg & radflgs[i - RADITM_FIRST]) ? btnOn : btnOff);
	}
    }
}				/* setfilflgs */



/****************************************************************************/
/****************************************************************************/
setfilnams (remfid, dlg)
DialogPtr dlg;
{
    filargs.filrem[0] = 0;	/* no remote file */

    strcpy (filargs.fillcl, &sfr.fName);	/* copy sfr into local name
						 * stg */

    if (remfid != 0)		/* fetch remote name if present */
	GetIText (gethdl (remfid, dlg), filargs.filrem);	/* in an edittext field */

    binary = (filargs.filflg & FIL_BINA);	/* selected binary mode? */
}				/* setfilnams */



Boolean isfolder;

/****************************************************************************/
/* sendmydlg - SFPGetFIle item hit filter for "send file" dialog.
 *
 * This filter is called by SFPGetFile to let the programmer handle hits
 * on his custom items.  Our items are 2 sets of buttons for selecting
 * the fork (data or resource) and the transfer mode (binary or text).
 * Also we have an EditText item for filling in the AS name.
 *
 * The buttons are set when the user clicks, but they also are given
 * values depending on the selection of a file.  If the file's type is
 * "APPL" then controls "Resource" and "Binary" are automaticlly set,
 * otherwise the controls "Data" and "Text" are set.  SF-File keeps the
 * SFReply upto date when a file name is selected, but...  unfortunately
 * we are called before the SFReply is updated and so when we notice
 * "getNmList" is hit we defer our update for one cycle.
 *
 * Our other item, the TextEdit item to set the "AS" file name, can be
 * enabled or disabled to allow the selection of a file name by typing a
 * character (standard SFGetFile stuff).  By using the global flag
 * sendasflg and an event filter we switch between sending the chars to
 * SF-File for filename selection and to ModalDialog for TextEdit of our
 * AS name.  Since it would be nice to get rid of that blinking cursor
 * in the TextEdit item our DITL has an invisible EditText item which
 * becomes active for this purpose.
 *
 * At startup the AS field is disabled (the invisible EditText is
 * current and sndasflg is FALSE).  The user can make AS active by
 * clicking in it.  The user can toggle by clicking on the StatText
 * "AS" -- though I won't tell anybody if you don't.
 *
 */
/****************************************************************************/
pascal short
sendmydlg (ITEM, DLG)
short ITEM;
DialogPtr DLG;
{
    Boolean isappl;		/* file is an application */
    short RSLT;

    p2cstr (&sfr.fName);	/* convert filename to C form */
    if (sendselflg) {		/* file name selection occured? */
	sendselflg = FALSE;	/* yes, don't do this again */

	zltor (&sfr.fName, filargs.filrem);	/* convert to remote form */
	SetIText (gethdl (SEND_ASFN, DLG),	/* display converted name */
		  filargs.filrem);	/* in "As" field */
	setfilnams (SEND_ASFN, DLG);	/* set file names since double */
	/* clicking fouls us */

	filargs.filflg &= ~FIL_RBDT;	/* turn off all of our flags */
	isappl = (sfr.fType == 'APPL');	/* application? */
	if ((sfr.fName.length == 0) && !isfolder) {	/* check for folder */
	    /* change the button if necessary */
	    SetCTitle (getctlhdl (getOpen, DLG), "Open");
	    isfolder = TRUE;
	} else if ((sfr.fName.length != 0) && isfolder) {
	    SetCTitle (getctlhdl (getOpen, DLG), "Send");
	    isfolder = FALSE;
	}

	if (sendusercvdef) {	/* use recieve file type defaults */
	    filargs.filflg = filargs.fildflg;
	} else {		/* figure out defaults on our own */
	    filargs.filflg |= (isappl) ?	/* update flags */
		(FIL_RSRC | FIL_BINA) :	/* application */
		(FIL_DATA | FIL_TEXT);	/* not application */
	}
    }
    switch (ITEM) {		/* according to the item */
      case getNmList:		/* user hit file name */
	if (sendasflg) {	/* "As" active? */
	    sendasflg = FALSE;	/* yes -> deactivate "As" name */
	    SelIText (DLG, SEND_INVT, 0, 256);	/* activate invisible
						 * editText  */
	}
	sendselflg = TRUE;	/* next time around set buttons */
	break;			/* nothing more to do in this pass */

      case SEND_ASFN:		/* hit EditText for "AS" */
	sendasflg = TRUE;	/* let modal filter pass to us */
	break;

      case getOpen:		/* done? */
	setfilnams (SEND_ASFN, DLG);	/* set file names */
	break;			/* done... */

      case SEND_ALL:		/* hit send all check box */
	filargs.filflg ^= FIL_ALLFL;	/* toggle send all flag */


	SetCtlValue (getctlhdl (SEND_ALL, DLG),	/* set the ctl value
						 * according to the flag */
		     (filargs.filflg & FIL_ALLFL) ? btnOn : btnOff);
	break;
    }

    setfilflgs (ITEM, DLG);	/* check for and handle radio items */
    c2pstr (&sfr.fName);	/* convert back to pascal form */
    RSLT = ITEM;		/* pass item back */
    return RSLT;
}				/* sendmydlg */



/****************************************************************************/
/* sendfilter - SFPGetFile event filter for "send file" dialog.
 *
 * This filter is the same form as a ModalDialog filter, it is used
 * internally by SFPGetFile's ModalDialog call.  Our filter allows the
 * user to select a file by typing the first character, which is a
 * normal function of SFGetFile but is broken when we use EditText in
 * our customized SFGetFile box.  Return 0x1000+char in itemhit to
 * SFGetFile instead of letting ModalDialog handle it with a call to
 * TextEdit.  Using a global flag "sendasflg" we decide to send the
 * character to TextEdit via ModalDialog, or to SFGetFile.
 *
 */
/****************************************************************************/
pascal Boolean
sendfilter (THEDIALOG, THEEVENT, ITEMHIT)
DialogPtr THEDIALOG;
EventRecord *THEEVENT;
short *ITEMHIT;
{
    Boolean RETVAL;		/* returned value */

    RETVAL = FALSE;		/* default is to left Modal handle */

    if (THEEVENT->what == keyDown)	/* key down? */
	if (!sendasflg) {	/* and the text isn't selected? */
	    /* then use as file name selector */
	    /* return char+0x1000 to SFPGetfile... */
	    *ITEMHIT = 0x1000 + (THEEVENT->message & 0x7f);
	    RETVAL = TRUE;	/* let SFP handle it, not modal */
	}
    return RETVAL;
}				/* sendfilter */



/****************************************************************************/
/* recvmydlg - SFPPutFile item filter for "receive file" dialog. */
/****************************************************************************/
pascal short
recvmydlg (ITEM, DLG)
short ITEM;
DialogPtr DLG;
{
    short RSLT;

    RSLT = ITEM;

    switch (ITEM) {
      case RECV_PROC:
	filargs.filflg &= ~FIL_DODLG;	/* no more dialogs for me! */
	RSLT = RECV_RBTN;	/* did "OK" */

      case RECV_RBTN:
	p2cstr (&sfr.fName);	/* convert filename to C form */
	setfilnams (0, DLG);	/* set names */
	c2pstr (&sfr.fName);	/* convert back to pascal form */
	break;
    }
    setfilflgs (ITEM, DLG);	/* check for and handle radios */
    return RSLT;
}				/* recvmydlg */



short recvpt[] = {75, 100};	/* Used as Point */
short sendpt[] = {75, 80};	/* Used as Point */

/****************************************************************************/
/* dosenddialog - Use SFPGetFile to fetch a file to send. */
/* lclf: local file name;  remf: remote file name */
/****************************************************************************/
dosenddialog (lclf, remf)
char *lclf[], *remf[];
{
    filargs.filflg = 0;
    sendasflg = FALSE;		/* "AS" starts off inactive */
    sendselflg = TRUE;		/* need to update file buttons */

    isfolder = FALSE;
    SFPGetFile ((Point *) sendpt, "", NILPROC, -1,	/* all file types, 2
							 * filters */
		NILPTR, sendmydlg, &sfr, DLG_SEND, sendfilter);
    *lclf = filargs.fillcl;
    *remf = filargs.filrem;
    filargs.filvol = sfr.vRefNum;	/* remember volume number */
    return (sfr.good);		/* pass back return */
}				/* dosenddialog */



/****************************************************************************/
/****************************************************************************/
dorecvdialog (fn, lclf)
char *lclf[], *fn;
{
    int err;

    if (!(filargs.filflg & FIL_DODLG)) { /* don't want to do dialogs? */
	*lclf = "";		/* then use a null string */
	return;			/* and return now */
    }
    for (;;) {			/* keep trying */
	filargs.filflg &= ~(FIL_RBDT);	/* clear file modes */
	filargs.filflg |= sfprtol (fn);	/* convert fn and set modes */
	SFPPutFile ((Point *) recvpt, "Receive as:", fn,
		    recvmydlg, &sfr, DLG_RECV, NILPROC);
	*lclf = filargs.fillcl;
	filargs.filvol = sfr.vRefNum;	/* remember volume number */

	if (!sfr.good) {	/* CANCEL */
	    cxseen = TRUE;	/* indicate cancel */
	    return;
	} else {
	    /* delete if there */
	    err = FSDelete (filargs.fillcl, filargs.filvol);
	    if (err == fnfErr)	/* everything ok? */
		return;		/* yes, return now */
	}

	if (ioutil (err))
	    return;		/* until no error */
    }
}				/* dorecvdialog */



/****************************************************************************/
/****************************************************************************/
initfilrecv ()
{
    filargs.filflg = filargs.fildflg;	/* default flags are active */
    warn = !(filargs.filflg & FIL_OKILL);	/* set kermit flag */
    binary = (filargs.filflg & FIL_BINA);	/* selected binary mode? */
    filargs.filsiz = 0;		/* no known size */
}				/* initfilrecv */



/****************************************************************************/
/* initialize the file settings, called only at init time		    */
/****************************************************************************/
initfilset ()
{
    filargs.filvol = 0;		/* default volume is always default */
    filargs.fildflg = FIL_DATA | FIL_TEXT;	/* default file setting */
    /* if no settings file */
}				/* initfilset */



/****************************************************************************/
/****************************************************************************/
int
dogetfdialog (remf)
char *remf[];
{
    DialogPtr getfdlg;
    ControlHandle getbhdl;
    Handle remfhdl;
    short item;

    getfdlg = GetNewDialog (DLG_GETF, NILPTR, (WindowPtr) - 1);
    circleOK(getfdlg);
    
    remfhdl = gethdl (GETF_REMF, getfdlg);
    getbhdl = getctlhdl (GETF_GETB, getfdlg);
    HiliteControl (getbhdl, 255);	/* start with deactive Get button */

    for (;;) {
	ModalDialog (NILPROC, &item);
	switch (item) {
	  case ok:
	    if (filargs.filrem[0] == '\0')	/* no file name? */
		break;		/* then they hit CR, don't allow */
	    *remf = filargs.filrem;	/* fill in for return */
	    initfilrecv ();	/* init recv flags */
	  case cancel:
	    DisposDialog (getfdlg);
	    return (item == ok);
	  case GETF_REMF:
	    GetIText (remfhdl, filargs.filrem);
	    HiliteControl (getbhdl, (filargs.filrem[0] == 0) ? 255 : 0);
	    break;
	}
    }
}				/* dogetfdialog */



#define RSXLEN 5		/* ".rsrc" length */

/****************************************************************************/
/* sfprtol - translate remote file name to a local file name and */
/*    	  and figure out the flags as well. */
/****************************************************************************/
int
sfprtol (fn)
char *fn;
{
    int l;
    register char *cp;
    
    for (cp = fn; *cp; cp++) {		/* convert to lower case */
	if (isupper(*cp))
	    *cp = tolower(*cp);
	if (*cp == ':')			/* nuke colons (for Mac) */
	    *cp = '_';
    }
    
    if ((l = strlen (fn)) > RSXLEN &&	/* big enough? */
	(strcmp (&fn[l - RSXLEN], ".rsrc") == 0 ||	/* and matches
							 * extension? */
	 strcmp (&fn[l - RSXLEN], ".RSRC") == 0)) {	/* either way? */
	fn[l - RSXLEN] = '\0';	/* so remove the extension */
	return (FIL_BINA | FIL_RSRC);	/* want rsrc and binary */
    }
    return (filargs.fildflg & FIL_RBDT);	/* else return default */
}				/* sfprtol */

/****************************************************************************/
/*
 *	set_cwd - set the working directory
 *	   Originally wdset() from NCSA Telnet for the Macintosh, v2.2
 */
/****************************************************************************/

short cwdpt[] = {100, 100};	/* Used as Point */

int
set_cwd()
{
	SFReply reply;
	int err;

	err=SetVol (0L, filargs.filvol);
	SFPPutFile ((Point *) sendpt, "Set Transfer Directory",
		"Doesn't Matter", 0L, &reply, CWDBOXID, 0L);

	if(!reply.good) return(-1);
	filargs.filvol = reply.vRefNum;
	err = SetVol (0L, filargs.filvol);
	if (err != noErr)
	    printerr("Trouble setting transfer directory:", err);
	return (err);
}
