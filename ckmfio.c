/* Version 0.9(37) - Paul Placeway at Ohio State, Jan 1988 */
/*  reformatted all of the code so that it would be 79 or fewer colums */
/*  May 88 - Added dummy zgtdir() function, Frank, CU.
/* Version 0.8(35) - Jim Noble at Planning Research Corporation, June 1987. */
/* Ported to Megamax native Macintosh C compiler. */
/* Edit by Bill on Thu May 30, 00:18 */
/* Do error handling, neaten up comments, and some code. */
/* Edit by Bill on Wed May 15, 16:09 */
/* Make zrtol call common sfprtol, .RSRC overrides default settings */
/* ckmfio.c, Mon Apr 29 17:48, Edit by Bill*2 */
/* Put null in translated name to tie it off. */
/* Make author text of new file to ???? instead of random string */
/* Do flushvol after closing a file */
/* Bill C., Apr 24 */
/* Change zchin to allow sending of files with high order bits on */
/* Bill C., Apr 22 */
/* Add error handling (informing) for more cases, e.g. can't delete */
/* Bill C., Apr 22 */
/* Fix Resource/Data fork stuff.  Uppercase things where needed */
/* ckzmac.c, Thu Apr 21 17:19, Edit by Bill */
/*  Ignore trying to close an not-openend file, driver does it alot */
/* ckzmac.c, Thu Apr 11 21:18, Edit by Bill */
/*  Catch error in ZOPENO when trying to open an existing file */
/* ckzmac.c, Thu Apr 14 20:07, Edit by Bill */
/*  Translate calls with ZCTERM to go to the console routines */

/*
 * File ckmfio  --  Kermit file system support for the Macintosh
 *
 * Copyright (C) 1985, Trustees of Columbia University in the City of
 * New York.  Permission is granted to any individual or institution to
 * use, copy, or redistribute this software so long as it is not sold
 * for profit, provided this copyright notice is retained.
 *
 */

/* Definitions of some Unix system commands */

#define DIRCMDSTR "ls"
#define DELCMDSTR "rm"
#define SPCCMDSTR "sp"

char *DIRCMD = DIRCMDSTR;	/* For directory listing */
char *DELCMD = DELCMDSTR;	/* For file deletion */
char *SPACMD = SPCCMDSTR;	/* Space for all available volumes */
char *TYPCMD = "";		/* For typing a file */
char *SPACM2 = "";		/* For space in specified directory */
char *WHOCMD = "";		/* For seeing who's logged in */

/*
  Functions (n is one of the predefined file numbers from ckermi.h):

   zopeni(n,name)   -- Opens an existing file for input.
   zopeno(n,name)   -- Opens a new file for output.
   zclose(n)        -- Closes a file.
   zchin(n)         -- Gets the next character from an input file.
   zsout(n,s)       -- Write a null-terminated string to output file, buffered.
   zsoutl(n,s)      -- Like zsout, but appends a line terminator.
   zsoutx(n,s,x)    -- Write x characters to output file, unbuffered.
   zchout(n,c)      -- Add a character to an output file, unbuffered.
   zchki(name)      -- Check if named file exists and is readable, return size.
   zchko(name)      -- Check if named file can be created.
   znewn(name,s)    -- Make a new unique file name based on the given name.
   zdelet(name)     -- Delete the named file.
   zxpand(string)   -- Expands the given wildcard string into a list of files.
   znext(string)    -- Returns the next file from the list in "string".
   zxcmd(cmd)       -- Execute the command in a lower fork.
   zclosf()         -- Close input file associated with zxcmd()'s lower fork.
   zrtol(n1,n2)     -- Convert remote filename into local form.
   zltor(n1,n2)     -- Convert local filename into remote form.
 */

#include "ckcdeb.h"		/* Debug() and tlog() defs */
#include "ckcker.h"		/* Kermit definitions */

#define	__SEG__ ckmfio
#include <ctype.h>
#include <files.h>
#include <errors.h>
#include <devices.h>
#include <packages.h>
#include <osutils.h>
#include <toolutils.h>
#include <events.h>		/* needed for ckmasm.h */

#include "ckmdef.h"		/* Common Mac module definitions */
#include "ckmres.h"		/* Resource defs */
#include "ckmasm.h"		/* Assembler code */

/* These should all be settable by the File Settings Menu */

OSType authortext = 'MACA';	/* set creator to "MacWrite" */

#define FS_WIND 1		/* file is a text edit buffer */
#define FS_OPEN 2		/* file has been opened */
#define FS_RSRC 4		/* opened in resource fork */
#define FS_DATA 8
#define FS_PIPE 16		/* file is a memory buffer */

#define PIPESIZE 128
typedef struct {
    int charsleft;		/* nuber of unread characters in the buffer */
    ProcPtr refill;		/* pointer to the refill procedure */
    char *currptr;		/* characters left before next pipe refill */
    char pipebuf[PIPESIZE];	/* buffer to keep the pipes content */
}   MACPIPE;

typedef struct {
    short frefnum;		/* file reference number (pascal) */
    int fstatus;		/* file status bits */
    MACPIPE *fpipe;		/* pointer to a pipe */
}   MACFILE;

