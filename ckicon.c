char *connv = "Connect Command for Amiga, V4D(002) 27 July 86";
 
/*  C K I C O N  --  Dumb terminal connection to remote system, for Amiga  */
/*
 Author: Frank da Cruz (SY.FDC@CU20B),
 Columbia University Center for Computing Activities, January 1985.
 Copyright (C) 1985, Trustees of Columbia University in the City of New York.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained.

 Modified for Amiga by Jack J. Rouse, The Software Distillery

 Uses the following special functions from ckitio:
    conttb() -- Prepares for connect mode.
    contte() -- Terminates connect mode.  If any active write does
		terminate in 1 second, it is aborted and the serial
		device restarted to break XOFF deadlocks.
    ttocq(c) -- Outputs character to serial device.  If it can not
		be sent immediately, it is queued (as when XOFF'ed).
		If the queue (currently 64 characters) overflows, it
		returns an error.
    contti() -- Waits for console or serial input, returning console
		character, or -2 if serial input is available first.
		It sends queued serial output when the serial device
		is ready for it.
*/
 
#include <stdio.h>
#include <ctype.h>			/* Character types */
#include "ckcdeb.h"
#include "ckcker.h"
 
extern int local, speed, escape, duplex, parity, flow, seslog, mdmtyp;
extern int errno;
extern char ttname[], sesfil[];
extern CHAR dopar();

/* Variables global to this module */
extern char *chstr();
static int active;
static int logging;			/* session logging active */
 
#define LBUFL 200			/* Line buffer */
char lbuf[LBUFL];

/*  C O N E C T  --  Perform terminal connection  */
 
conect() {
    int c, n, i;
    int seenescape;
    int overrun;

    if (!local) {
	printf("Sorry, you must 'set line' first\n");
	return(-2);
    }
    if (speed < 0) {
	printf("Sorry, you must 'set speed' first\n");
	return(-2);
    }
    if (escape < 0 || escape > 0177) {
	printf("Your escape character is not ASCII - %d\n",escape);
	return(-2);
    }
    if (ttopen(ttname,&local,mdmtyp) < 0) {
	printf("Sorry, can't open %s\n",ttname);
	return(-2);
    }
    printf("Connecting thru %s, speed %d.\r\n",ttname,speed);
    printf("The escape character is %s (%d).\r\n",chstr(escape),escape);
    printf("Type the escape character followed by C to get back,\r\n");
    printf("or followed by ? to see other options.\r\n");
    if (seslog) printf("(Session logged to %s.)\r\n",sesfil);
 
    /* Condition console terminal and communication line */	    
 
    if (conbin(escape) < 0) {
	printf("Sorry, can't condition console terminal\n");
	return(-2);
    }
    if (ttvt(speed,flow,parity) < 0) {
	conres();
	printf("Sorry, Can't condition communication line\n");
	return(-2);
    }

    connoi();			/* disable interrupts so we can use ^C, ^D */
    conttb();			/* prepare contti interface */
    active = logging = 1;
    seenescape = overrun = 0;
    while (active) {
	if ((c = contti()) >= 0) {
	    if (seenescape) {
		doesc(c);
		seenescape = 0;
	    }
	    else if (c == escape)
		seenescape = 1;
	    else if ((i = ttocq(dopar(c))) >= 0) { /* Ordinary character */
		overrun = 0;
		if (duplex)
		{   /* Half duplex? */
		    conoc(c);	/* Yes, also echo it. */
		    if (seslog && logging) 	/* And maybe log it. */
			if (zchout(ZSFILE,c) < 0)
			    seslog = 0;
		}
	    }
	    else if (i == -2) { /* overrun */
		if (!overrun) conoc(BEL);
		overrun = 1;
	    }
	    else {
		printf("Can't send character\n");
		active = 0;
	    }
	}
	else if (c != -2)
	{
		printf("Can't get character\n");
		break;
	}
	if (!active) break;
	n = ttchk();
	if (n > LBUFL)
	    n = LBUFL;
	else if (n < 0) {
	    printf("Can't get character\n");
	    active = 0;
	}
	if (n > 0 && (n = ttxin(n, lbuf)) > 0) {
	    for (i = 0; i < n; ++i) lbuf[i] &= 0177;	/* Strip */
	    conxo(n, lbuf);
	    if (seslog) zsoutx(ZSFILE,lbuf,n);		/* Log */
	}
    }

    contte();			/* clean up contti stuff */
    conres();			/* Reset the console. */
    /* reset funny modes and print reassuring message */
    printf("\017\23320h[Back at Local System]\n");
    return(0);
}

/*  H C O N N E  --  Give help message for connect.  */
 
hconne() {
    int c;
    static char *hlpmsg[] = {"\
\r\nC to close the connection, or:",
"\r\n  0 (zero) to send a null",
"\r\n  B to send a BREAK",
"\r\n  H to hangup and close connection",
"\r\n  Q to suspend logging",
"\r\n  R to resume logging",
"\r\n  S for status",
"\r\n  ? for help",
"\r\n escape character twice to send the escape character.\r\n\r\n",
"" };
 
    conola(hlpmsg);			/* Print the help message. */
    conol("Command>");			/* Prompt for command. */
    while ((c = contti()) < 0);		/* wait for char, ignoring errors */
    conoc(c);				/* Echo it. */
    conoll("");
    c &= 0177;				/* Strip any parity. */
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
    char temp[50];
    int n;
  
    c &= 0177;
    while (1) {
	if (c == escape) {		/* Send escape character */
	    d = dopar(c); ttocq(d); return;
    	} else				/* Or else look it up below. */
	    if (isupper(c)) c = tolower(c);
 
	switch (c) {
 
	case 'c':			/* Close connection */
	case '\03':
	    active = 0; conol("\r\n"); return;
 
	case 'b':			/* Send a BREAK signal */
	case '\02':
	    ttsndb(); return;
 
	case 'h':			/* Hangup */
	case '\010':
	    tthang(); active = 0; conol("\r\n"); return;

	case 'q':
	case '\021':
	    if (logging && seslog) conoll("(Logging suspended)");
	    logging = 0;
	    return;

	case 'r':
	case '\022':
	    if (!logging && seslog) conoll("(Logging resumed)");
	    logging = 1;
	    return;

	case 's':			/* Status */
	case '\023':
	    conol("\r\nConnected thru ");
	    conol(ttname);
	    if (speed >= 0) {
		sprintf(temp,", speed %d",speed); conol(temp);
	    }
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
		if (!logging) conol(" (suspended)");
            }
	    if ((n = ttonq()) > 0) {
		sprintf(temp, ", %d output chars queued", n);
		conol(temp);
	    }
	    conoll(""); return;
 
	case '?':			/* Help */
	    c = hconne(); continue;
 
	case '0':			/* Send a null */
	    c = '\0'; d = dopar(c); ttocq(d); return;
 
	case SP:			/* Space, ignore */
	    return;
 
	default:			/* Other */
	    conoc(BEL); return; 	/* Invalid esc arg, beep */
    	}	    
    }
}    
