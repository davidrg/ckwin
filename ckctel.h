/* ckctel.h -- Symbol and macro definitions for C-Kermit telnet support */

/*
  Authors: Jeffrey Altman <jaltman@columbia.edu>,
           Frank da Cruz <fdc@columbia.edu>
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2000,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

  Notes:
  . Only one source file should include #defines for
    TELCMDS, TELOPTS, TELOPT_STATES, SLC_NAMES, and AUTH_NAMES.
  . This file should be used inplace of "arpa/telnet.h"
*/

#ifndef CKCTEL_H
#define	CKCTEL_H

/*
  Definitions for the TELNET protocol.
  can't rely on library header files for any of them.
*/
#ifndef IAC				/* First the telnet commands */
#define IAC 255
#endif /* IAC */
#ifndef DONT
#define DONT 254
#endif /* DONT */
#ifndef DO
#define DO 253
#endif /* DO */
#ifndef WONT
#define WONT 252
#endif /* WONT */
#ifndef WILL
#define WILL 251
#endif /* WILL */
#ifndef SB
#define SB 250
#endif /* SB */
#ifndef TN_GA
#define TN_GA 249
#endif /* TN_GA */
#ifndef TN_EL
#define TN_EL 248
#endif /* TN_EL */
#ifndef TN_EC
#define TN_EC 247
#endif /* TN_EC */
#ifndef TN_AYT
#define TN_AYT 246
#endif /* TN_AYT */
#ifndef TN_AO
#define TN_AO 245
#endif /* TN_AO */
#ifndef TN_IP
#define TN_IP 244
#endif /* TN_IP */
#ifndef BREAK
#define BREAK 243
#endif /* BREAK */
#ifndef TN_DM
#define TN_DM 242
#endif /* TN_DM */
#ifndef TN_NOP
#define TN_NOP 241
#endif /* TN_NOP */
#ifndef SE
#define SE 240
#endif /* SE */
#ifndef TN_EOR
#define TN_EOR 239
#endif /* TN_EOR */
#ifndef TN_ABORT
#define TN_ABORT 238
#endif /* TN_ABORT */
#ifndef TN_SUSP
#define TN_SUSP 237
#endif /* TN_SUSP */
#ifndef TN_EOF
#define TN_EOF 236
#endif /* TN_EOF */
#ifndef LAST_TN_CMD
#define LAST_TN_CMD 236

#define TN_SAK 200              /* IBM Secure Attention Key */
#endif /* LAST_TN_CMD */

#define SYNCH	242		/* for telfunc calls */

#ifdef TELCMDS
char *telcmds[] = {
	"EOF", "SUSP", "ABORT", "EOR",
	"SE", "NOP", "DMARK", "BRK", "IP", "AO", "AYT", "EC",
	"EL", "GA", "SB", "WILL", "WONT", "DO", "DONT", "IAC", 0
};
#else /* TELCMDS */
extern char *telcmds[];
#endif /* TELCMDS */

#define	TELCMD_FIRST	TN_EOF
#define	TELCMD_LAST	IAC
#define	TELCMD_OK(x)	((unsigned int)(x) <= TELCMD_LAST && \
			 (unsigned int)(x) >= TELCMD_FIRST || \
                          (unsigned int)(x) == TN_SAK)
#define	TELCMD(x)	(TELCMD_OK(x)? ((x) == TN_SAK?"SAK": \
                         telcmds[(x)-TELCMD_FIRST]):"UNKNOWN")

/* Then the options */
/* NB: the following platforms have TELOPT_AUTHENTICATION defined as */
/* 45 instead of 37.                                                 */
#ifdef TELOPT_AUTHENTICATION
#ifdef __osf__
#undef TELOPT_AUTHENTICATION
#endif /* __osf__ */
#ifndef IRIX
#undef TELOPT_AUTHENTICATION
#endif /* IRIX */
#ifndef ultrix
#undef TELOPT_AUTHENTICATION
#endif /* ultrix */
#endif /* TELOPT_AUTHENTICATION */

