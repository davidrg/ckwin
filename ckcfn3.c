/*  C K C F N 3  --  Packet buffer management for C-Kermit  */

/* (plus assorted functions tacked on at the end) */

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

#include "ckcsym.h"
#include "ckcdeb.h"
#include "ckcasc.h"
#include "ckcker.h"
#include "ckcxla.h"

#ifndef NODISPO
#ifdef pdp11
#define NODISPO
#endif /* pdpd11 */
#endif /* NODISPO */

extern int unkcs, wmax, wcur, discard, bctu, bctl, local, fdispla, what,
  sendmode, opnerr;
extern long sendstart;

extern char * ofn2;
extern char ofn1[];
extern int ofn1x;

#ifdef CK_LABELED
extern int xflg;
#endif /* CK_LABELED */
extern CHAR *data;
extern char filnam[];
#ifndef NOFRILLS
extern int rprintf, rmailf;		/* REMOTE MAIL, PRINT */
extern char optbuf[];			/* Options buffer for mail, print */
#endif /* NOFRILLS */
extern int wslots;
extern int fblksiz, frecl, forg, frecfm, fncact, fncsav, fcctrl, lf_opts;
#ifdef pdp11
  extern CHAR srvcmd[];
  extern char tmpbuf[];
  CHAR *pktmsg = (CHAR *) tmpbuf;
#else
#ifdef DYNAMIC
  extern CHAR *srvcmd;
  extern CHAR *pktmsg;
#else
  extern CHAR srvcmd[];
  extern CHAR pktmsg[];
#endif /* DYNAMIC */
#endif /* pdp11 */

extern int binary, spsiz;
extern int pktnum, cxseen, czseen, bsave, bsavef, nfils, stdinf;
extern int memstr, stdouf, keep, sndsrc, hcflg;
extern int server, en_cwd;

extern int
  atenci, atenco, atdati, atdato, atleni, atleno, atblki, atblko,
  attypi, attypo, atsidi, atsido, atsysi, atsyso, atdisi, atdiso; 

#ifdef STRATUS
extern int atfrmi, atfrmo, atcrei, atcreo, atacti, atacto;
#endif /* STRATUS */

#ifdef datageneral
extern int quiet;
#endif /* datageneral */

extern long fsize, filcnt, ffc, tfc;

#ifndef NOCSETS
extern int tcharset, fcharset;
extern int ntcsets;
extern struct csinfo tcsinfo[], fcsinfo[];

/* Pointers to translation functions */
#ifdef CK_ANSIC
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* Character set */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])(CHAR); /* translation functions */
extern CHAR (*rx)(CHAR); /* Pointer to input character translation function */
extern CHAR (*sx)(CHAR); /* Pointer to output character translation function */
#else
extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])();	/* Character set */
extern CHAR (*xlr[MAXTCSETS+1][MAXFCSETS+1])();	/* translation functions. */
extern CHAR (*rx)();	/* Pointer to input character translation function */
extern CHAR (*sx)();    /* Pointer to output character translation function */
#endif /* CK_ANSIC */
#endif /* NOCSETS */

/* Variables global to Kermit that are defined in this module */

int winlo;				/* packet number at low window edge  */

int sbufnum;				/* number of free buffers */
int dum001 = 1234;			/* protection... */
int sbufuse[MAXWS];			/* buffer in-use flag */
int dum003 = 1111;
int rbufnum;				/* number of free buffers */
int dum002 = 4321;			/* more protection */
int rbufuse[MAXWS];			/* buffer in-use flag */
int sseqtbl[64];			/* sequence # to buffer # table */
int rseqtbl[64];			/* sequence # to buffer # table */
int sacktbl[64];			/* sequence # ack table */

#ifdef DYNAMIC
struct pktinfo *s_pkt = NULL;		/* array of pktinfo structures */
struct pktinfo *r_pkt = NULL;		/* array of pktinfo structures */
#else
struct pktinfo s_pkt[MAXWS];		/* array of pktinfo structures */
struct pktinfo r_pkt[MAXWS];		/* array of pktinfo structures */
#endif /* DYNAMIC */

#ifdef DEBUG
char xbuf[200];				/* For debug logging */
#endif /* DEBUG */

#ifdef DYNAMIC
CHAR *bigsbuf = NULL, *bigrbuf = NULL;
#else
char bigsbt[8];				/* Protection (shouldn't need this). */
					/* BUT DON'T REMOVE IT! */
CHAR bigsbuf[SBSIZ + 5];		/* Send-packet buffer area */
char bigrbt[8];				/* Safety padding */
CHAR bigrbuf[RBSIZ + 5];		/* Receive-packet area */
#endif
int bigsbsiz = SBSIZ;			/* Sizes of big send & rcv buffers. */
int bigrbsiz = RBSIZ;

/* FUNCTIONS */

/* For sanity, use "i" for buffer slots, "n" for packet numbers. */

/* I N I B U F S */

/*
  Allocates the big send and receive buffers.
  Call with size for big send buffer (s) and receive buffer (r).
  These sizes can be different.
  Attempts to allocate buffers of the requested size, but if it can't,
  it will allocate smaller ones.
  Sets global variables bigsbsiz and bigrbsiz to the actual sizes,
  and bigsbuf and bigrbuf pointing to the actual buffers.
  Designed to be called more than once.
  Returns 0 on success, -1 on failure.
*/

CHAR *bigbufp = NULL;

int
inibufs(s,r) int s, r; {
#ifdef DYNAMIC
    unsigned
      int size;
#ifdef OS2
    unsigned		/* Don't you wish everybody had unsigned long... */
#endif /* OS2 */  
      long z;
    int x;

    debug(F101,"inibufs s","",s);
    debug(F101,"inibufs r","",r);

    if (s < 80 || r < 80) return(-1);	/* Validate arguments. */

    if (!s_pkt) {			/* Allocate packet info structures */
	if (!(s_pkt = (struct pktinfo *) malloc(sizeof(struct pktinfo)*MAXWS)))
	  fatal("ini_pkts: no memory for s_pkt");
    }
    for (x = 0; x < MAXWS; x++)
      s_pkt[x].pk_adr = NULL;		/* Initialize addresses */

    if (!r_pkt) {
	if (!(r_pkt = (struct pktinfo *) malloc(sizeof(struct pktinfo)*MAXWS)))
	  fatal("ini_pkts: no memory for s_pkt");
    }
    for (x = 0; x < MAXWS; x++)
      r_pkt[x].pk_adr = NULL;		/* Initialize addresses */

    if (!srvcmd) {			/* Allocate srvcmd buffer */
	srvcmd = (CHAR *) malloc(r + 100);
	if (!srvcmd) return(-1);
	*srvcmd = NUL;
    }
    if (!pktmsg) {			/* Allocate pktmsg buffer */
	pktmsg = (CHAR *) malloc(81);
	if (!pktmsg) return(-1);
	*pktmsg = NUL;
    }

    if (bigbufp) {			/* Free previous buffers, if any. */
	free(bigbufp);
	bigbufp = NULL;
    }
    size = s + r + 40;			/* Combined requested size + padding */
    z  = (unsigned) s + (unsigned) r + 40;
    debug(F101,"inibufs size 1","",size);
    debug(F101,"inibufs size z","",z);
    if ((long) size != z) {
	debug(F100,"inibufs overflow","",0);
	size = 65535;
    }	

    /* Try to get the space.  If malloc fails, try to get a little less. */
    /* (Obviously, this algorithm can be refined.) */

    while (!(bigbufp = (CHAR *) malloc(size))) {
	debug(F101,"inibufs bigbuf malloc failed","",size);
	size = (size * 2) / 3;		/* Failed, cut size by 1/3. */
	if (size < 200)			/* Try again until too small. */
	  return(-1);
    }
    debug(F101,"inibufs size 2","",size); /* OK, we got some space. */

/*
  Now divide the allocated space between the send and receive buffers in the
  requested proportion.  The natural formula would be (s / (s + r)) * size
  (for the send buffer), but that doesn't work with integer arithmetic and we
  can't use floating point because some machines don't have it.  This can be
  rearranged as (s * size) / (s + r).  But (s * size) can be VERY large, too
  large for 32 bits.  So let's do it this way.  This arithmetic works for
  buffer sizes up to about 5,000,000.
*/
#define FACTOR 20L
    z = ( (long) s * FACTOR ) / ( (long) s + (long) r );
    x = ( z * ( (long) size / FACTOR ) );
    if (x < 0) return(-1);		/* Catch overflow */

    bigsbsiz = x - 5;			/* Size of send buffer */
    bigsbuf = bigbufp;			/* Address of send buffer */
    debug(F101,"inibufs bigsbsiz","",bigsbsiz);

    bigrbsiz = size - x - 5;		/* Size of receive buffer */
    bigrbuf = bigbufp + x;		/* Addresss of receive buffer */
    debug(F101,"inibufs bigrbsiz","",bigrbsiz);

    return(0);				/* Success */
#else					/* No dynamic allocation */
    bigsbsiz = SBSIZ;			/* Just use the symbols */
    bigrbsiz = RBSIZ;			/* ... */
    return(0);				/* Success. */
#endif /* DYNAMIC */
}