MACFILE fp[ZNFILS] = {		/* File information */
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
};



/****************************************************************************/
/*  Z O P E N I --  Open an existing file for input.
 *
 * The file name has been returned from and the volume reference
 * number set by SFGetFile.
 *
 * Returns:
 *  TRUE: file opened ok
 *  FALSE: some error.
 */
/****************************************************************************/
zopeni (n, name)
int n;
char *name;
{
    int err;
    register MACFILE *fpp;

    if (chkfn (n)) {
	printerr ("At zopeni file is already open ", n);
	return (FALSE);
    }
    fpp = &fp[n];

    if (n == ZCTERM) {		/* Terminal open? */
	if (chkfn (ZIFILE))	/* Check current ZOFILE */
	    printerr ("ZIFILE already open...: ", n);
	fp[ZIFILE].fstatus = FS_WIND;	/* redirect... here it is */
	fpp->fstatus = FS_WIND;	/* Indicate this is open too */
	return (conopen ());	/* Return from low level open */
    }
    if (n == ZSYSFN)		/* trying to open a pipe? */
	return (zxcmd (name));	/* yes... */

    if (n == ZIFILE &&		/* opening input file? */
	(filargs.filflg & FIL_RSRC))	/* and they said resource? */
	err = OpenRF (name, filargs.filvol, &fpp->frefnum);
    else			/* else some other channel or data */
	err = FSOpen (name, filargs.filvol, &fpp->frefnum);

    if (err != noErr)		/* check for open error */
	return (ioutil (err));	/* failed... */

    fpp->fstatus = FS_OPEN | (	/* set flags */
			   (filargs.filflg & FIL_RSRC) ? FS_RSRC : FS_DATA);

    GetEOF (fpp->frefnum, &filargs.filsiz);	/* set size for screen */
    return (TRUE);		/* Return success */
}				/* zopeni */



/****************************************************************************/
/*  Z O P E N O  --  Open a new file for output.
 *
 * Returns:
 *  TRUE: File opened ok
 *  FALSE: some error has occured or channel occupied.
 *
 */
/****************************************************************************/
zopeno (n, name)
int n;
char *name;
{
    OSType forktext;
    int err;
    FInfo finfo;
    register MACFILE *fpp;

    if (chkfn (n)) {
	printerr ("zopeno - file is already open: ", n);
	return (FALSE);
    }
    fpp = &fp[n];

    if (n == ZCTERM || n == ZSTDIO) {	/* Terminal open? */
	if (chkfn (ZOFILE))	/* Check current ZOFILE */
	    printerr ("ZOFILE already open...: ", n);
	fp[ZOFILE].fstatus = FS_WIND;	/* yes, redirect... here it is */
	fpp->fstatus = FS_WIND;	/* Indicate this is open too */
	return (conopen ());	/* Return from low level open */
    }
    if (n == ZOFILE && (filargs.filflg & FIL_RSRC))
	forktext = 'APPL';
    else
	forktext = 'TEXT';	/* Make fork reflect fork choice */

    err = Create (name, filargs.filvol, authortext, forktext);
    if (err == dupFNErr) {	/* duplicate file? */
	if (!ioutil (FSDelete (name,	/* Try to delete it */
			       filargs.filvol)))	/* checking for failure */
	    return (FALSE);	/* failed... */
	err = Create (name, filargs.filvol,	/* recreate */
		      authortext, forktext);
    }
    if (err != noErr)		/* some error? */
	return (ioutil (err));	/* yes, do message and return */

/* set file's folder from filargs.filfldr which is either the */
/* applications folder or the settings file folder */

    GetFInfo (name, filargs.filvol, &finfo);	/* read current finder info */
    finfo.fdFldr = filargs.filfldr;	/* set new folder */
    SetFInfo (name, filargs.filvol, &finfo);	/* and tell system about it */

    if (n == ZOFILE &&		/* is it our transferred file? */
	(filargs.filflg & FIL_RSRC))	/* want to use resource fork?  */
	err = OpenRF (name, filargs.filvol,	/* yes... */
		      &fpp->frefnum);
    else			/* else data, or some other file */
	err = FSOpen (name, filargs.filvol, &fpp->frefnum);

    if (err != noErr)		/* able to open? */
	return (ioutil (err));	/* no. fail return now */

    fp[n].fstatus = FS_OPEN |
	((filargs.filflg & FIL_RSRC) ? FS_RSRC : FS_DATA);

    return (TRUE);		/* done ok */
}				/* zopeno */



/****************************************************************************/
/*  Z C L O S E  --  Close the given file.
 *
 * Returns:
 *  TRUE: file closed ok.
 *  FLASE: some error has occured.
 *
 */