/* telnet options */
#ifndef TELOPT_BINARY
#define TELOPT_BINARY	0	/* 8-bit data path (RFC 856)*/
#endif
#ifndef TELOPT_ECHO
#define TELOPT_ECHO	1	/* echo (RFC 857)*/
#endif
#ifndef TELOPT_RCP
#define	TELOPT_RCP	2	/* prepare to reconnect (NIC 50005)*/
#endif
#ifndef	TELOPT_SGA
#define	TELOPT_SGA	3	/* suppress go ahead (RFC 858) */
#endif
#ifndef	TELOPT_NAMS
#define	TELOPT_NAMS	4	/* approximate message size (ETHERNET) */
#endif
#ifndef	TELOPT_STATUS
#define	TELOPT_STATUS	5	/* give status (RFC 859) */
#endif
#ifndef	TELOPT_TM
#define	TELOPT_TM	6	/* timing mark (RFC 860) */
#endif
#ifndef	TELOPT_RCTE
#define	TELOPT_RCTE	7	/* remote controlled transmission and echo */
#endif                          /* (RFC 726) */
#ifndef TELOPT_NAOL
#define TELOPT_NAOL 	8	/* negotiate about output line width */
#endif                          /* (NIC 50005) */
#ifndef TELOPT_NAOP
#define TELOPT_NAOP 	9	/* negotiate about output page size */
#endif                          /* (NIC 50005) */
#ifndef TELOPT_NAOCRD
#define TELOPT_NAOCRD	10	/* negotiate about CR disposition (RFC 652) */
#endif                          /* [Historic] */
#ifndef TELOPT_NAOHTS
#define TELOPT_NAOHTS	11	/* negotiate about horizontal tabstops */
#endif                          /* (RFC 653) [Historic] */
#ifndef TELOPT_NAOHTD
#define TELOPT_NAOHTD	12	/* negotiate about horiz tab disposition */
#endif                          /* (RFC 654) [Historic] */
#ifndef TELOPT_NAOFFD
#define TELOPT_NAOFFD	13	/* negotiate about formfeed disposition */
#endif                          /* (RFC 655) [Historic] */
#ifndef TELOPT_NAOVTS
#define TELOPT_NAOVTS	14	/* negotiate about vertical tab stops */
#endif                          /* (RFC 656) [Historic] */
#ifndef TELOPT_NAOVTD
#define TELOPT_NAOVTD	15	/* negotiate about vertical tab disposition */
#endif                          /* (RFC 657) [Historic] */
#ifndef TELOPT_NAOLFD
#define TELOPT_NAOLFD	16	/* negotiate about output LF disposition */
#endif                          /* (RFC 658) [Historic] */
#ifndef TELOPT_XASCII
#define TELOPT_XASCII	17	/* extended ascii character set */
#endif                          /* (RFC 698) */
#ifndef	TELOPT_LOGOUT
#define	TELOPT_LOGOUT	18	/* force logout (RFC 727) */
#endif
#ifndef	TELOPT_BM
#define	TELOPT_BM	19	/* byte macro (RFC 735) */
#endif
#ifndef	TELOPT_DET
#define	TELOPT_DET	20	/* data entry terminal (RFC 1043, 732) */
#endif
#ifndef	TELOPT_SUPDUP
#define	TELOPT_SUPDUP	21	/* supdup protocol (RFC 736, 734) */
#endif
#ifndef	TELOPT_SUPDUPOUTPUT
#define	TELOPT_SUPDUPOUTPUT 22	/* supdup output (RFC 749) */
#endif
#ifndef	TELOPT_SNDLOC
#define	TELOPT_SNDLOC	23	/* send location (RFC 779) */
#endif
#ifndef	TELOPT_TTYPE
#define	TELOPT_TTYPE	24	/* terminal type (RFC 1091) */
#endif
#ifndef	TELOPT_EOR
#define	TELOPT_EOR	25	/* end or record (RFC 885) */
#endif
#ifndef	TELOPT_TUID
#define	TELOPT_TUID	26	/* TACACS user identification (RFC 927) */
#endif
#ifndef	TELOPT_OUTMRK
#define	TELOPT_OUTMRK	27	/* output marking (RFC 933) */
#endif
#ifndef	TELOPT_TTYLOC
#define	TELOPT_TTYLOC	28	/* terminal location number (RFC 946) */
#endif
#ifndef	TELOPT_3270REGIME
#define	TELOPT_3270REGIME 29	/* 3270 regime (RFC 1041) */
#endif
#ifndef	TELOPT_X3PAD
#define	TELOPT_X3PAD	30	/* X.3 PAD (RFC 1053) */
#endif
#ifndef	TELOPT_NAWS
#define	TELOPT_NAWS	31	/* window size (RFC 1073) */
#endif
#ifndef	TELOPT_TSPEED
#define	TELOPT_TSPEED	32	/* terminal speed (RFC 1079) */
#endif
#ifndef	TELOPT_LFLOW
#define	TELOPT_LFLOW	33	/* remote flow control (RFC 1372) */
#endif
#ifndef TELOPT_LINEMODE
#define TELOPT_LINEMODE	34	/* Linemode option (RFC 1184) */
#endif
#ifndef TELOPT_XDISPLOC
#define TELOPT_XDISPLOC	35	/* X Display Location (RFC 1096) */
#endif
#ifndef TELOPT_OLD_ENVIRON
#define TELOPT_OLD_ENVIRON 36	/* Old - Environment variables (RFC 1408) */
#endif
#ifndef	TELOPT_AUTHENTICATION
#define	TELOPT_AUTHENTICATION 37/* Authenticate (RFC 1409) */
#endif
#ifndef	TELOPT_ENCRYPTION
#define	TELOPT_ENCRYPTION 38    /* Encryption option (Borman,Tso) */
#endif
#ifndef TELOPT_NEWENVIRON
#define TELOPT_NEWENVIRON 39    /* New - Environment variables (RFC 1572) */
#endif
#ifndef TELOPT_3270E
#define TELOPT_3270E    40      /* 3270 Extended (RFC 1647) */
#endif
#ifndef TELOPT_XAUTH
#define TELOPT_XAUTH    41      /* ??? (Earhart) */
#endif
#ifndef TELOPT_CHARSET
#define TELOPT_CHARSET  42      /* Character-set (RFC 2066) */
#endif
#ifndef TELOPT_RSP
#define TELOPT_RSP      43      /* Remote Serial Port (Barnes) */
#endif
#ifndef TELOPT_COM_PORT
#define TELOPT_COM_PORT 44      /* Com Port Control (RFC 2217) */
#endif
#ifndef TELOPT_SLE
#define TELOPT_SLE      45      /* Suppress Local Echo (Atmar) - rejected */
#endif
#ifndef TELOPT_START_TLS
#define TELOPT_START_TLS 46     /* Start TLS Authentication/Encryption */
#endif                          /* (Boe) */
#ifndef TELOPT_KERMIT
#define TELOPT_KERMIT   47      /* Kermit (altman) */
#endif
#ifndef TELOPT_SEND_URL
#define TELOPT_SEND_URL 48      /* Send URL */
#endif

