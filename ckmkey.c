/*
 * CKMKEY.C
 *
 * This file contains all procedures and data related to the handling
 * of key macros.
 *
 * Matthias Aebi, ECOFIN Research and Consulting, Ltd., Oct 1987
 *
 * Chaged 12/11/87 Paul Placeway @ Ohio State University: changed the
 *  internal storage of a key macro from a C string to a Pascal string
 *  (so that I could bind Control-Space to send ASCII NUL (0x00)
 *
 * Copyright (C) 1985, Trustees of Columbia University in the City of
 * New York.  Permission is granted to any individual or institution to
 * use, copy, or redistribute this software so long as it is not sold
 * for profit, provided this copyright notice is retained.
 *
 */

#include "ckcdeb.h"		/* Kermit definitions */
#include "ckcker.h"		/* Kermit definitions */

#define	__SEG__ ckmkey
#include <resources.h>
#include <events.h>
#include <dialogs.h>
#include <controls.h>
#include <toolutils.h>
#include <Memory.h>
#include <ctype.h>

#include "ckmdef.h"		/* Common Mac module definitions */
#include "ckmres.h"		/* resource defs */


/* KSET This structure tells for which key / modifier combinations */
/* there is a special macro declaration. */
/* MSET This structure holds all macrokey definitons in a packed */
/* form. To be useful it has to be unpacked into a macrodefs	 */
/* structure. */

hmacrodefs macroshdl;

char keytable[512];



/****************************************************************************/
/* Look for a macrodefinition for theCode and return its table index if */
/* found or -1 if not found */
/****************************************************************************/
short
FindMacro (theCode)
short theCode;
{
    short i;
    short num;
    macrodefs *macros;

    macros = *macroshdl;

    num = macros->numOfMacros;
    for (i = 0; (i < num) && (macros->mcr[i].code != theCode); i++);

    if (i >= num)
	return (-1);
    else
	return (i);
}				/* FindMacro */



/****************************************************************************/
/* Set theCode at theIndex to -1 which means this entry is empty. Release */
/* the memory block occupied to save the macrostring if necessary */
/****************************************************************************/
DeleteMacro (theIndex)
short theIndex;
{
    macrodefs *macros;

    macros = *macroshdl;

    macros->mcr[theIndex].code = -1;
    if (macros->mcr[theIndex].len > 4)	/* release memory block if there is
					 * one */
	DisposPtr ((Ptr) macros->mcr[theIndex].macro);
}				/* DeleteMacro */



/****************************************************************************/
/* Add a new macro table entry at the end and return its index */
/****************************************************************************/
short
NewMacro ()
{
    macrodefs *macros;
    long hsize;
    short num;

    macros = *macroshdl;
    macros->numOfMacros++;
    num = macros->numOfMacros;

    hsize = GetHandleSize ((Handle) macroshdl);
    if ((num * sizeof (macrorec) + MacroBaseSize) > hsize) {
	/* allocate room for 10 more definitions */
	HUnlock ((Handle) macroshdl);
	SetHandleSize ((Handle) macroshdl, hsize + 10 * sizeof (macrorec));
	HLock ((Handle) macroshdl);
    }
    /* init the code field to -1 (empty) */
    macros = *macroshdl;
    macros->mcr[--num].code = -1;

    return (num);
}				/* NewMacro */



/****************************************************************************/
/* Enter theCode and theStr into the (empty!) entry at tabIndex. SetMacro  */
/* does	not release a previously existing string. Do this with DeleteMacro */
/* first. */
/****************************************************************************/
SetMacro (theIndex, theCode, theFlags, theStr)
short theIndex;
short theCode;
char *theFlags;
char *theStr;			/* PWP: note: this is now a Pascal string */
{
    macrodefs *macros;
    short slen;
    Ptr sptr;

    macros = *macroshdl;

    if (macros->mcr[theIndex].code != -1)	/* check for free entry */
	printerr ("snh SetMacro", 0);

    macros->mcr[theIndex].code = theCode;
    macros->mcr[theIndex].flags = *theFlags;
    if (*theFlags) {	/* (PWP) save a bit of space: if flags, then no string */
	macros->mcr[theIndex].macro = 0;
	macros->mcr[theIndex].len = 1;
    } else {
	slen = theStr[0] + 1;	/* PWP: was ... = strlen(theStr); */
	if (slen > 4) {
	    sptr = NewPtr (slen);
	    BlockMove (theStr, sptr, slen);	/* PWP: yes, we save the length too */
	    macros->mcr[theIndex].macro = (long) sptr;
	} else {
	    BlockMove (theStr, &macros->mcr[theIndex].macro, slen);
	}
	macros->mcr[theIndex].len = slen;
    }
}				/* SetMacro */



