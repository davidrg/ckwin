/*
>From zehntel!ihnp4!ixn5c!inuxc!pur-ee!uiucdcs!uiucuxc!root Sat Jul  2 04:31:32 1983
Subject: Ansitar.c - (nf)
Newsgroups: net.sources

#N:uiucuxc:12500005:000:19678
uiucuxc!root    Jul  1 23:55:00 1983
*
* 10/28/83 DRB: new option 'D': read variable length records (D format)
* 10/29/83 FGH: 1. wildcard ability added to filenames.
* 		2. when output filename isn't valid for Unix (as it's perhaps
*		   taken from a foreign tape), this program now just skips
*		   that file rather than exiting the whole job.
* 10/30/83 FGH: 1. some problems with varunblock's handling of end of record.
*		   still must use 'D' option with 'b' such that b's argument
*		   exceeds the tape's physical record size (2048 in my case)
*		2. changed varunblock/doxtract to correct the byte count.
*
* 10/31/83 DRB:
*	new option P - create/read files in "pip" (FILES-11) counted format.
*	handle tape read error
* From zehntel!tektronix!ucbcad!ucbvax!decvax!microsoft!uw-beaver!cornell!vax135!floyd!harpo!utah-cs!utah-gr!thomas Tue Aug  2 21:27:08 1983
*    (=Spencer)   (#ifdef PIP)
# MACKAY@WASHINGTON.ARPA
* 04/24/84 PAM:
*		1. Upgrade to ANSI X3.27 System level 3.
*		   Program now reads and writes HDR2 label 
*		   for deblocking info.
*		2. When reading, sets blocksize only if D-type, blocksize
*		   and linesize (aka record-length) if F-type.
*		   Neither D nor b/B need be specified on the
*		   command line if tape has valid HDR2 labels.
*		3. Full HDR2 info provided in "verbose mode."
*		   (Ought to check matching EOF2 and print string
*		   for HDR3-9/EOF3-9 labels)
*		4. If D is used as an option in writing, a default
*		   blocksize of 2048 is chosen, based on the ISO
*		   recommendation for maximum blocksize.  Line length
*		   is limited to 255 bytes, which seems a reasonable
*		   maximum.
*
* 8/30/84	eac (cucca) Set default blocksize to 8192.  Ignore ISO
*		recommendation for 'D' tapes.
*/

#define CUCCA
#define VARIAN
#define PIP 

#ifdef VARIAN
#define DEBUG	  0	/* set to nonzero to debug new Varian code */
#define DBG       if(DEBUG) fprintf(stderr,
#define WILDCARD '*'
#endif VARIAN

#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>

/* ansitar -- archiver for ansi-format labelled tapes
 *
 * ansitar [crxtvbl] [blocksize] [labelsize] file ...
 *
 * The options are similar to tar:
 *	c - create a tape
 *	r - replace (update) a tape
 *	x - extract files
 *	t - print table of contents
 *	v - set verbose mode: for crx, print names; for t print labels
 *	0-9 - select drive 0-9
 *	b - use next argument as a block size
 *	B - use next argument as tape block size, and
 *		following argument as line size (for blocking/unblocking)
 *	l - use next arg as a label size
 *	U - select upper(out)/lower(in) case translation of names
 *	V - use next argument as a VSN
 *	R - use RT11 label and name conventions (UGH!)
 *	S - use RSTS conventions (==RT11 except 80-byte labels)
 *	P - create/read files in "pip" (FILES-11) counted format.
 *	D - create/read files with variable length records (D format)
 *
 * When extracting from tape to UNIX, the filename may contain * to 
 * indicate a wild card; however, the asterisk should be enclosed 
 * in quotes, however, to prevent the shell from interpreting it.  
 *
 * Examples of usage:
 *	ansitar tv			Verbose table of contents
 *	ansitar xvDUb 2000 file1 file2  Extract file1 and file2 from
 *					tape with 2000 bytes/block
 *					and variable length records
 *
 * BUGS:
 *
 * Warning: The routines skipfile and backspace use nonstandard
 * (4.1BSD) ioctl calls!
 */

#define	Skiparg()	argc--; argv++
#define	MAXLINE	256
#define	TRUE	1
#define	FALSE	0
#define	READ	0
#define	READWRITE	2
#define ISOBLOCK	2048

/* field sizes */
#define	LABID	3
#define	SERIAL	6
#define	OWNER	14
#define	FILEID	17
#define	SETID	6
#define	SECNO	4
#define	SEQNO	4
#define	GENNO	4
#define	GENVSNO	2
#define	CRDATE	6
#define	EXDATE	6
#define	BLOCKS	6
#define	SYSTEM	13
#define	BLKMAX	5
#define	RECMAX	5
#define	BYTOFFS	2
#define DBYTES	4

/* pad fields (reserved for future use) */
#define	VRES1	20
#define	VRES2	6
#define	VRES3	28
#define	HRES1	7
#define	H2RES1	35
#define	H2RES2	28

/* Volume header label */
struct vol {
	char	v_labid[LABID];		/* label identifier "VOL" */
	char	v_labno;		/* label number */
	char	v_serial[SERIAL];	/* volume serial number */
	char	v_access;		/* accessibility */
	char	v_res1[VRES1];		/* reserved for future use */
	char	v_res2[VRES2];		/* reserved for future use */
	char	v_owner[OWNER];		/* owner identifier */
	char	v_res3[VRES3];		/* reserved for future use */
	char	v_stdlabel;		/* standard label flag */
	};




