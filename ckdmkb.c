/* The boo and de-boo programs do not work, and I found them too cumbersome
 * and complicated to install on the DG.  In their place, I have provided
 * a few other tools:
 *
 *      uuencode ansd uudecode:  
 *          Convert any binary file to a non-binary file, and preserve the
 *          DG record and file types, if done on DG systems.
 *      sq and usq:
 *          CP/M (and others) squeeze and unsqueeze programs.  Should be
 *          compatible with Unix and other hosts.
 *      compress:
 *          a more efficient compression than sq/usq.  Also compatible with
 *          many hosts.
 *
 * If anyone can get these other files working, more power to you.  The
 * uudecode program is slow, but the squeeze programs are very fast.  The
 * uudecode could be faster, but I did not want to wasted alot of time on
 * it.  Anyway, the uudecode source is very simple, and should be easy to
 * convert to another language.  If you do convert the uudecode program,
 * please let me know, so that it can be distributed to hosts that do not
 * have the C compiler.
 *
 *   Phil Julian, SAS Institute, Inc., Box 8000, Cary, NC 27512-8000
 *
 */

/* MSBMKB.C
 *
 * Update history:
 *
 * Modified 3/11/86 Howie Kaye -- Columbia University
 * added UNIX and Microsoft C compatibility
 * changed I/O to be buffered
 * note: there is a bug built into the EOF handling which causes the 
 * output file to grow everytime a file is packed/unpacked.  This is 
 * because 2 nulls and a space are added at the end of each run.  As 
 * the data is past the end of the file, it does not affect the program
 * produced.
 *
 * [1]	 Version adapted from the DEC-20 code to run on Lattice-C (v 2.14)
 * on an IBM PC/AT under DOS 3.0.      Alan Phillips, Lancaster University UK
 *
 * Original by Bill Catchings, Columbia University, July 1984
 */
 
/*
 * This program takes a file and encodes it into printable characters.
 * These printable files can then be decoded by the programs MSPCBOOT.BAS
 * or MSPCTRAN.BAS as the need may be.	The file is encoded by taking
 * three consecutive eight bit bytes and dividing them into four six bit
 * bytes.  An ASCII zero was then added to the resulting four characters.
 * to make them all printable ASCII characters in the range of the
 * character zero to the character underscore.	In order to reduce the
 * size of the file null repeat count was used.  The null repeat count
 * compresses up to 78 consecutive nulls into only two characters.  This
 * is done by using the character tilde (~) as an indication that a group
 * of repetitive nulls has occured.  The character following the tilde is
 * number of nulls in the group.  The number is also converted in to a
 * printable character by adding an ASCII zero.  The highest number of
 * nulls is therefore the highest printable character tilde.  This is
 * equal to tilde minus zero nulls or 78 nulls.  Because of the three
 * byte to four byte encoding the repeat counting can only start with
 * the first character of a three byte triplet.
 *
 * This C program was written specifically for the DEC-20 and as such
 * will not easily be transported to another system.  The main problem
 * lies in the file I/O routines.  It is necessary to make sure that
 * untranslated eight bit bytes are input from the input file.	The
 * main change would be to make the OPEN statement reflect this for
 * your particular system and brand of UNIX and C.  The rest of the
 * program should be transportable with little or no problems.
 */
 
/*
 * set msdos if to be compiled on an msdos machine 
 */	 
 
#define MSDOS 	0
#define UNIX	0
#define TOPS20  0
 
#include <stdio.h>		/* Standard UNIX i/o definitions */
 
#if	MSDOS 		/* [1] */
#include <fcntl.h>
#endif
#if UNIX
#include <sys/file.h>
#endif				/* [1] */
#if TOPS20
#include <file.h>
#endif
/* Datageneral is already defined */
#ifdef datageneral
#include <sys_calls.h>
#include <packets/filestatus.h>         /* Used for ?GNFN */
#include <packets:normal_io.h>
#include <paru.h>
struct p_nio_ex w_io_parms;             /* ?write system call structure */
struct p_nio_ex r_io_parms;             /* ?read system call structure */
P_FSTAT buf; 
int ac0,ac2;
char name[256];
#define R_ACC 4
#ifdef putc
#undef putc
#endif
#define putc(c,file) { char ch = (char) (c); dg_binw(fchannel(file),&ch,1); }
#define write(filen,chs,len) dg_binw(channel(filen),chs,len)
#define read(filen,chs,len)  dg_binr(channel(filen),chs,len)
#endif
 
