/* Paul Placeway, Ohio State -- changed the format of saved Is and Cs to */
/*  {number, item, item, ...} */
/* Version 0.8(35) - Jim Noble at Planning Research Corporation, June 1987. */
/* Ported to Megamax native Macintosh C compiler. */
/* Edit by Frank on Jun 20 17:20 */
/* Don't set sio chip parity */
/* Edit by Bill on May 29 01:01 */
/* Add key configurator */
/* Edit by Bill on May 10 9:24 */
/* Saving settings file to a different disk doesn't work and may bomb */
/* Edit by Bill on May 8 7:17 */
/* Save default file settings, now incompatable with existing save files! */
/* Edit by Bill & Jeff on May 1 14:16 */
/* findfinderfiles was bombing because of fName[1] definition of myAppFile */
/* Edit by Bill on Apr 30 05:50 */
/* Call FlushVol after saving the settings */

/*
 * file ckmsav.c
 *
 * Module of MacKermit containing code for saving and restoring
 * various variables.
 *
 * Copyright (C) 1985, Trustees of Columbia University in the City of
 * New York.  Permission is granted to any individual or institution to
 * use, copy, or redistribute this software so long as it is not sold
 * for profit, provided this copyright notice is retained.
 *
 */

#include "ckcdeb.h"		/* Kermit definitions */
#include "ckcker.h"		/* Kermit definitions */

#define	__SEG__ ckmsav
#include <files.h>
#include <errors.h>
#include <resources.h>
#include <packages.h>
#include <segload.h>
#include <windows.h>
#include <osutils.h>
#include <menus.h>

#include "ckmdef.h"		/* Common Mac module definitions */
#include "ckmres.h"		/* resource defs */

OSType kermtype = ApplCreator, settingstype = 'KERS';

int scrinvert;			/* intermediate container for screeninvert */
int scrsize;			/* ditto for size */
int savinnum;			/* intermediate container for innum (I/O port) */
char savmcmdactive;		/* intermediate container for mcmdactive */

extern int drop_dtr;

extern int dfprty;                      /* Default parity */
extern int dfflow;                      /* Default flow control */

int *inames[] = {
    &speed, &parity, &duplex, &delay, &mypadn,
    &npad, &timint, &rtimo, &urpsiz, &spsiz,
    &turnch, &turn, &bctr, &filargs.fildflg,
    &newline, &autowrap, &scrinvert, &autorepeat,
    &smoothscroll, &transparent, &keep, &blockcursor,
    &mouse_arrows, &visible_bell, &eightbit_disp,
    &blinkcursor, &scrsize, &savinnum, &sendusercvdef,
    &drop_dtr, &flow
};

#define NINAMES (sizeof(inames) / sizeof(int *))

char *cnames[] = {
    &mypadc, &padch, &eol, &seol, &stchr, &mystch,
    &fkeysactive, &savmcmdactive
};

#define NCNAMES (sizeof(cnames) / sizeof(char *))

typedef long **IHandle;		/* handle to int[] */
typedef char **CHandle;		/* handle to char[] */

extern WindowPtr terminalWindow;



/****************************************************************************/
/* Delete the specified resource if it exists in the current resource file */
/****************************************************************************/
KillResource (type, id)
ResType type;
int id;
{
    Handle theRsrc;

    theRsrc = GetResource (type, id);
    if ((theRsrc != NIL) && (HomeResFile (theRsrc) == CurResFile ())) {
	RmveResource (theRsrc);
	if (ResError () != noErr) {	/* check for error */
	    printerr ("Could not remove old resource: ", ResError ());
	    return;
	}
	DisposHandle (theRsrc);
    }
}				/* KillResource */



