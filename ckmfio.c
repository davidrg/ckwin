/* Version 0.9(37) - Paul Placeway at Ohio State, Jan 1988 */
/*  reformatted all of the code so that it would be 79 or fewer colums */
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
   zinfill()	    -- (re) fill the file input buffer, and return the first character
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
   zchdir(dirnam)   -- Change working directory.
   zhome()          -- Return pointer to home directory name string.
   zkself()         -- Kill self, log out own job.
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

/* (PWP) external def. of things used in buffered file input and output */
extern CHAR zinbuffer[], zoutbuffer[];
extern CHAR *zinptr, *zoutptr;
extern int zincnt, zoutcnt;
static long zcnt_written;

/* These should all be settable by the File Settings Menu */

#define FS_WIND 1		/* file is a text edit buffer */
#define FS_OPEN 2		/* file has been opened */
#define FS_RSRC 4		/* opened in resource fork */
#define FS_DATA 8
#define FS_PIPE 16		/* file is a memory buffer */
#define FS_MACB 32		/* we are sending a file in MacBinary format */

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

static long iflen = -1;			/* Input file length */
static long oflen = -1;			/* Output file length */

char printfbuf[256];

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
    zincnt = 0;		/* (PWP) clear buffer input count */
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

    if (n == ZIFILE) {		/* opening input file? */
    	/* if we are doing MacBinary format */
	if ((filargs.filflg & (FIL_RSRC|FIL_DATA)) == (FIL_RSRC|FIL_DATA)) {
	    err = macbinopen(name, fpp);
	    if (err == noErr)
	    	return (TRUE);
	    else
	    	return (ioutil (err));
	} else {
	    if (filargs.filflg & FIL_RSRC)	/* and they said resource? */
		err = OpenRF (name, filargs.filvol, &fpp->frefnum);
	    else			/* else some other channel or data */
		err = FSOpen (name, filargs.filvol, &fpp->frefnum);
	    if (err != noErr)		/* check for open error */
		return (ioutil (err));	/* failed... */
	}
    }

    fpp->fstatus = FS_OPEN | (	/* set flags */
			   (filargs.filflg & FIL_RSRC) ? FS_RSRC : FS_DATA);

    GetEOF (fpp->frefnum, &filargs.filsiz);	/* set size for screen */
    fsize = filargs.filsiz;	/* PWP: set size for screen */
    return (TRUE);		/* Return success */
}				/* zopeni */

MBHead filHead;
static char MBname[64];

void
bzero (b, n)
register char *b;
register int n;
{
    while (n-- > 0)
	*b++ = 0;
}

void
bcopy (a, b, n)
register char *a, *b;
register int n;
{
    while (n-- > 0)
	*b++ = *a++;
}

/* these next three taken from NCSA Telnet 2.2 */
int
GetFileInfo(vol,name,iop)
short vol;
char *name;
FileParam *iop;
{
    char thename[64];
	
    strncpy (thename, name, 64);	/* make my own copy of this */
    thename[63] = '\0';
    c2pstr(thename);
	
    iop->ioNamePtr = thename;
    iop->ioVRefNum=vol;
    iop->ioFVersNum=iop->ioFDirIndex=0;
    PBGetFInfo(iop, FALSE);
    return (iop->ioResult);
}

int
SetFileInfo(vol,name,iop)
short vol;
char *name;
FileParam *iop;
{
    char thename[64];
	
    strncpy (thename, name, 64);	/* make my own copy of this */
    thename[63] = '\0';
    c2pstr(thename);
	
    iop->ioNamePtr = thename;
    iop->ioVRefNum=vol;
    iop->ioFVersNum=iop->ioFDirIndex=0;
    PBSetFInfo(iop, FALSE);
    return (iop->ioResult);
}

int
MakeTextFile(vol,name,iop)
short vol;
char *name;
FileParam *iop;
{
    GetFileInfo(vol,name,iop);
    iop->ioFlFndrInfo.fdType='TEXT';
    iop->ioFlFndrInfo.fdCreator='EDIT';
    SetFileInfo(vol,name,iop);
    return (iop->ioResult);
}

