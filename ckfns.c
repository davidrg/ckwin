char *fnsv = "C-Kermit functions, 4.2(026) 5 Mar 85";

/*  C K F N S  --  System-independent Kermit protocol support functions...  */

/*  ...Part 1 (others moved to ckfns2 to make this module small enough) */

/*
 System-dependent primitives defined in:

   ckx???.c -- terminal i/o
   cxz???.c -- file i/o, directory structure

*/

#include "ckermi.h"			/* Symbol definitions for Kermit */

/* Externals from ckmain.c */

extern int spsiz, timint, npad, chklen, ebq, ebqflg, rpt, rptq, rptflg, capas;

extern int pktnum, prvpkt, sndtyp, fsize, bctr, bctu,
  size, osize, maxsize, spktl, nfils, stdouf, warn, timef;

extern int parity, speed, turn, turnch, 
 delay, displa, pktlog, tralog, seslog, xflg, mypadn;

extern long filcnt, ffc, flci, flco, tlci, tlco, tfc;

extern int deblog, hcflg, binary, fncnv, local, server, cxseen, czseen;

extern char padch, mypadc, eol, ctlq, myctlq, sstate, *hlptxt;

extern char filnam[], sndpkt[], recpkt[], data[], srvcmd[], *srvptr, stchr, 
 mystch;

extern char *cmarg, *cmarg2, **cmlist;
char *strcpy();

/* Variables local to this module */

static char *memptr;			/* Pointer for memory strings */

static char cmdstr[100];		/* Unix system command string */

static int  sndsrc;			/* Flag for where to send from: */
					/* -1: name in cmdata */
					/*  0: stdin          */
					/* >0: list in cmlist */

static int  memstr,			/* Flag for input from memory string */
     t,					/* Current character */
     next;				/* Next character */

/*  E N C S T R  --  Encode a string from memory. */

/*  Call this instead of getpkt() if source is a string, rather than a file. */

encstr(s) char* s; {
    int m; char *p;

    m = memstr; p = memptr;		/* Save these. */

    memptr = s;				/* Point to the string. */
    memstr = 1;				/* Flag memory string as source. */
    next = -1;				/* Initialize character lookahead. */
    getpkt(spsiz);			/* Fill a packet from the string. */
    memstr = m;				/* Restore memory string flag */
    memptr = p;				/* and pointer */
    next = -1;				/* Put this back as we found it. */
    debug(F111,"encstr",data,size);
}

/* E N C O D E - Kermit packet encoding procedure */

