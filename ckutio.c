#ifdef aegis
char *ckxv = "Aegis Communications support, 6.0.169, 6 Sep 96";
#else
#ifdef Plan9
char *ckxv = "Plan 9 Communications support, 6.0.169, 6 Sep 96";
#else
char *ckxv = "UNIX Communications support, 6.0.169, 6 Sep 96";
#endif /* Plan9 */
#endif /* aegis */

/*  C K U T I O  */

/* C-Kermit interrupt, communications control and I/O functions for UNIX */

/*
  NOTE TO CONTRIBUTORS: This file, and all the other C-Kermit files, must be
  compatible with C preprocessors that support only #ifdef, #else, #endif,
  #define, and #undef.  Please do not use #if, logical operators, or other
  preprocessor features in any of the portable C-Kermit modules.  You can,
  of course, use these constructions in system-specific modules when they
  are supported.
*/

/*
  Author: Frank da Cruz (fdc@columbia.edu),
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 1996, Trustees of Columbia University in the City of New
  York.  All rights reserved.
*/

/* Includes */

#include "ckcsym.h"			/* This must go first   */
#include "ckcdeb.h"			/* This must go second  */
#include <errno.h>			/* System error numbers */

#ifdef __386BSD__
#define ENOTCONN 57
#else
#ifdef __bsdi__
#define ENOTCONN 57
#else
#ifdef __FreeBSD__
#define ENOTCONN 57
#endif /* __FreeBSD__ */
#endif /* __bsdi__ */
#endif /* __386BSD__ */

#ifdef Plan9
#define SELECT
#include <sys/time.h>
#include <select.h>
#define FD_SETSIZE (3 * sizeof(long) * 8)
static struct timeval tv;
#endif /* Plan9 */

#include "ckcnet.h"			/* Symbols for network types. */

#ifdef XENIX /* Was M_UNIX but XENIX implies M_UNIX and applies to XENIX too */
/*
  <sys/socket.h> included above via "ckcnet.h" defines FIONREAD as
  something.  Due to this, in_chk() uses the FIONREAD instead of RDCHK
  and the hot keys during file transfer (X to cancel file etc) do not
  work because FIONREAD doesn't work even though it is defined.

  NOTE: This might also be true elsewhere.
*/
#ifdef FIONREAD
#undef FIONREAD
#endif /* FIONREAD */
#endif /* XENIX */

#ifdef CK_SCOV5				/* Ditto for SCO OpenServer 5.0 */
#ifdef FIONREAD
#undef FIONREAD
#endif /* FIONREAD */
#endif /* XENIX */

/*
  The directory-related includes are here because we need to test some
  file-system-related symbols to find out which system we're being compiled
  under.  For example, MAXNAMLEN is defined in BSD4.2 but not 4.1.
*/
#ifdef SDIRENT				/* Directory bits... */
#define DIRENT
#endif /* SDIRENT */

#ifdef XNDIR
#include <sys/ndir.h>
#else /* !XNDIR */
#ifdef NDIR
#include <ndir.h>
#else /* !NDIR, !XNDIR */
#ifdef RTU
#include "/usr/lib/ndir.h"
#else /* !RTU, !NDIR, !XNDIR */
#ifdef DIRENT
#ifdef SDIRENT
#include <sys/dirent.h>
#else
#include <dirent.h>
#endif /* SDIRENT */
#else /* !RTU, !NDIR, !XNDIR, !DIRENT, i.e. all others */
#include <sys/dir.h>
#endif /* DIRENT */
#endif /* RTU */
#endif /* NDIR */
#endif /* XNDIR */

/* Definition of HZ, used in msleep() */

#ifdef MIPS
#define HZ ( 1000 / CLOCK_TICK )
#else
#ifdef ATTSV
#ifndef NAP
#ifndef TRS16
#include <sys/param.h>
#else
#define HZ ( 1000 / CLOCK_TICK )
#endif /* TRS16 */
#ifdef NAPHACK
#define nap(x) (void)syscall(3112, (x))
#define NAP
#endif /* NAPHACK */
#endif /* NAP */
#endif /* ATTSV */
#endif /* MIPS */

#ifdef M_UNIX
#undef NGROUPS_MAX		/* Prevent multiple definition warnings */
#endif /* M_UNIX */

/*
  NOTE: HP-UX 8.0 has a <sys/poll.h>, but there is no corresponding
  library routine, so _poll comes up undefined at link time.
*/
#ifdef CK_POLL
#ifndef AIXRS			/* IBM AIX needs special handling */
#include <poll.h>		/* "standard" (SVID) i/o multiplexing, etc */
#else /* AIXRS */
#ifdef SVR4			/* AIX 3.2 is like SVID... */
#include <poll.h>
#else				/* But AIX 3.1 is not ... */
#include <sys/poll.h>		/* The include file is in include/sys */
#define events reqevents	/* And it does not map IBM-specific member */
#define revents rtnevents	/* names to the System V equivalents */
#endif /* SVR4 */
#endif /* AIXRS */
#endif /* CK_POLL */

#include <signal.h>                     /* Signals */

/* For setjmp and longjmp */

#ifndef ZILOG
#include <setjmp.h>
#else
#include <setret.h>
#endif /* ZILOG */

/* Maximum length for the name of a tty device */

#ifndef DEVNAMLEN
#define DEVNAMLEN 32
#endif /* DEVNAMLEN */

#ifdef	NETCONN
#undef DEVNAMLEN
#define DEVNAMLEN 256			/* Longer field for host:service */
#endif  /* NETCONN */

/*
  The following test differentiates between 4.1 BSD and 4.2 & later.
  If you have a 4.1BSD system with the DIRENT library, this test could
  mistakenly diagnose 4.2BSD and then later enable the use of system calls
  that aren't defined.  If indeed there are such systems, we can use some
  other way of testing for 4.1BSD, or add yet another compile-time switch.
*/  
#ifdef BSD4
#ifdef MAXNAMLEN
#ifndef FT21				/* Except for Fortune. */
#ifndef FT18
#ifndef BELLV10				/* And Bell Labs Research UNIX V10 */
#define BSD42
#endif /* BELLV10 */
#endif /* FT18 */
#endif /* FT21 */
#endif /* MAXNAMLEN */
#endif /* BSD4 */
/*
 MINIX 1.0 support added by Charles Hedrick,
 Rutgers University <hedrick@aramis.rutgers.edu>.
 MINIX also has V7 enabled.
*/
#ifdef MINIX
#define TANDEM 0
#define MYREAD
#include <limits.h>
#endif /* MINIX */

#ifdef CK_REDIR		/* <sys/wait.h> needed only for REDIRECT command. */
/*
  If anybody can figure out how to make this work with NeXTSTEP, be
  my guest!  (NeXTBlah/NeXTBlah/bsd/sys/wait.h does not define WEXITSTATUS)
*/
#ifndef CK_WAIT_H			/* If wait.h not already included... */
#include <sys/wait.h>			/* Include it */
#endif /* CK_WAIT_H */
#endif /* CK_REDIR */

#include "ckuver.h"			/* Version herald */
char *ckxsys = HERALD;

/* UUCP lockfile name definition */

#ifndef NOUUCP

/* Name of UUCP tty device lockfile */

#ifdef LINUXFSSTND
#ifndef HDBUUCP
#define HDBUUCP
#endif /* HDBUUCP */
#endif /* LINUXFSSTND */

#ifdef ACUCNTRL
#define LCKDIR
#endif /* ACUCNTRL */

/*
  PIDSTRING means use ASCII string to represent pid in lockfile.
*/
#ifndef PIDSTRING
#ifdef HDBUUCP
#define PIDSTRING
#else
#ifdef BSD44
#define PIDSTRING
#else
#ifdef RTAIX
#define PIDSTRING
#else
#ifdef AIXRS
#define PIDSTRING
#else
#ifdef COHERENT
#define PIDSTRING
#endif /* COHERENT */
#endif /* AIXRS */
#endif /* RTAIX */
#endif /* BSD44 */
#endif /* HDBUUCP */
#endif /* PIDSTRING */

/* Now the PIDSTRING exceptions... */

#ifdef HPUX10				/* HP-UX 10.00 */
#ifdef PIDSTRING
#undef PIDSTRING
#endif /* PIDSTRING */
#endif /* HPUX10 */

#ifdef __bsdi__				/* BSDI (at least thru 1.1) */
#ifdef PIDSTRING
#undef PIDSTRING
#endif /* PIDSTRING */
#endif /* __bsdi__ */

#ifdef OSF32				/* Digital UNIX (OSF/1) 3.2 */
#ifdef PIDSTRING
#undef PIDSTRING
#endif /* PIDSTRING */
#endif /* OSF32 */

/*
  LOCK_DIR is the name of the lockfile directory.
  If LOCK_DIR is already defined (e.g. on command line), we don't change it.
*/
#ifndef LOCK_DIR
#ifdef BSD44
#ifdef __386BSD__
#define LOCK_DIR "/var/spool/lock";
#else
#ifdef __FreeBSD__
#define LOCK_DIR "/var/spool/lock";
#else
#ifdef __NetBSD__
#define LOCK_DIR "/var/spool/lock";
#else
#define LOCK_DIR "/var/spool/uucp";
#endif /* __NetBSD__ */
#endif /* __FreeBSD__ */
#endif /* __386BSD__ */
#else
#ifdef DGUX430
#define LOCK_DIR "/var/spool/locks";
#else
#ifdef HPUX10
#define LOCK_DIR "/var/spool/locks";
#else
#ifdef RTAIX				/* IBM RT PC AIX 2.2.1 */
#define LOCK_DIR "/etc/locks";
#else
#ifdef AIXRS
#define LOCK_DIR "/etc/locks";
#else
#ifdef ISIII
#define LOCK_DIR "/etc/locks";
#else
#ifdef HDBUUCP
#ifdef M_SYS5
#define LOCK_DIR "/usr/spool/uucp";
#else
#ifdef M_UNIX
#define LOCK_DIR "/usr/spool/uucp";
#else
#ifdef SVR4
#define LOCK_DIR "/var/spool/locks";
#else
#ifdef SUNOS4
#define LOCK_DIR "/var/spool/locks";
#else
#ifdef LINUXFSSTND
#define LOCK_DIR "/var/lock";
#else
#define LOCK_DIR "/usr/spool/locks";
#endif /* LINUXFSSTND */
#endif /* SUNOS4 */
#endif /* SVR4 */
#endif /* M_UNIX */
#endif /* M_SYS5 */
#else
#ifdef LCKDIR
#define LOCK_DIR "/usr/spool/uucp/LCK";
#else
#ifdef COHERENT
#define LOCK_DIR "/usr/spool/uucp";
#else
#define LOCK_DIR "/usr/spool/uucp";
#endif /* COHERENT */
#endif /* LCKDIR */
#endif /* HDBUUCP */
#endif /* ISIII */
#endif /* AIXRS */
#endif /* RTAIX */
#endif /* HPUX10 */
#endif /* DGUX430 */
#endif /* BSD44 */
#endif /* !LOCK_DIR (outside ifndef) */
   
#ifdef OSF2				/* OSF/1 2.0 or later */
#ifdef LOCK_DIR				/* (maybe 1.x too, who knows...) */
#undef LOCK_DIR
#define LOCK_DIR "/var/spool/locks";
#endif /* LOCK_DIR */
#endif /* OSF2 */

#ifdef SVR4
#ifndef BSD44
#ifndef LOCKF
#define LOCKF				/* Use lockf() on tty device in SVR4 */
#endif /* LOCKF */
#endif /* BSD44 */
#endif /* SVR4 */

#ifdef NOLOCKF				/* But NOLOCKF cancels LOCKF */
#ifdef LOCKF
#undef LOCKF
#endif /* LOCKF */
#endif /* NOLOCKF */

#endif /* !NOUUCP */

/*
  MYREAD means use our internally defined nonblocking buffered read routine.
*/
#ifdef ATTSV
#define MYREAD
#endif /* ATTSV */

#ifdef ATT7300
#ifndef MYREAD
#define MYREAD
#endif /* MYREAD */
/* bits for attmodem: internal modem in use, restart getty */
#define ISMODEM 1
#define DOGETY 512
#endif  /* ATT7300 */

#ifdef BSD42
#define MYREAD
#endif /* BSD42 */

#ifdef POSIX
#define MYREAD
#endif /* POSIX */
#ifdef __bsdi__
#ifndef O_NDELAY
#define O_NDELAY O_NONBLOCK
#endif /* O_NDELAY */
#endif /* __bsdi__ */

/*
 Variables available to outside world:

   dftty  -- Pointer to default tty name string, like "/dev/tty".
   dfloc  -- 0 if dftty is console, 1 if external line.
   dfprty -- Default parity
   dfflow -- Default flow control
   ckxech -- Flag for who echoes console typein:
     1 - The program (system echo is turned off)
     0 - The system (or front end, or terminal).
   functions that want to do their own echoing should check this flag
   before doing so.

   flfnam  -- Name of lock file, including its path, e.g.,
                "/usr/spool/uucp/LCK..cul0" or "/etc/locks/tty77"
   lkflfn  -- Name of link to lock file, including its paths
   haslock -- Flag set if this kermit established a uucp lock.
   backgrd -- Flag indicating program executing in background ( & on
                end of shell command). Used to ignore INT and QUIT signals.
   rtu_bug -- Set by stptrap().  RTU treats ^Z as EOF (but only when we handle
                SIGTSTP)

 Functions for assigned communication line (either external or console tty):

   sysinit()               -- System dependent program initialization
   syscleanup()            -- System dependent program shutdown
   ttopen(ttname,local,mdmtyp,timo) -- Open the named tty for exclusive access.
   ttclos()                -- Close & reset the tty, releasing any access lock.
   ttsspd(cps)             -- Set the transmission speed of the tty.
   ttgspd()                -- Get (read) the the transmission speed of the tty.
   ttpkt(speed,flow,parity) -- Put the tty in packet mode and set the speed.
   ttvt(speed,flow)        -- Put the tty in virtual terminal mode.
                                or in DIALING or CONNECTED modem control state.
   ttres()                 -- Restore original tty modes.
   ttscarr(carrier)        -- Set carrier control mode, on/off/auto.
   ttinl(dest,max,timo)    -- Timed read line from the tty.
   ttinc(timo)             -- Timed read character from tty.
   myread()                -- Raw mode bulk buffer read, gives subsequent
                                chars one at a time and simulates FIONREAD.
   myunrd(c)               -- Places c back in buffer to be read (one only)
   ttchk()                 -- See how many characters in tty input buffer.
   ttxin(n,buf)            -- Read n characters from tty (untimed).
   ttol(string,length)     -- Write a string to the tty.
   ttoc(c)                 -- Write a character to the tty.
   ttflui()                -- Flush tty input buffer.
   ttsndb()                -- Send BREAK signal.
   ttsndlb()               -- Send Long BREAK signal.

   ttlock(ttname)          -- "Lock" tty device against uucp collisions.
   ttunlck()               -- Unlock tty device.

                              For ATT7300/Unix PC, System V:
   attdial(ttname,speed,telnbr) -- dials ATT7300/Unix PC internal modem
   offgetty(ttname)        -- Turns off getty(1m) for comms line
   ongetty(ttname)         -- Restores getty() to comms line
*/

/*
Functions for console terminal:

   congm()   -- Get console terminal modes.
   concb(esc) -- Put the console in single-character wakeup mode with no echo.
   conbin(esc) -- Put the console in binary (raw) mode.
   conres()  -- Restore the console to mode obtained by congm().
   conoc(c)  -- Unbuffered output, one character to console.
   conol(s)  -- Unbuffered output, null-terminated string to the console.
   conola(s) -- Unbuffered output, array of strings to the console.
   conxo(n,s) -- Unbuffered output, n characters to the console.
   conchk()  -- Check if characters available at console (bsd 4.2).
                Check if escape char (^\) typed at console (System III/V).
   coninc(timo)  -- Timed get a character from the console.
   congks(timo)  -- Timed get keyboard scan code.
   conint()  -- Enable terminal interrupts on the console if not background.
   connoi()  -- Disable terminal interrupts on the console if not background.

Time functions

   msleep(m) -- Millisecond sleep
   ztime(&s) -- Return pointer to date/time string
   rtimer() --  Reset timer
   gtimer()  -- Get elapsed time since last call to rtimer()
*/

/* Conditional Includes */

/* Whether to include <sys/file.h> */

#ifdef RTU				/* RTU doesn't */
#define NOFILEH
#endif /* RTU */

#ifdef CIE				/* CIE does. */
#undef NOFILEH
#endif /* CIE */

#ifdef BSD41				/* 4.1 BSD doesn't */
#define NOFILEH
#endif /* BSD41 */

#ifdef is68k				/* is68k (whatever that is)  */
#define NOFILEH
#endif /* is68k */

#ifdef MINIX				/* MINIX */
#define NOFILEH
#endif /* MINIX */

#ifdef COHERENT				/* Coherent */
#define NOFILEH
#endif /* COHERENT */

#ifndef NOFILEH				/* Now include if selected. */
#include <sys/file.h>
#endif /* NOFILEH */

/* POSIX */

#ifdef BSD44ORPOSIX			/* POSIX uses termios.h */
#define TERMIOS
#ifdef __bsdi__
#ifdef POSIX
#undef _POSIX_SOURCE			/* Get extra stuff from termios.h */
#endif /* POSIX */
#endif /* __bsdi__ */
#include <termios.h>
#ifdef __bsdi__
#ifdef POSIX
#define _POSIX_SOURCE
#endif /* POSIX */
#endif /* __bsdi__ */
#ifndef BSD44				/* Really POSIX */
#ifndef CK_QNX32
#define NOSYSIOCTLH			/* No ioctl's allowed. */
#undef ultrix				/* Turn off any ultrix features. */
#endif /* CK_QNX32 */
#endif /* BSD44 */
#endif /* POSIX */

/* System III, System V */

#ifdef ATTSV
#ifndef BSD44
#ifndef POSIX
#include <termio.h>
#endif /* POSIX */
#endif /* BSD44 */
/* #ifdef SVR4 */			/* Sys V R4 and later */
#ifdef TERMIOX
/* Need this for termiox structure, RTS/CTS and DTR/CD flow control */
#include <termiox.h>
  struct termiox rctsx;
#else
#ifdef STERMIOX
#include <sys/termiox.h>
  struct termiox rctsx;
#endif /* STERMIOX */
#endif /* TERMIOX */
/* #endif */ /* SVR4 */
#endif /* ATTSV */

#ifdef COHERENT			/* Use termio.h, not sgtty.h for Coherent */
#include <termio.h>
#endif /* COHERENT */

#ifdef MINIX				/* MINIX uses ioctl's */
#define NOSYSIOCTLH			/* but has no <sys/ioctl.h> */
#endif /* MINIX */

/* Others */

#ifndef NOSYSIOCTLH			/* Others use ioctl() */
#ifdef SUN4S5
/*
  This is to get rid of cpp warning messages that occur because all of
  these symbols are defined by both termios.h and ioctl.h on the SUN.
*/
#undef ECHO
#undef NL0
#undef NL1
#undef TAB0
#undef TAB1
#undef TAB2
#undef XTABS
#undef CR0
#undef CR1
#undef CR2
#undef CR3
#undef FF0
#undef FF1
#undef BS0
#undef BS1
#undef TOSTOP
#undef FLUSHO
#undef PENDIN
#undef NOFLSH
#endif /* SUN4S5 */
#include <sys/ioctl.h>
#endif /* NOSYSIOCTLH */

#ifdef BELLV10
#include <sys/filio.h>			/* For FIONREAD */
#ifdef FIONREAD
#define MYREAD
#endif /* MYREAD */
#endif /* BELLV10 */

/* Whether to include <fcntl.h> */

#ifndef is68k				/* Only a few don't have this one. */
#ifndef BSD41
#ifndef FT21
#ifndef FT18
#ifndef COHERENT
#include <fcntl.h>
#endif /* COHERENT */
#endif /* FT18 */
#endif /* FT21 */
#endif /* BSD41 */
#endif /* not is68k */

#ifdef COHERENT
#ifdef _I386
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif /* _I386 */
#endif /* COHERENT */

#ifdef ATT7300				/* Unix PC, internal modem dialer */
#include <sys/phone.h>
#endif /* ATT7300 */

#ifdef HPUX				/* HP-UX variations. */
#define HPUXJOBCTL
#include <sys/modem.h>			/* HP-UX modem signals */
#ifdef hp9000s500			/* Model 500 */
#undef HPUXJOBCTL
#endif /* hp9000s500 */
#ifdef HPUXPRE65
#undef HPUXJOBCTL
typedef long mflag;
#endif /* HPUXPRE65 */
#ifdef HPUXJOBCTL
#include <sys/bsdtty.h>			/* HP-UX Berkeley tty support */
#endif /* HPUXJOBCTL */
#endif /* HPUX */

/* AT&T Sys V or POSIX */

#ifdef SVORPOSIX			/* Sys V or POSIX */
#ifdef M_UNIX
#include <sys/time.h>
#else
#ifdef OXOS				/* Olivetti X/OS */
#include <sys/time.h>
#else
#ifdef BSD44				/* 4.4BSD */
#include <sys/time.h>
#else
#ifdef __linux__			/* Linux */
/* Kludge to fix redefinition in <linux/wait.h> */
#undef WNOHANG
#undef WUNTRACED
#include <linux/fs.h>
#include <linux/tty.h>
#include <sys/time.h>
#ifdef LINUXHISPEED
#include <linux/serial.h>
#endif /* LINUXHISPEED */
#else
#ifdef AIXRS				/* IBM AIX */
#include <sys/time.h>
#else
#ifdef IRIX60
#include <sys/time.h>
#endif /* IRIX60 */
#endif /* AIXRS */
#endif /* __linux__ */
#endif /* BSD44 */
#endif /* OXOS */
#endif /* M_UNIX */

#ifdef NOIEXTEN				/* This is broken on some systems */
#undef IEXTEN				/* like Convex/OS 9.1 */
#endif /* NOIEXTEN */
#ifndef IEXTEN				/* Turn off ^O/^V processing. */
#define IEXTEN 0			/* Needed, at least, on BSDI. */
#endif /* IEXTEN */

#else /* BSD, V7, Coherent, Minix, et al. */

#include <sgtty.h>                      /* So we use <sgtty.h> */
#ifndef BELLV10
#ifndef PROVX1				/* Now <sys/time.h> ... */
#ifndef V7
#ifndef BSD41
#ifndef COHERENT
#include <sys/time.h>                   /* Clock info (for break generation) */
#endif /* COHERENT */
#endif /* BSD41 */
#endif /* V7 */
#endif /* PROVX1 */
#endif /* BELLV10 */
#endif /* SVORPOSIX */

#ifdef BELLV10
#include <sys/timeb.h>
#endif /* BELLV10 */

#ifdef PS2AIX10
#include <sys/time.h>                   /* Clock info for PS/2 AIX 1.x */
#endif /* PS2AIX10 */

#ifdef OSF				/* DEC OSF/1 1.0 */
#include <sys/timeb.h>
#endif /* OSF */

#ifdef BSD41				/* BSD 4.1 */
#include <sys/timeb.h>
#endif /* BSD41 */

#ifdef FT21				/* For:Pro 2.1 */
#include <sys/timeb.h>
#endif /* FT21 */

#ifdef BSD29				/* BSD 2.9 */
#include <sys/timeb.h>
#endif /* BSD29 */

#ifdef TOWER1
#include <sys/timeb.h>                  /* Clock info for NCR Tower */
#endif /* TOWER1 */

#ifdef COHERENT
#include <sys/timeb.h>                  /* Clock info for NCR Tower */
#endif /* COHERENT */

/*
  Pick up definitions needed for select() if we don't have them already.
  Normally they come from <sys/types.h> but some systems get them from
  <sys/select.h>...  Rather than hardwire all of them into the source, we
  include it if SELECT_H is defined in compile-time CFLAGS.
*/
#ifdef SELECT_H
#include <sys/select.h>
#endif /* SELECT_H */

#ifdef aegis
#include "/sys/ins/base.ins.c"
#include "/sys/ins/error.ins.c"
#include "/sys/ins/ios.ins.c"
#include "/sys/ins/sio.ins.c"
#include "/sys/ins/pad.ins.c"
#include "/sys/ins/time.ins.c"
#include "/sys/ins/pfm.ins.c"
#include "/sys/ins/pgm.ins.c"
#include "/sys/ins/ec2.ins.c"
#include "/sys/ins/type_uids.ins.c"
#include <default_acl.h>
#undef TIOCEXCL
#undef FIONREAD
#endif /* aegis */

#ifdef sxaE50				/* PFU Compact A SX/A TISP V10/L50 */
#undef FIONREAD
#endif /* sxaE50 */

/* The following #defines are catch-alls for those systems */
/* that didn't have or couldn't find <file.h>... */

#ifndef FREAD
#define FREAD 0x01
#endif /* FREAD */

#ifndef FWRITE
#define FWRITE 0x10
#endif /* FWRITE */

#ifndef O_RDONLY
#define O_RDONLY 000
#endif /* O_RDONLY */

/* Declarations */

#ifdef OXOS
#undef TCGETA
#undef TCSETA
#undef TCSETAW
#undef TCSETAF
#define TCGETA TCGETS
#define TCSETA TCSETS
#define TCSETAW TCSETSW
#define TCSETAF TCSETSF
#define termio termios
#endif /* OXOS */

#ifdef HPUX
#include <time.h>
#else
#ifdef _POSIX_SOURCE			/* This includes MINIX */
#ifndef AIXRS
#include <time.h>
#endif /* AIXRS */
#ifdef __GNUC__
#ifdef XENIX
/*
  Because Xenix <time.h> doesn't declare time() if we're using gcc.
*/
time_t time();
#endif /* XENIX */
#endif /* __GNUC__ */
#else
time_t time();				/* All Unixes should have this... */
#endif /* _POSIX_SOURCE */
#endif /* HPUX */
/* Special stuff for V7 input buffer peeking */

#ifdef  V7
int kmem[2] = { -1, -1};
char *initrawq(), *qaddr[2]={0,0};
#define CON 0
#define TTY 1
#endif /* V7 */

/* dftty is the device name of the default device for file transfer */
/* dfloc is 0 if dftty is the user's console terminal, 1 if an external line */

#ifndef DFTTY
#ifdef PROVX1
    char *dftty = "/dev/com1.dout"; /* Only example so far of a system */
    char *dfmdm = "none";
    int dfloc = 1;                  /* that goes in local mode by default */
#else
    char *dftty = CTTNAM;               /* Remote by default, use normal */
    char *dfmdm = "none";
    int dfloc = 0;                      /* controlling terminal name. */
#endif /* PROVX1 */
#else
    char *dftty = DFTTY;		/* Default location specified on */
    char *dfmdm = "none";		/* command line. */
    int dfloc = 1;                      /* controlling terminal name. */
#endif /* DFTTY */

#ifdef RTU
    int rtu_bug = 0;		    /* set to 1 when returning from SIGTSTP */
#endif /* RTU */

    int dfprty = DEFPAR;                /* Default parity (0 = none) */
    int ttprty = 0;                     /* The parity that is in use. */
    int ttpflg = 0;			/* Parity not sensed yet. */
    static int ttpmsk = 0xff;		/* Parity stripping mask. */
    int ttmdm = 0;                      /* Modem in use. */
    int ttcarr = CAR_AUT;		/* Carrier handling mode. */
    int dfflow = FLO_XONX;		/* Default is Xon/Xoff */
    int backgrd = 0;                    /* Assume in foreground (no '&' ) */
#ifdef ultrix
    int iniflags = 0;			/* fcntl flags for ttyfd */
#endif /* ultrix */
    int fdflag = 0;			/* Flag for redirected stdio */
    int ttfdflg = 0;			/* Open File descriptor was given */
    int tvtflg = 0;			/* Flag that ttvt has been called */
    long ttspeed = -1L;			/* For saving speed */
    int ttflow = -9;			/* For saving flow */
    int ttld = -1;			/* Line discipline */

#ifdef sony_news
    static int km_con = -1;		/* Kanji mode for console tty */
    static int km_ext = -1;		/* Kanji mode for external device */
#endif /* sony_news */

/*
  Having this module rely on external globals is bad, but fixing this
  requires overhaul of the ck*tio.c modules for all the different operating
  systems supported by C-Kermit.  Left for a future release.
*/
extern int ttnproto;			/* Defined in ckcnet.c */
extern int ttnet;			/* Defined in ckcnet.c */
extern int xfrcan, xfrchr, xfrnum;	/* Defined in ckcmai.c */
extern int suspend;

int ckxech = 0; /* 0 if system normally echoes console characters, else 1 */

/* Declarations of variables global within this module */

static time_t tcount;			/* Elapsed time counter */
static SIGTYP (*saval)() = NULL;	/* For saving alarm() handler */
#ifndef NOJC
static SIGTYP (*jchdlr)() = NULL;	/* For checking suspend handler */
#endif /* NOJC */
static int jcshell = -1;		/* And flag for result */

/*
  BREAKNULS is defined for systems that simulate sending a BREAK signal
  by sending a bunch of NUL characters at low speed.
*/
#ifdef PROVX1
#ifndef BREAKNULS
#define BREAKNULS
#endif /* BREAKNULS */
#endif /* PROVX1 */

#ifdef V7
#ifndef BREAKNULS
#define BREAKNULS
#endif /* BREAKNULS */
#endif /* V7 */

#ifdef BREAKNULS
static char				/* A string of nulls */
*brnuls = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
#endif /* BREAKNULS */

#ifdef CK_POSIX_SIG			/* Longjump buffers */
static sigjmp_buf sjbuf;		/* POSIX signal handling */
#else
static jmp_buf sjbuf;
#endif /* CK_POSIX_SIG */

#ifdef V7
static jmp_buf jjbuf;
#endif /* V7 */

/* static */				/* (Not static any more) */
int ttyfd = -1;				/* TTY file descriptor */

#ifdef Plan9
int ttyctlfd  = -1;   /* TTY control channel - What? UNIX doesn't have one? */
int consctlfd = -1;			/* Console control channel */
int noisefd = -1;			/* tone channel */
static int ttylastspeed = -1;		/* So we can lie about the speed */
#endif /* Plan9 */

int telnetfd = 0;			/* File descriptor is for telnet */
int x25fd = 0;				/* File descriptor is for X.25 */

static int lkf = 0,                     /* Line lock flag */
    cgmf = 0,                           /* Flag that console modes saved */
    xlocal = 0,                         /* Flag for tty local or remote */
    curcarr = 0;			/* Carrier mode: require/ignore. */

static int netconn = 0;			/* 1 if network connection active */

static char escchr;                     /* Escape or attn character */

#ifdef COHERENT
#ifdef SELECT
    static struct timeval tv;		/* Needed for -DSELECT in makefile */
#endif /* SELECT */
#endif /* COHERENT */

#ifdef QNX
    static struct timeval tv;		/* For getting time, from sys/time.h */
    static struct timezone tz;
#else
#ifdef SOLARIS
    static struct timeval tv;		/* For getting time, from sys/time.h */
    static struct timezone tz;
#else
#ifdef HPUX
    static struct timeval tv;		/* For getting time, from sys/time.h */
    static struct timezone tz;
#else
#ifdef __linux__
    static struct timeval tv;		/* For getting time, from sys/time.h */
    static struct timezone tz;
#else
#ifdef PS2AIX10
    static struct timeval tv;		/* For getting time, from sys/time.h */
    static struct timezone tz;
#else
#ifdef AIXRS
    static struct timeval tv;		/* For getting time, from sys/time.h */
    static struct timezone tz;
#else
#ifdef BSD44
    static struct timeval tv;		/* For getting time, from sys/time.h */
    static struct timezone tz;
#else
#ifdef BSD42
    static struct timeval tv;		/* For getting time, from sys/time.h */
    static struct timezone tz;
#ifdef OSF
    static struct timeb ftp;            /* And from sys/timeb.h */
#endif /* OSF */
#else
#ifdef OXOS
    static struct timeval tv;		/* For getting time, from sys/time.h */
    static struct timezone tz;
#else
#ifdef SELECT
#ifndef BELLV10
    static struct timeval tv;
    static struct timezone tz;
#endif /* BELLV10 */
#endif /* SELECT */
#endif /* OXOS */
#endif /* BSD42 */
#endif /* BSD44 */
#endif /* AIXRS */
#endif /* PS2AIX10 */
#endif /* __linux__ */
#endif /* HPUX */
#endif /* SOLARIS */
#endif /* QNX */

#ifdef BSD29
    static long xclock;			/* For getting time from sys/time.h */
    static struct timeb ftp;            /* And from sys/timeb.h */
#endif /* BSD29 */

#ifdef BSD41
    static long xclock;			/* For getting time from sys/time.h */
    static struct timeb ftp;            /* And from sys/timeb.h */
#endif /* BSD41 */

#ifdef BELLV10
    static long xclock;			/* For getting time from sys/time.h */
    static struct timeb ftp;            /* And from sys/timeb.h */
#endif /* BELLV10 */

#ifdef FT21
    static long xclock;			/* For getting time from sys/time.h */
    static struct timeb ftp;            /* And from sys/timeb.h */
#endif /* FT21 */

#ifdef TOWER1
    static long xclock;			/* For getting time from sys/time.h */
    static struct timeb ftp;		/* And from sys/timeb.h */
#endif /* TOWER1 */

#ifdef COHERENT
    static long xclock;			/* For getting time from sys/time.h */
    static struct timeb ftp;		/* And from sys/timeb.h */
#endif /* COHERENT */

#ifdef V7
    static long xclock;
#endif /* V7 */

/* sgtty/termio information... */

#ifdef BSD44ORPOSIX			/* POSIX or BSD44 */
  static struct termios
    ttold, ttraw, tttvt, ttcur,
    ccold, ccraw, cccbrk;
#else					/* BSD, V7, etc */

#ifdef COHERENT				/* Hack alert... */
#define ATTSV
#endif /* COHERENT */

#ifdef ATTSV
  static struct termio ttold = {0};	/* Init'd for word alignment, */
  static struct termio ttraw = {0};	/* which is important for some */
  static struct termio tttvt = {0};	/* systems, like Zilog... */
  static struct termio ttcur = {0};
  static struct termio ccold = {0};
  static struct termio ccraw = {0};
  static struct termio cccbrk = {0};
#else
  static struct sgttyb                  /* sgtty info... */
    ttold, ttraw, tttvt, ttcur, 	/* for communication line */
    ccold, ccraw, cccbrk;		/* and for console */
#ifdef BELLV10
  static struct ttydevb			/* Device info... */
    tdold, tdcur;			/* for communication device */
#endif /* BELLV10 */
#ifdef TIOCGETC
  static struct tchars tchold, tchnoi;

  static int tcharf;
#endif /* TIOCGETC */
#ifdef TIOCGLTC
  static struct ltchars ltchold, ltchnoi;
  static int ltcharf;
#endif /* TIOCGLTC */
  int lmodef = 0;			/* Local modes */
  int lmode = 0;
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */

#ifdef PROVX1
  static struct sgttyb ttbuf;
#endif /* PROVX1 */

#ifdef ultrix
/* do we really need this? */
  static struct sgttyb vanilla;
#endif /* ultrix */

#ifdef ATT7300
static int attmodem = 0;                /* ATT7300 internal-modem status */
struct updata dialer = {0};		/* Condition dialer for data call */
#endif /* ATT7300 */

#ifndef NOUUCP
char flfnam[80];			/* UUCP lock file path name */
#ifdef HPUX10
char lock2[80];				/* Name of second lockfile */
#endif /* HPUX10 */
#else
char flfnam[8];
#endif /* NOUUCP */

#ifdef RTAIX
char lkflfn[80];			/* and possible link to it */
#endif /* RTAIX */
int haslock = 0;			/* =1 if this kermit locked uucp */

#ifndef OXOS
#ifdef SVORPOSIX
static int conesc = 0;                  /* set to 1 if esc char (^\) typed */
#else
#ifdef V7
static int conesc = 0;
#else
#ifdef C70
static int conesc = 0;
#endif /* C70 */
#endif /* V7 */
#endif /* SVORPOSIX */
#endif /* OXOS */

static char ttnmsv[DEVNAMLEN];		/* Copy of open path for tthang */

#ifdef aegis
static status_$t st;                    /* error status return value */
static short concrp = 0;                /* true if console is CRP pad */
#define CONBUFSIZ 10
static char conbuf[CONBUFSIZ];          /* console readahead buffer */
static int  conbufn = 0;                /* # chars in readahead buffer */
static char *conbufp;                   /* next char in readahead buffer */
static uid_$t ttyuid;                   /* tty type uid */
static uid_$t conuid;                   /* stdout type uid */

/* APOLLO Aegis main()
 * establish acl usage and cleanup handling
 *    this makes sure that CRP pads
 *    get restored to a usable mode
 */
main(argc,argv) int argc; char **argv; {
        status_$t status;
        pfm_$cleanup_rec dirty;

        PID_T pid = getpid();

        /* acl usage according to invoking environment */
        default_acl(USE_DEFENV);

        /* establish a cleanup continuation */
        status = pfm_$cleanup(dirty);
        if (status.all != pfm_$cleanup_set) {
                /* only handle faults for the original process */
                if (pid == getpid() && status.all > pgm_$max_severity) {
		    /* blew up in main process */
		    status_$t quo;
		    pfm_$cleanup_rec clean;

		    /* restore the console in any case */
		    conres();

		    /* attempt a clean exit */
		    debug(F101, "cleanup fault status", "", status.all);

		    /* doexit(), then send status to continuation */
		    quo = pfm_$cleanup(clean);
		    if (quo.all == pfm_$cleanup_set)
		      doexit(pgm_$program_faulted,-1);
		    else if (quo.all > pgm_$max_severity)
		      pfm_$signal(quo); /* blew up in doexit() */
                }
                /* send to the original continuation */
                pfm_$signal(status);
                /*NOTREACHED*/
	    }
        return(ckcmai(argc, argv));
}
#endif /* aegis */

/* ANSI-style prototypes for internal functions. */
/* Functions used outside this module are prototyped in ckcker.h. */

#ifdef apollo
_PROTOTYP( SIGTYP timerh, () );
_PROTOTYP( SIGTYP cctrap, () );
_PROTOTYP( SIGTYP esctrp, () );
_PROTOTYP( SIGTYP sig_ign, () );
#else
_PROTOTYP( SIGTYP timerh, (int) );
_PROTOTYP( SIGTYP cctrap, (int) );
_PROTOTYP( SIGTYP esctrp, (int) );
#endif /* apollo */
_PROTOTYP( int do_open, (char *) );
_PROTOTYP( static int in_chk, (int) );
_PROTOTYP( static int ttrpid, (char *) );
_PROTOTYP( static int ttlock, (char *) );
_PROTOTYP( static int ttunlck, (void) );
_PROTOTYP( int mygetbuf, (void) );
_PROTOTYP( int myfillbuf, (void) );
_PROTOTYP( VOID conbgt, (int) );
#ifdef ACUCNTRL
_PROTOTYP( VOID acucntrl, (char *, char *) );
#endif /* ACUCNTRL */

#ifdef BSD44ORPOSIX
_PROTOTYP( int carrctl, (struct termios *, int) );
#else
#ifdef ATTSV
_PROTOTYP( int carrctl, (struct termio *, int) );
#else
_PROTOTYP( int carrctl, (struct sgttyb *, int) );
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */

#ifdef ATT7300
_PROTOTYP( int attdial, (char *, long, char *) );
_PROTOTYP( int offgetty, (char *) );
_PROTOTYP( int ongetty, (char *) );
#endif /* ATT7300 */

#ifdef BEBOX
#ifdef BE_DR_7
static double time_started = 0.0;
#include <kernel/OS.h>
struct ALARM_STRUCT {
    thread_id thread;
    int time;
};
static thread_id alarm_thread = -1;
static struct ALARM_STRUCT alarm_struct;
_PROTOTYP( long do_alarm, (void *) );
_PROTOTYP( unsigned int alarm, (unsigned int) );
_PROTOTYP( void alarm_expired, (void) );
#endif /* BE_DR_7 */
#endif /* BEBOX */

#ifdef CK_ANSIC
static char *
xxlast(char *s, char c)
#else
static char *
xxlast(s,c) char *s; char c;
#endif /* CK_ANSIC */
/* xxlast */ {		/*  Last occurrence of character c in string s. */
    int i;
    for (i = (int)strlen(s); i > 0; i--)
        if ( s[i-1] == c ) return( s + (i - 1) );
    return(NULL);
}

/* Timeout handler for communication line input functions */

SIGTYP
timerh(foo) int foo; {
    ttimoff();
#ifdef BEBOX
    alarm_expired();
#endif /* BEBOX */
#ifdef CK_POSIX_SIG
    siglongjmp(sjbuf,1);
#else
    longjmp(sjbuf,1);
#endif /* CK_POSIX_SIG */
}

/* Control-C trap for communication line input functions */

int cc_int;				/* Flag */
SIGTYP (* occt)();			/* For saving old SIGINT handler */

SIGTYP
cctrap(foo) int foo; {			/* Needs arg for ANSI C */
  cc_int = 1;				/* signal() prototype. */
  return;
}

/*  S Y S I N I T  --  System-dependent program initialization.  */

/*
 * ttgwsiz() returns:
 *	1    tt_rows and tt_cols are known, both altered, both > 0
 *	0    tt_rows and/or tt_cols are known, both altered, one or both <= 0
 *	-1   tt_rows and tt_cols are unknown and unaltered
 */

int
ttgwsiz() {
#ifndef NONAWS
    extern int tt_rows, tt_cols;

#ifdef QNX
/*
  NOTE: TIOCGWSIZ works here too, but only in the 32-bit version.
  This code works for both the 16- and 32-bit versions.
*/
    extern int dev_size(int, int, int, int *, int *);
    int r, c;

    if (dev_size(0, -1, -1, &r, &c) == 0) {
	debug(F101,"ttgwsiz QNX r","",r);
	debug(F101,"ttgwsiz QNX c","",c);
	tt_rows = r;
	tt_cols = c;
	return ((r > 0 && c > 0) ? 1 : 0);
    } else return(-1);
#else /* QNX */
#ifdef TIOCGWINSZ

/* Note, this was M_UNIX, changed to XENIX to allow cross compilation... */
#ifdef XENIX				/* SCO UNIX 3.2v4.0 */
#include <sys/stream.h>			/* typedef mblk_t needed by ptem.h */
#include <sys/ptem.h>			/* for ttgwsiz() */
#endif /* XENIX */

#ifdef I386IX				/* Ditto for Interactive */
#include <sys/stream.h>
#include <sys/ptem.h>
#endif /* I386IX */

/* Note, the above might be needed for some other older SVR3 Intel makes... */

    struct winsize w;
    if (ioctl(0, (int)TIOCGWINSZ, (char *)&w) < 0)
	return(-1);
    else if (w.ws_row > 0 && w.ws_col > 0) {
	tt_rows = w.ws_row;
	tt_cols = w.ws_col;
	debug(F101,"ttgwsiz tt_rows","",tt_rows);
	debug(F101,"ttgwsiz tt_cols","",tt_cols);
	return(1);
    } else return(0);
#endif /* TIOCGWINSZ */
#endif /* QNX */
#endif /* NONAWS */
}

int
sysinit() {
    int x;
/*
  BEFORE ANYTHING ELSE: Initialize the setuid package.
  Change to the user's real user and group ID.
  If this can't be done, don't run at all.
*/
    debug(F100,"sysinit calling priv_ini","",0);
    if (x = priv_ini()) {
	if (x & 1) fprintf(stderr,"Fatal: setuid failure.\n");
	if (x & 2) fprintf(stderr,"Fatal: setgid failure.\n");
	if (x & 4) fprintf(stderr,"Fatal: C-Kermit setuid to root!\n");
	exit(1);
    }
#ifndef NOJC
/*
  Get the initial job control state.
  If it is SIG_IGN, that means the shell does not support job control,
  and so we'd better not suspend ourselves.
*/
#ifdef SIGTSTP
    jchdlr = signal(SIGTSTP,SIG_IGN);
    if (jchdlr == SIG_IGN) {
	jcshell = 0;
	debug(F100,"sysinit jchdlr: SIG_IGN","",0);
    } else if (jchdlr == SIG_DFL) {
	debug(F100,"sysinit jchdlr: SIG_DFL","",0);
	jcshell = 1;
    } else {
	debug(F100,"sysinit jchdlr: other","",0);
	jcshell = 3;
    }
    (VOID) signal(SIGTSTP,jchdlr);	/* Put it back... */
#endif /* SIGTSTP */
#endif /* NOJC */

    conbgt(0);				/* See if we're in the background */
    congm();				/* Get console modes */

    (VOID) signal(SIGALRM,SIG_IGN);	/* Ignore alarms */
#ifndef SIGUSR1				/* User-defined signals */
#define SIGUSR1 30
#endif /* SIGUSR1 */

#ifndef SIGUSR2
#define SIGUSR2 31
#endif /* SIGUSR2 */

    (VOID) signal(SIGUSR1,SIG_IGN);	/* Ignore user-defined signals */
    (VOID) signal(SIGUSR2,SIG_IGN);	/*  */

#ifdef SIGDANGER			/* Ignore danger signals */
/*
  This signal is sent when the system is low on swap space.  Processes
  that don't handle it are candidates for termination.  If swap space doesn't
  clear out enough, we still might be terminated via kill() -- nothing we can
  do about that!  Conceivably, this could be improved by installing a real
  signal handler that warns the user, but that would be pretty complicated,
  since we are not always in control of the screen -- e.g. during remote-mode
  file transfer.
*/
    (VOID) signal(SIGDANGER,SIG_IGN);	/* e.g. on RS/6000 AIX */
#endif /* SIGDANGER */

#ifdef SIGPIPE
/*
  This one comes when a TCP/IP connection is broken by the remote.
  We prefer to catch this situation by examining error codes from write().
*/
    (VOID) signal(SIGPIPE,SIG_IGN);
#endif /* SIGPIPE */

#ifdef ultrix
    gtty(0,&vanilla);			/* Get sgtty info */
    iniflags = fcntl(0,F_GETFL,0);	/* Get flags */
#else
#ifdef AUX
    set42sig();				/* Don't ask! (hakanson@cs.orst.edu) */
#endif /* AUX */
#endif /* ultrix */

    ttgwsiz();				/* Get window (screen) dimensions. */

#ifdef Plan9
    if (!backgrd) {
    	consctlfd = open("/dev/consctl", O_WRONLY);
    	/*noisefd = open("/dev/noise", O_WRONLY)*/
    }
    ckxech = 1;
#endif /* Plan9 */

    return(0);
}

/*  S Y S C L E A N U P  --  System-dependent program cleanup.  */

int
syscleanup() {
#ifdef ultrix
    stty(0,&vanilla);                   /* Get sgtty info */
    fcntl(0,F_SETFL,iniflags);		/* Restore flags */
#endif /* ultrix */
    /* No need to call anything in the suid package here, right? */
    return(0);
}

/*  T T O P E N  --  Open a tty for exclusive access.  */

/*
  Call with:
    ttname: character string - device name or network host name.
    lcl:
  If called with lcl < 0, sets value of lcl as follows:
  0: the terminal named by ttname is the job's controlling terminal.
  1: the terminal named by ttname is not the job's controlling terminal.
  But watch out: if a line is already open, or if requested line can't
  be opened, then lcl remains (and is returned as) -1.
    modem:
  Less than zero: ttname is a network host name.
  Zero or greater: ttname is a terminal device name.    
  Zero means a local connection (don't use modem signals).
  Positive means use modem signals.  
   timo:
  0 = no timer.
  nonzero = number of seconds to wait for open() to return before timing out.

  Returns:
    0 on success
   -5 if device is in use
   -4 if access to device is denied
   -3 if access to lock directory denied
   -2 upon timeout waiting for device to open
   -1 on other error
*/
static int ttotmo = 0;			/* Timeout flag */
/* Flag kept here to avoid being clobbered by longjmp.  */

int
ttopen(ttname,lcl,modem,timo) char *ttname; int *lcl, modem, timo; {

#ifdef BSD44
#define ctermid(x) strcpy(x,"")
#else
#ifdef SVORPOSIX
#ifndef CIE
    extern char *ctermid();		/* Wish they all had this! */
#else					/* CIE Regulus */
#define ctermid(x) strcpy(x,"")
#endif /* CIE */
#endif /* SVORPOSIX */
#endif /* BSD44 */

#ifdef ultrix
    int temp = 0;
#endif /* ultrix */

    char *x;				/* Worker string pointer */

#ifndef MINIX
    extern char* ttyname();
#endif /* MINIX */
    char cname[DEVNAMLEN+4];

#ifndef pdp11
#define NAMEFD	 /* Feature to allow name to be an open file descriptor */
#endif /* pdp11 */

#ifdef NAMEFD
    char *p;
    debug(F101,"ttopen telnetfd","",telnetfd);
#endif /* NAMEFD */

    debug(F111,"ttopen entry modem",ttname,modem);
    debug(F101," ttyfd","",ttyfd);
    debug(F101," lcl","",*lcl);

#ifdef MAXNAMLEN
    debug(F100,"ttopen MAXNAMLEN defined","",0);
#else
    debug(F100,"ttopen MAXNAMLEN *NOT* defined","",0);
#endif

#ifdef BSD4
    debug(F100,"ttopen BSD4 defined","",0);
#else
    debug(F100,"ttopen BSD4 *NOT* defined","",0);
#endif

#ifdef BSD42
    debug(F100,"ttopen BSD42 defined","",0);
#else
    debug(F100,"ttopen BSD42 *NOT* defined","",0);
#endif /* BSD42 */

#ifdef MYREAD
    debug(F100,"ttopen MYREAD defined","",0);
#else
    debug(F100,"ttopen MYREAD *NOT* defined","",0);
#endif /* MYREAD */

    ttpmsk = 0xff;
    if (ttyfd > -1) {			/* if device already opened */
        if (strncmp(ttname,ttnmsv,DEVNAMLEN)) /* are new & old names equal? */
          ttclos(ttyfd);		/* no, close old ttname, open new */
        else 				/* else same, ignore this call, */
	  return(0);			/* and return. */
    }

#ifdef	NETCONN
    if (modem < 0) {			/* modem < 0 = special code for net */
	int x;
	ttmdm = modem;
	modem = -modem;			/* Positive network type number */
	fdflag = 0;			/* Stdio not redirected. */
	netconn = 1;			/* And it's a network connection */
	debug(F111,"ttopen net",ttname,modem);
#ifdef NAMEFD
	for (p = ttname; isdigit(*p); p++) ; /* Check for all digits */
 	if (*p == '\0' && (telnetfd || x25fd)) { /* Avoid X.121 addresses */
	    ttyfd = atoi(ttname);	/* Is there a way to test it's open? */
	    ttfdflg = 1;		/* We got an open file descriptor */
	    debug(F111,"ttopen got open network fd",ttname,ttyfd);
	    strncpy(ttnmsv,ttname,DEVNAMLEN); /* Remember the "name". */
	    x = 1;			/* Return code is "good". */
	    if (telnetfd) {
		ttnet = NET_TCPB;
		ttnproto = NP_TELNET;
#ifdef SUNX25
	    } else if (x25fd) {
		ttnet = NET_SX25;
		ttnproto = NP_NONE;
#endif /* SUNX25 */		
	    }
	} else {			/* Host name or address given */
#endif /* NAMEFD */
	    x = netopen(ttname, lcl, modem); /* (see ckcnet.h) */
	    if (x > -1) {
		strncpy(ttnmsv,ttname,DEVNAMLEN);
	    } else netconn = 0;
#ifdef NAMEFD
	}
#endif /* NAMEFD */

#ifdef sony_news			/* Sony NEWS */
	if (ioctl(ttyfd,TIOCKGET,&km_ext) < 0) { /* Get Kanji mode */
	    perror("ttopen error getting Kanji mode (network)");
	    debug(F111,"ttopen error getting Kanji mode","network",0);
	    km_ext = -1;		/* Make sure this stays undefined. */
	}
#endif /* sony_news */

	xlocal = *lcl = 1;		/* Network connections are local. */
	debug(F101,"ttopen net x","",x);
#ifdef COMMENT
/* Let netopen() do this */
	if (x > -1 && !x25fd)
	  x = tn_ini();			/* Initialize TELNET protocol */
#endif /* COMMENT */
	return(x);
    } else {				/* Terminal device */
#endif	/* NETCONN */

#ifdef NAMEFD
/*
  This code lets you give Kermit an open file descriptor for a serial
  communication device, rather than a device name.  Kermit assumes that the
  line is already open, locked, conditioned with the right parameters, etc.
*/
	for (p = ttname; isdigit(*p); p++) ; /* Check for all digits */
	if (*p == '\0') {
	    ttyfd = atoi(ttname);	/* Is there a way to test it's open? */
	    debug(F111,"ttopen got open fd",ttname,ttyfd);
	    strncpy(ttnmsv,ttname,DEVNAMLEN); /* Remember the "name". */
	    xlocal = *lcl = 1;		/* Assume it's local. */
	    netconn = 0;		/* Assume it's not a network. */
	    tvtflg = 0;			/* Might need to initialize modes. */
	    ttmdm = modem;		/* Remember modem type. */
	    fdflag = 0;			/* Stdio not redirected. */
	    ttfdflg = 1;		/* Flag we were opened this way. */

#ifdef sony_news			/* Sony NEWS */
	    /* Get device Kanji mode */
	    if (ioctl(ttyfd,TIOCKGET,&km_ext) < 0) {
		perror("ttopen error getting Kanji mode");
		debug(F101,"ttopen error getting Kanji mode","",0);
		km_ext = -1;		/* Make sure this stays undefined. */
	    }
#endif /* sony_news */
	    return(0);			/* Return success */
	}
#endif /* NAMEFD */
#ifdef NETCONN
    }
#endif /* NETCONN */

/* Here we have to open a serial device of the given name. */

    occt = signal(SIGINT, cctrap);	/* Set Control-C trap, save old one */

    tvtflg = 0;			/* Flag for use by ttvt(). */
				/* 0 = ttvt not called yet for this device */

    fdflag = (!isatty(0) || !isatty(1)); /* Flag for stdio redirected */
    debug(F101,"ttopen fdflag","",fdflag);

    ttmdm = modem;                      /* Make this available to other fns */
    xlocal = *lcl;                      /* Make this available to other fns */

/* Code for handling bidirectional tty lines goes here. */
/* Use specified method for turning off logins and suppressing getty. */

#ifdef ACUCNTRL
    /* Should put call to priv_on() here, but that would be very risky! */
    acucntrl("disable",ttname);         /* acucntrl() program. */
    /* and priv_off() here... */
#else
#ifdef ATT7300
    if ((attmodem & DOGETY) == 0)       /* offgetty() program. */
      attmodem |= offgetty(ttname);	/* Remember response.  */
#endif /* ATT7300 */
#endif /* ACUCNTRL */

/*
 In the following section, we open the tty device for read/write.
 If a modem has been specified via "set modem" prior to "set line"
 then the O_NDELAY parameter is used in the open, provided this symbol
 is defined (e.g. in fcntl.h), so that the program does not hang waiting
 for carrier (which in most cases won't be present because a connection
 has not been dialed yet).  O_NDELAY is removed later on in ttopen().  It
 would make more sense to first determine if the line is local before
 doing this, but because ttyname() requires a file descriptor, we have
 to open it first.  See do_open().

 Now open the device using the desired treatment of carrier.
 If carrier is REQUIRED, then open could hang forever, so an optional
 timer is provided.  If carrier is not required, the timer should never
 go off, and should do no harm...
*/
    ttotmo = 0;				/* Flag no timeout */
    if (timo > 0) {
	int xx;
	saval = signal(SIGALRM,timerh);	/* Timed, set up timer. */
	xx = alarm(timo);		/* Timed open() */
	debug(F101,"ttopen alarm","",xx);
	if (
#ifdef CK_POSIX_SIG
	    sigsetjmp(sjbuf,1)
#else
	    setjmp(sjbuf)
#endif /* CK_POSIX_SIG */
	    ) {
	    ttotmo = 1;			/* Flag timeout. */
	} else ttyfd = do_open(ttname);
	ttimoff();
	debug(F111,"ttopen","modem",modem);
	debug(F101," ttyfd","",ttyfd);
	debug(F101," alarm return","",ttotmo);
    } else ttyfd = do_open(ttname);
    debug(F111,"ttopen ttyfd",ttname,ttyfd);
    if (ttyfd < 0) {			/* If couldn't open, fail. */
#ifdef ATT7300
	if (attmodem & DOGETY)		/* was getty(1m) running before us? */
	  ongetty(ttnmsv);		/* yes, restart on tty line */
	attmodem &= ~DOGETY;		/* no phone in use, getty restored */
#else
#ifdef ACUCNTRL
        /* Should put call to priv_on() here, but that would be risky! */
	acucntrl("enable",ttname);	/* acucntrl() program. */	
	/* and priv_off() here... */
#endif /* ACUNTRL */
#endif /* ATT7300 */

	signal(SIGINT,occt);		/* Put old Ctrl-C trap back. */
	if (errno == EACCES) {		/* Device is protected against user */
	    perror(ttname);		/* Print message */
	    debug(F111,"ttopen tty access denied",ttname,errno);
	    return(-4);
	} else return(ttotmo ? -2 : -1);
    }

#ifdef Plan9
    /* take this opportunity to open the control channel */
    if (p9openttyctl(ttname) < 0)
#else
    /* Make sure it's a real tty. */
    if (!isatty(ttyfd) && strcmp(ttname,"/dev/null"))
#endif /* Plan9 */
      {
	fprintf(stderr,"%s is not a terminal device\n",ttname);
	debug(F110,"ttopen not a tty",ttname,0);
	close(ttyfd);
	ttyfd = -1;
	signal(SIGINT,occt);
	return(-1);
    }

#ifdef aegis
	/* Apollo C runtime claims that console pads are tty devices, which
	 * is reasonable, but they aren't any good for packet transfer. */
	ios_$inq_type_uid((short)ttyfd, ttyuid, st);
	if (st.all != status_$ok) {
	    fprintf(stderr, "problem getting tty object type: ");
	    error_$print(st);
	} else if (ttyuid != sio_$uid) { /* reject non-SIO lines */
	    close(ttyfd); ttyfd = -1;
	    errno = ENOTTY; perror(ttname);
	    signal(SIGINT,occt);
	    return(-1);
	}
#endif /* aegis */

    strncpy(ttnmsv,ttname,DEVNAMLEN);   /*  Keep copy of name locally. */

/* Caller wants us to figure out if line is controlling tty */

    if (*lcl < 0) {
	int x0 = 0, x1 = 0;
        if (strcmp(ttname,CTTNAM) == 0) {   /* "/dev/tty" always remote */
            xlocal = 0;
	    debug(F111," ttname=CTTNAM",ttname,xlocal);

    /* If any of 0, 1, or 2 not redirected, we can use ttyname() to get */
    /* the name of the controlling terminal... */

/*
  Warning: on some UNIX systems (SVR4?), ttyname() reportedly opens /dev but
  never closes it.  If it is called often enough, we run out of file
  descriptors and subsequent open()'s of other devices or files can fail.
*/

        } else if ((x0 = isatty(0)) || (x1 = isatty(1)) || isatty(2)) {
#ifndef MINIX
	    if (x0)
	      x = ttyname(0);		/* and compare it with the */
	    else if (x1)		/* tty device name. */
	      x = ttyname(1);
	    else x = ttyname(2);
            strncpy(cname,x,DEVNAMLEN); /* (copy from internal static buf) */
	    debug(F110," cname",x,0);
#ifdef BEBOX
            x = ttnmsv;			/* ttyname() is broken */
#else
            x = ttyname(ttyfd);         /* Get real name of ttname. */
#endif /* BEBOX */		
	    if (x) {
		if (*x) {
		    xlocal = (strncmp(x,cname,DEVNAMLEN) == 0) ? 0 : 1;
		    debug(F111," ttyname",x,xlocal);
		}
	    } else xlocal = 1;
#else
	    xlocal = 1;			/* Can't do this test in MINIX */
#endif /* MINIX */
        } else {                        /* Else, if stdin redirected... */
#ifdef SVORPOSIX
/* System V provides nice ctermid() function to get name of controlling tty */
            ctermid(cname);             /* Get name of controlling terminal */
            debug(F110," ctermid",cname,0);

            x = ttyname(ttyfd);         /* Compare with name of comm line. */
#ifdef BEBOX
            xlocal = 1;			/* ttyname always returns /dev/tty */
#else
            xlocal = (strncmp(x,cname,DEVNAMLEN) == 0) ? 0 : 1;
#endif /* BEBOX */		
            debug(F111," ttyname",x,xlocal);
#else
/* Just assume local */
            xlocal = 1;
#endif /* SVORPOSIX */
            debug(F101," redirected stdin","",xlocal);
        }
    }

#ifndef NOFDZERO
/* Note, the following code was added so that Unix "idle-line" snoopers */
/* would not think Kermit was idle when it was transferring files, and */
/* maybe log people out. */
    if (xlocal == 0) {			/* Remote mode */
	if (fdflag == 0) {		/* Standard i/o is not redirected */
	    debug(F100,"ttopen setting ttyfd = 0","",0);
#ifdef LYNXOS
	    /* On Lynx OS, fd 0 is open for read only. */
	    dup2(ttyfd,0);
#endif /* LYNXOS */
	    close(ttyfd);		/* Use file descriptor 0 */
	    ttyfd = 0;
	} else {			/* Standard i/o is redirected */
	    debug(F101,"ttopen stdio redirected","",ttyfd);
	}
    }
#endif /* NOFDZERO */

/* Now check if line is locked -- if so fail, else lock for ourselves */
/* Note: After having done this, don't forget to delete the lock if you */
/* leave ttopen() with an error condition. */

    lkf = 0;                            /* Check lock */
    if (xlocal > 0) {
	int xx; int xpid;
        if ((xx = ttlock(ttname)) < 0) { /* Can't lock it. */
            debug(F111,"ttopen ttlock fails",ttname,xx);
	    /* WARNING - This close() can hang if tty is an empty socket... */
            close(ttyfd);		/* Close the device. */
	    ttyfd = -1;			/* Erase its file descriptor. */
	    signal(SIGINT,occt);	/* Put old SIGINT back. */
	    if (xx == -2) {		/* If lockfile says device in use, */
#ifndef NOUUCP
		debug(F111,"ttopen reading lockfile pid",flfnam,xx);
		priv_on();
		xpid = ttrpid(flfnam);	/* Try to read pid from lockfile */
		priv_off();		/* Turn privs back off. */
		if (xpid > -1) {	/* If we got a pid */
		    printf("Locked by process %d\n",xpid); /* tell them. */
		} else if (*flfnam) {
		    char *p = malloc(280); /* Print a directory listing. */
/*
  Note: priv_on() won't help here, because we do not pass privs along to
  to inferior processes, in this case ls.  So if the real user does not have
  directory-listing access to the lockfile directory, this will result in
  something like "not found".  That's why we try this only as a last resort.
*/
		    if (p) {		/* If we got the space... */
			extern char *DIRCMD;
			sprintf(p,"%s %s",DIRCMD,flfnam);
			zsyscmd(p);	/* Get listing. */
			if (p) {	/* free the space */
			    free(p);
			    p = NULL;
			}
		    }
		}
#endif /* NOUUCP */
		return(-5);		/* Code for device in use */
	    } else return(-3);		/* Access denied */
        } else lkf = 1;
    }

/* Got the line, now set the desired value for local. */

    if (*lcl != 0) *lcl = xlocal;

/* Some special stuff for v7... */

#ifdef  V7
#ifndef MINIX
    if (kmem[TTY] < 0) {		/*  If open, then skip this.  */
	qaddr[TTY] = initrawq(ttyfd);   /* Init the queue. */
	if ((kmem[TTY] = open("/dev/kmem", 0)) < 0) {
	    fprintf(stderr, "Can't read /dev/kmem in ttopen.\n");
	    perror("/dev/kmem");
	    exit(1);
	}
    }
#endif /* !MINIX */
#endif /* V7 */

/* No failure returns after this point */

#ifdef ultrix
    ioctl(ttyfd, TIOCMODEM, &temp);
#ifdef TIOCSINUSE
    if (xlocal && ioctl(ttyfd, TIOCSINUSE, NULL) < 0) {
	fprintf(stderr, "Can't set in-use flag on %s.\n",ttname);
	perror("TIOCSINUSE");
    }
#endif /* TIOCSINUSE */
#endif /* ultrix */

/* Get tty device settings */

#ifdef BSD44ORPOSIX			/* POSIX */
    tcgetattr(ttyfd,&ttold);
    debug(F101,"ttopen tcgetattr ttold.c_lflag","",ttold.c_lflag);
    tcgetattr(ttyfd,&ttraw);
    tcgetattr(ttyfd,&tttvt);
#else					/* BSD, V7, and all others */
#ifdef ATTSV				/* AT&T UNIX */
    ioctl(ttyfd,TCGETA,&ttold);
    debug(F101,"ttopen ioctl TCGETA ttold.c_lflag","",ttold.c_lflag);
    ioctl(ttyfd,TCGETA,&ttraw);
    ioctl(ttyfd,TCGETA,&tttvt);
#else
#ifdef BELLV10
    ioctl(ttyfd,TIOCGETP,&ttold);
    debug(F101,"ttopen BELLV10 ttold.sg_flags","",ttold.sg_flags);
    ioctl(ttyfd,TIOCGDEV,&tdold);
    debug(F101,"ttopen BELLV10 tdold.flags","",tdold.flags);
#else
    gtty(ttyfd,&ttold);
    debug(F101,"ttopen gtty ttold.sg_flags","",ttold.sg_flags);
#endif /* BELLV10 */

#ifdef sony_news			/* Sony NEWS */
    if (ioctl(ttyfd,TIOCKGET,&km_ext) < 0) { /* Get console Kanji mode */
	perror("ttopen error getting Kanji mode");
	debug(F101,"ttopen error getting Kanji mode","",0);
	km_ext = -1;			/* Make sure this stays undefined. */
    }
#endif /* sony_news */

#ifdef TIOCGETC
    tcharf = 0;				/* In remote mode, also get */
    if (xlocal == 0) {			/* special characters */
	if (ioctl(ttyfd,TIOCGETC,&tchold) < 0) {
	    debug(F100,"ttopen TIOCGETC failed","",0);
	} else {
	    tcharf = 1;			/* It worked. */
	    ioctl(ttyfd,TIOCGETC,&tchnoi); /* Get another copy */
	    debug(F100,"ttopen TIOCGETC ok","",0);
	}
    }	
#else
    debug(F100,"ttopen TIOCGETC not defined","",0);
#endif /* TIOCGETC */

#ifdef TIOCGLTC
    ltcharf = 0;			/* In remote mode, also get */
    if (xlocal == 0) {			/* local special characters */
	if (ioctl(ttyfd,TIOCGLTC,&ltchold) < 0) {
	    debug(F100,"ttopen TIOCGLTC failed","",0);
	} else {
	    ltcharf = 1;		/* It worked. */
	    ioctl(ttyfd,TIOCGLTC,&ltchnoi); /* Get another copy */
	    debug(F100,"ttopen TIOCGLTC ok","",0);
	}
    }	
#else
    debug(F100,"ttopen TIOCGLTC not defined","",0);
#endif /* TIOCGLTC */

#ifdef TIOCLGET
    lmodef = 0;
    if (ioctl(ttyfd,TIOCLGET,&lmode) < 0) {
	debug(F100,"ttopen TIOCLGET failed","",0);
    } else {
	lmodef = 1;
	debug(F100,"ttopen TIOCLGET ok","",0);
    }
#endif /* TIOCLGET */

#ifdef BELLV10
    ioctl(ttyfd,TIOCGETP,&ttraw);
    ioctl(ttyfd,TIOCGETP,&tttvt);
#else
    gtty(ttyfd,&ttraw);                 /* And a copy of it for packets*/
    gtty(ttyfd,&tttvt);                 /* And one for virtual tty service */
#endif /* BELLV10 */

#endif /* ATTSV */
#endif /* BSD44ORPOSIX */

/* Section for changing line discipline.  It's restored in ttres(). */

#ifdef AIXRS
#ifndef AIX41
    { union txname ld_name; int ld_idx = 0;
      ttld = 0;
        do {
  	  ld_name.tx_which = ld_idx++;
  	  ioctl(ttyfd, TXGETCD, &ld_name);
	  if (!strncmp(ld_name.tx_name, "rts", 3))
  	    ttld |= 1;
        } while (*ld_name.tx_name);
        debug(F101,"AIX line discipline","",ttld);
      }
#endif /* AIX41 */
#endif /* AIXRS */

#ifdef BSD41
/* For 4.1BSD only, force "old" tty driver, new one botches TANDEM. */
    { int k;
      ioctl(ttyfd, TIOCGETD, &ttld);	/* Get and save line discipline */
      debug(F101,"4.1bsd line discipline","",ttld);
      k = OTTYDISC;			/* Switch to "old" discipline */
      k = ioctl(ttyfd, TIOCSETD, &k);
      debug(F101,"4.1bsd tiocsetd","",k);
    }
#endif /* BSD41 */

#ifdef aegis
    /* This was previously done before the last two TCGETA or gtty above,
     * in both the ATTSV and not-ATTSV case.  If it is not okay to have only
     * one copy if it here instead, give us a shout!
     */
    sio_$control((short)ttyfd, sio_$raw_nl, false, st);
    if (xlocal) {       /* ignore breaks from local line */
        sio_$control((short)ttyfd, sio_$int_enable, false, st);
        sio_$control((short)ttyfd, sio_$quit_enable, false, st);
    }
#endif /* aegis */

#ifdef VXVE
    ttraw.c_line = 0;                   /* STTY line 0 for VX/VE */
    tttvt.c_line = 0;                   /* STTY line 0 for VX/VE */
    ioctl(ttyfd,TCSETA,&ttraw);
#endif /* vxve */

/* If O_NDELAY was used during open(), then remove it now. */

#ifdef O_NDELAY
    if (fcntl(ttyfd, F_GETFL, 0) & O_NDELAY) {

#ifndef aegis
	if (fcntl(ttyfd,F_SETFL, fcntl(ttyfd, F_GETFL, 0) & ~O_NDELAY) < 0 )
	    perror("Can't unset O_NDELAY");
#endif /* aegis */
	/* Some systems, notably Xenix (don't know how common this is in
	 * other systems), need special treatment to get rid of the O_NDELAY
	 * behaviour on read() with respect to carrier presence (i.e. read()
	 * returning 0 when carrier absent), even though the above fcntl()
	 * is enough to make read() wait for input when carrier is present.
	 * This magic, in turn, requires CLOCAL for working when the carrier
	 * is absent. But if xlocal == 0, presumably you already have CLOCAL
	 * or you have a carrier, otherwise you wouldn't be running this.
	 */
#ifdef ATTSV
#ifdef BSD44
#ifdef __bsdi__
	if (xlocal)
	  ttraw.c_cflag |= CLOCAL;
#else
#ifdef __FreeBSD__
	if (xlocal)
	  ttraw.c_cflag |= CLOCAL;
#endif /* __FreeBSD__ */
#endif /* __bsdi__ */
	if (tcsetattr(ttyfd, TCSADRAIN, &ttraw) < 0)
	  perror("tcsetattr");
#else /* !BSD44 */
	if (xlocal) {
	    ttraw.c_cflag |= CLOCAL;
	    ioctl(ttyfd, TCSETA, &ttraw);
	}
#endif /* BSD44 */
#endif /* ATTSV */
#ifndef NOCOTFMC /* = NO Close(Open()) To Force Mode Change */
/* Reportedly lets uugetty grab the device in SCO UNIX 3.2 / XENIX 2.3 */
	close( priv_opn(ttname, O_RDWR) ); /* Magic to force change. */
#endif /* NOCOTFMC */
    }
#endif /* O_NDELAY */

/* Instruct the system how to treat the carrier, and set a few other tty
 * parameters.
 *
 * This also undoes the temporary setting of CLOCAL that may have been done
 * for the close(open()) above (except in Xenix).  Also throw in ~ECHO, to
 * prevent the other end of the line from sitting there talking to itself,
 * producing garbage when the user performs a connect.
 *
 * SCO Xenix unfortunately seems to ignore the actual state of CLOCAL.
 * Now it thinks CLOCAL is always on. It seems the only real solution for
 * Xenix is to switch between the lower and upper case device names.
 *
 * This section may at some future time expand into setting a complete
 * collection of tty parameters, or call a function shared with ttpkt()/
 * ttvt() that does so.  On the other hand, the initial parameters are not
 * that important, since ttpkt() or ttvt() should always fix that before
 * any communication is done.  Well, we'll see...
 */
    if (xlocal) {
    	curcarr = -2;
	carrctl(&ttraw, ttcarr == CAR_ON);

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifdef SVORPOSIX
	ttraw.c_lflag &= ~ECHO;
	ttold.c_lflag &= ~ECHO;
#ifdef BSD44ORPOSIX
	tcsetattr(ttyfd, TCSADRAIN, &ttraw);
#else
	ioctl(ttyfd, TCSETA, &ttraw);
#endif /* BSD44ORPOSIX */

#else /* BSD, etc */
	ttraw.sg_flags &= ~ECHO;
	ttold.sg_flags &= ~ECHO;
#ifdef BELLV10
	ioctl(ttyfd,TIOCSETP,&ttraw);
#else
	stty(ttyfd,&ttraw);
#endif /* BELLV10 */
#endif /* SVORPOSIX */

#ifdef COHERENT
#undef SVORPOSIX
#endif /* COHERENT */

	/* ttflui(); */  /*  This fails for some reason.  */
    }
    
    /* Get current speed */

#ifndef BEBOX
    ttspeed = ttgspd();
#else
    ttspeed = 19200;
#endif /* !BEBOX */
    debug(F101,"ttopen ttspeed","",ttspeed);

    /* Done, make entries in debug log, restore Ctrl-C trap, and return. */

    debug(F101,"ttopen, ttyfd","",ttyfd);
    debug(F101," lcl","",*lcl);
    debug(F111," lock file",flfnam,lkf);
    signal(SIGINT,occt);
    return(0);
}


/*  D O _ O P E N  --  Do the right kind of open() call for the tty. */

int
do_open(ttname) char *ttname; {

#ifndef	O_NDELAY			/* O_NDELAY not defined */
    return(priv_opn(ttname,2));
#else					/* O_NDELAY defined */

#ifdef ATT7300
/*
 Open comms line without waiting for carrier so initial call does not hang
 because state of "modem" is likely unknown at the initial call  -jrd.
 If this is needed for the getty stuff to work, and the open would not work
 without O_NDELAY when getty is still on, then this special case is ok.
 Otherwise, get rid of it. -ske
*/
    return(priv_opn(ttname, O_RDWR | O_NDELAY));

#else	/* !ATT7300 */

    /* Normal case. Use O_NDELAY according to SET CARRIER. See ttscarr(). */
  
    return(priv_opn(ttname, O_RDWR | ((ttcarr != CAR_ON) ? O_NDELAY : 0) ));

#endif	/* !ATT7300 */
#endif	/* O_NDELAY */
}

/*  T T C L O S  --  Close the TTY, releasing any lock.  */

int
ttclos(foo) int foo; {			/* Arg req'd for signal() prototype */
    int xx, x = 0;

    debug(F101,"ttclos ttyfd","",ttyfd);
    if (ttyfd < 0) return(0);           /* Wasn't open. */

    if (ttfdflg)			/* If we inherited ttyfd from */
      return(0);			/* another process, don't close it. */

    tvtflg = 0;				/* (some day get rid of this...) */

#ifdef	NETCONN
    if (netconn) {			/* If it's a network connection. */
	debug(F100,"ttclos closing net","",0);
	netclos();			/* Let the network module close it. */
	netconn = 0;			/* No more network connection. */
	debug(F101,"ttclos ttyfd after netclos","",ttyfd);
	return(0);
    }
#endif	/* NETCONN */

    if (xlocal) {			/* We're closing a SET LINE device */
#ifdef FT21				/* Fortune 2.1-specific items ... */
	ioctl(ttyfd,TIOCHPCL, NULL);
#endif /* FT21 */
#ifdef ultrix				/* Ultrix-specific items ... */
#ifdef TIOCSINUSE
	/* Unset the INUSE flag that we set in ttopen() */
	ioctl(ttyfd, TIOCSINUSE, NULL);
#endif /* TIOCSINUSE */
	ioctl(ttyfd, TIOCNMODEM, &x);
#ifdef COMMENT
	/* What was this? */
	ioctl(ttyfd, TIOCNCAR, NULL);
#endif /* COMMENT */
#endif /* ultrix */
	debug(F100,"ttclos about to call ttunlck","",0);
        if (ttunlck())                  /* Release uucp-style lock */
	  fprintf(stderr,"Warning, problem releasing lock\r\n");

    }
    if (
	xlocal
#ifdef NOFDZERO
	|| ttyfd > 0
#endif /* NOFDZERO */
	) {
	saval = signal(SIGALRM,timerh); /* Enable timer interrupt. */
	xx = alarm(8);			/* Allow 8 seconds for this. */
	debug(F101,"ttclos alarm","",xx);
	if (
#ifdef CK_POSIX_SIG
	    sigsetjmp(sjbuf,1)
#else
	    setjmp(sjbuf)
#endif /* CK_POSIX_SIG */
	    ) {				/* Timer went off? */
	    x = -1;
	} else {			/* What we're really trying to do */
	    debug(F100,"ttclos calling tthang()","",0);
	    tthang();			/* Hang up first, then... */
	    debug(F100,"ttclos calling ttres()","",0);
	    ttres();			/* Reset device modes. */
	}
	debug(F101,"ttclos about to call close","",ttyfd);
	close(ttyfd);			/* Close the device. */
	x = 1;
	ttimoff();			/* Turn off timer. */
	if (x < 0) {
	    fprintf(stderr,"?Timed out closing device: %s\n",ttnmsv);
	    debug(F100,"ttclos timed out","",0);
	}
    }

/* For bidirectional lines, restore getty if it was there before. */

    if (xlocal) {

#ifdef ACUCNTRL				/* 4.3BSD acucntrl() method. */
	acucntrl("enable",ttnmsv);	/* Enable getty on the device. */
#else
#ifdef ATT7300				/* ATT UNIX PC (3B1, 7300) method. */
	if (attmodem & DOGETY)		/* Was getty(1m) running before us? */
	  ongetty(ttnmsv);		/* Yes, restart getty on tty line */
	attmodem &= ~DOGETY;		/* No phone in use, getty restored */
#endif /* ATT7300 */
#endif /* System-dependent getty-restoring methods */
    }

#ifdef sony_news
    km_ext = -1;			/* Invalidate device's Kanji-mode */
#endif /* sony_news */

    ttyfd = -1;                         /* Invalidate the file descriptor. */
    debug(F100,"ttclos done","",0);
    return(0);
}

