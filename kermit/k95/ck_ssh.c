char *cksshv = "SSH support, 8.0.001, 13 Sep 2000";
/*
  C K _ S S H . C --  OpenSSL Interface for C-Kermit

  Copyright (C) 1985, 2001,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

  Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
    All rights reserved

  Includes work by Niels Provos <provos@citi.umich.edu> to work with OpenSSL.

*/

#include "ckcdeb.h"
#ifdef SSH
#undef SSH
#endif /* SSH */

#ifdef CK_SSL
#ifdef SSH

#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <openssl/rand.h>
#include <openssl/x509_vfy.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/dsa.h>
#include <openssl/rsa.h>

#ifdef OS2
#include "ckossl.h"
#endif /* OS2 */

static char *client_version_string = NULL;
static char *server_version_string = NULL;


/* Flag indicating whether IPv4 or IPv6.
   This should be a SET TCP ... command.  Need to look at supporting
   IPv6 in ckcnet.c.   AF_UNSPEC means both IPv4 and IPv6. */
static int IPv4or6 = AF_INET;

/* Flag indicating whether debug mode is on.
   SET SSH DEBUG {ON, OFF }
*/
static int debug_flag = 0;

/* Flag indicating whether a tty should be allocated */
/* In other words, was Kermit started from a tty?    */
static int tty_flag = 0;

/* don't exec a shell */
static int no_shell_flag = 0;
static int no_tty_flag = 0;

/*
 * Flag indicating that nothing should be read from stdin.
 * Not sure we should support this in Kermit
 */
static int stdin_null_flag = 0;

/*
 * Flag indicating that ssh should fork after authentication.  This is useful
 * so that the pasphrase can be entered manually, and then ssh goes to the
 * background.  We will not support this in Kermit.
 */
static int fork_after_authentication_flag = 0;

/*
 * General data structure for command line options and options configurable
 * in configuration files.
 */

#define SSH_MAX_IDENTITY_FILES            100
#define SSH_MAX_FORWARDS_PER_DIRECTION    100

/* Data structure for representing a forwarding request. */
typedef struct {
        u_short   port;         /* Port to forward. */
        char     *host;         /* Host to connect. */
        u_short   host_port;    /* Port to connect on host. */
}       Forward;

/* Data structure for representing option data. */
typedef enum {
        SYSLOG_LEVEL_QUIET,
        SYSLOG_LEVEL_FATAL,
        SYSLOG_LEVEL_ERROR,
        SYSLOG_LEVEL_INFO,
        SYSLOG_LEVEL_VERBOSE,
        SYSLOG_LEVEL_DEBUG
}       LogLevel;

typedef struct {
        int     forward_agent;  /* Forward authentication agent. */
        int     forward_x11;    /* Forward X11 display. */
        char   *xauth_location; /* Location for xauth program */
        int     gateway_ports;  /* Allow remote connects to forwarded ports. */
        int     use_privileged_port;    /* Don't use privileged port if false. *
        int     rhosts_authentication;  /* Try rhosts authentication. */
        int     rhosts_rsa_authentication;      /* Try rhosts with RSA
                                                 * authentication. */
        int     rsa_authentication;     /* Try RSA authentication. */
        int     dsa_authentication;     /* Try DSA authentication. */
        int     skey_authentication;    /* Try S/Key or TIS authentication. */
#ifdef KRB4
        int     kerberos_authentication;        /* Try Kerberos
                                                 * authentication. */
#endif
#ifdef AFS
        int     kerberos_tgt_passing;   /* Try Kerberos tgt passing. */
        int     afs_token_passing;      /* Try AFS token passing. */
#endif
        int     password_authentication;        /* Try password
                                                 * authentication. */
        int     fallback_to_rsh;/* Use rsh if cannot connect with ssh. */
        int     use_rsh;        /* Always use rsh (don\'t try ssh). */
        int     batch_mode;     /* Batch mode: do not ask for passwords. */
        int     check_host_ip;  /* Also keep track of keys for IP address */
        int     strict_host_key_checking;       /* Strict host key checking. */
        int     compression;    /* Compress packets in both directions. */
        int     compression_level;      /* Compression level 1 (fast) to 9
                                         * (best). */
        int     keepalives;     /* Set SO_KEEPALIVE. */
        LogLevel log_level;     /* Level for logging. */

        int     port;           /* Port to connect. */
        int     connection_attempts;    /* Max attempts (seconds) before
                                         * giving up */
        int     number_of_password_prompts;     /* Max number of password
                                                 * prompts. */
        int     cipher;         /* Cipher to use. */
        char   *ciphers;        /* SSH2 ciphers in order of preference. */
        int     protocol;       /* Protocol in order of preference. */
        char   *hostname;       /* Real host to connect. */
        char   *proxy_command;  /* Proxy command for connecting the host. */
        char   *user;           /* User to log in as. */
        int     escape_char;    /* Escape character; -2 = none */

        char   *system_hostfile;/* Path for /etc/ssh_known_hosts. */
        char   *user_hostfile;  /* Path for $HOME/.ssh/known_hosts. */
        char   *system_hostfile2;        char   *user_hostfile2;

        int     num_identity_files;     /* Number of files for RSA identities. *
        int     num_identity_files2;    /* DSA identities. */
        char   *identity_files[SSH_MAX_IDENTITY_FILES];
        char   *identity_files2[SSH_MAX_IDENTITY_FILES];
    
        /* Local TCP/IP forward requests. */
        int     num_local_forwards;
        Forward local_forwards[SSH_MAX_FORWARDS_PER_DIRECTION];
    
        /* Remote TCP/IP forward requests. */
        int     num_remote_forwards;
        Forward remote_forwards[SSH_MAX_FORWARDS_PER_DIRECTION];
} Options;

static Options options;

/*
 * Name of the host we are connecting to.  This is the name given on the
 * command line, or the HostName specified for the user-supplied name in a
 * configuration file.
 */
static char *host;

/* socket address the host resolves to */
static struct sockaddr_storage hostaddr;

/*
 * Flag to indicate that we have received a window change signal which has
 * not yet been processed.  This will cause a message indicating the new
 * window size to be sent to the server a little later.  This is volatile
 * because this is updated in a signal handler.
 */
static volatile int received_window_change_signal = 0;

/* Value of argv[0] (set in the main program). */
static char *av0;

/* Flag indicating whether we have a valid host private key loaded. */
static int host_private_key_loaded = 0;

/* Host private key. */
static RSA *host_private_key = NULL;

/* Original real UID. */
static uid_t original_real_uid;

/* command to be executed */
static Buffer command;

/* SSH command line usage.  Use this to design the Kermit "ssh" personality. */

