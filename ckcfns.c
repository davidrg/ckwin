char *fnsv = "C-Kermit functions, 5A(120) 18 Sep 94";

/*  C K C F N S  --  System-independent Kermit protocol support functions.  */

/*  ...Part 1 (others moved to ckcfn2,3 to make this module smaller) */

/*
  Author: Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 1994, Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.
*/
/*
 System-dependent primitives defined in:

   ck?tio.c -- terminal i/o
   cx?fio.c -- file i/o, directory structure
*/
#include "ckcsym.h"			/* Needed for Stratus VOS */
#include "ckcasc.h"			/* ASCII symbols */
#include "ckcdeb.h"			/* Debug formats, typedefs, etc. */
#include "ckcker.h"			/* Symbol definitions for Kermit */
#include "ckcxla.h"			/* Character set symbols */

/* Externals from ckcmai.c */
extern int spsiz, spmax, rpsiz, timint, srvtim, rtimo, npad, ebq, ebqflg,
 rpt, rptq, rptflg, capas, keep, fncact, pkttim, autopar, spsizr, xitsta;
extern int pktnum, bctr, bctu, bctl, fmask, clfils, sbufnum,
 size, osize, spktl, nfils, warn, timef, spsizf, sndtyp, rcvtyp, success;
extern int parity, turn, network, what, whatru, fsecs,
 delay, displa, xflg, mypadn;
extern long filcnt, ffc, flci, flco, tlci, tlco, tfc, fsize, sendstart, rs_len;
extern long filrej, oldcps, cps ;
extern int fblksiz, frecl, frecfm, forg, fcctrl;
extern int spackets, rpackets, timeouts, retrans, crunched, wmax, wcur;
extern int hcflg, binary, savmod, fncnv, local, server, cxseen, czseen;
extern int nakstate, discard, rejection;
extern int rq, rqf, sq, wslots, wslotn, wslotr, winlo, urpsiz, rln;
extern int fnspath, fnrpath;
extern int atcapr, atcapb, atcapu;
extern int lpcapr, lpcapb, lpcapu;
extern int swcapr, swcapb, swcapu;
extern int lscapr, lscapb, lscapu;
extern int rscapr, rscapb, rscapu;
extern int bsave, bsavef, rptena, rptmin;
extern int sseqtbl[];
extern int numerrs;
extern long rptn;
extern int maxtry;
extern int stdouf;
extern int sendmode;
#ifdef OS2
extern struct zattr iattr ;
#endif /* OS2 */

#ifndef NOCSETS
extern int tcharset, fcharset;
extern int ntcsets;
extern struct csinfo tcsinfo[], fcsinfo[];
#endif /* NOCSETS */

extern int
  atenci, atenco, atdati, atdato, atleni, atleno, atblki, atblko,
  attypi, attypo, atsidi, atsido, atsysi, atsyso, atdisi, atdiso; 

extern int bigsbsiz, bigrbsiz;
extern char *versio;

#ifdef DYNAMIC
  extern CHAR *srvcmd;
#else
  extern CHAR srvcmd[];
#endif /* DYNAMIC */
extern CHAR padch, mypadc, eol, seol, ctlq, myctlq, sstate, myrptq;
extern CHAR *data, padbuf[], stchr, mystch;
extern CHAR *srvptr;
extern CHAR *rdatap;
extern char *cmarg, *cmarg2, *hlptxt, **cmlist, filnam[], fspec[];

_PROTOTYP( CHAR *rpar, (void) );
_PROTOTYP( int lslook, (unsigned int b) );	/* Locking Shift Lookahead */
_PROTOTYP( int szeof, (CHAR *s) );

/* International character sets */

#ifndef NOCSETS
/* Pointers to translation functions */
#ifdef CK_ANSIC
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* Character set */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* translation functions */
#else
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])();	/* Character set */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])();	/* translation functions. */
#endif /* CK_ANSIC */
_PROTOTYP( CHAR (*rx), (CHAR) );	/* Input translation function */
_PROTOTYP( CHAR (*sx), (CHAR) );	/* Output translation function */
_PROTOTYP( CHAR ident, (CHAR) );	/* Identity translation function */
#endif /* NOCSETS */

/* Windowing things */

extern int rseqtbl[];			/* Rec'd-packet sequence # table */

/* (PWP) external def. of things used in buffered file input and output */

#ifdef DYNAMIC
extern char *zinbuffer, *zoutbuffer;
#else
extern char zinbuffer[], zoutbuffer[];
#endif
extern char *zinptr, *zoutptr;
extern int zincnt, zoutcnt;

/*
  Variables defined in this module but shared by other modules.
*/

char * rf_err = "Error receiving file";	/* rcvfil() error message */

#ifdef CK_SPEED
short ctlp[256] = {		/* Control-Prefix table */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* C0  */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* G0  */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1, /* DEL */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* C1  */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* G1  */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1  /* 255 */
};
#endif /* CK_SPEED */

int sndsrc;		/* Flag for where to get names of files to send: */
					/* -1: znext() function */
					/*  0: stdin */
					/* >0: list in cmlist */

int  memstr;				/* Flag for input from memory string */

#ifdef pdp11
CHAR myinit[25];			/* Copy of my Send-Init data */
#else
CHAR myinit[100];			/* Copy of my Send-Init data */
#endif /* pdp11 */

/* Variables local to this module */

#ifdef TLOG
static char *fncnam[] = { 
  "rename", "replace", "backup", "append", "discard", "ask", "update", ""
};
#endif /* TLOG */

static char *memptr;			/* Pointer for memory strings */

#ifdef pdp11
static char cmdstr[50];			/* System command string. */
#else
static char cmdstr[256];
#endif /* pdp11 */

static int drain;			/* For draining stacked-up ACKs. */

static int first;			/* Flag for first char from input */
static CHAR t,				/* Current character */
    next;				/* Next character */

static int ebqsent = 0;			/* 8th-bit prefix bid that I sent */
static int lsstate = 0;			/* Locking shift state */
static int lsquote = 0;			/* Locking shift quote */

#ifdef datageneral
extern int quiet;
#endif

/*  E N C S T R  --  Encode a string from memory. */

/*
  Call this instead of getpkt() if source is a string, rather than a file.
  Note: Character set translation is never done in this case.
*/

#ifdef COMMENT
#define ENCBUFL 200
#ifndef pdp11
CHAR encbuf[ENCBUFL];
#else
/* This is gross, but the pdp11 root segment is out of space */
/* Will allocate it in ckuusr.c. */
extern CHAR encbuf[];
#endif /* pdp11 */
#endif /* COMMENT */

/*
  Encode packet data from a string in memory rather than from a file.
  Returns 0 on success, -1 if the string could not be completely encoded
  into the currently negotiated data field length.
*/
int
encstr(s) CHAR* s; {
#ifdef COMMENT
    int m; char *p;
    CHAR *dsave;

    if (!s) s = (CHAR *)"";
    if ((m = (int)strlen((char *)s)) > ENCBUFL) {
	debug(F111,"encstr string too long for buffer",s,ENCBUFL);
	s[ENCBUFL] = '\0';
    }
    if (m > spsiz-bctl-3) {
	debug(F111,"encstr string too long for packet",s,spsiz-bctl-3);
	s[spsiz-bctl-3] = '\0';
    }
    m = memstr; p = memptr;		/* Save these. */

    memptr = (char *)s;			/* Point to the string. */
    memstr = 1;				/* Flag memory string as source. */
    first = 1;				/* Initialize character lookahead. */
    dsave = data;			/* Boy is this ugly... */
    data = encbuf + 7;			/* Why + 7?  See spack()... */
    *data = NUL;			/* In case s is empty */
    getpkt(spsiz,0);		/* Fill a packet from the string. */
    data = dsave;			/* (sorry...) */
    memstr = m;				/* Restore memory string flag */
    memptr = p;				/* and pointer */
    first = 1;				/* Put this back as we found it. */
    return(0);
#else
/*
  Recoded 30 Jul 94 to use the regular data buffer and the negotiated
  maximum packet size.  Previously we were limited to the length of encbuf[].
  Also, to return a failure code if the entire encoded string would not fit.
*/
    int m, rc, slen; char *p;
    if (!s) s = (CHAR *)"";		/* Watch out for null pointers. */
    slen = strlen((char *)s);		/* Length of source string. */
    rc = 0;				/* Return code. */
    m = memstr; p = memptr;		/* Save these. */
    memptr = (char *)s;			/* Point to the string. */
    memstr = 1;				/* Flag memory string as source. */
    first = 1;				/* Initialize character lookahead. */
    *data = NUL;			/* In case s is empty */
    getpkt(spsiz,0);			/* Fill a packet from the string. */


    if (
#ifdef COMMENT
	*memptr
#else
	memptr < (char *)(s + slen)
#endif /* COMMENT */
	) {				/* This means we didn't encode */
	rc = -1;			/* the whole string. */
	debug(F101,"encstr string too big","",size);
    } else
      debug(F101,"encstr fits OK, size","",size);
    memstr = m;				/* Restore memory string flag */
    memptr = p;				/* and pointer */
    first = 1;				/* Put this back as we found it. */
    return(rc);
#endif /* COMMENT */
}

#ifdef COMMENT
/*
  We don't use this routine any more -- the code has been incorporated
  directly into getpkt() to reduce per-character function call overhead.
  Also, watch out: it hasn't been updated since it was commented out a
  long time ago.
*/
/* E N C O D E - Kermit packet encoding procedure */

VOID
encode(a) CHAR a; {			/* The current character */
    int a7;				/* Low order 7 bits of character */
    int b8;				/* 8th bit of character */
 
#ifndef NOCSETS
    if (!binary && sx) a = (*sx)(a);	/* Translate. */
#endif /* NOCSETS */

    if (rptflg)	{   	    		/* Repeat processing? */
        if (a == next && (first == 0)) { /* Got a run... */
	    if (++rpt < 94)		/* Below max, just count */
                return;
	    else if (rpt == 94) {	/* Reached max, must dump */
                data[size++] = rptq;
                data[size++] = tochar(rpt);
		rptn += rpt;		/* Count, for stats */
                rpt = 0;
	    }
        } else if (rpt == 1) {		/* Run broken, only 2? */
            rpt = 0;			/* Yes, reset repeat flag & count. */
	    encode(a);			/* Do the character twice. */
	    if (size <= maxsize) osize = size;
	    rpt = 0;
	    encode(a);
	    return;
	} else if (rpt > 1) {		/* More than two */
            data[size++] = rptq;	/* Insert the repeat prefix */
            data[size++] = tochar(++rpt); /* and count. */
	    rptn += rpt;
            rpt = 0;			/* Reset repeat counter. */
        }
    }
    a7 = a & 0177;			/* Isolate ASCII part */
    b8 = a & 0200;			/* and 8th (parity) bit. */

    if (ebqflg && b8) {			/* Do 8th bit prefix if necessary. */
        data[size++] = ebq;
        a = a7;
    }
    if ((a7 < SP) || (a7==DEL))	{	/* Do control prefix if necessary */
        data[size++] = myctlq;
	a = ctl(a);
    }
    if (a7 == myctlq)			/* Prefix the control prefix */
        data[size++] = myctlq;

    if ((rptflg) && (a7 == rptq))	/* If it's the repeat prefix, */
        data[size++] = myctlq;		/* quote it if doing repeat counts. */

    if ((ebqflg) && (a7 == ebq))	/* Prefix the 8th bit prefix */
        data[size++] = myctlq;		/* if doing 8th-bit prefixes */

    data[size++] = a;			/* Finally, insert the character */
    data[size] = '\0';			/* itself, and mark the end. */
}
#endif /* COMMENT */

/*  Output functions passed to 'decode':  */

int			       /*  Put character in server command buffer  */
#ifdef CK_ANSIC
putsrv(char c)
#else
putsrv(c) register char c;
#endif /* CK_ANSIC */
/* putsrv */ {
    *srvptr++ = c;
    *srvptr = '\0';		/* Make sure buffer is null-terminated */
    return(0);
}

