/*  File CKOCON.H  --  OS/2 C-Kermit keyboard verb definitions & tables.  */

/*
  Author: Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
            The Kermit Project, Columbia University, New York City
          Jeffrey E Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1993, 2004, Trustees of Columbia University in the City of New
  York.
*/
#ifndef CKOCON_H
#define CKOCON_H

/*
 * ============================ features =====================================
 */

/*
 * =============================#defines======================================
 */

/* Symbols for characters used in Compose sequences */

#define C_BREVE  (CHAR)'%'              /* Breve accent */
#define C_CARON  (CHAR)'&'              /* Caron (hacek) */
#define C_OGONEK (CHAR)'='              /* Ogonek */
#define C_ACUTE  (CHAR)0x27             /* Acute accent */
#define C_DACUTE (CHAR)'#'              /* Double acute accent */
#define C_RING   (CHAR)'*'              /* Ring above */
#define C_DOT    (CHAR)'.'              /* Dot above */
#define C_CEDILLA (CHAR)','             /* Cedilla */

#ifdef NT
#ifndef APIRET
#define APIRET DWORD
#endif /* APIRET */
#ifndef SEM_INDEFINITE_WAIT
#define SEM_INDEFINITE_WAIT INFINITE
#endif /* SEM_INDEFINITE_WAIT */
#ifndef DCWW_WAIT
#define DCWW_WAIT -1
#endif /* DCWW_WAIT */
#ifndef ERROR_VIO_ROW
#define ERROR_VIO_ROW 1
#endif
#ifndef ERROR_VIO_COL
#define ERROR_VIO_COL 2
#endif
#ifndef CCHMAXPATH
#define CCHMAXPATH _MAX_PATH
#endif
#ifndef TID
#define TID HANDLE
#endif
#ifndef PIB
#define PIB HANDLE
#define PPIB HANDLE*
#endif
#ifndef TIB
#define TIB HANDLE
#define PTIB HANDLE*
#endif
#ifndef HVIO
#define HVIO HANDLE
#endif
#ifndef HMTX
#define HMTX HANDLE
#endif
#ifndef HAB
#define HAB HANDLE
#endif
#ifndef HKBD
#define HKBD HANDLE
#endif
#ifndef HDIR
#define HDIR HANDLE
#endif
#endif /* NT */

#ifndef min
#define min(a,b) ((a) <= (b) ? (a) : (b))
#endif /* min */

#ifdef NT
#define MAXSCRNCOL  256            /* Maximum screen columns */
#define MAXSCRNROW  128            /* Maximum screen rows    */
#define MAXTERMSIZE (MAXSCRNCOL*MAXSCRNROW)
#else /* NT */
/* OS/2 WARP allows for screen widths up to 255 characters.
   WARP says the max number of
   characters on the screen can be 8192.  8192 / 132 = 62.06 so
   we allow a maximum of 62 rows on the screen.

   The 8192 limit is enforced by the command parser.
*/
#define MAXSCRNCOL 162            /* Maximum screen columns - upto 255 */
#define MAXSCRNROW 102            /* Maximum screen rows  - upto 102   */
#define MAXTERMSIZE 8192
#endif /* NT */
#define MAXTERMCOL MAXSCRNCOL
#define MAXTERMROW (MAXSCRNROW-1)


#ifdef TRUE
#undef TRUE
#endif /* TRUE */
#define TRUE 1

#ifdef FALSE
#undef FALSE
#endif /* FALSE */
#define FALSE 0

#define UPWARD   6                      /* Scroll up */
#define DOWNWARD 7                      /* Scroll down */
#define LBUFSIZE roll.bufsize           /* Maximum lines in rollback buffer */

/*
  The video adapter stores each screen character in a two-byte cell.  The
  first byte of each cell contains the 8-bit character value.  The second byte
  contains the video attributes for the character, and looks like this:

         Background      Foreground
         Color           Color
    +---+---+---+---+---+---+---+---+
    | b | R | G | B | i | R | G | B |
    +---+---+---+---+---+---+---+---+
      b = blink       i = intensity
      0 = nonblinking 0 = normal
      1 = blinking    1 = high

  The i bit applies to the foreground color.  The meaning of the b bit depends
  on the VIO state; it can mean (a) the foreground character blinks, or (b) the
  background color is high-intensity.  C-Kermit uses (b) because blinking only
  works in a fullscreen session.  See the call to VioSetState().  The RGB
  bits select the desired mixture of Red, Green, and Blue.

  The swapcolors macro exchanges the fore- and background colors (the RGB
  bits) but leaves the intensity/blink bits where there are.  Thus if the
  foreground color is bold and the background color is not bold, the same
  will be true after swapcolors().
*/
#define swapcolors(x) (((x)&(unsigned)0x88)|(((x)&0x70)>>4)|(((x)&0x07)<<4))
#define byteswapcolors(x) ((((x)&0x70)>>4)|(((x)&0x07)<<4)|((x)&(unsigned char)0x88))