#ifndef TELOPT_FORWARD_X
#define TELOPT_FORWARD_X 49     /* X Windows Forwarding (not yet allocated) */
#endif /* TELOPT_FORWARD_X */

#ifndef TELOPT_PRAGMA_LOGON
#define TELOPT_PRAGMA_LOGON 138 /* Encrypted Logon option (PragmaSys) */
#endif
#ifndef TELOPT_SSPI_LOGON
#define TELOPT_SSPI_LOGON 139   /* MS SSPI Logon option (PragmaSys) */
#endif
#ifndef TELOPT_PRAGMA_HEARTBEAT
				/* Server Send Heartbeat option (PragmaSys) */
#define TELOPT_PRAGMA_HEARTBEAT 140
#endif

#define TELOPT_IBM_SAK 200      /* IBM Secure Attention Key */

/*
  IBM Secure Attention Key (SAK) Option

  In addition to terminal negotiation, the telnet command allows
  negotiation for the Secure Attention Key (SAK)
  option. This option, when supported, provides the local user with
  a secure communication path to the remote
  host for tasks such as changing user IDs or passwords. If the remote
  host supports the SAK function, a trusted
  shell is opened on the remote host when the telnet send sak
  subcommand is issued. The SAK function can
  also be assigned to a single key available in telnet input mode,
  using the set sak subcommand.

  TN_SAK

  Sends the TELNET SAK (Secure Attention Key) sequence, which causes
  the remote system to invoke the trusted shell. If the SAK is not
  supported, then an error message is displayed that reads:
    Remote side does not support SAK.
*/

#ifndef	TELOPT_EXOPL
#define	TELOPT_EXOPL	255	/* Extended-options-list (RFC 861) */
#endif

#ifdef NTELOPTS
#undef NTELOPTS
#endif /* NTELOPTS */
/* The Telnet Option space is no longer being allocated by ICANN as a */
/* continuous list.  In other words it is becoming sparse.  But we do */
/* not want to have to allocate memory for a long list of strings and */
/* structs which will never be used.  Therefore, the NTELOPTS define  */
/* can no longer be equal to TELOPT_LAST+1.  In fact, the notion of   */
/* TELOPT_FIRST and TELOPT_LAST no longer make sense.                 */
#define	NTELOPTS        54

#define TELOPT_FIRST     TELOPT_BINARY
#define TELOPT_LAST      TELOPT_IBM_SAK

/*
  The following macros speed us up at runtime but are too complex
  for some preprocessors / compilers; if your compiler bombs on ckctel.c
  with "Instruction table overflow" or somesuch, rebuild with -DNOTOMACROS.
*/
#ifndef NOTOMACROS
#ifndef TELOPT_MACRO
#define TELOPT_MACRO
#endif /* TELOPT_MACRO */
#endif /* NOTOMACROS */

#ifdef TELOPT_MACRO
#define TELOPT_INDEX(x) (((x)>=0 && (x)<= TELOPT_FORWARD_X)?(x):\
        ((x)>=TELOPT_PRAGMA_LOGON && (x)<=TELOPT_PRAGMA_HEARTBEAT)?(x)-89: \
        ((x) == TELOPT_IBM_SAK)?(x)-148: NTELOPTS)

#define	TELOPT_OK(x)	(((x) >= TELOPT_BINARY && (x) <= TELOPT_FORWARD_X) ||\
             ((x) >= TELOPT_PRAGMA_LOGON && (x) <= TELOPT_PRAGMA_HEARTBEAT) ||\
             ((x) == TELOPT_IBM_SAK))
#define	TELOPT(x)	(TELOPT_OK(x)?telopts[TELOPT_INDEX(x)]:"UNKNOWN")
#else /* TELOPT_MACRO */
_PROTOTYP(int telopt_index,(int));
_PROTOTYP(int telopt_ok,(int));
_PROTOTYP(CHAR * telopt, (int));

#define TELOPT_INDEX(x) telopt_index(x)
#define TELOPT_OK(x)    telopt_ok(x)
#define TELOPT(x)       telopt(x)
#endif /* TELOPT_MACRO */

#ifdef TELOPTS
char *telopts[NTELOPTS+2] = {
/*   0 */ "BINARY",   "ECHO",         "RCP",  "SUPPRESS-GO-AHEAD",
/*   4 */ "NAME",   "STATUS", "TIMING-MARK", "RCTE",
/*   8 */ "NAOL", "NAOP",  "NAOCRD", "NAOHTS",
/*  12 */ "NAOHTD", "NAOFFD", "NAOVTS",  "NAOVTD",
/*  16 */ "NAOLFD", "EXTEND-ASCII", "LOGOUT", "BYTE-MACRO",
/*  20 */ "DATA-ENTRY-TERMINAL", "SUPDUP", "SUPDUP-OUTPUT",  "SEND-LOCATION",
/*  24 */ "TERMINAL-TYPE",  "END-OF-RECORD", "TACACS-UID", "OUTPUT-MARKING",
/*  28 */ "TTYLOC", "3270-REGIME", "X.3-PAD", "NAWS",
/*  32 */ "TSPEED", "LFLOW", "LINEMODE", "XDISPLOC",
/*  36 */ "OLD-ENVIRON", "AUTHENTICATION", "ENCRYPTION", "NEW-ENVIRONMENT",
/*  40 */ "TN3270E","XAUTH","CHARSET", "remote-serial-port",
/*  44 */ "COM-PORT-CONTROL","suppress-local-echo","start-tls","kermit",
/*  48 */ "send-url", "forward-X",
/* 138 */ "pragma-logon", "sspi-logon", "pragma-heartbeat",
/* 200 */ "ibm-sak",
          "unknown",
	0
};
#else /*TELOPTS */
extern char * telopts[];
#endif /* TELOPTS */