/* file header/eof label */
struct hdr {
	char	h_labid[LABID];		/* label identifier: "HDR" or "EOF" */
	char	h_labno;		/* label number */
	char	h_fileid[FILEID];	/* file identifier */
	char	h_setid[SETID];		/* file set identifier */
	char	h_secno[SECNO];		/* file section number */
	char	h_seqno[SEQNO];		/* file sequence number */
	char	h_genno[GENNO];		/* generation number */
	char	h_genvsno[GENVSNO];	/* generation vsn number */
	char	h_crdate[CRDATE];	/* creation date */
	char	h_exdate[EXDATE];	/* expiration date */
	char	h_access;		/* accessibility */
	char	h_blocks[BLOCKS];	/* block count */
	char	h_system[SYSTEM];	/* system code */
	char	h_x1[7];		/* reserved */
	};

/* deblocking hdr2/eof2 label */
struct hd2 {
	char	h2_labid[LABID];	/* label identifier: "HDR" or "EOF" */
	char	h2_labno;		/* label number "2" */
	char	h2_datatype;		/* type identifier "D" or "F" */
	char	h2_blkmax[BLKMAX];	/* maximum block size in bytes */
	char	h2_recmax[RECMAX];	/* maximum record size in bytes */
	char	h2_x1[H2RES1];		/* reserved */
	char	h2_bytoffs[BYTOFFS];	/* extra stuff at start of block */
	char	h2_x2[H2RES2];		/* reserved */
	};



struct vol vol1;
struct hdr hdr1, eof1;
struct hd2 hdr2, eof2;

char *tapefile = "/dev/rmt8";
char *buffer, *linebuffer, *malloc(), *realloc(), *index();
char **filetab;
#ifdef CUCCA
unsigned blocksize = 8192;
#else
unsigned blocksize = 512;
#endif CUCCA
unsigned linesize = 0;
int bfactor = 0;
int labelsize = sizeof(struct vol);

int tapeunit = 0;
int tf;

char	*defvsn = "";

char curdate[CRDATE+1];
char *alongtime = " 99364";

#ifdef PIP
int create, replace, xtract, table, verbose, confirm, pipfile;
#else
int create, replace, xtract, table, verbose, confirm;
#endif

int blocking;
int RT11, Upper;
#ifdef VARIAN
int Varlen;
#endif VARIAN
int Fixlen;

main(argc, argv)
	int argc;
	char **argv;
{
	char *ap;
	int bufsize, openmode;

	if (argc < 2)
		usage();
	Skiparg();

	openmode = READ;
	ap = argv[0];
	Skiparg();
	while (*ap) {
		switch (*ap) {
			case 'c':
				create++;
				openmode = READWRITE;
				break;
			case 'r':
				replace++;
				openmode = READWRITE;
				break;
			case 'x':
				xtract++;
				break;
			case 't':
				table++;
				break;
			case 'v':
				verbose++;
				break;
			case 'w':
				confirm++;
				break;
			case 'b':
			case 'B':
			case 'F':
				blocking++;
				blocksize = atoi(argv[0]);
				if (blocksize <= 0)
					fatal("bad block size %s\n", argv[0]);
				Skiparg();
				if ((*ap == 'B') || (*ap == 'F')) {
					linesize = atoi(argv[0]);
					if (linesize <= 0 ||
					    blocksize % linesize != 0)
						fatal("bad line size %s\n", argv[0]);
					bfactor = blocksize / linesize;
					Skiparg();
					}
				if (*ap == 'F') Fixlen++;
				break;
			case 'l':
				labelsize = atoi(argv[0]);
				if (labelsize < sizeof(struct hdr))
					fatal("label size must be >= %d\n",
						sizeof(struct hdr));
				Skiparg();
				break;
			case 'U':
				Upper++;
				break;
			case 'V':
				defvsn = argv[0];
				if (strlen(defvsn) > SERIAL)
					defvsn[SERIAL] = '\0';
				Skiparg();
				break;
			case 'S':
			case 'R':
				RT11++;
				Upper++;
				if (*ap == 'R')
					labelsize = 512;
				break;
#ifdef VARIAN
			case 'D':
				Varlen++;
				break;
#endif VARIAN
#ifdef PIP
 			case 'P':
 				pipfile++;
 				break;
#endif PIP 
			default:
				if (isdigit(*ap)) {
					tapeunit = *ap;
					break;
					}
				fatal("bad flag: %c\n", *ap);
			}
		ap++;
		}

	filetab = argv;
	filetab[argc] = NULL;

	if (tapeunit)
		tapefile[strlen(tapefile)-1] = tapeunit;
	tf = open(tapefile, openmode);
	if (tf < 0)
		fatal("can't open %s%s\n", tapefile,
			(openmode != READ) ? " for writing" : "");
	bufsize = max(blocksize, labelsize);
	buffer = malloc(bufsize + 10);
	if (buffer == NULL)
		fatal("can't allocate buffer of %d bytes\n", blocksize);
	if (linesize) {
		linebuffer = malloc(linesize + 10);
		if (linebuffer == NULL)
			fatal("can't allocate line buffer of %d bytes\n",
				linesize);
		}
	getansidate(curdate);

	if (verbose && blocking) {
		printf("Blocksize: %d", blocksize);
		if (linesize)
			printf(" Linesize: %d", linesize);
		putc('\n', stdout);
		}

	if (create || replace)
		doupdate(tf);
	else if (xtract)
		doxtract(tf);
	else if (table)
		dotable(tf);
	else
		usage();
	exit(0);
}




usage()
{
	fatal("usage: ansitar crxtvbl [blocksize] [labelsize] file ...\n");
}