/****************************************************************************/
/* savevals - save variables for MacKermit */
/****************************************************************************/
savevals ()
{
    IHandle ihdl;
    CHandle chdl;

    SFReply savr;
    Point where;
    int err;
    int rfnum;
    int i;
    FInfo finf;
    char name[256];

    GetWTitle (terminalWindow, name);

    SetPt (&where, 75, 115);
    SFPutFile (&where, "Save variables in file:", name, NILPROC, &savr);
    if (!savr.good)		/* did they hit cancel? */
	return;			/* yes, so return now */

    SetVol (NILPTR, savr.vRefNum);	/* select volume for rsrc file */

    p2cstr (&savr.fName);
    rfnum = OpenResFile (&savr.fName);
    err = ResError ();
    if (err == noErr) {		/* file exists, clear old resources */
	/* Be sure to delete the old resources, if they alraedy */
	/* exist. Otherwise the resources will be added to the */
	/* existing file (adding more and more resources with */
	/* the same type and number). Unfortunately, for setting */
	/* the parameters, Kermit always uses the first, i.e. */
	/* the !oldest! set of resources! */
	KillResource (SAVI_TYPE, SIVER);
	KillResource (SAVC_TYPE, SCVER);
	KillResource (KSET_TYPE, KSVER);
	KillResource (MSET_TYPE, KMVER);
    } else if ((err == resFNotFound) || (err == fnfErr)) {
	/* file not existing ? */
	CreateResFile (&savr.fName);	/* try to create */
	if (ResError () != noErr) {	/* check for error */
	    printerr ("Unknown error from create: ", ResError ());
	    return;
	}
	/* set the file finder infos */
	err = GetFInfo (&savr.fName, savr.vRefNum, &finf);
	if (err != noErr)
	    printerr ("Can't get finder info for file: ", err);
	else {
	    finf.fdFldr = filargs.filfldr;	/* use same folder as
						 * application */
	    finf.fdType = settingstype;	/* set type */
	    finf.fdCreator = kermtype;	/* set creator */
	    err = SetFInfo (&savr.fName, savr.vRefNum, &finf);
	    if (err != noErr)
		printerr ("Can't set finder info: ", err);
	}
	/* try open again */
	rfnum = OpenResFile (&savr.fName);
	if (rfnum == -1) {	/* failed to open? */
	    printerr ("Couldn't Open resource file: ", ResError ());
	    return;
	}
    } else {
	printerr ("Couldn't Open resource file: ", err);
	return;
    }

    scrinvert = screeninvert;	/* save the current value */
    scrsize = screensize;
    savinnum = innum;		/* save current port too */
    savmcmdactive = mcmdactive;

    /*
     * PWP: changed the format so {count, item, item, ...} so that we can
     * load older versions without dying
     */

    ihdl = (IHandle) NewHandle ((long) (NINAMES + 1) * sizeof (int));
    (*ihdl)[0] = NINAMES;
    for (i = 0; i < NINAMES; i++)	/* copy from indirect table */
	(*ihdl)[i + 1] = *inames[i];
    AddResource ((Handle) ihdl, SAVI_TYPE, SIVER, "");

    chdl = (CHandle) NewHandle ((long) (NCNAMES + 1) * sizeof (char));
    (*chdl)[0] = NCNAMES;
    for (i = 0; i < NCNAMES; i++)	/* copy from indirect table */
	(*chdl)[i + 1] = *cnames[i];
    AddResource ((Handle) chdl, SAVC_TYPE, SCVER, "");

    savekset ();		/* save key bit table */
    savemset ();		/* save key macros table */

    CloseResFile (rfnum);
    FlushVol (NILPTR, savr.vRefNum);	/* flush the bits out */

    SetWTitle (terminalWindow, &savr.fName);
}				/* savevals */



/****************************************************************************/
/* do a Load settings... dialog */
/****************************************************************************/
loadvals ()
{
    SFReply savr;
    Point where;

    SetPt (&where, 75, 115);
    SFGetFile (&where, "Load variables from:", NILPROC, 1, &settingstype,
	       NILPROC, &savr);

    if (!savr.good)		/* did they hit cancel? */
	return;			/* yes, so return now */

    p2cstr (&savr.fName);
    doloadvals (&savr.fName, savr.vRefNum);	/* do the load */
}				/* loadvals */



