char *ckzv = "VMS file support, 1.0(010), 24 Jan 88";
char *ckzsys = " Vax/VMS";

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
 */

/* Definitions of some VMS system commands */

char *DIRCMD = "DIRECTORY ";		/* For directory listing */
char *DELCMD = "DELETE ";		/* For file deletion */
char *TYPCMD = "TYPE ";			/* For typing a file */
char *SPACMD = "DIRECTORY/TOTAL";	/* Space/quota of current directory */
char *SPACM2 = "DIRECTORY/TOTAL ";	/* Space/quota of current directory */
char *WHOCMD = "SHOW USERS";		/* For seeing who's logged in */
char *PWDCMD = "SHOW DEFAULT";		/* For seeing current directory */

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
 */


/* Includes */

#include "ckcker.h"
#include "ckcdeb.h"
#include <stdio.h>
#include <ctype.h>
#include <rms.h>
#include <descrip.h>
#include <dvidef.h>
#include <iodef.h>
#include <errno.h>

#define MAXWLD 500			/* Maximum wildcard filenames */


/* Declarations */

FILE *fp[ZNFILS] = { 			/* File pointers */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static int fcount;			/* Number of files in wild group */
char *getenv(), *strcpy();		/* For finding home directory */

static char *mtchs[MAXWLD],		/* Matches found for filename */
     **mtchptr;				/* Pointer to current match */


/***  Z K S E L F --  Log self out  ***/

/*** (someone please check if this works in VMS) ***/

zkself() {
    return (kill(0,9));
}


/*  Z O P E N I  --  Open an existing file for input. */

zopeni(n,name) int n; char *name; {
    debug(F111," zopeni",name,n);
    debug(F101,"  fp","",(int) fp[n]);
    if (n == ZSYSFN) {			/* Input from a system function? */
	return(zxcmd(name));		/* Try to fork the command */
    }
    if (n == ZSTDIO) {			/* Standard input? */
	if (isatty(0)) {
	    fprintf(stderr,"?Terminal input not allowed\n");
	    debug(F110,"zopeni: attempts input from unredirected stdin","",0);
	    return(0);
	}
	fp[ZIFILE] = stdin;
	return(1);
    }
    if (chkfn(n) != 0) return(0);
    fp[n] = fopen(name,"r");		/* Real file. */
    debug(F111," zopeni", name, (int) fp[n]);
    if (fp[n] == NULL) perror(name);	/* +1, want a useful message	*/
    return((fp[n] != NULL) ? 1 : 0);
}

/*  Z O P E N O  --  Open a new file for output.  */

zopeno(n,name) int n; char *name; {

    int fildes;
    extern int binary;

    debug(F111," zopeno",name,n);
    if (chkfn(n) != 0) return(0);
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
    if (n == ZOFILE && binary != 0)
	fildes = creat(name, 0, "mrs=512", "rfm=fix");
    else
	fildes = creat(name, 0, "rat=cr", "rfm=var");
    fp[n] = (fildes == -1) ? NULL : fdopen(fildes, "w");
    if (fp[n] == NULL) perror(name);		/* +1, print useful msg	*/
    if (n == ZDFILE && isatty(fileno(fp[n])))
	setbuf(fp[n],NULL); /* Make debugging file unbuffered */
    debug(F101, " fp[n]", "", (int) fp[n]);
    return((fp[n] != NULL) ? 1 : 0);
}

/*  Z C L O S E  --  Close the given file.  */

/*  Returns 0 if arg out of range, 1 if successful, -1 if close failed.  */

zclose(n) int n; {
    int x;
    if (chkfn(n) < 1) return(0);
    if ((fp[n] != stdout) && (fp[n] != stdin)) x = fclose(fp[n]);
    fp[n] = NULL;
    return((x == EOF) ? -1 : 1);
}

/*  Z C H I N  --  Get a character from the input file.  */

static int subprocess_input = 0, sub_count;
static int input_mbxchn, output_mbxchn, child_pid = 0;
static char *sub_ptr, sub_buf[200];

get_subprc_line() {
    struct { short status, size, trm, trmsize; } subiosb;
    if ((SYS$QIOW(0, output_mbxchn, IO$_READVBLK, &subiosb, 0, 0,
		sub_buf, sizeof sub_buf, 0, 0, 0, 0) & 7) != 1
	|| (subiosb.status & 7) != 1) return(-1);
    if (subiosb.size == 29
     && strncmp(sub_buf, ">>> END OF KERMIT COMMAND <<<",
		subiosb.size) == 0) {
	subprocess_input = 0;
	return(-1);
    }
    sub_buf[subiosb.size] = '\n';
    sub_buf[subiosb.size + 1] = '\0';
    sub_count = subiosb.size;
    sub_ptr = sub_buf;
    return(0);
}

zchin(n,c) int n; char *c; {
    int a;
    if (n == ZIFILE && subprocess_input) {
	if (--sub_count < 0)
	    if (get_subprc_line()) return(-1);
	a = *sub_ptr++;
    } else {
	if (chkfn(n) < 1) return(-1);
	a = getc(fp[n]);
    }
    if (a == EOF) return(-1);
    *c = (a & 0377);
    return(0);
}


/*  Z S O U T  --  Write a string to the given file, buffered.  */

zsout(n,s) int n; char *s; {
    if (chkfn(n) < 1) return(-1);
    fputs(s, fp[n]);			/* Don't use fprintf here MM */
    return(0);
}

/*  Z S O U T L  --  Write string to file, with line terminator, buffered  */

zsoutl(n,s) int n; char *s; {
    if (chkfn(n) < 1) return(-1);
    fputs(s, fp[n]);			/* Don't use fprintf MM */
    putc('\n', fp[n]);
    return(0);
}

/*  Z S O U T X  --  Write x characters to file, unbuffered.  */

zsoutx(n,s,x) int n, x; char *s; {
    if (chkfn(n) < 1) return(-1);
    return(write(fp[n]->_file,s,x));
}


/*  Z C H O U T  --  Add a character to the given file.  */

zchout(n,c) int n; char c; {
    if (chkfn(n) < 1) return(-1);
    if (n == ZSFILE)
    	return(write(fp[n]->_file,&c,1)); /* Use unbuffered for session log */
    else {
	if (putc(c,fp[n]) == EOF)	/* If true, maybe there was an error */
	    return(ferror(fp[n]));	/* Check to make sure */
	else				/* Otherwise... */
	    return(0);			/* There was no error. */
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
    int x; long pos;

    x = open(name, 0);
    if (x < 0) {
	debug(F111,"zchki stat fails",name,errno);
	return(-1);
    }
    pos = lseek(x, 0, 2);
    close(x);
    return(pos);
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
    delete(name);
}


/*  Z R T O L  --  Convert remote filename into local form  */

/*  For VMS, we eliminate all special characters and truncate.  */
/*  Doesn't allow the longer filespecs that VMS V4 supports.    */
/*  Assumes version number delimited by semicolon, not period.  */
/*  Should really use RMS to parse filespec components.  -- DS  */

zrtol(name,name2) char *name, *name2; {
    int count;
    char *cp;

    count = 9;
    for ( cp = name2; *name != '\0'; name++ ) {
	switch (*name) {
	    case '.':			/* File type */
	    	count = 3;		/* Max length for this field */
		*cp++ = '.';
		break;
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
    *cp = '\0';				/* End of name */
    debug(F110,"zrtol: ",name2,0);
}


/*  Z L T O R  --  Convert filename from local format to common form.   */

zltor(name,name2) char *name, *name2; {
    char *cp, *pp;

    for (cp = pp = name; *cp != '\0'; cp++) {	/* strip path name */
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

    debug(F110,"zltor: ",name2,0);
}    


/*  Z C H D I R  --  Change directory  */

zchdir(dirnam) char *dirnam; {
    char *hd;
    if (*dirnam == '\0') hd = getenv("HOME");
    else hd = dirnam;
    return((chdir(hd) == 0) ? 1 : 0);
}


/*  Z H O M E  --  Return pointer to user's home directory  */

char *
zhome() {
    return(getenv("HOME"));
}

/*  Z G T D I R  --  Return pointer to user's current directory  */

char *
zgtdir() {
/*    char *getcwd();
/*    char cwdbuf[100];
/*    char *buf;
/*    buf = cwdbuf;
/*    return(getcwd(buf,100));
*/
    return("");  /* Can't seem to make LINK find getcwd()... */
}

/*  Z X C M D -- Run a system command so its output can be read like a file */

zxcmd(comand) char *comand; {
    char input_mbxnam[10], output_mbxnam[10];
    char cmdbuf[200];

    if (child_pid == 0) {
    	struct dsc$descriptor_s inpdsc, outdsc;
    	struct { short buflen, code; char *bufadr; short *retlen; } itmlst[2];

        SYS$CREMBX(0, &input_mbxchn, 0, 0, 0, 0, 0);
        itmlst[0].buflen = sizeof input_mbxnam;
        itmlst[0].code   = DVI$_DEVNAM;
        itmlst[0].bufadr = input_mbxnam;
        itmlst[0].retlen = 0;
        itmlst[1].buflen = 0;
        itmlst[1].code   = 0;
        SYS$GETDVI(0, input_mbxchn, 0, itmlst, 0, 0, 0, 0);
        SYS$WAITFR(0);

        SYS$CREMBX(0, &output_mbxchn, 0, 0, 0, 0, 0);
        itmlst[0].buflen = sizeof output_mbxnam;
        itmlst[0].bufadr = output_mbxnam;
        SYS$GETDVI(0, output_mbxchn, 0, itmlst, 0, 0, 0, 0);
        SYS$WAITFR(0);

        inpdsc.dsc$w_length  = strlen(input_mbxnam);
        inpdsc.dsc$b_dtype   = DSC$K_DTYPE_T;
        inpdsc.dsc$b_class   = DSC$K_CLASS_S;
        inpdsc.dsc$a_pointer = input_mbxnam;

        outdsc.dsc$w_length  = strlen(output_mbxnam);
        outdsc.dsc$b_dtype   = DSC$K_DTYPE_T;
        outdsc.dsc$b_class   = DSC$K_CLASS_S;
        outdsc.dsc$a_pointer = output_mbxnam;

        LIB$SPAWN(0, &inpdsc, &outdsc, &1, 0, &child_pid);
	SYS$QIOW(0, input_mbxchn, IO$_WRITEVBLK | IO$M_NOW, 0, 0, 0,
	    "$ SET NOON", 10, 0, 0, 0, 0);
    }

    strcpy(cmdbuf, "$ ");
    strcat(cmdbuf, comand);
    SYS$QIOW(0, input_mbxchn, IO$_WRITEVBLK | IO$M_NOW, 0, 0, 0,
	    cmdbuf, strlen(cmdbuf), 0, 0, 0, 0);
    SYS$QIOW(0, input_mbxchn, IO$_WRITEVBLK | IO$M_NOW, 0, 0, 0,
	    "$ WRITE SYS$OUTPUT \">>> END OF KERMIT COMMAND <<<\"",
	    50, 0, 0, 0, 0);
    subprocess_input = 1;
    sub_count = 0;
    return(1);
}

/*  Z C L O S F  - close the suprocess output file.  */

zclosf() {
}

/*  Z K I L L F  - kill the subprocess used for host commands  */
/*  The return value is 1 if the subprocess was killed successfully. */
/*			-1 if there was no subprocess to kill. */

zkillf() {
    if (child_pid == 0) return(-1);
    return((SYS$DELPRC(&child_pid) & 7) == 1);
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
    static char buf[100];
    char *bp, *xp;
    int len = 0, n = 0, d = 0, t;

    strcpy(buf, fn);			/* Version numbers are handled by OS */
    *s = buf;
}


/*  Wildcard expansion for VMS is easy;  we just use a run-time library call.
*/
fgen(pat,resarry,len)
char *pat,*resarry[];
int len;
{
    struct dsc$descriptor_s file_spec, result, deflt;
    long context;
    int count, slen, status, plen;
    char *pp, *rp, result_string[256], *strchr();

    file_spec.dsc$w_length  = strlen(pat);
    file_spec.dsc$b_dtype   = DSC$K_DTYPE_T;
    file_spec.dsc$b_class   = DSC$K_CLASS_S;
    file_spec.dsc$a_pointer = pat;

    result.dsc$w_length  = sizeof result_string;
    result.dsc$b_dtype   = DSC$K_DTYPE_T;
    result.dsc$b_class   = DSC$K_CLASS_S;
    result.dsc$a_pointer = result_string;

    deflt.dsc$w_length  = 3;
    deflt.dsc$b_dtype   = DSC$K_DTYPE_T;
    deflt.dsc$b_class   = DSC$K_CLASS_S;
    deflt.dsc$a_pointer = "*.*";

    count = 0;
    context = 0;
    pp = strchr(pat, ']');
    if (pp == 0) pp = strchr(pat, ':');
    if (pp == 0) plen = 0;
    else plen = pp - pat + 1;
    while (count < len
	   && (status = LIB$FIND_FILE(&file_spec, &result, &context, &deflt))
		== RMS$_NORMAL) {
	rp = strchr(result_string, ']') + 1;
	slen = strchr(rp, ' ') - rp;
    	resarry[count] = malloc(slen + plen + 1);
	if (plen != 0)
	    strncpy(resarry[count], pat, plen);
	strncpy(resarry[count] + plen, rp, slen);
	resarry[count][slen + plen] = '\0';
	++count;
    }
#ifdef DVI$_ALT_HOST_TYPE
    lib$find_file_end(&context);	/* Only on V4 and later */
#endif
    if (status == RMS$_FNF) return(0);
    if (status == RMS$_NMF) return(count);
    return(-1);
}

system(s)  char *s;  {
    struct dsc$descriptor_s cmd;

    if ( *s ) {
	zxcmd(s);
	while (!get_subprc_line())
	    fputs(sub_buf, stdout);
	putchar('\n');
    } else {
	LIB$SPAWN();
    }
}