int					/*  Output character to console.  */
#ifdef CK_ANSIC
puttrm(char c)
#else
puttrm(c) register char c;
#endif /* CK_ANSIC */
/* puttrm */ {
#ifndef NOSPL    
    extern char * qbufp;		/* If REMOTE QUERY active, */
    extern int query, qbufn;		/* also store response in */
    if (query && qbufn++ < 1024) {	/* query buffer. */
	*qbufp++ = c;
	*qbufp = NUL;
    } /* else */			/* else means don't display */
#endif /* NOSPL */
    conoc(c);
    return(0);
}

int					/*  Output char to file. */
#ifdef CK_ANSIC
putfil(char c)
#else
putfil(c) register char c;
#endif /* CK_ANSIC */
/* putfil */ {
    if (zchout(ZOFILE, (char) (c & fmask)) < 0) {
	czseen = 1;   			/* If write error... */
	debug(F101,"putfil zchout write error, setting czseen","",1);
	return(-1);
    }
    return(0);
}

/* D E C O D E  --  Kermit packet decoding procedure */

/*
 Call with string to be decoded and an output function.
 Returns 0 on success, -1 on failure (e.g. disk full).

 This is the "inner loop" when receiving files, and must be coded as 
 efficiently as possible.  Note some potential problems:  if a packet
 is badly formed, having a prefixed sequence ending prematurely, this
 function, as coded, could read past the end of the packet.  This has
 never happened, thus the additional (time-consuming) tests have not
 been added.
*/

static CHAR *xdbuf;	/* Global version of decode()'s buffer pointer */
                        /* for use by translation functions. */

/* Function for pushing a character onto decode()'s input stream. */

VOID
#ifdef CK_ANSIC
zdstuff(CHAR c)
#else
zdstuff(c) CHAR c;
#endif /* CK_ANSIC */
/* zdstuff */ {	
    xdbuf--;				/* Back up the pointer. */
    *xdbuf = c;				/* Stuff the character. */
}

int
#ifdef CK_ANSIC
decode(CHAR *buf, int (*fn)(char), int xlate)
#else
decode(buf,fn,xlate) register CHAR *buf; register int (*fn)(); int xlate;
#endif /* CK_ANSIC */
/* decode */ {
    register unsigned int a, a7, a8, b8; /* Various copies of current char */
    int t;				/* Int version of character */
    int ssflg;				/* Character was single-shifted */

/*
  Catch the case in which we are asked to decode into a file that is not open,
  for example, if the user interrupted the transfer, but the other Kermit
  keeps sending.
*/
    if ((cxseen || czseen || discard) && (fn == putfil))
      return(0);

    xdbuf = buf;			/* Make global copy of pointer. */
    rpt = 0;				/* Initialize repeat count. */

    while ((a = *xdbuf++ & 0xFF) != '\0') { /* Get next character. */
	if (a == rptq && rptflg) {	/* Got a repeat prefix? */
	    rpt = xunchar(*xdbuf++ & 0xFF); /* Yes, get the repeat count, */
	    rptn += rpt;
	    a = *xdbuf++ & 0xFF;	/* and get the prefixed character. */
	}
	b8 = lsstate ? 0200 : 0;	/* 8th-bit value from SHIFT-STATE */
	if (ebqflg && a == ebq) {	/* Have 8th-bit prefix? */
	    b8 ^= 0200;			/* Yes, invert the 8th bit's value, */
	    ssflg = 1;			/* remember we did this, */
	    a = *xdbuf++ & 0xFF;	/* and get the prefixed character. */
	} else ssflg = 0;

	if (a == ctlq) {		/* If control prefix, */
	    a  = *xdbuf++ & 0xFF;	/* get its operand */
	    a7 = a & 0x7F;		/* and its low 7 bits. */
	    if ((a7 >= 0100 && a7 <= 0137) || a7 == '?') { /* Controllify */
		a = ctl(a);		/* if in control range. */
		a7 = a & 0x7F;
	    }
	} else a7 = a & 0x7f;		/* Not a control character */

	if (a7 < 32 || a7 == 127) {	/* Control character? */
	    if (lscapu) {		/* If doing locking shifts... */
		if (lsstate)		/* If SHIFTED */
		  a8 = (a & ~b8) & 0xFF; /* Invert meaning of 8th bit */
		else			/* otherwise */
		  a8 = a | b8;		/* OR in 8th bit */
		/* If we're not in a quoted sequence */
		if (!lsquote && (!lsstate || !ssflg)) {
		    if (a8 == DLE) {	/* Check for DLE quote */
			lsquote = 1;	/* prefixed by single shift! */
			continue;
		    } else if (a8 == SO) { /* Check for Shift-Out */
			lsstate = 1;	/* SHIFT-STATE = SHIFTED */
			continue;
		    } else if (a8 == SI) { /* or Shift-In */
			lsstate = 0;	/* SHIFT-STATE = UNSHIFTED */
			continue;
		    }
		} else lsquote = 0;
	    }
	}
	a |= b8;			/* OR in the 8th bit */
	if (rpt == 0) rpt = 1;		/* If no repeats, then one */
	if (!binary) {			/* If in text mode, */
#ifdef NLCHAR
	    if (a == CR) continue;	/* Discard carriage returns, */
    	    if (a == LF) a = NLCHAR; 	/* convert LF to system's newline. */
#endif /* NLCHAR */

#ifndef NOCSETS				/* Character-set translation */
#ifdef KANJI				/* For Kanji transfers, */
	    if (tcharset != TC_JEUC)	/* postpone translation. */
#endif /* KANJI */
	      if (xlate && rx) a = (*rx)((CHAR) a); /* Translate charset */
#endif /* NOCSETS */
    	}
	if (fn == putfil) { /* (PWP) speedup via buffered output and a macro */
	    for (; rpt > 0; rpt--) {	/* Output the char RPT times */
#ifndef NOCSETS
#ifdef KANJI
		if (!binary && tcharset == TC_JEUC &&
		    fcharset != FC_JEUC) { /* Translating from J-EUC */
		    if (ffc == 0L) xkanjf();
		    if (xkanji(a,fn) < 0)  /* to something else? */
		      return(-1);
		    else t = 1;
		} else
#endif /* KANJI */
#endif /* NOCSETS */
#ifdef OS2
		  if (xflg) {		  /* For OS/2 only, use unbuffered */
		      char _a;
		      _a = a & fmask;
		      t = write(0,&_a,1); /* writes to console screen, to */
		      if (t < 1) t = -1;  /* reduce jerkiness. */
		  } else
#endif /* OS2 */
		    t = zmchout(a & fmask); /* zmchout is a macro */
		if (t < 0)
		  return(-1);
		ffc++;			/* Count the character */
	    }
	} else {			/* Output to something else. */
	    a &= fmask;			/* Apply file mask */
	    for (; rpt > 0; rpt--) {	/* Output the char RPT times */
		if ((*fn)((char) a) < 0) return(-1); /* Send to output func. */
#ifdef COMMENT
/*
  This was causing the server to count extra bytes that were part of
  server command packets, like FINISH.
*/
		ffc++;
#endif /* COMMENT */
	    }
	}
    }
    return(0);
}

/*  G E T P K T -- Fill a packet data field  */

/*
 Gets characters from the current source -- file or memory string.
 Encodes the data into the packet, filling the packet optimally.
 Set first = 1 when calling for the first time on a given input stream
 (string or file).

 Call with:
 bufmax -- current send-packet size
 xlate  -- flag: 0 to skip character-set translation, 1 to translate

 Uses global variables:
 t     -- current character.
 first -- flag: 1 to start up, 0 for input in progress, -1 for EOF.
 next  -- next character.
 data  -- pointer to the packet data buffer.
 size  -- number of characters in the data buffer.
 memstr - flag that input is coming from a memory string instead of a file.
 memptr - pointer to string in memory.
 (*sx)()  character set translation function

Returns the size as value of the function, and also sets global "size",
and fills (and null-terminates) the global data array.  Returns 0 upon eof.

Rewritten by Paul W. Placeway (PWP) of Ohio State University, March 1989.
Incorporates old getchx() and encode() inline to reduce function calls,
uses buffered input for much-improved efficiency, and clears up some
confusion with line termination (CRLF vs LF vs CR).

Rewritten again by Frank da Cruz to incorporate locking shift mechanism,
May 1991.
*/

/*
  Lookahead function to decide whether locking shift is worth it.  Looks at
  the next four input characters to see if all of their 8th bits match the
  argument.  Call with 0 or 0200.  Returns 1 if so, 0 if not.  If we don't
  happen to have at least 4 more characters waiting in the input buffer,
  returns 1.  Note that zinptr points two characters ahead of the current
  character because of repeat-count lookahead.
*/

#ifdef KANJI
int
kgetf() {
    return(zminchar());
}

int
kgetm() {
    int x;
    if (x = *memptr++) return(x);
    else return(-1);
}
#endif /* KANJI */

int
lslook(b) unsigned int b; {		/* Locking Shift Lookahead */
    int i;
    if (zincnt < 3)			/* If not enough chars in buffer, */
      return(1);			/* force shift-state switch. */
    b &= 0200;				/* Force argument to proper form. */
    for (i = -1; i < 3; i++)		/* Look at next 5 characters to */
      if (((*(zinptr+i)) & 0200) != b)	/* see if all their 8th bits match.  */
	return(0);			/* They don't. */
    return(1);				/* They do. */
}

