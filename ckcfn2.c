/*  C K C F N 2  --  System-independent Kermit protocol support functions... */

/*  ...Part 2 (continued from ckcfns.c)  */

/*
 Author: Frank da Cruz (fdc@cunixc.cc.columbia.edu, FDCCU@CUVMA.BITNET),
 Columbia University Center for Computing Activities.
 First released January 1985.
 Copyright (C) 1985, 1989, Trustees of Columbia University in the City of New 
 York.  Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained. 
*/
/*
 Note -- if you change this file, please amend the version number and date at
 the top of ckcfns.c accordingly.
*/

#include "ckcsym.h"		/* Conditional compilation (for Macintosh) */
#include "ckcker.h"
#include "ckcdeb.h"

extern int spsiz, rpsiz, timint, npad, ebq, ebqflg, rpt, rptq, rptflg, capas;
extern int pktnum, prvpkt, sndtyp, bctr, bctu, rsn, rln, maxtry, size;
extern int osize, maxsize, spktl, nfils, stdouf, warn, timef, parity, speed;
extern int turn, turnch,  delay, displa, pktlog, tralog, seslog, xflg, mypadn;
extern int deblog, hcflg, binary, fncnv, local, server, cxseen, czseen;
extern long filcnt, ffc, flci, flco, tlci, tlco, tfc, fsize;
extern char *cmarg, *cmarg2, **cmlist;
extern CHAR padch, mypadc, eol, seol, ctlq, myctlq, sstate, *hlptxt;
extern CHAR filnam[], sndpkt[], recpkt[], data[], srvcmd[];
extern CHAR *srvptr, stchr, mystch, *rdatap;

char *strcpy();				/* Forward declarations */
unsigned int chk2();			/* of non-int functions */
unsigned int chk3();
CHAR dopar();				/* ... */

static CHAR partab[] = {		/* Even parity table for dopar() */

    '\000', '\201', '\202', '\003', '\204', '\005', '\006', '\207',
    '\210', '\011', '\012', '\213', '\014', '\215', '\216', '\017',
    '\220', '\021', '\022', '\223', '\024', '\225', '\226', '\027',
    '\030', '\231', '\232', '\033', '\234', '\035', '\036', '\237',
    '\240', '\041', '\042', '\243', '\044', '\245', '\246', '\047',
    '\050', '\251', '\252', '\053', '\254', '\055', '\056', '\257',
    '\060', '\261', '\262', '\063', '\264', '\065', '\066', '\267',
    '\270', '\071', '\072', '\273', '\074', '\275', '\276', '\077',
    '\300', '\101', '\102', '\303', '\104', '\305', '\306', '\107',
    '\110', '\311', '\312', '\113', '\314', '\115', '\116', '\317',
    '\120', '\321', '\322', '\123', '\324', '\125', '\126', '\327',
    '\330', '\131', '\132', '\333', '\134', '\335', '\336', '\137',
    '\140', '\341', '\342', '\143', '\344', '\145', '\146', '\347',
    '\350', '\151', '\152', '\353', '\154', '\355', '\356', '\157',
    '\360', '\161', '\162', '\363', '\164', '\365', '\366', '\167',
    '\170', '\371', '\372', '\173', '\374', '\175', '\176', '\377'
};

/* CRC generation tables */

static unsigned int crcta[16] = {0, 010201, 020402, 030603, 041004,
  051205, 061406, 071607, 0102010, 0112211, 0122412, 0132613, 0143014,
  0153215, 0163416, 0173617};

static unsigned int crctb[16] = {0, 010611, 021422, 031233, 043044,
  053655, 062466, 072277, 0106110, 0116701, 0127532, 0137323, 0145154,
  0155745, 0164576, 0174367};

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
    int type, numtry;

    if (sstate != 0) {			/* If a start state is in effect, */
	type = sstate;			/* return it like a packet type, */
	sstate = 0;			/* and then nullify it. */
	return(type);
    } else type = rpack();		/* Else, try to read a packet. */

debug(F111,"input",rdatap,type);

/* If it's the same packet we just sent, it's an echo.  Read another. */

    if (type == sndtyp) type = rpack();

    chkint();				/* Check for console interrupts. */
