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

extern CHAR filnam[];

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
    scrck = -1;
    scrnak = scrpkt = 0;
    scrpacln = scrcksum = scrwinsz = 0;
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

	if (tlevel < 0)	{	/* if no takefile running */

	    SetStrText (SRES_ITEXT, "Type a key or click the mouse to continue.",
	    	scrdlg);	/* yes, so set the text */
	    /*
	     * wait for mouse or key down and discard the event when it
	     * happens
	     */
	    while (!GetNextEvent (keyDownMask + mDownMask, &dummyEvt))
		 /* do nothing */ ;
	}
    }
    DisposDialog (scrdlg);
    scrdlg = NULL;
}				/* scrdispose */



/* ststrings - translation of SCR_ST subfunctions to descriptive text */

char *ststrings[] = {
    ": Transferred OK",		/* ST_OK */
    ": Discarded",		/* ST_DISC */
    ": Interrupted",		/* ST_INT */
    ": Skipped ",		/* ST_SKIP */
    ": Fatal error"		/* ST_ERR */
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
    SRES_PTEXT,			/* SCR_ST - status (goes in prev. text area) */
    SRES_UNDEF,			/* SCR_PN - packet number */
    SRES_UNDEF,			/* SCR_PT - packet type (special) */
    SRES_BTEXT,			/* SCR_TC - transaction complete */
    SRES_ITEXT,			/* SCR_EM - error msg (does alert) */
    SRES_ITEXT,			/* SCR_WM - warning message */
    SRES_BTEXT,			/* SCR_TU - arb text */
    SRES_BTEXT,			/* SCR_TN - arb text */
    SRES_BTEXT,			/* SCR_TZ - arb text */
    SRES_BTEXT,			/* SCR_QE - arb text */
    SRES_ITEXT			/* SCR_DT - date text */
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
    extern int spktl, rln, bctu, wslots;
    static char last_st = ST_OK;/* PWP: saves the most recent value of the
				 * status indication */

    miniparser (TRUE);		/* keep the mac going */

#ifdef COMMENT
    if (f == SCR_EM) {		/* error message? (warnings go into dialog) */
	printerr (s, 0);	/* display it */
	return;			/* and return */
    }
#endif
    
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
      case SCR_WM:		/* warning message */
      case SCR_EM:
	SysBeep(3);		/* get the user's attention */
	Delay ((long) 10, &i);
	SysBeep(3);
	Delay ((long) 10, &i);
	SysBeep(3);
	if (n != 0L) {
	    strcpy (buf, s);
	    strcat (buf, " ");
	    s = &buf[strlen(buf)];
	    NumToString(n, s);
	    s = buf;
	}
	break;
      
      case SCR_AN:		/* "AS" name is comming */
	if ((filargs.filflg & (FIL_RSRC | FIL_DATA)) == (FIL_RSRC | FIL_DATA)) {
	    /* in MacBinary mode */
	    SetStrText (SRES_FFORK, "", scrdlg);
	    SetStrText (SRES_FMODE, "MacBinary Mode", scrdlg);
	} else {
	    SetStrText (SRES_FFORK, (filargs.filflg & FIL_RSRC) ?
		    "RSRC Fork" : "Data Fork", scrdlg);
	    SetStrText (SRES_FMODE, (filargs.filflg & FIL_BINA) ?
		    "Binary Mode" : "Text Mode", scrdlg);
	}
	break;

      case SCR_PT:		/* packet type? */
	/* packet length */
	if (protocmd == SEND_FIL) {	/* sent a packet, see ckcfn2.c, spack() */
	    i = spktl-bctu;
	    if (i+2 <= MAXPACK) i += 2;	/* short packet */
	} else {		/* recieved a packet -- see ckcfn2.c, rpack() */
	    i = rln + bctu;
	    if (rln <= MAXPACK)	/* if it was a short packet */
		i += 2;		/* then add space for SEQ and TYPE */
	}
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
	if (wslots != scrwinsz) {
	    scrwinsz = wslots;
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
	if (f == ST_SKIP)
	    strcpy(buf, s);
	else
	    strcpy (buf, filnam);	/* file name; should be same as filargs.fillcl */
	strcat (buf, ststrings[c]);	/* add status */
	s = buf;
	SetStrText (SRES_BTEXT, "", scrdlg);	/* clear eg. remote size */
	SetStrText (SRES_ITEXT, "", scrdlg);	/* clear eg. date */
	break;

      case SCR_TC:		/* transaction completed */
	if (!server) {		/* are we a server? */
	    scrdispose (last_st != ST_OK); /* if not, dispose the screen */
	    return;		/* and we are done */
	}
	s = "Server transaction complete";
	break;

#ifdef COMMENT
      case SCR_DT:		/* file creation date */
        strcpy (buf, "Creation date: __/__/__ ");
	buf[15] = s[4];
	buf[16] = s[5];
	buf[18] = s[6];
	buf[19] = s[7];
	buf[21] = s[2];
	buf[22] = s[3];
	strcat (buf, &s[8]);
	s = buf;
	break;
#endif /* COMMENT */

      case SCR_QE:		/* quantity equals */
        strcpy (buf, s);
	strcat (buf, " = ");
	s = &buf[strlen(buf)];
	NumToString(n, s);
	/* p2cstr(s); */
	s = buf;
	scrck = -1;	/* force update of # Ks transfered */
	break;
    }

    if (rnum != SRES_UNDEF)	/* have DITL number for this? */
	SetStrText (rnum, s, scrdlg);	/* yes, so set the text */

    if ((i = ffc / 1024) != scrck) {	/* more K's xmitted (or new val)? */
	scrck = i;		/* remember new value */
	NumToString (scrck, buf);	/* convert to number */
	if (fsize != 0) {	/* know the size? (only local or w/attrs) */
	    strcat (buf, "/");	/* make it be a fraction */
	    NumToString (fsize / 1024,	/* figure this one out (was filargs.filsiz) */
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