/*
	fprintf(stderr, "Usage: %s [options] host [command]\n", av0);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -l user     Log in using this user name.\n");
	fprintf(stderr, "  -n          Redirect input from /dev/null.\n");
	fprintf(stderr, "  -A          Enable authentication agent forwarding.\n");
	fprintf(stderr, "  -a          Disable authentication agent forwarding.\n");
#ifdef AFS
	fprintf(stderr, "  -k          Disable Kerberos ticket and AFS token forwarding.\n");
#endif				/* AFS */
        fprintf(stderr, "  -X          Enable X11 connection forwarding.\n");
	fprintf(stderr, "  -x          Disable X11 connection forwarding.\n");
	fprintf(stderr, "  -i file     Identity for RSA authentication (default: ~/.ssh/identity).\n");
	fprintf(stderr, "  -t          Tty; allocate a tty even if command is given.\n");
	fprintf(stderr, "  -T          Do not allocate a tty.\n");
	fprintf(stderr, "  -v          Verbose; display verbose debugging messages.\n");
	fprintf(stderr, "  -V          Display version number only.\n");
	fprintf(stderr, "  -P          Don't allocate a privileged port.\n");
	fprintf(stderr, "  -q          Quiet; don't display any warning messages.\n");
	fprintf(stderr, "  -f          Fork into background after authentication.\n");
	fprintf(stderr, "  -e char     Set escape character; ``none'' = disable (default: ~).\n");

	fprintf(stderr, "  -c cipher   Select encryption algorithm: "
			"``3des'', "
			"``blowfish''\n");
	fprintf(stderr, "  -p port     Connect to this port.  Server must be on the same port.\n");
	fprintf(stderr, "  -L listen-port:host:port   Forward local port to remote address\n");
	fprintf(stderr, "  -R listen-port:host:port   Forward remote port to local address\n");
	fprintf(stderr, "              These cause %s to listen for connections on a port, and\n", av0);
	fprintf(stderr, "              forward them to the other side by connecting to host:port.\n");
	fprintf(stderr, "  -C          Enable compression.\n");
	fprintf(stderr, "  -N          Do not execute a shell or command.\n");
	fprintf(stderr, "  -g          Allow remote hosts to connect to forwarded ports.\n");
	fprintf(stderr, "  -4          Use IPv4 only.\n");
	fprintf(stderr, "  -6          Use IPv6 only.\n");
	fprintf(stderr, "  -2          Force protocol version 2.\n");
	fprintf(stderr, "  -o 'option' Process the option as if it was read from a configuration file.\n");
*/

/* Function prototypes */
_PROTOTYP(static int ssh_session,(void));
_PROTOTYP(static int ssh_session2,(void));
_PROTOTYP(VOID client_set_session_ident,(int id));

/*
 * Connects to the given host using rsh (or prints an error message and exits
 * if rsh is not available).  This function never returns.
 */
static void
rsh_connect(char *host, char *user, Buffer * command)
{
	char *args[10];
	int i;

	log("Using rsh.  WARNING: Connection will not be encrypted.");
	/* Build argument list for rsh. */
	i = 0;
	args[i++] = _PATH_RSH;
	/* host may have to come after user on some systems */
	args[i++] = host;
	if (user) {
		args[i++] = "-l";
		args[i++] = user;
	}
	if (buffer_len(command) > 0) {
		buffer_append(command, "\0", 1);
		args[i++] = buffer_ptr(command);
	}
	args[i++] = NULL;
	if (debug_flag) {
		for (i = 0; args[i]; i++) {
			if (i != 0)
				fprintf(stderr, " ");
			fprintf(stderr, "%s", args[i]);
		}
		fprintf(stderr, "\n");
	}
	execv(_PATH_RSH, args);
	perror(_PATH_RSH);
	exit(1);
}

/*
 * Main program for the ssh client.
 */
