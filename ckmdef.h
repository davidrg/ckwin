/* Version 0.8(35) - Jim Noble at Planning Research Corporation, June 1987. */
/* Ported to Megamax native Macintosh C compiler. */

/* Symbol Definitions */

#define TRUE      1		/* Boolean constants */
#define FALSE     0
#define NIL				0
#define NULL			0
#define NILPTR		(Ptr) NIL
#define NILPROC		(ProcPtr) NIL
#define ALLTYPES -1		/* All types for SFGetFile */

#define MYBUFSIZE   2048	/* Serial driver replacement buffer */

#define OKBtn	ok		/* OK and Cancel constants */
#define QuitBtn	cancel		/* Defined by Dialog Mgr */

#define btnOff	0
#define btnOn	1

#define getctlhdl(item,dp) (ControlHandle) gethdl(item,dp)

#define ApplCreator 'KR09'

/* settings for parity */

#define MPARITY_ODD (oddParity+stop10+data7)
#define MPARITY_EVEN (evenParity+stop10+data7)
#define MPARITY_MARK (noParity+stop20+data7)
#define MPARITY_SPACE (noParity+stop10+data8)
#define MPARITY_NONE (noParity+stop10+data8)

#define KPARITY_ODD 'o'
#define KPARITY_EVEN 'e'
#define KPARITY_MARK 'm'
#define KPARITY_SPACE 's'
#define KPARITY_NONE 0

typedef struct {		/* file arguments since Kermit */
    char fillcl[64], filrem[64];/* doesn't pass all the info */
    int filflg;			/* we need... */
    int fildflg;		/* default flags */
    short filvol;		/* file volume */
    int filfldr;		/* file folder for new files */
    long filsiz;		/* size of file after open */
    long rsrcsiz;		/* size of resource fork (for MacBinary fmt) */
}   FILINF;


/* Flags in filflg, fildflg */

#define FIL_BINA	 1
#define FIL_TEXT	 2
#define FIL_RSRC	 4
#define FIL_DATA	 8
#define FIL_OKILL 16		/* same as kermit's !warn */
#define FIL_DODLG 32		/* show dialogs on recv */
#define FIL_ALLFL 64		/* send all files from the curr folder */

#define FIL_RBDT (FIL_RSRC | FIL_BINA | FIL_DATA | FIL_TEXT)

/* (PWP) MacBinary is selected when both FIL_RSRC and FIL_DATA are selected */

/* MacBinary format is the following header, the data fork (padded to 128),
   and the resource fork (padded to 128) */
/* MacBinary header structure (lifted from NCSA Telnet 2.2) */
typedef struct MBHead {		/* MacBinary Header */
    char zero1;
    char name[64];
    char type[4];
    char creator[4];
    char flags;
    char zero2;
    char location[6];
    char protected;
    char zero3;
    char dflen[4];
    char rflen[4];
    char cdate[4];
    char mdate[4];
    char filler[27];
    char typeid[2];
} MBHead;


/* type definitions for keyboard macro structures */
typedef struct {
    short code;			/* key and modification code for this
				 * keymacro */
    char flags;			/* flags for this key macro definition */
    char len;			/* length of the macro string in bytes */
    long macro;			/* if len <=4 contains the macrostring else */
}   macrorec;			/* it contains a pointer to the macrostring */

#define shortBreak	1	/* flag for short break */
#define longBreak	2	/* flag for long break */

#define leftArrowKey	3	/* send left arrow */
#define rightArrowKey	4	/* send right arrow */
#define upArrowKey	5
#define downArrowKey	6

#define keypf1		7
#define keypf2		8
#define keypf3		9
#define keypf4		10

#define keycomma	11
#define keyminus	12
#define keyperiod	13
	      /* there is no keyslash */
#define key0		15
#define key1		16
#define key2		17
#define key3		18
#define key4		19
#define key5		20
#define key6		21
#define key7		22
#define key8		23
#define key9		24

#define keyenter	25

#define NUMOFMODS 4
typedef struct {
    short modbits;		/* modifier combination */
    char prefix[20];		/* prefix string */
}   modrec;

/* the following type is used as an overlay for a memory	 */
/* block which contains the macros. It is !not! used for a */
/* static variable. So don't worry about its size 				 */
typedef struct {
    modrec modtab[NUMOFMODS];	/* modifier table */
    short numOfMacros;		/* number of macros the table contains */
    macrorec mcr[1024];		/* array of macro definitions */
}   macrodefs;

typedef struct {		/* dummy struct definiton to get the size of */
    /* the base fields in macrodefs easily */
    modrec modtab[NUMOFMODS];	/* modifier table */
    short numOfMacros;		/* number of macros the table contains */
}   macrodefsize;

#define MacroBaseSize sizeof(macrodefsize)

typedef macrodefs **hmacrodefs;	/* handle to a macrodefs structure */

/* Globals Kermit variables */