int
macbinopen(name, fpp)
char *name;
MACFILE *fpp;
{
    FileParam finfo;
    char *cp;
    int i, err;
    
	/* open first fork of name for reading and fill input buffer
	   with MacBinary header */
    /* save file name for later */
    strncpy (MBname, name, 64);
    MBname[64] = 0;
    
    /* clear out the header */
    bzero (&filHead, sizeof(MBHead));
    
    /* put the name into place */
    strncpy (&filHead.name[0], name, 64);
    filHead.name[63] = '\0';
    c2pstr(filHead.name);
    
    /* get the file info */
    if ((err = GetFileInfo( filargs.filvol, name, &finfo)) != noErr) {
	printerr("macbinopen: problem with GetFileInfo", err);
	return (err);
    }

    bcopy( &finfo.ioFlFndrInfo, &filHead.type[0], sizeof(FInfo) );
    filHead.protected = (filHead.zero2 & 0x40)?1:0;
    filHead.zero2 = 0;
    bcopy(&finfo.ioFlLgLen, &filHead.dflen[0], 4);
    bcopy(&finfo.ioFlRLgLen, &filHead.rflen[0], 4);
    bcopy(&finfo.ioFlCrDat, &filHead.cdate[0], 4);
    bcopy(&finfo.ioFlMdDat, &filHead.mdate[0], 4);

    filargs.filsiz=finfo.ioFlLgLen;
    filargs.rsrcsiz=finfo.ioFlRLgLen;
    
    fsize = ((filargs.filsiz + 127) & ~127) +
	    ((filargs.rsrcsiz + 127) & ~127) + 128;
    
    bcopy (&filHead, zinbuffer, 128);	/* put header in xfer buffer */
    zincnt = 128;			/* init buffer to 128 to send */
    zinptr = zinbuffer;	   /* set pointer to beginning, (== &zinbuffer[0]) */

    if (filargs.filsiz <= 0) {
	/* we do the data fork first, then the resource fork */
	if ((err = OpenRF (name, filargs.filvol, &fpp->frefnum)) != noErr) {
	     printerr("macbinopen: problem with OpenRF", err);
	     return (err);
	}
	fpp->fstatus = FS_OPEN | FS_RSRC | FS_MACB;
    } else {
	/* we do the data fork first, then the resource fork */
	if ((err = FSOpen (name, filargs.filvol, &fpp->frefnum)) != noErr) {
	     printerr("macbinopen: problem with FSOpen", err);
	     return (err);
	}
	fpp->fstatus = FS_OPEN | FS_DATA | FS_MACB;
    }
    return (noErr);
}

