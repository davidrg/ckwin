char *fnsv = "C-Kermit functions, 4.0(023) 1 Feb 85";

/*  C K F N S  --  System-independent Kermit protocol support functions  */
/*
 System-dependent primitives defined in:

   ckx???.c -- terminal i/o
   cxz???.c -- file i/o, directory structure

*/

#include "ckermi.h"			/* Symbol definitions for Kermit */

/* Externals from ckmain.c */

extern int spsiz, timint, npad, chklen, ebq, ebqflg, rpt, rptq, rptflg, capas;

extern int pktnum, prvpkt, numtry, oldtry, sndtyp, fsize, bctr, bctu,
  inpktl, size, osize, maxsize, spktl, nfils, stdouf, warn, timef;

extern int parity, speed, turn, turnch, ffc, flci, flco, tfc, tlci, tlco, 
    filcnt, delay, displa, pktlog, tralog, seslog, xflg, /* memstr, */ mypadn;

extern int deblog, hcflg, image, binary, fncnv, local, server, cxseen, czseen;

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


/*  I N P U T  --  Attempt to read packet number 'pktnum'.  */

/*
 This is the function that feeds input to Kermit's finite state machine.

 If a special start state is in effect, that state is returned as if it were
 the type of an incoming packet.  Otherwise:

 . If the desired packet arrives within MAXTRY tries, return its type,
   with its data stored in the global 'data' array.

 . If the previous packet arrives again, resend the last packet and wait for
   another to come in.

 . If the desired packet does not arrive within MAXTRY tries, return indicating
   that an error packet should be sent.
*/

input() {
    int len, num, type, numtry;

    if (sstate != 0) {			/* If a start state is in effect, */
	type = sstate;			/* return it like a packet type, */
	sstate = 0;			/* and then nullify it. */
	*data = '\0';
	return(type);
    } else type = rpack(&len,&num,data); /* Else, try to read a packet. */

/* If it's the same packet we just sent, it's an echo.  Read another. */

    if (type == sndtyp) type = rpack(&len,&num,data);

    chkint();				/* Check for console interrupts. */
/*
 If previous packet again, a timeout pseudopacket, or a bad packet, try again.
*/
    for (numtry = 0; num == prvpkt || type == 'T' || type == 'Q' ; numtry++)
    {
	if (numtry > MAXTRY) {		/* If too many tries, give up */
	    strcpy(data,"Timed out.");	/* and send a timeout error packet. */
	    return('E');
	}
	resend();			/* Else, send last packet again, */
	type = rpack(&len,&num,data);	/* and try to read a new one. */
	chkint();			/* Look again for interruptions. */
    }
    return(type);			/* Success, return packet type. */
}


/*  S P A C K  --  Construct and send a packet  */

spack(type,num,len,dat) char type, *dat; int num, len; {
    int i,j;
    
    j = dopar(padch);
    for (i = 0; i < npad; sndpkt[i++] = j)  /* Do any requested padding */
    	;
    sndpkt[i++] = dopar(mystch);	/* Start packet with the start char */
    sndpkt[i++] = dopar(tochar(len+bctu+2));	/* Put in the length */
    sndpkt[i++] = dopar(tochar(num));		/* The packet number */
    sndpkt[i++] = sndtyp = dopar(type);		/* Packet type */

    for (j = len; j > 0; j-- ) sndpkt[i++] = dopar(*dat++); /* Data */

    sndpkt[i] = '\0';			/* Mark end for block check */
    switch(bctu) {
	case 1: 			/* Type 1 - 6 bit checksum */
	    sndpkt[i++] = dopar(tochar(chk1(sndpkt+1)));
	    break;
	case 2:				/* Type 2 - 12 bit checksum*/
	    j = chk2(sndpkt+1);
	    sndpkt[i++] = dopar(tochar((j & 07700) >> 6));
	    sndpkt[i++] = dopar(tochar(j & 077));
	    break;
        case 3:				/* Type 3 - 16 bit CRC-CCITT */
	    j = chk3(sndpkt+1);
	    sndpkt[i++] = dopar(tochar((j & 0170000) >> 12));
	    sndpkt[i++] = dopar(tochar((j & 07700) >> 6));
	    sndpkt[i++] = dopar(tochar(j & 077));
	    break;
	}
    for (j = npad; j > 0; j-- ) sndpkt[i++] = dopar(padch); /* Padding */

    sndpkt[i++] = dopar(eol);		/* EOL character */
    sndpkt[i] = '\0';			/* End of the packet */
    ttol(sndpkt,spktl=i);		/* Send the packet just built */
    flco += spktl;			/* Count the characters */
    tlco += spktl;
    if (pktlog) zsoutl(ZPFILE,sndpkt);	/* If logging packets, log it */
    screen(type,num,sndpkt);		/* Update screen */
}

