/*  C K U S R 3 --  "User Interface" for Unix Kermit, part 3  */

/*  SET and REMOTE commands; screen, debug, interrupt, and logging functions */


/* Includes */

#include "ckermi.h"
#include "ckcmd.h"
#include "ckuser.h"

/* Variables */

extern int size, spsiz, npad, timint, speed, local, server, image, flow,
  displa, binary, fncnv, delay, parity, deblog, escape, xargc,
  turn, duplex, cxseen, czseen, nfils, ckxech, pktlog, seslog, tralog, stdouf,
  turnch, chklen, bctr, bctu, fsize, dfloc, mdmtyp,
  rptflg, ebqflg, warn, quiet, cnflg, timef, mypadn;

extern long filcnt, tlci, tlco, ffc, tfc;

extern char *versio, *protv, *ckxv, *ckzv, *fnsv, *connv, *dftty, *cmdv;
extern char *cmarg, *cmarg2, **xargv, **cmlist;
extern char mystch, sstate, mypadc, padch, eol, ctlq, filnam[], ttname[];
char *strcpy();

/* Declarations from cmd package */

extern char cmdbuf[];			/* Command buffer */

/* From main ckuser module... */

extern char line[100], *lp;		/* Character buffer for anything */
extern char debfil[50],			/* Debugging log file name */
 pktfil[50],				/* Packet log file name */
 sesfil[50],				/* Session log file name */
 trafil[50];				/* Transaction log file name */

extern int tlevel;			/* Take Command file level */
extern FILE *tfile[];			/* Array of take command fd's */

/* Keyword tables for SET commands */


/* Block checks */

struct keytab blktab[] = {
    "1", 1, 0,
    "2", 2, 0,
    "3", 3, 0
};

/* Duplex keyword table */

struct keytab dpxtab[] = {
    "full", 	 0, 0,
    "half",      1, 0
};

struct keytab filtab[] = {
    "display", XYFILD, 0,
    "names",   XYFILN, 0,
    "type",    XYFILT, 0,
    "warning", XYFILW, 0
};
int nfilp = (sizeof(filtab) / sizeof(struct keytab));

/* Flow Control */

struct keytab flotab[] = {
    "none",     0, 0,
    "xon/xoff", 1, 0
};
int nflo = (sizeof(flotab) / sizeof(struct keytab));

/*  Handshake characters  */

struct keytab hshtab[] = {
    "bell", 007, 0,
    "cr",   015, 0,
    "esc",  033, 0,
    "lf",   012, 0,
    "none", 999, 0,  /* (can't use negative numbers) */
    "xoff", 023, 0,
    "xon",  021, 0
};
int nhsh = (sizeof(hshtab) / sizeof(struct keytab));

struct keytab fntab[] = {   		/* File naming */
    "converted", 1, 0,
    "literal",   0, 0
};

struct keytab fttab[] = {		/* File types */
    "binary",    1, 0,
    "text",      0, 0
};

extern struct keytab mdmtab[] ;		/* Modem types (in module ckdial.c) */
extern int nmdm;


/* Parity keyword table */

struct keytab partab[] = {
    "even",    'e', 0,
    "mark",    'm', 0,
    "none",      0, 0,
    "odd",     'o', 0,
    "space",   's', 0
};
int npar = (sizeof(partab) / sizeof(struct keytab));


/* On/Off table */

struct keytab onoff[] = {
    "off",       0, 0,
    "on",        1, 0
};

