static char *sccsid = "@(#)Kermit version 3.0(0) 8/1/84";
/*
 *  K e r m i t	 File Transfer Utility
 *
 *  UNIX Kermit, Columbia University, 1981, 1982, 1983, 1984
 *	Bill Catchings, Bob Cattani, Chris Maio, Frank da Cruz,
 *	Alan Crosswell, Jeff Damens
 *
 *  Also:   Jim Guyton, Rand Corporation
 *	    Walter Underwood, Ford Aerospace
 *
 *  usage:  kermit c [lbphe line baud par escape-char]	to connect
 *	    kermit s [ddiflbpt line baud par] file ...	to send files
 *	    kermit r [ddiflbpt line baud par]		to receive files
 *
 *  where   c=connect, s=send, r=receive,
 *	    d=debug, i=image mode, f=no filename conversion, l=tty line,
 *	    b=baud rate, e=escape char, h=half duplex, t=turnaround char,
 *	    p=parity, z=log packet transactions.
 *
 *  For remote Kermit, format is either:
 *	    kermit r					to receive files
 *  or	    kermit s file ...				to send files
 *
 */

/*
 *  Modification History:
 *
 *  July 84 Bill Catchings and Jeff Damens - Add necessary commands for
 *	communicating with IBM mainframes (t,p,h).  Also started to
 *	make it more modular.  Eventually all conditionals should be
 *	removed.  After that we will go to a LEX version which will
 *	implement all the states needed for a full server version.  A
 *	command parser is also needed.  Limited VAX VMS support was also
 *	added (no connect command).  Link together KERMIT.C, KER%%%.C
 *	(UNX for all present UNIX versions, VMS for VAX VMS) and KERCN%.C
 *	(V for Venix on the Pro and U for UNIX this module not used for
 *	VMS).
 *
 *  May 21 84 - Roy Smith (CUCS), strip parity from checksum in rpack()
 *
 *  Oct. 17 Included fixes from Alan Crosswell (CUCCA) for IBM_UTS:
 *	    - Changed MYEOL character from \n to \r.
 *	    - Change char to int in bufill so getc would return -1 on
 *	      EOF instead of 255 (-1 truncated to 8 bits)
 *	    - Added read() in rpack to eat the EOL character
 *	    - Added fflush() call in printmsg to force the output
 *	    NOTE: The last three changes are not conditionally compiled
 *		  since they should work equally well on any system.
 *
 *	    Changed Berkeley 4.x conditional compilation flag from
 *		UNIX4X to UCB4X.
 *	    Added support for error packets and cleaned up the printing
 *		routines.
 */

/* Conditional compilation for different machines/operating systems */
/* One and only one of the following lines should be 1 */

#define UCB4X	    1	    /* Berkeley 4.x UNIX */
#define TOPS_20	    0	    /* TOPS-20 */
#define IBM_UTS	    0	    /* Amdahl UTS on IBM systems */
#define VAX_VMS	    0	    /* VAX/VMS */

/* Conditional compilation for the different Unix variants */
/* 0 means don't compile it, nonzero means do */

#if UCB4X
#define V6_LIBS	    0	    /* Dont't use retrofit libraries */
#define NO_FIONREAD 0	    /* We have ioctl(FIONREAD,...) for flushinput() */
#define NO_TANDEM   0	    /* We have TANDEM line discipline (xon/xoff) */
#endif

#if IBM_UTS
#define V6_LIBS	    0	    /* Don't use retrofit libraries */
#define NO_FIONREAD 1	    /* No ioctl(FIONREAD,...) for flushinput() */
#define NO_TANDEM   1	    /* No TANDEM line discipline (xon/xoff) */
#endif

#if TOPS_20
#define V6_LIBS	    0	    /* Don't use retrofit libraries */
#define NO_FIONREAD 1	    /* No ioctl(FIONREAD,...) for flushinput() */
#define NO_TANDEM   1	    /* No TANDEM line discipline (xon/xoff) */
#endif

#if VAX_VMS
#define V6_LIBS	    0	    /* Don't use retrofit libraries */
#define NO_FIONREAD 1	    /* No ioctl(FIONREAD,...) for flushinput() */
#define NO_TANDEM   1	    /* No TANDEM line discipline (xon/xoff) */
#endif

#include <stdio.h>	    /* Standard UNIX definitions */
#include <ctype.h>

#if V6_LIBS
#include <retrofit/sgtty.h>
#include <retrofit/signal.h>
#include <retrofit/setjmp.h>
#else
#include <signal.h>
#include <setjmp.h>
#endif

#if !(V6_LIBS | VAX_VMS)
#include <sgtty.h>
#endif

#if NO_TANDEM
#define TANDEM	    0	    /* define it to be nothing if it's unsupported */
#endif


/* Symbol Definitions */

#define MAXPACKSIZ  94	    /* Maximum packet size */
#define SOH	    1	    /* Start of header */
#define CR	    13	    /* ASCII Carriage Return */
#define SP	    32	    /* ASCII space */
#define DEL	    127	    /* Delete (rubout) */
#define XON	    17	    /* ASCII XON */

#define MAXTRY	    10	    /* Times to retry a packet */
#define MYQUOTE	    '#'	    /* Quote character I will use */
#define MYPAD	    0	    /* Number of padding characters I will need */
#define MYPCHAR	    0	    /* Padding character I need (NULL) */

