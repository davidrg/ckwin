/* ckcker.h -- Symbol and macro definitions for C-Kermit */

/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 1996, Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.
*/

#ifndef CKCKER_H
#define CKCKER_H

/*
  If NEWDEFAULTS is defined then:
   - RECEIVE PACKET-LENGTH is 2048 rather than 90
   - WINDOW is 20 rather than 1
   - BLOCK-CHECK is 3 rather than 1
   - FILE TYPE is BINARY rather than TEXT
*/
#ifdef OS2				/* OS/2, Windows NT, Windows 95 */
#ifndef NEWDEFAULTS
#define NEWDEFAULTS
#endif /* NEWDEFAULTS */
#endif /* OS2 */

#ifdef NOICP				/* No Interactive Command Parser */
#ifndef NOSPL				/* implies... */
#define NOSPL				/* No Script Programming Language */
#endif /* NOSPL */
#ifndef NOCSETS				/* No character-set translation */
#define NOCSETS				/* because the only way to set it up */
#endif /* NOCSETS */			/* is with interactive commands */
#endif /* NOICP */

#ifdef pdp11				/* There is a maximum number of */
#ifndef NOCKSPEED			/* of -D's allowed on the CC */
#define NOCKSPEED			/* command line, so some of them */
#endif /* NOCKSPEED */			/* have to go here... */
#ifndef NOREDIRECT
#define NOREDIRECT
#endif /* NOREDIRECT */
#ifdef WHATAMI
#undef WHATAMI
#endif /* WHATAMI */
#endif /* pdp11 */

#define LOGINLEN 32			/* Length of server login field */

/* Control-character (un)prefixing options */

#define PX_ALL  0			/* Prefix all control chars */
#define PX_CAU  1			/* Unprefix cautiously */
#define PX_WIL  2			/* Unprefix with wild abandon */
#define PX_NON  3			/* Unprefix all (= prefix none) */

/* Destination codes */

#define  DEST_D 0	/*  DISK */
#define  DEST_S 1	/*  SCREEN */
#define  DEST_P 2	/*  PRINTER */

/* File transfer protocols */

#define  PROTO_K    0	/*   Kermit   */
#ifdef CK_XYZ
#define  PROTO_X    1	/*   XMODEM   */
#define  PROTO_Y    2	/*   YMODEM   */
#define  PROTO_G    3	/*   YMODEM-g */
#define  PROTO_Z    4	/*   ZMODEM   */
#define  PROTO_O    5   /*   OTHER    */
#define  NPROTOS    6   /*   How many */
#else
#define  NPROTOS    1   /*   How many */
#endif /* CK_XYZ */

struct ck_p {				/* C-Kermit Protocol info structure */
    char * p_name;			/* Protocol name */
    int rpktlen;			/* Packet length - receive */
    int spktlen;			/* Packet length - send */
    int spktflg;			/* ... */
    int winsize;			/* Window size */
    int prefix;				/* Control-char prefixing options */
    int fnca;				/* Filename collision action */
    int fncn;				/* Filename conversion */
    int fnsp;				/* Send filename path stripping */
    int fnrp;				/* Receive filename path stripping */
    char * h_b_init;		/* Host receive initiation string - text   */
    char * h_t_init;		/* Host receive initiation string - binary */
    char * p_b_scmd;		/* SEND cmd for external protocol - text   */
    char * p_t_scmd;		/* SEND cmd for external protocol - binary */
    char * p_b_rcmd;		/* RECV cmd for external protocol - text   */
    char * p_t_rcmd;		/* RECV cmd for external protocol - binary */
};

struct filelist {			/* Send-file list element */
    char * fl_name;			/* Filename */
    int fl_mode;			/* Transfer mode */
    char * fl_alias;			/* Name to send the file under */
    struct filelist * fl_next;		/* Pointer to next element */
};

/* Kermit system IDs and associated properties... */

struct sysdata {
    char *sid_code;	/* Kermit system ID code */
    char *sid_name;	/* Descriptive name */
    short sid_unixlike;	/* Tree-structured directory with separators */
    char  sid_dirsep;	/* Directory separator character if unixlike */
    short sid_dev;	/* Can start with dev: */
    short sid_case;	/* Bit mapped: 1 = case matters, 2 = case preserved */
    short sid_recfm;    /* Text record separator */
/*
   0 = unknown or nonstream
   1 = cr
   2 = lf
   3 = crlf
*/
};