static int
main(int ac, char **av)
{
	int i, opt, optind, exit_status, ok;
	u_short fwd_port, fwd_host_port;
	char *optarg, *cp, buf[256];
	struct stat st;
	struct passwd *pw, pwcopy;
	int dummy;
	uid_t original_effective_uid;

	init_rng();

	/*
	 * Save the original real uid.  It will be needed later (uid-swapping
	 * may clobber the real uid).
	 */
	original_real_uid = getuid();
	original_effective_uid = geteuid();

	/* If we are installed setuid root be careful to not drop core. */
	if (original_real_uid != original_effective_uid) {
		struct rlimit rlim;
		rlim.rlim_cur = rlim.rlim_max = 0;
		if (setrlimit(RLIMIT_CORE, &rlim) < 0)
			fatal("setrlimit failed: %.100s", strerror(errno));
	}
	/*
	 * Use uid-swapping to give up root privileges for the duration of
	 * option processing.  We will re-instantiate the rights when we are
	 * ready to create the privileged port, and will permanently drop
	 * them when the port has been created (actually, when the connection
	 * has been made, as we may need to create the port several times).
	 */
	temporarily_use_uid(original_real_uid);

	/*
	 * Set our umask to something reasonable, as some files are created
	 * with the default umask.  This will make them world-readable but
	 * writable only by the owner, which is ok for all files for which we
	 * don't set the modes explicitly.
	 */
	umask(022);

	/* Save our own name. */
	av0 = av[0];

	/* Initialize option structure to indicate that no values have been set. */
	initialize_options(&options);

	/* Parse command-line arguments. */
	host = NULL;

	/* If program name is not one of the standard names, use it as host name. */
	if (strchr(av0, '/'))
		cp = strrchr(av0, '/') + 1;
	else
		cp = av0;
	if (strcmp(cp, "rsh") && strcmp(cp, "ssh") && strcmp(cp, "rlogin") &&
	    strcmp(cp, "slogin") && strcmp(cp, "remsh"))
		host = cp;

	for (optind = 1; optind < ac; optind++) {
		if (av[optind][0] != '-') {
			if (host)
				break;
			if ((cp = strchr(av[optind], '@'))) {
				if(cp == av[optind])
					usage();
				options.user = av[optind];
				*cp = '\0';
				host = ++cp;
			} else
				host = av[optind];
			continue;
		}
		opt = av[optind][1];
		if (!opt)
			usage();
		if (strchr("eilcpLRo", opt)) {	/* options with arguments */
			optarg = av[optind] + 2;
			if (strcmp(optarg, "") == 0) {
				if (optind >= ac - 1)
					usage();
				optarg = av[++optind];
			}
		} else {
			if (av[optind][2])
				usage();
			optarg = NULL;
		}
		switch (opt) {
		case '2':
			options.protocol = SSH_PROTO_2;
			break;
		case '4':
			IPv4or6 = AF_INET;
			break;
		case '6':
			IPv4or6 = AF_INET6;
			break;
		case 'n':
			stdin_null_flag = 1;
			break;
		case 'f':
			fork_after_authentication_flag = 1;
			stdin_null_flag = 1;
			break;
		case 'x':
			options.forward_x11 = 0;
			break;
		case 'X':
			options.forward_x11 = 1;
			break;
		case 'g':
			options.gateway_ports = 1;
			break;
		case 'P':
			options.use_privileged_port = 0;
			break;
		case 'a':
			options.forward_agent = 0;
			break;
		case 'A':
			options.forward_agent = 1;
			break;
#ifdef AFS
		case 'k':
			options.kerberos_tgt_passing = 0;
			options.afs_token_passing = 0;
			break;
#endif
		case 'i':
			if (stat(optarg, &st) < 0) {
				fprintf(stderr, "Warning: Identity file %s does not exist.\n",
					optarg);
				break;
			}
			if (options.num_identity_files >= SSH_MAX_IDENTITY_FILES)
				fatal("Too many identity files specified (max %d)",
				      SSH_MAX_IDENTITY_FILES);
			options.identity_files[options.num_identity_files++] =
				xstrdup(optarg);
			break;
		case 't':
			tty_flag = 1;
			break;
		case 'v':
		case 'V':
			fprintf(stderr, "SSH Version %s, protocol versions %d.%d/%d.%d.\n",
			    SSH_VERSION,
			    PROTOCOL_MAJOR_1, PROTOCOL_MINOR_1,
			    PROTOCOL_MAJOR_2, PROTOCOL_MINOR_2);
			fprintf(stderr, "Compiled with SSL (0x%8.8lx).\n", SSLeay());
			if (opt == 'V')
				exit(0);
			debug_flag = 1;
			options.log_level = SYSLOG_LEVEL_DEBUG;
			break;
		case 'q':
			options.log_level = SYSLOG_LEVEL_QUIET;
			break;
		case 'e':
			if (optarg[0] == '^' && optarg[2] == 0 &&
			    (unsigned char) optarg[1] >= 64 && (unsigned char) optarg[1] < 128)
				options.escape_char = (unsigned char) optarg[1] & 31;
			else if (strlen(optarg) == 1)
				options.escape_char = (unsigned char) optarg[0];
			else if (strcmp(optarg, "none") == 0)
				options.escape_char = -2;
			else {
				fprintf(stderr, "Bad escape character '%s'.\n", optarg);
				exit(1);
			}
			break;
		case 'c':
			if (ciphers_valid(optarg)) {
				/* SSH2 only */
				options.ciphers = xstrdup(optarg);
				options.cipher = SSH_CIPHER_ILLEGAL;
			} else {
				/* SSH1 only */
				options.cipher = cipher_number(optarg);
				if (options.cipher == -1) {
					fprintf(stderr, "Unknown cipher type '%s'\n", optarg);
					exit(1);
				}
			}
			break;
		case 'p':
			options.port = atoi(optarg);
			break;
		case 'l':
			options.user = optarg;
			break;
		case 'R':
			if (sscanf(optarg, "%hu/%255[^/]/%hu", &fwd_port, buf,
			    &fwd_host_port) != 3 &&
			    sscanf(optarg, "%hu:%255[^:]:%hu", &fwd_port, buf,
			    &fwd_host_port) != 3) {
				fprintf(stderr, "Bad forwarding specification '%s'.\n", optarg);
				usage();
				/* NOTREACHED */
			}
			add_remote_forward(&options, fwd_port, buf, fwd_host_port);
			break;
		case 'L':
			if (sscanf(optarg, "%hu/%255[^/]/%hu", &fwd_port, buf,
			    &fwd_host_port) != 3 &&
			    sscanf(optarg, "%hu:%255[^:]:%hu", &fwd_port, buf,
			    &fwd_host_port) != 3) {
				fprintf(stderr, "Bad forwarding specification '%s'.\n", optarg);
				usage();
				/* NOTREACHED */
			}
			add_local_forward(&options, fwd_port, buf, fwd_host_port);
			break;
		case 'C':
			options.compression = 1;
			break;
		case 'N':
			no_shell_flag = 1;
			no_tty_flag = 1;
			break;
		case 'T':
			no_tty_flag = 1;
			break;
		case 'o':
			dummy = 1;
			if (process_config_line(&options, host ? host : "", optarg,
					 "command-line", 0, &dummy) != 0)
				exit(1);
			break;
		default:
			usage();
		}
	}

	/* Check that we got a host name. */
	if (!host)
		usage();

	/* Initialize the command to execute on remote host. */
	buffer_init(&command);

	SSLeay_add_all_algorithms();

	/*
	 * Save the command to execute on the remote host in a buffer. There
	 * is no limit on the length of the command, except by the maximum
	 * packet size.  Also sets the tty flag if there is no command.
	 */
	if (optind == ac) {
		/* No command specified - execute shell on a tty. */
		tty_flag = 1;
	} else {
		/* A command has been specified.  Store it into the
		   buffer. */
		for (i = optind; i < ac; i++) {
			if (i > optind)
				buffer_append(&command, " ", 1);
			buffer_append(&command, av[i], strlen(av[i]));
		}
	}

	/* Cannot fork to background if no command. */
	if (fork_after_authentication_flag && buffer_len(&command) == 0 && !no_shell_flag)
		fatal("Cannot fork into background without a command to execute.");

	/* Allocate a tty by default if no command specified. */
	if (buffer_len(&command) == 0)
		tty_flag = 1;

	/* Do not allocate a tty if stdin is not a tty. */
	if (!isatty(fileno(stdin))) {
		if (tty_flag)
			fprintf(stderr, "Pseudo-terminal will not be allocated because stdin is not a terminal.\n");
		tty_flag = 0;
	}
	/* force */
	if (no_tty_flag)
		tty_flag = 0;

	/* Get user data. */
	pw = getpwuid(original_real_uid);
	if (!pw) {
		fprintf(stderr, "You don't exist, go away!\n");
		exit(1);
	}
	/* Take a copy of the returned structure. */
	memset(&pwcopy, 0, sizeof(pwcopy));
	pwcopy.pw_name = xstrdup(pw->pw_name);
	pwcopy.pw_passwd = xstrdup(pw->pw_passwd);
	pwcopy.pw_uid = pw->pw_uid;
	pwcopy.pw_gid = pw->pw_gid;
#ifdef HAVE_PW_CLASS_IN_PASSWD
	pwcopy.pw_class = xstrdup(pw->pw_class);
#endif
	pwcopy.pw_dir = xstrdup(pw->pw_dir);
	pwcopy.pw_shell = xstrdup(pw->pw_shell);
	pw = &pwcopy;

	/* Initialize "log" output.  Since we are the client all output
	   actually goes to the terminal. */
	log_init(av[0], options.log_level, SYSLOG_FACILITY_USER, 0);

	/* Read per-user configuration file. */
	snprintf(buf, sizeof buf, "%.100s/%.100s", pw->pw_dir, SSH_USER_CONFFILE);
	read_config_file(buf, host, &options);

	/* Read systemwide configuration file. */
	read_config_file(HOST_CONFIG_FILE, host, &options);

	/* Fill configuration defaults. */
	fill_default_options(&options);

	/* reinit */
	log_init(av[0], options.log_level, SYSLOG_FACILITY_USER, 0);

	/* check if RSA support exists */
	if ((options.protocol & SSH_PROTO_1) &&
	    rsa_alive() == 0) {
		log("%s: no RSA support in libssl and libcrypto.  See ssl(8).",
		    __progname);
		log("Disabling protocol version 1");
		options.protocol &= ~ (SSH_PROTO_1|SSH_PROTO_1_PREFERRED);
	}
	if (! options.protocol & (SSH_PROTO_1|SSH_PROTO_2)) {
		fprintf(stderr, "%s: No protocol version available.\n",
		    __progname);
 		exit(1);
	}

	if (options.user == NULL)
		options.user = xstrdup(pw->pw_name);

	if (options.hostname != NULL)
		host = options.hostname;

	/* Find canonic host name. */
	if (strchr(host, '.') == 0) {
		struct addrinfo hints;
		struct addrinfo *ai = NULL;
		int errgai;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = IPv4or6;
		hints.ai_flags = AI_CANONNAME;
		hints.ai_socktype = SOCK_STREAM;
		errgai = getaddrinfo(host, NULL, &hints, &ai);
		if (errgai == 0) {
			if (ai->ai_canonname != NULL)
				host = xstrdup(ai->ai_canonname);
			freeaddrinfo(ai);
		}
	}
	/* Disable rhosts authentication if not running as root. */
	if (original_effective_uid != 0 || !options.use_privileged_port) {
		options.rhosts_authentication = 0;
		options.rhosts_rsa_authentication = 0;
	}
	/*
	 * If using rsh has been selected, exec it now (without trying
	 * anything else).  Note that we must release privileges first.
	 */
	if (options.use_rsh) {
		/*
		 * Restore our superuser privileges.  This must be done
		 * before permanently setting the uid.
		 */
		restore_uid();

		/* Switch to the original uid permanently. */
		permanently_set_uid(original_real_uid);

		/* Execute rsh. */
		rsh_connect(host, options.user, &command);
		fatal("rsh_connect returned");
	}
	/* Restore our superuser privileges. */
	restore_uid();

	/*
	 * Open a connection to the remote host.  This needs root privileges
	 * if rhosts_{rsa_}authentication is enabled.
	 */

	ok = ssh_connect(host, &hostaddr, options.port,
			 options.connection_attempts,
			 !options.rhosts_authentication &&
			 !options.rhosts_rsa_authentication,
			 original_real_uid,
			 options.proxy_command);

	/*
	 * If we successfully made the connection, load the host private key
	 * in case we will need it later for combined rsa-rhosts
	 * authentication. This must be done before releasing extra
	 * privileges, because the file is only readable by root.
	 */
	if (ok && (options.protocol & SSH_PROTO_1)) {
		Key k;
		host_private_key = RSA_new();
		k.type = KEY_RSA;
		k.rsa = host_private_key;
		if (load_private_key(HOST_KEY_FILE, "", &k, NULL))
			host_private_key_loaded = 1;
	}
	/*
	 * Get rid of any extra privileges that we may have.  We will no
	 * longer need them.  Also, extra privileges could make it very hard
	 * to read identity files and other non-world-readable files from the
	 * user's home directory if it happens to be on a NFS volume where
	 * root is mapped to nobody.
	 */

	/*
	 * Note that some legacy systems need to postpone the following call
	 * to permanently_set_uid() until the private hostkey is destroyed
	 * with RSA_free().  Otherwise the calling user could ptrace() the
	 * process, read the private hostkey and impersonate the host.
	 * OpenBSD does not allow ptracing of setuid processes.
	 */
	permanently_set_uid(original_real_uid);

	/*
	 * Now that we are back to our own permissions, create ~/.ssh
	 * directory if it doesn\'t already exist.
	 */
	snprintf(buf, sizeof buf, "%.100s/%.100s", pw->pw_dir, SSH_USER_DIR);
	if (stat(buf, &st) < 0)
		if (mkdir(buf, 0700) < 0)
			error("Could not create directory '%.200s'.", buf);

	/* Check if the connection failed, and try "rsh" if appropriate. */
	if (!ok) {
		if (options.port != 0)
			log("Secure connection to %.100s on port %hu refused%.100s.",
			    host, options.port,
			    options.fallback_to_rsh ? "; reverting to insecure method" : "");
		else
			log("Secure connection to %.100s refused%.100s.", host,
			    options.fallback_to_rsh ? "; reverting to insecure method" : "");

		if (options.fallback_to_rsh) {
			rsh_connect(host, options.user, &command);
			fatal("rsh_connect returned");
		}
		exit(1);
	}
	/* Expand ~ in options.identity_files. */
	/* XXX mem-leaks */
	for (i = 0; i < options.num_identity_files; i++)
		options.identity_files[i] =
			tilde_expand_filename(options.identity_files[i], original_real_uid);
	for (i = 0; i < options.num_identity_files2; i++)
		options.identity_files2[i] =
			tilde_expand_filename(options.identity_files2[i], original_real_uid);
	/* Expand ~ in known host file names. */
	options.system_hostfile = tilde_expand_filename(options.system_hostfile,
	    original_real_uid);
	options.user_hostfile = tilde_expand_filename(options.user_hostfile,
	    original_real_uid);
	options.system_hostfile2 = tilde_expand_filename(options.system_hostfile2,
	    original_real_uid);
	options.user_hostfile2 = tilde_expand_filename(options.user_hostfile2,
	    original_real_uid);

	/* Log into the remote system.  This never returns if the login fails. */
	ssh_login(host_private_key_loaded, host_private_key,
		  host, (struct sockaddr *)&hostaddr, original_real_uid);

	/* We no longer need the host private key.  Clear it now. */
	if (host_private_key_loaded)
		RSA_free(host_private_key);	/* Destroys contents safely */

	exit_status = compat20 ? ssh_session2() : ssh_session();
	packet_close();
	return exit_status;
}