#define MYTIME	    10	    /* Seconds after which I should be timed out */
#define MAXTIM	    60	    /* Maximum timeout interval */
#define MINTIM	    2	    /* Minumum timeout interval */

#define TRUE	    -1	    /* Boolean constants */
#define FALSE	    0

#define DEFESC	    '^'	    /* Default escape character for CONNECT */
#define DEFTRN	    FALSE   /* Default turn around */
#define DEFLCH	    FALSE   /* Default local echo */
#define DEFPAR	    FALSE   /* Default parity */
#define DEFIM	    FALSE   /* Default image mode */
#if UCB4X | VAX_VMS | IBM_UTS
#define DEFFNC	    TRUE    /* Default file name conversion */
#else
#define DEFFNC	    FALSE
#endif
#if IBM_UTS | VAX_VMS
#define MYEOL	    '\r'    /* End-Of-Line character for UTS or VMS systems */
#else
#define MYEOL	    '\n'    /* End-Of-Line character I need */
#endif




/* Macro Definitions */

/*
 * tochar: converts a control character to a printable one by adding a space.
 *
 * unchar: undoes tochar.
 *
 * ctl:	   converts between control characters and printable characters by
 *	   toggling the control bit (ie. ^A becomes A and A becomes ^A).
 *
 * unpar:  turns off the parity bit.
 */

#define tochar(ch)  ((ch) + ' ')
#define unchar(ch)  ((ch) - ' ')
#define ctl(ch)	    ((ch) ^ 64 )
#define unpar(ch)   ((ch) & 127)

/* Conditional Global Variables */

#if VAX_VMS
int	confd,			/* FD of the controlling terminal */
	ttnam,			/* Line name pointer */
	oldtty[4],
	oldcon[4];
#else
struct	sgttyb
	oldtty;			/* Controlling tty raw mode */
#endif

/* Global Variables */

int	size,		    /* Size of present data */
	rpsiz,		    /* Maximum receive packet size */
	spsiz,		    /* Maximum send packet size */
	pad,		    /* How much padding to send */
	timint,		    /* Timeout for foreign host on sends */
	n,		    /* Packet number */
	numtry,		    /* Times this packet retried */
	oldtry,		    /* Times previous packet retried */
	ttyfd,		    /* File descriptor of tty for I/O, 0 if remote */
	remote,		    /* -1 means we're a remote kermit */
	image,		    /* -1 means 8-bit mode */
	parflg,		    /* TRUE means use parity specified */
	turn,		    /* TRUE means look for turnaround char (XON) */
	lecho,		    /* TRUE for locally echo chars in connect mode */
	debug,		    /* Indicates level of debugging output (0=none) */
	pktdeb,		    /* TRUE means log all packet to a file */
	filnamcnv,	    /* -1 means do file name case conversions */
	speed,		    /* speed to set */
	filecount;	    /* Number of files left to send */

char	state,		    /* Present state of the automaton */
	cchksum,	    /* Our (computed) checksum */
	padchar,	    /* Padding character to send */
	eol,		    /* End-Of-Line character to send */
	escchr,		    /* Connect command escape character */
	quote,		    /* Quote character in incoming data */
	**filelist,	    /* List of files to be sent */
	*filnam,	    /* Current file name */
	recpkt[MAXPACKSIZ], /* Receive packet buffer */
	packet[MAXPACKSIZ]; /* Packet buffer */

FILE	*fp,		    /* File pointer for current disk file */
	*dpfp;		    /* File pointer for debugging packet log file */

jmp_buf env;		    /* Environment ptr for timeout longjump */


/*
 *  m a i n
 *
 *  Main routine - parse command and options, set up the
 *  tty lines, and dispatch to the appropriate routine.
 */