#ifndef NOSPL
/*
  The IF REMOTE-ONLY command is available only in versions
  that actually can be used in remote mode, and only if we have
  an interactive command parser.
*/
#define CK_IFRO
#ifdef MAC
#undef CK_IFRO
#else
#ifdef GEMDOS
#undef CK_IFRO
#endif /* GEMDOS */
#endif /* MAC */
#endif /* NOSPL */

/* Systems whose CONNECT modules can execute Application Program Commands */

#ifndef NOAPC				/* Unless they said NO APC */
#ifndef CK_APC				/* And they didn't already define it */
#ifdef OS2				/* OS/2 gets it */
#define CK_APC
#endif /* OS2 */
#ifdef UNIX				/* UNIX gets it */
#define CK_APC
#endif /* UNIX */
#ifdef VMS				/* VMS too */
#define CK_APC
#endif /* VMS */
#endif /* CK_APC */
#endif /* NOAPC */

#ifdef NOSPL				/* Script programming language */
#ifdef CK_APC				/* is required for APC. */
#undef CK_APC
#endif /* CK_APC */
#endif /* NOSPL */

#ifdef CK_APC				/* APC buffer length */
#define APCBUFLEN (CMDBL + 10)
#define APC_OFF 0	/* APC OFF (disabled) */
#define APC_ON 1	/* APC ON (enabled for non-dangerous commands) */
#define APC_UNCH 2	/* APC UNCHECKED (enabled for ALL commands) */
#define APC_INACTIVE 0	/* APC not in use */
#define APC_REMOTE   1	/* APC in use from Remote */
#define APC_LOCAL    2	/* APC being used from with Kermit */
#ifndef CK_AUTODL	/* Autodownload */
#ifdef OS2
#define CK_AUTODL
#else
#ifdef UNIX
#define CK_AUTODL
#else
#ifdef VMS
#define CK_AUTODL
#endif /* VMS */
#endif /* UNIX */
#endif /* OS2 */
#endif /* CK_AUTODL */
#endif /* CK_APC */

/* Codes for what we are doing now */

#define W_NOTHING  0			/* Nothing */
#define W_SEND     2			/* SENDing or MAILing */
#define W_RECV     4			/* RECEIVEing or GETting */
#define W_REMO     8			/* Doing a REMOTE command */
#define W_CONNECT 16			/* CONNECT mode */
#define W_COMMAND 32			/* Command mode */

#ifndef NOWHATAMI
#ifndef WHATAMI
#define WHATAMI
#endif /* WHATAMI */
#endif /* NOWHATAMI */

#ifdef WHATAMI				/* Bit mask positions for WHATAMI */
#define WM_SERVE  1			/* Server mode */
#define WM_FMODE  2			/* File transfer mode */
#define WM_FNAME  4			/* File name conversion */
#define WM_FLAG3  8			/* Flag for Bit3 */
#define WM_FLAG  32			/* Flag that says we got this info */
#endif /* WHATAMI */

/* Terminal types */
#define VT100     0			/* Also for VT52 mode */
#define TEKTRONIX 1

/* Normal packet and window size */

#define MAXPACK 94			/* Maximum unextended packet size */
					/* Can't be more than 94. */
#ifdef pdp11				/* Maximum sliding window slots */
#define MAXWS  8
#else
#define MAXWS 32			/* Can't be more than 32. */
#endif /* pdp11 */

/* Maximum long packet size for sending packets */
/* Override these from cc command line via -DMAXSP=nnn */

#ifdef DYNAMIC
#ifndef MAXSP
#define MAXSP 9024
#endif /* MAXSP */
#else  /* not DYNAMIC */
#ifndef MAXSP
#ifdef pdp11
#define MAXSP 1024
#else
#define MAXSP 2048
#endif /* pdp11 */
#endif /* MAXSP */
#endif /* DYNAMIC */

/* Maximum long packet size for receiving packets */
/* Override these from cc command line via -DMAXRP=nnn */

#ifdef DYNAMIC
#ifndef MAXRP 
#define MAXRP 9024
#endif /* MAXRP */
#else  /* not DYNAMIC */
#ifndef MAXRP 
#ifdef pdp11
#define MAXRP 1024
#else
#define MAXRP 2048
#endif /* pdp11 */
#endif /* MAXRP */
#endif /* DYNAMIC */
/*
  Default sizes for windowed packet buffers.
  Override these from cc command line via -DSBSIZ=nnn, -DRBSIZ=nnn.
  Or just -DBIGBUFOK.
*/

