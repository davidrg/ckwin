char *ckzv = "Amiga file support, 4F(005) 15 Oct 89";
 
/* C K I F I O  --  Kermit file system support for the Amiga */

/*
 Author: Frank da Cruz (SY.FDC@CU20B),
 Columbia University Center for Computing Activities, January 1985.
 Copyright (C) 1985, Trustees of Columbia University in the City of New York.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained.
 
 Modified for Amiga by Jack J. Rouse, The Software Distillery

 Further modified for C Kermit version 4F(095) by Stephen Walton,
 California State University, Northridge, ecphssrw@afws.csun.edu
*/

/* Includes */
 
#include "ckcker.h"		/* Kermit definitions */
#include "ckcdeb.h"		/* Typedefs, formats for debug() */
#include <stdio.h>		/* Unix Standard i/o */
#include <ctype.h>
#define MAXNAMLEN 30

char *ckzsys = " Amiga";

/* Definitions of some Amiga system commands */
 
char *DIRCMD = "list ";		/* For directory listing */
char *DELCMD = "delete ";		/* For file deletion */
char *TYPCMD = "type ";			/* For typing a file */
char *PWDCMD = "cd ";			/* For saying where I am */
 
char *SPACMD = "info ";
 
char *SPACM2 = "info ";			/* should be space in specified directory */
 
char *WHOCMD = "status ";		/* Check process status */

#define MAXWLD 300

/*
  Functions (n is one of the predefined file numbers from ckermi.h):
 
   zopeni(n,name)   -- Opens an existing file for input.
   zopeno(n,name)   -- Opens a new file for output.
   zclose(n)        -- Closes a file.
   zchin(n,&c)      -- Gets the next character from an input file.
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
   zkself()         -- Kill self, log out own job (simply exits)
 */

/* Declarations */
 
FILE *fp[ZNFILS] = { 			/* File pointers */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL };
 
/* (PWP) external def. of things used in buffered file input and output */
extern CHAR zinbuffer[], zoutbuffer[];
extern CHAR *zinptr, *zoutptr;
extern int zincnt, zoutcnt;

static long iflen = -1;			/* Input file length. */
static long oflen = -1;			/* Output file length. */

static int fcount;			/* Number of files in wild group */
static char  nambuf[MAXNAMLEN+2];	/* Buffer for a filename */
char *malloc(), *strcpy();		/* System functions */
char *strrchr();
 
static char *mtchs[MAXWLD],		/* Matches found for filename */
     **mtchptr;				/* Pointer to current match */

/* utility functions from ckiutl.c */
extern int existobj();
struct DirHandle	/* fake structure definition */
{
    int _foo_;
};
extern struct DirHandle *opendir();
extern char *readdir();
extern closedir();

/*  Z K S E L F  --  Kill Self: log out own job, if possible.  */
 
zkself() {				/* For "bye", but no guarantee! */
	doexit(GOOD_EXIT);
}

/*  Z O P E N I  --  Open an existing file for input. */
 
zopeni(n,name) int n; char *name; {
    debug(F111," zopeni",name,n);
    debug(F101,"  fp","",(int) fp[n]);
    if (chkfn(n) != 0) return(0);
    if (n == ZSYSFN) {			/* Input from a system function? */
        debug(F110," invoking zxcmd",name,0);
        *nambuf = '\0';			/* Invalidate file name */
	return(zxcmd(name));		/* Try to fork the command */
    }
    if (n == ZSTDIO) {			/* Standard input? */
	if (isatty(0)) {
	    ermsg("Terminal input not allowed");
	    debug(F110,"zopeni: attempts input from unredirected stdin","",0);
	    return(0);
	}
	fp[ZIFILE] = stdin;
	return(1);
    }
    fp[n] = fopen(name,"r");		/* Real file. */
    debug(F111," zopeni", name, (int) fp[n]);
    if (fp[n] == NULL) perror("zopeni");
    return((fp[n] != NULL) ? 1 : 0);
}
 
/*  Z O P E N O  --  Open a new file for output.  */
 
