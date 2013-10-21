/*
 * Copyright 1989 - 1994, Julianne Frances Haugh
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Julianne F. Haugh nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JULIE HAUGH AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL JULIE HAUGH OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef _WIN32
#include <windows.h>            /* For WNetGetUser */
#else
#include <config.h>
#endif

#ifndef OS2
#ifndef _WIN32
#include "prototypes.h"
#include "defines.h"
#endif
#endif

#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef OS2
#ifndef _WIN32
#include <io.h>
#endif
#endif
#include <fcntl.h>
#include <signal.h>

#ifndef _WIN32
#ifndef OS2
#include <pwd.h>
#include "pwauth.h"
#include "pwio.h"
#include "getdef.h"
#else /* OS2 */
#define bzero(x,y) memset(x,0,y)
#define tpw_lock() 1
#define tpw_unlock() 1
#define SYSLOG(x)
struct passwd {
        char    *pw_name;
        char    *pw_passwd;
        int     pw_uid;
        int     pw_gid;
        char    *pw_age;
        char    *pw_comment;
        char    *pw_gecos;
        char    *pw_dir;
        char    *pw_shell;
};
#endif /* OS2 */
#else /* _WIN32 */
#define bzero(x,y) memset(x,0,y)
#define tpw_lock() 1
#define tpw_unlock() 1
#define SYSLOG(x)
struct passwd {
        char    *pw_name;
        char    *pw_passwd;
        int     pw_uid;
        int     pw_gid;
        char    *pw_age;
        char    *pw_comment;
        char    *pw_gecos;
        char    *pw_dir;
        char    *pw_shell;
};
#endif /* _WIN32 */

/* EPS STUFF */

#include "t_pwd.h"
static int do_update_eps = 0;
struct t_pw eps_passwd;

/*
 * Global variables
 */

static char *name;      /* The name of user whose password is being changed */
static char myname[64]; /* The current user's name */
static char *Prog;              /* Program name */
static int amroot;              /* The real UID was 0 */

static int
        aflg = 0,               /* -a - add user */
        dflg = 0,               /* -d - delete password */
        qflg = 0;               /* -q - quiet mode */

/*
 * set to 1 if there are any flags which require root privileges,
 * and require username to be specified
 */
static int anyflag = 0;

#ifdef AGING
static long age_min = 0;        /* Minimum days before change   */
static long age_max = 0;        /* Maximum days until change     */
#endif

static int do_update_age = 0;
static char crypt_passwd[128];  /* The "old-style" password, if present */

/*
 * External identifiers
 */

extern char *crypt_make_salt();
#if !defined(__GLIBC__)
extern char *l64a();
#endif

extern  int     optind;         /* Index into argv[] for current option */
extern  char    *optarg;        /* Pointer to current option value */

#ifdef  NDBM
extern  int     sp_dbm_mode;
extern  int     pw_dbm_mode;
#endif

/*
 * #defines for messages.  This facilities foreign language conversion
 * since all messages are defined right here.
 */

#define USAGE \
        "usage: %s [ -a | -d | -q  ] name\n"
#define ADMUSAGE \
        "       %s [ -x max ] [ -n min ] [ -w warn ] [ -i inact ] name\n"
#define ADMUSAGE2 \
        "       %s { -l | -u | -d | -S | -e } name\n"
#define OLDPASS "Old password:"
#define ROOTPASS "Enter 'root' password:"
#define NEWPASSMSG \
"Enter the new password (minimum of %d, maximum of %d characters)\n\
Please use a combination of upper and lower case letters and numbers.\n"
#define CHANGING "Changing password for %s\n"
#define ADDING "Adding user %s\n"
#define NEWPASS "New password:"
#define NEWPASS2 "Re-enter new password:"
#define WRONGPWD "Incorrect password for %s.\n"
#define WRONGPWD2 "incorrect password for `%s'"
#define NOMATCH "They don't match; try again.\n"
#define CANTCHANGE "The password for %s cannot be changed.\n"
#define CANTCHANGE2 "password locked for `%s'"

#define BADPASS "Bad password:  %s.  "
#define EPSFAIL "Unable to update EPS password.\n"
#define NOEPSCONF "Warning: configuration file missing; please run 'tconf'\n"

#define TOOSOON "Sorry, the password for %s cannot be changed yet.\n"
#define TOOSOON2 "now < minimum age for `%s'"