static void
x11_get_proto(char *proto, int proto_len, char *data, int data_len)
{
	char line[512];
	FILE *f;
	int got_data = 0, i;

	if (options.xauth_location) {
		/* Try to get Xauthority information for the display. */
		snprintf(line, sizeof line, "%.100s list %.200s 2>/dev/null",
		    options.xauth_location, getenv("DISPLAY"));
		f = popen(line, "r");
		if (f && fgets(line, sizeof(line), f) &&
		    sscanf(line, "%*s %s %s", proto, data) == 2)
			got_data = 1;
		if (f)
			pclose(f);
	}
	/*
	 * If we didn't get authentication data, just make up some
	 * data.  The forwarding code will check the validity of the
	 * response anyway, and substitute this data.  The X11
	 * server, however, will ignore this fake data and use
	 * whatever authentication mechanisms it was using otherwise
	 * for the local connection.
	 */
	if (!got_data) {
		u_int32_t rand = 0;

		strlcpy(proto, "MIT-MAGIC-COOKIE-1", proto_len);
		for (i = 0; i < 16; i++) {
			if (i % 4 == 0)
				rand = arc4random();
			snprintf(data + 2 * i, data_len - 2 * i, "%02x", rand & 0xff);
			rand >>= 8;
		}
	}
}

