/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1986 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors.                                                    */
/* | o  | ||    Dave Baker     Ed Burnette  Stan Chow    Jay Denebeim        */
/* |  . |//     Gordon Keener  Jack Rouse   John Toebes  Doug Walker         */
/* ======          BBS:(919)-471-6436      VOICE:(919)-469-4210              */ 
/*                                                                           */
/* Contributed to Columbia University for inclusion in C-Kermit.             */
/* Permission is granted to any individual or institution to use, copy, or   */
/* redistribute this software so long as it is not sold for profit, provided */
/* this copyright notice is retained.                                        */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*  C K I U T L -- July 17, 1986
 * Utility functions for C-Kermit on the Amiga
 */
 
/*
 Author: Jack Rouse, The Software Distillery

 The file status routines assume all file protection modes are real, instead
 of just delete protection on files and write protection on disks.
*/

#include <stdio.h>
#undef NULL
#include "exec/types.h"
#include "exec/exec.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#define fh_Interact fh_Port
#define fh_Process  fh_Type
#if LAT310 | AZTEC_C
#include "fcntl.h"
#include "signal.h"
#else
#include "lattice/ios1.h"	/* defines ufbs structure */
#endif

/* external routine definitions */
APTR AllocMem();
LONG AllocSignal();
struct IORequest *CheckIO();
VOID CloseDevice();
VOID CloseLibrary();
LONG DoIO();
struct MsgPort *FindPort();
struct Task *FindTask();
VOID FreeMem();
VOID FreeSignal();
struct Message *GetMsg();
LONG OpenDevice();
struct Library *OpenLibrary();
VOID PutMsg();
VOID ReplyMsg();
VOID SendIO();
LONG SetSignal();
VOID Signal();
LONG Wait();
LONG WaitIO();
struct Message *WaitPort();

struct IORequest *CreateExtIO();
struct MsgPort *CreatePort();

VOID Close();
LONG DeleteFile();
LONG Execute();
BPTR Input(), Output();
LONG IoErr();
LONG IsInteractive();
BPTR Lock();
BPTR Open();
LONG Read();
VOID UnLock();   
LONG WaitForChar();

/* portable library */
char *malloc();

#ifdef AZTEC_C
/* translate Unix file handle (0, 1, or 2) to AmigaDOS file handle */
#define DOSFH(n) (_devtab[n].fd)
/* translate Unix file handle (0, 1, or 2) to Aztec file handle */
#define FILENO(n) (n)
#else
#ifdef LAT310
/* translate Unix file handle (0, 1, or 2) to AmigaDOS file handle */
#define DOSFH(n) fileno(&_iob[n])
/* translate Unix file handle (0, 1, or 2) to Lattice file handle */
#define FILENO(n) fileno(&_iob[n])
#else
/* Lattice runtime externals */
extern struct UFB _ufbs[];
extern int Enable_Abort;
#define DOSFH(n) (_ufbs[n].ufbfh)
#define FILENO(n) (n)
#endif
#endif

/* Amiga Kermit externals (defined in ckitio.c) */
extern struct Process *CurProc;
extern struct CommandLineInterface *CurCLI;

/*
 * CreatePacket -- allocate and set up a AmigaDOS packet
 */
struct DosPacket *CreatePacket()
{
	register struct StandardPacket *sp;

	sp = (struct StandardPacket *)
	     AllocMem((LONG)sizeof(struct StandardPacket),
		      (LONG)MEMF_PUBLIC|MEMF_CLEAR);
	if (sp == NULL) return(NULL);
	sp->sp_Pkt.dp_Link = &sp->sp_Msg;
	sp->sp_Msg.mn_Node.ln_Type = NT_MESSAGE;
	sp->sp_Msg.mn_Node.ln_Name = (char *)&sp->sp_Pkt;
	sp->sp_Msg.mn_Length = sizeof(struct DosPacket);
	return(&sp->sp_Pkt);
}

/*
 * DeletePacket -- deallocate packet from CreatePacket()
 */
VOID DeletePacket(pkt)
struct DosPacket *pkt;
{
	FreeMem(pkt->dp_Link, (LONG)sizeof(struct StandardPacket));
}

/*
 * system(cmd) -- execute a command
 *    provides no sensible return value
 */
system(cmd)
char *cmd;
{
	BPTR fh;

	fflush(stdout);
	if (cmd == NULL || *cmd == '\0')
	{
		fh = Open("CON:0/0/640/200/Kermit CLI", (LONG)MODE_NEWFILE);
		if (fh)
		{
			Execute("", fh, (BPTR)NULL);

			/* fix 'endcli' bug */
			((struct FileHandle *)BADDR(fh))->fh_Pos = 0;

			Close(fh);
		}
	}
	else
		Execute(cmd, (BPTR)NULL, DOSFH(1));
}

/*
 * getcwd -- get current working directory text
 */
char *getcwd(buf, len)
register char *buf;
int len;
{
	register UBYTE *dirname;

	if (CurCLI == NULL) return(NULL);
	dirname = (UBYTE *)BADDR(CurCLI->cli_SetName);
	if (len < *dirname + 1) return(NULL);
	strncpy(buf, dirname + 1, *dirname);
	buf[*dirname] = 0;
	return(buf);
}