/* is this a MacBinary I header -- see standard-macbinary-ii.txt (on Sumex) */
is_macbinary(h)
MBHead h;
{
    register int i;
    long l;
    
    if ((h.zero1 != 0) || (h.zero2 != 0) || (h.zero3 != 0)) {
	printerr("zeros", 0);
    	return (0);
    }
    if ((h.name[0] == 0) || (h.name[0] & 0xc0)) {
	printerr("name length", h.name[0]);
    	return (0);
    }
    bcopy (&h.dflen[0], &l, 4);
    if ((l < 0) || (l > 0x7fffff)) {
	printerr("data length", l);
        return (0);
    }
    bcopy (&h.rflen[0], &l, 4);
    if ((l < 0) || (l > 0x7fffff))  {
	printerr("rsrc length", l);
        return (0);
    }
#ifdef COMMENT
    /* MacBinary II uses this area for more stuff, so don't check it */
    for (i = 2; i < 27; i++) {
    	if (h.filler[i] != 0) {
	    printerr("filler index:", i);
	    return (0);
	}
    }
#endif /* COMMENT */
    return (1);
}


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
    OSType forktext, authortext;
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
	zoutcnt = 0;		/* (PWP) reset output buffer */
	zoutptr = zoutbuffer;
	return (conopen ());	/* Return from low level open */
    }
    if (n == ZOFILE) {
	zoutcnt = 0;		/* (PWP) reset output buffer */
	zoutptr = zoutbuffer;
	if ((filargs.filflg & (FIL_RSRC|FIL_DATA)) == (FIL_RSRC|FIL_DATA)) {
	    /* if MacBinary */
	    forktext = 'rsrc';
	    authortext = 'RSED';    /* a ResEdit document (for the moment) */
	} else if (filargs.filflg & FIL_RSRC) {
	    forktext = 'APPL';
	    authortext = '????';
	} else {
	    forktext = 'TEXT';		/* Make fork reflect fork choice */
	    authortext = 'MACA';	/* set creator to "MacWrite" */
	}
    }

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

    if (n == ZOFILE) {		/* is it our transferred file? */
	zcnt_written = 0;
    	/* if we are doing MacBinary format */
	if ((filargs.filflg & (FIL_RSRC|FIL_DATA)) == (FIL_RSRC|FIL_DATA)) {
	    /* save file name for later */
	    strncpy (MBname, name, 64);
	    MBname[64] = 0;
	    /* we delay opening any forks until we have the MacBin header */
	    fp[n].fstatus = FS_OPEN | FS_MACB;	/* neither DATA nor RSRC yet */
	    return (TRUE);		/* done ok */
	} else { 	/* not MacBinary */
	    if (filargs.filflg & FIL_RSRC)	/* want to use resource fork?  */
		err = OpenRF (name, filargs.filvol,	/* yes... */
		      &fpp->frefnum);
	    else			/* else data, or some other file */
		err = FSOpen (name, filargs.filvol, &fpp->frefnum);
	}
    } else {
	err = FSOpen (name, filargs.filvol, &fpp->frefnum);
    }
    
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
    int err = noErr, e2 = 0;
    register MACFILE *fpp;
    FileParam finfo;
    Point old_location;

    if (!chkfn (n))		/* is it opened? */
	return (FALSE);		/* no return now */

    if ((n == ZOFILE) && (zoutcnt > 0))	/* (PWP) output leftovers */
	e2 = zoutdump();

    fpp = &fp[n];

    if (fpp->fstatus == FS_WIND)/* is this a window? */
	fp[ZCTERM].fstatus = 0;	/* yes, clear ZCTERM */
    else if (fpp->fstatus == FS_PIPE)	/* is this a pipe? */
	fp[ZSYSFN].fstatus = 0;	/* yes, no pipe now, clear ZSYSFN */
    else {
	if (fpp->fstatus & FS_MACB) {	/* if MacBinary format */
	    if (!cxseen && !czseen &&
	        (zcnt_written > ((filargs.rsrcsiz + 127) & ~127)) ||
		(zcnt_written < filargs.rsrcsiz)) {
		sprintf(printfbuf, "Resource fork size mismatch: should be %d is %d",
		    filargs.rsrcsiz, zcnt_written);
		printerr(printfbuf, 0);
	    }
	    SetEOF (fpp->frefnum, filargs.rsrcsiz);	/* trim padding off */
	}
	err = FSClose (fpp->frefnum);	/* else use OS close */
	if (err == noErr)	/* and if that worked */
	    /* PWP: the above if should be ==, NOT != !!!!! */
	    err = FlushVol (NILPTR,	/* flush buffers in case write worked */
			    filargs.filvol);
	if (err != noErr)
	    printerr("zclose(): problem closing/flushing:", err);
	    
	if (fpp->fstatus & FS_MACB) {
				/* if MacBinary and no error */
	    if ((err = GetFileInfo(filargs.filvol, MBname, &finfo)) != noErr) {
		sprintf(printfbuf, "Could not GetFileInfo on \"%s\": error %d",
		    MBname, err);
		printerr(printfbuf,0);
	    } else {
		old_location = finfo.ioFlFndrInfo.fdLocation;
	    
		filHead.protected &= 01;	/* nuke all but low order bit */
	    
		bcopy(&filHead.type[0], &finfo.ioFlFndrInfo, sizeof(FInfo));
		bcopy(&filHead.cdate[0], &finfo.ioFlCrDat, 4);
		bcopy(&filHead.mdate[0], &finfo.ioFlMdDat, 4);

		/* As per the MacBinary II doc, I clear the following flags:
		 *
		 *  0 - Set if file/folder is on the desktop (Finder 5.0 and later)
		 *  1 - bFOwnAppl (used internally)
		 *  8 - Inited (seen by Finder)
		 *  9 - Changed (used internally by Finder)
		 * 10 - Busy (copied from File System busy bit)
		 */
		finfo.ioFlFndrInfo.fdFlags &= 0xf8fc;

		/* finfo.ioFlFndrInfo.fdFldr = filargs.filfldr; */	/* set new folder */
		finfo.ioFlFndrInfo.fdFldr = 0;	/* set new folder */
		finfo.ioFlFndrInfo.fdLocation = old_location;	/* old_location */
	    
		if (finfo.ioFlLgLen != filargs.filsiz) {
		    sprintf(printfbuf, "%s: Data fork size mismatch: should be %d is %d",
		    	MBname, filargs.filsiz, finfo.ioFlLgLen);
		    printerr(printfbuf, 0);
		}
		if (finfo.ioFlRLgLen != filargs.rsrcsiz) {
		    sprintf(printfbuf, "%s: Resource fork size mismatch: should be %d is %d",
		    	MBname, filargs.rsrcsiz, finfo.ioFlRLgLen);
		    printerr(printfbuf, 0);
		}
		/* finfo.ioFlRLgLen=out->rlen; */
		/* finfo.ioFlLgLen =out->dlen; */
	
		if ((err = SetFileInfo(filargs.filvol, MBname, &finfo)) != noErr)
		    printerr("Could not SetFileInfo:", err);
		/* make sure the new data got to disk */
		p2cstr(filHead.name);
		if ((err = Rename(MBname, filargs.filvol, filHead.name)) != noErr)
		    printerr("Could not rename file:", err);
		err = FlushVol (NILPTR, filargs.filvol);
	    }
	}
    }

    fpp->fstatus = 0;		/* clear out status word */
    if (n == ZOFILE || n == ZIFILE)	/* turn off both flags */
	filargs.filflg &= ~(FIL_RSRC | FIL_DATA);

    iflen = -1;			/* Invalidate file length */

    if (e2 < 0) {
	(void) ioutil (err);
	return (FALSE);
    } else {
	return (ioutil (err));	/* return according to io operations */
    }
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

    if (n == ZIFILE)
	return (zminchar());	/* (PWP) go through the macro */

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