zopeno(n,name) int n; char *name; {
    debug(F111," zopeno",name,n);
    if (chkfn(n) != 0) return(0);
    if ((n == ZCTERM) || (n == ZSTDIO)) {   /* Terminal or standard output */
	fp[ZOFILE] = stdout;
	debug(F101," fp[]=stdout", "", (int) fp[n]);
	return(1);
    }
    fp[n] = fopen(name,"w");		/* A real file, try to open */
    if (fp[n] == NULL) {
        perror("zopeno can't open");
    } else {
        if (n == ZDFILE) setbuf(fp[n],NULL); /* Debugging file unbuffered */
    }
    zoutcnt = 0;		/* (PWP) reset output buffer */
    zoutptr = zoutbuffer;
    if (n != ZDFILE)
      debug(F101, " fp[n]", "", (int) fp[n]);
    return((fp[n] != NULL) ? 1 : 0);
}

/*  Z C L O S E  --  Close the given file.  */
 
/*  Returns 0 if arg out of range, 1 if successful, -1 if close failed.  */
 
zclose(n) int n; {
    int x, x2;

    if (chkfn(n) < 1) return(0);	/* Check range of n */

    if ((n == ZOFILE) && (zoutcnt > 0))	/* (PWP) output leftovers */
      x2 = zoutdump();
    else
      x2 = 0;

    if ((n == ZIFILE) && fp[ZSYSFN]) {	/* If system function */
    	x = zclosf();			/* do it specially */
    } else {
    	if ((fp[n] != stdout) && (fp[n] != stdin)) x = fclose(fp[n]);
	fp[n] = NULL;
    }
    iflen = -1;				/* Invalidate file length */
    if (x == EOF)			/* if we got a close error */
	return (-1);
    else if (x2 < 0)		/* or an error flushing the last buffer */
	return (-1);		/* then return an error */
    else
	return (1);
}
 
/*  Z C H I N  --  Get a character from the input file.  */
 
/*  Returns -1 if EOF, 0 otherwise with character returned in argument  */
 
zchin(n,c) int n; char *c; {
    int a;

    /* (PWP) Just in case this gets called when it shoudn't */
    if (n == ZIFILE)
	return (zminchar());

    /* if (chkfn(n) < 1) return(-1); */
    a = getc(fp[n]);
    if (a == EOF) return(-1);
    *c = a & 0377;
    return(0);
}

/*
 * (PWP) (re)fill the buffered input buffer with data.  All file input
 * should go through this routine, usually by calling the zminchar()
 * macro (in ckcker.h).
 */

zinfill() {
    zincnt = fread(zinbuffer, sizeof (char), INBUFSIZE, fp[ZIFILE]);
    if (zincnt == 0) return (-1); /* end of file */
    zinptr = zinbuffer;	   /* set pointer to beginning, (== &zinbuffer[0]) */
    zincnt--;			/* one less char in buffer */
    return((int)(*zinptr++) & 0377); /* because we return the first */
}

/*  Z S O U T  --  Write a string to the given file, buffered.  */
 
zsout(n,s) int n; char *s; {
    if (chkfn(n) < 1) return(-1);
    fputs(s,fp[n]);
    return(0);
}
 
/*  Z S O U T L  --  Write string to file, with line terminator, buffered  */
 
zsoutl(n,s) int n; char *s; {
    if (chkfn(n) < 1) return(-1);
    fputs(s,fp[n]);
    fputs("\n",fp[n]);
    return(0);
}
 
/*  Z S O U T X  --  Write x characters to file, unbuffered.  */
 
zsoutx(n,s,x) int n, x; char *s; {
    if (chkfn(n) < 1) return(-1);
    return(write(fileno(fp[n]),s,x));
}
 
 
/*  Z C H O U T  --  Add a character to the given file.  */
 
/*  Should return 0 or greater on success, -1 on failure (e.g. disk full)  */
 
zchout(n,c) int n; CHAR c; {
    if (chkfn(n) < 1) return(-1);
    if (n == ZSFILE)
    	return(write(fileno(fp[n]),&c,1)); /* Use unbuffered for session log */
    else {				/* Buffered for everything else */
	if (putc(c,fp[n]) == EOF)	/* If true, maybe there was an error */
	    return(ferror(fp[n])?-1:0);	/* Check to make sure */
	else				/* Otherwise... */
	    return(0);			/* There was no error. */
    }
}