/*  D O P A R  --  Add an appropriate parity bit to a character  */

dopar (ch) char ch; {
    int a;
    switch (parity) {
	case 'm':  return(ch | 128);		/* Mark */
	case 's':  return(ch & 127);		/* Space */
	case 'o':  ch |= 128;			/* Odd (fall thru) */
	case 'e':				/* Even */
	    a = (ch & 15) ^ ((ch >> 4) & 15);
	    a = (a & 3) ^ ((a >> 2) & 3);
	    a = (a & 1) ^ ((a >> 1) & 1);
	    return(ch | (a << 7));
	default:   return(ch);
    }
}


/*  C H K 1  --  Compute a type-1 Kermit 6-bit checksum.  */

chk1(pkt) char *pkt; {
    int chk;
    chk = chk2(pkt);
    return((((chk & 0300) >> 6) + chk) & 077);
}


/*  C H K 2  --  Compute the numeric sum of all the bytes in the packet.  */

chk2(pkt) char *pkt; {
    unsigned int chk;
    int p;
    for (chk = 0; *pkt != '\0'; *pkt++) {
    	p = (parity) ? *pkt & 0177 : *pkt;
	chk += p;
    }
    return(chk);
}


/*  C H K 3  --  Compute a type-3 Kermit block check.  */
/*
 Calculate the 16-bit CRC of a null-terminated string using a byte-oriented
 tableless algorithm invented by Andy Lowry (Columbia University).  The
 magic number 010201 is derived from the CRC-CCITT polynomial x^16+x^12+x^5+1.
 Note - this function could adapted for strings containing imbedded 0's
 by including a length argument.
*/
chk3(s) char *s; {
    int c, q;
    int crc = 0;

    while ((c = *s++) != '\0') {
	if (parity) c &= 0177;
	q = (crc ^ c) & 017;		/* Low-order nibble */
	crc = (crc >> 4) ^ (q * 010201);
	q = (crc ^ (c >> 4)) & 017;	/* High order nibble */
	crc = (crc >> 4) ^ (q * 010201);
    }
    return(crc);
}


/* Functions for sending various kinds of packets */

ack() {					/* Send an ordinary acknowledgment. */
    spack('Y',pktnum,0,"");		/* No data. */
    nxtpkt(&pktnum);			/* Increment the packet number. */
}					/* Note, only call this once! */

ack1(s) char *s; {			/* Send an ACK with data. */
    spack('Y',pktnum,strlen(s),s);	/* Send the packet. */
    nxtpkt(&pktnum);			/* Increment the packet number. */
}					/* Only call this once! */

nack() {				/* Negative acknowledgment. */
    spack('N',pktnum,0,"");		/* NAK's never have data. */
}

resend() {				/* Send the old packet again. */
    ttol(sndpkt,spktl);
    screen('%',pktnum,sndpkt);
    if (pktlog) zsoutl(ZPFILE,sndpkt);
}

errpkt(reason) char *reason; {		/* Send an error packet. */
    encstr(reason);
    spack('E',pktnum,size,data);
}