/* TELNET Newline Mode */

#define TNL_CR     0			/* CR sends bare carriage return */
#define TNL_CRNUL  1			/* CR and NUL */
#define TNL_CRLF   2			/* CR and LF */
#define TNL_LF     3			/* LF instead of CR */

/* TELNET Negotiation Mode */

#define    TN_NG_RF  0			/*  Negotiation REFUSED */
#define    TN_NG_AC  1			/*  Negotiation ACCEPTED */
#define    TN_NG_RQ  2			/*  Negotiation REQUESTED */
#define    TN_NG_MU  3			/*  Negotiation REQUIRED (must) */


/* Systems where we know we can define TELNET NAWS automatically. */

#ifndef CK_NAWS				/* In other words, if both */
#ifdef CK_TTGWSIZ			/* TNCODE and TTGWSIZ are defined */
#define CK_NAWS				/* then we can do NAWS. */
#endif /* CK_TTGWSIZ */
#endif /* CK_NAWS */

#ifdef CK_FORWARD_X
#define MAXFWDX 64                      /* Num of X windows to be fwd'd */
#endif /* MAXFWDX */

/* Telnet State structures and definitions */
struct _telopt_state {
  unsigned char def_server_me_mode;   /* Default Negotiation Mode */
  unsigned char def_server_u_mode;    /* Default Negotiation Mode */
  unsigned char def_client_me_mode;   /* Default Negotiation Mode */
  unsigned char def_client_u_mode;    /* Default Negotiation Mode */
  unsigned char me_mode;              /* Telnet Negotiation Mode */
  unsigned char u_mode;               /* Telnet Negotiation Mode */
  unsigned char me;                   /* Am I ?                  */
  unsigned char u;                    /* Are you?                */
  unsigned char unanswered_will;      /* Sent Will, Waiting for DO/DONT */
  unsigned char unanswered_do;        /* Send DO, Waiting for WILL/WONT */
  unsigned char unanswered_wont;      /* Sent WONT, Waiting for DONT */
  unsigned char unanswered_dont;      /* Sent DONT, Waiting for WONT */
  unsigned char unanswered_sb;        /* Sent SB,   Waiting for SB (server) */
  union {
#ifdef IKS_OPTION
    struct _telopt_kermit {         /* Kermit Option States */
      unsigned char me_start;       /* I have a Server active */
      unsigned char me_req_start;   /* Sent Req-Start, Waiting for response */
      unsigned char me_req_stop;    /* Sent Req-Stop, Waiting for response  */
      unsigned char u_start;        /* You have a Server active */
      unsigned char sop;            /* Have we received the SOP char? */
    } kermit;
#endif /* IKS_OPTION */
#ifdef CK_ENCRYPTION
    struct _telopt_encrypt {        /* Encryption Option States */
      unsigned char need_to_send;
      unsigned char  stop;          /* Is encryption stopped?   */
    } encrypt;
#endif /* CK_ENCRYPTION */
#ifdef CK_NAWS
    struct _telopt_naws {           /* NAWS Option Information  */
      unsigned char need_to_send;
      int x;                        /* Last Width               */
      int y;                        /* Last Height              */
    } naws;
#endif /* CK_NAWS */
#ifdef CK_SSL
    struct _telopt_start_tls {      /* Start TLS Option             */
       unsigned char u_follows;     /* u ready for TLS negotiation  */
       unsigned char me_follows;    /* me ready for TLS negotiation */
    } start_tls;
#endif /* CK_SSL */
    struct _telopt_term {          /* Terminal Type            */
       unsigned char need_to_send;
       unsigned char type[41];     /* Last terminal type       */
    } term;
#ifdef CK_ENVIRONMENT
    struct _telopt_new_env {
       unsigned char need_to_send;
       unsigned char * str;
       int             len;
    } env;
#ifdef CK_XDISPLOC
    struct _telopt_xdisp {
       unsigned char need_to_send;
    } xdisp;
#endif /* CK_XDISPLOC */
#endif /* CK_ENVIRONMENT */
#ifdef CK_SNDLOC
    struct _telopt_sndloc {
       unsigned char need_to_send;
    } sndloc;
#endif /* CK_SNDLOC */
#ifdef CK_FORWARD_X
    struct _telopt_fwd_x {
        unsigned char need_to_send;
        int listen_socket;
        struct _channel {
            int fd;
            int id;
        } channel[MAXFWDX];
#ifdef NT
        int thread_started;
#endif /* NT */
    } forward_x;
#endif /* CK_FORWARD_X */
    /* additional options such as New Environment or Send Location */
  } sb;
};
typedef struct _telopt_state telopt_state, *p_telopt_state;

/* telopt_states[] is the array of structs which the state of each telnet */
/* option is stored.  We allocate one more than we need in case we are    */
/* sent telnet options that we do not recognize.  If by some chance the   */
/* TELOPT_OK() check is skipped, TELOPT_INDEX() will force the option to  */
/* use the extra cell.                                                    */

#ifdef TELOPT_STATES
telopt_state telopt_states[NTELOPTS+1];
#else /* TELOPT_STATES */
extern telopt_state telopt_states[];
#endif /* TELOPT_STATES */

#define TELOPT_ME(x) (telopt_states[TELOPT_INDEX(x)].me)
#define TELOPT_U(x)  (telopt_states[TELOPT_INDEX(x)].u)
#define TELOPT_ME_MODE(x) \
        (telopt_states[TELOPT_INDEX(x)].me_mode)
