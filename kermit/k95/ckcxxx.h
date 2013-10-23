/* Kermit 95 variable declarations for GUI             -*-C-*-               */

/* #include "ckcdeb.h" */

extern char *copyright[];       /* Copyright notice text */

/* File transfer protocol parameters */

extern int protocol;            /* File transfer protocol */
extern struct ck_p ptab[];      /* Settings for each protocol */
/*
  struct ck_p is defined in ckcker.h and has 10 members -- packet
  length, window size, control-char prefixing options, filename collision
  options, pathname stripping options, which are kept on a per-protocol
  basis.  These are, in general, duplicates of protocol-specific global
  variables.  The relevant SET commands set the values in both places,
  depending on what the current protocol is.  Whenever we switch protocols,
  we set the global variables to whatever is in this struct for that protocol.
*/

/* Packet retransmission limit */

extern int maxtry;              /* SET RETRY value */

/* Packet lengths */

extern int urpsiz;              /* SET RECEIVE PACKET-LENGTH value */
extern int rpsiz;               /* Biggest non-long packet to receive */
extern int maxrps;              /* (internal) Max incoming long packet size */

extern int spsizr;              /* SET SEND PACKET-LENGTH value */
extern int spsizf;              /* Flag that SET SEND PACKET given */
extern int maxsps;              /* (internal) Maximum outbound l.p. size */
extern int spmax;               /* Biggest packet we can send, negotiated */

/* Sliding Windows */

extern int wslots;              /* (internal) Window size currently in use */
extern int wslotr;              /* SET WINDOW value */
extern int wslotn;              /* Window size negotiated */

/* Packet framing */

extern CHAR mystch;             /* SET SEND START-OF-PACKET value */
extern CHAR stchr;              /* SET RECEIVE START-OF-PACKET value */
extern CHAR seol;               /* SET SEND END-OF-PACKET value */
extern CHAR eol;                /* SET RECEIVE END-OF-PACKET value */

/* Control-character prefixing */

extern CHAR myctlq;             /* SET SEND CONTROL-PREFIX value */
extern CHAR ctlq;               /* SET RECEIVE CONTROL-PREFIX value */
extern int prefixing;           /* SET PREFIXING value */
extern short ctlp[];            /* SET CONTROL PREFIX table */

/* Packet timeouts */

extern int timint;              /* Timeout interval I actually use */
extern int pkttim;              /* SET RECEIVE TIMEOUT value */
extern int rtimo;               /* SET SEND TIMEOUT value */
extern int timef;               /* Flag that SET SEND TIMEOUT given */

/* Padding */

extern int npad;                /* SET SEND PADDING value */
extern int mypadn;              /* SET RECEIVE PADDING value */
extern CHAR padch;              /* SET SEND PAD-CHARACTER value */
extern CHAR mypadc;             /* SET RECEIVE PAD-CHARACTER value */

/* Block check (1, 2, 3, B) */

extern int bctr;                /* SET BLOCK-CHECK value */
extern int bctu;                /* Block check type actually used */
extern int bctl;                /* Block check length */

/* Eighth-bit prefixing */

extern int ebq;                 /* 8th bit prefix */
extern int ebqflg;              /* 8th-bit quoting active flag */
extern int rqf;                 /* (internal) 8bq negotiation flag */
extern int rq;                  /* (internal) Received 8bq bid */
extern int sq;                  /* (internal) Sent 8bq bid */

/* Repeat-count compression */

extern int rpt;                 /* (internal) Current repeat count */
extern int rptq;                /* Repeat prefix character */
extern int rptflg;              /* Repeat processing active flag */
extern int rptena;              /* Repeat processing enabled */
extern int rptmin;              /* Repeat-count minimum */
extern CHAR myrptq;             /* Repeat prefix I want to use */

/* Remote-mode file-transfer cancellation -- doesn't apply to K95 (?) */

