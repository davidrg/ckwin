char *connv = "Connect Command for Data General, V4C(015) 11 May 87";

/*  C K U C O N  --  Dumb terminal connection to remote system, for Unix  */
/*
 This module should work under all versions of Unix.  It calls externally
 defined system-dependent functions for i/o, but depends upon the existence
 of the fork() function.

 Author: Frank da Cruz (SY.FDC@CU20B),
 Columbia University Center for Computing Activities, January 1985.
 Copyright (C) 1985, Trustees of Columbia University in the City of New York.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained. 

 Enhanced by H. Fischer to detect when child process (modem reader)
 reports that the communications line has been broken and hang up.
 Also enhanced to allow escaping from connect state to command
 interpreter, to allow sending/receiving without breaking connection.
  
 This module was adapted to the Data General computers by:

    Phil Julian, SAS Institute, Inc., Box 8000, Cary, NC 27512-8000
  
 The idea for conect(), talker(), and listener() (which is integral 
 to conect()) came from the Kermit for MV/UX, whose authors are:
 
    Jim Guyton, Rand Corporation
    Walter Underwood, Ford Aerospace
    David Ragozin, Univ. Washinton
    John Sambrook, Univ. Washington, Bioengineering.

*/

#include <stdio.h>
#include <ctype.h>			/* Character types */
#include "ckcdeb.h"
#include "ckcker.h"
#include <signal.h>
#include <setjmp.h>

#ifndef SIGUSR1
#define SIGUSR1 16
#endif

#ifdef datageneral
#include <sgtty.h>     	       /* Set/Get tty modes */
#include <multitask.h>                                                          
#include <packets:common.h>
#include <packets/normal_io.h>

/* Delimiter table -- defaults to CR only */
extern short idel_tbl[7] = {0x4,0,0,0,0,0,0};

/* Globals and defines for DG multi-tasking connect command */
void talker();                 /* talker sub-task */                        
#define  TALKID     17         /* task id of talker task */                   
#define  TALKPRI    200        /* priority of talker (was 2) */
#define  STACK      0          /* default stack size (was 1024) */

extern int ttyfd,ttyfdout;
#define MAXINBUF 2048
static int inbufsize = MAXINBUF;       /* current size of input buffer */
extern long times();
#endif datageneral

extern int local, speed, escape, duplex, parity, flow, seslog, mdmtyp;
extern int errno;

extern char ttname[], sesfil[];
extern char dopar();

int i, active;				/* variables global to this module */
char *chstr();
char temp[50];

#define LBUFL 200			/* line buffer */
char lbuf[LBUFL];

/* connect state parent/child communication signal handlers */

static jmp_buf env_con;			/* envir ptr for connect errors */

static
conn_int() {				/* modem read failure handler, */
    longjmp(env_con,1);			/* notifies parent process to stop */
}

/*  c o n e c t  --  perform terminal connection  */

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

/* condition console terminal and communication line */	    

    	if (conbin(escape) < 0) {
	    printf("Sorry, can't condition console terminal\n");
	    return(-2);
    	}
	if (ttvt(speed,flow) < 0) {
	    conres();
	    printf("Sorry, can't condition communication line\n");
	    return(-2);
    	}

/* cont'd... */

/* ...connect, cont'd */


#ifdef datageneral
                                                                                
/* The channel must be duplicated in order to allow simultaneous writes to
   it  from the 'talker' task.  Otherwise, the 'listener' hogs the channel
   and prevents any access.  ttopen() opens up two channels for this
   purpose.
   
   This code was adapted from the Kermit written for MV/UX, dgmvux.c .
   I have kept the same name as some of the original variables and some
   of the original comments (with a "**" line prefix).  But I have 
   changed the code alot.  I do appreciate the idea, and have listed 
   the names of the MV/UX Kermit authors:
   
        Jim Guyton, Rand Corporation
        Walter Underwood, Ford Aerospace
        David Ragozin, Univ. Washinton
        John Sambrook, Univ. Washington, Bioengineering.
        
*/
/*                                                                              
**    Under MV/UX a true fork() call does not exist; however, we can            
**    create multiple asynchronous tasks with a program.  We create             
**    two sub-tasks, ``listener'' and ``talker'' to handle the data             
**    communication.  For the main task to regain control one or both           
**    of the tasks must set the terminate flag; therefore it is cleared         
**    before either task is initiated.                                          
*/                                                                              
    active = 1;                                                              

    setto(channel(ttyfdout),60); /* Set timeouts to 60 seconds */
    setto(channel(ttyfd),2);     /* Set timeouts to 2 (not 10) seconds */
    
    mfinit(stdout,1);      /* Protect output to terminal screen */

    if (mtask(talker, STACK, TALKID, TALKPRI) != 0)                             
    {                                                                           
        perror("Can't initiate talker task.");
        return(-2);
    }                                                                           

    while (active) {
    	/* The original listener() task is inserted here.  This leaves the 
    	 * talker task as the only multiple task.
    	 */
        int i;
        char buffer[MAXINBUF+1];                /* I/O buffer for output */
        inbufsize = MAXINBUF-1;

        c = dgncinb(ttyfd,&buffer,inbufsize);
         
        if (c < 0) { /* Comm line hangup detected */
              if (errno == 9999)  /* Modem disconnected, or in progress */
                   printf("\r\nCommunications disconnect ");
              else {
                   /* Data overrun error or timeout */
                   if ((c == -EROVR) || (c == -2)) continue;
                   perror("\r\nCan't get character");
              }
              active=0;                         /* wait to die */
              continue;
        } /* c < 0 */
        buffer[c] = '\0';    /* Null terminate based on length of data */
        for (i = 0; i < c; i++) { buffer[i] &= 0177; }
        conxo(c,buffer);    	    	    	/* Output */
        if (seslog) zsoutx(ZSFILE,buffer,c);  	/* Log */
    }  /* while active */
                                                            
    printf("\r\n[Back at Local System]\n");
    mfinit(stdout,0);      /* Un-protect output to terminal screen */