#define TELOPT_U_MODE(x)  \
        (telopt_states[TELOPT_INDEX(x)].u_mode)
#define TELOPT_UNANSWERED_WILL(x) \
        (telopt_states[TELOPT_INDEX(x)].unanswered_will)
#define TELOPT_UNANSWERED_DO(x)   \
        (telopt_states[TELOPT_INDEX(x)].unanswered_do)
#define TELOPT_UNANSWERED_WONT(x) \
        (telopt_states[TELOPT_INDEX(x)].unanswered_wont)
#define TELOPT_UNANSWERED_DONT(x)   \
        (telopt_states[TELOPT_INDEX(x)].unanswered_dont)
#define TELOPT_UNANSWERED_SB(x)   \
        (telopt_states[TELOPT_INDEX(x)].unanswered_sb)
#define TELOPT_SB(x) \
        (telopt_states[TELOPT_INDEX(x)].sb)
#define TELOPT_DEF_S_ME_MODE(x) \
        (telopt_states[TELOPT_INDEX(x)].def_server_me_mode)
#define TELOPT_DEF_S_U_MODE(x)  \
        (telopt_states[TELOPT_INDEX(x)].def_server_u_mode)
#define TELOPT_DEF_C_ME_MODE(x) \
        (telopt_states[TELOPT_INDEX(x)].def_client_me_mode)
#define TELOPT_DEF_C_U_MODE(x)  \
        (telopt_states[TELOPT_INDEX(x)].def_client_u_mode)

#ifdef TELOPT_MODES
char * telopt_modes[4] = {
    "REFUSED", "ACCEPTED", "REQUESTED", "REQUIRED"
};
#else /* TELOPT_MODES */
extern char * telopt_modes[];
#endif /* TELOPT_MODES */

#ifdef TELOPT_MACRO
#define	TELOPT_MODE_OK(x) ((unsigned int)(x) <= TN_NG_MU)
#define	TELOPT_MODE(x) (TELOPT_MODE_OK(x)?telopt_modes[(x)-TN_NG_RF]:"UNKNOWN")
#else /* TELOPT_MACRO */
_PROTOTYP(int telopt_mode_ok,(int));
_PROTOTYP(CHAR * telopt_mode,(int));

#define TELOPT_MODE_OK(x) telopt_mode_ok(x)
#define TELOPT_MODE(x)    telopt_mode(x)
#endif /* TELOPT_MACRO */

/* Sub-option qualifiers */
#define	TELQUAL_IS	0	/* option is... */
#define	TELQUAL_SEND	1	/* send option */
#define	TELQUAL_INFO	2	/* ENVIRON: informational version of IS */
#define	TELQUAL_REPLY	2	/* AUTHENTICATION: client version of IS */
#define	TELQUAL_NAME	3	/* AUTHENTICATION: client version of IS */

#define TEL_ENV_VAR     0
#define TEL_ENV_VALUE   1
#define TEL_ENV_ESC     2
#define TEL_ENV_USERVAR 3

#define	LFLOW_OFF		0	/* Disable remote flow control */
#define	LFLOW_ON		1	/* Enable remote flow control */
#define	LFLOW_RESTART_ANY	2	/* Restart output on any char */
#define	LFLOW_RESTART_XON	3	/* Restart output only on XON */

/*
 * LINEMODE suboptions
 */

#define	LM_MODE		1
#define	LM_FORWARDMASK	2
#define	LM_SLC		3

#define	MODE_EDIT	0x01
#define	MODE_TRAPSIG	0x02
#define	MODE_ACK	0x04
#define MODE_SOFT_TAB	0x08
#define MODE_LIT_ECHO	0x10

#define	MODE_MASK	0x1f

/* Not part of protocol, but needed to simplify things... */
#define MODE_FLOW		0x0100
#define MODE_ECHO		0x0200
#define MODE_INBIN		0x0400
#define MODE_OUTBIN		0x0800
#define MODE_FORCE		0x1000

#define	SLC_SYNCH	1
#define	SLC_BRK		2
#define	SLC_IP		3
#define	SLC_AO		4
#define	SLC_AYT		5
#define	SLC_EOR		6
#define	SLC_ABORT	7
#define	SLC_EOF		8
#define	SLC_SUSP	9
#define	SLC_EC		10
#define	SLC_EL		11
#define	SLC_EW		12
#define	SLC_RP		13
#define	SLC_LNEXT	14
#define	SLC_XON		15
#define	SLC_XOFF	16
#define	SLC_FORW1	17
#define	SLC_FORW2	18
#define SLC_MCL         19
#define SLC_MCR         20
#define SLC_MCWL        21
#define SLC_MCWR        22
#define SLC_MCBOL       23
#define SLC_MCEOL       24
#define SLC_INSRT       25
#define SLC_OVER        26
#define SLC_ECR         27
#define SLC_EWR         28
#define SLC_EBOL        29
#define SLC_EEOL        30

#define	NSLC		30

/*
 * For backwards compatability, we define SLC_NAMES to be the
 * list of names if SLC_NAMES is not defined.
 */
#define	SLC_NAMELIST	"0", "SYNCH", "BRK", "IP", "AO", "AYT", "EOR", \
			"ABORT", "EOF", "SUSP", "EC", "EL", "EW", "RP", \
			"LNEXT", "XON", "XOFF", "FORW1", "FORW2", \
                        "MCL", "MCR", "MCWL", "MCWR", "MCBOL", "MCEOL", \
                        "INSRT", "OVER", "ECR", "EWR", "EBOL", "EEOL", 0
