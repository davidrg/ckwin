/*
 * Steven Schultz - sms@moe.2bsd.com
 *
 *	@(#)ctimed.c	1.0 (2.11BSD) 1996/6/25
 *
 * ctimed - the daemon that supports the ctime() and getpw*() stubs
 * 	    in 'libcstubs.a'.
*/

#include	<signal.h>
#include	<stdio.h>
#include	<setjmp.h>
#include	<sys/ioctl.h>
#include	<sys/types.h>
#include	<sys/time.h>
#include	<pwd.h>
#include	<utmp.h>

/*
 * These should probably be placed in an include file.  If you add anything
 * here then you will also have to modify /usr/src/usr.lib/libstubs/stubs.c
 * (if for no other reason than to add the stub code).
*/

#define	CTIME	1
#define	ASCTIME	2
#define	TZSET	3
#define	LOCALTIME 4
#define	GMTIME	5
#define	OFFTIME	6

#define	GETPWENT	7
#define	GETPWNAM	8
#define	GETPWUID	9
#define	SETPASSENT	10
#define	ENDPWENT	11

extern	struct	tm	*offtime();

	jmp_buf	env;
	char	*cp;
	char	junk[256 + sizeof (struct passwd) + 4];
	long	off;
	time_t	l;
	void	timeout(), checkppid();
	struct	tm	tmtmp, *tp;

main()
	{
	register int i;
	register struct passwd *pw;
	struct	itimerval it;
	u_char	c, xxx;
	int	len, tosslen;
	uid_t	uid;

	signal(SIGPIPE, SIG_DFL);
	for	(i = getdtablesize(); --i > 2; )
		close(i);
/*
 * Need a timer running while we disassociate from the control terminal
 * in case of a modem line which has lost carrier.
*/
	timerclear(&it.it_interval);
	it.it_value.tv_sec = 5;
	it.it_value.tv_usec = 0;
	signal(SIGALRM, timeout);
	setitimer(ITIMER_REAL, &it, (struct itimerval *) NULL);
	if	(setjmp(env) == 0)
		{
		i = open("/dev/tty", 0);
		if	(i >= 0)
			{
			ioctl(i, TIOCNOTTY, NULL);
			close(i);
			}
		}
/*
 * Now start a timer with one minute refresh.  In the signal service
 * routine, check the parent process id to see if this process has
 * been orphaned and if so exit.  This is primarily aimed at removing
 * the 'ctimed' process left behind by 'sendmail's multi-fork startup
 * but may prove useful in preventing accumulation of 'ctimed' processes
 * in other circumstances as well.  Normally this process is short
 * lived.
*/
	it.it_interval.tv_sec = 60;
	it.it_interval.tv_usec = 0;
	it.it_value.tv_sec = 60;
	it.it_value.tv_usec = 0;
	signal(SIGALRM, checkppid);
	setitimer(ITIMER_REAL, &it, (struct itimerval *) NULL);

	while	(read(fileno(stdin), &c, 1) == 1)
		{
		switch	(c)
			{
			case	CTIME:
				l = 0L;
				getb(fileno(stdin), &l, sizeof l);
				cp = ctime(&l);
				write(fileno(stdout), cp, 26);
				break;
			case	ASCTIME:
				getb(fileno(stdin), &tmtmp, sizeof tmtmp);
				cp = asctime(&tmtmp);
				write(fileno(stdout), cp, 26);
				break;
			case	TZSET:
				(void) tzset();
				break;
			case	LOCALTIME:
				l = 0L;
				getb(fileno(stdin), &l, sizeof l);
				tp = localtime(&l);
				write(fileno(stdout), tp, sizeof (*tp));
				strcpy(junk, tp->tm_zone);
				junk[24] = '\0';
				write(fileno(stdout), junk, 24);
				break;
			case	GMTIME:
				l = 0L;
				getb(fileno(stdin), &l, sizeof l);
				tp = gmtime(&l);
				write(fileno(stdout), tp, sizeof (*tp));
				strcpy(junk, tp->tm_zone);
				junk[24] = '\0';
				write(fileno(stdout), junk, 24);
				break;
			case	OFFTIME:
				getb(fileno(stdin), &l, sizeof l);
				getb(fileno(stdin), &off, sizeof off);
#ifdef	__bsdi__
				l += off;
				tp = localtime(&l);
#else
				tp = offtime(&l, off);
#endif
				write(fileno(stdout), tp, sizeof (*tp));
				break;
			case	GETPWENT:
				pw = getpwent();
				do_pw(pw);
				break;
			case	GETPWNAM:
				getb(fileno(stdin), &len, sizeof (int));
				if	(len > UT_NAMESIZE)
					{
					tosslen = len - UT_NAMESIZE;
					len = UT_NAMESIZE;
					}
				else
					tosslen = 0;
				getb(fileno(stdin), junk, len);
				for	(;tosslen; tosslen--)
					getb(fileno(stdin), &xxx, 1);
				junk[len] = '\0';
				pw = getpwnam(junk);
				do_pw(pw);
				break;
			case	GETPWUID:
				getb(fileno(stdin), &uid, sizeof (uid_t));
				pw = getpwuid(uid);
				do_pw(pw);
				break;
			case	SETPASSENT:
				getb(fileno(stdin), &len, sizeof (int));
				if	(setpassent(len))
					len = 1;
				else
					len = 0;
				write(fileno(stdout), &len, sizeof (int));
				break;
			case	ENDPWENT:
				endpwent();
				break;
			default:
				abort("switch");
			}
		}
	}

getb(f, p, n)
	int	f;
	register char	*p;
	register int	n;
	{
	register int	i;

	while	(n)
		{
		i = read(f, p, n);
		if	(i <= 0)
			return;
		p += i;
		n -= i;
		}
	}

void
timeout()
	{

	longjmp(env, 1);
	}

void
checkppid()
	{

	if	(getppid() == 1)
		exit(0);
	}

do_pw(pw)
	struct passwd *pw;
	{
	int	len;

	if	(!pw)
		{
		len = 0;
		write(fileno(stdout), &len, sizeof (int));
		return;
		}
	len = packpwtobuf(pw, junk);
	write(fileno(stdout), &len, sizeof (int));
	write(fileno(stdout), pw, sizeof (*pw));
	write(fileno(stdout), junk, len);
	return;
	}

packpwtobuf(pw, buf)
	register struct passwd *pw;
	char	*buf;
	{
	register char *cp = buf;
	register char *dp;

	dp = pw->pw_name;
	pw->pw_name = (char*) 0;
	while	(*cp++ = *dp++)
		;
	dp = pw->pw_passwd;
	pw->pw_passwd = (char*) (cp - buf);
	while	(*cp++ = *dp++)
		;
	dp = pw->pw_class;
	pw->pw_class = (char*) (cp - buf);
	while	(*cp++ = *dp++)
		;
	dp = pw->pw_gecos;
	pw->pw_gecos = (char*) (cp - buf);
	while	(*cp++ = *dp++)
		;
	dp = pw->pw_dir;
	pw->pw_dir = (char*) (cp - buf);
	while	(*cp++ = *dp++)
		;
	dp = pw->pw_shell;
	pw->pw_shell = (char*) (cp - buf);
	while	(*cp++ = *dp++)
		;
	return(cp - buf);
	}