/****************************************************************************/
zclose (n)
int n;
{
    int err = noErr;
    register MACFILE *fpp;

    if (!chkfn (n))		/* is it opened? */
	return (FALSE);		/* no return now */

    fpp = &fp[n];

    if (fpp->fstatus == FS_WIND)/* is this a window? */
	fp[ZCTERM].fstatus = 0;	/* yes, clear ZCTERM */
    else if (fpp->fstatus == FS_PIPE)	/* is this a pipe? */
	fp[ZSYSFN].fstatus = 0;	/* yes, no pipe now, clear ZSYSFN */
    else {
	err = FSClose (fpp->frefnum);	/* else use OS close */
	if (err == noErr)	/* and if that worked */
	    /* i PWP: the above if should be ==, NOT != !!!!! */
	    err = FlushVol (NILPTR,	/* flush buffers in case write worked */
			    filargs.filvol);
    }

    fpp->fstatus = 0;		/* clear out status word */
    if (n == ZOFILE || n == ZIFILE)	/* turn off both flags */
	filargs.filflg &= ~(FIL_RSRC | FIL_DATA);

    return (ioutil (err));	/* return according to io operations */
}				/* zclose */



/****************************************************************************/
/*  Z C H I N  --  Get a character from the input file.
 *
 * Returns:
 *  0: Ok
 * -1: EOF (or other error).
 *
 */
/****************************************************************************/
zchin (n, c)
int n;
char *c;
{
    int err;
    long rdcnt;			/* pascal long */
    register MACFILE *fpp;
    register MACPIPE *pipe;

    if (!chkfn (n))
	return (0);

    fpp = &fp[n];

    if (fpp->fstatus == FS_WIND) {	/* a window? */
	printerr ("zchin called for FS_WIND file: ", n);
	return (0);
    }
    if (fpp->fstatus == FS_PIPE) {	/* a pipe? */
	pipe = fpp->fpipe;

	if (pipe->charsleft <= 0) {	/* test for characters left */
	    pipe->currptr = pipe->pipebuf;	/* restart at the beginning
						 * of the buffer */

	    if (pipe->refill != NILPROC) {	/* refill the pipe if
						 * possible */
		saveA0 ();	/* push content of A0 to stack */
		loadA0 (*(pipe->refill));	/* load the content of refill
						 * to A0 */
		execute ();	/* call the refill procedure */
		restoreA0 ();	/* get A0 back from the stack */
	    } else
		*(pipe->currptr) = '\0';	/* make it end otherwise */
	}
	if (*(pipe->currptr) == '\0')	/* is this eo-pipe? */
	    return (-1);	/* yes, fail return */

	*c = *(pipe->currptr)++;/* read character */
	(pipe->charsleft)--;	/* decrement characters left */

	return (0);		/* success */
    }
    rdcnt = 1;
    err = FSRead (fpp->frefnum, &rdcnt, c);
    if (err == eofErr)
	return (-1);		/* Failure return */
    return (ioutil (err) ? 0 : -1);	/* success or unknown failure */
}				/* zchin */



/****************************************************************************/
/*  Z S O U T  --  Write a string to the given file, buffered.
 *
 * Returns:
 *  0: OK
 * -1: Error
 *
 */
/****************************************************************************/
zsout (n, s)
int n;
char *s;
{
    long wrcnt;			/* pascal long */

    if (n == ZCTERM || fp[n].fstatus == FS_WIND)
	return (conol (s));

    wrcnt = (long) strlen (s);
    return (ioutil (FSWrite (fp[n].frefnum, &wrcnt, s)) ? 0 : -1);
}				/* zsout */



/****************************************************************************/
/*  Z S O U T L  --  Write string to file, with line terminator, buffered.
 *
 * Returns:
 *  0: OK
 * -1: Error
 *
 */
/****************************************************************************/
zsoutl (n, s)
int n;
char *s;
{
    long wrcnt;			/* pascal long */
    int err;

    if (n == ZCTERM || fp[n].fstatus == FS_WIND)
	return (conoll (s));

    wrcnt = (long) strlen (s);
    err = FSWrite (fp[n].frefnum, &wrcnt, s);
    if (err == noErr) {
	wrcnt = 2;
	err = FSWrite (fp[n].frefnum, &wrcnt, "\r\n");
    }
    return (ioutil (err) ? 0 : -1);
}				/* zsoutl */



/****************************************************************************/
/*  Z S O U T X  --  Write x characters to file, unbuffered.
 *
 * Returns:
 *  0: OK
 * -1: Error
 */
/****************************************************************************/
zsoutx (n, s, x)
int n, x;
char *s;
{
    long size;

    if (n == ZCTERM || fp[n].fstatus == FS_WIND)
	return (conxo (s, x));

    size = x;
    return (ioutil (FSWrite (fp[n].frefnum, &size, s)) ? 0 : -1);
}				/* zsoutx */



/****************************************************************************/
/*  Z C H O U T  --  Add a character to the given file. */
/*						*/
/* Returns:		*/
/*  0: OK			*/
/* -1: Error	*/
/****************************************************************************/
zchout (n, c)
int n;
char c;
{
    long wrcnt;			/* pascal long */
    int err;

    if (n == ZCTERM || fp[n].fstatus == FS_WIND) {
	conoc (c);		/* Then send to console routine */
	return (0);		/* Then send to console routine */
    }
    wrcnt = 1;
    err = FSWrite (fp[n].frefnum, &wrcnt, &c);
    if (err != noErr)		/* error occured? */
	sstate = 'a';		/* yes, abort protocol */
    return (ioutil (err) ? 0 : -1);	/* else return code */
}				/* zchout */



