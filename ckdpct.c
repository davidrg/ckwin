/* The boo and de-boo programs do not work, and I found them too cumbersome
 * and complicated to install on the DG.  In their place, I have provided
 * a few other tools:
 *
 *      uuencode and uudecode:  
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

/*
 * MSBPCT.C
 *
 * Howie Kaye -- Columbia University 3/11/86
 *
 * sibling program to MSBMKB.C.  It is used to unpack BOO files, used for
 * encoding binary files into text, and back.  This program does the decoding.
 * It is meant to replace the program "MSBPCT.BAS", and runs approximately
 * 200 times faster.
 *
 * For documentation on BOO file format, see MSBMKB.C
 * This program runs, as is, under Microsoft C on MSDOS, and under UNIX(4.2).
 *
 * Modification history:
 *
 * 3/23/86 - Davide P. Cervone -- University of Rochester
 *   added AMIGA and VAX11C support
 *
 * 3/24/86 - Martin Knoblauch -- TH-Darmstadt              (MK001)
 *   test if 1. line of inputfile is delimited by "\r\n" instead of "\n"
 *
 * 5/5/86 - John Matthews, U of Delaware.
 *   Explicitly close the files.
 *
 * 5/8/86 - L. John Junod, DTNSRDC.
 *   Adapt for Computer Innovations MS-DOS CI-86 Compiler
 *   Improve too many args error message
 */
 
/***
#define MSDOS
#define CI86
***/
 
#include <stdio.h>
 
#ifdef AMIGA
#include <fcntl.h>
#else
#endif
 
#ifdef MSDOS 
#ifndef CI86
#include <fcntl.h>
#endif
#else
#ifdef vax11c
#include <file.h>
#else
#ifdef datageneral
#include <fcntl.h>
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
#define write(filen,chs,len) dg_binw(filen,chs,len)
#else
#include <sys/file.h>
#endif
#endif
#endif
 
#define fixchr(x) ((x) -'0')
#define NULLCHR fixchr('#')
 
yes_or_no_p(arg) char *arg; {
  int c,x;
  while (1) {
    printf("%s",arg);
    c = getchar();
    if (c == '\n') continue;
    while ((x = getchar()) != '\n')
      if (x == EOF) return(0);
    if ((c == 'Y') || (c == 'y')) return(1);
    if ((c == 'N') || (c == 'n') || (c == EOF)) return(0);
    printf("Please answer 'Y' or 'N'\n");
  }
}
 
main(argc,argv) char **argv; {
#ifdef AMIGA
  char *infile = "CKIKER.BOO";          /* input file name, with default */
#else
#ifdef datageneral
  char *infile = "CKDKER.BOO";          /* input file name, with default */
#else
  char *infile = "MSKERMIT.BOO";        /* input file name, with default */
#endif
#endif
  char outfile[100];                    /* output file name */
  FILE *ifp, *ofp;                      /* i/o files */
  char inline[100],outline[200];
  int f;
 
  if (argc > 2) {                       /* check for too many args */
    printf("Too many args. Usage: msbpct [inputfile]\n");
    exit(1);
  }
  if (argc > 1) {                       /* check for input file */
    infile = argv[1];
  }
#ifdef datageneral
  /* Initialize the i/o block for putc() */
  zero((char *) &w_io_parms, sizeof(w_io_parms));
  w_io_parms.isti = $IBIN|$RTDY|$ICRF|$OFOT;
  w_io_parms.isti &= ~$IPST;
  w_io_parms.imrs = 2048;
  w_io_parms.ibad = -1;
  w_io_parms.ircl = -1;

  if ((ifp = dg_open(infile,$OFIN|$RTDS|$ICRF,$FUDF)) == NULL) {
#else
  if ((ifp = fopen(infile,"r")) == NULL) { /* open input file */
#endif
    printf("%s not found.\n",infile);   /* failure? */
    exit(1);
  }
 
  fgets(outfile,100,ifp);               /* get output file name */
  if ((outfile[strlen(outfile)-2] == '\r')|    /* MK001 */
      (outfile[strlen(outfile)-2] == '\n')) {
    outfile[strlen(outfile)-2] = '\0';
  }
  else {
    outfile[strlen(outfile)-1] = '\0';
  }
 
  if ((ofp = fopen(outfile,"r")) != NULL) {
    char msg[100];
    sprintf(msg,"output file '%s' already exists.  continue (y/n)? ",outfile);
    if (!yes_or_no_p(msg)) {
      printf("ok.  bye\n");
      exit(0);
    }
    else {
      fclose(ofp);
    }
  }
 
#ifndef MSDOS
#ifndef O_BINARY
#define O_BINARY 0
#endif
#endif
 
#ifdef AMIGA
  if ((ofp = fopen(outfile,"w")) == NULL) {
    printf("could not open %s\n",outfile); /* failure */
    exit(0);
  }
#else
#ifdef CI86
   if((ofp = fopen(outfile,"wb")) == NULL){
    printf("could not open %s\n",outfile); /* failure */
    exit(0);
   }
#else
#ifdef datageneral
   if ((ofp = dg_open(outfile,$OFOT|$IBIN|$OFCR|$OFCE|$RTUN,$FUNX)) == NULL) {
   /* if((ofp = fopen(outfile,"k")) == NULL){ */
    perror("Open outfile");
    printf("Could not open %s\n",outfile); /* failure */
    exit(0);
   }
#else
  f = open(outfile,O_CREAT|O_WRONLY|O_TRUNC|O_BINARY,0x1ff);
  if ((ofp = fdopen(f,"w")) == NULL) { /* open it */
    printf("could not open %s\n",outfile); /* failure? */
    exit(1);
  }
#endif
#endif
#endif
  printf("%s ==> %s\n",infile,outfile); /* announce our intentions */
 
  while(fgets(inline,100,ifp) != NULL) { /* till EOF */
    int index=0,outindex=0;
    while (index < strlen(inline) && inline[index] != '\n' &&
           inline[index] != '\r')       /* end of line? */
      if (fixchr(inline[index]) == NULLCHR) {   /* null compress char... */
        int rptcnt;
        int i;
 
        index++;
        rptcnt = fixchr(inline[index]); /* get repeat count */
        for (i=0; i<rptcnt; i++)        /* output the nulls */
          putc('\0',ofp);
        index++;                        /* pass the count field */
      }
      else {                            /* a quad to decode... */
        int a, b, c ,d;
 
        a = fixchr(inline[index]);
        index++;
        b = fixchr(inline[index]);
        index++;
        c = fixchr(inline[index]);
        index++;
        d = fixchr(inline[index]);
        index++;
 
                                        /* output the bytes */
        putc(((a*4)+(b/16)) & 255,ofp);
        putc(((b*16)+(c/4)) & 255,ofp);
        putc(((c*64)+d) & 255,ofp);
      }
    }
#ifdef AMIGA
    putc('\032',ofp);                   /* final ^Z for micros*/
#endif
#ifdef MSDOS
    putc('\032',ofp);                   /* final ^Z */
#endif
 
    fclose(ofp);			/* Close the files */
    fclose(ifp);
}					/* End of program */


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



