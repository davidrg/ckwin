/* DEC/CMS REPLACEMENT HISTORY, Element CKVCON.C */
/* *2    29-AUG-1989 00:31:38 BUDA "Remove hanging qiow" */
/* *1    11-APR-1989 22:55:34 BUDA "Initial creation" */
/* DEC/CMS REPLACEMENT HISTORY, Element CKVCON.C */
char *connv = "Connect Command, V4.2(011) 23 Mar 89 for VAX/VMS";

/*  C K V C O N  --  Dumb terminal connection to remote system, for VMS  */
/*
 Adapted from the Unix C-Kermit connect module by S. Rubenstein for systems
 without fork().  This version of conect() uses contti(&c, &src) to return
 when a character is available from either the console or the comm line,
 to allow sending/receiving without breaking connection.
*
* Edit by
* 011 23-Mar-1989 mab	Clean up doesc() code.  Add malloc() in place of
		 	static buffer space.  Also increase buffer space.
* 010 06-Mar-1989 mab	General cleanup
*
*/

#include "ckvvms.h"
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

#define LBUFL 500			/* Line buffer */
static char *lbuf = 0;



/*  C O N E C T  --  Perform terminal connection  */

conect() {
    int pid, 			/* process id of child (modem reader) */
	parent_id,		/* process id of parent (keyboard reader) */
	n;
    int c;			/* c is a character, but must be signed 
				   integer to pass thru -1, which is the
				   modem disconnection signal, and is
				   different from the character 0377 */
    char errmsg[50], *erp, *cp;

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


	if (!(lbuf = malloc(LBUFL))) {
	    printf("Sorry, Can't allocate buffer space\n");
	    return(-2);
	}

/* ...connect, cont'd */

	active = 1;
	do {
	    int src;

	    contti(&c, &src);
/*
 * src = -1 Line error
 *        1 Character from comm line
 * 	  0 Character from console
 */
	    if (src < 0) { 		/* Comm line hangup or other error */
/*
 * We should check WHY src < 0 and not just dive under for ANY
 * reason.
 */
		printf("\r\nC-Kermit: Communications line failure\r\n");
		active = 0;
	    }
	    else
		if (!src) {
/*
 * Character from console
 */
#ifdef mabworkonthiscode
		    c &= 0177;
#endif
		    if (c == escape) {	/* Look for escape char */
			conresne();	/* Restore to normal attributes */
#ifdef mabworkonthiscode
			c = coninc(0) & 0177;
#else
			c = coninc(0);
#endif
			doesc(c);
			conbin(escape);
		    }
		    else {		/* Ordinary character */
			ttoc(dopar(c));
			if (duplex) {	/* Half duplex? */
			    conoc(c);	/* Yes, also echo it. */
			    if (seslog) zchout(ZSFILE,c); /* Maybe log it. */
			}			
		    }
                }
	        else
		{
/*
 * Character from comm. line
 */
#ifdef mabworkonthiscode
		c &= 0177;		/* Got a char, strip parity. */
#endif
		conoc(c);		/* Put it on the screen. */
		if (seslog) zchout(ZSFILE,c); /* If logging, log it. */
		n = ttchk();	/* Any more left in buffer? */

/*
 * Check the routine ttchk and see if it eats a character when it checks the
 * typeahead count.
 */
		if (n > 0) {
		    if (n > LBUFL) n = LBUFL;  /* Get them all at once. */
		    if ((n = ttxin(n,lbuf)) > 0) {
#ifdef mabworkonthiscode
		        for (i = 0, cp = lbuf; i < n; i++) *cp++ &= 0177;
#endif
		        conxo(n,lbuf);
		        if (seslog) zsoutx(ZSFILE,lbuf,n);
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
"\r\n ",
"\r\nC to close the connection, or:",
"\r\n  0 (zero) to send a null",
"\r\n  B to send a BREAK",
"\r\n  H to hangup and close connection",
"\r\n  S for status",
"\r\n  ? for help",
"\r\n escape character twice to send the escape character.\r\n\r\n",
" ",
"" };

/*
 * Need to save term characteristics/ allow disable binary mode
 * print message, get text and then restore previous state.
 */
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

    if (c < SP) {
	sprintf(s,"CTRL-%c",ctl(c));
    } else sprintf(s,"'%c'",c);
    return(s);
}



/*  D O E S C  --  Process an escape character argument  */

doesc(c) register unsigned char c; {
    int d;
    char sbuf[35];
  
    c &= 0177;			    /* Make into a 7 bit character */

    if (c == escape) {              /* Send escape character */
        d = dopar(c);
        ttoc(d);
    } else {
                                    /* Or else look it up below. */
        if (isupper(c)) c = tolower(c);
        if (iscntrl(c)) c += 'a' - '\001';
        
	switch (c) {
	    case 'b':
		ttsndb();		/* Send a BREAK */
		break;
	    case 'c':			/* Close connection */
		active = 0;
		conol("\r\n");
		break;
	    case 'h':			/* Hang up the line */
		tthang();
		break;
	    case 's':			/* Status */
		sprintf(sbuf,"\r\nConnected thru %s",ttname);
		conol(sbuf);
		sprintf(sbuf,", speed: %d",speed);
		conol(sbuf);
		if (seslog) {
		    sprintf(sbuf,", logging file: %s",sesfil);
		    conol(sbuf);
		}
		conoll("");
		break;
	    case '?':			/* Help */
		hconne();
		break;
	    case '0':			/* Send a NULL */
		c = '\0';
		d = dopar(c);
		ttoc(d);
		break;
	    case SP:			/* Ignore space */
		break;
	    default:			/* Beep on everything else */
		conoc(BEL);
        }
    }           
}    
