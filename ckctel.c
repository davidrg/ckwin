char *cktelv = "Telnet support, 7.0.191, 30 Dec 1999";
#define CKCTEL_C

int sstelnet = 0;                       /* Do server-side Telnet negotiation */

/*  C K C T E L  --  Telnet support  */

/*
  Authors:
    Telnet protocol by Frank da Cruz and Jeffrey Altman.
    Other contributions as indicated in the code.

  Copyright (C) 1985, 2000,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/*
  NOTE TO CONTRIBUTORS: This file, and all the other shared (ckc and cku)
  C-Kermit source files, must be compatible with C preprocessors that support
  only #ifdef, #else, #endif, #define, and #undef.  Please do not use #if,
  logical operators, or other preprocessor features in this module.  Also,
  don't use any ANSI C constructs except within #ifdef CK_ANSIC..#endif.
*/

#include "ckcsym.h"
#include "ckcdeb.h"

#include "ckcker.h"
#define TELCMDS                         /* to define name array */
#define TELOPTS                         /* to define name array */
#define SLC_NAMES                       /* to define name array */
#define ENCRYPT_NAMES
#define AUTH_NAMES
#define TELOPT_STATES
#define TELOPT_MODES
#include "ckcnet.h"
#include "ckctel.h"
#ifdef CK_SSL
#include "ck_ssl.h"
#endif /* CK_SSL */

#ifdef TNCODE

#ifdef OS2                              /* For terminal type name string */
#include "ckuusr.h"
#ifndef NT
#include <os2.h>
#undef COMMENT
#endif /* NT */
#include "ckocon.h"
extern int tt_type, max_tt;
extern struct tt_info_rec tt_info[];
extern char ttname[];
#endif /* OS2 */

#ifdef OS2
#include <assert.h>
#ifdef NT
#include <setjmpex.h>
#else /* NT */
#include <setjmp.h>
#endif /* NT */
#include <signal.h>
#include "ckcsig.h"
#endif /* OS2 */

#define HEXDISP

#ifdef CK_NAWS                          /* Negotiate About Window Size */
#ifdef RLOGCODE
_PROTOTYP( int rlog_naws, (void) );
#endif /* RLOGCODE */
#endif /* CK_NAWS */

int tn_init = 0;                        /* Telnet protocol initialized flag */
static int tn_first = 1;                /* First time init flag */
extern int tn_exit;                     /* Exit on disconnect */
extern int inserver;                    /* Running as IKSD */
char *tn_term = NULL;                   /* Terminal type override */

#ifdef CK_SNDLOC
char *tn_loc = NULL;                    /* Location override */
#endif /* CK_SNDLOC */
int tn_nlm = TNL_CRLF;                  /* Telnet CR -> CR LF mode */
int tn_b_nlm = TNL_CR;                  /* Telnet Binary CR RAW mode */
int tn_b_meu = 0;                       /* Telnet Binary ME means U too */
int tn_b_ume = 0;                       /* Telnet Binary U means ME too */
int tn_wait_flg = 1;                    /* Telnet Wait for Negotiations */
int tn_infinite = 0;                    /* Telnet Bug Infinite-Loop-Check */
int tn_rem_echo = 1;                    /* We will echo if WILL ECHO */
int tn_b_xfer = 0;                      /* Telnet Binary for Xfers? */
int tn_sb_bug = 1;                      /* Telnet BUG - SB w/o WILL or DO */
int tn_no_encrypt_xfer = 0;             /* Turn off Telnet Encrypt? */
int tn_auth_how = TN_AUTH_HOW_ANY;
int tn_auth_enc = TN_AUTH_ENC_ANY;
int tn_deb = 0;                         /* Telnet Debug mode */

#ifdef OS2
int ttnum = -1;                         /* Last Telnet Terminal Type sent */
int ttnumend = 0;                       /* Has end of list been found */
#endif /* OS2 */

#define TN_MSG_LEN 8196
char tn_msg[TN_MSG_LEN];                /* Telnet data can be rather long */
char hexbuf[TN_MSG_LEN];

/*
  In order to prevent an infinite telnet negotiation loop we maintain a
  count of the number of times the same telnet negotiation message is
  sent. When this count hits MAXTNCNT, we do not send any more of the
  message. The count is stored in the tncnts[][] array.

  The tncnts[][] array is indexed by negotiation option (SUPPRESS GO AHEAD,
  TERMINAL TYPE, NAWS, etc. - see the tnopts[] array) and the four
  negotiation message types (WILL, WONT, DO, DONT).  All telnet negotiations
  are kept track of in this way.

  The count for a message is zeroed when the "opposite" message is sent.
  WILL is the opposite of WONT, and DO is the opposite of DONT.
  For example sending "WILL SGA" increments tncnts[TELOPT_SGA][0]
  and zeroes tncnts[TELOPT_SGA][1].

  The code that does this is in tn_sopt().

  rogersh@fsj.co.jp, 18/3/1995

  8/16/1998 - with the recent rewrite of the telnet state machine I don't
  think this code is necessary anymore.  However, it can't do any harm so
  I am leaving it in.    - Jeff

  12/28/1998 - all references to tncnts[] must be done with TELOPT_INDEX(opt)
  because the Telnet option list is no longer contiguous.  We also must
  allocate NTELOPTS + 1 because the TELOPT_INDEX() macro returns NTELOPTS
  for an invalid option number.
*/

#define MAXTNCNT 4      /* Permits 4 intermediate telnet firewalls/gateways */

char tncnts[NTELOPTS+1][4];             /* Counts */
char tnopps[4] = { 1,0,3,2 };           /* Opposites */

#ifdef CK_ENVIRONMENT
#define TSBUFSIZ 1024
char tn_env_acct[64];
char tn_env_disp[64];
char tn_env_job[64];
char tn_env_prnt[64];
char tn_env_sys[64];
int tn_env_flg = 1;
#else /* CK_ENVIRONMENT */
#define TSBUFSIZ 41
int tn_env_flg = 0;
#endif /* CK_ENVIRONMENT */

#ifndef NOSIGWINCH
#ifdef CK_NAWS                          /* Window size business */
#ifdef UNIX
#include <signal.h>
#endif /* UNIX */
#endif /* CK_NAWS */
#endif /* NOSIGWINCH */

unsigned char sb[TSBUFSIZ+8];           /* Buffer for subnegotiations */

int tn_duplex = 1;                      /* Local echo */

extern char uidbuf[];                   /* User ID buffer */
extern int quiet, ttnet, ttnproto, debses, what, duplex;
extern int seslog, sessft, whyclosed;
#ifdef OS2
extern int tt_rows[], tt_cols[];
extern int tt_status;
extern int scrninitialized[];
#else /* OS2 */
extern int tt_rows, tt_cols;            /* Everybody has this */
#endif /* OS2 */
extern int cmd_cols, cmd_rows;
extern char namecopy[];
extern char myipaddr[];             /* Global copy of my IP address */

int sw_armed = 0;                       /* SIGWINCH armed flag */

#ifndef NOSIGWINCH
#ifdef CK_NAWS                          /* Window size business */
#ifdef SIGWINCH
#ifdef UNIX

SIGTYP
winchh(foo) int foo; {
    int x = 0;
#ifdef CK_TTYFD
#ifndef VMS
    extern int ttyfd;
#endif /* VMS */
#endif /* CK_TTYFD */
    debug(F100,"SIGWINCH caught","",0);
    signal(SIGWINCH,winchh);            /* Re-arm the signal */
#ifdef CK_TTYFD
    if
#ifdef VMS
      (vmsttyfd() == -1)
#else
      (ttyfd == -1)
#endif /* VMS */
#else
      (!local)
#endif /* CK_TTYFD */
	return;

    x = ttgwsiz();                      /* Get new window size */
/*
  This should be OK.  It might seem that sending this from
  interrupt level could interfere with another TELNET IAC string
  that was in the process of being sent.  But we always send
  TELNET strings with a single write(), which should prevent mixups.
*/
    if (x > 0 && tt_rows > 0 && tt_cols > 0) {
        tn_snaws();
#ifdef RLOGCODE
        rlog_naws();
#endif /* RLOGCODE */
    }
    return;
}
#endif /* UNIX */
#endif /* SIGWINCH */
#endif /* CK_NAWS */
#endif /* NOSIGWINCH */

#ifndef TELOPT_MACRO
int
telopt_index(opt) int opt; {
    if (opt >= 0 && opt <= TELOPT_FORWARD_X)
      return(opt);
    else if (opt >= TELOPT_PRAGMA_LOGON && opt <= TELOPT_PRAGMA_HEARTBEAT)
      return(opt-89);
    else if (opt == TELOPT_IBM_SAK)
      return(opt-148);
    else
      return(NTELOPTS);
}

int
telopt_ok(opt) int opt; {
    return((opt >= TELOPT_BINARY && opt <= TELOPT_FORWARD_X) ||
           (opt >= TELOPT_PRAGMA_LOGON && opt <= TELOPT_PRAGMA_HEARTBEAT) ||
           (opt == TELOPT_IBM_SAK));
}

CHAR *
telopt(opt) int opt; {
    if (telopt_ok(opt))
      return((CHAR *)telopts[telopt_index(opt)]);
    else
      return((CHAR *)"UNKNOWN");
}

int
telopt_mode_ok(opt) int opt; {
    return((unsigned int)(opt) <= TN_NG_MU);
}

CHAR *
telopt_mode(opt) int opt; {
    if (telopt_mode_ok(opt))
      return((CHAR *)telopt_modes[opt-TN_NG_RF]);
    else
      return((CHAR *)"UNKNOWN");
}
#endif /* TELOPT_MACRO */

static int
tn_outst(notquiet) int notquiet; {
    int outstanding = 0;
    int x = 0;
#ifdef CK_ENCRYPTION
    int e = 0;
    int d = 0;
#endif /* CK_ENCRYPTION */

    if (tn_wait_flg) {
        for (x = TELOPT_FIRST; x <= TELOPT_LAST; x++) {
            if (TELOPT_OK(x)) {
                if (TELOPT_UNANSWERED_WILL(x)) {
                    if ( notquiet )
                      printf("?Telnet waiting for response to WILL %s\r\n",
                             TELOPT(x));
                    debug(F111,"tn_outst","unanswered WILL",x);
                    outstanding = 1;
                    if ( !notquiet )
                      break;
                }
                if (TELOPT_UNANSWERED_DO(x)) {
                    if ( notquiet )
                      printf("?Telnet waiting for response to DO %s\r\n",
                             TELOPT(x));
                    debug(F111,"tn_outst","unanswered DO",x);
                    outstanding = 1;
                    if ( !notquiet )
                      break;
                }
                if (TELOPT_UNANSWERED_WONT(x)) {
                    if ( notquiet )
                      printf("?Telnet waiting for response to WONT %s\r\n",
                             TELOPT(x));
                    debug(F111,"tn_outst","unanswered WONT",x);
                    outstanding = 1;
                    if ( !notquiet )
                      break;
                }
                if (TELOPT_UNANSWERED_DONT(x)) {
                    if ( notquiet )
                      printf("?Telnet waiting for response to DONT %s\r\n",
                             TELOPT(x));
                    debug(F111,"tn_outst","unanswered DONT",x);
                    outstanding = 1;
                    if ( !notquiet )
                      break;
                }
                if (TELOPT_UNANSWERED_SB(x)) {
                    if ( notquiet )
                      printf("?Telnet waiting for response to SB %s\r\n",
                             TELOPT(x));
                    debug(F111,"tn_outst","unanswered SB",x);
                    outstanding = 1;
                    if ( !notquiet )
                      break;
                }
            }
        }
        if (!outstanding && !notquiet) {
#ifdef CK_ENCRYPTION
            e = ck_tn_encrypting();
            d = ck_tn_decrypting();
            if (TELOPT_ME(TELOPT_ENCRYPTION)) {
                if (TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop && e ||
                    !TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop && !e
                    ) {
                    if ( notquiet )
                      printf("?Telnet waiting for WILL %s subnegotiation\r\n",
                             TELOPT(TELOPT_ENCRYPTION));
                    debug(F111,
                          "tn_outst",
                          "encryption mode switch",
                          TELOPT_ENCRYPTION
                          );
                    outstanding = 1;
                }
            }
            if (TELOPT_U(TELOPT_ENCRYPTION)) {
                if (TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop && d ||
                    !TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop && !d
                    ) {
                    if ( notquiet )
                      printf("?Telnet waiting for DO %s subnegotiation\r\n",
                             TELOPT(TELOPT_ENCRYPTION));
                    debug(F111,
                          "tn_outst",
                          "decryption mode switch",
                           TELOPT_ENCRYPTION
                          );
                    outstanding = 1;
                }
            }
#endif /* CK_ENCRYPTION */
#ifdef CK_AUTHENTICATION
            if (TELOPT_ME(TELOPT_AUTHENTICATION) &&
                ck_tn_auth_in_progress()
                ) {
                if ( notquiet )
                  printf("?Telnet waiting for WILL %s subnegotiation\r\n",
                         TELOPT(TELOPT_AUTHENTICATION));
                debug(F111,
                      "tn_outst",
                      "ME authentication in progress",
                       TELOPT_AUTHENTICATION
                      );
                outstanding = 1;
            } else if (TELOPT_U(TELOPT_AUTHENTICATION) &&
                       ck_tn_auth_in_progress()
                       ) {
                if ( notquiet )
                  printf("?Telnet waiting for DO %s subnegotiation\r\n",
                         TELOPT(TELOPT_AUTHENTICATION));
                debug(F111,
                      "tn_outst",
                      "U authentication in progress",
                      TELOPT_AUTHENTICATION
                      );
                outstanding = 1;
            }
#endif /* CK_AUTHENTICATION */
        }
    } /* if (tn_wait_flg) */

#ifdef IKS_OPTION
    /* Even if we are not waiting for Telnet options we must wait for */
    /* Kermit Telnet Subnegotiations if we have sent a request to the */
    /* other guy.  Otherwise we will get out of sync.                 */
    if (!outstanding) {
        if (TELOPT_U(TELOPT_KERMIT) &&
            (TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start ||
             TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop ||
             !TELOPT_SB(TELOPT_KERMIT).kermit.sop)
            ) {
            if ( notquiet )
              printf("?Telnet waiting for DO %s subnegotiation\r\n",
                     TELOPT(TELOPT_KERMIT));
            debug(F111,"tn_outst","U kermit in progress",TELOPT_KERMIT);
            outstanding = 1;
        }
    }
#endif /* IKS_OPTION */
    return(outstanding);
}

/* tn_wait() -- Wait for response to Telnet negotiation. */
/*
  Wait for up to <timeout> seconds for the response to arrive.
  Place all non-telnet data into Telnet Wait Buffer.
  If response does arrive return 1, else return 0.
*/
#ifndef TN_WAIT_BUF_SZ
#define TN_WAIT_BUF_SZ 4096
#endif /* TN_WAIT_BUF_SZ */
static char tn_wait_buf[TN_WAIT_BUF_SZ];
static int  tn_wait_idx = 0;
#ifndef TN_TIMEOUT
#define TN_TIMEOUT 120
#endif /* TN_TIMEOUT */
static int tn_wait_tmo = TN_TIMEOUT;

#ifdef CKSPINNER
VOID
prtwait(state) int state; {
    switch (state % 4) {
      case 0:
        printf("/");
        break;
      case 1:
        printf("-");
        break;
      case 2:
        printf("\\");
        break;
      case 3:
        printf("|");
        break;
    }
}
#endif /* CKSPINNER */

static int nflag = 0;

int
#ifdef CK_ANSIC
tn_wait(char * where)
#else
tn_wait(where) char * where;
#endif /* CK_ANSIC */
/* tn_wait */ {
    extern int ckxech, local;
    int ch = 0, try = 0, count = 0;
    int outstanding;

    debug(F110,"tn_wait waiting for",where,0);
    tn_wait_tmo = TN_TIMEOUT;
    debug(F111,"tn_wait","timeout",tn_wait_tmo);
    outstanding = tn_outst(0);

    /* The following is meant to be !(||).  We only want to return */
    /* immediately if both the tn_wait_flg && tn_outst() are false */
    if (!(outstanding || tn_wait_flg))  /* If no need to wait */
      return(1);                        /* Don't. */

    if (tn_deb || debses) tn_debug("<wait for outstanding negotiations>");

    if (!sstelnet && !quiet) {
#ifdef CKSPINNER
        prtwait(try);
#endif /* CKSPINNER */
    }

    /* Wait up to TN_TIMEOUT sec for responses to outstanding telnet negs */
    while ((tn_wait_idx < TN_WAIT_BUF_SZ) &&
           (count || ((outstanding || try == 0) && ttchk() >= 0))
           ) {
#ifdef NTSIG
        ck_ih();
#endif /* NTSIG */
        ch = ttinc(1);
        if (ch == -1) {                 /* Timed out */
            try++;
            if (!sstelnet && !quiet) {  /* Let user know... */
#ifdef CKSPINNER
                printf("\b");
                prtwait(try);
#else
                if (nflag == 0) {
                    printf(" Negotiations.");
                    nflag++;
                }
                if (nflag > 0) {
                    printf(".");
                    nflag++;
                    fflush(stdout);
                }
#endif /* CKSPINNER */
            }
            if ( try > tn_wait_tmo ) {
                if (!sstelnet) {
                    printf(
                       "\r\n?Telnet Protocol Timeout - connection closed\r\n");
                    if (tn_deb || debses)
                      tn_debug(
                       "<telnet protocol timeout - connection closed>");
                    tn_outst(1);
                }
                /* if we do not close the connection, then we will block */
                /* the next time we hit a wait.  and if we don't we will */
                /* do the wrong thing if the host sends 0xFF and does    */
                /* not intend it to be an IAC.                           */
                ttclos(0);
                whyclosed = WC_TELOPT;
                return(-1);
            }
            continue;
        } else if (ch < -1) {
            printf("\r\n?Connection closed by peer.\r\n");
            if (tn_deb || debses) tn_debug("<connection closed by peer>");
            return(-1);
        }
        switch (ch) {
          case IAC:
#ifdef CKSPINNER
            if (!sstelnet && !quiet)
              printf("\b");
#endif /* CKSPINNER */
            ch = tn_doop((CHAR)(ch & 0xff),inserver?ckxech:duplex,ttinc);
#ifdef CKSPINNER
            if (!sstelnet && !quiet)
              prtwait(try);
#endif /* CKSPINNER */
            debug(F101,"tn_wait tn_doop","",ch);
            switch (ch) {
              case 1:
                duplex = 1;             /* Turn on echoing */
                if (inserver)
                  ckxech = 1;
                break;
              case 2:
                duplex = 0;             /* Turn off echoing */
                if (inserver)
                  ckxech = 0;
                break;
              case 3:
                tn_wait_buf[tn_wait_idx++] = IAC;
                break;
              case 4:                   /* IKS event */
              case 6:                   /* Logout */
                break;
              case -1:
                printf("?Telnet Option negotiation error.\n");
                if (tn_deb || debses)
                  tn_debug("<Telnet Option negotiation error>");
                return(-1);
              case -2:
                printf("?Connection closed by peer.\n");
                if (tn_deb || debses) tn_debug("<Connection closed by peer>");
                return(-2);
              default:
                if (ch < 0) {
                  if (tn_deb || debses) tn_debug("<Unknown connection error>");
                  return(ch);
                }
            } /* switch */
            break;
          default:
            tn_wait_buf[tn_wait_idx++] = (CHAR)(ch & 0xff);
        } /* switch */
        outstanding = tn_outst(0);
        count = ttchk();
        if (!try)
          try++;
    } /* while */

    if (tn_wait_idx == TN_WAIT_BUF_SZ) {
      if (tn_deb || debses) tn_debug("<Telnet Wait Buffer filled>");
      return(0);
    }

    if (!sstelnet && !quiet) {
#ifdef CKSPINNER
        printf("\b \b");
#else
        if (nflag > 0) {
            printf(" (OK)\n");
            nflag = -1;
        }
#endif /* CKSPINNER */
    }
    if (tn_deb || debses) tn_debug("<no outstanding negotiations>");
    return(0);
}

/* Push data from the Telnet Wait Buffer into the I/O Queue */
/* Return 1 on success                                      */

