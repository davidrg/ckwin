/* Version 0.8(35) - Jim Noble at Planning Research Corporation, June 1987. */
/* Ported to Megamax native Macintosh C compiler. */
/* Edit by Bill on Wed May 15, 15:53 */
/* A K is 1024 not 512 */

/*
 * file ckmscr.c
 *
 * Module of mackermit containing code for display of status dialog.
 *
 * Bill Schilit, May 1984
 *
 * Copyright (C) 1985, Trustees of Columbia University in the City of
 * New York.  Permission is granted to any individual or institution to
 * use, copy, or redistribute this software so long as it is not sold
 * for profit, provided this copyright notice is retained.
 *
 */

#include "ckcdeb.h"		/* C kermit debuggig */
#include "ckcker.h"		/* general kermit defs */

#define	__SEG__ ckmscr
#include <controls.h>
#include <dialogs.h>
#include <osutils.h>
#include <events.h>

#include "ckmdef.h"		/* General Mac defs */
#include "ckmres.h"		/* Mac resource equates */

int scrpkt, scrnak;		/* NAK, PKT counts */
long scrck;			/* char (K) count */
int scrpacln, scrcksum, scrwinsz;	/* pkt len, checksum, win size */
DialogPtr scrdlg = (DialogPtr) NULL;	/* screen's dialog */



/****************************************************************************/
/* scrcreate - create the status display.  Called when a protocol
 *    	       menu item is selected and a display is desired (I don't
 *    	       think you'd want to see this for REMOTE command).
 *
 */
/****************************************************************************/
scrcreate ()
{
    char *s;

    if (scrdlg != NULL)
	printerr ("scrcreate with active screen!", 0);

    scrdlg = GetNewDialog (SCRBOXID, NILPTR, (WindowPtr) - 1);
    scrck = scrnak = scrpkt = 0;
    SetStrText (SRES_DIR, (protocmd == SEND_FIL) ? "   Sending" : "Receiving",
		scrdlg);
    miniparser (TRUE);		/* keep things moving */
}				/* scrcreate */



/****************************************************************************/
/* scrdispose - called to finish up the status display, on a
 *    	      	transaction complete screen() call.
 */
/****************************************************************************/
scrdispose (wait)
Boolean wait;
{
    int i;
    EventRecord dummyEvt;

    if (scrdlg == NULL)
	printerr ("scrdispose called with no screen active!", 0);

    SysBeep (1);

    if (wait) {			/* deactivate buttons */
	HiliteControl (getctlhdl (SRES_CANF, scrdlg), 255);
	HiliteControl (getctlhdl (SRES_CANG, scrdlg), 255);

	if (tlevel < 0)		/* if no takefile running */

	    /*
	     * wait for mouse or key down and discard the event when it
	     * happens
	     */
	    while (!GetNextEvent (keyDownMask + mDownMask, &dummyEvt))
		 /* do nothing */ ;
    }
    DisposDialog (scrdlg);
    scrdlg = NULL;
}				/* scrdispose */



/* ststrings - translation of SCR_ST subfunctions to descriptive text */

char *ststrings[] = {
    "Transferred OK",		/* ST_OK */
    "Discarded",		/* ST_DISC */
    "Interrupted",		/* ST_INT */
    "Skipped ",			/* ST_SKIP */
    "Fatal error"		/* ST_ERR */
};

/* scrtosresnum - table to translate from SCR_XXX values into resource
 *    	       	  item numbers.  Entries we aren't interested in are
 *    	          set to SRES_UNDEF.
 */

int scrtoresnum[] = {
    SRES_UNDEF,			/* 0 - nothing */
    SRES_FILN,			/* SCR_FN - filename */
    SRES_AFILN,			/* SCR_AN - as filename */
    SRES_UNDEF,			/* SCR_FS - file size */
    SRES_UNDEF,			/* SCR_XD - x-packet data */
    SRES_BTEXT,			/* SCR_ST - status */
    SRES_UNDEF,			/* SCR_PN - packet number */
    SRES_UNDEF,			/* SCR_PT - packet type (special) */
    SRES_BTEXT,			/* SCR_TC - transaction complete */
    SRES_UNDEF,			/* SCR_EM - error msg (does alert) */
    SRES_UNDEF,			/* SCR_WM - warning message */
    SRES_BTEXT,			/* SCR_TU - arb text */
    SRES_BTEXT,			/* SCR_TN - arb text */
    SRES_BTEXT,			/* SCR_TZ - arb text */
    SRES_BTEXT			/* SCR_QE - arb text */
};

/****************************************************************************/
/* screen - display status information on the screen during protocol.
 *    	    Here we just set the items in their StatText dialog boxes,
 *    	    updates occur through the miniparser, which we are nice
 *    	    enough to call here to handle things.
 *
 */