/*
 * update the current directory name in the CLI process structure
 *
 * This version generates the new name from the current name and
 * the specified path, something like the CD command does.
 * A much better version could be written using Parent(), Examine(),
 * and VolInfo to reconstitute a name from a lock, but Parent() doesn't
 * work with RAM: in V1.1.  The current implementation, like the CD
 * command, has difficulty with backing up the directory tree.  For example
 * { chdir("c:"); chdir("/"); } results in a dir name of "c:/".  However,
 * this version shouldn't get as far out of sync as the CD command.  (The
 * same sequence with the CD command puts you in the parent dir of "c:" but
 * with a directory name of "c:".)
 */
static void update_dirname(name)
register char *name;
{
	register UBYTE *dirname;		/* DOS directory name BSTR */
	char buf[100];				/* about same size as DOS */
	register char *tail;
	char *strrchr();

	/* locate the DOS copy of the directory name */
	if (CurCLI == NULL) return;
	dirname = (UBYTE *)BADDR(CurCLI->cli_SetName);

	/* if the name is anchored (like "DF1:") simply replace the name */
	if (strrchr(name, ':') != NULL)
	{
		*dirname = strlen(name);
		strncpy(&dirname[1], name, *dirname);
		return;
	}

	/* name is relative to current directory, copy name to work with */
	strncpy(buf, &dirname[1], *dirname);
	tail = &buf[*dirname];

	/* traverse the path in the name */
	while (*name)
	{
		/* go to parent dir? */
		if (*name == '/')
		{	/* remove a component from the directory path */

			/* advance past parent slash */
			++name;

			/* if at colon, can't back up */
			if (tail[-1] == ':')
			{
				*tail++ = '/';
				continue;
			}

			/* if at slash, see if name given */
			if (tail[-1] == '/')
			{
				/* if no name, can't back up */
				if (tail[-2] == '/' || tail[-2] == ':')
				{
					*tail++ = '/';
					continue;
				}

				/* remove trailing slash */
				--tail;
			}

			/* remove remainder of component */
			while (tail[-1] != '/' && tail[-1] != ':')
				--tail;
		}
		else
		{	/* add component to directory path */
			/* add slash if necessary to separate name */
			if (tail[-1] != ':' && tail[-1] != '/')
				*tail++ = '/';

			/* add component name */
			while (*name && *name != '/')
				*tail++ = *name++;

			/* add trailing slash if specified */
			if (*name == '/')
				*tail++ = *name++;
		}
	}

	/* set BSTR to derived name */
	*dirname = tail - buf;
	strncpy(&dirname[1], buf, *dirname);
}

/*
 * change current directory
 */
int chdir(name)
char *name;
{
	BPTR lock;
	BPTR oldlock;
	struct FileInfoBlock *fib;

	/* ignore chdir("") */
	if (*name == 0) return(0);

	/* try to look at the object */ 
	lock = Lock(name, (LONG)ACCESS_READ);
	if (lock == NULL) return(-1);

	/* make sure the file is a directory */
	fib = (struct FileInfoBlock *)malloc(sizeof(*fib));
	if (fib == NULL || !Examine(lock, fib) || fib->fib_DirEntryType <= 0)
	{
		if (fib) free(fib);
		UnLock(lock);
		return(-1);
	}

	/* don't need file info any more */
	free(fib);

	/* change the current directory */
	oldlock = CurrentDir(lock);

	/* update the DOS copy of the directory name */
	update_dirname(name);

	/* unlock the previous current directory */
	if (oldlock) UnLock(oldlock);

	return(0);
}

#if !LAT310 & !AZTEC_C
/*
 * print an error message with explanation
 * (no explanation currently)
 */
int perror(str)
char *str;
{
	extern int errno;
	LONG oserr = IoErr();

	printf("%s: errno=%d, oserr=%ld\n", str, errno, oserr);
}
#endif

/*
 * isatty(fd) -- determine if given file is a (virtual) terminal
 *    only works for fd=0, 1, or 2 (stdin, stdout, and stderr resp.)
 */ 
int isatty(fd)
int fd;
{
	return(IsInteractive(DOSFH(fd)) != 0);
}

/*
 * readstat -- determine file's read status
 *    returns -3 if file read protected
 *    returns -2 if file is a directory
 *    returns -1 if file doesn't exist
 *    returns file size otherwise
 */
long readstat(name)
char *name;
{
	BPTR lock;
	struct FileInfoBlock *fib;
	long size;

	/* locate the file */ 
	if ((lock = Lock(name, (LONG)ACCESS_READ)) == NULL) return(-1);

	/* allocate a file info block */
	if ((fib = (struct FileInfoBlock *)malloc(sizeof(*fib))) == NULL)
	{	size = -1; goto exit; }

	/* make sure it's not a directory */
	if (!Examine(lock, fib) || fib->fib_DirEntryType >= 0)
	{	size = -2; goto exit; }

	/* make sure it's readable */
	if (fib->fib_Protection & FIBF_READ)
	{	size = -3; goto exit; }

	size = fib->fib_Size;
exit:
	if (fib) free(fib);
	UnLock(lock);
	return(size);
}