#ifndef MAXGETPATH			/* Maximum number of directories */
#ifdef BIGBUFOK				/* for GET path... */
#define MAXGETPATH 128
#else
#define MAXGETPATH 16
#endif /* BIGBUFOK */
#endif /* MAXGETPATH */

#ifndef NOSPL				/* Query buffer length */
#ifdef OS2
#define QBUFL 4095
#else
#define QBUFL 1023
#endif /* OS2 */
#endif /* NOSPL */

#ifdef DYNAMIC
#ifndef SBSIZ 
#ifdef BIGBUFOK				/* If big buffers are safe... */
#define SBSIZ 90500			/* Allow for 10 x 9024 or 20 x 4096 */
#else					/* Otherwise... */
#ifdef pdp11
#define SBSIZ 3020
#else
#define SBSIZ 9050			/* Allow for 3 x 3000, etc. */
#endif /* pdp11 */
#endif /* BIGBUFOK */
#endif /* SBSIZ */

#ifndef RBSIZ 
#ifdef BIGBUFOK
#define RBSIZ 90500
#else
#ifdef pdp11
#define RBSIZ 3020
#else
#define RBSIZ 9050
#endif /* pdp11 */
#endif /* BIGBUFOK */
#endif /* RBSIZ */
#else  /* not DYNAMIC */
#ifdef pdp11
#define SBSIZ 3020
#define RBSIZ 3020
#else
#ifndef SBSIZ
#define SBSIZ (MAXSP * (MAXWS + 1))
#endif /* SBSIZ */
#ifndef RBSIZ
#define RBSIZ (MAXRP * (MAXWS + 1))
#endif /* RBSIZ */
#endif /* pdp11 */
#endif /* DYNAMIC */

/* Kermit parameters and defaults */

#define CTLQ	   '#'			/* Control char prefix I will use */
#define MYEBQ	   '&'			/* 8th-Bit prefix char I will use */
#define MYRPTQ	   '~'			/* Repeat count prefix I will use */

#define MAXTRY	    10			/* Times to retry a packet */
#define MYPADN	    0			/* How many padding chars I need */
#define MYPADC	    '\0'		/* Which padding character I need */

#define DMYTIM	    8			/* Initial timeout interval to use. */
#define URTIME	    15			/* Timeout interval to use on me. */
#define DSRVTIM     0			/* Default server cmd wait timeout. */

#define DEFTRN	    0			/* Default line turnaround handshake */

#define MYEOL	    CR			/* Incoming packet terminator. */

#ifdef NEWDEFAULTS
#define DRPSIZ	  4095			/* Default incoming packet size. */
#define DFWSIZ      20			/* Default window size */
#define DFBCT        3			/* Default block-check type */
#else
#define DRPSIZ	    90			/* Default incoming packet size. */
#define DFWSIZ       1			/* Default window size */
#define DFBCT        3			/* Default block-check type */
#endif /* NEWDEFAULTS */

#define DSPSIZ	    90			/* Default outbound packet size. */

#define DDELAY      1			/* Default delay. */
#define DSPEED	    9600		/* Default line speed. */

#ifdef OS2				/* Default CONNECT-mode */
#define DFESC 29			/* escape character */
#else
#ifdef NEXT				/* Ctrl-] for PC and NeXT */
#define DFESC 29
#else
#ifdef GEMDOS				/* And Atari ST */
#define DFESC 29
#else
#define DFESC 28			/* Ctrl-backslash for others */
#endif /* GEMDOS */
#endif /* NEXT */
#endif /* OS2 */

#ifdef NOPUSH				/* NOPUSH implies NOJC */
#ifndef NOJC				/* (no job control) */
#define NOJC
#endif /* NOJC */
#endif /* NOPUSH */

#ifdef UNIX				/* Default for SET SUSPEND */
#ifdef NOJC				/* UNIX but job control disabled */
#define DFSUSP      0
#else					/* UNIX, job control enabled. */
#define DFSUSP      1
#endif /* NOJC */
#else
#define DFSUSP      0
#endif /* UNIX */

/* Files */

#define ZCTERM      0	    	/* Console terminal */
#define ZSTDIO      1		/* Standard input/output */
#define ZIFILE	    2		/* Current input file (SEND, etc) (in) */
#define ZOFILE      3	    	/* Current output file (RECEIVE, GET) (out) */
#define ZDFILE      4	    	/* Current debugging log file (out) */
#define ZTFILE      5	    	/* Current transaction log file (out) */
#define ZPFILE      6	    	/* Current packet log file (out) */
#define ZSFILE      7		/* Current session log file (out) */
#define ZSYSFN	    8		/* Input/Output from a system function */
#define ZRFILE      9           /* Local file for READ (in) */
#define ZWFILE     10           /* Local file for WRITE (out) */
#define ZMFILE     11		/* Miscellaneous output file, e.g. for XLATE */
#define ZNFILS     12	    	/* How many defined file numbers */

