char *connv = "Connect Command for Unix, V4.0(002) 24 Jan 85";

/*  C O N N E C T  --  Dumb terminal connection to remote system  */

/*
 This module should work under all versions of Unix.  It calls externally
 defined system-depended functions for i/o, but depends upon the existence
 of the fork() function.
*/

#include "ckermi.h"

extern int local, speed, escape, handsh, duplex, parity, flow, seslog;
extern char ttname[], sesfil[];

int i, active;				/* Variables global to this module */
char *chstr();
#define LBUFL 100			/* Line buffer */
char lbuf[LBUFL];

/*  C O N E C T  --  Perform terminal connection  */

conect() {
    int pid, n;
    char c;
    char errmsg[50], *erp;

	if (!local) {
	    printf("Sorry, you must 'set line' first\n");
	    return;
	}
	if (speed < 0) {
	    printf("Sorry, you must 'set speed' first\n");
	    return;
        }
	if ((escape < 0) || (escape > 0177)) {
	    printf("Your escape character is not ASCII - %d\n",escape);
	    return;
	}
	if (ttopen(ttname) < 0) {
	    erp = errmsg;
	    sprintf(erp,"Sorry, can't open %s",ttname);
	    perror(errmsg);
	    return;
    	}
    	printf("Connecting thru %s, speed %d.\r\n",ttname,speed);
	printf("The escape character is %s (%d).\r\n",chstr(escape),escape);
	printf("Type the escape character followed by C to get back,\r\n");
	printf("or followed by ? to see other options.\r\n");
	if (seslog) printf("(Session logged to %s.)\r\n",sesfil);

/* cont'd... */


/* ...connect, cont'd */


/* Condition console terminal and communication line */	    

    	if (conbin() < 0) {
	    printf("Sorry, can't condition console terminal\n");
	    return;
    	}
	if (ttvt(speed,flow) < 0) {
	    conres();
	    printf("Sorry, Can't condition communication line\n");
	    return;
    	}
	pid = fork();			/* All ok, make a fork */
	if (pid) {			
	    active = 1;			/* This fork reads, sends keystrokes */
	    while (active) {
		c = coninc() & 0177;
		if (c == escape) {   	/* Look for escape char */
		    c = coninc() & 0177;
		    doesc(c);
		} else {		/* Ordinary character */
		    ttoc(dopar(c));	/* Send it out with desired parity */
		    if (duplex) {	/* Half duplex? */
			conoc(c);	/* Yes, also echo it. */
			if (seslog) zchout(ZSFILE,c);	/* And maybe log it. */
    	    	    }			
		}
    	    }
	    kill(pid,9);		/* Done, kill inferior. */
	    wait(0);			/* Wait till gone. */
	    conres();			/* Reset the console. */
	    ttclos();			/* Reset & close communication line. */
	    printf("C-Kermit Disconnected\n");
	    return;
	} else {			/* Inferior reads, prints port input */
	    while (1) {
		c = ttinc(0) & 0177;	/* Wait for a character. */
		conoc(c);
		if (seslog) zchout(ZSFILE,c);
		n = ttchk();		/* Any more left in buffer? */
		if (n > 0) {
		    if (n > LBUFL) n = LBUFL;  /* Get them all at once. */
		    if ((n = ttxin(n,lbuf)) > 0) {
			for (i = 0; i < n; i++) lbuf[i] &= 0177;
			conxo(n,lbuf);
			if (seslog) zsoutx(ZSFILE,lbuf,n);
		    }
	    	}
	    }
    	}
}


/*  H C O N N E  --  Give help message for connect.  */

hconne() {
    int c;
    char *hlpmsg = "\
\r\nC to close the connection, or:\
\r\n  S for status\
\r\n  ? for help\
\r\n  B to send a BREAK\
\r\n  0 to send a null\
\r\n escape character twice to send the escape character.\r\n\r\n";

    conol(hlpmsg);			/* Print the help message. */
    conol("Command>");			/* Prompt for command. */
    c = coninc();
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
    int d;
  
    c &= 0177;
    while (1) {
	if (c == escape) {		/* Send escape character */
	    d = dopar(c);
	    ttoc(d);
	    return;
    	} else				/* Or else look it up below. */
	    if (isupper(c)) c = tolower(c);

	switch (c) {

	case 'c':			/* Close connection */
	case '\03':
	    active = 0;
	    conol("\r\n");
	    return;

	case 'b':			/* Send a BREAK */
	case '\02':
	    ttsndb();
	    return;

	case 's':			/* Status */
	case '\023':
	    conol("\r\nConnected thru ");
	    conoll(ttname);
	    if (seslog) {
		conol(", logging to ");
		conol(sesfil);
            }
	    return;

	case '?':			/* Help */
	    c = hconne();
	    continue;

	case '0':			/* Send a null */
	    c = '\0';
	    d = dopar(c);
	    ttoc(d);
	    return;

	case SP:			/* Space, ignore */
	    return;

	default:			/* Other */
	    conoc(BEL); 		/* Invalid esc arg, beep */
	    return;
    	}	    
    }
}    