scmd(t,dat) char t, *dat; {		/* Send a packet of the given type */
    encstr(dat);			/* Encode the command string */
    ttflui();				/* Flush pending input. */
    spack(t,pktnum,size,data);
}

srinit() {				/* Send R (GET) packet */
    encstr(cmarg);			/* Encode the filename. */
    ttflui();				/* Flush pending input. */
    spack('R',pktnum,size,data);	/* Send the packet. */
}

nxtpkt(num) int *num; {
    prvpkt = *num;			/* Save previous */
    *num = (*num + 1) % 64;		/* Increment packet number mod 64 */
}

sigint() {				/* Terminal interrupt handler */
    errpkt("User typed ^C");
    doexit();
}


/* R P A C K  --  Read a Packet */

rpack(l,n,dat) int *l, *n; char *dat; {
    int i, j, x, done, pstart, pbl;
    char chk[4], xchk[4], t, type;

    chk[3] = xchk[3] = 0;
    i = inlin();			/* Read a line */
    if (i != 0) {
	debug(F101,"rpack: inlin","",i);
	screen('T',pktnum,"");
	return('T');
    }
    debug(F110,"rpack: inlin ok, recpkt",recpkt,0);

/* Look for start of packet */

    for (i = 0; ((t = recpkt[i]) != stchr) && (i < RBUFL) ; i++)
    	;
    if (++i >= RBUFL) return('Q');	/* Skip rest if not found */

/* now "parse" the packet */

    debug(F101,"entering rpack with i","",i);
    done = 0;
    while (!done) {
	debug(F101,"rpack starting at i","",i);
        pstart = i;			/* remember where packet started */

/* length */

	if ((t = recpkt[i++]) == stchr) continue; /* Resynch if SOH */

   /***	if (t == 2) doexit(); *** uncomment this to allow ^A^B cause exit ***/

	if (t == MYEOL) return('Q');
	*l = unchar(t);			/* Packet length */
	debug(F101," pkt len","",*l);

/* sequence number */

	if ((t = recpkt[i++]) == stchr) continue;
	if (t == MYEOL) return('Q');
	*n = unchar(t);
	debug(F101,"rpack: n","",*n);

/* cont'd... */


/* ...rpack(), cont'd */


/* type */

	if ((type = recpkt[i++]) == stchr) continue;
	if (type == MYEOL) return('Q');
	debug(F101,"rpack: type","",type);

	if ((type == 'S') || (type == 'I')) pbl = 1;	/* Heuristics for  */
	else if (type == 'N') pbl = *l - 2;    /* syncing block check type */
	else pbl = bctu;

	*l -= (pbl + 2);		/* Now compute data length */
	debug(F101,"rpack: bctu","",bctu);
	debug(F101," pbl","",pbl);
	debug(F101," data length","",*l);

/* data */

	dat[0] = '\0';			/* Return null string if no data */
	for (j=0; j<*l; i++,j++)
	    if ((dat[j] = recpkt[i]) == stchr) continue;
		else if (dat[j] == MYEOL) return('Q');
	dat[j] = '\0';

/* get the block check */

    	debug(F110," packet chk",recpkt+i,0);
    	for (j = 0; j < pbl; j++) {
	    chk[j] = recpkt[i];
	    debug(F101," chk[j]","",chk[j]);
	    if (chk[j] == stchr) break;
	    if (chk[j] == eol) return('Q');
	    recpkt[i++] = '\0';
	}
	chk[j] = 0;
	debug(F111," chk array, j",chk,j);
	if (j != pbl) continue;		/* Block check right length? */
	done = 1;			/* Yes, done. */
    }

/* cont'd... */


/* ...rpack(), cont'd */


/* Got packet, now check the block check */

    switch (pbl) {
	case 1:
	    xchk[0] = tochar(chk1(&recpkt[pstart]));
	    if (chk[0] != xchk[0]) {
		if (deblog) {
		    debug(F000,"rpack: chk","",chk[0]);
		    debug(F000," should be ","",xchk[0]);
		}
		screen('Q',n,recpkt);
		return('Q');
	    }
	    break;
	case 2:
	    x = chk2(&recpkt[pstart]);
	    xchk[0] = tochar((x & 07700) >> 6);
	    xchk[1] = tochar(x & 077);
	    if (deblog) {
		debug(F000," xchk[0]","=",xchk[0]);
		debug(F000," xchk[1]","=",xchk[1]);
	    }
	    if ((xchk[0] != chk[0]) || (xchk[1] != chk[1])) {
		debug(F100," bct2's don't compare","",0);
		screen('Q',n,recpkt);
		return('Q');
            }
	    break;
	case 3:
	    x = chk3(&recpkt[pstart]);
	    xchk[0] = tochar((x & 0170000) >> 12);
	    xchk[1] = tochar((x & 07700) >> 6);
	    xchk[2] = tochar(x & 077);
	    if (deblog) {
		debug(F000," xchk[0]","=",xchk[0]);
		debug(F000," xchk[1]","=",xchk[1]);
		debug(F000," xchk[2]","=",xchk[2]);
            }
	    if ((xchk[0] != chk[0]) || 
	    	(xchk[1] != chk[1]) || 
		(xchk[2] != chk[2])) {
		    debug(F100," bct3's don't compare","",0);
		    screen('Q',n,recpkt);
		    return('Q');
	    }
	    break;
        }

/* Good packet, return its type */

    ttflui();				/* Done, flush any remaining. */
    screen(type,*n,recpkt);		/* Update screen */
    return(type);
}