int
tn_push() {
#ifdef NETLEBUF
    extern int tt_push_inited;
#endif /* NETLEBUF */
    if (tn_wait_idx) {
        hexdump((CHAR *)"tn_push",tn_wait_buf,tn_wait_idx);
#ifdef NETLEBUF
        if (!tt_push_inited)            /* Local handling */
          le_init();
        le_puts((CHAR *)tn_wait_buf,tn_wait_idx);
#else                                   /* External handling... */
#ifdef OS2                              /* K95 has its own way */
        le_puts((CHAR *)tn_wait_buf,tn_wait_idx);
#else
#ifdef TTLEBUF                          /* UNIX, etc */
        le_puts((CHAR *)tn_wait_buf,tn_wait_idx);
#else
/*
  If you see this message in AOS/VS, OS-9, VOS, etc, you need to copy
  the #ifdef TTLEBUF..#endif code from ckutio.c to the corresponding
  places in your ck?tio.c module.
*/
        printf("tn_push called but not implemented - data lost.\n");
#endif /* NETLEBUF */
#endif /* UNIX */
#endif /* OS2 */
        tn_wait_idx = 0;
    }
    tn_wait_tmo = TN_TIMEOUT;           /* Reset wait timer stats */
    return(1);
}

/*  T N _ S O P T  */
/*
   Sends a telnet option, avoids loops.
   Returns 1 if command was sent, 0 if not, -1 on error.
*/
int
tn_sopt(cmd,opt) int cmd, opt; {        /* TELNET SEND OPTION */
    CHAR buf[5];

    if (ttnet != NET_TCPB) return(-1);  /* Must be TCP/IP */
    if (ttnproto != NP_TELNET) return(-1); /* Must be telnet protocol */
    if (!TELCMD_OK(cmd)) return(-1);
    if (TELOPT_OK(opt)) {
        if (cmd == DO && TELOPT_UNANSWERED_DO(opt)) return(0);
        if (cmd == WILL && TELOPT_UNANSWERED_WILL(opt)) return(0);
        if (cmd == DONT && TELOPT_UNANSWERED_DONT(opt)) return(0);
        if (cmd == WONT && TELOPT_UNANSWERED_WONT(opt)) return(0);
    }
#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if (cmd == DO && opt == TELOPT_AUTHENTICATION)
      buf[0] = 0;

    if (tn_infinite && TELOPT_OK(opt)) { /* See comment above about   */
        int index = TELOPT_INDEX(opt);   /* preventing infinite loops */
        int m = cmd - WILL;

        if (tncnts[index][m] > MAXTNCNT) {
            if (tn_deb || debses || deblog) {
                sprintf(tn_msg,"TELNET negotiation loop %s %s",
                        TELCMD(cmd),
                        TELOPT(opt));
                debug(F101,tn_msg,"",opt);
                if (tn_deb || debses) tn_debug(tn_msg);
            }
            return(0);
        }
        tncnts[index][m]++;
        tncnts[index][tnopps[m]] = 0;
    }
    buf[0] = (CHAR) IAC;
    buf[1] = (CHAR) (cmd & 0xff);
    buf[2] = (CHAR) (opt & 0xff);
    buf[3] = (CHAR) 0;
    if ((tn_deb || debses || deblog) && cmd != SB) {
        sprintf(tn_msg,"TELNET SENT %s %s",TELCMD(cmd),
                TELOPT(opt));
        debug(F101,tn_msg,"",opt);
    }

    if (ttol(buf,3) < 3) {
        return(-1);
    }

    /* Only display the command if it was actually sent. */
    if ((tn_deb || debses) && cmd != SB) tn_debug(tn_msg);

    if (TELOPT_OK(opt)) {
        if (cmd == DONT && TELOPT_UNANSWERED_DO(opt))
          TELOPT_UNANSWERED_DO(opt) = 0;
        if (cmd == WONT && TELOPT_UNANSWERED_WILL(opt))
          TELOPT_UNANSWERED_WILL(opt) = 0;
        if (cmd == DO && TELOPT_UNANSWERED_DONT(opt))
          TELOPT_UNANSWERED_DONT(opt) = 0;
        if (cmd == WILL && TELOPT_UNANSWERED_WONT(opt))
          TELOPT_UNANSWERED_WONT(opt) = 0;
    }
    return(1);
}

/* Send a telnet sub-option */
/* Returns 1 if command was sent, 0 if not, -1 on error */

int
tn_ssbopt(opt,sub,data,len) int opt, sub; CHAR * data; int len; {
    CHAR buf[256];
    int n,m;

    if (ttnet != NET_TCPB) return(0);   /* Must be TCP/IP */
    if (ttnproto != NP_TELNET) return(0); /* Must be telnet protocol */
    if (!TELOPT_OK(opt)) return(-1);
    if (len < 0 || len > 250) {
        debug(F111,"Unable to Send TELNET SB - data too long","len",len);
        return(-1);                     /* Data too long */
    }
#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        if (ttchk() < 0)
          return(-1);
        else
          return(1);
    }
#endif /* CK_SSL */

    if (!data) len = 0;

    buf[0] = (CHAR) IAC;
    buf[1] = (CHAR) (SB & 0xff);
    buf[2] = (CHAR) (opt & 0xff);
    buf[3] = (CHAR) (sub & 0xff);
    if (data && len > 0) {
        memcpy(&buf[4],data,len);
    }
    buf[4+len] = (CHAR) IAC;
    buf[5+len] = (CHAR) SE;

    if (tn_deb || debses || deblog) {
        if (opt == TELOPT_START_TLS && sub == 1)
          sprintf(tn_msg,"TELNET SENT SB %s FOLLOWS IAC SE",
                  TELOPT(opt)
                  );
        else if (opt == TELOPT_TTYPE && sub == 1)
          sprintf(tn_msg,"TELNET SENT SB %s SEND IAC SE", TELOPT(opt));
        else if (opt == TELOPT_TTYPE && sub == 0)
          sprintf(tn_msg,"TELNET SENT SB %s IS %s IAC SE", TELOPT(opt),data);
        else if (opt == TELOPT_NEWENVIRON) {
            int i, quote;
            sprintf(tn_msg,"TELNET SENT SB %s %s",
                    TELOPT(TELOPT_NEWENVIRON),
                    sub == TELQUAL_SEND ? "SEND" :
                    sub == TELQUAL_IS ? "IS" :
                    sub == TELQUAL_INFO ?"INFO" : "UNKNOWN" );
            for (i = 0, quote = 0; i < len; i++) {
                if (quote) {
                    sprintf(hexbuf,"%02x",data[i]);
                    strcat(tn_msg,hexbuf);
                    quote = 0;
                } else {
                    switch (data[i]) {
                      case TEL_ENV_USERVAR:
                        strcat(tn_msg," USERVAR ");
                        break;
                      case TEL_ENV_VAR:
                        strcat(tn_msg," VAR ");
                        break;
                      case TEL_ENV_VALUE:
                        strcat(tn_msg," VALUE ");
                        break;
                      case TEL_ENV_ESC:
                        strcat(tn_msg," ESC ");
                        quote = 1;
                        break;
                      case IAC:
                        strcat(tn_msg," IAC ");
                        break;
                      default:
                        sprintf(hexbuf,"%c",data[i]);
                        strcat(tn_msg,hexbuf);
                    }
                }
            }
            strcat(tn_msg," IAC SE");
        } else
          sprintf(tn_msg,
                  "TELNET SENT SB %s %02x <data> IAC SE",
                  TELOPT(opt),
                  sub
                  );
        debug(F101,tn_msg,"",opt);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
    if (ttol(buf,6+len) < 6+len)
      return(-1);
    return(1);
}

/*
  tn_flui() -- Processes all waiting data for Telnet commands.
  All non-Telnet data is to be stored into the Telnet Wait Buffer.
  Returns 1 on success.
*/
int
tn_flui() {
    extern int ckxech;
    int x = 0;

    /* Wait up to 5 sec for responses to outstanding telnet negotiations */
    while (x >= 0 && ttchk() > 0  && tn_wait_idx < TN_WAIT_BUF_SZ) {
        x = ttinc(1);
        switch (x) {
          case IAC:
            x = tn_doop((CHAR)(x & 0xff),inserver?ckxech:duplex,ttinc);
            debug(F101,"tn_flui tn_doop","",x);
            switch (x) {
              case 1:                   /* Turn on echoing */
                duplex = 1;
                if (inserver)
                  ckxech = 1;
                break;
              case 2:                   /* Turn off echoing */
                duplex = 0;
                if (inserver)
                  ckxech = 0;
                break;
              case 3:
                tn_wait_buf[tn_wait_idx++] = IAC;
                break;
              case 4:                   /* IKS event */
              case 6:                   /* Logout */
                break;
            }
            break;
          default:
            if (x >= 0)
              tn_wait_buf[tn_wait_idx++] = x;
        }
    }
    return(1);
}

#ifdef CK_FORWARD_X
int
#ifdef CK_ANSIC
fwdx_tn_sb( unsigned char * sb, int n )
#else
fwdx_tn_sb( sb, n ) unsigned char * sb; int n;
#endif /* CK_ANSIC */
{
    unsigned short hchannel, nchannel;
    unsigned char * p;
    int rc = -1;

    /* as a security precaution should add a test here to check to make sure */
    /* we have negotiated FORWARD_X with the peer.                           */

    switch (sb[0]) {
    case FWDX_SCREEN:
        if (sstelnet && n == 4)
            rc = fwdx_create_listen_socket(sb[1]);
        break;
    case FWDX_OPEN:
        if ( !sstelnet && n == 5 ) {
            p = (unsigned char *) &nchannel;
            p[0] = sb[1];
            p[1] = sb[2];

            hchannel = ntohs(nchannel);
            rc = fwdx_open_client_channel(hchannel);
            if ( rc < 0 ) {
                /* Failed; Send CLOSE channel */
                fwdx_send_close(hchannel);
            }
#ifdef NT
            if ( !TELOPT_SB(TELOPT_FORWARD_X).forward_x.thread_started ) {
                ckThreadBegin( &fwdx_thread,32655, 0, FALSE, 0 ) ;
                TELOPT_SB(TELOPT_FORWARD_X).forward_x.thread_started = 1;
            }
#endif /* NT */
        }
        break;
    case FWDX_CLOSE:
        p = (unsigned char *) &nchannel;
        p[0] = sb[1];
        p[1] = sb[2];

        hchannel = ntohs(nchannel);
        rc = fwdx_close_channel(hchannel);
        break;
    case FWDX_DATA:
        p = (unsigned char *) &nchannel;
        p[0] = sb[1];
        p[1] = sb[2];

        hchannel = ntohs(nchannel);
        rc = fwdx_write_data_to_channel(hchannel,(char *)&sb[3],n-5);
        break;
    }

    if ( rc < 0 ) {
        if ( sstelnet ) {
            if (tn_sopt(WONT,TELOPT_FORWARD_X) < 0)
                return(-1);
            TELOPT_UNANSWERED_WONT(TELOPT_FORWARD_X) = 1;
            return(-1);
        } else {
            if (tn_sopt(DONT,TELOPT_FORWARD_X) < 0)
                return(-1);
            TELOPT_UNANSWERED_DONT(TELOPT_FORWARD_X) = 1;
            return(-1);
        }
    }
    return(0);
}

int
fwdx_send_close(channel) int channel; {
    int nchannel;
    CHAR * p;

    nchannel = htons(channel);
    p = (unsigned char *) &nchannel;

    sb[0] = (CHAR) IAC;                 /* I Am a Command */
    sb[1] = (CHAR) SB;                  /* Subnegotiation */
    sb[2] = TELOPT_FORWARD_X;           /* Forward X */
    sb[3] = FWDX_CLOSE;                  /* Open */
    sb[4] = p[0];
    sb[5] = p[1];
    sb[6] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb[7] = (CHAR) SE;                  /* marked by IAC SE */
    if (ttol((CHAR *)sb,8) < 0) {       /* Send it. */
        return(-1);
    }
#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        sprintf(tn_msg,"TELNET SENT SB %s CLOSE %02x %02x IAC SE",
        TELOPT(TELOPT_FORWARD_X),p[0],p[1]);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
#endif /* DEBUG */
    return(0);
}

int
fwdx_send_open(channel) int channel; {
    int nchannel;
    CHAR * p;

    nchannel = htons(channel);
    p = (unsigned char *) &nchannel;

    sb[0] = (CHAR) IAC;                 /* I Am a Command */
    sb[1] = (CHAR) SB;                  /* Subnegotiation */
    sb[2] = TELOPT_FORWARD_X;           /* Forward X */
    sb[3] = FWDX_OPEN;                  /* Open */
    sb[4] = p[0];
    sb[5] = p[1];
    sb[6] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb[7] = (CHAR) SE;                  /* marked by IAC SE */
    if (ttol((CHAR *)sb,8) < 0) {       /* Send it. */
        return(-1);
    }
#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        sprintf(tn_msg,"TELNET SENT SB %s OPEN %02x %02x IAC SE",
        TELOPT(TELOPT_FORWARD_X),p[0],p[1]);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
#endif /* DEBUG */
    return(0);
}


#endif /* CK_FORWARD_X */

#ifdef IKS_OPTION
/*
  iks_wait() -- Wait for an IKS subnegotiation response.
  sb - is either KERMIT_REQ_START or KERMIT_REQ_STOP depending on the desired
       state of the peer's Kermit server.
  flushok - specifies whether it is ok to throw away non-Telnet data
       if so, then we call ttflui() instead of tn_flui().
  Returns:
   1 if the desired state is achieved or if it is unknown.
   0 if the desired state is not achieved.
*/
int
#ifdef CK_ANSIC
iks_wait(int sb, int flushok)
#else /* CK_ANSIC */
iks_wait(sb,flushok) int sb; int flushok;
#endif /* CK_ANSIC */
{
    int tn_wait_save = tn_wait_flg;
    int x;

    if (TELOPT_U(TELOPT_KERMIT)) {
        switch (sb) {
          case KERMIT_REQ_START:
            debug(F111,
                  "iks_wait KERMIT_REQ_START",
                  "u_start",
                  TELOPT_SB(TELOPT_KERMIT).kermit.u_start
                  );
            tn_siks(KERMIT_REQ_START);
            tn_wait_flg = 1;            /* Kermit Option MUST wait */
            do {
                if (flushok)
                  tn_wait_idx = 0;
                x = tn_wait("iks_wait() me_iks_req_start");
            } while (x == 0 && flushok && tn_wait_idx == TN_WAIT_BUF_SZ);
            tn_wait_flg = tn_wait_save;
            if (flushok)
              tn_wait_idx = 0;
	    if (tn_wait_idx == TN_WAIT_BUF_SZ) {
		/*
		 * We are attempting to start a kermit server on the peer
		 * the most likely reason is because we want to perform a
		 * file transfer.  But there is a huge amount of non telnet
		 * negotiation data coming in and so we have not been able
		 * to find the response.  So we will lie and assume that
		 * response is 'yes'.  The worse that will happen is that
		 * a RESP_STOP is received after we enter protocol mode.
		 * And the protocol operation will be canceled.
		 */
		tn_push();
		return(1);
	    } else {
		tn_push();
		return(TELOPT_SB(TELOPT_KERMIT).kermit.u_start);
	    }
          case KERMIT_REQ_STOP:
            debug(F111,
                  "iks_wait KERMIT_REQ_STOP",
                  "u_start",
                  TELOPT_SB(TELOPT_KERMIT).kermit.u_start
                  );
            tn_siks(KERMIT_REQ_STOP);
            tn_wait_flg = 1;            /* Kermit Option MUST wait */
            do {
                if (flushok)
                  tn_wait_idx = 0;
                x = tn_wait("iks_wait() me_iks_req_stop");
            } while (x == 0 && flushok && tn_wait_idx == TN_WAIT_BUF_SZ);
            tn_wait_flg = tn_wait_save;
            if (flushok)
              tn_wait_idx = 0;

	    if (tn_wait_idx == TN_WAIT_BUF_SZ) {
		/*
		 * We are attempting to stop a kermit server on the peer
		 * the most likely reason being that we want to enter
		 * CONNECT mode.  But there is a huge amount of non telnet
		 * negotiation data coming in and so we have not been able
		 * to find the response.  So we will lie and assume that
		 * the answer is 'yes' and allow the CONNECT command to
		 * succeed.  The worst that happens is that CONNECT mode
		 * swallows the incoming data displaying it to the user
		 * and then it resumes Kermit client mode.
		 */
		tn_push();
		return(1);
	    } else {
		tn_push();
		return(!TELOPT_SB(TELOPT_KERMIT).kermit.u_start);
	    }
        }
        tn_push();
    }
    return(1);
}

int
#ifdef CK_ANSIC
iks_tn_sb( char * sb, int n )
#else
iks_tn_sb( sb, n ) char * sb; int n;
#endif /* CK_ANSIC */
{
#ifndef NOXFER
    extern int server;
#ifdef NOICP
    extern int autodl;
    int inautodl = 0, cmdadl = 1;
    extern int local;
#else
#ifdef CK_AUTODL
    extern int autodl, inautodl, cmdadl;
    extern int local;
#endif /* CK_AUTODL */
#endif /* NOICP */
    switch (sb[0]) {
      case KERMIT_START:                /* START */
        TELOPT_SB(TELOPT_KERMIT).kermit.u_start = 1;
        return(4);

      case KERMIT_STOP:                 /* STOP */
        TELOPT_SB(TELOPT_KERMIT).kermit.u_start = 0;
        return(4);

      case KERMIT_REQ_START:            /* REQ-START */
        if (inserver) {
#ifdef CK_AUTODL
            cmdadl = 1;                 /* Turn on packet detection */
#endif /* CK_AUTODL */
            TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 1;
            tn_siks(KERMIT_RESP_START);
        } else if (TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
            tn_siks(KERMIT_RESP_START);
        } else {
#ifdef CK_AUTODL
            if ((local && what == W_CONNECT && autodl) ||
                (local && what != W_CONNECT && inautodl)
                )
              tn_siks(KERMIT_RESP_START); /* STOP */
            else
#endif /* CK_AUTODL */
              tn_siks(KERMIT_RESP_STOP);
        }
        return(4);

      case KERMIT_REQ_STOP:             /* REQ-STOP */
        /* The protocol requires that the request be responded to */
        /* either by changing states or by reporting the current  */
        /* state.  */

        /* We need to provide the user some way of dictating what */
        /* the policies should be.  For instance, if we are in    */
        /* CONNECT mode with autodownload ON and we get a REQ-STOP*/
        /* what should the proper response be?                    */

        if (inserver
#ifdef CK_AUTODL
            || !local && cmdadl
#endif /* CK_AUTODL */
            ) {
#ifdef CK_AUTODL
            cmdadl = 0;                 /* Turn off packet detection */
#endif /* CK_AUTODL */
            tn_siks(KERMIT_RESP_STOP);
        } else if (server) {
            extern int en_fin;
            if (en_fin) {               /* If the server is allowed to stop */
                tn_siks(KERMIT_RESP_STOP);
            } else {                    /* We are not allowed to stop */
                tn_siks(KERMIT_RESP_START);
            }
#ifdef CK_AUTODL
        } else if ((local && what == W_CONNECT && autodl) ||
                   (local && what != W_CONNECT && inautodl)
                   ) {
            /* If we are a pseudo-server and the other side requests */
            /* that we stop, tell then that we have even though we   */
            /* have not.  Otherwise, the other side might refuse to  */
            /* enter SERVER mode.                                    */

            tn_siks(KERMIT_RESP_STOP);  /* STOP */
#endif /* CK_AUTODL */

        } else {
            /* If we are not currently in any mode that accepts */
            /* Kermit packets then of course report that we are */
            /* not being a Kermit server.                       */

            tn_siks(KERMIT_RESP_STOP);  /* STOP */
        }
        return(4);

      case KERMIT_SOP: {                /* SOP */
          extern CHAR stchr;            /* Incoming SOP character */
          stchr = sb[1];
          TELOPT_SB(TELOPT_KERMIT).kermit.sop = 1;
          return(4);
      }

      case KERMIT_RESP_START:           /* START */
        TELOPT_SB(TELOPT_KERMIT).kermit.u_start = 1;
        if (TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start) {
            TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start = 0;
        } else if (TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop) {
            /* If we have issued a request to stop a Kermit Server */
            /* and the response is Start, then we must report this */
            /* to the caller.                                      */
            TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop = 0;
        }
        return(4);

      case KERMIT_RESP_STOP:            /* STOP */
        TELOPT_SB(TELOPT_KERMIT).kermit.u_start = 0;
        if (TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start) {
            TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start = 0;
            /* If we have issued a request to start a Kermit Server */
            /* and the response is Stop, then we must report this   */
            /* to the caller.                                       */
        } else if (TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop) {
            TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop = 0;
        }
        return(4);

      default:
        return(0);

    } /* switch (sb[0]) */
#else
    return(0);
#endif /* NOXFER */
}
#endif /* IKS_OPTION */