/* Symbol Definitions */
 
#define MAXPACK 	80	/* Maximum packet size */
 
#define MYRPTQ		'~'     /* Repeat count prefix I will use */
#define DATALEN 	78	/* Length of data buffer */
 
#define TRUE		-1	/* Boolean constants */
#define FALSE		0
 
/* Macros */
 
#define tochar(ch)  ((ch) + '0')
 
/* Global Variables */
 
int	maxsize,		/* Max size for data field */
	fd,			/* File pointer of file to read/write */
	ofd,
#if	!(MSDOS | UNIX)         /* [1] */
	nc,			/* Count of input characters */
	oc,			/* Count of output characters */
	otot,			/* What char number we are processing */
#endif
	rpt,			/* repeat count */
	rptq,			/* repeat quote */
	rptflg, 		/* repeat processing flag */
	size,			/* size of present data */
#if	(MSDOS|UNIX|datageneral) /* [1] */
	t,			/* Current character value as 16 bit */
#endif				/* [1] */
	eoflag; 		/* Set when file is empty. */
 
#if	(MSDOS|UNIX)            /* [1] */
long	nc,			/* Count of input characters */
	oc,			/* Number of output chars */
	otot;			/* What char number we are processing */
#endif				/* [1] */
 
char	one,
	two,
	three,
#if	!(MSDOS|UNIX|datageneral)   /* [1] */
	t,			/* Current character */
#endif				/* [1] */
	*filnam,		/* Current file name */
	*ofile,
	packet[MAXPACK];	/* Packet buffer */

main(argc,argv) 			/* Main program */
int argc;				/* Command line argument count */
char **argv;				/* Pointers to args */
{
    char sfile();			/* Send file routine & ret code */
#ifdef datageneral
    /* Initialize the i/o block for putc() */
    zero((char *) &w_io_parms, sizeof(w_io_parms));
    w_io_parms.isti = $IBIN|$RTDY|$ICRF|$OFOT;
    w_io_parms.isti &= ~$IPST;
    w_io_parms.imrs = 2048;
    w_io_parms.ibad = -1;
    w_io_parms.ircl = -1;
    
    zero((char *) &r_io_parms, sizeof(r_io_parms));
    r_io_parms.isti = $IBIN|$RTDY|$ICRF|$OFIN;
    r_io_parms.isti &= ~$IPST;
    r_io_parms.imrs = 2048;
    r_io_parms.ibad = -1;
    r_io_parms.ircl = -1;
   
#endif
    if (--argc != 2) usage();		/* Make sure there's a command line. */
    rptq = MYRPTQ;			/* Repeat Quote */
    rptflg = TRUE;			/* Repeat Count Processing Flag */
 
    filnam = *++argv;			/* Get file to send */
    ofile = *++argv;			/* Output file to create */
    sfile();
#if	(MSDOS|UNIX) 			/* [1] */
    printf("Done, in: %ld, out: %ld, efficiency: %.2f%%\n",nc,oc,(100.0*nc)/oc);
#else
    printf("Done, in: %d, out: %d, efficiency: %.2f%%\n",nc,oc,(100.0*nc)/oc);
#endif					/* [1] */
}

/*
   S F I L E - Send a whole file
*/
 