/*  D O P R M  --  Set a parameter.  */
/*
 Returns:
  -2: illegal input
  -1: reparse needed
   0: success
*/
doprm(xx) int xx; {
    int x, y, z;
    char *s;

switch (xx) {

case XYLINE:
    if ((x = cmtxt("Device name",dftty,&s)) < 0) return(x);
    ttclos();				/* close old line, if any was open */
    strcpy(ttname,s);
    if (strcmp(ttname,dftty) == 0) local = dfloc; else local = 1;
    if (ttopen(ttname,local,mdmtyp) < 0 ) { /* Can we open the new line? */
	perror("Sorry, can't open line");
	return(-2);			/* If not, give bad return */
	}
    return(0);

case XYCHKT:
    if ((y = cmkey(blktab,3,"","1")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    bctr = y;
    return(0);

case XYDEBU:
    return(seton(&deblog));

case XYDELA:
    y = cmnum("Number of seconds before starting to send","5",10,&x);
    debug(F101,"XYDELA: y","",y);
    return(setnum(&delay,x,y));

case XYDUPL:
    if ((y = cmkey(dpxtab,2,"","full")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    duplex = y;
    return(0);

case XYEOL:
    y = cmnum("Decimal ASCII code for packet terminator","0",10,&x);
    y = setcc(&z,x,y);
    eol = z;
    return(y);

case XYESC:
    y = cmnum("Decimal ASCII code for escape character","",10,&x);
    return(setcc(&escape,x,y));

case XYFILE:
    if ((y = cmkey(filtab,nfilp,"File parameter","")) < 0) return(y);
    switch (y) {
	int z;
	case XYFILD:			/* Display */
	    y = seton(&z);
	    if (y < 0) return(y);
	    quiet = !z;
	    return(0);

	case XYFILN:			/* Names */
	    if ((x = cmkey(fntab,2,"how to handle filenames","converted")) < 0)
	    	return(x);
	    if ((z = cmcfm()) < 0) return(z);
	    fncnv = x;
	    return(0);

	case XYFILT:			/* Type */
	    if ((x = cmkey(fttab,2,"type of file","text")) < 0)
	    	return(x);
	    if ((z = cmcfm()) < 0) return(z);
	    binary = x;
	    return(0);

	case XYFILW:			/* Warning/Write-Protect */
	    return(seton(&warn));
    }

case XYFLOW:				/* Flow control */
    if ((y = cmkey(flotab,nflo,"","xon/xoff")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    flow = y;
    return(0);

case XYHAND:				/* Handshake */
    if ((y = cmkey(hshtab,nhsh,"","none")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    turn = (y > 0127) ? 0 : 1 ;
    turnch == y;
    return(0);

case XYLEN:
    y = cmnum("Maximum number of characters in a packet","80",10,&x);
    return(setnum(&spsiz,x,y));

case XYMARK:
    y = cmnum("Decimal ASCII code for packet-start character","1",10,&x);
    y = setcc(&z,x,y);
    mystch = z;
    return(y);

case XYMODM:
    if ((x=cmkey(mdmtab,nmdm,"type of modem, direct means none","direct")) < 0)
	return(x);
    if ((z = cmcfm()) < 0) return(z);
    mdmtyp = x;
    return(0);

	
case XYNPAD:
    y = cmnum("How many padding characters for inbound packets","0",10,&x);
    return(setnum(&mypadn,x,y));

case XYPADC:
    y = cmnum("Decimal ASCII code for inbound pad character","0",10,&x);
    y = setcc(&z,x,y);
    mypadc = z;
    return(y);

case XYPARI:
    if ((y = cmkey(partab,npar,"","none")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    parity = y;
    ebqflg = 1;				/* Flag we want 8th-bit prefixing */
    return(0);

case XYPROM:
    if ((x = cmtxt("Program's command prompt","C-Kermit>",&s)) < 0) return(x);
    cmsetp(s);
    return(0);

case XYSPEE:
    if (!local) {
	printf("\nSpeed setting can only be done on an external line\n");
	printf("You must 'set line' before issuing this command\n");
	return(0);
    }	
    lp = line;
    sprintf(lp,"Baud rate for %s",ttname);
    if ((y = cmnum(line,"",10,&x)) < 0) return(y);
    if (y = (cmcfm()) < 0) return(y);
    y = chkspd(x);
    if (y < 0) 
    	printf("?Unsupported line speed - %d\n",x);
    else {
    	speed = y;
	printf("%s: %d baud\n",ttname,speed);
    }
    return(0);

case XYTIMO:
    y = cmnum("Interpacket timeout interval","5",10,&x);
    y = setnum(&timint,x,y);
    if (y > -1) timef = 1;
    return(y);

default:
    if (x = (cmcfm()) < 0) return(x);
    printf("Not working yet - %s\n",cmdbuf);
    return(0);
    }
}

/*  C H K S P D  --  Check if argument is a valid baud rate  */

chkspd(x) int x; {
    switch (x) {
	case 0:
	case 110:
	case 150:
	case 300:
	case 600:
	case 1200:
	case 1800:
	case 2400:
	case 4800:
	case 9600:
	    return(x);
	default: 
	    return(-1);
      }
}

/*  S E T O N  --  Parse on/off (default on), set parameter to result  */

seton(prm) int *prm; {
    int x, y;
    if ((y = cmkey(onoff,2,"","on")) < 0) return(y);
    if ((x = cmcfm()) < 0) return(x);
    *prm = y;
    return(0);
}

/*  S E T N U M  --  Set parameter to result of cmnum() parse.  */
/*
 Call with x - number from cnum parse, y - return code from cmnum
*/
setnum(prm,x,y) int x, y, *prm; {
    debug(F101,"setnum",y);
    if (y < 0) return(y);
    if (x > 94) {
	printf("\n?Sorry, 94 is the maximum\n");
	return(-2);
    }
    if ((y = cmcfm()) < 0) return(y);
    *prm = x;
    return(0);
}

/*  S E T C C  --  Set parameter to an ASCII control character value.  */

setcc(prm,x,y) int x, y, *prm; {
    if (y < 0) return(y);
    if ((x > 037) && (x != 0177)) {
	printf("\n?Not in ASCII control range - %d\n",x);
	return(-2);
    }
    if ((y = cmcfm()) < 0) return(y);
    *prm = x;
    return(0);
}

/*  D O R M T  --  Do a remote command  */

dormt(xx) int xx; {
    int x;
    char *s, sbuf[50], *s2;

    if (xx < 0) return(xx);
    switch (xx) {

case XZCWD:				/* CWD */
    if ((x = cmtxt("Remote directory name","",&s)) < 0) return(x);
    debug(F111,"XZCWD: ",s,x);
    *sbuf = NUL;
    s2 = sbuf;
    if (*s != NUL) {			/* If directory name given, */
					/* get password on separate line. */
        if (tlevel > -1) {		/* From take file... */

	    *line = NUL;
	    if (fgets(sbuf,50,tfile[tlevel]) == NULL)
	    	ermsg("take file ends prematurely in 'remote cwd'");
	    debug(F110," pswd from take file",s2,0);

        } else {			/* From terminal... */

	    printf(" Password: "); 		/* get a password */
	    while ((x = getchar()) != '\n') {   /* without echo. */
	    	if ((x &= 0177) == '?') {
	    	    printf("? Password of remote directory\n Password: ");
		    s2 = sbuf;
		    *sbuf = NUL;
	    	}
	    	else if (x == ESC)	/* Mini command line editor... */
	    	    putchar(BEL);
		else if (x == BS || x == 0177)
		    *s2--;
		else if (x == 025) {
		    s2 = sbuf;
		    *sbuf = NUL;
		}
	    	else
		    *s2++ = x;
            }
	    *s2 = NUL;
	    putchar('\n');
        }
        s2 = sbuf;
    } else s2 = "";
    debug(F110," password",s2,0);
    sstate = setgen('C',s,s2,"");
    return(0);

case XZDEL:				/* Delete */
    if ((x = cmtxt("Name of remote file(s) to delete","",&s)) < 0) return(x);
    return(sstate = rfilop(s,'E'));

case XZDIR:				/* Directory */
    if ((x = cmtxt("Remote directory or file specification","",&s)) < 0)
    	return(x);
    return(sstate = setgen('D',s,"",""));

case XZHLP:				/* Help */
    if (x = (cmcfm()) < 0) return(x);
    sstate = setgen('H',"","","");
    return(0);

case XZHOS:				/* Host */
    if ((x = cmtxt("Command for remote system","",&cmarg)) < 0) return(x);
    return(sstate = 'c');

case XZPRI:				/* Print */
    if ((x = cmtxt("Remote file(s) to print on remote printer","",&s)) < 0)
    	return(x);
    return(sstate = rfilop(s,'S'));

case XZSPA:				/* Space */
    if ((x = cmtxt("Confirm, or remote directory name","",&s)) < 0) return(x);
    return(sstate = setgen('U',s,"",""));
    
case XZTYP:				/* Type */
    if ((x = cmtxt("Remote file specification","",&s)) < 0) return(x);
    return(sstate = rfilop(s,'T'));

case XZWHO:
    if ((x = cmtxt("Remote user name, or carriage return","",&s)) < 0)
    	return(x);
    return(sstate = setgen('W',s,"",""));

default:
    if (x = (cmcfm()) < 0) return(x);
    printf("not working yet - %s\n",cmdbuf);
    return(-2);
    }
}



/*  R F I L O P  --  Remote File Operation  */

rfilop(s,t) char *s, t; {
    if (*s == NUL) {
	printf("?File specification required\n");
	return(-2);
    }
    debug(F111,"rfilop",s,t);
    return(setgen(t,s,"",""));
}

/*  S C R E E N  --  Screen display function  */
/*  
   c - a character or small integer
   n - an long integer
   s - a string.
 Fill in this routine with the appropriate display update for the system.
 This version is for a dumb tty, and uses the arguments like this:
   c:     0 - print s on a new line, followed by a space.
          1 - print s at current screen position.
          2 - print s at current position, followed by newline.
	  3 - print "s: n" at current position, followed by newline.
      other - print c as a character.
   n: if c is 'other', used to decide whether or how to print c.
   s: as above.
 Horizontal screen position is kept current if screen is only updated by 
 calling this function.  Wraparound is done at column 78.
*/
screen(c,n,s) char c; long n; char *s; {
    static int p = 0;			/* Screen position */
    int len;  char buf[80];
    len = strlen(s);
    if (!displa || quiet) return;	/* No update if display flag off */
    switch (c) {
	case 0:				/* Print s on newline */
	    conoll("");			/* Start new line, */
	    conol(s);			/* Print string. */
	    conoc(SP);			/* Leave a space. */
	    p = len + 1;		/* Set position counter. */
	    return;
	case 1:
	    if (p + len > 78) { conoll(""); p = 0; }
	    conol(s);  if ((p += len) > 78) conoll("");
	    return;
	case 2:				/* Print s, then newline */
	    if (p + len > 78) conoll("");
	    conoll(s);  p = 0;
	    return;
	case 3:
	    sprintf(buf,"%s: %ld",s,n);  conoll(buf);  p = 0;
	    return;
	case BS:			/* Backspace */
	    if (p > 0) p--;
	case BEL:			/* Beep */
	    conoc(c);
	    return;
	default:			/* Packet type display */
	    if (c == 'Y') return;	/* Don't bother with ACKs */
	    if (c == 'D') {		/* Only show every 4th data packet */
		c = '.';
		if (n % 4) return;
	    }
	    if (p++ > 78) {		/* If near left margin, */
		conoll("");		/* Start new line */
		p = 0;			/* and reset counter. */
	    }
	    conoc(c);			/* Display the character. */
	    return;
    }
}

/*  I N T M S G  --  Issue message about terminal interrupts  */

intmsg(n) long n; {
    extern char *chstr();
    char buf[80];

    if ((!displa) || (quiet)) return;
#ifdef UXIII
    (void) conchk();	/* clear out pending escape-signals in ckxbsd.c */
#endif
    if (n == 1) {
	screen(2,0l,"");
#ifdef UXIII
				/* we need to signal before kb input */
	sprintf(buf,"Type escape (%s) followed by:",chstr(escape));
	screen(2,0l,buf);
#endif
#ifdef PROVX1
	screen(2,0l,"(transfer cannot be interrupted from keyboard)");
#else
	screen(2,0l,"CTRL-F to cancel file,  CTRL-R to resend current packet");
	screen(2,0l,"CTRL-B to cancel batch, CTRL-A for status report...");
#endif
    }
    else screen(1,0l," ");
}

/*  C H K I N T  --  Check for console interrupts  */

/*** should rework not to destroy typeahead ***/

chkint() {
    int ch, cn;

    if ((!local) || (quiet)) return(0);	/* Only do this if local & not quiet */
    cn = conchk();			/* Any input waiting? */
    debug(F101,"conchk","",cn);

    while (cn > 0) {			/* Yes, read it. */
	cn--;
			/* give read 5 seconds for interrupt character */
	if ((ch = coninc(5)) < 0) return(0);
	switch (ch & 0177) {
	    case 0001:			/* CTRL-A */
		screen(2,0l,"^A  Status report:");
		screen(1,0l,     " file type: ");
		if (binary) screen(2,0l,"binary"); else screen(2,0l,"text");
		screen(3,filcnt," file number");
		screen(3,ffc,   " characters ");
		screen(3,(long) bctu,  " block check");
		screen(3,(long)rptflg," compression");
		screen(3,(long)ebqflg," 8th-bit prefixing");
		continue;
	    case 0002:			/* CTRL-B */
	    	screen(1,0l,"^B - Cancelling Batch ");
	    	czseen = 1;
		continue;
	    case 0006:			/* CTRL-F */
	    	screen(1,0l,"^F - Cancelling File ");
	    	cxseen = 1;
		continue;
	    case 0022:	    	    	/* CTRL-R */
	    	screen(1,0l,"^R - Resending ");
	    	resend();
		return(1);
	    default:			/* Anything else, just beep */
	    	screen(BEL,0l,"");
		continue;
    	}
    }
    return(0);
}

/*  D E B U G  --  Enter a record in the debugging log  */

/*
 Call with a format, two strings, and a number:
   f  - Format, a bit string in range 0-7.
        If bit x is on, then argument number x is printed.
   s1 - String, argument number 1.  If selected, printed as is.
   s2 - String, argument number 2.  If selected, printed in brackets.
   n  - Int, argument 3.  If selected, printed preceded by equals sign.

   f=0 is special: print s1,s2, and interpret n as a char.
*/
debug(f,s1,s2,n) int f, n; char *s1, *s2; {
    static char s[200];
    char *sp = s;

    if (!deblog) return;		/* If no debug log, don't */
    switch (f) {
    	case F000:			/* 0, print both strings, */
	    sprintf(sp,"%s%s%c\n",s1,s2,n); /*  and interpret n as a char */
	    zsout(ZDFILE,s);
	    break;
    	case F001:			/* 1, "=n" */
	    sprintf(sp,"=%d\n",n);
	    zsout(ZDFILE,s);
	    break;
    	case F010:			/* 2, "[s2]" */
	    sprintf(sp,"[%s]\n",s2);
	    zsout(ZDFILE,"");
	    break;
    	case F011:			/* 3, "[s2]=n" */
	    sprintf(sp,"[%s]=%d\n",s2,n);
	    zsout(ZDFILE,s);
	    break;
    	case F100:			/* 4, "s1" */
	    zsoutl(ZDFILE,s1);
	    break;
    	case F101:			/* 5, "s1=n" */
	    sprintf(sp,"%s=%d\n",s1,n);
	    zsout(ZDFILE,s);
	    break;
    	case F110:			/* 6, "s1[s2]" */
	    sprintf(sp,"%s[%s]\n",s1,s2);
	    zsout(ZDFILE,s);
	    break;
    	case F111:			/* 7, "s1[s2]=n" */
	    sprintf(sp,"%s[%s]=%d\n",s1,s2,n);
	    zsout(ZDFILE,s);
	    break;
	default:
	    sprintf(sp,"\n?Invalid format for debug() - %d\n",n);
	    zsout(ZDFILE,s);
    }
}

/*  T L O G  --  Log a record in the transaction file  */
/*
 Call with a format and 3 arguments: two strings and a number:
   f  - Format, a bit string in range 0-7, bit x is on, arg #x is printed.
   s1 - String, argument number 1.
   s2 - String, argument number 2.
   n  - Int, argument 3.
*/
tlog(f,s1,s2,n) int f; long n; char *s1, *s2; {
    static char s[200];
    char *sp = s; int x;
    
    if (!tralog) return;		/* If no transaction log, don't */
    switch (f) {
    	case F000:			/* 0 (special) "s1 n s2"  */
	    sprintf(sp,"%s %ld %s\n",s1,n,s2);
	    zsout(ZTFILE,s);
	    break;
    	case F001:			/* 1, " n" */
	    sprintf(sp," %ld\n",n);
	    zsout(ZTFILE,s);
	    break;
    	case F010:			/* 2, "[s2]" */
	    x = strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    sprintf(sp,"[%s]\n",s2);
	    zsout(ZTFILE,"");
	    break;
    	case F011:			/* 3, "[s2] n" */
	    x = strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    sprintf(sp,"[%s] %ld\n",s2,n);
	    zsout(ZTFILE,s);
	    break;
    	case F100:			/* 4, "s1" */
	    zsoutl(ZTFILE,s1);
	    break;
    	case F101:			/* 5, "s1: n" */
	    sprintf(sp,"%s: %ld\n",s1,n);
	    zsout(ZTFILE,s);
	    break;
    	case F110:			/* 6, "s1 s2" */
	    x = strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    sprintf(sp,"%s %s\n",s1,s2);
	    zsout(ZTFILE,s);
	    break;
    	case F111:			/* 7, "s1 s2: n" */
	    x = strlen(s2);
	    if (s2[x] == '\n') s2[x] = '\0';
	    sprintf(sp,"%s %s: %ld\n",s1,s2,n);
	    zsout(ZTFILE,s);
	    break;
	default:
	    sprintf(sp,"\n?Invalid format for tlog() - %ld\n",n);
	    zsout(ZTFILE,s);
    }
}
