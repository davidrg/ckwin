/*  C K F N S 2  --  System-independent Kermit protocol support functions... */

/*  ...Part 2 (continued from ckfns.c)  */
/*
 Note -- if you change this file, please amend the version number and date at
 the top of ckfns.c accordingly.
*/

#include "ckermi.h"

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
	    sndpkt[i++] = dopar(tochar(( (unsigned)(j & 0170000)) >> 12));
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
    screen(type,(long)num,sndpkt);	/* Update screen */
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
    unsigned int c, q;
    LONG crc = 0;

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
    int w;

    for (w = 0; w < timint - 2; w++) {	/* be extra sure no stuff is */
	ttflui();			/*    still comming in */
	sleep(1);
	if (!ttchk() ) ttinc(1);	/* be extra sure no stuff in SIII/V */
	if (!ttchk() ) break;
    }
    ttol(sndpkt,spktl);
    screen('%',(long)pktnum,sndpkt);
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
    doexit(0);				/* Exit with status = 0 */
}

/* R P A C K  --  Read a Packet */

rpack(l,n,dat) int *l, *n; char *dat; {
    int i, j, x, done, pstart, pbl;
    char chk[4], xchk[4], t, type;

    chk[3] = xchk[3] = 0;
    i = inlin();			/* Read a line */
    if (i != 0) {
	debug(F101,"rpack: inlin","",i);
	screen('T',(long)pktnum,"");
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

   /***	if (t == 2) doexit(0); *** uncomment this to allow ^A^B cause exit ***/

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
		screen('Q',(long)n,recpkt);
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
		screen('Q',(long)n,recpkt);
		return('Q');
            }
	    break;
	case 3:
	    x = chk3(&recpkt[pstart]);
	    xchk[0] = tochar(( (unsigned)(x & 0170000)) >> 12);
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
		    screen('Q',(long)n,recpkt);
		    return('Q');
	    }
	    break;
        }

/* Good packet, return its type */

    ttflui();				/* Done, flush any remaining. */
    screen(type,(long)(*n),recpkt);	/* Update screen */
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
    return(0);
}