/*  Buffered file i/o ...  */
#ifdef pdp11
#define INBUFSIZE 512
#define OBUFSIZE 512
#else
#ifdef VMS		/* In VMS, allow for longest possible RMS record */
#define INBUFSIZE 32768			/* File input buffer size */
#define OBUFSIZE 32768			/* File output buffer size */
#else  /* Not VMS */
#ifdef BIGBUFOK				/* Systems where memory is */
#define INBUFSIZE 32768			/* not a problem... */
#define OBUFSIZE 32768
#else /* Not BIGBUFOK */
#ifdef STRATUS
#ifdef DYNAMIC
#define INBUFSIZE 32767			/* File input buffer size */
#define OBUFSIZE 32767			/* File output buffer size */
#else /* STRATUS, not DYNAMIC */
#define INBUFSIZE 4096			/* File input buffer size */
#define OBUFSIZE 4096			/* File output buffer size */
#endif /* DYNAMIC */
#else /* not STRATUS */
#ifdef OS2				/* OS/2 and friends */
#ifdef NT
#define INBUFSIZE 32768			/* Windows 95 and NT */
#define OBUFSIZE 32768
#else
#define INBUFSIZE 4095			/* OS/2 HPFS block allocation */
#define OBUFSIZE 4095
#endif /* NT */
#else
#define INBUFSIZE 1024
#define OBUFSIZE 1024
#endif /* OS2 */
#endif /* STRATUS */
#endif /* BIGBUFOK */
#endif /* VMS */
#endif /* pdp11 */

/* get the next char; sorta like a getc() macro */
#define zminchar() (((--zincnt)>=0) ? ((int)(*zinptr++) & 0377) : zinfill())

/* stuff a character into the input buffer */
#define zmstuff(c) zinptr--, *zinptr = c, zincnt++

/* put a character to a file, like putchar() macro */
#define zmchout(c) \
((*zoutptr++=(char)(c)),(((++zoutcnt)>=OBUFSIZE)?zoutdump():0))

/* Screen functions */

#define SCR_FN 1    	/* filename */
#define SCR_AN 2    	/* as-name */
#define SCR_FS 3 	/* file-size */
#define SCR_XD 4    	/* x-packet data */
#define SCR_ST 5      	/* File status: */
#define   ST_OK   0   	/*  Transferred OK */
#define   ST_DISC 1 	/*  Discarded */
#define   ST_INT  2     /*  Interrupted */
#define   ST_SKIP 3 	/*  Skipped */
#define   ST_ERR  4 	/*  Fatal Error */
#define   ST_REFU 5     /*  Refused (use Attribute codes for reason) */
#define   ST_INC  6	/*  Incompletely received */
#define   ST_MSG  7	/*  Informational message */
#define SCR_PN 6    	/* packet number */
#define SCR_PT 7    	/* packet type or pseudotype */
#define SCR_TC 8    	/* transaction complete */
#define SCR_EM 9    	/* error message */
#define SCR_WM 10   	/* warning message */
#define SCR_TU 11	/* arbitrary undelimited text */
#define SCR_TN 12   	/* arbitrary new text, delimited at beginning */
#define SCR_TZ 13   	/* arbitrary text, delimited at end */
#define SCR_QE 14	/* quantity equals (e.g. "foo: 7") */
#define SCR_CW 15	/* close screen window */
#define SCR_CD 16       /* display current directory */

/* Macros */

#define tochar(ch)  (((ch) + SP ) & 0xFF )	/* Number to character */
#define xunchar(ch) (((ch) - SP ) & 0xFF )	/* Character to number */
#define ctl(ch)     (((ch) ^ 64 ) & 0xFF )	/* Controllify/Uncontrollify */
#define unpar(ch)   (((ch) & 127) & 0xFF )	/* Clear parity bit */

#ifndef NODIAL

/* Modem capabilities (bit values) */
#define CKD_AT   1			/* Hayes AT commands and responses */
#define CKD_V25  2			/* V.25bis commands and responses */
#define CKD_SB   4			/* Speed buffering */
#define CKD_EC   8			/* Error correction */
#define CKD_DC  16			/* Data compression */
#define CKD_HW  32			/* Hardware flow control */
#define CKD_SW  64			/* (Local) software flow control */
#define CKD_KS 128			/* Kermit spoofing */
#define CKD_TB 256			/* Made by Telebit */