/*  T T H A N G  --  Hangup phone line or network connection.  */
/*
  Returns:
  0 if it does nothing.
  1 if it believes that it hung up successfully.
 -1 if it believes that the hangup attempt failed.
*/

#define HUPTIME 500			/* Milliseconds for hangup */

int
tthang() {
#ifdef NOLOCAL
    return(0);
#else
    int x = 0;				/* Sometimes used as return code. */
#ifndef POSIX
    int z;				/* worker */
#endif /* POSIX */

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifdef SVORPOSIX			/* AT&T, POSIX, HPUX declarations. */
    int spdsav;				/* for saving speed */
#ifdef BSD44ORPOSIX
    int spdsavi;
#endif /* BSD44ORPOSIX */
#ifdef HPUX
/*
  Early versions of HP-UX omitted the mflag typedef.  If you get complaints
  about it, just change it to long (or better still, unsigned long).
*/
    mflag
      dtr_down = 00000000000,
      modem_rtn,
      modem_sav;
    char modem_state[64];
#endif /* HPUX */
    int flags;				/* fcntl flags */
    unsigned short ttc_save;
#endif /* SVORPOSIX */

#ifdef COHERENT
#undef SVORPOSIX
#endif /* COHERENT */

    if (ttyfd < 0) return(0);           /* Don't do this if not open  */
    if (xlocal < 1) return(0);		/* Don't do this if not local */

#ifdef NETCONN
    if (netconn)			/* Network connection. */
      return((netclos() < 0) ? -1 : 1);	/* Just close it. */
#endif /* NETCONN */

/* From here down, we handle real tty devices. */

#ifdef BSD44ORPOSIX
/* Should add some error checking here... */
/*
  NOTE: This should work for QNX too, but in case it doesn't, it is also
  possible to toggle DTR using qnx_ioctl(ttyfd,QCTL_DEV_CTL,&new,&old);
  see <sys/qioctl.h> and QNX News V10, Sep 96, p44.
*/
    debug(F100,"tthang POSIX style","",0);
    tcgetattr(ttyfd, &ttcur);		/* Get current attributes */
    spdsav = cfgetospeed(&ttcur);	/* Get current speed */
    spdsavi = cfgetispeed(&ttcur);	/* Get current speed */
    cfsetospeed(&ttcur,B0);		/* Replace by 0 */
    cfsetispeed(&ttcur,B0);
    if (tcsetattr(ttyfd,TCSADRAIN,&ttcur) == -1)
      debug(F100,"tthang tcsetattr fails","",errno);
    msleep(HUPTIME);			/* Sleep */
    cfsetospeed(&ttcur,spdsav);		/* Restore previous speed */
    cfsetispeed(&ttcur,spdsavi);
    tcsetattr(ttyfd,TCSADRAIN,&ttcur);
    return(1);

#else /* BSD44ORPOSIX */

#ifdef aegis				/* Apollo Aegis */
    sio_$control((short)ttyfd, sio_$dtr, false, st);    /* DTR down */
    msleep(HUPTIME);					/* pause */
    sio_$control((short)ttyfd, sio_$dtr, true,  st);    /* DTR up */
    return(1);
#endif /* aegis */

#ifdef ANYBSD				/* Any BSD version. */
#ifdef TIOCCDTR				/* Except those that don't have this */
    debug(F100,"tthang BSD style","",0);
    if (ioctl(ttyfd,TIOCCDTR,0) < 0) {	/* Clear DTR. */
	debug(F101,"tthang TIOCCDTR fails","",errno);
	return(-1);
    }
    msleep(HUPTIME);			/* For about 1/2 sec */
    errno = 0;
    x = ioctl(ttyfd,TIOCSDTR,0);	/* Restore DTR */
    if (x < 0) {
	/*
	  For some reason, this tends to fail with "no such device or address"
	  but the operation still works, probably because of the close/open
	  later on.  So let's not scare the user unnecessarily here.
	*/
	debug(F101,"tthang TIOCSDTR errno","",errno); /* Log the error */
	x = 1;				/* Pretend we succeeded */
    } else if (x == 0) x = 1;		/* Success */
#ifdef COMMENT
#ifdef FT21
    ioctl(ttyfd, TIOCSAVEMODES, 0);
    ioctl(ttyfd, TIOCHPCL, 0);
    close(ttyfd);			/* Yes, must do this twice */
    if ((ttyfd = open(ttnmsv,2)) < 0)	/* on Fortune computers... */
      return(-1);			/* (but why?) */
    else x = 1;
#endif /* FT21 */
#endif /* COMMENT */
#endif /* TIOCCDTR */
    close(do_open(ttnmsv));		/* Clear i/o error condition */
    errno = 0;
#ifdef COMMENT
/* This is definitely dangerous.  Why was it here? */
    z = ttvt(ttspeed,ttflow);		/* Restore modes. */
    debug(F101,"tthang ttvt returns","",z);
    return(z < 0 ? -1 : 1);
#else
    return(x);
#endif /* COMMENT */
#endif /* ANYBSD */

#ifdef ATTSV				
/* AT&T UNIX section, includes HP-UX and generic AT&T System III/V... */

#ifdef HPUX 
/* Hewlett Packard allows explicit manipulation of modem signals. */

#ifdef COMMENT
/* Old way... */
    debug(F100,"tthang HP-UX style","",0);
    if (ioctl(ttyfd,MCSETAF,&dtr_down) < 0)        /* lower DTR */
      return(-1);		    	           /* oops, can't. */
    msleep(HUPTIME);			           /* Pause half a second. */
    x = 1;				           /* Set return code */
    if (ioctl(ttyfd,MCGETA,&modem_rtn) > -1) {     /* Get line status. */
	if ((modem_rtn & MDCD) != 0)      	   /* Check if CD is low. */
	  x = -1;                                  /* CD didn't drop, fail. */
    } else x = -1;

    /* Even if above calls fail, RTS & DTR should be turned back on. */
    modem_rtn = MRTS | MDTR;
    if (ioctl(ttyfd,MCSETAF,&modem_rtn) < 0) x = -1;
    return(x);
#else
/* New way, from Hellmuth Michaelis */
    debug(F100,"tthang HP-UX style, HPUXDEBUG","",0);
    if (ioctl(ttyfd,MCGETA,&modem_rtn) == -1) { /* Get current status. */
	debug(F100,"tthang HP-UX: can't get modem lines, NO HANGUP!","",0);
	return(-1);
    }
    sprintf(modem_state,"%#lx",modem_rtn);
    debug(F110,"tthang HP-UX: modem lines = ",modem_state,0); 
    modem_sav = modem_rtn;		/* Save current modem signals */
    modem_rtn &= ~MDTR;			/* Turn DTR bit off */
    sprintf(modem_state,"%#lx",modem_rtn);
    debug(F110,"tthang HP-UX: DTR down = ",modem_state,0); 
    if (ioctl(ttyfd,MCSETAF,&modem_rtn) < 0) { /* lower DTR */
	debug(F100,"tthang HP-UX: can't lower DTR!","",0);
	return(-1);			/* oops, can't. */
    }
    msleep(HUPTIME);			/* Pause half a second. */
    x = 1;				/* Set return code */
    if (ioctl(ttyfd,MCGETA,&modem_rtn) > -1) { /* Get line status. */
	sprintf(modem_state,"%#lx",modem_rtn);
	debug(F110,"tthang HP-UX: modem lines got = ",modem_state,0);    
	if ((modem_rtn & MDCD) != 0) {	/* Check if CD is low. */
	    debug(F100,"tthang HP-UX: DCD not down","",0);
	    x = -1;			/* CD didn't drop, fail. */
	} else {
	    debug(F100,"tthang HP-UX: DCD down","",0);
	}
    } else {
	x = -1;
	debug(F100,"tthang HP-UX: can't get DCD status !","",0);
    }

    /* Even if above calls fail, DTR should be turned back on. */

    modem_sav |= MDTR;
    if (ioctl(ttyfd,MCSETAF,&modem_sav) < 0) {
	x = -1;
	debug(F100,"tthang HP-UX: can't set saved state","",0);
    } else {
	sprintf(modem_state,"%#lx",modem_sav);
	debug(F110,"tthang HP-UX: final modem lines = ",modem_state,0);    
    }    	
    return(x);
#endif /* COMMENT */

#else /* AT&T but not HP-UX */

/* SVID for AT&T System V R3 defines ioctl's for handling modem signals. */
/* It is not known how many, if any, systems actually implement them, */
/* so we include them here in ifdef's. */

#ifndef _IBMR2
/*
  No modem-signal twiddling for IBM RT PC or RS/6000.
  In AIX 3.1 and earlier, the ioctl() call is broken.
  This code could be activated for AIX 3.1 with PTF 2006 or later
  (e.g. AIX 3.2), but close/open does the job too, so why bother.
*/
#ifdef TIOCMBIS				/* Bit Set */
#ifdef TIOCMBIC				/* Bit Clear */
#ifdef TIOCM_DTR			/* DTR */

/* Clear DTR, sleep 300 msec, turn it back on. */
/* If any of the ioctl's return failure, go on to the next section. */

    z = TIOCM_DTR;			/* Code for DTR. */
#ifdef COMMENT
/*
  This was the cause of the troubles with the Solaris Port Monitor.
  The problem is: RTS never comes back on.  Moral: Don't do it!
  (But why doesn't it come back on?  See the TIOCMBIS call...)
*/
#ifdef TIOCM_RTS			/* Lower RTS too if symbol is known. */
    z |= TIOCM_RTS;			
#endif /* TIOCM_RTS */
#endif /* COMMENT */

    debug(F101,"tthang TIOCM signal mask","",z);
    if (ioctl(ttyfd,TIOCMBIC,&z) > -1) {   /* Try to lower DTR. */
	debug(F100,"tthang TIOCMBIC ok","",0);
	msleep(HUPTIME);		   /* Pause half a second. */
	if (ioctl(ttyfd,TIOCMBIS,&z) > -1) { /* Try to turn it back on. */
	    debug(F100,"tthang TIOCMBIS ok","",0);
#ifndef CLSOPN
	    return(1);			/* Success, done. */
#endif /* CLSOPN */
	} else {			/* Couldn't raise, continue. */
	    debug(F101,"tthang TIOCMBIS errno","",errno);
	}
    } else {				/* Couldn't lower, continue. */
 	debug(F101,"tthang TIOCMBIC errno","",errno);
    }
#endif /* TIOCM_DTR */
#endif /* TIOCMBIC */
#endif /* TIOCMBIS */
#endif /* _IBMR2 */
/*
  General AT&T UNIX case, not HPUX.  The following code is highly suspect.  No
  two AT&T-based systems seem to do this the same way.  The object is simply
  to turn off DTR and then turn it back on.  SVID says the universal method
  for turning off DTR is to set the speed to zero, and this does seem to do
  the trick in all cases.  But neither SVID nor any known man pages say how to
  turn DTR back on again.  Some variants, like most Xenix implementations,
  raise DTR again when the speed is restored to a nonzero value.  Others
  require the device to be closed and opened again, but this is risky because
  getty could seize the device during the instant it is closed.
*/

/* Return code for ioctl failures... */
#ifdef ATT6300
    x = 1;				/* ATT6300 doesn't want to fail... */
#else
    x = -1;
#endif /* ATT6300 */

    debug(F100,"tthang get settings","",0);
    if (ioctl(ttyfd,TCGETA,&ttcur) < 0) /* Get current settings. */
      return(x);			/* Fail if this doesn't work. */
    if ((flags = fcntl(ttyfd,F_GETFL,0)) < 0) /* Get device flags. */
      return(x);
    ttc_save = ttcur.c_cflag;		/* Remember current speed. */
    spdsav = ttc_save & CBAUD;
    debug(F101,"tthang speed","",spdsav);

#ifdef O_NDELAY
    debug(F100,"tthang turning O_NDELAY on","",0);
    fcntl(ttyfd, F_SETFL, flags | O_NDELAY); /* Activate O_NDELAY */
#endif /* O_NDELAY */

#ifdef ATT7300 /* This is the way it is SUPPOSED to work */
    ttcur.c_cflag &= ~CBAUD;		/* Change the speed to zero.  */
#else
#ifdef RTAIX
    ttcur.c_cflag &= ~CBAUD;		/* Change the speed to zero.  */
#else          /* This way really works but may be dangerous */
#ifdef u3b2
    ttcur.c_cflag = ~(CBAUD|CLOCAL);	/* Special for AT&T 3B2s */
					/* (CLOCAL must be OFF) */
#else
#ifdef SCO3R2				/* SCO UNIX 3.2 */
/*
  This is complete nonsense, but an SCO user claimed this change made
  hanging up work.  Comments from other SCO UNIX 3.2 users would be 
  appreciated.
*/
    ttcur.c_cflag = CBAUD|B0;
#else
#ifdef AIXRS				/* AIX on RS/6000 */
/*
  Can't set speed to zero on AIX 3.1 on RS/6000 64-port adapter,
  even though you can do it on the built-in port and the 8- and 16-port
  adapters.  (Untested on 128-port adapter.)
*/
    ttcur.c_cflag = CLOCAL|HUPCL|spdsav; /* Speed 0 causes EINVAL */
#else					/* None of the above */
/*
  Set everything, including the speed, to zero, except for the CLOCAL
  and HUPCL bits.
*/
    ttcur.c_cflag = CLOCAL|HUPCL;
#endif /* AIXRS */
#endif /* SCO3R2 */
#endif /* u3b2 */
#endif /* RTAIX */
#endif /* ATT7300 */

#ifdef COMMENT
    /* and if none of those work, try one of these... */
    ttcur.c_cflag = 0;
    ttcur.c_cflag = CLOCAL;
    ttcur.c_cflag &= ~(CBAUD|HUPCL);
    ttcur.c_cflag &= ~(CBAUD|CREAD);
    ttcur.c_cflag &= ~(CBAUD|CREAD|HUPCL);
    /* or other combinations */
#endif /* COMMENT */

#ifdef TCXONC
    debug(F100,"tthang TCXONC","",0);
    ioctl(ttyfd, TCXONC, 1);
#endif /* TCXONC */

#ifdef TIOCSTART
    debug(F100,"tthang TIOCSTART","",0);
    ioctl(ttyfd, TIOCSTART, 0);
#endif /* TIOCSTART */

    if (ioctl(ttyfd,TCSETAF,&ttcur) < 0) { /* Fail if we can't. */
	fcntl(ttyfd, F_SETFL, flags);	/* Restore flags */
	return(-1);			/* before returning. */
    }
    msleep(300);			/* Give modem time to notice. */

/* Now, even though it doesn't say this in SVID or any man page, we have */
/* to close and reopen the device.  This is not necessary for all systems, */
/* but it's impossible to predict which ones need it and which ones don't. */

#ifdef ATT7300
/*
  Special handling for ATT 7300 UNIX PC and 3B1, which have "phone"
  related ioctl's for their internal modems.  attmodem has getty status and 
  modem-in-use bit.  Reportedly the ATT7300/3B1 PIOCDISC call is necessary, 
  but also ruins the file descriptor, and no other phone(7) ioctl call can fix 
  it.  Whateverit does, it seems to escape detection with PIOCGETA and TCGETA.
  The only way to undo the damage is to close the fd and then reopen it.
*/
    if (attmodem & ISMODEM) {
	debug(F100,"tthang attmodem close/open","",0);
	ioctl(ttyfd,PIOCUNHOLD,&dialer); /* Return call to handset. */
	ioctl(ttyfd,PIOCDISC,&dialer);	/* Disconnect phone. */
	close(ttyfd);			/* Close and reopen the fd. */
	ttyfd = priv_opn(ttnmsv, O_RDWR | O_NDELAY);
	attmodem &= ~ISMODEM;		/* Phone no longer in use. */
    }
#else /* !ATT7300 */
/* It seems we have to close and open the device for other AT&T systems */
/* too, and this is the place to do it.  The following code does the */
/* famous close(open(...)) magic by default.  If that doesn't work for you, */
/* then try uncommenting the following statement or putting -DCLSOPN in */
/* the makefile CFLAGS. */

/* #define CLSOPN */

#ifndef SCO32 /* Not needed by, and harmful to, SCO UNIX 3.2 / Xenix 2.3 */

#ifdef O_NDELAY
#define OPENFLGS O_RDWR | O_NDELAY
#else
#define OPENFLGS O_RDWR
#endif

#ifndef CLSOPN
/* This method is used by default, i.e. unless CLSOPN is defined. */
/* It is thought to be safer because there is no window where getty */
/* can seize control of the device.  The drawback is that it might not work. */

    debug(F101,"tthang close(open()), OPENFLGS","",OPENFLGS);
    close(priv_opn(ttnmsv, OPENFLGS));

#else
/* This method is used if you #define CLSOPN.  It is more likely to work */
/* than the previous method, but it's also more dangerous. */

    debug(F101,"tthang close/open, OPENFLGS","",OPENFLGS);
    close(ttyfd);
    msleep(10);
    ttyfd = priv_opn(ttnmsv, OPENFLGS);	/* Open it again */
#endif /* CLSOPN */
#undef OPENFLGS

#endif /* SCO32 */
#endif /* ATT7300 */

/* Now put all flags & modes back the way we found them. */
/* (Does the order of ioctl & fcntl matter ? ) */

    debug(F100,"tthang restore settings","",0);
    ttcur.c_cflag = ttc_save;		/* Get old speed back. */
    if (ioctl(ttyfd,TCSETAF,&ttcur) < 0) /* ioctl parameters. */
      return(-1); 
#ifdef O_NDELAY
/*
  This is required for IBM RT and RS/6000, probably helps elsewhere too (?).
  After closing a modem line, the modem will probably not be asserting
  carrier any more, so we should not require carrier any more.  If this
  causes trouble on non-IBM UNIXes, change the #ifdef to use _IBMR2 rather
  than O_NDELAY.
*/
    flags &= ~O_NDELAY;			/* Don't require carrier on reopen */
#endif /* O_NDELAY */
    if (fcntl(ttyfd,F_SETFL,flags) < 0)	/* fcntl parameters */
      return(-1);

    return(1);
#endif /* not HPUX */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
#endif /* NOLOCAL */
}

/*
  Major change in 5A(174).  We used to use LPASS8, if it was defined, to
  allow 8-bit data and Xon/Xoff flow control at the same time.  But this
  LPASS8 business seems to have been causing trouble for everybody but me!
  For example, Annex terminal servers, commonly used with Encore computers,
  do not support LPASS8 even though the Encore itself does.  Ditto for many
  other terminal servers, TELNET connections, rlogin connections, etc etc.
  Now, reportedly, even vanilla 4.3 BSD systems can't do this right on their
  serial lines, even though LPASS8 is a feature of 4.3BSD.  So let's turn it
  off for everybody.  That means we goes back to using raw mode, with no
  flow control.  Phooey.

  NOTE: This must be done before the first reference to LPASS8 in this file,
  and after the last #include statment.
*/
#ifdef LPASS8
#undef LPASS8
#endif /* LPASS8 */

/*  T T R E S  --  Restore terminal to "normal" mode.  */

/* ske@pkmab.se: There are two choices for what this function should do.
 * (1) Restore the tty to current "normal" mode, with carrier treatment
 * according to ttcarr, to be used after every kermit command. (2) Restore
 * the tty to the state it was in before kermit opened it. These choices
 * conflict, since ttold can't hold both choices of tty parameters.  ttres()
 * is currently being called as in choice (1), but ttold basically holds
 * the initial parameters, as in (2), and the description at the beginning
 * of this file says (2).
 *
 * I don't think restoring tty parameters after all kermit commands makes
 * much of a difference.  Restoring them upon exit from kermit may be of
 * some use in some cases (when the line is not restored automatically on
 * close, by the operating system).
 *
 * I can't choose which one it should be, so I haven't changed it. It
 * probably works as it is, too. It would probably even work even with
 * ttres() entirely deleted...
 *
 * (from fdc: Actually, this function operates in remote mode too, so
 * it restores the console (command) terminal to whatever mode it was
 * in before packet operations began, so that commands work right again.)
 */
int
ttres() {                               /* Restore the tty to normal. */
    int x;

    if (ttyfd < 0) return(-1);          /* Not open. */

    if (ttfdflg) return(0);		/* Don't mess with terminal modes if */
					/* we got ttyfd from another process */
#ifdef	NETCONN
    if (netconn) return (0);		/* Network connection, do nothing */
#endif	/* NETCONN */

/* Real terminal device, so restore its original modes */

#ifdef BSD44ORPOSIX			/* For POSIX like this */
    x = tcsetattr(ttyfd,TCSADRAIN,&ttold);
#else					/* For all others... */
#ifdef ATTSV                            /* For AT&T versions... */
    x = ioctl(ttyfd,TCSETAW,&ttold);	/* Restore tty modes this way. */
#else
/* Here we restore the modes for BSD */

#ifdef LPASS8				/* Undo "pass8" if it were done */
    if (lmodef) {
	if (ioctl(ttyfd,TIOCLSET,&lmode) < 0)
	  debug(F100,"ttres TIOCLSET failed","",0);
	else
	  debug(F100,"ttres TIOCLSET ok","",0);
    }
#endif /* LPASS8 */

#ifdef CK_DTRCTS		   /* Undo hardware flow if it were done */
    if (lmodef) {
 	if (ioctl(ttyfd,TIOCLSET,&lmode) < 0)
 	  debug(F100,"ttres TIOCLSET failed","",0);
 	else
 	  debug(F100,"ttres TIOCLSET ok","",0);
    }
#endif /* CK_DTRCTS */

#ifdef TIOCGETC				/* Put back special characters */
    if (tcharf && (xlocal == 0)) {
	if (ioctl(ttyfd,TIOCSETC,&tchold) < 0)
	  debug(F100,"ttres TIOCSETC failed","",0);
	else
	  debug(F100,"ttres TIOCSETC ok","",0);
    }
#endif /* TIOCGETC */

#ifdef TIOCGLTC				/* Put back local special characters */
    if (ltcharf && (xlocal == 0)) {
	if (ioctl(ttyfd,TIOCSLTC,&ltchold) < 0)
	  debug(F100,"ttres TIOCSLTC failed","",0);
	else
	  debug(F100,"ttres TIOCSLTC ok","",0);
    }
#endif /* TIOCGLTC */

#ifdef BELLV10
    x = ioctl(ttyfd,TIOCSETP,&ttold);	/* Restore both structs */
    x = ioctl(ttyfd,TIOCSDEV,&tdold);
#else
    x = stty(ttyfd,&ttold);             /* Restore tty modes the old way. */
#endif /* BELLV10 */

    if (!xlocal)
      msleep(100);			/* This replaces sleep(1)... */
					/* Put back sleep(1) if tty is */
					/* messed up after close. */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */

    debug(F101,"ttres tty modes restore","",x);
#ifndef QNX
    if (x < 0) debug(F101,"ttres errno","",errno);
#endif /* QNX */

#ifdef AIXRS
#ifndef AIX41
    x = ioctl(ttyfd, ttld & 1 ? TXADDCD : TXDELCD, "rts");
    debug(F101,"ttres AIX line discipline rts restore","",x);
#endif /* AIX41 */
#endif /* AIXRS */

#ifdef BSD41
    if (ttld > -1) {			/* Put back line discipline */
	x = ioctl(ttyfd, TIOCSETD, &ttld);
	debug(F101,"ttres line discipline restore","",x);
	if (x < 0) debug(F101,"...ioctl errno","",errno);
	ttld = -1;
    }
#endif /* BSD41 */

#ifdef sony_news
    x = xlocal ? km_ext : km_con;	/* Restore Kanji mode. */
    if (x != -1) {			/* Make sure we know original modes. */
	if (ioctl(ttyfd,TIOCKSET, &x) < 0) {
	    perror("ttres can't set Kanji mode");
	    debug(F101,"ttres error setting Kanji mode","",x);
	    return(-1);
	}
    }
    debug(F100,"ttres set Kanji mode ok","",0);
#endif /* sony_news */

    tvtflg = 0;				/* Invalidate terminal mode settings */
    return(x);
}

#ifndef NOUUCP
#ifdef HPUX10

/* Aliases (different drivers) for HP-UX 10.00 dialout devices: */

static char *devprefix[] = { "tty", "ttyd", "cul", "cua", "" };

/*  T T C H K P I D  --  Check lockfile pid  */
/*
  Read pid from lockfile named f, check that it's still valid.
  If so, return 1.
  On failure to read pid, return 1.
  Otherwise, try to delete lockfile f and return 0 if successful, else 1.
*/
static int
ttchkpid(f) char *f; {
    int pid, x;
    priv_on();
    pid = ttrpid(f);			/* Read pid from file. */
    priv_off();
    if (pid > -1) {			/* If we were able to read the pid.. */
	debug(F101,"ttchkpid pid","",pid);
	errno = 0;			/* See if process still exists. */
	x = kill((PID_T) pid,0);
	debug(F101,"ttchkpid kill","",x);
	debug(F101,"ttchkpid kill errno","",errno);
	if (x < 0 && errno == ESRCH) { /* pid is invalid */
	    debug(F111,"removing stale lock",f,pid);
	    if (!backgrd)
	      printf("Removing stale lock %s (pid %d terminated)\n", f, pid);
	    priv_on();
	    x = unlink(f);		/* Remove the lockfile. */
	    priv_off();
	    debug(F111,"ttchkpid unlink",f,x);
	    if (x > -1)
	      return(0);		/* Device is not locked after all */
	    else if (!backgrd)
	      perror("Failure to remove stale lock file");
	}
	return(1);
    }
}
#endif /* HPUX10 */

/*  T T R P I D  --  Read pid from lockfile "name" (used by ttlock) */

static int
ttrpid(name) char *name; {
    int x, fd, pid;

    debug(F110,"ttrpid",name,0);
    if (!name) return(-1);
    if (!*name) return(-1);
    fd = open(name,O_RDONLY);		/* Try to open lockfile. */
    if (fd > 0) {

#ifdef PIDSTRING
	char buf[12];
	x = read(fd, buf, 11);		/* For HDBUUCP, read pid string */
	debug(F101,"ttrpid PIDSTRING read","",x);
	if (x < 0) return(-1);
	buf[11] = '\0';
	if (x == 11)
	  x = sscanf(buf,"%d",&pid);	/* Get the integer pid from it. */
#else
	x = read(fd, (char *)&pid, sizeof(pid)); /* Else read integer pid */
	debug(F101,"ttrpid integer read","",x);
#endif /* PIDSTRING */

	if (x < 0) pid = -1;		/* Return any errors. */
	close(fd);			/* Close the lockfile. */
    } else pid = -1;
    debug(F101,"ttrpid pid","",pid);
    return(pid);
}
#endif /* NOUUCP */

/*  T T L O C K  */

/*
  This function attempts to coordinate use of the communication device with
  other copies of Kermit and any other program that follows the UUCP
  device-locking conventions, which, unfortunately, vary among different UNIX
  implementations.  The idea is to look for a file of a certain name, the
  "lockfile", in a certain directory.  If such a file is found, then the line
  is presumed to be in use, and Kermit should not use it.  If no such file is
  found, Kermit attempts to create one so that other programs will not use the
  same line at the same time.  Because the lockfile and/or the directory it's
  in might lack write permission for the person running Kermit, Kermit could
  find itself running setuid to uucp or other user that does have the
  necessary permissions.  At startup, Kermit has changed its effective uid to
  the user's real uid, and so ttlock() must switch back to the original
  effective uid in order to create the lockfile, and then back again to the
  real uid to prevent unauthorized access to other directories or files owned
  by the user the program is setuid to.

  Totally rewritten for C-Kermit 5A to eliminate windows of vulnerability,
  based on suggestions from Warren Tucker.  Call with pointer to name of 
  tty device.  Returns:

   0 on success
  -1 on failure

  Note: Once privileges are turned on using priv_on(), it is essential that
  they are turned off again before this function returns.
*/
#ifdef SVR4				/* Lockfile uses device numbers. */
/*
  Although I can't find this in writing anywhere (e.g. in SVID for SVR4),
  it is the behavior of the "reference version" of SVR4, i.e. the Intel
  port from UNIX Systems Laboratories, then called Univel UnixWare,
  then called Novell UnixWare, now called SCO Unixware...
*/
#ifndef LFDEVNO				/* Define this for SVR4 */
#ifndef AIXRS				/* But not for RS/6000 AIX 3.2, etc. */
#ifndef BSD44				/* If anybody else needs it... */
#ifndef __386BSD__
#ifndef __FreeBSD__
#ifndef HPUX10
#ifndef IRIX51				/* SGI IRIX 5.1 or later */
#ifndef CK_SCOV5			/* SCO Open Server 5.0 */
#define LFDEVNO
#endif /* CK_SCOV5 */
#endif /* IRIX51 */
#endif /* HPUX10 */
#endif /* __FreeBSD__ */
#endif /* __386BSD__ */
#endif /* BSD44 */
#endif /* AIXRS */
#endif /* LFDEVNO */			/* ... define it here or on CC */
#endif /* SVR4 */			/* command line. */

#ifdef COHERENT
#define LFDEVNO
#endif /* COHERENT */

#ifdef LFDEVNO
#include <sys/stat.h>			/* For major() & minor() macros. */
					/* Should be in <sys/types.h>. */
#ifndef major				/* If we didn't find it */
#ifdef SVR4				/* then for Sys V R4 */
#include <sys/mkdev.h>			/* look here */
#else					/* or for SunOS versions */
#ifdef SUNOS4				/* ... */
#include <sys/sysmacros.h>		/* look here */
#else					/* Otherwise take a chance: */
#define	major(dev) ( (int) ( ((unsigned)(dev) >> 8) & 0xff))
#define	minor(dev) ( (int) ( (dev) & 0xff))
#endif /* SUNOS4 */
#endif /* SVR4 */
#endif /* major */
#endif /* LFDEVNO */
/*
  Note for RS/6000: routines ttylock(devicename), ttyunlock(devicename),
  and ttylocked(devicename) from the standard library (libc.a) could be 
  used here instead.  It's not clear whether there is any advantage in this.
*/

static int
ttlock(ttdev) char *ttdev; {
    int x;

#ifdef NOUUCP
    strcpy(flfnam,"NOLOCK");
    haslock = 1;
    return(0);
#else /* !NOUUCP */

#ifndef HPUX10

    int lockfd;				/* File descriptor for lock file. */
    PID_T pid;				/* Process id of this process. */
    int fpid;				/* pid found in existing lockfile. */
    int tries;				/* How many times we've tried... */
#ifdef LFDEVNO
    struct stat devbuf;			/* For device numbers (SVR4). */
#endif /* LFDEVNO */

#ifdef PIDSTRING
    char pid_str[12];			/* My pid in string format. */
#endif /* PIDSTRING */

    char *device, *devname;

#define LFNAML 50			/* Max length for lock file name. */
    char lockfil[LFNAML];		/* Lock file name */
#ifdef RTAIX
    char lklockf[LFNAML];		/* Name for link to lock file  */
#endif /* RTAIX */
    char tmpnam[LFNAML+30];		/* Temporary lockfile name. */
    char *lockdir = LOCK_DIR;		/* Defined near top of this file, */
					/* or on cc command line. */

    haslock = 0;                        /* Not locked yet. */
    *flfnam = '\0';			/* Lockfile name is empty. */
    pid = getpid();			/* Get id of this process. */

/*  Construct name of lockfile and temporary file */

/*  device  = name of tty device without the path, e.g. "ttyh8" */
/*  lockfil = name of lock file, without path, e.g. "LCK..ttyh8" */

    device = ((devname = xxlast(ttdev,'/')) != NULL ? devname+1 : ttdev);

#ifdef ISIII				/* Interactive System III, PC/IX */
    strcpy(lockfil, device);
#else
#ifdef LFDEVNO				/* Lockfilename has device numbers. */
    if (stat(ttdev,&devbuf) < 0)
      return(-1);
#ifdef COHERENT
    sprintf(lockfil,"LCK..%d.%d",
	    major(devbuf.st_rdev),	   /* major device number */
	    0x1f & minor(devbuf.st_rdev)); /* minor device number */
#else
    sprintf(lockfil,"LK.%03d.%03d.%03d",
	    major(devbuf.st_dev),	/* inode */
	    major(devbuf.st_rdev),	/* major device number */
	    minor(devbuf.st_rdev));	/* minor device number */
#endif /* COHERENT */
#else					/* Others... */
#ifdef PTX				/* Dynix PTX */
    if (device != &ttdev[5] && strncmp(ttdev,"/dev/",5)==0) {
	sprintf(lockfil,"LCK..%.3s%s", &ttdev[5], device);
    } else
#endif /* PTX */
    sprintf(lockfil,"LCK..%s", device);
#ifdef RTAIX
    strcpy(lklockf,device);
#endif /* RTAIX */
#endif /* LFDEVNO */
#endif /* ISIII */

#ifdef CK_SCOV5				/* SCO Open Server 5.0 */
    {
	/* Lowercase the entire filename. */
        /* SCO says we must do this in V5.0 and later. */
	/* BUT... watch out for devices -- like Digiboard Portserver */
	/* That can have hundreds of ports... */
	char *p = (char *)(lockfil + 5);
	while (*p) { if (isupper(*p)) *p = (char) tolower(*p); p++; }
    }
#else
#ifdef M_XENIX				/* SCO Xenix */
    {
	int x; char c;
	x = (int)strlen(lockfil) - 1;	/* Get last letter of device name. */
	if (x > 0) {			/* If it's uppercase, lower it. */
	    c = lockfil[x];
	    if (c >= 'A' && c <= 'Z') lockfil[x] += ('a' - 'A');
	}
    }
#endif /* M_XENIX */
#endif /* CK_SCOV5 */

/*  flfnam = full lockfile pathname, e.g. "/usr/spool/uucp/LCK..ttyh8" */
/*  tmpnam = temporary unique, e.g. "/usr/spool/uucp/LTMP..pid" */

    sprintf(flfnam,"%s/%s",lockdir,lockfil);
#ifdef RTAIX
    sprintf(lkflfn,"%s/%s",lockdir,lklockf);
#endif /* RTAIX */
    sprintf(tmpnam,"%s/LTMP.%05d",lockdir,(int) pid);
    debug(F110,"ttlock flfnam",flfnam,0);
    debug(F110,"ttlock tmpnam",tmpnam,0);

    priv_on();				/* Turn on privileges if possible. */
    lockfd = creat(tmpnam, 0444);	/* Try to create temp lock file. */
    if (lockfd < 0) {			/* Create failed. */
	debug(F111,"ttlock creat failed",tmpnam,errno);
	if (errno == ENOENT) {
	    perror(lockdir);	    
	    printf("UUCP not installed or Kermit misconfigured\n");
	} else {
	    perror(lockdir);
	    unlink(tmpnam);		/* Get rid of the temporary file. */
	}
	priv_off();			/* Turn off privileges!!! */
	return(-1);			/* Return failure code. */
    }
/* Now write the pid into the temp lockfile in the appropriate format */

#ifdef PIDSTRING			/* For Honey DanBer UUCP, */
    sprintf(				/* write PID as decimal string */
	    pid_str,
#ifdef LINUXFSSTND			/* The "Linux File System Standard" */
#ifdef FSSTND10				/* Version 1.0 calls for */
	    "%010d\n",			/* leading zeros */
#else					/* while version 1.2 calls for */
	    "%10d\n",			/* leading spaces */
#endif /* FSSTND10 */
#else
#ifdef COHERENT
	    "%d\n",			/* with leading nothing */
#else
	    "%10d\n",			/* with leading blanks */
#endif /* COHERENT */
#endif /* LINUXFSSTND */
	    (int) pid
	    );
    write(lockfd, pid_str, 11);
    debug(F111,"ttlock hdb pid string",pid_str,(int) pid);

#else /* Not PIDSTRING, use integer PID */

    write(lockfd, (char *)&pid, sizeof(pid) );
    debug(F101,"ttlock pid","",(int) pid);

#endif /* PIDSTRING */

/* Now try to rename the temp file to the real lock file name. */
/* This will fail if a lock file of that name already exists.  */

    close(lockfd);			/* Close the temp lockfile. */
    chmod(tmpnam,0444);			/* Permission for a valid lock. */    
    tries = 0;
    while (!haslock && tries++ < 2) {
	haslock = (link(tmpnam,flfnam) == 0); /* Create a link to it. */
	if (haslock) {			      /* If we got the lockfile */
#ifdef RTAIX
	    link(flfnam,lkflfn);
#endif /* RTAIX */ 

#ifdef LOCKF
#ifdef F_TLOCK
/*
  Advisory file locking works on SVR4, so we use it.  In fact, it is
  necessary in some cases, e.g. when SLIP is involved.  But it still doesn't
  seem prevent multiple users accessing the same device by different names.
*/
            while ( lockf(ttyfd, F_TLOCK, 0L) != 0 ) {
                debug(F111, "ttlock: lockf returns errno", "", errno);
                if ( (++tries >= 3) || (errno != EAGAIN) ) {
                    x = unlink(flfnam); /* remove the lockfile */
                    debug(F111,"ttlock unlink",flfnam,x);
                    haslock = 0;
		    break;
		}
                sleep(2);
	    }
	    if (haslock)		/* If we got an advisory lock */
#endif /* F_TLOCK */
#endif /* LOCKF */
	      break;			/* We're done. */

	} else {			/* We didn't create a new lockfile. */
	    if ((fpid = ttrpid(flfnam)) > -1) {	/* Read pid from old one. */
		if (fpid > 0) {
		    debug(F101,"ttlock fpid","",fpid);
		    errno = 0;		/* See if process still exists. */
		    x = kill((PID_T) fpid,0);
		    debug(F101,"ttlock kill","",x);
		    debug(F101,"ttlock kill errno","",errno);
		    if (x < 0 && errno == ESRCH) { /* pid is invalid */
			debug(F111,"ttlock removing stale lock",flfnam,
			      fpid);
			if (!backgrd)
			  printf(
				"Removing stale lock %s (pid %d terminated)\n",
				 flfnam,fpid);
			x = unlink(flfnam); /* remove the lockfile. */
			debug(F111,"ttlock unlink",flfnam,x);
			continue;	/* and go back and try again. */
		    } else if ((x < 0 && errno == EPERM) || x == 0) {
			unlink(tmpnam);	/* Delete the tempfile */
			debug(F101,"ttlock found tty locked","",fpid);
			priv_off();	/* Turn off privs */
			return(-2);	/* Code for device is in use. */
		    }
		} else {
		    debug(F101,"ttlock can't get fpid","",fpid);
		    break;
		}
	    } else break;		/* Couldn't read pid from old file */
	}
    }
    unlink(tmpnam);			/* Unlink (remove) the temp file. */
    priv_off();				/* Turn off privs */
    return(haslock ? 0 : -1);		/* Return link's return code. */

#else /* HPUX10 */

/*
  HP-UX 10.00 gets its own copy of this routine.  The behavior of this routine
  is modeled after the observed behavior of the HP-UX 10.00 IC2 'cu' program.
  Dialout device names consist of a prefix, an instance (interface) number,
  the letter "p", and a port number, e.g.  tty0p0, cul1p0, cua0p1, etc.  Each
  physical device can have (at least) four names (drivers): "tty" (direct
  out), "cul" (dialout), "cua" (automatic dialout), and "ttyd" (dialin).  Each
  of these is followed by the <instance>p<port>, e.g. "0p0", which is referred
  to below as the "unit".  This routine checks for all four aliases of a given
  unit.  If it decides the device is free, then if it is a "cua" or "cul"
  device, it creates a second lockfile with the "ttyd" prefix.  This is
  exactly what cu does.  If the "set line" device does not happen to have a
  unit number, then it is used literally and no synomyms are searched for and
  only one lockfile is created.
*/
#define LFNAML 80			/* Max length for lock file name. */

    int lockfd;				/* File descriptor for lock file. */
    PID_T pid;				/* Process ID of this process. */
    int fpid;				/* pid found in existing lockfile. */
    int tries;				/* How many times we've tried... */
    int i, k;				/* Workers */

    char *device, *devname;		/* "/dev/xxx", "xxx" */
    char *unit, *p;			/* <instance>p<port> part of xxx */

    char lockfil[LFNAML];		/* Lockfile name (no path) */
    char tmpnam[LFNAML];		/* Temporary lockfile name. */
    char *lockdir = "/var/spool/locks";	/* Lockfile directory */

    haslock = 0;                        /* Not locked yet. */
    *flfnam = '\0';			/* Lockfile name is empty. */
    lock2[0] = '\0';			/* Second one too. */
    pid = getpid();			/* Get my process ID */
/*
  Construct name of lockfile and temporary file...
  device  = name of tty device without the path, e.g. "tty0p0"
  lockfil = name of lock file, without path, e.g. "LCK..tty0p0"
*/
    device = ((devname = xxlast(ttdev,'/')) != NULL ? devname+1 : ttdev);
    debug(F110,"TTLOCK device",device,0);
    sprintf(lockfil,"LCK..%s", device);

    k = 0;				/* Assume device is not locked */
    unit = device;			/* Unit = <instance>p<port> */
    while (*unit && !isdigit(*unit))	/* Search for digit... */
      unit++;
    p = unit;				/* Verify <num>p<num> format... */
    debug(F110,"TTLOCK unit 1",unit,0);
    while (*p && isdigit(*p++)) ;	/* This does not assume */
    if (*p++ == 'p')			/* any particular number of */
      while (*p && isdigit(*p++)) ;	/* digits before & after "p". */
    if (*p) unit = "";
    debug(F110,"TTLOCK unit 2",unit,0);

    if (*unit) {			/* Device name has unit number. */
	/* The following loop not only searches for the various lockfile    */
	/* synonyms, but also removes all -- not just one -- stale lockfile */
	/* for the device, should there be more than one.  See ttchkpid().  */
	for (i = 0; *devprefix[i]; i++) { /* For each driver... */
	    /* Make lockfile name */
	    sprintf(lock2,"%s/LCK..%s%s",lockdir,devprefix[i],unit);
	    priv_on();			/* Privs on */
	    k = zchki(lock2) != -1;	/* See if lockfile exists */
	    priv_off();			/* Privs off */
	    debug(F111,"TTLOCK check for lock A",lock2,k);
	    if (k) if (ttchkpid(lock2)) { /* If pid still active, fail. */
		strcpy(flfnam,lock2);
		return(-2);
	    }
	}
    } else {				/* Some other device-name format */
	/* This takes care of symbolic links, etc... */
	/* But does not chase them down! */
	sprintf(lock2,"%s/LCK..%s",lockdir,device); /* Use the name as-is */
	priv_on();
	k = zchki(lock2) != -1;		/* Check for existing lockfile */
	priv_off();
	debug(F111,"TTLOCK check for lock B",lock2,k);
	if (k) if (ttchkpid(lock2)) {	/* Check pid from lockfile */
	    strcpy(flfnam,lock2);
	    return(-2);
	}
    }
/*
  Get here only if there is no (more) lockfile, so now we make one (or two)...
  flfnam = full lockfile pathname, e.g. "/usr/spool/uucp/LCK..cul0p0".
  tmpnam = unique temporary filname, e.g. "/usr/spool/uucp/LTMP..pid".
*/
    sprintf(flfnam,"%s/%s",lockdir,lockfil); /* Our SET LINE device */

    /* If dialout device, also make one for corresponding dialin device */
    lock2[0] = '\0';
    if (!strncmp(device,"cu",2) && *unit) 
      sprintf(lock2, "%s/LCK..ttyd%s", lockdir, unit);

    sprintf(tmpnam,"%s/LTMP.%05d",lockdir,(int) pid); /* Make temporary name */
#ifdef DEBUG
    if (deblog) {
	debug(F110,"TTLOCK flfnam",flfnam,0);
	debug(F110,"TTLOCK lock2",lock2,0);	
	debug(F110,"TTLOCK tmpnam",tmpnam,0);
    }
#endif /* DEBUG */
/*
   Lockfile permissions...
   444 is standard, HP-UX 10.00 uses 664.  It doesn't matter.
   Kermit uses 444; the difference lets us tell whether Kermit created
   the lock file.
*/
    priv_on();				/* Turn on privileges. */
    lockfd = creat(tmpnam, 0444);	/* Try to create temporary file. */
    if (lockfd < 0) {			/* Create failed. */
	debug(F111,"ttlock creat failed",tmpnam,errno);
	if (errno == ENOENT) {
	    perror(lockdir);	    
	    printf("UUCP not installed or Kermit misconfigured\n");
	} else {
	    perror(lockdir);
	    unlink(tmpnam);		/* Get rid of the temporary file. */
	}
	priv_off();			/* Turn off privileges!!! */
	return(-1);			/* Return failure code. */
    }
    debug(F110,"TTLOCK temp ok",tmpnam,0);

/* Now write our pid into the temp lockfile in integer format. */

    i = write(lockfd, (char *)&pid, sizeof(pid));

#ifdef DEBUG
    if (deblog) {
	debug(F101,"TTLOCK pid","",pid);
	debug(F101,"TTLOCK sizeof pid","",sizeof(pid));
	debug(F101,"TTLOCK write pid returns","",i);
    }
#endif /* DEBUG */

/*
  Now try to rename the temporary file to the real lockfile name.
  This will fail if a lock file of that name already exists, which
  will catch race conditions with other users.
*/
    close(lockfd);			/* Close the temp lockfile. */
    chmod(tmpnam,0444);

    tries = 0;
    while (!haslock && tries++ < 2) {
	haslock = (link(tmpnam,flfnam) == 0); /* Create a link to it. */
	debug(F101,"TTLOCK link","",haslock);
	if (haslock) {			/* If we made the lockfile... */
  
#ifndef NOLOCKF				/* Can be canceled with -DNOLOCKF */
/*
  Create an advisory lock on the device through its file descriptor.
  This code actually seems to work.  If it is executed, and then another
  process tries to open the same device under a different name to circumvent
  the lockfile, they get a "device busy" error.
*/
	    debug(F100,"TTLOCK LOCKF code...","",0);
            while ( lockf(ttyfd, F_TLOCK, 0L) != 0 ) {
                debug(F111, "TTLOCK lockf error", "", errno);
                if ( (++tries >= 3) || (errno != EAGAIN) ) {
                    x = unlink(flfnam); /* Remove the lockfile */
		    if (errno == EACCES)
		      printf("Device already locked by another process\n");
                    haslock = 0;
		    break;
		}
                sleep(2);
	    }
#endif /* NOLOCKF */

	    if (haslock) {		/* If we made the lockfile ... */
		if (lock2[0]) {		/* if there is to be a 2nd lockfile */
		    lockfd = creat(lock2, 0444); /* Create it */
		    debug(F111,"TTLOCK lock2 creat", lock2, lockfd);
		    if (lockfd > -1) {	/* Created OK, write pid. */
			write(lockfd, (char *)&pid, sizeof(pid) );
			close(lockfd);	/* Close and */
			chmod(lock2, 0444); /* set permissions. */
		    } else {		 /* Not OK, but don't fail. */
			lock2[0] = '\0'; /* Just remember it's not there. */
		    }
		}
		break;			/* and we're done. */
	    }
	}
    }
    unlink(tmpnam);			/* Unlink (remove) the temp file. */
    priv_off();				/* Turn off privs */
    i = haslock ? 0 : -1;		/* Our return value */
    debug(F101,"TTLOCK returns","",i);
    return(i);
#endif /* HPUX10 */
#endif /* !NOUUCP */
}