/* M A K E B U F  --  Makes and clears a new buffers.  */

/* Call with: */
/*  slots:  number of buffer slots to make, 1 to 32 */
/*  bufsiz: size of the big buffer */
/*  buf:    address of the big buffer */
/*  xx:     pointer to array of pktinfo structures for these buffers */

/* Subdivides the big buffer into "slots" buffers. */

/* Returns: */
/*  -1 if too many or too few slots requested,     */
/*  -2 if slots would be too small.      */
/*   n (positive) on success = size of one buffer. */
/*   with pktinfo structure initialized for this set of buffers. */

int
makebuf(slots,bufsiz,buf,xx)
/* makebuf */ int slots, bufsiz; CHAR buf[]; struct pktinfo *xx; {

    CHAR *a;
    int i, size;

    debug(F101,"makebuf","",slots);
    debug(F101,"makebuf bufsiz","",bufsiz);
    debug(F101,"makebuf MAXWS","",MAXWS);

    if (slots > MAXWS || slots < 1) return(-1);
    if (bufsiz < slots * 10 ) return(-2);

    size = bufsiz / slots;		/* Divide up the big buffer. */
    a = buf;				/* Address of first piece. */

    for (i = 0; i < slots; i++) {
	struct pktinfo *x = &xx[i];
	x->bf_adr = a;			/* Address of this buffer */
	x->bf_len = size;		/* Length of this buffer */
	x->pk_len = 0;			/* Length of data field */
        x->pk_typ = ' ';		/* packet type */
	x->pk_seq = -1;			/* packet sequence number */
        x->pk_flg = 0;			/* ack'd bit */
        x->pk_rtr = 0;			/* retransmissions */
	*a = '\0';			/* Clear the buffer */
	a += size;			/* Position to next buffer slot */
    }
    return(size);
}

/*  M A K S B U F  --  Makes the send-packet buffer  */

int
mksbuf(slots) int slots; {
    int i, x;
    sbufnum = 0;
    if ((x = makebuf(slots,bigsbsiz,bigsbuf,s_pkt)) < 0) {
	debug(F101,"mksbuf makebuf return","",x);
	return(x);
    }
    debug(F101,"mksbuf makebuf return","",x);
    for (i = 0; i < 64; i++) {		/* Initialize sequence-number- */
	sseqtbl[i] = -1;		/* to-buffer-number table. */
        sacktbl[i] = 0;
    }
    for (i = 0; i < MAXWS; i++)
      sbufuse[i] = 0;			/* Mark each buffer as free */
    sbufnum = slots;
    wcur = 0;
    return(x);
}

/*  M A K R B U F  --  Makes the receive-packet buffer  */

int
mkrbuf(slots) int slots; {
    int i, x;
    rbufnum = 0;
    if ((x = makebuf(slots,bigrbsiz,bigrbuf,r_pkt)) < 0) {
	debug(F101,"mkrbuf makebuf return","",x);
	return(x);
    }
    debug(F101,"mkrbuf makebuf return","",x);
    for (i = 0; i < 64; i++) {		/* Initialize sequence-number- */
	rseqtbl[i] = -1;		/* to-buffer-number table. */
    }
    for (i = 0; i < MAXWS; i++)
      rbufuse[i] = 0;			/* Mark each buffer as free */
    rbufnum = slots;
    wcur = 0;
    return(x);
}

/*  W I N D O W  --  Resize the window to n  */

int
window(n) int n; {
    debug(F101,"window","",n);
    if (n < 1 || n > MAXWS) return(-1);
    if (mksbuf(n) < 0) return(-1);
    if (mkrbuf(n) < 0) return(-1);
    wslots = n;
#ifdef DEBUG
    if (deblog) dumpsbuf();
    if (deblog) dumprbuf();
#endif /* DEBUG */
    return(0);
}

/*  G E T S B U F  --  Allocate a send-buffer.  */

/*  Call with packet sequence number to allocate buffer for. */
/*  Returns: */
/*   -4 if argument is invalid (negative, or greater than 63) */
/*   -3 if buffers were thought to be available but really weren't (bug!) */
/*   -2 if the number of free buffers is negative (bug!) */
/*   -1 if no free buffers. */
/*   0 or positive, packet sequence number, with buffer allocated for it. */

int
getsbuf(n) int n; {			/* Allocate a send-buffer */
    int i;
    if (n < 0 || n > 63) {
	debug(F101,"getsbuf bad arg","",n);
	return(-4);	/* Bad argument */
    }
    debug(F101,"getsbuf, packet","",n);
    debug(F101,"getsbuf, sbufnum","",sbufnum);
    if (sbufnum == 0) return(-1);	/* No free buffers. */
    if (sbufnum < 0) return(-2);	/* Shouldn't happen. */
    for (i = 0; i < wslots; i++)	/* Find the first one not in use. */
      if (sbufuse[i] == 0) {		/* Got one? */
	  sbufuse[i] = 1;		/* Mark it as in use. */
	  sbufnum--;			/* One less free buffer. */
	  *s_pkt[i].bf_adr = '\0';	/* Zero the buffer data field */
	  s_pkt[i].pk_seq = n;		/* Put in the sequence number */
          sseqtbl[n] = i;		/* Back pointer from sequence number */
          sacktbl[n] = 0;		/* ACK flag */
	  s_pkt[i].pk_len = 0;		/* Data field length now zero. */
	  s_pkt[i].pk_typ = ' ';	/* Blank the packet type too. */
	  s_pkt[i].pk_flg = 0;		/* Zero the flags */
	  s_pkt[i].pk_rtr = 0;		/* Zero the retransmission count */
	  data = s_pkt[i].bf_adr + 7;	/* Set global "data" address. */
	  if ((what & (W_SEND|W_REMO)) && (++wcur > wmax))
	    wmax = wcur;		/* For statistics. */
	  return(n);			/* Return its index. */
      }
    sbufnum = 0;			/* Didn't find one. */
    return(-3);				/* Shouldn't happen! */
}

int
getrbuf() {				/* Allocate a receive buffer */
    int i;
    debug(F101,"getrbuf rbufnum","",rbufnum);
    debug(F101,"getrbuf wslots","",wslots);
    debug(F101,"getrbuf dum002","",dum002);
    debug(F101,"getrbuf dum003","",dum003);
    if (rbufnum == 0) return(-1);	/* No free buffers. */
    if (rbufnum < 0) return(-2);	/* Shouldn't happen. */
    for (i = 0; i < wslots; i++)	/* Find the first one not in use. */
      if (rbufuse[i] == 0) {		/* Got one? */
	  rbufuse[i] = 1;		/* Mark it as in use. */
	  *r_pkt[i].bf_adr = '\0';	/* Zero the buffer data field */
	  rbufnum--;			/* One less free buffer. */
	  debug(F101,"getrbuf new rbufnum","",rbufnum);
	  if ((what & W_RECV) && (++wcur > wmax))
	    wmax = wcur;		/* For statistics. */
	  return(i);			/* Return its index. */
      }
    debug(F101,"getrbuf foulup","",i);
    rbufnum = 0;			/* Didn't find one. */
    return(-3);				/* Shouldn't happen! */
}