int
getpkt(bufmax,xlate) int bufmax, xlate; { /* Fill one packet buffer */
    register CHAR rt = t, rnext;	  /* Register shadows of the globals */
    register CHAR *dp, *odp, *odp2, *p1, *p2; /* pointers... */
    register int x;			/* Loop index. */
    register int a7;			/* Low 7 bits of character */
    static CHAR leftover[9] = { '\0','\0','\0','\0','\0','\0','\0','\0','\0' };
    CHAR xxls, xxdl, xxrc, xxss, xxcq;	/* Pieces of prefixed sequence */
    int n;				/* worker */

/*
  Assume bufmax is the receiver's total receive-packet buffer length.
  Our whole packet has to fit into it, so we adjust the data field length.
  We also decide optimally whether it is better to use a short-format or
  long-format packet when we're near the borderline.
*/
    n = bufmax - 5;			/* Space for Data and Checksum */
    if (n > 92 && n < 96) n = 92;	/* "Short" Long packets don't pay */
    if (n > 92 && lpcapu == 0)		/* If long packets needed, */
      n = 92;				/* make sure they've been negotiated */
    bufmax = n - bctl;			/* Space for data */
    if (n > 92) bufmax -= 3;		/* Long packet needs header chksum */

    if (first == 1) {		/* If first character of this file... */
	if (!memstr) ffc = 0L;	/* Reset file character counter */
	first = 0;		/* Next character won't be first */
	*leftover = '\0';	/* Discard any interrupted leftovers, */

	/* get first character of file into rt, watching out for null file */

#ifndef NOCSETS
#ifdef KANJI
	if (!binary && tcharset == TC_JEUC && xlate) {
	    x = zkanjf();
	    if ((x = zkanji( memstr ? kgetm : kgetf )) < 0) {
	        first = -1;
	        size = 0;
	        if (x == -2) {
	            debug(F100,"getpkt(zkanji): input error","",0);
	            cxseen = 1;
	        } else debug(F100,"getpkt(zkanji): empty string/file","",0);
	        return (0);
	    }
	    if (!memstr) ffc++;
	    rt = x;
	} else {
#endif /* KANJI */
#endif /* not NOCSETS */
	if (memstr) {			/* Reading data from memory string */
	    if ((rt = *memptr++) == '\0') { /* end of string ==> EOF */
		first = -1;
	        size = 0;
		debug(F100,"getpkt: empty string","",0);
		return (0);
	    }

	} else {			/* Reading data from a file */

	    if ((x = zminchar()) < 0) { /* End of file or input error */
		first = -1;
	        size = 0;
		if (x == -2) {		/* Error */
		    debug(F100,"getpkt: input error","",0);
		    cxseen = 1;		/* Interrupt the file transfer */
		} else debug(F100,"getpkt: empty file","",0); /* Empty file */
		return(0);
	    }
	    ffc++;			/* Count a file character */
	    rt = x;			/* Convert int to char */
	    debug(F101,"getpkt zminchar","",rt);
	}
#ifndef NOCSETS
#ifdef KANJI
	}
#endif /* KANJI */
#endif /* not NOCSETS */

	rt &= fmask;			/* Apply SET FILE BYTESIZE mask */
	debug(F101,"getpkt fmask","",fmask);
	debug(F101,"getpkt new rt","",rt);

#ifndef NOCSETS
	if (xlate) {
	    debug(F101,"getpkt about to call translate function","",rt);
	    debug(F101,"tcharset","",tcharset);
	    debug(F101,"fcharset","",fcharset);
	}
#ifdef KANJI
	if (tcharset != TC_JEUC)
#endif /* KANJI */
	  if (!binary && sx && xlate) {
	      rt = (*sx)(rt); /* Translate */
	      debug(F101," translate function returns","",rt);
	  }
#endif /* not NOCSETS */

	/* PWP: handling of NLCHAR is done later (in the while loop)... */

    } else if ((first == -1) && (*leftover == '\0')) /* EOF from last time? */
        return(size = 0);
/*
  Here we handle characters that were encoded for the last packet but
  did not fit, and so were saved in the "leftover" array.
*/
    dp = data;				/* Point to packet data buffer */
    for (p1 = leftover; (*dp = *p1) != '\0'; p1++, dp++) /* Copy leftovers */
    	;
    *leftover = '\0';			/* Delete leftovers */
    if (first == -1)			/* Handle EOF */
      return(size = (dp - data));
  
/* Now fill up the rest of the packet. */

    rpt = 0;				/* Initialize character repeat count */

    while (first > -1) {		/* Until EOF... */
#ifndef NOCSETS
#ifdef KANJI
	if (!binary && xlate && tcharset == TC_JEUC) {
	    if ((x = zkanji( memstr ? kgetm : kgetf )) < 0) {
	        first = -1;
	        if (x == -2) cxseen = 1;
	    } else if (!memstr) ffc++;
	    rnext = x & fmask;
	} else {
#endif /* KANJI */
#endif /* not NOCSETS */
	if (memstr) {			/* Get next char from memory string */
	    if ((x = *memptr++) == '\0') /* End of string means EOF */
	      first = -1;		/* Flag EOF for next time. */
	    rnext = x & fmask;		/* Apply file mask */
	} else {
	    if ((x = zminchar()) < 0) { /* Real file, check for EOF */
		first = -1;		/* Flag eof for next time. */
		if (x == -2) cxseen = 1; /* If error, cancel this file. */
	    } else ffc++;		/* Count the character */
	    rnext = x & fmask;		/* Apply file mask */
	} 
#ifndef NOCSETS
#ifdef KANJI
	}
#endif /* KANJI */
#endif /* not NOCSETS */

#ifndef NOCSETS
#ifdef KANJI
	if (tcharset != TC_JEUC)
#endif /* KANJI */
	    if (!binary && sx && xlate) {
		rnext = (*sx)(rnext); /* Translate */
		debug(F101,"getpkt xlated rnext to","",rnext);
	    }
#endif /* not NOCSETS */

	odp = dp;			/* Remember where we started. */
	xxls = xxdl = xxrc = xxss = xxcq = NUL;	/* Clear these. */

/*
  Now encode the character according to the options that are in effect:
    ctlp[]: whether this control character needs prefixing.
    binary: text or binary mode.
    rptflg: repeat counts enabled.
    ebqflg: 8th-bit prefixing enabled.
    lscapu: locking shifts enabled.
*/
	if (rptflg) {			/* Repeat processing is on? */
	    if (
#ifdef NLCHAR
		/*
		 * If the next char is really CRLF, then we cannot
		 * be doing a repeat (unless CR,CR,LF which becomes
		 * "~ <n-1> CR CR LF", which is OK but not most efficient).
		 * I just plain don't worry about this case.  The actual
		 * conversion from NL to CRLF is done after the rptflg if...
		 */
	    (binary || (rnext != NLCHAR)) &&
#endif /* NLCHAR */
	    (rt == rnext) && (first == 0)) { /* Got a run... */
		if (++rpt < 94) {	/* Below max, just count */
		    continue;		/* go back and get another */
		}
		else if (rpt == 94) {	/* Reached max, must dump */
		    xxrc = tochar(rpt);	/* Put the repeat count here */
		    rptn += rpt;	/* Accumulate it for statistics */
		    rpt = 0;		/* And reset it */
		}
	    } else if (rpt > 1) {	/* More than two */
		xxrc = tochar(++rpt);	/* and count. */
		rptn += rpt;
		rpt = 0;		/* Reset repeat counter. */
	    }
	    /*
	      If (rpt == 1) we must encode exactly two characters.
	      This is done later, after the first character is encoded.
	    */
	}

#ifdef NLCHAR
	if (!binary && (rt == NLCHAR)) { /* It's the newline character */
	    if (lscapu && lsstate) {	/* If SHIFT-STATE is SHIFTED */
		if (ebqflg) {		/* If single shifts enabled, */
		    *dp++ = ebq;	/* insert a single shift. */
		} else {		/* Otherwise must shift in. */
		    *dp++ = myctlq;	/* Insert shift-out code */
		    *dp++ = 'O';
		    lsstate = 0;	/* Change shift state */
		}
	    }
#ifdef CK_SPEED
	    if (ctlp[CR]) {
		*dp++ = myctlq;		/* Insert carriage return directly */
		*dp++ = 'M';
	    } else *dp++ = CR;		/* Perhaps literally */
#else /* !CK_SPEED */
	    *dp++ = myctlq;		/* Insert carriage return directly */
	    *dp++ = 'M';
#endif /* CK_SPEED */
	    rt = LF;			/* Now make next char be linefeed. */
	}
#endif /* NLCHAR */

/*
  Now handle the 8th bit of the file character.  If we have an 8-bit
  connection, we preserve the 8th bit.  If we have a 7-bit connection,
  we employ either single or locking shifts (if they are enabled).
*/
	a7 = rt & 0177;			/* Get low 7 bits of character */
	if (rt & 0200) {		/* 8-bit character? */
	    if (lscapu) {		/* Locking shifts enabled? */
		if (!lsstate) {		/* Not currently shifted? */
		    x = lslook(0200);	/* Look ahead */
		    if (x != 0 || ebqflg == 0) { /* Locking shift decision */
			xxls = 'N';	   /* Need locking shift-out */
			lsstate = 1;	   /* and change to shifted state */
		    } else if (ebqflg) {   /* Not worth it */
			xxss = ebq;	   /* Use single shift */
		    }
		}
		rt = a7;		/* Replace character by 7-bit value */
	    } else if (ebqflg) {	/* 8th bit prefixing is on? */
		xxss = ebq;		/* Insert single shift */
		rt = a7;		/* Replace character by 7-bit value */
	    }
/*
  In case we have a 7-bit connection and this is an 8-bit character, AND
  neither locking shifts nor single shifts are enabled, then the character's
  8th bit will be destroyed in transmission, and a block check error will
  occur.
*/
	} else if (lscapu) {		/* 7-bit character */

	    if (lsstate) {		/* Comes while shifted out? */
		x = lslook(0);		/* Yes, look ahead */
		if (x || ebqflg == 0) {	/* Time to shift in. */
		    xxls = 'O';		/* Set shift-in code */
		    lsstate = 0;	/* Exit shifted state */
		} else if (ebqflg) {	/* Not worth it, stay shifted out */
		    xxss = ebq;		/* Insert single shift */
		}
	    }
	}
	/* If data character is significant to locking shift protocol... */
	if (lscapu && (a7 == SO || a7 == SI || a7 == DLE))
	  xxdl = 'P';			/* Insert datalink escape */

	if (
#ifdef CK_SPEED
	    ctlp[rt]
#else
	    (a7 < SP) || (a7 == DEL)
#endif /* CK_SPEED */
	    ) {				/* Do control prefixing if necessary */
	    xxcq = myctlq;		/* The prefix */
	    rt = ctl(rt);		/* Uncontrollify the character */
	}
	if (a7 == myctlq)		/* Always prefix the control prefix */
	  xxcq = myctlq;

	if ((rptflg) && (a7 == rptq))	/* If it's the repeat prefix, */
	  xxcq = myctlq;		/* prefix it if doing repeat counts */

	if ((ebqflg) && (a7 == ebq))	/* Prefix the 8th-bit prefix */
	  xxcq = myctlq;		/* if doing 8th-bit prefixes */

/* Now construct the entire sequence */

	if (xxls) { *dp++ = myctlq; *dp++ = xxls; } /* Locking shift */
	odp2 = dp;				    /* (Save this place) */
	if (xxdl) { *dp++ = myctlq; *dp++ = xxdl; } /* Datalink escape */
	if (xxrc) { *dp++ =   rptq; *dp++ = xxrc; } /* Repeat count */
	if (xxss) { *dp++ = ebq; }		    /* Single shift */
	if (xxcq) { *dp++ = myctlq; }	    	    /* Control prefix */
	*dp++ = rt;			/* Finally, the character itself */

	if (rpt == 1) {			/* Exactly two copies? */
	    rpt = 0;
	    p2 = dp;			/* Save place temporarily */
	    for (p1 = odp2; p1 < p2; p1++) /* Copy the old chars over again */
	      *dp++ = *p1;
	    if ((p2-data) <= bufmax) odp = p2; /* Check packet bounds */
	}
	rt = rnext;			/* Next character is now current. */

/* Done encoding the character.  Now take care of packet buffer overflow. */

	if ((dp-data) >= bufmax) {	/* If too big, save some for next. */
	    size = (dp-data);		/* Calculate the size. */
	    *dp = '\0';			/* Mark the end. */
	    if ((dp-data) > bufmax) {	/* if packet is overfull */
		/* copy the part that doesn't fit into the leftover buffer, */
		/* taking care not to split a prefixed sequence. */
		for (p1 = leftover, p2=odp; (*p1 = *p2) != '\0'; p1++,p2++)
		    ;
		debug(F111,"getpkt leftover",leftover,size);
		debug(F101," osize","",(odp-data));
		size = (odp-data);	/* Return truncated packet. */
		*odp = '\0';		/* Mark the new end */
	    }
	    t = rt; next = rnext;	/* save for next time */
	    return(size);
	}
    }					/* Otherwise, keep filling. */

    size = (dp-data);			/* End of file */
    *dp = '\0';				/* Mark the end of the data. */
    debug(F111,"getpkt eof/eot",data,size); /* Fell thru before packet full, */
    return(size);		     /* return partially filled last packet. */
}

/*  T I N I T  --  Initialize a transaction  */