main(argc,argv)
int argc;			    /* Character pointers to and count of */
char **argv;				/* command line arguments */
{
    char *ttyname,			/* tty name for LINE argument */
	*cp;				/* char pointer */
    int cflg, rflg, sflg;		/* flags for CONNECT, RECEIVE, SEND */

    if (argc < 2) usage();		/* Make sure there's a command line */

    cp = *++argv; argv++; argc -= 2;	/* Set up pointers to args */

/*  Initialize these values and hope the first packet will get across OK */

    eol = CR;				/* EOL for outgoing packets */
    quote = '#';			/* Standard control-quote char "#" */
    pad = 0;				/* No padding */
    padchar = NULL;			/* Use null if any padding wanted */

    speed = cflg = sflg = rflg = 0;	/* Turn off all parse flags */
    ttyname = 0;			/* Default is remote mode */

    pktdeb = FALSE;			/* No packet file debugging */
    turn = DEFTRN;			/* Default turnaround */
    lecho = DEFLCH;			/* Default local echo */
    parflg = DEFPAR;			/* Default parity */
    image = DEFIM;			/* Default image mode */
    filnamcnv = DEFFNC;			/* Default filename case conversion */

    escchr = DEFESC;			/* Default escape character */
 
    while ((*cp) != NULL)		/* Parse characters in first arg. */
	switch (*cp++)
	{
	    case 'c': cflg++; break;	/* C = Connect command */
	    case 's': sflg++; break;	/* S = Send command */
	    case 'r': rflg++; break;	/* R = Receive command */

	    case 'd':			/* D = Increment debug mode count */
		debug++; break;
		
	    case 'f':
		filnamcnv = FALSE;	/* F = don't do case conversion */
		break;			/*     on filenames */

	    case 'i':			/* I = Image (8-bit) mode */
		image = TRUE; break;

	    case 'h':			/* H = Half duplex mode */
		lecho = TRUE; break;

	    case 't':			/* T = Turnaround character (XON) */
		turn = TRUE; break;

	    case 'l':			/* L = specify tty line to use */
		if (argc--) ttyname = *argv++;
		else usage(); 
		if (debug) printf("Line to remote host is %s\n",ttyname); 
		break;
		
	    case 'e':			/* E = specify escape char */
		if (argc--) escchr = **argv++;
		else usage();
		if (debug) printf("Escape char is \"%c\"\n",escchr);
		break;
		
	    case 'p':			/* P = specify parity */
		if (argc--)
		    switch ((parflg = **argv++))
                    {
			case 'n':  parflg = FALSE; break; /* None, no parity */
			case 'e':
			case 'm':
			case 'o':
			case 's':  break;
			default:   parflg = FALSE; usage(); 
		    }
		else usage();
		if (debug) printf("Parity is %c\n",parflg);
		break;
		
	    case 'z':			/* Use packet file debugging */
		pktdeb = TRUE; break;

/* Move conditional commands to expansion command table */

	    case 'b':			/* B = specify baud rate */
#if UCB4X
		if (argc--) speed = atoi(*argv++);
		else usage();
		if (debug) printf("Line speed to remote host is %d\n",speed);
		break;
#else
		printmsg("Speed setting implemented for Unix only.");
		exit(1);
#endif
	    default: usage();
	}

/* Done parsing */

    if ((cflg+sflg+rflg) != 1)		/* Only one command allowed */
	usage();

    if (ttyname)			/* If LINE was specified, we */
    {					/* operate in local mode */
	ttyfd = ttopen(ttyname);	/* Open the tty line */
	ttbin(ttyfd,&oldtty);		/* Put the tty in binary mode */
	remote = FALSE;			/* Indicate we're in local mode */
    }
    else				/* No LINE specified so we operate */
    {					/* in remote mode (ie. controlling */
	ttyfd = ttopen(0);		/* tty is the communications line) */
	ttbin(ttyfd,&oldtty);		/* Put the tty in binary mode */
	remote = TRUE;
    }
    
/* All set up, now execute the command that was given. */

    if (!remote) printf("%s\n",sccsid+4);	/* print version # */
    if (pktdeb)				/* Open packet file if requested */
	if ((dpfp=fopen("PACKET.LOG","w"))==NULL)
	{
	    if (debug) printf("Cannot create PACKET.LOG\n");
	    pktdeb = FALSE;		/* Say file not being used */
	}

    if (debug)
    {
	printf("Debugging level = %d\n\n",debug);
	if (pktdeb) printf("Logging all packets to PACKET.LOG\n\n");

	if (cflg) printf("Connect command\n\n");
	if (sflg) printf("Send command\n\n");
	if (rflg) printf("Receive command\n\n");
    }
  
    if (cflg) connect();		/* Connect command */

    if (sflg)				/* Send command */ 
    {
	if (argc--) filnam = *argv++;	/* Get file to send */
	else
	{
	    ttres(ttyfd,&oldtty);	/* Restore controlling tty's modes */
	    usage();			/* and give error */
	}
	fp = NULL;			/* Indicate no file open yet */
	filelist = argv;		/* Set up the rest of the file list */
	filecount = argc;		/* Number of files left to send */
	if (sendsw() == FALSE)		/* Send the file(s) */
	    printmsg("Send failed.");	/* Report failure */
	else				/*  or */
	    printmsg("done.");		/* success */
    }

    if (rflg)				/* Receive command */
    {
	if (recsw() == FALSE)		/* Receive the file(s) */
	    printmsg("Receive failed.");
	else				/* Report failure */
	    printmsg("done.");		/* or success */
    }
    ttres(ttyfd,&oldtty);		/* Restore controlling tty's modes */
    if (pktdeb) fclose(dpfp);		/* Close the debug file */
}


/*
 *  s e n d s w
 *
 *  Sendsw is the state table switcher for sending files.  It loops until
 *  either it finishes, or an error is encountered.  The routines called
 *  by sendsw are responsible for changing the state.
 *
 */

sendsw()
{
    char sinit(), sfile(), sdata(), seof(), sbreak();

    state = 'S';			/* Send initiate is the start state */
    n = 0;				/* Initialize message number */
    numtry = 0;				/* Say no tries yet */
    if (remote) sleep(MYTIME);		/* Sleep to give the guy a chance */
    while(TRUE)				/* Do this as long as necessary */
    {
	if (debug) printf("sendsw state: %c\n",state);
	switch(state)
	{
	    case 'S':	state = sinit();  break; /* Send-Init */
	    case 'F':	state = sfile();  break; /* Send-File */
	    case 'D':	state = sdata();  break; /* Send-Data */
	    case 'Z':	state = seof();	  break; /* Send-End-of-File */
	    case 'B':	state = sbreak(); break; /* Send-Break */
	    case 'C':	return (TRUE);		 /* Complete */
	    case 'A':	return (FALSE);		 /* "Abort" */
	    default:	return (FALSE);		 /* Unknown, fail */
	}
    }
}


/*
 *  s i n i t
 *
 *  Send Initiate: send this host's parameters and get other side's back.
 */