/*
 If previous packet again, a timeout pseudopacket, or a bad packet, try again.
*/
    for (numtry = 0;
      (rsn == prvpkt || type == 'T' || type == 'Q' || type == 'N');
      numtry++) {
	if (numtry > maxtry) {		/* If too many tries, give up */
	    strcpy(data,"Timed out.");	/* and send a timeout error packet, */
	    rdatap = data;		/* and pretend we read one. */
	    return('E');
	}
	if (type == 'E') return('E');	/* Don't even bother about seq no */
	if ((type == 'N') && (rsn == ((pktnum+1) & 63))) {
					/* NAK for next packet */
	    return('Y');		/* is ACK for current. */
	} else {    
	    resend();			/* Else, send last packet again, */
	}
	if (sstate != 0) {		/* If an interrupt routine has set */
	    type = sstate;		/* sstate behind our back, return */
	    sstate = 0;			/* that. */
	    *data = '\0';
	    return(type);
	} else type = rpack();		/* Else try to read a packet. */
	chkint();			/* Look again for interruptions. */
	if (type == sndtyp) type = rpack();
    }
    ttflui();			/* Got what we want, clear input buffer. */
    return(type);		/* Success, return packet type. */
}

/*  S P A C K  --  Construct and send a packet  */

/*
 spack() sends a packet of the given type, sequence number n, with len
 data characters pointed to by d, in either a regular or extended-
 length packet, depending on length.  Returns the number of bytes
 actually sent, or else -1 upon failure.  Uses global npad, padch,
 mystch, bctu.  Leaves packet in null-terminated global sndpkt[] array for
 later retransmission.  Updates global sndpktl (send-packet length).
*/

spack(type,n,len,d) char type, *d; int n, len; {
    int i, j, lp; CHAR *sohp = sndpkt; CHAR pc;
    unsigned crc;

    spktl = 0;
    pc = dopar(padch);			/* The pad character, if any. */
    for (i = 0; i < npad; sndpkt[i++] = pc) /* Do any requested padding */
      sohp++;
    sndpkt[i++] = dopar(mystch);	/* MARK */
    lp = i++;				/* Position of LEN, fill in later */
    sndpkt[i++] = dopar(tochar(n));	/* SEQ field */
    sndpkt[i++] = dopar(sndtyp = type);	/* TYPE field */
    j = len + bctu;			/* Length of data + block check */
    if (j+2 > MAXPACK) {		/* Long packet? */
        sndpkt[lp] = dopar(tochar(0));	/* Yes, set LEN to zero */
        sndpkt[i++] = dopar(tochar(j / 95)); /* High part */
        sndpkt[i++] = dopar(tochar(j % 95)); /* Low part */
        sndpkt[i] = '\0';		/* Header checksum */
        sndpkt[i++] = dopar(tochar(chk1(sndpkt+lp)));
    } else sndpkt[lp] = dopar(tochar(j+2)); /* Normal LEN */

    while (len-- > 0) sndpkt[i++] = dopar(*d++); /* Packet data */
    sndpkt[i] = '\0';			/* Null-terminate */

    switch (bctu) {			/* Block check */
	case 1:				/* 1 = 6-bit chksum */
	    sndpkt[i++] = dopar(tochar(chk1(sndpkt+lp)));
	    break;
	case 2:				/* 2 = 12-bit chksum */
	    j = chk2(sndpkt+lp);
	    sndpkt[i++] = dopar((unsigned)tochar((j >> 6) & 077));
	    sndpkt[i++] = dopar((unsigned)tochar(j & 077));
	    break;
        case 3:				/* 3 = 16-bit CRC */
	    crc = chk3(sndpkt+lp);
	    sndpkt[i++] = dopar((unsigned)tochar(((crc & 0170000)) >> 12));
	    sndpkt[i++] = dopar((unsigned)tochar((crc >> 6) & 077));
	    sndpkt[i++] = dopar((unsigned)tochar(crc & 077));
	    break;
    }
    sndpkt[i++] = dopar(seol);		/* End of line (packet terminator) */
    sndpkt[i] = '\0';			/* Terminate string */
    if (ttol(sndpkt,i) < 0) return(-1);	/* Send the packet */
    spktl = i;				/* Remember packet length */
    flco += spktl;			/* Count the characters */
    tlco += spktl;
    if (pktlog) {			/* If logging packets, log it */
	zsout(ZPFILE,"s-");
	if (*sndpkt) zsoutl(ZPFILE,sndpkt); else zsoutl(ZPFILE,sohp);
    }	
    screen(SCR_PT,type,(long)n,sohp);	/* Update screen */
    return(i);				/* Return length */
}