int
tinit() {
    int x;

#ifndef NOCSETS
    if (tcharset == TC_TRANSP) {	/* Character set translation */
	rx = sx = NULL;			/* Transparent, no translation */
#ifdef KANJI
    } else if (tcharset == TC_JEUC) {
	rx = sx = NULL;			/* Transparent, no translation */      
#endif /* KANJI */
    } else {				/* otherwise */
	rx = xlr[tcharset][fcharset];	/* Input translation function */
	sx = xls[tcharset][fcharset];	/* Output translation function */
    }
    debug(F101,"tinit tcharset","",tcharset);
    debug(F101,"tinit fcharset","",fcharset);
#ifdef COMMENT
    debug(F101,"tinit sx   ","",sx);
    debug(F101,"tinit rx   ","",rx);
#endif /* COMMENT */
#endif /* NOCSETS */
    myinit[0] = '\0';			/* Haven't sent init string yet */
    retrans = 0;			/* Packet retransmission count */
    sndtyp = 0;				/* No previous packet */
    xflg = 0;				/* Reset x-packet flag */
    memstr = 0;				/* Reset memory-string flag */
    memptr = NULL;			/*  and pointer */
    bctu = bctl = 1;			/* Reset block check type to 1 */
    autopar = 0;			/* Automatic parity detection flag */
    rqf = -1;				/* Reset 8th-bit-quote request flag */
    ebq = MYEBQ;			/* Reset 8th-bit quoting stuff */
    ebqflg = 0;				/* 8th bit quoting not enabled */
    ebqsent = 0;			/* No 8th-bit prefix bid sent yet */
    sq = 'Y';				/* 8th-bit prefix bid I usually send */
    if (savmod) {			/* If global file mode was saved, */
    	binary = savmod;		/*  restore it, */
	savmod = 0;			/*  unsave it. */
    }
    pktnum = 0;				/* Initial packet number to send */
    cxseen = czseen = discard = 0;	/* Reset interrupt flags */
    *filnam = '\0';			/* Clear file name */
    spktl = 0;				/* And its length */
    nakstate = 0;			/* Assume not in a NAK'ing state */
    numerrs = 0;			/* Transmission error counter */
    if (server) 			/* If acting as server, */
      timint = srvtim;			/* Use server timeout interval. */
    else				/* Otherwise */
      timint = chktimo(rtimo,timef);	/* Begin by using local value */
    spsiz = spsizr;			/* Initial send-packet size */
    wslots = 1;				/* One window slot */
    wslotn = 1;				/* No window negotiated yet */
    winlo = 0;				/* Packet 0 is at window-low */
    x = mksbuf(1);			/* Make a 1-slot send-packet buffer */
    if (x < 0) return(x);
    x = getsbuf(0);			/* Allocate first send-buffer. */
    debug(F101,"tinit getsbuf","",x);
    if (x < 0) return(x);
    dumpsbuf();
    x = mkrbuf(wslots);			/* & a 1-slot receive-packet buffer. */
    if (x < 0) return(x);
    what = W_NOTHING;			/* Doing nothing so far... */
    lsstate = 0;			/* Initialize locking shift state */
    whatru = 0;
    return(0);
}

VOID
pktinit() {				/* Initialize packet sequence */
    pktnum = 0;				/* number & window low. */
    winlo = 0;
}

/*  R I N I T  --  Respond to S or I packet  */

VOID
rinit(d) CHAR *d; {
    char *tp;
    ztime(&tp);
    tlog(F110,"Transaction begins",tp,0L); /* Make transaction log entry */
    tlog(F110,"Global file mode:", binary ? "binary" : "text", 0L);
    tlog(F110,"Collision action:", fncnam[fncact],0);
    tlog(F100,"","",0);
    filcnt = filrej = 0;		/* Init file counters */
    spar(d);
    ack1(rpar());
#ifdef datageneral
    if ((local) && (!quiet))            /* Only do this if local & not quiet */
        consta_mt();                    /* Start the asynch read task */
#endif /* datageneral */
}


/*  R E S E T C  --  Reset per-transaction character counters */

VOID
resetc() {
    rptn = 0;				/* Repeat counts */
    fsecs = flci = flco = 0L;		/* File chars in and out */
    tfc = tlci = tlco = 0L;		/* Total file, line chars in & out */
#ifdef COMMENT
    fsize = -1L;			/* File size */
#else
    if (what != W_SEND)
      fsize = -1L;
    debug(F101,"resetc fsize","",fsize);
#endif /* COMMENT */
    timeouts = retrans = 0;		/* Timeouts, retransmissions */
    spackets = rpackets = 0;		/* Packet counts out & in */
    crunched = 0;			/* Crunched packets */
    wcur = 0;				/* Current window size */
    wmax = 0;				/* Maximum window size used */
}

/*  S I N I T  --  Get & verify first file name, then send Send-Init packet */
/*
 Returns:
   1 if send operation begins successfully
   0 if send operation fails
*/
#ifdef DYNAMIC
char *cmargbuf = NULL;
#else
char cmargbuf[256];
#endif /* DYNAMIC */
char *cmargp[2];

int
sinit() {
    int x;				/* Worker int */
    char *tp, *xp, *m;			/* Worker string pointers */

    filcnt = filrej = 0;		/* Initialize file counters */
    sndsrc = nfils;			/* Source for filenames */
#ifdef DYNAMIC
    if (!cmargbuf && !(cmargbuf = malloc(256)))
	fatal("sinit: no memory for cmargbuf");
#endif /* DYNAMIC */
    cmargbuf[0] = NUL;			/* Initialize name buffer */

    debug(F101,"sinit nfils","",nfils);
    debug(F110,"sinit cmarg",cmarg,0);
    debug(F110,"sinit cmarg2",cmarg2,0);
    if (nfils == 0) {			/* Sending from stdin or memory. */
	if ((cmarg2 != NULL) && (*cmarg2)) {
	    cmarg = cmarg2;		/* If F packet, "as-name" is used */
	    cmarg2 = "";		/* if provided */
	} else cmarg = "stdin";		/* otherwise just use "stdin" */
	strcpy(cmargbuf,cmarg);
	cmargp[0] = cmargbuf;
	cmargp[1] = "";
	cmlist = cmargp;
	nfils = 1;
    }
#ifdef COMMENT
    if (nfils < 1) {			/* Filespec pointed to by cmarg */
	if (nfils < 0) sndsrc = 1;
	nfils = 1;			/* Change it to cmlist */
	strcpy(cmargbuf,cmarg);		/* so we have a consistent way */
	cmargp[0] = cmargbuf;		/* of going thru the file list. */
	cmargp[1] = "";
	cmlist = cmargp;
    }

/* At this point, cmlist contains the list of filespecs to send */

    debug(F111,"sinit *cmlist",*cmlist,nfils);

    xp = *cmlist;			/* Save this for messages */
#else
    xp = (nfils < 0) ? cmarg : *cmlist;
#endif

    x = gnfile();			/* Get first filename. */
    m = NULL;				/* Error message pointer */
    debug(F101,"sinit gnfil","",x);
    switch (x) {
      case -5: m = "Too many files match wildcard"; break;
      case -4: m = "Cancelled"; break;
      case -3: m = "Read access denied"; break;
      case -2: m = "File is not readable"; break;
      case -1: m = iswild(filnam) ? "No files match" : "File not found";
	break;
      case  0: m = "No filespec given!" ; break;
      default:
	break;
    }
    debug(F101,"sinit nfils","",nfils);
    debug(F110,"sinit filnam",filnam,0);
    if (x < 1) {			/* Didn't get a file. */
	if (server)			/* Doing GET command */
	  errpkt((CHAR *)m);		/* so send Error packet. */
	else				/* Doing SEND command */
	  screen(SCR_EM,0,0l,m);	/* so print message. */
	tlog(F110,xp,m,0L);		/* Make transaction log entry. */
	freerbuf(rseqtbl[0]);		/* Free the buffer the GET came in. */
	return(0);			/* Return failure code */
    }
    if (!local && !server) sleep(delay); /* Delay if requested */
#ifdef datageneral
    if ((local) && (!quiet))            /* Only do this if local & not quiet */
        consta_mt();                    /* Start the asynch read task */
#endif /* datageneral */
    freerbuf(rseqtbl[0]);		/* Free the buffer the GET came in. */
    sipkt('S');				/* Send the Send-Init packet. */
    ztime(&tp);				/* Get current date/time */
    tlog(F110,"Transaction begins",tp,0L); /* Make transaction log entry */
    tlog(F110,"Global file mode:", binary ? "binary" : "text", 0L);
    tlog(F100,"","",0);
    debug(F111,"sinit ok",filnam,0);
    return(1);
}

int
#ifdef CK_ANSIC
sipkt(char c)				/* Send S or I packet. */
#else
sipkt(c) char c;
#endif
/* sipkt */ {
    CHAR *rp; int k;
    debug(F101,"sipkt pktnum","",pktnum);
    k = sseqtbl[pktnum];		/* Find slot for this packet */
    debug(F101,"sipkt k","",k);
    if (k < 0) {			/* No slot? */
	k = getsbuf(winlo = pktnum);	/* Make one. */
	debug(F101,"sipkt getsbuf","",k);    
    }
    ttflui();				/* Flush pending input. */
    rp = rpar();			/* Get protocol parameters. */
    return(spack(c,pktnum,(int)strlen((char *)rp),rp)); /* Send them. */
}

/*  X S I N I T  --  Retransmit S-packet  */
/*
  For use in the GET-SEND sequence, when we start to send, but receive another
  copy of the GET command because the receiver didn't get our S packet.
  This retransmits the S packet and frees the receive buffer for the ACK.
  This special case is necessary because packet number zero is being re-used.
*/
VOID
xsinit() {
    int k;
    k = rseqtbl[0];
    debug(F101,"xsinit k","",k);
    if (k > -1)
      freerbuf(k);
    resend(0);
}

/*  R C V F I L -- Receive a file  */

/*
  Incoming filename is in data field of F packet.
  This function decodes it into the srvcmd buffer, substituting an
  alternate "as-name", if one was given.
  Then it does any requested transformations (like converting to
  lowercase), and finally if a file of the same name already exists, 
  takes the desired collision action.
*/
#ifdef pdp11
#define XNAMLEN 65
#else
#define XNAMLEN 256
#endif /* pdp11 */

char ofn1[XNAMLEN];			/* Buffer for output file name */
char * ofn2;				/* Pointer to backup file name */
int ofn1x;				/* Flag output file already exists */
int opnerr;				/* Flag for open error */