static int
ssh_session(void)
{
	int type;
	int i;
	int plen;
	int interactive = 0;
	int have_tty = 0;
	struct winsize ws;
	int authfd;
	char *cp;

	/* Enable compression if requested. */
	if (options.compression) {
		debug("Requesting compression at level %d.", options.compression_level);

		if (options.compression_level < 1 || options.compression_level > 9)
			fatal("Compression level must be from 1 (fast) to 9 (slow, best).");

		/* Send the request. */
		packet_start(SSH_CMSG_REQUEST_COMPRESSION);
		packet_put_int(options.compression_level);
		packet_send();
		packet_write_wait();
		type = packet_read(&plen);
		if (type == SSH_SMSG_SUCCESS)
			packet_start_compression(options.compression_level);
		else if (type == SSH_SMSG_FAILURE)
			log("Warning: Remote host refused compression.");
		else
			packet_disconnect("Protocol error waiting for compression response.");
	}
	/* Allocate a pseudo tty if appropriate. */
	if (tty_flag) {
		debug("Requesting pty.");

		/* Start the packet. */
		packet_start(SSH_CMSG_REQUEST_PTY);

		/* Store TERM in the packet.  There is no limit on the
		   length of the string. */
		cp = getenv("TERM");
		if (!cp)
			cp = "";
		packet_put_string(cp, strlen(cp));

		/* Store window size in the packet. */
		if (ioctl(fileno(stdin), TIOCGWINSZ, &ws) < 0)
			memset(&ws, 0, sizeof(ws));
		packet_put_int(ws.ws_row);
		packet_put_int(ws.ws_col);
		packet_put_int(ws.ws_xpixel);
		packet_put_int(ws.ws_ypixel);

		/* Store tty modes in the packet. */
		tty_make_modes(fileno(stdin));

		/* Send the packet, and wait for it to leave. */
		packet_send();
		packet_write_wait();

		/* Read response from the server. */
		type = packet_read(&plen);
		if (type == SSH_SMSG_SUCCESS) {
			interactive = 1;
			have_tty = 1;
		} else if (type == SSH_SMSG_FAILURE)
			log("Warning: Remote host failed or refused to allocate a pseudo tty.");
		else
			packet_disconnect("Protocol error waiting for pty request response.");
	}
	/* Request X11 forwarding if enabled and DISPLAY is set. */
	if (options.forward_x11 && getenv("DISPLAY") != NULL) {
		char proto[512], data[512];
		/* Get reasonable local authentication information. */
		x11_get_proto(proto, sizeof proto, data, sizeof data);
		/* Request forwarding with authentication spoofing. */
		debug("Requesting X11 forwarding with authentication spoofing.");
		x11_request_forwarding_with_spoofing(0, proto, data);

		/* Read response from the server. */
		type = packet_read(&plen);
		if (type == SSH_SMSG_SUCCESS) {
			interactive = 1;
		} else if (type == SSH_SMSG_FAILURE) {
			log("Warning: Remote host denied X11 forwarding.");
		} else {
			packet_disconnect("Protocol error waiting for X11 forwarding");
		}
	}
	/* Tell the packet module whether this is an interactive session. */
	packet_set_interactive(interactive, options.keepalives);

	/* Clear agent forwarding if we don\'t have an agent. */
	authfd = ssh_get_authentication_socket();
	if (authfd < 0)
		options.forward_agent = 0;
	else
		ssh_close_authentication_socket(authfd);

	/* Request authentication agent forwarding if appropriate. */
	if (options.forward_agent) {
		debug("Requesting authentication agent forwarding.");
		auth_request_forwarding();

		/* Read response from the server. */
		type = packet_read(&plen);
		packet_integrity_check(plen, 0, type);
		if (type != SSH_SMSG_SUCCESS)
			log("Warning: Remote host denied authentication agent forwarding.");
	}
	/* Initiate local TCP/IP port forwardings. */
	for (i = 0; i < options.num_local_forwards; i++) {
		debug("Connections to local port %d forwarded to remote address %.200s:%d",
		      options.local_forwards[i].port,
		      options.local_forwards[i].host,
		      options.local_forwards[i].host_port);
		channel_request_local_forwarding(options.local_forwards[i].port,
						 options.local_forwards[i].host,
						 options.local_forwards[i].host_port,
						 options.gateway_ports);
	}

	/* Initiate remote TCP/IP port forwardings. */
	for (i = 0; i < options.num_remote_forwards; i++) {
		debug("Connections to remote port %d forwarded to local address %.200s:%d",
		      options.remote_forwards[i].port,
		      options.remote_forwards[i].host,
		      options.remote_forwards[i].host_port);
		channel_request_remote_forwarding(options.remote_forwards[i].port,
						  options.remote_forwards[i].host,
						  options.remote_forwards[i].host_port);
	}

	/* If requested, let ssh continue in the background. */
	if (fork_after_authentication_flag)
		if (daemon(1, 1) < 0)
			fatal("daemon() failed: %.200s", strerror(errno));

	/*
	 * If a command was specified on the command line, execute the
	 * command now. Otherwise request the server to start a shell.
	 */
	if (buffer_len(&command) > 0) {
		int len = buffer_len(&command);
		if (len > 900)
			len = 900;
		debug("Sending command: %.*s", len, buffer_ptr(&command));
		packet_start(SSH_CMSG_EXEC_CMD);
		packet_put_string(buffer_ptr(&command), buffer_len(&command));
		packet_send();
		packet_write_wait();
	} else {
		debug("Requesting shell.");
		packet_start(SSH_CMSG_EXEC_SHELL);
		packet_send();
		packet_write_wait();
	}

	/* Enter the interactive session. */
	return client_loop(have_tty, tty_flag ? options.escape_char : -1, 0);
}

static void
init_local_fwd(void)
{
	int i;
	/* Initiate local TCP/IP port forwardings. */
	for (i = 0; i < options.num_local_forwards; i++) {
		debug("Connections to local port %d forwarded to remote address %.200s:%d",
		      options.local_forwards[i].port,
		      options.local_forwards[i].host,
		      options.local_forwards[i].host_port);
		channel_request_local_forwarding(options.local_forwards[i].port,
						 options.local_forwards[i].host,
						 options.local_forwards[i].host_port,
						 options.gateway_ports);
	}
}

static void
client_init(int id, void *arg)
{
	int len;
	debug("client_init id %d arg %d", id, (int)arg);

	if (no_shell_flag)
		goto done;

	if (tty_flag) {
		struct winsize ws;
		char *cp;
		cp = getenv("TERM");
		if (!cp)
			cp = "";
		/* Store window size in the packet. */
		if (ioctl(fileno(stdin), TIOCGWINSZ, &ws) < 0)
			memset(&ws, 0, sizeof(ws));

		channel_request_start(id, "pty-req", 0);
		packet_put_cstring(cp);
		packet_put_int(ws.ws_col);
		packet_put_int(ws.ws_row);
		packet_put_int(ws.ws_xpixel);
		packet_put_int(ws.ws_ypixel);
		packet_put_cstring("");		/* XXX: encode terminal modes */
		packet_send();
		/* XXX wait for reply */
	}
	if (options.forward_x11 &&
	    getenv("DISPLAY") != NULL) {
		char proto[512], data[512];
		/* Get reasonable local authentication information. */
		x11_get_proto(proto, sizeof proto, data, sizeof data);
		/* Request forwarding with authentication spoofing. */
		debug("Requesting X11 forwarding with authentication spoofing.");
		x11_request_forwarding_with_spoofing(id, proto, data);
		/* XXX wait for reply */
	}

	len = buffer_len(&command);
	if (len > 0) {
		if (len > 900)
			len = 900;
		debug("Sending command: %.*s", len, buffer_ptr(&command));
		channel_request_start(id, "exec", 0);
		packet_put_string(buffer_ptr(&command), len);
		packet_send();
	} else {
		channel_request(id, "shell", 0);
	}
	/* channel_callback(id, SSH2_MSG_OPEN_CONFIGMATION, client_init, 0); */
done:
	/* register different callback, etc. XXX */
	client_set_session_ident(id);
}

static int
ssh_session2(void)
{
	int window, packetmax, id;
	int in, out, err;

	if (stdin_null_flag) {
		in = open("/dev/null", O_RDONLY);
	} else {
		in = dup(STDIN_FILENO);
	}
	out = dup(STDOUT_FILENO);
	err = dup(STDERR_FILENO);

	if (in < 0 || out < 0 || err < 0)
		fatal("dup() in/out/err failed");

	/* should be pre-session */
	init_local_fwd();
	
	/* If requested, let ssh continue in the background. */
	if (fork_after_authentication_flag)
		if (daemon(1, 1) < 0)
			fatal("daemon() failed: %.200s", strerror(errno));

	window = 32*1024;
	if (tty_flag) {
		packetmax = window/8;
	} else {
		window *= 2;
		packetmax = window/2;
	}

/*XXX MAXPACK */
	id = channel_new(
	    "session", SSH_CHANNEL_OPENING, in, out, err,
	    window, packetmax, CHAN_EXTENDED_WRITE, xstrdup("client-session"));

	channel_open(id);
	channel_register_callback(id, SSH2_MSG_CHANNEL_OPEN_CONFIRMATION, client_init, (void *)0);

	return client_loop(tty_flag, tty_flag ? options.escape_char : -1, id);
}