/****************************************************************************/
/* Save theStr as macrostring for the code. Reuse any empty table entires */
/****************************************************************************/
InsertMacro (theCode, theFlags, theStr)
short theCode;
char *theFlags;
char *theStr;			/* PWP: a Pascal string */
{
    short idx;

    HLock ((Handle) macroshdl);

    if (FindMacro (theCode) >= 0)	/* does the entry exist already */
	printerr ("snh InsertMacro", 0);

    idx = FindMacro (-1);	/* look for a free entry */
    if (idx < 0)		/* create a new free entry if none is
				 * available */
	idx = NewMacro ();
    SetMacro (idx, theCode, theFlags, theStr);

    HUnlock ((Handle) macroshdl);
}				/* InsertMacro */



/****************************************************************************/
/* Remove the macro definition from the table and mark its entry as empty */
/****************************************************************************/
RemoveMacro (theCode)
short theCode;
{
    short idx;

    HLock ((Handle) macroshdl);

    idx = FindMacro (theCode);	/* look for the entry */
    if (idx >= 0)
	DeleteMacro (idx);	/* delete it if we found it */
    else
	printerr ("snh RemoveMacro", 0);

    HUnlock ((Handle) macroshdl);
}				/* RemoveMacro */



/****************************************************************************/
/* Replace the macro definition in the table */
/****************************************************************************/
ReplaceMacro (theCode, theFlags, theStr)
short theCode;
char *theFlags;
char *theStr;
{
    short idx;

    HLock ((Handle) macroshdl);

    idx = FindMacro (theCode);	/* look for the entry */
    if (idx >= 0) {
	DeleteMacro (idx);	/* delete it if we found it */
	/* reuse it immediately */
	SetMacro (idx, theCode, theFlags, theStr);
    } else
	printerr ("snh ReplaceMacro", 0);

    HUnlock ((Handle) macroshdl);
}				/* ReplaceMacro */



/****************************************************************************/
/* Get the macro string for theCode from the table */
/****************************************************************************/
GetMacro (theCode, theFlags, theStr)
short theCode;
char *theFlags;
char *theStr;
{
    short idx;
    macrodefs *macros;
    short slen;
    Ptr sptr;

    HLock ((Handle) macroshdl);
    macros = *macroshdl;

    idx = FindMacro (theCode);	/* look for the entry */
    if (idx >= 0) {
	slen = macros->mcr[idx].len;

	if (slen > 4)
	    sptr = (Ptr) macros->mcr[idx].macro;
	else
	    sptr = &macros->mcr[idx].macro;

	BlockMove (sptr, theStr, slen);
	/* *(theStr + slen) = '\0'; */

	*theFlags = macros->mcr[idx].flags;
    } else
	printerr ("snh GetMacro", theCode);

    HUnlock ((Handle) macroshdl);
}				/* GetMacro */



/****************************************************************************/
/* dipose all macro strings */
/****************************************************************************/
DisposeMacros ()
{
    short i;
    short num;
    macrodefs *macros;

    HLock ((Handle) macroshdl);
    macros = *macroshdl;

    num = macros->numOfMacros;
    for (i = 0; i < num; i++)
	if (macros->mcr[i].code != -1)
	    DeleteMacro (i);

    macros->numOfMacros = 0;
    HUnlock ((Handle) macroshdl);
}				/* DisposeMacros */



/****************************************************************************/
/* compress '\' expressions */
/****************************************************************************/
EncodeString (s, flags)
char *s;			/* PWP: takes a C string, returns a Pascal
				 * string. */