/*  T T U N L O C K  */

static int
ttunlck() {                             /* Remove UUCP lockfile(s). */
#ifndef NOUUCP

    if (haslock && *flfnam) {
	priv_on();			/* Turn privileges on.  */
	if (unlink(flfnam) < 0)		/* Remove the lockfile. */
	  printf("Warning - Can't remove lockfile: %s\n", flfnam);
	*flfnam = '\0';			/* Erase the name. */

#ifdef RTAIX
	if (unlink(lkflfn) < 0)		/* Remove link to lockfile */
	  printf("Warning - Can't remove secondary lockfile: %s\n", lkflfn);
	*lkflfn = '\0';
#else
#ifdef HPUX10
	if (lock2[0]) {			/* If there is a second lockfile, */
	    if (unlink(lock2) < 0)	/*  remove it too. */
	      printf("Warning - Can't remove secondary lockfile: %s\n",lock2);
	    lock2[0] = '\0';		/* Forget its name. */
	}
#endif /* HPUX10 */
#endif /* RTAIX */

#ifdef LOCKF
#ifdef F_ULOCK
        (VOID) lockf(ttyfd, F_ULOCK, 0L); /* Remove advisory lock */
#endif /* F_ULOCK */
#endif /* LOCKF */
	priv_off();			/* Turn privileges off. */
    }
#endif /* !NOUUCP */
    return(0);
}

/*
  4.3BSD-style UUCP line direction control.
  (Stan Barber, Rice U, 1980-something...)
*/
#ifndef NOUUCP
#ifdef ACUCNTRL
VOID
acucntrl(flag,ttname) char *flag, *ttname; {
    char x[DEVNAMLEN+32], *device, *devname;

    if (strcmp(ttname,CTTNAM) == 0 || xlocal == 0) /* If not local, */
      return;				/* just return. */
    device = ((devname = xxlast(ttname,'/')) != NULL ? devname+1 : ttname);
    if (strncmp(device,"LCK..",4) == 0) device += 5;
    sprintf(x,"/usr/lib/uucp/acucntrl %s %s",flag,device);
    debug(F110,"called ",x,0);
    zsyscmd(x);
}
#endif /* ACUCNTRL */
#endif /* NOUUCP */

/*
  T T H F L O W  --  Set or Reset hardware flow control.

  This is an attempt to collect all hardware-flow-control related code
  into a single module.  Thanks to Rick Sladkey and John Kohl for lots of
  help here.  Overview:

  Hardware flow control is not supported in many UNIX implementions.  Even
  when it is supported, there is no (ha ha) "standard" for the programming
  interface.  In general, 4.3BSD and earlier (sometimes), 4.4BSD, System V,
  SunOS, AIX, etc, have totally different methods.  (And, not strictly
  relevant here, the programming interface often brings one only to a no-op
  in the device driver!)

  Among all these, we have two major types of APIs: those in which hardware
  flow control is determined by bits in the same termio/termios/sgtty mode
  word(s) that are used for controlling such items as CBREAK vs RAW mode, and
  which are also used by the ttvt(), ttpkt(), conbin(), and concb() routines
  for changing terminal modes.  And those that use entirely different
  mechanisms.

  In the first category, it is important that any change in the mode bits be
  reflected in the relevant termio(s)/sgtty structure, so that subsequent
  changes to that structure do not wipe out the effects of this routine.  That
  is why a pointer, attrs, to the appropriate structure is passed as a
  parameter to this routine.

  The second category should give us no worries, since any changes to hardware
  flow control accomplished by this routine should not affect the termio(s)/
  sgtty structures, and therefore will not be undone by later changes to them.

  The second argument, status, means to turn on hardware flow control if
  nonzero, and to turn it off if zero.

  Returns: 0 on success, -1 on failure.
*/
static int
tthflow(flow, status, attrs)
    int flow,				/* Type of flow control (ckcdeb.h) */
    status;				/* Nonzero = turn it on */
					/* Zero = turn it off */
#ifdef BSD44ORPOSIX			/* POSIX or BSD44 */
    struct termios *attrs;
#else					/* System V */
#ifdef ATTSV
    struct termio *attrs;
#else					/* BSD, V7, etc */
    struct sgttyb *attrs;		/* sgtty info... */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
/* tthflow */ {

#ifdef Plan9
    return p9tthflow(flow, status);
#else

    int x = 0;				/* tthflow() return code */

#ifndef OXOS				/* NOT Olivetti X/OS... */
/*
  For SunOS 4.0 and later in the BSD environment ...

  The declarations are copied and interpreted from the System V header files,
  so we don't actually have to pull in all the System V junk when building
  C-Kermit for SunOS in the BSD environment, which would be dangerous because
  having those symbols defined would cause us to take the wrong paths through
  the code.  The code in this section is used in both the BSD and Sys V SunOS
  versions.
*/
#ifdef SUNOS41
/*
  In SunOS 4.1 and later, we use the POSIX calls rather than ioctl calls
  because GNU CC uses different formats for the _IOxxx macros than regular CC;
  the POSIX forms work for both.  But the POSIX calls are not available in
  SunOS 4.0.
*/
#define CRTSCTS 0x80000000		/* RTS/CTS flow control */
#define TCSANOW 0			/* Do it now */

    struct termios {
	unsigned long c_iflag;		/* Input modes */
	unsigned long c_oflag;		/* Output modes */
	unsigned long c_cflag;		/* Control modes */
	unsigned long c_lflag;		/* Line discipline modes */
	char c_line;
	CHAR c_cc[17];
    };
    struct termios temp;

_PROTOTYP( int tcgetattr, (int, struct termios *) );
_PROTOTYP( int tcsetattr, (int, int, struct termios *) );
/*
  When CRTSCTS is set, SunOS won't do output unless both CTS and CD are
  asserted.  So we don't set CRTSCTS unless CD is up.  This should be OK,
  since we don't need RTS/CTS during dialing, and after dialing is complete,
  we should have CD.  If not, we still communicate, but without RTS/CTS.
*/
    int mflags;				/* Modem signal flags */
    if (!status) {			/* Turn hard flow off */
	if (tcgetattr(ttyfd, &temp) > -1 && /* Get device attributes */
	    (temp.c_cflag & CRTSCTS)) { /* Check for RTS/CTS */
	    temp.c_cflag &= ~CRTSCTS;	/* It's there, remove it */
	    x = tcsetattr(ttyfd,TCSANOW,&temp);
	}
    } else {				/* Turn hard flow on */
	if (ioctl(ttyfd,TIOCMGET,&mflags) > -1 && /* Get modem signals */
	    (mflags & TIOCM_CAR)) {		/* Check for CD */
	    debug(F100,"tthflow SunOS has CD","",0);
	    if (tcgetattr(ttyfd, &temp) > -1 && /* Get device attributes */
		!(temp.c_cflag & CRTSCTS)) { /* Check for RTS/CTS */
		temp.c_cflag |= CRTSCTS;	/* Not there, add it */
		x = tcsetattr(ttyfd,TCSANOW,&temp);
	    }
	} else {
	    x = -1;
	    debug(F100,"tthflow SunOS no CD","",0);
	}
    }
#else
#ifdef QNX
    struct termios temp;
    if (tcgetattr(ttyfd, &temp) > -1) {	/* Get device attributes */
	if (!status) {			/* Turn hard flow off */
	    if ((temp.c_cflag & (IHFLOW|OHFLOW)) == (IHFLOW|OHFLOW)) {
		temp.c_cflag &= ~(IHFLOW|OHFLOW); /* It's there, remove it */
		attrs->c_cflag &= ~(IHFLOW|OHFLOW);
		x = tcsetattr(ttyfd,TCSANOW,&temp);
	    }
	} else {			/* Turn hard flow on */
	    if ((temp.c_cflag & (IHFLOW|OHFLOW)) != (IHFLOW|OHFLOW)) {
		temp.c_cflag |= (IHFLOW|OHFLOW); /* Not there, add it */
		temp.c_iflag &= ~(IXON|IXOFF);   /* Bye to IXON/IXOFF */
		ttraw.c_lflag |= IEXTEN;         /* Must be on */
		x = tcsetattr(ttyfd,TCSANOW,&temp);
		attrs->c_cflag |= (IHFLOW|OHFLOW);
		attrs->c_iflag &= ~(IXON|IXOFF);
	    }
	}
    } else {
	x = -1;
	debug(F100, "tthflow QNX getattr fails", "", 0);
    }
#else
#ifdef POSIX_CRTSCTS
/*
  POSIX_CRTSCTS is defined in ckcdeb.h or on CC command line.
  Note: Do not assume CRTSCTS is a one-bit field!
*/
    struct termios temp;
    if (tcgetattr(ttyfd, &temp) > -1) {	/* Get device attributes */
	if (!status) {			/* Turn hard flow off */
	    if ((temp.c_cflag & CRTSCTS) == CRTSCTS) { /* Check for RTS/CTS */
		temp.c_cflag &= ~CRTSCTS;	/* It's there, remove it */
		attrs->c_cflag &= ~CRTSCTS;
		x = tcsetattr(ttyfd,TCSANOW,&temp);
	    }
	} else {			/* Turn hard flow on */
	    if ((temp.c_cflag & CRTSCTS) != CRTSCTS) {
		temp.c_cflag |= CRTSCTS; /* Not there, add it */
		temp.c_iflag &= ~(IXON|IXOFF|IXANY); /* Bye to IXON/IXOFF */
		x = tcsetattr(ttyfd,TCSANOW,&temp);
		attrs->c_cflag |= CRTSCTS;
		attrs->c_iflag &= ~(IXON|IXOFF|IXANY);
	    }
	}
    } else {
	x = -1;
	debug(F100, "tthflow POSIX_CRTSCTS no attr", "", 0);
    }
#else
#ifdef SUNOS4
/*
  SunOS 4.0 (and maybe earlier?).  This code is dangerous because it
  prevents compilation with GNU gcc, which uses different formats for the
  _IORxxx macros than regular cc.  SunOS 4.1 and later can use the POSIX
  routines above, which work for both cc and gcc.
*/
#define TCGETS _IOR(T, 8, struct termios) /* Get modes into termios struct */
#define TCSETS _IOW(T, 9, struct termios) /* Set modes from termios struct */
#define CRTSCTS 0x80000000		  /* RTS/CTS flow control */

	struct termios {
	    unsigned long c_iflag;		/* Input modes */
	    unsigned long c_oflag;		/* Output modes */
	    unsigned long c_cflag;		/* Control modes */
	    unsigned long c_lflag;		/* Line discipline modes */
	    char c_line;
	    CHAR c_cc[17];
	};
	struct termios temp;
	if (ioctl(ttyfd,TCGETS,&temp) > -1) {	/* Get terminal modes. */
	    if (status) {			/* Turn hard flow on */
		temp.c_cflag |= CRTSCTS;	/* Add RTS/CTS to them. */
		x = ioctl(ttyfd,TCSETS,&temp);	/* Set them again. */
		attrs->c_cflag |= CRTSCTS;	/* Add to global info. */
	    } else {				/* Turn hard flow off */
		temp.c_cflag &= ~CRTSCTS;
		x = ioctl(ttyfd,TCSETS,&temp);
		attrs->c_cflag &= ~CRTSCTS;
	    }
	}
#else					/* Not SunOS 4.0 or later */
#ifdef AIXRS				/* IBM AIX RS/6000 */
#ifndef AIX41				/* But only pre-4.x == SVR4 */
    if (status) {
	if ((x = ioctl(ttyfd, TXADDCD, "rts")) < 0 && errno != EBUSY)
	  debug(F100,"hardflow TXADDCD (rts) error", "", 0);
    } else {
	if ((x = ioctl(ttyfd, TXDELCD, "rts")) < 0 && errno != EINVAL)
	  debug(F100,"hardflow TXDELCD (rts) error", "", 0);
    }
#endif /* AIX41 */
#else					/* Not AIX RS/6000 */

#ifdef ATTSV				/* System V... */

#ifdef CK_SCOV5				/* SCO Open Server 5.0 */
#define CK_SCOUNIX
#else
#ifdef M_UNIX				/* SCO UNIX 3.2v4.x or earlier */
#define CK_SCOUNIX
#endif /* M_UNIX */
#endif /* CK_SCOV5 */

#ifdef CK_SCOUNIX
#ifdef POSIX				/* SCO - POSIX untested !!! */
    struct termios temp;
    if ((x = tcgetattr(ttyfd, &temp)) == -1)
#else  /* POSIX */
    struct termio temp;
    if ((x = ioctl(ttyfd, TCGETA, &temp)) == -1)
#endif /* POSIX */
    {
	debug(F100,"SCO hardflow get modes error", "", 0);
    } else {
	if (status) {			/* Turn it ON */
	    temp.c_cflag |= RTSFLOW|CTSFLOW;
	    attrs->c_cflag |= RTSFLOW|CTSFLOW;
#ifdef ORTSFL
	    temp.c_cflag &= ~ORTSFL;
	    attrs->c_cflag &= ~ORTSFL;
#endif /* ORTSFL */
	    temp.c_iflag &= ~(IXON|IXOFF|IXANY);
	    attrs->c_iflag &= ~(IXON|IXOFF|IXANY);
	} else {			/* Turn it OFF */
#ifdef ORTSFL
	    temp.c_cflag &= ~(RTSFLOW|CTSFLOW|ORTSFL);
	    attrs->c_cflag &= ~(RTSFLOW|CTSFLOW|ORTSFL);
#else  /* ORTSFL */
	    temp.c_cflag &= ~(RTSFLOW|CTSFLOW);
	    attrs->c_cflag &= ~(RTSFLOW|CTSFLOW);
#endif /* ORTSFL */
	}
	if ((x =
#ifdef POSIX
	tcsetattr(ttyfd, &temp)
#else
	ioctl(ttyfd, TCSETA, &temp)
#endif /* POSIX */
	)  == -1 )
	  debug(F100,"SCO hardflow set modes error", "", 0);
    }
#else /* Not SCO UNIX */
    if (!status) {			/* Turn it OFF */
#ifdef RTSXOFF
	debug(F100,"tthflow ATTSV RTS/CTS OFF","",0);
	rctsx.x_hflag &= ~(RTSXOFF|CTSXON);
	x = ioctl(ttyfd,TCSETX,&rctsx);
	debug(F101,"tthflow ATTSV RTSXOFF OFF","",x);
#endif /* RTSXOFF */
#ifdef DTRXOFF
	debug(F100,"tthflow ATTSV DTR/CD OFF","",0);
	rctsx.x_hflag &= ~(DTRXOFF|CDXON);
	x = ioctl(ttyfd,TCSETX,&rctsx);
	debug(F101,"tthflow ATTSV DTRXOFF OFF","",x);
#endif /* DTRXOFF */
    } else {				/* Turn it ON. */
	if (flow == FLO_RTSC) {	/* RTS/CTS Flow control... */
#ifdef RTSXOFF
	    /* This is the preferred way, according to SVID R4 */
	    if (ioctl(ttyfd,TCGETX,&rctsx) > -1) {
		debug(F100,"tthflow ATTSV RTS/CTS ON","",0);
		rctsx.x_hflag |= RTSXOFF | CTSXON;
		x = ioctl(ttyfd,TCSETX,&rctsx);
	    }
#else
	    x = -1;
#endif /* RTSXOFF */
	} else if (flow == FLO_DTRC) {	/* DTR/CD Flow control... */
#ifdef DTRXOFF
	    /* This is straight out of SVID R4 */
	    if (ioctl(ttyfd,TCGETX,&rctsx) > -1) {
		debug(F100,"tthflow ATTSV DTR/CD ON","",0);
		rctsx.x_hflag &= ~(DTRXOFF|CDXON);
		x = ioctl(ttyfd,TCSETX,&rctsx);
	    }
#else
	    x = -1;
#endif /* DTRXOFF */
	}
    }
#endif /* CK_SCOUNIX */

#else /* not System V... */

#ifdef CK_DTRCTS
#ifdef LDODTR
#ifdef LDOCTS
    x = LDODTR | LDOCTS;		/* Found only on UTEK? */
    if (flow == FLO_DTRT && status) {	/* Use hardware flow control */
	if (lmodef) {
	    x = ioctl(ttyfd,TIOCLBIS,&x);
	    if (x < 0) {
	        debug(F100,"hardflow TIOCLBIS error","",0);
	    } else {
		lmodef++;
		debug(F100,"hardflow TIOCLBIS ok","",0);
	    }
	}
    } else {
	if (lmodef) {
	    x = ioctl(ttyfd,TIOCLBIC,&x);
	    if (x < 0) {
	        debug(F100,"hardflow TIOCLBIC error","",0);
	    } else {
		lmodef++;
		debug(F100,"hardflow TIOCLBIC ok","",0);
	    }
	}
    }
#endif /* LDODTR */
#endif /* LDOCTS */
#endif /* CK_DTRCTS */
#endif /* ATTSV */
#endif /* AIXRS */
#endif /* SUNOS4 */
#endif /* QNX */
#endif /* POSIX_CRTSCTS */
#endif /* SUNOS41 */

#else /* OXOS */

    struct termios temp;		/* Olivetti X/OS ... */

    x = ioctl(ttyfd,TCGETS,&temp);
    if (x == 0) {
	temp.c_cflag &= ~(CRTSCTS|CDTRCTS|CBRKFLOW|CDTRDSR|CRTSDSR);
	if (status) {
	    switch (flow) {
	      case FLO_RTSC: temp.c_cflag |= CRTSCTS; /* RTS/CTS (hard) */
		break;
	      case FLO_DTRT: temp.c_cflag |= CDTRCTS; /* DTR/CTS (hard) */
		break;
	    }
	}
	x = ioctl(ttyfd,TCSETS,&temp);
    }
#endif /* OXOS */
    return(x);

#endif /* Plan9 */

}

/*  T T P K T  --  Condition the communication line for packets */
/*                 or for modem dialing */

/*
  If called with speed > -1, also set the speed.
  Returns 0 on success, -1 on failure.

  NOTE: the "xflow" parameter is supposed to be the currently selected
  type of flow control, but for historical reasons, this parameter is also
  used to indicate that we are dialing.  Therefore, when the true flow
  control setting is needed, we access the external variable "flow", rather
  than trusting our "xflow" argument.
*/
int
#ifdef CK_ANSIC
ttpkt(long speed, int xflow, int parity)
#else
ttpkt(speed,xflow,parity) long speed; int xflow, parity;
#endif /* CK_ANSIC */
/* ttpkt */ {
#ifndef NOLOCAL
    int s2;
    int s = -1;
#endif /* NOLOCAL */
#ifndef SVORPOSIX
    int x;
#endif /* SVORPOSIX */
    extern int flow;			/* REAL flow-control setting */

    if (ttyfd < 0) return(-1);          /* Not open. */

    debug(F101,"ttpkt parity","",parity);
    debug(F101,"ttpkt xflow","",xflow);
    debug(F101,"ttpkt speed","",(int) speed);

    ttprty = parity;                    /* Let other tt functions see these. */
    ttpflg = 0;				/* Parity not sensed yet */
    ttspeed = speed;			/* Make global copy for this module */
    ttpmsk = ttprty ? 0177 : 0377;	/* Parity stripping mask */

    debug(F101,"ttpkt ttpmsk","",ttpmsk);
    debug(F101,"ttpkt netconn","",netconn);

#ifdef NETCONN				/* Nothing to do for telnet */
    if (netconn) return (0);
#endif /* NETCONN */

#ifndef Plan9
    if (ttfdflg && !isatty(ttyfd)) return(0);
#endif /* Plan9 */

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifndef SVORPOSIX			/* Berkeley, V7, etc. */
#ifdef LPASS8
/*
 For some reason, with BSD terminal drivers, you can't set FLOW to XON/XOFF
 after having previously set it to NONE without closing and reopening the
 device.  Unless there's something I overlooked below...
*/
    if (ttflow == FLO_NONE && flow == FLO_XONX && xlocal == 0) {
	debug(F101,"ttpkt executing horrible flow kludge","",0);
	ttclos(0);			/* Close it */
	x = 0;
	ttopen(ttnmsv,&x,ttmdm,0);	/* Open it again */
    }
#endif /* LPASS8 */
#endif /* SVORPOSIX */

#ifdef COHERENT
#undef SVORPOSIX
#endif /* COHERENT */

    if (xflow != FLO_DIAL && xflow != FLO_DIAX)
      ttflow = xflow;			/* Now make this available too. */

#ifndef NOLOCAL
    if (xlocal) {
	s2 = (int) (speed / 10L);	/* Convert bps to cps */
	s = ttsspd(s2);			/* Check and set the speed */
	debug(F101,"ttpkt carrier","",xflow);
 	carrctl(&ttraw, xflow != FLO_DIAL /* Carrier control */
		&& (ttcarr == CAR_ON || (ttcarr == CAR_AUT && ttmdm != 0)));
	tvtflg = 0;			/* So ttvt() will work next time */
    }
#endif /* NOLOCAL */

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifndef SVORPOSIX			/* BSD section */
    if (flow == FLO_RTSC ||		/* Hardware flow control */
	flow == FLO_DTRC ||
	flow == FLO_DTRT) {
	tthflow(flow, 1, &ttraw);
	debug(F100,"ttpkt hard flow, TANDEM off, RAW on","",0);
	ttraw.sg_flags &= ~TANDEM;	/* Turn off software flow control */
	ttraw.sg_flags |= RAW;		/* Enter raw mode */
    } else if (flow == FLO_NONE) {	/* No flow control */
	debug(F100,"ttpkt no flow, TANDEM off, RAW on","",0);
	ttraw.sg_flags &= ~TANDEM;	/* Turn off software flow control */
	tthflow(flow, 0, &ttraw);	/* Turn off any hardware f/c too */
	ttraw.sg_flags |= RAW;		/* Enter raw mode */
    } else if (flow == FLO_KEEP) {	/* Keep device's original setting */
	debug(F100,"ttpkt keeping original TANDEM","",0);
	ttraw.sg_flags &= ~TANDEM;
	ttraw.sg_flags |= (ttold.sg_flags & TANDEM);
	/* NOTE: We should also handle hardware flow control here! */
    }

/* SET FLOW XON/XOFF is in effect, or SET FLOW KEEP resulted in Xon/Xoff */

    if ((flow == FLO_XONX) || (ttraw.sg_flags & TANDEM)) {
	debug(F100,"ttpkt turning on TANDEM","",0);
	ttraw.sg_flags |= TANDEM;	/* So ask for it. */

#ifdef LPASS8				/* Can pass 8-bit data through? */
/* If the LPASS8 local mode is available, then flow control can always  */
/* be used, even if parity is none and we are transferring 8-bit data.  */
/* But we only need to do all this if Xon/Xoff is requested. */
/* BUT... this tends not to work through IP or LAT connections, terminal */
/* servers, telnet, rlogin, etc, so it is currently disabled. */
	x = LPASS8;			/* If LPASS8 defined, then */
	debug(F100,"ttpkt executing LPASS8 code","",0);
	if (lmodef) {			/* TIOCLBIS must be too. */
	    x = ioctl(ttyfd,TIOCLBIS,&x); /* Try to set LPASS8. */
	    if (x < 0) {
		debug(F100,"ttpkt TIOCLBIS error","",0);
	    } else {
		lmodef++;
		debug(F100,"ttpkt TIOCLBIS ok","",0);
	    }
	}
/*
 But if we use LPASS8 mode, we must explicitly turn off
 terminal interrupts of all kinds.
*/
#ifdef TIOCGETC				/* Not rawmode, */
	if (tcharf && (xlocal == 0)) {	/* must turn off */
	    tchnoi.t_intrc = -1;	/* interrupt character */
	    tchnoi.t_quitc = -1;	/* and quit character. */
	    tchnoi.t_startc = 17;	/* Make sure xon */
	    tchnoi.t_stopc = 19;	/* and xoff not ignored. */
#ifndef NOBRKC
	    tchnoi.t_eofc = -1;		/* eof character. */ 
	    tchnoi.t_brkc = -1;		/* brk character. */ 
#endif /* NOBRKC */
	    if (ioctl(ttyfd,TIOCSETC,&tchnoi) < 0) {
		debug(F100,"ttpkt TIOCSETC failed","",0);
	    } else {
		tcharf = 1;
		debug(F100,"ttpkt TIOCSETC ok","",0);
	    }
#ifdef COMMENT
/* only for paranoid debugging */
	    if (tcharf) {
		struct tchars foo;
		char tchbuf[100];
		ioctl(0,TIOCGETC,&foo);
		sprintf(tchbuf,
		    "intr=%d,quit=%d, start=%d, stop=%d, eof=%d, brk=%d",
		    foo.t_intrc, foo.t_quitc, foo.t_startc,
		    foo.t_stopc, foo.t_eofc,  foo.t_brkc);
		debug(F110,"ttpkt chars",tchbuf,0);
	    }
#endif /* COMMENT */
	}
	ttraw.sg_flags |= CBREAK;	/* Needed for unknown reason */
#endif /* TIOCGETC */

/* Prevent suspend during packet mode */
#ifdef TIOCGLTC				/* Not rawmode, */
	if (ltcharf && (xlocal == 0)) {	/* must turn off */
	    ltchnoi.t_suspc = -1;	/* suspend character */
	    ltchnoi.t_dsuspc = -1;	/* and delayed suspend character */
	    if (ioctl(ttyfd,TIOCSLTC,&tchnoi) < 0) {
		debug(F100,"ttpkt TIOCSLTC failed","",0);
	    } else {
		ltcharf = 1;
		debug(F100,"ttpkt TIOCSLTC ok","",0);
	    }
	}
#endif /* TIOCGLTC */

#else /* LPASS8 not defined */

/* Previously, BSD-based implementations always */
/* used rawmode for packets.  Now, we use rawmode only if parity is NONE. */
/* This allows the flow control requested above to actually work, but only */
/* if the user asks for parity (which also means they get 8th-bit quoting). */

	if (parity) {			/* If parity, */
	    ttraw.sg_flags &= ~RAW;	/* use cooked mode */
#ifdef COMMENT
/* WHY??? */
	    if (xlocal)
#endif /* COMMENT */
	      ttraw.sg_flags |= CBREAK;
	    debug(F101,"ttpkt cooked, cbreak, parity","",parity);
#ifdef TIOCGETC				/* Not rawmode, */
	    if (tcharf && (xlocal == 0)) { /* must turn off */
		tchnoi.t_intrc = -1;	/* interrupt character */
		tchnoi.t_quitc = -1;	/* and quit character. */
		tchnoi.t_startc = 17;	/* Make sure xon */
		tchnoi.t_stopc = 19;	/* and xoff not ignored. */
#ifndef NOBRKC
		tchnoi.t_eofc = -1;	/* eof character. */ 
		tchnoi.t_brkc = -1;	/* brk character. */ 
#endif /* NOBRKC */
		if (ioctl(ttyfd,TIOCSETC,&tchnoi) < 0) {
		    debug(F100,"ttpkt TIOCSETC failed","",0);
		} else {
		    tcharf = 1;
		    debug(F100,"ttpkt TIOCSETC ok","",0);
		}
	    }
#endif /* TIOCGETC */
#ifdef TIOCGLTC				/* Not rawmode, */
/* Prevent suspend during packet mode */
	    if (ltcharf && (xlocal == 0)) { /* must turn off */
		ltchnoi.t_suspc = -1;	/* suspend character */
		ltchnoi.t_dsuspc = -1;	/* and delayed suspend character */
		if (ioctl(ttyfd,TIOCSLTC,&tchnoi) < 0) {
		    debug(F100,"ttpkt TIOCSLTC failed","",0);
		} else {
		    ltcharf = 1;
		    debug(F100,"ttpkt TIOCSLTC ok","",0);
		}
	    }
#endif /* TIOCGLTC */
	} else {			/* If no parity, */
	    ttraw.sg_flags |= RAW;	/* must use 8-bit raw mode. */
	    debug(F101,"ttpkt setting rawmode, parity","",parity);
	}
#endif /* LPASS8 */
    } /* End of Xon/Xoff section */

    /* Don't echo, don't map CR to CRLF on output, don't fool with case */
#ifdef LCASE
    ttraw.sg_flags &= ~(ECHO|CRMOD|LCASE);
#else
    ttraw.sg_flags &= ~(ECHO|CRMOD);
#endif /* LCASE */

#ifdef TOWER1
    ttraw.sg_flags &= ~ANYP;            /* Must set this on old Towers */
#endif /* TOWER1 */

#ifdef BELLV10
    if (ioctl(ttyfd,TIOCSETP,&ttraw) < 0) return(-1); /* Set the new modes. */
#else
    if (stty(ttyfd,&ttraw) < 0) return(-1); /* Set the new modes. */
#endif /* BELLV10 */
    debug(F100,"ttpkt stty ok","",0);

#ifdef sony_news
    x = xlocal ? km_ext : km_con;	/* Put line in ASCII mode. */
    if (x != -1) {			/* Make sure we know original modes. */
	x &= ~KM_TTYPE;
	x |= KM_ASCII;
	if (ioctl(ttyfd,TIOCKSET, &x) < 0) {
	    perror("ttpkt can't set ASCII mode");
	    debug(F101,"ttpkt error setting ASCII mode","",x);
	    return(-1);
	}
    }
    debug(F100,"ttpkt set ASCII mode ok","",0);
#endif /* sony_news */

    if (xlocal == 0)			/* Turn this off so we can read */
      signal(SIGINT,SIG_IGN);		/* Ctrl-C chars typed at console */

    tvtflg = 0;				/* So ttvt() will work next time */
    return(0);

#endif /* Not ATTSV or POSIX */

/* AT&T UNIX and POSIX */

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifdef SVORPOSIX
    if (flow == FLO_XONX) {		/* Xon/Xoff */
	ttraw.c_iflag |= (IXON|IXOFF);
	tthflow(flow, 0, &ttraw);
    } else if (flow == FLO_NONE) {	/* None */
	/* NOTE: We should also turn off hardware flow control here! */
	ttraw.c_iflag &= ~(IXON|IXOFF);
	tthflow(flow, 0, &ttraw);
    } else if (flow == FLO_KEEP) {	/* Keep */
	ttraw.c_iflag &= ~(IXON|IXOFF);	/* Turn off Xon/Xoff flags */
	ttraw.c_iflag |= (ttold.c_iflag & (IXON|IXOFF)); /* OR in old ones */
	/* NOTE: We should also handle hardware flow control here! */
#ifdef __linux__
/* In Linux case, we do this, which is unlikely to be portable */
        ttraw.c_cflag &= ~CRTSCTS;	/* Turn off RTS/CTS flag */
        ttraw.c_cflag |= (ttold.c_cflag & CRTSCTS); /* OR in old one */
#endif /* __linux__ */
    } else if (flow == FLO_RTSC ||	/* Hardware */
	       flow == FLO_DTRC ||
	       flow == FLO_DTRT) {
	ttraw.c_iflag &= ~(IXON|IXOFF);	/* (190) */
	tthflow(flow, 1, &ttraw);
    }
    ttraw.c_lflag &= ~(ICANON|ECHO);
    ttraw.c_lflag &= ~ISIG;		/* Do NOT check for interrupt chars */

#ifndef OXOS
#ifdef QNX
    if (flow != FLO_RTSC && flow != FLO_DTRC && flow != FLO_DTRT)
#endif /* QNX */
#ifndef COHERENT
      ttraw.c_lflag &= ~IEXTEN;		/* Turn off ^O/^V processing */
#endif /* COHERENT */
#else /* OXOS */
    ttraw.c_cc[VDISCARD] = ttraw.c_cc[VLNEXT] = CDISABLE;
#endif /* OXOS */
    ttraw.c_lflag |= NOFLSH;		/* Don't flush */
    ttraw.c_iflag |= (BRKINT|IGNPAR);
#ifdef ATTSV
#ifdef BSD44
    ttraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|INPCK|ISTRIP|IXANY);