/* Initialize telnet settings - set default values for ME and U modes */
int
tn_set_modes() {
    int opt,cmd;

    /* initialize all options to refuse in both directions */
    for (opt = 0; opt < NTELOPTS; opt++) {
        TELOPT_ME(opt) = 0;
        TELOPT_U(opt)  = 0;
        TELOPT_UNANSWERED_WILL(opt) = 0;
        TELOPT_UNANSWERED_DO(opt)   = 0;
        TELOPT_UNANSWERED_WONT(opt) = 0;
        TELOPT_UNANSWERED_DONT(opt)   = 0;
        TELOPT_UNANSWERED_SB(opt)   = 0;
        TELOPT_ME_MODE(opt) = TN_NG_RF;
        TELOPT_U_MODE(opt) = TN_NG_RF;
        TELOPT_DEF_S_ME_MODE(opt) = TN_NG_RF;
        TELOPT_DEF_S_U_MODE(opt) = TN_NG_RF;
        TELOPT_DEF_C_ME_MODE(opt) = TN_NG_RF;
        TELOPT_DEF_C_U_MODE(opt) = TN_NG_RF;
        for (cmd = 0; cmd < 4; cmd ++)
          tncnts[TELOPT_INDEX(opt)][cmd] = 0;
    }
#ifdef IKS_OPTION
    TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.u_start = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.sop = 0;
#endif /* IKS_OPTION */

#ifdef CK_ENCRYPTION
    TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop = 0;
#endif /* CK_ENCRYPTION */

#ifdef  CK_NAWS
    TELOPT_SB(TELOPT_NAWS).naws.x = 0;
    TELOPT_SB(TELOPT_NAWS).naws.y = 0;
#endif /* CK_NAWS */

#ifdef CK_SSL
    TELOPT_SB(TELOPT_START_TLS).start_tls.u_follows = 0;
    TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows = 0;
#endif /* CK_SSL */

    /* Now set the ones we want to accept to the proper values */
    TELOPT_DEF_S_ME_MODE(TELOPT_SGA) = TN_NG_RQ;
    TELOPT_DEF_S_U_MODE(TELOPT_SGA) = TN_NG_RQ;
    TELOPT_DEF_C_ME_MODE(TELOPT_SGA) = TN_NG_AC;
    TELOPT_DEF_C_U_MODE(TELOPT_SGA) = TN_NG_AC;

    TELOPT_DEF_S_ME_MODE(TELOPT_BINARY) = TN_NG_AC;
    TELOPT_DEF_S_U_MODE(TELOPT_BINARY) = TN_NG_AC;
    TELOPT_DEF_C_ME_MODE(TELOPT_BINARY) = TN_NG_AC;
    TELOPT_DEF_C_U_MODE(TELOPT_BINARY) = TN_NG_AC;

    TELOPT_DEF_S_ME_MODE(TELOPT_LOGOUT) = TN_NG_AC;
    TELOPT_DEF_S_U_MODE(TELOPT_LOGOUT) = TN_NG_AC;
    TELOPT_DEF_C_ME_MODE(TELOPT_LOGOUT) = TN_NG_AC;
    TELOPT_DEF_C_U_MODE(TELOPT_LOGOUT) = TN_NG_AC;

#ifdef IKS_OPTION
    TELOPT_DEF_S_ME_MODE(TELOPT_KERMIT) = TN_NG_RQ;
    TELOPT_DEF_S_U_MODE(TELOPT_KERMIT) = TN_NG_RQ;
    TELOPT_DEF_C_ME_MODE(TELOPT_KERMIT) = TN_NG_RQ;
    TELOPT_DEF_C_U_MODE(TELOPT_KERMIT) = TN_NG_RQ;
#endif /* IKS_OPTION */

#ifdef CK_ENCRYPTION
    TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RQ;
    TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RQ;
    TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RQ;
    TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RQ;
#endif /* CK_ENCRYPTION */

    TELOPT_DEF_S_ME_MODE(TELOPT_ECHO) = TN_NG_RQ;
    TELOPT_DEF_S_U_MODE(TELOPT_TTYPE) = TN_NG_RQ;

#ifdef CK_ENVIRONMENT
    TELOPT_DEF_S_U_MODE(TELOPT_NEWENVIRON) = TN_NG_RQ;
#endif /* CK_ENVIRONMENT */

#ifdef CK_AUTHENTICATION
    TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = TN_NG_RQ;
#endif /* CK_AUTHENTICATION */

#ifdef CK_SSL
    if (ck_ssleay_is_installed()) {
        TELOPT_DEF_S_U_MODE(TELOPT_START_TLS) = TN_NG_RQ;
        TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS) = TN_NG_AC;
    }
#endif /* CK_SSL */

#ifdef CK_NAWS
    TELOPT_DEF_S_U_MODE(TELOPT_NAWS) = TN_NG_RQ;
#endif /* CK_NAWS */

    TELOPT_DEF_C_U_MODE(TELOPT_ECHO) = TN_NG_AC;
    TELOPT_DEF_C_ME_MODE(TELOPT_TTYPE) = TN_NG_RQ;

#ifdef CK_ENVIRONMENT
    TELOPT_DEF_C_ME_MODE(TELOPT_NEWENVIRON) = TN_NG_RQ;
#endif /* CK_ENVIRONMENT */

#ifdef CK_AUTHENTICATION
    TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = TN_NG_RQ;
#endif /* CK_AUTHENTICATION */

#ifdef CK_NAWS
    TELOPT_DEF_C_ME_MODE(TELOPT_NAWS) = TN_NG_RQ;
#endif /* CK_NAWS */

#ifdef CK_SNDLOC
    TELOPT_DEF_C_ME_MODE(TELOPT_SNDLOC) = TN_NG_RQ;
#endif /* CK_SNDLOC */

#ifdef CK_FORWARD_X
    TELOPT_DEF_C_U_MODE(TELOPT_FORWARD_X) = TN_NG_RQ;
#endif /* CK_FORWARD_X */

    /* Set the initial values for currently known mode */
    for (opt = TELOPT_FIRST; opt <= TELOPT_LAST; opt++) {
        if (TELOPT_OK(opt)) {
            TELOPT_ME_MODE(opt) = sstelnet ?
              TELOPT_DEF_S_ME_MODE(opt) :
                TELOPT_DEF_C_ME_MODE(opt);
            TELOPT_U_MODE(opt) = sstelnet ?
              TELOPT_DEF_S_U_MODE(opt) :
                TELOPT_DEF_C_U_MODE(opt);
        }
    }
    return(1);
}


/* Send Delayed Subnegotiations */

VOID
tn_sdsb() {
    if (TELOPT_SB(TELOPT_TTYPE).term.need_to_send) {
        tn_sttyp();
        TELOPT_SB(TELOPT_TTYPE).term.need_to_send = 0;
    }
#ifdef CK_ENVIRONMENT
    if (TELOPT_SB(TELOPT_NEWENVIRON).env.need_to_send &&
        TELOPT_SB(TELOPT_NEWENVIRON).env.str) {
        tn_snenv((CHAR *)TELOPT_SB(TELOPT_NEWENVIRON).env.str,
                 TELOPT_SB(TELOPT_NEWENVIRON).env.len);
        free(TELOPT_SB(TELOPT_NEWENVIRON).env.str);
        TELOPT_SB(TELOPT_NEWENVIRON).env.str=NULL;
        TELOPT_SB(TELOPT_NEWENVIRON).env.len=0;
        TELOPT_SB(TELOPT_NEWENVIRON).env.need_to_send = 0;
    }
#ifdef CK_XDISPLOC
    if (TELOPT_SB(TELOPT_XDISPLOC).xdisp.need_to_send) {
        tn_sxdisploc();
        TELOPT_SB(TELOPT_XDISPLOC).xdisp.need_to_send = 0;
    }
#endif /* CK_XDISPLOC */
#endif /* CK_ENVIRONMENT */
#ifdef CK_NAWS
    if (TELOPT_SB(TELOPT_NAWS).naws.need_to_send) {
        tn_snaws();
        TELOPT_SB(TELOPT_NAWS).naws.need_to_send = 0;
    }
#endif /* CK_NAWS */
#ifdef CK_SNDLOC
    if (TELOPT_SB(TELOPT_SNDLOC).sndloc.need_to_send) {
        tn_sndloc();
        TELOPT_SB(TELOPT_SNDLOC).sndloc.need_to_send = 0;
    }
#endif /* CK_SNDLOC */
#ifdef CK_FORWARD_X
    if (TELOPT_SB(TELOPT_FORWARD_X).forward_x.need_to_send) {
        tn_sndfwdx();
        TELOPT_SB(TELOPT_FORWARD_X).forward_x.need_to_send = 0;
    }
#endif /* CK_FORWARD_X */
}

int
tn_reset() {
    int x,opt,cmd;

    tn_wait_idx = 0;                    /* Clear the tn_push() buffer */
    tn_wait_tmo = TN_TIMEOUT;           /* Reset wait timer stats */

    nflag = 0;

    /* Reset the TELNET OPTIONS counts */
    for (opt = TELOPT_FIRST; opt <= TELOPT_LAST; opt++) {
        if (TELOPT_OK(opt)) {
            TELOPT_ME(opt) = 0;
            TELOPT_U(opt)  = 0;
            TELOPT_UNANSWERED_WILL(opt) = 0;
            TELOPT_UNANSWERED_DO(opt)   = 0;
            TELOPT_UNANSWERED_WONT(opt) = 0;
            TELOPT_UNANSWERED_DONT(opt)   = 0;
            TELOPT_UNANSWERED_SB(opt)   = 0;
            TELOPT_ME_MODE(opt) = sstelnet ?
              TELOPT_DEF_S_ME_MODE(opt) :
                TELOPT_DEF_C_ME_MODE(opt);
            TELOPT_U_MODE(opt) = sstelnet ?
              TELOPT_DEF_S_U_MODE(opt) :
                TELOPT_DEF_C_U_MODE(opt);

#ifdef DEBUG
            if (deblog) {
                switch (TELOPT_ME_MODE(opt)) {
                  case TN_NG_RF:
                    debug(F110,"tn_ini ME REFUSE ",TELOPT(opt),0);
                    break;
                  case TN_NG_AC:
                    debug(F110,"tn_ini ME ACCEPT ",TELOPT(opt),0);
                    break;
                  case TN_NG_RQ:
                    debug(F110,"tn_ini ME REQUEST",TELOPT(opt),0);
                    break;
                  case TN_NG_MU:
                    debug(F110,"tn_ini ME REQUIRE",TELOPT(opt),0);
                    break;
                }
                switch (TELOPT_U_MODE(opt)) {
                  case TN_NG_RF:
                    debug(F110,"tn_ini U  REFUSE ",TELOPT(opt),0);
                    break;
                  case TN_NG_AC:
                    debug(F110,"tn_ini U  ACCEPT ",TELOPT(opt),0);
                    break;
                  case TN_NG_RQ:
                    debug(F110,"tn_ini U  REQUEST",TELOPT(opt),0);
                    break;
                  case TN_NG_MU:
                    debug(F110,"tn_ini U  REQUIRE",TELOPT(opt),0);
                    break;
                }
            }
#endif /* DEBUG */
            for (cmd = 0; cmd < 4; cmd ++)
              tncnts[TELOPT_INDEX(opt)][cmd] = 0;
        }
    }
#ifdef CK_ENVIRONMENT
    if (!tn_env_flg) {
        TELOPT_ME_MODE(TELOPT_NEWENVIRON) = TN_NG_RF;
        TELOPT_U_MODE(TELOPT_NEWENVIRON) = TN_NG_RF;
    }
#endif /* CK_ENVIRONMENT */
#ifdef CK_SNDLOC
    if (!tn_loc)
        TELOPT_DEF_C_ME_MODE(TELOPT_SNDLOC) = TN_NG_RF;
#endif /* CK_SNDLOC */
#ifdef IKS_OPTION
    TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.u_start = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.sop = 0;
#endif /* IKS_OPTION */
#ifdef CK_ENCRYPTION
    TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop = 0;
    TELOPT_SB(TELOPT_ENCRYPTION).encrypt.need_to_send = 0;
#endif /* CK_ENCRYPTION */
#ifdef  CK_NAWS
    TELOPT_SB(TELOPT_NAWS).naws.need_to_send = 0;
    TELOPT_SB(TELOPT_NAWS).naws.x = 0;
    TELOPT_SB(TELOPT_NAWS).naws.y = 0;
#endif /* CK_NAWS */
    TELOPT_SB(TELOPT_TTYPE).term.need_to_send = 0;
    TELOPT_SB(TELOPT_TTYPE).term.type[0] = '\0';
#ifdef CK_ENVIRONMENT
    TELOPT_SB(TELOPT_NEWENVIRON).env.need_to_send = 0;
    if (tn_first)
        TELOPT_SB(TELOPT_NEWENVIRON).env.str=NULL;
    else if (TELOPT_SB(TELOPT_NEWENVIRON).env.str) {
        free(TELOPT_SB(TELOPT_NEWENVIRON).env.str);
        TELOPT_SB(TELOPT_NEWENVIRON).env.str=NULL;
    }
    TELOPT_SB(TELOPT_NEWENVIRON).env.len=0;
#ifdef CK_XDISPLOC
    TELOPT_SB(TELOPT_XDISPLOC).xdisp.need_to_send = 0;
#endif /* CK_XDISPLOC */
#endif /* CK_ENVIRONMENT */
#ifdef CK_SNDLOC
    TELOPT_SB(TELOPT_SNDLOC).sndloc.need_to_send = 0;
#endif /* CK_SNDLOC */
#ifdef CK_FORWARD_X
    TELOPT_SB(TELOPT_FORWARD_X).forward_x.need_to_send = 0;
    if ( TELOPT_SB(TELOPT_FORWARD_X).forward_x.listen_socket != -1 ) {

    }
    TELOPT_SB(TELOPT_FORWARD_X).forward_x.listen_socket = -1;
    for ( x=0 ; x<MAXFWDX ; x++ ) {
        TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[x].fd = -1;
        TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[x].id = -1;
    }
#ifdef NT
    TELOPT_SB(TELOPT_FORWARD_X).forward_x.thread_started = 0;
#endif /* NT */
#endif /* CK_FORWARD_X */
#ifdef CK_SSL
    if (tls_only_flag || ssl_only_flag) {
        TELOPT_ME_MODE(TELOPT_START_TLS) = TN_NG_RF;
        TELOPT_U_MODE(TELOPT_START_TLS) = TN_NG_RF;
    }
    TELOPT_SB(TELOPT_START_TLS).start_tls.u_follows = 0;
    TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows = 0;
#endif /* CK_SSL */

#ifdef CK_ENCRYPTION
    if (!ck_crypt_is_installed()
#ifdef CK_SSL
        || tls_only_flag || ssl_only_flag
#endif /* CK_SSL */
        ) {
        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
    }
#endif /* CK_ENCRYPTION */

    tn_first = 0;                       /* No longer the first time init */

#ifdef OS2
    ttnum = -1;                         /* Reset TermType negotiation */
    ttnumend = 0;
#endif /* OS2 */

    return(0);
}

/* Start a telnet connection. */
/* Returns -1 on error, 0 if nothing happens, 1 if init msgs sent ok */

int
tn_ini() {
    int wait, x, opt;
    debug(F101,"tn_ini ttnproto","",ttnproto);
    debug(F101,"tn_ini tn_init","",tn_init);

    if (ttnet != NET_TCPB)              /* Make sure connection is TCP/IP */
      return(0);
    if (tn_init)                        /* Have we done this already? */
      return(0);                        /* Don't do it again. */

    tn_reset();                         /* Reset telnet parameters */

    if (ttnproto == NP_RLOGIN) {        /* Reset flags even when RLOGIN */
        tn_init = 1;
        return(0);
    } else if (ttnproto == NP_NONE) {   /* If not talking to a telnet port, */
        ttnproto = NP_TELNET;           /* pretend it's telnet anyway, */
        tn_init = 1;                    /* but don't send initial options. */
        debug(F100,"tn_ini skipping telnet negotiations","",0);
        return(0);
    } else if (ttnproto == NP_TCPRAW) { /* Raw socket requested. */
        return(0);
    } else if (ttnproto == NP_KERMIT) {
        /* switching to Telnet protocol */
        debug(F100,"tn_start switching from Kermit to Telnet","",0);
        ttnproto = NP_TELNET;
    }
    debug(F111,"tn_start","sstelnet",sstelnet);
    wait = 0;

#ifdef CK_SSL
    if (!TELOPT_ME(TELOPT_START_TLS) &&
        TELOPT_ME_MODE(TELOPT_START_TLS) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_START_TLS) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_START_TLS) = 1;
        wait = 1;
    }
    if (!TELOPT_U(TELOPT_START_TLS) &&
        TELOPT_U_MODE(TELOPT_START_TLS) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_START_TLS) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_START_TLS) = 1;
        wait = 1;
    }
#ifdef COMMENT
/*
  We can put off waiting for this until after we have requested AUTH.  The
  next draft will specify how the WILL side is to decide between these
  conflicting options.
*/
    if (wait) {
        if (tn_wait("start_tls") < 0) {
            tn_push();
            return(-1);
        }
        wait = 0;
    }
#endif /* COMMENT */
#endif /* CK_SSL */

#ifdef CK_AUTHENTICATION
    if (tn_init)                /* tn_ini() might be called recursively */
      return(0);
    if (!TELOPT_ME(TELOPT_AUTHENTICATION) &&
        TELOPT_ME_MODE(TELOPT_AUTHENTICATION) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_AUTHENTICATION) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_AUTHENTICATION) = 1;
        wait = 1;
    }
    if (!TELOPT_U(TELOPT_AUTHENTICATION) &&
        TELOPT_U_MODE(TELOPT_AUTHENTICATION) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_AUTHENTICATION) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_AUTHENTICATION) = 1;
        wait = 1;
    }
#ifdef CK_ENCRYPTION
    if (TELOPT_U_MODE(TELOPT_AUTHENTICATION) == TN_NG_RF &&
         TELOPT_ME_MODE(TELOPT_AUTHENTICATION) == TN_NG_RF) {
        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
    }
#endif /* CK_ENCRYPTION */
#endif /* CK_AUTHENTICATION */

#ifdef CK_NAWS
    if (!sstelnet) {
        /* Console terminal screen rows and columns */
#ifdef OS2
        debug(F101,
              "tn_ini tt_rows 1",
              "",
              VscrnGetHeight(VTERM)-(tt_status?1:0)
              );
        debug(F101,"tn_ini tt_cols 1","",VscrnGetWidth(VTERM));
        /* Not known yet */
        if (VscrnGetWidth(VTERM) < 0 ||
            VscrnGetHeight(VTERM)-(tt_status?1:0) < 0) {
            ttgwsiz();                  /* Try to find out */
        }
        debug(F101,
              "tn_ini tt_rows 2",
              "",
              VscrnGetHeight(VTERM)-(tt_status?1:0)
              );
        debug(F101,"tn_ini tt_cols 2","",VscrnGetWidth(VTERM));
        /* Now do we know? */
        if (VscrnGetWidth(VTERM) > 0 &&
            VscrnGetHeight(VTERM)-(tt_status?1:0) > 0) {
            if (!TELOPT_ME(TELOPT_NAWS) &&
                TELOPT_ME_MODE(TELOPT_NAWS) >= TN_NG_RQ) {
                if (tn_sopt(WILL, TELOPT_NAWS) < 0)
                  return(-1);
                TELOPT_UNANSWERED_WILL(TELOPT_NAWS) = 1;
                wait = 1;
            }
        }
#else /* OS2 */
        debug(F101,"tn_ini tt_rows 1","",tt_rows);
        debug(F101,"tn_ini tt_cols 1","",tt_cols);
        if (tt_rows < 0 || tt_cols < 0) { /* Not known yet */
            ttgwsiz();                  /* Try to find out */
        }
        debug(F101,"tn_ini tt_rows 2","",tt_rows);
        debug(F101,"tn_ini tt_cols 2","",tt_cols);
        if (tt_rows > 0 && tt_cols > 0) { /* Now do we know? */
            if (!TELOPT_ME(TELOPT_NAWS) &&
                TELOPT_ME_MODE(TELOPT_NAWS) >= TN_NG_RQ) {
                if (tn_sopt(WILL, TELOPT_NAWS) < 0)
                  return(-1);
                TELOPT_UNANSWERED_WILL(TELOPT_NAWS) = 1;
                wait = 1;
            }
        }
#endif /* OS2 */
    } else {
        if (!TELOPT_U(TELOPT_NAWS) &&
            TELOPT_U_MODE(TELOPT_NAWS) >= TN_NG_RQ) {
            if (tn_sopt(DO, TELOPT_NAWS) < 0)
              return(-1);
            TELOPT_UNANSWERED_DO(TELOPT_NAWS) = 1;
            wait = 1;
        }
    }