char *flags;
{
    register char *t, *b;
    register int v, i;
    char buf[256];

    *flags = '\0';		/* no flags set */

    if ((strcmp (s, "\\break") == 0) ||
	(strcmp (s, "\\shortbreak") == 0)) {
	*flags = shortBreak;
	return;
    } else
    if (strcmp (s, "\\longbreak") == 0) {
	*flags = longBreak;
	return;
    } else
    if (strcmp (s, "\\leftarrow") == 0) {
	*flags = leftArrowKey;
	return;
    } else
    if (strcmp (s, "\\rightarrow") == 0) {
	*flags = rightArrowKey;
	return;
    } else
    if (strcmp (s, "\\uparrow") == 0) {
	*flags = upArrowKey;
	return;
    } else
    if (strcmp (s, "\\downarrow") == 0) {
	*flags = downArrowKey;
	return;
    } else
    if (strcmp (s, "\\pf1") == 0) {
	*flags = keypf1;
	return;
    } else
    if (strcmp (s, "\\pf2") == 0) {
	*flags = keypf2;
	return;
    } else
    if (strcmp (s, "\\pf3") == 0) {
	*flags = keypf3;
	return;
    } else
    if (strcmp (s, "\\pf4") == 0) {
	*flags = keypf4;
	return;
    } else
    if (strcmp (s, "\\enter") == 0) {
	*flags = keyenter;
	return;
    } else
    if ((strncmp (s, "\\keypad", 7) == 0) && (s[8] == '\0')) {
	if ((s[7] >= ',') && (s[7] <= '9')) {
	    *flags = keycomma + (s[7] - ',');
	}
	return;
    }
    *(s + 255) = '\0';
    b = s;
    t = buf;

    while (*s != '\0') {
	if (*s != '\\')
	    *t = *s++;
	else if isdigit(*++s) {		/* if \digits */
	    /* the current char was a backslash */
	    for (i = 0, v = 0; i < 3; i++) {
		/* only do the first 3 digits: \0335 -> ^[5 */
		if (!isdigit(*s))
		    break;
		v = (8 * v) + (int) *s++ - (int) '0';
	    }
	    *t = (char) v % 256;
	} else {
	    switch (*s) {
	      case 'b':	/* backspace */
		*t = '\010';
		break;

	      case 't':	/* tab */
		*t = '\011';
		break;

	      case 'n':	/* newline -- linefeed */
		*t = '\012';
		break;

	      case 'f':	/* formfeed */
		*t = '\014';
		break;

	      case 'r':	/* return */
		*t = '\015';
		break;

	      case '^':	/* \^c --> control-C */
		s++;
		if (*s == '?')
		    *t = '\177';/* special case */
		else
		    *t = (*s & 037);
		break;

	      default:
		*t = *s;
	    }
	    s++;
	}
	t++;
    }
    b[0] = (char) (t - buf);	/* PWP: the length */
    BlockMove (buf, &b[1], b[0]);	/* copy the new string in */
}				/* EncodeString */



/****************************************************************************/
/* Decode the pascal string into a C string with '\' notation */
/****************************************************************************/
DecodeString (s, flags)
char *s;			/* takes a Pascal string, returns a C string */
char flags;			/* PWP: note! not a pointer */
{
    register unsigned char ch;
    register char *tp;
    char numStr[4];
    char t[400];		/* PWP: actually, this probably won't
				 * overflow 256, but be safe */
    register int i, j;

    switch (flags) {
      case shortBreak:
	strcpy (s, "\\break");
	return;

      case longBreak:
	strcpy (s, "\\longbreak");
	return;

      case leftArrowKey:
	strcpy (s, "\\leftarrow");
	return;

      case rightArrowKey:
	strcpy (s, "\\rightarrow");
	return;
	
      case upArrowKey:
	strcpy (s, "\\uparrow");
	return;
	
      case downArrowKey:
	strcpy (s, "\\downarrow");
	return;
	
      case keypf1:
      case keypf2:
      case keypf3:
      case keypf4:
	strcpy (s, "\\pf");
	s[3] = flags - keypf1 + '1';
	s[4] = '\0';
	return;
	
      case keyenter:
	strcpy (s, "\\enter");
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
	strcpy (s, "\\keypad");
	s[7] = flags - keycomma + ',';
	s[8] = '\0';
	return;
    }

    tp = t;
    for (i = 1; i <= s[0]; i++) {	/* PWP: step through a Pascal string */
	ch = s[i];
	if ((ch < ' ') || (ch > 126)) {
	    *tp++ = '\\';
	    j = (long) ch & 0377;	/* mask of sign extension */
	    *tp++ = (j / 0100) + '0';	/* 64s digit */
	    j &= 077;
	    *tp++ = (j / 010) + '0';	/* 8s digit */
	    j &= 07;
	    *tp++ = j + '0';	/* 1s digit */
	} else if (ch == '\\') {
	    *tp++ = '\\';
	    *tp++ = '\\';
	} else
	    *tp++ = ch;
    }
    *tp = '\0';
    t[255] = '\0';		/* be extra safe */
    strcpy (s, t);		/* copy it into place */
}				/* DecodeString */