dotable(tf)
	int tf;
{
	char fileid[FILEID+1];
	int files, n;
	int blocks;
	int lbseq;
	long bytes;

	getvol(tf, &vol1);
	prtvol(&vol1);
	putc('\n', stdout);
	files = 0;
	lbseq = '1';
	while (gethdr(tf, &hdr1)) {
		files++;
		if (verbose)
			prthdr(&hdr1);
		getmark(tf, &hdr2, lbseq);
		sncpy(fileid, hdr1.h_fileid, FILEID);
		blocks = 0;
		bytes = 0L;
		while ((n = getrec(tf, buffer, blocksize)) > 0) {
			blocks++;
			bytes += n;
			}
		lbseq = '1';
		geteof(tf, &eof1);
		if (verbose)
			prthdr(&eof1);
		getmark(tf, &hdr2, lbseq);
		cmphdreof(&hdr1, &eof1);
		if (verbose)
			putc('\n', stdout);
		printf("t %s %d blocks %D bytes\n", fileid, blocks, bytes);
		if (linesize) 
			printf("\t F-type: blocksize: %d linesize %d\n", blocksize, linesize);
		else
			printf("\t D-type: blocksize: %d\n", blocksize);
		if (verbose)
			putc('\n', stdout);
		}
	printf("\n%d files\n", files);
}




doxtract(tf)
	int tf;
{
	char fileid[FILEID+1];
	FILE *fp;
	long bytes;
	int blocks;
	int n, xall;
	int lbseq;
#ifdef VARIAN
	int newn;
#endif VARIAN

	lbseq = '1';
	xall = (filetab[0] == NULL);
	getvol(tf, &vol1);
	if (verbose)
		prtvol(&vol1);
	while (gethdr(tf, &hdr1)) {
		getmark(tf, &hdr2, lbseq);
		sncpy(fileid, hdr1.h_fileid, FILEID);
		trimsp(fileid);
		if (RT11)
			fromRT11(fileid);

		/* if 1.you're doing all the files or 2. this file matches */
		/* one of the the names in the arglist, then copy it, else */
		/* skip this file. filetab is really 'argv' list.	   */
		if ( (xall || lookup(filetab, fileid, Upper)) &&
		     checkw('x', fileid) ) {
			if (Upper)  makelower(fileid);	/* name to lower case*/
			fp = fopen(fileid, "w");
#ifdef VARIAN
			if (fp == NULL) 
			    {
			    printf("can't create %s - will skip\n", fileid);
			    skipfile(tf);
			    }
#else
			if (fp == NULL) fatal("can't create %s\n", fileid);
#endif VARIAN
			else{	/* file OK, do the copy */
			    blocks = 0;
			    bytes = 0L;
			    while ((n = getrec(tf, buffer, blocksize)) > 0) {
				if (linesize)
					lunblock(fp, buffer, n, linesize);
				else
#ifdef VARIAN
				    if (Varlen)
					{
					newn = varunblock(fp,buffer,n);
					n = newn;  /* now it's # written */
					}
				    else
#endif VARIAN
#ifdef PIP
 				      if (pipfile)
 					pipunblock(fp, buffer, n);
				      else
#endif PIP
					fwrite(buffer, n, 1, fp);
				blocks++;
				bytes += n;
				}
			    fclose(fp);
			    if (verbose)
				printf("x %s %d blocks %D bytes\n",
					fileid, blocks, bytes);
			    }   /* end 'if file open was OK' */
			}	/* end 'if filename OK or copying all' */
		else	/* if this file is not be copied */
			skipfile(tf);

		lbseq = '1';
		geteof(tf, &eof1);
		cmphdreof(&hdr1, &eof1);
		getmark(tf, &hdr2, lbseq);
		}
}




doupdate(tf)
	int tf;
{
	int i, n;
	int blocks;
	long bytes;
	char line[MAXLINE];
	char fileid[FILEID+1];
	char recmax[RECMAX+1];
	FILE *fp;
	int sequence;
 	int lbseq;


	lbseq = '1';
	sequence = 0;
	if (create) {
		initvol(&vol1);
		putvol(tf, &vol1);
		}
	else {	/* replace */
		getvol(tf, &vol1);
		while (gethdr(tf, &hdr1)) {
			sncpy(line, hdr1.h_seqno, SEQNO);
			sequence = atoi(line);
			getmark(tf, &hdr2, lbseq);
			skipfile(tf);
			lbseq = '1';
			geteof(tf, &eof1);
			getmark(tf, &hdr2, lbseq);
			}
		backspace(tf);
		}
	for (i=0; filetab[i] != NULL; i++) {
		if (!checkw('a', filetab[i]))
			continue;
		strncpy(fileid, filetab[i], FILEID);
		fileid[FILEID] = '\0';
		fp = fopen(fileid, "r");
		if (fp == NULL)
			fatal("can't open %s\n", fileid);
		sequence++;
		if (RT11)
			toRT11(fileid);
		if (Upper)
			makeupper(fileid);
		inithdr(&hdr1, fileid, sequence);
		puthdr(tf, &hdr1);
		if (Varlen) {
#ifdef CUCCA
			blocksize = ISOBLOCK;
#endif CUCCA
			realloc(buffer,blocksize+10);
			linesize = 0;
			inithdr2(&hdr2, 'D', blocksize, 0);
			n = dblock(fp, buffer, blocksize);
			utoaz(n, recmax, RECMAX);
			blcopy(hdr2.h2_recmax, recmax, RECMAX);
			puthdr(tf, &hdr2);
			fclose(fp);
			fp = fopen("#tmp.tmp", "r");
			if (fp == NULL) fatal("can't re-read #tmp.tmp\n");
			}
		if (Fixlen) {
			inithdr2(&hdr2, 'F', blocksize, linesize);
			puthdr(tf, &hdr2);
			}
		tapemark(tf);
		blocks = 0;
		bytes = 0L;
		if (linesize)
			while ((n = lblock(fp, buffer, linesize, bfactor)) > 0) {
				if (n % 2) {
					buffer[n] = '\0';
					n++;
					}
				n = write(tf, buffer, n);
				blocks++;
				bytes += n;
				}
#ifdef PIP
 		else if (pipfile)
 			while ((n = pipblock(fp, buffer, blocksize)) > 0)
 			{
 				n = write(tf, buffer, n);
 				blocks++;
 				bytes += n;
 			}
  		     else
#else
		else
#endif PIP
			while ((n = fread(buffer, sizeof(char), blocksize, fp)) > 0) {
				if (n % 2) {
					buffer[n] = '\0';
					n++;
					}
				n = write(tf, buffer, n);
				blocks++;
				bytes += n;
				}
		fclose(fp);
		tapemark(tf);
		blcopy(hdr1.h_labid, "EOF", LABID);
		utoaz(blocks, line, BLOCKS);
		blcopy(hdr1.h_blocks, line, BLOCKS);
		puthdr(tf, &hdr1);
		if ((Varlen) || (Fixlen)) {
			blcopy(hdr2.h2_labid, "EOF", LABID);
			puthdr(tf, &hdr2);
			}
		tapemark(tf);
		if (verbose)
			printf("a %s %d blocks %D bytes\n",
				fileid, blocks, bytes);
		}
	tapemark(tf);
}