char sinit()
{
    int num, len;			/* Packet number, length */

if (debug) printf("In sinit retries: %d\n",numtry);

    if (numtry++ > MAXTRY) return('A'); /* If too many tries, give up */
    spar(packet);			/* Fill up init info packet */

    flushinput();			/* Flush pending input */

    spack('S',n,6,packet);		/* Send an S packet */
    switch(rpack(&len,&num,recpkt))	/* What was the reply? */
    {
	case 'N':  return(state);	/* NAK, try it again */

	case 'Y':			/* ACK */
	    if (n != num)		/* If wrong ACK, stay in S state */
		return(state);		/* and try again */
	    rpar(recpkt);		/* Get other side's init info */

	    if (eol == 0) eol = '\n';	/* Check and set defaults */
	    if (quote == 0) quote = '#';

	    numtry = 0;			/* Reset try counter */
	    n = (n+1)%64;		/* Bump packet count */
	    return('F');		/* OK, switch state to F */

	case 'E':			/* Error packet received */
	    prerrpkt(recpkt);		/* Print it out and */
	    return('A');		/* abort */

	case FALSE: return(state);	/* Receive failure, try again */

	default: return('A');		/* Anything else, just "abort" */
   }
 }


/*
 *  s f i l e
 *
 *  Send File Header.
 */

char sfile()
{
    int num, len;			/* Packet number, length */
    char filnam1[50],			/* Converted file name */
	*newfilnam,			/* Pointer to file name to send */
	*cp;				/* char pointer */

    if (numtry++ > MAXTRY) return('A'); /* If too many tries, give up */
    
    if (fp == NULL)			/* If not already open, */
    {	if (debug) printf("   Opening %s for sending.\n",filnam);
	fp = fopen(filnam,"r");		/* open the file to be sent */
	if (fp == NULL)			/* If bad file pointer, give up */
	{
	    error("Cannot open file %s",filnam);
	    return('A');
	}
    }

    strcpy(filnam1, filnam);		/* Copy file name */
    newfilnam = cp = filnam1;
    while (*cp != '\0')			/* Strip off all leading directory */
	if (*cp++ == '/')		/* names (ie. up to the last /). */
	    newfilnam = cp;

    if (filnamcnv)			/* Convert lower case to upper	*/
	for (cp = newfilnam; *cp != '\0'; cp++)
	    if (islower(*cp)) *cp = toupper(*cp);

    len = cp - newfilnam;		/* Compute length of new filename */

    printmsg("Sending %s as %s",filnam,newfilnam);

    spack('F',n,len,newfilnam);		/* Send an F packet */
    switch(rpack(&len,&num,recpkt))	/* What was the reply? */
    {			
	case 'N':			/* NAK, just stay in this state, */
	    num = (--num<0 ? 63:num);	/* unless it's NAK for next packet */
	    if (n != num)		/* which is just like an ACK for */ 
		return(state);		/* this packet so fall thru to... */

	case 'Y':			/* ACK */
	    if (n != num) return(state); /* If wrong ACK, stay in F state */
	    numtry = 0;			/* Reset try counter */
	    n = (n+1)%64;		/* Bump packet count */
	    size = bufill(packet);	/* Get first data from file */
	    return('D');		/* Switch state to D */

	case 'E':			/* Error packet received */
	    prerrpkt(recpkt);		/* Print it out and */
	    return('A');		/* abort */

	case FALSE: return(state);	/* Receive failure, stay in F state */

	default:    return('A');	/* Something else, just "abort" */
    }
}


/*
 *  s d a t a
 *
 *  Send File Data
 */

char sdata()
{
    int num, len;			/* Packet number, length */

    if (numtry++ > MAXTRY) return('A'); /* If too many tries, give up */

    spack('D',n,size,packet);		/* Send a D packet */
    switch(rpack(&len,&num,recpkt))	/* What was the reply? */
    {		    
	case 'N':			/* NAK, just stay in this state, */
	    num = (--num<0 ? 63:num);	/* unless it's NAK for next packet */
	    if (n != num)		/* which is just like an ACK for */
		return(state);		/* this packet so fall thru to... */
		
	case 'Y':			/* ACK */
	    if (n != num) return(state); /* If wrong ACK, fail */
	    numtry = 0;			/* Reset try counter */
	    n = (n+1)%64;		/* Bump packet count */
	    if ((size = bufill(packet)) == EOF) /* Get data from file */
		return('Z');		/* If EOF set state to that */
	    return('D');		/* Got data, stay in state D */

	case 'E':			/* Error packet received */
	    prerrpkt(recpkt);		/* Print it out and */
	    return('A');		/* abort */

	case FALSE: return(state);	/* Receive failure, stay in D */

	default:    return('A');	/* Anything else, "abort" */
    }
}


/*
 *  s e o f
 *
 *  Send End-Of-File.
 */

char seof()
{
    int num, len;			/* Packet number, length */
    if (numtry++ > MAXTRY) return('A'); /* If too many tries, "abort" */

    spack('Z',n,0,packet);		/* Send a 'Z' packet */
    switch(rpack(&len,&num,recpkt))	/* What was the reply? */
    {
	case 'N':			/* NAK, just stay in this state, */
	    num = (--num<0 ? 63:num);	/* unless it's NAK for next packet, */
	    if (n != num)		/* which is just like an ACK for */
		return(state);		/* this packet so fall thru to... */

	case 'Y':			/* ACK */
	    if (n != num) return(state); /* If wrong ACK, hold out */
	    numtry = 0;			/* Reset try counter */
	    n = (n+1)%64;		/* and bump packet count */
	    if (debug) printf("	  Closing input file %s, ",filnam);
	    fclose(fp);			/* Close the input file */
	    fp = NULL;			/* Set flag indicating no file open */ 

	    if (debug) printf("looking for next file...\n");
	    if (gnxtfl() == FALSE)	/* No more files go? */
		return('B');		/* if not, break, EOT, all done */
	    if (debug) printf("	  New file is %s\n",filnam);
	    return('F');		/* More files, switch state to F */

	case 'E':			/* Error packet received */
	    prerrpkt(recpkt);		/* Print it out and */
	    return('A');		/* abort */

	case FALSE: return(state);	/* Receive failure, stay in Z */

	default:    return('A');	/* Something else, "abort" */
    }
}