extern char *cmarg,		/* pointer to cmd argument (file) */
   *cmarg2,			/* pointer to second cmd argument */
    mypadc,			/* padding char to ask for */
    padch, eol,			/* end of line char to send */
    seol, mystch,		/* outbound packet start character */
    stchr, sstate;		/* kermit protocol state */

extern long ffc,		/* file character count  */
    fsize;			/* size of current file */

extern int speed,		/* speed of port */
    parity,			/* current parity setting */
    flow,			/* TRUE if using flow control */
    duplex,			/* TRUE if full duplex */
    binary,			/* TRUE if file mode is binary */
    displa,			/* TRUE for calls to screen() */
    seslog,			/* TRUE if writing session log */
    tralog,			/* TRUE if writing transaction log */
    bctr,			/* block check type (1, 2, or 3) */
    delay,			/* initial delay before sending */
    mypadn,			/* something to do with padding */
    npad, timint,		/* timeout interval in seconds */
    rtimo, spsiz,		/* max packet size we send */
    rpsiz, urpsiz,		/* user requested packet size */
    turnch,			/* line "turnaround" character */
    turn,			/* if TRUE "turnaround" enabled */
    quiet,			/* if TRUE screen() should be quiet */
    nfils,			/* */
    warn,			/* TRUE does name conversion, else */
				/* deletes duplicate names on recv */
    server,			/* TRUE if we are in server mode */
    cxseen,			/* if TRUE abort file */
    czseen,			/* if TRUE abort group */
    keep,			/* Keep incomplete files */
    tlevel,			/* > -1 if take file is running */
    newline,			/* TRUE if linefeed after carriage return */
    autowrap,			/* TRUE if autowrap is on */
    screeninvert,		/* TRUE if inverted screen */
    autorepeat,			/* TRUE if auto repeat is on */
    smoothscroll,		/* TRUE if smooth scrolling is on */
    transparent,		/* TRUE if control characters are not shown */
    blockcursor,		/* TRUE if block cursor is used */
    mouse_arrows,		/* TRUE if we send arrow keys on mouse downs */
    visible_bell,		/* true if we do blink instead of bell */
    eightbit_disp,		/* do 8 bit wide to the screen */
    screensize,			/* number of lines on the screen */
    graphicsinset[4],		/* (UoR) current character sets */
    blinkcursor,		/* TRUE if we make the cursor blink */
    sendusercvdef;		/* use rec. file mode defaults as send defaults */

extern Boolean mcmdactive,	/* TRUE if menu command keys are active */
    fkeysactive;		/* TRUE if FKEYs are enabled */

#define ASCII_SET	0	/* (UoR) VT100 character set numbers */

#define GRAF_SET	2	/* (PWP) Note: these refer to the 7 bit      */
#define TECH_SET	3	/*  character set part.  Eg. full ISO 8859/1 */
#define DECINTL_SET	4	/*  is formed by USA_SET in GL and LAT1_SET  */
#define LAT1_SET	5	/*  in GR.  Read ISO 2022 about 4 times and  */
#define LAT2_SET	6	/*  you will understand what is going on     */
#define LAT3_SET	7
#define LAT4_SET	8
#define LATCYR_SET	9
#define LATARAB_SET	10
#define LATGREEK_SET	11
#define LATHEBREW_SET	12
#define LAT5_SET	13

/* the order of these must match that of ckmres.h */
#define USA_NAT		0	/* (PWP) "national" character set numbers */
#define UK_NAT		1
#define DUTCH_NAT	2
#define FINNISH_NAT	3
#define FRENCH_NAT	4
#define FRENCHCAN_NAT	5
#define GERMAN_NAT	6
#define ITALIAN_NAT	7
#define NORWEGIAN_NAT	8
#define PORTUGUESE_NAT	9
#define SPANISH_NAT	10
#define SWEDISH_NAT	11
#define SWISS_NAT	12


/* Mac global variables */

extern short innum,		/* serial line input number */
    outnum;			/* serial line output number */

extern int protocmd;		/* current cmd that made us enter */
 /* protocol or 0 if protocol is */
 /* not running, or -1 if remote cmd */
extern FILINF filargs;		/* file information flags */

/*
 * ScrDmpEnb is a flag byte used by the toolbox event manager to say
 * whether command-shift-1 through command-shift-9 do anything special
 * like dump the screen, eject the diskettes, etc.  This byte is used in
 * INIT0, the keyboard mapping routine (see IM index under scrdmpenb).
 *
 * N.B: The location of this value may change if MAC comes out with a
 * new SYSTEM.
 *
 */

#define ScrDmpEnb (*(unsigned char *) 0x2f8)
#define scrdmpenabled ((unsigned char) 0xff)
#define scrdmpdisabled ((unsigned char) 0x00)


/****************************************************************************/
pascal void
Debugger ()
/****************************************************************************/
extern 0xA9FF;			/* invoke MacsBug */