/*
 * Connect to the given ssh server using a proxy command.
 */
static int
ssh_proxy_connect(const char *host, u_short port, uid_t original_real_uid,
		  const char *proxy_command)
{
    Buffer command;
    const char *cp;
    char *command_string;
    int pin[2], pout[2];
    pid_t pid;
    char strport[NI_MAXSERV];

    /* Convert the port number into a string. */
    snprintf(strport, sizeof strport, "%hu", port);

    /* Build the final command string in the buffer by making the
    appropriate substitutions to the given proxy command. */
    buffer_init(&command);
    for (cp = proxy_command; *cp; cp++) {
        if (cp[0] == '%' && cp[1] == '%') {
            buffer_append(&command, "%", 1);
            cp++;
            continue;
        }
        if (cp[0] == '%' && cp[1] == 'h') {
            buffer_append(&command, host, strlen(host));
            cp++;
            continue;
        }
        if (cp[0] == '%' && cp[1] == 'p') {
            buffer_append(&command, strport, strlen(strport));
            cp++;
            continue;
        }
        buffer_append(&command, cp, 1);
    }
    buffer_append(&command, "\0", 1);

    /* Get the final command string. */
    command_string = buffer_ptr(&command);

    /* Create pipes for communicating with the proxy. */
    if (pipe(pin) < 0 || pipe(pout) < 0)
        fatal("Could not create pipes to communicate with the proxy: %.100s",
               strerror(errno));

    debug("Executing proxy command: %.500s", command_string);

    /* Fork and execute the proxy command. */
    if ((pid = fork()) == 0) {
        char *argv[10];

        /* Child.  Permanently give up superuser privileges. */
        permanently_set_uid(original_real_uid);

        /* Redirect stdin and stdout. */
        close(pin[1]);
        if (pin[0] != 0) {
            if (dup2(pin[0], 0) < 0)
                perror("dup2 stdin");
            close(pin[0]);
        }
        close(pout[0]);
        if (dup2(pout[1], 1) < 0)
            perror("dup2 stdout");
        /* Cannot be 1 because pin allocated two descriptors. */
        close(pout[1]);

        /* Stderr is left as it is so that error messages get
           printed on the user's terminal. */
        argv[0] = _PATH_BSHELL;
        argv[1] = "-c";
        argv[2] = command_string;
        argv[3] = NULL;

        /* Execute the proxy command.  Note that we gave up any
           extra privileges above. */
        execv(_PATH_BSHELL, argv);
        perror(_PATH_BSHELL);
        exit(1);
    }
    /* Parent. */
    if (pid < 0)
        fatal("fork failed: %.100s", strerror(errno));

    /* Close child side of the descriptors. */
    close(pin[0]);
    close(pout[1]);

    /* Free the command name. */
    buffer_free(&command);

    /* Set the connection file descriptors. */
    packet_set_connection(pout[0], pin[1]);

    return 1;
}

/*
 * Creates a (possibly privileged) socket for use as the ssh connection.
 */
static int
ssh_create_socket(uid_t original_real_uid, int privileged, int family)
{
	int sock;

	/*
	 * If we are running as root and want to connect to a privileged
	 * port, bind our own socket to a privileged port.
	 */
	if (privileged) {
		int p = IPPORT_RESERVED - 1;
		sock = rresvport_af(&p, family);
		if (sock < 0)
			error("rresvport: af=%d %.100s", family, strerror(errno));
		else
			debug("Allocated local port %d.", p);
	} else {
		/*
		 * Just create an ordinary socket on arbitrary port.  We use
		 * the user's uid to create the socket.
		 */
		temporarily_use_uid(original_real_uid);
		sock = socket(family, SOCK_STREAM, 0);
		if (sock < 0)
			error("socket: %.100s", strerror(errno));
		restore_uid();
	}
	return sock;
}

/*
 * Opens a TCP/IP connection to the remote server on the given host.
 * The address of the remote host will be returned in hostaddr.
 * If port is 0, the default port will be used.  If anonymous is zero,
 * a privileged port will be allocated to make the connection.
 * This requires super-user privileges if anonymous is false.
 * Connection_attempts specifies the maximum number of tries (one per
 * second).  If proxy_command is non-NULL, it specifies the command (with %h
 * and %p substituted for host and port, respectively) to use to contact
 * the daemon.
 */
static int
ssh_connect(const char *host, struct sockaddr_storage * hostaddr,
	    u_short port, int connection_attempts,
	    int anonymous, uid_t original_real_uid,
	    const char *proxy_command)
{
	int sock = -1, attempt;
	struct servent *sp;
	struct addrinfo hints, *ai, *aitop;
	char ntop[NI_MAXHOST], strport[NI_MAXSERV];
	int gaierr;
	struct linger linger;

	debug("ssh_connect: getuid %u geteuid %u anon %d",
	      (u_int) getuid(), (u_int) geteuid(), anonymous);

	/* Get default port if port has not been set. */
	if (port == 0) {
		sp = getservbyname(SSH_SERVICE_NAME, "tcp");
		if (sp)
			port = ntohs(sp->s_port);
		else
			port = SSH_DEFAULT_PORT;
	}
	/* If a proxy command is given, connect using it. */
	if (proxy_command != NULL)
		return ssh_proxy_connect(host, port, original_real_uid, proxy_command);

	/* No proxy command. */

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = IPv4or6;
	hints.ai_socktype = SOCK_STREAM;
	snprintf(strport, sizeof strport, "%d", port);
	if ((gaierr = getaddrinfo(host, strport, &hints, &aitop)) != 0)
		fatal("%s: %.100s: %s", __progname, host,
		    gai_strerror(gaierr));

	/*
	 * Try to connect several times.  On some machines, the first time
	 * will sometimes fail.  In general socket code appears to behave
	 * quite magically on many machines.
	 */
	for (attempt = 0; attempt < connection_attempts; attempt++) {
		if (attempt > 0)
			debug("Trying again...");

		/* Loop through addresses for this host, and try each one in
		   sequence until the connection succeeds. */
		for (ai = aitop; ai; ai = ai->ai_next) {
			if (ai->ai_family != AF_INET && ai->ai_family != AF_INET6)
				continue;
			if (getnameinfo(ai->ai_addr, ai->ai_addrlen,
			    ntop, sizeof(ntop), strport, sizeof(strport),
			    NI_NUMERICHOST|NI_NUMERICSERV) != 0) {
				error("ssh_connect: getnameinfo failed");
				continue;
			}
			debug("Connecting to %.200s [%.100s] port %s.",
				host, ntop, strport);

			/* Create a socket for connecting. */
			sock = ssh_create_socket(original_real_uid,
			    !anonymous && geteuid() == 0 && port < IPPORT_RESERVED,
			    ai->ai_family);
			if (sock < 0)
				continue;

			/* Connect to the host.  We use the user's uid in the
			 * hope that it will help with tcp_wrappers showing
			 * the remote uid as root.
			 */
			temporarily_use_uid(original_real_uid);
			if (connect(sock, ai->ai_addr, ai->ai_addrlen) >= 0) {
				/* Successful connection. */
				memcpy(hostaddr, ai->ai_addr, ai->ai_addrlen);
				restore_uid();
				break;
			} else {
				debug("connect: %.100s", strerror(errno));
				restore_uid();
				/*
				 * Close the failed socket; there appear to
				 * be some problems when reusing a socket for
				 * which connect() has already returned an
				 * error.
				 */
				shutdown(sock, SHUT_RDWR);
				close(sock);
			}
		}
		if (ai)
			break;	/* Successful connection. */

		/* Sleep a moment before retrying. */
		sleep(1);
	}

	freeaddrinfo(aitop);

	/* Return failure if we didn't get a successful connection. */
	if (attempt >= connection_attempts)
		return 0;

	debug("Connection established.");

	/*
	 * Set socket options.  We would like the socket to disappear as soon
	 * as it has been closed for whatever reason.
	 */
	/* setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on)); */
	linger.l_onoff = 1;
	linger.l_linger = 5;
	setsockopt(sock, SOL_SOCKET, SO_LINGER, (void *) &linger, sizeof(linger));

	/* Set the connection. */
	packet_set_connection(sock, sock);

	return 1;
}