/*  D O P A R  --  Add an appropriate parity bit to a character  */

CHAR
dopar(ch) CHAR ch; {
    unsigned int a;
    if (!parity) return(ch & 255); else a = ch & 127;
    switch (parity) {
	case 'e':  return(partab[a]);	    /* Even */
	case 'm':  return(a | 128);         /* Mark */
	case 'o':  return(partab[a] ^ 128); /* Odd */
	case 's':  return(a);		    /* Space */
	default:   return(a);
    }
}

/*  C H K 1  --  Compute a type-1 Kermit 6-bit checksum.  */

chk1(pkt) char *pkt; {
    unsigned int chk;
    chk = chk2(pkt);
    chk = (((chk & 0300) >> 6) + chk) & 077;
    return(chk);
}

/*  C H K 2  --  Compute the numeric sum of all the bytes in the packet.  */

unsigned int
chk2(pkt) CHAR *pkt; {
    long chk; unsigned int m;
    m = (parity) ? 0177 : 0377;
    for (chk = 0; *pkt != '\0'; pkt++)
      chk += *pkt & m;
    return(chk & 07777);
}


/*  C H K 3  --  Compute a type-3 Kermit block check.  */
/*
 Calculate the 16-bit CRC-CCITT of a null-terminated string using a lookup 
 table.  Assumes the argument string contains no embedded nulls.
*/
unsigned int
chk3(pkt) CHAR *pkt; {
    LONG c, crc;
    unsigned int m;
    m = (parity) ? 0177 : 0377;
    for (crc = 0; *pkt != '\0'; pkt++) {
	c = (*pkt & m) ^ crc;
	crc = (crc >> 8) ^ (crcta[(c & 0xF0) >> 4] ^ crctb[c & 0x0F]);
    }
    return(crc & 0xFFFF);
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
    if (spktl)				/* If buffer has something, */
    	ttol(sndpkt,spktl);		/* resend it, */
    else nack();			/* otherwise send a NAK. */
    
    debug(F111,"resend",sndpkt,spktl);
    screen(SCR_PT,'%',(long)pktnum,"(resend)");	/* Say resend occurred */
    if (pktlog) {
	zsout(ZPFILE,"s-");
	zsoutl(ZPFILE,"(resend)"); /* Log packet if desired */
    }
}

errpkt(reason) char *reason; {		/* Send an error packet. */
    encstr(reason);
    spack('E',pktnum,size,data);
    clsif(); clsof(1);
    screen(SCR_TC,0,0l,"");
}

scmd(t,dat) char t, *dat; {		/* Send a packet of the given type */
    encstr(dat);			/* Encode the command string */
    spack(t,pktnum,size,data);
}

srinit() {				/* Send R (GET) packet */
    encstr(cmarg);			/* Encode the filename. */
    spack('R',pktnum,size,data);	/* Send the packet. */
}

nxtpkt(num) int *num; {
    prvpkt = *num;			/* Save previous */
    *num = (*num + 1) % 64;		/* Increment packet number mod 64 */
}

sigint() {				/* Terminal interrupt handler */
    errpkt("User typed ^C");
    doexit(GOOD_EXIT);			/* Exit program */
}

/* R P A C K  --  Read a Packet */