/*
 * (PWP) (re)fill the buffered input buffer with data.  All file
 * input should go through this routine, usually by calling the
 * zminchar() macro
 */

zinfill() {
    int err;
    long rdcnt;			/* pascal long */
    register MACFILE *fpp;
    char *cp;

    fpp = &fp[ZIFILE];

    /* if not an open file; just get one character */
    if (!(fpp->fstatus & FS_OPEN)) {
	zincnt = 0;
	if (zchin (ZIFILE, zinbuffer) < 0)
	    return (-1);
	return (zinbuffer[0]);
    }
    
    rdcnt = INBUFSIZE;
    err = FSRead (fpp->frefnum, &rdcnt, zinbuffer);
    zincnt = rdcnt;			/* set number actually read */

    /* check for any errors */
    if ((err != noErr) && (err != eofErr) && (!ioutil(err)))
	return (-1);

    /*
     * PWP: FSRead will return eofErr when it reads the last
     * partial block of data.  If rdcnt > 0, then we still have
     * the last bit of the file to send out...
     */
    if (err == eofErr) {
	if (rdcnt == 0) {	/* if actual EOF */
	    return (-1);		/* EOF return */
	} else {		/* last real block */
	    if (fpp->fstatus & FS_MACB) {	/* if MacBinary format */
		/* pad out things */
		rdcnt = (128 - (filargs.filsiz % 128)) % 128;
		for (cp = &zinbuffer[zincnt]; rdcnt > 0; rdcnt--)
		    *cp++ = 0;
			
		zincnt = (zincnt + 127) & ~127;	/* pad out to 128 bytes */
		    
		if (fpp->fstatus & FS_DATA) {	/* if we were doing the data fork */
		    fpp->fstatus &= ~FS_DATA;
		    
		    if (filargs.rsrcsiz != 0) {	/* if a resource fork */
			if ((err = FSClose (fpp->frefnum)) != noErr) {
			    printerr("zinfill: trouble closing data fork:", err);
			    return (-1);
			}

			/* open the resource fork 'cause we do that next */
			if ((err = OpenRF (MBname, filargs.filvol,
			    		       &fpp->frefnum)) != noErr) {
			    printerr("zinfill: trouble closing data fork:", err);
			    return (-1);
			}

			/* in case anyone else needs to know */
			fpp->fstatus |= FS_RSRC;
		    }
		}
	    }
	}
    }
    
    zinptr = zinbuffer;	   /* set pointer to beginning, (== &zinbuffer[0]) */
    zincnt--;			/* one less char in buffer */
    return((int)(*zinptr++) & 0377); /* because we return the first */
}


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
    
    if (n == ZOFILE)	/* (PWP) just in case */
	return (zmchout(c));
    
    wrcnt = 1;
    err = FSWrite (fp[n].frefnum, &wrcnt, &c);
    if (err != noErr)		/* error occured? */
	sstate = 'a';		/* yes, abort protocol */
    return (ioutil (err) ? 0 : -1);	/* else return code */
}				/* zchout */