/*
 *
 * =============================typedefs======================================
 */
#ifndef __cplusplus
typedef int bool;
#else
#ifndef __BOOL_DEFINED
typedef int bool;
#endif
#endif

#ifndef KUI
#ifdef OS2ONLY
typedef struct _CK_VIDEOMODEINFO {
      UCHAR  fbType;
      UCHAR  color;
      USHORT col;
      USHORT row;
      USHORT hres;
      USHORT vres;
      USHORT wcol;
      USHORT wrow;
} CK_VIDEOMODEINFO, *PCK_VIDEOMODEINFO ;
#else /* OS2ONLY */
typedef struct _CK_VIDEOMODEINFO {
      UCHAR  color;
      USHORT col;
      USHORT row;
      USHORT sbcol;
      USHORT sbrow;
} CK_VIDEOMODEINFO, *PCK_VIDEOMODEINFO ;
#endif /* OS2ONLY */
#endif /* KUI */

typedef struct _CK_CURSORINFO {
      USHORT   yStart;             /* top (negative is percentage) */
      USHORT   cEnd;               /* bottom (negative is percentage) */
      USHORT   cx;                 /* width */
      USHORT   attr;               /* -1 means hidden */
} CK_CURSORINFO, *PCK_CURSORINFO ;

typedef struct ascreen_rec {    /* Structure for saving screen info */
#ifndef KUI
    CK_VIDEOMODEINFO    mi;
#endif /* KUI */
    unsigned char       ox;
    unsigned char       oy;
    unsigned char       att;
    struct cell_struct  *scrncpy;
} ascreen;

/*
  Note, the following are related to the VT terminal screen, not the PC video
  adapter.  That is, 'attribute' (above) is the PC video attribute byte,
  'attrib' (below) is the VT terminal attribute structure.  etc.
*/
typedef struct _vtattrib {      /* Character (SGR) attributes, 1 bit each */
    unsigned reversed:1;        /* Reverse video */
    unsigned blinking:1;        /* Blinking */
    unsigned underlined:1;      /* Underlined (simulated) */
    unsigned bold:1;            /* Bold (high intensity) */
    unsigned invisible:1;       /* Invisible */
    unsigned unerasable:1;      /* Uneraseable by DECSCA */
    unsigned graphic:1;         /* Graphic character */
    unsigned dim:1;             /* Dim */
    unsigned wyseattr:1;        /* Wyse Attribute */
    unsigned italic:1;          /* Italic */
    unsigned hyperlink:1;       /* Hyperlink */
    unsigned short linkid;      /* Hyperlink Index */
} vtattrib ;

/*
 *
 *  Virtual buffer stuff
 *
 */
#define VT_CHAR_ATTR_NORMAL           ((USHORT) 0x0000)
#define VT_CHAR_ATTR_BOLD             ((USHORT) 0x0001)
#define VT_CHAR_ATTR_UNDERLINE        ((USHORT) 0x0002)
#define VT_CHAR_ATTR_BLINK            ((USHORT) 0x0004)
#define VT_CHAR_ATTR_REVERSE          ((USHORT) 0x0008)
#define VT_CHAR_ATTR_INVISIBLE        ((USHORT) 0x0010)
#define VT_CHAR_ATTR_PROTECTED        ((USHORT) 0x0020)
#define VT_CHAR_ATTR_GRAPHIC          ((USHORT) 0x0040)
#define VT_CHAR_ATTR_DIM              ((USHORT) 0x0080)
#define WY_CHAR_ATTR                  ((USHORT) 0x0100)
#define KUI_CHAR_ATTR_UPPER_HALF      ((USHORT) 0x0200)
#define KUI_CHAR_ATTR_LOWER_HALF      ((USHORT) 0x0400)
#define VT_CHAR_ATTR_ITALIC           ((USHORT) 0x0800)
#define VT_CHAR_ATTR_HYPERLINK        ((USHORT) 0x1000)

