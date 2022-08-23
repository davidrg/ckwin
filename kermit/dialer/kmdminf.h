extern "C" {

#ifndef _PROTOTYP
#define _PROTOTYP( func, parms ) func parms
#endif
#define OLDMODEMS

    /*
    * Define symbolic modem numbers.
    *
    * The numbers MUST correspond to the ordering of entries
    * within the modemp array, and start at one (1).
    *
    * It is assumed that there are relatively few of these
    * values, and that the high(er) bytes of the value may
    * be used for modem-specific mode information.
    *
    * REMEMBER that only the first eight characters of these
    * names are guaranteed to be unique.
    */

#define		n_ATTDTDM	 1
#define         n_ATTISN         2
#define		n_ATTMODEM	 3
#define		n_CCITT		 4
#define		n_CERMETEK	 5
#define		n_DF03		 6
#define		n_DF100		 7
#define		n_DF200		 8
#define		n_GDC		 9
#define		n_HAYES		10
#define		n_PENRIL	11
#define		n_RACAL		12
#define		n_UNKNOWN       13
#define		n_VENTEL	14
#define		n_CONCORD	15
#define		n_ATTUPC	16	/* aka UNIX PC and ATT7300 */
#define		n_ROLM          17      /* Rolm CBX DCM */
#define		n_MICROCOM	18	/* Microcoms in SX command mode */
#define         n_USR           19	/* Modern USRs */
#define         n_TELEBIT       20      /* Telebits of all kinds */
#define         n_DIGITEL       21	/* Digitel DT-22 (CCITT variant) */
#define         n_H_1200        22	/* Hayes 1200 */
#define		n_H_ULTRA       23	/* Hayes Ultra and maybe Optima */
#define		n_H_ACCURA      24	/* Hayes Accura and maybe Optima */
#define         n_PPI           25	/* Practical Peripherals */
#define         n_DATAPORT      26	/* AT&T Dataport */
#define         n_BOCA          27	/* Boca */
#define		n_MOTOROLA      28	/* Motorola Fastalk or Lifestyle */
#define		n_DIGICOMM	29	/* Digicomm Connection */
#define		n_DYNALINK      30	/* Dynalink 1414VE */
#define		n_INTEL		31	/* Intel 14400 Faxmodem */
#define		n_UCOM_AT	32	/* Microcoms in AT mode */
#define		n_MULTI		33	/* Multitech MT1432 */
#define		n_SUPRA		34	/* SupraFAXmodem */
#define	        n_ZOLTRIX	35	/* Zoltrix */
#define		n_ZOOM		36	/* Zoom */
#define		n_ZYXEL		37	/* ZyXEL */
#define         n_TAPI          38	/* TAPI Line modem - whatever it is */
#define         n_TBNEW         39	/* Newer Telebit models */
#define		n_MAXTECH       40	/* MaxTech XM288EA */
#define         n_UDEF          41	/* User-Defined */
#define         n_RWV32         42	/* Generic Rockwell V.32 */
#define         n_RWV32B        43	/* Generic Rockwell V.32bis */
#define         n_RWV34         44	/* Generic Rockwell V.34 */
#define		n_MWAVE		45	/* IBM Mwave Adapter */
#define         n_TELEPATH      46	/* Gateway Telepath */
#define         n_MICROLINK     47	/* MicroLink modems */
#define         n_CARDINAL      48	/* Cardinal modems */
#define         n_GENERIC       49      /* Generic high-speed */
#define         n_XJACK         50	/* Megahertz X-Jack */
#define         n_SPIRITII      51	/* Quickcomm Spirit II */
#define         n_MONTANA       52	/* Motorola Montana */
#define         n_COMPAQ        53	/* Compaq Data+Fax Modem */
#define         n_FUJITSU       54	/* Fujitsu Fax/Modem Adpater */
#define         n_MHZATT        55	/* Megahertz AT&T V.34 */
#define         n_SUPRASON      56	/* SupraSonic */
#define         n_BESTDATA      57	/* Best Data */
#define         n_ATT1900       58      /* AT&T STU III Model 1900 */
#define         n_ATT1910       59      /* AT&T STU III Model 1910 */
#define         n_KEEPINTOUCH   60      /* AT&T KeepInTouch */
#define         n_USRX2         61	/* USR XJ-1560 X2 56K */
#define         n_ROLMAT        62	/* ROLM 244PC or 600-series with AT */
#define		n_ATLAS         63      /* Atlas / Newcom ixfC 33.6 */
#define         n_CODEX         64	/* Motorola Codex 326X Series */
#define         n_MT5634ZPX     65	/* Multitech MT5634ZPX */
#define         n_ULINKV250     66	/* Microlink ITU-T V.250 56K */
#define         n_ITUTV250      67	/* Generic ITU-T V.250 */
#define         n_RWV90         68	/* Generic Rockwell V.34 */
#define         n_SUPRAX        69      /* Diamond Supra Express V.90 */
#define         n_LUCENT        70      /* Lucent Venus chipset */
#define         n_PCTEL         71      /* PCTel chipset */
#define         n_CONEXANT      72      /* Conexant modem family */
#define		n_ZOOMV34	73	/* Zoom */
#define		n_ZOOMV90	74	/* Zoom */
#define         n_ZOOMV92       75      /* ZOOM V.92 */
#define         n_MOTSM56       76	/* Motorola SM56 chipset */
#define		MAX_MDM		76	/* Number of modem types */

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
#define CKD_ID 512			/* Has Caller ID */

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
    char * sp_on_str;		/* Speaker on string */
    char * sp_off_str;		/* Speaker off string */
    char * vol1_str;		/* Volume low string */
    char * vol2_str;		/* Volume med string */
    char * vol3_str;		/* Volume high string */
    char * ignoredt;		/* Ignore Dial Tone string */
    char * ini2;                /* Last-min init string (not used in dialer) */
    long   max_speed;		/* Maximum interface speed */
    long   capas;		/* Capability bits */
    /* function to read modem's response string to a non-dialing command */
    _PROTOTYP( int (*ok_fn), (int,int) );
};

#ifndef M_OLD
#define M_OLD 0
#endif
#ifndef CM_INV
#define CM_INV 1
#define CM_ABR 2
#endif

}