#endif /* CK_NAWS */

    if (!TELOPT_ME(TELOPT_SGA) &&
        TELOPT_ME_MODE(TELOPT_SGA) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_SGA) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_SGA) = 1;
        wait = 1;
    }
    if (!TELOPT_U(TELOPT_SGA) &&
        TELOPT_U_MODE(TELOPT_SGA) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_SGA) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_SGA) = 1;
        wait = 1;
    }
    if (!tn_duplex) {
        if (!TELOPT_U(TELOPT_ECHO) &&
            TELOPT_U_MODE(TELOPT_ECHO) >= TN_NG_RQ) {
            if (tn_sopt(DO, TELOPT_ECHO) < 0)
              return(-1);
            TELOPT_UNANSWERED_DO(TELOPT_ECHO) = 1;
            wait = 1;
        }
    }
    if (!TELOPT_ME(TELOPT_ECHO) &&
        TELOPT_ME_MODE(TELOPT_ECHO) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_ECHO) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_ECHO) = 1;
        wait = 1;
    }

    debug(F100,"tn_ini about to send WILL TTYPE if requested","",0);
/*
  Talking to TELNET port, so send WILL TERMINAL TYPE and DO SGA.
  Also send WILL NAWS if we know our screen dimensions.
*/
    if (!TELOPT_ME(TELOPT_TTYPE) &&
        TELOPT_ME_MODE(TELOPT_TTYPE) >= TN_NG_RQ) {
        if ((x = tn_sopt(WILL,TELOPT_TTYPE)) < 0) {
            debug(F101,"tn_ini tn_sopt WILL TTYPE failed","",x);
            return(-1);
        }
        TELOPT_UNANSWERED_WILL(TELOPT_TTYPE) = 1;
        wait = 1;
        debug(F100,"tn_ini sent WILL TTYPE ok","",0);
    }
    if (!TELOPT_U(TELOPT_TTYPE) &&
        TELOPT_U_MODE(TELOPT_TTYPE) >= TN_NG_RQ) {
        if ((x = tn_sopt(DO,TELOPT_TTYPE)) < 0) {
            debug(F101,"tn_ini tn_sopt DO TTYPE failed","",x);
            return(-1);
        }
        TELOPT_UNANSWERED_DO(TELOPT_TTYPE) = 1;
        wait = 1;
        debug(F100,"tn_ini sent DO TTYPE ok","",0);
    }
    if (!TELOPT_ME(TELOPT_BINARY) &&
        TELOPT_ME_MODE(TELOPT_BINARY) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_BINARY) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_BINARY) = 1;
        wait = 1;
    }
    if (!TELOPT_U(TELOPT_BINARY) &&
        TELOPT_U_MODE(TELOPT_BINARY) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_BINARY) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_BINARY) = 1;
        wait = 1;
    }
#ifdef CK_SNDLOC
    if (tn_loc) {
        if (!TELOPT_ME(TELOPT_SNDLOC) &&
            TELOPT_ME_MODE(TELOPT_SNDLOC) >= TN_NG_RQ) {
            if (tn_sopt(WILL, TELOPT_SNDLOC) < 0)
              return(-1);
            TELOPT_UNANSWERED_WILL(TELOPT_SNDLOC) = 1;
            wait = 1;
        }
    }
#endif /* CK_SNDLOC */
#ifdef CK_ENVIRONMENT
#ifdef CK_XDISPLOC
    if (!TELOPT_ME(TELOPT_XDISPLOC) &&
         TELOPT_ME_MODE(TELOPT_XDISPLOC) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_XDISPLOC) < 0)
            return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_XDISPLOC) = 1;
        wait = 1;
    }
#endif /* CK_XDISPLOC */
    /* Will send terminal environment. */
    if (!TELOPT_ME(TELOPT_NEWENVIRON) &&
        TELOPT_ME_MODE(TELOPT_NEWENVIRON) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_NEWENVIRON) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_NEWENVIRON) = 1;
        wait = 1;
    }
    if (!TELOPT_U(TELOPT_NEWENVIRON) &&
        TELOPT_U_MODE(TELOPT_NEWENVIRON) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_NEWENVIRON) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_NEWENVIRON) = 1;
        wait = 1;
    }
#endif /* CK_ENVIRONMENT */

    /* Take care of any other telnet options that require handling. */

    for (opt = TELOPT_FIRST; opt <= TELOPT_LAST; opt++) {
        switch (opt) {
          case TELOPT_AUTHENTICATION:
          case TELOPT_ENCRYPTION:
          case TELOPT_TTYPE:
          case TELOPT_NAWS:
          case TELOPT_BINARY:
          case TELOPT_NEWENVIRON:
          case TELOPT_SNDLOC:
          case TELOPT_XDISPLOC:
          case TELOPT_SGA:
          case TELOPT_ECHO:
          case TELOPT_KERMIT:
          case TELOPT_START_TLS:
            break;
          default:
            if (TELOPT_OK(opt)) {
                if (!TELOPT_ME(opt) &&
                    TELOPT_ME_MODE(opt) >= TN_NG_RQ) {
                    if (tn_sopt(WILL, opt) < 0)
                      return(-1);
                    TELOPT_UNANSWERED_WILL(opt) = 1;
                    wait = 1;
                }
                if (!TELOPT_U(opt) &&
                    TELOPT_U_MODE(opt) >= TN_NG_RQ) {
                    if (tn_sopt(DO, opt) < 0)
                      return(-1);
                    TELOPT_UNANSWERED_DO(opt) = 1;
                    wait = 1;
                }
            }
        }
    }
    if (wait) {
        if (tn_wait("pre-encrypt") < 0) {
            tn_push();
            return(-1);
        }
        wait = 0;
    }

#ifdef CK_ENCRYPTION
    if (tn_init)                /* tn_ini() may be called recursively */
      return(0);

    if (!TELOPT_ME(TELOPT_ENCRYPTION) &&
        TELOPT_ME_MODE(TELOPT_ENCRYPTION) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_ENCRYPTION) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_ENCRYPTION) = 1;
        wait = 1;
    }
    if (!TELOPT_U(TELOPT_ENCRYPTION) &&
        TELOPT_U_MODE(TELOPT_ENCRYPTION) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_ENCRYPTION) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_ENCRYPTION) = 1;
        wait = 1;
    }

    /* If we are going to encrypt, we want to do it before we send any more */
    /* data, especially the terminal type and environment variables.        */
    if (wait) {
        if (tn_wait("post-encrypt") < 0) {
            tn_push();
            return(-1);
        }
        wait = 0;
    }
#endif /* CK_ENCRYPTION */

    tn_sdsb();

    if (tn_init)                   /* tn_ini() may be called recursively */
        return(0);

#ifdef IKS_OPTION
    /* Kermit Server negotiation must go last */
    /* Send U before ME */

    if (!TELOPT_U(TELOPT_KERMIT) &&
        TELOPT_U_MODE(TELOPT_KERMIT) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_KERMIT) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_KERMIT) = 1;
        wait = 1;
    }
    if (!TELOPT_ME(TELOPT_KERMIT) &&
        TELOPT_ME_MODE(TELOPT_KERMIT) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_KERMIT) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_KERMIT) = 1;
        wait = 1;
    }
#endif /* IKS_OPTION */

    if (wait) {
        if (tn_wait("end of telnet negotiations") < 0) {
            tn_push();
            return(-1);
        }
        wait = 0;
    }

    tn_sdsb();                          /* Send delayed subnegotiations */
    tn_push();
    tn_init = 1;                        /* Remember successful completion. */

    /* Don't send anything else! */

    debug(F101,"tn_ini duplex","",duplex);
    debug(F101,"tn_ini done, tn_init","",tn_init);
    return(1);
}

VOID
tn_debug(s) char *s; {
#ifdef OS2
    void cwrite(unsigned short);
    char *p = s;
    _PROTOTYP (void os2bold, (void));
#endif /* OS2 */

    if (!(tn_deb || debses))
      return;
    debug(F111,"tn_debug",s,what);
#ifdef OS2
    if ( what == W_COMMAND ) {
        extern unsigned char colorcmd;
        colorcmd ^= 0x8 ;
        printf("%s\r\n",s);
        colorcmd ^= 0x8 ;
    }
    if (!scrninitialized[VTERM]) {
        USHORT x,y;
        checkscreenmode();
        GetCurPos(&y, &x);
        SaveCmdMode(x+1,y+1);
        scrninit();
        RestoreCmdMode();
    }
    os2bold();                          /* Toggle boldness */
    while (*p)
      cwrite((CHAR) *p++);              /* Go boldly ... */
    os2bold();                          /* Toggle boldness back */
    if (debses) {
        debses = 0;
        cwrite((CHAR) '\015');
        cwrite((CHAR) '\012');
        debses = 1;
    } else {
        cwrite((CHAR) '\015');
        cwrite((CHAR) '\012');
    }
#else
    if (what != W_CONNECT && what != W_COMMAND)
      return;                           /* CONNECT/command must be active */
    conoll(s);
#endif /* OS2 */
}