/*  F R E E S B U F  --  Free send-buffer for given packet sequence number */

/*  Returns:  */
/*   1 upon success  */
/*  -1 if specified buffer does not exist */

int
freesbuf(n) int n; {			/* Release send-buffer for packet n. */
    int i;

    debug(F101,"freesbuf","",n);
    if (n < 0 || n > 63)		/* No such packet. */
      return(-1);
    i = sseqtbl[n];			/* Get the window slot number. */
    if (i > -1 && i <= wslots) {
	sseqtbl[n] = -1;		/* If valid, remove from seqtbl */
 	sbufnum++;			/* and count one more free buffer */
	sbufuse[i] = 0;			/* and mark it as free, */
	if (what & (W_SEND|W_REMO))	/* decrement active slots */
	  wcur--;			/* for statistics and display. */
    } else {
	debug(F101," sseqtbl[n]","",sseqtbl[n]);
	return(-1);
    }

/* The following is done only so dumped buffers will look right. */

    if (1) {
	*s_pkt[i].bf_adr = '\0';	/* Zero the buffer data field */
	s_pkt[i].pk_seq = -1;		/* Invalidate the sequence number */
	s_pkt[i].pk_len = 0;		/* Data field length now zero. */
	s_pkt[i].pk_typ = ' ';		/* Blank the packet type too. */
	s_pkt[i].pk_flg = 0;		/* And zero the flag */
	s_pkt[i].pk_rtr = 0;		/* And the retries field. */
    }
    return(1);
}

int
freerbuf(i) int i; {			/* Release receive-buffer slot "i". */
    int n;

/* NOTE !! Currently, this function frees the indicated buffer, but */
/* does NOT erase the data.  The program counts on this.  Will find a */
/* better way later.... */

    debug(F101,"freerbuf, slot","",i);
    if (i < 0 || i >= wslots) {		/* No such slot. */
	debug(F101,"freerbuf no such slot","",i);
	return(-1);
    }
    n = r_pkt[i].pk_seq;		/* Get the packet sequence number */
    debug(F101,"freerbuf, packet","",n);
    if (n > -1 && n < 64)		/* If valid, remove from seqtbl */
      rseqtbl[n] = -1;
    if (rbufuse[i] != 0) {		/* If really allocated, */
	rbufuse[i] = 0;			/* mark it as free, */
	rbufnum++;			/* and count one more free buffer. */
	if (what & W_RECV)		/* Keep track of current slots */
	  wcur--;			/*  for statistics and display */
	debug(F101,"freerbuf, new rbufnum","",rbufnum);
    }

/* The following is done only so dumped buffers will look right. */

    if (1) {
     /* *r_pkt[i].bf_adr = '\0'; */	/* Zero the buffer data field */
	r_pkt[i].pk_seq = -1;		/* And from packet list */
	r_pkt[i].pk_len = 0;		/* Data field length now zero. */
	r_pkt[i].pk_typ = ' ';		/* Blank the packet type too. */
	r_pkt[i].pk_flg = 0;		/* And zero the flag */
	r_pkt[i].pk_rtr = 0;		/* And the retries field. */
    }
    return(1);
}

/* This is like freerbuf, except it's called with a packet sequence number */
/* rather than a packet buffer index. */

VOID
freerpkt(seq) int seq; {
    int k;
    debug(F101,"freerpkt seq","",seq);
    k = rseqtbl[seq];
    debug(F101,"freerpkt k","",k);
    if (k > -1) {
	k = freerbuf(k);
	debug(F101,"freerpkt freerbuf","",k);
    }
}


/*  C H K W I N  --  Check if packet n is in window. */

/*  Returns: */
/*    0 if it is in the current window,  */
/*   +1 if it would have been in previous window (e.g. if ack was lost), */
/*   -1 if it is outside any window (protocol error),   */
/*   -2 if either of the argument packet numbers is out of range.  */

/* Call with packet number to check (n), lowest packet number in window */
/* (bottom), and number of slots in window (slots).  */

int
chkwin(n,bottom,slots) int n, bottom, slots; {
    int top, prev;

    debug(F101,"chkwin packet","",n);
    debug(F101,"chkwin winlo","",bottom);
    debug(F101,"chkwin slots","",slots);

/* First do the easy and common cases, where the windows are not split. */

    if (n < 0 || n > 63 || bottom < 0 || bottom > 63)
      return(-2);

    if (n == bottom) return(0);		/* In a perfect world... */

    top = bottom + slots;		/* Calculate window top. */
    if (top < 64 && n < top && n >= bottom)
      return(0);			/* In current window. */

    prev = bottom - slots;		/* Bottom of previous window. */
    if (prev > -1 && n < bottom && n > prev)
      return(1);			/* In previous. */

/* Now consider the case where the current window is split. */

    if (top > 63) {			/* Wraparound... */
	top -= 64;			/* Get modulo-64 sequence number */
	if (n < top || n >= bottom) {
	    return(0);			/* In current window. */
	} else {			/* Not in current window. */
	    if (n < bottom && n >= prev) /* Previous window can't be split. */
	      return(1);		/* In previous window. */
	    else
	      return(-1);		/* Not in previous window. */
	}
    }

/* Now the case where current window not split, but previous window is. */ 

    if (prev < 0) {			/* Is previous window split? */
	prev += 64;			/* Yes. */
	if (n < bottom || n >= prev)
	  return(1);			/* In previous window. */
    } else {				/* Previous window not split. */
	if (n < bottom && n >= prev)
	  return(1);			/* In previous window. */
    }
    
/* It's not in the current window, and not in the previous window... */

    return(-1);				/* So it's not in any window. */
}

int
dumpsbuf() {				/* Dump send-buffers */
#ifdef DEBUG
    int j, x;				/* to debug log. */

    if (! deblog) return(0);
    x = zsoutl(ZDFILE,"SEND BUFFERS:");
    if (x < 0) {
	deblog = 0;
	return(0);
    }
    x=zsoutl(ZDFILE,"buffer inuse address length data type seq flag retries");
    if (x < 0) {
	deblog = 0;
	return(0);
    }
    for ( j = 0; j < wslots; j++ ) {
	sprintf(xbuf,"%4d%6d%10d%5d%6d%4c%5d%5d%6d\n",
	       j,
	       sbufuse[j],
	       s_pkt[j].bf_adr, 
	       s_pkt[j].bf_len,
	       s_pkt[j].pk_len,
	       s_pkt[j].pk_typ,
	       s_pkt[j].pk_seq,
	       s_pkt[j].pk_flg,
	       s_pkt[j].pk_rtr
	       );
	if (zsout(ZDFILE,xbuf) < 0)  {
	    deblog = 0;
	    return(0);
	}
	if (s_pkt[j].pk_adr) {
	    x = (int)strlen((char *) s_pkt[j].pk_adr);
	    if (x)
	      sprintf(xbuf,"[%.72s%s]\n",s_pkt[j].pk_adr, x > 72 ? "..." : "");
	    else
	      sprintf(xbuf,"[(empty string)]\n");
	} else {
	    sprintf(xbuf,"[(null pointer)]\n");
	}
	if (zsout(ZDFILE,xbuf) < 0) {
	    deblog = 0;
	    return(0);
	}
    }
    sprintf(xbuf,"free: %d, winlo: %d\n", sbufnum, winlo);
    if (zsout(ZDFILE,xbuf) < 0) {
	deblog = 0;
	return(0);
    }
#endif /* DEBUG */
    return(0);
}
int
dumprbuf() {				/* Dump receive-buffers */
#ifdef DEBUG
    int j, x;
    if (! deblog) return(0);
    if (zsoutl(ZDFILE,"RECEIVE BUFFERS:") < 0) {
	deblog = 0;
	return(0);
    }
    x=zsoutl(ZDFILE,"buffer inuse address length data type seq flag retries");
    if (x < 0) {
	deblog = 0;
	return(0);
    }
    for ( j = 0; j < wslots; j++ ) {
	sprintf(xbuf,"%4d%6d%10d%5d%6d%4c%5d%5d%6d\n",
	       j,
	       rbufuse[j],
	       r_pkt[j].bf_adr, 
	       r_pkt[j].bf_len,
	       r_pkt[j].pk_len,
	       r_pkt[j].pk_typ,
	       r_pkt[j].pk_seq,
	       r_pkt[j].pk_flg,
	       r_pkt[j].pk_rtr
	       );
	if (zsout(ZDFILE,xbuf) < 0) {
	    deblog = 0;
	    return(0);
	}
	x = (int)strlen((char *)r_pkt[j].bf_adr);
	sprintf(xbuf,"[%.72s%s]\n",r_pkt[j].bf_adr, x > 72 ? "..." : "");
	if (zsout(ZDFILE,xbuf) < 0)  {
	    deblog = 0;
	    return(0);
	}
    }
    sprintf(xbuf,"free: %d, winlo: %d\n", rbufnum, winlo);
    if (zsout(ZDFILE,xbuf) < 0)  {
	deblog = 0;
	return(0);
    }
#endif /* DEBUG */
    return(0);
}