getvol(tf, volp)
	int tf;
	struct vol *volp;
{
	int n;

	if (labelsize == sizeof(struct vol))
		n = read(tf, (char *)volp, sizeof(struct vol));
	else {
		n = read(tf, buffer, labelsize);
		bcopy((char *)volp, buffer, sizeof(struct vol));
		}
#ifdef PIP
 	if (n<0)
 	{
 	    perror("Tape read error");
 	    exit(1);
 	}
#endif PIP
	if (n != labelsize ||
	    strncmp(volp->v_labid, "VOL", LABID) != 0 ||
	    volp->v_labno != '1') {
		printf("Warning: Volume label (VOL1) missing\n");
		backspace(tf);
		return;
		}

	/* check for RT11 boot block between VOL1 and first HDR1 */
	if (RT11) {
		/* must have labelsize = 512 */
		n = read(tf, buffer, labelsize);
		bcopy((char *)&hdr1, buffer, sizeof(struct hdr));
		if (n == labelsize && strncmp(hdr1.h_labid, "HDR", LABID) == 0)
			backspace(tf);
		else
			printf("Possible RT11 bootstrap block.\n");
		}
}




int
gethdr(tf, hdrp)
	int tf;
	struct hdr *hdrp;
{
	int n;

	if (labelsize == sizeof(struct hdr))
		n = read(tf, (char *)hdrp, sizeof(struct hdr));
	else {
		n = read(tf, buffer, labelsize);
		bcopy((char *)hdrp, buffer, sizeof(struct hdr));
		}
	if (n == 0)
		return(FALSE);
	if (n != labelsize ||
	    strncmp(hdrp->h_labid, "HDR", LABID) != 0 ||
	    hdrp->h_labno != '1')
		hdrerr(tf, hdrp);
	return(TRUE);
}




hdrerr(tf, hdrp)
	int tf;
	struct hdr *hdrp;
{
	int found, n;

	printf("Warning: File label (HDR1) error - skipping\n");
	found = FALSE;
	while (!found) {
		skipfile(tf);
		if (labelsize == sizeof(struct hdr))
			n = read(tf, (char *)hdrp, sizeof(struct hdr));
		else {
			n = read(tf, buffer, labelsize);
			bcopy((char *)hdrp, buffer, sizeof(struct hdr));
			}
		if ((n == labelsize) &&
		    (strncmp(hdrp->h_labid, "HDR", LABID) == 0))
			found = TRUE;
		}
}




geteof(tf, eofp)
	int tf;
	struct hdr *eofp;
{
	int n;

	if (labelsize == sizeof(struct hdr))
		n = read(tf, eofp, sizeof(struct hdr));
	else {
		n = read(tf, buffer, labelsize);
		bcopy((char *)eofp, buffer, sizeof(struct hdr));
		}
	if (n != labelsize ||
	    strncmp(eofp->h_labid, "EOF", LABID) != 0 ||
	    eofp->h_labno != '1')
		printf("Warning: File label (EOF1) error\n");
}




int
getrec(f, buf, size)
	int f;
	char *buf;
	int size;
{
	int n;

	n = read(f, buf, size);
	if (n < 0)
		fatal("Read error (record may be larger than %db)\n",
			size);
	return(n);
}




getmark(tf, hdr2p, labseq)
	int tf;
	int labseq;
	struct hd2 *hdr2p;
{
	char rec[sizeof(struct hdr)];
	char blkmax[BLKMAX+1], recmax[RECMAX+1];
	char lstring[sizeof(struct hdr) + 1];
	int n;
	int s;