/* DIAL command result codes */
#define DIA_UNK   -1			/* No DIAL command given yet */
#define DIA_OK     0			/* DIAL succeeded */
#define DIA_NOMO   1			/* Modem type not specified */
#define DIA_NOLI   2			/* Communication line not spec'd */
#define DIA_OPEN   3			/* Line can't be opened */
#define DIA_NOSP   4			/* Speed not specified */
#define DIA_HANG   5			/* Hangup failure */
#define DIA_IE     6			/* Internal error (malloc, etc) */
#define DIA_IO     7			/* I/O error */
#define DIA_TIMO   8			/* Dial timeout expired */
#define DIA_INTR   9			/* Dialing interrupted by user */
#define DIA_NRDY  10			/* Modem not ready */
#define DIA_PART  11			/* Partial dial command OK */
#define DIA_DIR   12			/* Dialing directory error */
#define DIA_HUP   13			/* Modem was hung up OK */
#define DIA_ERR   20			/* Modem command error */
#define DIA_NOIN  21			/* Failure to initialize modem */
#define DIA_BUSY  22			/* Phone busy */
#define DIA_NOCA  23			/* No carrier */
#define DIA_NODT  24			/* No dialtone */
#define DIA_RING  25			/* Ring, incoming call */
#define DIA_NOAN  26			/* No answer */
#define DIA_DISC  27			/* Disconnected */
#define DIA_VOIC  28			/* Answered by voice */
#define DIA_NOAC  29			/* Access denied, forbidden call */
#define DIA_BLCK  30			/* Blacklisted */
#define DIA_DELA  31			/* Delayed */
#define DIA_FAX   32			/* Fax */
#define DIA_UERR  98			/* Unknown error */
#define DIA_UNSP  99		/* Unspecified failure detected by modem */

#define MDMINF	struct mdminf

MDMINF {			/* Structure for modem-specific information */
    
    char * name;		/* Descriptive name */
    char * pulse;		/* Command to force pulse dialing */
    char * tone;		/* Command to force tone dialing */
    int    dial_time;		/* Time modem allows for dialing (secs) */
    char * pause_chars;		/* Character(s) to tell modem to pause */
    int	   pause_time;		/* Time associated with pause chars (secs) */
    char * wake_str;		/* String to wakeup modem & put in cmd mode */
    int	   wake_rate;		/* Delay between wake_str characters (msecs) */
    char * wake_prompt;		/* String prompt after wake_str */
    char * dmode_str;		/* String to put modem in dialing mode */
    char * dmode_prompt;	/* String prompt for dialing mode */
    char * dial_str;		/* Dialing string, with "%s" for number */
    int    dial_rate;		/* Interchar delay to modem (msec) */
    int    esc_time;		/* Escape sequence guard time (msec) */
    int    esc_char;		/* Escape character */
    char * hup_str;		/* Hangup string */
    char * hwfc_str;		/* Hardware flow control string */
    char * swfc_str;		/* Software flow control string */
    char * nofc_str;		/* No flow control string */
    char * ec_on_str;		/* Error correction on string */
    char * ec_off_str;		/* Error correction off string */
    char * dc_on_str;		/* Data compression on string */
    char * dc_off_str;		/* Data compression off string */
    char * aa_on_str;		/* Autoanswer on string */
    char * aa_off_str;		/* Autoanswer off string */
    char * sb_on_str;		/* Speed buffering on string */
    char * sb_off_str;		/* Speed buffering off string */
    long   max_speed;		/* Maximum interface speed */
    long   capas;		/* Capability bits */
    /* function to read modem's response string to a non-dialing command */
    _PROTOTYP( int (*ok_fn), (int,int) );
};
#endif /* NODIAL */

/* Symbols for File Attributes */

#define AT_XALL  0			/* All of them */
#define AT_ALLY  1			/* All of them on (Yes) */
#define AT_ALLN  2			/* All of them off (no) */
#define AT_LENK  3			/* Length in K */
#define AT_FTYP  4			/* File Type */
#define AT_DATE  5			/* Creation date */
#define AT_CREA  6			/* Creator */
#define AT_ACCT  7			/* Account */
#define AT_AREA  8			/* Area */
#define AT_PSWD  9			/* Password for area */
#define AT_BLKS 10			/* Blocksize */
#define AT_ACCE 11			/* Access */
#define AT_ENCO 12			/* Encoding */
#define AT_DISP 13			/* Disposition */
#define AT_LPRO 14			/* Local Protection */
#define AT_GPRO 15			/* Generic Protection */
#define AT_SYSI 16			/* System ID */
#define AT_RECF 17			/* Record Format */
#define AT_SYSP 18			/* System-Dependent Parameters */
#define AT_LENB 19			/* Length in Bytes */
#define AT_EOA  20			/* End of Attributes */

