char *ckzv = "Unix file support, 4.1(015) 28 Feb 85";

/* C K Z B S D  --  Kermit file system support for Unix systems */

/* F. da Cruz, Columbia University Center for Computing Activities */

/* Berkeley Unix Version 4.x */
#ifdef BSD4
char *ckzsys = " 4.x BSD";
#endif

/* DEC Professional-300 series with Venturcom Venix 1.0 */
#ifdef PROVX1
char *ckzsys = " DEC Pro-3xx/Venix 1.0";
#endif

/* NCR Tower support contributed by Kevin O'Kane, U. of Tennessee */
/* Tower OS is like Sys III but with BSD features -- mostly follows BSD */
#ifdef TOWER1
char *ckxsys = " NCR Tower 1632, OS 1.02";
#endif

/* Sys III/V, Xenix, PC/IX,... support by Herm Fischer, Litton Data Systems */
#ifdef UXIII
#ifdef XENIX
char *ckzsys = " Xenix/286";
#else
#ifdef PCIX
char *ckzsys = " PC/IX";
#else
#ifdef ISIII
char *ckzsys = " Interactive Systems Corp, System III";
#else
char *ckzsys = " AT&T System III/System V";
#endif
#endif
#endif
#endif

/* Definitions of some Unix system commands */

char *DIRCMD = "ls -l ";		/* For directory listing */
char *DELCMD = "rm -f ";		/* For file deletion */
char *TYPCMD = "cat ";			/* For typing a file */

#ifdef BSD4
char *SPACMD = "pwd ; quota ; df .";	/* Space/quota of current directory */
#else
char *SPACMD = "df ";
#endif

char *SPACM2 = "df ";			/* For space in specified directory */

#ifdef BSD4
char *WHOCMD = "finger ";		/* For seeing who's logged in */
#else
char *WHOCMD = "who ";			/* For seeing who's logged in */
#endif

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
 */

/* Includes */

#include "ckermi.h"			/* Kermit definitions, ctype, stdio */
#include <sys/types.h>			/* Data types */
#include <sys/dir.h>			/* Directory structure */
#include <sys/stat.h>			/* File status */
#include <pwd.h>			/* Password file for shell name */

#ifndef PROVX1
#include <sys/file.h>			/* File access */
#endif

/* Some systems define these in include files, others don't... */

#ifndef R_OK
#define R_OK 4				/* For access */
#endif

#ifndef W_OK
#define W_OK 2
#endif

#ifdef PROVX1
#define MAXNAMLEN DIRSIZ		/* Max file name length */
#endif

#ifdef UXIII
#include <fcntl.h>
#define MAXNAMLEN DIRSIZ
#endif

#ifndef O_RDONLY
#define O_RDONLY 000
#endif

#ifndef MAXNAMLEN
#define MAXNAMLEN 14			/* If still not defined... */
#endif

#define MAXWLD 500			/* Maximum wildcard filenames */


/* Declarations */