/*
 * writestat -- determines file's write status
 *    returns 0 if file should be writable, -1 otherwise
 *
 * The following logic is used to determine if a file is writable:
 *    1.  If the file exists, it must not be write or delete protected.
 *        (Delete protection subsumes write protection for overwriting.)
 *    2.  The parent directory must not be write protected ???
 *    3.  The volume that the parent directory resides on must be
 *        validated and not write protected.
 */
int writestat(name)
register char *name;
{
	register char *p;
	char *lastslash;
	char path[100];
	BPTR lock;
	struct FileInfoBlock *fib;
	struct InfoData *id = NULL;
	int rc = -1;

	/* allocate a FileInfoBlock */
	fib = (struct FileInfoBlock *)malloc(sizeof(*fib));
	if (fib == NULL) goto exit;

	/* see if the file exists */
	if ((lock = Lock(name, (LONG)ACCESS_READ)) != NULL)
	{
		/* make sure it's not a directory or write protected */
		if (!Examine(lock, fib) ||
		    fib->fib_DirEntryType >= 0 ||
		    (fib->fib_Protection & (FIBF_WRITE|FIBF_DELETE)) )
			goto exit;
		UnLock(lock);
	}

	/* strip path from name */
	lastslash = NULL;
	for (p = path; *p = *name; ++p, ++name)
		if (*p == '/' || *p == ':')
			lastslash = p;

	/* make sure the path exists */
	if (lastslash)
	{
		lastslash[1] = 0;
		lock = Lock(path, (LONG)ACCESS_READ);
	}
	else
		lock = (CurProc->pr_CurrentDir)
		       ? DupLock(CurProc->pr_CurrentDir)
		       : NULL;

	/* make sure it is a directory that is not write protected */
	if (lock == NULL || !Examine(lock, fib) ||
	    fib->fib_DirEntryType <= 0 ||
	    (fib->fib_Protection & FIBF_WRITE) )
		goto exit;

	/* get device info */	
	if ((id = (struct InfoData *)malloc(sizeof(*id))) == NULL) goto exit;

	/* make sure the disk is writeable */
	if (Info(lock, id) && id->id_DiskState == ID_VALIDATED)
		rc = 0;

	/* clean up */
exit:
	if (id) free(id);
	if (fib) free(fib);
	if (lock) UnLock(lock);
    	return(rc);
}

/*
 * pipeopen -- execute command to read output like a file
 */
#define PIPEHOLDER "RAM:Pipe-Holder"
FILE *pipeopen(command)
char *command;
{
	BPTR fh;

	/* create holder file */
	fh = Open(PIPEHOLDER, (LONG)MODE_NEWFILE);
	if (fh == NULL) return(0);

	/* execute the command */
	Execute(command, (BPTR)NULL, fh);

	/* close the holder file */
	Close(fh);

	/* reopen it for input */
	return(fopen(PIPEHOLDER, "r"));
}
 
/*
 * pipeclose -- clean up after pipe open
 */
pipeclose(f)
FILE *f;
{
	fclose(f);
	DeleteFile(PIPEHOLDER);
}

/*
 * existobj -- return true if file system object exists
 */
int existobj(name)
char *name;
{
	BPTR lock;

	if (*name == 0) return(CurProc->pr_CurrentDir != NULL);
	if ((lock = Lock(name, (LONG)ACCESS_READ)) == NULL)
		return(0);
	UnLock(lock);
	return(1);
}

/* opendir handle structure */
struct DirHandle {
	struct FileInfoBlock fib;
	BPTR lock;
};

/*
 * attempt to open a directory, fill in a handle structure
 */
struct DirHandle *opendir(name)
char *name;
{
	BPTR lock;
	struct DirHandle *dh;

	/* get lock on name ("" is current dir) */
	lock = (*name != 0)		? Lock(name, (LONG)ACCESS_READ) :
	       (CurProc->pr_CurrentDir) ? DupLock(CurProc->pr_CurrentDir)
					: NULL;
	if (lock == NULL) return(NULL);
	dh = (struct DirHandle *)malloc(sizeof(*dh));
	if (dh == NULL)
	{
		UnLock(lock);
		return(NULL);
	}
	if (!Examine(lock, &dh->fib) || dh->fib.fib_DirEntryType <= 0)
	{
		UnLock(lock);
		free(dh);
		return(NULL);
	}
	/* pass it into the caller's care */
	dh->lock = lock;
	return(dh);
}

/*
 * return name for next entry in dir
 */
char *readdir(dh)
struct DirHandle *dh;
{
	return(ExNext(dh->lock, &dh->fib) ? dh->fib.fib_FileName : NULL);
}

/*
 * finish accessing a directory
 */
VOID closedir(dh)
struct DirHandle *dh;
{
	UnLock(dh->lock);
	free(dh);
}