/****************************************************************************/
/*  C H K F N  --  Internal function to verify file number is ok.
 *
 * Returns:
 *   TRUE  - file is open
 *  FALSE  - file is not open
 *
 * Issues an error message if the file number is not in range.
 *
 */
/****************************************************************************/
chkfn (n)
int n;
{
    switch (n) {
      case ZCTERM:
      case ZSTDIO:
      case ZIFILE:
      case ZOFILE:
      case ZDFILE:
      case ZTFILE:
      case ZPFILE:
      case ZSYSFN:
      case ZSFILE:
	break;
      default:
	debug (F101, "chkfn: file number out of range", "", n);
	printerr ("chkfn - file number not in range: ", n);
	return (FALSE);		/* ugh */
    }
    return ((fp[n].fstatus != 0));	/* if open, fstatus is nonzero */
}				/* chkfn */



/****************************************************************************/
/*  Z C H K I  --  Check if input file exists and is readable.
 *
 * Returns:
 *  >= 0 if the file can be read (returns the size).
 *    -1 if file doesn't exist or can't be accessed,
 *    -2 if file exists but is not readable (e.g. a directory file).
 *    -3 if file exists but protected against read access.
 */
/****************************************************************************/
long
zchki (name)
char *name;
{
    long size;
    int err;
    FileParam info;

    if (strcmp (name, "stdin") == 0)	/* stdin is a pipe */
	return (PIPESIZE);	/* return size of buffer */

    c2pstr (name);		/* convert to a pascal string */
    info.ioFVersNum = 0;	/* No version number */
    info.ioFDirIndex = 0;	/* Use the file name */
    info.ioNamePtr = name;	/* Point to the file name */
    info.ioVRefNum = filargs.filvol;	/* Volume number */
    err = PBGetFInfo (&info, FALSE);	/* Get info on file */
    p2cstr (name);		/* put the name back */

    if (err == fnfErr)		/* file not found? */
	return (-1);		/* then that is what they want */

    if (err != noErr) {		/* any other error? */
	printerr ("zchki failed: ", err);	/* tell me about it */
	return (-1);
    }
    size = (filargs.filflg & FIL_RSRC) ?	/* if thinking about RSRC */
	info.ioFlRPyLen :	/* return that size, */
	info.ioFlPyLen;		/* else DATA */
    return (size);		/* did ok */
}				/* zchki */



/****************************************************************************/
/*  Z C H K O  --  Check if output file can be created.
 *
 * Returns
 *  0: Write OK
 * -1: write permission for the file should be denied.
 */
/****************************************************************************/
zchko (name)
char *name;
{
    char volname[100];
    VolumeParam info;

    info.ioVolIndex = 0;	/* Use the vol ref num only */
    info.ioNamePtr = volname;	/* Pointer to the volume name */
    info.ioVRefNum = filargs.filvol;	/* Volume reference number */
    if (!ioutil (PBGetVInfo (&info, 0)))	/* Get info on vol,
						 * synchronously */
	return (-1);		/* failed... */

    if ((info.ioVAtrb & 0x8000) != 0)	/* Write locked? */
	return (-1);		/* yes... */

    return (0);			/* else success */
}				/* zchko */



/****************************************************************************/
/*  Z D E L E T  --  Delete the named file and return TRUE if successful */
/****************************************************************************/
zdelet (name)
char *name;
{
    return (ioutil (FSDelete (name, filargs.filvol)));
}				/* zdelet */



/****************************************************************************/
/*  Z R T O L  --  Convert remote filename into local form.
 *
 * Check here to see if this should go into the resource fork (.rsrc)
 * or into the data fork (.data).
 *
 */
/****************************************************************************/
zrtol (name, name2)
char *name, *name2;
{

    strcpy (name2, name);	/* copy name to destination */

    if (filargs.filflg & (FIL_DODLG))	/* selected by user? */
	return;			/* won't be called but... */

    filargs.filflg &= ~(FIL_RBDT);	/* clear out flags */
    filargs.filflg |= sfprtol (name2);	/* convert name2 and set flags */
    binary = (filargs.filflg & FIL_BINA);	/* selected binary mode? */
    return;
}				/* zrtol */



/****************************************************************************/
/*  Z L T O R  --  Convert filename from local format to common form. */
/****************************************************************************/
zltor (name, name2)
char *name, *name2;
{
    int dc = 0;

    while (*name != '\0') {
	if (*name == ' ')
	    name++;		/* Skip spaces */
	else if ((*name == '.') && (++dc > 1)) {
	    *name2++ = 'X';	/* Just 1 dot */
	    name++;
	} else
	    *name2++ = (islower (*name)) ? toupper (*name++) : *name++;
    }
    *name2++ = '\0';		/* deposit final null */
    return;
}				/* zltor */