int					/* Returns success ? 1 : 0 */
rcvfil(n) char *n; {
#ifdef OS2
#ifdef __32BIT__
    char *zs, *longname, *newlongname, *pn; /* OS/2 long name items */
#endif /* __32BIT__ */
#endif /* OS2 */
#ifdef DTILDE
    char *dirp, *tilde_expand();
#endif /* DTILDE */
    int dirflg;

    opnerr = 0;
    ofn2 = NULL;			/* No new name (yet) */
    lsstate = 0;			/* Cancel locking-shift state */
    srvptr = srvcmd;			/* Decode file name from packet. */
    decode(rdatap,putsrv,0);		/* Don't xlate charsets. */
    if (*srvcmd == '\0')		/* Watch out for null F packet. */
      strcpy((char *)srvcmd,"NONAME");
#ifdef DTILDE
    if (*srvcmd == '~') {
	dirp = tilde_expand((char *)srvcmd); /* Expand tilde, if any. */
	if (*dirp != '\0') strcpy((char *)srvcmd,dirp);
    }
#else
#ifdef OS2
    if (isalpha(*srvcmd) && srvcmd[1] == ':' && srvcmd[2] == '\0')
      strcat((char *)srvcmd,"NONAME");
#endif /* OS2 */
#endif /* DTILDE */
    screen(SCR_FN,0,0l,(char *)srvcmd);	/* Put it on screen if local */
    debug(F110,"rcvfil",(char *)srvcmd,0); /* Debug log entry */
    debug(F110,"rcvfil cmarg2",cmarg2,0);
    tlog(F110,"Receiving",(char *)srvcmd,0L); /* Transaction log entry */
    if (cmarg2) {			/* Check for alternate name */
        if (*cmarg2) {
	    debug(F110,"rcvfil substituting cmarg2",cmarg2,0);
	    strcpy((char *)srvcmd,cmarg2); /* Got one, use it. */
	}
    }
    cmarg2 = "";			/* Done with alternate name */

    if ((int)strlen((char *)srvcmd) > XNAMLEN) /* Watch out for overflow */
      *(srvcmd + XNAMLEN - 1) = NUL;

    /* At this point, srvcmd[] contains the incoming filename or as-name */

    if (fnrpath) {			/* RECEIVE PATHNAMES OFF? */
	char *t;			/* Yes. */
	zstrip((char *)srvcmd,&t);	/* Off with it. */
	debug(F110,"rcvfil zstrip",t,0);
	if (!t)				/* Be sure we didn't strip too much */
	  strcpy(ofn1,"UNKNOWN");
	else if (*t == '\0')
	  strcpy(ofn1,"UNKNOWN");
	else strcpy(ofn1,t);
	strcpy((char *)srvcmd,ofn1);	/* Now copy it back. */
    }

    /* Now srvcmd contains incoming filename with path possibly stripped */

    if (fncnv)				/* FILE NAMES CONVERTED? */
      zrtol((char *)srvcmd,(char *)ofn1); /* Yes, convert to local form */
    else
      strcpy(ofn1,(char *)srvcmd);	/* No, copy literally. */

    /* Now the incoming filename, possibly converted, is in ofn1[]. */

#ifdef OS2
    /* Don't refuse the file just because the name is illegal. */
    if (!IsFileNameValid(ofn1)) {	/* Name is OK for OS/2? */
#ifdef __32BIT__
	char *zs = NULL;
	zstrip(ofn1, &zs);		/* Not valid, strip unconditionally */
	if (zs) {
	    iattr.longname.len = strlen(zs); /* Store in attribute structure */
	    if (iattr.longname.val)	/* Free previous longname, if any */
	      free(iattr.longname.val);
	    iattr.longname.val = (char *) malloc( iattr.longname.len + 1 ) ;
	    if (iattr.longname.val)	/* Remember this (illegal) name */
	      strcpy( iattr.longname.val, zs ) ;
	}
#endif /* __32BIT__ */
	debug(F110,"rcvfil: invalid file name",ofn1,0);
	ChangeNameForFAT(ofn1);	/* Change to an acceptable name */
	debug(F110,"rcvfil: FAT file name",ofn1,0);

    } else {				/* Name is OK. */

	debug(F110,"rcvfil: valid file name",ofn1,0);
#ifdef __32BIT__
	iattr.longname.len = 0;
	if (iattr.longname.val)		/* Free previous longname, if any */
	  free(iattr.longname.val);
	iattr.longname.val = NULL;	/* This file doesn't need a longname */
#endif /* __32BIT__ */
    }
#endif /* OS2 */
    debug(F110,"rcvfil as",ofn1,0);

/* Filename collision action section. */

    dirflg =				/* Is it a directory name? */
#ifdef CK_TMPDIR
        isdir(ofn1)
#else
	0
#endif /* CK_TMPDIR */
	  ;
    debug(F101,"rcvfil dirflg","",dirflg);
    ofn1x = (zchki(ofn1) != -1);	/* File already exists? */
    debug(F101,"rcvfil ofn1x",ofn1,ofn1x);

    if ( (
#ifdef UNIX
	strcmp(ofn1,"/dev/null") &&	/* It's not the null device? */
#endif /* UNIX */
	!stdouf ) &&			/* Not copying to standard output? */
	ofn1x ||			/* File of same name exists? */
	dirflg ) {			/* Or file is a directory? */
        debug(F111,"rcvfil exists",ofn1,fncact);
	switch (fncact) {		/* Yes, do what user said. */
	  case XYFX_A:			/* Append */
	    debug(F100,"rcvfil append","",0);
	    if (dirflg) {
		rf_err = "Can't append to a directory";
		tlog(F100," error - can't append to directory","",0);
		discard = opnerr = 1;
		return(0);
	    }
	    tlog(F110," appending to",ofn1,0);
	    break;
	  case XYFX_Q:			/* Query (Ask) */
	    break;			/* not implemented */
	  case XYFX_B:			/* Backup (rename old file) */
	    if (dirflg) {
		rf_err = "Can't rename existing directory";
		tlog(F100," error - can't rename directory","",0);
		discard = opnerr = 1;
		return(0);
	    }
	    znewn(ofn1,&ofn2);		/* Get new unique name */
	    tlog(F110," backup:",ofn2,0);
	    debug(F110,"rcvfil backup ofn1",ofn1,0);
	    debug(F110,"rcvfil backup ofn2",ofn2,0);
#ifdef OS2
#ifdef __32BIT__
/*
  In case this is a FAT file system, we can't change only the FAT name, we
  also have to change the longname from the extended attributes block.
  Otherwise, we'll have many files with the same longname and if we copy them
  to an HPFS volume, only one will survive.
*/
	    if (os2getlongname(ofn1, &longname) > -1) {
		if (strlen(longname)) {
		    char tmp[10];
		    extern int ck_znewn;
		    sprintf(tmp,".~%d~",ck_znewn);
		    newlongname =
		      (char *) malloc(strlen(longname) + strlen(tmp) + 1 ) ;
		    if ( newlongname ) {
			strcpy( newlongname, longname ) ;
			strcat( newlongname, tmp ) ;
			os2setlongname(ofn1, newlongname);
			free(newlongname) ;
		    }
		}
	    } else debug(F100,"rcvfil os2getlongname failed","",0);
#endif /* __32BIT__ */
#endif /* OS2 */

#ifdef COMMENT
	    /* Do this later, in opena()... */
	    if (zrename(ofn1,ofn2) < 0) {
		rf_err = "Can't transform filename";
		debug(F110,"rcvfil rename fails",ofn1,0);
		discard = opnerr = 1;
		return(0);
	    }
#endif /* COMMENT */
	    break;

	  case XYFX_D:			/* Discard (refuse new file) */
	    discard = 1;
	    rejection = 1;		/* Horrible hack: reason = name */
	    debug(F101,"rcvfil discard","",discard);
	    tlog(F100," refused: name","",0);
	    break;

	  case XYFX_R:			/* Rename incoming file */
	    znewn(ofn1,&ofn2);		/* Make new name for it */
#ifdef OS2
#ifdef __32BIT__
		if (iattr.longname.len) {
		    char tmp[10];
		    extern int ck_znewn;
		    sprintf(tmp,".~%d~",ck_znewn);
		    newlongname =
		      (char *) malloc(iattr.longname.len + strlen(tmp) + 1);
		    if (newlongname) {
			strcpy( newlongname, iattr.longname.val);
			strcat( newlongname, tmp);
			debug(F110,
			      "Rename Incoming: newlongname",newlongname,0);
			iattr.longname.len = strlen(newlongname);
			if (iattr.longname.val)
			  free(iattr.longname.val);
			iattr.longname.val = newlongname ;
		    }
		}
#endif /* __32BIT__ */
#endif /* OS2 */
	    break;
	  case XYFX_X:			/* Replace old file */
	    debug(F100,"rcvfil overwrite","",0);
	    if (dirflg) {
		rf_err = "Can't overwrite existing directory";
		tlog(F100," error - can't overwrite directory","",0);
		discard = opnerr = 1;
#ifdef COMMENT
		return(0);
#else
		break;
#endif /* COMMENT */
	    }
	    tlog(F110,"overwriting",ofn1,0);
	    break;
	  case XYFX_U:			/* Refuse if older */
	    debug(F100,"rcvfil update","",0);
	    if (dirflg) {
		rf_err = "File has same name as existing directory";
		tlog(F110," error - directory exists:",ofn1,0);
		discard = opnerr = 1;
#ifdef COMMENT
		/* Don't send an error packet, just refuse the file */
		return(0);
#endif /* COMMENT */
	    }
	    break;			/* Not here, we don't have */
					/* the attribute packet yet. */
	  default:
	    debug(F101,"rcvfil bad collision action","",fncact);
	    break;
	}
    }
    debug(F110,"rcvfil ofn1",ofn1,0);
    debug(F110,"rcvfil ofn2",ofn2,0);
    if (fncact == XYFX_R && ofn1x && ofn2) { /* Renaming incoming file? */
	screen(SCR_AN,0,0l,ofn2);	/* Display renamed name */
	strcpy(n, ofn2);		/* Return it */
    } else {				/* No */
	screen(SCR_AN,0,0l,ofn1);	/* Display regular name */
	strcpy(n, ofn1);		/* and return it. */
    }

#ifdef CK_MKDIR
/*  Create directory(s) if necessary.  */
    if (!discard && !fnrpath) {		/* RECEIVE PATHAMES ON? */
	debug(F110,"rcvfil calling zmkdir",ofn1,0); /* Yes */
	if (zmkdir(ofn1) < 0) {
	    debug(F100,"zmkdir fails","",0);
	    tlog(F110," error - directory creation failure:",ofn1,0);
	    rf_err = "Directory creation failure.";
	    discard = 1;
	    return(0);
	}
    }
#else
    debug(F110,"sfile CK_MKDIR not defined",ofn1,0);
#endif /* CK_MKDIR */

#ifndef NOICP
/* #ifndef MAC */
/* Why not Mac? */
    strcpy(fspec,ofn1);			/* Here too for \v(filespec) */
/* #endif */ 
#endif /* NOICP */
    debug(F110,"rcvfil: n",n,0);
    ffc = 0L;				/* Init per-file counters */
    cps = oldcps = 0L ;
    rs_len = 0L;
    rejection = -1;
    fsecs = gtimer();			/* Time this file started */
    filcnt++;
    intmsg(filcnt);
    return(1);				/* Successful return */
}


/*  R E O F  --  Receive End Of File packet for incoming file */

/*
  Closes the received file.
  Returns:
    0 on success.
   -1 if file could not be closed.
    2 if disposition was mail, mail was sent, but temp file not deleted.
    3 if disposition was print, file was printed, but not deleted.
   -2 if disposition was mail and mail could not be sent
   -3 if disposition was print and file could not be printed
*/
int
reof(f,yy) char *f; struct zattr *yy; {
    int x;
    char *p;
    char c;

    debug(F111,"reof fncact",f,fncact);
    debug(F101,"reof discard","",discard);
    success = 1;			/* Assume status is OK */
    lsstate = 0;			/* Cancel locking-shift state */
    if (discard) {			/* Handle attribute refusals, etc. */
	debug(F101,"reof discarding","",0);
	success = 0;			/* Status = failed. */
	if (rejection == '#' ||		/* Unless rejection reason is */
	    rejection ==  1  ||		/* date or name (SET FILE COLLISION */
	    rejection == '?')		/* UPDATE or DISCARD) */
	  success = 1;			    
	debug(F101,"reof success","",success);
	filrej++;			/* Count this rejection. */
	discard = 0;			/* We never opened the file, */
	return(0);			/* so we don't close it. */
    }
#ifdef DEBUG
    if (deblog) {
	debug(F101,"reof cxseen","",cxseen);
	debug(F101,"reof czseen","",czseen);
	debug(F110,"reof rdatap",rdatap,0);
    }
#endif /* DEBUG */
    if (cxseen == 0) cxseen = (*rdatap == 'D');	/* Got cancel directive? */
    success = (cxseen || czseen) ? 0 : 1; /* Set SUCCESS flag appropriately */
    if (!success) filrej++;		/* "Uncount" this file */
    debug(F101,"reof success","",czseen);
    x = clsof(cxseen || czseen);	/* Close the file (resets cxseen) */
    if (x < 0) {			/* If failure to close, FAIL */
	if (success) filrej++;
	success = 0;
    }
    if (success && atcapu) zstime(f,yy,0); /* Set file creation date */
#ifdef OS2
#ifdef __32BIT__
    if (success && yy->longname.len)
      os2setlongname( f, yy->longname.val ) ;
#endif /* __32BIT__ */
#endif /* OS2 */
    if (success == 0) xitsta |= W_RECV;	/* And program return code */

/* Handle dispositions from attribute packet... */

#ifndef NOFRILLS
    if (yy->disp.len != 0) {
	p = yy->disp.val;
	c = *p++;
	if (c == 'M') {			/* Mail to user. */
	    x = zmail(p,filnam);	/* Do the system's mail command */
	    if (x < 0) success = 0;	/* Remember status */
	    tlog(F110,"mailed",filnam,0L);
	    tlog(F110," to",p,0L);
	    zdelet(filnam);		/* Delete the file */
	} else if (c == 'P') {		/* Print the file. */
	    x = zprint(p,filnam);	/* Do the system's print command */
	    if (x < 0) success = 0;	/* Remember status */
	    tlog(F110,"printed",filnam,0L);
	    tlog(F110," with options",p,0L);
#ifndef VMS
#ifndef STRATUS
	    /* spooler will delete file after print complete in VOS & VMS */
	    if (zdelet(filnam) && x == 0) x = 3; /* Delete the file */
#endif /* STRATUS */
#endif /* VMS */
	}
    }
#endif /* NOFRILLS */
    debug(F101,"reof returns","",x);
    *filnam = '\0';
    return(x);
}

/*  R E O T  --  Receive End Of Transaction  */

VOID
reot() {
    cxseen = czseen = discard = 0;	/* Reset interruption flags */
    tstats();
}

/*  S F I L E -- Send File header or teXt header packet  */

/*  Call with x nonzero for X packet, zero for F packet  */
/*  Returns 1 on success, 0 on failure                   */