/*** Some misc functions also moved here from the other ckcfn*.c modules ***/
/*** to even out the module sizes. ***/

/* Attribute Packets. */

/*
  Call with xp == 0 if we're sending a real file (F packet),
  or xp != 0 for screen data (X packet).
  Returns 0 on success, -1 on failure.
*/
int
sattr(xp) int xp; {			/* Send Attributes */
    int i, j, aln;
    char *tp;
    struct zattr x;

    if (zsattr(&x) < 0) return(-1);	/* Get attributes or die trying */
    if (nxtpkt() < 0) return(-1);	/* Next packet number */
    i = 0;				/* i = Data field character number */
    if (atsido) {			/* System type */
	data[i++] = '.';
	data[i++] = tochar(x.systemid.len); /*  Copy from attr structure */
	for (j = 0; j < x.systemid.len; j++)
	  data[i++] = x.systemid.val[j];
    }
#ifdef STRATUS
    if (atcreo) {			/* File creator */
	data[i++] = '$';
	data[i++] = tochar(x.creator.len); /*  Copy from attr structure */
	for (j = 0; j < x.creator.len; j++)
	  data[i++] = x.creator.val[j];
    }
    if (atacto) {			/* File account */
	data[i++] = '%';
	data[i++] = tochar(x.account.len); /*  Copy from attr structure */
	for (j = 0; j < x.account.len; j++)
   	  data[i++] = x.account.val[j];
    }
    if (atfrmo) {			/* Packet data format */
	data[i++] = '/';
	data[i++] = tochar(x.recfm.len); /*  Copy from attr structure */
	for (j = 0; j < x.recfm.len; j++)
	  data[i++] = x.recfm.val[j];
    }
#endif /* STRATUS */
    if (attypo) {			/* File type */
	data[i++] = '"';
	if (
#ifdef VMS
	binary == XYFT_I || binary == XYFT_L || /* IMAGE or LABELED */
	!strncmp(x.recfm.val,"F",1)	/* or RECFM=Fxxxxxx */
#else
	binary				/* User said SET FILE TYPE BINARY  */
#endif /* VMS */
	    ) {				/* Binary */
	    data[i++] = tochar(2);	/*  Two characters */
	    data[i++] = 'B';		/*  B for Binary */
	    data[i++] = '8';		/*  8-bit bytes (note assumption...) */
#ifdef CK_LABELED
	    if (binary != XYFT_L
#ifdef VMS
		&& binary != XYFT_I
#endif /* VMS */
		)
		binary = XYFT_B;
#endif /* CK_LABELED */
	} else {			/* Text */
	    data[i++] = tochar(3);	/*  Three characters */
	    data[i++] = 'A';		/*  A = (extended) ASCII with CRLFs */
	    data[i++] = 'M';		/*  M for carriage return */
	    data[i++] = 'J';		/*  J for linefeed */
#ifdef VMS
	    binary = XYFT_T;		/* We automatically detected text */
#endif /* VMS */

#ifdef NOCSETS
	    data[i++] = '*';		/* Encoding */
	    data[i++] = tochar(1);	/* Length of value is 1 */
	    data[i++] = 'A';		/* A for ASCII */
#else
	    if (tcharset == TC_TRANSP) { /* Transfer character set */
		data[i++] = '*';	/* Encoding */
		data[i++] = tochar(1);	/* Length of value is 1 */
		data[i++] = 'A';	/* A for ASCII */
	    } else {
		tp = tcsinfo[tcharset].designator;
		if ((tp != NULL) && (aln = (int)strlen(tp)) > 0) {
		    data[i++] = '*';	/* Encoding */
		    data[i++] = tochar(aln+1); /* Length of designator. */
		    data[i++] = 'C';           /* Text in specified charset. */
		    for (j = 0; j < aln; j++)  /* Copy designator */
		      data[i++] = *tp++;       /*  Example: *&I6/100 */
		}
	    }
#endif /* NOCSETS */
	}
    }
    if ((xp == 0) && (x.length > -1L)) { /* If it's a real file */

	if (atdato && (aln = x.date.len) > 0) {	/* Creation date, if any */
	    data[i++] = '#';
	    data[i++] = tochar(aln);
	    for (j = 0; j < aln; j++)
	      data[i++] = x.date.val[j];
	}
	if (atleno) {
	    data[i] = '!';			/* File length in K */
	    sprintf((char *) &data[i+2],"%ld",x.lengthk);
	    aln = (int)strlen((char *)(data+i+2));
	    data[i+1] = tochar(aln);
	    i += aln + 2;

	    data[i] = '1';			/* File length in bytes */
	    sprintf((char *) &data[i+2],"%ld",x.length);
	    aln = (int)strlen((char *)(data+i+2));
	    data[i+1] = tochar(aln);
	    i += aln + 2;
	}
	if (atblko && fblksiz) {		/* Blocksize, if set */
	    data[i] = '(';
	    sprintf((char *) &data[i+2],"%d",fblksiz);
	    aln = (int)strlen((char *)(data+i+2));
	    data[i+1] = tochar(aln);
	    i += aln + 2;
	}
    }
#ifndef NOFRILLS
    if ((rprintf || rmailf) && atdiso) { /* MAIL, or REMOTE PRINT?  */
	data[i++] = '+';		/* Disposition */
        data[i++] = tochar((int)strlen(optbuf) + 1); /* Options, if any */
	if (rprintf)
	  data[i++] = 'P';		/* P for Print */
	else
	  data[i++] = 'M';		/* M for Mail */
	for (j = 0; optbuf[j]; j++)	/* Copy any options */
	  data[i++] = optbuf[j];
    }
#endif /* NOFRILLS */
#ifdef CK_RESEND
    if (sendmode == SM_RESEND) {
	data[i++] = '+';		/* Disposition */
        data[i++] = tochar(1);
	data[i++] = 'R';		/* is RESEND */
    }
#endif /* CK_RESEND */
    data[i++] = '@';			/* End of Attributes */
    data[i++] = SP;			/* Length 0 */
    data[i] = '\0';			/* Make sure it's null-terminated */
    aln = (int)strlen((char *)data);	/* Get overall length of attributes */

/* Change this code to break attribute data up into multiple packets! */

    j = (spsiz < 95) ? (92 - bctl) : (spsiz - 2 - bctl);
    if (aln > j) {			/* Check length of result */
	spack('A',pktnum,0,(CHAR *)"");	/* send an empty attribute packet */
	debug(F101,"sattr pkt too long","",aln); /* if too long */
	debug(F101,"sattr spsiz","",spsiz);
    } else {				/* Otherwise */
	spack('A',pktnum,aln,data);	/* send the real thing. */
	debug(F111,"sattr",data,aln);
    }

    return(0);
}

static char *refused = "";

static char *reason[] = {
    "size", "type", "date", "creator", "account", "area", "password",
    "blocksize", "access", "encoding", "disposition", "protection",
    "protection", "origin", "format",
    "sys-dependent",			/* 0 */
    "size",				/* 1 */
    "2",				/* 2 */
    "3",				/* 3 */
    "4",				/* 4 */
    "5",				/* 5 */
    "6",				/* 6 */
    "7",				/* 7 */
    "8",				/* 8 */
    "9",				/* 9 */
    ":",				/* : */
    ";",				/* ; */
    "<",				/* < */
    "=",				/* = */
    ">",				/* > */
    "name",				/* ? */
    "@"
};
static int nreason = sizeof(reason) / sizeof(char *);
int rejection = -1;