/*
 * Waits for the server identification string, and sends our own
 * identification string.
 */
static void
ssh_exchange_identification()
{
	char buf[256], remote_version[256];	/* must be same size! */
	int remote_major, remote_minor, i, mismatch;
	int connection_in = packet_get_connection_in();
	int connection_out = packet_get_connection_out();

	/* Read other side\'s version identification. */
	for (;;) {
		for (i = 0; i < sizeof(buf) - 1; i++) {
			int len = atomicio(read, connection_in, &buf[i], 1);
			if (len < 0)
				fatal("ssh_exchange_identification: read: %.100s", strerror(errno));
			if (len != 1)
				fatal("ssh_exchange_identification: Connection closed by remote host");
			if (buf[i] == '\r') {
				buf[i] = '\n';
				buf[i + 1] = 0;
				continue;		/**XXX wait for \n */
			}
			if (buf[i] == '\n') {
				buf[i + 1] = 0;
				break;
			}
		}
		buf[sizeof(buf) - 1] = 0;
		if (strncmp(buf, "SSH-", 4) == 0)
			break;
		debug("ssh_exchange_identification: %s", buf);
	}
	server_version_string = xstrdup(buf);

	/*
	 * Check that the versions match.  In future this might accept
	 * several versions and set appropriate flags to handle them.
	 */
	if (sscanf(server_version_string, "SSH-%d.%d-%[^\n]\n",
	    &remote_major, &remote_minor, remote_version) != 3)
		fatal("Bad remote protocol version identification: '%.100s'", buf);
	debug("Remote protocol version %d.%d, remote software version %.100s",
	      remote_major, remote_minor, remote_version);

	compat_datafellows(remote_version);
	mismatch = 0;

	switch(remote_major) {
	case 1:
		if (remote_minor == 99 &&
		    (options.protocol & SSH_PROTO_2) &&
		    !(options.protocol & SSH_PROTO_1_PREFERRED)) {
			enable_compat20();
			break;
		}
		if (!(options.protocol & SSH_PROTO_1)) {
			mismatch = 1;
			break;
		}
		if (remote_minor < 3) {
			fatal("Remote machine has too old SSH software version.");
		} else if (remote_minor == 3) {
			/* We speak 1.3, too. */
			enable_compat13();
			if (options.forward_agent) {
				log("Agent forwarding disabled for protocol 1.3");
				options.forward_agent = 0;
			}
		}
		break;
	case 2:
		if (options.protocol & SSH_PROTO_2) {
			enable_compat20();
			break;
		}
		/* FALLTHROUGH */
	default:
		mismatch = 1;
		break;
	}
	if (mismatch)
		fatal("Protocol major versions differ: %d vs. %d",
		    (options.protocol & SSH_PROTO_2) ? PROTOCOL_MAJOR_2 : PROTOCOL_MAJOR_1,
		    remote_major);
	if (compat20)
		packet_set_ssh2_format();
	/* Send our own protocol version identification. */
	snprintf(buf, sizeof buf, "SSH-%d.%d-%.100s\n",
	    compat20 ? PROTOCOL_MAJOR_2 : PROTOCOL_MAJOR_1,
	    compat20 ? PROTOCOL_MINOR_2 : PROTOCOL_MINOR_1,
	    SSH_VERSION);
	if (atomicio(write, connection_out, buf, strlen(buf)) != strlen(buf))
		fatal("write: %.100s", strerror(errno));
	client_version_string = xstrdup(buf);
	chop(client_version_string);
	chop(server_version_string);
	debug("Local version string %.100s", client_version_string);
}

static int
read_yes_or_no(const char *prompt, int defval)
{
	char buf[1024];
	FILE *f;
	int retval = -1;

	if (isatty(0))
		f = stdin;
	else
		f = fopen("/dev/tty", "rw");

	if (f == NULL)
		return 0;

	fflush(stdout);

	while (1) {
		fprintf(stderr, "%s", prompt);
		if (fgets(buf, sizeof(buf), f) == NULL) {
			/* Print a newline (the prompt probably didn\'t have one). */
			fprintf(stderr, "\n");
			strlcpy(buf, "no", sizeof buf);
		}
		/* Remove newline from response. */
		if (strchr(buf, '\n'))
			*strchr(buf, '\n') = 0;

		if (buf[0] == 0)
			retval = defval;
		if (strcmp(buf, "yes") == 0)
			retval = 1;
		if (strcmp(buf, "no") == 0)
			retval = 0;

		if (retval != -1) {
			if (f != stdin)
				fclose(f);
			return retval;
		}
	}
}

/*
 * check whether the supplied host key is valid, return only if ok.
 */