int
sfile(x) int x; {
#ifdef pdp11
#define PKTNL 64
#else
#define PKTNL 256
#endif /* pdp11 */
    char pktnam[PKTNL+1];		/* Local copy of name */
    char *s;

    /* cmarg2 or filnam (with that precedence) have the file's name */

    lsstate = 0;			/* Cancel locking-shift state */
    if (nxtpkt() < 0) return(0);	/* Bump packet number, get buffer */
    if (x == 0) {			/* F-Packet setup */
    	if (cmarg2 && *cmarg2) {	/* If we have a send-as name, */
	    debug(F111,"sfile cmarg2",cmarg2,cmarg2);
	    strncpy(pktnam,cmarg2,PKTNL); /* copy it literally, */
	    cmarg2 = "";		/* and blank it out for next time. */
    	} else {			/* Otherwise... */
	    debug(F101,"sfile fnspath","",fnspath);
	    if (fnspath) {		/* Stripping path names? */
		char *t;		/* Yes. */
		zstrip(filnam,&t);	/* Strip off the path. */
		debug(F110,"sfile zstrip",t,0);
		if (!t) t = "UNKNOWN";	/* Be cautious... */
		else if (*t == '\0')
		  t = "UNKNOWN";
		strncpy(pktnam,t,PKTNL); /* Copy stripped name literally. */
	    } else {			/* No stripping. */
		strcpy(pktnam,filnam);	/* Copy whole name. */
	    }
	    /* pktnam[] has the packet name, filnam[] has the original name. */
	    /* But we still need to convert pktnam if FILE NAMES CONVERTED.  */

	    if (fncnv) {		/* If converting names, */
		zltor(pktnam,(char *)srvcmd); /* convert it to common form, */
		strcpy(pktnam,(char *)srvcmd); /* with srvcmd as temp buffer */
		*srvcmd = NUL;
	    }
    	}
    	debug(F110,"sfile",filnam,0);	/* Log debugging info */
    	debug(F110," pktnam",pktnam,0);
    	if (openi(filnam) == 0) 	/* Try to open the input file */
	  return(0); 		
#ifdef CK_RESEND
	if (sendmode == SM_PSEND)	/* PSENDing? */
	  if (sendstart > 0L)		/* Starting position */
	    if (zfseek(sendstart) < 0)	/* seek to it... */
	      return(0);
#endif /* CK_RESEND */
    	s = pktnam;			/* Name for packet data field */
#ifdef OS2
	/* Never send a disk letter. */
	if (isalpha(*s) && (*(s+1) == ':'))
	  s += 2;
#endif /* OS2 */

    } else {				/* X-packet setup, not F-packet. */

    	debug(F110,"sxpack",cmdstr,0);	/* Log debugging info */
    	s = cmdstr;			/* Name for data field */
    }

    /* Now s points to the string that goes in the packet data field. */

    encstr((CHAR *)s);			/* Encode the name. */
					/* Send the F or X packet */
    /* If the encoded string did not fit into the packet, it was truncated. */
   
#ifdef COMMENT
    spack((char) (x ? 'X' : 'F'), pktnum, size, encbuf+7);
#else
    spack((char) (x ? 'X' : 'F'), pktnum, size, data);
#endif

    if (x == 0) {			/* Display for F packet */
    	if (displa) {			/* Screen */
	    screen(SCR_FN,'F',(long)pktnum,filnam);
	    screen(SCR_AN,0,0l,pktnam);
	    screen(SCR_FS,0,fsize,"");
    	}
    	tlog(F110,"Sending",filnam,0L);	/* Transaction log entry */
    	tlog(F110," as",pktnam,0L);
	if (binary) {			/* Log file mode in transaction log */
	    tlog(F101," mode: binary","",(long) binary);
	} else {			/* If text mode, check character set */
	    tlog(F100," mode: text","",0L);
#ifndef NOCSETS
	    tlog(F110," file character set",fcsinfo[fcharset].name,0L);
	    if (tcharset == TC_TRANSP)
	      tlog(F110," xfer character set","transparent",0L);
	    else
	      tlog(F110," xfer character set",tcsinfo[tcharset].name,0L);
#endif /* NOCSETS */
	}
    } else {				/* Display for X-packet */

    	screen(SCR_XD,'X',(long)pktnum,cmdstr);	/* Screen */
    	tlog(F110,"Sending from:",cmdstr,0L);	/* Transaction log */
    }
    intmsg(++filcnt);			/* Count file, give interrupt msg */
    first = 1;				/* Init file character lookahead. */
    ffc = 0L;				/* Init file character counter. */
    cps = oldcps = 0L ; /* Init cps statistics */
    rejection = -1;
    fsecs = gtimer();			/* Time this file started */
    debug(F101,"SFILE fsecs","",fsecs);
    return(1);
}

/*  S D A T A -- Send a data packet */

/*
  Returns -1 if no data to send (end of file).  If there is data, a data
  packet is sent, and sdata() returns 1.

  For window size greater than 1, keep sending data packets until window
  is full or characters start to appear from the other Kermit, whichever
  happens first.

  In the windowing case, when there is no more data left to send (or when
  sending has been interrupted), sdata() does nothing and returns 0 each time
  it is called until the current packet number catches up to the last data
  packet that was sent.
*/

int
sdata() {
    int i, x, len;
    
    debug(F101,"sdata entry, first","",first);
    debug(F101," drain","",drain);

/* The "drain" flag is used with window size > 1.  It means we have sent  */
/* our last data packet.  If called and drain is not zero, then we return */
/* 0 as if we had sent an empty data packet, until all data packets have  */
/* been ACK'd, then then we can finally return -1 indicating EOF, so that */
/* the protocol can switch to seof state.  This is a kludge, but at least */
/* it's localized...  */

    if (first == 1) drain = 0;		/* Start of file, init drain flag. */

    if (drain) {			/* If draining... */
	debug(F101,"sdata draining, winlo","",winlo);
	if (winlo == pktnum)		/* If all data packets are ACK'd */
	  return(-1);			/* return EOF indication */
	else				/* otherwise */
	  return(0);			/* pretend we sent a data packet. */
    }
    debug(F101,"sdata sbufnum","",sbufnum);
    for (i = sbufnum; i > 0; i--) {
        debug(F101,"sdata countdown","",i);
	x = nxtpkt();			/* Get next pkt number and buffer */
	debug(F101,"sdata packet","",pktnum);
	if (x < 0) return(0);
/***	dumpsbuf(); */
	if (cxseen || czseen) {		/* If interrupted, done. */
	    if (wslots > 1) {
		drain = 1;
		debug(F101,"sdata cx/zseen, drain","",cxseen);
		return(0);
	    } else {
		return(-1);
	    }
	}
#ifdef COMMENT
	if (spsiz > 94)			/* Fill the packet's data buffer */
	  len = getpkt(spsiz-bctl-6,1);	/* long packet */
	else				/*  or */
	  len = getpkt(spsiz-bctl-3,1);	/* short packet */
#else
	len = getpkt(spsiz,1);
#endif /* COMMENT */
	if (len == 0) {			/* Done if no data. */
	    if (pktnum == winlo) return(-1);
	    drain = 1;			/* But can't return -1 until all */
	    debug(F101,"sdata eof, drain","",drain);
	    return(0);			/* ACKs are drained. */
	}
	spack('D',pktnum,len,data);	/* Send the data packet. */
	x = ttchk();			/* Peek at input buffer. */
	debug(F101,"sdata ttchk","",x);	/* ACKs waiting, maybe?  */
/*
  Here we check to see if any ACKs or NAKs have arrived, in which case we
  break out of the D-packet-sending loop and return to the state switcher
  to process them.  This is what makes our windows slide instead of lurch.
*/
	if (
#ifdef GEMDOS
/*
  In the Atari ST version, ttchk() can only return 0 or 1.  But note: x will
  probably always be > 0, since the as-yet-unread packet terminator from the
  last packet is probably still in the buffer, so sliding windows will
  probably never happen when the Atari ST is the file sender.  The alternative
  is to say "if (0)", in which case the ST will always send a window full of
  packets before reading any ACKs or NAKs.
*/
	    x > 0
  
#else /* !GEMDOS */
/*
  In other versions, ttchk() returns the actual count.
  It can't be a Kermit packet if it's less than five bytes long.
*/
	    x > 4
  
#endif /* GEMDOS */
	    )
	  return(1);			/* Yes, stop sending data packets */
    }					/* and go try to read the ACKs. */
    return(1);
}


/*  S E O F -- Send an End-Of-File packet */

/*  Call with a string pointer to character to put in the data field, */
/*  or else a null pointer or "" for no data.  */

/*
  There are two "send-eof" functions.  seof() is used to send the normal eof
  packet at the end of a file's data (even if the file has no data), or when
  a file transfer is interrupted.  sxeof() is used to send an EOF packet that
  occurs because of attribute refusal.  The difference is purely a matter of
  buffer allocation and packet sequence number management.  Both functions
  act as "front ends" to the common send-eof function, szeof().
*/

/* Code common to both seof() and sxeof() */

int
szeof(s) CHAR *s; {
    lsstate = 0;			/* Cancel locking-shift state */
    if ((s != NULL) && (*s != '\0')) {
	spack('Z',pktnum,1,s);
	xitsta |= W_SEND;
	tlog(F100," *** interrupted, sending discard request","",0L);
	filrej++;
    } else {
	spack('Z',pktnum,0,(CHAR *)"");
    }
    discard = 0;			/* Turn off per-file discard flag */
    return(0);
}

int
seof(s) CHAR *s; {

/*
  ckcpro.w, before calling seof(), sets window size back to 1 and then calls
  window(), which clears out the old buffers.  This is OK because the final
  data packet for the file has been ACK'd.  However, sdata() has already
  called nxtpkt(), which set the new value of pktnum which seof() will use.
  So all we need to do here is is allocate a new send-buffer.
*/
    if (getsbuf(pktnum) < 0) {	/* Get a buffer for packet n */
	debug(F101,"seof can't get s-buffer","",pktnum);
	return(-1);
    }
    return(szeof(s));
}

/*
  Version of seof() to be called when sdata() has not been called before.  The
  difference is that this version calls nxtpkt() to allocate a send-buffer and
  get the next packet number.
*/
int
sxeof(s) CHAR *s; {
    int x;
    x = nxtpkt();			/* Get next pkt number and buffer */
    if (x < 0)
      debug(F101,"sxeof nxtpkt fails","",pktnum);
    else
      debug(F101,"sxeof packet","",pktnum);
    return(szeof(s));
}

/*  S E O T -- Send an End-Of-Transaction packet */

int
seot() {
    if (nxtpkt() < 0) return(-1);	/* Bump packet number, get buffer */
    spack('B',pktnum,0,(CHAR *)"");	/* Send the EOT packet */
    cxseen = czseen = discard = 0;	/* Reset interruption flags */
    tstats();				/* Log timing info */
    return(0);
}


/*   R P A R -- Fill the data array with my send-init parameters  */

CHAR dada[20];				/* Use this instead of data[]. */
					/* To avoid some kind of wierd */
					/* addressing foulup in spack()... */
					/* (which might be fixed now...) */