char *
getreason(s) char *s; {			/* Decode attribute refusal reason */
    char c, *p;
    if (rejection == 1)			/* Kludge for SET FIL COLL DISCARD */
      return("name");			/* when other Kermit doesn't... */
    p = s;
    if (*p++ != 'N') return("");	/* Should start with N */
    else if ((c = *p) > SP) {		/* get reason, */
	rejection = c;			/* remember it, */
	c -= '!';			/* get offset */
	p = ((unsigned int) ((CHAR) c) <= nreason) ? reason[c] : "unknown";
    }
    return(p);
}

int
rsattr(s) CHAR *s; {			/* Read response to attribute packet */
    debug(F111,"rsattr: ",s,*s);
    if (*s == 'N') {			/* If it's 'N' followed by anything, */
	refused = getreason((char *)s);	/* they are refusing, get reason. */
	debug(F110," refused",refused,0);
	tlog(F110," refused:",refused,0L);
	return(-1);	
    }
#ifdef CK_RESEND
    if (sendmode == SM_RESEND && *s == '1') { /* RESEND length */
	int n; long z; CHAR *p;
	p = s + 1;
	n = xunchar(*p++);
	debug(F101,"rsattr RESEND n","",n);
	z = 0L;
	while (n-- > 0)			/* We assume the format is good. */
	  z = 10L * z + (long) (*p++ - '0');
	debug(F101,"rsattr RESEND z","",z);
	if (z > 0L) sendstart = z;
	debug(F101,"rsattr RESEND sendstart","",sendstart);
	if (sendstart > 0L)
	  if (zfseek(sendstart) < 0)	/* Input file is already open. */
	    return(0);
#ifdef CK_CURSES
	if (fdispla == XYFD_C)
	  screen(SCR_FS,0,fsize,"");	/* Refresh file transfer display */
#endif /* CK_CURSES */
    }
#endif /* CK_RESEND */
    refused = "";
    return(0);
}

long rs_len = 0L;			/* Length of file being resent to */

