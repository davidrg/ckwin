#ifndef lint
static char sccsid[] = "@(#)uudecode.c	5.3-1 (Berkeley) 4/10/85";
#endif
/* modified by ajr to use checksums */
/* modified by fnf to use Keith Pyle's suggestion for compatibility */

/* Adapted by Phil Julian, SAS Institute, Inc., for use on the Data General
 * minicomputers.  #ifdefs surround host-dependent code.
 * 27 May 1987
 */

/*
 * uudecode [input]
 *
 * create the specified file, decoding as you go.
 * used with uuencode.
 */
#include <stdio.h>

#ifdef unix
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef datageneral
#include <sys_calls.h>
#include <packets/filestatus.h>         /* Used for ?GNFN */
#include <packets:normal_io.h>
#include <paru.h>
int rec_format, file_type;
struct p_nio_ex w_io_parms;             /* ?write system call structure */
int chanout;                            /* Output channel */
#ifdef putc
#undef putc
#endif
#define putc(c,file) { char ch = (char) (c); dg_binw(fchannel(file),&ch,1); }
#endif

#define SUMSIZE 64

/* single character decode */
#define DEC(c)	(((c) - ' ') & 077)

main(argc, argv)
char **argv;
{
	FILE *in, *out;
	int mode;
#ifdef datageneral
	char dest[256];     /* Filenames can be larger */
	char buf[512];
#else
	char dest[128];
	char buf[80];
#endif

	/* optional input arg */
	if (argc > 1) {
		if ((in = fopen(argv[1], "r")) == NULL) {
			perror(argv[1]);
			exit(1);
		}
		argv++; argc--;
	} else
		in = stdin;

	if (argc != 1) {
		printf("Usage: uudecode [infile]\n");
		exit(2);
	}

	/* search for header line */
	for (;;) {
		if (fgets(buf, sizeof buf, in) == NULL) {
			fprintf(stderr, "No begin line\n");
			exit(3);
		}
		if (strncmp(buf, "begin ", 6) == 0)
			break;
	}
#ifdef datageneral
        /* DG format has extra information, and the record format field must
         * be a legal value.  Other systems only store two items: the file 
         * mode and the file name.  We store the record format, the file
         * name and the file type.
         * Legal record formats are current betwee $RTDY and $RTVB (which 
         * is IBM tape only).
         */
	if ((sscanf(buf, "begin %o %s %o", &rec_format, dest, &file_type) < 3) 
	|| ((rec_format < $RTDY) && (rec_format > $RTVB))) {
            /* Not a DG generated file -- use defaults of Dynamic data and
             * a user data file (UDF).
             */
	    rec_format = $RTDY;
	    file_type  = $FUDF;
	}
#else
	sscanf(buf, "begin %o %s", &mode, dest);
#endif

#ifdef unix
	/* handle ~user/file format */
	if (dest[0] == '~') {
		char *sl;
		struct passwd *getpwnam();
		char *index();
		struct passwd *user;
		char dnbuf[100];

		sl = index(dest, '/');
		if (sl == NULL) {
			fprintf(stderr, "Illegal ~user\n");
			exit(3);
		}
		*sl++ = 0;
		user = getpwnam(dest+1);
		if (user == NULL) {
			fprintf(stderr, "No such user as %s\n", dest);
			exit(4);
		}
		strcpy(dnbuf, user->pw_dir);
		strcat(dnbuf, "/");
		strcat(dnbuf, sl);
		strcpy(dest, dnbuf);
	}
#endif

	/* create output file */
#ifdef datageneral
        zero((char *) &w_io_parms, sizeof(w_io_parms));
        w_io_parms.isti = $IBIN|$RTDY|$ICRF|$OFOT;
        w_io_parms.isti &= ~$IPST;
        w_io_parms.imrs = 2048;
        w_io_parms.ibad = -1;
        w_io_parms.ircl = -1;

        if (rec_format == 0) rec_format = $RTUN; 
        out = dg_open(dest, $OFOT|$IBIN|$OFCR|$OFCE|rec_format,file_type);
        if (out == NULL) {
		perror(dest);
		exit(4);
	}
	chanout = fchannel(out);
#else
	out = fopen(dest, "w");
	if (out == NULL) {
		perror(dest);
		exit(4);
	}
#if (unix || !MANX)
	chmod(dest, mode);
#endif
#endif datageneral

	decode(in, out);

	if (fgets(buf, sizeof buf, in) == NULL || strcmp(buf, "end\n")) {
		fprintf(stderr, "No end line\n");
		exit(5);
	}
	exit(0);
}

/*
 * copy from in to out, decoding as you go along.
 */
decode(in, out)
FILE *in;
FILE *out;
{
	char buf[80];
	char *bp;
	int n, checksum, line;
	int warnings = 5;

	for (line = 1; ; line++) {
		/* for each input line */
		if (fgets(buf, sizeof buf, in) == NULL) {
			printf("Short file\n");
			exit(10);
		}

		checksum = 0;
		n = DEC(buf[0]);
		if (n <= 0)
			break;

		bp = &buf[1];
		while (n > 0) {
			checksum = (checksum+outdec(bp, out, n)) % SUMSIZE;
			bp += 4;
			n -= 3;
		}

		if (*bp != '\n' && checksum != DEC(*bp))
			if (warnings > 0) {
				printf("Checksum error, line %d.\n",line);
				warnings--;
			} else if (warnings == 0) {
				printf("more...\n");
				warnings--;
			}
	}
}

/*
 * output a group of 3 bytes (4 input characters).
 * the input chars are pointed to by p, they are to
 * be output to file f.  n is used to tell us not to
 * output all of them at the end of the file.
 * we return a checksum increment.
 */
int outdec(p, f, n)
char *p;
FILE *f;
{
	int c1, c2, c3;

	c1 = DEC(*p) << 2 | DEC(p[1]) >> 4;
	c2 = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
	c3 = DEC(p[2]) << 6 | DEC(p[3]);

	if (n >= 1)
		putc(c1, f);
	if (n >= 2)
		putc(c2, f);
	if (n >= 3)
		putc(c3, f);

	return((c1+c2+c3) % SUMSIZE);
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

/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 */

#ifndef NULL		/* MANX has bogus "#define NULL 0L" somewhere */
#define	NULL	0
#endif

char *
index(sp, c)
register char *sp, c;
{
	do {
		if (*sp == c)
			return(sp);
	} while (*sp++);
	return(NULL);
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