CHAR *
rpar() {
    if (rpsiz > MAXPACK)		/* Biggest normal packet I want. */
      dada[0] = tochar(MAXPACK);	/* If > 94, use 94, but specify */
    else				/* extended packet length below... */
      dada[0] = tochar(rpsiz);		/* else use what the user said. */
    dada[1] = tochar(chktimo(pkttim,0)); /* When I want to be timed out */
    dada[2] = tochar(mypadn);		/* How much padding I need (none) */
    dada[3] = ctl(mypadc);		/* Padding character I want */
    dada[4] = tochar(eol);		/* End-Of-Line character I want */
    dada[5] = myctlq;			/* Control-Quote character I send */

    switch (rqf) {			/* 8th-bit prefix (single-shift) */
      case -1:				/* I'm opening the bids */
      case  1:				/* Other Kermit already bid 'Y' */
	if (parity) ebq = sq = MYEBQ ;	/* So I reply with '&' if parity */
	break;				/*  otherwise with 'Y'. */
      case  0:				/* Other Kermit bid nothing */
      case  2:				/* Other Kermit sent a valid prefix */
	break;				/* So I reply with 'Y'. */
    }
    debug(F000,"rpar 8bq sq","",sq);
    debug(F000,"rpar 8bq ebq","",ebq);
    if (lscapu == 2)			/* LOCKING-SHIFT FORCED */
      dada[6] = 'N';			/* requires no single-shift */
    else				/* otherwise send prefix or 'Y' */
      dada[6] = sq;
    ebqsent = dada[6];			/* And remember what I really sent */

    dada[7] = (bctr == 4) ? 'B' : bctr + '0'; /* Block check type */
    if (rptena) {
	if (rptflg)			/* Run length encoding */
	  dada[8] = rptq;		/* If receiving, agree */
	else				/* by replying with same character. */
	  dada[8] = rptq = myrptq;	/* When sending use this. */
    } else dada[8] = SP;		/* Not enabled, put a space here. */

    /* CAPAS mask */

    dada[9] = tochar((lscapr ? lscapb : 0) | /* Locking shifts */
		     (atcapr ? atcapb : 0) | /* Attribute packets */
		     (lpcapr ? lpcapb : 0) | /* Long packets */
		     (swcapr ? swcapb : 0) | /* Sliding windows */
		     (rscapr ? rscapb : 0)); /* RESEND */
    dada[10] = tochar(swcapr ? wslotr : 1);  /* CAPAS+1 = Window size */
    if (urpsiz > 94)
      rpsiz = urpsiz - 1;		/* Long packets ... */
    dada[11] = tochar(rpsiz / 95);	/* Long packet size, big part */
    dada[12] = tochar(rpsiz % 95);	/* Long packet size, little part */
#ifndef WHATAMI
    dada[13] = '\0';			/* Terminate the init string */
#else
    dada[13] = '0';			/* CAPAS+4 = WONT CHKPNT */
    dada[14] = '_';			/* CAPAS+5 = CHKINT (reserved) */
    dada[15] = '_';			/* CAPAS+6 = CHKINT (reserved) */
    dada[16] = '_';			/* CAPAS+7 = CHKINT (reserved) */
    dada[17] = tochar( WM_FLAG   |	/* CAPAS+8 = WHATAMI... */
      (server ? WM_SERVE : 0)    |	/* I am (not) a server */
	(binary ? WM_FMODE : 0)  |	/*  My file transfer mode is ... */
	  (fncnv ? WM_FNAME : 0)); 	/*    My filename conversion is ... */
    dada[18] = NUL;			/* Terminate the init string */
#endif /* WHATAMI */

#ifdef DEBUG
    if (deblog) {
	debug(F110,"rpar",dada,0);
	rdebu(dada,(int)strlen((char *)dada));
    }
#endif /* DEBUG */
    strcpy((char *)myinit,(char *)dada);
    return(dada);			/* Return pointer to string. */
}

int
spar(s) CHAR *s; {			/* Set parameters */
    int x, y, lpsiz;

    debug(F110,"entering spar",s,0);

    s--;				/* Line up with field numbers. */

/* Limit on size of outbound packets */
    x = (rln >= 1) ? xunchar(s[1]) : 80;
    lpsiz = spsizr;			/* Remember what they SET. */
    if (spsizf) {			/* SET-command override? */
	if (x < spsizr) spsiz = x;	/* Ignore LEN unless smaller */
    } else {				/* otherwise */
	spsiz = (x < 10) ? 80 : x;	/* believe them if reasonable */
    }
    spmax = spsiz;			/* Remember maximum size */

/* Timeout on inbound packets */
    if (timef) {
	timint = rtimo;			/* SET SEND TIMEOUT value overrides */
    } else {				/* Otherwise use requested value, */
	x = (rln >= 2) ? xunchar(s[2]) : rtimo; /* if it is legal. */
	timint = (x < 0) ? rtimo : x;
    }
    timint = chktimo(timint,timef);	/* Adjust if necessary */

/* Outbound Padding */
    npad = 0; padch = '\0';
    if (rln >= 3) {
	npad = xunchar(s[3]);
	if (rln >= 4) padch = ctl(s[4]); else padch = 0;
    }
    if (npad) {
	int i;
	for (i = 0; i < npad; i++) padbuf[i] = dopar(padch);
    }

/* Outbound Packet Terminator */
    seol = (rln >= 5) ? xunchar(s[5]) : CR;
    if ((seol < 1) || (seol > 31)) seol = CR;

/* Control prefix that the other Kermit is sending */
    x = (rln >= 6) ? s[6] : '#';
    ctlq = ((x > 32 && x < 63) || (x > 95 && x < 127)) ? x : '#';

/* 8th-bit prefix */
/*
  NOTE: Maybe this could be simplified using rcvtyp.
  If rcvtyp == 'Y' then we're reading the ACK,
  otherwise we're reading the other Kermit's initial bid.
  But his horrendous code works, so leave it alone for now.
*/
    rq = (rln >= 7) ? s[7] : 0;
    if (rq == 'Y') rqf = 1;
      else if ((rq > 32 && rq < 63) || (rq > 95 && rq < 127)) rqf = 2;
        else rqf = 0;
    debug(F000,"spar 8bq rq","",rq);
    debug(F000,"spar 8bq sq","",sq);
    debug(F000,"spar 8bq ebq","",ebq);
    debug(F101,"spar 8bq rqf","",rqf);
    switch (rqf) {
      case 0:				/* Field is missing from packet. */
	ebqflg = 0;			/* So no 8th-bit prefixing. */
	break;
      case 1:				/* Other Kermit sent 'Y' = Will Do. */
	/*
          When I am the file receiver, ebqsent is 0 because I didn't send a
          negotiation yet.  If my parity is set to anything other than NONE,
          either because my user SET PARITY or because I detected parity bits
          on this packet, I reply with '&', otherwise 'Y'.

	  When I am the file sender, ebqsent is what I just sent in rpar(),
          which can be 'Y', 'N', or '&'.  If I sent '&', then this 'Y' means
          the other Kermit agrees to do 8th-bit prefixing.

          If I sent 'Y' or 'N', but then detected parity on the ACK packet
          that came back, then it's too late: there is no longer any way for
          me to tell the other Kermit that I want to do 8th-bit prefixing, so
          I must not do it, and in that case, if there is any 8-bit data in 
          the file to be transferred, the transfer will fail because of block
          check errors.

          The following clause covers all of these situations:
	*/
	if (parity && (ebqsent == 0 || ebqsent == '&')) {
	    ebqflg = 1;
	    ebq = MYEBQ;
	}
	break;
      case 2:				/* Other Kermit send a valid prefix */
	if (ebqflg = (ebq == sq || sq == 'Y'))
	  ebq = rq;
    }
    if (lscapu == 2) {     /* But no single-shifts if LOCKING-SHIFT FORCED */
	ebqflg = 0;
	ebq = 'N';
    }

/* Block check */
    x = 1;
    if (rln >= 8) {
	if (s[8] == 'B') x = 4;
	else x = s[8] - '0';
	if ((x < 1) || (x > 4)) x = 1;
    }
    bctr = x;

/* Repeat prefix */

    rptflg = 0;				/* Assume no repeat-counts */
    if (rln >= 9) {			/* Is there a repeat-count field? */
	char t;				/* Yes. */
	t = s[9];			/* Get its contents. */
/*
  If I'm sending files, then I'm reading these parameters from an ACK, and so
  this character must agree with what I sent.
*/
	if (rptena) {			/* If enabled ... */
	    if ((char) rcvtyp == 'Y') {	/* Sending files, reading ACK. */
		if (t == myrptq) rptflg = 1;
	    } else {			/* I'm receiving files */
		if ((t > 32 && t < 63) || (t > 95 && t < 127)) {
		    rptflg = 1;
		    rptq = t;
		}
	    }
	} else rptflg = 0;
    }

/* Capabilities */

    atcapu = lpcapu = swcapu = rscapu = 0; /* Assume none of these. */
    if (lscapu != 2) lscapu = 0;	/* Assume no LS unless forced. */
    y = 11;				/* Position of next field, if any */
    if (rln >= 10) {
        x = xunchar(s[10]);
	debug(F101,"spar capas","",x);
        atcapu = (x & atcapb) && atcapr; /* Attributes */
	lpcapu = (x & lpcapb) && lpcapr; /* Long packets */
	swcapu = (x & swcapb) && swcapr; /* Sliding windows */
	rscapu = (x & rscapb) && rscapr; /* RESEND */
	debug(F101,"spar lscapu","",lscapu);
	debug(F101,"spar lscapr","",lscapr);
	debug(F101,"spar ebqflg","",ebqflg);
	if (lscapu != 2) lscapu = ((x & lscapb) && lscapr && ebqflg) ? 1 : 0;
	debug(F101,"spar swcapr","",swcapr);
	debug(F101,"spar swcapu","",swcapu);
	debug(F101,"spar lscapu","",lscapu);
	for (y = 10; (xunchar(s[y]) & 1) && (rln >= y); y++) ;
	debug(F101,"spar y","",y);
    }

/* Long Packets */
    debug(F101,"spar lpcapu","",lpcapu);
    if (lpcapu) {
        if (rln > y+1) {
	    x = xunchar(s[y+2]) * 95 + xunchar(s[y+3]);
	    debug(F101,"spar lp len","",x);
	    if (spsizf) {		/* If overriding negotiations */
		spsiz = (x < lpsiz) ? x : lpsiz; /* do this, */
	    } else {			         /* otherwise */
		spsiz = (x > MAXSP) ? MAXSP : x; /* do this. */
	    }
	    if (spsiz < 10) spsiz = 80;	/* Be defensive... */
	}
    }
    /* (PWP) save current send packet size for optimal packet size calcs */
    spmax = spsiz;
    debug(F101,"spar lp spmax","",spmax);
    timint = chktimo(timint,timef);	/* Recalculate the packet timeout! */
    
/* Sliding Windows... */

    if (swcapr) {			/* Only if requested... */
        if (rln > y) {			/* See what other Kermit says */
	    x = xunchar(s[y+1]);
	    debug(F101,"spar window","",x);
	    wslotn = (x > MAXWS) ? MAXWS : x;
/*
  wslotn = negotiated size (from other Kermit's S or I packet).
  wslotr = requested window size (from this Kermit's SET WINDOW command).
*/
	    if (wslotn > wslotr)	/* Use the smaller of the two */
	      wslotn = wslotr;
	    if (wslotn < 1)		/* Watch out for bad negotiation */
	      wslotn = 1;
	    if (wslotn > 1) {
		swcapu = 1;		/* We do windows... */
		if (wslotn > maxtry)	/* Retry limit must be greater */
		  maxtry = wslotn + 1;	/* than window size. */
	    }
	    debug(F101,"spar window after adjustment","",x);
	} else {			/* No window size specified. */
	    wslotn = 1;			/* We don't do windows... */
	    debug(F101,"spar window","",x);
	    swcapu = 0;
	    debug(F101,"spar no windows","",wslotn);
	}
    }

/* Now recalculate packet length based on number of windows.   */
/* The nogotiated number of window slots will be allocated,    */
/* and the maximum packet length will be reduced if necessary, */
/* so that a windowful of packets can fit in the big buffer.   */

    if (wslotn > 1) {			/* Shrink to fit... */
	x = adjpkl(spsiz,wslotn,bigsbsiz);
	if (x < spsiz) {
	    spsiz = spmax = x;
	    debug(F101,"spar sending, redefine spsiz","",spsiz);
	}
    }
#ifdef WHATAMI
    if (rln > y+7)			/* Get WHATAMI info if any */
      whatru = xunchar(s[y+8]);
#endif /* WHATAMI */

/* Record parameters in debug log */
#ifdef DEBUG
    if (deblog) sdebu(rln);
#endif /* DEBUG */
    numerrs = 0;			/* Start counting errors here. */
    return(0);
}