#ifdef	SLC_NAMES
char *slc_names[] = {
	SLC_NAMELIST
};
#else
extern char *slc_names[];
#define	SLC_NAMES SLC_NAMELIST
#endif

#define	SLC_NAME_OK(x)	((unsigned int)(x) <= NSLC)
#define SLC_NAME(x)	(SLC_NAME_OK(x)?slc_names[x]:"UNKNOWN")

#define	SLC_NOSUPPORT	0
#define	SLC_CANTCHANGE	1
#define	SLC_VARIABLE	2
#define	SLC_DEFAULT	3
#define	SLC_LEVELBITS	0x03

#define	SLC_FUNC	0
#define	SLC_FLAGS	1
#define	SLC_VALUE	2

#define	SLC_ACK		0x80
#define	SLC_FLUSHIN	0x40
#define	SLC_FLUSHOUT	0x20

#define	OLD_ENV_VAR	1
#define	OLD_ENV_VALUE	0
#define	NEW_ENV_VAR	0
#define	NEW_ENV_VALUE	1
#define	ENV_ESC		2
#define ENV_USERVAR	3

#define FWDX_SCREEN     0
#define FWDX_OPEN       1
#define FWDX_CLOSE      2
#define FWDX_DATA       3

/*
 * AUTHENTICATION suboptions
 */

/*
 * Who is authenticating who ...
 */
#define	AUTH_CLIENT_TO_SERVER	0	/* Client authenticating server */
#define	AUTH_SERVER_TO_CLIENT	1	/* Server authenticating client */
#define	AUTH_WHO_MASK		1

/*
 * amount of authentication done
 */
#define	AUTH_HOW_ONE_WAY	0
#define	AUTH_HOW_MUTUAL		2
#define	AUTH_HOW_MASK		2

/*
 * should we be encrypting?
 */
#define AUTH_ENCRYPT_OFF	     0
#define AUTH_ENCRYPT_USING_TELOPT    4
#define AUTH_ENCRYPT_AFTER_EXCHANGE 16
#define AUTH_ENCRYPT_RESERVED       20
#define AUTH_ENCRYPT_MASK	    20

/*
 * will we be forwarding?
 * if we want to activate the use of this flag then
 *   #define USE_INI_CRED_FWD
 */
#define INI_CRED_FWD_OFF	0
#define INI_CRED_FWD_ON		8
#define INI_CRED_FWD_MASK	8
#define USE_INI_CRED_FWD

#define	AUTHTYPE_NULL		0
#define	AUTHTYPE_KERBEROS_V4	1
#define	AUTHTYPE_KERBEROS_V5	2
#define	AUTHTYPE_SPX		3
#define	AUTHTYPE_MINK		4
#define AUTHTYPE_SRP            5
#define AUTHTYPE_RSA            6
#define AUTHTYPE_SSL            7
#define AUTHTYPE_LOKI          10
#define AUTHTYPE_SSA           11
#define AUTHTYPE_KEA_SJ        12
#define AUTHTYPE_KEA_INTEG     13
#define AUTHTYPE_DSS           14
#define AUTHTYPE_NTLM          15
#ifdef AUTHTYPE_CNT
#undef AUTHTYPE_CNT
#endif /* AUTHTYPE_CNT */
#define	AUTHTYPE_CNT	       16

/*
 * AUTHTYPEs Last updated 21 March 1999
 * from http://www.isi.edu/in-notes/iana/assignments/telnet-options
 */

#define	AUTHTYPE_AUTO		99

#ifdef	AUTH_NAMES
char *authtype_names[] = {
    "NULL",                     /* RFC 1416 */
    "KERBEROS_V4",              /* RFC 1416 */
    "KERBEROS_V5",              /* RFC 1416 */
    "SPX",                      /* RFC 1416 */
    "MINK",                     /* Unknown */
    "SRP",                      /* Wu */
    "RSA",                      /* RFC 1416 */
    "SSL",                      /* Hudson */
    "IANA_8",                   /* not assigned by IANA */
    "IANA_9",                   /* not assigned by IANA */
    "LOKI",                     /* RFC 1416 */
    "SSA",                      /* Schoch */
    "KEA_SJ",                   /* Horting */
    "KEA_SJ_INTEG",             /* Horting */
    "DSS",                      /* Horting */
    "NTLM",                     /* Kahn <louisk@microsoft.com> */
    0
};
char * authmode_names[] = {
    "CLIENT_TO_SERVER|ONE_WAY",
    "SERVER_TO_CLIENT|ONE_WAY",
    "CLIENT_TO_SERVER|MUTUAL",
    "SERVER_TO_CLIENT|MUTUAL",
    "CLIENT_TO_SERVER|ONE_WAY|ENCRYPT_USING_TELOPT",
    "SERVER_TO_CLIENT|ONE_WAY|ENCRYPT_USING_TELOPT",
    "CLIENT_TO_SERVER|MUTUAL|ENCRYPT_USING_TELOPT",
    "SERVER_TO_CLIENT|MUTUAL|ENCRYPT_USING_TELOPT",
    "CLIENT_TO_SERVER|ONE_WAY|CRED_FWD",
    "SERVER_TO_CLIENT|ONE_WAY|CRED_FWD",
    "CLIENT_TO_SERVER|MUTUAL|CRED_FWD",
    "SERVER_TO_CLIENT|MUTUAL|CRED_FWD",
    "CLIENT_TO_SERVER|ONE_WAY|ENCRYPT_USING_TELOPT|CRED_FWD",
    "SERVER_TO_CLIENT|ONE_WAY|ENCRYPT_USING_TELOPT|CRED_FWD",
    "CLIENT_TO_SERVER|MUTUAL|ENCRYPT_USING_TELOPT|CRED_FWD",
    "SERVER_TO_CLIENT|MUTUAL|ENCRYPT_USING_TELOPT|CRED_FWD",
    "CLIENT_TO_SERVER|ONE_WAY|ENCRYPT_AFTER_EXCHANGE",
    "SERVER_TO_CLIENT|ONE_WAY|ENCRYPT_AFTER_EXCHANGE",
    "CLIENT_TO_SERVER|MUTUAL|ENCRYPT_AFTER_EXCHANGE",
    "SERVER_TO_CLIENT|MUTUAL|ENCRYPT_AFTER_EXCHANGE",
    "CLIENT_TO_SERVER|ONE_WAY|ENCRYPT_RESERVED",
    "SERVER_TO_CLIENT|ONE_WAY|ENCRYPT_RESERVED",
    "CLIENT_TO_SERVER|MUTUAL|ENCRYPT_RESERVED",
    "SERVER_TO_CLIENT|MUTUAL|ENCRYPT_RESERVED",
    "CLIENT_TO_SERVER|ONE_WAY|ENCRYPT_AFTER_EXCHANGE|CRED_FWD",
    "SERVER_TO_CLIENT|ONE_WAY|ENCRYPT_AFTER_EXCHANGE|CRED_FWD",
    "CLIENT_TO_SERVER|MUTUAL|ENCRYPT_AFTER_EXCHANGE|CRED_FWD",
    "SERVER_TO_CLIENT|MUTUAL|ENCRYPT_AFTER_EXCHANGE|CRED_FWD",
    "CLIENT_TO_SERVER|ONE_WAY|ENCRYPT_RESERVED|CRED_FWD",
    "SERVER_TO_CLIENT|ONE_WAY|ENCRYPT_RESERVED|CRED_FWD",
    "CLIENT_TO_SERVER|MUTUAL|ENCRYPT_RESERVED|CRED_FWD",
    "SERVER_TO_CLIENT|MUTUAL|ENCRYPT_RESERVED|CRED_FWD",
    0
};
#else
extern char *authtype_names[];
extern char *authmode_names[];
#endif
#define AUTHMODE_CNT  32