encode(a) int a; {			/* The current character */
    int a7;				/* Low order 7 bits of character */
    int b8;				/* 8th bit of character */

    if (rptflg)	{			/* Repeat processing? */
        if (a == next) {		/* Got a run... */
	    if (++rpt < 94)		/* Below max, just count */
                return;
	    else if (rpt == 94) {	/* Reached max, must dump */
                data[size++] = rptq;
                data[size++] = tochar(rpt);
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

/* D E C O D E  --  Kermit packet decoding procedure */

/* Call with string to be decoded and an output function. */

decode(buf,fn) char *buf; int (*fn)(); {
    unsigned int a, a7, b8;		/* Low order 7 bits, and the 8th bit */

    rpt = 0;

    while ((a = *buf++) != '\0') {
	if (rptflg) {			/* Repeat processing? */
	    if (a == rptq) {		/* Yes, got a repeat prefix? */
		rpt = unchar(*buf++);	/* Yes, get the repeat count, */
		a = *buf++;		/* and get the prefixed character. */
	    }
	}
	b8 = 0;				/* Check high order "8th" bit */
	if (ebqflg) {			/* 8th-bit prefixing? */
	    if (a == ebq) {		/* Yes, got an 8th-bit prefix? */
		b8 = 0200;		/* Yes, remember this, */
		a = *buf++;		/* and get the prefixed character. */
	    }
	}
	if (a == ctlq) {		/* If control prefix, */
	    a  = *buf++;		/* get its operand. */
	    a7 = a & 0177;		/* Only look at low 7 bits. */
	    if ((a7 >= 0100 && a7 <= 0137) || a7 == '?') /* Uncontrollify */
	    a = ctl(a);			/* if in control range. */
	}
	a |= b8;			/* OR in the 8th bit */
	if (rpt == 0) rpt = 1;		/* If no repeats, then one */
	for (; rpt > 0; rpt--) {	/* Output the char RPT times */
	    if (a == CR && !binary) break; /* But skip CR if binary. */
	    ffc++, tfc++;		/* Count the character */
	    (*fn)(a);			/* Send it to the output function. */
	}
    }
}


/*  Output functions passed to 'decode':  */

putsrv(c) char c; { 	/*  Put character in server command buffer  */
    *srvptr++ = c;
    *srvptr = '\0';	/* Make sure buffer is null-terminated */
}

puttrm(c) char c; {     /*  Output character to console.  */
    conoc(c);
}

putfil(c) char c; {	/*  Output char to file. */
    zchout(ZOFILE,c);
}

/*  G E T P K T -- Fill a packet data field  */

/*
 Gets characters from the current source -- file or memory string.
 Encodes the data into the packet, filling the packet optimally.

 Uses global variables:
 t     -- current character.
 next  -- next character.
 data  -- the packet data buffer.
 size  -- number of characters in the data buffer.

Returns the size as value of the function, and also sets global size,
and fills (and null-terminates) the global data array.

Before calling getpkt the first time for a given source (file or string),
set the variable 'next' to -1.
*/

getpkt(maxsize) int maxsize; {		/* Fill one packet buffer */
    int i;				/* Loop index. */

    static char leftover[6] = { '\0', '\0', '\0', '\0', '\0', '\0' };

    if (next < 0) t = getch();		/* Get first character of file. */

    /* Do any leftovers */

    for (size = 0; (data[size] = leftover[size]) != '\0'; size++)
    	;
    *leftover = '\0';

    /* Now fill up the rest of the packet. */

    while(t >= 0) {			/* Until EOF... */
	next = getch();			/* Get next character for lookahead. */
	osize = size;			/* Remember current position. */
        encode(t);			/* Encode the current character. */
        t = next;			/* Next is now current. */

	if (size == maxsize) 		/* If the packet is exactly full, */
            return(size);		/* and return. */

	if (size > maxsize) {		/* If too big, save some for next. */
	    for (i = 0; (leftover[i] = data[osize+i]) != '\0'; i++)
	    	;
	    size = osize;		/* Return truncated packet. */
	    data[size] = '\0';
	    return(size);
	}
    }
    return(size);			/* Return any partial final buffer. */
}

/*  G E T C H  -- Get the next character from file (or pipe). */
 
/*  Convert newlines to CRLFs if newline/CRLF mapping is being done. */

getch()	{				/* Get next character */
    int a, x;				/* The character to return. */
    static int b = 0;			/* A character to remember. */
    
    if (b > 0) {			/* Do we have a newline saved? */
	b = 0;				/* Yes, return that. */
	return('\n');
    }

    if (memstr)				/* Try to get the next character */
    	x = ((a = *memptr++) == '\0');	/* from the appropriate source, */
    else				/* memory or the current file. */
    	x = ((a = zchin(ZIFILE)) < 0 );

    if (x)
    	return(-1);			/* No more, return -1 for EOF. */
    else {				/* Otherwise, read the next char. */
	ffc++, tfc++;			/* Count it. */
	if (a == '\n' && !binary) {	/* If nl and we must do nl-CRLF */
	    b = a;			/* mapping, save the nl, */
	    return(CR);			/* and return a CR. */
	} else return(a);		/* General case, return the char. */
    }
}


/*  C A N N E D  --  Check if current file transfer cancelled */

canned(buf) char *buf; {
    if (*buf == 'X') cxseen = 1;
    if (*buf == 'Z') czseen = 1;
    debug(F101,"canned: cxseen","",cxseen);
    debug(F101," czseen","",czseen);
    return((czseen || cxseen) ? 1 : 0);
}

/*  T I N I T  --  Initialize a transaction  */

tinit() {
    xflg = 0;				/* reset x-packet flag */
    memstr = 0;				/* reset memory-string flag */
    memptr = NULL;			/*  and pointer */
    bctu = 1;				/* reset block check type to 1 */
    filcnt = 0;				/* reset file counter */
    tfc = tlci = tlco = 0;		/* reset character counters */
    prvpkt = -1;			/* reset packet number */
    pktnum = 0;
    if (server) {			/* If acting as server, */
	timint = 30;			/* use 30 second timeout, */
	nack();				/* send a NAK */
    }
}


/*  R I N I T  --  Respond to S packet  */

rinit(d) char *d; {
    char *tp;
    ztime(&tp);
    tlog(F110,"Transaction begins",tp,0l); /* Make transaction log entry */
    tfc = tlci = tlco = 0;
    spar(d);
    rpar(d);
    ack1(d);
}

/*  S I N I T  --  Make sure file exists, then send Send-Init packet */

sinit() {
    int x; char *tp;

    sndsrc = nfils;			/* Where to look for files to send */
    ztime(&tp);
    tlog(F110,"Transaction begins",tp,0l); /* Make transaction log entry */
    debug(F101,"sinit: sndsrc","",sndsrc);
    if (sndsrc < 0) {			/* Must expand from 'send' command */
	nfils = zxpand(cmarg);		/* Look up literal name. */
	if (nfils < 0) {
	    screen(2,0l,"?Too many files");
	    return(0);
        } else if (nfils == 0) {	/* If none found, */
	    char xname[100];		/* convert the name. */
	    zrtol(cmarg,xname);
	    nfils = zxpand(xname); 	/* Look it up again. */
	}
	if (nfils < 1) {		/* If no match, report error. */
	    if (server) 
	    	errpkt("File not found");
	    else
		screen(2,0l,"?File not found");
	    return(0);
	}
	x = gnfile();			/* Position to first file. */
	if (x < 1) {
	    if (!server) 
	    	screen(2,0l,"?No readable file to send");
            else
	    	errpkt("No readable file to send");
	    return(0);
    	} 
    } else if (sndsrc > 0) {		/* Command line arglist -- */
	x = gnfile();			/* Get the first file from it. */
	if (x < 1) return(0);		/* (if any) */
    } else if (sndsrc == 0) {		/* stdin or memory always exist... */
	cmarg2 = "";			/* No alternate name */
	strcpy(filnam,"stdin");		/* If F packet, filnam is used. */
	tlog(F110,"Sending from",cmdstr,0l); /* If X packet, cmdstr is used. */
    }

    debug(F101,"sinit: nfils","",nfils);
    debug(F110," filnam",filnam,0);
    debug(F110," cmdstr",cmdstr,0);
    ttflui();				/* Flush input buffer. */
    x = rpar(data);			/* Send a Send-Init packet. */
    if (!local && !server) sleep(delay);
    spack('S',pktnum,x,data);
    return(1);
}

sipkt() {
    int x;
    x = rpar(data);			/* Send an I-Packet. */
    spack('I',pktnum,x,data);
}

/*  R C V F I L -- Receive a file  */

rcvfil() {
    int x;
    ffc = flci = flco = 0;		/* Init per-file counters */
    srvptr = srvcmd;			/* Decode packet data. */
    decode(data,putsrv);
    screen(0,0l,srvcmd);			/* Update screen */
    screen(1,0l,"=> ");
    tlog(F110,"Receiving",srvcmd,0l);	/* Transaction log entry */
    if (cmarg2 != NULL) {               /* Check for alternate name */
        if (*cmarg2 != '\0') {
            strcpy(srvcmd,cmarg2);	/* Got one, use it. */
	    *cmarg2 = '\0';
        }
    }
    x = openo(srvcmd,filnam);		/* Try to open it */
    if (x) {
	tlog(F110," as",filnam,0l);
	screen(2,0l,filnam);
	intmsg(++filcnt);
    } else {
        tlog(F110,"Failure to open",filnam,0l);
	screen(2,0l,"*** error");
    }
    return(x);				/* Pass on return code from openo */
}

/*  R E O F  --  Receive End Of File  */

reof() {

    if (cxseen == 0) cxseen = (*data == 'D');
    clsof();
    if (cxseen || czseen) {
	tlog(F100," *** Discarding","",0l);
    } else {
	tlog(F100," end of file","",0l);
	tlog(F101,"  file characters        ","",ffc);
	tlog(F101,"  communication line in  ","",flci);
	tlog(F101,"  communication line out ","",flco);
    }
}

/*  R E O T  --  Receive End Of Transaction  */

reot() {
    char *tp;
    cxseen = czseen = 0; 
    ztime(&tp);
    tlog(F110,"End of transaction",tp,0l);
    if (filcnt > 1) {
	tlog(F101," files","",filcnt);
	tlog(F101," total file characters   ","",tfc);
	tlog(F101," communication line in   ","",tlci);
	tlog(F101," communication line out  ","",tlco);
    }
}

/*  S F I L E -- Send File header packet for global "filnam" */

sfile() {
    char pktnam[100];			/* Local copy of name */

    if (fncnv) {
	if (*cmarg2 != '\0') {		/* If we have a send-as name, */
	    zltor(cmarg2,pktnam);	/* convert it to common form, */
	    cmarg2 = "";		/* and blank it out for next time, */
	} else zltor(filnam,pktnam);	/* otherwise use the real file name. */
    } else {
	if (*cmarg2 != '\0')		/* Same as above, but without */
	    strcpy(pktnam,cmarg2);	/* name conversion */
        else strcpy(filnam,pktnam);
    }

    debug(F110,"sfile",filnam,0);
    if (openi(filnam) == 0) 		/* Try to open the file */
	return(0); 		

    rpt = flci = flco = ffc = 0;	/* OK, Init counters, etc. */
    encstr(pktnam);			/* Encode the name. */
    nxtpkt(&pktnum);			/* Increment the packet number */
    ttflui();				/* Clear pending input */
    spack('F',pktnum,size,data); 	/* Send the F packet */
    if (displa) {
	screen(0,(long)pktnum,filnam);	/* Update screen */
	screen(1,0l,"=> ");
	screen(1,0l,pktnam);
	screen(3,(long)fsize,", size");
	intmsg(++filcnt);		/* Count file, give interrupt msg */
    }
    tlog(F110,"Sending",filnam,0l);	/* Transaction log entry */
    tlog(F110," as",pktnam,0l);
    next = -1;				/* Init file character lookahead. */
    return(1);
}


/* Send an X Packet -- Like SFILE, but with Text rather than File header */

sxpack() {				/* Send an X packet */
    debug(F110,"sxpack",cmdstr,0);
    encstr(cmdstr);			/* Encode any data. */
    rpt = flci = flco = ffc = 0;	/* Init counters, etc. */
    next = -1;				/* Init file character lookahead. */
    nxtpkt(&pktnum);			/* Increment the packet number */
    spack('X',pktnum,size,data);	/* No incrementing pktnum */
    screen(0,(long)pktnum,cmdstr);		/* Update screen. */
    intmsg(++filcnt);
    tlog(F110,"Sending from:",cmdstr,0l);
    return(1);
}

/*  S D A T A -- Send a data packet */

sdata() {
    int len;
    if (cxseen || czseen) return(0);	/* If interrupted, done. */
    if ((len = getpkt(spsiz-chklen-3)) == 0) return(0); /* If no data, done. */
    nxtpkt(&pktnum);			/* Increment the packet number */
    spack('D',pktnum,len,data);		/* Send the packet */
    return(1);
}


/*  S E O F -- Send an End-Of-File packet */

seof() {
    nxtpkt(&pktnum);			/* Increment the packet number */
    if (czseen || cxseen) {
	spack('Z',pktnum,1,"D");
	tlog(F100," *** interrupted, sending discard request","",0l);
    } else {
	spack('Z',pktnum,0,"");
	tlog(F100," end of file","",0l);
	tlog(F101,"  file characters        ","",ffc);
	tlog(F101,"  communication line in  ","",flci);
	tlog(F101,"  communication line out ","",flco);
    }
}


/*  S E O T -- Send an End-Of-Transaction packet */

seot() {
    char *tp;
    nxtpkt(&pktnum);			/* Increment the packet number */
    spack('B',pktnum,0,"");
    cxseen = czseen = 0; 
    ztime(&tp);
    tlog(F110,"End of transaction",tp,0l);
    if (filcnt > 1) {
	tlog(F101," files","",filcnt);
	tlog(F101," total file characters   ","",tfc);
	tlog(F101," communication line in   ","",tlci);
	tlog(F101," communication line out  ","",tlco);
    }
}

/*   R P A R -- Fill the data array with my send-init parameters  */

rpar(data) char data[]; {
    data[0] = tochar(spsiz);		/* Biggest packet I can receive */
    data[1] = tochar(URTIME);		/* When I want to be timed out */
    data[2] = tochar(mypadn);		/* How much padding I need (none) */
    data[3] = ctl(mypadc);		/* Padding character I want */
    data[4] = tochar(MYEOL);		/* End-Of-Line character I want */
    data[5] = CTLQ;			/* Control-Quote character I send */
    if (ebqflg) data[6] = ebq = '&';
    	else data[6] = 'Y';		/* 8-bit quoting */
    data[7] = bctr + '0';		/* Block check type */
    data[8] = MYRPTQ;			/* Do repeat counts */
    data[9] = '\0';
    return(9);				/* Return the length. */
}

/*   S P A R -- Get the other system's Send-Init parameters.  */

spar(data) char data[]; {
    int len, x;

    len = strlen(data);		    	/* Number of fields */

    spsiz = (len-- > 0) ? unchar(data[0]) : DSPSIZ; 	/* Packet size */
    if (spsiz < 10) spsiz = DSPSIZ;

    x = (len-- > 0) ? unchar(data[1]) : DMYTIM;	/* Timeout */
    if (!timef) {			/* Only use if not overridden */
	timint = x;
	if (timint < 0) timint = DMYTIM;
    }

    npad = 0; padch = '\0';		    	    	/* Padding */
    if (len-- > 0) {
	npad = unchar(data[2]);
	if (len-- > 0) padch = ctl(data[3]); else padch = 0;
    }

    eol = (len-- > 0) ? unchar(data[4]) : '\r';	    	/* Terminator  */
    if ((eol < 2) || (eol > 037)) eol = '\r';

    ctlq = (len-- > 0) ? data[5] : CTLQ;    	    	/* Control prefix */

    if (len-- > 0) {			    	    	/* 8th-bit prefix */
	ebq = data[6];
	if ((ebq > 040 && ebq < 0100) || (ebq > 0140 && ebq < 0177)) {
	    ebqflg = 1;
	} else if (parity && (ebq == 'Y')) {
	    ebqflg = 1;
	    ebq = '&';
	} else if (ebq == 'N') {
	    ebqflg = 0;
	} else ebqflg = 0;
    } else ebqflg = 0;

    chklen = 1;				    	    	/* Block check */
    if (len-- > 0) {
	chklen = data[7] - '0';
	if ((chklen < 1) || (chklen > 3)) chklen = 1;
    }
    bctr = chklen;

    if (len-- > 0) {			    	    	/* Repeat prefix */
	rptq = data[8]; 
	rptflg = ((rptq > 040 && rptq < 0100) || (rptq > 0140 && rptq < 0177));
    } else rptflg = 0;

    if (deblog) sdebu(len);
}

/*  S D E B U  -- Record spar results in debugging log  */

sdebu(len) int len; {
    debug(F111,"spar: data",data,len);
    debug(F101," spsiz ","",spsiz);
    debug(F101," timint","",timint);
    debug(F101," npad  ","",npad);
    debug(F101," padch ","",padch);
    debug(F101," eol   ","",eol);
    debug(F101," ctlq  ","",ctlq);
    debug(F101," ebq   ","",ebq);
    debug(F101," ebqflg","",ebqflg);
    debug(F101," chklen","",chklen);
    debug(F101," rptq  ","",rptq);
    debug(F101," rptflg","",rptflg);
}

/*  G N F I L E  --  Get the next file name from a file group.  */

/*  Returns 1 if there's a next file, 0 otherwise  */

gnfile() {
    int x, y;

/* If file group interruption (C-Z) occured, fail.  */

    debug(F101,"gnfile: czseen","",czseen);

    if (czseen) {
	tlog(F100,"Transaction cancelled","",0l);
	return(0);
    }

/* If input was stdin or memory string, there is no next file.  */

    if (sndsrc == 0) return(0);

/* If file list comes from command line args, get the next list element. */

    y = -1;
    while (y < 0) {			/* Keep trying till we get one... */

	if (sndsrc > 0) {
	    if (nfils-- > 0) {
		strcpy(filnam,*cmlist++);
		debug(F111,"gnfile: cmlist filnam",filnam,nfils);
	    } else {
		*filnam = '\0';
		debug(F101,"gnfile cmlist: nfils","",nfils);
		return(0);
	    }
	}

/* Otherwise, step to next element of internal wildcard expansion list. */

	if (sndsrc < 0) {
	    x = znext(filnam);
	    debug(F111,"gnfile znext: filnam",filnam,x);
	    if (x == 0) return(0);
	}

/* Get here with a filename. */

	y = zchki(filnam);		/* Check if file readable */
	if (y < 0) {
	    debug(F110,"gnfile skipping:",filnam,0);
	    tlog(F111,filnam,"not sent, reason",(long)y);
	    screen(0,0l,"Skipping");
	    screen(2,0l,filnam);
	} else fsize = y;
    }    	
    return(1);
}

/*  O P E N I  --  Open an existing file for input  */

openi(name) char *name; {
    int x, filno;
    if (memstr) return(1);		/* Just return if file is memory. */

    debug(F110,"openi",name,0);
    debug(F101," sndsrc","",sndsrc);

    filno = (sndsrc == 0) ? ZSTDIO : ZIFILE;    /* ... */

    debug(F101," file number","",filno);

    if (zopeni(filno,name)) {		/* Otherwise, try to open it. */
	debug(F110," ok",name,0);
    	return(1);
    } else {				/* If not found, */
	char xname[100];		/* convert the name */
	zrtol(name,xname);		/* to local form and then */
	debug(F110," zrtol:",xname,0);
	x = zopeni(filno,xname);	/* try opening it again. */
	debug(F101," zopeni","",x);
	if (x) {
	    debug(F110," ok",xname,0);
	    return(1);			/* It worked. */
        } else {
	    screen(2,0l,"Can't open file");  /* It didn't work. */
	    tlog(F110,xname,"could not be opened",0l);
	    debug(F110," openi failed",xname,0);
	    return(0);
        }
    }
}

/*  O P E N O  --  Open a new file for output.  */

/*  Returns actual name under which the file was opened in string 'name2'. */

openo(name,name2) char *name, *name2; {
    char xname[100], *xp;

    if (stdouf)				/* Receiving to stdout? */
	return(zopeno(ZSTDIO,""));

    debug(F110,"openo: name",name,0);

    xp = xname;
    if (fncnv)				/* If desired, */
    	zrtol(name,xp);			/* convert name to local form */
    else				/* otherwise, */
    	strcpy(xname,name);		/* use it literally */

    debug(F110,"openo: xname",xname,0);

    if (warn) {				/* File collision avoidance? */
	if (zchki(xname) != -1) {	/* Yes, file exists? */
	    znewn(xname,&xp);		/* Yes, make new name. */
	    strcpy(xname,xp);
	    debug(F110," exists, new name ",xname,0);
        }
    }
    if (zopeno(ZOFILE,xname) == 0) {	/* Try to open the file */
	debug(F110,"openo failed",xname,0);
	tlog(F110,"Failure to open",xname,0l);
	return(0);
    } else {
	strcpy(name2,xname);
	debug(F110,"openo ok, name2",name2,0);
	return(1);
    }
}

/*  O P E N T  --  Open the terminal for output, in place of a file  */

opent() {
    ffc = tfc = 0;
    return(zopeno(ZCTERM,""));
}

/*  C L S I F  --  Close the current input file. */

clsif() {
    if (memstr) {			/* If input was memory string, */
	memstr = 0;			/* indicate no more. */
    } else if (hcflg) {
	zclosf();			/* If host cmd close fork, */
    } else zclose(ZIFILE);		/* else close input file. */

    screen(1,0l," [OK]");
    hcflg = cxseen = 0;			/* Reset flags. */
}


/*  C L S O F  --  Close an output file.  */

clsof() {
    zclose(ZOFILE);			/* Close it. */
    if (czseen || cxseen) {
	zdelet(filnam);   		/* Delete it if interrupted. */
	debug(F100,"Discarded","",0);
	tlog(F100,"Discarded","",0l);
	screen(1,0l," [Discarded]");
    } else {
	debug(F100,"Closed","",0);
	screen(1,0l," [OK]");
    }
    cxseen = 0;
}

/*  S N D H L P  --  Routine to send builtin help  */

sndhlp() {
    nfils = 0;				/* No files, no lists. */
    xflg = 1;				/* Flag we must send X packet. */
    strcpy(cmdstr,"help text");		/* Data for X packet. */
    next = -1;				/* Init getch lookahead */
    memstr = 1;				/* Just set the flag. */
    memptr = hlptxt;			/* And the pointer. */
    return(sinit());
}


/*  C W D  --  Change current working directory  */

/*
 String passed has first byte as length of directory name, rest of string
 is name.  Fails if can't connect, else ACKs (with name) and succeeds. 
*/

cwd(vdir) char *vdir; {
    vdir[unchar(*vdir) + 1] = '\0';	/* End with a null */
    if (zchdir(vdir+1)) {
	encstr(vdir+1);
	ack1(data);
	tlog(F110,"Changed directory to",vdir+1,0l);
	return(1); 
    } else {
	tlog(F110,"Failed to change directory to",vdir+1,0l);
	return(0);
    }
}


/*  S Y S C M D  --  Do a system command  */

/*  Command string is formed by concatenating the two arguments.  */

syscmd(prefix,suffix) char *prefix, *suffix; {
    char *cp;

    for (cp = cmdstr; *prefix != '\0'; *cp++ = *prefix++) ;
    while (*cp++ = *suffix++) ;

    debug(F110,"syscmd",cmdstr,0);
    if (zxcmd(cmdstr) > 0) {
	debug(F100,"zxcmd ok","",0);
	nfils = sndsrc = 0;		/* Flag that input from stdin */
	xflg = hcflg = 1;		/* And special flags for pipe */
	return (sinit());		/* Send S packet */
    } else {
	debug(F100,"zxcmd failed","",0);
	return(0);
    }
}