#define VT_LINE_ATTR_NORMAL           ((USHORT) 0x00)
#define VT_LINE_ATTR_DOUBLE_WIDE      ((USHORT) 0x01)
#define VT_LINE_ATTR_DOUBLE_HIGH      ((USHORT) 0x02)
#define VT_LINE_ATTR_UPPER_HALF       ((USHORT) 0x04)
#define VT_LINE_ATTR_LOWER_HALF       ((USHORT) 0x00)

#define WY_LINE_ATTR_BG_NORMAL        ((USHORT) 0x00)
#define WY_LINE_ATTR_BG_BOLD          ((USHORT) 0x10)
#define WY_LINE_ATTR_BG_INVISIBLE     ((USHORT) 0x20)
#define WY_LINE_ATTR_BG_DIM           ((USHORT) 0x40)

#define WY_LINE_ATTR_PROTECTED        ((USHORT) 0x08)

typedef struct cell_struct {          /* to be used with VioWrtNCell() */
#ifdef NT
    unsigned short c ;                /* unicode character */
#else /* CKOUNI_OUT */
    unsigned char c ;                 /* character */
#endif /* CKOUNI_OUT */
    unsigned char a ;                 /* attribute */
} viocell ;

typedef struct videoline_struct {
    unsigned short      width ;           /* number of valid chars */
    viocell *           cells ;           /* valid to length width */
    unsigned short *    vt_char_attrs ;   /* bitwise & of VT_CHAR_ATTR Values */
    unsigned short *    hyperlinks;       /* hyperlink index values */
    unsigned short      vt_line_attr ;
    short               markbeg ;
    short               markshowend ;
    short               markend ;
    } videoline ;

typedef struct pos_struct {
    unsigned short x, y ;
    } position ;

typedef struct popup_struct {
    unsigned short height,
                   width ;                     /* before centering */
    unsigned char  a ;                         /* attribute */
    unsigned char  c[MAXTERMROW][MAXTERMCOL] ; /* character */
    unsigned char  gui;                        /* gui? */
    } videopopup ;

typedef struct videobuffer_struct {
    unsigned long linecount ;
    videoline *   lines ;      /* of count linecount */
    unsigned long beg,         /* beginning of scrollable region */
                  top,         /* first line of write to terminal screen */
                  scrolltop,   /* top of scroll screen */
                  end ;        /* end of scrollable region */
    position      cursor ;     /* cursor position */
    videopopup *  popup ;      /* popup menu      */
    long          marktop,     /* first line marked */
                  markbot ;    /* last line marked  */
    int           width,       /* width of virtual screen */
                  height ;     /* height of virtual screen */
    int           bookmark[10];/* bookmarks */
    int           hscroll ;    /* column to display from */
    int           display_height;
   } videobuffer ;

/* Video Buffer IDs */
#define VCMD    0   /* command mode */
#define VTERM   1   /* terminal mode */
#define VCS     2   /* client-server mode */
#define VSTATUS 3   /* Terminal Mode Status Line */
#define VNUM    4   /* number of video modes */

/* Escape-sequence parser state definitions. */
/* For explanation, see ckucon.c, where this code is copied from. */

#define ES_NORMAL   0                   /* Normal, not in escape sequence */
#define ES_GOTESC   1                   /* Current character is ESC */
#define ES_ESCSEQ   2                   /* Inside an escape sequence */
#define ES_GOTCSI   3                   /* Inside a control sequence */
#define ES_STRING   4                   /* Inside DCS,OSC,PM, or APC string */
#define ES_TERMIN   5                   /* 1st char of string terminator */
#define ES_GOTTILDE 6                   /* Beginning of HZ sequence */
#define ES_TILDESEQ 7

#define ESCBUFLEN 128

/* DECSASD settings */
#define SASD_TERMINAL 0
#define SASD_STATUS   1

/* DECSSDT settings */
#define  SSDT_BLANK 0
#define  SSDT_INDICATOR 1
#define  SSDT_HOST_WRITABLE 2

_PROTOTYP( void setdecssdt, (int));
_PROTOTYP( void setdecsasd, (bool));

enum charsetsize { cs94, cs96, cs128, csmb } ;
struct _vtG {
    unsigned char designation, def_designation ;
    enum charsetsize size, def_size ;
    bool national ;
    bool c1, def_c1 ;
#ifndef CKOUNI
    CHAR (*rtoi)(CHAR) ;
    CHAR (*itol)(CHAR) ;
    CHAR (*ltoi)(CHAR) ;
    CHAR (*itor)(CHAR) ;
#else /* CKOUNI */
    USHORT (*rtoi)(CHAR) ;
    int    (*itol)(USHORT) ;
    USHORT (*ltoi)(CHAR) ;
    int    (*itor)(USHORT) ;
#endif /* CKOUNI */
    BOOL init ;
    };