/* Kermit packet information structure */

struct pktinfo {			/* Packet information structure */
    CHAR *bf_adr;			/*  buffer address */
    int   bf_len;			/*  buffer length */
    CHAR *pk_adr;			/* Packet address within buffer */
    int   pk_len;			/*  length of data within buffer */
    int   pk_typ;			/*  packet type */
    int   pk_seq;			/*  packet sequence number */
    int   pk_rtr;			/*  retransmission count */
};

/* Send Modes (indicating which type of SEND command was used) */

#define SM_SEND     0
#define SM_MSEND    1
#define SM_RESEND   2
#define SM_PSEND    3
#define SM_MAIL     4
#define SM_PRINT    5

/* File-related symbols and structures */

#define XMODE_A 0	/* Transfer mode Automatic */
#define XMODE_M 1	/* Transfer mode Manual    */

#define   XYFILN 0  	/*  Naming  */
#define     XYFN_L 0	/*    Literal */
#define     XYFN_C 1	/*    Converted */
#define   XYFILT 1  	/*  Type    */
#define     XYFT_T 0    /*    Text  */
#define     XYFT_B 1    /*    Binary */
#define     XYFT_I 2    /*    Image or Block (VMS) */
#define     XYFT_L 3	/*    Labeled (tagged binary) (VMS or OS/2) */
#define     XYFT_U 4    /*    Binary Undefined (VMS) */
#define     XYFT_M 5	/*    MacBinary (Macintosh) */
#define   XYFILW 2      /*  Warning */
#define   XYFILD 3      /*  Display */
#define     XYFD_N 0    /*    None, Off */
#define     XYFD_R 1    /*    Regular, Dots */
#define     XYFD_C 2    /*    Cursor-positioning (e.g. with curses) */
#define     XYFD_S 3    /*    Simple counter */
#define   XYFILC 4      /*  Character set */
#define   XYFILF 5      /*  Record Format */
#define     XYFF_S  0   /*    Stream */
#define     XYFF_V  1   /*    Variable */
#define     XYFF_VB 2   /*    Variable with RCW's */
#define     XYFF_F  3   /*    Fixed length */
#define     XYFF_U  4   /*    Undefined */
#define   XYFILR 6      /*  Record length */
#define   XYFILO 7      /*  Organization */
#define     XYFO_S 0    /*    Sequential */
#define     XYFO_I 1    /*    Indexed */
#define     XYFO_R 2    /*    Relative */
#define   XYFILP 8      /*  Printer carriage control */
#define     XYFP_N 0    /*    Newline (imbedded control characters) */
#define     XYFP_F 1    /*    FORTRAN (space, 1, +, etc, in column 1 */
#define     XYFP_P 2    /*    Special printer carriage controls */
#define     XYFP_X 4    /*    None */
#define   XYFILX 9      /*  Collision Action */
#define     XYFX_A 3    /*    Append */
#define     XYFX_Q 5    /*    Ask */
#define     XYFX_B 2    /*    Backup */
#define     XYFX_D 4    /*    Discard */
#define     XYFX_R 0    /*    Rename */
#define     XYFX_X 1    /*    Replace */
#define     XYFX_U 6    /*    Update */
#define   XYFILB 10     /*  Blocksize */
#define   XYFILZ 11     /*  Disposition */
#define     XYFZ_N 0    /*    New, Create */
#define     XYFZ_A 1    /*    New, append if file exists, else create */
#define     XYFZ_O 2    /*    Old, file must exist */
#define     XYFZ_X 3    /*    Output to pipe/process */
#define     XYFZ_Y 4    /*    Input from pipe/process */
#define   XYFILS 12     /*  File Byte Size */
#define   XYFILL 13     /*  File Label (VMS) */
#define   XYFILI 14     /*  File Incomplete */
#define   XYFILQ 15     /*  File path action (strip or not) */
#define   XYFILG 16     /*  File download directory */
#define   XYFILA 17     /*  Line terminator for local text files */
#define     XYFA_L 012  /*    LF (as in UNIX) */
#define     XYFA_C 015  /*    CR (as in OS-9 or Mac OS) */
#define     XYFA_2 000  /*  CRLF -- Note: this must be defined as 0 */
#define   XYFILY 18     /*  Destination */

