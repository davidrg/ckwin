/* ckuuid.c, Test program for C-Kermit's uid-managing code. */
/* priv_*() routines by Kristoffer Ericksson (ske) */
/* this program by Frank da Cruz (fdc), 5-26-90 */
/* Modified by Dean Long, 5-28-90 */
/* Modified ske, fdc, 6-10-90 */
/* Modified to check access() and state conclusions, fdc, 12-29-92 */
/* 4.4BSD semantics added by John Kohl <jtk@kolvir.blrc.ma.us> 20 Sep 94 */

/*
INSTRUCTIONS

Compile and load the program in each of the following ways:

   $ cc -DANYBSD -DSAVEDUID -o ckuuid1 ckuuid.c            (1)
   $ cc -DANYBSD -o ckuuid2 ckuuid.c                       (2)
   $ cc -DANYBSD -DNOSETREU -o ckuuid3 ckuuid.c            (3)
   $ cc -DANYBSD -DSETEUID -DNOSETREU -o ckuuid5 ckuuid.c  (4)
   $ cc -o ckuuid4 ckuuid.c                                (5)

(1) is for Berkeley-based systems that have setregid() and setreuid() and that
    have the saved-original-effective-uid feature, similar to AT&T System V.

(2) is for Berkeley-based systems that have setregid and setreuid, but do not
    have the saved-original-effective-uid feature.

(3) is for Berkeley-based systems that don't have setregid() and setreuid().

(4) is for BSD4.4 and others that have seteuid()/setegid() and
    saved-original-effective-uid feature.

(5) is for all others, including all AT&T-based versions, Xenix, etc.

After building the program, run it to make sure that the uid's don't change
(they shouldn't if the program is not setuid'd).

Now make the program setuid and setgid to someone else, e.g.:

   $ su
   Password: xxxx
   su% chown uucp.uucp ckuuid2
   su% chmod ug+s ckuuid2
   su% exit

and then run it, recording the results.  Give the name of a directory (not a
regular file) that you should have write-access to on the command line, e.g.:

   $ script
   $ who am i
   $ ls -lg ./ckuuid2
   $ ./ckuuid2 .
   $ exit

Read the output and make sure that the uids and gids can be changed back
and forth.

In steps 2, 4, and 5, check to see if access() worked correctly when you
have privs turned off.  If it doesn't, try recompiling with -DSW_ACC_ID
(swap IDs for access()) and repeat the above procedure.

Please report the results (mail the typescript file) back to me,
fdc@columbia.edu, letting me know exactly what kind of machine you
have, and which version of UNIX.
*/ 

/* Includes */

#include <stdio.h>
#include <sys/stat.h>
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif /* S_ISDIR */

#ifndef UID_T				/* Define these on the CC command */
#define UID_T int			/* line if your compiler complains */
#endif /* UID_T */			/* about mismatched types, e.g. */
					/* -DUID_T=uid_t -DGID_T=gid_t */
#ifndef GID_T
#define GID_T int
#endif /* GID_T */

/* setuid package variables */

/* User and group IDs */

static int realuid = (UID_T) -1, privuid = (UID_T) -1;
static int realgid = (GID_T) -1, privgid = (GID_T) -1;
static int ttpuid, ttpgid;

/* UID-holding variables */

int oku = 0;				/* uid switching works ok */
int okg = 0;				/* gid switching works ok */

int uida, uidb;				/* Variables for remembering ids... */
int gida, gidb;

int euid, egid;
int ruid, rgid;

/* Function to get and print the current real and effective uid and gid */

chuid() {
    uida = getuid();
    uidb = geteuid();
    printf("     getuid = %d, geteuid = %d\n",uida,uidb);
    gida = getgid();
    gidb = getegid();
    printf("     getgid = %d, getegid = %d\n",gida,gidb);
}

/* Main program */

struct stat statbuf;
char *myname;

