/* DEC/CMS REPLACEMENT HISTORY, Element CKVFIO.C */
/* *5    29-AUG-1989 00:31:53 BUDA "Add code to check for system() function by version of VAXC" */
/* *4    12-JUN-1989 23:09:43 BUDA "Add encode logic" */
/* *3    18-APR-1989 23:36:23 BUDA "#ifdef chkfn and code gtdir for V4" */
/*  2U1  18-APR-1989 22:12:00 BUDA "Work on attribute packet" */
/* *2    16-APR-1989 17:57:21 BUDA "Fix remote command failure" */
/* *1    11-APR-1989 22:55:40 BUDA "Initial creation" */
/* DEC/CMS REPLACEMENT HISTORY, Element CKVFIO.C */
char *ckzsys = " VAX/VMS";

/* C K V F I O  --  Kermit file system support for VAX/VMS */

/* Stew Rubenstein, Harvard University Chemical Labs */
/*  (c) 1985 President and Fellows of Harvard College  */
/*  Based on CKZUNX.C, 4.1(015) 28 Feb 85 */
/* Also, Martin Minow (MM), Digital Equipment Corporation, Maynard MA */
/* Also, Dan Schullman (DS), Digital Equipment Corporation, Maynard MA */
/* Adapted from ckufio.c, by... */
/* F. da Cruz (FdC), Columbia University Center for Computing Activities */

/* Edit history
 * 003 20-Mar-85 MM  fixed fprintf bug in zsout.c
 * 004 21-Mar-84 MM  create text files in variable-stream.
 * 005  8-May-85 MM  filled in zkself (not tested), fixed other minor bugs
 * 006  5-Jul-85 DS  handle version number in zltor, zrtol
 * 007 11-Jul-85 FdC fix zclose() to give return codes
 * 008 19-Mar-86 FdC Fix system() for "!", zopeni() for REMOTE commands.
 * 008 17-Sep-87 FdC Define PWDCMD.
 * 090 (???)
 * 010 24-Jan-88 FdC Add zgtdir() function, even tho it doesn't work...
 * 011 14-Feb-89 mab Make zgtdir() work in V2/V3 C envirements,
 *		     Make zkself work using delprc() using Will Wood's changes.
 * 012 26-Feb-89 mab Add function that searches for kermit.ini file in various
 *                   ways
 * 013 05-Mar-89 mab Add Barry Archers enhancements/fixes.
 * 014 15-Mar-89 mab Check for non-null data, not array of pointers in
 *                   zkermini
 * 015 04-Apr-89 mab Add latent support for attribute packet.  Clean up
 *		     file name translation code.
 * 016 05-Apr-89 mab Add PWP code to optimize packetizing.
 * 017 16-Apr-89 mab PWP changes broke REMOTE command.  Fixed.
 * 018 18-Apr-89 mab #ifdef chkfn.  This removes a lot of overhead.
 *		     Add code to gtdir() for V4.x.
 * 019 12-Jun-89 mab Add PWP's encode logic
 * 020 09-Jul-89 mab Add logic to check for system() availability
 * 021 10-Jul-89 mab Fix SHOW USER USERNAME.  Added space after 'SHOW USER'.
 */

/* Definitions of some VMS system commands */

char *DIRCMD = "DIRECTORY ";		/* For directory listing */
char *DELCMD = "DELETE ";		/* For file deletion */
char *TYPCMD = "TYPE ";			/* For typing a file */
char *SPACMD = "DIRECTORY/TOTAL/SIZE=ALL "; /* Space/quota of cur dir */
char *SPACM2 = "DIRECTORY/TOTAL/SIZE=ALL "; /* Space/quota of cur dir */
char *WHOCMD = "SHOW USERS ";		/* For seeing who's logged in */
char *PWDCMD = "SHOW DEFAULT ";		/* For seeing current directory */


/*
  Functions (n is one of the predefined file numbers from ckermi.h):

   zopeni(n,name)   -- Opens an existing file for input.
   zopeno(n,name)   -- Opens a new file for output.
   zclose(n)        -- Closes a file.
   zchin(n)         -- Gets the next character from an input file.
   zsout(n,s)       -- Write a null-terminated string to output file, buffered.
   zsoutl(n,s)      -- Like zsout, but appends a line terminator.
   zsoutx(n,s,x)    -- Write x characters to output file, unbuffered.
   zchout(n,c)      -- Add a character to an output file, unbuffered.
   zchki(name)      -- Check if named file exists and is readable, return size.
   zchko(name)      -- Check if named file can be created.
   znewn(name,s)    -- Make a new unique file name based on the given name.
   zdelet(name)     -- Delete the named file.
   zxpand(string)   -- Expands the given wildcard string into a list of files.
   znext(string)    -- Returns the next file from the list in "string".
   zxcmd(cmd)       -- Execute the command in a lower fork.
   zclosf()         -- Close input file associated with zxcmd()'s lower fork.
   zrtol(n1,n2)     -- Convert remote filename into local form.
   zltor(n1,n2)     -- Convert local filename into remote form.
   zchdir(dirnam)   -- Change working directory.
   zhome()          -- Return pointer to home directory name string.
   zkself()         -- Log self out
   zsattr(struc zattr *) -- Return attributes for file which is being sent.
   zkermini(n1,n2)  -- Find kermit.ini using default scanning process
 */



