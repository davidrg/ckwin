char *dialv = "Dial Command for Unix, V0.0(005) 5 Mar 85";

/*  C K D I A L  --  Dialing program for connection to remote system */

/*
 This module should work under all versions of Unix.  It calls externally
 defined system-depended functions for i/o, but depends upon the existence
 of various modem control functions.

 Author: Herm Fischer, Litton Data Systems, Van Nuys CA (HFISCHER@USC-ECLB)
*/

#include "ckermi.h"
#include <signal.h>
#include <setjmp.h>
#include "ckcmd.h"

extern int local, speed, flow, mdmtyp;
extern char ttname[], sesfil[];

#define HAYES 1			/* for mdmtyp settings */
#define VENTEL 2
#define HAYESNV 3		/* internal use, non-verbal V0 setting */

struct keytab mdmtab[] = {		/* Modem types for command parsing */
    "direct",	0, 0,
    "hayes",	HAYES, 0,
    "ventel",	VENTEL, 0
};
int nmdm = (sizeof(mdmtab) / sizeof(struct keytab));

#define DIALING 4		/* for ttvt parameter */
#define CONNECT 5

#define CONNECTED 1		/* for completion status */
#define FAILED	  2

static int tries = 0;

#define LBUFL 100
static char lbuf[LBUFL];
static char *lbp;

static jmp_buf sjbuf;

static
timerh() {			/* timer interrupt handler */
    longjmp(sjbuf,1);
}

static
stripp(s) char *s; {	/* parity stripper aid */
    for ( ; *s ; *s++ &= 0177 );
}

/*  D I A L  --  Dial up the remote system */