#else
    ttraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|IUCLC|INPCK|ISTRIP|IXANY);
#endif /* BSD44 */
#else /* POSIX */
    ttraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|INPCK|ISTRIP);
#endif /* ATTSV */
    ttraw.c_oflag &= ~OPOST;
    ttraw.c_cflag &= ~(CSIZE|PARENB);
    ttraw.c_cflag |= (CS8|CREAD|HUPCL);
#ifdef IX370
    ttraw.c_cc[4] = 48;  /* So Series/1 doesn't interrupt on every char */
    ttraw.c_cc[5] = 1;
#else
#ifndef VEOF	/* for DGUX this is VEOF, not VMIN */
    ttraw.c_cc[4] = 1;   /* [VMIN]  return max of this many characters or */
#else
#ifndef OXOS
#ifdef VMIN
    ttraw.c_cc[VMIN] = 1;
#endif /* VMIN */
#else /* OXOS */
    ttraw.c_min = 1;
#endif /* OXOS */
#endif /* VEOF */
#ifndef VEOL	/* for DGUX this is VEOL, not VTIME */
    ttraw.c_cc[5] = 0;	 /* [VTIME] when this many secs/10 expire w/no input */
#else
#ifndef OXOS
#ifdef VTIME
    ttraw.c_cc[VTIME] = 0;
#endif /* VTIME */
#else /* OXOS */
    ttraw.c_time = 0;
#endif /* OXOS */
#endif /* VEOL */
#endif /* IX370 */

#ifdef VINTR				/* Turn off interrupt character */
    if (xlocal == 0)			/* so ^C^C can break us out of */
      ttraw.c_cc[VINTR] = 0;		/* packet mode. */
#endif /* VINTR */

#ifdef Plan9
    if (p9ttyparity('n') < 0)
	return -1;
#else
#ifdef BSD44ORPOSIX
#ifdef BEBOX
    ttraw.c_cc[VMIN] = 0;		/* DR7 can only poll. */
#endif /* BEBOX */
    if (tcsetattr(ttyfd,TCSADRAIN,&ttraw) < 0) return(-1);
#else /* ATTSV */
    if (ioctl(ttyfd,TCSETAW,&ttraw) < 0) return(-1);  /* set new modes . */
#endif /* BSD44ORPOSIX */
#endif /* Plan9 */
    tvtflg = 0;
    return(0);
#endif /* ATTSV */

#ifdef COHERENT
#undef SVORPOSIX
#endif /* COHERENT */

}

/*  T T S E T F L O W  --  Set flow control immediately.  */

int
ttsetflow(flow) int flow; {
    if (ttyfd < 0)			/* Communications channel must open */
      return(-1);

    if (flow == FLO_RTSC ||		/* Hardware flow control... */
	flow == FLO_DTRC ||
	flow == FLO_DTRT) {
	tthflow(flow, 1, &ttraw);
#ifndef SVORPOSIX
	ttraw.sg_flags &= ~TANDEM;	/* Turn off software flow control */
#else
	ttraw.c_iflag &= ~(IXON|IXOFF);
#endif /* SVORPOSIX */

    } else if (flow == FLO_XONX) {	/* Xon/Xoff... */

#ifndef SVORPOSIX
	ttraw.sg_flags |= TANDEM;
#else
	ttraw.c_iflag |= (IXON|IXOFF);
#endif /* SVORPOSIX */
	tthflow(FLO_RTSC, 0, &ttraw);	/* Turn off hardware flow control */

    } else if (flow == FLO_NONE) {	/* No flow control */

#ifndef SVORPOSIX
	ttraw.sg_flags &= ~TANDEM;	/* Turn off software flow control */
#else
	ttraw.c_iflag &= ~(IXON|IXOFF);
#endif /* SVORPOSIX */
	tthflow(FLO_RTSC, 0, &ttraw);	/* Turn off any hardware f/c too */
    }

/* Set the new modes... */

#ifndef SVORPOSIX			/* BSD and friends */
#ifdef BELLV10
    if (ioctl(ttyfd,TIOCSETP,&ttraw) < 0)
      return(-1);
#else
    if (stty(ttyfd,&ttraw) < 0)
      return(-1);
#endif /* BELLV10 */
#else
#ifdef BSD44ORPOSIX			/* POSIX */
    if (tcsetattr(ttyfd,TCSADRAIN,&ttraw) < 0)
      return(-1);
#else					/* System V */
    if (ioctl(ttyfd,TCSETAW,&ttraw) < 0)
      return(-1);
#endif /* BSD44ORPOSIX */
#endif /* SVORPOSIX */
    return(0);
}

/*  T T V T -- Condition communication line for use as virtual terminal  */

#ifndef NOLOCAL
int
#ifdef CK_ANSIC
ttvt(long speed, int flow)
#else
ttvt(speed,flow) long speed; int flow;
#endif /* CK_ANSIC */
/* ttvt */ {
    int s, s2;

    debug(F101,"ttvt ttyfd","",ttyfd);
    debug(F101,"ttvt tvtflg","",tvtflg);
    debug(F101,"ttvt speed","",speed);
    debug(F101,"ttvt flow","",flow);
    if (ttyfd < 0) return(-1);          /* Not open. */
#ifdef NETCONN
    ttpmsk = 0xff;
    if (netconn) {
	tvtflg = 1;			/* Network connections */
	debug(F100,"ttvt network connection, skipping...","",0);
	return(0);			/* require no special setup */
    }
#endif /* NETCONN */

    if (tvtflg != 0 && speed == ttspeed && flow == ttflow && ttcarr == curcarr)
      {
	  debug(F100,"ttvt modes already set, skipping...","",0);
	  return(0);			/* Already been called. */
      }

    if (ttfdflg
#ifndef Plan9
	&& !isatty(ttyfd)
#endif /* Plan9 */
	) {
	debug(F100,"ttvt using external fd, skipping...","",0);
	return(0);
    }

    debug(F100,"ttvt setting modes...","",0);

    if (xlocal) {			/* For external lines... */
	s2 = (int) (speed / 10L);
	s = ttsspd(s2);			/* Check/set the speed */
	carrctl(&tttvt, flow != FLO_DIAL /* Do carrier control */
		&& (ttcarr == CAR_ON || (ttcarr == CAR_AUT && ttmdm != 0)));
    } else
      s = s2 = -1;

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifndef SVORPOSIX
    /* Berkeley, V7, etc */
    if (flow == FLO_RTSC ||		/* Hardware flow control */
	flow == FLO_DTRC ||
	flow == FLO_DTRT)
      tthflow(flow, 1, &tttvt);
    if (flow == FLO_XONX) {		/* Xon/Xoff flow control */
	tttvt.sg_flags |= TANDEM;	/* Ask for it. */
	tthflow(flow, 0, &tttvt);	/* Turn off hardware f/c */
    } else if (flow == FLO_KEEP) {
	tttvt.sg_flags &= ~TANDEM;
	tttvt.sg_flags |= (ttold.sg_flags & TANDEM);
	/* NOTE: We should also handle hardware flow control here! */
    } else if (flow == FLO_NONE) {
	tttvt.sg_flags &= ~TANDEM;	/* No Xon/Xoff */
	tthflow(flow, 0, &tttvt);	/* Turn off hardware f/c */
    }
    tttvt.sg_flags |= RAW;              /* Raw mode in all cases */
#ifdef TOWER1
    tttvt.sg_flags &= ~(ECHO|ANYP);     /* No echo or parity */
#else
    tttvt.sg_flags &= ~ECHO;            /* No echo */
#endif /* TOWER1 */

#ifdef BELLV10
    if (ioctl(ttyfd,TIOCSETP,&tttvt) < 0) /* Set the new modes */
      return(-1);
#else
    if (stty(ttyfd,&tttvt) < 0)		/* Set the new modes */
      return(-1);
#endif /* BELLV10 */

#else /* It is ATTSV or POSIX */

    if (flow == FLO_RTSC ||		/* Hardware flow control */
	flow == FLO_DTRC ||
	flow == FLO_DTRT) {
	tthflow(flow, 1, &tttvt);
    } else if (flow == FLO_XONX) {	/* Software flow control */
	tttvt.c_iflag |= (IXON|IXOFF);	/* On if requested. */
	tthflow(flow, 0, &tttvt);	/* Turn off hardware f/c */
    } else if (flow == FLO_KEEP) {
	tttvt.c_iflag &= ~(IXON|IXOFF);	/* Turn off Xon/Xoff flags */
	tttvt.c_iflag |= (ttold.c_iflag & (IXON|IXOFF)); /* OR in old ones */
	/* NOTE: We should also handle hardware flow control here! */
#ifdef __linux__
        tttvt.c_cflag &= ~CRTSCTS;	/* Turn off RTS/CTS flag */
        tttvt.c_cflag |= (ttold.c_cflag & CRTSCTS); /* OR in old one */
#endif /* __linux__ */
    } else if (flow == FLO_NONE) {	/* NONE */
	tthflow(flow, 0, &tttvt);	/* Turn off hardware f/c */
	tttvt.c_iflag &= ~(IXON|IXOFF);	/* Turn off Xon/Xoff */
    }

#ifndef OXOS
#ifdef COHERENT
    tttvt.c_lflag &= ~(ISIG|ICANON|ECHO);
#else
    tttvt.c_lflag &= ~(ISIG|ICANON|ECHO|IEXTEN);
#endif /* COHERENT */
#ifdef QNX
    /* Needed for hwfc */
    if (flow == FLO_RTSC || flow == FLO_DTRC || flow == FLO_DTRT)
      tttvt.c_lflag |= IEXTEN;
#endif /* QNX */
#else /* OXOS */
    tttvt.c_lflag &= ~(ISIG|ICANON|ECHO);
    tttvt.c_cc[VDISCARD] = tttvt.c_cc[VLNEXT] = CDISABLE;
#endif /* OXOS */
    tttvt.c_iflag |= (IGNBRK|IGNPAR);
#ifdef ATTSV
#ifdef BSD44
    tttvt.c_iflag &= ~(INLCR|IGNCR|ICRNL|BRKINT|INPCK|ISTRIP|IXANY);
#else
    tttvt.c_iflag &= ~(INLCR|IGNCR|ICRNL|IUCLC|BRKINT|INPCK|ISTRIP|IXANY);
#endif /* BSD44 */
#else /* POSIX */
    tttvt.c_iflag &= ~(INLCR|IGNCR|ICRNL|BRKINT|INPCK|ISTRIP);
#endif /* ATTSV */
    tttvt.c_oflag &= ~OPOST;
    tttvt.c_cflag &= ~(CSIZE|PARENB);
    tttvt.c_cflag |= (CS8|CREAD|HUPCL);
#ifndef VEOF	/* DGUX termio has VEOF at entry 4, see comment above */
    tttvt.c_cc[4] = 1;
#else
#ifndef OXOS
#ifdef VMIN
    tttvt.c_cc[VMIN] = 1;
#endif /* VMIN */
#else /* OXOS */
    tttvt.c_min = 1;
#endif /* OXOS */
#endif /* VEOF */
#ifndef VEOL	/* DGUX termio has VEOL at entry 5, see comment above */
    tttvt.c_cc[5] = 0;
#else
#ifndef OXOS
#ifdef VTIME
    tttvt.c_cc[VTIME] = 0;
#endif /* VTIME */
#else /* OXOS */
    tttvt.c_time = 0;
#endif /* OXOS */
#endif /* VEOL */

#ifdef Plan9
    if (p9ttyparity('n') < 0)
      return -1;
#else
#ifdef BSD44ORPOSIX
#ifdef BEBOX
    tttvt.c_cc[VMIN] = 0;		/* DR7 can only poll. */
#endif /* BEBOX */
    if (tcsetattr(ttyfd,TCSADRAIN,&tttvt) < 0) return(-1);
#else /* ATTSV */
    if (ioctl(ttyfd,TCSETAW,&tttvt) < 0) return(-1);  /* set new modes . */
#endif /* BSD44ORPOSIX */
#endif /* Plan9 */
#endif /* ATTSV */

    ttspeed = speed;			/* Done, remember how we were */
    ttflow = flow;			/* called, so we can decide how to */
    tvtflg = 1;				/* respond next time. */
    debug(F101,"ttvt done","",tvtflg);
    return(0);

#ifdef COHERENT
#undef SVORPOSIX
#endif /* COHERENT */

}
#endif /* NOLOCAL */

/*  T T S S P D  --  Checks and sets transmission rate.  */

/*  Call with speed in characters (not bits!) per second. */
/*  Returns -1 on failure, 0 if it did nothing, 1 if it changed the speed. */

#ifndef NOLOCAL
int
ttsspd(cps) int cps; {
#ifdef POSIX
/* Watch out, speed_t should be unsigned, so don't compare with -1, etc... */
    speed_t				
#else
    int
#endif /* POSIX */
      s, s2;
    int ok = 1;				/* Speed check result, assume ok */

#ifdef __linux__			/* Linux... */
#ifdef ASYNC_SPD_MASK
    unsigned int spd_flags = 0;
    struct serial_struct serinfo;
#endif /* ASYNC_SPD_MASK */
#endif /* __linux__ */

    debug(F101,"ttsspd cps","",cps);
    debug(F101,"ttsspd ttyfd","",ttyfd);
    debug(F101,"ttsspd xlocal","",xlocal);

    if (ttyfd < 0 || xlocal == 0 )	/* Don't set speed on console */
      return(0);

#ifdef	NETCONN
    if (netconn)
      return(0);
#endif	/* NETCONN */

    if (cps < 0) return(-1);
    s = s2 = 0;				/* NB: s and s2 might be unsigned */

    /* First check that the given speed is valid. */

#ifdef Plan9
    ok = 1;
#else
    switch (cps) {
#ifndef MINIX
      case 0:   s = B0;    break;
      case 5:   s = B50;   break;
      case 7:   s = B75;   break;
#endif /* MINIX */
      case 11:  s = B110;  break;
#ifndef MINIX
      case 15:  s = B150;  break;
      case 20:  s = B200;  break;
#endif /* MINIX */
      case 30:  s = B300;  break;
#ifndef MINIX
      case 60:  s = B600;  break;
#endif /* MINIX */
      case 120: s = B1200; break;
#ifndef MINIX
      case 180: s = B1800; break;
#endif /* MINIX */
      case 240: s = B2400; break;
      case 480: s = B4800; break;
#ifndef MINIX
      case 888: s = B75; s2 = B1200; break; /* 888 = 75/1200 split speed */
#endif /* MINIX */
      case 960: s = B9600; break;
#ifdef B14400
      case 1440: s = B14400; break;
#endif /* B14400 */
#ifdef B19200
      case 1920: s = B19200; break;
#else
#ifdef EXTA
      case 1920: s = EXTA; break;
#endif /* EXTA */
#endif /* B19200 */
#ifdef B28800
      case 2880: s = B28800; break;
#endif /* B28800 */
#ifdef B38400
      case 3840: s = B38400;
#ifdef __linux__
#ifdef ASYNC_SPD_MASK
        spd_flags = ~ASYNC_SPD_MASK;	/* Nonzero, but zero flags */
#endif /* ASYNC_SPD_MASK */
#endif /* __linux__ */
	break;
#else /* B38400 not defined... */
#ifdef EXTB
      case 3840: s = EXTB; break;
#endif /* EXTB */
#endif /* B38400 */

#ifdef HPUX
#ifdef _B57600
      case 5760: s = _B57600; break;
#endif /* _B57600 */
#ifdef _B115200
      case 11520: s = _B115200; break;
#endif /* _B115200 */
#else
#ifdef __linux__
#ifdef ASYNC_SPD_MASK
/*
  This bit from <carlo@sg.tn.tudelft.nl>:
  "Only note to make is maybe this: When the ASYNC_SPD_CUST flags are set then
  setting the speed to 38400 will set the custom speed (and ttgspd returns
  38400), but speeds 57600 and 115200 won't work any more because I didn't
  want to mess up the speed flags when someone is doing sophisticated stuff
  like custom speeds..."
*/
      case 5760: s = B38400; spd_flags = ASYNC_SPD_HI; break;
      case 11520: s = B38400; spd_flags = ASYNC_SPD_VHI; break;
#endif /* ASYNC_SPD_MASK */
#else
#ifdef QNX
#ifdef B57600
      case 5760: s = B57600; break;
#endif /* B57600 */
#ifdef B76800
      case 7680: s = B76800; break;
#endif /* B76800 */
#ifdef B115200
      case 11520: s = B115200; break;
#endif /* B115200 */
#else
#ifdef B57600
      case 5760: s = B57600; break;
#endif /* B57600 */
#ifdef B115200
      case 11520: s = B115200; break;
#endif /* B115200 */
#endif /* QNX */
#endif /* __linux__ */
#endif /* HPUX */
      default:
	ok = 0;				/* Good speed not found, so not ok */
	break;
    }
#endif /* Plan9 */
    debug(F101,"ttsspd ok","",ok);
    debug(F101,"ttsspd s","",s);

    if (!ok) {
	debug(F100,"ttsspd fails","",0);
	return(-1);
    } else {
#ifdef BSD44ORPOSIX
	int x;
#endif /* BSD44ORPOSIX */

	if (!s2) s2 = s;		/* Set input speed */

#ifdef Plan9
	if (p9ttsspd(cps) < 0)
	  return -1;
#else
#ifdef BSD44ORPOSIX
	x = tcgetattr(ttyfd,&ttcur);	/* Get current speed */
	debug(F101,"ttsspd tcgetattr","",x);
	if (x < 0)
	  return(-1);
#ifdef __linux__
#ifdef ASYNC_SPD_MASK
	debug(F101,"ttsspd spd_flags","",spd_flags);
	if (spd_flags && spd_flags != ASYNC_SPD_CUST) {
	    if (ioctl(ttyfd, TIOCGSERIAL, &serinfo) < 0) {
		debug(F100,"ttsspd: TIOCGSERIAL failed","",0);
		return(-1);
	    } else debug(F100,"ttsspd: TIOCGSERIAL ok","",0);
	    serinfo.flags &= ~ASYNC_SPD_MASK;
	    serinfo.flags |= (spd_flags & ASYNC_SPD_MASK);
	    if (ioctl(ttyfd, TIOCSSERIAL, &serinfo) < 0)
	      return(-1);
	}
#endif /* ASYNC_SPD_MASK */
#endif /* __linux__ */
	cfsetospeed(&ttcur,s);
	cfsetispeed(&ttcur,s2);
	cfsetospeed(&ttraw,s);
	cfsetispeed(&ttraw,s2);
	cfsetospeed(&tttvt,s);
	cfsetispeed(&tttvt,s2);
	cfsetospeed(&ttold,s);
	cfsetispeed(&ttold,s2);
	x = tcsetattr(ttyfd,TCSADRAIN,&ttcur);
	debug(F101,"ttsspd tcsetattr","",x);
	if (x < 0) return(-1);
#else
#ifdef ATTSV
	if (cps == 888) return(-1);	/* No split speeds, sorry. */
	if (ioctl(ttyfd,TCGETA,&ttcur) < 0) return(-1);
	ttcur.c_cflag &= ~CBAUD;
	ttcur.c_cflag |= s;
	tttvt.c_cflag &= ~CBAUD;
	tttvt.c_cflag |= s;
	ttraw.c_cflag &= ~CBAUD;
	ttraw.c_cflag |= s;
	ttold.c_cflag &= ~CBAUD;
	ttold.c_cflag |= s;
	if (ioctl(ttyfd,TCSETAW,&ttcur) < 0) return(-1);
#else
#ifdef BELLV10
	if (ioctl(ttyfd,TIOCGDEV,&tdcur) < 0) return(-1);
	tdcur.ispeed = s2;
	tdcur.ospeed = s;
	errno = 0;
	ok = ioctl(ttyfd,TIOCSDEV,&tdcur);
	debug(F101,"ttsspd BELLV10 ioctl","",ok);
	if (ok < 0) {
	    perror(ttnmsv);
	    debug(F101,"ttsspd BELLV10 errno","",ok);
	    return(-1);
	}
#else
	if (gtty(ttyfd,&ttcur) < 0) return(-1);
	ttcur.sg_ospeed = s; ttcur.sg_ispeed = s2;
	tttvt.sg_ospeed = s; tttvt.sg_ispeed = s2;
	ttraw.sg_ospeed = s; ttraw.sg_ispeed = s2;
	ttold.sg_ospeed = s; ttold.sg_ispeed = s2;
	if (stty(ttyfd,&ttcur) < 0) return(-1);
#endif /* BELLV10 */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
#endif /* Plan9 */
    }
    return(1);				/* Return 1 = success. */
}
#endif /* NOLOCAL */

/* C O N G S P D  -  Get speed of console terminal  */

long
congspd() {
/*
  This is a disgusting hack.  The right way to do this would be to pass an
  argument to ttgspd(), but then we'd need to change the Kermit API and
  all of the ck?tio.c modules.  (Currently used only for rlogin.)
*/
    int t1, t2;
    long spd;
#ifdef NETCONN
    t2 = netconn;
    netconn = 0;
#endif /* NETCONN */
    t1 = ttyfd;
    ttyfd = -1;
    spd = ttgspd();
    debug(F101,"congspd","",spd);
#ifdef NETCONN
    netconn = t2;
#endif /* NETCONN */
    ttyfd = t1;
    return(spd);
}

/* T T G S P D  -  Get speed of currently selected tty line  */

/*
  Unreliable.  After SET LINE, it returns an actual speed, but not necessarily
  the real speed.  On some systems, it returns the line's nominal speed, from
  /etc/ttytab.  Even if you SET SPEED to something else, this function might
  not notice.
*/
long
ttgspd() {				/* Get current serial device speed */
#ifdef POSIX
    speed_t				/* Should be unsigned */
#else
    int					/* Isn't unsigned */
#endif /* POSIX */
      s;
    long ss;
#ifdef __linux__
#ifdef ASYNC_SPD_MASK
    unsigned int spd_flags = 0;
    struct serial_struct serinfo;
#endif /* ASYNC_SPD_MASK */
#endif /* __linux__ */

#ifdef NETCONN
    if (netconn) return(-1);		/* -1 if network connection */
#endif /* NETCONN */

    debug(F101,"ttgspd ttyfd","",ttyfd);
#ifdef Plan9
    if (ttyfd < 0)
      ss = -1;
    else
      ss = ttylastspeed;
#else
#ifdef __linux__
#ifdef ASYNC_SPD_MASK
    debug(F100,"ttgspd Linux ASYNC_SPD_MASK defined","",0);
#else
    debug(F100,"ttgspd Linux ASYNC_SPD_MASK not defined","",0);
#endif /* ASYNC_SPD_MASK */
#endif /* __linux__ */

    if (ttyfd < 0) {
#ifdef BSD44ORPOSIX
	s = cfgetospeed(&ccold);
	debug(F101,"ttgspd cfgetospeed 1 BSDORPOSIX","",s);
#else
#ifdef ATTSV
	s = ccold.c_cflag & CBAUD;
	debug(F101,"ttgspd cfgetospeed 1 ATTSV","",s);
#else
	s = ccold.sg_ospeed;		/* (obtained by congm()) */
	debug(F101,"ttgspd cfgetospeed 1 catch-all","",s);
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */

    } else {
#ifdef BSD44ORPOSIX
	if (tcgetattr(ttyfd,&ttcur) < 0) return(-1);
	s = cfgetospeed(&ttcur);
	debug(F101,"ttgspd cfgetospeed 2 BSDORPOSIX","",s);
#ifdef __linux__
#ifdef ASYNC_SPD_MASK
	if (ioctl(ttyfd,TIOCGSERIAL,&serinfo) > -1)
	  spd_flags = serinfo.flags & ASYNC_SPD_MASK;
	debug(F101,"ttgspd spd_flags","",spd_flags);
#endif /* ASYNC_SPD_MASK */
#endif /* __linux__ */
#else
#ifdef ATTSV
	if (ioctl(ttyfd,TCGETA,&ttcur) < 0) return(-1);
	s = ttcur.c_cflag & CBAUD;
	debug(F101,"ttgspd cfgetospeed 2 ATTSV","",s);
#else
#ifdef BELLV10
	if (ioctl(ttyfd,TIOCGDEV,&tdcur) < 0) return(-1);
	s = tdcur.ospeed;
	debug(F101,"ttgspd cfgetospeed 2 BELLV10","",s);
#else
	if (gtty(ttyfd,&ttcur) < 0) return(-1);
	s = ttcur.sg_ospeed;
	debug(F101,"ttgspd cfgetospeed 2 catch-all","",s);
#endif /* BELLV10 */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
    }
    debug(F101,"ttgspd code","",s);
#ifdef __linux__
#ifdef ASYNC_SPD_MASK
    debug(F101,"ttgspd spd_flags","",spd_flags);
#endif /* ASYNC_SPD_MASK */
#endif /* __linux__ */
    switch (s) {
#ifdef B0
      case B0:    ss = 0L; break;
#endif /* B0 */

#ifndef MINIX
/*
 MINIX defines the Bxx symbols to be bps/100, so B50==B75, B110==B134==B150,
 etc, making for many "duplicate case in switch" errors, which are fatal.
*/
#ifdef B50
      case B50:   ss = 50L; break;
#endif /* B50 */
#ifdef B75
      case B75:   ss = 75L; break;
#endif /* B75 */
#endif /* MINIX */

#ifdef B110
      case B110:  ss = 110L; break;
#endif /* B110 */

#ifndef MINIX
#ifdef B134
      case B134:  ss = 134L; break;
#endif /* B134 */
#ifdef B150
      case B150:  ss = 150L; break;
#endif /* B150 */
#endif /* MINIX */

#ifdef B200
      case B200:  ss = 200L; break;
#endif /* B200 */

#ifdef B300
      case B300:  ss = 300L; break;
#endif /* B300 */

#ifdef B600
      case B600:  ss = 600L; break;
#endif /* B600 */

#ifdef B1200
      case B1200: ss = 1200L; break;
#endif /* B1200 */

#ifdef B1800
      case B1800: ss = 1800L; break;
#endif /* B1800 */

#ifdef B2400
      case B2400: ss = 2400L; break;
#endif /* B2400 */

#ifdef B4800
      case B4800: ss = 4800L; break;
#endif /* B4800 */

#ifdef B9600
      case B9600: ss = 9600L; break;
#endif /* B9600 */

#ifdef B19200
      case B19200: ss = 19200L; break;
#else
#ifdef EXTA
      case EXTA: ss = 19200L; break;
#endif /* EXTA */
#endif /* B19200 */

#ifndef MINIX
#ifdef B38400
      case B38400:
        ss = 38400L;
#ifdef __linux__
#ifdef ASYNC_SPD_MASK
        switch(spd_flags) {
          case ASYNC_SPD_HI:  ss =  57600L; break;
          case ASYNC_SPD_VHI: ss = 115200L; break;
	}
#endif /* ASYNC_SPD_MASK */
#endif /* __linux__ */
        break;
#else
#ifdef EXTB
      case EXTB: ss = 38400L; break;
#endif /* EXTB */
#endif /* B38400 */
#endif /* MINIX */

#ifdef HPUX
#ifdef _B57600
      case _B57600: ss = 57600L; break;
#endif /* _B57600 */
#ifdef _B115200
      case _B115200: ss = 115200L; break;
#endif /* _B115200 */
#else
#ifdef QNX
#ifdef B57600
      case B57600: ss = 57600L; break;
#endif /* B57600 */
#ifdef B76800
      case B76800: ss = 76800L; break;
#endif /* B76800 */
#ifdef B115200
      case B115200: ss = 115200L; break;
#endif /* B115200 */
#else
#ifdef B57600
      case B57600: ss = 57600L; break;
#endif /* B57600 */
#ifdef B115200
      case B115200: ss = 115200L; break;
#endif /* B115200 */
#endif /* QNX */
#endif /* HPUX */

      default:
	ss = -1; break;
    }
#endif /* Plan9 */
    debug(F101,"ttgspd speed","",ss);
    return(ss);
}

/* ckumyr.c by Kristoffer Eriksson, ske@pkmab.se, 15 Mar 1990. */

#ifdef COHERENT
#ifdef FIONREAD
#undef FIONREAD
#endif /* FIONREAD */
#define FIONREAD TIOCQUERY
#define PEEKTYPE int
#else  /* Not COHERENT */
#define PEEKTYPE long
#endif /* COHERENT */

#ifdef MYREAD

/* Private buffer for myread() and its companions.  Not for use by anything
 * else.  ttflui() is allowed to reset them to initial values.  ttchk() is
 * allowed to read my_count.
 *
 * my_item is an index into mybuf[].  Increment it *before* reading mybuf[].
 *
 * A global parity mask variable could be useful too.  We could use it to
 * let myread() strip the parity on its own, instead of stripping sign
 * bits as it does now.
 */
#ifdef BIGBUFOK
#define MYBUFLEN 4096
#else
#ifdef pdp11
#define MYBUFLEN 256
#else
#define MYBUFLEN 1024
#endif /* pdp11 */
#endif /* BIGBUFOK */

#ifdef SUNX25
#undef MYBUFLEN
#define MYBUFLEN 256

/*
  On X.25 connections, there is an extra control byte at the beginning.
*/
static CHAR x25buf[MYBUFLEN+1];		/* Communication device input buffer */
static CHAR  *mybuf = x25buf+1;
#else
static CHAR mybuf[MYBUFLEN];
#endif /* SUNX25 */

static int my_count = 0;		/* Number of chars still in mybuf */
static int my_item = -1;		/* Last index read from mybuf[] */

/* myread() -- Efficient read of one character from communications line.
 *
 * Uses a private buffer to minimize the number of expensive read() system
 * calls.  Essentially performs the equivalent of read() of 1 character, which
 * is then returned.  By reading all available input from the system buffers
 * to the private buffer in one chunk, and then working from this buffer, the
 * number of system calls is reduced in any case where more than one character
 * arrives during the processing of the previous chunk, for instance high
 * baud rates or network type connections where input arrives in packets.
 * If the time needed for a read() system call approaches the time for more
 * than one character to arrive, then this mechanism automatically compensates
 * for that by performing bigger read()s less frequently.  If the system load
 * is high, the same mechanism compensates for that too.
 *
 * myread() is a macro that returns the next character from the buffer.  If the
 * buffer is empty, mygetbuf() is called.  See mygetbuf() for possible error
 * returns.
 *
 * This should be efficient enough for any one-character-at-a-time loops.
 * For even better efficiency you might use memcpy()/bcopy() or such between
 * buffers (since they are often better optimized for copying), but it may not
 * be worth it if you have to take an extra pass over the buffer to strip
 * parity and check for CTRL-C anyway.
 *
 * Note that if you have been using myread() from another program module, you
 * may have some trouble accessing this macro version and the private variables
 * it uses.  In that case, just add a function in this module, that invokes the
 * macro.
 */
#define myread()  (--my_count < 0 ? mygetbuf() : 255 & (int)mybuf[++my_item])

/* Specification: Push back up to one character onto myread()'s queue.
 *
 * This implementation: Push back characters into mybuf. At least one character
 * must have been read through myread() before myunrd() may be used.  After
 * EOF or read error, again, myunrd() can not be used.  Sometimes more than
 * one character can be pushed back, but only one character is guaranteed.
 * Since a previous myread() must have read its character out of mybuf[],
 * that guarantees that there is space for at least one character.  If push
 * back was really needed after EOF, a small addition could provide that.
 *
 * myunrd() is currently not called from anywhere inside kermit...
 */
#ifdef NOTUSED
myunrd(ch) CHAR ch; {
    if (my_item >= 0) {
	mybuf[my_item--] = ch;
	++my_count;
    }
}
#endif

/* mygetbuf() -- Fill buffer for myread() and return first character.
 *
 * This function is what myread() uses when it can't get the next character
 * directly from its buffer.  First, it calls a system dependent myfillbuf()
 * to read at least one new character into the buffer, and then it returns
 * the first character just as myread() would have done.  This function also
 * is responsible for all error conditions that myread() can indicate.
 *
 * Returns: When OK	=> a positive character, 0 or greater.
 *	    When EOF	=> -2.
 *	    When error	=> -3, error code in errno.
 *
 * Older myread()s additionally returned -1 to indicate that there was nothing
 * to read, upon which the caller would call myread() again until it got
 * something.  The new myread()/mygetbuf() always gets something.  If it 
 * doesn't, then make it do so!  Any program that actually depends on the old
 * behaviour will break.
 *
 * The older version also used to return -2 both for EOF and other errors,
 * and used to set errno to 9999 on EOF.  The errno stuff is gone, EOF and
 * other errors now return different results, although Kermit currently never
 * checks to see which it was.  It just disconnects in both cases.
 *
 * Kermit lets the user use the quit key to perform some special commands
 * during file transfer.  This causes read(), and thus also mygetbuf(), to
 * finish without reading anything and return the EINTR error.  This should
 * be checked by the caller.  Mygetbuf() could retry the read() on EINTR,
 * but if there is nothing to read, this could delay Kermit's reaction to
 * the command, and make Kermit appear unresponsive.
 *
 * The debug() call should be removed for optimum performance.
 */
int
mygetbuf() {
    errno = 0;
    my_count = myfillbuf();
#ifdef DEBUG
    if (deblog) debug(F101, "myfillbuf read", "", my_count);
#endif /* DEBUG */
    if (my_count <= 0) {
	debug(F101,"mygetbuf errno","",errno);
#ifdef TCPSOCKET
	if (netconn && ttnet == NET_TCPB && errno != 0) {
	    if (errno != EINTR) {
		perror("TCP/IP");	/* Say why */
		ttclos(0);		/* Close the connection. */
	    }
	    return(-3);
	}
#endif /* TCPSOCKET */
	if (!netconn && xlocal && errno) {
	    if (errno != EINTR) {
		perror("SERIAL");	/* Say why */
		ttclos(0);		/* Close the connection. */
	    }
	    return(-3);
	}
	/* Otherwise do what we used to do . . . */
	return(my_count < 0 ? -3 : -2);
    }
    --my_count;
    return((unsigned)(0xff & mybuf[my_item = 0]));
}

/* myfillbuf():
 * System-dependent read() into mybuf[], as many characters as possible.
 *
 * Returns: OK => number of characters read, always more than zero.
 *          EOF => 0
 *          Error => -1, error code in errno.
 *
 * If there is input available in the system's buffers, all of it should be
 * read into mybuf[] and the function return immediately.  If no input is
 * available, it should wait for a character to arrive, and return with that
 * one in mybuf[] as soon as possible.  It may wait somewhat past the first
 * character, but be aware that any such delay lengthens the packet turnaround
 * time during kermit file transfers.  Should never return with zero characters
 * unless EOF or irrecoverable read error.
 *
 * Correct functioning depends on the correct tty parameters being used.
 * Better control of current parameters is required than may have been the
 * case in older Kermit releases.  For instance, O_NDELAY (or equivalent) can 
 * no longer be sometimes off and sometimes on like it used to, unless a 
 * special myfillbuf() is written to handle that.  Otherwise the ordinary 
 * myfillbuf()s may think they have come to EOF.
 *
 * If your system has a facility to directly perform the functioning of
 * myfillbuf(), then use it.  If the system can tell you how many characters
 * are available in its buffers, then read that amount (but not less than 1).
 * If the system can return a special indication when you try to read without
 * anything to read, while allowing you to read all there is when there is
 * something, you may loop until there is something to read, but probably that
 * is not good for the system load.
 */

#ifdef SVORPOSIX
	/* This is for System III/V with VMIN>0, VTIME=0 and O_NDELAY off,
	 * and CLOCAL set any way you like.  This way, read() will do exactly
	 * what is required by myfillbuf(): If there is data in the buffers
	 * of the O.S., all available data is read into mybuf, up to the size
	 * of mybuf.  If there is none, the first character to arrive is
	 * awaited and returned.
	 */
int
myfillbuf() {
#ifdef sxaE50
    /* From S. Dezawa at Fujifilm in Japan.  I don't know why this is */
    /* necessary for the sxa E50, but it is. */
    return read(ttyfd, mybuf, 255);
#else
#ifdef BEBOX
    int n;
    while (1) {
        n = read(ttyfd, mybuf, sizeof(mybuf));
        if (n > 0) {
            debug(F101,"SVORPOSIX myfillbuf","",n);
            return(n);
        }
        snooze(1000.0);
    }
#else
    int n;
    n = read(ttyfd, mybuf, sizeof(mybuf));
    debug(F101,"SVORPOSIX myfillbuf","",n);
    return(n);
#endif /* BEBOX */
#endif /* sxaE50 */
}

#else /* not AT&T or POSIX */

#ifdef aegis
	/* This is quoted from the old myread().  The semantics seem to be
	 * alright, but maybe errno would not need to be set even when
	 * there is no error?  I don't know aegis.
	 */
int
myfillbuf() {
    int count;

    count = ios_$get((short)ttyfd, ios_$cond_opt, mybuf, 256L, st);
    errno = EIO;
    if (st.all == ios_$get_conditional_failed) /* get at least one */
      count = ios_$get((short)ttyfd, 0, mybuf, 1L, st);
    if (st.all == ios_$end_of_file)
      return(0);
    else if (st.all != status_$ok) {
	errno = EIO;
	return(-1);
    }
    return(count);
}
#else /* !aegis */

#ifdef FIONREAD
	/* This is for systems with FIONREAD.  FIONREAD returns the number
	 * of characters available for reading. If none are available, wait
	 * until something arrives, otherwise return all there is.
	 */
int
myfillbuf() {
    PEEKTYPE avail;
    int x;

#ifdef SUNX25
/*
  SunLink X.25 support in this routine from Stefaan A. Eeckels, Eurostat (CEC).
  Depends on SunOS having FIONREAD, not because we use it, but just so this
  code is grouped correctly within the #ifdefs.  Let's hope Solaris keeps it.

  We call x25xin() instead of read() so that Q-Bit packets, which contain
  X.25 service-level information (e.g. PAD parameter changes), can be processed
  transparently to the upper-level code.  This is a blocking read, and so
  we depend on higher-level code (such as ttinc()) to set any necessary alarms.
*/    
    extern int nettype;
    if (netconn && nettype == NET_SX25) {
	while ((x = x25xin(sizeof(x25buf), x25buf)) < 1) ;
	return(x - 1);	        /* "-1" compensates for extra status byte */
    }    
#endif /* SUNX25 */

    errno = 0;
    x = ioctl(ttyfd, FIONREAD, &avail);
#ifdef DEBUG
    if (deblog) {
	debug(F101,"myfillbuf FIONREAD","",x);
	debug(F101,"myfillbuf FIONREAD avail","",avail);
	debug(F101,"myfillbuf FIONREAD errno","",errno);
    }
#endif /* DEBUG */
    if (x < 0 || avail == 0)
      avail = 1;

    if (avail > MYBUFLEN)
      avail = MYBUFLEN;

    errno = 0;
    x = read(ttyfd, mybuf, (int) avail);
#ifdef DEBUG
    if (deblog) {
	debug(F101,"myfillbuf avail","",avail);
	debug(F101,"myfillbuf read","",x);
	debug(F101,"myfillbuf read errno","",errno);
    }
#endif /* DEBUG */
    return(x);
}