static void
check_host_key(char *host, struct sockaddr *hostaddr, Key *host_key,
	const char *user_hostfile, const char *system_hostfile)
{
	Key *file_key;
	char *type = key_type(host_key);
	char *ip = NULL;
	char hostline[1000], *hostp;
	HostStatus host_status;
	HostStatus ip_status;
	int local = 0, host_ip_differ = 0;
	int salen;
	char ntop[NI_MAXHOST];

	/*
	 * Force accepting of the host key for loopback/localhost. The
	 * problem is that if the home directory is NFS-mounted to multiple
	 * machines, localhost will refer to a different machine in each of
	 * them, and the user will get bogus HOST_CHANGED warnings.  This
	 * essentially disables host authentication for localhost; however,
	 * this is probably not a real problem.
	 */
	/**  hostaddr == 0! */
	switch (hostaddr->sa_family) {
	case AF_INET:
		local = (ntohl(((struct sockaddr_in *)hostaddr)->sin_addr.s_addr) >> 24) == IN_LOOPBACKNET;
		salen = sizeof(struct sockaddr_in);
		break;
	case AF_INET6:
		local = IN6_IS_ADDR_LOOPBACK(&(((struct sockaddr_in6 *)hostaddr)->sin6_addr));
		salen = sizeof(struct sockaddr_in6);
		break;
	default:
		local = 0;
		salen = sizeof(struct sockaddr_storage);
		break;
	}
	if (local) {
		debug("Forcing accepting of host key for loopback/localhost.");
		return;
	}

	/*
	 * Turn off check_host_ip for proxy connects, since
	 * we don't have the remote ip-address
	 */
	if (options.proxy_command != NULL && options.check_host_ip)
		options.check_host_ip = 0;

	if (options.check_host_ip) {
		if (getnameinfo(hostaddr, salen, ntop, sizeof(ntop),
		    NULL, 0, NI_NUMERICHOST) != 0)
			fatal("check_host_key: getnameinfo failed");
		ip = xstrdup(ntop);
	}

	/*
	 * Store the host key from the known host file in here so that we can
	 * compare it with the key for the IP address.
	 */
	file_key = key_new(host_key->type);

	/*
	 * Check if the host key is present in the user\'s list of known
	 * hosts or in the systemwide list.
	 */
	host_status = check_host_in_hostfile(user_hostfile, host, host_key, file_key);
	if (host_status == HOST_NEW)
		host_status = check_host_in_hostfile(system_hostfile, host, host_key, file_key);
	/*
	 * Also perform check for the ip address, skip the check if we are
	 * localhost or the hostname was an ip address to begin with
	 */
	if (options.check_host_ip && !local && strcmp(host, ip)) {
		Key *ip_key = key_new(host_key->type);
		ip_status = check_host_in_hostfile(user_hostfile, ip, host_key, ip_key);

		if (ip_status == HOST_NEW)
			ip_status = check_host_in_hostfile(system_hostfile, ip, host_key, ip_key);
		if (host_status == HOST_CHANGED &&
		    (ip_status != HOST_CHANGED || !key_equal(ip_key, file_key)))
			host_ip_differ = 1;

		key_free(ip_key);
	} else
		ip_status = host_status;

	key_free(file_key);

	switch (host_status) {
	case HOST_OK:
		/* The host is known and the key matches. */
		debug("Host '%.200s' is known and matches the %s host key.",
		    host, type);
		if (options.check_host_ip) {
			if (ip_status == HOST_NEW) {
				if (!add_host_to_hostfile(user_hostfile, ip, host_key))
					log("Failed to add the %s host key for IP address '%.30s' to the list of known hosts (%.30s).",
					    type, ip, user_hostfile);
				else
					log("Warning: Permanently added the %s host key for IP address '%.30s' to the list of known hosts.",
					    type, ip);
			} else if (ip_status != HOST_OK)
				log("Warning: the %s host key for '%.200s' differs from the key for the IP address '%.30s'",
				    type, host, ip);
		}
		break;
	case HOST_NEW:
		/* The host is new. */
		if (options.strict_host_key_checking == 1) {
			/* User has requested strict host key checking.  We will not add the host key
			   automatically.  The only alternative left is to abort. */
			fatal("No %s host key is known for %.200s and you have requested strict checking.", type, host);
		} else if (options.strict_host_key_checking == 2) {
			/* The default */
			char prompt[1024];
			char *fp = key_fingerprint(host_key);
			snprintf(prompt, sizeof(prompt),
			    "The authenticity of host '%.200s' can't be established.\n"
			    "%s key fingerprint is %s.\n"
			    "Are you sure you want to continue connecting (yes/no)? ",
			    host, type, fp);
			if (!read_yes_or_no(prompt, -1))
				fatal("Aborted by user!\n");
		}
		if (options.check_host_ip && ip_status == HOST_NEW && strcmp(host, ip)) {
			snprintf(hostline, sizeof(hostline), "%s,%s", host, ip);
			hostp = hostline;
		} else
			hostp = host;

		/* If not in strict mode, add the key automatically to the local known_hosts file. */
		if (!add_host_to_hostfile(user_hostfile, hostp, host_key))
			log("Failed to add the host to the list of known hosts (%.500s).",
			    user_hostfile);
		else
			log("Warning: Permanently added '%.200s' (%s) to the list of known hosts.",
			    hostp, type);
		break;
	case HOST_CHANGED:
		if (options.check_host_ip && host_ip_differ) {
			char *msg;
			if (ip_status == HOST_NEW)
				msg = "is unknown";
			else if (ip_status == HOST_OK)
				msg = "is unchanged";
			else
				msg = "has a different value";
			error("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
			error("@       WARNING: POSSIBLE DNS SPOOFING DETECTED!          @");
			error("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
			error("The %s host key for %s has changed,", type, host);
			error("and the key for the according IP address %s", ip);
			error("%s. This could either mean that", msg);
			error("DNS SPOOFING is happening or the IP address for the host");
			error("and its host key have changed at the same time");
		}
		/* The host key has changed. */
		error("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		error("@    WARNING: REMOTE HOST IDENTIFICATION HAS CHANGED!     @");
		error("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		error("IT IS POSSIBLE THAT SOMEONE IS DOING SOMETHING NASTY!");
		error("Someone could be eavesdropping on you right now (man-in-the-middle attack)!");
		error("It is also possible that the %s host key has just been changed.", type);
		error("Please contact your system administrator.");
		error("Add correct host key in %.100s to get rid of this message.",
		      user_hostfile);

		/*
		 * If strict host key checking is in use, the user will have
		 * to edit the key manually and we can only abort.
		 */
		if (options.strict_host_key_checking)
			fatal("%s host key for %.200s has changed and you have requested strict checking.", type, host);

		/*
		 * If strict host key checking has not been requested, allow
		 * the connection but without password authentication or
		 * agent forwarding.
		 */
		if (options.password_authentication) {
			error("Password authentication is disabled to avoid trojan horses.");
			options.password_authentication = 0;
		}
		if (options.forward_agent) {
			error("Agent forwarding is disabled to avoid trojan horses.");
			options.forward_agent = 0;
		}
		/*
		 * XXX Should permit the user to change to use the new id.
		 * This could be done by converting the host key to an
		 * identifying sentence, tell that the host identifies itself
		 * by that sentence, and ask the user if he/she whishes to
		 * accept the authentication.
		 */
		break;
	}
	if (options.check_host_ip)
		xfree(ip);
}

/*
 * Starts a dialog with the server, and authenticates the current user on the
 * server.  This does not need any extra privileges.  The basic connection
 * to the server must already have been established before this is called.
 * If login fails, this function prints an error and never returns.
 * This function does not require super-user privileges.
 */
static void
ssh_login(int host_key_valid, RSA *own_host_key, const char *orighost,
    struct sockaddr *hostaddr, uid_t original_real_uid)
{
	struct passwd *pw;
	char *host, *cp;
	char *server_user, *local_user;

	/* Get local user name.  Use it as server user if no user name was given. */
	pw = getpwuid(original_real_uid);
	if (!pw)
		fatal("User id %u not found from user database.", original_real_uid);
	local_user = xstrdup(pw->pw_name);
	server_user = options.user ? options.user : local_user;

	/* Convert the user-supplied hostname into all lowercase. */
	host = xstrdup(orighost);
	for (cp = host; *cp; cp++)
		if (isupper(*cp))
			*cp = tolower(*cp);

	/* Exchange protocol version identification strings with the server. */
	ssh_exchange_identification();

	/* Put the connection into non-blocking mode. */
	packet_set_nonblocking();

	/* key exchange */
	/* authenticate user */
	if (compat20) {
		ssh_kex2(host, hostaddr);
		ssh_userauth2(server_user, host);
	} else {
		ssh_kex(host, hostaddr);
		ssh_userauth(local_user, server_user, host, host_key_valid, own_host_key);
	}
}
#endif /* SSH */
#endif /* CK_SSL */