/*
  Process in-band Telnet negotiation characters from the remote host.
  Call with the telnet IAC character and the current duplex setting
  (0 = remote echo, 1 = local echo), and a pointer to a function to call
  to read more characters.  Returns:
    6 if DO LOGOUT was received and accepted
    5 if the Kermit start of packet character has changed
    4 if state of remote Internet Kermit Service has changed
    3 if a quoted IAC was received
    2 if local echo must be changed to remote
    1 if remote echo must be changed to local
    0 if nothing happens or no action necessary
   -1 on failure (= internal or i/o error)
*/
#ifdef IKS_OPTION
int
tn_siks(cmd) int cmd; {         /* TELNET SEND IKS SUB */
#ifndef NOXFER
    CHAR buf[8];
    extern CHAR mystch;                 /* Outgoing Start of Packet Char */
    int n,m;

    if (ttnet != NET_TCPB) return(0);   /* Must be TCP/IP */
    if (ttnproto != NP_TELNET) return(0); /* Must be telnet protocol */
    if (cmd < KERMIT_START || cmd > KERMIT_RESP_STOP) /* Illegal subcommand */
      return(-1);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */
    if (cmd == KERMIT_START || cmd == KERMIT_RESP_START) {
        TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 1;
    } else if (cmd == KERMIT_STOP || cmd == KERMIT_RESP_STOP) {
        TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 0;
    } else if (cmd == KERMIT_REQ_STOP)
      TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop = 1;
    else if (cmd == KERMIT_REQ_START)
      TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start = 1;

    if (cmd == KERMIT_SOP) {
        buf[0] = (CHAR) IAC;
        buf[1] = (CHAR) SB;
        buf[2] = (CHAR) TELOPT_KERMIT;
        buf[3] = (CHAR) (cmd & 0xff);
        buf[4] = (CHAR) mystch;
        buf[5] = (CHAR) IAC;
        buf[6] = (CHAR) SE;
        buf[7] = (CHAR) 0;
        if (tn_deb || debses || deblog) {
            sprintf(tn_msg,"TELNET SENT SB KERMIT SOP %02X IAC SE",mystch);
            debug(F101,tn_msg,"",cmd);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
        if (ttol(buf,7) < 7)
          return(-1);
    } else {
        buf[0] = (CHAR) IAC;
        buf[1] = (CHAR) SB;
        buf[2] = (CHAR) TELOPT_KERMIT;
        buf[3] = (CHAR) (cmd & 0xff);
        buf[4] = (CHAR) IAC;
        buf[5] = (CHAR) SE;
        buf[6] = (CHAR) 0;
        if (tn_deb || debses || deblog) {
            char * s = 0;
            switch (cmd) {
              case KERMIT_START: s = "START"; break;
              case KERMIT_STOP: s = "STOP"; break;
              case KERMIT_REQ_START: s = "REQ-START"; break;
              case KERMIT_REQ_STOP: s = "REQ-STOP"; break;
              case KERMIT_RESP_START: s = "RESP-START"; break;
              case KERMIT_RESP_STOP:  s = "RESP-STOP"; break;
            }
            sprintf(tn_msg,"TELNET SENT SB kermit %s IAC SE",s);
            debug(F101,tn_msg,"",cmd);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
        if (ttol(buf,6) < 6)
          return(-1);
    }
    return(1);
#else
    return(-1);
#endif /* NOXFER */
}
#endif /* IKS_OPTION */

/* tn_sb() performs Telnet Subnegotiation Parsing and Debugging */
/* returns <= 0 on error, 1 on success */
/* the length returned includes the IAC SE bytes */

int
#ifdef CK_ANSIC                         /* TELNET SB */
tn_sb( int opt, int * len, int (*fn)(int) )
#else
tn_sb( opt, len, fn ) int opt; int * len; int (*fn)();
#endif /* CK_ANSIC */
/* tn_sb */ {
    int c, x, y, n, m, flag;
    debug(F100,"Entering tn_sb()","",0);
    *len = 0;                   /* Initialize Len to 0 */
    n = flag = 0;               /* Flag for when done reading SB */
    while (n < TSBUFSIZ) {      /* Loop looking for IAC SE */
        if ((y = (*fn)(0)) < 0) /* Read a byte */
          return(y);
        y &= 0xff;              /* Make sure it's just 8 bits. */
        sb[n++] = (char) y;     /* Deposit in buffer. */
        if (seslog && sessft == XYFT_D) { /* Take care of session log */
            logchar((char) y);
        }
        if (y == IAC) {         /* If this is an IAC                */
            if (flag) {         /* If previous char was IAC         */
                n--;            /* it's quoted, keep one IAC        */
                flag = 0;       /* and turn off the flag.           */
            } else flag = 1;    /* Otherwise set the flag.          */
        } else if (flag) {      /* Something else following IAC     */
            if (y == SE)        /* If not SE, it's a protocol error */
              break;
            else if (y == DONT) { /* Used DONT instead of SE */
                debug(F100,
                      "TELNET Subnegotiation error - used DONT instead of SE!",
                      ""
                      ,0
                      );
                if (tn_deb || debses)
                  tn_debug(
                     "TELNET Subnegotiation error - used DONT instead of SE!");
                flag = 3;
                break;
            } else {            /* Other protocol error */
                flag = 0;
                break;
            }
        }
#ifdef COMMENT
        /* This test is sure to be triggered when we are using encryption */
        /* Therefore, we must take it out, otherwise, our subnegotiations */
        /* will be truncated and failure will result.                     */
        /* This bug was only seen on BeOS DR7 and has been fixed.         */
        else if (!flag && y == SE) { /* Forgot the IAC ? */
            flag = 2;
            debug(F100,
                   "TELNET Subnegotiation error - forgot the IAC before SE!",
                   "",
                  0
                  );
            if (tn_deb || debses)
              tn_debug(
"TELNET Subnegotiation error - forgot the IAC before SE!");
            break;
        }
#endif /* COMMENT */

#ifdef CK_FORWARD_X
        if ( opt == TELOPT_FORWARD_X && sb[0] == FWDX_DATA &&
             n == (TSBUFSIZ-2) ) {
            /* do not let the buffer over flow */
            /* write the data to the channel and continue processing */
            /* the incoming data until IAC SE is reached. */
            sb[n++] = IAC;
            sb[n++] = SE;
            if ( deblog || tn_deb || debses ) {
                int i;
                sprintf(tn_msg,"TELNET RCVD SB %s DATA ",TELOPT(opt));
#ifdef HEXDISP
                {
                    int was_hex = 1;
                    for (i=1; i < n-2; i++) {
                        if (sb[i] < 32 || sb[i] >= 127) {
                            sprintf(hexbuf,"%s%02X ",was_hex?"":"\" ",sb[i]);
                            was_hex = 1;
                        } else {
                            sprintf(hexbuf,"%s%c",was_hex?"\"":"",sb[i]);
                            was_hex = 0;
                        }
                        strcat(tn_msg,hexbuf);
                    }
                    if (!was_hex)
                        strcat(tn_msg,"\" ");
                }
#else /* HEXDISP */
                memcpy(hexbuf,&sb[i],n-i-2);
                hexbuf[n-i-2] = ' ';
                hexbuf[n-i-1] = '\0';
                strcat(tn_msg,hexbuf);
#endif /* HEXDISP */
                if (flag == 2)
                    strcat(tn_msg,"SE");
                else if (flag == 3)
                    strcat(tn_msg," IAC DONT");
                else
                    strcat(tn_msg," IAC SE");
                debug(F100,tn_msg,"",0);
                if (tn_deb || debses)
                    tn_debug(tn_msg);
            }

            if ( fwdx_tn_sb(sb,n) < 0 )
                return(0);

            /* reset leave the msg type and channel number in place */
            n = 3;
        }
#endif /* CK_FORWARD_X */
    }
    debug(F111,"tn_sb end of while loop","flag",flag);
    if (!flag) {                        /* Make sure we got a valid SB */
        debug(F111, "TELNET Subnegotiation prematurely broken","opt",opt);
        if (tn_deb || debses)
          tn_debug("TELNET Subnegotiation prematurely broken");
        /* Was -1 but that would be an I/O error, so absorb it and go on. */
        return(0);
    }
    if (deblog || tn_deb || debses) {
        int i;
        char * s[16];
        for (i = 0; i < 16; i++)
          s[i] = "";
        if (opt == TELOPT_NAWS) {
            i = 0;
        } else {
            i = 1;
            s[0] = "UNKNOWN";

            switch (sb[0]) {
              case 0:
                if (opt == TELOPT_FORWARD_X)
                  s[0] = "SCREEN";
                else if (opt == TELOPT_KERMIT)
                  s[0] = "START";
                else if (opt == TELOPT_LFLOW)
                  s[0] = "OFF";
                else
                  s[0] = "IS";
                if (opt == TELOPT_ENCRYPTION) {
                    i++;
                    if (sb[1] < ENCTYPE_CNT) {
                        s[1] = enctype_names[sb[1]];
                        i++;
                        switch(sb[2]) {
                          case 1:
                            s[2] = "FB64_IV";
                            break;
                          case 2:
                            s[2] = "FB64_IV_OK";
                            break;
                          case 3:
                            s[2] = "FB64_IV_BAD";
                            break;
                          case 4:
                            s[2] = "FB64_CHALLENGE";
                            break;
                          case 5:
                            s[2] = "FB64_RESPONSE";
                            break;
                        }
                    } else {
                        s[1] = "UNKNOWN";
                    }
                }
                if (opt == TELOPT_AUTHENTICATION) {
                    i += 2;
                    s[1] = AUTHTYPE_NAME(sb[1]);
                    s[2] = AUTHMODE_NAME(sb[2]);
                    if (sb[1]) {
                        i++;
                        switch (sb[3]) {
                          case 0:
                            switch (sb[1]) {
                              case AUTHTYPE_NTLM:
                                s[3] = "NTLM_AUTH";
                                break;
                              default:
                                s[3] = "AUTH";
                            }
                            break;
                          case 1:
                            switch (sb[1]) {
                              case AUTHTYPE_SSL:
                                s[3] = "START";
                                break;
                              case AUTHTYPE_NTLM:
                                s[3] = "NTLM_CHALLENGE";
                                break;
                              default:
                                s[3] = "REJECT";
                            }
                            break;
                          case 2:
                            switch (sb[1]) {
                              case AUTHTYPE_NTLM:
                                s[3] = "NTLM_RESPONSE";
                                break;
                              default:
                                s[3] = "ACCEPT";
                            }
                            break;
                          case 3:
                            switch (sb[1]) {
                              case AUTHTYPE_NTLM:
                                s[3] = "NTLM_ACCEPT";
                                break;
                              case 1:   /* KERBEROS_v4 */
                              case 5:   /* SRP */
                                s[3] = "CHALLENGE";
                                break;
                              case 2:   /* KERBEROS_v5 */
                                s[3] = "RESPONSE";
                                break;
                              case AUTHTYPE_SSL:
                                s[3] = "REJECT";
                                break;
                            }
                            break;
                          case 4:
                            switch (sb[1]) {
                              case AUTHTYPE_NTLM:
                                s[3] = "NTLM_REJECT";
                                break;
                              case 1:   /* KERBEROS_V4 */
                              case 5:   /* SRP */
                                s[3] = "RESPONSE";
                                break;
                              case 2:   /* KERBEROS_V5 */
                                s[3] = "FORWARD";
                                break;
                            }
                            break;
                          case 5:
                            switch (sb[1]) {
                              case 5:   /* SRP */
                                s[3] = "FORWARD";
                                break;
                              case 2:   /* KERBEROS_V5 */
                                s[3] = "FORWARD_ACCEPT";
                                break;
                            }
                            break;
                          case 6:
                            switch (sb[1]) {
                              case 5:   /* SRP */
                                s[3] = "FORWARD_ACCEPT";
                                break;
                              case 2: /* KERBEROS_V5 */
                                s[3] = "FORWARD_REJECT";
                                break;
                            }
                            break;
                          case 7:
                            switch (sb[1]) {
                              case 5:   /* SRP */
                                s[3] = "FORWARD_REJECT";
                                break;
                            }
                            break;
                          case 8:
                            switch (sb[1]) {
                              case 5: /* SRP */
                                s[3] = "EXP";
                                break;
                            }
                            break;
                          case 9:
                            switch (sb[1]) {
                              case 5: /* SRP */
                                s[3] = "PARAMS";
                                break;
                            }
                            break;
                        }
                    }
                }
                break;
              case 1:
                switch (opt) {
                  case TELOPT_FORWARD_X:
                    s[0] = "OPEN";
                    break;
                  case TELOPT_LFLOW:
                    s[0] = "ON";
                    break;
                  case TELOPT_KERMIT:
                    s[0] = "STOP";
                    break;
                  case TELOPT_AUTHENTICATION:
                    s[0] = "SEND";
                    hexbuf[0] = '\0';
                    for (; i < n-2; i += 2) {
                        sprintf(tn_msg,
                                "%s %s ",
                                authtype_names[sb[i]],
                                authmode_names[sb[i+1]]
                                );
                        strcat(hexbuf,tn_msg);
                    }
                    s[1] = hexbuf;
                    break;

                  case TELOPT_ENCRYPTION:
                    s[0] = "SUPPORT";
                    while (i < n-2) {
                        s[i] = enctype_names[sb[i]];
                        i++;
                    }
                    break;

                  case TELOPT_START_TLS:
                    s[0] = "FOLLOWS";
                    break;
                  default:
                    s[0] = "SEND";
                }
                break;

              case 2:
                switch (opt) {
                case TELOPT_FORWARD_X:
                    s[0] = "CLOSE";
                    break;
                  case TELOPT_LFLOW:
                    s[0] = "RESTART-ANY";
                    break;
                  case TELOPT_KERMIT:
                    s[0] = "REQ-START";
                    break;
                  case TELOPT_NEWENVIRON:
                    s[0] = "INFO";
                    break;
                  case TELOPT_AUTHENTICATION:
                    s[0] = "REPLY";
                    i=4;
                    s[1] = authtype_names[sb[1]];
                    s[2] = authmode_names[sb[2]];
                    switch (sb[3]) {
                      case 0:
                        switch (sb[1]) {
                          case AUTHTYPE_NTLM:
                            s[3] = "NTLM_AUTH";
                            break;
                          default:
                            s[3] = "AUTH";
                        }
                        break;
                      case 1:
                        switch (sb[1]) {
                          case AUTHTYPE_NTLM:
                            s[3] = "NTLM_CHALLENGE";
                            break;
                          default:
                            s[3] = "REJECT";
                        }
                        break;
                      case 2:
                        switch (sb[1]) {
                          case AUTHTYPE_NTLM:
                            s[3] = "NTLM_RESPONSE";
                            break;
                          default:
                            s[3] = "ACCEPT";
                        }
                        break;
                      case 3:
                        switch (sb[1]) {
                          case AUTHTYPE_NTLM:
                            s[3] = "NTLM_ACCEPT";
                            break;
                          case AUTHTYPE_KERBEROS_V4:
                          case AUTHTYPE_SRP:
                            s[3] = "CHALLENGE";
                            break;
                          case AUTHTYPE_KERBEROS_V5:
                            s[3] = "RESPONSE";
                            break;
                        }
                        break;
                      case 4:
                        switch (sb[1]) {
                          case AUTHTYPE_NTLM:
                            s[3] = "NTLM_REJECT";
                            break;
                          case AUTHTYPE_KERBEROS_V4:
                          case AUTHTYPE_SRP:
                            s[3] = "RESPONSE";
                            break;
                          case AUTHTYPE_KERBEROS_V5:
                            s[3] = "FORWARD";
                            break;
                        }
                        break;
                      case 5:
                        switch (sb[1]) {
                          case AUTHTYPE_SRP:
                            s[3] = "FORWARD";
                            break;
                          case AUTHTYPE_KERBEROS_V5:
                            s[3] = "FORWARD_ACCEPT";
                            break;
                        }
                        break;
                      case 6:
                        switch (sb[1]) {
                          case AUTHTYPE_SRP:
                            s[3] = "FORWARD_ACCEPT";
                            break;
                          case AUTHTYPE_KERBEROS_V5:
                            s[3] = "FORWARD_REJECT";
                            break;
                        }
                        break;
                      case 7:
                        switch (sb[1]) {
                          case AUTHTYPE_SRP:
                            s[3] = "FORWARD_REJECT";
                            break;
                        }
                        break;
                      case 8:
                        switch (sb[1]) {
                          case AUTHTYPE_SRP:
                            s[3] = "EXP";
                            break;
                        }
                        break;
                      case 9:
                        switch (sb[1]) {
                          case AUTHTYPE_SRP:
                            s[3] = "PARAMS";
                            break;
                        }
                        break;
                    }
                    break;
                  case TELOPT_ENCRYPTION:
                    s[0] = "REPLY";
                    s[1] = enctype_names[sb[1]];
                    i++;
                    switch (sb[2]) {
                      case 1:
                        i++;
                        s[2] = "FB64_IV";
                        break;
                      case 2:
                        i++;
                        s[2] = "FB64_IV_OK";
                        break;
                      case 3:
                        i++;
                        s[2] = "FB64_IV_BAD";
                        break;
                      case 4:
                        i++;
                        s[2] = "FB64_CHALLENGE";
                        break;
                      case 5:
                        i++;
                        s[2] = "FB64_RESPONSE";
                        break;
                    }
                    break;
                }
                break;
              case 3:
                switch (opt) {
                  case TELOPT_FORWARD_X:
                    s[0] = "DATA";
                    break;
                  case TELOPT_LFLOW:
                    s[0] = "RESTART-XON";
                    break;
                  case TELOPT_KERMIT:
                    s[0] = "REQ-STOP";
                    break;
                  case TELOPT_AUTHENTICATION:
                    s[0] = "NAME";
                    break;
                  case TELOPT_ENCRYPTION:
                    s[0] = "START";
                    break;
                }
                break;
              case 4:
                switch (opt) {
                  case TELOPT_KERMIT:
                    s[0] = "SOP";
                    break;
                  case TELOPT_ENCRYPTION:
                    s[0] = "END";
                    break;
                }
                break;
              case 5:
                switch (opt) {
                case TELOPT_ENCRYPTION:
                    s[0] = "REQUEST-START";
                    break;
                }
                break;
              case 6:
                switch (opt) {
                  case TELOPT_ENCRYPTION:
                    s[0] = "REQUEST-END";
                    break;
                }
                break;
              case 7:
                switch (opt) {
                  case TELOPT_ENCRYPTION:
                    s[0] = "ENC-KEYID";
                    break;
                }
                break;
              case 8:
                switch (opt) {
                  case TELOPT_KERMIT:
                    s[0] = "RESP-START";
                    break;
                  case TELOPT_ENCRYPTION:
                    s[0] = "DEC-KEYID";
                    break;
                }
                break;
              case 9:
                switch (opt) {
                  case TELOPT_KERMIT:
                    s[0] = "RESP-STOP";
                    break;
                }
                break;
            }
        }
#ifdef M_XENIX
        {
          int len, param, param_len;
          sprintf(tn_msg,
                "TELNET RCVD SB %s ",
                TELOPT(opt));
          len = strlen(tn_msg);
          for (param = 0; param <= 15; param++) {
            param_len = strlen(s[param]);
            if (param_len > 0) {
              strcpy(&tn_msg[len], s[param]);
              len += param_len;
              tn_msg[len++] = ' ';
            }
          }
          tn_msg[len] = '\0';
        }
#else
        sprintf(tn_msg,
                "TELNET RCVD SB \
%s %s %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                TELOPT(opt),s[0],
                 s[ 1], s[ 1][0]?" ":"",
                 s[ 2], s[ 2][0]?" ":"",
                 s[ 3], s[ 3][0]?" ":"",
                 s[ 4], s[ 4][0]?" ":"",
                 s[ 5], s[ 5][0]?" ":"",
                 s[ 6], s[ 6][0]?" ":"",
                 s[ 7], s[ 7][0]?" ":"",
                 s[ 8], s[ 8][0]?" ":"",
                 s[ 9], s[ 9][0]?" ":"",
                 s[10], s[10][0]?" ":"",
                 s[11], s[11][0]?" ":"",
                 s[12], s[12][0]?" ":"",
                 s[13], s[13][0]?" ":"",
                 s[14], s[14][0]?" ":"",
                 s[15], s[15][0]?" ":"");
#endif /* M_XENIX */
#ifdef HEXDISP
        {
            int was_hex = 1;
            for (; i < n-2; i++) {
                if (sb[i] < 32 || sb[i] >= 127) {
                    sprintf(hexbuf,"%s%02X ",was_hex?"":"\" ",sb[i]);
                    was_hex = 1;
                } else {
                    sprintf(hexbuf,"%s%c",was_hex?"\"":"",sb[i]);
                    was_hex = 0;
                }
                strcat(tn_msg,hexbuf);
            }
            if (!was_hex)
              strcat(tn_msg,"\" ");
        }
#else /* HEXDISP */
        memcpy(hexbuf,&sb[i],n-i-2);
        hexbuf[n-i-2] = ' ';
        hexbuf[n-i-1] = '\0';
        strcat(tn_msg,hexbuf);
#endif /* HEXDISP */
        if (flag == 2)
          strcat(tn_msg,"SE");
        else if (flag == 3)
          strcat(tn_msg," IAC DONT");
        else
          strcat(tn_msg," IAC SE");
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses)
          tn_debug(tn_msg);
    }
    *len = n;           /* return length */
    return(1);          /* success */
}

static char rows_buf[16] = { 0, 0 }; /* LINES Environment variable */
static char cols_buf[16] = { 0, 0 }; /* COLUMNS Enviornment variable */
static char term_buf[64] = { 0, 0 }; /* TERM Environment variable */

#ifdef CK_CURSES
#ifndef VMS
#ifndef COHERENT
_PROTOTYP(int tgetent,(char *, char *));
#endif /* COHERENT */
#else
#ifdef __DECC
_PROTOTYP(int tgetent,(char *, char *));
#endif /* __DECC */
#endif /* VMS */
extern char * trmbuf;                   /* Real curses */
#endif /* CK_CURSES */

#ifdef CK_ENCRYPTION
static int
tn_no_encrypt()
{
    /* Prevent Encryption from being negotiated */
    TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
    TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;

    /* Cancel any negotiation that might have started */
    ck_tn_enc_stop();

    if (TELOPT_ME(TELOPT_ENCRYPTION) ||
         TELOPT_UNANSWERED_WILL(TELOPT_ENCRYPTION)) {
        TELOPT_ME(TELOPT_ENCRYPTION) = 0;
        if (tn_sopt(WONT,TELOPT_ENCRYPTION) < 0)
            return(-1);
        TELOPT_UNANSWERED_WONT(TELOPT_ENCRYPTION) = 1;
    }
    if (TELOPT_U(TELOPT_ENCRYPTION) ||
         TELOPT_UNANSWERED_DO(TELOPT_ENCRYPTION)) {
        TELOPT_U(TELOPT_ENCRYPTION) = 0;
        if (tn_sopt(DONT,TELOPT_ENCRYPTION) < 0)
            return(-1);
        TELOPT_UNANSWERED_DONT(TELOPT_ENCRYPTION) = 1;
    }
    return(0);
}
#endif /* CK_ENCRYPTION */

/* The following note came from the old SGA negotiation code.  This should */
/* no longer be necessary with the New Telnet negotiation state machine.   */
/*
  Note: The following is proper behavior, and required for talking to the
  Apertus interface to the NOTIS library system, e.g. at Iowa State U:
  scholar.iastate.edu.  Without this reply, the server hangs forever.  This
  code should not be loop-inducing, since C-Kermit never sends WILL SGA as
  an initial bid, so if DO SGA comes, it is never an ACK.
*/
/*
  Return values:
  -1 = Telnet Opton negotiation error
  -2 = Connection closed by peer
  -3 = Connection closed by us
  0  = Success
  1  = Echoing on
  2  = Echoing off
  3  = Quoted IAC
  4  = IKS Event
  5  = (unassigned)
  6  = Logout
*/
int
#ifdef CK_ANSIC                         /* TELNET DO OPTION */
tn_doop(CHAR z, int echo, int (*fn)(int))
#else
tn_doop(z, echo, fn) CHAR z; int echo; int (*fn)();
#endif /* CK_ANSIC */
/* tn_doop */ {
    int c, x, y, n, m, flag;
#ifdef IKS_OPTION
    extern int server;
    extern int local;
#ifdef NOICP
    extern int autodl;
    int inautodl = 0, cmdadl = 1;
#else
#ifdef CK_AUTODL
    extern int autodl, inautodl, cmdadl;
#endif /* CK_AUTODL */
#endif /* NOICP */
#endif /* IKS_OPTION */

    if (z != (CHAR) IAC) {
        debug(F101,"tn_doop bad call","",z);
        return(-1);
    }
    if (ttnet != NET_TCPB)              /* Check network type */
      return(0);
    if (ttnproto != NP_TELNET &&
         ttnproto != NP_NONE)           /* Check protocol */
      return(0);

/* Have IAC, read command character. */

    while ((c = (*fn)(0)) == -1);       /* Read command character */
    if (c < 0)
      return(c);
    c &= 0xFF;                          /* Strip high bits */

    if (!TELCMD_OK(c)) {
        sprintf(tn_msg,"TELNET RCVD UNKNOWN (%d)",c);
        debug(F101,tn_msg,"",c);
        if (tn_deb || debses) tn_debug(tn_msg);
        return(0);
    }
    if (ttnproto == NP_NONE) {
        debug(F100,"tn_doop discovered a Telnet command",
              "ttnproto = NP_TELNET",0);
        ttnproto = NP_TELNET;
    }
    if (seslog && sessft == XYFT_D) {   /* Copy to session log, if any. */
        logchar((char)z);
        logchar((char)c);
    }

    if (c == (CHAR) IAC)                /* Quoted IAC */
      return(3);

    if (c < SB) {                       /* Other command with no arguments. */
        if (deblog || tn_deb || debses) {
            sprintf(tn_msg,"TELNET RCVD %s",TELCMD(c));
            debug(F101,tn_msg,"",c);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
        switch (c) {                    /* What we would like to do here    */
          case TN_GA:                   /* Is substitute ASCII characters   */
            break;                      /* for the Telnet Command so that   */
          case TN_EL:                   /* the command may be processed by  */
            break;                      /* either the internal emulator or  */
          case TN_EC:                   /* by the superior process or shell */
            break;
          case TN_AYT:
            ttol((CHAR *)"[Yes]\015\012",7);
            break;
          case TN_AO:
#ifdef BETATEST
            bleep(BP_NOTE);
#endif /* BETATEST */
            break;
          case TN_IP:
            break;
          case BREAK:
            break;
          case TN_DM:
            break;
          case TN_NOP:
            break;
          case SE:
            break;
          case TN_EOR:
            break;
          case TN_ABORT:
            break;
          case TN_SUSP:
            break;
          case TN_EOF:
            break;
          case TN_SAK:
            break;
        }
        return(0);
    }

/* SB, WILL, WONT, DO, or DONT need more bytes... */

    if ((x = (*fn)(0)) < 0)             /* Get the option. */
      return(x);
    x &= 0xff;                          /* Trim to 8 bits. */

    if (seslog && sessft == XYFT_D) {   /* Session log */
        logchar((char) x);
    }
    if ((deblog || tn_deb || debses) && c != SB) {
        sprintf(tn_msg,"TELNET RCVD %s %s",TELCMD(c),TELOPT(x));
        debug(F101,tn_msg,"",x);
        if (tn_deb || debses) tn_debug(tn_msg);
    }

    /* Now handle the command */
    switch (c) {
      case WILL:
#ifdef CK_SSL
        if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
            return(0);
#endif /* CK_SSL */
        if (!TELOPT_OK(x) || TELOPT_U_MODE(x) == TN_NG_RF) {
            if (tn_sopt(DONT,x) < 0)
              return(-1);
            if (TELOPT_UNANSWERED_DO(x))
                TELOPT_UNANSWERED_DO(x) = 0;
        } else if (!TELOPT_U(x)) {
            if (!TELOPT_UNANSWERED_DO(x)) {
                if (tn_sopt(DO,x) < 0)
                  return -1;
            }
            if (TELOPT_UNANSWERED_DO(x))
                TELOPT_UNANSWERED_DO(x) = 0;
            TELOPT_U(x) = 1;

            switch (x) {
#ifdef CK_SSL
              case TELOPT_START_TLS:
                /*
                   If my proposal is accepted, at this point the Telnet
                   protocol is turned off and a TLS negotiation takes
                   place.

                   Start by sending SB START_TLS FOLLOWS  to signal
                   we are ready.  Wait for the peer to send the same
                   and then start the TLS negotiation.

                   If the TLS negotiation succeeds we call tn_ini()
                   again to reset the telnet state machine and restart
                   the negotiation process over the now secure link.

                   If the TLS negotiation fails, we call ttclos()
                   to terminate the connection.

                   Only the server should receive a WILL START_TLS
                 */
                tn_ssbopt(TELOPT_START_TLS,1,NULL,0);
                TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows = 1;
                break;
#endif /* CK_SSL */

#ifdef CK_AUTHENTICATION
              case TELOPT_AUTHENTICATION: {
                  /* We now have to perform a SB SEND to identify the  */
                  /* supported authentication types to the other side. */
                  extern int authentication_version;
                  authentication_version = AUTHTYPE_AUTO;
                  ck_tn_auth_request();
                  break;
              }
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
              case TELOPT_ENCRYPTION:
                if (!(TELOPT_ME(TELOPT_AUTHENTICATION) ||
                      TELOPT_U(TELOPT_AUTHENTICATION))
                    ) {
                    if (tn_sopt(DONT,x) < 0)
                      return(-1);
                    TELOPT_U(x) = 0;
                } else {
                    if (ck_tn_auth_in_progress()) {
                        TELOPT_SB(TELOPT_ENCRYPTION).encrypt.need_to_send = 1;
                    } else {
                        /* Perform subnegotiation */
                        ck_encrypt_send_support();
                    }
                    if (!(TELOPT_ME(x) || TELOPT_UNANSWERED_WILL(x))
                        && TELOPT_ME_MODE(x) != TN_NG_RF) {
                        if (tn_sopt(WILL, x) < 0)
                          return(-1);
                        TELOPT_UNANSWERED_WILL(x) = 1;
                    }
                }
                break;
#endif /* CK_ENCRYPTION */
#ifdef IKS_OPTION
              case TELOPT_KERMIT:
                if (!TELOPT_ME(x)) {
                    /* Tell the other side what Start of Packet Character */
                    tn_siks(KERMIT_SOP); /* SOP */

                    if (!TELOPT_UNANSWERED_WILL(x) &&
                        TELOPT_ME_MODE(x) != TN_NG_RF) {
                        if (tn_sopt(WILL, x) < 0)
                          return(-1);
                        TELOPT_UNANSWERED_WILL(x) = 1;
                    }
                }
                break;
#endif /* IKS_OPTION */
              case TELOPT_BINARY:
                if (!TELOPT_ME(x)) {
                    if (!TELOPT_UNANSWERED_WILL(x) &&
                        TELOPT_ME_MODE(x) >= TN_NG_RQ) {
                        if (tn_sopt(WILL, x) < 0)
                          return(-1);
                        TELOPT_UNANSWERED_WILL(x) = 1;
                    }
                }
                break;
              case TELOPT_ECHO:
                if (echo) {
                    if (TELOPT_UNANSWERED_DO(x))
                      TELOPT_UNANSWERED_DO(x) = 0;
                    return(2);
                }
                break;
              case TELOPT_TTYPE:
                /* SB TTYPE SEND */
                tn_ssbopt(TELOPT_TTYPE,TELQUAL_SEND,NULL,0);
                TELOPT_UNANSWERED_SB(TELOPT_TTYPE)=1;
                break;
#ifdef CK_ENVIRONMENT
              case TELOPT_NEWENVIRON:   /* SB NEW-ENVIRON SEND */
#ifdef CK_AUTHENTICATION
                if (ck_tn_auth_valid() != AUTH_VALID)
#endif /* CK_AUTHENTICATION */
		{
                  char request[6];      /* request it */
                  sprintf(request,"%cUSER",TEL_ENV_VAR);
                  tn_ssbopt(TELOPT_NEWENVIRON,TELQUAL_SEND,request,5);
                  TELOPT_UNANSWERED_SB(TELOPT_NEWENVIRON)=1;
                }
                break;
#endif /* CK_ENVIRONMENT */
#ifdef CK_FORWARD_X
               case TELOPT_FORWARD_X:
                  if ( !tn_outst(0) || tn_init ) {
                      if (tn_sndfwdx() < 0) {
                          if (tn_sopt(DONT,x) < 0)
                              return(-1);
                          TELOPT_UNANSWERED_DONT(x) = 1;
                      }
                  } else {
                      TELOPT_SB(TELOPT_FORWARD_X).forward_x.need_to_send = 1;
                  }
                  break;
#endif /* CK_FORWARD_X */
            } /* switch */
        } else {
            if (TELOPT_UNANSWERED_DO(x))
                TELOPT_UNANSWERED_DO(x) = 0;
        }
        break;
      case WONT:
#ifdef CK_SSL
        if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
            return(0);
#endif /* CK_SSL */
        if (!TELOPT_OK(x) || TELOPT_U(x) || TELOPT_UNANSWERED_DO(x)) {
            /* David Borman says we should not respond DONT when */
            /* the WONT is a response to a DO that we sent.   */
            if (!(TELOPT_UNANSWERED_DO(x) || TELOPT_UNANSWERED_DONT(x)))
              if (tn_sopt(DONT,x) < 0)
                return(-1);
            if (TELOPT_UNANSWERED_DONT(x))
                TELOPT_UNANSWERED_DONT(x) = 0;
            if (TELOPT_UNANSWERED_DO(x))
                TELOPT_UNANSWERED_DO(x) = 0;
            if (TELOPT_U(x)) {
                TELOPT_U(x) = 0;
            }
            switch(x) {
#ifdef CK_SSL
            case TELOPT_START_TLS:
                if (sstelnet && TELOPT_U_MODE(x) == TN_NG_MU) {
                    printf("Telnet Start-TLS refused.\n");
                    ttclos(0);
                    whyclosed = WC_TELOPT;
                    return(-3);
                }
                break;
#endif /* CK_SSL */
#ifdef CK_AUTHENTICATION
              case TELOPT_AUTHENTICATION:
                if (sstelnet && TELOPT_U_MODE(x) == TN_NG_MU) {
                    printf("Telnet authentication refused.\n");
                    ttclos(0);
                    whyclosed = WC_TELOPT;
                    return(-3);
                } else if (TELOPT_U_MODE(x) == TN_NG_RQ) {
                    TELOPT_U_MODE(x) = TN_NG_AC;
                }
                if (ck_tn_auth_in_progress())
                  printf("Telnet authentication refused.\n");
#ifdef CK_ENCRYPTION
                if (sstelnet) {
                    if (tn_no_encrypt()<0)
                        return(-1);
                }
#endif /* CK_ENCRYPTION */
                break;
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
              case TELOPT_ENCRYPTION:
                ck_tn_enc_stop();
                break;
#endif /* CK_ENCRYPTION */
#ifdef IKS_OPTION
              case TELOPT_KERMIT:
                TELOPT_SB(x).kermit.u_start = 0;
                TELOPT_SB(x).kermit.me_req_start = 0;
                TELOPT_SB(x).kermit.me_req_stop = 0;
                break;
#endif /* IKS_OPTION */
              case TELOPT_NAWS: {
                  /* The client does not support NAWS. */
                  /* Assume a height of 24 and a width of 80 */
                  if (sstelnet
#ifdef IKSD
                   || inserver
#endif /* IKSD */
                   ) {
                      int w = 80, h = 24;
                      if (tcp_incoming) {
#ifdef OS2
                          tt_cols[VTERM] = w;
                          tt_rows[VTERM] = h;
                          VscrnSetWidth(VTERM, w);
                          VscrnSetHeight(VTERM, h+(tt_status?1:0));
#else /* OS2 */
                          tt_cols = w;
                          tt_rows = h;
#endif /* OS2 */
                      } else {
#ifdef OS2
                          tt_cols[VCMD] = w;
                          tt_rows[VCMD] = h;
                          VscrnSetWidth(VCMD, w);
                          VscrnSetHeight(VCMD, h);
#endif /* OS2 */
                          cmd_cols = w;
                          cmd_rows = h;
                      }
                      /* Add LINES and COLUMNS to the environment */
                      sprintf((char *)rows_buf,"LINES=%d",h);
                      sprintf((char *)cols_buf,"COLUMNS=%d",w);
#ifdef OS2ORUNIX
#ifndef NOPUTENV
                      putenv(rows_buf);
                      putenv(cols_buf);
#endif /* NOPUTENV */
#endif /* OS2ORUNIX */
                  }
                  break;
              }
              case TELOPT_ECHO:
                if (!echo) {
                    if (TELOPT_UNANSWERED_DO(x))
                      TELOPT_UNANSWERED_DO(x) = 0;
                    return(1);
                }
                break;
            }
        } else {
            if (TELOPT_UNANSWERED_DONT(x))
                TELOPT_UNANSWERED_DONT(x) = 0;
            if (TELOPT_UNANSWERED_DO(x))
                TELOPT_UNANSWERED_DO(x) = 0;
        }
        if (TELOPT_U_MODE(x) == TN_NG_MU) {
            sprintf(tn_msg,
            "Peer refuses TELNET DO %s negotiations - terminating connection",
                    TELOPT(x)
                    );
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
            printf("%s\n",tn_msg);
            ttclos(0);
            whyclosed = WC_TELOPT;
            return(-3);
        }
#ifdef COMMENT
        if (x == TELOPT_ECHO && !echo) /* Special handling for echo */
          return(1);                   /* because we allow 'duplex' */
#endif /* COMMENT */
        break;

      case DO:
#ifdef CK_SSL
        if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
            return(0);
#endif /* CK_SSL */
        if (!TELOPT_OK(x) || TELOPT_ME_MODE(x) == TN_NG_RF) {
            if (tn_sopt(WONT,x) < 0)
              return(-1);
            if (TELOPT_UNANSWERED_WILL(x))
                TELOPT_UNANSWERED_WILL(x) = 0;
        } else if (!TELOPT_ME(x)) {
            if (!TELOPT_UNANSWERED_WILL(x)) {
                if (tn_sopt(WILL,x) < 0)
                  return(-1);
            }
            if (TELOPT_UNANSWERED_WILL(x))
                TELOPT_UNANSWERED_WILL(x) = 0;
            TELOPT_ME(x) = 1;

            switch (x) {
#ifdef CK_SSL
              case TELOPT_START_TLS:
                /*
                   If my proposal is accepted at this point the Telnet
                   protocol is turned off and a TLS negotiation takes
                   place.

                   Start by sending SB START_TLS FOLLOWS  to signal
                   we are ready.  Wait for the peer to send the same
                   and then start the TLS negotiation.

                   If the TLS negotiation succeeds we call tn_ini()
                   again to reset the telnet state machine and restart
                   the negotiation process over the now secure link.

                   If the TLS negotiation fails, we call ttclos()
                   to terminate the connection.  Then we set the
                   U_MODE and ME_MODE for TELOPT_START_TLS to REFUSE
                   and then call ttopen() to create a new connection
                   to the same host but this time do not attempt
                   TLS security.

                   Only the client should receive DO START_TLS.
                */
                tn_ssbopt(TELOPT_START_TLS,1,NULL,0);
                TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows = 1;
                break;
#endif /* CK_SSL */

#ifdef CK_AUTHENTICATION
              case TELOPT_AUTHENTICATION: {
                  /* We don't know what authentication we are using yet */
                  /* but it is not NULL until a failure is detected so */
                  /* use AUTO in the meantime. */
                  extern int authentication_version;
                  authentication_version = AUTHTYPE_AUTO;
                  break;
              }
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
              case TELOPT_ENCRYPTION:
                if (!(TELOPT_ME(TELOPT_AUTHENTICATION) ||
                      TELOPT_U(TELOPT_AUTHENTICATION))
                    ) {
                    if (tn_sopt(WONT,x) < 0)
                      return(-1);
                    TELOPT_ME(x) = 0;
                } else {
                    if (!(TELOPT_U(x) || TELOPT_UNANSWERED_DO(x))
                        && TELOPT_U_MODE(x) != TN_NG_RF) {
                        if (tn_sopt(DO, x) < 0)
                          return(-1);
                        TELOPT_UNANSWERED_DO(x) = 1;
                    }
                }
                break;
#endif /* CK_ENCRYPTION */
#ifdef IKS_OPTION
              case TELOPT_KERMIT:
/* If currently processing Kermit server packets, must tell the other side */

                debug(F111,"tn_doop","what",what);
                debug(F111,"tn_doop","server",server);
#ifdef CK_AUTODL
                debug(F111,"tn_doop","autodl",autodl);
                debug(F111,"tn_doop","inautodl",inautodl);
                debug(F111,"tn_doop","cmdadl",cmdadl);
#endif /* CK_AUTODL */

                if (server
#ifdef CK_AUTODL
                    || (local && ((what == W_CONNECT && autodl) ||
                                  (what != W_CONNECT && inautodl)))
                    || (!local && cmdadl)
#endif /* CK_AUTODL */
                    ) {
                    tn_siks(KERMIT_START);      /* START */
                }
                if (!TELOPT_U(x)) {
                    /* Tell the other side what Start of Packet Character */
                    tn_siks(KERMIT_SOP);             /* SOP */
                    if (!TELOPT_UNANSWERED_DO(x) &&
                        TELOPT_U_MODE(x) != TN_NG_RF) {
                        if (tn_sopt(DO, x) < 0)
                          return(-1);
                        TELOPT_UNANSWERED_DO(x) = 1;
                    }
                }
                break;
#endif /* IKS_OPTION */

              case TELOPT_BINARY:
                if (!TELOPT_U(x)) {
                    if (!TELOPT_UNANSWERED_DO(x) &&
                        TELOPT_U_MODE(x) >= TN_NG_RQ) {
                        if (tn_sopt(DO, x) < 0)
                          return(-1);
                        TELOPT_UNANSWERED_DO(x) = 1;
                    }
                }
                break;
              case TELOPT_NAWS:
#ifndef NOSIGWINCH
#ifdef SIGWINCH
#ifdef UNIX
                if (sw_armed++ < 1) {   /* Catch window-size changes. */
                    debug(F100,"tn_doop arming SIGWINCH","",0);
                    signal(SIGWINCH,winchh);
                }
#else
                debug(F100,"SIGWINCH defined but not used","",0);
#endif /* UNIX */
#else  /* SIGWINCH */
                debug(F100,"SIGWINCH not defined","",0);
#endif /* SIGWINCH */
#else  /* NOSIGWINCH */
                debug(F100,"NOSIGWINCH defined","",0);
#endif /* NOSIGWINCH */
#ifdef CK_NAWS
                if ( !tn_outst(0) || tn_init ) {
                    if (tn_snaws() < 0)
                        return(-1);
                } else {
                    TELOPT_SB(TELOPT_NAWS).naws.need_to_send = 1;
                }
#endif /* CK_NAWS */
                break;
              case TELOPT_LOGOUT:
                ttclos(0);              /* And then hangup */
                whyclosed = WC_TELOPT;
#ifdef IKSD
                if (inserver
#ifndef NOLOCAL
                    && !local
#endif /* NOLOCAL */
                    )
                  doexit(GOOD_EXIT,0);
#endif /* IKSD */
                if (TELOPT_UNANSWERED_WILL(x))
                  TELOPT_UNANSWERED_WILL(x) = 0;
                return(6);
#ifdef CK_SNDLOC
               case TELOPT_SNDLOC:
                  if ( !tn_outst(0) || tn_init ) {
                      if (tn_sndloc() < 0)
                          return(-1);
                  } else {
                      TELOPT_SB(TELOPT_SNDLOC).sndloc.need_to_send = 1;
                  }
                  break;
#endif /* CK_SNDLOC */
            } /* switch */
        } else {
          if (TELOPT_UNANSWERED_WILL(x))
            TELOPT_UNANSWERED_WILL(x) = 0;
        }
        break;

      case DONT:
#ifdef CK_SSL
        if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
            return(0);
#endif /* CK_SSL */
        if (!TELOPT_OK(x) || TELOPT_ME(x) || TELOPT_UNANSWERED_WILL(x)) {
            /* David Borman says we should not respond DONT when */
            /* the WONT is a response to a WILL that we sent. */
            if (!(TELOPT_UNANSWERED_WILL(x) || TELOPT_UNANSWERED_WONT(x)))
              if (tn_sopt(WONT,x) < 0)
                return(-1);

            if (TELOPT_UNANSWERED_WILL(x))
                TELOPT_UNANSWERED_WILL(x) = 0;
            if (TELOPT_UNANSWERED_WONT(x))
                TELOPT_UNANSWERED_WONT(x) = 0;
            if (TELOPT_ME(x))
              TELOPT_ME(x) = 0;

            switch (x) {
#ifdef CK_SSL
            case TELOPT_START_TLS:
                if (!sstelnet && TELOPT_ME_MODE(x) == TN_NG_MU) {
                    printf("Telnet Start-TLS refused.\n");
                    ttclos(0);
                    whyclosed = WC_TELOPT;
                    return(-3);
                }
                break;
#endif /* CK_SSL */
#ifdef CK_AUTHENTICATION
              case TELOPT_AUTHENTICATION:
                if (!sstelnet && TELOPT_ME_MODE(x) == TN_NG_MU) {
                    printf("Telnet authentication refused.\n");
                    ttclos(0);
                    whyclosed = WC_TELOPT;
                    return(-3);
                } else if (TELOPT_ME_MODE(x) == TN_NG_RQ) {
                    TELOPT_ME_MODE(x) = TN_NG_AC;
                }
                if (ck_tn_auth_in_progress())
                  printf("Telnet authentication refused.\n");
#ifdef CK_ENCRYPTION
                if (!sstelnet) {
                    if (tn_no_encrypt()<0)
                        return(-1);
                }
#endif /* CK_ENCRYPTION */
                break;
#endif /* CK_AUTHENTICATION */
              case TELOPT_ENCRYPTION:
#ifdef CK_ENCRYPTION
                ck_tn_enc_stop();
#endif /* CK_ENCRYPTION */
                break;
              case TELOPT_KERMIT:
#ifdef IKS_OPTION
                TELOPT_SB(x).kermit.me_start = 0;
#endif /* IKS_OPTION */
                break;
              default:
                break;
            } /* switch */
        } else {
          if (TELOPT_UNANSWERED_WILL(x))
              TELOPT_UNANSWERED_WILL(x) = 0;
          if (TELOPT_UNANSWERED_WONT(x))
              TELOPT_UNANSWERED_WONT(x) = 0;
        }
        if (TELOPT_ME_MODE(x) == TN_NG_MU) {
            sprintf(tn_msg,
           "Peer refuses TELNET WILL %s negotiations - terminating connection",
                    TELOPT(x)
                    );
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
            printf("%s\n",tn_msg);
            ttclos(0);
            whyclosed = WC_TELOPT;
            return(-3);
        }
        break;
      case SB:
        if ((y = tn_sb(x,&n,fn)) <= 0)
          return(y);

#ifdef CK_SSL
        /* Do not process subnegotiations other than START_TLS after we */
        /* have agreed to begin the TLS negotiation sequence.           */
        if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows &&
             x != TELOPT_START_TLS)
            break;
#endif /* CK_SSL */

        if (!TELOPT_OK(x)) {
            hexdump("unknown telnet subnegotiation",sb,n);
            break;
        } else if ( !(TELOPT_ME(x) || TELOPT_U(x)) ) {
            hexdump("telnet option not negotiated",sb,n);
            if (!tn_sb_bug)
                break;
            if (TELOPT_UNANSWERED_WILL(x)) {
                TELOPT_UNANSWERED_WILL(x) = 0;
                TELOPT_U(x) = 1;
                sprintf(tn_msg,
         "TELNET DO %s (implied by receipt of SB - protocol error ignored)",
                         TELOPT(x)
                         );
                debug(F100,tn_msg,"",0);
                if (tn_deb || debses) tn_debug(tn_msg);
            }
            if (TELOPT_UNANSWERED_DO(x)) {
                TELOPT_UNANSWERED_DO(x) = 0;
                TELOPT_ME(x) = 1;
                sprintf(tn_msg,
         "TELNET WILL %s (implied by receipt of SB - protocol error ignored)",
                         TELOPT(x)
                         );
                debug(F100,tn_msg,"",0);
                if (tn_deb || debses) tn_debug(tn_msg);
             }
        }

        TELOPT_UNANSWERED_SB(x)=0;
        switch (x) {
#ifdef CK_FORWARD_X
          case TELOPT_FORWARD_X:
            return(fwdx_tn_sb(sb, n));
#endif /* CK_FORWARD_X */
#ifdef CK_SSL
          case TELOPT_START_TLS: {
              /*
                 the other side is saying SB START_TLS FOLLOWS
                 the incoming channel is now ready for starting the
                 TLS negotiation.
                 */
              int def_tls_u_mode, def_tls_me_mode;
              int def_enc_u_mode, def_enc_me_mode;
              int rc = 0;

              TELOPT_SB(TELOPT_START_TLS).start_tls.u_follows = 1;
              /* Preserve the default modes and make sure we will */
              /* refuse START_TLS when we retry. */
              if (sstelnet) {
                  def_tls_u_mode = TELOPT_DEF_S_U_MODE(TELOPT_START_TLS);
                  def_tls_me_mode = TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS);
                  TELOPT_DEF_S_U_MODE(TELOPT_START_TLS) = TN_NG_RF;
                  TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS)= TN_NG_RF;
#ifdef CK_ENCRYPTION
                  def_enc_u_mode = TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION);
                  def_enc_me_mode = TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION);
                  TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                  TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION)= TN_NG_RF;
#endif /* CK_ENCRYPTION */
              } else {
                  def_tls_u_mode = TELOPT_DEF_C_U_MODE(TELOPT_START_TLS);
                  def_tls_me_mode = TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS);
                  TELOPT_DEF_C_U_MODE(TELOPT_START_TLS) = TN_NG_RF;
                  TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS)= TN_NG_RF;