enum markmodes { notmarking, inmarkmode, marking } ;
enum helpscreen { hlp_normal, hlp_rollback, hlp_markmode, hlp_compose,
    hlp_escape, hlp_mouse, hlp_bookmark, hlp_search, hlp_ucs2 } ;

/*
 *
 * =============================prototypes====================================
 */

_PROTOTYP( void rdcomwrtscr, (void *) ) ;
_PROTOTYP( int  scriptwrtbuf, (unsigned short) ) ;
_PROTOTYP( void savescreen, (ascreen *,int,int) ) ;
_PROTOTYP( int restorescreen, (ascreen *) ) ;
_PROTOTYP( void reverserange, (SHORT, SHORT, SHORT, SHORT) ) ;
_PROTOTYP( USHORT ReadCellStr, ( viocell *, PUSHORT, USHORT, USHORT ) );
_PROTOTYP( USHORT WrtCellStr, ( viocell *, USHORT, USHORT, USHORT ) );
_PROTOTYP( USHORT ReadCharStr, ( viocell *, PUSHORT, USHORT, USHORT ) );
_PROTOTYP( USHORT WrtCharStrAtt, ( PCH, USHORT, USHORT, USHORT, PBYTE ) );
_PROTOTYP( USHORT WrtNCell, ( viocell, USHORT, USHORT, USHORT ) );
#ifndef KUI
_PROTOTYP( USHORT GetMode, ( PCK_VIDEOMODEINFO ) );
_PROTOTYP( USHORT SetMode, ( PCK_VIDEOMODEINFO ) );
#endif /* KUI */
_PROTOTYP( USHORT GetCurType, ( PCK_CURSORINFO ) );
_PROTOTYP( USHORT SetCurType, ( PCK_CURSORINFO ) );
_PROTOTYP( USHORT GetCurPos, ( PUSHORT, PUSHORT ) );
_PROTOTYP( USHORT SetCurPos, ( USHORT, USHORT ) );
_PROTOTYP( USHORT ShowBuf, ( USHORT, USHORT ) );
_PROTOTYP( USHORT Set132Cols, ( int ) );
_PROTOTYP( USHORT Set80Cols, ( int ) );
_PROTOTYP( USHORT SetCols, ( int ) );
_PROTOTYP( APIRET VscrnInit, ( BYTE ) ) ;
_PROTOTYP( USHORT VscrnScrollLf, ( BYTE, USHORT, USHORT, USHORT, USHORT, USHORT, viocell ) );
_PROTOTYP( USHORT VscrnScrollRt, ( BYTE, USHORT, USHORT, USHORT, USHORT, USHORT, viocell ) );
_PROTOTYP( USHORT VscrnScrollUp, ( BYTE, USHORT, USHORT, USHORT, USHORT, USHORT, viocell ) );
_PROTOTYP( USHORT VscrnScrollDn, ( BYTE, USHORT, USHORT, USHORT, USHORT, USHORT, viocell ) );
_PROTOTYP( USHORT VscrnWrtCell, ( BYTE, viocell, vtattrib, USHORT, USHORT ) );
_PROTOTYP( USHORT VscrnWrtCharStrAtt, ( BYTE vmode, PCH CharStr, USHORT Length,
                         USHORT Row, USHORT Column, PBYTE Attr ) ) ;
_PROTOTYP( USHORT VscrnWrtUCS2StrAtt, ( BYTE vmode, PUSHORT UCS2Str, USHORT Length,
                                        USHORT Row, USHORT Column, PBYTE Attr ) ) ;
_PROTOTYP( void   TermScrnUpd, ( void * ) ) ;