/****************************************************************************/
/*  Z C H D I R  --  Change directory or volumes */
/****************************************************************************/
zchdir (dirnam)
char *dirnam;
{
    int err;
    int volnum;
    WDPBRec vinfo;
    short *FSFCBLen = 0x3F6;

    if (*FSFCBLen < 0) {	/* if no HFS ROM's */
	err = SetVol (dirnam, 0);
	volnum = 0;
    } else {			/* use HFS calls */
	c2pstr (dirnam);
	vinfo.ioVRefNum = 0;	/* open a workimg directory */
	vinfo.ioWDDirID = 0;
	vinfo.ioWDProcID = 'ERIK';
	vinfo.ioNamePtr = dirnam;
	err = PBOpenWD (&vinfo, FALSE);
	p2cstr (dirnam);
	if (err != noErr)
	    return (FALSE);

	err = SetVol (NIL, vinfo.ioVRefNum);
	volnum = vinfo.ioVRefNum;
    }

    if (err == noErr) {		/* set default volume */
	screen (SCR_TN, 0, 0l, dirnam);
	filargs.filvol = volnum;/* make default */
    } else
	screen (SCR_TN, 0, 0l, "Can't set directory");

    return (err == noErr);	/* return ok or fail */
}				/* zchdir */

/****************************************************************************/
/*  Z G T D I R  --  Get name of current directory or volume */
/****************************************************************************/
char *
zgtdir ()
{
    return("(unknown)");
}

/****************************************************************************/
/* initialize the fields of a pipe */
/****************************************************************************/
zinitpipe (pipe, refillproc)
MACPIPE *pipe;
ProcPtr refillproc;
{
    pipe->refill = refillproc;
    pipe->currptr = pipe->pipebuf;
    pipe->charsleft = 0;
    *(pipe->currptr) = '\0';
}				/* zinitpipe */



/****************************************************************************/
/* fill the pipe; last is TRUE if it is the */
/* last time the pipe has to be filled  */
/****************************************************************************/
zfillpipe (pipe, str, last)
MACPIPE *pipe;
char *str;
Boolean last;
{
    int len;

    len = strlen (str);
    if (last)
	len++;

    if (len > PIPESIZE) {
	len = PIPESIZE;
	if (last)
	    str[PIPESIZE - 1] = '\0';	/* make sure we keep the eop
					 * character */
	printerr ("pipe overflow! characters may be lost");
    }
    memcpy (pipe->pipebuf, str, len);
    pipe->charsleft = len;
}				/* zfillpipe */



/****************************************************************************/
/* sprintf uses 12 kByte. This is the reason to use a simpler formatter here */
/* formatnum returns a right adjusted numberstring padded with fillc */
/* Numbers which do not fit into width are truncated on the left. */
/* Make sure str is at least 'width+1' bytes wide */
/****************************************************************************/
formatnum (num, fillc, width, str)
char *str;
long num;
char fillc;
int width;
{
    int i;
    char numstr[12];		/* -2147483647 is the longest string */
    /* that can be returned from NumToString */

    NumToString (num, numstr);
    i = strlen (numstr);

    while ((i >= 0) && (width >= 0))
	str[width--] = numstr[i--];

    while (width >= 0)
	str[width--] = fillc;
}				/* formatnum */



MACPIPE cmdpipe;

int volindex;
char spaceheader[60] = "\
Free      Name\n\
--------- --------------------\n";

/****************************************************************************/
/* loop through all available volumes and display the space left */
/****************************************************************************/
zlspace ()
{
    int err;
    Str255 name;
    long free;
    char outstr[60];
    VolumeParam vinfo;

    name.length = 0;
    vinfo.ioVolIndex = volindex;
    vinfo.ioNamePtr = &name;
    err = PBGetVInfo (&vinfo, FALSE);

    if (err == noErr) {
	free = vinfo.ioVFrBlk * vinfo.ioVAlBlkSiz;
	formatnum (free, ' ', 9, outstr);
	strcat (outstr, " ");
	p2cstr (&name);
	strcat (outstr, &name);
	strcat (outstr, "\n");
	zfillpipe (&cmdpipe, outstr, FALSE);
	volindex++;
    } else
	zfillpipe (&cmdpipe, "", TRUE);

}				/* zlspace */



int fileindex;
char dirheader[100] = "\
Size    Type Crea Last Modification Name\n\
------- ---- ---- ----------------- --------------------\n";

