char *ckzv = "OS-9 file support, 17 Jul 89";
char *ckzsys = " OS-9/68000";
 
/* c k 9 F I O  --  Kermit file system support for OS-9/68k systems */
 
/*
 Author: Peter Scholz,
 Ruhr University Bochum, Department for Analytical Chemistry,
 Federal Republic of Germany,   February 1987
 
 04/30/87 Robert Larson		Cleanup, merge with standard C-kermit
 04/07/89 Robert Larson		Update for ckermit 4f(77)
 07/16/89 Robert Larson		4f(85)

 adapted from Unix C-Kermit
 Author: Frank da Cruz (SY.FDC@CU20B),
 Columbia University Center for Computing Activities, January 1985.
 Copyright (C) 1985, Trustees of Columbia University in the City of New York.
 
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained.
*/
/* Includes */
 
#include "ckcker.h"		/* Kermit definitions */
#include "ckcdeb.h"		/* Typedefs, debug formats, etc */
#include <ctype.h>		/* Character types */
#include <stdio.h>		/* Standard i/o */
#include <dir.h>		/* Directory structure */
#include <direct.h>
#include <modes.h>
 
/* Definitions of some system commands */
 
char *DIRCMD = "dir ";		/* For directory listing */
char *DELCMD = "del ";		/* For file deletion */
char *TYPCMD = "list ";		/* For typing a file */
char *PWDCMD = "pd ";		/* For saying where I am */

char *SPACMD = "free ";
char *SPACM2 = "free ";		/* For space in specified directory */
 
char *WHOCMD = "procs ";	/* we have no who yet*/
 
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
   zkself()         -- Kill self, log out own job.
   zsattr(struc zattr *) -- Return attributes for file which is being sent.
 */
 
 
/* Some systems define these in include files, others don't... */
#define R_OK S_IREAD		/* For access */
#define W_OK S_IWRITE
#define O_RDONLY 000
 
#ifndef	MAXNAMLEN
#define MAXNAMLEN 28		/* If still not defined... */
#endif
 
#define MAXWLD 500
 
/* Declarations */
 
FILE *fp[ZNFILS] = {    /* File pointers */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL };
 
/* (PWP) external def. of things used in buffered file input and output */
extern CHAR zinbuffer[], zoutbuffer[];
extern CHAR *zinptr, *zoutptr;
extern int zincnt, zoutcnt;

static long iflen = -1;			/* Input file length */
static long oflen = -1;			/* Output file length */

static int pid;				/* pid of child fork */
static int fcount;			/* Number of files in wild group */
static char nambuf[MAXNAMLEN+2];	/* Buffer for a filename */
char *malloc(), *getenv(), *strcpy();	/* System functions */
extern errno;				/* System error code */
 
static char *mtchs[MAXWLD],		/* Matches found for filename */
     **mtchptr;				/* Pointer to current match */
 
/*  Z K S E L F  --  Kill Self: log out own job, if possible.  */
 
zkself() {				/* For "bye", but no guarantee! */
    return(kill(getpid(),0));
}
 
#define MAXPATH 128

char *zgtdir() {
    /* I'm sure this can be done better... */
    static char cwd[MAXPATH];
    char backpath[MAXPATH];
    char *bpp = backpath, *path = cwd;
    DIR *dirp;
    struct direct *dp;
    long inode, inode2;
    char dots[MAXPATH];

    if((dirp = opendir(".")) == NULL || readdir(dirp) == NULL ||
		(dp = readdir(dirp)) == NULL) {
	closedir(dirp);
	return (char *)NULL;
    }
    inode = dp->d_addr;
    *path++ = '/';
    _gs_devn(dirp->dd_fd, path);
    path += strlen(path);
    closedir(dirp);
    strcpy(dots, "..");
    for(;;) {
	if((dirp = opendir(dots)) == NULL || readdir(dirp) == NULL ||
		    (dp = readdir(dirp)) == NULL) {
	    closedir(dirp);
	    return (char *)NULL;
	}
	inode2 = dp->d_addr;
	if(inode == inode2) break;	/* .. and . are same, we are done */
	do {
	    if((dp = readdir(dirp)) == NULL) {
		closedir(dirp);
		return (char *)NULL;
	    }
	} while(dp->d_addr != inode);
	*bpp++ = '/';
	strcpy(bpp, dp->d_name);
	bpp += strlen(bpp);
	closedir(dirp);
	inode = inode2;
	strcat(dots, "/..");
    }
    while(bpp > backpath) {
	*bpp = '\0';
	while(*--bpp != '/') {}
	strcpy(path, bpp);
	path += strlen(path);
    }
    return cwd;
}