/*  I N C H R  --  Input character from communication line, with timeout  */
    	
inchr(timo) int timo; {
    int c;
    c = ttinc(timo);
    debug(F101,"inchr ttinc","",c);
    if (c < 0) return(c); 		/* Get a character */
    if (parity) c = c & 0177;		/* If parity on, discard parity bit. */
    debug(F101," after parity","",c);
    return(c);
}


/*  I N L I N  -- Input a line (up to break char) from communication line  */

/*  Returns 0 on success, nonzero on failure  */

inlin() {
    int e, i, j, k;

    e = (turn) ? turnch : MYEOL;
    i = j = k = 0;
    if (parity) {
    	while ((j != e) && (i < RBUFL) && (k < MAXTRY)) {
	    j = inchr(1);		/* Get char, 1 second timeout */
	    debug(F101,"inlin inchr","",j);
	    if (j < 0) k++;		/* Timed out. */
	    else {
		if (j) recpkt[i++] = j;	/* Save it */
		k = 0;			/* Reset timeout counter. */
	    }
	}
    } else {
    	i = ttinl(recpkt,RBUFL,timint,e);	/* Get them all at once */
	if (i < 0) k = 1;
    }
    debug(F111,"inlin",recpkt,i);
    debug(F101," timeouts","",k);
    if (i < 1) return(1);
    if (pktlog) zsoutl(ZPFILE,recpkt);
    if (k > MAXTRY) return(1);
    tlci += i;				/* Count the characters. */
    flci += i;
    recpkt[i+1] = '\0';			/* Terminate the input string. */
if (0) {    
    if (turn) {				/* If doing line turnaround, */
	debug(F101,"inlin: looking for turnaround","",turnch);
	for (i = 0 ; i < MAXTRY ; i++) {
	    j = inchr(1);
	    if (pktlog) zchout(ZPFILE,j);
	    if (j == turnch) break;     /* Wait for turnaround char */
        }
    }
}
    return(0);
}


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
    tlog(F110,"Transaction begins",tp,0); /* Make transaction log entry */
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
    tlog(F110,"Transaction begins",tp,0); /* Make transaction log entry */
    debug(F101,"sinit: sndsrc","",sndsrc);
    if (sndsrc < 0) {			/* Must expand from 'send' command */
	nfils = zxpand(cmarg);		/* Look up literal name. */
	if (nfils < 0) {
	    screen(2,0,"?Too many files");
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
		screen(2,0,"?File not found");
	    return(0);
	}
	x = gnfile();			/* Position to first file. */
	if (x < 1) {
	    if (!server) 
	    	screen(2,0,"?No readable file to send");
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
	tlog(F110,"Sending from",cmdstr,0); /* If X packet, cmdstr is used. */
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


/*  R C V F I L -- Receive a file  */

rcvfil() {
    int x;

    ffc = flci = flco = 0;		/* Init per-file counters */
    srvptr = srvcmd;			/* Decode packet data. */
    decode(data,putsrv);
    screen(0,0,srvcmd);			/* Update screen */
    screen(1,0,"=> ");
    tlog(F110,"Receiving",srvcmd,0);	/* Transaction log entry */
    if (*cmarg2 != '\0') {		/* Check for alternate name */
	strcpy(srvcmd,cmarg2);		/* Got one, use it. */
	*cmarg2 = '\0';
    }
    x = openo(srvcmd,filnam);		/* Try to open it */
    if (x) {
	tlog(F110," as",filnam,0);
	screen(2,0,filnam);
	intmsg(++filcnt);
    } else {
        tlog(F110,"Failure to open",filnam,0);
	screen(2,0,"*** error");
    }
    return(x);				/* Pass on return code from openo */
}

/*  R E O F  --  Receive End Of File  */

reof() {

    if (cxseen == 0) cxseen = (*data == 'D');
    clsof();

    if (cxseen || czseen) {
	tlog(F100," *** Discarding","",0);
    } else {
	tlog(F100," end of file","",0);
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
    tlog(F110,"End of transaction",tp,0);
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
	screen(0,pktnum,filnam);	/* Update screen */
	screen(1,0,"=> ");
	screen(1,0,pktnam);
	screen(3,fsize,", size");
	intmsg(++filcnt);		/* Count file, give interrupt msg */
    }
    tlog(F110,"Sending",filnam,0);	/* Transaction log entry */
    tlog(F110," as",pktnam,0);
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
    screen(0,pktnum,cmdstr);		/* Update screen. */
    intmsg(++filcnt);
    tlog(F110,"Sending from:",cmdstr,0);
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
	tlog(F100," *** interrupted, sending discard request","",0);
    } else {
	spack('Z',pktnum,0,"");
	tlog(F100," end of file","",0);
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
    tlog(F110,"End of transaction",tp,0);
    if (filcnt > 1) {
	tlog(F101," files","",filcnt);
	tlog(F101," total file characters   ","",tfc);
	tlog(F101," communication line in   ","",tlci);
	tlog(F101," communication line out  ","",tlco);
    }
}


/*   R P A R -- Fill the data array with my send-init parameters  */

rpar(data) char data[]; {
    data[0] = tochar(spsiz-1);		/* Biggest packet I can receive */
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
	tlog(F100,"Transaction cancelled","",0);
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
	    tlog(F111,filnam,"not sent, reason",y);
	    screen(0,0,"Skipping");
	    screen(2,0,filnam);
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
	    screen(2,0,"Can't open file");  /* It didn't work. */
	    tlog(F110,xname,"could not be opened",0);
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
	tlog(F110,"Failure to open",xname,0);
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

    screen(1,0," [OK]");
    hcflg = cxseen = 0;			/* Reset flags. */
}


/*  C L S O F  --  Close an output file.  */

clsof() {
    zclose(ZOFILE);			/* Close it. */
    if (czseen || cxseen) {
	zdelet(filnam);   		/* Delete it if interrupted. */
	debug(F100,"Discarded","",0);
	tlog(F100,"Discarded","",0);
	screen(1,0," [Discarded]");
    } else {
	debug(F100,"Closed","",0);
	screen(1,0," [OK]");
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
	tlog(F110,"Changed directory to",vdir+1,0);
	return(1); 
    } else {
	tlog(F110,"Failed to change directory to",vdir+1,0);
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
