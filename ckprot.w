char *protv = "C-Kermit Protocol Module 4.2(015), 5 Mar 85"; /* -*-C-*- */

/* C K P R O T  -- C-Kermit Protocol Module, in Wart preprocessor notation. */

/* Authors: Jeff Damens, Bill Catchings, Frank da Cruz (Columbia University) */

#include "ckermi.h"
/*
 Note -- This file may also be preprocessed by the Unix Lex program, but 
 you must indent the above #include statement before using Lex, and then
 restore it to the left margin in the resulting C program before compilation.
 Also, the invocation of the "wart()" function below must be replaced by an
 invocation  of the "yylex()" function.  It might also be necessary to remove
 comments in the %%...%% section.
*/


/* State definitions for Wart (or Lex) */

%states ipkt rfile rdata ssinit ssdata sseof sseot serve generic get rgen


/* External C-Kermit variable declarations */

  extern char sstate, *versio, *srvtxt, *cmarg, *cmarg2;
  extern char data[], filnam[], srvcmd[], ttname[], *srvptr;
  extern int pktnum, timint, nfils, image, hcflg, xflg, speed, flow, mdmtyp;
  extern int prvpkt, cxseen, czseen, server, local, displa, bctu, bctr, quiet;
  extern int putsrv(), puttrm(), putfil(), errpkt();
  extern char *DIRCMD, *DELCMD, *TYPCMD, *SPACMD, *SPACM2, *WHOCMD;


/* Local variables */

  static char vstate = 0;  		/* Saved State   */
  static char vcmd = 0;    		/* Saved Command */
  static int x;				/* General-purpose integer */


/* Macros - Note, BEGIN is predefined by Wart (and Lex) */

#define SERVE  tinit(); BEGIN serve
#define RESUME if (server) { SERVE; } else return

%%
/* Protocol entry points, one for each start state (sstate) */

s { tinit();	    	    	    	/* Do Send command */
    if (sinit()) BEGIN ssinit;
       else RESUME; }

v { tinit(); BEGIN get; }                                       /* Receive */
r { tinit(); vstate = get;  vcmd = 0;   sipkt(); BEGIN ipkt; }	/* Get */
c { tinit(); vstate = rgen; vcmd = 'C'; sipkt(); BEGIN ipkt; }	/* Host */
g { tinit(); vstate = rgen; vcmd = 'G'; sipkt(); BEGIN ipkt; }	/* Generic */

x { SERVE; }	    	    	    	/* Be a Server */

/* Dynamic states: <current-states>input-character { action } */

<rgen,get,serve>S { rinit(data); bctu = bctr; BEGIN rfile; } /* Send-Init */

<ipkt>Y  { spar(data);			/* Get ack for I-packet */
    	   if (vcmd) { scmd(vcmd,cmarg); vcmd = 0; }
    	   if (vstate == get) srinit();
	   BEGIN vstate; }

<ipkt>E  { if (vcmd) scmd(vcmd,cmarg);	/* Get E for I-packet (ignore) */
    	   vcmd = 0; if (vstate == get) srinit();
	   BEGIN vstate; }

<serve>R { srvptr = srvcmd; decode(data,putsrv); /* Get Receive-Init */
	   cmarg = srvcmd;  nfils = -1;
    	   if (sinit()) BEGIN ssinit; else { SERVE; } }

<serve>I { spar(data); rpar(data); ack1(data);	 /* Get Init Parameters */
	   pktnum = 0; prvpkt = -1; }

<serve>G { srvptr = srvcmd; decode(data,putsrv); /* Get & decode command. */
	   putsrv('\0'); putsrv('\0');
	   sstate = srvcmd[0]; BEGIN generic; }

<serve>C { srvptr = srvcmd;		    	 /* Get command for shell */
	   decode(data,putsrv); putsrv('\0');
	   if (syscmd("",srvcmd)) BEGIN ssinit;
	   else { errpkt("Can't do shell command"); SERVE; } }

<serve>. { errpkt("Unimplemented server function"); SERVE; } /* Other */

<generic>C { if (!cwd(srvcmd+1)) errpkt("Can't change directory"); /* CWD */
    	     SERVE; }