_PROTOTYP( videoline * VscrnGetLineFromTop, ( BYTE, SHORT ) ) ;
_PROTOTYP( videoline * VscrnGetLine, ( BYTE, SHORT ) ) ;
_PROTOTYP( USHORT VscrnGetLineVtAttr, ( BYTE, SHORT ) ) ;
_PROTOTYP( USHORT VscrnSetLineVtAttr, ( BYTE, SHORT, USHORT ) ) ;
_PROTOTYP( vtattrib VscrnGetVtCharAttr, ( BYTE, SHORT, SHORT ) ) ;
_PROTOTYP( USHORT VscrnSetVtCharAttr, ( BYTE, SHORT, SHORT, vtattrib ) ) ;
_PROTOTYP( viocell * VscrnGetCells, ( BYTE, SHORT ) ) ;
_PROTOTYP( viocell * VscrnGetCell, ( BYTE, SHORT, SHORT ) ) ;
_PROTOTYP( LONG VscrnMoveTop, ( BYTE, LONG ) ) ;
_PROTOTYP( LONG VscrnMoveScrollTop, ( BYTE, LONG ) ) ;
_PROTOTYP( LONG VscrnMoveBegin, ( BYTE, LONG ) ) ;
_PROTOTYP( LONG VscrnMoveEnd, ( BYTE, LONG ) ) ;
_PROTOTYP( UCHAR VscrnGetLineWidth, ( BYTE, SHORT ) ) ;
_PROTOTYP( ULONG VscrnGetTop, ( BYTE ) ) ;
_PROTOTYP( ULONG VscrnGetScrollTop, ( BYTE ) ) ;
_PROTOTYP( ULONG VscrnGetScrollHorz, ( BYTE ) ) ;
_PROTOTYP( ULONG VscrnGetBegin, ( BYTE ) ) ;
_PROTOTYP( ULONG VscrnGetEnd, ( BYTE ) ) ;
_PROTOTYP( LONG VscrnSetTop, ( BYTE, LONG ) ) ;
_PROTOTYP( LONG VscrnSetScrollTop, ( BYTE, LONG ) ) ;
_PROTOTYP( LONG VscrnSetScrollHorz, ( BYTE, LONG ) ) ;
_PROTOTYP( LONG VscrnSetBegin, ( BYTE, LONG ) ) ;
_PROTOTYP( LONG VscrnSetEnd, ( BYTE, LONG ) ) ;
_PROTOTYP( ULONG VscrnGetBufferSize, ( BYTE ) ) ;
_PROTOTYP( ULONG VscrnSetBufferSize, ( BYTE, ULONG ) ) ;
_PROTOTYP( VOID VscrnSetWidth, ( BYTE, int ) ) ;
_PROTOTYP( VOID VscrnSetHeight, ( BYTE, int ) ) ;
_PROTOTYP( VOID VscrnSetDisplayHeight, ( BYTE, int ) ) ;
_PROTOTYP( int VscrnGetWidth, ( BYTE ) ) ;
_PROTOTYP( int VscrnGetHeight, ( BYTE ) ) ;
_PROTOTYP( int VscrnGetDisplayHeight, ( BYTE ) ) ;
_PROTOTYP( position * VscrnSetCurPos, ( BYTE, SHORT, SHORT ) ) ;
_PROTOTYP( position * VscrnGetCurPos, ( BYTE ) ) ;
_PROTOTYP( VOID VscrnSetBookmark, ( BYTE, int, int ) ) ;
_PROTOTYP( int VscrnGetBookmark, ( BYTE, int ) ) ;


_PROTOTYP( bool IsWARPed, ( void ) ) ;
_PROTOTYP( APIRET VscrnIsDirty, ( int ) ) ;
_PROTOTYP( void VscrnScroll, (BYTE, int, int, int, int,int, CHAR) ) ;
_PROTOTYP( BOOL IsOS2FullScreen, (void) ) ;
_PROTOTYP( void SmoothScroll, (void) ) ;
_PROTOTYP( void JumpScroll, (void ) ) ;


_PROTOTYP( APIRET VscrnSelect, ( BYTE, int ) ) ;
_PROTOTYP( APIRET VscrnURL, ( BYTE, USHORT, USHORT       ) ) ;
_PROTOTYP( APIRET CopyVscrnToKbdBuffer, ( BYTE, int ) ) ;
_PROTOTYP( APIRET CopyVscrnToClipboard, ( BYTE, int ) ) ;
_PROTOTYP( APIRET CopyVscrnToPrinter, ( BYTE, int ) ) ;
_PROTOTYP( APIRET CopyClipboardToKbdBuffer, ( BYTE ) ) ;
_PROTOTYP( BOOL   VscrnIsLineMarked, ( BYTE, LONG ) ) ;
_PROTOTYP( BOOL   VscrnIsMarked, ( BYTE, LONG, SHORT ) ) ;
_PROTOTYP( BOOL   VscrnIsClear, ( BYTE ) ) ;
_PROTOTYP( void   VscrnSetPopup, ( BYTE, videopopup * ) ) ;
_PROTOTYP( void   VscrnResetPopup, ( BYTE ) ) ;
_PROTOTYP( bool   VscrnIsPopup, ( BYTE) ) ;
_PROTOTYP( void   VscrnMark, ( BYTE, LONG, SHORT, SHORT ) ) ;
_PROTOTYP( void   VscrnUnmark, ( BYTE, LONG, SHORT, SHORT ) ) ;
_PROTOTYP( void   VscrnUnmarkAll, ( BYTE ) ) ;
_PROTOTYP( void   shovscrn, ( void ) ) ;
_PROTOTYP( void   markdownone, ( BYTE ) ) ;
_PROTOTYP( void   markupone, ( BYTE ) ) ;
_PROTOTYP( void   markstart, ( BYTE ) ) ;
_PROTOTYP( void   markcancel, ( BYTE ) ) ;
_PROTOTYP( void   markleftone, ( BYTE ) ) ;
_PROTOTYP( void   markrightone, ( BYTE ) ) ;
_PROTOTYP( void   markdownscreen, ( BYTE ) ) ;
_PROTOTYP( void   markupscreen, ( BYTE ) ) ;
_PROTOTYP( void   markleftpage, ( BYTE ) ) ;
_PROTOTYP( void   markrightpage, ( BYTE ) ) ;
_PROTOTYP( void   markhomescreen, ( BYTE ) ) ;
_PROTOTYP( void   markendscreen, ( BYTE ) ) ;

