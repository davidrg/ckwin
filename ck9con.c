char *connv = "Connect Command for Os9/68k, V4E(017)+blarson 14 Feb 88";

/*  C K 9 C O N  --  Dumb terminal connection to remote system, for osk  */
/*
 Modified from ckucon.c by Bob Larson (blarson@ecla.usc.edu)

 Author: Frank da Cruz (SY.FDC@CU20B),
 Columbia University Center for Computing Activities, January 1985.
 Copyright (C) 1985, Trustees of Columbia University in the City of New York.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained. 
*/

#include <stdio.h>
#include <ctype.h>			/* Character types */
#include "ckcdeb.h"
#include "ckcker.h"
#include <signal.h>

extern int local, speed, escape, duplex, parity, flow, seslog, mdmtyp;
extern int errno, cmask, fmask;
extern char ttname[], sesfil[];
extern CHAR dopar();

extern int ticks;

int i, active;				/* Variables global to this module */
char *chstr();
char temp[50];

#define LBUFL 200			/* Line buffer */
char lbuf[LBUFL];


/*  C O N E C T  --  Perform terminal connection  */

conect() {
    int c;			/* c is a character, but must be signed 
				   integer to pass thru -1, which is the
				   modem disconnection signal, and is
				   different from the character 0377 */
    char errmsg[50], *erp;
    int n;
    extern int ttyfd;

	if (!local) {
	    printf("Sorry, you must 'set line' first\n");
	    return(-2);
	}
	if (speed < 0) {
	    printf("Sorry, you must 'set speed' first\n");
	    return(-2);
        }
	if ((escape < 0) || (escape > 0177)) {
	    printf("Your escape character is not ASCII - %d\n",escape);
	    return(-2);
	}
	if (ttopen(ttname,&local,mdmtyp) < 0) {
	    erp = errmsg;
	    sprintf(erp,"Sorry, can't open %s",ttname);
	    perror(errmsg);
	    return(-2);
    	}
    	printf("Connecting thru %s, speed %d.\n",ttname,speed);
	printf("The escape character is %s (%d).\n",chstr(escape),escape);
	printf("Type the escape character followed by C to get back,\n");
	printf("or followed by ? to see other options.\n");
	if (seslog) printf("(Session logged to %s.)\n",sesfil);

/* Condition console terminal and communication line */	    

    	if (conbin(escape) < 0) {
	    printf("Sorry, can't condition console terminal\n");
	    return(-2);
    	}
	if (ttvt(speed,flow) < 0) {
	    conres();
	    printf("Sorry, Can't condition communication line\n");
	    return(-2);
    	}

/* cont'd... */

/* ...connect, cont'd */


	active = 1;
	while (active) {
	    /* _ss_ssig is not reliable, but better than chewing CPU */
	    ticks = 0x80000034;		/* .2 sec */
	    if(_gs_rdy(0)<=0 && _gs_rdy(ttyfd)<=0) {
	        _ss_ssig(0, SIGARB);
		_ss_ssig(ttyfd, SIGARB);
	        tsleep(ticks);
	    }
	    while(_gs_rdy(0)>0) {
		c = coninc(0) & cmask;	/* Get character from keyboard */
		if ((c & 0177) == escape) { /* Look for escape char */
		    c = coninc(0) & 0177;   /* Got esc, get its arg */
		    doesc(c);		    /* And process it */
		} else {		/* Ordinary character */
		    if (ttoc(dopar(c)) > -1) {
		    	if (duplex) {	    /* Half duplex? */
			    conoc(c);	    /* Yes, also echo it. */
			    if (seslog)     /* And maybe log it. */
			    	if (zchout(ZSFILE,c) < 0) seslog = 0;
			}
    	    	    } else {
			perror("\r\lCan't send character");
			active = 0;
		    }
		}
	    }
	    while ((n = ttchk()) > 0) {	/* Any more left in buffer? */
		if (n > LBUFL) n = LBUFL;   /* Get them all at once. */
		if ((n = ttxin(n,lbuf)) > 0) {
		    for (i = 0; i < n; i++) lbuf[i] &= cmask;  /* Strip */
		    conxo(n,lbuf);	    	    	    	   /* Output */
		    if (seslog) zsoutx(ZSFILE,lbuf,n);  	   /* Log */
		}
	    }
	}
	_ss_rel(ttyfd);
	_ss_rel(0);
	conres();			/* Reset the console. */
	printf("[Back at Local System]\n");
	return(0);
}

/*  H C O N N E  --  Give help message for connect.  */

hconne() {
    int c;
    static char *hlpmsg[] = {"\
\r\lC to close the connection, or:",
"\r\l  0 (zero) to send a null",
"\r\l  B to send a BREAK",
"\r\l  H to hangup and close connection",
"\r\l  S for status",
"\r\l  ? for help",
"\r\l escape character twice to send the escape character.\r\l\r\l",
"" };

    conola(hlpmsg);			/* Print the help message. */
    conol("Command>");			/* Prompt for command. */
    c = coninc(0) & 0177;		/* Get character, strip any parity. */
    conoc(c);				/* Echo it. */
    conoll("");
    return(c);				/* Return it. */
}


/*  C H S T R  --  Make a printable string out of a character  */

char *
chstr(c) int c; {
    static char s[8];
    char *cp = s;

    if (c < SP) {
	sprintf(cp,"CTRL-%c",ctl(c));
    } else sprintf(cp,"'%c'\n",c);
    cp = s;
    return(cp);
}

/*  D O E S C  --  Process an escape character argument  */

doesc(c) char c; {
    CHAR d;
  
    while (1) {
	if (c == escape) {		/* Send escape character */
	    d = dopar(c); ttoc(d); return;
    	} else				/* Or else look it up below. */
	    if (isupper(c)) c = tolower(c);

	switch (c) {

	case 'c':			/* Close connection */
	case '\03':
	    active = 0; conol("\r\l"); return;

	case 'b':			/* Send a BREAK signal */
	case '\02':
	    ttsndb(); return;

	case 'h':			/* Hangup */
	case '\010':
	    tthang(); active = 0; conol("\r\l"); return;

	case 's':			/* Status */
	    conol("\r\lConnected thru ");
	    conol(ttname);
	    if (speed >= 0) {
		sprintf(temp,", speed %d",speed); conol(temp);
	    }
	    sprintf(temp,", %d bits",(cmask == 0177) ? 7 : 8);
	    if (parity) {
		conol(", ");
		switch (parity) {
		    case 'e': conol("even");  break;
		    case 'o': conol("odd");   break;
		    case 's': conol("space"); break;
		    case 'm': conol("mark");  break;
		}
		conol(" parity");
	    }
	    if (seslog) {
		conol(", logging to "); conol(sesfil);
            }
	    conoll(""); return;

	case '?':			/* Help */
	    c = hconne(); continue;

	case '0':			/* Send a null */
	    c = '\0'; d = dopar(c); ttoc(d); return;

	case SP:			/* Space, ignore */
	    return;

	default:			/* Other */
	    conoc(BEL); return; 	/* Invalid esc arg, beep */
    	}	    
    }
}    