main(argc, argv) int argc; char **argv; {
    int x, acc_ok = 1;
    char *path = NULL;

    myname = argv[0];
    if (argc < 2)
      usage();
    else
      path = argv[1];

/* Announce which options we were compiled with. */

#ifndef NOSETREU
#ifndef SETREUID
#define SETREUID
#endif /* SETREUID */
#endif /* NOSETREU */

#ifdef BSD
#ifndef ANYBSD
#define ANYBSD
#endif /* ANYBSD */
#endif /* BSD */

#ifdef ANYBSD
#ifndef SAVEDUID
      printf("BSD, with SAVEDUID not defined\n");
#else
      printf("BSD, with SAVEDUID defined\n");
#endif
#ifdef NOSETREU
      printf("No setre[ug]id, using set[ug]id\n");
#else
      printf("Using setre[ug]id\n");
#endif
#else
      printf("Not BSD\n");
#endif

/* Print uids and gids before, during and after switching back & forth. */

    printf("\n1. ids at startup...\n");
    chuid();
    euid = uidb; egid = gidb;

    /* Initialize uid package, change to real uid. */
    printf("\n2. changing to real user and group ids...\n");
    x = priv_ini();
    printf("   priv_ini returns %d\n",x);
    chuid();
    if (stat(path,&statbuf) == -1) {	/* Do this after switching to self */
	perror(path);
	exit(1);
    }
    if (!S_ISDIR (statbuf.st_mode)) {
	printf("%s: not a directory\n");
	usage();
    }
    acc_ok = chkaccess(path);
    ruid = uidb; rgid = gidb;
    
    printf("   this program %s setuid\n",(ruid != euid) ? "IS" : "is NOT");
    printf("   this program %s setgid\n",(rgid != egid) ? "IS" : "is NOT");
    if (ruid == euid || rgid == egid) {
	printf("\nprogram is not privileged\n");
	printf("please chown owner AND group and add \"ug+s\" bits\n\n");
	exit(0);
    }
    /* Try to change back to effective uid */
    printf("\n3. changing to original user and group ids...\n");
    x = priv_on();
    printf("   priv_on returns %d\n",x);
    chuid();
#if defined(SAVEDUID) || defined(SETEUID)
    printf("   saved-original-effective-uid feature %s present\n",
	   (uidb == euid) ? "IS" : "is NOT");
    printf("   saved-original-effective-gid feature %s present\n",
	   (gidb == egid) ? "IS" : "is NOT");
#else
    printf("   original effective uid %s\n",
	   (uidb == euid) ? "restored ok" : "NOT restored");
    printf("   original effective gid %s\n",
	   (gidb == egid) ? "restored ok" : "NOT restored");
#endif /* SAVEDUID */
    if (uidb != euid || gidb != egid) goto conclude;

    oku = okg = 1;			/* Seems OK so far */

    /* Change back to real uid */
    printf("\n4. switching back to real ids...\n"); 
    x = priv_off();
    printf("   priv_off returns %d\n",x);
    chuid();
    x = chkaccess(path);
    if (x < 0) acc_ok = x;
    if (uidb != ruid) {
	printf("  FAILURE to restore real uid\n");
	oku = 0;
	goto conclude;
    }
    if (gidb != rgid) {
	printf("  FAILURE to restore real gid\n");
	okg = 0;
	goto conclude;
    }
    printf("   real ids restored ok\n");

    /* Change back to real uid, e.g. for a fork */
    printf("\n5. cancelling privileges permanently...\n");
    x = priv_can();
    printf("   priv_can returns %d\n",x);
    chuid();
    x = chkaccess(path);
    if (x < 0) acc_ok = x;
#ifdef FORK
    {
	pid_t pid;
	if (pid = fork()) {
	    /* Try to change back to effective uid */
	    priv_can();
	    printf("\n6. IN FORK: trying to restore canceled privileges\n");
	    x = priv_on();
	    printf("   priv_on returns %d\n",x);
	    chuid();

	    printf("   privilege cancellation %s\n",
		   (uidb == euid || gidb == egid) ? "FAILED" : "SUCCEEDED");
	    _exit(0);
	}	    
    }
#else

    /* Try to change back to effective uid */
    printf("\n6. trying to restore canceled privileges\n");
    x = priv_on();
    printf("   priv_on returns %d\n",x);
    chuid();

    printf("   privilege cancellation %s\n",
	   (uidb == euid || gidb == egid) ? "FAILED" : "SUCCEEDED");
    if (uidb == euid) oku = 0;
    if (gidb == egid) okg = 0;
#endif /* FORK */

    
conclude:
    printf("\nCONCLUSIONS:\n");
    printf("   It %s safe to install C-Kermit setuid.\n",
	   oku ? "IS" : "is NOT");
    printf("   It %s safe to install C-Kermit setgid.\n",
	   okg ? "IS" : "is NOT");
#ifdef ANYBSD
    printf("...when built in the BSD environment with these options:\n");
#ifdef SAVEDUID
    printf("   -DSAVEDUID included\n");
#else
    printf("   -DSAVEDUID omitted\n");
#endif
#ifdef NOSETREU
    printf("   -DNOSETREU included\n");
#else
    printf("   -DNOSETREU omitted (-DSETREUID implied)\n");
#endif
#ifdef SW_ACC_ID
    printf("   -DSW_ACC_ID included\n");
#else
    printf("   -DSW_ACC_ID omitted\n");
#endif
#ifdef SETEUID
    printf("   -DSETEUID included\n");
#else
    printf("   -DSETEUID omitted\n");
#endif
#else
    printf("...when built in the System V or POSIX environment\n");
#endif /* ANYBSD */
    if (acc_ok != 1) {
	if (acc_ok < 0) {
	    printf(
"\nBUT if %s is a directory that you would normally have write\n\
access to, then something is wrong with this system's access() function.\n\n",
		   path, path);
	    printf("Try rebuilding %s -DSW_ACC_ID.\n",
#ifdef SW_ACC_ID
		   "without"
#else
		   "with"
#endif /* SW_ACC_ID */
		   );
	} else {
	    printf("\nAND access() seems to work properly.\n");
	}
    }
    exit(0);
}
  