#ifdef CK_ENCRYPTION
                  def_enc_u_mode = TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION);
                  def_enc_me_mode = TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION);
                  TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                  TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION)= TN_NG_RF;
#endif /* CK_ENCRYPTION */
              }
              /* Negotiate TLS */
              ttnproto = NP_TLS;
              tn_init = 0;
              if (ck_tn_tls_negotiate()<0) {
                  /* we failed.  disconnect and if we are the client */
                  /* then reconnect and try without START_TLS.       */
                  extern char * line;
                  int x = -1;
                  extern int mdmtyp;

                  if (sstelnet) {
                      printf("TLS failed:  Disconnecting.\n");
                      TELOPT_DEF_S_U_MODE(TELOPT_START_TLS)  = def_tls_u_mode;
                      TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS) = def_tls_me_mode;
#ifdef CK_ENCRYPTION
                     TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION)  = def_enc_u_mode;
                     TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION) = def_enc_me_mode;
#endif /* CK_ENCRYPTION */
                      ttclos(0);
                      whyclosed = WC_TELOPT;
                      ttnproto = NP_TELNET;
                      rc = -3;
                  } else {
                      extern tls_norestore;
                      printf("TLS failed:  Disconnecting...\n");
#ifdef CK_ENCRYPTION
                     TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION)  = def_enc_u_mode;
                     TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) = def_enc_me_mode;