/****************************************************************************/
screen (f, c, n, s)
int f;
char c;
char *s;
long n;
{
    int rnum;
    long i;
    int itype;
    Handle ihdl;
    Rect ibox;
    char buf[256];
    extern int spktl, rln, bctu, wsize;
    static char last_st = ST_OK;/* PWP: saves the most recent value of the
				 * status indication */

    miniparser (TRUE);		/* keep the mac going */

    if (f == SCR_EM || f == SCR_WM) {	/* error/warning message? */
	printerr (s, 0);	/* display it */
	return;			/* and return */
    }
    if ((scrdlg == NULL) || quiet)	/* not using it or silent? */
	return;			/* but nothing for us to do */

    if (f == SCR_FN) {		/* seeing a file name? */
	SetStrText (SRES_AFILN, "", scrdlg);	/* and the name also */
	if (protocmd == RECV_FIL ||	/* seeing a file name from */
	    protocmd == GETS_FIL)	/* a receive command? */
	    dorecvdialog (s, &cmarg2);	/* yes, allow user to do dialog */
    }
    rnum = scrtoresnum[f];	/* load default DITL number */
    /* where result will be posted */

    switch (f) {		/* according to function... */
      case SCR_AN:		/* "AS" name is comming */
	SetStrText (SRES_FFORK, (filargs.filflg & FIL_RSRC) ?
		    "RSRC Fork" : "Data Fork", scrdlg);
	SetStrText (SRES_FMODE, (filargs.filflg & FIL_BINA) ?
		    "Binary Mode" : "Text Mode", scrdlg);
	break;

      case SCR_PT:		/* packet type? */
	/* packet length */
	i = (spktl > rln) ? spktl : rln;
	if (i != scrpacln) {
	    scrpacln = i;
	    NumToString (scrpacln, buf);	/* PWP: do xmit length */
	    SetStrText (SRES_PACSZ, buf, scrdlg);
	}
	/* checksum type */
	if (bctu != scrcksum) {
	    scrcksum = bctu;
	    NumToString (scrcksum, buf);	/* PWP: and rec length */
	    SetStrText (SRES_CKSUM, buf, scrdlg);
	}
	/* window size */
	if (wsize != scrwinsz) {
	    scrwinsz = wsize;
	    NumToString (scrwinsz, buf);	/* PWP: and rec length */
	    SetStrText (SRES_WINSZ, buf, scrdlg);
	}
	if (c == 'Y')
	    return;		/* don't do anything for yaks */

	if (c == 'N' || c == 'Q' ||	/* check for all types of */
	    c == 'T' || c == '%')	/* NAK */
	    i = ++scrnak, rnum = SRES_NRTY; /* increment nak counter */
	else
	    i = ++scrpkt, rnum = SRES_NPKT; /* else increment pkt counter */

	NumToString (i, buf);	/* translate to number */
	s = buf;		/* new buffer */
	break;			/* all done */

      case SCR_ST:		/* status */
	last_st = c;		/* PWP: save for later */
	if (c == ST_SKIP) {	/* if skipped... */
	    strcpy (buf, ststrings[c]);	/* there is something else */
	    strcat (buf, s);	/* add in filename */
	    s = buf;
	} else
	    s = ststrings[c];	/* use subfunction description */
	break;

      case SCR_TC:		/* transaction completed */
	if (!server) {		/* are we a server? */
	    scrdispose (last_st != ST_OK); /* if not, dispose the screen */
	    return;		/* and we are done */
	}
	s = "Server transaction complete";
	break;
    }

    if (rnum != SRES_UNDEF)	/* have DITL number for this? */
	SetStrText (rnum, s, scrdlg);	/* yes, so set the text */

    if ((i = ffc / 1024) != scrck) {	/* more K's xmitted (or new val)? */
	scrck = i;		/* remember new value */
	NumToString (scrck, buf);	/* convert to number */
	if (filargs.filsiz != 0) {	/* know the size? (only local) */
	    strcat (buf, "/");	/* make it be a fraction */
	    NumToString (filargs.filsiz / 1024,	/* figure this one out */
			 (char *) buf + strlen (buf));
	}
	SetStrText (SRES_KXFER, buf, scrdlg);	/* set new value */
    }
}				/* screen */



/****************************************************************************/
/* scrmydlg - handle dialog events occuring in the screen (status)
 *    	      dialog.  Called by the miniparser when a dialog event
 *    	      occurs and we are supposed to handle it.
 */
/****************************************************************************/
scrmydlg (item)
int item;
{
    switch (item) {
      case SRES_CANF:
	cxseen = TRUE;
	break;
      case SRES_CANG:
	czseen = TRUE;
	break;
    }
}				/* scrmydlg */