	s = labseq;
        s++;
	n = read(tf, rec, sizeof(rec));
	if (n == 0)
		return;
	if (n == sizeof(struct hdr) && (strncmp("HDR", rec, 3)==0)) {
		bcopy((char *)hdr2p, rec, sizeof(struct hdr));
		if (hdr2p->h2_labno == s) {
			sncpy(blkmax, hdr2p->h2_blkmax, BLKMAX);
			sncpy(recmax, hdr2p->h2_recmax, RECMAX);
			if ((verbose) && (table)) 
				prthdr2(&hdr2);
			blocksize = max(blocksize, atoi(blkmax));
			if (hdr2p->h2_datatype == 'D') {
				linesize = 0;
				Varlen = TRUE; }
			else  { linesize = max(linesize, atoi(recmax));
				Varlen = FALSE; }
			realloc(buffer,blocksize+10);
			}
		}
	else if ((verbose) && (table)) {
		sncpy(lstring, rec, sizeof(struct hdr));
		printf("%s\n", lstring);
		}
	/* skip HDR3-9 */
	while (n == sizeof(struct hdr) &&
	      (strncmp("HDR", rec, 3)==0 || strncmp("EOF", rec, 3)==0)) {
		n = read(tf, rec, sizeof(rec));
		if (n == 0)
			return;
		else if ((verbose) && (table)) {
			sncpy(lstring, rec, sizeof(struct hdr));
			printf("%s\n", lstring);
			}
		}
	printf("Warning: tape mark missing\n");
}




cmphdreof(hdrp, eofp)
	struct hdr *hdrp, *eofp;
{
	char line[MAXLINE];

	static int len = FILEID+SETID+SECNO+SEQNO+GENNO+GENVSNO+
			CRDATE+EXDATE+1;

	if (strncmp(hdrp->h_fileid, eofp->h_fileid, len) != 0 ||
	    strncmp(hdrp->h_system, eofp->h_system, SYSTEM) != 0) {
		sncpy(line, hdrp->h_fileid, FILEID);
		fprintf(stderr, "Warning: HDR and EOF labels for %s disagree\n",
			line);
		}
}




putvol(tf, volp)
	int tf;
	struct vol *volp;
{
	int len;

	if (labelsize == sizeof(struct vol)) {
		write(tf, volp, sizeof(struct vol));
		return;
		}
	bcopy(buffer, (char *)volp, sizeof(struct vol));
	len = labelsize - sizeof(struct vol);
	blcopy(&buffer[sizeof(struct vol)], "", len);
	write(tf, buffer, labelsize);
}




puthdr(tf, hdrp)
	int tf;
	struct hdr *hdrp;
{
	int len;

	if (labelsize == sizeof(struct hdr)) {
		write(tf, hdrp, sizeof(struct hdr));
		return;
		}
	bcopy(buffer, (char *)hdrp, sizeof(struct hdr));
	len = labelsize - sizeof(struct hdr);
	blcopy(&buffer[sizeof(struct hdr)], "", len);
	write(tf, buffer, labelsize);
}




prtvol(volp)
	struct vol *volp;
{
	char labid[LABID+1], serial[SERIAL+1], owner[OWNER+1];

	sncpy(labid, volp->v_labid, LABID);
	sncpy(serial, volp->v_serial, SERIAL);
	sncpy(owner, volp->v_owner, OWNER);
	printf("Volume label:\n");
	printf("\tLabel: %s%c Serial: %s  Access: %c\n",
		labid, volp->v_labno, serial, volp->v_access);
	printf("\tOwner: %s  Standard: %c\n",
		owner, volp->v_stdlabel);
}




prthdr(hdrp)
	struct hdr *hdrp;
{
	char labid[LABID+1], fileid[FILEID+1], setid[SETID+1];
	char secno[SECNO+1], seqno[SEQNO+1];
	char genno[GENNO+1], genvsno[GENVSNO+1];
	char crdate[CRDATE+1], exdate[EXDATE+1];
	char blocks[BLOCKS+1], system[SYSTEM+1];

	sncpy(labid, hdrp->h_labid, LABID);
	sncpy(fileid, hdrp->h_fileid, FILEID);
	sncpy(setid, hdrp->h_setid, SETID);
	sncpy(secno, hdrp->h_secno, SECNO);
	sncpy(seqno, hdrp->h_seqno, SEQNO);
	sncpy(genno, hdrp->h_genno, GENNO);
	sncpy(genvsno, hdrp->h_genvsno, GENVSNO);
	sncpy(crdate, hdrp->h_crdate, CRDATE);
	sncpy(exdate, hdrp->h_exdate, EXDATE);
	sncpy(blocks, hdrp->h_blocks, BLOCKS);
	sncpy(system, hdrp->h_system, SYSTEM);

	printf("File Label:\n");
	printf("\tLabel: %s%c  File: %s\n",
		labid, hdrp->h_labno, fileid);
	printf("\tSet: %s  Section: %s  Sequence: %s\n",
		setid, secno, seqno);
	printf("\tGeneration: %s  Generation Version: %s\n",
		genno, genvsno);
	printf("\tCreated: %s  Expires: %s  Access: %c\n",
		crdate, exdate, hdrp->h_access);
	printf("\tBlocks: %s  System: %s\n",
		blocks, system);
}

prthdr2(hdrp)
	struct hd2 *hdrp;
{
	char labid[LABID+1]; 
	char blkmax[BLKMAX+1], recmax[RECMAX+1];
	char bytoffs[BYTOFFS+1];

	sncpy(labid, hdrp->h2_labid, LABID);
	sncpy(blkmax, hdrp->h2_blkmax, BLKMAX);
	sncpy(recmax, hdrp->h2_recmax, RECMAX);
	sncpy(bytoffs, hdrp->h2_bytoffs, BYTOFFS);