dial(telnbr) char *telnbr; {

    char c;
    char *i;
    int waitct, status;
    char errmsg[50], *erp;
    int augMdm;		/* mdmtyp with switch settings added */
    int mdmEcho = 0;	/* assume modem does not echo */
    int n;
    int (*savAlrm)();	/* save incomming alarm function */

	if (!mdmtyp) {
	    printf("Sorry, you must 'set modem' first\n");
	    return(-2);
	}
	augMdm = mdmtyp;	/* internal use, to add dialer switches info*/

	if (!local) {
	    printf("Sorry, you must 'set line' first\n");
	    return(-2);
	}
	if (speed < 0) {
	    printf("Sorry, you must 'set speed' first\n");
	    return(-2);
        }
	if (ttopen(ttname,local,mdmtyp) < 0) {/* Open, no wait for carrier */
	    erp = errmsg;
	    sprintf(erp,"Sorry, can't open %s",ttname);
	    perror(errmsg);
	    return(-2);
    	}
/* cont'd... */

					/* interdigit waits for tone dial */
/* ...dial, cont'd */


	waitct = 1*strlen(telnbr) ;	/* compute time to dial worst case */
	switch (augMdm) {
	    case HAYES:
	    case HAYESNV:
		waitct += 35;		/* dial tone + carrier waits + slop */
		for (i=telnbr; *i; i++) if (*i == ',') waitct += 2;
		break;
	    case VENTEL:
		waitct += 10;		/* guess actual time for dialtones */
		waitct += 10;	/* ventel's apparent patience for carrier */
		for (i=telnbr; *i; i++) if (*i == '%') waitct += 5;
		break;
	    }

       printf("Dialing thru %s, speed %d, number %s.\r\n",ttname,speed,telnbr);
       printf("The timeout for completing the call is %d seconds.\r\n",waitct);

/* Condition console terminal and communication line */	    
				/* place line into "clocal" dialing state */
	if ( ttpkt(speed,DIALING) < 0 )  {
	    printf("Sorry, Can't condition communication line\n");
	    return(-2);
    	}

/* Put modem into command state */

    savAlrm = signal(SIGALRM,timerh);
    alarm(10);			/* give modem 10 seconds to wake up */
    if (setjmp(sjbuf)) {
	alarm(0);
	signal(SIGALRM,savAlrm);	/* cancel timer */
	ttpkt(speed,CONNECT);	/* cancel dialing state ioctl */
	printf("Sorry, unable to complete dialed connection\r\n");
	return(-2);
	}
    ttflui();			/* flush input buffer if any */

#define OKAY 1			/* modem attention attempt status */
#define IGNORE 2
#define GOT_O -2
#define GOT_A -3

switch (augMdm) {
    case HAYES:
    case HAYESNV:
	while(tries++ < 4) {
	    ttol("AT\r",3);	/* signal for attention, look for response */
	    status = 0;
	    while ( status <= 0 ) {
		switch (ttinc(0) & 0177) {
		    case 'A':			/* echoing, ignore */
			status = GOT_A;
			break;
		    case 'T':
			if (status == GOT_A) {
			    mdmEcho = 1;	/* expect echoing later */
			    status = 0;
			    break;
			}
			status = IGNORE;
			break;
		    case '\n':
		    case '\r':
			status = 0;
			break;
		    case '0':			/* numeric result code */
			augMdm = HAYESNV;	/* nonverbal result codes */
			status = OKAY;
			break;
		    case 'O':			/* maybe English result code*/
			status = GOT_O;
			break;
		    case 'K':
			if (status == GOT_O) {
			    augMdm = HAYES;
			    status = OKAY;
			    break;
			}			/* else its default anyway */
		    default:
			status = IGNORE;
			break;
		    }
		}
	    if (status == OKAY) break;
	    if (status == IGNORE) ttflui();
	    sleep(1);		/* wait before retrying */
	}
        if (status != 0) break;
        printf("Sorry, can't initialize modem\n");
	ttpkt(speed,CONNECT);		/* cancel dialing state ioctl */
	alarm(0);
	signal(SIGALRM,savAlrm);	/* cancel timer */
        return(-2);

/* cont'd... */

					/* interdigit waits for tone dial */
/* ...dial, cont'd */

    case VENTEL:
	ttoc('\r');		/* Put Ventel into command mode */
	sleep(1);
	ttoc('\r');
	sleep(1);
	ttoc('\r');
	while( (ttinc(0) & 0177) != '$');
	break;
    }
    alarm(0);			/* turn off alarm */
    sleep(1);			/* give things settling time */

		
/* Dial the number */

switch (augMdm) {
    case HAYES:
    case HAYESNV:		  
        sprintf(lbuf,"AT DT %s\r",telnbr);
	break;
    case VENTEL:
	sprintf(lbuf,"<K%s\r>",telnbr);
	break;
    }

    alarm(waitct);		/* time to allow for connecting */
    ttflui();			/* clear out stuff from waking modem up */
    ttol(lbuf,strlen(lbuf));	/* send dialing string */

/* cont'd... */

					/* interdigit waits for tone dial */
/* ...dial, cont'd */


/* Check for connection */

    status = 0;
    while (status == 0) {
      switch (augMdm) {
	case HAYES:
	    for (n = 0; n < LBUFL; n++) lbuf[n] = '\0';
	    n = ttinl(lbuf,LBUFL,0,'\n');
	    if (n > 2) {
		lbp = lbuf;
		while ((*lbp == '\r') || (*lbp == '\n')) lbp++;
		stripp(lbp);
		if (strncmp(lbp,"CONNECT",7) == 0) status = CONNECTED;
		if (strncmp(lbp,"NO CARRIER",10) == 0) status = FAILED;
	    }
	    break;

	case HAYESNV:
	    c = ttinc(0) & 0177;
	    if (mdmEcho) {		/* sponge up dialing string */
		mdmEcho = c!='\r';	/* until return is echoed */
		break;
		}
	    if (c == '1') status = CONNECTED;
	    if (c == '3') status = FAILED;
	    if (c == '5') status = CONNECTED;
	    break;

	case VENTEL:
	    if ( (ttinc(0) & 0177) == '!') status = CONNECTED;
	    break;
	}
    }
	
/* Place line into modem-control (non-clocal) state */

    if (status == 0) printf("Sorry, Can't get response from modem\r\n");
    else if (status == CONNECTED) printf("Connected!\r\n");
    else if (status == FAILED) printf("Sorry, No Carrier\r\n");
    else printf("Failed to complete call\r\n");

    ttpkt(speed,CONNECT);	/* cancel dialing state ioctl */
    alarm(0);
    signal(SIGALRM,savAlrm);	/* cancel timer */
    return((status==CONNECTED) ? 0 : -2);
}