#else
	parent_id = getpid();		/* get parent id for signalling */
	pid = fork();			/* All ok, make a fork */
	if (pid == -1) {
	    conres();			/* Reset the console. */
	    perror("Can't create keyboard fork");
	    printf("[Back at Local System]\n");
	    return(0);
	}
	    
	if (pid) {			
	  active = 1;			/* This fork reads, sends keystrokes */
	  if (!setjmp(env_con)) {	/* comm error in child process */
	    signal(SIGUSR1,conn_int);	/* routine for child process exit */
	    while (active) {
		c = coninc(0) & 0177;	/* Get character from keyboard */
		if (c == escape) {   	/* Look for escape char */
		    c = coninc(0) & 0177;   /* Got esc, get its arg */
		    doesc(c);		    /* And process it */
		} else {		/* Ordinary character */
		    if (ttoc(dopar(c)) > -1) {
		    	if (duplex) {	/* Half duplex? */
			    conoc(c);	/* Yes, also echo it. */
			    if (seslog) 	/* And maybe log it. */
			    	if (zchout(ZSFILE,c) < 0) seslog = 0;
			}
    	    	    } else {
			perror("\r\nCan't send character");
			active = 0;
		    }
		}
	      }
    	    }				/* Come here on death of child */
	    kill(pid,9);		/* Done, kill inferior fork. */

	    wait(0);			/* Wait till gone. */
	    conres();			/* Reset the console. */
	    printf("\r\n[Back at Local System]\n");
	    return(0);

	} else {			/* Inferior reads, prints port input */

	    while (1) {			/* Fresh read, wait for a character */
		if ((c = ttinc(0)) < 0) { /* Comm line hangup detected */
		    if (errno == 9999)	/* this value set by ckutio.c myread */
			 printf("\r\nCommunications disconnect ");
		    else perror("\r\nCan't get character");
		    kill(parent_id,SIGUSR1);	/* notify parent. */
		    pause();		/* Wait to be killed by parent. */
                }
		c &= 0177;		/* Got a char, strip parity, etc */
		conoc(c);		/* Put it on the screen. */
		if (seslog) zchout(ZSFILE,c);	/* If logging, log it. */
		while ((n = ttchk()) > 0) {	/* Any more left in buffer? */
		    if (n > LBUFL) n = LBUFL;   /* Get them all at once. */
		    if ((n = ttxin(n,lbuf)) > 0) {
			for (i = 0; i < n; i++) lbuf[i] &= 0177;   /* Strip */

			conxo(n,lbuf);	    	    	    	   /* Output */
			if (seslog) zsoutx(ZSFILE,lbuf,n);  	   /* Log */
		    }
	    	}
	    }
    	}
#endif
}

#ifdef datageneral

                                                                                
/**               
 **  t a l k e r 
 **                                                                             
 ** Special MV/UX sub-task to send keyboard characters to remote system.        
 **                                                                             
 **/
                                                                                
void talker()
{                                                                               
     int c;			/* c is a character, but must be signed 
				   integer to pass thru -1, which is the
				   modem disconnection signal, and is
				   different from the character 0377 */
     char ch;
     int n;
     
     /* Code was extracted from the 'child' process code in conect() */
     while (active) {
	c = coninc(0) & 0177;       /* Get character from keyboard */
        if (c == escape) {          /* Look for escape char */
	    c = coninc(0) & 0177;   /* Got esc, get its arg */
	    doesc(c);		    /* And process it */
	} else {		    /* Ordinary character */
             if (ttoc(dopar(c)) > -1) {
	    	 if (duplex) {      /* Half duplex? */
		     conoc(c);	    /* Yes, also echo it. */
                     if (seslog)    /* And maybe log it. */
			 if (zchout(ZSFILE,c) < 0) seslog = 0;
		 }
    	     } else {
		 perror("\r\nCan't send character");
		 active = 0;
	       }
          }
     }
     conres();			/* Reset the console. */
}                                                                   
#endif datageneral


/*  H C O N N E  --  Give help message for connect.  */

hconne() {
    int c;

    static char *hlpmsg[] = {"\
\r\nC to close the connection, or:",
"\r\n  0 (zero) to send a null",
"\r\n  B to send a BREAK",
"\r\n  H to hangup and close connection",
"\r\n  S for status",
"\r\n  ? for help",
"\r\n escape character twice to send the escape character.\r\n\r\n",
"" };

    conola(hlpmsg);			/* Print the help message. */
    conol("Command>");			/* Prompt for command. */
    c = coninc(0);
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
  
    c &= 0177;
    while (1) {
	if (c == escape) {		/* Send escape character */
	    d = dopar(c); ttoc(d); return;
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

	case 's':			/* Status */
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