#define EXECFAILED "%s: Cannot execute %s"
#define EXECFAILED2 "cannot execute %s"
#define WHOAREYOU "%s: Cannot determine your user name.\n"
#define UNKUSER "%s: Unknown user %s\n"
#define NOPERM "You may not change the password for %s.\n"
#define NOPERM2 "can't change pwd for `%s'"
#define UNCHANGED "The password for %s is unchanged.\n"

#define PWDBUSY "Cannot lock the password file; try again later.\n"
#define OPNERROR "Cannot open the password file.\n"
#define UPDERROR "Error updating the password entry.\n"
#define CLSERROR "Cannot commit password file changes.\n"
#define DBMERROR "Error updating the DBM password entry.\n"

#define PWDBUSY2 "can't lock password file"
#define OPNERROR2 "can't open password file"
#define UPDERROR2 "error updating password entry"
#define CLSERROR2 "can't rewrite password file"
#define DBMERROR2 "error updaring dbm password entry"

#define NOTROOT "Cannot change ID to root.\n"
#define NOTROOT2 "can't setuid(0)"
#define TRYAGAIN "Try again.\n"
#define PASSWARN \
        "\nWarning: weak password (enter it again to use it anyway).\n"
#define CHANGED "Password changed.\n"
#define CHGPASSWD "password for `%s' changed by user `%s'"
#define NOCHGPASSWD "did not change password for `%s'"
#define DELETING "Deleting password for '%s'.\n"
#define DELETED "Password for '%s' deleted.\n"
#define ADDED   "Password set for user '%s'\n"
#define NOTADDED "Unable to add user '%s'\n"

/*
 * usage - print command usage and exit
 */

static void
usage(status)
    int status;
{
    fprintf(stderr, USAGE, Prog);
    if (amroot) {
        fprintf(stderr, ADMUSAGE, Prog);
        fprintf(stderr, ADMUSAGE2, Prog);
    }
    exit(status);
}

/*
 * new_password - validate old password and replace with new
 * (both old and new in global "char crypt_passwd[128]")
 */

/*ARGSUSED*/
static int
new_password()
{
    char        clear[128];     /* Pointer to clear text */
    char        *cipher;        /* Pointer to cipher text */
    char        *cp;            /* Pointer to getpass() response */
    char        orig[128];      /* Original password */
    char        pass[128];      /* New password */
    int i;              /* Counter for retries */
    int warned;
    int pass_max_len=127;
    int rc = 0;

    /*
     * Authenticate the user.  The user will be prompted for their
     * own password.
     */

    bzero(clear, 128);
    bzero(orig, 128);
    bzero(pass, 128);

    if (!amroot) {

        /* EPS STUFF */

        int retval;

        cipher = NULL;

        t_getpass (clear, 128, OLDPASS);
        if ((retval = t_verifypw (name, clear)) > -1)
        {
            endtpent();
            retval = (retval == 0) ? 1 : 0;
        }
        else
        {
            if (strlen (clear) > pass_max_len)
            {
                memset(clear+pass_max_len, 0, strlen (clear+pass_max_len));
                clear[pass_max_len] = '\0';
            }
        }

        if (retval != 0)
        {
            SYSLOG((LOG_WARN, WRONGPWD2, name));
            fprintf(stderr, WRONGPWD, name);
            return -1;
        }
        else
        {
            strcpy(orig, clear);
            bzero(clear, strlen (clear));
            if (cipher) bzero(cipher, strlen (cipher));
        }
    } else {
        orig[0] = '\0';
    }

    /*
     * Get the new password.  The user is prompted for the new password
     * and has five tries to get it right.  The password will be tested
     * for strength, unless it is the root user.  This provides an escape
     * for initial login passwords.
     */

    if (!qflg)
        printf(NEWPASSMSG, 5, 127);

    warned = 0;
    for (i = 5; i > 0; i--) {
        t_getpass (clear, 128, NEWPASS);
        cp = clear;
        if (!cp) {
            bzero (orig, sizeof orig);
            return -1;
        }
        strcpy(pass, cp);
        bzero(cp, strlen(cp));

        t_getpass (clear, 128, NEWPASS2);
        cp = clear;
        if (!cp) {
            bzero (orig, sizeof orig);
            bzero (clear, sizeof clear);
            bzero (pass, sizeof pass);
            return -1;
        }
        if (strcmp (cp, pass)) {
            bzero (clear, sizeof clear);
            bzero (pass, sizeof pass);
            fprintf (stderr, NOMATCH);
        } else
            break;
    }
    bzero (orig, sizeof orig);

    if (i == 0) {
        bzero (clear, sizeof clear);
        bzero (pass, sizeof pass);
        return -1;
    }

    /*
     * Encrypt the password, then wipe the cleartext password.
     */

    /* EPS STUFF */
    {
        struct t_conf *tc;
        struct t_confent *tcent;

        if ((tc = t_openconf(NULL)) == NULL ||
             (tcent = t_getconflast(tc)) == NULL)
        {
            fprintf(stderr, NOEPSCONF);
            do_update_eps = 0;
            rc = -1;
        }
        else
        {
            do_update_eps = 1;
            if (t_makepwent (&eps_passwd, name, pass, NULL, tcent) == NULL)
                rc = -1;
        }

        if (tc) t_closeconf (tc);
        pass[pass_max_len] = '\0';
    }
    return rc;
}