char sfile()				/* Send a file */
{
    char *i;
 
#if	MSDOS 			/* [1] */
#ifndef O_RAW
#define O_RAW O_BINARY
#endif O_RAW
#endif MSDOS
#if UNIX
#define O_RAW 0
#endif UNIX
#ifdef datageneral
    FILE *temp1,*temp2;
    /* temp1 = fopen(filnam,"j"); */
    temp1 = dg_open(filnam,$OFIN|$IBIN|$ICRF|$RTDY,$FUNX); 
    if (temp1 == NULL) fd = -1;
    else               fd = fileno(temp1);
#else
#if (MSDOS | UNIX)
    fd = open(filnam,O_RDONLY | O_RAW,0x1ff);
#else
    fd = open(filnam,FATT_RDONLY | FATT_BINARY | FATT_DEFSIZE,0x1ff);
#endif					/* [1] */
#endif					/* [1] */
    if (fd < 0) 			/* Report any errors */
    {
	printf("\n?Error opening file \"%s\"\n",filnam);
	exit(1);
    }
 
#ifdef datageneral
    temp2 = dg_open(ofile,$OFOT|$IBIN|$OFCR|$OFCE|$RTDY,$FUDF);
    if (temp2 == NULL) ofd = -1;
    else ofd = fileno(temp2);
#else
#if (MSDOS | UNIX)
    ofd = open(ofile,O_CREAT|O_WRONLY|O_TRUNC|O_RAW,0x1ff);
#else
    ofd = open(ofile,FATT_WRONLY | FATT_CREATE | FATT_BINARY,0x1ff);
#endif					/* [1] */
#endif					/* [1] */
 
    if (ofd < 0)
    {
	printf("\n?error opening file \"%s\"\n",ofile);
	exit(1);
    }
 
    oc = strlen(filnam);		/* Get the string length. */
    for (i=filnam; *i != '\0'; i++)     /* Uppercase the file name. */
	if (*i >= 'a' && *i <= 'z') *i ^= 040;
    write(ofd,filnam,oc);		/* Write the file name in the file. */
#if (!UNIX && !datageneral)
    write(ofd,"\r\n",2);
#else
    write(ofd,"\n",1);
#endif
    maxsize = DATALEN - 5;
    rpt = 0;				/* Zero the repeat count. */
    oc = nc = 0;			/* Output & input character counts. */
    otot = 1;				/* Start with first char of triplet. */
    while (getbuf() > 0)		/* While not EOF, get a packet. */
    {
#if (!UNIX && !datageneral)
	packet[size++] = '\r';          /* Explicit CRLF. */
#endif
	packet[size++] = '\n';
	packet[size] = '\0';
	oc += size;			/* Count output size. */
	write(ofd,packet,size); 	/* Write the packet to the file. */
     /* printf("%d: %s",size,packet);*/ /* Print on the screen for testing. */
    }
#if	(MSDOS|datageneral)		/* [1] */
    close(fd);				/* close the files neatly */
    close(ofd);
#endif					/* [1] */
}
/*
   G E T B U F -- Do one packet.
*/
 
getbuf()			   /* Fill one packet buffer. */
{
    if (eoflag != 0) return(-1);	/* If at the end of file, stop. */
    size = 0;
    while((t = getch()) >= 0)		/* t == -1 means EOF. */
    {
	nc++;				/* Count the character. */
	process(t);			/* Process the character. */
	if (size >= maxsize)		/* If the packet is full, */
	{
	    packet[size] = '\0';        /*  terminate the string. */
	    return(size);
	}
    }
    eoflag = -1;			/* Say we hit the end of the file. */
    process(0); 			/* Clean out any remaining chars. */
    process(0);
    process(' ');
    packet[size] = '\0';                /* Return any partial final buffer. */
    return(size);
}

/* P R O C E S S -- Do one character. */
 
process(a)
char a;
{
    if (otot == 1)			/* Is this the first of three chars? */
    {
	if (a == 0)			/* Is it a null? */
	{
	    if (++rpt < 78)		/* Below max nulls, just count. */
		return;
	    else if (rpt == 78) 	/* Reached max number, must output. */
	    {
		packet[size++] = rptq;	/* Put in null repeat char and */
		packet[size++] = tochar(rpt); /* number of nulls. */
		packet[size] = '\0';
		rpt = 0;
		return;
	    }
	}
	else
	{
	    if (rpt == 1)		/* Just one null? */
	    {
		one = 0;		/* Say the first char was a null. */
		two = a;		/* This char is the second one. */
		otot = 3;		/* Look for the third char. */
		rpt = 0;		/* Restart null count. */
		return;
	    }
	    if (rpt > 1)		/* Some number of nulls? */
	    {
		packet[size++] = rptq;	/* Insert the repeat prefix */
		packet[size++] = tochar(rpt); /* and count. */
		packet[size] = '\0';
		rpt = 0;		/* Reset repeat counter. */
	    }
	    one = a;			/* Set first character. */
	    otot = 2;			/* Say we are at the second char. */
	}
    }
    else if (otot == 2)
    {
	two = a;			/* Set second character. */
	otot = 3;			/* Say we are at the third char. */
    }
    else
    {
	three = a;
	otot = 1;			/* Start over at one. */
	pack(one,two,three);		/* Pack in the three characters. */
    }
}