#ifndef W_OK
#define W_OK 2
#endif /* W_OK */

int
chkaccess(path) char *path; {
    int x;
    if (path) {
#ifdef SW_ACC_ID
	printf("   access check: temporarily swapping ids...\n");
	priv_on();
	x = access(path, W_OK);
	priv_off();
#else
	printf("   access check: no temporary id swapping...\n");
	x = access(path, W_OK);
#endif /* SW_ACC_ID */
	printf("   write-access to %s %s\n", path, x ? "FAILED" : "OK");
	return(x);
    }
    return(1);
}

/* 
  Starting here is Kristoffer's code, as modified by fdc, dlong, et al.
*/

/*
  setuid package, by Kristoffer Eriksson, with contributions from Dean
  Long and fdc.
*/

#ifndef AIX370
extern int getuid(), getgid(), geteuid(), getegid(), getreuid(), getregid();
#endif

/*
Subject: Set-user-id
To: fdc@watsun.cc.columbia.edu (Frank da Cruz)
Date: Sat, 21 Apr 90 4:48:25 MES
From: Kristoffer Eriksson <ske@pkmab.se>

This is a set of functions to be used in programs that may be run set-user-id
and/or set-group-id. They handle both the case where the program is not run
with such privileges (nothing special happens then), and the case where one
or both of these set-id modes are used.  The program is made to run with the
user's real user and group ids most of the time, except for when more
privileges are needed.  Don't set-user-id to "root".

This works on System V and POSIX.  In BSD, it depends on the
"saved-set-user-id" feature.
*/

#define UID_ROOT 0			/* Root user and group ids */
#define GID_ROOT 0

/* P R I V _ I N I  --  Initialize privileges package  */

/* Called as early as possible in a set-uid or set-gid program to store the
 * set-to uid and/or gid and step down to the users real uid and gid. The
 * stored id's can be temporarily restored (allowed in System V) during
 * operations that require the privilege.  Most of the time, the program
 * should execute in unpriviliged state, to not impose any security threat.
 *
 * Note: Don't forget that access() always uses the real id:s to determine
 * file access, even with privileges restored.
 *
 * Returns an error mask, with error values or:ed together:
 *   1 if setuid() fails,
 *   2 if setgid() fails, and
 *   4 if the program is set-user-id to "root", which can't be handled.
 *
 * Only the return value 0 indicates real success. In case of failure,
 * those privileges that could be reduced have been, at least, but the
 * program should be halted nonetheless.
 *
 * Also note that these functions do not expect the uid or gid to change
 * without their knowing. It may work if it is only done temporarily, but
 * you're on your own.
 */
