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
extern int en_asg;              /* REMOTE ASSIGN ENABLED */
extern int en_que;              /* REMOTE QUERY ENABLED */

/* TRANSMIT command settings (see doshow() in ckuus5.c) */

extern int xmitx;               /* SET TRANSMIT ECHO value */
extern char xmitbuf[];          /* SET TRANSMIT EOF value */
extern int xmitf;               /* SET TRANSMIT FILL value */
extern int xmitl;               /* SET TRANSMIT LINEFEED value */
extern int xmits;               /* SET TRANSMIT LOCKING-SHIFT value */
extern int xmitw;               /* SET TRANSMIT PAUSE value */
extern int xmitp;               /* SET TRANSMIT PROMPT value */

/* Communication variables */

extern int local;               /* 1 = local mode, 0 = remote mode */
extern int cktapi;              /* 1 = TAPI, 0 = not TAPI */
extern char ttname[];           /* Name of communication device */
extern int mdmtyp;              /* SET MODEM TYPE value */
/*
   mdmtyp < 0 means network connection:
     network type = -mdmtyp
     network host name = ttname
   mdmtyp = 0 means direct serial connection.
   mdmtyp > 0 is modem type index.
*/
extern long speed;              /* SET SPEED value (serial only) */
extern int parity;              /* SET PARITY value */
extern int flow;                /* SET FLOW-CONTROL value */
extern int turnch;              /* SET HANDSHAKE value */
extern int turn;                /* SET HANDSHAKE flag */
extern int escape;              /* SET ESCAPE value */
extern int delay;               /* SET DELAY value */

/* Modem items */

extern long dialcapas;          /* SET MODEM CAPABILITIES */
extern int carrier;             /* SET MODEM CARRIER, SET CARRIER value */
extern int cdtimo;              /* SET (MODEM) CARRIER ON timeout value */
extern int dialdc;              /* SET MODEM COMPRESSION value */
extern int dialec;              /* SET MODEM ERROR-CORRECTION value */
extern int dialesc;             /* SET MODEM ESCAPE-CHARACTER value */
extern int dialfc;              /* SET MODEM FLOW-CONTROL value */
extern int dialmhu;             /* SET MODEM HANGUP-METHOD value */
extern int dialksp;             /* SET MODEM KERMIT-SPOOF value */
extern long dialmax;            /* SET MODEM MAXIMUM-SPEED value */
extern int mdmspd;              /* SET MODEM SPEED-MATCHING value */
extern int mdmtyp;              /* SET MODEM TYPE value */
/*
  User-set values for modem command strings, overriding built-in values
  If null, use built-in value from appropriate MDMINF struct in ckudia.c.
  See shomodem() in ckuus3.c for example of how to interpret.
*/
extern char * dialini;          /* SET MODEM COMMAND INIT-STRING */
extern char * dialcmd;          /* SET MODEM COMMAND DIAL-COMMAND */
extern char * dialdcon;         /* SET MODEM COMMAND DATA-COMPRESSION ON */
extern char * dialdcoff;        /* SET MODEM COMMAND DATA-COMPRESSION OFF */
extern char * dialecon;         /* SET MODEM COMMAND ERROR-CORRECTION ON */
extern char * dialecoff;        /* SET MODEM COMMAND ERROR-CORRECTION OFF */
extern char * dialaaon;         /* SET MODEM COMMAND AUTOANSWER ON */
extern char * dialaaoff;        /* SET MODEM COMMAND AUTOANSWER OFF */
extern char * dialhcmd;         /* SET MODEM COMMAND AUTOANSWER OFF */
extern char * dialhwfc;         /* SET MODEM COMMAND HARDWARE-FLOW */
extern char * dialswfc;         /* SET MODEM COMMAND SOFTWARE-FLOW */
extern char * dialnofc;         /* SET MODEM COMMAND NO-FLOW */
extern char * dialpulse;        /* SET MODEM COMMAND PULSE */
extern char * dialtone;         /* SET MODEM COMMAND TONE */

/* Dialing items */

extern int dialatmo;            /* ANSWER timeout */
extern char *dialnum;           /* DIAL number */
extern int dialsta;             /* Status of most recent dial operation */