/****************************************************************************/
/****************************************************************************/
findfinderfiles ()
{
    short msg, cnt;
    int err;
    AppFile apf;
    FInfo ainfo;

    CountAppFiles (&msg, &cnt);	/* anything clicked by user? */
    if (cnt == 0 || msg == appPrint) {	/* or they want us to print (?) */
	filargs.filfldr = fDesktop;	/* forget about loading values */
	loadkset ();		/* make new files appear on desk */
	return;			/* use our default KSET */
    }
    GetAppFiles (1, &apf);	/* get the first one */
    p2cstr (&apf.fName);

    ClrAppFiles (1);		/* done with this */
    doloadvals (&apf.fName, apf.vRefNum);	/* load the file */
    err = GetFInfo (&apf.fName,	/* get settings file info */
		    apf.vRefNum, &ainfo);
    if (err != noErr)
	printerr ("Couldn't GetFInfo for default folder: ", err);
    filargs.filfldr = ainfo.fdFldr;	/* use appl or text file's folder */
}				/* findfinderfiles */



extern MenuHandle menus[MAX_MENU + 1];	/* handle on our menus */

/****************************************************************************/
/****************************************************************************/
doloadvals (fn, refnum)
char *fn;
int refnum;
{
    int rfnum;
    int i, n;
    IHandle resinames;
    CHandle rescnames;
    int old_screensize = screensize;

    SetVol (NILPTR, refnum);	/* select volume */
    rfnum = OpenResFile (fn);	/* open the resource file */
    if (rfnum == -1) {
	printerr ("Couldn't open file: ", ResError ());
	return;
    };

    /* load 'SAVI' resource, the saved integer values, */
    /* 'SAVC' saved characters */

    if ((resinames = (IHandle) GetResource (SAVI_TYPE, SIVER)) == NIL ||
	(rescnames = (CHandle) GetResource (SAVC_TYPE, SCVER)) == NIL) {
	CloseResFile (rfnum);
	printerr ("Can't load your settings, damaged file or wrong version.",
		  0);
	return;			/* and return */
    }
    cursor_erase ();		/* hide the current cursor */

    /*
     * PWP: changed the format to {count, item, item, ...} so that we can
     * load older versions without dieing
     */

    n = (*resinames)[0];
    if (n > NINAMES)
	n = NINAMES;
    for (i = 0; i < n; i++)
	*inames[i] = (*resinames)[i + 1];

    n = (*rescnames)[0];
    if (n > NCNAMES)
	n = NCNAMES;
    for (i = 0; i < n; i++)
	*cnames[i] = (*rescnames)[i + 1];

    cursor_draw ();		/* show the new cursor */

    loadkset ();		/* load new KSET */
    loadmset ();		/* release current MSET and load new one */

    CloseResFile (rfnum);	/* no longer needed */
    
    /* change the screen if necessary */
    if (scrsize != screensize)	/* if we changed size */
	grow_term_to (scrsize);

    if (scrinvert != screeninvert)
	invert_term ();

    if (savinnum != innum) {	/* if using the other port */
	port_close();
	port_open(savinnum);
    }
    
    /* tell serial driver about new vals */
    (void) setserial (innum, outnum, speed, KPARITY_NONE);
    
    /* Frank changed main() to call init and then set flow, parity, etc.
       so we make sure they will be set right (again) after we return. */
    dfprty = parity;                    /* Set initial parity, */
    dfflow = flow;                      /* and flow control. */

    /* set the two check menus */
    ScrDmpEnb = (fkeysactive) ? scrdmpenabled : scrdmpdisabled;
    CheckItem (menus[SETG_MENU], SCRD_SETG, (fkeysactive));
    if (savmcmdactive != mcmdactive) {
	mcmdactive = savmcmdactive;
	setup_menus();
    }
    CheckItem (menus[SETG_MENU], MCDM_SETG, (mcmdactive));

    SetWTitle (terminalWindow, fn);

    /* (PWP) bounds check the values we just got to be double extra safe */
    
    if (urpsiz > MAXRP-8) {
    	printerr("Recieve packet lengh is too big", urpsiz);
	urpsiz = MAXRP-8;
    }
    if (spsiz > MAXSP) {
    	printerr("Send packet length is too big", spsiz);
	spsiz = MAXSP;
    }
}				/* doloadvals */