/*
 rpack reads a packet and returns the packet type, or else Q if the
 packet was invalid, or T if a timeout occurred.  Upon successful return, sets
 the values of global rsn (received sequence number),  rln (received
 data length), and rdatap (pointer to null-terminated data field).
*/
rpack() {
    int i, j, x, try, type, lp;		/* Local variables */
    unsigned crc;
    CHAR pbc[4];			/* Packet block check */
    CHAR *sohp = recpkt;		/* Pointer to SOH */
    CHAR e;				/* Packet end character */

    rsn = rln = -1;			/* In case of failure. */
    *recpkt = '\0';			/* Clear receive buffer. */
    rdatap = recpkt;			/* Initialize this. */
    
    e = (turn) ? turnch : eol;		/* Use any handshake char for eol */

/* Try several times to get a "line".  This allows for hosts that echo our */
/* normal CR packet terminator as CRLF.  Don't diagnose CRLF as an */
/* invalid packet. */

#define TTITRY 3

    for (try = 0; try < TTITRY; try++) { /* Try x times to get a "line". */
	j = ttinl(recpkt,MAXRP,timint,e); 
	if (j < 0) {
	    if (j < -1) doexit(BAD_EXIT); /* Bail out if ^C^C typed. */
	    debug(F101,"rpack: ttinl fails","",j);
	    screen(SCR_PT,'T',(long)pktnum,"");
	    return('T');		/* Otherwise, call it a timeout. */
	}
	tlci += j;			/* All OK, Count the characters. */
	flci += j;

	for (i = 0; (recpkt[i] != stchr) && (i < j); i++)
	  sohp++;			/* Find mark */
	if (i++ < j) break;		/* Found it. */
    }
    if (try >= TTITRY) return('Q');	/* Diagnose bad packet. */

    debug(F111,"ttinl",sohp,j);		/* Log packet if requested. */
    if (pktlog) {
	zsout(ZPFILE,"r-");
	zsoutl(ZPFILE,sohp);
    }
    lp = i;				/* Remember LEN position. */
    if ((j = xunchar(recpkt[i++])) == 0) {
        if ((j = lp+5) > MAXRP) return('Q'); /* Long packet */
	x = recpkt[j];			/* Header checksum. */
	recpkt[j] = '\0';		/* Calculate & compare. */
	if (xunchar(x) != chk1(recpkt+lp)) return('Q');
	recpkt[j] = x;			/* Checksum ok. */
	rln = xunchar(recpkt[j-2]) * 95 + xunchar(recpkt[j-1]) - bctu;
	j = 3;				/* Data offset. */
    } else if (j < 3) {
	debug(F101,"rpack packet length less than 3","",j);
	return('Q');
    } else {
	rln = j - bctu - 2;		/* Regular packet */
	j = 0;				/* No extended header */
    }
    rsn = xunchar(recpkt[i++]);		/* Sequence number */
    type = recpkt[i++];			/* Packet type */
    i += j;				/* Where data begins */
    rdatap = recpkt+i;			/* The data itself */
    if ((j = rln + i) > MAXRP ) {
	debug(F101,"packet sticks out too far","",j);
	return('Q'); /* Find block check */
    }
/** debug(F101,"block check at","",j); **/
    for (x = 0; x < bctu; x++)		/* Copy it */
      pbc[x] = recpkt[j+x];

    pbc[x] = '\0';
/** debug(F110,"block check",pbc,bctu); **/
    recpkt[j] = '\0';			/* Null-terminate data */

    switch (bctu) {			/* Check the block check */
	case 1:
	    if (xunchar(*pbc) != chk1(recpkt+lp)) {
		debug(F110,"checked chars",recpkt+lp,0);
	        debug(F101,"block check","",xunchar(*pbc));
		debug(F101,"should be","",chk1(recpkt+lp));
		return('Q');
 	    }
	    break;
	case 2:
	    x = xunchar(*pbc) << 6 | xunchar(pbc[1]);
	    if (x != chk2(recpkt+lp)) {
		debug(F110,"checked chars",recpkt+lp,0);
	        debug(F101,"block check","", x);
		debug(F101,"should be","", chk2(recpkt+lp));
		return('Q');
	    }
	    break;
	case 3:
	    crc = (xunchar(pbc[0]) << 12)
	        | (xunchar(pbc[1]) << 6)
		| (xunchar(pbc[2]));
	    if (crc != chk3(recpkt+lp)) {
		debug(F110,"checked chars",recpkt+lp,0);
	        debug(F101,"block check","",xunchar(*pbc));
		debug(F101,"should be","",chk3(recpkt+lp));
		return('Q');
	    }
	    break;
	default: return('Q');
    }
    screen(SCR_PT,type,(long)rsn,sohp);	/* Update screen */
    return(type);			/* Return packet type */
}