/*
 * check_password - test a password to see if it can be changed
 *
 *      check_password() sees if the invoker has permission to change the
 *      password for the given user.
 */

/*ARGSUSED*/
static char *
date_to_str(time_t t)
{
        static char buf[80];
        struct tm *tm;

        tm = gmtime(&t);
        sprintf(buf, "%02d/%02d/%02d",
                tm->tm_mon + 1, tm->tm_mday, tm->tm_year % 100);
        return buf;
}

static const char *
pw_status(pass)
        const char *pass;
{
        if (*pass == '*' || *pass == '!')
                return "L";
        if (*pass == '\0')
                return "NP";
        return "P";
}

/*
 * print_status - print current password status
 */

static void
print_status(pw)
        const struct passwd *pw;
{
    printf("%s %s\n", pw->pw_name, pw_status(pw->pw_passwd));
}


static void
fail_exit(status)
        int status;
{
        tpw_unlock();
        exit(status);
}

static void
oom()
{
        fprintf(stderr, "%s: out of memory\n", Prog);
        fail_exit(3);
}

static long
getnumber(str)
        const char *str;
{
        long val;
        char *cp;

        val = strtol(str, &cp, 10);
        if (*cp)
                usage(6);
        return val;
}

/*
 * passwd - change a user's password file information
 *
 *      This command controls the password file and commands which are
 *      used to modify it.
 *
 *      The valid options are
 *
 *      -l      lock the named account (*)
 *      -u      unlock the named account (*)
 *      -d      delete the password for the named account (*)
 *      -e      expire the password for the named account (*)
 *      -x #    set sp_max to # days (*)
 *      -n #    set sp_min to # days (*)
 *      -w #    set sp_warn to # days (*)
 *      -i #    set sp_inact to # days (*)
 *      -S      show password status of named account
 *      -g      execute gpasswd command to interpret flags
 *      -f      execute chfn command to interpret flags
 *      -s      execute chsh command to interpret flags
 *      -k      change password only if expired
 *
 *      (*) requires root permission to execute.
 *
 *      All of the time fields are entered in days and converted to the
 *      appropriate internal format.  For finer resolute the chage
 *      command must be used.
 *
 *      Exit status:
 *      0 - success
 *      1 - permission denied
 *      2 - invalid combination of options
 *      3 - unexpected failure, password file unchanged
 *      5 - password file busy, try again later
 *      6 - invalid argument to option
 */