/****************************************************************************/
/* loop through all the files on the current volume / directory */
/****************************************************************************/
zldir ()
{
    int err;
    CInfoPBRec info;
    WDPBRec vinfo;

    Str255 name;
    DateTimeRec dtrec;

    char outstr[60];
    char type[10];
    char tmpstr[11];
    char datestr[11];
    char hourstr[3];
    char minutestr[3];

    unsigned long secs;
    long size;

    short *FSFCBLen = 0x3F6;

    if (*FSFCBLen < 0) {
	errpkt ("Sorry, the server uses 64 kByte ROM's");
	zfillpipe (&cmdpipe, "", TRUE);
	return;
    }
    PBHGetVol (&vinfo, FALSE);

    /* loop through all the files starting at the first one */
    name.length = 0;
    info.hfileInfo.ioFDirIndex = fileindex;	/* Get next file name */
    info.hfileInfo.ioNamePtr = &name;	/* Point to the empty file name */
    info.hfileInfo.ioVRefNum = vinfo.ioWDVRefNum;	/* Directory / Volume
							 * number */
    info.hfileInfo.ioDirID = vinfo.ioWDDirID;	/* Directory / Volume number */
    err = PBGetCatInfo (&info, FALSE);	/* Get info on file */

    if (err == noErr) {
	if (info.hfileInfo.ioFlAttrib & ioDirMask) {	/* a directory if it's
							 * true */
	    secs = info.dirInfo.ioDrMdDat;
	    strcpy (type, "#########");
	    strcpy (outstr, "       ");
	} else {		/* a file otherwise */
	    secs = info.hfileInfo.ioFlMdDat;
	    size = info.hfileInfo.ioFlLgLen + info.hfileInfo.ioFlRLgLen;
	    strcpy (type, "         ");
	    memcpy (type, &info.hfileInfo.ioFlFndrInfo.fdType, 4);
	    memcpy (type + 5, &info.hfileInfo.ioFlFndrInfo.fdCreator, 4);
	    formatnum (size, ' ', 7, outstr);
	}
	IUDateString (secs, shortDate, tmpstr);
	strcpy (datestr, "   ");
	datestr[10 - strlen (tmpstr)] = '\0';
	strcat (datestr, tmpstr);

	Secs2Date (secs, &dtrec);
	formatnum (dtrec.hour, ' ', 2, hourstr);
	formatnum (dtrec.minute, '0', 2, minutestr);

	p2cstr (&name);

	strcat (outstr, " ");
	strcat (outstr, type);
	strcat (outstr, " ");
	strcat (outstr, datestr);
	strcat (outstr, " ");
	strcat (outstr, hourstr);
	strcat (outstr, ":");
	strcat (outstr, minutestr);
	strcat (outstr, "  ");
	strcat (outstr, &name);
	strcat (outstr, "\n");

	zfillpipe (&cmdpipe, outstr, FALSE);
	fileindex++;
    } else
	zfillpipe (&cmdpipe, "", TRUE);

}				/* zldir */



#define CMD_RSRC	1
#define CMD_DATA	2
#define CMD_TEXT	3
#define CMD_BINA	4
#define CMD_DIR		5
#define CMD_DEL		6
#define CMD_SPC		7
#define CMD_UNK 255

char *cmdtab[] = {
    "fork rsrc",
    "fork data",
    "mode binary",
    "mode text",
    DIRCMDSTR,
    DELCMDSTR,
    SPCCMDSTR
};

int toktab[] = {
    CMD_RSRC,
    CMD_DATA,
    CMD_BINA,
    CMD_TEXT,
    CMD_DIR,
    CMD_DEL,
    CMD_SPC
};

#define NTOKS (sizeof (toktab)/sizeof(int))

/****************************************************************************/
/*  Z X C M D -- Run a system command so its output can be read like a file.
 *
 * Used on the MAC to implement MAC settings commands -- commands from a
 * remote system when in server mode that change internal variables.
 *
 */
/****************************************************************************/
int
zxcmd (comand)
char *comand;
{
    int sc;
    char theStr[120];
    int retCd;

    fp[ZIFILE].fstatus = FS_PIPE;	/* set input from pipe */
    fp[ZIFILE].fpipe = &cmdpipe;/* init pointer to command pipe */

    switch (sc = getcmd (comand)) {
      case CMD_RSRC:
      case CMD_DATA:
	zinitpipe (&cmdpipe, NILPROC);
	zfillpipe (&cmdpipe, "Default Fork set OK\n", TRUE);
	filargs.filflg &= ~(FIL_RSRC | FIL_DATA);	/* turn off  */
	filargs.filflg |= (sc == CMD_RSRC) ? FIL_RSRC : FIL_DATA;
	return (TRUE);		/* ok */

      case CMD_TEXT:
      case CMD_BINA:
	zinitpipe (&cmdpipe, NILPROC);
	zfillpipe (&cmdpipe, "Default Mode set OK\n", TRUE);
	filargs.filflg &= ~(FIL_TEXT | FIL_BINA);
	filargs.filflg |= (sc == CMD_BINA) ? FIL_BINA : FIL_TEXT;
	return (TRUE);		/* ok */

      case CMD_DIR:
	zinitpipe (&cmdpipe, zldir);
	zfillpipe (&cmdpipe, dirheader, FALSE);
	fileindex = 1;		/* start at the first file on */
	/* the current volume / directory */
	return (TRUE);		/* always ok */

      case CMD_DEL:
	strcpy (theStr, comand + strlen (DELCMDSTR));	/* the filename
							 * immediately  */
	retCd = zdelet (theStr);/* follows the command name */
	if (retCd) {
	    zinitpipe (&cmdpipe, NILPROC);
	    strcat (theStr, " deleted.");
	    zfillpipe (&cmdpipe, theStr, true);
	}
	return (retCd);

      case CMD_SPC:
	zinitpipe (&cmdpipe, zlspace);	/* init pipe for space listing */
	zfillpipe (&cmdpipe, spaceheader, FALSE);	/* copy the header to
							 * the pipe */
	volindex = 1;		/* start with the first volume */
	return (TRUE);		/* always ok */

      default:
	return (FALSE);		/* fail, unknown */
    }
}				/* zxcmd */