#define	AUTHTYPE_NAME_OK(x)	((unsigned int)(x) < AUTHTYPE_CNT)
#define	AUTHTYPE_NAME(x)      (AUTHTYPE_NAME_OK(x)?authtype_names[x]:"UNKNOWN")

#define	AUTHMODE_NAME_OK(x)	((unsigned int)(x) < AUTHMODE_CNT)
#define	AUTHMODE_NAME(x)      (AUTHMODE_NAME_OK(x)?authmode_names[x]:"UNKNOWN")

/* Kerberos Authentication Message Identifiers */
#define	KRB_AUTH		0	/* Authentication data follows */
#define	KRB_REJECT		1	/* Rejected (reason might follow) */
#define	KRB_ACCEPT		2	/* Accepted */
#define KRB4_CHALLENGE		3
#define KRB4_RESPONSE		4
#define	KRB5_RESPONSE		3	/* Response for mutual auth. */
#define KRB5_FORWARD            4       /* Forwarded credentials follow */
#define KRB5_FORWARD_ACCEPT     5       /* Forwarded credentials accepted */
#define KRB5_FORWARD_REJECT     6       /* Forwarded credentials rejected */

/* Secure Remote Password Authentication Message Identifiers */
#define	SRP_AUTH		0	/* Authentication data follows */
#define	SRP_REJECT		1	/* Rejected (reason might follow) */
#define	SRP_ACCEPT		2	/* Accepted */
#define SRP_CHALLENGE		3
#define SRP_RESPONSE		4
#define SRP_EXP                 8       /* */
#define SRP_PARAMS              9       /* */

/* Telnet Auth using KEA and SKIPJACK */

#define KEA_CERTA_RA              1
#define KEA_CERTB_RB_IVB_NONCEB   2
#define KEA_IVA_RESPONSEB_NONCEA  3
#define KEA_RESPONSEA             4

/* Tim Hudson's SSL Authentication Message Identifiers */
#define SSL_START     1
#define SSL_ACCEPT    2
#define SSL_REJECT    3

/* Microsoft NTLM Authentication Message Identifiers */
#define NTLM_AUTH   0
#define NTLM_CHALLENGE 1
#define NTLM_RESPONSE  2
#define NTLM_ACCEPT 3
#define NTLM_REJECT 4

/* Generic Constants */
#define AUTH_SUCCESS    0
#define AUTH_FAILURE    255

/*
 * ENCRYPTion suboptions
 */
#define	ENCRYPT_IS		0	/* I pick encryption type ... */
#define	ENCRYPT_SUPPORT		1	/* I support encryption types ... */
#define	ENCRYPT_REPLY		2	/* Initial setup response */
#define	ENCRYPT_START		3	/* Am starting to send encrypted */
#define	ENCRYPT_END		4	/* Am ending encrypted */
#define	ENCRYPT_REQSTART	5	/* Request you start encrypting */
#define	ENCRYPT_REQEND		6	/* Request you send encrypting */
#define	ENCRYPT_ENC_KEYID	7
#define	ENCRYPT_DEC_KEYID	8
#define	ENCRYPT_CNT		9

#define	ENCTYPE_ANY		0
#define	ENCTYPE_DES_CFB64	1
#define	ENCTYPE_DES_OFB64	2
#define ENCTYPE_DES3_CFB64      3
#define ENCTYPE_DES3_OFB64      4
#define ENCTYPE_CAST5_40_CFB64  8
#define ENCTYPE_CAST5_40_OFB64  9
#define ENCTYPE_CAST128_CFB64   10
#define ENCTYPE_CAST128_OFB64   11
#ifdef ENCTYPE_CNT
#undef ENCTYPE_CNT
#endif
#define ENCTYPE_CNT             12

