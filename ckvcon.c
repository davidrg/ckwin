char *connv = "Connect Command, V4.2(008) 5 Jul 85";

/*  C K V C O N  --  Dumb terminal connection to remote system, for VMS  */
/*
 Adapted from the Unix C-Kermit connect module by S. Rubenstein for systems
 without fork().  This version of conect() uses contti(&c, &src) to return
 when a character is available from either the console or the comm line,
 to allow sending/receiving without breaking connection.
*/
#include "ckcker.h"
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>

#ifndef SIGUSR1
#define SIGUSR1 16
#endif

extern int local, speed, escape, duplex, parity, flow, seslog, mdmtyp;
extern char ttname[], sesfil[];

int i, active;				/* Variables global to this module */
char *chstr();

#define LBUFL 100			/* Line buffer */
char lbuf[LBUFL];


/*  C O N E C T  --  Perform terminal connection  */

conect() {
    int pid, 			/* process id of child (modem reader) */
	parent_id,		/* process id of parent (keyboard reader) */
	n;
    int c;			/* c is a character, but must be signed 
				   integer to pass thru -1, which is the
				   modem disconnection signal, and is
				   different from the character 0377 */
    char errmsg[50], *erp;

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
	if (ttvt(speed,flow) < 0) {
	    conres();
	    printf("Sorry, Can't condition communication line\n");
	    return(-2);
    	}

/* cont'd... */


/* ...connect, cont'd */

	active = 1;
	do {
	    int src;

	    contti(&c, &src);
	    if (src < 0) { 		/* Comm line hangup or other error */
		printf("\r\nC-Kermit: Communications line failure\r\n");
		active = 0;
	    } else if (src) {		/* Character from comm. line */
		c &= 0177;		/* Got a char, strip parity. */
		conoc(c);		/* Put it on the screen. */
		if (seslog) zchout(ZSFILE,c); /* If logging, log it. */
		n = ttchk();	/* Any more left in buffer? */
		if (n > 0) {
		    if (n > LBUFL) n = LBUFL;  /* Get them all at once. */
		    if ((n = ttxin(n,lbuf)) > 0) {
		        for (i = 0; i < n; i++) lbuf[i] &= 0177;
		        conxo(n,lbuf);
		        if (seslog) zsoutx(ZSFILE,lbuf,n);
		    }
		}
	    } else {			/* Character from console */
		c &= 0177;
		if (c == escape) {	/* Look for escape char */
		    c = coninc(0) & 0177;
		    doesc(c);
		} else {		/* Ordinary character */
		    ttoc(dopar(c));
		    if (duplex) {	/* Half duplex? */
			conoc(c);	/* Yes, also echo it. */
			if (seslog) zchout(ZSFILE,c);	/* And maybe log it. */
    	    	    }			
		}
	    }
	} while (active);
	cancio();
	conres();
	printf("\r\nC-Kermit Disconnected\r\n");
	return(0);
}


/*  H C O N N E  --  Give help message for connect.  */

hconne() {
    int c;
    static char *hlpmsg[] = {
" ",
"\r\nC to close the connection, or:",
"\r\n  0 (zero) to send a null",
"\r\n  B to send a BREAK",
"\r\n  H to hangup and close connection",
"\r\n  S for status",
"\r\n  ? for help",
"\r\n escape character twice to send the escape character.\r\n\r\n",
" ",
"" };
    conola(hlpmsg);			/* Print the help message. */
    conol("Command>");			/* Prompt for command. */
    c = coninc(0);
    conoc(c);				/* Echo it. */
    conoll("");
    doesc(c);
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
    int d;
  
    c &= 0177;

    if (c == escape) {              /* Send escape character */
        d = dopar(c);
        ttoc(d);
    } else {
                                    /* Or else look it up below. */
        if (isupper(c)) c = tolower(c);
        if (iscntrl(c)) c += 'a' - '\001';
        
        if (c == 'c') {                 /* Close connection */
            active = 0;
            conol("\r\n");
        } else if (c == 'b') {          /* Send a BREAK */
            ttsndb();
        } else if (c == 'h') {          /* Hang up the line */
            tthang();
        } else if (c == 's') {          /* Status */
            conol("\r\nConnected thru ");
            conoll(ttname);
            if (seslog) {
                conol(", logging to ");
                conol(sesfil);
            }
        } else if (c == '?') {          /* Help */
	    hconne();
        } else if (c == '0') {          /* Send a null */
            c = '\0';
            d = dopar(c);
            ttoc(d);
        } else if (c != SP) {           /* Space, ignore */
            conoc(BEL);                 /* Invalid esc arg, beep */
        }
    }           
}    