/*  Z O P E N I  --  Open an existing file for input. */
 
zopeni(n,name) int n; char *name; {
    debug(F111," zopeni",name,n);
    debug(F101,"  fp","",(int) fp[n]);
    if (chkfn(n) != 0) return(0);
    zincnt = 0;				/* Reset input buffer */
    if (n == ZSYSFN) {   		/* Input from a system function? */
        debug(F110," invoking zxcmd",name,0);
	*nambuf = '\0';			/* No file name this time... */
	return(zxcmd(name));		/* Try to fork the command */
    }
    if (n == ZSTDIO) {   /* Standard input? */
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
	zoutcnt = 0;
	zoutptr = zoutbuffer;
	return(1);
    }
    fp[n] = fopen(name,"w");  /* A real file, try to open */
    if (fp[n] == NULL) {
        perror("zopeno can't open");
    } else {
	/* chown(name, getuid(), getgid());     In case set[gu]id */
        if (n == ZDFILE) setbuf(fp[n],NULL); /* Debugging file unbuffered */
    }
    zoutcnt = 0;		/* (PWP) reset output buffer */
    zoutptr = zoutbuffer;
    debug(F101, " fp[n]", "", (int) fp[n]);
    return((fp[n] != NULL) ? 1 : 0);
}
 
/*  Z C L O S E  --  Close the given file.  */
 
/*  Returns 0 if arg out of range, 1 if successful, -1 if close failed.  */
 