/* (PWP) buffered character output routine to speed up file IO */
zoutdump()
{
    long wrcnt, tmpcnt;			/* pascal long */
    int err;
    FileParam finfo;
    char *outp;
    
    if ((wrcnt = zoutcnt) <= 0)
        return (0);

    if (fp[ZOFILE].fstatus == FS_WIND) {	/* if console output */
	conxo(zoutbuffer, zoutcnt);
	zoutcnt = 0;
	zoutptr = zoutbuffer;
	return(0);
    }

    outp = zoutbuffer;
    if (fp[ZOFILE].fstatus & FS_MACB) {	/* if MacBinary format */
	if (!(fp[ZOFILE].fstatus & (FS_RSRC | FS_DATA))) {  /* looking for header */
	    if (zoutcnt < 128)	/* we don't have all of the header yet */
		return (0);

	    bcopy(zoutbuffer, &filHead, 128);

	    if (!is_macbinary(filHead)) {
		screen(SCR_WM, 0, 0l, "Not a MacBinary file, reverting to binary, data fork");
		screen(SCR_AN,0,0l,MBname);	/* stop saying MacBinary mode */
		err = FSOpen (MBname, filargs.filvol, &fp[ZOFILE].frefnum);
		if (err != noErr) {
		    printerr("zoutdump(): trouble with FSOpen:", err);
		    (void) ioutil(err);
		    return (-1);
		}
		fp[ZOFILE].fstatus = FS_OPEN | FS_DATA;	/* unset FS_MACB */
		goto norm_file;
	    }

	    /* adjust pointers */
	    wrcnt = zoutcnt - 128;
	    outp = &zoutbuffer[128];
	    
	    /* get the sizes from header */
	    bcopy(&filHead.dflen[0], &filargs.filsiz, 4);
	    bcopy(&filHead.rflen[0], &filargs.rsrcsiz, 4);

	    if (filargs.filsiz > 0) {	/* is there a data fork?  */
		err = FSOpen (MBname, filargs.filvol, &fp[ZOFILE].frefnum);
		if (err != noErr) printerr("zoutdump(): trouble with FSOpen:", err);
		fp[ZOFILE].fstatus |= FS_DATA;
	    } else {			/* else data, or some other file */
		err = OpenRF (MBname, filargs.filvol,	/* yes... */
		      &fp[ZOFILE].frefnum);
		if (err != noErr) printerr("zoutdump(): trouble with OpenRF:", err);
		fp[ZOFILE].fstatus |= FS_RSRC;
	    }
	    
	    if (err != noErr) {
		sstate = 'a';		/* yes, abort protocol */
		return (ioutil (err) ? 0 : -1);
	    }
	    
	    zcnt_written = 0;

	    /* screen(SCR_AN,0,0l,MBname); */	/* Make screen say MacBinary mode */
	    
	    if (wrcnt <= 0) {	/* if nothing to write */
		zoutcnt = 0;
		zoutptr = zoutbuffer;
		return (0);	/* we are done this time */
	    }
	}
	
	/* can't be "else if" here, above if may feed this if */
	if (fp[ZOFILE].fstatus & FS_DATA) {	/* if doing data fork */
	    if (zcnt_written + wrcnt > filargs.filsiz) {
		tmpcnt = wrcnt;		/* save old amount of data */

		/* figure how how much we really should write */
		wrcnt = filargs.filsiz - zcnt_written;
		err = FSWrite (fp[ZOFILE].frefnum, &wrcnt, outp);
		zcnt_written += wrcnt;
		outp += wrcnt;
		
		wrcnt = tmpcnt - wrcnt;	/* adjust to reflect the write */
		
		/* close data fork, open rsrc fork */
		if (err == noErr)
		    err = FSClose (fp[ZOFILE].frefnum);
		if (err == noErr)	/* and if that worked */
		    err = FlushVol (NILPTR,	/* flush  OS buffers */
			    filargs.filvol);
		if (err == noErr)
		    err = OpenRF (MBname, filargs.filvol,	/* yes... */
				  &fp[ZOFILE].frefnum);
		if (err != noErr) {		/* error occured? */
		    zoutcnt = 0;
		    zoutptr = zoutbuffer;
		    sstate = 'a';		/* yes, abort protocol */
		    return (ioutil (err) ? 0 : -1);	/* else return code */
		}
		
		fp[ZOFILE].fstatus &= ~FS_DATA;
		fp[ZOFILE].fstatus |= FS_RSRC;
		
		/* skip padding */
		while (zcnt_written & 127) {
		    outp++;
		    wrcnt--;
		    zcnt_written++;
		}
		zcnt_written = 0;	/* restart counter for rsrc fork */

		if (wrcnt <= 0) {	/* if nothing to write */
		    zoutcnt = 0;
		    zoutptr = zoutbuffer;
		    return (0);		/* we are done this time */
		}
	    }
	}
    }
  norm_file:
    err = FSWrite (fp[ZOFILE].frefnum, &wrcnt, outp);
    zcnt_written += wrcnt;
    zoutcnt = 0;
    zoutptr = zoutbuffer;
    if (err != noErr) {		/* error occured? */
	sstate = 'a';		/* yes, abort protocol */
	return (ioutil (err) ? 0 : -1);	/* else return code */
    }
    return (0);		/* no problems */
}

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
    iflen = (filargs.filflg & FIL_RSRC) ?	/* if thinking about RSRC */
	info.ioFlRPyLen :	/* return that size, */
	info.ioFlPyLen;		/* else DATA */

    return (iflen);		/* did ok */
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
    short *FSFCBLen = (short *)0x3F6;

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
/*  Z H O M E  --  Return pointer to user's home directory  */
/****************************************************************************/