#endif /* CK_ENCRYPTION */
                      /* if START_TLS is not REQUIRED, then retry without it */
                      if ( def_tls_me_mode != TN_NG_MU ) {
                          tls_norestore = 1;
                          ttclos(0);
                          whyclosed = WC_TELOPT;
                          tls_norestore = 0;
                          ttnproto = NP_TELNET;
                          printf("Reconnecting without TLS.\n");
                          sleep(2);
                          if (ttopen(line,&x,mdmtyp,0)<0)
                              rc = -3;
                      } else {
                          TELOPT_DEF_C_U_MODE(TELOPT_START_TLS) =
                            def_tls_u_mode;
                          TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS) =
                            def_tls_me_mode;
                          ttclos(0);
                          whyclosed = WC_TELOPT;
                          ttnproto = NP_TELNET;
                          rc = -3;
                      }
                  }
              } else {
                  /* we succeeded.  restart telnet negotiations from */
                  /* the beginning.  However, if we have received a  */
                  /* client certificate and we are a server, then do */
                  /* not offer TELOPT_AUTH */
                  if ( ck_tn_auth_valid() == AUTH_VALID ) {
                      TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = TN_NG_RF;
                      TELOPT_DEF_S_ME_MODE(TELOPT_AUTHENTICATION)= TN_NG_RF;
                  }
                  ttnproto = NP_TELNET;
                  if (tn_ini() < 0)
                    if (ttchk() < 0)
                      rc = -1;
              }
              /* Restore the default modes */
              if (sstelnet) {
                  TELOPT_DEF_S_U_MODE(TELOPT_START_TLS)  = def_tls_u_mode;
                  TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS) = def_tls_me_mode;
#ifdef CK_ENCRYPTION
                  TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION)  = def_enc_u_mode;
                  TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION) = def_enc_me_mode;
#endif /* CK_ENCRYPTION */
              } else {
                  TELOPT_DEF_C_U_MODE(TELOPT_START_TLS)  = def_tls_u_mode;
                  TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS) = def_tls_me_mode;
#ifdef CK_ENCRYPTION
                  TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION)  = def_enc_u_mode;
                  TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) = def_enc_me_mode;
#endif /* CK_ENCRYPTION */
              }
              return(rc);
          }
#endif /* CK_SSL */
#ifdef CK_AUTHENTICATION
          case TELOPT_AUTHENTICATION:
            if (ck_tn_sb_auth(sb,n) < 0) {
                if (sstelnet && TELOPT_U_MODE(x) == TN_NG_MU) {
                    ttclos(0);
                    whyclosed = WC_TELOPT;
                    return(-3);
                } else if (!sstelnet && TELOPT_ME_MODE(x) == TN_NG_MU) {
                    ttclos(0);
                    whyclosed = WC_TELOPT;
                    return(-3);
                } else {
                    if (TELOPT_ME_MODE(x) == TN_NG_RQ)
                      TELOPT_ME_MODE(x) = TN_NG_AC;
                    if (TELOPT_U_MODE(x) == TN_NG_RQ)
                      TELOPT_U_MODE(x) = TN_NG_AC;
                }
                if (TELOPT_ME(x)) {
                    TELOPT_ME(x) = 0;
                    if (tn_sopt(WONT,x) < 0)
                      return(-1);
                }
                if (TELOPT_U(x)) {
                    TELOPT_U(x) = 0;
                    if (tn_sopt(DONT,x) < 0)
                      return(-1);
                }
#ifdef CK_ENCRYPTION
                if (tn_no_encrypt()<0)
                    return(-1);
#endif /* CK_ENCRYPTION */
            } else {
#ifdef CK_ENCRYPTION
                if (!ck_tn_auth_in_progress()) { /* we are finished */
                    if (ck_tn_authenticated() == AUTHTYPE_SSL) {
                        /* TLS was successful.  Disable ENCRYPTION */
                        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    }
                    if (TELOPT_SB(TELOPT_ENCRYPTION).encrypt.need_to_send) {
                        ck_encrypt_send_support();
                        TELOPT_SB(TELOPT_ENCRYPTION).encrypt.need_to_send = 0;
                    }
                }
#endif /* CK_ENCRYPTION */
            }
            break;
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
          case TELOPT_ENCRYPTION:
            if (ck_tn_sb_encrypt(sb, n) < 0) {
                if (TELOPT_U_MODE(x) == TN_NG_MU ||
                    TELOPT_ME_MODE(x) == TN_NG_MU)
                  {
                      ttclos(0);
                      whyclosed = WC_TELOPT;
                      return(-3);
                } else {
                    if (TELOPT_ME_MODE(x) == TN_NG_RQ)
                      TELOPT_ME_MODE(x) = TN_NG_AC;
                    if (TELOPT_U_MODE(x) == TN_NG_RQ)
                      TELOPT_U_MODE(x) = TN_NG_AC;
                }
                if (TELOPT_ME(x)) {
                    TELOPT_ME(x) = 0;
                    if (tn_sopt(WONT,x) < 0)
                      return(-1);
                }
                if (TELOPT_U(x)) {
                    TELOPT_U(x) = 0;
                    if (tn_sopt(DONT,x) < 0)
                      return(-1);
                }
            }
            break;
#endif /* CK_ENCRYPTION */
#ifdef IKS_OPTION
          case TELOPT_KERMIT:
            return(iks_tn_sb((char *)sb, n));
#endif /* IKS_OPTION */
          case TELOPT_TTYPE:
            switch (sb[0]) {
              case TELQUAL_SEND:        /* SEND terminal type? */
                if ( !tn_outst(0) || tn_init ) {
                    if (tn_sttyp() < 0) /* Yes, so send it. */
                        return(-1);
                } else {
                    TELOPT_SB(TELOPT_TTYPE).term.need_to_send = 1;
                }
                break;
              case TELQUAL_IS: {        /* IS terminal type? */
                  /* IS terminal type -- remote gave us its current type */
                  int i = 0;
#ifndef OS2
                  CHAR oldterm[64], *p;
#endif /* OS2 */
                  /* Isolate the specified terminal type string */
                  while (sb[i++] != IAC) {
                      if (i == 40 ||    /* max len of term string - RFC */
                          sb[i] == IAC) {
                          sb[i] = '\0';
                          break;
                      }
                  }
#ifdef OS2
                  strupr(&(sb[1]));     /* Upper case it */
                  for (i = 0; i <= max_tt; i++) { /* find it in our list */
                      if (!strcmp(&(sb[1]),tt_info[i].x_name)
                          && i != TT_VTNT) /* can't support VTNT as server */
                        {
                          /* Set terminal type to the one chosen */
                          if (i != tt_type)
                            settermtype(i,0);
                          break;
                      }
                  }
                  if (i > max_tt &&
                      strcmp(&(sb[1]),TELOPT_SB(TELOPT_TTYPE).term.type)) {
                      /* Couldn't find the specified term type */
                      sb[40] = '\0';
                      strcpy(TELOPT_SB(TELOPT_TTYPE).term.type,&(sb[1]));
                      /* SB TTYPE SEND */
                      tn_ssbopt(TELOPT_TTYPE,TELQUAL_SEND,NULL,0);
                      TELOPT_UNANSWERED_SB(TELOPT_TTYPE)=1;
                  }
#else /* OS2 */
                  p = (CHAR *) getenv("TERM");
                  if (p)
                    ckstrncpy((char *)oldterm,(char *)p,63);
                  else
                    oldterm[0] = '\0';
                  cklower((char *)&(sb[1])); /* Lower case new term */
                  sprintf(term_buf,"TERM=%s",&(sb[1]));
#ifdef OS2ORUNIX
#ifndef NOPUTENV
                  putenv(term_buf);
#endif /* NOPUTENV */
#endif /* OS2ORUNIX */
#ifdef CK_CURSES
#ifndef MYCURSES
#ifndef COHERENT
                  if (trmbuf) {
                      if (tgetent(trmbuf,(char *)&sb[1]) < 1) {
                          /* Unsupported terminal.  If new and old terminal */
                          /* types do not match, ask for another type. */
                          if (strcmp((char *)oldterm,(char *)&sb[1])) {
                              /* SB TTYPE SEND */
                              tn_ssbopt(TELOPT_TTYPE,TELQUAL_SEND,NULL,0);
                              TELOPT_UNANSWERED_SB(TELOPT_TTYPE)=1;
                          }
                      }
                  }
#endif /* COHERENT */
#endif /* MYCURSES */
#endif /* CK_CURSES */
#endif /* OS2 */
              }
            }
            break;
#ifdef CK_ENVIRONMENT
#ifdef CK_XDISPLOC
          case TELOPT_XDISPLOC:         /* Send X-Display Location */
            if (sb[0] == TELQUAL_SEND) {/* SEND X-Display Loc? */
                if ( !tn_outst(0) || tn_init ) {
                    if (tn_sxdisploc() < 0)     /* Yes, so send it. */
                        return(-1);
                } else {
                    TELOPT_SB(TELOPT_XDISPLOC).xdisp.need_to_send = 1;
                }
            }
            /* IS -- X Display Location (not supported) */
            else if (sb[0] == TELQUAL_IS) {
                int i = 0;
                /* Isolate the specified X-display string */
                while (sb[i++] != IAC) {
                    if (i >= TSBUFSIZ)
                      return (-1);
                    if (sb[i] == IAC) {
                        sb[i] = '\0';
                        break;
                    }
                }
                debug(F110,"TELNET SB XDISPLOC IS",&sb[1],0);
            }
            break;
#endif /* CK_XDISPLOC */
#endif /* CK_ENVIRONMENT */
          case TELOPT_NAWS:
              if (sstelnet
#ifdef IKSD
                   || inserver
#endif /* IKSD */
                   ) {
                  int w = 0, h = 0;
                  int i = 0;
                  /* At this point sb[] should contain width and height */
                  if (sb[i] == IAC) i++;
                  w = (sb[i++] << 8);   /* save upper height */
                  if (sb[i] == IAC) i++;
                  w += sb[i++];         /* save the width */
                  if (sb[i] == IAC) i++;
                  h = (sb[i++] << 8);   /* save upper height */
                  if (sb[i] == IAC) i++;
                  h += sb[i++];
                  debug(F111,"tn_doop NAWS SB","width",w);
                  debug(F111,"tn_doop NAWS SB","height",h);

                  if (w == 0)
                    w = 80;
                  if (h == 0)
                    h = 24;
                  if (tcp_incoming) {
#ifdef OS2
                      tt_cols[VTERM] = w;
                      tt_rows[VTERM] = h;
                      VscrnSetWidth(VTERM, w);
                      VscrnSetHeight(VTERM, h+(tt_status?1:0));
#ifdef IKSD
                      if (inserver) {
                          cmd_cols = tt_cols[VCMD] = w;
                          cmd_rows = tt_rows[VCMD] = h;
                          VscrnSetWidth(VCMD, w);
                          VscrnSetHeight(VCMD, h);
                      }
#endif /* IKSD */
#else /* OS2 */
                      tt_cols = w;
                      tt_rows = h;
#endif /* OS2 */
                  } else {
#ifdef OS2
                      tt_cols[VCMD] = w;
                      tt_rows[VCMD] = h;
                      VscrnSetWidth(VCMD, w);
                      VscrnSetHeight(VCMD, h);
#endif /* OS2 */
                      cmd_cols = w;
                      cmd_rows = h;
                  }
                  /* Add LINES and COLUMNS to the environment */
                  sprintf((char *)rows_buf,"LINES=%d",h);
                  sprintf((char *)cols_buf,"COLUMNS=%d",w);
#ifdef OS2ORUNIX
#ifndef NOPUTENV
                  putenv(rows_buf);
                  putenv(cols_buf);
#endif /* NOPUTENV */
#endif /* OS2ORUNIX */
              }
              break;
#ifdef CK_ENVIRONMENT
          case TELOPT_NEWENVIRON:
            switch (sb[0]) {
              case TELQUAL_IS:                  /* IS */
              case TELQUAL_INFO:                /* INFO */
                if (sb[0] == TELQUAL_IS)
                  debug(F101,"tn_doop NEW-ENV SB IS","",n-3);
                else
                  debug(F101,"tn_doop NEW-ENV SB INFO","",n-3);
                if (sstelnet || inserver) { /* Yes, receive it. */
                    if (tn_rnenv((CHAR *)&sb[1],n-3) < 0)
                      return(-1);
                }
                break;
              case TELQUAL_SEND:        /* SEND */
                if ( sstelnet || inserver )         /* ignore if server */
                    break;
                /* We need to take the sb[] and build a structure */
                /* containing all of the variables and types that */
                /* we are supposed to keep track of and send to   */
                /* the host, then call tn_snenv().                  */
                /* Or we can punt ...                               */
                if ( !tn_outst(0) || tn_init ) {
                  if (tn_snenv((CHAR *)&sb[1],n-3) < 0) /* Yes, send it. */
                     return(-1);
                } else {
#ifndef VMS
                  CHAR * xxx;
                  xxx = (CHAR *) malloc(n-1);
#else
                  unsigned char * xxx;
                  xxx = (unsigned char *) malloc(n-1);
#endif /* VMS */
                  /* Postpone sending until end of tn_ini() */
                  TELOPT_SB(TELOPT_NEWENVIRON).env.str = xxx;
                  if (TELOPT_SB(TELOPT_NEWENVIRON).env.str) {
                  ckmemcpy((char *)TELOPT_SB(TELOPT_NEWENVIRON).env.str,
                            (char *)&sb[1],n-3);
                  TELOPT_SB(TELOPT_NEWENVIRON).env.str[n-3] = IAC;
                  TELOPT_SB(TELOPT_NEWENVIRON).env.str[n-2] = '\0';
                  TELOPT_SB(TELOPT_NEWENVIRON).env.len = n-3;
                  TELOPT_SB(TELOPT_NEWENVIRON).env.need_to_send = 1;
                  }
                }
                break;
              }
              break;
#endif /* CK_ENVIRONMENT */
#ifdef CK_SNDLOC
          case TELOPT_SNDLOC: {
              if ( deblog ) {
                  sb[n-2] = '\0';
                  debug(F110,"TELNET Send-Location",sb,0);
              }
              break;
          }
#endif /* CK_SNDLOC */
          } /* switch */
        break;
    }
    return(0);
}

#ifdef CK_ENVIRONMENT

/* Telnet receive new environment */
/* Returns -1 on error, 0 if nothing happens, 1 on success */
/* In order for this code to work, sb[len] == IAC          */
/* We currently only support the USER environment variable */

int
#ifdef CK_ANSIC
tn_rnenv(CHAR * sb, int len)
#else
tn_rnenv(sb, len) CHAR * sb; int len;
#endif /* CK_ANSIC */
/* tn_rnenv */ {                        /* Receive new environment */
    char varname[17];
    char value[65];
    char * reply = 0, * s = 0;
    int i,j,k,n;                                /* Worker. */
    int type = 0; /* 0 for NONE, 1 for VAR, 2 for USERVAR, */
                  /* 3 for VALUE in progress */

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);
    if (sb == NULL) return(-1);

    if (len == 0) return(1);

    /*
    Pairs of <type> [VAR=0, VALUE=1, ESC=2, USERVAR=3] <value> "unterminated"
    follow here until done...
    */
    for (i = 0, j = 0, k = 0, type = 0, varname[0]= '\0'; i <= len; i++) {
        switch (sb[i]) {
        case TEL_ENV_VAR:               /* VAR */
        case TEL_ENV_USERVAR:           /* USERVAR */
        case IAC:                       /* End of the list */
            switch (type) {
	      case 0:			/* Nothing in progress */
                /* If we get IAC only, then that means there were */
                /* no environment variables to send.  we are done */
                if (j == 0 && sb[i] == IAC)
                    return(1);
	      case 1:			/* VAR in progress */
	      case 2:			/* USERVAR in progress */
	      case 3:			/* VALUE in progress */
                value[k] = '\0';
                varname[j] = '\0';
                debug(F111,"tn_rnenv varname",varname,type);
                debug(F111,"tn_rnenv value",value,type);
                if (!strcmp(varname,"USER")) {
#ifdef CK_AUTHENTICATION
                    if (ck_tn_auth_valid() != AUTH_VALID) {
                        extern char szUserNameRequested[];
                        debug(F100,"tn_rnenv != AUTH_VALID","",0);
                        ckstrncpy(szUserNameRequested,value,UIDBUFLEN);
                        ckstrncpy(uidbuf,value,UIDBUFLEN);
#ifdef CK_SSL
                        if (ssl_active_flag) {
                            if ( tls_is_user_valid(ssl_con, uidbuf) ) {
                                extern char szUserNameAuthenticated[];
                                ckstrncpy(szUserNameAuthenticated,uidbuf,
                                           UIDBUFLEN);
                                auth_finished(AUTH_VALID);
                            }
                        } else if (tls_active_flag) {
                            if ( tls_is_user_valid(tls_con, uidbuf) ) {
                                extern char szUserNameAuthenticated[];
                                ckstrncpy(szUserNameAuthenticated,uidbuf,
                                           UIDBUFLEN);
                                auth_finished(AUTH_VALID);
                            }
                        }
#endif /* CK_SSL */
                    } else {    /* AUTH_VALID */
                        debug(F110,"tn_rnenv AUTH_VALID uidbuf",uidbuf,0);
                        if ( strcmp(value,uidbuf) ) {
                            extern char szUserNameRequested[];
                            ckstrncpy(uidbuf,value,UIDBUFLEN);
                            ckstrncpy(szUserNameRequested,value,UIDBUFLEN);
                            auth_finished(AUTH_USER);
#ifdef CK_SSL
                            if (ssl_active_flag || tls_active_flag) {
                                if ( tls_is_user_valid(ssl_con, uidbuf) )
                                    auth_finished(AUTH_VALID);
                            }
#endif /* CK_SSL */
                        }
                    }
#else /* CK_AUTHENTICATION */
                    ckstrncpy(uidbuf,value,UIDBUFLEN);
#endif /* CK_AUTHENTICATION */
                }
                break;
            }
            varname[0] = '\0';
            value[0] = '\0';
            j = 0;
            k = 0;
            type = (sb[i] == TEL_ENV_USERVAR ? 2 :      /* USERVAR */
                    sb[i] == TEL_ENV_VAR ? 1 :  /* VAR */
                     0
                     );
            break;
        case TEL_ENV_VALUE: /* VALUE */
            if ( type == 1 || type == 2 )
                type = 3;
            break;
        case TEL_ENV_ESC:       /* ESC */
            /* Take next character literally */
            if ( ++i >= len )
                break;
            /* otherwise, fallthrough so byte will be added to string. */
        default:
            switch (type) {
            case 1:     /* VAR in progress */
            case 2:     /* USERVAR in progress */
                if ( j < 16 )
                    varname[j++] = sb[i];
                break;
            case 3:
                if ( k < 64 )
                    value[k++] = sb[i];
                break;
            }
        }
    }
    return(0);
}

/* Telnet send new environment */
/* Returns -1 on error, 0 if nothing happens, 1 on success */
/* In order for this code to work, sb[len] == IAC          */