/*
 *  s b r e a k
 *
 *  Send Break (EOT)
 */

char sbreak()
{
    int num, len;			/* Packet number, length */
    if (numtry++ > MAXTRY) return('A'); /* If too many tries "abort" */

    spack('B',n,0,packet);		/* Send a B packet */
    switch (rpack(&len,&num,recpkt))	/* What was the reply? */
    {
	case 'N':			/* NAK, just stay in this state, */
	    num = (--num<0 ? 63:num);	/* unless NAK for previous packet, */
	    if (n != num)		/* which is just like an ACK for */
		return(state);		/* this packet so fall thru to... */

	case 'Y':			/* ACK */
	    if (n != num) return(state); /* If wrong ACK, fail */
	    numtry = 0;			/* Reset try counter */
	    n = (n+1)%64;		/* and bump packet count */
	    return('C');		/* Switch state to Complete */

	case 'E':			/* Error packet received */
	    prerrpkt(recpkt);		/* Print it out and */
	    return('A');		/* abort */

	case FALSE: return(state);	/* Receive failure, stay in B */

	default:    return ('A');	/* Other, "abort" */
   }
}


/*
 *  r e c s w
 *
 *  This is the state table switcher for receiving files.
 */

recsw()
{
    char rinit(), rfile(), rdata();	/* Use these procedures */

    state = 'R';			/* Receive-Init is the start state */
    n = 0;				/* Initialize message number */
    numtry = 0;				/* Say no tries yet */

    while(TRUE)
    {
	if (debug) printf(" recsw state: %c\n",state);
	switch(state)			/* Do until done */
	{
	    case 'R':	state = rinit(); break; /* Receive-Init */
	    case 'F':	state = rfile(); break; /* Receive-File */
	    case 'D':	state = rdata(); break; /* Receive-Data */
	    case 'C':	return(TRUE);		/* Complete state */
	    case 'A':	return(FALSE);		/* "Abort" state */
	}
    }
}

    
/*
 *  r i n i t
 *
 *  Receive Initialization
 */
  
char rinit()
{
    int len, num;			/* Packet length, number */

    if (numtry++ > MAXTRY) return('A'); /* If too many tries, "abort" */

    switch(rpack(&len,&num,packet))	/* Get a packet */
    {
	case 'S':			/* Send-Init */
	    rpar(packet);		/* Get the other side's init data */
	    spar(packet);		/* Fill up packet with my init info */
	    spack('Y',n,6,packet);	/* ACK with my parameters */
	    oldtry = numtry;		/* Save old try count */
	    numtry = 0;			/* Start a new counter */
	    n = (n+1)%64;		/* Bump packet number, mod 64 */
	    return('F');		/* Enter File-Receive state */

	case 'E':			/* Error packet received */
	    prerrpkt(recpkt);		/* Print it out and */
	    return('A');		/* abort */

	case FALSE:			/* Didn't get packet */
	    spack('N',n,0,0);		/* Return a NAK */
	    return(state);		/* Keep trying */

	default:     return('A');	/* Some other packet type, "abort" */
    }
}


/*
 *  r f i l e
 *
 *  Receive File Header
 */

char rfile()
{
    int num, len;			/* Packet number, length */
    char filnam1[50];			/* Holds the converted file name */

    if (numtry++ > MAXTRY) return('A'); /* "abort" if too many tries */

    switch(rpack(&len,&num,packet))	/* Get a packet */
    {
	case 'S':			/* Send-Init, maybe our ACK lost */
	    if (oldtry++ > MAXTRY) return('A'); /* If too many tries "abort" */
	    if (num == ((n==0) ? 63:n-1)) /* Previous packet, mod 64? */
	    {				/* Yes, ACK it again with  */
		spar(packet);		/* our Send-Init parameters */
		spack('Y',num,6,packet);
		numtry = 0;		/* Reset try counter */
		return(state);		/* Stay in this state */
	    }
	    else return('A');		/* Not previous packet, "abort" */

	case 'Z':			/* End-Of-File */
	    if (oldtry++ > MAXTRY) return('A');
	    if (num == ((n==0) ? 63:n-1)) /* Previous packet, mod 64? */
	    {				/* Yes, ACK it again. */
		spack('Y',num,0,0);
		numtry = 0;
		return(state);		/* Stay in this state */
	    }
	    else return('A');		/* Not previous packet, "abort" */

	case 'F':			/* File Header (just what we want) */
	    if (num != n) return('A');	/* The packet number must be right */
	    strcpy(filnam1, packet);	/* Copy the file name */

	    if (filnamcnv)		/* Convert upper case to lower */
		for (filnam=filnam1; *filnam != '\0'; filnam++)
		    if (isupper(*filnam)) *filnam = tolower(*filnam);


	    if ((fp=fopen(filnam1,"w"))==NULL) /* Try to open a new file */
	    {
		error("Cannot create %s",filnam1); /* Give up if can't */
		return('A');
	    }
	    else			/* OK, give message */
		printmsg("Receiving %s as %s",packet,filnam1);

	    spack('Y',n,0,0);		/* Acknowledge the file header */
	    oldtry = numtry;		/* Reset try counters */
	    numtry = 0;			/* ... */
	    n = (n+1)%64;		/* Bump packet number, mod 64 */
	    return('D');		/* Switch to Data state */

	case 'B':			/* Break transmission (EOT) */
	    if (num != n) return ('A'); /* Need right packet number here */
	    spack('Y',n,0,0);		/* Say OK */
	    return('C');		/* Go to complete state */

	case 'E':			/* Error packet received */
	    prerrpkt(recpkt);		/* Print it out and */
	    return('A');		/* abort */

	case FALSE:			/* Didn't get packet */
	    spack('N',n,0,0);		/* Return a NAK */
	    return(state);		/* Keep trying */

	default:    return ('A');	/* Some other packet, "abort" */
    }
}