struct tt_info_rec {			/* Terminal emulation info */
    char *x_name;
    char *x_id;
};

/* ANSI-style forward declarations for protocol-related functions. */

_PROTOTYP( int input, (void) );
_PROTOTYP( int inibufs, (int, int) );
_PROTOTYP( int makebuf, (int, int, CHAR [], struct pktinfo *) );
_PROTOTYP( int mksbuf, (int) );
_PROTOTYP( int mkrbuf, (int) );
_PROTOTYP( int spack, (char, int, int, CHAR *) );
_PROTOTYP( VOID proto, (void) );
_PROTOTYP( int rpack, (void) );
_PROTOTYP( int ack1, (CHAR *) );
_PROTOTYP( int ackn, (int) );
_PROTOTYP( int ackns, (int, CHAR *) );
_PROTOTYP( int nack, (int) );
_PROTOTYP( int resend, (int) );
_PROTOTYP( int errpkt, (CHAR *) );
_PROTOTYP( VOID logpkt, (char, int, CHAR *) );
_PROTOTYP( CHAR dopar, (CHAR) );
_PROTOTYP( int chk1, (CHAR *) );
_PROTOTYP( unsigned int chk2, (CHAR *) );
_PROTOTYP( unsigned int chk3, (CHAR *, int) );
_PROTOTYP( int sipkt, (char) );
_PROTOTYP( int sinit, (void) );
_PROTOTYP( VOID rinit, (CHAR *) );
_PROTOTYP( int spar, (CHAR *) );
_PROTOTYP( int rcvfil, (char *) );
_PROTOTYP( CHAR * rpar, (void) );
_PROTOTYP( CHAR * rpar, (void) );
_PROTOTYP( int gnfile, (void) );
_PROTOTYP( int getsbuf, (int) );
_PROTOTYP( int getrbuf, (void) );
_PROTOTYP( int freesbuf, (int) );
_PROTOTYP( int freerbuf, (int) );
_PROTOTYP( int dumpsbuf, (void) );
_PROTOTYP( int dumprbuf, (void) );
_PROTOTYP( VOID freerpkt, (int) );
_PROTOTYP( int chkwin, (int, int, int) );
_PROTOTYP( int rsattr, (CHAR *) );
_PROTOTYP( char *getreason, (char *) );
_PROTOTYP( int scmd, (char, CHAR *) );
_PROTOTYP( int encstr, (CHAR *) );
_PROTOTYP( int decode, (CHAR *, int (*)(char), int) );
_PROTOTYP( int fnparse, (char *) );
_PROTOTYP( int syscmd, (char *, char *) );
_PROTOTYP( int cwd, (char *) );
_PROTOTYP( VOID screen, (int, char, long, char *) );
_PROTOTYP( int remset, (char *) );
_PROTOTYP( int initattr, (struct zattr *) );
_PROTOTYP( int gattr, (CHAR *, struct zattr *) );
_PROTOTYP( int adebu, (char *, struct zattr *) );
_PROTOTYP( int canned, (CHAR *) );
_PROTOTYP( int opent, (struct zattr *) );
_PROTOTYP( int opena, (char *, struct zattr *) );
_PROTOTYP( int openi, (char *) );
_PROTOTYP( int openo, (char *, struct zattr *, struct filinfo *) );
_PROTOTYP( int reof, (char *, struct zattr *) );
_PROTOTYP( VOID reot, (void) );
_PROTOTYP( int sfile, (int) );
_PROTOTYP( int sattr, (int) );
_PROTOTYP( int sdata, (void) );
_PROTOTYP( int seof, (CHAR *) );
_PROTOTYP( int sxeof, (CHAR *) );
_PROTOTYP( int seot, (void) );
_PROTOTYP( int window, (int) );
_PROTOTYP( int errmsg, (char *) );
_PROTOTYP( int clsif, (void) );
_PROTOTYP( int clsof, (int) );
_PROTOTYP( CHAR setgen, (char, char *, char *, char *) );
_PROTOTYP( int getpkt, (int, int) );
_PROTOTYP( int putsrv, (char) );
_PROTOTYP( int puttrm, (char) );
_PROTOTYP( int putque, (char) );
_PROTOTYP( int putfil, (char) );
_PROTOTYP( VOID zdstuff, (CHAR) );
_PROTOTYP( int tinit, (void) );
_PROTOTYP( VOID pktinit, (void) );
_PROTOTYP( VOID rinit, (CHAR *) );
_PROTOTYP( VOID resetc, (void) );
_PROTOTYP( VOID xsinit, (void) );
_PROTOTYP( int adjpkl, (int,int,int) );
_PROTOTYP( int chktimo, (int,int) );
_PROTOTYP( int nxtpkt, (void) );
_PROTOTYP( int ack, (void) );
_PROTOTYP( int ackns, (int, CHAR *) );
_PROTOTYP( int ackn, (int) );
_PROTOTYP( int ack1, (CHAR *) );
_PROTOTYP( int nack, (int) );
_PROTOTYP( VOID rcalcpsz, (void) );
_PROTOTYP( int resend, (int) );
_PROTOTYP( int errpkt, (CHAR *) );
_PROTOTYP( int srinit, (int, int) );
_PROTOTYP( VOID tstats, (void) );
_PROTOTYP( VOID fstats, (void) );
_PROTOTYP( VOID intmsg, (long) );
_PROTOTYP( VOID ermsg, (char *) );
_PROTOTYP( int chkint, (void) );
_PROTOTYP( VOID sdebu, (int) );
_PROTOTYP( VOID rdebu, (CHAR *, int) );
_PROTOTYP( char * dbchr, ( int ) );
#ifdef COMMENT
_PROTOTYP( SIGTYP stptrap, (int, int) );
_PROTOTYP( SIGTYP trap, (int, int) );
#else
_PROTOTYP( SIGTYP stptrap, (int) );
_PROTOTYP( SIGTYP trap, (int) );
#endif /* COMMENT */
_PROTOTYP( char * ck_errstr, (void) );