int
gattr(s, yy) CHAR *s; struct zattr *yy; { /* Read incoming attribute packet */
    char c, d;
    char *ff;
    int aln, i;

#define ABUFL 40			/* Temporary buffer for conversions */
    char abuf[ABUFL];
#define FTBUFL 10			/* File type buffer */
    static char ftbuf[FTBUFL];
#define DTBUFL 40			/* File creation date */
    static char dtbuf[DTBUFL];
#define TSBUFL 10			/* Transfer syntax */
    static char tsbuf[TSBUFL];
#define IDBUFL 10			/* System ID */
    static char idbuf[IDBUFL];
#ifndef DYNAMIC
#define DSBUFL 100			/* Disposition */
    static char dsbuf[DSBUFL];
#define SPBUFL 512			/* System-dependent parameters */
    static char spbuf[SPBUFL];
#else
#define DSBUFL 100			/* Disposition */
    static char *dsbuf = NULL;
#define SPBUFL 512			/* System-dependent parameters */
    static char *spbuf = NULL;
#endif /* DYNAMIC */
#define RPBUFL 20			/* Attribute reply */
    static char rpbuf[RPBUFL];

    char *rp;				/* Pointer to reply buffer */
    int retcode;			/* Return code */

    d = SP;				/* Initialize disposition */
    ff = filnam;			/* Filename returned by rcvfil */
    if (fncact == XYFX_R && ofn1x && ofn1) /* But watch out for FC=RENAME */
      ff = ofn1;			/* because we haven't renamed it yet */

/* Fill in the attributes we have received */

    rp = rpbuf;				/* Initialize reply buffer */
    *rp++ = 'N';			/* for negative reply. */
    *rp = NUL;
    retcode = 0;			/* Initialize return code. */

    while (c = *s++) {			/* Get attribute tag */
	aln = xunchar(*s++);		/* Length of attribute string */
	switch (c) {
	  case '!':			/* File length in K */
	    for (i = 0; (i < aln) && (i < ABUFL); i++) /* Copy it */
	      abuf[i] = *s++;
	    abuf[i] = '\0';		/* Terminate with null */
	    if (i < aln) s += (aln - i); /* If field was too long for buffer */
	    yy->lengthk = atol(abuf);	/* Convert to number */
	    break;

	  case '"':			/* file type */
	    for (i = 0; (i < aln) && (i < FTBUFL); i++)
	      ftbuf[i] = *s++;		/* Copy it into a static string */
	    ftbuf[i] = '\0';
	    if (i < aln) s += (aln - i);
	    if (attypi) {		/* TYPE attribute is enabled? */
		yy->type.val = ftbuf;	/* Pointer to string */
		yy->type.len = i;	/* Length of string */
		debug(F111,"gattr file type",ftbuf,i);
		debug(F101,"gattr binary","",binary);
		if (			/* Unknown type? */
		    (*ftbuf != 'A' && *ftbuf != 'B' && *ftbuf != 'I')
#ifdef CK_LABELED
/* ... Or our FILE TYPE is LABELED and the incoming file is text... */
		    || (binary == XYFT_L && *ftbuf == 'A' && !xflg)
#endif /* CK_LABELED */		    
		    ) {
		    retcode = -1;	/* Reject the file */
		    *rp++ = c;
		    if (!opnerr) tlog(F100," refused: type","",0);
		    break;
		}
/*
  The following code moved here from opena() so we set binary mode
  as soon as requested by the attribute packet.  That way when the first
  data packet comes, the mode of transfer can be displayed correctly
  before opena() is called.
*/
		if (bsavef) {		/* If somehow file mode */
		    binary = bsave;	/* was saved but not restored, */
		    bsavef = 0;		/* restore it. */
		    debug(F101,"gattr restoring binary","",binary);
		}
		if (yy->type.val[0] == 'A') { /* Check received attributes. */
		    bsave = binary;	/* ASCII.  Save global file type */
		    bsavef = 1;		/* ( restore it in clsof() ) */
		    binary = XYFT_T;	/* Set current type to Text. */
		    debug(F100,"gattr attribute A = text","",binary); /*  */
		} else if (yy->type.val[0] == 'B') {
		    bsave = binary;	/* Binary.  Save global file type */
		    bsavef = 1;
#ifdef CK_LABELED
		    if (binary != XYFT_L
#ifdef VMS
			&& binary != XYFT_U /* VMS special case */
#endif /* VMS */
			)
#endif /* CK_LABELED */
#ifdef MAC
		    if (binary != XYFT_M) /* If not MacBinary... */
#endif /* MAC */
		      binary = XYFT_B;
		    debug(F101,"gattr attribute B = binary","",binary);
		}
	    }
	    break;

	  case '#':			/* File creation date */
	    for (i = 0; (i < aln) && (i < DTBUFL); i++)
	      dtbuf[i] = *s++;		/* Copy it into a static string */
	    if (i < aln) s += (aln - i);
	    dtbuf[i] = '\0';
	    if (atdati) {
		yy->date.val = dtbuf;	/* Pointer to string */
		yy->date.len = i;	/* Length of string */
		if (fncact == XYFX_U) {	/* Receiving in update mode? */
		    if (zstime(ff,yy,1) > 0) { /* Compare dates */
			*rp++ = c;	/* Discard if older, reason = date. */
			if (!opnerr) tlog(F100," refused: date","",0);
			retcode = -1;	/* Rejection notice. */
		    }
		}				
	    }
	    break;

	  case '(':			/* File Block Size */
	    for (i = 0; (i < aln) && (i < ABUFL); i++) /* Copy it */
	      abuf[i] = *s++;
	    abuf[i] = '\0';		/* Terminate with null */
	    if (i < aln) s += (aln - i);
	    if (atblki)
	      yy->blksize = atol(abuf); /* Convert to number */
	    break;

	  case '*':			/* Encoding (transfer syntax) */
	    for (i = 0; (i < aln) && (i < TSBUFL); i++)
	      tsbuf[i] = *s++;		/* Copy it into a static string */
	    if (i < aln) s += (aln - i);
	    tsbuf[i] = '\0';
	    if (atenci) {
		yy->encoding.val = tsbuf; /* Pointer to string */
		yy->encoding.len = i;	/* Length of string */
		debug(F101,"gattr encoding",tsbuf,i);
		switch (*tsbuf) {
#ifndef NOCSETS
		  case 'A':		/* Normal, nothing special */
		    tcharset = TC_TRANSP; /* Transparent chars untranslated */
		    break;
		  case 'C':		/* Specified character set */
		    for (i = 0; i < ntcsets; i++) { 
			if (!strcmp(tcsinfo[i].designator,tsbuf+1)) break;
		    }
		    debug(F101,"gattr xfer charset lookup","",i);
		    if (i == ntcsets) {	/* If unknown character set, */
			debug(F110,"gattr: xfer charset unknown",tsbuf+1,0);
			if (!unkcs) {	/* and SET UNKNOWN DISCARD, */
			    retcode = -1; /* reject the file. */
			    *rp++ = c;
			    if (!opnerr)
			      tlog(F100," refused: character set","",0);
			}
		    } else {
			tcharset = tcsinfo[i].code; /* if known, use it */
			rx = xlr[tcharset][fcharset]; /* xlation function */
		    }
		    debug(F101,"gattr tcharset","",tcharset);
		break;
#endif /* NOCSETS */
	      default:			/* Something else. */
		debug(F110,"gattr unk encoding attribute",tsbuf,0);
		if (!unkcs) {		/* If SET UNK DISC */
		    retcode = -1;
		    *rp++ = c;
		    if (!opnerr) tlog(F100," refused: encoding","",0);
		}
		break;
		}
	    }
	    break;

	  case '+':			/* Disposition */
#ifdef DYNAMIC
	    if (!dsbuf)
		if ((dsbuf = malloc(DSBUFL+1)) == NULL)
		    fatal("gtattr: no memory for dsbuf");
#endif /* DYNAMIC */
	    for (i = 0; (i < aln) && (i < DSBUFL); i++)
	      dsbuf[i] = *s++;		/* Copy it into a separate string */
	    dsbuf[i] = '\0';
	    if (i < aln) s += (aln - i);
	    rs_len = 0;
	    if (atdisi) {
		yy->disp.val = dsbuf;	/* Pointer to string */
		yy->disp.len = i;	/* Length of string */
		d = *dsbuf;
#ifndef NODISPO
/*
  Define NODISPO to disable receipt of mail or print files and of RESEND.
*/
		if (
#ifndef datageneral			/* MAIL supported only for */
#ifndef OS2				/* UNIX, VMS, and OS-9 */
#ifndef MAC
#ifndef GEMDOS
#ifndef AMIGA
		    d != 'M' &&		/* MAIL */
#endif /* AMIGA */
#endif /* GEMDOS */
#endif /* MAC */
#endif /* OS/2 */
#endif /* datageneral */
#ifdef CK_RESEND
		    d != 'R' &&		/* RESEND */
#endif /* CK_RESEND */
		    d != 'P') {		/* PRINT */
		    retcode = -1;
		    *rp++ = c;
		    if (!opnerr) tlog(F100," refused: disposition","",0);
		}
		if (d == 'R') {		/* File is being resent */
#ifdef CK_RESEND
		    rs_len = zchki(ff); /* Get length of file */
		    debug(F111,"gattr RESEND",ff,rs_len);
#ifdef VMS
		    rs_len &= (long) -512; /* Ensure block boundary if VMS */
		    rs_len -= 512;	  /* In case last block not complete */
		    debug(F111,"gattr rs_len",ff,rs_len);
#endif /* VMS */
		    if (rs_len < 0L) rs_len = 0L;
		    if (rs_len > 0L) {
			fncsav = fncact; /* Save collision action */
			fncact = XYFX_A; /* Switch to APPEND */
		    }
#else
		    retcode = -1;	/* This shouldn't happen */
		    *rp++ = c;		/* 'cause it wasn't negotiated. */
		    if (!opnerr) tlog(F100," refused: resend","",0);
#endif /* CK_RESEND */
		}
#else  /* NODISPO */
		retcode = -1;
		*rp++ = c;
		if (!opnerr) tlog(F100," refused: resend","",0);
#endif /* NODISPO */
	    }
	    break;

	  case '.':			/* Sender's system ID */
	    for (i = 0; (i < aln) && (i < IDBUFL); i++)
	      idbuf[i] = *s++;		/* Copy it into a static string */
	    idbuf[i] = '\0';
	    if (i < aln) s += (aln - i);
	    if (atsidi) {
		yy->systemid.val = idbuf; /* Pointer to string */
		yy->systemid.len = i;	/* Length of string */
	    }
	    break;

	  case '0':			/* System-dependent parameters */
#ifdef DYNAMIC
	    if (!spbuf && !(spbuf = malloc(SPBUFL)))
		fatal("gattr: no memory for spbuf");
#endif /* DYNAMIC */
	    for (i = 0; (i < aln) && (i < SPBUFL); i++)
	      spbuf[i] = *s++;		/* Copy it into a static string */
	    spbuf[i] = '\0';
	    if (i < aln) s += (aln - i);
	    if (atsysi) {
		yy->sysparam.val = spbuf; /* Pointer to string */
		yy->sysparam.len = i;	/* Length of string */
	    }
	    break;

	  case '1':			/* File length in bytes */
	    for (i = 0; (i < aln) && (i < ABUFL); i++) /* Copy it */
	      abuf[i] = *s++;
	    abuf[i] = '\0';		/* Terminate with null */
	    if (i < aln) s += (aln - i);
	    yy->length = atol(abuf);	/* Convert to number */
	    debug(F111,"gattr length",abuf,(int) yy->length);
	    break;

#ifdef STRATUS
	  case '/':			/* Format of data in packets */
	    /* We just copy it into the record and let the fio module */
	    /* figure out what to do with it. */
	    for (i = 0; (i < aln) && (i < ABUFL); i++) /* Copy it */
	      abuf[i] = *s++;
	    abuf[i] = '\0';		/* Terminate with null */
	    if (atsysi) {		/* same switch as OS-DEPENDENT */
		yy->recfm.val = spbuf;	/* Pointer to string */
		yy->recfm.len = i;	/* Length of string */
	    }
	    break;
#endif /* STRATUS */

	  default:			/* Unknown attribute */
	    s += aln;			/* Just skip past it */
	    break;
	}
    }

    /* Check file length now, because we also need to know the file type */
    /* in case zchkspa() differentiates text and binary (VMS version does) */

    if (atleni) {			/* Length attribute enabled? */
	if (yy->length > -1L) {		/* Length-in-bytes attribute rec'd? */
	    if (!zchkspa(ff,(yy->length))) { /* Check space */
		retcode = -1;		     /* Not enuf */
		*rp++ = '1';
		if (!opnerr) tlog(F100," refused: length","",0);
	    }
	} else if (yy->lengthk > -1L) {	/* Length in K attribute rec'd? */
	    if (!zchkspa(ff,(yy->lengthk * 1024))) {
		retcode = -1;		/* Check space */
		*rp++ = '!';
		if (!opnerr) tlog(F100," refused: length","",0);
	    }
	}
    }
    if (yy->length > -1L) {		/* Remember the file size */
	fsize = yy->length;		
    } else if (yy->lengthk > -1L) {
	fsize = yy->lengthk * 1024L;
    } else fsize = -1L;

#ifdef DEBUG
    if (deblog) {
	sprintf(abuf,"%ld",fsize);
	debug(F110,"gattr fsize",abuf,0);
    }
#endif /* DEBUG */
    if (retcode == 0) rp = rpbuf;	/* Null reply string if accepted */
    *rp = '\0';				/* End of reply string */
#ifdef CK_RESEND
    if (d == 'R') {			/* Receiving a RESEND? */
	debug(F101,"gattr RESEND","",retcode);
	/* We ignore retcodes because this overrides */
	if (binary != XYFT_B) {		/* Reject if not binary */
	    retcode = -1;		/* in case type field came */
	    strcpy(rpbuf,"N+");		/* after the disposition field */
	    debug(F111,"gattr RESEND not binary",rpbuf,binary);
	} else {			/* Binary mode */
	    retcode = 0;		/* Accept the file */
	    sprintf(rpbuf+2,"%ld",rs_len); /* Reply with length of file */
	    rpbuf[0] = '1';		/* '1' means Length in Bytes */
	    rpbuf[1] = tochar((int)strlen(rpbuf+2)); /* Length of length */
	    debug(F111,"gattr RESEND OK",rpbuf,retcode);
	}
    }
#endif /* CK_RESEND */
    if (retcode == 0 && discard != 0) {	/* Do we still have a discard flag? */
	strcpy(rpbuf,"N?");		/* Yes, must be filename collision */
	retcode = -1;			/* "?" = name (reply-only code) */
    }
    yy->reply.val = rpbuf;		/* Add it to attribute structure */
    yy->reply.len = (int)strlen(rpbuf);
    if (retcode < 0) {			/* If we are rejecting */
	discard = 1;			/* remember to discard the file */
	rejection = rpbuf[1];		/* and use the first reason given. */
    }
    debug(F111,"gattr return",rpbuf,retcode);
    return(retcode);
}