/*
 *  r d a t a
 *
 *  Receive Data
 */

char rdata()
{
    int num, len;			/* Packet number, length */
    if (numtry++ > MAXTRY) return('A'); /* "abort" if too many tries */

    switch(rpack(&len,&num,packet))	/* Get packet */
    {
	case 'D':			/* Got Data packet */
	    if (num != n)		/* Right packet? */
	    {				/* No */
		if (oldtry++ > MAXTRY)
		    return('A');	/* If too many tries, abort */
		if (num == ((n==0) ? 63:n-1)) /* Else check packet number */
		{			/* Previous packet again? */
		    spack('Y',num,6,packet); /* Yes, re-ACK it */
		    numtry = 0;		/* Reset try counter */
		    return(state);	/* Don't write out data! */
		}
		else return('A');	/* sorry, wrong number */
	    }
	    /* Got data with right packet number */
	    bufemp(packet,len);		/* Write the data to the file */
	    spack('Y',n,0,0);		/* Acknowledge the packet */
	    oldtry = numtry;		/* Reset the try counters */
	    numtry = 0;			/* ... */
	    n = (n+1)%64;		/* Bump packet number, mod 64 */
	    return('D');		/* Remain in data state */

	case 'F':			/* Got a File Header */
	    if (oldtry++ > MAXTRY)
		return('A');		/* If too many tries, "abort" */
	    if (num == ((n==0) ? 63:n-1)) /* Else check packet number */
	    {				/* It was the previous one */
		spack('Y',num,0,0);	/* ACK it again */
		numtry = 0;		/* Reset try counter */
		return(state);		/* Stay in Data state */
	    }
	    else return('A');		/* Not previous packet, "abort" */

	case 'Z':			/* End-Of-File */
	    if (num != n) return('A');	/* Must have right packet number */
	    spack('Y',n,0,0);		/* OK, ACK it. */
	    fclose(fp);			/* Close the file */
	    n = (n+1)%64;		/* Bump packet number */
	    return('F');		/* Go back to Receive File state */

	case 'E':			/* Error packet received */
	    prerrpkt(recpkt);		/* Print it out and */
	    return('A');		/* abort */

	case FALSE:			/* Didn't get packet */
	    spack('N',n,0,0);		/* Return a NAK */
	    return(state);		/* Keep trying */

	default:     return('A');	/* Some other packet, "abort" */
    }
}


/*
 *	KERMIT utilities.
 */

dopar (ch)
char ch;
{
    int a;

    if (!parflg) return(ch);			/* false, no parity */
    ch &= 0177;
    switch (parflg)
    {
	case 'm':  return(ch | 128);		/* Mark */
	case 's':  return(ch & 127);		/* Space */
	case 'o':  ch |= 128;			/* Odd */
	case 'e':				/* Even */
	    a = (ch & 15) ^ ((ch >> 4) & 15);
	    a = (a & 3) ^ ((a >> 2) & 3);
	    a = (a & 1) ^ ((a >> 1) & 1);
	    return((ch & 0177) | (a << 7));
	default:   return(ch);
    }
}


clkint()				/* Timer interrupt handler */
{
    longjmp(env,TRUE);			/* Tell rpack to give up */
}


/*
 *  s p a c k
 *
 *  Send a Packet
 */

spack(type,num,len,data)
char type, *data;
int num, len;
{
    int i;				/* Character loop counter */
    char chksum, buffer[100];		/* Checksum, packet buffer */
    register char *bufp;		/* Buffer pointer */

    if (debug>1)			/* Display outgoing packet */
    {
	if (data != NULL)
	    data[len] = '\0';		/* Null-terminate data to print it */
	printf("  spack type:  %c\n",type);
	printf("	 num:  %d\n",num);
	printf("	 len:  %d\n",len);
	if (data != NULL)
	    printf("	data: \"%s\"\n",data);
    }
  
    bufp = buffer;			/* Set up buffer pointer */
    for (i=1; i<=pad; i++) write(ttyfd,&padchar,1); /* Issue any padding */

    *bufp++ = dopar(SOH);		/* Packet marker, ASCII 1 (SOH) */
    *bufp++ = dopar(tochar(len+3));	/* Send the character count */
    chksum  = tochar(len+3);		/* Initialize the checksum */
    *bufp++ = dopar(tochar(num));	/* Packet number */
    chksum += tochar(num);		/* Update checksum */
    *bufp++ = dopar(type);		/* Packet type */
    chksum += type;			/* Update checksum */

    for (i=0; i<len; i++)		/* Loop for all data characters */
    {
	*bufp++ = dopar(data[i]);	/* Get a character */
	chksum += data[i];		/* Update checksum */
    }
    chksum = (((chksum&0300) >> 6)+chksum)&077; /* Compute final checksum */
    *bufp++ = dopar(tochar(chksum));	/* Put it in the packet */
    *bufp++ = dopar(eol);		/* Extra-packet line terminator */
    if (pktdeb)
    {
	*bufp = '\0';			/* Terminate the string */
	fprintf(dpfp,"\nSpack:%s",buffer);
    }
    write(ttyfd,buffer,bufp-buffer);	/* Send the packet */
}

