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


/* type definitions for keyboard macro structures */
typedef struct {
    short code;			/* key and modification code for this
				 * keymacro */
    char flags;			/* flags for this key macro definition */
    char len;			/* length of the macro string in bytes */
    long macro;			/* if len <=4 contains the macrostring else */
}   macrorec;			/* it contains a pointer to the macrostring */

#define shortBreak 1		/* flag for short break */
#define longBreak	 2	/* flag for long break */

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

extern long ffc;		/* file character count  */

extern int speed,		/* speed of port */
    parity,			/* current parity setting */
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
    nat_chars,			/* half transparent -- show undef. control
				 * chars */
    blinkcursor;		/* TRUE if we make the cursor blink */

extern Boolean mcmdactive,	/* TRUE if menu command keys are active */
    fkeysactive;		/* TRUE if FKEYs are enabled */

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