int
priv_ini() {
    int err = 0;

    /* Save real ID:s. */
    realuid = getuid();
    realgid = getgid();

    /* Save current effective ID:s, those set to at program exec. */
    ttpuid = privuid = geteuid();
    ttpgid = privgid = getegid();

    /* If running set-uid, go down to real uid, otherwise remember that
     * no privileged uid is available.
     *
     * Exceptions:
     *
     * 1) If the real uid is already "root" and the set-uid uid (the
     * initial effective uid) is not "root", then we would have trouble
     * if we went "down" to "root" here, and then temporarily back to the
     * set-uid uid (not "root") and then again tried to become "root". I
     * think the "saved set-uid" is lost when changing uid from effective
     * uid "root", which changes all uid, not only the effective uid. But
     * in this situation, we can simply go to "root" and stay there all
     * the time. That should give sufficient privilege (understatement!),
     * and give the right uids for subprocesses.
     *
     * 2) If the set-uid (the initial effective uid) is "root", and we
     * change uid to the real uid, we can't change it back to "root" when
     * we need the privilege, for the same reason as in 1). Thus, we can't
     * handle programs that are set-user-id to "root" at all. The program
     * should be halted.  Use some other uid. "root" is probably too
     * privileged for such things, anyway.  (The uid is reverted to the
     * real uid for the lifetime of the program.)
     *
     * These two exceptions have the effect that the "root" uid will never
     * be one of the two uids that are being switched between, which also
     * means we don't have to check for such cases in the switching
     * functions.
     *
     * Note that exception 1) is handled by these routines (by constantly
     * running with uid "root", while exception 2) is a serious error, and
     * is not provided for at all in the switching functions.
     */
    if (realuid == privuid)
	privuid = (UID_T) -1;		/* Not running set-user-id. */

    /* If running set-gid, go down to real gid, otherwise remember that
     * no privileged gid is available.
     *
     * There are no exception like there is for the user id, since there
     * is no group id that is privileged in the manner of uid "root".
     * There could be equivalent problems for group changing if the
     * program sometimes ran with uid "root" and sometimes not, but
     * that is already avoided as explained above.
     *
     * Thus we can expect always to be able to switch to the "saved set-
     * gid" when we want, and back to the real gid again. You may also
     * draw the conclusion that set-gid provides for fewer hassles than
     * set-uid.
     */

    if (realgid == privgid)		/* If not running set-user-id, */
      privgid = (GID_T) -1;		/*  remember it this way. */

    err = priv_off();			/* Turn off setuid privilege. */

    if (privuid == UID_ROOT)		/* If setuid to root, */
      err |= 4;				/* return this error. */

    if (realuid == UID_ROOT)		/* If real id is root, */
      privuid = (UID_T) -1;		/* stay root at all times. */

    return(err);
}


/* Macros for hiding the differences in UID/GID setting between various Unix
 * systems. These macros should always be called with both the privileged ID
 * and the non-privileged ID. The one in the second argument, will become the
 * effective ID. The one in the first argument will be retained for later
 * retrieval.
 */
#ifdef SETREUID
#ifdef SAVEDUID
/* On BSD systems with the saved-UID feature, we just juggle the effective
 * UID back and forth, and leave the real UID at its true value.  The kernel
 * allows switching to both the current real UID, the effective UID, and the
 * UID which the program is set-UID to.  The saved set-UID always holds the
 * privileged UID for us, and the real UID will always be the non-privileged,
 * and we can freely choose one of them for the effective UID at any time.
 */
#define switchuid(hidden,active) setreuid( (UID_T) -1, active)
#define switchgid(hidden,active) setregid( (GID_T) -1, active)

#else   /* SETREUID,!SAVEDUID */

/* On systems with setreXid() but without the saved-UID feature, notably
 * BSD 4.2, we swap the real and effective UIDs each time.  It's
 * the effective UID that we are interested in, but we have to retain the
 * unused UID somewhere to enable us to restore it later, and we do this
 * in the real UID.  The kernel only allows switching to either the current 
 * real or the effective UID, unless you're "root".
 */
#define switchuid(hidden,active)	setreuid(hidden,active)
#define switchgid(hidden,active)	setregid(hidden,active)
#endif

#else /* !SETREUID, !SAVEDUID */

#ifdef SETEUID
/*
  BSD 4.4 works similarly to System V and POSIX (see below), but uses
  seteXid() instead of setXid() to change effective IDs.  In addition, the
  seteXid() functions work the same for "root" as for other users.
*/
#define switchuid(hidden,active)	seteuid(active)
#define switchgid(hidden,active)	setegid(active)