#ifdef	ENCRYPT_NAMES
char *encrypt_names[] = {
	"IS", "SUPPORT", "REPLY", "START", "END",
	"REQUEST-START", "REQUEST-END", "ENC-KEYID", "DEC-KEYID",
	0
};
char *enctype_names[] = {
    "ANY",
    "DES_CFB64",                /* Ts'o */
    "DES_OFB64",                /* Ts'o */
    "DES3_CFB64",               /* Altman */
    "DES3_OFB64",               /* Altman */
    "UNKNOWN-5",
    "UNKNOWN-6",
    "UNKNOWN-7",
    "CAST5_40_CFB64",           /* Wu */
    "CAST5_40_OFB64",           /* Wu */
    "CAST128_CFB64",            /* Wu */
    "CAST128_OFB64",            /* Wu */
    0
};
#else
extern char *encrypt_names[];
extern char *enctype_names[];
#endif

#define	ENCRYPT_NAME_OK(x)	((unsigned int)(x) < ENCRYPT_CNT)
#define	ENCRYPT_NAME(x)		encrypt_names[x]

#define	ENCTYPE_NAME_OK(x)	((unsigned int)(x) < ENCTYPE_CNT)
#define	ENCTYPE_NAME(x)		enctype_names[x]

/* For setting the state of validUser */

#define AUTH_REJECT     0       /* Rejected */
#define AUTH_UNKNOWN    1       /* We don't know who he is, but he's okay */
#define AUTH_OTHER      2       /* We know him, but not his name */
#define AUTH_USER       3       /* We know he name */
#define AUTH_VALID      4       /* We know him, and he needs no password */

/* Kermit Option Subnegotiations */

#define KERMIT_START      0
#define KERMIT_STOP       1
#define KERMIT_REQ_START  2
#define KERMIT_REQ_STOP   3
#define KERMIT_SOP        4
#define KERMIT_RESP_START 8
#define KERMIT_RESP_STOP  9

/* For SET TELNET AUTH HOW  */
#define TN_AUTH_HOW_ANY     0
#define TN_AUTH_HOW_ONE_WAY 1
#define TN_AUTH_HOW_MUTUAL  2

/* For SET TELNET AUTH ENCRYPT */
#define TN_AUTH_ENC_ANY     0
#define TN_AUTH_ENC_NONE    1
#define TN_AUTH_ENC_TELOPT  2
#define TN_AUTH_ENC_EXCH    3  /* not used in Kermit */

/* Telnet protocol functions defined in C-Kermit */

_PROTOTYP( int tn_ini, (void) );	/* Telnet protocol support */
_PROTOTYP( int tn_reset, (void));
_PROTOTYP( int tn_set_modes, (void));
_PROTOTYP( int tn_sopt, (int, int) );
_PROTOTYP( int tn_doop, (CHAR, int, int (*)(int) ) );
_PROTOTYP( int tn_sttyp, (void) );
_PROTOTYP( int tn_snenv, (CHAR *, int) ) ;
_PROTOTYP( int tn_rnenv, (CHAR *, int) ) ;
_PROTOTYP( int tn_wait, (char *) ) ;
_PROTOTYP( int tn_push, (void) ) ;
_PROTOTYP( int tnsndbrk, (void) );
_PROTOTYP( VOID tn_debug, (char *));
#ifdef IKS_OPTION
_PROTOTYP( int tn_siks, (int) );
_PROTOTYP( int iks_wait, (int, int) );
#endif /* IKS_OPTION */
#ifdef CK_NAWS
_PROTOTYP( int tn_snaws, (void) );
#endif /* CK_NAWS */
#ifdef CK_XDISPLOC
_PROTOTYP( int tn_sxdisploc, (void) );
#endif /* CK_XDISPLOC */
#ifdef CK_SNDLOC
_PROTOTYP( int tn_sndloc, (void) );
#endif /* CK_SNDLOC */
#ifdef CK_FORWARD_X
_PROTOTYP( int fwdx_create_listen_socket,(int));
_PROTOTYP( int fwdx_open_client_channel,(int));
_PROTOTYP( int fwdx_open_server_channel,(VOID));
_PROTOTYP( int fwdx_close_channel,(int));
_PROTOTYP( int fwdx_write_data_to_channel,(int, char *,int));
_PROTOTYP( int fwdx_send_data_from_channel,(int, char *,int));
_PROTOTYP( int fwdx_close_all,(VOID));
_PROTOTYP( int fwdx_tn_sb,(unsigned char *, int));
_PROTOTYP( int tn_sndfwdx, (void));
_PROTOTYP( int fwdx_send_close,(int));
_PROTOTYP( int fwdx_send_open,(int));
_PROTOTYP( VOID fwdx_check_sockets,(fd_set *));
_PROTOTYP( int fwdx_init_fd_set,(fd_set *));
#ifdef NT
_PROTOTYP( VOID fwdx_thread,(VOID *));
#endif /* NT */
#endif /* CK_FORWARD_X */

#ifndef CKCTEL_C			/* These are declared in ckctel.c */
extern int tn_init;                     /* Telnet protocol initialized flag */
extern char *tn_term;			/* Terminal type override */
extern int sstelnet;                    /* Server side telnet? */
extern int tn_deb;			/* Telnet option debugging flag */
#endif /* CKCTEL_C */
#endif /* CKCTEL_H */