/*  I N I T A T T R  --  Initialize file attribute structure  */

int
initattr(yy) struct zattr *yy; {
    yy->lengthk = yy->length = -1L;
    yy->type.val = "";
    yy->type.len = 0;
    yy->date.val = "";
    yy->date.len = 0;
    yy->encoding.val = "";
    yy->encoding.len = 0;
    yy->disp.val = "";
    yy->disp.len = 0;
    yy->systemid.val = "";
    yy->systemid.len = 0;
    yy->sysparam.val = "";
    yy->sysparam.len = 0;
    yy->creator.val = "";
    yy->creator.len = 0;
    yy->account.val = "";
    yy->account.len = 0;
    yy->area.val = "";
    yy->area.len = 0;
    yy->password.val = "";
    yy->password.len = 0;
    yy->blksize = -1L;
    yy->access.val = "";
    yy->access.len = 0;
    yy->lprotect.val = "";
    yy->lprotect.len = 0;
    yy->gprotect.val = "";
    yy->gprotect.len = 0;
    yy->recfm.val = "";
    yy->recfm.len = 0;
    yy->reply.val = "";
    yy->reply.len = 0;
    return(0);
}

/*  A D E B U -- Write attribute packet info to debug log  */

int
adebu(f,zz) char *f; struct zattr *zz; {
#ifdef DEBUG
    if (deblog == 0) return(0);
    debug(F110,"Attributes for incoming file ",f,0);
    debug(F101," length in K","",(int) zz->lengthk);
    debug(F111," file type",zz->type.val,zz->type.len);
    debug(F111," creation date",zz->date.val,zz->date.len);
    debug(F111," creator",zz->creator.val,zz->creator.len);
    debug(F111," account",zz->account.val,zz->account.len);
    debug(F111," area",zz->area.val,zz->area.len);
    debug(F111," password",zz->password.val,zz->password.len);
    debug(F101," blksize","",(int) zz->blksize);
    debug(F111," access",zz->access.val,zz->access.len);
    debug(F111," encoding",zz->encoding.val,zz->encoding.len);
    debug(F111," disposition",zz->disp.val,zz->disp.len);
    debug(F111," lprotection",zz->lprotect.val,zz->lprotect.len);
    debug(F111," gprotection",zz->gprotect.val,zz->gprotect.len);
    debug(F111," systemid",zz->systemid.val,zz->systemid.len);
    debug(F111," recfm",zz->recfm.val,zz->recfm.len);
    debug(F111," sysparam",zz->sysparam.val,zz->sysparam.len);
    debug(F101," length","",(int) zz->length);
    debug(F110," reply",zz->reply.val,0);
#endif /* DEBUG */
    return(0);
}

/*  O P E N A -- Open a file, with attributes.  */
/*
  This function tries to open a new file to put the arriving data in.  The
  filename is the one in the srvcmd buffer.  File collision actions are:
  OVERWRITE (the existing file is overwritten), RENAME (the new file is
  renamed), BACKUP (the existing file is renamed), DISCARD (the new file is
  refused), UPDATE (the incoming file replaces the existing file only if the
  incoming file has a newer creation date).

  Returns 0 on failure, nonzero on success.
*/
extern char *rf_err;

int
opena(f,zz) char *f; struct zattr *zz; {
    int x, dispos = 0;
    static struct filinfo fcb;		/* Must be static! */

    debug(F110,"opena f",f,0);
    debug(F101,"opena discard","",discard);

    adebu(f,zz);			/* Write attributes to debug log */

    ffc = 0L;				/* Init file-character counter */

    /* Set up file control structure */

    fcb.bs = fblksiz;			/* Blocksize */
#ifndef NOCSETS
    fcb.cs = fcharset;			/* Character set */
#else
    fcb.cs = 0;				/* Character set */
#endif /* NOCSETS */
    fcb.rl = frecl;			/* Record Length */
    fcb.fmt = frecfm;			/* Record Format */
    fcb.org = forg;			/* Organization */
    fcb.cc = fcctrl;			/* Carriage control */
    fcb.typ = binary;			/* Type */
    fcb.dsp = (fncact == XYFX_A) ? XYFZ_A : XYFZ_N; /* Disposition */
    fcb.os_specific = '\0';		/* OS-specific info */
#ifdef CK_LABELED
    fcb.lblopts = lf_opts;		/* Labeled file options */
#else
    fcb.lblopts = 0;
#endif /* CK_LABELED */

    if (zz->disp.len > 0) {		/* Incoming file has a disposition? */
	debug(F111,"open disposition",zz->disp.val,zz->disp.len);
	dispos = (int) (*(zz->disp.val));
    }
    debug(F101,"opena dispos","",dispos);

    if (!dispos) {			         /* No special disposition? */
	if (fncact == XYFX_B && ofn1x && ofn2) { /* File collision = BACKUP? */
	    if (zrename(ofn1,ofn2) < 0) {        /* Rename existing file. */
		debug(F110,"opena rename fails",ofn1,0);
		rf_err = "Can't create backup file";
		return(0);
	    } else debug(F110,"opena rename ok",ofn2,0);
	}
    } else if (dispos == 'R') {		/* Receiving a RESEND */
#ifdef COMMENT
        if (fncact == XYFX_R)		/* and file collision = RENAME */
	  if (ofn1x)
#endif
	    f = ofn1;			/* use original name. */
    }
    debug(F111,"opena [file]=mode: ",f,fcb.dsp);
    if (x = openo(f,zz,&fcb)) {		/* Try to open the file. */
	tlog(F110," local name:",f,0L);	/* OK, open, record name. */
	if (binary) {			/* Log file mode in transaction log */
	    tlog(F101," mode: binary","",(long) binary);
	} else {			/* If text mode, check character set */
	    tlog(F100," mode: text","",0L);
#ifndef NOCSETS
	    tlog(F110," file character-set",fcsinfo[fcharset].name,0L);
	    if (tcharset == TC_TRANSP)
	      tlog(F110," xfer character-set","transparent",0L);
	    else
	      tlog(F110," xfer character-set",tcsinfo[tcharset].name,0L);
#endif /* NOCSETS */
	    debug(F111," opena charset",zz->encoding.val,zz->encoding.len);
	}
	if (fsize > -1L) screen(SCR_FS,0,fsize,"");

#ifdef datageneral
/* Need to turn on multi-tasking console interrupt task here, since multiple */
/* files may be received. */
        if ((local) && (!quiet))        /* Only do this if local & not quiet */
            consta_mt();                /* Start the asynch read task */
#endif /* datageneral */

    } else {				/* Did not open file OK. */
#ifdef ATTSV
	extern int errno;
#ifdef BSD44
#ifdef __386BSD__
#ifndef NDSYSERRLIST
	extern char *sys_errlist[];
#endif /* NDSYSERRLIST */
#else
#ifndef __bsdi__
#ifndef NDSYSERRLIST
	extern const char *const sys_errlist[];
#endif /* NDSYSERRLIST */
#endif /* __bsdi__ */
#endif /* __386BSD__ */
	char *e;
	e = (char *) sys_errlist[errno];
	screen(SCR_EM,0,0l,e);
#else /* !BSD44 */
#ifndef NDSYSERRLIST
	extern char *sys_errlist[];
#endif /* NDSYSERRLIST */
	screen(SCR_EM,0,0l,sys_errlist[errno]);
#endif /* BSD44 */

#else /* !ATTSV */

#ifdef BSD4
#ifndef NDSYSERRLIST
	extern char *sys_errlist[];
#endif /* NDSYSERRLIST */
	extern int errno;
	screen(SCR_EM,0,0l,sys_errlist[errno]);
#else
	screen(SCR_EM,0,0l,"Can't open output file");
#endif /* BSD4 */
#endif /* ATTSV */

        tlog(F110,"Failure to open",f,0L);
    }
    return(x);				/* Pass on return code from openo */
}