#else /* !FIONREAD */
/* Add other systems here, between #ifdef and #else, e.g. NETCONN. */
/* When there is no other possibility, read 1 character at a time. */
int
myfillbuf() {
    return read(ttyfd, mybuf, 1);
}

#endif /* !FIONREAD */
#endif /* !aegis */
#endif /* !ATTSV */

#endif /* MYREAD */

/*  T T F L U I  --  Flush tty input buffer */

int
ttflui() {
#ifdef BSD44
    int n;
#endif /* BSD44 */
#ifndef SVORPOSIX
    int n;
#endif /* SVORPOSIX */
#ifdef Plan9
    int n;
#endif /* Plan9 */

#ifdef MYREAD
/*
  Flush internal MYREAD buffer *FIRST*, in all cases.
*/
    my_count = 0;			/* Reset count to zero */
    my_item = -1;			/* And buffer index to -1 */
#endif /* MYREAD */

#ifdef NETCONN
/*
  Network flush is done specially, in the network support module.
*/
    debug(F100,"ttflui netflui","",0);
    if (netconn) return(netflui());
#endif /* NETCONN */

    debug(F101,"ttflui ttyfd","",ttyfd);
    if (ttyfd < 0) return(-1);

#ifdef aegis
    sio_$control((short)ttyfd, sio_$flush_in, true, st);
    if (st.all != status_$ok) {
	fprintf(stderr, "flush failed: "); error_$print(st);
    } else {      /* sometimes the flush doesn't work */
        for (;;) {
	    char buf[256];
            /* eat all the characters that shouldn't be available */
            ios_$get((short)ttyfd, ios_$cond_opt, buf, 256L, st); /* (void) */
            if (st.all == ios_$get_conditional_failed) break;
            fprintf(stderr, "flush failed(2): "); error_$print(st);
        }
    }
#else
#ifdef BSD44				/* 4.4 BSD */
    n = FREAD;                          /* Specify read queue */
    debug(F100,"ttflui BSD44","",0);
    ioctl(ttyfd,TIOCFLUSH,&n);
#else
#ifdef Plan9
#undef POSIX				/* Uh oh... */
#endif /* Plan9 */
#ifdef POSIX				/* POSIX */
    debug(F100,"ttflui POSIX","",0);
    tcflush(ttyfd,TCIFLUSH);
#else
#ifdef ATTSV				/* System V */
#ifndef VXVE
    debug(F100,"ttflui ATTSV","",0);
    ioctl(ttyfd,TCFLSH,0);
#endif /* VXVE */
#else					/* Not BSD44, POSIX, or Sys V */
#ifdef TIOCFLUSH			/* Those with TIOCFLUSH defined */
#ifdef ANYBSD				/* Berkeley */
    n = FREAD;                          /* Specify read queue */
    debug(F100,"ttflui TIOCFLUSH ANYBSD","",0);
    ioctl(ttyfd,TIOCFLUSH,&n);
#else					/* Others (V7, etc) */
    debug(F100,"ttflui TIOCFLUSH","",0);
    ioctl(ttyfd,TIOCFLUSH,0);
#endif /* ANYBSD */
#else					/* All others... */
/*
  No system call (that we know about) for input buffer flushing.
  So see how many there are and read them in a loop, using ttinc().
  ttinc() is buffered, so we're not getting charged with a system call 
  per character, just a function call.
*/
    if ((n = ttchk()) > 0) {
	debug(F101,"ttflui read loop","",n);
	while ((n--) && ttinc(0) > 0) ;
    }
#endif /* TIOCFLUSH */
#endif /* ATTSV */
#endif /* POSIX */
#ifdef Plan9
#define POSIX
#endif /* Plan9 */
#endif /* BSD44 */
#endif /* aegis */
    return(0);
}

void
ttflux() {
#ifdef MYREAD
/*
  Flush internal MYREAD buffer.
*/
    my_count = 0;			/* Reset count to zero */
    my_item = -1;			/* And buffer index to -1 */
#endif /* MYREAD */
}

int
ttfluo() {				/* Flush output buffer */
#ifdef Plan9
    return 0;
#else
#ifdef POSIX
    return(tcflush(ttyfd,TCOFLUSH));
#else
#ifdef OXOS
    return(ioctl(ttyfd,TCFLSH,1));
#else
    return(0);				/* All others, nothing */
#endif /* OXOS */
#endif /* POSIX */
#endif /* Plan9 */
}

/* Interrupt Functions */

/* Set up terminal interrupts on console terminal */

#ifndef FIONREAD			/* We don't need esctrp() */
#ifndef SELECT				/* if we have any of these... */
#ifndef CK_POLL
#ifndef RDCHK

#ifndef OXOS
#ifdef SVORPOSIX
SIGTYP
esctrp(foo) int foo; {			/* trap console escapes (^\) */
    signal(SIGQUIT,SIG_IGN);            /* ignore until trapped */
    conesc = 1;
    debug(F101,"esctrp caught SIGQUIT","",conesc);
}
#endif /* SVORPOSIX */
#endif /* OXOS */

#ifdef V7
SIGTYP
esctrp(foo) int foo; {			/* trap console escapes (^\) */
    signal(SIGQUIT,SIG_IGN);            /* ignore until trapped */
    conesc = 1;
    debug(F101,"esctrp caught SIGQUIT","",conesc);
}
#endif /* V7 */

#ifdef C70
SIGTYP
esctrp(foo) int foo; {			/* trap console escapes (^\) */
    conesc = 1;
    signal(SIGQUIT,SIG_IGN);            /* ignore until trapped */
}
#endif /* C70 */

#endif /* RDCHK */
#endif /* CK_POLL */
#endif /* SELECT */
#endif /* FIONREAD */

/*  C O N B G T  --  Background Test  */

static int jc = 0;			/* 0 = no job control */

/*
  Call with flag == 1 to prevent signal test, which can not be expected
  to work during file transfer, when SIGINT probably *is* set to SIG_IGN.  

  Call with flag == 0 to use the signal test, but only if the process-group
  test fails, as it does on some UNIX systems, where getpgrp() is buggy,
  requires an argument when the man page says it doesn't, or vice versa.
 
  If flag == 0 and the process-group test fails, then we determine background
  status simply (but not necessarily reliably) from isatty().

  conbgt() sets the global backgrd = 1 if we appear to be in the background,
  and to 0 if we seem to be in the foreground.  conbgt() is highly prone to
  misbehavior.
*/
VOID
conbgt(flag) int flag; {
    int x = -1,				/* process group or SIGINT test */
        y = 0;				/* isatty() test */
/*
  Check for background operation, even if not running on real tty, so that
  background flag can be set correctly.  If background status is detected,
  then Kermit will not issue its interactive prompt or most messages.
  If your prompt goes away, you can blame (and fix?) this function.
*/

/* Use process-group test if possible. */

#ifdef POSIX				/* We can do it in POSIX */
#define PGROUP_T
#else
#ifdef BSD4				/* and in BSD 4.x. */
#define PGROUP_T
#else
#ifdef HPUXJOBCTL			/* and in most HP-UX's */
#define PGROUP_T
#else
#ifdef TIOCGPGRP			/* and anyplace that has this ioctl. */
#define PGROUP_T
#endif /* TIOCGPGRP */
#endif /* HPUXJOBCTL */
#endif /* BSD4 */
#endif /* POSIX */

#ifdef MIPS				/* Except if it doesn't work... */
#undef PGROUP_T
#endif /* MIPS */

#ifdef PGROUP_T
/*
  Semi-reliable process-group test.  Check whether this process's group is
  the same as the controlling terminal's process group.  This works if the
  getpgrp() call doesn't lie (as it does in the SUNOS System V environment).
*/
    PID_T mypgrp = (PID_T)0;		/* Kermit's process group */
    PID_T ctpgrp = (PID_T)0;		/* The terminal's process group */
#ifndef _POSIX_SOURCE
/*
  The getpgrp() prototype is obtained from system header files for POSIX 
  and Sys V R4 compilations.  Other systems, who knows.  Some complain about
  a duplicate declaration here, others don't, so it's safer to leave it in
  if we don't know for certain.
*/
#ifndef SVR4
#ifndef PS2AIX10
    extern PID_T getpgrp();
#endif /* PS2AIX10 */
#endif /* SVR4 */
#endif /* _POSIX_SOURCE */

/* Get my process group. */

#ifdef SVR3 /* Maybe this should be ATTSV? */
/* This function is not described in SVID R2 */
    mypgrp = getpgrp();
    debug(F101,"ATTSV conbgt process group","",(int) mypgrp);
#else
#ifdef POSIX
    mypgrp = getpgrp();
    debug(F101,"POSIX conbgt process group","",(int) mypgrp);
#else
#ifdef OSFPC
    mypgrp = getpgrp();
    debug(F101,"OSF conbgt process group","",(int) mypgrp);
#else
#ifdef QNX
    mypgrp = getpgrp();
    debug(F101,"QNX conbgt process group","",(int) mypgrp);
#else
#ifdef OSF40
    mypgrp = getpgrp();
    debug(F101,"Digital UNIX 4.0 conbgt process group","",(int) mypgrp);
#else /* BSD, V7, etc */
    mypgrp = getpgrp(0);
    debug(F101,"BSD conbgt process group","",(int) mypgrp);
#endif /* OSF40 */
#endif /* QNX */
#endif /* OSFPC */
#endif /* POSIX */
#endif /* SVR3 */

/* Now get controlling tty's process group */
#ifdef BSD44ORPOSIX
    ctpgrp = tcgetpgrp(1);		/* The POSIX way */
    debug(F101,"POSIX conbgt terminal process group","",(int) ctpgrp);
#else
    ioctl(1, TIOCGPGRP, &ctpgrp);	/* Or the BSD way */
    debug(F101,"non-POSIX conbgt terminal process group","",(int) ctpgrp);
#endif /* BSD44ORPOSIX */

    if ((mypgrp > (PID_T) 0) && (ctpgrp > (PID_T) 0))
      x = (mypgrp == ctpgrp) ? 0 : 1;	/* If they differ, then background. */
    else x = -1;			/* If error, remember. */
    debug(F101,"conbgt process group test","",x);
#endif /* PGROUP_T */

/* Try to see if job control is available */

#ifdef NOJC				/* User override */
    jc = 0;				/* No job control allowed */
    debug(F111,"NOJC","jc",jc);
#else
#ifdef BSD44
    jc = 1;
#else
#ifdef SVR4ORPOSIX			/* POSIX actually tells us */
    debug(F100,"SVR4ORPOSIX jc test...","",0);
#ifdef _SC_JOB_CONTROL
#ifdef __bsdi__
    jc = 1;
#else
#ifdef __386BSD__
    jc = 1;
#else
    jc = sysconf(_SC_JOB_CONTROL);	/* Whatever system says */
    if (jc < 0) {
	debug(F101,"sysconf fails, jcshell","",jcshell);
	jc = (jchdlr == SIG_DFL) ? 1 : 0;
    } else
      debug(F111,"sysconf(_SC_JOB_CONTROL)","jc",jc);
#endif /* __386BSD__ */
#endif /* __bsdi__ */
#else
#ifdef _POSIX_JOB_CONTROL
    jc = 1;				/* By definition */
    debug(F111,"_POSIX_JOB_CONTROL is defined","jc",jc);
#else
    jc = 0;				/* Assume job control not allowed */
    debug(F111,"SVR4ORPOSIX _SC/POSIX_JOB_CONTROL not defined","jc",jc);
#endif /* _POSIX_JOB_CONTROL */
#endif /* _SC_JOB_CONTROL */
#else
#ifdef BSD4
    jc = 1;				/* Job control allowed */
    debug(F111,"BSD job control","jc",jc);
#else
#ifdef SVR3JC
    jc = 1;				/* JC allowed */
    debug(F111,"SVR3 job control","jc",jc);
#else
#ifdef OXOS
    jc = 1;				/* JC allowed */
    debug(F111,"X/OS job control","jc",jc);
#else
#ifdef HPUX9
    jc = 1;				/* JC allowed */
    debug(F111,"HP-UX 9.0 job control","jc",jc);
#else
#ifdef HPUX9
    jc = 1;				/* JC allowed */
    debug(F111,"HP-UX 10.0 job control","jc",jc);
#else
    jc = 0;				/* JC not allowed */
    debug(F111,"job control catch-all","jc",jc);
#endif /* HPUX10 */
#endif /* HPUX9 */
#endif /* OXOS */
#endif /* SVR3JC */
#endif /* BSD4 */
#endif /* SVR4ORPOSIX */
#endif /* BSD44 */
#endif /* NOJC */
    debug(F101,"conbgt jc","",jc);
#ifndef NOJC
    debug(F101,"conbgt jcshell","",jcshell);
/*
  At this point, if jc == 1 but jcshell == 0, it means that the OS supports
  job control, but the shell or other process we are running under does not
  (jcshell is set in sysinit()) and so if we suspend ourselves, nothing good
  will come of it.  So...
*/
    if (jc < 0) jc = 0;
    if (jc > 0 && jcshell == 0) jc = 0;
#endif /* NOJC */

/*
  Another background test.
  Test if SIGINT (terminal interrupt) is set to SIG_IGN (ignore),
  which is done by the shell (sh) if the program is started with '&'.
  Unfortunately, this is NOT done by csh or ksh so watch out!
  Note, it's safe to set SIGINT to SIG_IGN here, because further down
  we always set it to something else.
*/
    if (x < 0 && !flag) {		/* Didn't get good results above... */

	SIGTYP (*osigint)();

	osigint = signal(SIGINT,SIG_IGN);	/* What is SIGINT set to? */
	x = (osigint == SIG_IGN) ? 1 : 0;	/* SIG_IGN? */
	debug(F101,"conbgt osigint","",osigint);
	debug(F101,"conbgt signal test","",x);
    }

/* Also check to see if we're running with redirected stdio. */
/* This is not really background operation, but we want to act as though */
/* it were. */

    y = (isatty(0) && isatty(1)) ? 1 : 0;
    debug(F101,"conbgt isatty test","",y);

#ifdef BSD29
/* The process group and/or signal test doesn't work under these... */
    backgrd = !y;
#else
#ifdef sxaE50
    backgrd = !y;
#else
#ifdef MINIX
    backgrd = !y;
#else
    if (x > -1)
      backgrd = (x || !y) ? 1 : 0;
    else backgrd = !y;
#endif /* BSD29 */
#endif /* sxaE50 */
#endif /* MINIX */
    debug(F101,"conbgt backgrd","",backgrd);
}

/*  C O N I N T  --  Console Interrupt setter  */

/*
  First arg is pointer to function to handle SIGTERM & SIGINT (like Ctrl-C).
  Second arg is pointer to function to handle SIGTSTP (suspend).
*/

VOID					/* Set terminal interrupt traps. */
#ifdef CK_ANSIC
#ifdef apollo
conint(f,s) SIGTYP (*f)(), (*s)();
#else
conint(SIGTYP (*f)(int), SIGTYP (*s)(int))
#endif /* apollo */
#else
conint(f,s) SIGTYP (*f)(), (*s)();
#endif /* CK_ANSIC */
/* conint */ {

    conbgt(0);				/* Do background test. */

/* Set the desired handlers for hangup and software termination. */

    signal(SIGTERM,f);                  /* Software termination */

#ifdef COMMENT
/*
  Prior to edit 184, we used to trap SIGHUP here.  That is clearly wrong;
  on some systems, it would leave the user's process on the terminal after
  the phone hung up.  But the trap was here for a reason: most likely some
  UNIX systems (init, getty, or login) fail to properly restore the terminal
  modes after regaining control of a hung-up-upon login terminal.  Therefore
  removing this trap is likely to cause problems too.  A more sensible
  approach would be to use a special handler for HANGUP, which would restore
  the terminal modes and then exit().  But that could leave zombie processes
  around (like the lower CONNECT fork, or any fork started by zxcmd()), but
  there is probably no clean, portable, reliable way for Kermit to kill all
  its forks.  So we just exit() and hope that UNIX fixes the terminal modes
  before the next person tries to log in.
*/
    signal(SIGHUP,f);                   /* Hangup */
#endif /* COMMENT */

/* Now handle keyboard stop, quit, and interrupt signals. */
/* Check if invoked in background -- if so signals set to be ignored. */
/* However, if running under a job control shell, don't ignore them. */
/* We won't be getting any, as we aren't in the terminal's process group. */

    debug(F101,"conint backgrd","",backgrd);
    debug(F101,"conint jc","",jc);

    if (backgrd && !jc) {		/* In background, ignore signals */
	debug(F101,"conint background ignoring signals, jc","",jc);
#ifdef SIGTSTP
        signal(SIGTSTP,SIG_IGN);        /* Keyboard stop */
#endif /* SIGTSTP */
        signal(SIGQUIT,SIG_IGN);        /* Keyboard quit */
        signal(SIGINT,SIG_IGN);         /* Keyboard interrupt */
    } else {				/* Else in foreground or suspended */
	debug(F101,"conint foreground catching signals, jc","",jc);
        signal(SIGINT,f);               /* Catch terminal interrupt */

#ifdef SIGTSTP				/* Keyboard stop (suspend) */
	debug(F101,"conint SIGSTSTP","",s);
	if (s == NULL) s = SIG_DFL;
#ifdef NOJC				/* No job control allowed. */
	signal(SIGTSTP,SIG_IGN);
#else					/* Job control allowed */
	if (jc)				/* if available. */
	  signal(SIGTSTP,s);
	else
	  signal(SIGTSTP,SIG_IGN);
#endif /* NOJC */
#endif /* SIGTSTP */

#ifndef OXOS
#ifdef SVORPOSIX
#ifndef FIONREAD			/* Watch out, we don't know this... */
#ifndef SELECT
#ifndef CK_POLL
#ifndef RDCHK
        signal(SIGQUIT,esctrp);         /* Quit signal, Sys III/V. */
#endif /* RDCHK */
#endif /* CK_POLL */
#endif /* SELECT */
#endif /* FIONREAD */
        if (conesc) conesc = 0;         /* Clear out pending escapes */
#else
#ifdef V7
        signal(SIGQUIT,esctrp);         /* V7 like Sys III/V */
        if (conesc) conesc = 0;
#else
#ifdef aegis
        signal(SIGQUIT,f);              /* Apollo, catch it like others. */
#else
        signal(SIGQUIT,SIG_IGN);        /* Others, ignore like 4D & earlier. */
#endif /* aegis */
#endif /* V7 */
#endif /* SVORPOSIX */
#endif /* OXOS */
    }
}


/*  C O N N O I  --  Reset console terminal interrupts */

SIGTYP					/* Dummy function to ignore signals */
#ifdef CK_ANSIC
sig_ign(int foo)
#else
sig_ign(foo) int foo;
#endif /* CK_ANSIC */
/* sig_IGN */ {				/* Just like the real one, but has  */
}					/* different address. */

VOID
connoi() {                              /* Console-no-interrupts */

    debug(F100,"connoi","",0);
#ifdef SIGTSTP
    signal(SIGTSTP,SIG_DFL);
#endif /* SIGTSTP */
    /* Note the locally defined replacement for SIG_IGN that is used here */
    /* for the SIGINT setting.  This is done so that the Sys V background */
    /* test -- (signal(SIGINT,SIG_IGN) == SIG_IGN) -- can work.  If we use */
    /* the real SIG_IGN here, then conint will always decide that this */ 
    /* program is running in the background! */

    signal(SIGINT,sig_ign);		/* <--- note! */

    signal(SIGHUP,SIG_DFL);
    signal(SIGQUIT,SIG_IGN);
    signal(SIGTERM,SIG_IGN);
}

/*  I N I T R A W Q  --  Set up to read /dev/kmem for character count.  */

#ifdef  V7
/*
 Used in Version 7 to simulate Berkeley's FIONREAD ioctl call.  This
 eliminates blocking on a read, because we can read /dev/kmem to get the
 number of characters available for raw input.  If your system can't
 or you won't let the world read /dev/kmem then you must figure out a
 different way to do the counting of characters available, or else replace
 this by a dummy function that always returns 0.
*/
/*
 * Call this routine as: initrawq(tty)
 * where tty is the file descriptor of a terminal.  It will return
 * (as a char *) the kernel-mode memory address of the rawq character
 * count, which may then be read.  It has the side-effect of flushing
 * input on the terminal.
 */
/*
 * John Mackin, Physiology Dept., University of Sydney (Australia)
 * ...!decvax!mulga!physiol.su.oz!john
 *
 * Permission is hereby granted to do anything with this code, as
 * long as this comment is retained unmodified and no commercial
 * advantage is gained.
 */
#ifndef MINIX
#ifndef COHERENT
#include <a.out.h>
#include <sys/proc.h>
#endif /* COHERENT */
#endif /* MINIX */

#ifdef COHERENT
#include <l.out.h>
#include <sys/proc.h>
#endif /* COHERENT */

char *
initrawq(tty) int tty; {
#ifdef MINIX
    return(0);
#else
#ifdef UTS24
    return(0);
#else
#ifdef BSD29
    return(0);
#else
    long lseek();
    static struct nlist nl[] = {
        {PROCNAME},
        {NPROCNAME},
        {""}
    };
    static struct proc *pp;
    char *qaddr, *p, c;
    int m;
    PID_T pid, me;
    NPTYPE xproc;                       /* Its type is defined in makefile. */
    int catch();

    me = getpid();
    if ((m = open("/dev/kmem", 0)) < 0) err("kmem");
    nlist(BOOTNAME, nl);
    if (nl[0].n_type == 0) err("proc array");

    if (nl[1].n_type == 0) err("nproc");

    lseek(m, (long)(nl[1].n_value), 0);
    read (m, &xproc, sizeof(xproc));
    saval = signal(SIGALRM, catch);
    if ((pid = fork()) == 0) {
        while(1)
            read(tty, &c, 1);
    }
    alarm(2);

    if(setjmp(jjbuf) == 0) {
        while(1)
	  read(tty, &c, 1);
    }
    signal(SIGALRM, SIG_DFL);

#ifdef DIRECT
    pp = (struct proc *) nl[0].n_value;
#else
    if (lseek(m, (long)(nl[0].n_value), 0) < 0L) err("seek");
    if (read(m, &pp, sizeof(pp)) != sizeof(pp))  err("no read of proc ptr");
#endif
    lseek(m, (long)(nl[1].n_value), 0);
    read(m, &xproc, sizeof(xproc));

    if (lseek(m, (long)pp, 0) < 0L) err("Can't seek to proc");
    if ((p = malloc(xproc * sizeof(struct proc))) == NULL) err("malloc");
    if (read(m,p,xproc * sizeof(struct proc)) != xproc*sizeof(struct proc))
        err("read proc table");
    for (pp = (struct proc *)p; xproc > 0; --xproc, ++pp) {
        if (pp -> p_pid == (short) pid) goto iout;
    }
    err("no such proc");

iout:
    close(m);
    qaddr = (char *)(pp -> p_wchan);
    free (p);
    kill(pid, SIGKILL);
    wait((WAIT_T *)0);
    return (qaddr);
#endif
#endif
#endif
}

/*  More V7-support functions...  */

static VOID
err(s) char *s; {
    char buf[200];

    sprintf(buf, "fatal error in initrawq: %s", s);
    perror(buf);
    doexit(1,-1);
}

static VOID
catch(foo) int foo; {
    longjmp(jjbuf, -1);
}


/*  G E N B R K  --  Simulate a modem break.  */

#ifdef MINIX
#define BSPEED B110
#else
#define BSPEED B150
#endif /* MINIX */

VOID
genbrk(fn,msec) int fn, msec; {
    struct sgttyb ttbuf;
    int ret, sospeed, x, y;

    ret = ioctl(fn, TIOCGETP, &ttbuf);
    sospeed = ttbuf.sg_ospeed;
    ttbuf.sg_ospeed = BSPEED;
    ret = ioctl(fn, TIOCSETP, &ttbuf);
    y = (int)strlen(brnuls);
    x = ( BSPEED * 100 ) / msec;
    if (x > y) x = y;
    ret = write(fn, brnuls, (( BSPEED * 100 ) / msec ));
    ttbuf.sg_ospeed = sospeed;
    ret = ioctl(fn, TIOCSETP, &ttbuf);
    ret = write(fn, "@", 1);
    return;
}
#endif /* V7 */

/*
  I N C H K  --  Check if chars waiting to be read on given file descriptor.
  Returns number of characters waiting, 0 or greater.

  This routine is a merger of ttchk() and conchk().
  Conchk calls it with 0; ttchk() calls it with ttyfd.
*/
static int
in_chk(fd) int fd; {
    int x, n = 0;			/* Return value */

    if (fd < 0) return(0);

#ifdef FIONREAD
    x = ioctl(fd, FIONREAD, &n);	/* BSD and hopefully others */
    debug(F101,"in_chk FIONREAD return code","",x);
    debug(F101,"in_chk FIONREAD count","",n);
#else /* FIONREAD not defined */
#ifdef  V7
#ifdef MINIX
    n = 0;
#else
    lseek(kmem[TTY], (long) qaddr[TTY], 0); /* 7th Edition Unix */
    x = read(kmem[TTY], &n, sizeof(int));
    if (x != sizeof(int)) n = 0;
#endif /* MINIX */
#else /* Not V7 */
#ifdef PROVX1
    x = ioctl(fd, TIOCQCNT, &ttbuf);	/* DEC Pro/3xx Venix V.1 */
    n = ttbuf.sg_ispeed & 0377;
    if (x < 0) n = 0;
#else
#ifdef RDCHK				/* This mostly SCO-specific */
    n = rdchk(fd);
    debug(F101,"in_chk rdchk","",n);
#else /* No RDCHK */
#ifdef SELECT
#ifdef Plan9
    /* only allow select on the console ... don't ask */
    if (fd == 0 && ttyfd != 0)
#endif /* Plan9 */
      {
	fd_set rfds;			/* Read file descriptors */
#ifdef BELLV10
	FD_ZERO(rfds);			/* Initialize them */
	FD_SET(fd,rfds);		/* We want to look at this fd */
#else
	FD_ZERO(&rfds);			/* Initialize them */
	FD_SET(fd,&rfds);		/* We want to look at this fd */
	tv.tv_sec = tv.tv_usec = 0L;	/* A 0-valued timeval structure */
#endif /* BELLV10 */
#ifdef Plan9
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
#else
#ifdef BELLV10
	n = select( 128, rfds, (fd_set *)0, (fd_set *)0, 0 );
#else
#ifdef BSD44
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
#else
#ifdef BSD43
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
#else
#ifdef SOLARIS
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
#else
#ifdef QNX
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
#else
#ifdef COHERENT
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
#else
#ifdef SVR4
	n = select( FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, &tv );
#else
	n = select( FD_SETSIZE, &rfds, (int *)0, (int *)0, &tv );
#endif /* SVR4 */
#endif /* COHERENT */
#endif /* QNX */
#endif /* SOLARIS */
#endif /* BSD43 */
#endif /* BSD44 */
#endif /* BELLV10 */
#endif /* Plan9 */
	debug(F101,"in_chk select n","",n);
    }
#else  /* Not SELECT */
#ifdef CK_POLL
    {
      struct pollfd pfd;

      pfd.fd = fd;
      pfd.events = POLLIN;
      pfd.revents = 0;
      n = poll(&pfd, 1, 0);
      debug(F101,"in_chk poll","",n);
      if ((n > 0) && (pfd.revents & POLLIN))
	n = 1;
    }
#endif /* CK_POLL */
#endif /* SELECT */
#endif /* RDCHK */
#endif /* PROVX1 */
#endif /* V7 */
#endif /* FIONREAD */

/* From here down, treat console and communication device differently... */

    if (fd == 0 && ttyfd != 0) {	/* Console */

#ifdef SVORPOSIX
#ifndef FIONREAD
#ifndef SELECT
#ifndef CK_POLL
#ifndef RDCHK
/*
  This is the hideous hack used in System V and POSIX systems that don't
  support FIONREAD, rdchk(), select(), poll(), etc, in which the user's
  CONNECT-mode escape character is attached to SIGQUIT.  Used, obviously,
  only on the console.
*/
	if (conesc) {			/* Escape character typed == SIGQUIT */
	    debug(F100,"in_chk conesc","",conesc);
	    conesc = 0;
	    signal(SIGQUIT,esctrp);	/* Restore signal */
	    n += 1;
	}
#endif /* RDCHK */
#endif /* CK_POLL */
#endif /* SELECT */
#endif /* FIONREAD */
#endif /* SVORPOSIX */

	return(n);			/* Done with console */
    }

/* Communications device, might also have stuff buffered. */

#ifdef MYREAD
/*
  For myread() users, add the contents of myread()'s private buffer.
  Sometimes, this is all there is to construct a result of in_chk(ttyfd) on.
*/
    debug(F101,"in_chk my_count","",my_count);
    if (my_count > 0)
      n += my_count;
#endif /* MYREAD */

    return(n < 0 ? 0 : n);
}


/*  T T C H K  --  Tell how many characters are waiting in tty input buffer  */

/*  Some callers of this want to know whether there is something to read
 *  either in the system buffers or in our private buffers (and mostly don't
 *  care how many characters, just whether it's more than zero or not), while
 *  some others would be better off with the number of characters in our
 *  private buffers only.
 *
 *  Some systems can say how many characters there are in the system buffers.
 *  Others can not. For those that can't, the number in the private buffers
 *  will have to do (or should we put the tty into O_NDELAY-mode and try to
 *  read one character?). If the system can tell whether there is zero or
 *  more than zero characters, we can return 1 in the latter case even if the
 *  private buffer is empty. (That is good for sliding windows.)
 */
int
ttchk() {
    return(in_chk(ttyfd));
}

/*  T T X I N  --  Get n characters from tty input buffer  */

/*  Returns number of characters actually gotten, or -1 on failure  */

/*  Intended for use only when it is known that n characters are actually */
/*  Available in the input buffer.  */

int
ttxin(n,buf) int n; CHAR *buf; {
    register int x, c;

    debug(F101,"ttxin n","",n);
    if (n < 1) return(0);
#ifdef COMMENT
    ttpmsk = (ttprty) ? 0177 : 0377;	/* Parity stripping mask. */
    debug(F101,"ttxin ttpmsk","",ttpmsk);
#endif /* COMMENT */

#ifdef SUNX25
    if (netconn && (ttnet == NET_SX25))	/* X.25 connection */
      return(x25xin(n,buf));
#endif /* SUNX25 */

#ifdef MYREAD
    debug(F101,"ttxin MYREAD","",0);
    c = -2;
    for( x = 0; (x > -1) && (x < n) && (c = myread()) >= 0; )
      buf[x++] = c & ttpmsk;
    if (c < 0) {
	debug(F101,"ttxin myread returns","",c);
	if (c == -3) x = -1;
    }
#else
    x = read(ttyfd,buf,n);
    for (c = 0; c < n; c++) buf[c] &= ttpmsk;
    debug(F101," x","",x);
#endif /* MYREAD */
    if (x > 0) buf[x] = '\0';
    if (x < 0) x = -1;
    return(x);
}

/*  T T O L  --  Write string s, length n, to communication device.  */
/*
  Returns:
   >= 0 on success, number of characters actually written.
   -1 on failure.
*/
#define TTOLMAXT 5
int
ttol(s,n) int n; CHAR *s; {
    int x, len, tries;
    if (ttyfd < 0)			/* Not open? */
      return(-3);

    debug(F101,"ttol n","",n);
    debug(F110,"ttol s",s,0);

    tries = TTOLMAXT;			/* Allow up to this many tries */
    len = n;				/* Remember original length */
    while (n > 0 && tries-- > 0) {	/* Be persistent */
	debug(F101,"ttol try","",TTOLMAXT - tries);
	x = write(ttyfd,s,n);		/* Write string to device */
	if (x == n) {			/* Worked? */
	    debug(F101,"ttol ok","",x);	/* OK */
	    return(len);		/* Done */
	} else if (x < 0) {		/* No, got error? */
	    debug(F101,"ttol failed","",errno);
#ifdef TCPSOCKET
	    if (netconn && ttnet == NET_TCPB) {
		if (errno) perror("TCP/IP"); /* Say why */
		ttclos(0);		/* Close the connection. */
	    }
#endif /* TCPSOCKET */
	    return(-1);
	} else {			/* No error, so partial success */
	    debug(F101,"ttol partial","",x);
	    s += x;			/* Point to part not written yet */
	    n -= x;			/* Adjust length */
	    if (x > 0) msleep(100);	/* Wait 100 msec */
	}				/* Go back and try again */
    }
    return(n < 1 ? len : -1);		/* Return the results */
}

/*  T T O C  --  Output a character to the communication line  */

/*
 This function should only be used for interactive, character-mode operations,
 like terminal connection, script execution, dialer i/o, where the overhead
 of the signals and alarms does not create a bottleneck.
*/
int
#ifdef CK_ANSIC
ttoc(char c)
#else
ttoc(c) char c;
#endif /* CK_ANSIC */
/* ttoc */ {
#define TTOC_TMO 15			/* Timeout in case we get stuck */
    int xx;
    c &= 0xff;
    /* debug(F101,"ttoc","",(CHAR) c); */
    if (ttyfd < 0)			/* Check for not open. */
      return(-1);
    saval = signal(SIGALRM,timerh);	/* Enable timer interrupt */
    xx = alarm(TTOC_TMO);		/* for this many seconds. */
    if (xx < 0) xx = 0;			/* Save old alarm value. */
    /* debug(F101,"ttoc alarm","",xx); */
    if (
#ifdef CK_POSIX_SIG
	sigsetjmp(sjbuf,1)
#else
	setjmp(sjbuf)
#endif /* CK_POSIX_SIG */
	) {		/* Timer went off? */
	ttimoff();			/* Yes, cancel this alarm. */
	if (xx - TTOC_TMO > 0) alarm(xx - TTOC_TMO); /* Restore previous one */
        /* debug(F100,"ttoc timeout","",0); */
#ifdef NETCONN
	if (!netconn) {
#endif /* NETCONN */
	    debug(F101,"ttoc timeout","",c);
	    if (ttflow == FLO_XONX) {
		debug(F101,"ttoc flow","",ttflow); /* Maybe we're xoff'd */
#ifndef Plan9
#ifdef POSIX
		/* POSIX way to unstick. */
		debug(F100,"ttoc tcflow","",tcflow(ttyfd,TCOON));
#else
#ifdef BSD4				/* Berkeley way to do it. */
#ifdef TIOCSTART
/* .... Used to be "ioctl(ttyfd, TIOCSTART, 0);".  Who knows? */
		{
		  int x = 0;
		  debug(F101,"ttoc TIOCSTART","",ioctl(ttyfd, TIOCSTART, &x));
		}
#endif /* TIOCSTART */
#endif /* BSD4 */
					/* Is there a Sys V way to do this? */
#endif /* POSIX */
#endif /* Plan9 */
	    }
#ifdef NETCONN
        }
#endif /* NETCONN */
	return(-1);			/* Return failure code. */
    } else {
	if (write(ttyfd,&c,1) < 1) {	/* Try to write the character. */
	    ttimoff();			/* Failed, turn off the alarm. */
	    alarm(xx);			/* Restore previous alarm. */
	    debug(F101,"ttoc error","",errno); /* Log the error, */
	    return(-1);			/* and return the error code. */
	}
    }
    ttimoff();				/* Success, turn off the alarm. */
    alarm(xx);				/* Restore previous alarm. */
    return(0);				/* Return good code. */
}

