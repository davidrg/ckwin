char *protv = "C-Kermit Protocol Module 4.0(014), 5 Feb 85"; /* -*-C-*- */

/* C K P R O T  -- C-Kermit Protocol Module, in Wart preprocessor notation. */

/* Authors: Jeff Damens, Bill Catchings, Frank da Cruz (Columbia University) */

#include "ckermi.h"

/* Define the states for Wart */

%states rfile rdata ssinit ssdata sseof sseot serve generic get rgen

/* Declare external C variables */

  extern char sstate, *versio, *srvtxt, *cmarg, *cmarg2;
  extern char data[], filnam[], srvcmd[], ttname[], *srvptr;
  extern int pktnum, timint, nfils, image, hcflg, xflg, speed, flow;
  extern int prvpkt, cxseen, czseen, server, local, displa, bctu, bctr, quiet;
  extern int putsrv(), puttrm(), putfil(), errpkt();
  extern char *DIRCMD, *DELCMD, *TYPCMD, *SPACMD, *SPACM2, *WHOCMD;

#define SERVE  tinit(); BEGIN serve
#define RESUME if (server) { SERVE; } else return

%%
/* Protocol entry points, one for each start state (sstate) */

s { tinit();	    	    	    	    	    /* Do Send command */
    if (sinit()) BEGIN ssinit;
       else RESUME; }
v { tinit(); BEGIN get; }		    	    /* Do Receive command */
r { tinit(); srinit(); BEGIN get; }  	    	    /* Do Get command */
c { tinit(); scmd('C',cmarg); BEGIN rgen; }   	    /* Do host Command */
g { tinit(); scmd('G',cmarg); BEGIN rgen; } 	    /* Do Generic command */
x { SERVE; }	    	    	    	    	    /* Be a Server */

/***
 Note -- Need to add states to allow for sending I-packet before
 generic or host commands, and to ignore error packets received in response.
***/


/* Dynamic states: <current-states>input-character { action } */

<rgen,get,serve>S { rinit(data); bctu = bctr; BEGIN rfile; }

<serve>R { srvptr = srvcmd; decode(data,putsrv); /* Get Receive-Init */
	   cmarg = srvcmd;
	   nfils = -1;
    	   if (sinit()) BEGIN ssinit; else { SERVE; } }

<serve>I { spar(data);			/* Get Init-Parameters */
	   rpar(data);
	   ack1(data);
	   pktnum = 0; prvpkt = -1; }

<serve>G { srvptr = srvcmd; decode(data,putsrv); /* Get & decode command. */
	   putsrv('\0'); putsrv('\0');
	   sstate = srvcmd[0]; BEGIN generic; }

<serve>C { srvptr = srvcmd;		/* Get command for shell */
	   decode(data,putsrv);
	   putsrv('\0');
	   if (syscmd("",srvcmd)) BEGIN ssinit;
	   else { errpkt("Can't do shell command"); SERVE; } }

<serve>. { errpkt("Unimplemented server function"); SERVE; }

<generic>C { if (!cwd(srvcmd+1)) errpkt("Can't change directory");
    	     SERVE; }

<generic>D { if (syscmd(DIRCMD,srvcmd+2)) BEGIN ssinit;
    	     else { errpkt("Can't list directory"); SERVE; } }

<generic>E { if (syscmd(DELCMD,srvcmd+2)) BEGIN ssinit;
    	     else { errpkt("Can't remove file"); SERVE; } }

<generic>F { ack(); return; }

<generic>H { if (sndhlp()) BEGIN ssinit;
    	     else { errpkt("Can't send help"); SERVE; } }

<generic>T { if (syscmd(TYPCMD,srvcmd+2)) BEGIN ssinit;
    	     else { errpkt("Can't type file"); SERVE; } }

<generic>U { int x;			/* Disk Usage query */
    	     x = *(srvcmd+1);
    	     x = ((x == '\0') || (x == unchar(0)));
	     x = (x ? syscmd(SPACMD,"") : syscmd(SPACM2,srvcmd+2));
    	     if (x) BEGIN ssinit; else { errpkt("Can't check space"); SERVE; }}

<generic>W { if (syscmd(WHOCMD,srvcmd+2)) BEGIN ssinit;
    	     else { errpkt("Can't do who command"); SERVE; } }

<generic>. { errpkt("Unimplemented generic server function"); SERVE; }


/* Dynamic states, cont'd */


<rgen>Y { decode(data,puttrm); RESUME; }

<rgen,rfile>F { if (rcvfil()) { ack(); BEGIN rdata; }
		 else { errpkt("Can't open file"); RESUME; } }

<rgen,rfile>X { opent(); ack(); BEGIN rdata; }

<rfile>B { ack(); reot(); RESUME; }

<rdata>D { if (cxseen) ack1("X");
    	   else if (czseen) ack1("Z");
	   else ack();
	   decode(data,putfil); }

<rdata>Z { ack(); reof(); BEGIN rfile; }

<ssinit,ssdata,sseof,sseot>N { resend(); }

<ssinit>Y {  int x; char *s;
    	     spar(data);
    	     bctu = bctr;
	     if (xflg) { x = sxpack(); s = "Can't execute command"; }
	    	  else { x = sfile(); s = "Can't open file"; }
	     if (x) BEGIN ssdata; else { errpkt(s); RESUME; }
          }	    

<ssdata>Y { if (canned(data) || !sdata()) {
		clsif();
		seof();
		BEGIN sseof; } }

<sseof>Y { if (gnfile() > 0) {
		if (sfile()) BEGIN ssdata;
		else { errpkt("Can't open file") ; RESUME; }
	   } else {
		seot();
		BEGIN sseot; } }

<sseot>Y { RESUME; }

E { int x;				/* Error packet */
    ermsg(data);			/* Issue message */
    x = quiet; quiet = 1;		/* Close files silently */
    clsif(); clsof();
    quiet = x; RESUME; }

. { nack(); }				/* Anything else, nack */
%%


/*  P R O T O  --  Protocol entry function  */

proto() {

    extern int sigint();
    int x;

    conint(sigint);			/* Enable console interrupts */

/* Set up the communication line for file transfer. */

    if (local && (speed < 0)) {
	screen(2,0,"Sorry, you must 'set speed' first");
	return;
    }
    if (ttopen(ttname) < 0) {
	screen(2,0,"Can't open line");
	return;
    }
    x = (local) ? speed : -1;
    if (ttpkt(x,flow) < 0) {		/* Put line in packet mode, */
	screen(2,0,"Can't condition line"); /* setting speed, flow control */
	return;
    }
    if (sstate == 'x') {		/* If entering server mode, */
	server = 1;			/* set flag, */
	if (!quiet) {
	    if (!local)			/* and issue appropriate message. */
	    	conol(srvtxt);
	    else {
	    	conol("Entering server mode on ");
		conoll(ttname);
	    }
	}
    } else server = 0;
    sleep(1);

/*
 The 'wart()' function is generated by the wart program.  It gets a
 character from the input() routine and then based on that character and
 the current state, selects the appropriate action, according to the state
 table above, which is transformed by the wart program into a big case
 statement.  The function is active for one transaction.
*/

    wart();				/* Enter the state table switcher. */

/* Restore the communication line */
    
    ttclos();				/* Close the line. */
    if (server) {
	server = 0;
    	if (!quiet)  			/* Give appropriate message */
	    conoll("C-Kermit server done");
    } else
    	screen(BEL,0,"");		/* Or beep */
}