#else /* !SETEUID */

/* On System V and POSIX, the only thing we can change is the effective UID
 * (unless the current effective UID is "root", but initsuid() avoids that for
 * us).  The kernel allows switching to the current real UID or to the saved
 * set-UID.  These are always set to the non-privileged UID and the privileged
 * UID, respectively, and we only change the effective UID.  This breaks if
 * the current effective UID is "root", though, because for "root" setuid/gid
 * becomes more powerful, which is why initsuid() treats "root" specially.
 * Note: That special treatment maybe could be ignored for BSD?  Note: For
 * systems that don't fit any of these four cases, we simply can't support
 * set-UID.
 */
#define switchuid(hidden,active)	setuid(active)
#define switchgid(hidden,active)	setgid(active)
#endif /* SETEUID */
#endif /* SETREUID */
  

/* P R I V _ O N  --  Turn on the setuid and/or setgid */

/* Go to the privileged uid (gid) that the program is set-user-id
 * (set-group-id) to, unless the program is running unprivileged.
 * If setuid() fails, return value will be 1. If getuid() fails it
 * will be 2.  Return immediately after first failure, and the function
 * tries to restore any partial work done.  Returns 0 on success.
 * Group id is changed first, since it is less serious than user id.
 */
int
priv_on() {
    if (privgid != (GID_T) -1)
      if (switchgid(realgid,privgid))
        return(2);

    if (privuid != (UID_T) -1)
      if (switchuid(realuid,privuid)) {
	  if (privgid != (GID_T) -1)
	    switchgid(privgid,realgid);
	  return(1);
      }
    return(0);
}

/* P R I V _ O F F  --  Turn on the real uid and gid */

/* Return to the unprivileged uid (gid) after an temporary visit to
 * privileged status, unless the program is running without set-user-id
 * (set-group-id). Returns 1 for failure in setuid() and 2 for failure
 * in setgid() or:ed together. The functions tries to return both uid
 * and gid to unprivileged state, regardless of errors. Returns 0 on
 * success.
 */
int
priv_off() {
    int err = 0;

    if (privuid != (UID_T) -1)
       if (switchuid(privuid,realuid))
	  err |= 1;

    if (privgid != (GID_T) -1)
       if (switchgid(privgid,realgid))
	err |= 2;

    return(err);
}

/* Turn off privilege permanently.  No going back.  This is necessary before
 * a fork() on BSD43 machines that don't save the setUID or setGID, because
 * we swap the real and effective ids, and we don't want to let the forked
 * process swap them again and get the privilege back. It will work on other
 * machines too, such that you can rely on its effect always being the same,
 * for instance, even when you're in priv_on() state when this is called.
 * (Well, that part about "permanent" is on System V only true if you follow
 * this with a call to exec(), but that's what we want it for anyway.)
 * Added by Dean Long -- dlong@midgard.ucsc.edu
 */
int
priv_can() {

#ifdef SETREUID
    int err = 0;
    if (privuid != (UID_T) -1)
       if (setreuid(realuid,realuid))
	  err |= 1;

    if (privgid != (GID_T) -1)
        if (setregid(realgid,realgid))
 	  err |= 2;

    return(err);

#else
#ifdef SETEUID
    int err = 0;
    if (privuid != (UID_T) -1)
       if (setuid(realuid))
	  err |= 1;

    if (privgid != (GID_T) -1)
        if (setgid(realgid))
 	  err |= 2;
    return(err);
#else
    /* Easy way of using setuid()/setgid() instead of setreuid()/setregid().*/
    return(priv_off());
#endif /* SETEUID */
#endif /* SETREUID */
}

/*  P R I V _ C H K  --  Check privileges.  */

/*  Try to turn them off.  If turning them off did not succeed, cancel them */

int
priv_chk() {
    int x, y = 0;
    x = priv_off();			/* Turn off privs. */
    if (x != 0 || getuid() == privuid || geteuid() == privuid)
      y = priv_can();
    if (x != 0 || getgid() == privgid || getegid() == privgid)
      y = y | priv_can();
    return(y);
}

usage() {
    printf("usage: %s directory\n\n", myname);
    printf(
"please supply the name of a directory that you have write access to.\n");
    exit(1);
}