#define KeyPressed 2		/* dummy item. returned when a key is pressed */
#define myKeyCodeMask 0x7F00
#define keyModifierMask 0x1F00
short lastCode;

/****************************************************************************/
/* return KeyPressed and TRUE if a keyevent happened */
/****************************************************************************/
pascal Boolean
keyfilter (theDialog, theEvent, itemHit)
DialogPtr theDialog;
EventRecord *theEvent;
short *itemHit;
{
    Boolean retVal;
    char modstr[45];
    char theChar[3];
    char codeStr[5];

    retVal = (theEvent->what == keyDown);
    if (retVal) {
	*itemHit = KeyPressed;

	/* show modifiers pressed */
	*modstr = '\0';
	if (theEvent->modifiers & shiftKey)
	    strcat (modstr, " shift");
	if (theEvent->modifiers & alphaLock)
	    strcat (modstr, " lock");
	if (theEvent->modifiers & optionKey)
	    strcat (modstr, " option");

#ifndef controlKey
#define controlKey 4096		/* PWP: hack for beta MPW C */
#endif

	if (theEvent->modifiers & controlKey)
	    strcat (modstr, " control");
	if (theEvent->modifiers & cmdKey)
	    strcat (modstr, " command");

	strcpy (theChar, "  ");
	*(theChar + 1) = (theEvent->message & charCodeMask);
	strcat (modstr, theChar);

	lastCode = ((theEvent->message & myKeyCodeMask) >> 8) +
	    ((theEvent->modifiers & keyModifierMask) >> 1);
	NumToString (lastCode, codeStr);
	strcat (modstr, "  (");
	strcat (modstr, codeStr);
	strcat (modstr, ")");

	if (BitTst (keytable, lastCode))
	    strcat (modstr, " bound to:");
	else
	    strcat (modstr, " [unbound]");

	SetIText (gethdl (KY_MODIFIER, theDialog), modstr);
    }
    return (retVal);
}				/* keyfilter */



#define HelpBtn 3

/****************************************************************************/
/* runs the set key macros dialog */
/****************************************************************************/
keymacros ()
{
    DialogPtr macrodlg;
    DialogPtr macro2dlg;
    short itemhit;
    int i;
    char keystr[256];
    char flags;

    macrodlg = GetNewDialog (KEYBOXID, NILPTR, (WindowPtr) - 1);

    for (;;) {
	SetIText (gethdl (KY_MODIFIER, macrodlg), "Press the key to program");
	ModalDialog (keyfilter, &itemhit);

	switch (itemhit) {
	  case OKBtn:		/* finish up */
	    DisposDialog (macrodlg);	/* finished with the dialog */
	    return;		/* return */

	  case KeyPressed:	/* fall in from above */
	    macro2dlg = GetNewDialog (KEY2BOXID, NILPTR, (WindowPtr) - 1);
	    circleOK(macro2dlg);

	    /* display the current macrostring if there is one */
	    if (BitTst (keytable, lastCode)) {
		GetMacro (lastCode, &flags, keystr);
		DecodeString (keystr, flags);	/* decode invisible
						 * characters */
		SetIText (gethdl (KY_TEXT, macro2dlg), keystr);
		SelIText(macro2dlg, KY_TEXT, 0, 32767);
	    }
	    itemhit = 0;
	    while (itemhit == 0) {
		ModalDialog (NILPROC, &itemhit);

		switch (itemhit) {
		  case OKBtn:	/* finish up */
		    GetIText (gethdl (KY_TEXT, macro2dlg), keystr);
		    EncodeString (keystr, flags);	/* encode '\'
							 * expressions */
		    if (BitTst (keytable, lastCode))
			if (strlen (keystr) > 0)
			    ReplaceMacro (lastCode, flags, keystr);
			else {
			    RemoveMacro (lastCode);
			    BitClr (keytable, lastCode);
			}
		    else if (strlen (keystr) > 0) {
			InsertMacro (lastCode, flags, keystr);
			BitSet (keytable, lastCode);
		    }
		  case QuitBtn:
		    DisposDialog (macro2dlg);	/* finished with the dialog */
		    break;

		  case KY_HELP:
		    itemhit = Alert (ALERT_HELP, NILPROC);

		  default:
		    itemhit = 0;
		}
	    }
	}
    }
}				/* keymacros */



modrec modtable[NUMOFMODS];

