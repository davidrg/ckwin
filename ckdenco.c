#ifndef lint
static char sccsid[] = "@(#)uuencode.c	5.3-1 (Berkeley) 1/22/85";
#endif
/* modified by ajr to include checksums */

/* Adapted by Phil Julian, SAS Institute, Inc., for use on the Data General
 * minicomputers.  #ifdefs surround host-dependent code.
 * 27 May 1987
 */

/*
 * uuencode [input] output
 *
 * Encode a file so it can be mailed to a remote system.
 */
#include <stdio.h>

#ifdef unix
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef datageneral
#include <sys_calls.h>
#include <packets/filestatus.h>         /* Used for ?GNFN */
#include <packets:normal_io.h>
#include <paru.h>
struct p_nio_ex w_io_parms;             /* ?write system call structure */
P_FSTAT buf; 
int ac0,ac2;
char name[256];
#define R_ACC 4
#ifdef putc
#undef putc
#endif
#define putc(c,file) { char ch = (char) (c); dg_binw(fchannel(file),&ch,1); }
#endif

#define SUMSIZE 64

/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) ((c) ? ((c) & 077) + ' ': '`')

main(argc, argv)
char **argv;
{
	FILE *in;
#ifdef unix
	struct stat sbuf;
#endif
	int mode;

	/* optional 1st argument */
#ifdef datageneral
        name[0] = 0;            /* Signal that there is not filename */
	if (argc > 2) {
                strcpy (name,argv[1]);
		if ((in = fopen(argv[1], "j")) == NULL) {
#else
	if (argc > 2) {
		if ((in = fopen(argv[1], "r")) == NULL) {
#endif
			perror(argv[1]);
			exit(1);
		}
		argv++; argc--;
	} else
		in = stdin;

	if (argc != 2) {
#ifdef datageneral
		printf("Usage: x uuencode/l=localname [infile] remotefile\n");
#else
		printf("Usage: uuencode [infile] remotefile\n");
#endif
		exit(2);
	}

#ifdef unix
	/* figure out the input file mode */
	fstat(fileno(in), &sbuf);
	mode = sbuf.st_mode & 0777;
#else
#ifdef datageneral
        /* Initialize the i/o block for putc() */
        zero((char *) &w_io_parms, sizeof(w_io_parms));
        w_io_parms.isti = $IBIN|$RTDY|$ICRF|$OFOT;
        w_io_parms.isti &= ~$IPST;
        w_io_parms.imrs = 2048;
        w_io_parms.ibad = -1;
        w_io_parms.ircl = -1;

        if (name[0]) {
            /* For some reason, the DG croaks intermittently with a call to 
             * stat(), because the directory /etc/passwd may not exist.  We 
             * use ?fstat to circumvent this bug.
             */
 
            ac0 = (int) name;  ac2 = (int) &buf;
            if (sys_fstat(ac0,0,ac2)) { perror("sys_fstat:"); exit(-1); }

            /* buf.styp_type is the file format field.
             * buf.styp_format is the record format field.
             *
             * buf.scps is the record length if the type is fixed -- if you
             * need to use fixed records, then you would have to use a sys_open
             * to have control over the ircl value, or you would have to use
             * a sys_create call.  I have chosen only to use dg_open and 
             * simplify the process, since fixed files are seldom used and are
             * seldom binary.
             */

            if ((buf.styp_type >= $LDIR) && (buf.styp_type <= $HDIR)) {
                fprintf(stderr,"Illegal file type for encode!");
                exit(-2);
            }
            if (access(name,R_ACC) < 0) {     	/* Is the file accessible? */
                perror("Access failed:");
                exit(-3);			
            } 
        }
#else
	mode = 0777;
#endif
#endif
#ifdef datageneral
    {
        char temp[512]; int i;
        /* No format defaults to an undefined record format */
        if (buf.styp_format == 0) buf.styp_format = $RTUN;
	sprintf(temp, "begin %#o %s %#o\n", 
	              buf.styp_format, argv[1], buf.styp_type);
        dg_binw (fchannel(stdout), temp, strlen(temp));
    }
#else
	printf("begin %o %s\n", mode, argv[1]);
#endif

	encode(in, stdout);

#ifdef datageneral
        dg_binw (fchannel(stdout), "end\n", 4);
#else
	printf("end\n");
#endif
	exit(0);
}

/*
 * copy from in to out, encoding as you go along.
 */
encode(in, out)
FILE *in;
FILE *out;
{
	char buf[80];
	int i, n, checksum;

	for (;;) {
		/* 1 (up to) 45 character line */
		n = fr(in, buf, 45);
		putc(ENC(n), out);

		checksum = 0;
		for (i=0; i<n; i += 3)
			checksum = (checksum+outdec(&buf[i], out)) % SUMSIZE;

		putc(ENC(checksum), out);
		putc('\n', out);
		if (n <= 0)
			break;
	}
}

/*
 * output one group of 3 bytes, pointed at by p, on file f.
 * return the checksum increment.
 */
int outdec(p, f)
char *p;
FILE *f;
{
	int c1, c2, c3, c4;

	c1 = *p >> 2;
	c2 = (*p << 4) & 060 | (p[1] >> 4) & 017;
	c3 = (p[1] << 2) & 074 | (p[2] >> 6) & 03;
	c4 = p[2] & 077;
	putc(ENC(c1), f);
	putc(ENC(c2), f);
	putc(ENC(c3), f);
	putc(ENC(c4), f);

	return((p[0]+p[1]+p[2]) % SUMSIZE);
}

/* fr: like read but stdio */
int
fr(fd, buf, cnt)
FILE *fd;
char *buf;
int cnt;
{
	int c, i;

	for (i=0; i<cnt; i++) {
		c = getc(fd);
		if (c == EOF)
			return(i);
		buf[i] = c;
	}
	return (cnt);
}

#if (!unix && MANX)

perror (sp)
char *sp;
{
	if (sp && *sp) {
		fprintf (stderr, "%s: ");
	}
	fprintf (stderr, "<unknown error>\n");
}

#endif	/* Unix */


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
#endif