/*  T T I N L  --  Read a record (up to break character) from comm line.  */
/*
  Reads up to "max" characters from the communication line, terminating on:

    (a) the packet length field if the "turn" argument is zero, or
    (b) on the packet-end character (eol) if the "turn" argument is nonzero
    (c) a certain number of Ctrl-C's in a row

  and returns the number of characters read upon success, or if "max" was
  exceeded or the timeout interval expired before (a) or (b), returns -1.

  The characters that were input are copied into "dest" with their parity bits
  stripped if parity was selected.  Returns the number of characters read.
  Characters after the eol are available upon the next call to this function.

  The idea is to minimize the number of system calls per packet, and also to
  minimize timeouts.  This function is the inner loop of the program and must
  be as efficient as possible.  The current strategy is to use myread().

  WARNING: this function calls parchk(), which is defined in another module.
  Normally, ckutio.c does not depend on code from any other module, but there
  is an exception in this case because all the other ck?tio.c modules also
  need to call parchk(), so it's better to have it defined in a common place.

  Since this function has grown to have its fingers so deeply into the 
  protocol, it is slated for removal: rpack() should take care of everything.
*/
#ifdef CTRLC
#undef CTRLC
#endif /* CTRLC */
#define CTRLC '\03'
/*
  We have four different declarations here because:
  (a) to allow Kermit to be built without the automatic parity sensing feature
  (b) one of each type for ANSI C, one for non-ANSI.
*/
int
#ifdef PARSENSE
#ifdef CK_ANSIC
ttinl(CHAR *dest, int max,int timo, CHAR eol, CHAR start, int turn)
#else
ttinl(dest,max,timo,eol,start,turn) int max,timo,turn; CHAR *dest, eol, start;
#endif /* CK_ANSIC */
#else /* not PARSENSE */
#ifdef CK_ANSIC
ttinl(CHAR *dest, int max,int timo, CHAR eol)
#else
ttinl(dest,max,timo,eol) int max,timo; CHAR *dest, eol;
#endif /* __SDTC__ */
#endif /* PARSENSE */
/* ttinl */ {

#ifndef MYREAD
    CHAR ch;
#endif /* MYREAD */
#ifdef PARSENSE
    int pktlen = -1;
    int lplen = 0;
    int havelen = 0;
#endif /* PARSENSE */
    int cc = 0;				/* Character count */

    if (ttyfd < 0) return(-3);          /* Not open. */

    debug(F101,"ttinl max","",max);
    debug(F101,"ttinl timo","",timo);

    *dest = '\0';                       /* Clear destination buffer */
    if (timo < 0) timo = 0;		/* Safety */
    if (timo) {				/* Don't time out if timo == 0 */
	int xx;
	saval = signal(SIGALRM,timerh);	/* Enable timer interrupt */
	xx = alarm(timo);		/* Set it. */
	debug(F101,"ttinl alarm","",xx);
    }
    if (
#ifdef CK_POSIX_SIG
	sigsetjmp(sjbuf,1)
#else
	setjmp(sjbuf)
#endif /* CK_POSIX_SIG */
	) {                /* Timer went off? */
	debug(F100,"ttinl timout","",0); /* Get here on timeout. */
	/* debug(F110," with",(char *) dest,0); */
	ttimoff();			/* Turn off timer */
	return(-1);			/* and return error code. */
    } else {
	register int i, /* m, */ n;	/* local variables */
	int ccn = 0;
#ifdef PARSENSE
	int flag = 0;
	debug(F000,"ttinl start","",start);
#endif /* PARSENSE */

	ttpmsk = (ttprty) ? 0177 : 0377; /* Set parity stripping mask. */
#ifdef COMMENT
	m == ttpmsk;
#endif /* COMMENT */

#ifdef COMMENT
/*
  No longer needed.
*/
#ifdef SUNX25
        if (netconn && (ttnet == NET_SX25))
#ifdef PARSENSE
	  return(x25inl(dest,max,timo,eol,start));
#else
	  return(x25inl(dest,max,timo,eol));
#endif /* PARSENSE */
#endif /* SUNX25 */
#endif /* COMMENT */

/* Now read into destination, stripping parity and looking for the */
/* the packet terminator, and also for several Ctrl-C's typed in a row. */

	i = 0;				/* Destination index */
	debug(F101,"ttinl eol","",eol);

#ifdef MYREAD
	while (i < max-1) {
	    /* debug(F101,"ttinl i","",i); */
	    if ((n = myread()) < 0) {	/* Timeout or i/o error? */
#ifdef DEBUG
		if (deblog) {
		    debug(F101,"ttinl myread failure, n","",n);
		    debug(F101,"ttinl myread errno","",errno);
		}
#endif /* DEBUG */
		/* Don't let EINTR break packets. */
		if (n == -3 && errno == EINTR && i > 0) {
		    debug(F101,"ttinl myread i","",i);
		    continue;
		} else if (n == -2 && netconn /* && timo == 0 */ ) {
		    /* Here we try to catch broken network connections */
		    /* even when ioctl() and read() do not catch them */
		    debug(F100,"ttinl untimed network myread fails","",0);
		    n = -3;		/* Upgrade the error */
		    ttclos(0);
		}
		break;			/* Break out of while loop */
	    }
#else
	while ((i < max-1)  &&  (n = read(ttyfd, &ch, 1)) > 0) {
	    n = ch;
#endif /* MYREAD */

	    /* debug(F101,"ttinl char","", (n & ttpmsk)); */

#ifdef PARSENSE
/*
  Figure out what the length is supposed to be in case the packet
  has no terminator (as with Honeywell GCOS-8 Kermit).
*/
#ifndef xunchar
#define xunchar(ch) (((ch) - 32 ) & 0xFF )	/* Character to number */
#endif /* xunchar */
	    if ((flag == 0) && ((n & 0x7f) == start)) flag = 1;
	    if (flag) dest[i++] = n & ttpmsk;
#ifdef COMMENT
	    /* if (!flag && cc++ > (max / 2)) return(-1); */
#endif /* COMMENT */
/*
  If we have not been instructed to wait for a turnaround character, we
  can go by the packet length field.  If turn != 0, we must wait for the
  end of line (eol) character before returning.
*/
	    if (i == 2) {
		pktlen = xunchar(dest[1] & 0x7f);
		havelen = (pktlen > 1);
		debug(F101,"ttinl length","",pktlen);
	    } else if (i == 5 && pktlen == 0) {
		lplen = xunchar(dest[4] & 0x7f);
	    } else if (i == 6 && pktlen == 0) {
		pktlen = lplen * 95 + xunchar(dest[5] & 0x7f) + 5;
		havelen = 1;
		debug(F101,"ttinl length","",pktlen);
	    }
#else
	    dest[i++] = n & ttpmsk;
#endif /* PARSENSE */
/*
  Use parity mask, rather than always stripping parity, to check for
  cancellation.  Otherwise, runs like \x03\x83\x03 in packet could cancel
  the transfer when parity is NONE.
*/
	    /* Check cancellation */
	    if (!xlocal && xfrcan && ((n & ttpmsk) == xfrchr)) {
		if (++ccn >= xfrnum) {	/* If xfrnum in a row, bail out. */
		    if (timo) {		/* Clear timer. */
			ttimoff();
		    }
		    fprintf(stderr,"^C...\r\n"); /* Echo Ctrl-C */
		    return(-2);
		}
	    } else ccn = 0;		/* No cancellation, reset counter, */

#ifdef PARSENSE
	    if (flag == 0) {
		debug(F101,"ttinl skipping","",n);
		continue;
	    }
#endif /* PARSENSE */

    /* Check for end of packet */

	    if (
#ifdef PARSENSE
/*
  Purely length-driven if SET HANDSHAKE NONE (i.e. turn == 0).
  This allows packet terminators and handshake characters to appear
  literally inside a packet data field.
*/
		(havelen && (i > pktlen+1) &&
		 (!turn || (turn && (n & 0x7f) == eol)))
#ifdef COMMENT
		(!turn && havelen && (i > pktlen+1)) ||
		(turn && havelen && (i > pktlen+1) && (n & 0x7f) == eol)
#endif /* COMMENT */
#else /* !PARSENSE */
/*
  Built without PARSENSE, so just look for packet terminator.
*/
		((n & 0x7f) == eol)
#endif /* PARSENSE */
		) {
#ifndef PARSENSE
		debug(F101,"ttinl got eol","",eol);
		dest[i] = '\0';		/* Yes, terminate the string, */
		/* debug(F101,"ttinl i","",i); */
#else
		if ((n & 0x7f) != eol) {
		    debug(F101,"ttinl EOP length","",pktlen);
		    debug(F101,"ttinl i","",i);
		} else debug(F101,"ttinl got eol","",eol);
		dest[i] = '\0';		/* Terminate the string, */
		/* Parity checked yet? */
	        if (ttpflg++ == 0 && ttprty == 0) {
		    if ((ttprty = parchk(dest,start,i)) > 0) { /* No, check. */
			int j;
			debug(F101,"ttinl senses parity","",ttprty);
			debug(F110,"ttinl packet before",dest,0);
			ttpmsk = 0x7f;
			for (j = 0; j < i; j++)
			  dest[j] &= 0x7f;	/* Strip parity from packet */
			debug(F110,"ttinl packet after ",dest,0);
		    } else ttprty = 0;	/* restore if parchk error */
		}
#endif /* PARSENSE */
		if (timo) {			/* Turn off timer. */
		    ttimoff();
		}
		debug(F111,"ttinl got", dest,i);
		return(i);
	    }
	}				/* end of while() */
	ttimoff();
	return(n);
    }
}

/*  T T I N C --  Read a character from the communication line  */
/*
 On success, returns the character that was read, >= 0.
 On failure, returns -1 or other negative myread error code,
   or -2 if connection is broken or ttyfd < 0.
*/
int
ttinc(timo) int timo; {

    int n = 0;
#ifndef MYREAD
    CHAR ch = 0;
#endif /* MYREAD */

    if (ttyfd < 0) return(-2);          /* Not open. */
    if ((timo <= 0)			/* Untimed. */
#ifdef MYREAD
	|| (my_count > 0)		/* Buffered char already waiting. */
#endif /* MYREAD */
	) {
#ifdef MYREAD
        /* Comm line failure returns -1 thru myread, so no &= 0377 */
	n = myread();			/* Wait for a character... */
	/* debug(F000,"ttinc MYREAD n","",n); */
#ifdef TCPSOCKET
	if (netconn && (ttnproto == NP_TELNET) && (n > -1))
	  return((unsigned)(n & 0xff));
	else
#endif /* TCPSOCKET */
	return(n < 0 ? n : (unsigned)(n & ttpmsk));
#else
        while ((n = read(ttyfd,&ch,1)) == 0) /* Wait for a character. */
        /* Shouldn't have to loop in ver 5A. */
#ifdef NETCONN	  
	  if (netconn) {		/* Special handling for net */
	      netclos();		/* If read() returns 0 it means */
	      netconn = 0;		/* the connection has dropped. */
	      errno = ENOTCONN;
	      return(-2);
	  }
#endif /* NETCONN */
	  ;
     /* debug(F000,"ttinc","",ch); */
#ifdef TCPSOCKET
	if (netconn && (ttnproto == NP_TELNET) && (n > -1))
	  return((unsigned)(n & 0xff));
	else
#endif /* TCPSOCKET */
        return( (n < 1) ? -3 : (unsigned)(ch & ttpmsk) );
#endif /* MYREAD */
    } else {				/* Timed read */
	int xx;
	saval = signal(SIGALRM,timerh);	/* Set up handler, save old one. */
	xx = alarm(timo);		/* Set alarm, save old one. */
	if (
#ifdef CK_POSIX_SIG
	    sigsetjmp(sjbuf,1)
#else
	    setjmp(sjbuf)
#endif /* CK_POSIX_SIG */
	    ) {		/* Timer expired */
	    n = -1;			/* set flag */
	} else {
#ifdef MYREAD
	    n = myread();		/* If managing own buffer... */
	    /* debug(F101,"ttinc myread","",n); */
#else
	    n = read(ttyfd,&ch,1);	/* Otherwise call the system. */
	    /* debug(F101,"ttinc read","",n); */
	    if (n > 0)
	      n = ch & 255;
	    else
	      n = (n < 0) ? -3 : -2;	/* Special return codes. */
#endif /* MYREAD */
	}
	ttimoff();			/* Turn off the timer */
/* #ifdef COMMENT */
	if (n == -1) xx -= timo;	/* and restore any previous alarm */
	if (xx < 0) xx = 0;		/* adjusted by timeout interval */
	alarm(xx);			/* if timer expired. */
/* #endif */ /* COMMENT */
#ifdef NETCONN
	if (netconn) {
	    if (n == -2) {		/* read() returns 0 */
		netclos();		/* on network read failure */
		netconn = 0;
		errno = ENOTCONN;
	    }
	}
#endif	/* NETCONN */
#ifdef TCPSOCKET
	if (netconn && (ttnproto == NP_TELNET) && (n > -1))
	  return((unsigned)(n & 0xff));
	else
#endif /* TCPSOCKET */
	  /* Return masked char or neg. */
	  return( (n < 0) ? n : (unsigned)(n & ttpmsk) );
    }
}

/*  S N D B R K  --  Send a BREAK signal of the given duration  */

#ifndef NOLOCAL
static int
#ifdef CK_ANSIC
sndbrk(int msec) {			/* Argument is milliseconds */
#else
sndbrk(msec) int msec; {
#endif /* CK_ANSIC */
#ifndef POSIX
    int x, n;
#endif /* POSIX */

#ifdef OXOS
#define BSDBREAK
#endif /* OXOS */

#ifdef ANYBSD
#define BSDBREAK
#endif /* ANYBSD */

#ifdef BSD44
#define BSDBREAK
#endif /* BSD44 */

#ifdef COHERENT
#define BSDBREAK
#endif /* COHERENT */

#ifdef BELLV10
#ifdef BSDBREAK
#undef BSDBREAK
#endif /* BSDBREAK */
#endif /* BELLV10 */

#ifdef PROVX1
    char spd;
#endif /* PROVX1 */

    debug(F101,"ttsndb ttyfd","",ttyfd);
    if (ttyfd < 0) return(-1);          /* Not open. */

#ifdef Plan9
    return p9sndbrk(msec);
#else
#ifdef NETCONN
    if (netconn) 			/* Send network BREAK */
      return(netbreak());
#endif /* NETCONN */

    if (msec < 1 || msec > 5000) return(-1); /* Bad argument */

#ifdef POSIX				/* Easy in POSIX */
    debug(F101,"sndbrk POSIX","",msec);
    return(tcsendbreak(ttyfd,msec / 375));
#else
#ifdef PROVX1
    gtty(ttyfd,&ttbuf);                 /* Get current tty flags */
    spd = ttbuf.sg_ospeed;              /* Save speed */
    ttbuf.sg_ospeed = B50;              /* Change to 50 baud */
    stty(ttyfd,&ttbuf);                 /*  ... */
    n = (int)strlen(brnuls);		/* Send the right number of nulls */
    x = msec / 91;
    if (x > n) x = n;
    write(ttyfd,brnuls,n);
    ttbuf.sg_ospeed = spd;              /* Restore speed */
    stty(ttyfd,&ttbuf);                 /*  ... */
    return(0);
#else
#ifdef aegis
    sio_$control((short)ttyfd, sio_$send_break, msec, st);
    return(0);
#else
#ifdef BSDBREAK
    n = FWRITE;                         /* Flush output queue. */
/* Watch out for int vs long problems in &n arg! */
    debug(F101,"sndbrk BSDBREAK","",msec);
    ioctl(ttyfd,TIOCFLUSH,&n);          /* Ignore any errors.. */
    if (ioctl(ttyfd,TIOCSBRK,(char *)0) < 0) {  /* Turn on BREAK */
        perror("Can't send BREAK");
        return(-1);
    }
    x = msleep(msec);                    /* Sleep for so many milliseconds */
    if (ioctl(ttyfd,TIOCCBRK,(char *)0) < 0) {  /* Turn off BREAK */
        perror("BREAK stuck!!!");
        doexit(BAD_EXIT,-1);		/* Get out, closing the line. */
                                        /*   with bad exit status */
    }
    return(x);
#else
#ifdef ATTSV
/*
  No way to send a long BREAK in Sys V, so send a bunch of regular ones.
  (Actually, Sys V R4 is *supposed* to have the POSIX tcsendbreak() function,
  but there's no way for this code to know for sure.)
*/
    debug(F101,"sndbrk ATTSV","",msec);
    x = msec / 275;
    for (n = 0; n < x; n++) {
	/* Reportedly the cast breaks this function on some systems */
	/* But then why was it here in the first place? */
	if (ioctl(ttyfd,TCSBRK, /* (char *) */ 0) < 0) {
	    perror("Can't send BREAK");
	    return(-1);
	}
    }
    return(0);
#else
#ifdef  V7
    debug(F101,"sndbrk V7","",msec);
    return(genbrk(ttyfd,250));		/* Simulate a BREAK */
#else
    debug(F101,"sndbrk catchall","",msec);
    ttoc(0);ttoc(0);ttoc(0);ttoc(0);
    return(0);
#endif /* V7 */
#endif /* BSDBREAK */
#endif /* ATTSV */
#endif /* aegis */
#endif /* PROVX1 */
#endif /* POSIX */
#endif /* Plan9 */
}

/*  T T S N D B  --  Send a BREAK signal  */

int
ttsndb() {
    return(sndbrk(275));
}

/*  T T S N D L B  --  Send a Long BREAK signal  */

int
ttsndlb() {
    return(sndbrk(1500));
}
#endif /* NOLOCAL */

/*  M S L E E P  --  Millisecond version of sleep().  */

/*
  Call with number of milliseconds (thousandths of seconds) to sleep.
  Intended only for small intervals.  For big ones, just use sleep().
  Highly system-dependent.
  Returns 0 always, even if it didn't work.
*/

/* Define MSLFTIME for systems that must use an ftime() loop. */
#ifdef ANYBSD				/* For pre-4.2 BSD versions */
#ifndef BSD4
#define MSLFTIME
#endif /* BSD4 */
#endif /* ANYBSD */

#ifdef TOWER1				/* NCR Tower OS 1.0 */
#define MSLFTIME
#endif /* TOWER1 */

#ifdef COHERENT         /* Coherent... */
#ifndef _I386           /* Maybe Coherent/386 should get this, too */
#define MSLFTIME        /* Opinions are divided */
#endif /* _I386 */
#endif /* COHERENT */

#ifdef GETMSEC

/* Millisecond timer */

static long msecbase = 0L;		/* Unsigned long not portable */

long
getmsec() {				/* Milliseconds since base time */
    struct timeval xv;
    struct timezone xz;
    long secs, msecs;
    if (gettimeofday(&tv, &tz) < 0)
      return(-1);
    if (msecbase == 0L) {		/* First call, set base time. */
	msecbase = tv.tv_sec;
	debug(F101,"getmsec base","",msecbase);
    }
    return(((tv.tv_sec - msecbase) * 1000L) + (tv.tv_usec / 1000L));
}
#endif /* GETMSEC */

int
msleep(m) int m; {

#ifdef Plan9
    return _SLEEP(m);
#else
#ifdef SELECT
    int t1, x;
    debug(F101,"msleep SELECT 1","",m);
    if (m <= 0) return(0);
    if (m >= 1000) {			/* Catch big arguments. */
	sleep(m/1000);
	m = m % 1000;
	if (m < 10) return(0);
    }
    debug(F101,"msleep SELECT 2","",m);
#ifdef BELLV10
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, m );
    debug(F101,"msleep BELLV10 select","",x);
#else
#ifndef COHERENT
/* Hmmm...  what's all this then?  */
    if (gettimeofday(&tv, &tz) < 0) return(-1); /* Get current time. */
    t1 = tv.tv_sec;                     /* Seconds */
#endif /* COHERENT */
    tv.tv_sec = 0;                      /* Use select() */
    tv.tv_usec = m * 1000L;
#ifdef BSD44
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep BSD44 select","",x);
#else
#ifdef __linux__
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep __linux__ select","",x);
#else
#ifdef BSD43
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep BSD43 select","",x);
#else
#ifdef QNX
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep QNX select","",x);
#else
#ifdef COHERENT
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep COHERENT select","",x);
#else
#ifdef HPUX1000				/* 10.00 only, not 10.10 or later */
    x = select( 0, (int *)0, (int *)0, (int *)0, &tv );
    debug(F101,"msleep HP-UX 10.00 select","",x);
#else
#ifdef SVR4
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep SVR4 select","",x);
#else
#ifdef OSF40
    x = select( 0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
    debug(F101,"msleep BSD43 select","",x);
#else
    x = select( 0, (int *)0, (int *)0, (int *)0, &tv );
    debug(F101,"msleep catch-all select","",x);
#endif /* OSF40 */
#endif /* HP1000 */
#endif /* SVR4 */
#endif /* COHERENT */
#endif /* QNX */
#endif /* BSD43 */
#endif /* __linux__ */
#endif /* BSD44 */
#endif /* BELLV10 */
    return(0);

#else					/* Not SELECT */
#ifdef CK_POLL				/* We have poll() */
    struct pollfd pfd;			/* Supply a valid address for poll() */

#ifdef ODT30				/* But in SCO ODT 3.0 */
#ifdef NAP				/* we should use nap() instead */
    debug(F101,"msleep ODT 3.0 NAP","",m); /* because using poll() here */
    nap((long)m);			   /* seems to break dialing. */
    return(0);
#else
    debug(F101,"msleep ODT 3.0 POLL","",m);
    poll(&pfd, 0, m);
    return(0);
#endif /* NAP */
#else
    debug(F101,"msleep POLL","",m);
    poll(&pfd, 0, m);
    return(0);
#endif /* ODT30 */

/*
  We could handle the above more cleanly by just letting nap() take
  always take precedence over poll() in this routine, but there is no way
  to know whether that would break something else.
*/

#else					/* Not POLL */
#ifdef USLEEP
/*
  "This routine is implemented using setitimer(2); it requires eight
  system calls...".  In other words, it might take 5 minutes to sleep
  for 100 milliseconds...
*/
    debug(F101,"msleep USLEEP","",m);
    if (m >= 1000) {			/* Catch big arguments. */
	sleep(m/1000);
	m = m % 1000;
	if (m < 10) return(0);
    }
    usleep((unsigned int)(m * 1000));
    return(0);
#else
#ifdef aegis
    time_$clock_t dur;

    debug(F101,"msleep aegis","",m);
    dur.c2.high16 = 0;
    dur.c2.low32  = 250 * m; /* one millisecond = 250 four microsecond ticks */
    time_$wait(time_$relative, dur, st);
    return(0);
#else
#ifdef PROVX1
    debug(F101,"msleep Venix","",m);
    if (m <= 0) return(0);
    sleep(-((m * 60 + 500) / 1000));
    return(0);
#else
#ifdef NAP
    debug(F101,"msleep NAP","",m);
    nap((long)m);
    return(0);
#else
#ifdef ATTSV
#ifndef BSD44
    extern long times();		/* Or #include <times.h> ? */
#endif /* BSD44 */
    long t1, t2, tarray[4];
    int t3;

#ifdef COMMENT
/* This better be picked up in ckcdeb.h... */
    char *getenv();
#endif /* COMMENT */
    char *cp = getenv("HZ");
    int CLOCK_TICK;
    int hertz;

    if (cp && (hertz = atoi(cp))) {
        CLOCK_TICK  = 1000 / hertz;
    } else {				/* probably single user mode */
#ifdef HZ
        CLOCK_TICK  = 1000 / HZ;	
#else
	static warned = 0;
	/* HZ always exists in, for instance, SCO Xenix, so you don't have to
	 * make special #ifdefs for XENIX here, like in ver 4F. Also, if you
	 * have Xenix, you have should have nap(), so the best is to use -DNAP
	 * in the makefile. Most systems have HZ.
	 */
	CLOCK_TICK = 17;		/* 1/60 sec */
	if (!warned) {
          printf("warning: environment variable HZ bad... using HZ=%d\r\n",
		 1000 / CLOCK_TICK);
          warned = 1;
	}
#endif /* !HZ */
    }
    debug(F101,"msleep ATTSV","",m);
    if (m <= 0) return(0);
    if (m >= 1000) {			/* Catch big arguments. */
	sleep(m/1000);
	m = m % 1000;
	if (m < 10) return(0);
    }
    if ((t1 = times(tarray)) < 0) return(-1);
    while (1) {
        if ((t2 = times(tarray)) < 0) return(-1);
        t3 = ((int)(t2 - t1)) * CLOCK_TICK;
        if (t3 > m) return(t3);
    }
#else /* Not ATTSV */
#ifdef MSLFTIME				/* Use ftime() loop... */
    int t1, t3 = 0;
    debug(F101,"msleep MSLFTIME","",m);
    if (m <= 0) return(0);
    if (m >= 1000) {			/* Catch big arguments. */
	sleep(m/1000);
	m = m % 1000;
	if (m < 10) return(0);
    }
#ifdef QNX
    ftime(&ftp);			/* void ftime() in QNX */
#else
    if (ftime(&ftp) < 0) return(-1);	/* Get base time. */
#endif /* QNX */
    t1 = ((ftp.time & 0xff) * 1000) + ftp.millitm;
    while (1) {
        ftime(&ftp);			/* Get current time and compare. */
        t3 = (((ftp.time & 0xff) * 1000) + ftp.millitm) - t1;
        if (t3 > m) return(0);
    }
#else
/* This includes true POSIX, which has no way to do this. */
    debug(F101,"msleep busy loop","",m);
    if (m >= 1000) {			/* Catch big arguments. */
	sleep(m/1000);
	m = m % 1000;
	if (m < 10) return(0);
    }
    if (m > 0) while (m > 0) m--;	/* Just a dumb busy loop */
    return(0);
#endif /* MSLFTIME */
#endif /* ATTSV */
#endif /* NAP */
#endif /* PROVX1 */
#endif /* aegis */
#endif /* SELECT */
#endif /* CK_POLL */
#endif /* USLEEP */
#endif /* Plan9 */
}

/*  R T I M E R --  Reset elapsed time counter  */

VOID
rtimer() {
    tcount = time( (time_t *) 0 );
}


/*  G T I M E R --  Get current value of elapsed time counter in seconds  */

int
gtimer() {
    int x;
    x = (int) (time( (time_t *) 0 ) - tcount);
    debug(F101,"gtimer","",x);
    return( (x < 0) ? 0 : x );
}


/*  Z T I M E  --  Return date/time string  */

VOID
ztime(s) char **s; {

#undef ZTIMEV7				/* Which systems need to use */
#ifdef COHERENT				/* old UNIX Version 7 way... */
#define ZTIMEV7
#endif /* COHERENT */
#ifdef TOWER1
#define ZTIMEV7
#endif /* TOWER1 */
#ifdef ANYBSD
#ifndef BSD42
#define ZTIMEV7
#endif /* BSD42 */
#endif /* ANYBSD */
#ifdef V7
#ifndef MINIX
#define ZTIMEV7
#endif /* MINIX */
#endif /* V7 */
#ifdef POSIX
#define ZTIMEV7
#endif /* POSIX */

#ifdef HPUX1020
/*
  Prototypes are in <time.h>, included above.
*/
    time_t clock_storage;
    clock_storage = time((void *) 0);
    *s = ctime(&clock_storage);
    debug(F110,"ztime: HPUX 10.20",*s,0);
#else
#ifdef ATTSV				/* AT&T way */
/*  extern long time(); */		/* Theoretically these should */
    char *ctime();			/* already been dcl'd in <time.h> */
    long clock_storage;
    clock_storage = time(
#ifdef IRIX60
			 (time_t *)
#else
			 (long *)
#endif /* IRIX60 */
			 0 );
    *s = ctime( &clock_storage );
    debug(F110,"ztime: ATTSV",*s,0);
#else
#ifdef PROVX1				/* Venix 1.0 way */
    int utime[2];
    time(utime);
    *s = ctime(utime);
    debug(F110,"ztime: PROVX1",*s,0);
#else
#ifdef BSD42				/* 4.2BSD way */
    char *asctime();
    struct tm *localtime();
    struct tm *tp;
    gettimeofday(&tv, &tz);
    time(&tv.tv_sec);
    tp = localtime(&tv.tv_sec);
    *s = asctime(tp);
    debug(F110,"ztime: BSD42",*s,0);
#else
#ifdef MINIX				/* MINIX way */
#ifdef COMMENT
    extern long time();			/* Already got these from <time.h> */
    extern char *ctime();
#endif /* COMMENT */
    time_t utime[2];
    time(utime);
    *s = ctime(utime);
    debug(F110,"ztime: MINIX",*s,0);
#else
#ifdef ZTIMEV7				/* The regular way */
    char *asctime();
    struct tm *localtime();
    struct tm *tp;
#ifdef BEBOX
    unsigned long xclock;
#else
    long xclock;
#endif /* BEBOX */
    time(&xclock);
    tp = localtime(&xclock);
    *s = asctime(tp);
    debug(F110,"ztime: ZTIMEV7",*s,0);
#else					/* Catch-all for others... */
    *s = "Day Mon 00 00:00:00 0000\n"	/* Return dummy in asctime() format */
    debug(F110,"ztime: catch-all",*s,0);
#endif /* ZTIMEV7 */
#endif /* MINIX */
#endif /* BSD42 */
#endif /* PROVX1 */
#endif /* ATTSV */
#endif /* HPUX1020 */
}

/*  C O N G M  --  Get console terminal modes.  */

/*
  Saves initial console mode, and establishes variables for switching
  between current (presumably normal) mode and other modes.
  Should be called when program starts, but only after establishing
  whether program is in the foreground or background.
  Returns 1 if it got the modes OK, 0 if it did nothing, -1 on error.
*/
int
congm() {
    int fd;
    if (backgrd || !isatty(0)) {	/* If in background. */
	cgmf = -1;			/* Don't bother, modes are garbage. */
	return(-1);
    }
    if (cgmf > 0) return(0);		/* Already did this. */
    debug(F100,"congm getting modes","",0); /* Need to do it. */
#ifdef aegis
    ios_$inq_type_uid(ios_$stdin, conuid, st);
    if (st.all != status_$ok) {
	fprintf(stderr, "problem getting stdin objtype: ");
	error_$print(st);
    }
    concrp = (conuid == mbx_$uid);
    conbufn = 0;
#endif /* aegis */

#ifndef BEBOX
    if ((fd = open(CTTNAM,2)) < 0) {	/* Open controlling terminal */
#ifdef COMMENT
	fprintf(stderr,"Error opening %s\n", CTTNAM);
	perror("congm");
	return(-1);
#else
	fd = 0;
#endif /* COMMENT */
    }
#else
    fd = 0;
#endif /* !BEBOX */
#ifdef BSD44ORPOSIX
    if (tcgetattr(fd,&ccold) < 0) return(-1);
    if (tcgetattr(fd,&cccbrk) < 0) return(-1);
    if (tcgetattr(fd,&ccraw) < 0) return(-1);
#else
#ifdef ATTSV
    if (ioctl(fd,TCGETA,&ccold)  < 0) return(-1);
    if (ioctl(fd,TCGETA,&cccbrk) < 0) return(-1);
    if (ioctl(fd,TCGETA,&ccraw)  < 0) return(-1);
#ifdef VXVE
    cccbrk.c_line = 0;			/* STTY line 0 for CDC VX/VE */
    if (ioctl(fd,TCSETA,&cccbrk) < 0) return(-1);
    ccraw.c_line = 0;			/* STTY line 0 for CDC VX/VE */
    if (ioctl(fd,TCSETA,&ccraw) < 0) return(-1);
#endif /* VXVE */
#else
#ifdef BELLV10
    if (ioctl(fd,TIOCGETP,&ccold) < 0) return(-1);
    if (ioctl(fd,TIOCGETP,&cccbrk) < 0) return(-1);
    if (ioctl(fd,TIOCGETP,&ccraw) < 0) return(-1);
#else
    if (gtty(fd,&ccold) < 0) return(-1);
    if (gtty(fd,&cccbrk) < 0) return(-1);
    if (gtty(fd,&ccraw) < 0) return(-1);
#endif /* BELLV10 */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
#ifdef sony_news			/* Sony NEWS */
    if (ioctl(fd,TIOCKGET,&km_con) < 0) { /* Get console Kanji mode */
	perror("congm error getting Kanji mode");
	debug(F101,"congm error getting Kanji mode","",0);
	km_con = -1;			/* Make sure this stays undefined. */
	return(-1);
    }
#endif /* sony_news */
    if (fd > 0)
      close(fd);
    cgmf = 1;				/* Flag that we got them. */
    return(1);
}


/*  C O N C B --  Put console in cbreak mode.  */

/*  Returns 0 if ok, -1 if not  */

int
#ifdef CK_ANSIC
concb(char esc)
#else
concb(esc) char esc;
#endif /* CK_ANSIC */
/* concb */ {
    int x;
    debug(F101,"concb cgmf","",cgmf);
    debug(F101,"concb backgrd","",backgrd);

    if (cgmf < 1)			/* Did we get console modes yet? */
      if (!backgrd)			/* No, in background? */
	congm();			/* No, try to get them now. */
    if (cgmf < 1)			/* Still don't have them? */
      return(0);			/* Give up. */
    debug(F101,"concb ttyfd","",ttyfd);
    debug(F101,"concb ttfdflg","",ttfdflg);
    if (ttfdflg && ttyfd >= 0 && ttyfd < 3)
      return(0);
    if (!isatty(0)) return(0);          /* Only for real ttys */
    debug(F100,"concb isatty","",0);
    debug(F101,"concb suspend","",suspend);
    if (backgrd) return(0);		/* Do nothing if in background. */
    escchr = esc;                       /* Make this available to other fns */
    ckxech = 1;                         /* Program can echo characters */
#ifdef aegis
    conbufn = 0;
    if (concrp) return(write(1, "\035\002", 2));
    if (conuid == input_pad_$uid) {pad_$raw(ios_$stdin, st); return(0);}
#endif /* aegis */

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifdef Plan9
    x = p9concb();
#else
#ifndef SVORPOSIX			/* BSD, V7, etc */
    cccbrk.sg_flags |= CBREAK;          /* Set to character wakeup, */
    cccbrk.sg_flags &= ~ECHO;           /* no echo. */
#ifdef BELLV10
    x = ioctl(0,TIOCSETP,&cccbrk);
#else
    x = stty(0,&cccbrk);
#endif /* BELLV10 */
#else					/* Sys V and POSIX */
#ifndef OXOS
    debug(F101,"concb cccbrk.c_flag","",cccbrk.c_lflag);
#ifdef QNX
    /* Don't mess with IEXTEN */
    cccbrk.c_lflag &= ~(ICANON|ECHO);
#else
#ifdef COHERENT
    cccbrk.c_lflag &= ~(ICANON|ECHO);
#else
    cccbrk.c_lflag &= ~(ICANON|ECHO|IEXTEN);
#endif /* COHERENT */
#endif /* QNX */
    cccbrk.c_lflag |= ISIG;    	/* Allow SIGINT, etc, in command mode. */
#else /* OXOS */
    debug(F100,"concb OXOS is defined","",0);
    cccbrk.c_lflag &= ~(ICANON|ECHO);
    cccbrk.c_cc[VDISCARD] = cccbrk.c_cc[VLNEXT] = CDISABLE;
#endif /* OXOS */
#ifdef VSUSP
    debug(F101,"concb c_cc[VSUSP]","",cccbrk.c_cc[VSUSP]);
#endif /* VSUSP */
#ifndef VINTR
    debug(F101,"concb c_cc[0]","",cccbrk.c_cc[0]);
    cccbrk.c_cc[0] = 003;               /* Interrupt char is Control-C */
#else
    debug(F101,"concb c_cc[VINTR]","",cccbrk.c_cc[0]);
    cccbrk.c_cc[VINTR] = 003;
#endif /* VINTR */
#ifndef VQUIT
    cccbrk.c_cc[1] = escchr;            /* escape during packet modes */
#else
    cccbrk.c_cc[VQUIT] = escchr;
#endif /* VQUIT */
#ifndef VEOF
    cccbrk.c_cc[4] = 1;
#else
#ifndef OXOS
#ifdef VMIN
    cccbrk.c_cc[VMIN] = 1;
#endif /* VMIN */
#else /* OXOS */
    cccbrk.c_min = 1;
#endif /* OXOS */
#endif /* VEOF */
#ifdef ZILOG
    cccbrk.c_cc[5] = 0;
#else
#ifndef VEOL
    cccbrk.c_cc[5] = 1;
#else
#ifndef OXOS
#ifdef VTIME
    cccbrk.c_cc[VTIME] = 1;
#endif /* VTIME */
#else /* OXOS */
    cccbrk.c_time = 1;
#endif /* OXOS */
#endif /* VEOL */
#endif /* ZILOG */
#ifdef BSD44ORPOSIX			/* Set new modes */
    x = tcsetattr(0,TCSADRAIN,&cccbrk);
#else /* ATTSV */      			/* or the POSIX way */
    x = ioctl(0,TCSETAW,&cccbrk);	/* the Sys V way */
#endif /* BSD44ORPOSIX */
#endif /* SVORPOSIX */

#ifdef COHERENT
#undef SVORPOSIX
#endif /* COHERENT */

#ifndef aegis
#ifndef NOSETBUF
    if (x > -1) setbuf(stdout,NULL);    /* Make console unbuffered. */
#endif /* NOSETBUF */
#endif /* aegis */

#ifdef  V7
#ifndef MINIX
    if (kmem[CON] < 0) {
        qaddr[CON] = initrawq(0);
        if((kmem[CON] = open("/dev/kmem", 0)) < 0) {
            fprintf(stderr, "Can't read /dev/kmem in concb.\n");
            perror("/dev/kmem");
            exit(1);
        }
    }
#endif /* MINIX */
#endif /* V7 */
#endif /* Plan9 */
    debug(F101,"concb returns","",x);
    return(x);
}

/*  C O N B I N  --  Put console in binary mode  */

/*  Returns 0 if ok, -1 if not  */

int
#ifdef CK_ANSIC
conbin(char esc)
#else
conbin(esc) char esc;
#endif /* CK_ANSIC */
/* conbin */  {
    if (!isatty(0)) return(0);          /* only for real ttys */
    congm();				/* Get modes if necessary. */
    debug(F100,"conbin","",0);
    escchr = esc;                       /* Make this available to other fns */
    ckxech = 1;                         /* Program can echo characters */
#ifdef aegis
    conbufn = 0;
    if (concrp) return(write(1, "\035\002", 2));
    if (conuid == input_pad_$uid) {
	pad_$raw(ios_$stdin, st);
	return(0);
      }
#endif /* aegis */

#ifdef COHERENT
#define SVORPOSIX
#endif /* COHERENT */

#ifdef Plan9
    return p9conbin();
#else
#ifdef SVORPOSIX
#ifndef OXOS
#ifdef QNX
    ccraw.c_lflag &= ~(ISIG|ICANON|ECHO);
#else
#ifdef COHERENT
    ccraw.c_lflag &= ~(ISIG|ICANON|ECHO);
#else
    ccraw.c_lflag &= ~(ISIG|ICANON|ECHO|IEXTEN);
#endif /* COHERENT */
#endif /* QNX */
#else /* OXOS */
    ccraw.c_lflag &= ~(ISIG|ICANON|ECHO);
    ccraw.c_cc[VDISCARD] = ccraw.c_cc[VLNEXT] = CDISABLE;
#endif /* OXOS */
    ccraw.c_iflag |= (BRKINT|IGNPAR);
/*
  Note that for terminal sessions we disable Xon/Xoff flow control to allow
  the passage ^Q and ^S as data characters for EMACS, and to allow XMODEM
  transfers to work when C-Kermit is in the middle, etc.  Hardware flow
  control, if in use, is not affected.
*/
#ifdef ATTSV
#ifdef BSD44
    ccraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|IXON|IXANY|IXOFF
                        |INPCK|ISTRIP);
#else
    ccraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|IUCLC|IXON|IXANY|IXOFF
                        |INPCK|ISTRIP);
#endif /* BSD44 */
#else /* POSIX */
    ccraw.c_iflag &= ~(IGNBRK|INLCR|IGNCR|ICRNL|IXON|IXOFF|INPCK|ISTRIP);
#endif /* ATTSV */
    ccraw.c_oflag &= ~OPOST;
#ifdef COMMENT
/*
  WHAT THE HECK WAS THIS FOR?
  The B9600 setting (obviously) prevents CONNECT from working at any
  speed other than 9600 when you are logged in to the 7300 on a serial
  line.  Maybe some of the other flags are necessary -- if so, put back
  the ones that are needed.  This code is supposed to work the same, no
  matter whether you are logged in to the 7300 on the real console device,
  or through a serial port.
*/
#ifdef ATT7300
    ccraw.c_cflag = CLOCAL | B9600 | CS8 | CREAD | HUPCL;
#endif /* ATT7300 */
#endif /* COMMENT */

/*** Kermit used to put the console in 8-bit raw mode, but some users have
 *** pointed out that this should not be done, since some sites actually
 *** use terminals with parity settings on their Unix systems, and if we
 *** override the current settings and stop doing parity, then their terminals
 *** will display blotches for characters whose parity is wrong.  Therefore,
 *** the following two lines are commented out (Larry Afrin, Clemson U):
 ***
 ***   ccraw.c_cflag &= ~(PARENB|CSIZE);
 ***   ccraw.c_cflag |= (CS8|CREAD);
 ***
 *** Sys III/V sites that have trouble with this can restore these lines.
 ***/
#ifndef VINTR
    ccraw.c_cc[0] = 003;		/* Interrupt char is Ctrl-C */
#else
    ccraw.c_cc[VINTR] = 003;
#endif /* VINTR */
#ifndef VQUIT
    ccraw.c_cc[1] = escchr;		/* Escape during packet mode */
#else
    ccraw.c_cc[VQUIT] = escchr;
#endif /* VQUIT */
#ifndef VEOF
    ccraw.c_cc[4] = 1;
#else
#ifndef OXOS
#ifdef VMIN
    ccraw.c_cc[VMIN] = 1;
#endif /* VMIN */
#else /* OXOS */
    ccraw.c_min = 1;
#endif /* OXOS */
#endif /* VEOF */

#ifdef ZILOG
    ccraw.c_cc[5] = 0;
#else
#ifndef VEOL
    ccraw.c_cc[5] = 1;
#else
#ifndef OXOS
#ifdef VTIME
    ccraw.c_cc[VTIME] = 1;
#endif /* VTIME */
#else /* OXOS */
    ccraw.c_time = 1;
#endif /* OXOS */
#endif /* VEOL */
#endif /* ZILOG */

#ifdef BSD44ORPOSIX
    return(tcsetattr(0,TCSADRAIN,&ccraw));
#else
    return(ioctl(0,TCSETAW,&ccraw));	/* Set new modes. */
#endif /* BSD44ORPOSIX */

#else /* Berkeley, etc. */
    ccraw.sg_flags |= (RAW|TANDEM);     /* Set rawmode, XON/XOFF (ha) */
    ccraw.sg_flags &= ~(ECHO|CRMOD);    /* Set char wakeup, no echo */
#ifdef BELLV10
    return(ioctl(0,TIOCSETP,&ccraw));
#else
    return(stty(0,&ccraw));
#endif /* BELLV10 */
#endif /* SVORPOSIX */
#endif /* Plan9 */

#ifdef COHERENT
#undef SVORPOSIX
#endif /* COHERENT */

}