_PROTOTYP( void   markcopyclip, ( BYTE, int ) ) ;
_PROTOTYP( void   markselect, ( BYTE, int ) ) ;
_PROTOTYP( void   markcopyhost, ( BYTE, int ) ) ;
_PROTOTYP( void   markprint, ( BYTE, int ) ) ;

_PROTOTYP( USHORT getshiftstate, ( void ) ) ;

_PROTOTYP(int popuphelp, (int,enum helpscreen));/* Pop-up help panel maker */
_PROTOTYP(int popuperror, (int,char *));        /* Pop-up error message */
_PROTOTYP(int fkeypopup, (int));                /* Pop-up fkey labels */
_PROTOTYP(void ipadl25, (void));                /* Default status-line maker */
_PROTOTYP(char * line25, (int));                /* General-purpose status-line maker */
_PROTOTYP(void xline25, (char *));
_PROTOTYP(void save_status_line, (void));
_PROTOTYP(void restore_status_line, (void));
_PROTOTYP(void setenglishmode, (void));
_PROTOTYP(void setrussianmode, (void));
_PROTOTYP(void RestoreTermMode, (void));
_PROTOTYP(void RestoreCmdMode, (void));
_PROTOTYP(int sendchar, (unsigned char));
_PROTOTYP(int sendchars, (unsigned char *, int));
_PROTOTYP(void sendcharduplex, (unsigned char, int));
_PROTOTYP(void sendcharsduplex, (unsigned char *, int, int));
_PROTOTYP(void sendkeydef, (unsigned char *, int));
_PROTOTYP(void checkscreenmode, (void));
_PROTOTYP(void clearcmdscreen, (void));
_PROTOTYP(void cleartermscreen, (BYTE));
_PROTOTYP(void clearscrollback, (BYTE) ) ;
_PROTOTYP( unsigned char geterasecolor, (int));
_PROTOTYP(void clrtoeoln, (BYTE,CHAR));
_PROTOTYP(void clrbol_escape, (BYTE,CHAR));
_PROTOTYP(void clrbos_escape, (BYTE,CHAR));
_PROTOTYP(void clreoscr_escape, (BYTE,CHAR));
_PROTOTYP(void clrline_escape, (BYTE,CHAR));
_PROTOTYP(void clrcol_escape, (BYTE,CHAR));
_PROTOTYP(void clrrect_escape, (BYTE, int, int, int, int, CHAR)) ;
_PROTOTYP(void selclrtoeoln, (BYTE,CHAR));
_PROTOTYP(void selclrbol_escape, (BYTE,CHAR));
_PROTOTYP(void selclrbos_escape, (BYTE,CHAR));
_PROTOTYP(void selclreoscr_escape, (BYTE,CHAR));
_PROTOTYP(void selclrline_escape, (BYTE,CHAR));
_PROTOTYP(void selclrcol_escape, (BYTE,CHAR));
_PROTOTYP(void selclrrect_escape, (BYTE, int, int, int, int, CHAR)) ;
_PROTOTYP(void cursorleft, (int));
_PROTOTYP(void cursorright, (int));
_PROTOTYP(void cursorup, (int));
_PROTOTYP(void cursordown, (int));
_PROTOTYP(void boxrect_escape, (BYTE, int, int)) ;
_PROTOTYP(void esc25, (int));
_PROTOTYP(void flipscreen, (BYTE));
_PROTOTYP(void killcursor, (BYTE));
_PROTOTYP(void lgotoxy, (BYTE, int, int));
_PROTOTYP(void markmode, ( BYTE, int ) );
_PROTOTYP(void movetoscreen, (char *, int, int, int));
_PROTOTYP(void newcursor, (BYTE));
_PROTOTYP(void printeron, (void));
_PROTOTYP(int  printeropen, (void));
_PROTOTYP(int  printerclose, (void));
_PROTOTYP(void printeroff, (void));
_PROTOTYP(int  is_aprint,(void));
_PROTOTYP(int  is_xprint,(void));
_PROTOTYP(int  is_cprint,(void));
_PROTOTYP(int  is_uprint,(void));
_PROTOTYP(void prtchar, (BYTE));
_PROTOTYP(void prtstr, (char *,int));
_PROTOTYP(void prtscreen, (BYTE,int, int));
#ifdef BPRINT
    _PROTOTYP(int bprtstart, ( void ));
    _PROTOTYP(int bprtstop, ( void ));
    _PROTOTYP(void bprtthread, ( void * ));
    _PROTOTYP(int bprtwrite, ( char *, int ));