/*  C A N N E D  --  Check if current file transfer cancelled */

int
canned(buf) CHAR *buf; {
    if (*buf == 'X') cxseen = 1;
    if (*buf == 'Z') czseen = 1;
    debug(F101,"canned: cxseen","",cxseen);
    debug(F101," czseen","",czseen);
    return((czseen || cxseen) ? 1 : 0);
}


/*  O P E N I  --  Open an existing file for input  */

int
openi(name) char *name; {
    int x, filno;
    char *name2;

    if (memstr) return(1);		/* Just return if file is memory. */

    debug(F110,"openi",name,0);
    debug(F101," sndsrc","",sndsrc);

    filno = (sndsrc == 0) ? ZSTDIO : ZIFILE;    /* ... */

    debug(F101," file number","",filno);

    if (server && !en_cwd) {		/* If running as server */
	zstrip(name,&name2);		/* and CWD is disabled... */
	if (				/* check if pathname was included. */
#ifdef VMS
	zchkpath(name)
#else
	strcmp(name,name2)
#endif /* VMS */
        ) {
	    tlog(F110,name,"authorization failure",0L);
	    debug(F110," openi authorization failure",name,0);
	    return(0);
	} else name = name2;
    }
    if (zopeni(filno,name)) {		/* Otherwise, try to open it. */
	debug(F110," ok",name,0);
    	return(1);
    } else {				/* If not found, */
	char xname[100];		/* convert the name */
	zrtol(name,xname);		/* to local form and then */
	x = zopeni(filno,xname);	/* try opening it again. */
	debug(F101," zopeni","",x);
	if (x) {
	    debug(F110," ok",xname,0);
	    return(1);			/* It worked. */
        } else {
#ifdef COMMENT
	    screen(SCR_EM,0,0l,"Can't open file");  /* It didn't work. */
#endif /* COMMENT */
	    tlog(F110,xname,"could not be opened",0L);
	    debug(F110," openi failed",xname,0);
	    return(0);
        }
    }
}

/*  O P E N O  --  Open a new file for output.  */

static int isopen = 0;

int
openo(name,zz,fcb) char *name; struct zattr *zz; struct filinfo *fcb; {
    char *name2;

    if (stdouf)				/* Receiving to stdout? */
      return(zopeno(ZSTDIO,"",zz,NULL));

    debug(F110,"openo: name",name,0);

    if (cxseen || czseen || discard) {	/* If interrupted, get out before */
	debug(F100," open cancelled","",0); /* destroying existing file. */
	return(1);			/* Pretend to succeed. */
    }
    if (server && !en_cwd) {		/* If running as server */
	zstrip(name,&name2);		/* and CWD is disabled, */
	if (strcmp(name,name2)) {	/* check if pathname was included. */
	    tlog(F110,name,"authorization failure",0L);
	    debug(F110," openo authorization failure",name,0);
	    return(0);
	} else name = name2;
    }

    if (zopeno(ZOFILE,name,zz,fcb) == 0) { /* Try to open the file */
	isopen = 0;
	debug(F110,"openo failed",name,0);
	tlog(F110,"Failure to open",name,0L);
	return(0);
    } else {
	isopen = 1;
	debug(F110,"openo ok, name",name,0);
	return(1);
    }
}

/*  O P E N T  --  Open the terminal for output, in place of a file  */

int
opent(zz) struct zattr *zz; {
    ffc = tfc = 0L;
    if (bsavef) {			/* If somehow file mode */
	binary = bsave;			/* was saved but not restored, */
	bsavef = 0;			/* restore it. */
	debug(F101,"opena restoring binary","",binary);
    }
    bsave = binary;
    bsavef = 1;
    binary = XYFT_T;
    return(zopeno(ZCTERM,"",zz,NULL));
}

/*  C L S I F  --  Close the current input file. */

int
clsif() {
    int x = 0;
#ifdef datageneral
    if ((local) && (!quiet))    /* Only do this if local & not quiet */
        if (nfils < 1)          /* More files to send ... leave it on! */
            connoi_mt();
#endif /* datageneral */
    if (memstr) {			/* If input was memory string, */
	memstr = 0;			/* indicate no more. */
    } else x = zclose(ZIFILE);		/* else close input file. */
    if (cxseen || czseen)		/* If interrupted */
      screen(SCR_ST,ST_INT,0l,"");	/* say so */
    else if (discard)			/* If I'm refusing */
      screen(SCR_ST,ST_REFU,0l,refused); /* say why */
    else {				/* Otherwise */
	fstats();			/* update statistics */
	screen(SCR_ST,ST_OK,0l,"");	/* and say transfer was OK */
    }
    hcflg = 0;				/* Reset flags */
    sendstart = 0L;			/* Don't do this again! */
#ifdef COMMENT
/*
  This prevents a subsequent call to clsof() from deleting the file
  when given the discard flag.
*/
    *filnam = '\0';			/* and current file name */
#endif /* COMMENT */
    return(x);
}


/*  C L S O F  --  Close an output file.  */

/*  Call with disp != 0 if file is to be discarded.  */
/*  Returns -1 upon failure to close, 0 or greater on success. */

int
clsof(disp) int disp; {
    int x;

    debug(F101,"clsof disp","",disp);
    if (bsavef) {			/* If we saved global file type */
	debug(F101,"clsof restoring binary","",binary);
	binary = bsave;			/* restore it */
	bsavef = 0;			/* only this once. */
    }
    if (fncsav != -1) {			/* Saved file collision action... */
	fncact = fncsav;		/* Restore it. */
	fncsav = -1;			/* Unsave it. */
    }
#ifdef datageneral
    if ((local) && (!quiet))		/* Only do this if local & not quiet */
        connoi_mt();
#endif /* datageneral */
    if ((x = zclose(ZOFILE)) < 0) {	/* Try to close the file */
	tlog(F100,"Failure to close",filnam,0L);
	screen(SCR_ST,ST_ERR,0l,"");
    } else if (disp) {			/* Interrupted or refused */
	if (keep == 0) {		/* If not keeping incomplete files */
	    if (isopen &&		/* AND the file is actually open */
		*filnam && (what & W_RECV)) /* AND we're receiving!!!... */
	      zdelet(filnam);		    /* ONLY THEN, delete it */
	    debug(F100,"Incomplete: discarded","",0);
	    tlog(F100," incomplete: discarded","",0L);
	    screen(SCR_ST,ST_DISC,0l,"");
	} else {			/* Keep incomplete copy */
	    fstats();
	    if (!discard) { /* Unless discarding for other reason... */
		debug(F100,"Incomplete: Kept","",0);
		tlog(F100," incomplete: kept","",0L);
	    }
	    screen(SCR_ST,ST_INC,0l,"");
	}
    } else {				/* Nothing wrong, just keep it */
	debug(F100,"Closed","",0);	/* and give comforting messages. */
	fstats();
	screen(SCR_ST,ST_OK,0L,"");
    }
    rs_len = 0;
    isopen = 0;				/* It's not open any more. */
    cxseen = 0;				/* Reset per-file interruption flag */
    return(x);				/* Send back zclose() return code. */
}

#ifdef SUNOS4S5
tolower(c) char c; { return((c)-'A'+'a'); }
toupper(c) char c; { return((c)-'a'+'A'); }
#endif /* SUNOS4S5 */