FILE *fp[ZNFILS] = { 			/* File pointers */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static int pid;	    			/* pid of child fork */
static int fcount;			/* Number of files in wild group */
char *getenv(), *strcpy();		/* For finding home directory */
extern errno;				/* System error code */

static char *mtchs[MAXWLD],		/* Matches found for filename */
     **mtchptr;				/* Pointer to current match */

/*  Z O P E N I  --  Open an existing file for input. */

zopeni(n,name) int n; char *name; {
    debug(F111," zopeni",name,n);
    debug(F101,"  fp","",(int) fp[n]);
    if (chkfn(n) != 0) return(0);
    if (n == ZSTDIO) {			/* Standard input? */
	if (isatty(0)) {
	    fprintf(stderr,"?Terminal input not allowed\n");
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
    fp[n] = fopen(name,"w");		/* A real file */
    if (fp[n] == NULL) perror("zopeno");
    if (n == ZDFILE) setbuf(fp[n],NULL); /* Make debugging file unbuffered */
    debug(F101, " fp[n]", "", (int) fp[n]);
    return((fp[n] != NULL) ? 1 : 0);
}

/*  Z C L O S E  --  Close the given file.  */

zclose(n) int n; {
    if (chkfn(n) < 1) return(0);
    if ((fp[n] != stdout) && (fp[n] != stdin)) fclose(fp[n]);
    fp[n] = NULL;
    return(1);
}

/*  Z C H I N  --  Get a character from the input file.  */

zchin(n) int n; {
    int a;
    if (chkfn(n) < 1) return(-1);
    a = getc(fp[n]);
    return((a == EOF) ? -1 : a & 0377);
}

/*  Z S O U T  --  Write a string to the given file, buffered.  */

zsout(n,s) int n; char *s; {
    if (chkfn(n) < 1) return(-1);
    fprintf(fp[n],s);
    return(0);
}

/*  Z S O U T L  --  Write string to file, with line terminator, buffered  */

zsoutl(n,s) int n; char *s; {
    if (chkfn(n) < 1) return(-1);
    fprintf(fp[n],"%s\n",s);
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
    	putc(c,fp[n]);			/* Buffered for everything else */
	return(0);
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
zchki(name) char *name; {
    struct stat buf;
    int x;

    x = stat(name,&buf);
    if (x < 0) {
	debug(F111,"zchki stat fails",name,errno);
	return(-1);
    }
    x = buf.st_mode & S_IFMT;		/* Isolate file format field */
    if (x != S_IFREG) {
	debug(F111,"zchki skipping:",name,x);
	return(-2);
    }
    debug(F111,"zchki stat ok:",name,x);

    if ((x = access(name,R_OK)) < 0) { 	/* Is the file accessible? */
	debug(F111," access failed:",name,x); /* No */
    	return(-3);			
    } else {
	x = buf.st_size;
	debug(F111," access ok:",name,x); /* Yes */
	return( (x > -1) ? x : 0 );
    }
}

/*  Z C H K O  --  Check if output file can be created  */

/*
 Returns -1 if write permission for the file would be denied, 0 otherwise.
*/
zchko(name) char *name; {
    int i, x;
    char s[50], *sp;	

    sp = s;				/* Make a copy, get length */
    x = 0;
    while ((*sp++ = *name++) != '\0')
    	x++;
    if (x == 0) return(-1);		/* If no filename, fail. */

    debug(F101," length","",x);
    for (i = x; i > 0; i--)		/* Strip filename. */
	if (s[i-1] == '/') break;
    
    debug(F101," i","",i);
    if (i == 0)				/* If no path, use current directory */
    	strcpy(s,"./");			
    else				/* Otherwise, use given one. */
        s[i] = '\0';

    x = access(s,W_OK);			/* Check access of path. */
    if (x < 0) {
	debug(F111,"zchko access failed:",s,errno);
	return(-1);
    } else {
	debug(F111,"zchko access ok:",s,x);
	return(0);
    }
}

/*  Z D E L E T  --  Delete the named file.  */

zdelet(name) char *name; {
    unlink(name);
}


/*  Z R T O L  --  Convert remote filename into local form  */

/*  For UNIX, this means changing uppercase letters to lowercase.  */

zrtol(name,name2) char *name, *name2; {
    for ( ; *name != '\0'; name++) {
    	*name2++ = isupper(*name) ? tolower(*name) : *name;
    }
    *name2 = '\0';
}


/*  Z L T O R  --  Convert filename from local format to common form.   */

zltor(name,name2) char *name, *name2; {
    char work[100], *cp, *pp;
    int dc = 0;

    strcpy(work,name);
    for (cp = pp = work; *cp != '\0'; cp++) {	/* strip path name */
    	if (*cp == '/') {
	    pp = cp;
	    pp++;
	}
	else if (islower(*cp)) *cp = toupper(*cp); /* Uppercase letters */
	else if (*cp == '~') *cp = 'X';	/* Change tilde to 'X' */
	else if ((*cp == '.') && (++dc > 1)) *cp = 'X'; /* & extra dots */
    }
    cp = name2;				/* If nothing before dot, */
    if (*pp == '.') *cp++ = 'X';	/* insert 'X' */
    strcpy(cp,pp);
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

/*  Z X C M D -- Run a system command so its output can be read like a file */

zxcmd(comand) char *comand; {
    int pipes[2];
    if (pipe(pipes) != 0) return(0);	/* can't make pipe, fail */
    if ((pid = fork()) == 0) {		/* child */

/*#if BSD4*/			/* Code from Dave Tweten@AMES-NASA */
			/* readapted to use getpwuid to find login shell */
			/*   -- H. Fischer */
	char *shpath, *shname, *shptr;	/* to find desired shell */
	struct passwd *p;
	extern struct passwd * getpwuid();
	extern int getuid();
	char *defShel = "/bin/sh";	/* default shell */
/*#endif*/

	close(pipes[0]);		/* close input side of pipe */
	close(0);			/* close stdin */
	if (open("/dev/null",0) < 0) return(0);	/* replace input by null */

#ifndef UXIII
	dup2(pipes[1],1);		/* replace stdout & stderr */
	dup2(pipes[1],2);		/* by the pipe */
#else
	close(1);			/* simulate dup2 */
	if (dup(pipes[1]) != 1 )
	    conol("trouble duping stdout in routine zxcmd\n");
	close(2);			/* simulate dup2 */
	if (dup(pipes[1]) != 2 )
	    conol("trouble duping stderr in routine zxcmd\n");
#endif

	close(pipes[1]);		/* get rid of this copy of the pipe */

/**** 	shptr = shname = shpath = getenv("SHELL");  /* What shell? */
	p = getpwuid( getuid() );	/* get login data */
	if ( p == (struct passwd *) NULL || !*(p->pw_shell) ) shpath = defShel;
	  else shpath = p->pw_shell;
	shptr = shname = shpath;
	while (*shptr != '\0') if (*shptr++ == '/') shname = shptr;
	execl(shpath,shname,"-c",comand,0); /* Execute the command */

/****	execl("/bin/sh","sh","-c",comand,0); /* Execute the command */

	exit(0); }			/* just punt if it didn't work */

    close(pipes[1]);			/* don't need the output side */
    fp[ZIFILE] = fdopen(pipes[0],"r");	/* open a stream for it */
    return(1);
}

/*  Z C L O S F  - wait for the child fork to terminate and close the pipe. */

zclosf() {
    int wstat;
    fclose(fp[ZIFILE]);
    fp[ZIFILE] = NULL;
    while ((wstat = wait(0)) != pid && wstat != -1) ;
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

    bp = buf;
    while (*fn) {
	*bp++ = *fn++;
	len++;
    }
    *bp++ = '*';			/* Put a star on the end */
    *bp-- = '\0';

    n = zxpand(buf);			/* Expand the resulting wild name */
    
    while (n-- > 0) {			/* Find any existing name~d files */
	xp = *mtchptr++;
	xp += len;
	if (*xp == '~') {
	    t = atoi(xp+1);
	    if (t > d) d = t;		/* Get maximum d */
	}
    }
    sprintf(bp,"~%d",d+1);		/* Make and return name~(d+1) */
    *s = buf;
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

#define SSPACE 2000			/* size of string-generating buffer */
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
 head = splitpath(pat);
 if (*pat == '/')
 {
  scratch[0] = '/';
  sptr = scratch+1;
 }
 else
 {
  strcpy(scratch,"./");
  sptr = scratch+2;
 }					/* init buffer correctly */
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
#ifdef BSD4
 DIR *fd;
 struct direct *dirbuf;
#else
 int fd;
 struct direct dir_entry;
 struct direct *dirbuf = &dir_entry;
#endif
 struct stat statbuf;
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
  if (stat(sofar,&statbuf) == 0)	/* if current piece exists */
  {
      *endcur++ = '/';                  /* add slash to end */
      *endcur = '\0';			/* and end the string */
      traverse(pl -> fwd,sofar,endcur);
  }
  return;
 }
/* cont'd... */

/*...traverse, cont'd */

/* segment contains wildcards, have to search directory */
 *endcur = '\0';                        	/* end current string */
 if (stat(sofar,&statbuf) == -1) return;   	/* doesn't exist, forget it */
 if ((statbuf.st_mode & S_IFDIR) == 0) return;  /* not a directory, skip */
#ifdef BSD4
 if ((fd = opendir(sofar)) == NULL) return;  	/* can't open, forget it */
 while (dirbuf = readdir(fd))
#else
 if ((fd = open(sofar,O_RDONLY)) < 0) return;  	/* can't open, forget it */
 while ( read(fd,dirbuf,sizeof dir_entry) )
#endif
  if (dirbuf->d_ino != 0 && match(pl -> npart,dirbuf->d_name)) {
    char *eos;
    strcpy(endcur,dirbuf->d_name);
    eos = endcur + strlen(dirbuf->d_name);
    *eos = '/';                    /* end this segment */
    traverse(pl -> fwd,sofar,eos+1);
  }
#ifdef BSD4
 closedir(fd);
#else
 close(fd);
#endif
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
 if (strncmp(str,"./",2) == 0) str += 2;
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

match(pattern,string) char *pattern,*string; {
    char *psave,*ssave;			/* back up pointers for failure */
    psave = ssave = NULL;
    while (1) {
	for (; *pattern == *string; pattern++,string++)  /* skip first */
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