/* User interface functions needed by main program, etc. */

_PROTOTYP( VOID prescan, (int) );
_PROTOTYP( VOID setint, (void) );
_PROTOTYP( VOID doinit, (void) );
_PROTOTYP( VOID cmdini, (void) );
_PROTOTYP( int dotake, (char *) );
_PROTOTYP( int cmdlin, (void) );
_PROTOTYP( int conect, (void) );
_PROTOTYP( int ckcgetc, (int) );
_PROTOTYP( int ckcputc, (int) );
_PROTOTYP (int mdmhup, (void) );
_PROTOTYP( VOID herald, (void) );
_PROTOTYP( VOID fixcmd, (void) );
_PROTOTYP( int doarg, (char) );
_PROTOTYP( VOID usage, (void) );
_PROTOTYP( VOID doclean, (void) );
_PROTOTYP( int sndhlp, (char *) );
_PROTOTYP( VOID ckhost, (char *, int) );
_PROTOTYP( int gettcs, (int, int) );
_PROTOTYP( VOID makestr, (char **, char *) );
_PROTOTYP( VOID getdialenv, (void) );
_PROTOTYP( VOID setprefix, (int) );
_PROTOTYP( VOID initproto, (int,char *,char *,char *,char *,char *,char *) );
_PROTOTYP( char * getsysid, (char *) );
_PROTOTYP( int getsysix, (char *) );
#ifdef CK_TIMERS
_PROTOTYP( VOID rttinit, (void) );
_PROTOTYP( int getrtt, (int, int) );
#endif /* CK_TIMERS */

_PROTOTYP( int is_a_tty, (int) );
_PROTOTYP( int snddir, (char *) );
_PROTOTYP( int snddel, (char *) );
_PROTOTYP( int sndtype, (char *) );
_PROTOTYP( int dooutput, (char *) );
_PROTOTYP( int isabsolute, (char *) );
_PROTOTYP( int chkspkt, (char *) );
_PROTOTYP( VOID whoarewe, (void) );

#ifdef CK_APC
_PROTOTYP( int kstart, (CHAR) );
_PROTOTYP( int zstart, (CHAR) );
#ifdef CK_XYZ
_PROTOTYP( int chkspkt, (char *) );
#endif /* CK_XYZ */
#endif /* CK_APC */

#ifdef KANJI
_PROTOTYP( int zkanji, (int (*)(void)) ); /* Kanji function prototypes */
_PROTOTYP( int zkanjf, (void) );
_PROTOTYP( int zkanjz, (void) );
_PROTOTYP( int xkanjz, (int (*)(char) ) );
_PROTOTYP( int xkanji, (int, int (*)(char) ) );
#endif /* KANJI */

#endif /* CKCKER_H */

/* End of ckcker.h */