#endif /* BPRINT */
_PROTOTYP(void restorecursormode, (void));
_PROTOTYP(void scrninit, (void));
_PROTOTYP(void scrninit2, (void));
_PROTOTYP(void SaveTermMode, (int, int));
_PROTOTYP(void SaveCmdMode, (int, int));
_PROTOTYP(void scrollback, (BYTE,int));
_PROTOTYP(void setcursormode, (void));
_PROTOTYP(void setmargins, (int, int));
_PROTOTYP(void strinsert, (char *, char *));
_PROTOTYP(void wrtch, (unsigned short));
_PROTOTYP(int sendescseq, (char *));
_PROTOTYP(int ckcgetc, (int));
_PROTOTYP(void cwrite, (unsigned short));
_PROTOTYP(int concooked, (void));
_PROTOTYP(int conraw, (void));
_PROTOTYP(int xxesc, (char **));
_PROTOTYP( void updanswerbk, (void) ) ;
_PROTOTYP( void dokverb, (int,int) ) ;
_PROTOTYP( void settermtype, (int,int) );
_PROTOTYP( void settermstatus, (int) ) ;
_PROTOTYP( void debugses, (unsigned char) ) ;
_PROTOTYP( int kbdlocked, (void));

_PROTOTYP( APIRET OpenClipboardServer, (void) ) ;
_PROTOTYP( APIRET CloseClipboardServer, (void) ) ;
_PROTOTYP( PCHAR  GetTextFromClipboardServer, (void) ) ;
_PROTOTYP( BOOL   PutTextToClipboardServer, ( PCHAR ) ) ;

_PROTOTYP( BOOL   IsConnectMode, ( void ) ) ;
_PROTOTYP( void   SetConnectMode, ( BOOL, int ) ) ;
_PROTOTYP( void   getcmdcolor, ( void ) ) ;
_PROTOTYP( unsigned char ComputeColorFromAttr, (int, unsigned char, unsigned short));
_PROTOTYP( void   Win32ConsoleInit, (void));

#ifdef PCFONTS
_PROTOTYP( APIRET os2LoadPCFonts, ( void ) ) ;
_PROTOTYP( APIRET os2SetFont, (void) ) ;
_PROTOTYP( APIRET os2ResetFont, (void) ) ;
#endif /* PCFONTS */

_PROTOTYP( int CSisNRC, ( int ) );
_PROTOTYP( int ltorxlat, ( int, CHAR **));
_PROTOTYP( int rtolxlat, ( int ));
_PROTOTYP( int utolxlat, ( int ));
_PROTOTYP( int utorxlat, ( int, CHAR **));

_PROTOTYP( VOID SNI_bitmode, (int));
_PROTOTYP( VOID SNI_chcode, (int));

_PROTOTYP( const char * GetSelection,(void));
_PROTOTYP( const char * GetURL,(void));
_PROTOTYP( int          GetURLType, (void));
_PROTOTYP( int IsCellPartOfURL,( BYTE mode, USHORT row, USHORT col ));
_PROTOTYP( int IsURLChar, (USHORT));

_PROTOTYP( int os2_settermheight,(int));
_PROTOTYP( int os2_setcmdheight,(int));
_PROTOTYP( int os2_settermwidth,(int));
_PROTOTYP( int os2_setcmdwidth,(int));
#ifdef KUI
_PROTOTYP( int kui_setheightwidth,(int,int));
#endif /* KUI */