extern int xfrcan;              /* Transfer cancellation enabled */
extern int xfrchr;              /* Transfer cancel character value */
extern int xfrnum;              /* How many xfrchr needed to cancel */

/* Protocol capabilities -- K95 has all of them */
/* but their use must be negotiated with the other Kermit */

extern int lpcapr;              /* Long packets requested */
extern int lpcapu;              /* Long packets used */
extern int swcapr;              /* Sliding windows requested */
extern int swcapu;              /* Sliding windows used */
extern int atcapr;              /* Attribute packets requested */
extern int atcapu;              /* Attribute packets used */
extern int rscapr;              /* RESEND capability requested */
extern int rscapu;              /* RESEND capability used */
extern int lscapr;              /* Locking Shift requested */
extern int lscapu;              /* Locking Shift used */

/* Flags for whether to use particular attributes */

extern int atenci;              /* Encoding in */
extern int atenco;              /* Encoding out */
extern int atdati;              /* Date in */
extern int atdato;              /* Date out */
extern int atdisi;              /* Disposition in/out */
extern int atdiso;
extern int atleni;              /* Length in/out (both kinds) */
extern int atleno;
extern int atblki;              /* Blocksize in/out */
extern int atblko;
extern int attypi;              /* File type in/out */
extern int attypo;
extern int atsidi;              /* System ID in/out */
extern int atsido;
extern int atsysi;              /* Sys-dependent parameters in/out */
extern int atsyso;

/* File and/or file-transfer related variables */

extern int fcharset;            /* SET FILE CHARACTER-SET value */
extern int tcharset;            /* SET TRANSFER CHARACTER-SET value */
extern int language;            /* SET LANGUAGE value (for file transfer) */
extern int fncact;              /* SET FILE COLLISION value */
extern int fncnv;               /* SET FILE NAMES value */
extern int fnspath;             /* SET SEND PATHNAMES value */
extern int fnrpath;             /* SET RECEIVE PATHNAMES value */
extern int binary;              /* SET FILE TYPE value */
extern int fmask;               /* SET FILE BYTESIZE mask */
/*
  Bytesize 7 => fmask 0x7f
  Bytesize 8 => fmask 0xff
*/
extern char * dldir;            /* SET FILE DOWNLOAD-DIRECTORY value */
extern int keep;                /* SET FILE INCOMPLETE value */
extern int unkcs;               /* SET FILE UNKNOWN-CHAR value */

/* Protocol state info */

extern CHAR sstate;             /* Starting state for automaton */
extern int sendmode;            /* Type of SENDing (ckcker.h) */
extern int moving;              /* MOVE = send, then delete */
extern long sendstart;          /* PSEND/RESEND start position */
extern int server;              /* Nonzero = in server mode */
extern int xflg;                /* Receiving screen data from server */
extern int hcflg;               /* Server is doing REMOTE HOST cmd */
extern int cxseen;              /* Flag for cancelling a file */
extern int czseen;              /* Flag for cancelling file group */

/* Client/Server items *

extern int srvdis;              /* SET SERVER DISPLAY value */
extern int srvtim;              /* SET SERVER TIMEOUT value */

extern int en_cwd;              /* REMOTE CD/CWD ENABLED */
extern int en_cpy;              /* REMOTE COPY ENABLED */
extern int en_del;              /* REMOTE DELETE ENABLED */
extern int en_dir;              /* REMOTE DIRECTORY ENABLED */
extern int en_fin;              /* FINISH/BYE ENABLED */
extern int en_get;              /* GET ENABLED */
extern int en_hos;              /* REMOTE HOST disabled ENABLED */
extern int en_ren;              /* REMOTE RENAME ENABLED */
extern int en_sen;              /* SEND ENABLED */
extern int en_set;              /* REMOTE SET ENABLED */
extern int en_spa;              /* REMOTE SPACE ENABLED */
extern int en_typ;              /* REMOTE TYPE ENABLED */
extern int en_who;              /* REMOTE WHO ENABLED */
extern int en_bye;              /* BYE ENABLED */
extern