/*
 *  r p a c k
 *
 *  Read a Packet
 */

rpack(len,num,data)
int *len, *num;				/* Packet length, number */
char *data;				/* Packet data */
{
    int i, done;			/* Data character number, loop exit */
    char t,				/* Current input character */
	type,				/* Packet type */
	rchksum;			/* Checksum received from other host */

#if UCB4X				/* TOPS-20 can't handle timeouts... */
    if (setjmp(env)) return FALSE;	/* Timed out, fail */
    signal(SIGALRM,clkint);		/* Setup the timeout */
    if ((timint > MAXTIM) || (timint < MINTIM)) timint = MYTIME;
    alarm(timint);
#endif /* UCB4X */

    if (pktdeb) fprintf(dpfp,"\nRpack:");
    while (inchr() != SOH);		/* Wait for packet header */

    done = FALSE;			/* Got SOH, init loop */
    while (!done)			/* Loop to get a packet */
    {
	cchksum = 0;
	if ((t = cinchr()) == SOH) continue; /* Resynchronize if SOH */
	*len = unchar(t)-3;		/* Character count */

	if ((t = cinchr()) == SOH) continue; /* Resynchronize if SOH */
	*num = unchar(t);		/* Packet number */

	if ((t = cinchr()) == SOH) continue; /* Resynchronize if SOH */
	type = t;			/* Packet type */

/* Put len characters from the packet into the data buffer */

	for (i=0; i<*len; i++)
	    if ((data[i] = cinchr()) == SOH) continue; /* Resynch if SOH */

	data[*len] = 0;			/* Mark the end of the data */

	if ((t = inchr()) == SOH) continue; /* Resynchronize if SOH */
	rchksum = unchar(t);		/* Convert to numeric */
	done = TRUE;			/* Got checksum, done */
    }

    if (turn)				/* User requested trunaround char */
	while (inchr() != XON);		/* handling, spin until an XON */


#if UCB4X
    alarm(0);				/* Disable the timer interrupt */
#endif

    if (debug>1)			/* Display incoming packet */
    {
	if (data != NULL)
	    data[*len] = '\0';		/* Null-terminate data to print it */
	printf("  rpack type: %c\n",type);
	printf("	 num:  %d\n",*num);
	printf("	 len:  %d\n",*len);
	if (data != NULL)
	    printf("	    data: \"%s\"\n",data);
    }
					/* Fold in bits 7,8 to compute */
    cchksum = (((cchksum&0300) >> 6)+cchksum)&077; /* final checksum */

    if (cchksum != rchksum) return(FALSE);

    return(type);			/* All OK, return packet type */
}


/*
 * Get a parity adjusted character from the line, add it to the checksum
 * and return it.
 */

cinchr()
{
    char ch;

    ch = inchr();			/* Get a character */
    cchksum += ch;			/* Add to the checksum */
    return(ch);
}


/*
 * Get a character from the line.  Do any necessary parity stripping
 * and return the character.  
 */

inchr()
{
    char ch;

    read(ttyfd,&ch,1);
    if (pktdeb)				/* If debugging put a copy in file */
	fprintf(dpfp,"%c:%03o|",ch,ch);
    if(parflg) ch = unpar(ch);		/* Handle parity */
    return(ch);
}

/*
 *  b u f i l l
 *
 *  Get a bufferful of data from the file that's being sent.
 *  Only control-quoting is done; 8-bit & repeat count prefixes are
 *  not handled.
 */

bufill(buffer)
char buffer[];				/* Buffer */
{
    int i,				/* Loop index */
	t;				/* Char read from file */
    char t7;				/* 7-bit version of above */

    i = 0;				/* Init data buffer pointer */
    while((t = getc(fp)) != EOF)	/* Get the next character */
    {
	t7 = unpar(t);			/* Get low order 7 bits */

	if (t7 < SP || t7==DEL || t7==quote) /* Does this char require */
	{				    /* special handling? */
	    if (t=='\n' && !image)
	    {				/* Do LF->CRLF mapping if !image */
		buffer[i++] = quote;
		buffer[i++] = ctl('\r');
	    }
	    buffer[i++] = quote;	/* Quote the character */
	    if (t7 != quote)
	    {
		t = ctl(t);		/* and uncontrolify */
		t7 = ctl(t7);
	    }
	}
	if (!parflg)
	    buffer[i++] = t;		/* Deposit the character itself */
	else
	    buffer[i++] = t7;

	if (i >= spsiz-8) return(i);	/* Check length */
    }
    if (i==0) return(EOF);		/* Wind up here only on EOF */
    return(i);				/* Handle partial buffer */
}