typedef struct _hyperlink {
    int index;
    int type;
    char * str;
} hyperlink;

#define HYPERLINK_URL 1
#define HYPERLINK_UNC 2

_PROTOTYP( int hyperlink_add, (int, char *));
_PROTOTYP( hyperlink * hyperlink_get, (int));

#ifdef putchar
#undef putchar
#endif /* putchar */
#define putchar(x) conoc(x)

#ifdef printf
#undef printf
#endif /* printf */
#define printf Vscrnprintf
#ifdef fprintf
#undef fprintf
#endif /* fprintf */
#define fprintf Vscrnfprintf

#ifdef SV_NOTEFREQ
#define DEF_BEEP_FREQ SV_NOTEFREQ
#else
#define DEF_BEEP_FREQ 440
#endif

#ifdef SV_NOTEDURATION
#define DEF_BEEP_TIME SV_NOTEDURATION
#else
#define DEF_BEEP_TIME 100
#endif

#define ATTR_MODE_REAL 1
#define ATTR_MODE_INTENSITY 2
#define ATTR_MODE_COLOR 4

#define ATTR_PAGE_MODE 1
#define ATTR_LINE_MODE 2
#define ATTR_CHAR_MODE 4

_PROTOTYP( void loadtod, ( int, int ) ) ;

#ifndef NT

typedef struct _COORD {
    SHORT X;
    SHORT Y;
} COORD, *PCOORD;

typedef struct _SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
} SMALL_RECT, *PSMALL_RECT;

typedef struct _CHAR_INFO {
    union {
        USHORT UnicodeChar;
        CHAR   AsciiChar;
    } Char;
    USHORT Attributes;
} CHAR_INFO, *PCHAR_INFO;

typedef struct _KEY_EVENT_RECORD {
    ULONG bKeyDown;
    USHORT wRepeatCount;
    USHORT wVirtualKeyCode;
    USHORT wVirtualScanCode;
    union {
        USHORT UnicodeChar;
        CHAR   AsciiChar;
    } uChar;
    ULONG dwControlKeyState;
} KEY_EVENT_RECORD, *PKEY_EVENT_RECORD;

typedef struct _MOUSE_EVENT_RECORD {
    COORD dwMousePosition;
    ULONG dwButtonState;
    ULONG dwControlKeyState;
    ULONG dwEventFlags;
} MOUSE_EVENT_RECORD, *PMOUSE_EVENT_RECORD;

typedef struct _WINDOW_BUFFER_SIZE_RECORD {
    COORD dwSize;
} WINDOW_BUFFER_SIZE_RECORD, *PWINDOW_BUFFER_SIZE_RECORD;

typedef struct _MENU_EVENT_RECORD {
    ULONG dwCommandId;
} MENU_EVENT_RECORD, *PMENU_EVENT_RECORD;

typedef struct _FOCUS_EVENT_RECORD {
    ULONG bSetFocus;
} FOCUS_EVENT_RECORD, *PFOCUS_EVENT_RECORD;

typedef struct _INPUT_RECORD {
    USHORT EventType;
    union {
        KEY_EVENT_RECORD KeyEvent;
        MOUSE_EVENT_RECORD MouseEvent;
        WINDOW_BUFFER_SIZE_RECORD WindowBufferSizeEvent;
        MENU_EVENT_RECORD MenuEvent;
        FOCUS_EVENT_RECORD FocusEvent;
    } Event;
} INPUT_RECORD, *PINPUT_RECORD;

typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize;
    COORD dwCursorPosition;
    USHORT  wAttributes;
    SMALL_RECT srWindow;
    COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO, *PCONSOLE_SCREEN_BUFFER_INFO;
#endif /* NT */

#include "ckothr.h"
#include "ckosyn.h"

#ifdef CKLEARN
extern FILE * learnfp;
extern int learning;
extern ULONG learnt1;
extern char learnbuf[LEARNBUFSIZ];
extern int  learnbc;
extern int  learnbp;
extern int  learnst;

#define LEARN_NEUTRAL  0
#define LEARN_NET      1
#define LEARN_KEYBOARD 2
#endif /* CKLEARN */

extern int ConnectMode;
#ifdef NT
_inline
#else
_Inline
#endif
BOOL
IsConnectMode( void ) {
    extern int apcactive;
    return ConnectMode && !apcactive;
}

int gui_text_popup_create(char * title, int h, int w);
int gui_text_popup_append(unsigned short uch);
int gui_text_popup_close(void);
int gui_text_popup_wait(int seconds);
#endif /* CKOCON_H */