<generic>D { if (syscmd(DIRCMD,srvcmd+2)) BEGIN ssinit;	/* Directory */
    	     else { errpkt("Can't list directory"); SERVE; } }

<generic>E { if (syscmd(DELCMD,srvcmd+2)) BEGIN ssinit;	/* Erase */
    	     else { errpkt("Can't remove file"); SERVE; } }

<generic>F { ack(); return(0); }    	/* Finish */
<generic>L { ack(); ttres(); return(kill(0,9)); } /* Bye, but no guarantee! */

<generic>H { if (sndhlp()) BEGIN ssinit;
    	     else { errpkt("Can't send help"); SERVE; } }

<generic>T { if (syscmd(TYPCMD,srvcmd+2)) BEGIN ssinit;
    	     else { errpkt("Can't type file"); SERVE; } }

<generic>U { x = *(srvcmd+1);			/* Disk Usage query */
    	     x = ((x == '\0') || (x == unchar(0)));
	     x = (x ? syscmd(SPACMD,"") : syscmd(SPACM2,srvcmd+2));
    	     if (x) BEGIN ssinit; else { errpkt("Can't check space"); SERVE; }}

<generic>W { if (syscmd(WHOCMD,srvcmd+2)) BEGIN ssinit;
    	     else { errpkt("Can't do who command"); SERVE; } }

<generic>. { errpkt("Unimplemented generic server function"); SERVE; }

/* Dynamic states, cont'd */


<rgen>Y { decode(data,puttrm); RESUME; }    /* Got reply in ACK data */

<rgen,rfile>F { if (rcvfil()) { ack(); BEGIN rdata; }	/* A file is coming */
		else { errpkt("Can't open file"); RESUME; } }

<rgen,rfile>X { opent(); ack(); BEGIN rdata; }	/* Screen data is coming */

<rfile>B { ack(); reot(); RESUME; }	/* Got End Of Transmission */

<rdata>D { if (cxseen) ack1("X");	/* Got data. */
    	   else if (czseen) ack1("Z");
	   else ack();
	   decode(data,putfil); }

<rdata>Z { ack(); reof(); BEGIN rfile; }    /* Got End Of File */

<ssinit,ssdata,sseof,sseot>N { resend(); }  /* Got a NAK, resend. */

<ssinit>Y {  int x; char *s;		/* Got ACK to Send-Init */
    	     spar(data);
    	     bctu = bctr;
	     if (xflg) { x = sxpack(); s = "Can't execute command"; }
	    	  else { x = sfile(); s = "Can't open file"; }
	     if (x) BEGIN ssdata; else { errpkt(s); RESUME; }
          }	    

<ssdata>Y { if (canned(data) || !sdata()) { /* Got ACK to data */
		clsif(); seof();
		BEGIN sseof; } }

<sseof>Y { if (gnfile() > 0) {		/* Got ACK to EOF, get next file */
		if (sfile()) BEGIN ssdata;
		else { errpkt("Can't open file") ; RESUME; }
	   } else {			/* If no next file, EOT */
		seot();
		BEGIN sseot; } }

<sseot>Y { RESUME; }			/* Got ACK to EOT */

E { ermsg(data);			/* Error packet, issue message */
    x = quiet; quiet = 1;		/* Close files silently */
    clsif(); clsof();
    quiet = x; RESUME; }

. { nack(); }				/* Anything else, send NAK */
%%

/*  P R O T O  --  Protocol entry function  */

proto() {

    extern int sigint();
    int x;

    conint(sigint);			/* Enable console interrupts */

/* Set up the communication line for file transfer. */

    if (local && (speed < 0)) {
	screen(2,0l,"Sorry, you must 'set speed' first");
	return;
    }
    if (ttopen(ttname,local,mdmtyp) < 0) {
	screen(2,0l,"Can't open line");
	return;
    }
    x = (local) ? speed : -1;
    if (ttpkt(x,flow) < 0) {		/* Put line in packet mode, */
	screen(2,0l,"Can't condition line"); /* setting speed, flow control */
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
    
    if (server) {			/* Back from packet protocol. */
	server = 0;
    	if (!quiet)  			/* Give appropriate message */
	    conoll("C-Kermit server done");
    } else
    	screen(BEL,0l,"");		/* Or beep */
}