/****************************************************************************/
/****************************************************************************/
getcmd (cmd)
char *cmd;
{
    int k;

    for (k = 0; k < NTOKS; k++)
	if (strncmp (cmdtab[k], cmd, strlen (cmdtab[k])) == 0)
	    return (toktab[k]);	/* and return ID */
    return (CMD_UNK);		/* else unknown */

}				/* getcmd */



/****************************************************************************/
/*  Z C L O S F  - wait for the child fork to terminate and close the pipe. */
/****************************************************************************/
zclosf ()
{
    return;
}				/* zclosf */



int zindex;
int zfiles;
char *zname;

/****************************************************************************/
/*  Z X P A N D  --  Expand a wildcard string into an array of strings
 *
 * Returns the number of files that match fn, with data structures set up
 * so that first file (if any) will be returned by the next znext() call.
 */
/****************************************************************************/
zxpand (fn)
char *fn;
{
    int err;
    FileParam info;

    zfiles = 1;

    if ((filargs.filflg & FIL_ALLFL) ||	/* all files check box on or */
	(strcmp (fn, ":") == 0)) {	/* server 'get' with filname = ':' */

	/* the ioVNmFls field of the VolumeParam returns the number of files */

	/*
	 * !and! directories after PBGetInfo. This is why we have to count
	 * here.
	 */

	info.ioFVersNum = 0;	/* No version number */
	info.ioNamePtr = NIL;	/* Point to the file name */
	info.ioVRefNum = filargs.filvol;	/* Volume number */
	do {
	    info.ioFDirIndex = zfiles;	/* Get next file name */
	    err = PBGetFInfo (&info, FALSE);	/* Get info on file */
	    zfiles++;
	} while (err == noErr);
	zname = NIL;		/* no specific file */
	zfiles -= 2;		/* we counted 2 too high */

    } else
	zname = fn;		/* keep a pointer to that name */

    zindex = 0;			/* init the files sent counter */
    return (zfiles);
}				/* zxpand */



/****************************************************************************/
/*  Z N E X T  --  Get name of next file from list created by zxpand().
 *
 * Returns >0 if there's another file, with its name copied into the
 * arg string, or 0 if no more files in list.
 */
/****************************************************************************/
znext (fn)
char *fn;
{
    int err;
    Str255 name;
    FileParam info;

    zindex++;			/* next file */

    if (zindex > zfiles)
	return (0);		/* no more files */

    if (zname != NIL)
	strcpy (fn, zname);	/* Get the file's name */
    else {
	info.ioFVersNum = 0;	/* No version number */
	info.ioFDirIndex = zindex;	/* Get next file name */
	info.ioNamePtr = &name;	/* Point to the file name */
	info.ioVRefNum = filargs.filvol;	/* VolRefNum of the selected
						 * folder */
	err = PBGetFInfo (&info, FALSE);	/* Get info on file */
	if (err == noErr) {
	    p2cstr (&name);
	    strcpy (fn, &name);	/* Return the file's name */
	    *filargs.filrem = '\0';	/* reset remote name for folder
					 * transfer */
	} else {
	    printerr ("Error on reading next file name: ", err);
	    return (0);
	}
    }
    return (1);			/* fn contains the next file */
}				/* znext */



/****************************************************************************/
/*  Z N E W N  --  Make a new name for the given file  */
/****************************************************************************/
znewn (fn, s)
char *fn, **s;
{
    char *extp;
    int ver;

    strcpy (*s, fn);		/* copy in the name */
    if (strlen (*s) > 59)	/* don't allow long names */
	*s[59] = '\0';		/* it breaks the finder */
    extp = *s + strlen (*s);	/* find position of extension */
    *extp++ = '.';		/* add in the dot now */

    for (ver = 0; ver < 99; ver++) {	/* I'll try this many names */
	NumToString ((long) ver, extp);	/* add in the number */
	if (zchki (*s) == -1)	/* is this file known? */
	    return;		/* no, made a good one! */
    }
    fatal ("znewn failed to find unique name in 64 attempts", 0);
    return;
}				/* znewn */



/****************************************************************************/
pascal void
reset ()
/****************************************************************************/
extern 0x4E70;

/****************************************************************************/
/* zkself() - Kill self (reboot).  On other machines does a logout.
 *    	      Flush volumes and reboot.  Called by remote BYE.
 *
 */
/****************************************************************************/
zkself ()
{
    DrvQEl *drvqe;
    char vname[255];
    long vfreeb;
    short vrefnum;
    int err;

    /* handle on drive q */
    for (drvqe = (DrvQEl *) ((QHdr *) GetDrvQHdr ())->qHead;
	 drvqe != NULL;		/* while still something */
	 drvqe = drvqe->qLink) {/* step to next *//* for each drive */
	err = GetVInfo (drvqe->dQDrive, vname, &vrefnum, &vfreeb);
	if (err == noErr)
	    err = FlushVol (NILPTR, vrefnum);	/* flush the volume given
						 * refnum */
	else if (err != nsvErr)
	    screen (SCR_TN,0,0l,"Remote cmd: GetVinfo returned unknown code");
    }

    mac_cleanup ();		/* clean up before leaving */
    reset ();
}				/* zkself */