/* Includes */

#include "ckcker.h"
#include "ckcdeb.h"
#include "ckvvms.h"
#include <stdio.h>
#include <stat.h>
#include <ctype.h>
#include <rms.h>
#include <descrip.h>
#include <dvidef.h>
#include <iodef.h>
#include <jpidef.h>
#include <errno.h>
#include <signal.h>

#define MAXWLD 500			/* Maximum wildcard filenames */

/* (PWP) external def. of things used in buffered file input and output */
extern CHAR zinbuffer[], zoutbuffer[];
extern CHAR *zinptr, *zoutptr;
extern int zincnt, zoutcnt;
extern int binary;

/* Declarations */

FILE *fp[ZNFILS] = { 			/* File pointers */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static long iflen = -1;			/* Input file length */
static long oflen = -1;			/* Output file length */
static int fcount;			/* Number of files in wild group */
static char cwdbuf[NAM$C_MAXRSS];
static struct iosb_struct tmpiosb;	/* For QIOW */

extern unsigned long vms_status;        /* Used by CHECK_ERR */

char *getenv(), *strcpy();		/* For finding home directory */

static char *mtchs[MAXWLD],		/* Matches found for filename */
     **mtchptr;				/* Pointer to current match */

static unsigned
	   int input_mbxchn,
	   output_mbxchn,
	   child_pid = 0;



/***  Z K S E L F --  Log self out  ***/

/*** (someone please check if this works in VMS) ***/

zkself() {
    unsigned long int rms_s;

    rms_s = SYS$DELPRC(0,0);
    exit(rms_s == SS$_NORMAL);
}


/*  Z O P E N I  --  Open an existing file for input. */

zopeni(n,name) int n; char *name; {
    debug(F111," zopeni",name,n);
    debug(F101,"  fp","",(int) fp[n]);
    if (chkfn(n) != 0) return(0);
    if (n == ZSYSFN) {			/* Input from a system function? */
        debug(F110," invoking zxcmd",name,0);
	return(zxcmd(name));		/* Try to fork the command */
    }
    if (n == ZSTDIO) {			/* Standard input? */
	if (isatty(0)) {
	    ermsg("?Terminal input not allowed\n");
	    debug(F110,"zopeni: attempts input from unredirected stdin","",0);
	    return(0);
	}
	fp[ZIFILE] = stdin;
	return(1);
    }
    zincnt = 0;			/* (PWP) reset input buffer */
    fp[n] = fopen(name,"r");		/* Real file. */
    debug(F111," zopeni", name, (int) fp[n]);
    if (fp[n] == NULL) perror("zopeni");
    return((fp[n] != NULL) ? 1 : 0);
}

/*  Z O P E N O  --  Open a new file for output.  */

zopeno(n,name) int n; char *name; {

    int fildes;

    debug(F111," zopeno",name,n);
    if (chkfn(n) != 0) return(0);

    zoutcnt = 0;		/* (PWP) reset output buffer */
    zoutptr = zoutbuffer;

    if ((n == ZCTERM) || (n == ZSTDIO)) {   /* Terminal or standard output */
	fp[ZOFILE] = stdout;
	debug(F101," fp[]=stdout", "", (int) fp[n]);
	return(1);
    }
    /*
     * Create "binary" output files as fixed-block 512 byte records.
     * This should permit copying task images.  It is rumored that
     * Vax C will null-fill an incomplete final block.
     *
     * Create all debugging files (and normal output files) in
     * "vanilla" RMS -- variable length, implicit carriage control.
     * This way, old brain-damaged programs aren't suprised by
     * bizarre Unix-styled files.
     */
    if (n == ZOFILE && binary)
	fildes = creat(name, 0, "mrs=512", "rfm=fix");
    else
	if (n == ZDFILE || n == ZSFILE)
/* debugging file only, make it stream.  This makes the unbuffered part work */
	    fildes = creat(name, 0, "rat=cr", "rfm=stm"); 
	else
	    fildes = creat(name, 0, "rat=cr", "rfm=var");
    fp[n] = (fildes == -1) ? NULL : fdopen(fildes, "w");
    if (fp[n] == NULL) perror(name);		/* +1, print useful msg	*/
#ifdef mab
/*
 * This code does not work correctly.  Currently for each character that
 * is immediatly written, it is considered a record.   This should be
 * looked at in the future.
 */
    if (n == ZDFILE && isatty(fileno(fp[n])))
	setbuf(fp[n],NULL); /* Make debugging file unbuffered */
#endif
#ifdef mab
    zoutcnt = 0;		/* (PWP) reset output buffer */
    zoutptr = zoutbuffer;
#endif
    debug(F101, " fp[n]", "", (int) fp[n]);
    return((fp[n] != NULL) ? 1 : 0);
}

/*  Z C L O S E  --  Close the given file.  */

/*  Returns 0 if arg out of range, 1 if successful, -1 if close failed.  */

zclose(n) int n; {
    int x=0, x2=0;

    debug(F101," zclose","",n);
    if (chkfn(n) < 1) return(0);

    if ((n == ZOFILE) && (zoutcnt > 0))	/* (PWP) output leftovers */
	x2 = zoutdump();

    if ((child_pid) && ((!fp[n]) || (n == ZSYSFN)))
	x = zclosf();
    else {
	if ((fp[n] != stdout) && (fp[n] != stdin)) x = fclose(fp[n]);
	fp[n] = NULL;
    }
    iflen = -1;				/* Invalidate file length */
    debug(F101,"  x","",x);
    debug(F101,"  x2","",x2);
    if (x == EOF)		/* if we got a close error */
	return (-1);
    else if (x2 < 0)		/* or an error flushing the last buffer */
	return (-1);		/* then return an error */
    else
	return (1);
}

static int subprocess_input = 0, sub_count;
static char *sub_ptr, sub_buf[SUB_BUF_SIZE];

get_subprc_line() {
    struct iosb_struct subiosb;

    if ((vms_status = SYS$QIOW(QIOW_EFN, output_mbxchn, IO$_READVBLK,
	&subiosb, 0, 0, sub_buf, sizeof(sub_buf), 0, 0, 0, 0)) != SS$_NORMAL)
		return(-1);
    if (subiosb.status != SS$_NORMAL) return(-1);
    if (subiosb.size == 29
     && strncmp(sub_buf, ">>> END OF KERMIT COMMAND <<<",
		subiosb.size) == 0) {
	subprocess_input = 0;
	return(-1);
    }
    sub_buf[subiosb.size] = '\n';
    sub_buf[subiosb.size + 1] = '\0';
    sub_count = subiosb.size + 1;
    sub_ptr = sub_buf;
    return(0);
}

/*  Z C H I N  --  Get a character from the input file.  */

/*  Returns -1 if EOF, 0 otherwise with character returned in argument  */

zchin(n,c) int n; char *c; {
    int a;

#ifdef DEBUG
    if (chkfn(n) < 1) return(-1);
#endif

    if (n == ZIFILE && subprocess_input) {
	if (--sub_count < 0)
	    if (get_subprc_line()) return(-1);
	a = *sub_ptr++;
    } else {
    /* (PWP) Just in case this gets called when it shoudn't */
	return (zminchar());
/*	a = getc(fp[n]);	*/
    }
    if (a == EOF) return(-1);
    *c = (unsigned char)a;
    return(0);
}

/*
 * (PWP) (re)fill the buffered input buffer with data.  All file input
 * should go through this routine, usually by calling the zminchar()
 * macro (in ckcker.h).
 */

zinfill() {

    if (subprocess_input) {
	if (get_subprc_line()) return(-1);
/*
 * The size problem should never happen.  sub_buf of a size greater then
 * 1k is highly unlikely to be needed.
 */
	if (INBUFSIZE < SUB_BUF_SIZE) {
	    fprintf(stderr,"zinfill: sub_buf too large for zinbuffer");
	    exit();
	}
	zinptr = sub_buf;
	zincnt = sub_count;
    } else {
        zincnt = fread(zinbuffer, sizeof (char), INBUFSIZE, fp[ZIFILE]);
        if (zincnt == 0) return (-1); /* end of file */
	zinptr = zinbuffer; /* set pointer to beginning, (== &zinbuffer[0]) */
    }
    zincnt--;			/* one less char in buffer */
    return((int)(*zinptr++) & 0377); /* because we return the first */
}       



/*  Z S O U T  --  Write a string to the given file, buffered.  */

zsout(n,s) int n; char *s; {
#ifdef DEBUG
    if (chkfn(n) < 1) return(-1);
#endif
    fputs(s, fp[n]);			/* Don't use fprintf here MM */
    return(0);
}

/*  Z S O U T L  --  Write string to file, with line terminator, buffered  */

zsoutl(n,s) int n; char *s; {
#ifdef DEBUG
    if (chkfn(n) < 1) return(-1);
#endif
    fputs(s, fp[n]);			/* Don't use fprintf MM */
    putc('\n', fp[n]);
    return(0);
}

/*  Z S O U T X  --  Write x characters to file, unbuffered.  */

zsoutx(n,s,x) int n, x; char *s; {
#ifdef DEBUG
    if (chkfn(n) < 1) return(-1);
#endif
    return(write(fileno(fp[n]),s,x));
}


/*  Z C H O U T  --  Add a character to the given file.  */

zchout(n,c) register int n; char c; {
#ifdef DEBUG
    if (chkfn(n) < 1) return(-1);
#endif
    if (n == ZSFILE)
    	return(write(fileno(fp[n]),&c,1)); /* Use unbuffered for session log */
    else {
	if (putc(c,fp[n]) == EOF)	/* If true, maybe there was an error */
	    return(ferror(fp[n])?-1:0);	/* Check to make sure */
	else				/* Otherwise... */
	    return(0);			/* There was no error. */
    }
}

/* (PWP) buffered character output routine to speed up file IO */
zoutdump()
{
    static char *partial_buf = NULL;
    static char *partial_buf_pnt = NULL;
    static int  partial_buf_cnt = 0;
    char *vpnt, *tpnt;
    int   vcnt,  tcnt;

/*
 * Allocate buffer for temp usage when an overflow occurs.
 */
    if (partial_buf == NULL) {
	partial_buf = calloc(PARTIAL_BUF_SIZE,sizeof(char));
	if (partial_buf == NULL) {
	   perror("%CKERMIT-F-CKVFIO, Cannot allocate memory for partial_buf");
	    exit();
	}
	partial_buf_pnt = partial_buf;
    }

    debug(F111," zoutdump","zoutcnt",zoutcnt);
    debug(F101,"  partial_buf_cnt","",partial_buf_cnt);
/*
 * IF a negative value is passed, then act like everything is OK...
 * If a zero then check and see if anything is in the partial buffer
 * and flush if needed.
 */
    if (zoutcnt <= 0 && !partial_buf_cnt) return (0); /* nothing to output */

    if (!binary) { /* Ascii */
/*
 * If there data buffered up from the previous call, then we need
 * to copy data from the new buffer onto the tail of the old one
 * and write it out.  (Copy only up to LF).
 */
	vpnt = zoutbuffer;		/* Point to beginning of buffer */
	if (partial_buf_cnt) {
	    while (zoutcnt > 0 && *vpnt != LF) {
		*partial_buf_pnt++ = *vpnt++;
		zoutcnt--;
		partial_buf_cnt++;
		if (partial_buf_cnt > PARTIAL_BUF_SIZE) {
		fprintf(stderr,"%%CKERMIT-F-CKVFIO, Partial buffer overflows");
		    exit();
		}
/*
 * Skip over the CR by bumping pointers
 */
	    }
	    debug(F101,"  partial_buf_cnt","",partial_buf_cnt);
	    debug(F101,"  zoutcnt","",zoutcnt);
	    if (!fwrite (partial_buf, partial_buf_cnt, 1, fp[ZOFILE])) {
		debug(F101,"  ferror(partial)","",ferror(fp[ZOFILE]));
		return(ferror(fp[ZOFILE])?-1:0); /* Check to make sure */
	    }
	    zoutcnt--;
	    vpnt++;
	    partial_buf_cnt = 0;
	    partial_buf_pnt = partial_buf;
	}
/*
 * Check and see if there are any more characters to write to the file.
 * If there is not, we did them all in the previous code, exit fucntion.
 */
	if (zoutcnt < 1 && *vpnt != LF) {
	    zoutcnt = 0;		/* reset output buffer */
	    zoutptr = zoutbuffer;		
	    return(0);
	}

	while(zoutcnt > 0) {
	    tcnt = 0;
	    tpnt = vpnt;

/*
 * Find out size of next record.
 */
	    while (tcnt != zoutcnt) {
		tcnt++;
		if (*tpnt == LF) break;
		tpnt++;
	    }
/*
 * If the last byte is a CR, then we have a complete record and we should
 * write the record to the file.
 */
	    if (tcnt != zoutcnt) {
		if (!fwrite (vpnt, tcnt, 1, fp[ZOFILE])) {
		    debug(F101,"  ferror(partial)","",ferror(fp[ZOFILE]));
		    return(ferror(fp[ZOFILE])?-1:0); /* Check to make sure */
		}
/*
 * Make sure we bump over the CR. when updating the variables.
 */
		vpnt = ++tpnt;
		zoutcnt -= tcnt;
	    } else {
/*
 * Store overflow data into partial buffer for later writing.
 */
		while (zoutcnt > 0) {
		    *partial_buf_pnt++ = *vpnt++;
		    partial_buf_cnt++;
		    zoutcnt--;
		}
	    }
	}
	zoutcnt = 0;
	zoutptr = zoutbuffer;
	return(0);
    } else { /* binary */

	if (fwrite (zoutbuffer, zoutcnt, 1, fp[ZOFILE])) {
	    zoutcnt = 0;		/* reset output buffer */
	    zoutptr = zoutbuffer;
	    return(0);		/* things worked OK */
	} else {
	    debug(F101,"  ferror","",ferror(fp[ZOFILE]));
	    return(ferror(fp[ZOFILE])?-1:0); /* Check to make sure */
	}
    }
}




/*  C H K F N  --  Internal function to verify file number is ok  */

/*
 Returns:
  -1: File number n is out of range
   0: n is in range, but file is not open
   1: n in range and file is open
*/
chkfn(n) int n; {
    switch (n) {
	case ZCTERM:
	case ZSTDIO:
	case ZIFILE:
	case ZOFILE:
	case ZDFILE:
	case ZTFILE:
	case ZPFILE:
	case ZSFILE: break;
	case ZSYSFN:				/* System function's */
		return(0);
	default:
	    debug(F101,"chkfn: file number out of range","",n);
	    fprintf(stderr,"?File number out of range - %d\n",n);
	    return(-1);
    }
    return( (fp[n] == NULL) ? 0 : 1 );
}



/*  Z C H K I  --  Check if input file exists and is readable  */

/*
  Returns:
   >= 0 if the file can be read (returns the size).
     -1 if file doesn't exist or can't be accessed,
     -2 if file exists but is not readable (e.g. a directory file).
     -3 if file exists but protected against read access.
*/
/*
 For Berkeley Unix, a file must be of type "regular" to be readable.
 Directory files, special files, and symbolic links are not readable.
*/
long
zchki(name) char *name; {
    struct stat buf;
    int x; long y; 

    x = stat(name,&buf);
    if (x < 0) {
	debug(F111,"zchki stat fails",name,errno);
	return(-1);
    }
    x = buf.st_mode & S_IFMT;		/* Isolate file format field */
    if ((x != 0) && (x != S_IFREG)) {
	debug(F111,"zchki skipping:",name,x);
	return(-2);
    }
    debug(F111,"zchki stat ok:",name,x);

    if ((x = access(name,R_OK)) < 0) { 	/* Is the file accessible? */
	debug(F111," access failed:",name,x); /* No */
    	return(-3);			
    } else {
	iflen = buf.st_size;
	debug(F111," access ok:",name,(int) iflen); /* Yes */
	return( (iflen > -1) ? iflen : 0 );
    }
}



/*  Z C H K O  --  Check if output file can be created  */

/*
 Returns -1 if write permission for the file would be denied, 0 otherwise.
*/
zchko(name) char *name; {
    return(0);				/* Always creates new version */
}



/*  Z D E L E T  --  Delete the named file.  */

zdelet(name) char *name; {
    return(delete(name));
}


/*  Z R T O L  --  Convert remote filename into local form  */

zrtol(name,name2) char *name, *name2; {
    int count = 9, vflag = 0;
    char *cp, c;
    static char *spcl_set = "_-$[]<>:.\";";

    for (cp=name2; c = *name; name++) {
	if (islower(c)) c = toupper(c);
	if (!isalnum(c) &&
	    !strchr(spcl_set,c)) c = 'X';
	*cp++ = c;
    }

#ifdef VMS_V3
    count = 9;
    vflag = 0;
    for ( cp = name2; *name; name++ ) {
	switch (*name) {
	    case '.':			/* File type */
		if (!vflag) {
		    vflag++;		/* 
	    	    count = 3;		/* Max length for this field */
		    *cp++ = '.';
		    break;
		}
	    case ';':			/* Version */
	    	count = 5;
		*cp++ = ';';
		break;
	    default:
	    	if (count > 0 && isalnum(*name)) {
		    --count;
		    *cp++ = islower(*name) ? toupper(*name) : *name;
		}
		break;
	}
    }
#endif

    *cp = '\0';				/* End of name */
    debug(F110,"zrtol: ",name2,0);
}


/*  Z L T O R  --  Convert filename from local format to common form.   */

zltor(name,name2) char *name, *name2; {
    char *cp, *pp;

/*
 * Copy name to output string
 */
    strcpy(name2,name);
/*
 * Parse the filename and type, with the defualt filename of "X"
 */
    parse_fname(name2, 100, "X", PARSE_NAME|PARSE_TYPE);

#ifdef VMS_V3
    for (cp = pp = name; *cp ; cp++) {	/* strip path name */
    	if (*cp == ']' || *cp == ':') {
	    pp = cp;
	    pp++;
	}
    }
    for ( ; --cp >= pp; ) {		/* From end to beginning */
	if (!isdigit(*cp)) {		/* if not numeric, then */
	    if (*cp == '-') --cp;	/* if minus sign, skip over, or */
	    if (*cp == ';') *cp = '\0'; /* if version delim, make end */
	    break;
	}
    }
    cp = name2;				/* If nothing before dot, */
    if (*pp == '.') *cp++ = 'X';	/* insert 'X' */
    strcpy(cp,pp);
#endif

    debug(F110,"zltor: ",name2,0);
}    


/*  Z C H D I R  --  Change directory  */

zchdir(dirnam) char *dirnam; {

    char   *zgtdir();
    char   dir_buff[NAM$C_MAXRSS];
    int    status;

    if (*dirnam == '\0')
        strcpy(dirnam,getenv("HOME"));            /* default to current dir */

    status = chdir(dirnam);         /* change first in parent proc */
        /* then change it in the child process -> keep in synch! */
    if ((child_pid != 0) && (status == 0)) {
	/* construct command for child proc */
	strcpy(dir_buff,"$ set def ");
	strcat(dir_buff,zgtdir());
	SYS$QIOW(QIOW_EFN, input_mbxchn, IO$_WRITEVBLK | IO$M_NOW,
		&tmpiosb, 0, 0, dir_buff, strlen(dir_buff), 0, 0, 0, 0);
       }
    return(status == 0);
}


/*  Z H O M E  --  Return pointer to user's home directory  */

char *
zhome() {
    return(getenv("HOME"));
}

/*  Z G T D I R  --  Return pointer to user's current directory  */

char *
zgtdir() {
#ifdef VMS_V40 | VMS_V42 | VMS_V44

    static char *gtdir_buf = 0;
    static char sysdisk[] = "SYS$DISK";
    char tmp_buf[NAM$C_MAXRSS+1];
    struct dsc$descriptor_s
	tmp_buf_dsc = {sizeof(tmp_buf),DSC$K_DTYPE_T,DSC$K_CLASS_S,&tmp_buf},
	sysdisk_dsc = {sizeof(sysdisk)-1,DSC$K_DTYPE_T,DSC$K_CLASS_S,&sysdisk};
    unsigned short int buf_len;

/*
 * Allocate buffer dynamically, first time through.  This makes the image
 * smaller.
 */
    if (!gtdir_buf) gtdir_buf = malloc(NAM$C_MAXRSS+1);

/*
 * Translate device name.
 */

    LIB$SYS_TRNLOG(	&sysdisk_dsc,
			&buf_len,
			&tmp_buf_dsc,
			0,
			0,
			0);
    tmp_buf[buf_len] = '\0';
    strcpy(gtdir_buf,tmp_buf);

/*
 * Get directory name.
 */
    SYS$SETDDIR(	0,	/* New dir addr */
			&buflen,/* length addr */
			&tmp_buf_dsc);
    tmp_buf[buf_len] = '\0';
    strcat(gtdir_buf,tmp_buf);

    return(&gtdir_buf);  /* Can't seem to make LINK find getcwd()... */
#else
    char *getcwd();
    char *buf;

    buf = cwdbuf;
    return(getcwd(buf,100));
#endif
}


/*  Z X C M D -- Run a system command so its output can be read like a file */

zxcmd(comand) char *comand; {
    char input_mbxnam[10], output_mbxnam[10];
    char cmdbuf[200];

    if (child_pid == 0) {
    	struct dsc$descriptor_s
	    inpdsc = {sizeof(input_mbxnam),DSC$K_DTYPE_T,
			DSC$K_CLASS_S,&input_mbxnam}, 
	    outdsc = {sizeof(output_mbxnam),DSC$K_DTYPE_T,
			DSC$K_CLASS_S,&output_mbxnam};
    	struct itmlst dvilst[] = 
		{{sizeof(input_mbxnam),DVI$_DEVNAM,&input_mbxnam,0},
		{0,0,0,0}};

        CHECK_ERR("zxcmd:SYS$CREMBX,I",SYS$CREMBX(0, &input_mbxchn, 0,
		0, 0, 0,0));
        CHECK_ERR("zxcmd:SYS$GETDVIW,I",SYS$GETDVIW(0, input_mbxchn, 0,
		dvilst,&tmpiosb, 0, 0, 0));
	debug(F110," zxcmd:input_mbxnam",input_mbxnam,0);

        CHECK_ERR("zxcmd:SYS$CREMBX,O",SYS$CREMBX(0, &output_mbxchn,
		SUB_BUF_SIZE, 0, 0, 0, 0));
        dvilst[0].len = sizeof(output_mbxnam);
        dvilst[0].adr = &output_mbxnam;
        CHECK_ERR("zxcmd:SYS$GETDVIW,O",SYS$GETDVIW(0, output_mbxchn, 0,
		dvilst, &tmpiosb, 0, 0, 0));
	debug(F110," zxcmd:output_mbxnam",output_mbxnam,0);

        CHECK_ERR("zxcmd:LIB$SPAWN",LIB$SPAWN(0, &inpdsc, &outdsc, &1,
		0, &child_pid));
	CHECK_ERR("zxcmd:SYS$QIOW1",SYS$QIOW(QIOW_EFN, input_mbxchn,
		IO$_WRITEVBLK | IO$M_NOW, &tmpiosb, 0, 0, "$ SET NOON", 10,
		0, 0, 0, 0));
    }

    strcpy(cmdbuf, "$ ");
    strcat(cmdbuf, comand);
    debug(F110," zxcmd",cmdbuf,0);
    CHECK_ERR("zxcmd:SYS$QIOW2",SYS$QIOW(QIOW_EFN, input_mbxchn,
	IO$_WRITEVBLK | IO$M_NOW, &tmpiosb, 0, 0, cmdbuf, strlen(cmdbuf),
	0, 0, 0, 0));
    CHECK_ERR("zxcmd:SYS$QIOW3",SYS$QIOW(QIOW_EFN, input_mbxchn,
	IO$_WRITEVBLK | IO$M_NOW, &tmpiosb, 0, 0,
	"$ WRITE SYS$OUTPUT \">>> END OF KERMIT COMMAND <<<\"",
	    50, 0, 0, 0, 0));
    subprocess_input = 1;
    sub_count = 0;
    return(1);
}

/*  Z C L O S F  - close the suprocess output file.  */

zclosf() {
    unsigned long int sys_s;

    if (child_pid) {
	debug(F101,"zclosf child_pid =","",child_pid);
	if (CHECK_ERR("zclosf: SYSDELPRC ",SYS$DELPRC(&child_pid,0)))
	    child_pid = 0;
    }
    fclose(fp[ZIFILE]);
    fp[ZIFILE] = fp[ZSYSFN] = NULL;
    return(1);
}




/*  Z X P A N D  --  Expand a wildcard string into an array of strings  */
/*
  Returns the number of files that match fn1, with data structures set up
  so that first file (if any) will be returned by the next znext() call.
*/
zxpand(fn) char *fn; {
    fcount = fgen(fn,mtchs,MAXWLD);	/* Look up the file. */
    if (fcount > 0) {
	mtchptr = mtchs;		/* Save pointer for next. */
    }
    debug(F111,"zxpand",mtchs[0],fcount);
    return(fcount);
}


/*  Z N E X T  --  Get name of next file from list created by zxpand(). */
/*
 Returns >0 if there's another file, with its name copied into the arg string,
 or 0 if no more files in list.
*/
znext(fn) char *fn; {
    if (fcount-- > 0) strcpy(fn,*mtchptr++);
    else *fn = '\0';
    debug(F111,"znext",fn,fcount+1);
    return(fcount+1);
}


/*  Z N E W N  --  Make a new name for the given file  */

znewn(fn,s) char *fn, **s; {
    static char buf[NAM$C_MAXRSS];

    strcpy(buf, fn);			/* Version numbers are handled by OS */
    *s = buf;
}

/*
 * fgen:
 *  This is the actual name generator.  It is passed a string,
 *  possibly containing wildcards, and an array of character pointers.
 *  It finds all the matching filenames and stores them into the array.
 *  The returned strings are allocated from a static buffer local to
 *  this module (so the caller doesn't have to worry about deallocating
 *  them); this means that successive calls to fgen will wipe out
 *  the results of previous calls.  This isn't a problem here
 *  because we process one wildcard string at a time.
 *
 * Input: a wildcard string, an array to write names to, the
 *        length of the array.
 * Returns: the number of matches.  The array is filled with filenames
 *          that matched the pattern.  If there wasn't enough room in the
 *	    array, -1 is returned.
 */
fgen(pat,resarry,len)
char *pat,*resarry[];
int len;
{
    struct dsc$descriptor_s
	file_spec = {0,DSC$K_DTYPE_T,DSC$K_CLASS_S,0},
	result = {0,DSC$K_DTYPE_T,DSC$K_CLASS_D,0},
	deflt = {0,DSC$K_DTYPE_T,DSC$K_CLASS_S,0};
    unsigned long context = 0, status;
    int count = 0;
    char *def_str = "*.*";

    file_spec.dsc$w_length  = strlen(pat);
    file_spec.dsc$a_pointer = pat;

    deflt.dsc$w_length  = sizeof(def_str)-1;
    deflt.dsc$a_pointer = def_str;

    while (count < len
	   && (status = LIB$FIND_FILE(&file_spec, &result, &context, &deflt))
		== RMS$_NORMAL) {
    	resarry[count] = malloc(result.dsc$w_length + 1);
	strncpy(resarry[count], result.dsc$a_pointer, result.dsc$w_length);
	resarry[count][result.dsc$w_length] = '\0';
	count++;
    }
#ifdef DVI$_ALT_HOST_TYPE
    LIB$FIND_FILE_END(&context);	/* Only on V4 and later */
#endif
    LIB$SFREE1_DD(&result);
    if (status == RMS$_FNF) return(0);
    if (status == RMS$_NMF) return(count);
    return(-1);
}

#ifdef VAXC023
system(s)
char *s;
{
    if (*s) {
	zxcmd(s);
	while (!get_subprc_line())
	    fputs(sub_buf, stdout);
	putchar('\n');
    } else {
	LIB$SPAWN();
    }
}
#endif

/*
 * Find ini file.  If none is found, then return NULL string.
 */

zkermini(char *s, int s_len, char *def)
{
    FILE fd;
    struct dsc$descriptor_s
		dsc_in = {0,DSC$K_DTYPE_T,DSC$K_CLASS_S,0},
		dsc_out = {0,DSC$K_DTYPE_T,DSC$K_CLASS_D,0},
		dsc_def = {0,DSC$K_DTYPE_T,DSC$K_CLASS_S,0};
    int max_len;
    long unsigned int rms_s;
    unsigned long find_file_context = 0;

    struct TRNLIST {
	char *name;			/* Ascii */
	unsigned char flag;		/* <> 0 to not use default */
	} *p;

    static struct TRNLIST slist[] = {
			{"ckermit_ini:", 0},
			{"sys$login:",   0},
			{"",             0}};

    p = slist;
    while(*p->name) {

	dsc_in.dsc$w_length = strlen(p->name);	/* length of work area */
	dsc_in.dsc$a_pointer = p->name;		/* Address of string */

	if (!(p->flag)) {
    	    dsc_def.dsc$w_length = strlen(def);	/* length of work area */
	    dsc_def.dsc$a_pointer = def;	/* Address of string */
	}
	else {
	    dsc_def.dsc$w_length = 0;		/* length of work area */
	    dsc_def.dsc$a_pointer = 0;		/* Address of string */
        }

	rms_s = LIB$FIND_FILE(
				&dsc_in,	/* File spec */
				&dsc_out,	/* Result file spec */
				&find_file_context, /* Context */
				&dsc_def,	/* Default file spec */
				0,		/* Related spec */
				0,		/* STV error */
				0);		/* Flags */

	if (rms_s == RMS$_NORMAL) {
	    max_len = ((unsigned short int) dsc_out.dsc$w_length < s_len ? 
		(unsigned short int) dsc_out.dsc$w_length : 0);
	    if (!max_len)
		fprintf(stderr,
		"%%ZKERMINI out string not long enough, ignoring .ini file\n");
	    else
	        strncpy(s,dsc_out.dsc$a_pointer,max_len);
	    LIB$FIND_FILE_END(&find_file_context);
	    LIB$SFREE1_DD(&dsc_out);		/* Return dyno memory */
	    return(0);
	}
	*p++;
	LIB$FIND_FILE_END(&find_file_context);
    };
    *s = '\0';			/* Return NULL string */
    LIB$SFREE1_DD(&dsc_out);
    return(0);
}

parse_fname(cp, cp_len, defnam, flag)
char *cp;		/* Pointer to file spec to parse */
int cp_len;		/* Length of cp field */
char *defnam;		/* Default file spec */
int flag;		/* Flag word PARSE_xxx */
{
    struct FAB fab;
    struct NAM nam;
    char expanded_name[NAM$C_MAXRSS];
    int long rms_status;
    int cur_len = 0;

    fab = cc$rms_fab;
    fab.fab$l_nam = &nam;
    fab.fab$l_fna = cp;
    fab.fab$b_fns = strlen(cp);
    if (defnam) {
	fab.fab$b_dns = strlen(defnam);
	fab.fab$l_dna = defnam;
    } else
	fab.fab$l_dna = 0;

    nam = cc$rms_nam;
    nam.nam$l_esa = &expanded_name;
    nam.nam$b_ess = sizeof(expanded_name);

    if (!CHECK_ERR("%%CKERMIT-W-PARSE, ",
		SYS$PARSE(&fab)))
	return(-1);

    *cp = NULL;			/* Make a zero length string */
    if ((PARSE_NODE & flag) && nam.nam$b_node &&
		cur_len+nam.nam$b_node < cp_len) {
	cur_len += nam.nam$b_node;
	strncat(cp, nam.nam$l_node, (int)nam.nam$b_node);
    }
    if ((PARSE_DEVICE & flag) && nam.nam$b_dev &&
		cur_len+nam.nam$b_dev < cp_len) {
	cur_len += nam.nam$b_dev;
	strncat(cp, nam.nam$l_dev, (int)nam.nam$b_dev);
    }
    if ((PARSE_DIRECTORY & flag) && nam.nam$b_dir &&
		cur_len+nam.nam$b_dir < cp_len) {
	cur_len += nam.nam$b_dir;
	strncat(cp, nam.nam$l_dir, (int)nam.nam$b_dir);
    }
    if ((PARSE_NAME & flag) && nam.nam$b_name &&
		cur_len+nam.nam$b_name < cp_len) {
	cur_len += nam.nam$b_name;
	strncat(cp, nam.nam$l_name, (int)nam.nam$b_name);
    }
    if ((PARSE_TYPE & flag) && nam.nam$b_type &&
		cur_len+nam.nam$b_type < cp_len) {
	cur_len += nam.nam$b_type;
	strncat(cp, nam.nam$l_type, (int)nam.nam$b_type);
    }
    if ((PARSE_VERSION & flag) && nam.nam$b_ver &&
		cur_len+nam.nam$b_ver < cp_len) {
	cur_len += nam.nam$b_ver;
	strncat(cp, nam.nam$l_ver, (int)nam.nam$b_ver);
    }
    return(cur_len);
}



/*  Z S A T T R */
/*
 Fills in a Kermit file attribute structure for the file which is to be sent.
 Returns 0 on success with the structure filled in, or -1 on failure.
 If any string member is null, then it should be ignored.
 If any numeric member is -1, then it should be ignored.
*/
zsattr(xx) struct zattr *xx; {
    long k;

    k = iflen % 1024L;			/* File length in K */
    if (k != 0L) k = 1L;
    xx->lengthk = (iflen / 1024L) + k;
    xx->type.len = 0;			/* File type can't be filled in here */
    xx->type.val = "";
    xx->date.len = 0;			/* File creation date */
    xx->date.val = "";
    xx->creator.len = 0;		/* File creator */
    xx->creator.val = "";
    xx->account.len = 0;		/* File account */
    xx->account.val = "";
    xx->area.len = 0;			/* File area */
    xx->area.val = "";
    xx->passwd.len = 0;			/* Area password */
    xx->passwd.val = "";
    xx->blksize = -1L;			/* File blocksize */
    xx->access.len = 0;			/* File access */
    xx->access.val = "";
    xx->encoding.len = 0;		/* Transfer syntax */
    xx->encoding.val = 0;
    xx->disp.len = 0;			/* Disposition upon arrival */
    xx->disp.val = "";
    xx->lprotect.len = 0;		/* Local protection */
    xx->lprotect.val = "";
    xx->gprotect.len = 0;		/* Generic protection */
    xx->gprotect.val = "";
    xx->systemid.len = 2;		/* System ID for DEC/VMS */
    xx->systemid.val = "D7";
    xx->recfm.len = 0;			/* Record format */
    xx->recfm.val = "";
    xx->sysparam.len = 0;		/* System-dependent parameters */
    xx->sysparam.val = "";
    xx->length = iflen;			/* Length */
    return(0);
}

/* Z M A I L - Place holder */

zmail()
{
}

/* Z P R I N T  - Place holder */

zprint()
{
}