/*  G N F I L E  --  Get name of next file to send  */
/*
  Expects global sndsrc to be:
   -1: next filename to be obtained by calling znext().
    0: no next file name
    1: (or greater) next filename to be obtained from **cmlist.
  Returns:
    1, with name of next file in filnam.
    0, no more files, with filnam set to empty string.
   -1, file not found
   -2, file is not readable
   -3, read access denied
   -4, cancelled
   -5, too many files match wildcard
*/

int
gnfile() {
    int x; long y;
    int retcode = 0;

    debug(F101,"gnfile sndsrc","",sndsrc);
    fsize = -1L;			/* Initialize file size */
    if (sndsrc == 0) {			/* It's not really a file */
	if (nfils > 0) {		/* It's a pipe, or stdin */
	    strcpy(filnam, *cmlist);	/* Copy its "name" */
	    nfils = 0;			/* There is no next file */
	    return(1);			/* OK this time */
	} else return(0);		/* but not next time */
    }

/* If file group interruption (C-Z) occurred, fail.  */

    if (czseen) {
	tlog(F100,"Transaction cancelled","",0L);
        debug(F100,"gnfile czseen","",0);
	return(-4);
    }

/* Loop through file list till we find a readable, sendable file */

    y = -1L;				/* Loop exit (file size) variable */
    while (y < 0L) {			/* Keep trying till we get one... */
	if (sndsrc > 0) {		/* File list in cmlist */
	    debug(F101,"gnfile nfils","",nfils);
	    if (nfils-- > 0) {		/* Still some left? */
		strcpy(filnam,*cmlist++);
		debug(F111,"gnfile cmlist filnam",filnam,nfils);
		if (!clfils) {		/* Expand only if not from cmdline */
		    x = zxpand(filnam);
		    debug(F101,"gnfile zxpand","",x);
		    if (x == 1) {
			znext(filnam);
			goto gotnam;
		    }
		    if (x == 0) {
			retcode = -1; /* None match */
			continue;
		    }
		    if (x < 0) return(-5); /* Too many to expand */
		    sndsrc = -1;	/* Change send-source to znext() */
		}
	    } else {			/* We're out of files. */
		debug(F101,"gnfile done","",nfils);
		*filnam = '\0';
		return(retcode);
	    }
	}

/* Otherwise, step to next element of internal wildcard expansion list. */

	if (sndsrc < 0) {
	    x = znext(filnam);
	    debug(F111,"gnfile znext",filnam,x);
	    if (x == 0) {		/* If no more, */
		sndsrc = 1;		/* go back to list */
		continue;
	    }
	}

/* Get here with a filename. */

gotnam:
	if (sndsrc) {
	    y = zchki(filnam);		/* Check if file readable */
	    retcode = (int) y;		/* Possible return code */
	    if (y == -1L) {		/* If not found */
		debug(F110,"gnfile skipping:",filnam,0);
		tlog(F111,filnam,"not sent, reason",(long)y);
		screen(SCR_ST,ST_SKIP,0l,filnam);
		continue;
	    } else if (y < 0) {
		continue;
	    } else {
		fsize = y;
		return(1);
	    }
	} else return(1);		/* sndsrc is 0... */
    }
    *filnam = '\0';			/* Should never get here */
    return(0);
}

/*  S N D H L P  --  Routine to send builtin help  */

int
sndhlp(p) char *p; {
#ifndef NOSERVER
    nfils = 0;				/* No files, no lists. */
    xflg = 1;				/* Flag we must send X packet. */
    strcpy(cmdstr,versio);		/* Data for X packet. */
    first = 1;				/* Init getchx lookahead */
    memstr = 1;				/* Just set the flag. */
    memptr = p;				/* And the pointer. */
    if (binary) {			/* If file mode is binary, */
	savmod = binary;		/*  remember to restore it later. */
	binary = XYFT_T;			/*  turn it back to text for this, */
    }
    return(sinit());
#else
    return(0);
#endif /* NOSERVER */
}

#ifdef OS2
/*  S N D S P A C E -- send disk space message  */
int
sndspace(int drive) {
#ifndef NOSERVER
    static char spctext[64];
    if (drive)
      sprintf(spctext, " Drive %c: %ldK free\n", drive, 
	      zdskspace(drive - 'A' + 1) / 1024L);
    else
      sprintf(spctext, " Free space: %ldK\n", zdskspace(0)/1024L);
    nfils = 0;			/* No files, no lists. */
    xflg = 1;			/* Flag we must send X packet. */
    strcpy(cmdstr,"free space");/* Data for X packet. */
    first = 1;			/* Init getchx lookahead */
    memstr = 1;			/* Just set the flag. */
    memptr = spctext;		/* And the pointer. */
    if (binary) {		/* If file mode is binary, */
        savmod = binary;	/*  remember to restore it later. */
        binary = XYFT_T;		/*  turn it back to text for this, */
    }
    return(sinit());
#else
    return(0);
#endif /* NOSERVER */
}
#endif /* OS2 */

/*  C W D  --  Change current working directory  */

/*
 String passed has first byte as length of directory name, rest of string
 is name.  Fails if can't connect, else ACKs (with name) and succeeds. 
*/

int
cwd(vdir) char *vdir; {
    char *cdd, *zgtdir(), *dirp;

    vdir[xunchar(*vdir) + 1] = '\0';	/* Terminate string with a null */
    dirp = vdir+1;
    tlog(F110,"Directory requested: ",dirp,0L);
    if (zchdir(dirp)) {		/* Try to change */
	cdd = zgtdir();		/* Get new working directory. */
	debug(F110,"cwd",cdd,0);
	encstr((CHAR *)cdd);
#ifdef COMMENT
	ack1((CHAR *)(encbuf+7));
#else
	ack1(data);
#endif /* COMMENT */
	screen(SCR_CD,0,0l,cdd);
	tlog(F110,"Changed directory to",cdd,0L);
	return(1); 
    } else {
	debug(F110,"cwd failed",dirp,0);
	tlog(F110,"Failed to change directory to",dirp,0L);
	return(0);
    }
}


/*  S Y S C M D  --  Do a system command  */

/*  Command string is formed by concatenating the two arguments.  */

int
syscmd(prefix,suffix) char *prefix, *suffix; {
    char *cp;

    if (!prefix)
      return(0);
    if (!*prefix)
      return(0);
    for (cp = cmdstr; *prefix != '\0'; *cp++ = *prefix++) ;
    while (*cp++ = *suffix++) ;		/* copy suffix */

    debug(F110,"syscmd",cmdstr,0);
    if (zxcmd(ZIFILE,cmdstr) > 0) {
	debug(F110,"syscmd zxcmd ok",cmdstr,0);
	nfils = sndsrc = 0;		/* Flag that input is from stdin */
	xflg = hcflg = 1;		/* And special flags for pipe */
	if (binary) {			/* If file mode is binary, */
	    savmod = binary;		/*  remember to restore it later. */
	    binary = XYFT_T;			/*  turn it back to text for this, */
	}
	return (sinit());		/* Send S packet */
    } else {
	debug(F100,"syscmd zxcmd failed",cmdstr,0);
	return(0);
    }
}

/*  R E M S E T  --  Remote Set  */
/*  Called by server to set variables as commanded in REMOTE SET packets.  */
/*  Returns 1 on success, 0 on failure.  */

int
remset(s) char *s; {
    int len, i, x, y;
    char *p;

    len = xunchar(*s++);		/* Length of first field */
    p = s + len;			/* Pointer to second length field */
    *p++ = '\0';			/* Zero out second length field */
    x = atoi(s);			/* Value of first field */
    debug(F111,"remset",s,x);
    debug(F110,"remset",p,0);
    switch (x) {			/* Do the right thing */
      case 132:				/* Attributes (all, in) */
	atcapr = atoi(p);
	return(1);
      case 133:				/* File length attributes */
      case 233:				/* IN/OUT combined */
      case 148:				/* Both kinds of lengths */
      case 248:
	atleni = atleno = atoi(p);
	return(1);
      case 134:				/* File Type (text/binary) */
      case 234:
	attypi = attypo = atoi(p);
	return(1);
      case 135:				/* File creation date */
      case 235:
	atdati = atdato = atoi(p);
	return(1);
      case 139:				/* File Blocksize */
      case 239:
	atblki = atblko = atoi(p);
	return(1);
      case 141:				/* Encoding / Character Set */
      case 241:
	atenci = atenco = atoi(p);
	return(1);
      case 142:				/* Disposition */
      case 242:
	atdisi = atdiso = atoi(p);
	return(1);
      case 145:				/* System ID */
      case 245:
	atsidi = atsido = atoi(p);
	return(1);
      case 147:				/* System-Dependent Info */
      case 247:
	atsysi = atsyso = atoi(p);
	return(1);
      case 232:				/* Attributes (all, out) */
	atcapr = atoi(p);
	return(1);
      case 300:				/* File type (text, binary) */
	binary = atoi(p);
	return(1);
      case 301:				/* File name conversion */
	fncnv = 1 - atoi(p);		/* (oops) */
	return(1);
      case 302:				/* File name collision */
	x = atoi(p);
	if (x == XYFX_R) warn = 1;	/* Rename */
	if (x == XYFX_X) warn = 0;	/* Replace */
	fncact = x;
	return(1);
      case 310:				/* Incomplete File Disposition */
	keep = atoi(p);			/* Keep, Discard */
	return(1);
      case 311:				/* Blocksize */
	fblksiz = atoi(p);
	return(1);
      case 312:				/* Record Length */
	frecl = atoi(p);
	return(1);
      case 313:				/* Record format */
	frecfm = atoi(p);
	return(1);
      case 314:				/* File organization */
	forg = atoi(p);
	return(1);
      case 315:				/* File carriage control */
	fcctrl = atoi(p);
	return(1);
      case 400:				/* Block check */
	y = atoi(p);
	if (y < 5 && y > 0) {
	    bctr = y;
	    return(1);
	} else if (*p == 'B') {
	    bctr = 4;
	    return(1);
	}
	return(0);
      case 401:				/* Receive packet-length */
	rpsiz = urpsiz = atoi(p);
	if (urpsiz > MAXRP) urpsiz = MAXRP; /* Max long-packet length */
	if (rpsiz > 94) rpsiz = 94;	    /* Max short-packet length */
	urpsiz = adjpkl(urpsiz,wslots,bigrbsiz);
	return(1);
      case 402:				/* Receive timeout */
	y = atoi(p);			/* Client is telling us */
	if (y > -1 && y < 999) {	/* the timeout that it wants */
	    pkttim = chktimo(y,timef);	/* us to tell it to use. */
	    return(1);
	} else return(0);
      case 403:				/* Retry limit */
	y = atoi(p);
	if (y > -1 && y < 95) {
	    maxtry = y;
	    return(1);
	} else return(0);
      case 404:				/* Server timeout */
	y = atoi(p);
	if (y < 0) return(0);
	srvtim = y;
	return(1);

#ifndef NOCSETS
      case 405:				/* Transfer character set */
	for (i = 0; i < ntcsets; i++) { 
	    if (!strcmp(tcsinfo[i].designator,p)) break;
	}
	debug(F101,"remset xfer charset lookup","",i);
	if (i == ntcsets) return(0);
	tcharset = tcsinfo[i].code;	/* if known, use it */
	if (tcharset == TC_TRANSP)
	  rx = NULL;
	else
	  rx = xlr[tcharset][fcharset];	/* translation function */
	return(1);
#endif /* NOCSETS */

      case 406:				/* Window slots */
	y = atoi(p);
	if (y == 0) y = 1;
	if (y < 1 && y > MAXWS) return(0);
	wslotr = y;
	swcapr = 1;
	urpsiz = adjpkl(urpsiz,wslots,bigrbsiz);
	return(1);
      default:				/* Anything else... */
	return(0);
    }
}

/* Adjust packet length based on number of window slots and buffer size */

int
adjpkl(pktlen,slots,bufsiz) int pktlen, slots, bufsiz; {
    debug(F101,"adjpkl len","",pktlen);
    debug(F101,"adjpkl slots","",slots);
    debug(F101,"adjpkl bufsiz","",bufsiz);
    if (((pktlen + 6) * slots) > bufsiz)
      pktlen = (bufsiz / slots) - 6;
    debug(F101,"adjpkl new len","",pktlen);
    return(pktlen);
}