zclose(n) int n; {
    int x, x2;
    if (chkfn(n) < 1) return(0); /* Check range of n */

    if ((n == ZOFILE) && (zoutcnt > 0))	/* (PWP) output leftovers */
	x2 = zoutdump();

    if ((n == ZIFILE) && fp[ZSYSFN]) { /* If system function */
	x = zclosf();   /* do it specially */
    } else {
	if ((fp[n] != stdout) && (fp[n] != stdin)) x = fclose(fp[n]);
	fp[n] = NULL;
    }
    iflen = -1;				/* Invalidate file length */
    return ((x == EOF) || (x2 < 0)) ? -1 : 1;
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
 
/*  Z I N F I L L  --  Get a character from the input file.
 * (PWP) (re)fill the buffered input buffer with data.  All file input
 * should go through this routine, usually by calling the zminchar()
 * macro (in ckcker.h).
 */

zinfill() {
    zincnt = fread(zinbuffer, sizeof (char), INBUFSIZE, fp[ZIFILE]);
    if (zincnt == 0) return (-1);	/* end of file */
    zinptr = zinbuffer;	   /* set pointer to beginning, (== &zinbuffer[0]) */
    zincnt--;				/* one less char in buffer */
    return((int)(*zinptr++) & 0377);	/* because we return the first */
}

/*  Z S O U T  --  Write a string to the given file, buffered.  */
 
zsout(n,s) int n; char *s; {
    if (chkfn(n) < 1) return(-1);
    fputs(s,fp[n]);
    return(0);
}
 
/*  Z S O U T L  --  Write string to file, with line terminator, buffered  */
 
zsoutl(n,s) int n; char *s; {
    /* if (chkfn(n) < 1) return(-1); */
    fputs(s,fp[n]);
    fputs("\n",fp[n]);
    return(0);
}
 
/*  Z S O U T X  --  Write x characters to file, unbuffered.  */
 
zsoutx(n,s,x) int n, x; char *s; {
    /* if (chkfn(n) < 1) return(-1); */
    return(write(fileno(fp[n]),s,x));
}
 
 
/*  Z C H O U T  --  Add a character to the given file.  */
 
/*  Should return 0 or greater on success, -1 on failure (e.g. disk full)  */
 
zchout(n,c) register int n; char c; {
    /* if (chkfn(n) < 1) return(-1); */
    if (n == ZSFILE)
	return(write(fileno(fp[n]),&c,1)); /* Use unbuffered for session log */
    else {    /* Buffered for everything else */
	if (putc(c,fp[n]) == EOF) /* If true, maybe there was an error */
	    return(ferror(fp[n])); /* Check to make sure */
	else return(0);   /* There was no error. */
    }
}
 
/* (PWP) buffered character output routine to speed up file IO */
zoutdump()
{
    if (zoutcnt <= 0) return (0); /* nothing to output */

    if (fwrite (zoutbuffer, 1, zoutcnt, fp[ZOFILE])) {
	zoutcnt = 0;		/* reset output buffer */
	zoutptr = zoutbuffer;
	return(0);		/* things worked OK */
    } else {
	return(ferror(fp[ZOFILE])?-1:0); /* Check to make sure */
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
    struct fildes buf;
    int x;   
 
    if (access(name,0) < 0) {
	if(access(name,S_IFDIR)>=0) {
	    debug(F111,"zchki skipping:",name,errno);
	    return(-2);
	}
	debug(F111,"zchki can't access",name,errno);
	return(-1);
    }
    debug(F111,"zchki stat ok:",name,x);
 
    if ((x = access(name,R_OK)) < 0) {  /* Is the file accessible? */
	debug(F111," access failed:",name,x); /* No */
	return(-3);   
    }
    if((x = open(name,S_IREAD)) < 0) {
	debug(F111,"zchki can't open:",name,errno);
	return(-2);
    }
    iflen = _gs_size(x);		/* remember size */
    strncpy(nambuf,name,MAXNAMLEN);	/* and name globally */
    close(x);
    debug(F111," access ok:",name,(int) iflen); /* Yes */
    return( (iflen > -1) ? iflen : 0 );
}
 
/*  Z C H K O  --  Check if output file can be created  */
 
/*
 Returns -1 if write permission for the file would be denied, 0 otherwise.
*/
zchko(name) char *name; {
    int i, x;
    char s[50], *sp; 
 
    if(access(name,S_IFDIR)>=0) return -1;	/* it's a directory */
    sp = s;    /* Make a copy, get length */
    x = 0;
    while ((*sp++ = *name++) != '\0')
	x++;
    if (x == 0) return(-1);		/* If no filename, fail. */
 
    debug(F101," length","",x);
    for (i = x; i > 0; i--) {		/* Strip filename. */
	if(!isalnum(s[i-1])) {
	    switch(s[i-1]) {
	    	case '.':
		case '$':
		case '_': continue;
	   	case '/': break;
	    	default: return -1;	/* bad character */
	    }
	    break;
	}
    }
    debug(F101," i","",i);
    if (i == 0)				/* If no path, use current directory */
	strcpy(s,".");   
    else {				/* Otherwise, use given one. */
        if(i==x) return -1;		/* no filename! */
        s[--i] = '\0';
    }
    x = access(s,S_IFDIR|S_IWRITE);	/* Check access of path. */
    if (x < 0) {
    fprintf(stderr,"access failed %s   errno:%d\n",s,errno);
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
 
/*  For OS9, this means changing uppercase letters to lowercase.  */

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
    for (cp = name; *cp != '\0'; cp++) { /* strip path name */
	if (*cp == '/') {
	    dc = 0;
	    pp = work;
	}
	else if (islower(*cp)) *pp++ = toupper(*cp); /* Uppercase letters */
	else if ((*cp == '.') && (++dc > 1)) *pp++ = 'X'; /* & extra dots */
	else *pp++ = *cp;
    }
    *pp = '\0';    /* Tie it off. */
    cp = name2;    /* If nothing before dot, */
    if (*work == '.') *cp++ = 'X'; /* insert 'X' */
    strcpy(cp,work);
    debug(F110," name2",name2,0);
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
    int i,stdio[3],os9fork();
    if ((pipes[0]=open("/pipe",_READ))<0) return(0);
    if((pipes[1]=dup(pipes[0]))<0) {
	close(pipes[0]);
        return(0);
    }
    for(i=0;i<3;i++)
	stdio[i] = dup(i);
    close(0);   /* close stdin */
    if (open("/nil",_READ) < 0) return(0); /* replace input by null */
 
    close(1);   /* simulate dup2 */
    if (dup(pipes[1]) != 1 )
	conol("trouble duping stdout in routine zxcmd\n");
    close(2);   /* simulate dup2 */
    if (dup(pipes[1]) != 2 )
	conol("trouble duping stderr in routine zxcmd\n");
 
    pid = os9fork("shell",strlen(comand),comand,0,0,0,0);

    fp[ZIFILE] = fdopen(pipes[0],"r"); /* open a stream for it */
    close(pipes[1]);   /* don't need the output side */
    for(i=0;i<3;i++) {
	close(i);
	dup(stdio[i]);
    }
    fp[ZSYSFN] = fp[ZIFILE];  /* Remember. */
    return(1);
}
 
/*  Z C L O S F  - wait for the child fork to terminate and close the pipe. */
 
zclosf() {
    int wstat;
    fclose(fp[ZIFILE]);
    fp[ZIFILE] = fp[ZSYSFN] = NULL;
    while ((wstat = wait(0)) != pid && wstat != -1) ;
    return(1);
}
 
/*  Z X P A N D  --  Expand a wildcard string into an array of strings  */
/*
  Returns the number of files that match fn1, with data structures set up
  so that first file (if any) will be returned by the next znext() call.
*/
zxpand(fn) char *fn; {
    fcount = fgen(fn,mtchs,MAXWLD); /* Look up the file. */
    if (fcount > 0) {
	mtchptr = mtchs;  /* Save pointer for next. */
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
    static char buf[256];
    char *bp, *xp;
    int len = 0, n = 0, d = 0, t, i, power = 1;
#ifdef MAXNAMLEN
    int max = MAXNAMLEN;
#else
    int max = 14;
#endif
    bp = buf;
    while (*fn) {   /* Copy name into buf */
	*bp++ = *fn++;
	len++;
    }
    if (len > max-2) {    /* Don't let it get too long */
	bp = buf + max-2;
	len = max - 2;
    }
 
    for (i = 1; i < 4; i++) {  /* Try up to 999 times */
	power *= 10;
	*bp++ = '*';   /* Put a star on the end */
	*bp-- = '\0';
 
	n = zxpand(buf);  /* Expand the resulting wild name */
 
	while (n-- > 0) {  /* Find any existing name_d files */
	    xp = *mtchptr++;
	    xp += len;
	    if (*xp == '_') {
		t = atoi(xp+1);
		if (t > d) d = t; /* Get maximum d */
	    }
	}
	if (d < power-1) {
	    sprintf(bp,"_%d",d+1); /* Make name_(d+1) */
	    *s = buf;
	    return;
	}
	bp--; len--;
    }
/* If we ever get here, we'll overwrite the xxx~100 file... */
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
debug(F110,"before calling zfcdat",nambuf,0);
    if (nambuf) {
	xx->date.val = zfcdat(nambuf);	/* File creation date */
	xx->date.len = strlen(xx->date.val);
    } else {
	xx->date.len = 0;
	xx->date.val = "";
    }
debug(F111,"attr date",xx->date.val,xx->date.len);
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
    xx->systemid.len = 2;		/* System ID */
    xx->systemid.val = "UD";
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
    int fp;
    struct fildes fd;
    static char datbuf[9];

    if((fp = open(name, 0)) < 0) return "";
    if(_gs_gfd(fp, &fd, sizeof fd) >= 0) {
    	sprintf(datbuf,"%4d%02d%02d",fd.fd_dcr[0],fd.fd_dcr[1],fd.fd_dcr[2]);
    } else datbuf[0] = '\0';
    close(fp);
    debug(F111,"zcfdat",datbuf,strlen(datbuf));
    return datbuf;
}

/* Directory Functions for Unix, written by Jeff Damens, CUCCA, 1984. */
 
/*
 * The path structure is used to represent the name to match.
 * Each slash-separated segment of the name is kept in one
 * such structure, and they are linked together, to make
 * traversing the name easier.
 */
 
struct path {
              char npart[MAXNAMLEN]; /* name part of path segment */
              struct path *fwd;  /* forward ptr */
            };
 
#define SSPACE 2000   /* size of string-generating buffer */
static char sspace[SSPACE];             /* buffer to generate names in */
static char *freeptr,**resptr;          /* copies of caller's arguments */
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
    while (*p != '\0') {
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
 *     array, -1 is returned.
 * By: Jeff Damens, CUCCA, 1984.
 */
 
fgen(pat,resarry,len)
char *pat,*resarry[];
int len;
{
    struct path *head;
    char scratch[100+MAXNAMLEN],*sptr;
    head = splitpath(pat);
    if (*pat == '/') {
	scratch[0] = '/';
	sptr = scratch+1;
	*sptr = '\0';
    } else {
	strcpy(scratch,"./");
	sptr = scratch+2;
    }     /* init buffer correctly */
    numfnd = 0;                            /* none found yet */
    freeptr = sspace;   /* this is where matches are copied */
    resptr = resarry;   /* static copies of these so*/
    remlen = len;    /* recursive calls can alter them */
    traverse(head,scratch,sptr);  /* go walk the directory tree */
    for (; head != NULL; head = head -> fwd)
	free(head);    /* return the path segments */
    return(numfnd);   /* and return the number of matches */
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
 *   pointer that points to what we've traversed so far (this
 *   can be initialized to "/" to start the search at the root
 *   directory, or to "./" to start the search at the current
 *   directory), and a string pointer to the end of the string
 *   in the previous argument.
 * Returns: nothing.
 */
traverse(pl,sofar,endcur)
struct path *pl;
register char *sofar,*endcur;
{
    DIR *fd, *opendir();
    struct direct *dirbuf;
    debug(F110,"traverse ",sofar,0);
    if (pl == NULL) {
	*--endcur = '\0';		/* end string, overwrite trailing / */
	addresult(sofar);
	return;
    }
    if (!iswild(pl -> npart)) {
	strcpy(endcur,pl -> npart);
	endcur += strlen(endcur);
	if (access(sofar,S_IFDIR) == 0) { /* if current piece exists & dir */
	    *endcur++ = '/';		/* add slash to end */
	    *endcur = '\0';		/* and end the string */
	    traverse(pl -> fwd,sofar,endcur);
	} else if(access(sofar,0) == 0 && pl->fwd==NULL) {
	    addresult(sofar);
	}
	return;
    }
 
/* segment contains wildcards, have to search directory */
    *--endcur = '\0';			/* end current string */
    if (access(sofar,S_IFDIR) < 0) {
    	debug(F111,"traverse can't access directory",sofar,errno);
	return;	/* doesn't exist, forget it */
    }
    if ((fd = opendir(sofar)) == NULL) {
    	debug(F111,"traverse can't open directory",sofar,errno);
    	return;	/* can't open, forget it */
    }
    *endcur++ = '/';
    while (dirbuf = readdir(fd)) {
	if (dirbuf->d_addr != 0) {
	    strncpy(endcur,dirbuf->d_name,MAXNAMLEN); /* Get a null terminated copy!!! */
	    if(match(pl -> npart,endcur)) {
	    	char *eos;
	    	eos = endcur + strlen(endcur);
	    	*eos++ = '/';			/* end this segment */
	    	*eos = '\0';
	    	traverse(pl -> fwd,sofar,eos);
	    }
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
register char *str;
{
    register int l;
    if (strncmp(str,"./",2) == 0) str += 2;
    if (--remlen < 0) {
	numfnd = -1;
	return;
    }
    l = strlen(str) + 1;   /* size this will take up */
    if ((freeptr + l) > &sspace[SSPACE]) {
	numfnd = -1;   /* do not record if not enough space */
	return;
    }
    strcpy(freeptr,str);
    *resptr++ = freeptr;
    freeptr += l;
    numfnd++;
}
 
iswild(str)
register char *str;
{
    register char c;
    while ((c = *str++) != '\0')
	if (c == '*' || c == '?') return(1);
    return(0);
}
 
/*
 * match:
 *  pattern matcher.  Takes a string and a pattern possibly containing
 *  the wildcard characters '*' and '?'.  Returns true if the pattern
 *  matches the string, false otherwise.
 * Input: a string and a wildcard pattern.
 * Returns: 1 if match, 0 if no match.
 */
 
match(pattern,string) char *pattern,*string; {
    int i;
    i = _cmpnam(string,pattern,strlen(pattern))==0;
    debug(F111,"Match ",string,i);
    return i;
}

/* emulate unix perror function */

perror(string)
char *string;
{
    extern int errno;
    fprintf(stderr,"%s ERRNO: %d\n",string,errno);
}

#ifdef DTILDE
char *
tilde_expand(dirname)
register char *dirname;
{
    static char *home = NULL;
    static char temp[MAXNAMLEN];

    debug(F111,"tilde_expand dirname", dirname, dirname[0]);
    if(*dirname++ != '~' || (*dirname != '\0' && *dirname != '/')) return --dirname;
    if(home == NULL && (home = getenv("HOME")) == NULL) return --dirname;
    if(*dirname == '\0') return home;
    strcpy(temp, home);
    strcat(temp, dirname);
    return temp;
}
#endif