	printf("File Label:\n");
	printf("\tLabel: %s%c  Datatype: %c\n",
		labid, hdrp->h2_labno, hdrp->h2_datatype);
	printf("\tMaximum block size: %s  Maximum record size: %s\n",
		blkmax, recmax);
	printf("\tByte offset at head of block: %s\n",
		bytoffs);
}




initvol(volp)
	struct vol *volp;
{
	struct passwd *passwp, *getpwuid();

	blcopy(volp, "", sizeof(struct vol));

	blcopy(volp->v_labid, "VOL", LABID);
	volp->v_labno = '1';
	blcopy(volp->v_serial, defvsn, SERIAL);
	volp->v_access = ' ';
	passwp = getpwuid(getuid());
	blcopy(volp->v_owner, passwp->pw_name, OWNER);
	if ((Varlen) || (Fixlen))
		volp->v_stdlabel = '3'; 
	else 
		volp->v_stdlabel = '1';
}




inithdr(hdrp, filename, seq)
	struct hdr *hdrp;
	char *filename;
	int seq;
{
	char seqno[SEQNO+1];

	blcopy(hdrp, "", sizeof(struct hdr));

	blcopy(hdrp->h_labid, "HDR", LABID);
	hdrp->h_labno = '1';
	blcopy(hdrp->h_fileid, filename, FILEID);
	blcopy(hdrp->h_secno, "0001", SECNO);
	utoaz(seq, seqno, SEQNO);
	blcopy(hdrp->h_seqno, seqno, SEQNO);
	blcopy(hdrp->h_genno, "0001", GENNO);
	blcopy(hdrp->h_genvsno, "00", GENVSNO);
	blcopy(hdrp->h_crdate, curdate, CRDATE);
	blcopy(hdrp->h_exdate, alongtime, EXDATE);
	blcopy(hdrp->h_blocks, "000000", BLOCKS);
	blcopy(hdrp->h_system, "Unix V7", SYSTEM);
}

inithdr2(hdr2p, rectype, b, r)
	struct hd2 *hdr2p;
	int rectype;
	int b, r;
{
	char blkmax[BLKMAX+1];
	char recmax[RECMAX+1];

	blcopy(hdr2p, "", sizeof(struct hdr));

	blcopy(hdr2p->h2_labid, "HDR", LABID);
	hdr2p->h2_labno = '2';
	hdr2p->h2_datatype = rectype;
	utoaz(b, blkmax, BLKMAX);
	blcopy(hdr2p->h2_blkmax, blkmax, BLKMAX);
	utoaz(r, recmax, RECMAX);
	blcopy(hdr2p->h2_recmax, recmax, RECMAX);
	blcopy(hdr2p->h2_bytoffs, "00", BYTOFFS);
}


int
dblock(fp,buffer,blocksize)
    FILE *fp;
    char *buffer;
    int blocksize;
{
    FILE *fs;
    char dbuffer[MAXLINE+5];
#if DEBUG
    char tbuffer[ISOBLOCK + 1];
#endif
    char *bb, *db;
    int n, nl, nlm;
    int brem;
    int nf;
    int of = 0;

    fs = fopen("#tmp.tmp", "w");
    if (fs == NULL) fatal("can't create #tmp.tmp\n");
    bb = buffer; brem = blocksize + 1; n = 1; nlm = 0;
    while (n) {
        n = drecord(fp, dbuffer); 
        nlm = max(nlm, n);
#if DEBUG
        printf(" %s\n in dbuffer  %d brem\n", dbuffer, brem);
#endif
        if ((n) && ((brem -= n) > 0)) { 
            for(nl = 0; nl < n; ++nl) *bb++ = dbuffer[nl];  
#if DEBUG
		*bb = 0; printf("first try %s\n", buffer);
#endif
            } 
        else {     
	    brem += n;
#if DEBUG
            printf(" %d fill bytes needed\n", brem);
#endif
            while (brem-- > 0) *bb++ = '^'; 
            bb = buffer; nl = n; 
#if DEBUG
		strncpy(tbuffer, buffer, blocksize);printf("%s\n", tbuffer);
#endif
fprintf(stderr,"fs = %x, of = %d\n",fs,of++);
            nf = fwrite(buffer, blocksize, 1, fs); 
            if (n) { 
                for(nl = 0; nl < n; ++nl) *bb++ = dbuffer[nl];  
#if DEBUG
		strncpy(tbuffer, buffer, blocksize);printf("%s\n", tbuffer);
#endif
                brem = blocksize - n;
                } 
            }
        } 
    fclose(fs); 
    return(nlm); 
}



int
drecord(fp, lbuffer)
	FILE *fp;
	char *lbuffer;

{
	char count[DBYTES+1];
	int n;
	
	n = filldln(fp, lbuffer);
	utoaz(n, count, DBYTES);
	bcopy(lbuffer, count, DBYTES);
	return(n);
}




int
filldln(fp, lbuf)
	FILE *fp;
	char *lbuf;

{
	int i, linelim;
	register int c;
	register char *lb;

	lb = lbuf;
	linelim = MAXLINE+DBYTES;
	i = DBYTES;
	lb += DBYTES;
	while ((c = getc(fp)) && (c != EOF) && (c != '\n')) {
		*lb++ = c;
		if (i++ > linelim-1) break;
		}
	*lb = '\0';
	if (c == '\n') return(i);
	if (c == EOF) return(0);
	printf("%d byte line too long; Broken into parts.\n", i);
        return(i);
}