/****************************************************************************/
/* handle the modifier dialog */
/****************************************************************************/
keymoddialog ()
{
    DialogPtr moddlg;
    short itemhit;
    modrec tmodtable[NUMOFMODS];/* temporary copy of modtable */
    short i;
    short ignType;
    Handle ignHdl;
    Rect box;
    char theStr[256];
    GrafPtr savePort;

    GetPort (&savePort);

    moddlg = GetNewDialog (MODBOXID, NILPTR, (WindowPtr) - 1);
    circleOK(moddlg);
    SetPort (moddlg);

    for (i = 0; i < NUMOFMODS; i++)
	tmodtable[i] = modtable[i];	/* make a temporary copy */

    /* draw the gray lines */
    for (i = MOD_LIN1; i <= MOD_LINL; i++) {
	GetDItem (moddlg, i, &ignType, &ignHdl, &box);
	FillRect (&box, qd.gray);
    }

    /* set the texts in the edit fields */
    for (i = 0; i < NUMOFMODS; i++) {
	/* PWP: these are saved as pascal strings now... */
	theStr[0] = 0;		/* be double extra safe */
	BlockMove (tmodtable[i].prefix, theStr, (tmodtable[i].prefix[0] + 1));
	DecodeString (theStr, (char) 0);
	SetIText (gethdl (i + MOD_PRF1, moddlg), theStr);
    }

    /* set the checkboxes according to the bits set */
    for (i = MOD_CHK1; i <= MOD_CHKL; i++)
	SetCtlValue (getctlhdl (i, moddlg),
		     (tmodtable[(i - MOD_CHK1) / 9].modbits &
		      (1 << ((i - MOD_CHK1) % 9))) ? btnOn : btnOff);

    /* loop till ok or cancel is pressed */
    for (;;) {
	ModalDialog (NILPROC, &itemhit);
	if (itemhit == OKBtn) {
	    for (i = 0; i < NUMOFMODS; i++) {
		GetIText (gethdl (i + MOD_PRF1, moddlg), theStr);
		EncodeString (theStr);
		if ((unsigned) (theStr[0]) > 19) /* Limit the length of
						  * the thing */
		    theStr[0] = 19;
		BlockMove (theStr, tmodtable[i].prefix, 20);
	    }

	    /* write the temporary copy back */
	    for (i = 0; i < NUMOFMODS; i++)
		modtable[i] = tmodtable[i];

	    UpdateOptKey(1);		/* make Option key processing right */
	}
	if ((itemhit == OKBtn) || (itemhit == QuitBtn)) {
	    DisposDialog (moddlg);	/* finished with the dialog */
	    SetPort (savePort);
	    return;
	}
	if (itemhit == MOD_HELP) {
	    itemhit = Alert (ALERT_MODHELP, NILPROC);
	    /* draw the gray lines again */
	    for (i = MOD_LIN1; i <= MOD_LINL; i++) {
		GetDItem (moddlg, i, &ignType, &ignHdl, &box);
		FillRect (&box, qd.gray);
	    }
	}
	if (itemhit <= MOD_CHKL) {	/* is it a check box ? */
	    tmodtable[(itemhit - MOD_CHK1) / 9].modbits
		^= (1 << ((itemhit - MOD_CHK1) % 9));
	    SetCtlValue (getctlhdl (itemhit, moddlg),
			 (tmodtable[(itemhit - MOD_CHK1) / 9].modbits &
			  (1 << ((itemhit - MOD_CHK1) % 9))) ? btnOn : btnOff);
	}
    }
}				/* keymoddialog */



/****************************************************************************/
/* load and unpack the key macro tables */
/****************************************************************************/
loadkset ()
{
    Handle ktab;
    char *k;
    int i;
    THz curZone;

    /* load the bit table */
    ktab = GetResource (KSET_TYPE, KSVER);

    if (ktab == (Handle) NIL)
	printerr ("Could not load the key macros (KSET) [old version?]", 0);

    if ((ktab == (Handle) NIL) || (GetHandleSize (ktab) == 0)) {
	/* init the keytable with zeroes if ktab is empty or not available */
	/* ktab is empty in the resource fork of the program itself */
	k = keytable;
	for (i = 1; i <= sizeof (keytable); i++)
	    *k++ = '\0';
	return;
    }
    HLock (ktab);
    BlockMove (*ktab, keytable, sizeof (keytable));
    HUnlock (ktab);
    curZone = GetZone();		/* as per John Norstad's (Disinfectant) */
    SetZone(HandleZone(ktab));	/* "Toolbox Gotchas" */
    ReleaseResource(ktab);
    SetZone(curZone);
}				/* loadkset */