char *
zhome() {
    return("(directory unknown)");	/* PWP: for now */
}

/****************************************************************************/
/*  Z G T D I R  --  Return pointer to user's current directory  */
/****************************************************************************/

char *
zgtdir() {
    return ("(directory unknown)");	/* PWP: for now */
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

    short *FSFCBLen = (short *) 0x3F6;

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
    printerr ("znewn failed to find unique name in 99 attempts", 0);
    return;
}				/* znewn */

/*  Z S A T T R */
/*
 Fills in a Kermit file attribute structure for the file which is to be sent.
 Returns 0 on success with the structure filled in, or -1 on failure.
 If any string member is null, then it should be ignored.
 If any numeric member is -1, then it should be ignored.
*/
zsattr(xx) struct zattr *xx; {
    long k;

    k = iflen % 1024L;                  /* File length in K */
    if (k != 0L) k = 1L;
    xx->lengthk = (iflen / 1024L) + k;
    xx->type.len = 0;                   /* File type can't be filled in here */
    xx->type.val = "";
    xx->date.len = 0;                   /* File creation date */
    xx->date.val = "";
    xx->creator.len = 0;                /* File creator */
    xx->creator.val = "";
    xx->account.len = 0;                /* File account */
    xx->account.val = "";
    xx->area.len = 0;                   /* File area */
    xx->area.val = "";
    xx->passwd.len = 0;                 /* Area password */
    xx->passwd.val = "";
    xx->blksize = -1L;                  /* File blocksize */
    xx->access.len = 0;                 /* File access */
    xx->access.val = "";
    xx->encoding.len = 0;               /* Transfer syntax */
    xx->encoding.val = 0;
    xx->disp.len = 0;                   /* Disposition upon arrival */
    xx->disp.val = "";
    xx->lprotect.len = 0;               /* Local protection */
    xx->lprotect.val = "";
    xx->gprotect.len = 0;               /* Generic protection */
    xx->gprotect.val = "";
    xx->systemid.len = 2;               /* System ID */
    xx->systemid.val = "A3";		/* (A3: Apple Macintosh) */
    xx->recfm.len = 0;                  /* Record format */
    xx->recfm.val = "";
    xx->sysparam.len = 0;               /* System-dependent parameters */
    xx->sysparam.val = "";
    xx->length = iflen;                 /* Length */
    return(0);
}

/* Find initialization file. */

zkermini() {
/*  nothing yet...  this function added for benefit of VMS Kermit.  */
    return(0);
}

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

zmail(p,f) char *p; char *f; {		/* Send file f as mail to address p */
    screen (SCR_WM,0,0l,"There is no mail support in Mac Kermit.");
}

zprint(p,f) char *p; char *f; {		/* Print file f with options p */
    screen (SCR_WM,0,0l,"There is no printer support in Mac Kermit yet.");
}

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
	fnfErr, "File does not exist"
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
	FSWrite (tlogfile, &count, outstr);
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

char *
tilde_expand(dirname) char *dirname; {
    return("");
}