/* This routine does the actual three character to four character encoding.
 * The concept is relatively straight forward.	The first output character
 * consists of the first (high order or most significant) six bits of the
 * first input character.  The second output character is made from the
 * remaining two low order bits of the first input character and the first
 * four high order bits of the second input character.	The third output
 * character is built from the last four low order bits of the second input
 * character and the two high order bits of the third input character.	The
 * fourth and last output character consists of the six low order bit of
 * the third input character.  In this way the three eight bit input char-
 * acters (for a total of 24 bits) are divided into four six bit output
 * characters (also for a total of 24 bits).  In order to make the four
 * output characters printable an ASCII zero is then added to each of them.
 *
 */
 
pack(x,y,z)
char x,y,z;
{
    packet[size++] = tochar((x >> 2) & 077);
    packet[size++] = tochar(((x & 003) << 4) | ((y >> 4) & 017));
    packet[size++] = tochar(((y & 017) << 2) | ((z >> 6) & 003));
    packet[size++] = tochar(z & 077);
    packet[size] = '\0';
}

int
getch() 				/* Get next (or pushed) char. */
{
#if TOPS20
					/* really really inefficient. */
   return((read(fd,&a,1) > 0) ? (int) (a&0xff) : -1); /* (or -1 if EOF) */
#else
#ifndef BSIZE
#define BSIZE 500
#endif
   static int index = 0, count = 0;
   static char buf[BSIZE];
 
   if (count == 0) {
     count = read(fd,buf,BSIZE);
     if (count <= 0) return(-1);
     index = 0;
   }
   count--;
   return(buf[index++]&0xff);
 
#endif
}
 
usage() 				/* Give message if user makes */
{					/* a mistake in the command. */
    fprintf(stderr,"usage: msmkboo inputfile outputfile\n");
    exit(1);
}

#ifdef datageneral
/* D G _ B I N W -- Output len characters to the file number filenum 
 * 
 *  The syntax is like the Unix write command.
 *  This code was borrowed from my Kermit source -- ckdtio.c
 */

dg_binw(channel,chs,len) int channel, len; char *chs; 
{
     int ac2,err;

     if (len == 0) return(0);

     w_io_parms.ich = channel;
     w_io_parms.ibad = chs;
     w_io_parms.ircl = len;
     ac2 = &w_io_parms;
     
     if ((err = sys_write(ac2)) == 0) return(0);

     if ( err != ERLTL && err != EREOF ) {
    	  perror("dg_binw: sys_write ");
          exit(err);
     }
}

/* D G _ B I N R -- Binary input routine
 *
 *  The syntax is like the Unix read command.
 *  This code was borrowed from my Kermit source -- ckdtio.c
 */

dg_binr(channel,chs,len) int channel, len; char *chs; 
{
     int ac2,                       /* I/O parameter address block */
         err,                       /* Error from sys_read */
         irlr;                      /* Number of bytes read */

     r_io_parms.ich  = channel; 
     r_io_parms.ibad = chs;
     r_io_parms.ircl = len;
     ac2 = &r_io_parms;
     err = sys_read(ac2);
     irlr = r_io_parms.irlr;
     if (err == 0) return(irlr);

     if ((err != ERLTL) && (err != EREOF)) {
          /* NOT line-too-long, or EOF errors. */
    	  perror("dg_binr: sys_read ");
          return(-err);
     } else if (err == EREOF) return(-1);            /* EOF */
}
#endif datageneral