/****************************************************************************/
/* load and unpack the key macro table */
/****************************************************************************/
loadmset ()
{
    short i;
    short idx;
    short num;
    short theCode;
    Handle mtab;
    char *src;
    char flags;
    THz curZone;

    DisposeMacros ();		/* release all macro strings */

    /* load the bit table */
    mtab = GetResource (MSET_TYPE, KMVER);

    if (mtab == (Handle) NIL) {
	printerr ("Could not load the key macros (MSET) [old version?]", 0);
	return;
    }
    HLock (mtab);
    HLock ((Handle) macroshdl);
    src = *mtab;

    /* load the modifier information */
    BlockMove (src, modtable, sizeof (modtable));
    src += sizeof (modtable);

    UpdateOptKey(1);		/* make Option key processing right */

    /* get the number of macro key definitions */
    BlockMove (src, &num, sizeof (num));
    src += sizeof (num);

    for (i = 0; i < num; i++) {
	/* Get the code */
	BlockMove (src, &theCode, sizeof (theCode));
	src += sizeof (theCode);

	/* Get the flags */
	flags = *src++;

	/* Get the string */
	/* p2cstr(src);  -- PWP: it allready is a pascal string! */
	idx = NewMacro ();	/* create a new free entry */
	SetMacro (idx, theCode, &flags, src);
	src += src[0] + 1;	/* PWP: was strlen */
    }

    HUnlock ((Handle) macroshdl);
    HUnlock (mtab);
    curZone = GetZone();		/* as per John Norstad's (Disinfectant) */
    SetZone(HandleZone(mtab));	/* "Toolbox Gotchas" */
    ReleaseResource(mtab);
    SetZone(curZone);
}				/* loadmset */



/****************************************************************************/
/* save the key macro bit table */
/****************************************************************************/
savekset ()
{
    Handle ktab;
    char *k;
    int i;

    ktab = NewHandle (sizeof (keytable));

    if (ktab == (Handle) NIL) {
	printerr ("Could not save the key macros (KSET)", 0);
	return;
    }
    HLock (ktab);
    BlockMove (keytable, *ktab, sizeof (keytable));
    HUnlock (ktab);
    AddResource (ktab, KSET_TYPE, KSVER, "");
}				/* savekset */



/****************************************************************************/
savemset ()
/****************************************************************************/
/* pack and save the key macro table */
{
    short i;
    short num;
    short leng;
    short count;
    short theCode;
    int totalLen;
    char *dest;
    Ptr src;
    Handle mtab;
    macrodefs *macros;

    HLock ((Handle) macroshdl);
    macros = *macroshdl;

    num = macros->numOfMacros;
    totalLen = 0;
    count = 0;

    /* calculate the sum of the string lengths of all active table entries */
    for (i = 0; i < num; i++)
	if (macros->mcr[i].code != -1) {
	    totalLen += macros->mcr[i].len;
	    count++;
	};

    /* add the length for keycode and length  */
    /* information and the number of entries  */
    totalLen += count * (sizeof (macrorec) - sizeof (long)) + MacroBaseSize;
    mtab = NewHandle (totalLen);
    if (mtab == (Handle) NIL) {
	printerr ("Could not save the key macros (MSET)", 0);
	return;
    }
    HLock (mtab);
    dest = *mtab;

    /* save the modifier information */
    BlockMove (modtable, dest, sizeof (modtable));
    dest += sizeof (modtable);

    /* save the number of key macros */
    BlockMove (&num, dest, sizeof (num));
    dest += sizeof (num);

    /* save the whole rest of the table */
    for (i = 0; i < num; i++) {
	theCode = macros->mcr[i].code;
	if (theCode != -1) {
	    /* save the code number */
	    BlockMove (&theCode, dest, sizeof (theCode));
	    dest += sizeof (theCode);

	    /* save the flags */
	    leng = macros->mcr[i].len;
	    *dest++ = macros->mcr[i].flags;

	    /* save the string length (1 byte!) */
	    /* PWP: nope! it's allready a Pascal string */
	    /* *dest++ = macros->mcr[i].len; */

	    /* save the macro string */
	    if (leng > 4)
		src = (Ptr) macros->mcr[i].macro;   /* the address is stored here */
	    else
		src = &macros->mcr[i].macro;
	    BlockMove (src, dest, leng);
	    dest += leng;
	}
    }

    HUnlock (mtab);
    HUnlock ((Handle) macroshdl);

    AddResource (mtab, MSET_TYPE, KMVER, "");
}				/* savemset */