/*  C O N R E S  --  Restore the console terminal  */

int
conres() {
    debug(F101,"conres cgmf","",cgmf);
    if (cgmf < 1) return(0);		/* Do nothing if modes unchanged */
    if (!isatty(0)) return(0);          /* only for real ttys */
    debug(F100,"conres isatty ok","",0);
    ckxech = 0;                         /* System should echo chars */

#ifdef aegis
    conbufn = 0;
    if (concrp) return(write(1, "\035\001", 2));
    if (conuid == input_pad_$uid) {
	pad_$cooked(ios_$stdin, st);
	return(0);
    }
#endif /* aegis */

#ifdef Plan9
    p9conres();
#else
#ifdef BSD44ORPOSIX
    debug(F100,"conres restoring tcsetattr","",0);
    return(tcsetattr(0,TCSADRAIN,&ccold));
#else
#ifdef ATTSV
    debug(F100,"conres restoring ioctl","",0);
    return(ioctl(0,TCSETAW,&ccold));
#else /* BSD, V7, and friends */
#ifdef sony_news			/* Sony NEWS */
    if (km_con != -1)
      ioctl(0,TIOCKSET,&km_con);	/* Restore console Kanji mode */
#endif /* sony_news */
    msleep(100);
    debug(F100,"conres restoring stty","",0);
#ifdef BELLV10
    return(ioctl(0,TIOCSETP,&ccold));
#else
    return(stty(0,&ccold));
#endif /* BELLV10 */
#endif /* ATTSV */
#endif /* BSD44ORPOSIX */
#endif /* Plan9 */
}

/*  C O N O C  --  Output a character to the console terminal  */

int
#ifdef CK_ANSIC
conoc(char c)
#else
conoc(c) char c;
#endif /* CK_ANSIC */
/* conoc */ {
#ifdef Plan9
    return conwrite(&c,1);
#else
    return(write(1,&c,1));
#endif /* Plan9 */
}

/*  C O N X O  --  Write x characters to the console terminal  */

int
conxo(x,s) int x; char *s; {
#ifdef Plan9
    return(conwrite(s,x));
#else
    return(write(1,s,x));
#endif /* Plan9 */
}

/*  C O N O L  --  Write a line to the console terminal  */

int
conol(s) char *s; {
    int len;
    len = (int)strlen(s);
#ifdef Plan9
    return(conwrite(s,len));
#else
    return(write(1,s,len));
#endif /* Plan9 */
}

/*  C O N O L A  --  Write an array of lines to the console terminal */

int
conola(s) char *s[]; {
    int i;
    for (i=0 ; *s[i] ; i++) if (conol(s[i]) < 0) return(-1);;
    return(0);
}

/*  C O N O L L  --  Output a string followed by CRLF  */

int
conoll(s) char *s; {
    conol(s);
#ifdef Plan9
    return(conwrite("\r\n", 2));
#else
    return(write(1,"\r\n",2));
#endif /* Plan9 */
}

/*  C O N C H K  --  Return how many characters available at console  */
/*
  We could also use select() here to cover a few more systems that are not
  covered by any of the following, e.g. HP-UX 9.0x on the model 800.
*/
int
conchk() {
    static int contyp = 0;		/* +1 for isatty, -1 otherwise */

    if (contyp == 0)			/* This prevents unnecessary */
      contyp = (isatty(0) ? 1 : -1);	/* duplicated calls to isatty() */
    debug(F101,"conchk contyp","",contyp);
    if (backgrd || (contyp < 0))
      return(0);

#ifdef aegis
    if (conbufn > 0) return(conbufn);   /* use old count if nonzero */

    /* read in more characters */
    conbufn = ios_$get(ios_$stdin,
              ios_$cond_opt, conbuf, (long)sizeof(conbuf), st);
    if (st.all != status_$ok) conbufn = 0;
    conbufp = conbuf;
    return(conbufn);
#else
    return(in_chk(0));
#endif /* aegis */
}

/*  C O N I N C  --  Get a character from the console  */
/*
  Call with timo > 0 to do a timed read, timo == 0 to do an untimed blocking
  read.  Upon success, returns the character.  Upon failure, returns -1.
  A timed read that does not complete within the timeout period returns -1.
*/
int
coninc(timo) int timo; {
    int n = 0; CHAR ch;
    int xx;
#ifdef aegis				/* Apollo Aegis only... */
    debug(F101,"coninc timo","",timo);
    fflush(stdout);
    if (conchk() > 0) {
	--conbufn;
	return((unsigned)(*conbufp++ & 0xff));
    }
#endif /* aegis */

    if (timo <= 0 ) {                   /* Untimed, blocking read. */
	while (1) {			/* Keep trying till we get one. */
	    n = read(0, &ch, 1);	/* Read a character. */
	    if (n == 0) continue;	/* Shouldn't happen. */
	    if (n > 0)			/* If read was successful, */
	      return((unsigned)(ch & 0xff)); /* return the character. */

/* Come here if read() returned an error. */

	    debug(F101, "coninc(0) errno","",errno); /* Log the error. */
#ifndef OXOS
#ifdef SVORPOSIX
#ifdef CIE                             /* CIE Regulus has no EINTR symbol? */
#ifndef EINTR
#define EINTR 4
#endif /* EINTR */
#endif /* CIE */
/*
  This routine is used for several different purposes.  In CONNECT mode, it is
  used to do an untimed, blocking read from the keyboard in the lower CONNECT
  fork.  During local-mode file transfer, it reads a character from the
  console to interrupt the file transfer (like A for a status report, X to
  cancel a file, etc).  Obviously, we don't want the reads in the latter case
  to be blocking, or the file transfer would stop until the user typed
  something.  Unfortunately, System V does not allow the console device input
  buffer to be sampled nondestructively (e.g. by conchk()), so a kludge is
  used instead.  During local-mode file transfer, the SIGQUIT signal is armed
  and trapped by esctrp(), and this routine pretends to have read the quit
  character from the keyboard normally.  But, kludge or no kludge, the read()
  issued by this command, under System V only, can fail if a signal -- ANY
  signal -- is caught while the read is pending.  This can occur not only when
  the user types the quit character, but also during telnet negotiations, when
  the lower CONNECT fork signals the upper one about an echoing mode change.
  When this happens, we have to post the read() again.  This is apparently not
  a problem in BSD-based UNIX versions.
*/
	    if (errno == EINTR)		/* Read interrupted. */
	      if (conesc)  {		/* If by SIGQUIT, */
 		 conesc = 0;		/* the conesc variable is set, */
 		 return(escchr);	/* so return the escape character. */
	     } else continue;		/* By other signal, try again. */
#else
/*
  This might be dangerous, but let's do this on non-System V versions too,
  since at least one SunOS 4.1.2 user complains of immediate disconnections
  upon first making a TELNET connection.
*/
	    if (errno == EINTR)		/* Read interrupted. */
	      continue;
#endif /* SVORPOSIX */
#else /* OXOS */
	    if (errno == EINTR)		/* Read interrupted. */
	      continue;
#endif /* OXOS */
	    return(-1);			/* Error */
	}
    }

    if (timo <= 0)
      /* This should never happen */
      debug(F100,"coninc HORRIBLE ERROR","",0);

/* Timed read... */

    saval = signal(SIGALRM,timerh);	/* Set up timeout handler. */
    xx = alarm(timo);			/* Set the alarm. */
    debug(F101,"coninc alarm set","",xx);
    if (
#ifdef CK_POSIX_SIG
	sigsetjmp(sjbuf,1)
#else
	setjmp(sjbuf)
#endif /* CK_POSIX_SIG */
	)				/* The read() timed out. */
      n = -2;				/* Code for timeout. */
    else
      n = read(0, &ch, 1);
    ttimoff();				/* Turn off timer */
    if (n > 0)				/* Got character OK. */
      return((unsigned)(ch & 0xff));	/* Return it. */

/*
  read() returned an error.  Same deal as above, but without the loop.
*/
    debug(F101, "coninc(timo) n","",n);
    debug(F101, "coninc(timo) errno","",errno);
#ifndef OXOS
#ifdef SVORPOSIX
    if (n == -1 && errno == EINTR && conesc != 0) {
	conesc = 0;
	return(escchr);			/* User entered escape character. */
    } else				/* n == 0 shouldn't happen. */
#endif /* SVORPOSIX */
#endif /* ! OXOS */
      return(-1);
}

/*  C O N G K S  --  Console Get Keyboard Scancode  */

#ifndef congks
/*
  This function needs to be filled in with the various system-dependent
  system calls used by SUNOS, NeXT OS, Xenix, Aviion, etc, to read a full
  keyboard scan code.  For now, it's a dummy.
*/
int
congks(timo) int timo; {
    return(coninc(timo));
}
#endif /* congks */

#ifdef ATT7300

/*  A T T D I A L  --  Dial up the remote system using internal modem
 * Purpose: to open and dial a number on the internal modem available on the
 * ATT7300 UNIX PC.  Written by Joe Doupnik. Superceeds version written by
 * Richard E. Hill, Dickinson, TX. which employed dial(3c).
 * Uses information in <sys/phone.h> and our status int attmodem.
 */
attdial(ttname,speed,telnbr) char *ttname,*telnbr; long speed; {
    char *telnum;
    int ttclos();

    attmodem &= ~ISMODEM;                       /* modem not in use yet */
                    /* Ensure O_NDELAY is set, else i/o traffic hangs */
                    /* We turn this flag off once the dial is complete */
    fcntl(ttyfd, F_SETFL, fcntl(ttyfd, F_GETFL, 0) | O_NDELAY);

    /* Condition line, check availability & DATA mode, turn on speaker */
    if (ioctl(ttyfd,PIOCOFFHOOK, &dialer) == -1) {
        printf("cannot access phone\n");
        ttclos(0);
        return (-2);
    }
    ioctl(ttyfd,PIOCGETP,&dialer);      /* get phone dialer parameters */

    if (dialer.c_lineparam & VOICE) {	/* phone must be in DATA mode */
        printf(" Should not dial with modem in VOICE mode.\n");
        printf(" Exit Kermit, switch to DATA and retry call.\n");
        ttclos(0);
        return (-2);
    }
#ifdef ATTTONED				/* Old way, tone dialing only. */
    dialer.c_lineparam = DATA | DTMF;	/* Dial with tones, */
    dialer.c_lineparam &= ~PULSE;	/* not with pulses. */
#else
    /* Leave current pulse/tone state alone. */
    /* But what about DATA?  Add it back if you have trouble. */
    /* sys/phone says you get DATA automatically by opening device RDWR */
#endif
    dialer.c_waitdialtone = 5;                  /* wait 5 sec for dialtone */
#ifdef COMMENT
    dialer.c_feedback = SPEAKERON|NORMSPK|RINGON;  /* control speaker */
#else
    /* sys/phone says RINGON used only for incoming voice calls */
    dialer.c_feedback &= ~(SOFTSPK|LOUDSPK);
    dialer.c_feedback |= SPEAKERON|NORMSPK;
#endif
    dialer.c_waitflash = 500;                   /* 0.5 sec flash hook */
    if(ioctl(ttyfd,PIOCSETP,&dialer) == -1) {   /* set phone parameters */
        printf("Cannot set modem characteristics\n");
        ttclos(0);
        return (-2);
    }
    ioctl(ttyfd,PIOCRECONN,0);		/* Turns on speaker for pulse */

#ifdef COMMENT
    fprintf(stderr,"Phone line status. line_par:%o dialtone_wait:%o \
line_status:%o feedback:%o\n",
    dialer.c_lineparam, dialer.c_waitdialtone,
    dialer.c_linestatus, dialer.c_feedback);
#endif

    attmodem |= ISMODEM;                        /* modem is now in-use */
    sleep(1);
    for (telnum = telnbr; *telnum != '\0'; telnum++)    /* dial number */
#ifdef ATTTONED
      /* Tone dialing only */
      if (ioctl(ttyfd,PIOCDIAL,telnum) != 0) {
	  perror("Error in dialing");
	  ttclos(0);
	  return(-2);
      }
#else /* Allow Pulse or Tone dialing */
    switch (*telnum) {
      case 't': case 'T': case '%':	/* Tone dialing requested */
	dialer.c_lineparam |= DTMF;
	dialer.c_lineparam &= ~PULSE;
	if (ioctl(ttyfd,PIOCSETP,&dialer) == -1) {
	    printf("Cannot set modem to tone dialing\n");
	    ttclos(0);
	    return(-2);
	}
	break;
      case 'd': case 'D': case 'p': case 'P': case '^':
	dialer.c_lineparam |= PULSE;
	dialer.c_lineparam &= ~DTMF;
	if (ioctl(ttyfd,PIOCSETP,&dialer) == -1) {
	    printf("Cannot set modem to pulse dialing\n");
	    ttclos(0);
	    return(-2);
	}
	break;
      default:
        if (ioctl(ttyfd,PIOCDIAL,telnum) != 0) {
	    perror("Dialing error");
	    ttclos(0);
	    return(-2);
	}
	break;
    }
#endif

    ioctl(ttyfd,PIOCDIAL,"@");		/* terminator for data call */
    do {				/* wait for modems to Connect */
        if (ioctl(ttyfd,PIOCGETP,&dialer) != 0)	{ /* get params */
	    perror("Cannot get modems to connect");
	    ttclos(0);
	    return(-2);
	}
    } while ((dialer.c_linestatus & MODEMCONNECTED) == 0);
    /* Turn off O_NDELAY flag now. */
    fcntl(ttyfd, F_SETFL, fcntl(ttyfd, F_GETFL, 0) & ~O_NDELAY);
    signal(SIGHUP, ttclos);             /* hangup on loss of carrier */
    return(0);                          /* return success */
}

/*
  Offgetty, ongetty functions. These function get the 'getty(1m)' off
  and restore it to the indicated line.  Shell's return codes are:
    0: Can't do it.  Probably a user logged on.
    1: No need.  No getty on that line.
    2: Done, you should restore the getty when you're done.
  DOGETY System(3), however, returns them as 0, 256, 512, respectively.
  Thanks to Kevin O'Gorman, Anarm Software Systems.

   getoff.sh looks like:   geton.sh looks like:
     setgetty $1 0           setgetty $1 1
     err=$?                  exit $?
     sleep 2
     exit $err
*/

/*  O F F G E T T Y  --  Turn off getty(1m) for the communications tty line
 * and get status so it can be restarted after the line is hung up.
 */
int
offgetty(ttname) char *ttname; {
    char temp[30];
    while (*ttname != '\0') ttname++;       /* seek terminator of path */
    ttname -= 3;                            /* get last 3 chars of name */
    sprintf(temp,"/usr/bin/getoff.sh %s",ttname);
    return(zsyscmd(temp));
}

/*  O N G E T T Y  --  Turn on getty(1m) for the communications tty line */

int
ongetty(ttname) char *ttname; {
    char temp[30];
    while (*ttname != '\0') ttname++;       /* comms tty path name */
    ttname -= 3;
    sprintf(temp,"/usr/bin/geton.sh %s",ttname);
    return(zsyscmd(temp));
}
#endif /* ATT7300 */

/*  T T S C A R R  --  Set ttcarr variable, controlling carrier handling.
 *
 *  0 = Off: Always ignore carrier. E.g. you can connect without carrier.
 *  1 = On: Heed carrier, except during dialing. Carrier loss gives disconnect.
 *  2 = Auto: For "modem direct": The same as "Off".
 *            For real modem types: Heed carrier during connect, but ignore
 *                it anytime else.  Compatible with pre-5A C-Kermit versions.
 *
 * As you can see, this setting does not affect dialing, which always ignores
 * carrier (unless there is some special exception for some modem type).  It
 * does affect ttopen() if it is set before ttopen() is used.  This setting
 * takes effect on the next call to ttopen()/ttpkt()/ttvt().  And they are
 * (or should be) always called before any communications is tried, which
 * means that, practically speaking, the effect is immediate.
 *
 * Of course, nothing of this applies to remote mode (xlocal = 0).
 *
 * Someone has yet to uncover how to manipulate the carrier in the BSD
 * environment (or any non-termio using environment).  Until that time, this
 * will simply be a no-op for BSD.
 *
 * Note that in previous versions, the carrier was most often left unchanged
 * in ttpkt()/ttvt() unless they were called with FLO_DIAL or FLO_DIAX.  This
 * has changed.  Now it is controlled by ttcarr in conjunction with these
 * modes.
 */
int
ttscarr(carrier) int carrier; {
    ttcarr = carrier;
    debug(F101, "ttscarr","",ttcarr);
    return(ttcarr);
}

/* C A R R C T L  --  Set tty modes for carrier treatment.
 *
 * Sets the appropriate bits in a termio or sgttyb struct for carrier control
 * (actually, there are no bits in sgttyb for that), or performs any other
 * operations needed to control this on the current system.  The function does
 * not do the actual TCSETA or stty, since often we want to set other bits too
 * first.  Don't call this function when xlocal is 0, or the tty is not opened.
 *
 * We don't know how to do anything like carrier control on non-ATTSV systems,
 * except, apparently, ultrix.  See above.  It is also known that this doesn't
 * have much effect on a Xenix system.  For Xenix, one should switch back and
 * forth between the upper and lower case device files.  Maybe later. 
 * Presently, Xenix will stick to the mode it was opened with.
 *
 * carrier: 0 = ignore carrier, 1 = require carrier.
 * The current state is saved in curcarr, and checked to save labour.
 */
#ifdef SVORPOSIX
int
#ifdef BSD44ORPOSIX
carrctl(ttpar, carrier)	struct termios *ttpar; int carrier;
#else /* ATTSV */
carrctl(ttpar, carrier)	struct termio *ttpar; int carrier;
#endif /* BSD44ORPOSIX */
/* carrctl */ {
    debug(F101, "carrctl","",carrier);
    if (carrier)
      ttpar->c_cflag &= ~CLOCAL;
    else
      ttpar->c_cflag |= CLOCAL;
    return(0);
}
#else /* Berkeley, V7, et al... */
int
carrctl(ttpar, carrier) struct sgttyb *ttpar; int carrier; {
    debug(F101, "carrctl","",carrier);
    if (carrier == curcarr)
      return(0);
    curcarr = carrier;
#ifdef ultrix
#ifdef COMMENT
/*
  Old code from somebody at DEC that tends to get stuck, time out, etc.
*/
    if (carrier) {
	ioctl(ttyfd, TIOCMODEM, &temp);
	ioctl(ttyfd, TIOCHPCL, 0);
    } else {
	/* (According to the manuals, TIOCNCAR should be preferred */
	/* over TIOCNMODEM...) */
	ioctl(ttyfd, TIOCNMODEM, &temp);
    }
#else
/*
  New code from Jamie Watson that, he says, eliminates the problems.
*/
    if (carrier) {
	ioctl(ttyfd, TIOCCAR);
	ioctl(ttyfd, TIOCHPCL);
    } else {
	ioctl(ttyfd, TIOCNCAR);
    }
#endif /* COMMENT */
#endif /* ultrix */
    return(0);
}
#endif /* SVORPOSIX */


/*  T T G M D M  --  Get modem signals  */
/*
 Looks for RS-232 modem signals, and returns those that are on in as its
 return value, in a bit mask composed of the BM_xxx values defined in ckcdeb.h.
 Returns: 
 -3 Not implemented
 -2 if the communication device does not have modem control (e.g. telnet)
 -1 on error.
 >= 0 on success, with a bit mask containing the modem signals that are on.
*/

/*
  Define the symbol K_MDMCTL if we have Sys V R3 / 4.3 BSD style
  modem control, namely the TIOCMGET ioctl.
*/

#ifdef BSD43
#define K_MDMCTL
#endif

#ifdef SUNOS4
#define K_MDMCTL
#endif

#ifdef QNX
#define K_MDMCTL
#else
#ifdef TIOCMGET
#define K_MDMCTL
#endif /* TIOCMGET */
#endif /* QNX */

int
ttgmdm() {

#ifdef QNX
#include <sys/qioctl.h>

    unsigned long y, mdmbits[2] = { 0L, 0L };
    int z = 0;

    if (!qnx_ioctl( ttyfd, QCTL_DEV_CTL, &mdmbits[0], 8, &mdmbits[0], 4)) {
	y = mdmbits[0];
	if (y & 0x100000L) z |= BM_CTS;	/* Values from comment */
	if (y & 0x200000L) z |= BM_DSR;	/* in sys/qioctl.h */
	if (y & 0x800000L) z |= BM_DCD;
	if (y & 0x400000L) z |= BM_RNG;
	if (y & 0x000001L) z |= BM_DTR;
	if (y & 0x000002L) z |= BM_RTS;
	return(z);
    } else return(-1);
#else
#ifdef HPUX				/* HPUX has its own way */
    int x, z;

#ifdef HPUX10				/* Modem flag word */
    mflag y;				/* mflag typedef'd in <sys/modem.h> */
#else
#ifdef HPUX9
    mflag y;
#else
#ifdef HPUX8
    mflag y;
#else
    unsigned long y;			/* Not sure about pre-8.0... */
#endif /* HPUX8 */
#endif /* HPUX9 */
#endif /* HPUX10 */

    if (netconn)			/* Network connection */
      return(-2);			/* No modem signals */

    if (xlocal)				/* Get modem signals */
      x = ioctl(ttyfd,MCGETA,&y);
    else
      x = ioctl(0,MCGETA,&y);
    if (x < 0) return(-1);
    debug(F101,"ttgmdm","",y);

    z = 0;				/* Initialize return value */

/* Now set bits for each modem signal that is reported to be on. */

#ifdef MCTS
    /* Clear To Send */
    if (y & MCTS) z |= BM_CTS;
#endif
#ifdef MDSR
    /* Data Set Ready */
    if (y & MDSR) z |= BM_DSR;
#endif
#ifdef MDCD
    /* Carrier */
    if (y & MDCD) z |= BM_DCD;
#endif
#ifdef MRI
    /* Ring Indicate */
    if (y & MRI) z |= BM_RNG;
#endif
#ifdef MDTR
    /* Data Terminal Ready */
    if (y & MDTR) z |= BM_DTR;
#endif
#ifdef MRTS
    /* Request To Send */
    if (y & MRTS) z |= BM_RTS;
#endif
    return(z);

#else /* ! HPUX */

#ifdef K_MDMCTL
/*
  Note, <sys/ttycom> might have already been included by by <sys/ioctl.h>.
  Hence the following ifndef on a symbol which is defined there.
*/
#ifndef TIOCMGET
#include <sys/ttycom.h>
#endif /* TIOCMGET */

    int x, y, z;

    if (netconn)			/* Network connection */
      return(-2);			/* No modem signals */

    if (xlocal)
      x = ioctl(ttyfd,TIOCMGET,&y);	/* Get modem signals. */
    else
      x = ioctl(0,TIOCMGET,&y);
    debug(F101,"ttgmdm ioctl","",x);
    if (x < 0) {
	debug(F101,"ttgmdm errno","",errno);
	return(-1);
    } else debug(F101,"ttgmdm bits","",y);

    z = 0;				/* Initialize return value. */
#ifdef TIOCM_CTS
    /* Clear To Send */
    if (y & TIOCM_CTS) z |= BM_CTS;
#endif
#ifdef TIOCM_DSR
    /* Data Set Ready */
    if (y & TIOCM_DSR) z |= BM_DSR;
#endif
#ifdef TIOCM_CAR
    /* Carrier */
    if (y & TIOCM_CAR) z |= BM_DCD;
#endif
#ifdef TIOCM_RNG
    /* Ring Indicate */
    if (y & TIOCM_RNG) z |= BM_RNG;
#endif
#ifdef TIOCM_DTR
    /* Data Terminal Ready */
    if (y & TIOCM_DTR) z |= BM_DTR;
#endif
#ifdef TIOCM_RTS
    /* Request To Send */
    if (y & TIOCM_RTS) z |= BM_RTS;
#endif
    return(z);

#else /* !K_MDMCTL catch-All */

    if (netconn)			/* Network connection */
      return(-2);			/* No modem signals */

    return(-3);				/* Sorry, I don't know how... */

#endif /* K_MDMCTL */
#endif /* HPUX */
#endif /* QNX */
}

/*  P S U S P E N D  --  Put this process in the background.  */

/*
  Call with flag nonzero if suspending is allowed, zero if not allowed.
  Returns 0 on apparent success, -1 on failure (flag was zero, or
  kill() returned an error code.
*/
int
psuspend(flag) int flag; {

#ifdef RTU
    extern int rtu_bug;
#endif /* RTU */

    if (flag == 0) return(-1);

#ifdef NOJC
    return(-1);
#else
#ifdef SIGTSTP
/*
  The big question here is whether job control is *really* supported.
  There's no way Kermit can know for sure.  The fact that SIGTSTP is
  defined does not guarantee the Unix kernel supports it, and the fact
  that the Unix kernel supports it doesn't guarantee that the user's
  shell (or other process that invoked Kermit) supports it.
*/
#ifdef RTU
    rtu_bug = 1;
#endif /* RTU */
    if (kill(0,SIGSTOP) < 0
#ifdef MIPS
/* Let's try this for MIPS too. */
	&& kill(getpid(),SIGSTOP) < 0
#endif /* MIPS */
	) {				/* If job control, suspend the job */
	perror("suspend");
	debug(F101,"psuspend error","",errno);
	return(-1);
    }
    debug(F100,"psuspend ok","",0);
    return(0);
#else
    return(-1);
#endif /* SIGTSTP */
#endif /* NOJC */
}

/*
  setuid package, by Kristoffer Eriksson, with contributions from Dean
  Long and fdc.
*/

/* The following is for SCO when CK_ANSILIBS is defined... */
#ifdef M_UNIX
#ifdef CK_ANSILIBS
#ifndef NOGETID_PROTOS
#define NOGETID_PROTOS
#endif /* NOGETID_PROTOS */
#endif /* CK_ANSILIBS */
#endif /* M_UNIX */

#ifndef _POSIX_SOURCE
#ifndef SUNOS4
#ifndef NEXT
#ifndef PS2AIX10
#ifndef sequent
#ifndef NOGETID_PROTOS
extern UID_T getuid(), geteuid(), getreuid();
extern GID_T getgid(), getegid(), getregid();
#endif /* NOGETID_PROTOS */
#endif /* sequent */
#endif /* PS2AIX10 */
#endif /* NEXT */
#endif /* SUNOS4 */
#endif /* _POSIX_SOURCE */

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

/*
  The following code defines the symbol SETEUID for UNIX systems based
  on BSD4.4 (either -Encumbered or -Lite).  This program will then use
  seteuid() and setegid() instead of setuid() and setgid(), which still
  don't allow arbitrary switching.  It also avoids setreuid() and
  setregid(), which are included in BSD4.4 for compatibility only, are
  insecure, and print warnings to stderr under at least one system (NetBSD
  1.0).  Note that POSIX systems should still use setuid() and setgid();
  the seteuid() and setegid() functions are BSD4.4 extensions to the
  POSIX model.  Mike Long <mike.long@analog.com>, 8/94.
*/
#ifdef BSD44
#define SETEUID
#endif /* BSD44 */

/*
  The following construction automatically defines the symbol SETREUID for
  UNIX versions based on Berkeley Unix 4.2 and 4.3.  If this symbol is 
  defined, then this program will use getreuid() and getregid() calls in
  preference to getuid() and getgid(), which in Berkeley-based Unixes do
  not allow arbitrary switching back and forth of real & effective uid.
  This construction also allows -DSETREUID to be put on the cc command line
  for any system that has and wants to use setre[ug]id().  It also prevents
  automatic definition of SETREUID if -DNOSETREU is included on the cc 
  command line (or otherwise defined).
*/
#ifdef FT18				/* None of this for Fortune. */
#define NOSETREU
#endif /* FT18 */

#ifdef ANYBSD
#ifndef BSD29
#ifndef BSD41
#ifndef SETREUID
#ifndef NOSETREU
#ifndef SETEUID
#define SETREUID
#endif /* SETEUID */
#endif /* NOSETREU */                   
#endif /* SETREUID */
#endif /* !BSD41 */
#endif /* !BSD29 */
#endif /* ANYBSD */

/* Variables for user and group IDs. */

static UID_T realuid = (UID_T) -1, privuid = (UID_T) -1;
static GID_T realgid = (GID_T) -1, privgid = (GID_T) -1;


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
 * program should be aborted none-the-less.
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
    privuid = geteuid();
    privgid = getegid();

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
     * should be stopped.  Use some other uid.  "root" is probably too
     * privileged for such things, anyway. (The uid is reverted to the
     * real uid until termination.)
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
	if (setuid(realuid)) {
	    debug(F101,"setuid failed","",errno);
	    err |= 1;
	    debug(F101,"ruid","",getuid());
	    debug(F101,"euid","",geteuid());
	}
    debug(F101,"setuid","",realuid);
    if (privgid != (GID_T) -1)
        if (setgid(realgid)) {
	    debug(F101,"setgid failed","",errno);
	    err |= 2;
	    debug(F101,"rgid","",getgid());
	    debug(F101,"egid","",getegid());
	}
    debug(F101,"setgid","",realgid);
    return(err);
#else
    /* Easy way of using setuid()/setgid() instead of setreuid()/setregid().*/
    return(priv_off());
#endif /* SETEUID */
#endif /* SETREUID */
}

/* P R I V _ O P N  --  For opening protected files or devices. */

int
priv_opn(name, modes) char *name; int modes; {
    int x;
    priv_on();				/* Turn privileges on */
    x = open(name, modes);		/* Try to open the device */
    priv_off();				/* Turn privileges off */
    return(x);				/* Return open's return code */
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

UID_T
real_uid() {
    return(realuid);
}

VOID
ttimoff() {				/* Turn off any timer interrupts */
    /* int xx; */
/*
  As of 5A(183), we set SIGALRM to SIG_IGN (to ignore alarms) rather than to
  SIG_DFL (to catch alarms, or if there is no handler, to exit).  This is to
  cure (mask, really) a deeper problem with stray alarms that occurs on some
  systems, possibly having to do with sleep(), that caused core dumps.  It
  should be OK to do this, because no code in this module uses nested alarms.
  (But we still have to watch out for SCRIPT and DIAL...)
*/
    /* xx = */ alarm(0);
    /* debug(F101,"ttimoff alarm","",xx); */
    if (saval) {			/* Restore any previous */
	signal(SIGALRM,saval);		/* alarm handler. */
	/* debug(F101,"ttimoff alarm restoring saval","",saval); */
	saval = NULL;
    } else {
	signal(SIGALRM,SIG_IGN);	/* Used to be SIG_DFL */
	/* debug(F100,"ttimoff alarm SIG_IGN","",0); */
    }
}

#ifdef CK_REDIR
int
ttruncmd(s) char *s; {
    int r = 0;				/* Return code */
					/* 0 = failure, 1 = success */
#ifdef WIFEXITED
#ifndef WEXITSTATUS
#ifdef w_retcode
#define WEXITSTATUS(x) ((x).w_retcode)
#endif /* w_retcode */
#endif /* WEXITSTATUS */
#ifdef WEXITSTATUS
#define CK_FUN_OK
#endif /* WEXITSTATUS */
#endif /* WIFEXITED */
#ifdef NEXT
#define waitpid(pid,status,option) wait4((pid),(status),(option),NULL)
#else
#ifdef sony_news
#define waitpid(pid,status,option) wait4((pid),(status),(option),NULL)
#endif /* sony_news */
#endif /* NEXT */

#ifdef CK_FUN_OK
    PID_T pid;

    if (ttyfd == -1) {
	printf("?Sorry, device is not open\n");
	return(0);
    }
    conres();				/* Make console normal  */
    if ((pid = fork()) == 0) {		/* Make a child fork */
	/* give it stdin,out to the line */
	if (priv_can())			/* Turn off privs. */
	  exit(1);
	dup2(ttyfd, 0);
	dup2(ttyfd, 1);
	_exit(system(s) ? BAD_EXIT : 0);
    } else {
	WAIT_T status;			/* This is int for all but NeXT */
	SIGTYP (*istat)(), (*qstat)();

	if (pid == (PID_T) -1)		/* fork() failed? */
	  return(0);
	istat = signal(SIGINT,SIG_IGN); /* Let the fork handle keyboard */
	qstat = signal(SIGQUIT,SIG_IGN); /* interrupts itself... */

	if (waitpid(pid, &status, 0) != pid) {
	    printf("?Can't wait for child process?\r\n");
	} else {
	    if (WIFEXITED(status)) {
		if (WEXITSTATUS(status)) {
		    printf("?Command exit status: %d\r\n",
			   WEXITSTATUS(status));
		} else r = 1;
	    } else {
		printf("?Command did not exit - cancelling it now.\r\n");
		kill(pid, SIGKILL);
	    }
	}
	signal(SIGINT,istat);		/* Restore interrupts */
	signal(SIGQUIT,qstat);
    }
    concb((char)escchr);		/* Restore console to CBREAK mode */
#else
    printf("?Sorry, can't execute this command - pieces missing.\n");
#endif /* CK_FUN_OK */
    return(r);
}
#endif /* CK_REDIR */

#ifdef OXOS
#undef kill
#endif /* OXOS */

#ifdef OXOS
int
priv_kill(pid, sig) int pid, sig; {
    int	i;

    if (priv_on())
	debug(F100,"priv_kill priv_on failed","",0);
    i = kill(pid, sig);
    if (priv_off())
	debug(F100,"priv_kill priv_off failed","",0);
    return(i);
}
#endif /* OXOS */

#ifdef BEBOX
#ifdef BE_DR_7
/*
  alarm() function not supplied with Be OS DR7 - this one contributed by
  Neal P. Murphy.
*/

/*
  This should mimic the UNIX/POSIX alarm() function well enough, with the
  caveat that one's SIGALRM handler must call alarm_expired() to clean up vars
  and wait for the alarm thread to finish.
*/
unsigned int
alarm(unsigned int seconds) {
    long time_left = 0;

/* If an alarm is active, turn it off, saving the unused time */
    if (alarm_thread != -1) {
        /* We'll be generous and count partial seconds as whole seconds. */
        time_left = alarm_struct.time -
	  ((system_time() - time_started) / 1000000.0);

        /* Kill the alarm thread */
        kill_thread (alarm_thread);

        /* We need to clean up as though the alarm occured. */
        time_started = 0;
        alarm_struct.thread = -1;
        alarm_struct.time = 0;
        alarm_expired();
    }

/* Set a new alarm clock, if requested. */
    if (seconds > 0) {
        alarm_struct.thread = find_thread(NULL);
        alarm_struct.time = seconds;
        time_started = system_time();
        alarm_thread = spawn_thread (do_alarm,
                                     "alarm_thread",
                                     B_NORMAL_PRIORITY,
                                     (void *) &alarm_struct
				     );
        resume_thread (alarm_thread);
    }

/* Now return [unused time | 0] */
    return ((unsigned int) time_left);
}

/*
  This function is the departure from UNIX/POSIX alarm handling. In the case
  of Be's missing alarm() function, this stuff needs to be done in the SIGALRM
  handler. When Be implements alarm(), this function call can be eliminated
  from user's SIGALRM signal handlers.
*/

void
alarm_expired(void) {
    long ret_val;

    if (alarm_thread != -1) {
        wait_for_thread (alarm_thread, &ret_val);
        alarm_thread = -1;
    }
}

/*
  This is the function that snoozes the requisite number of seconds and then
  SIGALRMs the calling thread. Note that kill() wants a pid_t arg, whilst Be
  uses thread_id; currently they are both typdef'ed as long, but I'll do the
  cast anyway. This function is run in a separate thread.
*/

long
do_alarm (void *alarm_struct) {
    snooze ((double) ((struct ALARM_STRUCT *) alarm_struct)->time * 1000000.0);
    kill ((pid_t)((struct ALARM_STRUCT *) alarm_struct)->thread, SIGALRM);
    time_started = 0;
    ((struct ALARM_STRUCT *) alarm_struct)->thread = -1;
    ((struct ALARM_STRUCT *) alarm_struct)->time = 0;
}
#endif /* BE_DR_7 */
#endif /* BEBOX */

#ifdef Plan9

int
p9ttyctl(char letter, int num, int param) {
    char cmd[20];
    int len;

    if (ttyctlfd < 0)
      return -1;

    cmd[0] = letter;
    if (num)
      len = sprintf(cmd + 1, "%d", param) + 1;
    else {
	cmd[1] = param;
	len = 2;
    }
    if (write(ttyctlfd, cmd, len) == len) {
	cmd[len] = 0;
	/* fprintf(stdout, "wrote '%s'\n", cmd); */
	return 0;
    }
    return -1;
}

int
p9ttyparity(char l) {
    return p9ttyctl('p', 0, l);
}

int
p9tthflow(int flow, int status) {
    return p9ttyctl('m', 1, status);
}

int
p9ttsspd(int cps) {
    if (p9ttyctl('b', 1, cps * 10) < 0)
      return -1;
    ttylastspeed = cps * 10;
    return 0;
}

int
p9openttyctl(char *ttname) {
    char name[100];

    if (ttyctlfd >= 0) {
	close(ttyctlfd);
	ttyctlfd = -1;
	ttylastspeed = -1;
    }
    sprintf(name, "%sctl", ttname);
    ttyctlfd = open(name, 1);
    return ttyctlfd;
}

int
p9concb() {
    if (consctlfd >= 0) {
	if (write(consctlfd, "rawon", 5) == 5)
	  return 0;
    }
    return -1;
}

int
p9conbin() {
    return p9concb();
}

int
p9conres() {
    if (consctlfd >= 0) {
	if (write(consctlfd, "rawoff", 6) == 6)
	  return 0;
    }
    return -1;
}

int
p9sndbrk(int msec) {
    if (ttyctlfd >= 0) {
	char cmd[20];
	int i = sprintf(cmd, "k%d", msec);
	if (write(ttyctlfd, cmd, i) == i)
	  return 0;
    }
    return -1;
}

int
conwrite(char *buf, int n) {
    int x;
    static int length = 0;
    static int holdingcr = 0;
    int normal = 0;
    for (x = 0; x < n; x++) {
	char c = buf[x];
	if (c == 007) {
	    if (normal) {
		write(1, buf + (x - normal), normal);
		length += normal;
		normal = 0;
	    }
	    /* write(noisefd, "1000 300", 8); */
	    holdingcr = 0;
	} else if (c == '\r') {
	    if (normal) {
		write(1, buf + (x - normal), normal);
		length += normal;
		normal = 0;
	    }
	    holdingcr = 1;
	} else if (c == '\n') {
	    write(1, buf + (x - normal), normal + 1);
	    normal = 0;
	    length = 0;
	    holdingcr = 0;
	} else if (c == '\b') {
	    if (normal) {
		write(1, buf + (x - normal), normal);
		length += normal;
		normal = 0;
	    }
	    if (length) {
		write(1, &c, 1);
		length--;
	    }
	    holdingcr = 0;
	} else {
	    if (holdingcr) {
		char b = '\b';
		while (length-- > 0)
		  write(1, &b, 1);
		length = 0;	/* compiler bug */
	    }
	    holdingcr = 0;
	    normal++;
	}
    }
    if (normal) {
	write(1, buf + (x - normal), normal);
	length += normal;
    }
    return n;
}

void
conprint(char *fmt, ...) {
    char buf[1000];

    va_list ap;
    int i;

    va_start(ap, fmt);
    i = vsprintf(buf, fmt, ap);
    conwrite(buf, i);
}
#endif /* Plan9 */