int
main(argc, argv)
    int argc;
    char **argv;
{
    char    *cp;                    /* Miscellaneous character pointing  */
    int     flag;                   /* Current option to process     */
    unsigned long mynamelen;
    struct t_pwent pwent;
    struct t_pw *  pw;

        /*
         * The program behaves differently when executed by root
         * than when executed by a normal user.
         */

#ifdef WIN32
        amroot = 0;
#else
#ifdef OS2
        amroot = 0;
#else
        amroot = (getuid () == 0);
#endif
#endif

        /*
         * Get the program name.  The program name is used as a
         * prefix to most error messages.
         */

        Prog = argv[0];

        /*
         * The remaining arguments will be processed one by one and
         * executed by this command.  The name is the last argument
         * if it does not begin with a "-", otherwise the name is
         * determined from the environment and must agree with the
         * real UID.  Also, the UID will be checked for any commands
         * which are restricted to root only.
         */

#define FLAGS "adq"
        while ((flag = getopt(argc, argv, FLAGS)) != EOF) {
#undef FLAGS
                switch (flag) {
                case 'q':
                        qflg++;  /* ok for users */
                        break;
                case 'd':
                        dflg++;
                        anyflag = 1;
                        break;
                case 'a':
                        aflg++;
                        anyflag = 1;
                        break;
                default:
                        usage(6);
                }
        }


    if ( !ck_crypto_loaddll() ) {
        fprintf(stderr, "%s: unable to load crypto dll\n",Prog);
        exit(1);
    }

    ck_OPENSSL_add_all_algorithms_noconf();

        /*
         * Now I have to get the user name.  The name will be gotten
         * from the command line if possible.  Otherwise it is figured
         * out from the environment.
         */
        mynamelen = sizeof(myname);
#ifdef WIN32
        WNetGetUser( NULL, myname, &mynamelen);
#endif /* WIN32 */
#ifdef OS2
       {
           char * env = getenv("USER");
           if (env) {
               strncpy(myname,env,64);
               myname[63] = '\0';
           }
       }
#endif /* OS2 */
        if (optind < argc)
                name = argv[optind];
        else
                name = myname;


    if (anyflag && optind >= argc)
        usage(2);

#ifdef OS2
#define AMROOT
#else
#ifdef WIN32
#define AMROOT
#endif
#endif
#ifdef AMROOT
    pw= t_openpwbyname((char *)t_defaultpwd());
    if ( !pw ) {
        amroot = 1;
        if ( !aflg || strcmp("root",name) ) {
            fprintf(stderr,"Password file does not exist!!!!\n");
            fprintf(stderr,"The first entry in a password file must be 'root'\n");
            fprintf(stderr,"Execute '%s -a root'\n",Prog);
            exit(1);
        }
    } else if (anyflag) {
        char clear[128];
        int rc;

        t_closepw(pw);

        t_getpass (clear, 128, ROOTPASS);
        rc = t_verifypw ("root", clear);
        if (rc > -1)
            endtpent();
        if ( rc == 1 )
            amroot = 1;
    } else
        t_closepw(pw);
#endif

    if (anyflag && !amroot) {
        fprintf(stderr, "%s: Permission denied\n", Prog);
        exit(1);
    }

    if ( dflg ) {

        if ( !qflg )
            printf(DELETING, name);

        pwent.name = name;
        pwent.password.len = 0;
        pwent.password.data = NULL;


        /* delete the password */
        if (t_deletepw (NULL, name) < 0) {
            fprintf (stderr, EPSFAIL);
            exit(1);
        }

        if ( !qflg )
            printf(DELETED, name);

        return(0);
    }

    if ( aflg ) {
        pw= t_openpwbyname((char *)t_defaultpwd());

        if ( !qflg )
            printf(ADDING, name);

        if (pw && t_getpwbyname(pw,name)) {
            t_closepw(pw);
            fprintf(stderr,"user '%s' already exists in password file\n",name);
            exit(1);
        }
        if ( pw )
            t_closepw(pw);

        pwent.name = name;
        pwent.password.len = 0;
        pwent.password.data = NULL;

        if (new_password()) {
            fprintf(stderr, NOTADDED, name);
            exit(1);
        }

        if ( !qflg )
            printf(ADDED, name);

    }
    /*
     * If there are no other flags, just change the password.
     */

    if (!anyflag) {
        crypt_passwd[0] = '\0';

        /*
         * See if the user is permitted to change the password.
         * Otherwise, go ahead and set a new password.
         */

        /*
         * Let the user know whose password is being changed.
         */
        if (!qflg)
            printf(CHANGING, name);

#ifdef OS2
        amroot = 0;
#else
#ifdef NT
        amroot = 0;
#endif
#endif

        if (new_password()) {
            fprintf(stderr, UNCHANGED, name);
            exit(1);
        }
    }

/* EPS STUFF */

    if (do_update_eps)
    {
        /* try and see if the file is there, else create it */

        if ((pw = t_openpwbyname((char *)t_defaultpwd())) == NULL) {
            int fh = creat ((char *)t_defaultpwd(), 0600);
            if (fh == -1) {
                fprintf(stderr,"Unable to open password file\n");
                fail_exit(5);
            }
            close(fh);
        } else
             t_closepw(pw);

        /* change the password */
        if (t_changepw ((char *)t_defaultpwd(), &(eps_passwd.pebuf)) < 0) {
            fprintf (stderr, EPSFAIL);
            exit(1);
        }
    }
    else {
        fprintf (stderr, EPSFAIL);
        exit(1);
    }

    if (!qflg)
        printf(CHANGED);

    return(0);
}