int
lblock(fp, buffer, lsize, bfactor)
	FILE *fp;
	char *buffer;
	int lsize, bfactor;
{
	register char *lp, *linelim;
	register int c;
	int i;
	char *bp;

	bp = buffer;
	for (i=0; i<bfactor; i++) {
		lp = bp;
		linelim = &lp[lsize];
		while (lp < linelim) {
			c = getc(fp);
			if (c == '\n' || c == EOF)
				break;
			*lp++ = c;
			}
		if (c == EOF && lp == bp)
			break;
		while (c != '\n' && c != EOF)
			c = getc(fp);
		while (lp < linelim)
			*lp++ = ' ';
		bp += lsize;
		}
	return(bp - buffer);
}




lunblock(fp, buffer, blen, lsize)
	FILE *fp;
	char *buffer;
	int blen, lsize;
{
	register char *lastp, *bp1;
	char *bp, *buflim;

	buflim = &buffer[blen];
	bp = buffer;
	while (bp < buflim) {
		lastp = &bp[lsize-1];
		while (lastp >= bp && isspace(*lastp))
			lastp--;
		for (bp1=bp; bp1<=lastp; bp1++)
			putc(*bp1, fp);
		putc('\n', fp);
		bp += lsize;
		}
}




blcopy(dest, src, n)
	char *dest, *src;
	int n;
{
	int i;

	i=0;
	while (i < n && *src) {
		*dest++ = *src++;
		i++;
		}
	while (i++ < n)
		*dest++ = ' ';
}




sncpy(dest, src, n)
	char *dest, *src;
	int n;
{
	int i;

	i = 0;
	while (*src && i<n) {
		*dest++ = *src++;
		i++;
		}
	*dest = '\0';
}



utoaz(n, buf, size)
	int n;
	char *buf;
	int size;
{
	char *p;

	p = &buf[size-1];
	while (p >= buf && n != 0) {
		*p = '0' + (n % 10);
		n /= 10;
		p--;
		}
	while (p >= buf) {
		*p = '0';
		p--;
		}
}




tapemark(tf)
	int tf;
{
	struct mtop mtop;

	mtop.mt_count = 1;
	mtop.mt_op = MTWEOF;
	ioctl(tf, MTIOCTOP, &mtop);
}




skipfile(tf)
	int tf;
{
	struct mtop mtop;

	mtop.mt_count = 1;
	mtop.mt_op = MTFSF;
	ioctl(tf, MTIOCTOP, &mtop);
}




backspace(tf)
	int tf;
{
	struct mtop mtop;

	mtop.mt_count = 1;
	mtop.mt_op = MTBSR;
	ioctl(tf, MTIOCTOP, &mtop);
}




/* getansidate -- return the current date in ansi format
 *
 * Ansi dates are strings of the form " yyddd" where
 * yy = the year and ddd = the day in the year.  There must
 * be an initial blank.
 */

getansidate(curdate)
	char *curdate;
{
	time_t now, time();
	struct tm *timep, *localtime();

	now = time(NULL);
	timep = localtime(&now);
	curdate[0] = ' ';
	utoaz(timep->tm_year, &curdate[1], 2);
	utoaz(timep->tm_yday, &curdate[3], 3);
	curdate[6] = '\0';
}




int
lookup(tab, name, Upper)
	char *tab[];
	char *name;
	int Upper;
{
	int i;
	char lower[MAXLINE];

	if (Upper) {
		strcpy(lower, name);
		makelower(lower);
		}
	for (i=0; tab[i] != NULL; i++)
#ifdef VARIAN
		if (wildcmp(tab[i], name, WILDCARD) == 0 ||
		    (Upper && wildcmp(tab[i], lower, WILDCARD)==0))
#else
		if (strcmp(tab[i], name) == 0 ||
		    (Upper && strcmp(tab[i], lower)==0))

#endif VARIAN
			return(TRUE);
	return(FALSE);
}




makelower(s)
	char *s;
{
	register char *p;

	p = s;
	while (*p) {
		if (isupper(*p))
			*p = tolower(*p);
		p++;
		}
}




makeupper(s)
	char *s;
{
	register char *p;

	p = s;
	while (*p) {
		if (islower(*p))
			*p = toupper(*p);
		p++;
		}
}




int
haslower(p)
register char *p;
{

	while (*p) {
		if (islower(*p))
			return(TRUE);
		p++;
		}
	return(FALSE);
}




toRT11(name)
	char *name;
{
	char buf[32], *extp;

	extp = index(name, '.');
	if (extp != NULL) {
		*extp = '\0';
		extp++;
		}
	blcopy(buf, name, 6);
	buf[6] = '.';
	if (extp != NULL)
		blcopy(&buf[7], extp, 3);
	else
		strcpy(&buf[7], "ext");
	buf[10] = '\0';
	strcpy(name, buf);
}




fromRT11(name)
	char *name;
{
	char *op, *np;

	op = np = name;
	while (*op) {
		if (!isspace(*op))
			*np++ = *op;
		op++;
		}
	*np = '\0';
}





trimsp(s)
	char *s;
{
	register char *p;

	p = &s[strlen(s)-1];
	while (p >= s && isspace(*p))
		p--;
	*++p = '\0';
}




bcopy(dest, src, size)
	char *dest, *src;
	int size;
{
	while (size-- > 0)
		*dest++ = *src++;
}




int
max(a, b)
	int a, b;
{
	if (a > b)
		return(a);
	return(b);
}




int
checkw(c, name)
	char c, *name;
{
	if (!confirm)
		return(TRUE);
	printf("%c %s:", c, name);
	c = getchar();
	if (c != '\n')
		while ((c = getchar()) != '\n')
			;
	return(c == 'y' || c == 'Y');
}



/* VARARGS */
fatal(s, a1, a2, a3, a4)
	char *s;
{
	fprintf(stderr, "ansitar: ");
	fprintf(stderr, s, a1, a2, a3, a4);
	exit(1);
}