/* (PWP) buffered character output routine to speed up file IO */

zoutdump() {
    int x;
    zoutptr = zoutbuffer;		/* reset buffer pointer in all cases */
    debug(F101,"zoutdump chars","",zoutcnt);
    if (zoutcnt == 0) {			/* nothing to output */
	return(0);
    } else if (zoutcnt < 0) {		/* unexpected negative value */
	zoutcnt = 0;			/* reset output buffer count */
	return(-1);			/* and fail. */
    }
    if (x = fwrite(zoutbuffer, 1, zoutcnt, fp[ZOFILE])) {
	debug(F101,"zoutdump fwrite wrote","",x);
	zoutcnt = 0;			/* reset output buffer count */
	return(0);			/* things worked OK */
    } else {
	zoutcnt = 0;			/* reset output buffer count */
	x = ferror(fp[ZOFILE]);		/* get error code */
	debug(F101,"zoutdump fwrite error","",x);
	return(x ? -1 : 0);		/* return failure if error */
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
	case ZSFILE:
	case ZSYSFN: break;
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
long
zchki(name) char *name; {
	long size, readstat();

	size = readstat(name);
	debug(F111,"zchki file size",name,(int)size);
	iflen = size;
	strcpy(nambuf, name);		/* Remember file name globally. */
	return(size);
}

/*  Z C H K O  --  Check if output file can be created  */
 
/*
 Returns -1 if write permission for the file would be denied, 0 otherwise.
*/
zchko(name) char *name; {
    int rc = writestat(name);

#ifdef DEBUG
    if (rc < 0)
	debug(F111,"zchko access failed:",name,NULL);
    else
	debug(F111,"zchko access ok:",name,NULL);
#endif
    return(rc);
}

/*  Z D E L E T  --  Delete the named file.  */
 
zdelet(name) char *name; {
    unlink(name);
}
 
 
/*  Z R T O L  --  Convert remote filename into local form  */
 
/*  For AMIGA, this means changing uppercase letters to lowercase.  */
 
zrtol(name,name2) char *name, *name2; {
    for ( ; *name != '\0'; name++) {
    	*name2++ = isupper(*name) ? tolower(*name) : *name;
    }
    *name2 = '\0';
    debug(F110,"zrtol:",name2,0);
}
 
 
/*  Z L T O R  --  Local TO Remote */
 
/*  Convert filename from local format to common (remote) form.  */
 
zltor(name,name2) char *name, *name2; {
    char work[100], *cp, *pp;
    int dc = 0;
 
    debug(F110,"zltor",name,0);
    pp = work;
    if ((cp = strrchr(name, ':')) == NULL)
        cp = name;
    else
    	++cp;

    for (; *cp != '\0'; cp++) {	/* strip path name */
    	if (*cp == '/') {
	    dc = 0;
	    pp = work;
	}
	else if (islower(*cp)) *pp++ = toupper(*cp); /* Uppercase letters */
	else if (*cp == '~') *pp++ = 'X';	/* Change tilde to 'X' */
	else if (*cp == '#') *pp++ = 'X';	/* Change number sign to 'X' */
	else if ((*cp == '.') && (++dc > 1)) *pp++ = 'X'; /* & extra dots */
	else *pp++ = *cp;
    }
    *pp = '\0';				/* Tie it off. */
    cp = name2;				/* If nothing before dot, */
    if (*work == '.') *cp++ = 'X';	/* insert 'X' */
    strcpy(cp,work);
    debug(F110," name2",name2,0);
}    
 
/*  Z H O M E  --  Return pointer to user's home directory  */

/* we return "s:", which is where startup scripts are found */
char *
zhome() {
	return("s:");		/* very approximately */
}

/*  Z C H D I R  --  Change directory  */

zchdir(dirnam) char *dirnam; {
    return((chdir(dirnam) == 0) ? 1 : 0);
}

/*  Z G T D I R  --  Return pointer to user's current directory  */

char *
zgtdir() {
    return("");
}

/*  Z X C M D -- Run a system command so its output can be read like a file */
zxcmd(comand) char *comand; {
	FILE *f, *pipeopen();

	if ((f = pipeopen(comand)) == NULL) return(0);
	fp[ZIFILE] = f;			/* open a stream for it */
	fp[ZSYSFN] = fp[ZIFILE];	/* Remember. */
	return(1);
}
 
/*  Z C L O S F  - wait for the child fork to terminate and close the pipe. */
 
zclosf() {
	pipeclose(fp[ZIFILE]);
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
    static char buf[100];
    char *bp;
    int len = 0, d;
#ifdef MAXNAMLEN
    int maxlen = MAXNAMLEN;
#else
    int maxlen = 14;
#endif
 
    bp = buf;
    while (*fn) {			/* Copy name into buf */
	*bp++ = *fn++;
	len++;
    }
    if (len > maxlen-3) bp -= 3;	/* Don't let it get too long */

    /* 
     * On the Amiga, it takes much less time to determine
     * if a given file exists than to read all the file names in
     * a directory (or even just names with a certain prefix).
     */
    d = 0;
    do {
	sprintf(bp, "~%d", ++d);
    } while (zchki(buf) != -1 && d < 100);

    *s = buf;
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
    char *zfcdat();

    k = iflen % 1024L;			/* File length in K */
    if (k != 0L) k = 1L;
    xx->lengthk = (iflen / 1024L) + k;
    xx->type.len = 0;			/* File type can't be filled in here */
    xx->type.val = "";
    if (*nambuf) {
	xx->date.val = zfcdat(nambuf);	/* File creation date */
	xx->date.len = strlen(xx->date.val);
    } else {
	xx->date.len = 0;
	xx->date.val = "";
    }
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
    xx->systemid.len = 2;		/* System ID length */
    xx->systemid.val = "L3";		/* Amiga system ID code */
    xx->recfm.len = 0;			/* Record format */
    xx->recfm.val = "";
    xx->sysparam.len = 0;		/* System-dependent parameters */
    xx->sysparam.val = "";
    xx->length = iflen;			/* Length */
    return(0);
}

/* Z F C D A T -- Return a string containing the time stamp for a file */

char *
zfcdat(name) char *name; {

#ifdef TIMESTAMP

    struct stat buffer;
    struct tm *time_stamp, *localtime();
    static char datbuf[20];

    datbuf[0] = '\0';
    if(stat(name,&buffer) != 0) {
	debug(F110,"zcfdat stat failed",name,0);
	return("");
    }
    time_stamp = localtime(&(buffer.st_mtime));
    if (time_stamp->tm_year < 1900) time_stamp->tm_year += 1900;
    sprintf(datbuf,"%-4.4d%02.2d%02.2d %002.2d:%002.2d:%002.2d",
	    time_stamp->tm_year,
	    time_stamp->tm_mon + 1,
	    time_stamp->tm_mday,
	    time_stamp->tm_hour,
	    time_stamp->tm_min,
	    time_stamp->tm_sec);
    debug(F111,"zcfdat",datbuf,strlen(datbuf));
    return(datbuf);
#else
    return("");
#endif /* TIMESTAMP */
}

/*
 * Dummy functions for the Amiga.  Sending mail cannot be done;  I haven't
 * decided how to handle print requests yet.
 */

zmail(p,f) char *p; char *f; {		/* Send file f as mail to address p */
	return(0);
}

zprint(p,f) char *p; char *f; {		/* Print file f with flags p */
	return(0);
}


/* Directory Functions for Unix, written by Jeff Damens, CUCCA, 1984. */

/*
 * The path structure is used to represent the name to match.
 * Each slash-separated segment of the name is kept in one
 * such structure, and they are linked together, to make
 * traversing the name easier.
 */
 
struct path {
              char npart[MAXNAMLEN];	/* name part of path segment */
              struct path *fwd;		/* forward ptr */
            };
 
#define SSPACE 4000			/* size of string-generating buffer */

static char sspace[SSPACE];             /* buffer to generate names in */
static char *freeptr,**resptr;         	/* copies of caller's arguments */
static int remlen;                      /* remaining length in caller's array*/
static int numfnd;                      /* number of matches found */
 
/*
 * splitpath:
 *  takes a string and splits the slash-separated portions into
 *  a list of path structures.  Returns the head of the list.  The
 *  structures are allocated by malloc, so they must be freed.
 *  Splitpath is used internally by the filename generator.
 *
 * Input: A string.
 * Returns: A linked list of the slash-separated segments of the input.
 */

struct path *
splitpath(p)
char *p;
{
 struct path *head,*cur,*prv;
 int i;
 head = prv = NULL;
 if (*p == '/') p++;            /* skip leading slash */
 while (*p != '\0')
 {
   cur = (struct path *) malloc(sizeof (struct path));
   debug(F101,"splitpath malloc","",cur);
   if (cur == NULL) fatal("malloc fails in splitpath()");
   cur -> fwd = NULL;
   if (head == NULL) head = cur;
   else prv -> fwd = cur;       /* link into chain */
   prv = cur;
   for (i=0; i < MAXNAMLEN && *p != '/' && *p != '\0'; i++)
     cur -> npart[i] = *p++;
   cur -> npart[i] = '\0';      /* end this segment */
   if (i >= MAXNAMLEN) while (*p != '/' && *p != '\0') p++;
   if (*p == '/') p++;
 }
 return(head);
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
 * By: Jeff Damens, CUCCA, 1984.
 */
 
fgen(pat,resarry,len)
char *pat,*resarry[];
int len;
{
 struct path *head;
 char scratch[100],*sptr;
 char *tail;

 if ((tail = strrchr(pat, ':')) == NULL) /* locate unit name */
  tail = pat;				/* no unit name */
 else
  ++tail;				/* eat ':' */
 while (*tail == '/')			/* eat parent path slashes */
  ++tail;
 sptr = scratch;			/* init buffer correctly */
 while (pat < tail)
  *sptr++ = *pat++;
 head = splitpath(pat);
 numfnd = 0;                            /* none found yet */
 freeptr = sspace;			/* this is where matches are copied */
 resptr = resarry;			/* static copies of these so*/
 remlen = len;				/* recursive calls can alter them */
 traverse(head,scratch,sptr);		/* go walk the directory tree */
 for (; head != NULL; head = head -> fwd)
   free(head);				/* return the path segments */
 return(numfnd);			/* and return the number of matches */
}

/* traverse:
 *  Walks the directory tree looking for matches to its arguments.
 *  The algorithm is, briefly:
 *   If the current pattern segment contains no wildcards, that
 *   segment is added to what we already have.  If the name so far
 *   exists, we call ourselves recursively with the next segment
 *   in the pattern string; otherwise, we just return.
 *
 *   If the current pattern segment contains wildcards, we open the name
 *   we've accumulated so far (assuming it is really a directory), then read 
 *   each filename in it, and, if it matches the wildcard pattern segment, add
 *   that filename to what we have so far and call ourselves recursively on the
 *   next segment.
 *
 *   Finally, when no more pattern segments remain, we add what's accumulated
 *   so far to the result array and increment the number of matches.
 *
 * Input: a pattern path list (as generated by splitpath), a string
 *	  pointer that points to what we've traversed so far (this
 *	  can be initialized to "/" to start the search at the root
 *	  directory, or to "./" to start the search at the current
 *	  directory), and a string pointer to the end of the string
 *	  in the previous argument.
 * Returns: nothing.
 */
traverse(pl,sofar,endcur)
struct path *pl;
char *sofar,*endcur;
{
 struct DirHandle *fd;
 char *fname;

 if (pl == NULL)
 {
  *--endcur = '\0';                    /* end string, overwrite trailing / */
  addresult(sofar);
  return;
 }
 if (!iswild(pl -> npart))
 {
  strcpy(endcur,pl -> npart);
  endcur += strlen(pl -> npart);
  *endcur = '\0';                     	/* end current string */
  if (existobj(sofar))			/* if current piece exists */
  {
      *endcur++ = '/';			/* add slash to end */
      *endcur = '\0';			/* and end the string */
      traverse(pl -> fwd,sofar,endcur);
  }
  return;
 }
/* cont'd... */

/*...traverse, cont'd */
 
/* segment contains wildcards, have to search directory */
 *endcur = '\0';                        	/* end current string */
 if ((fd = opendir(sofar)) == NULL) return;  	/* can't open, forget it */
 while (fname = readdir(fd))
{
  if (match(pl -> npart,fname)) {
    char *eos;
    strcpy(endcur,fname);
    eos = endcur + strlen(fname);
    *eos = '/';                    /* end this segment */
    traverse(pl -> fwd,sofar,eos+1);
  }
}
 closedir(fd);
}

/*
 * addresult:
 *  Adds a result string to the result array.  Increments the number
 *  of matches found, copies the found string into our string
 *  buffer, and puts a pointer to the buffer into the caller's result
 *  array.  Our free buffer pointer is updated.  If there is no
 *  more room in the caller's array, the number of matches is set to -1.
 * Input: a result string.
 * Returns: nothing.
 */
 
addresult(str)
char *str;
{
 int l;
 if (--remlen < 0) {
  numfnd = -1;
  return;
 }
 l = strlen(str) + 1;			/* size this will take up */
 if ((freeptr + l) > &sspace[SSPACE]) {
    numfnd = -1;			/* do not record if not enough space */
    return;
  }
 strcpy(freeptr,str);
 *resptr++ = freeptr;
 freeptr += l;
 numfnd++;
}
 
iswild(str)
char *str;
{
 char c;
 while ((c = *str++) != '\0')
   if (c == '*' || c == '?') return(1);
 return(0);
}

#ifdef OLDMATCH
/*
 * match:
 *  pattern matcher.  Takes a string and a pattern possibly containing
 *  the wildcard characters '*' and '?'.  Returns true if the pattern
 *  matches the string, false otherwise.
 * by: Jeff Damens, CUCCA
 *
 * Input: a string and a wildcard pattern.
 * Returns: 1 if match, 0 if no match.
 */
 
match(pattern,string)
register char *pattern,*string;
{
    char *psave,*ssave;			/* back up pointers for failure */
    psave = ssave = NULL;
    while (1) {
	for (;
	     tolower(*pattern) == tolower(*string);
	     pattern++,string++)  /* skip first */
	    if (*string == '\0') return(1);	/* end of strings, succeed */
	if (*string != '\0' && *pattern == '?') {
	    pattern++;			/* '?', let it match */
	    string++;
	} else if (*pattern == '*') {	/* '*' ... */
	    psave = ++pattern;		/* remember where we saw it */
	    ssave = string;		/* let it match 0 chars */
	} else if (ssave != NULL && *ssave != '\0') {	/* if not at end  */
  					/* ...have seen a star */
	    string = ++ssave;		/* skip 1 char from string */
	    pattern = psave;		/* and back up pattern */
	} else return(0);		/* otherwise just fail */
    }
}
#else
/*
 * match -- match wildcard pattern to string
 *    allows multiple '*'s and works without backtracking
 *    upper and lower case considered equivalent
 *    written by Jack Rouse
 *    working without backtracking is cute, but is this usually going
 *       to be the most efficient method?
 */
match(pattern, target)
register char *pattern, *target;
{
	int link[MAXNAMLEN];		/* list of matches to try in pattern */
	register int first, last;	/* first and last items in list */
	register int here, next;	/* current and next list items */
	char lowch;			/* current target character */

	/* start out trying to match at first position */
	first = last = 0;
	link[0] = -1;

	/* go through the target */
	for (; *target; ++target)
	{
		/* get lowercase target character */
		lowch = tolower(*target);

		/* go through all positions this round and build next round */
		last = -1;
		for (here = first; here >= 0; here = next)
		{
			next = link[here];
			switch (pattern[here])
			{
			case '*':
				/* try match at here+1 this round */
				/*!!!check needed only if "**" allowed? */
				if (next != here + 1)
				{
					link[here + 1] = next;
					next = here + 1;
				}
				/* retry match at here next round */
				break;
			default:
				if (tolower(pattern[here]) != lowch)
					continue;
				/* matched, fall through */
			case '?':
				/* try match at here+1 next round */
				++here;
				break;
			}
			/* try match at here value next round */
			if (last < 0)
				first = here;
			else
				link[last] = here;
			last = here;
		}
		/* if no positions left, match failed */
		if (last == -1) return(0);
		/* terminate list */
		link[last] = -1;
	}

	/* at end of target, skip empty matches */
	while (pattern[last] == '*')
		++last;

	return(pattern[last] == '\0');
}
#endif