/*
 *	b u f e m p
 *
 *  Put data from an incoming packet into a file.
 */

bufemp(buffer,len)
char  buffer[];				/* Buffer */
int   len;				/* Length */
{
    int i;				/* Counter */
    char t;				/* Character holder */

    for (i=0; i<len; i++)		/* Loop thru the data field */
    {
	t = buffer[i];			/* Get character */
	if (t == MYQUOTE)		/* Control quote? */
	{				/* Yes */
	    t = buffer[++i];		/* Get the quoted character */
	    if ((t & 0177) != MYQUOTE)	/* Low order bits match quote char? */
		t = ctl(t);		/* No, uncontrollify it */
	}
	if (t==CR && !image)		/* Don't pass CR if in image mode */
	    continue;

	putc(t,fp);
    }
}


/*
 *  g n x t f l
 *
 *  Get next file in a file group
 */

gnxtfl()
{

#if VAX_VMS
    return FALSE;		/* Wildcarding doesn't work this way on VAX */
#endif
    if (debug) printf("	  gnxtfl: filelist = \"%s\"\n",*filelist);
    filnam = *(filelist++);
    if (filecount-- == 0) return FALSE; /* If no more, fail */
    else return TRUE;			/* else succeed */
}


/*
 *  s p a r
 *
 *  Fill the data array with my send-init parameters
 *
 */

spar(data)
char data[];
{
    data[0] = tochar(MAXPACKSIZ);	   /* Biggest packet I can receive */
    data[1] = tochar(MYTIME);		/* When I want to be timed out */
    data[2] = tochar(MYPAD);		/* How much padding I need */
    data[3] = ctl(MYPCHAR);		/* Padding character I want */
    data[4] = tochar(MYEOL);		/* End-Of-Line character I want */
    data[5] = MYQUOTE;			/* Control-Quote character I send */
}


/*  r p a r
 *
 *  Get the other host's send-init parameters
 *
 */

rpar(data)
char data[];
{
    spsiz = unchar(data[0]);		/* Maximum send packet size */
    timint = unchar(data[1]);		/* When I should time out */
    pad = unchar(data[2]);		/* Number of pads to send */
    padchar = ctl(data[3]);		/* Padding character to send */
    eol = unchar(data[4]);		/* EOL character I must send */
    quote = data[5];			/* Incoming data quote character */
}
 

/*
 *  f l u s h i n p u t
 *
 *  Dump all pending input to clear stacked up NACK's.
 *  (Implemented only for Berkeley Unix at this time).
 */

#if UCB4X&(~NO_FIONREAD)
flushinput()
{
    long int count;			/* Number of bytes ready to read */
    long int i;				/* Number of bytes to read in loop */

    ioctl(ttyfd, FIONREAD, &count);	/* See how many bytes pending read */
    if (!count) return;			/* If zero, then no input to flush */

    while (count)			/* Loop till all are flushed */
    {
	i = (count<sizeof(recpkt)) ?	/* Read min of count and size of */
	    count : sizeof(recpkt);	/*  the read buffer */
	read(ttyfd, recpkt, i);		/* Read a bunch */
	count -= i;			/* Subtract from amount to read */
    }
}
#else
flushinput()		/* Null version for non-Berkeley Unix */
{}
#endif /* UCB4X&(~FIONREAD) */


/*
 *  Kermit printing routines:
 *
 *  usage - print command line options showing proper syntax
 *  printmsg -	like printf with "Kermit: " prepended
 *  error - like printmsg if local kermit; sends a error packet if remote
 *  prerrpkt - print contents of error packet received from remote host
 */

/*
 *  u s a g e 
 *
 *  Print summary of usage info and quit
 */

usage()
{
#if UCB4X
    printf("Usage: kermit c[hlbep line baud esc.char par]   (connect mode)\n");
    printf("or:	   kermit s[tdiflbp line baud par] file ... (send mode)\n");
    printf("or:	   kermit r[tdiflbp line baud par]	    (receive mode)\n");
#else
    printf("Usage: kermit c[hlep line esc.char par]	 (connect mode)\n");
    printf("or:	   kermit s[tdiflp line par] file ...	 (send mode)\n");
    printf("or:	   kermit r[tdiflp line par]		 (receive mode)\n");
#endif
    exit(1);
}

/*
 *  p r i n t m s g
 *
 *  Print message on standard output if not remote.
 */

/*VARARGS1*/
printmsg(fmt, a1, a2, a3, a4, a5)
char *fmt;
{
    if (!remote)
    {
	printf("Kermit: ");
	printf(fmt,a1,a2,a3,a4,a5);
	printf("\n");
	fflush(stdout);			/* force output (UTS needs it) */
    }
}

/*
 *  e r r o r
 *
 *  Print error message.
 *
 *  If local, print error message with printmsg.
 *  If remote, send an error packet with the message.
 */

/*VARARGS1*/
error(fmt, a1, a2, a3, a4, a5)
char *fmt;
{
    char msg[80];
    int len;

    if (remote)
    {
	sprintf(msg,fmt,a1,a2,a3,a4,a5); /* Make it a string */
	len = strlen(msg);
	spack('E',n,len,msg);		/* Send the error packet */
    }
    else printmsg(fmt, a1, a2, a3, a4, a5);
}

/*
 *  p r e r r p k t
 *
 *  Print contents of error packet received from remote host.
 */
prerrpkt(msg)
char *msg;
{
    printf("Kermit aborting with this error from remote host:\n%s\n",msg);
}