int
#ifdef CK_ANSIC
tn_snenv(CHAR * sb, int len)
#else
tn_snenv(sb, len) CHAR * sb; int len;
#endif /* CK_ANSIC */
/* tn_snenv */ {                        /* Send new environment */
    char varname[16];
    char * reply = 0, * s = 0;
    int i,j,n;                          /* Worker. */
    int type = 0;       /* 0 for NONE, 1 for VAR, 2 for USERVAR in progress */
    extern int ck_lcname;
    char localuidbuf[UIDBUFLEN];
    char * uu = uidbuf;
    char * disp = NULL, tmploc[256];

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);
    if (!sb) return(-1);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    /* Must compute the DISPLAY string we are going to send to the host */
    /* If one is not assigned, do not send a string unless the user has */
    /* explicitedly requested we try to send one via X-Display Location */

    if (tn_env_disp[0]) {
        int colon = ckindex(":",tn_env_disp,0,0,1);
        if ( !colon ) {
            sprintf(tmploc,"%s:%s",myipaddr,tn_env_disp);
            disp = tmploc;
        } else if ( ckindex("localhost:",tn_env_disp,0,0,0) ||
                    ckindex("127.0.0.1:",tn_env_disp,0,0,0) ||
                    ckindex("0:",tn_env_disp,0,0,0) ) {
            sprintf(tmploc,"%s:%s",myipaddr,&tn_env_disp[colon]);
            disp = tmploc;
        } else
            disp = tn_env_disp;
    }
    else if (TELOPT_ME_MODE(TELOPT_XDISPLOC)) {
        sprintf(tmploc,"%s:0.0",myipaddr);
        disp = tmploc;
    }


    if (ck_lcname) {
        ckstrncpy(localuidbuf,uidbuf,UIDBUFLEN);
        cklower(localuidbuf);
        uu = localuidbuf;
    }

    hexdump((CHAR *)"tn_snenv sb[]",sb,len);
    debug(F110,"tn_snenv uidbuf",uidbuf,0);
    debug(F110,"tn_snenv localuidbuf",localuidbuf,0);
    debug(F110,"tn_snenv tn_env_sys",tn_env_sys,0);
    debug(F110,"tn_snenv tn_env_disp",tn_env_disp,0);
    debug(F110,"tn_snenv disp",disp,0);

    /* First determine the size of the buffer we will need */
    for (i = 0, j = 0, n = 0, type = 0, varname[0]= '\0'; i <= len; i++) {
        switch (sb[i]) {
          case TEL_ENV_VAR:             /* VAR */
          case TEL_ENV_USERVAR:         /* USERVAR */
          case IAC:                     /* End of the list */
            switch (type) {
              case 0:                   /* Nothing in progress */
                /* If we get IAC only, then that means send all */
                /* VAR and USERVAR.  But since we don't support */
                /* USERVAR yet, we can just pass through        */
                if (!(j == 0 && sb[i] == IAC))
                  break;
              case 1:                   /* VAR in progress */
                varname[j] = '\0' ;
                if (!varname[0]) {      /* Send All */
                    if (uu[0])
                      n += strlen(uu) + 4 + 2;
                    if (tn_env_job[0])
                      n += strlen(tn_env_job) + 3 + 2;
                    if (tn_env_acct[0])
                      n += strlen(tn_env_acct) + 4 + 2;
                    if (tn_env_prnt[0])
                      n += strlen(tn_env_prnt) + 7 + 2;
                    if (tn_env_sys[0])
                      n += strlen(tn_env_sys) + 10 + 2;
                    if (tn_env_disp[0])
                      n += strlen(tn_env_disp) + 7 + 2;
                } else if (!strcmp(varname,"USER") && uu[0])
                  n += strlen(uu) + 4 + 2;
                else if (!strcmp(varname,"JOB") && tn_env_job[0])
                  n += strlen(tn_env_job) + 3 + 2;
                else if (!strcmp(varname,"ACCT") && tn_env_acct[0])
                  n += strlen(tn_env_acct) + 4 + 2;
                else if (!strcmp(varname,"PRINTER") && tn_env_prnt[0])
                  n += strlen(tn_env_prnt) + 7 + 2;
                else if (!strcmp(varname,"SYSTEMTYPE") && tn_env_sys[0])
                  n += strlen(tn_env_sys) + 10 + 2;
                else if (!strcmp(varname,"DISPLAY") && disp)
                  n += strlen(disp) + 7 + 2;
                break;
              case 2:                   /* USERVAR in progress */
                break;                  /* We don't support this yet */
            }
            varname[0] = '\0';
            j = 0;
            type = (sb[i] == TEL_ENV_USERVAR ? 2 :      /* USERVAR */
                    sb[i] == TEL_ENV_VAR ? 1 :          /* VAR */
                    0
                   );
            break;
          case TEL_ENV_VALUE:           /* VALUE */
            /* Protocol Error */
            debug(F100, "TELNET Subnegotiation error - VALUE in SEND", "",0);
            if (tn_deb || debses)
              tn_debug("TELNET Subnegotiation error - VALUE in SEND");
            return(0);
          case TEL_ENV_ESC:     /* ESC */
            if (++i >= len)
              break;
          default:
            if (j < 16 )
              varname[j++] = sb[i];
        }
    }
    reply = malloc(n + 16);              /* Leave room for IAC stuff */
    if (!reply) {
        debug(F100, "TELNET Subnegotiation error - malloc failed", "",0);
        if (tn_deb || debses)
          tn_debug("TELNET Subnegotiation error - malloc failed");

        /* Send a return packet with no variables so that the host */
        /* may continue with additional negotiations               */
        if (tn_ssbopt(TELOPT_NEWENVIRON,TELQUAL_IS,"",0) < 0)
          return(-1);
        return(0);
    }

    /* Now construct the real reply */

    n = 0;                              /* Start at beginning of buffer */
/*
  Pairs of <type> [VAR=0, VALUE=1, ESC=2, USERVAR=3] <value> "unterminated"
  follow here until done...
*/
    for (i = 0, j = 0, type = 0, varname[0]= '\0'; i <= len; i++) {
        switch (sb[i]) {
          case TEL_ENV_VAR:             /* VAR */
          case TEL_ENV_USERVAR:         /* USERVAR */
          case IAC:                     /* End of the list */
            switch (type) {
              case 0:                   /* Nothing in progress */
                /* If we get IAC only, then that means send all */
                /* VAR and USERVAR.  But since we don't support */
                /* USERVAR yet, we can just pass through        */
                if (!(j == 0 && sb[i] == IAC))
                  break;
              case 1:                   /* VAR in progress */
                varname[j] = '\0';
                if (!varname[0]) {
                    /* Send All */
                    if (uu[0]) {
                        reply[n] = TEL_ENV_VAR; /* VAR */
                        strcpy(&reply[n+1],"USER");
                        reply[n+5] = TEL_ENV_VALUE;             /* VALUE */
                        strcpy(&reply[n+6],uu);
                        n += strlen(uu) + 4 + 2;
                    }
                    if (tn_env_job[0]) {
                        reply[n] = TEL_ENV_VAR; /* VAR */
                        strcpy(&reply[n+1],"JOB");
                        reply[n+4] = TEL_ENV_VALUE;     /* VALUE */
                        strcpy(&reply[n+5],tn_env_job);
                        n += strlen(tn_env_job) + 3 + 2;
                    }
                    if (tn_env_acct[0]) {
                        reply[n] = TEL_ENV_VAR; /* VAR */
                        strcpy(&reply[n+1],"ACCT");
                        reply[n+5] = TEL_ENV_VALUE;     /* VALUE */
                        strcpy(&reply[n+6],tn_env_acct);
                        n += strlen(tn_env_acct) + 4 + 2;
                    }
                    if (tn_env_prnt[0]) {
                        reply[n] = TEL_ENV_VAR; /* VAR */
                        strcpy(&reply[n+1],"PRINTER");
                        reply[n+8] = TEL_ENV_VALUE;     /* VALUE */
                        strcpy(&reply[n+9],tn_env_prnt);
                        n += strlen(tn_env_prnt) + 7 + 2;
                    }
                    if (tn_env_sys[0]) {
                        reply[n] = TEL_ENV_VAR; /* VAR */
                        strcpy(&reply[n+1],"SYSTEMTYPE");
                        reply[n+11] = TEL_ENV_VALUE; /* VALUE */
                        strcpy(&reply[n+12],tn_env_sys);
                        n += strlen(tn_env_sys) + 10 + 2;
                    }
                    if (disp) {
                        reply[n] = TEL_ENV_VAR; /* VAR */
                        strcpy(&reply[n+1],"DISPLAY");
                        reply[n+8] = TEL_ENV_VALUE;     /* VALUE */
                        strcpy(&reply[n+9],disp);
                        n += strlen(disp) + 7 + 2;
                    }
                } else if (!strcmp(varname,"USER") && uu[0]) {
                    reply[n] = TEL_ENV_VAR;     /* VAR */
                    strcpy(&reply[n+1],"USER");
                    reply[n+5] = TEL_ENV_VALUE; /* VALUE */
                    strcpy(&reply[n+6],uu);
                    n += strlen(uu) + 4 + 2;
                } else if (!strcmp(varname,"JOB") && tn_env_job[0]) {
                    reply[n] = TEL_ENV_VAR;     /* VAR */
                    strcpy(&reply[n+1],"JOB");
                    reply[n+4] = TEL_ENV_VALUE; /* VALUE */
                    strcpy(&reply[n+5],tn_env_job);
                    n += strlen(tn_env_job) + 3 + 2;
                } else if (!strcmp(varname,"ACCT") && tn_env_acct[0]) {
                    reply[n] = TEL_ENV_VAR;     /* VAR */
                    strcpy(&reply[n+1],"ACCT");
                    reply[n+5] = TEL_ENV_VALUE; /* VALUE */
                    strcpy(&reply[n+6],tn_env_acct);
                    n += strlen(tn_env_acct) + 4 + 2;
                } else if (!strcmp(varname,"PRINTER") && tn_env_prnt[0]) {
                    reply[n] = TEL_ENV_VAR;     /* VAR */
                    strcpy(&reply[n+1],"PRINTER");
                    reply[n+8] = TEL_ENV_VALUE; /* VALUE */
                    strcpy(&reply[n+9],tn_env_prnt);
                    n += strlen(tn_env_prnt) + 7 + 2;
                } else if (!strcmp(varname,"SYSTEMTYPE") && tn_env_sys[0]) {
                    reply[n] = TEL_ENV_VAR;     /* VAR */
                    strcpy(&reply[n+1],"SYSTEMTYPE");
                    reply[n+11] = TEL_ENV_VALUE;        /* VALUE */
                    strcpy(&reply[n+12],tn_env_sys);
                    n += strlen(tn_env_sys) + 10 + 2;
                } else if (!strcmp(varname,"DISPLAY") && disp) {
                    reply[n] = TEL_ENV_VAR;     /* VAR */
                    strcpy(&reply[n+1],"DISPLAY");
                    reply[n+8] = TEL_ENV_VALUE; /* VALUE */
                    strcpy(&reply[n+9],disp);
                    n += strlen(disp) + 7 + 2;
                }
                break;
            case 2:     /* USERVAR in progress */
                /* we don't support this yet */
                break;
            }
            varname[0] = '\0';
            j = 0;
            type = (sb[i] == TEL_ENV_USERVAR ? 2 :      /* USERVAR */
                    sb[i] == TEL_ENV_VAR ? 1 :  /* VAR */
                    0
                   );
            break;
          case TEL_ENV_VALUE: /* VALUE */
            /* Protocol Error */
            debug(F100, "TELNET Subnegotiation error - VALUE in SEND", "",0);
            if (tn_deb || debses)
              tn_debug("TELNET Subnegotiation error - VALUE in SEND");
            return(0);  /* Was -1 but that would be taken as */
                        /* an I/O error, so absorb it and go on. */
          case TEL_ENV_ESC:     /* ESC */
            /* Not sure what this for.  Quote next character? */
            break;
          default:
            varname[j++] = sb[i];
        }
    }
    if (tn_ssbopt(TELOPT_NEWENVIRON,TELQUAL_IS,reply,n) < 0) {
        free(reply);
        return(-1);
    }
    free(reply);
    return(1);
}
#endif /* CK_ENVIRONMENT */

/* Telnet send terminal type */
/* Returns -1 on error, 0 if nothing happens, 1 if type sent successfully */

int
tn_sttyp() {                            /* Send telnet terminal type. */
    char *ttn;                          /* Name of terminal type. */
#ifdef OS2
    static int alias = -1;              /* which alias are we using ? */
#endif /* OS2 */
    int i;                              /* Worker. */
    int tntermflg = 0;
    int settype = 0;

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_TTYPE)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */
    ttn = NULL;

#ifdef OS2
    if (!tn_term) {
        if (ttnum == -1) {
            ttnum = tt_type;
            settype = 0;
            alias = -1;
        } else if (ttnumend) {
            ttnumend = 0;
            settype = 0;
        } else {
            if (tt_info[tt_type].x_aliases[++alias] == NULL)  {
                if (--tt_type < 0)
                  tt_type = max_tt;
                if (ttnum == tt_type)
                  ttnumend = 1;
                settype = 1;
                alias = -1;
            }
        }
        if (tt_type >= 0 && tt_type <= max_tt) {
            if (alias == -1)
              ttn = tt_info[tt_type].x_name;
            else
              ttn = tt_info[tt_type].x_aliases[alias];
        } else
          ttn = NULL;
    }
    else settype = 0;
#endif /* OS2 */

    if (tn_term) {                      /* Terminal type override? */
        debug(F110,"tn_sttyp",tn_term,0);
        if (*tn_term) {
            ttn = tn_term;
            tntermflg = 1;
        }
    } else debug(F100,"tn_sttyp no term override","",0);

#ifndef datageneral
    if (!ttn) {                         /* If no override, */
        ttn = getenv("TERM");           /* get it from the environment. */
    }
#endif /* datageneral */
    if ((ttn == ((char *)0)) || ((int)strlen(ttn) >= TSBUFSIZ))
      ttn = "UNKNOWN";
    sb[0] = (CHAR) IAC;                 /* I Am a Command */
    sb[1] = (CHAR) SB;                  /* Subnegotiation */
    sb[2] = TELOPT_TTYPE;               /* Terminal Type */
    sb[3] = (CHAR) 0;                   /* Is... */
    for (i = 4; *ttn; ttn++,i++) {      /* Copy and uppercase it */
#ifdef VMS
        if (!tntermflg && *ttn == '-' &&
            (!strcmp(ttn,"-80") || !strcmp(ttn,"-132")))
          break;
        else
#endif /* VMS */
        sb[i] = (char) ((!tntermflg && islower(*ttn)) ? toupper(*ttn) : *ttn);
    }
    ttn = (char *)sb;                   /* Point back to beginning */
    if (deblog || tn_deb || debses) {
        sb[i] = '\0';                   /* For debugging */
        sprintf(tn_msg,"TELNET SENT SB %s IS %s IAC SE",
                 TELOPT(TELOPT_TTYPE),sb+4);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
    sb[i++] = (CHAR) IAC;               /* End of Subnegotiation */
    sb[i++] = (CHAR) SE;                /* marked by IAC SE */
    if (ttol((CHAR *)sb,i) < 0)         /* Send it. */
      return(-1);
#ifdef OS2
    if (settype)
        settermtype(tt_type,0);
    else {
        ipadl25();
        VscrnIsDirty(VTERM);
    }
#endif /* OS2 */
    return(1);
}

#ifdef CK_ENVIRONMENT
#ifdef CK_XDISPLOC

/* Telnet send xdisplay location */
/* Returns -1 on error, 0 if nothing happens, 1 if type sent successfully */

int
tn_sxdisploc() {                        /* Send telnet X display location. */
    char * disp=NULL;
    char tmploc[256];
    int i;

    tmploc[0] = '\0';
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_XDISPLOC)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */
    if (tn_env_disp[0]) {
        int colon = ckindex(":",tn_env_disp,0,0,1);
        if ( !colon ) {
            sprintf(tmploc,"%s:%s",myipaddr,tn_env_disp);
            disp = tmploc;
        } else if ( ckindex("localhost:",tn_env_disp,0,0,0) ||
                    ckindex("127.0.0.1:",tn_env_disp,0,0,0) ||
                    ckindex("0:",tn_env_disp,0,0,0) ) {
            sprintf(tmploc,"%s:%s",myipaddr,&tn_env_disp[colon]);
            disp = tmploc;
        } else
            disp = tn_env_disp;
    } else {
        sprintf(tmploc,"%s:0.0",myipaddr);
        disp = tmploc;
    }
    debug(F110,"tn_sxdisploc",disp,0);

    sb[0] = (CHAR) IAC;                 /* I Am a Command */
    sb[1] = (CHAR) SB;                  /* Subnegotiation */
    sb[2] = TELOPT_XDISPLOC;            /* X-Display Location */
    sb[3] = (CHAR) 0;                   /* Is... */
    for (i = 4; *disp; disp++,i++) {      /* Copy and uppercase it */
        sb[i] = (char) *disp;
    }
    if (deblog || tn_deb || debses) {
        sb[i] = '\0';                   /* For debugging */
        sprintf(tn_msg,
                "TELNET SENT SB %s IS %s IAC SE",
                TELOPT(TELOPT_XDISPLOC),sb+4);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
    sb[i++] = (CHAR) IAC;               /* End of Subnegotiation */
    sb[i++] = (CHAR) SE;                /* marked by IAC SE */
    if (ttol((CHAR *)sb,i) < 0)         /* Send it. */
      return(-1);
    return(1);
}
#endif /* CK_XDISPLOC */
#endif /* CK_ENVIRONMENT */

#ifdef CK_FORWARD_X
int
tn_sndfwdx() {                          /* Send Fwd X Screen number to host */
    unsigned char screen = 0;

    if (!TELOPT_U(TELOPT_FORWARD_X)) return(0);
#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    /*
     * The format of the DISPLAY variable is [<host>:]<display>[.<screen>]
     * where <host> is an optional DNS name or ip address with a default of
     * the localhost; the screen defaults to 0
     */

    if (tn_env_disp[0]) {
        int colon,dot;
        colon = ckindex(":",tn_env_disp,0,0,1);
        dot   = ckindex(".",&tn_env_disp[colon],0,0,1);

        if ( dot ) {
            screen = atoi(&tn_env_disp[colon+dot]);
        }
    } else {
        return(-1);
    }

    sb[0] = (CHAR) IAC;                 /* I Am a Command */
    sb[1] = (CHAR) SB;                  /* Subnegotiation */
    sb[2] = TELOPT_FORWARD_X;           /* Forward X */
    sb[3] = FWDX_SCREEN;                /* Screen */
    sb[4] = screen;
    sb[5] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb[6] = (CHAR) SE;                  /* marked by IAC SE */
    if (ttol((CHAR *)sb,7) < 0)         /* Send it. */
      return(-1);
#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        sprintf(tn_msg,"TELNET SENT SB %s SCREEN %02x IAC SE",
        TELOPT(TELOPT_FORWARD_X),screen);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
#endif /* DEBUG */
    return(0);
}
#endif /* CK_FORWARD_X */

#ifdef CK_SNDLOC
int
tn_sndloc() {                           /* Send location. */
    int i;                              /* Worker. */
    char *ttloc;

    if (!TELOPT_ME(TELOPT_SNDLOC)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */
    ttloc = (tn_loc ? tn_loc : "");     /* In case we are being called even */
                                        /* though there is no location. */
    sb[0] = (CHAR) IAC;                 /* I Am a Command */
    sb[1] = (CHAR) SB;                  /* Subnegotiation */
    sb[2] = TELOPT_SNDLOC;              /* Location */
    for (i = 3; *ttloc && i < TSBUFSIZ; ttloc++,i++) /* Copy it */
      sb[i] = (char) *ttloc;
    sb[i++] = (CHAR) IAC;               /* End of Subnegotiation */
    sb[i++] = (CHAR) SE;                /* marked by IAC SE */
    if (ttol((CHAR *)sb,i) < 0)         /* Send it. */
      return(-1);
    sb[i-2] = '\0';                     /* For debugging */
#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        sprintf(tn_msg,"TELNET SENT SB %s %s IAC SE",
        TELOPT(TELOPT_SNDLOC),sb+3);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
#endif /* DEBUG */
    return(0);
}
#endif /* CK_SNDLOC */

#ifdef CK_NAWS                  /*  NAWS = Negotiate About Window Size  */
int
tn_snaws() {                    /*  Send terminal width and height, RFC 1073 */
    int i = 0;
#ifdef OS2
    int x = VscrnGetWidth(VTERM),
    y = VscrnGetHeight(VTERM) - (tt_status ? 1 : 0);
#else /* OS2 */
    int x = tt_cols, y = tt_rows;
#endif /* OS2 */

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);
    if (!TELOPT_ME(TELOPT_NAWS)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    if (x == TELOPT_SB(TELOPT_NAWS).naws.x && /* Only send if changed */
        y == TELOPT_SB(TELOPT_NAWS).naws.y
        )
      return(0);
    TELOPT_SB(TELOPT_NAWS).naws.x = x;  /* Remember the size     */
    TELOPT_SB(TELOPT_NAWS).naws.y = y;

    sb[i++] = (CHAR) IAC;               /* Send the subnegotiation */
    sb[i++] = (CHAR) SB;
    sb[i++] = TELOPT_NAWS;
    sb[i++] = (CHAR) (x >> 8) & 0xff;
    if ((CHAR) sb[i-1] == (CHAR) IAC)   /* IAC in data must be doubled */
      sb[i++] = (CHAR) IAC;
    sb[i++] = (CHAR) (x & 0xff);
    if ((CHAR) sb[i-1] == (CHAR) IAC)
      sb[i++] = (CHAR) IAC;
    sb[i++] = (CHAR) (y >> 8) & 0xff;
    if ((CHAR) sb[i-1] == (CHAR) IAC)
      sb[i++] = (CHAR) IAC;
    sb[i++] = (CHAR) (y & 0xff);
    if ((CHAR) sb[i-1] == (CHAR) IAC)
      sb[i++] = (CHAR) IAC;
    sb[i++] = (CHAR) IAC;
    sb[i++] = (CHAR) SE;
    if (deblog || tn_deb || debses) {
        sprintf(tn_msg,"TELNET SENT SB NAWS %d %d IAC SE",x,y);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
    if (ttol((CHAR *)sb,i) < 0)         /* Send it. */
      return(-1);
    return (0);
}
#endif /* CK_NAWS */
#endif /* TNCODE */