extern char * diallac;          /* SET DIAL AREA-CODE value */
extern int dialcnf;             /* SET DIAL CONFIRMATION value */
extern int dialcon;             /* SET DIAL CONNECT value */
extern int dialcvt;             /* SET DIAL CONVERT-DIRECTORY value */
extern char * diallcc;          /* SET DIAL COUNTRY-CODE value */
extern int ndialdir;            /* SET DIAL DIRECTORY count */
extern char * dialdir[];        /* SET DIAL DIRECTORY list of filenames */
extern int dialdpy;             /* SET DIAL DISPLAY value */
extern int dialhng;             /* SET DIAL HANGUP value */
extern int dialint;             /* SET DIAL INTERVAL value */
extern char * dialixp;          /* SET DIAL INTL-PREFIX value */
extern char * dialixs;          /* SET DIAL INTL-SUFFIX value */
extern char * dialldp;          /* SET DIAL LD-PREFIX value */
extern char * diallds;          /* SET DIAL LD-SUFFIX value */
extern int dialmth;             /* SET DIAL METHOD value */
extern char * dialpxx;          /* SET DIAL PBX-EXCHANGE value */
extern char * dialpxi;          /* SET DIAL PBX-INSIDE-PREFIX value */
extern char * dialpxo;          /* SET DIAL PBX-OUTSIDE-PREFIX value */
extern char * dialnpr;          /* SET DIAL PREFIX value */
extern int dialrstr;            /* SET DIAL RESTRICT value */
extern int dialrtr;             /* SET DIAL RETRIES value */
extern int dialsrt;             /* SET DIAL SORT value */
extern char * dialsfx;          /* SET DIAL SUFFIX value */
extern int dialtmo;             /* SET DIAL TIMEOUT value */
extern int ntollfree;           /* SET DIAL TOLL-FREE-AREA-CODE count */
extern char * dialtfc[];        /* SET DIAL TOLL-FREE-AREA-CODE list */
extern char * dialtfp;          /* SET DIAL TOLL-FREE-PREFIX value */

/* Network items */

extern int nnetdir;             /* SET NETWORK DIRECTORY count */
extern char * netdir[];         /* SET NETWORK DIRECTORY list of file names */
extern char myhost[];           /* Local network host name */
extern int network;             /* Flag, nonzero for network connection */
extern int nettype;             /* Network type (see ckcnet.h) */
extern int ttnproto;            /* Virtual terminal protocol */
extern int tcp_avail;           /* TCP/IP loaded and available */
extern int dnet_avail;          /* DECnet loaded and available */
extern int tn_duplex;           /* SET TELNET ECHO value */
extern char * tn_term;          /* SET TELNET TERMINAL-TYPE value */
extern int tn_binary;           /* SET TELNET BINARY-MODE value */
extern int tn_nlm;              /* SET TELNET NEWLINE-MODE value */
extern int me_binary;           /* I'm in telnet binary mode */
extern int u_binary;            /* My partner is in telnet binary mode */

/* Terminal emulation, command window, etc.. */

extern struct tt_info_rec tt_info[];    /* Terminal-type-specific info */

extern unsigned char colornormal;       /* SET TERM COLOR TERM */
extern unsigned char colorselect;       /* SET TERM COLOR SELECTION */
extern unsigned char colorunderline;    /* SET TERM COLOR UNDERLINE */
extern unsigned char colorstatus;       /* SET TERM COLOR STATUS-LINE (N/A?) */
extern unsigned char colorhelp;         /* SET TERM COLOR HELP */
extern unsigned char colorborder;       /* SET TERM COLOR BORDER (N/A) */

extern int tt_rows;             /* Terminal rows = SET TERM HEIGHT */
extern int tt_cols;             /* Terminal columns = SET TERM WIDTH */
extern int cmd_rows;            /* Command screen rows = SET COMMAND HEIGHT */
extern int cmd_cols;            /* Command screen rows = SET COMMAND WIDTH */
extern int cmdmsk;              /* SET COMMAND BYTESIZE */
/*
  Bytesize 7 => cmdmsk 0x7f
  Bytesize 8 => cmdmsk 0xff
*/
extern int tt_answer;           /* SET TERM ANSWERBACK value */
extern int apcstatus;           /* SET TERM APC value */
extern int tt_arrow;            /* SET TERM ARROW value */
extern int autodl;              /* SET TERM AUTODOWNLOAD value */
extern int tt_bell;             /* SET TERM BELL value */
extern int cmask;               /* SET TERM BYTESIZE mask */
/*
  Bytesize 7 => cmask 0x7f
  Bytesize 8 => cmask 0xff
*/
extern int tcsr;                /* SET TERM CHARACTER-SET remote value */
extern int tcsl;                /* SET TERM CHARACTER-SET local value */
extern struct _vtG G[];         /* G0..G3 assignments */
extern struct _vtG * GL;        /* GL assignment */
extern struct _vtG * GR;        /* GR assignment */
extern struct _vtG * GNOW;
extern struct _vtG * SSGL;