struct {
    int errnum;
    char *errstr;
}   ioerrs[] = {

    {
	dirFulErr, "Directory is full"
    },
    {
	dskFulErr, "Disk is full"
    },
    {
	wPrErr, "Diskette is write protected"
    },
    {
	fLckdErr, "File is software locked"
    },
    {
	vLckdErr, "Volume is software locked"
    },
    {
	fBsyErr, "File is busy"
    },
    {
	opWrErr, "File is already open with write permission"
    },
    {
	0, NILPTR
    }
};

/****************************************************************************/
/* ioutil - handle the result from an IO call, checking for an
 *    	    error return and displaying an appropriate error
 *    	    message.  Returns TRUE if no error occured, FALSE
 *    	    otherwise.
 */
/****************************************************************************/
int
ioutil (err)
int err;
{
    int e;

    if (err == noErr)
	return (TRUE);

    for (e = 0; ioerrs[e].errnum != 0 && ioerrs[e].errnum != err; e++);

    if (ioerrs[e].errstr == NILPTR)	/* anything there? */
	printerr ("Unknown IO error: ", err);
    else
	printerr (ioerrs[e].errstr, 0);

    return (FALSE);
}				/* ioutil */


#ifdef COMMENT
PWP: my copy of IM 2 says that this _is_ a standard toolbox call, so we
don't need this at all.

/****************************************************************************/
/*
 * OpenRF is not a standard Toolbox routine but acts identically to FSOpen
 * except that it opens the resource fork instead of the data fork.
 */
/****************************************************************************/
int 
OpenRF (fileName, vRefNum, refNum)
char *fileName;
int vRefNum;
short *refNum;
{
    IOParam pb;

    c2pstr (fileName);
    pb.ioNamePtr = fileName;
    pb.ioVRefNum = vRefNum;
    pb.ioVersNum = 0;
    pb.ioPermssn = 0;
    pb.ioMisc = (Ptr) 0;
    PBOpenRF (&pb, 0);
    *refNum = pb.ioRefNum;
    p2cstr (fileName);
    return pb.ioResult;
}				/* OpenRF */
#endif COMMENT

extern short dfltVol;

#ifdef TLOG
short tlogfile;
char tlogname[] = "Kermit Transaction";

/****************************************************************************/
/****************************************************************************/
opentlog ()
{
    return (openlogfile("Transaction log name:", tlogname, &tlogfile));
}

/****************************************************************************/
/****************************************************************************/
closetlog ()
{
    FSClose (tlogfile);
    FlushVol (NIL, dfltVol);
}				/* closetlog */



/****************************************************************************/
/****************************************************************************/
tlog (f, s1, s2, n)
int f;
long n;
char *s1;
char *s2;
{
    char numstr[12];
    char outstr[256];
    long count;

    if (tralog) {
	strcpy (outstr, s1);

	if (strlen (s2)) {
	    strcat (outstr, " ");
	    strcat (outstr, s2);
	}
	if (n) {
	    NumToString (n, numstr);
	    strcat (outstr, " ");
	    strcat (outstr, numstr);
	}
	strcat (outstr, "\r");

	count = strlen (outstr);
	FSWrite (tlogfile, &count, &outstr);
    }
}				/* tlog */

#endif



short slogfile;
char slogname[] = "Kermit Session";

/****************************************************************************/
/****************************************************************************/
openslog ()
{
    return (openlogfile("Session log name:", slogname, &slogfile));
}


/****************************************************************************/
/****************************************************************************/
openlogfile(prompt, name, fdp)
char *prompt, *name;
short *fdp;
{
    int err;
    FInfo info;
    SFReply sfr;	/* holds file info */
    Point where;
    
    SetPt(&where, 75, 80);
    SFPutFile (&where, prompt, name, NILPROC, &sfr);
    if (!sfr.good)	/* if canceled */
    	return (0);

    p2cstr(&sfr.fName);
    err = Create (&sfr.fName, sfr.vRefNum, 'MACA', 'TEXT');
    if (err != dupFNErr)
	if (!ioutil (err))
	    return (0);

    err = FSOpen (&sfr.fName, sfr.vRefNum, fdp); /* open the logfile */
    if (!ioutil (err))
	return (0);

    SetFPos (*fdp, fsFromLEOF, 0);	/* set file pointer to eof */
    return (1);
}				/* openslog */



/****************************************************************************/
/****************************************************************************/
closeslog ()
{
    int err;
    /* scrtolog (); */	/* dump the rest of the screen to the logfile */
    err = FSClose (slogfile);
    if (!ioutil (err))
	return (0);
    FlushVol (NIL, dfltVol);
}				/* closeslog */



/****************************************************************************/
/* write a maximum of n characters from s to the session log file */
/* skip all trailing blanks */
/****************************************************************************/
slog (s, n)
char *s;
int n;
{
    long count;
    char *c;

    /* skip all non visible characters */
    for (c = s + n - 1; (*c <= ' ') && (c >= s); c--);

    /* adjust count and write to file */
    count = (long) (c - s + 1);
    FSWrite (slogfile, &count, s);

    /* write a cr at end of line */
    count = 1;
    FSWrite (slogfile, &count, "\r");
}				/* slog */