#ifdef VARIAN
/* varunblock - unblock variable length records ("D" format)	
 * Beginning of each record contains a 4 digit number which is the
 *   length of the record (including the count).
 * There are no line terminators (CR or LF).
 */
varunblock(fp, buffer, blen)
	FILE *fp;
	char *buffer;
	int blen;
{	/* the function now returns the # of bytes it decided to write */
	register char *lastp, *bp1;
	char *bp, *buflim;
	int count;
	int newblen;	/* this will count the actual bytes written out */

	buflim = &buffer[blen];
	bp = buffer;
	newblen = 0;
#if DEBUG	
	DBG" varunblock called. length %d\n",blen);
	if(DEBUG)hexdmp(" buffer passed to varunblock",buffer,blen);
	DBG"  ");
#endif DEBUG

	while (bp < buflim) {
#if DEBUG
		DBG"  count being made from %2x %2x %2x %2x %2x\n",
			*bp,*(bp+1),*(bp+2),*(bp+3),*(bp+4));
#endif
		sscanf(bp,"%4d",&count);
		count = count - 4;	/* count includes the count itself */
#if DEBUG
		DBG"  count %3d\n     ",count);
#endif
		if (*bp == 0x5e) 	/* if fill encountered for length */
			{		/* then abandon this record */
#if DEBUG
			DBG" hit filler. assume eor. break\n");
#endif
			break;
			}
		bp = bp+4;		/* point to the end of line        */
		lastp = bp + count;
		while (bp<lastp)
			{
#if DEBUG
			if(DEBUG) putc(*bp, stderr);
#endif
			putc(*bp++, fp);
			newblen++;
			}
#if DEBUG
		if(DEBUG) putc('\n', stderr);
#endif
		putc('\n', fp);
		newblen++;
		}

	return(newblen);
}



wildcmp(wilds,str,wc)	/* compare 2 strings. 1st can have a wildcard '*' */
char   *wilds;		/* input string possibly containing a wildcard chr*/
char   *str;		/* 2nd input string.  wild chrs in it aren't 'wild'*/
char    wc;		/* the 'wild card' character, often a '*'	  */
{	/* function return is 0 if equal (like strcmp). < > not supported */
	char  *l,*s;
	int    flag;	    /* will watch for multiple *'s in 'wild'*/
	char   wcrd;

	if (strlen(str) == 0) return(-1);	/* don't match phantom */
	wcrd = wc & 127;

	flag = 0;	/* wildcard not encountered yet */
	for (l=wilds,s=str ; *l != '\0' ;     )
	    {
	    if (*l == wcrd)	/* if wildcard emcountered, adjust pointers*/
		{
		if (flag)  return(0);
		flag = 1;
		l++;	/* skip the wildcard */
		s = str + strlen(str) - (strlen(wilds) - (l - wilds) );
		}
	    else{	/* no wildcard, increment pointers normally */
	    	if (*l != *s) return(*l - *s);
		l++;
		s++;
		}
	    }

	if (*s != '\0') return(*l - *s);
	return(0);	/* returning "success" indicator. they were equal */
}
#endif VARIAN

#ifdef PIP
 
 /*****************************************************************
  * TAG( pipunblock )
  * 
  * Unblock pip records.  These are in a counted format with a 4 byte
  * count, followed by the record.  (Count includes the length of the
  * count).  Blank space at the end of a block is filled with '^'
  * characters.
  */
 
 pipunblock(fp, buffer, n)
 FILE *fp;
 char *buffer;
 {
     register char *cp;
     int len, i;
 
     for (cp=buffer; cp<buffer+n; )
     {
 	for (i=0, len=0; i<4; i++)
 	    len = len*10 + *cp++ - '0';
 	for (len -= 4; len > 0; len--)
 	    putc(*cp++, fp);
 	putc('\n', fp);
 	while (*cp == '^' && cp<buffer+n)
 	    cp++;
     }
 }
 
 /*****************************************************************
  * TAG( pipblock )
  * 
  * Read lines from the input file and block them pip-style into the
  * buffer.  Make logical blocks of 512 bytes and fill space at the end
  * of the logical blocks with '^' characters.
  */
 
 pipblock(fp, buffer, blocksize)
 FILE *fp;
 char *buffer;
 {
     register char *cp;
     static char linebuf[512];	     /* lines not allowed longer than this? */
     static int nline = 0;
     int n;
 
     if (blocksize%512)
     {
 	fprintf(stderr, "Blocksize must be a multiple of 512 for pip tapes\n");
 	exit(1);
     }
     for (cp=buffer; cp<(buffer+blocksize); )
     {
 	if (nline <= 0)
 	    if (fgets(linebuf, 508, fp) == NULL)
 		nline = -1;
 	    else
 	    {
 		linebuf[508] = '\0';
 		nline = strlen(linebuf);
 		if (linebuf[nline-1] == '\n')
 		    linebuf[--nline] = '\0';
 	    }
 
 	if (nline > (508 - (cp-buffer)%512) ||
 	    (nline < 0 && (cp-buffer)%512 != 0))
 	    while ( (cp-buffer)%512 != 0 )
 		*cp++ = '^';
 
 	if (cp-buffer >= blocksize || nline < 0)
 	    return cp-buffer;
 
 	sprintf(cp, "%04d", nline+4);
 	cp += 4;
 	strncpy(cp, linebuf, 508);
 	cp += nline;
 	nline = 0;
     }
 
     return cp-buffer;	       /* should never get here, but you never know */
}
#endif PIP