extern int tt_codepage;         /* SET TERM CODE-PAGE value */
extern int tt_crd;              /* SET TERM CR-DISPLAY value */
extern int tt_cursor;           /* SET TERM CURSOR value */
extern int debses;              /* SET TERM DEBUG value */
extern int duplex;              /* SET DUPLEX / SET TERM ECHO value */
extern int tt_font;             /* SET TERM FONT (N/A?) */
extern int tt_keypad;           /* SET TERM KEYPAD value */
extern int sosi;                /* SET TERM LOCKING-SHIFT value */
extern int tt_mouse;            /* SET TERM MOUSE value */
extern int tnlm;                /* SET TERM NEWLINE-MODE value */
extern int tt_pacing;           /* SET TERM OUTPUT-PACING value */
extern int tt_roll;             /* SET TERM ROLL value */
extern int tt_updmode;          /* SET TERM SCREEN-UPDATE mode */
extern int tt_update;           /* SET TERM SCREEN-UPDATE time */
extern int tt_scrsize;          /* SET TERM SCROLLBACK value */
extern int tt_ctstmo;           /* SET TERM TRANSMIT-TIMEOUT value */
extern int tt_type;             /* SET TERM TYPE value */
extern int max_tt;              /* Number of terminal types */
extern int tt_type_mode;        /* Terminal type set by host command */
extern int tt_modechg;          /* SET TERM VIDEO-CHANGE value */
extern int tt_wrap;             /* SET TERM WRAP value */

/* Printing */

extern char * printfile;        /* SET PRINTER value */

/* Program control */

extern int success;             /* Status of last command */
extern int xitsta;              /* Program exit (return) value */
extern int priority;            /* SET PRIORITY value */
char * zhome();

/* Logs */

extern int deblog;              /* Flag for debug logging */
extern char debfil[];           /* Debug log filename */
extern int pktlog;              /* Flag for packet logging */
extern char pktil[];            /* Packet log filename */
extern int seslog;              /* Flat for session logging */
extern char sesfil[];           /* Session log filename */
extern int tralog;              /* Flag for transaction logging */
extern char trafil[];           /* Transaction log filename */

/* For statistics reporting -- For file transfer display (?) */

extern int timeouts;            /* Number of timeouts */
extern int spackets;            /* Number of packets sent */
extern int rpackets;            /* Number of packets received */
extern int retrans;             /* Number of retransmissions */
extern int crunched;            /* Number of damaged packets */
extern int wmax;                /* Biggest window size actually used */
extern int wcur;                /* Current window size */

extern long filcnt;             /* Number of files in transaction */
extern long filrej;             /* Number of files rejected in transaction */
extern long flci;               /* Characters from line, current file */
extern long flco;               /* Chars to line, current file  */
extern long tlci;               /* Chars from line in transaction */
extern long tlco;               /* Chars to line in transaction */
extern long ffc;                /* Chars to/from current file */
extern long tfc;                /* Chars to/from files in transaction */
extern long ccu;                /* Control chars unprefixed in transaction */
extern long ccp;                /* Control chars prefixed in transaction */
extern long rptn;               /* Repeated characters compressed */
extern long cps;                /* Current characters-per-second */

extern int tsecs;               /* Total seconds for protocol transaction */
extern int fsecs;               /* Seconds to transfer this file */

/* File-related variables -- For file transfer display (?) */

extern char filnam[];           /* Name of file being transferred */
extern int nfils;               /* Number of files in file group */
extern long fsize;              /* Size of current file */

/* Packet-related variables -- For file transfer display (?) */

extern int pktnum;              /* Current packet number */
extern int sndtyp;              /* Type of packet just sent (char) */
extern int rcvtyp;              /* Type of packet just recd (char) */
extern int rsn;                 /* packet sequence number 0-63 */
extern int spktl;               /* Length of packet being sent */
extern int rpktl;               /* Length of packet just received */
extern int pktpaus;             /* Interpacket pause interval, msec */
extern int rprintf;             /* REMOTE PRINT flag */
extern int rmailf;              /* MAIL flag */
extern int autopar;             /* 1 if parity was detected automatically */

/*
 * For EMACS . . .
 *
 * Local Variables:
 * Comment-column: 32
 * End:
 */
