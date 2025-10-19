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
#ifndef __GNUC__
#define APIRET DWORD
#else
#define APIRET unsigned __LONG32
#endif /* __GNUC__ */
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
#ifdef KUI

#if (defined(_MSC_VER) && _MSC_VER > 1400) || defined(__GNUC__)
/*
 * Don't increase max columns on compilers capable of targeting older versions
 * of windows (VC++ 2005 is the last one capable of building for 9x and NT)
 * as they could potentially be running on more resource constrained machines.
 *
 * This is a bit arbitrary and a bit of a hack - someday we'll allow this
 * maximum to be changed at runtime solving this problem properly.
 */
#define MAXSCRNCOL  512            /* Maximum screen columns */
#define MAXSCRNROW  256            /* Maximum screen rows    */
#else
#define MAXSCRNCOL  256            /* Maximum screen columns */
#define MAXSCRNROW  128            /* Maximum screen rows    */
#endif
#else /* KUI */
#define MAXSCRNCOL  256            /* Maximum screen columns */
#define MAXSCRNROW  128            /* Maximum screen rows    */
#endif /* KUI */
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
 *
 * =============================typedefs======================================
 */
#ifndef __cplusplus
typedef int bool;
#else
#ifndef __BOOL_DEFINED
#ifndef __WATCOMC__
#ifndef __GNUC__
typedef int bool;
#endif
#endif
#endif
#endif

/* ------------------------------------------------------------------------- */
/* Kermit 95 supports being built with one of three levels of colour support
 *   16-colours  - The *only* option prior to K95 3.0 beta.8. Only used for the
 *                 console and OS/2 versions now, but it remains a custom build
 *                 option for K95G. Uses an unsigned char for storage. Uses the
 *                 macro CK_COLORS_16.
 *   256-colours - The default option for K95G. Supports color palettes up to
 *                 256-colors. SGR-38/48 RGB values are mapped on to the current
 *                 color palette. Memory usage is about the same as the 16-color
 *                 version due to structure padding. Uses an unsigned short for
 *                 storage. Uses the macro CK_COLORS_256
 *   24-bit RGB  - The default option for K95G on modern windows where the
 *                 signifciantly higher memory needs of the terminal buffers are
 *                 less likely to be an issue. Uses a struct containing seven
 *                 unsigned chars (FG R/G/B, BG R/G/B, and flags) for storage.
 *                 Uses the macro CK_COLORS_24BIT.
 * The details of how colour information is stored is largely hidden behind a
 * series of increasingly complex and awful macros in this header. In hindsight
 * some of these macros probably should have been functions. Maybe some day they
 * will be.
 *
 * To have *some* level of confidence that these macros all behave as expected,
 * there is a set of unit tests in color_tests.c - if you're making changes to
 * any of the colour macros in here its worth running those tests to make sure
 * everything is still working properly.
 *
 * In all builds, multiple different colour palettes are now supported, which
 * can be chosen with SET TERM COLOR PALETTE. In 16-colour builds, they're only
 * used as a source of RGB values for mapping on to the 16-colour palette, while
 * in 256-colour and 24-bit RGB builds they're used for display.
 */

/* Colour palettes available */
#define CK_PALETTE_8       1   /* Standard ANSI palette - not currently supported */
#define CK_PALETTE_16      2   /* Standard 16-color aixterm */
#define CK_PALETTE_XT88    3   /* xterm 88-color */
#define CK_PALETTE_XT256   4   /* xterm 256-color */
#define CK_PALETTE_XTRGB   5   /* 24-bit RGB + xterm-256 */
#define CK_PALETTE_XTRGB88 6   /* 24-bit RGB + xterm-88 */

#ifdef COMMENT
/* For the Wyse WY-370 colour palette, a bunch of extra work needs to be done:
 *   - Populate the palette with the correct colours (the manual only gives names)
 *   - Select the WY370 palette automatically when choosing the WY370 emulation
 *   - Update the cell_video_attr_*_rgb macros in this header to support the palette
 *   - Don't apply the sgrcols/color_index_to_vio fixup when using the WY370 palette
 *     as its bottom 8 colours aren't ANSI, and its bottom 16 colours aren't AIXTERM
 *   - Update the macros here to treat the WY370 palette like RGB mode when it comes
 *     to the intensity bit (don't try to preserve or set it when using WY370 colours)
 *   - There might be more WY370 control sequences that need to be implemented to
 *     support configuring the palette, etc, or control sequences that aren't fully
 *     implemented right now
 *   - SHO GUI /PAL should probably not show the color cube output as the Wyse
 *     palette doesn't seem to use one
 * Most code should at least be aware of the WY370 palette when CK_PALETTE_WY370
 * is defined, the remaining work is mostly applying the special treatments in the
 * macros here and around calls to color_index_to_vio/sgrcols. It doesn't make
 * much sense to do any of that until the real colour values are known though.
 */
#define CK_PALETTE_WY370 7   /* Wyse WY370 */
#endif

/* The OS/2 console only supports 16 colours */
#ifdef OS2ONLY
#ifdef CK_COLORS_24BIT
#undef CK_COLORS_24BIT
#endif /* CK_COLORS_24BIT */
#ifdef CK_COLORS_256
#undef CK_COLORS_256
#endif /* CK_COLORS_256 */
#endif

/* And the Win32 console version also only supports 16 colours */
#ifndef KUI
#ifdef CK_COLORS_24BIT
#undef CK_COLORS_24BIT
#endif /* CK_COLORS_24BIT */
#ifdef CK_COLORS_256
#undef CK_COLORS_256
#endif /* CK_COLORS_256 */
#endif

int color_index_to_vio(int index);
int nearest_palette_color_rgb(int palette_id, unsigned char r, unsigned char g, unsigned char b);
int nearest_palette_color_palette(int palette_id, int palette_index);
unsigned char current_palette_max_index();
int color_index_to_vio(int index);

#ifdef CK_COLORS_24BIT
/* ***************************** RGB-COLORS *****************************/
/* ***************************** RGB-COLORS *****************************/
/* ***************************** RGB-COLORS *****************************/
#define CK_DEFAULT_PALETTE CK_PALETTE_XTRGB

#define CK_RGB_FLAG_RGB         0
#define CK_RGB_FLAG_FG_INDEXED  1
#define CK_RGB_FLAG_BG_INDEXED  2
#define CK_RGB_FLAG_INDEXED (CK_RGB_FLAG_FG_INDEXED | CK_RGB_FLAG_BG_INDEXED)

/* Foreground R/G/B, Background R/G/B
   Flags:
     0x01 - Foreground is indexed colour (not RGB)
     0x02 - Backgroudn is indexed colour (not RGB)
   Foreground RGB value is stored in: fg_r, fg_g, fg_b
   Foreground indexed value is stored in: fg_b
   Background RGb value is stored in: bg_r, bg_g, bg_b
   Background indexed value is stored in: bg_b
 */
typedef struct struct_cell_video_attr_t {
    unsigned char flags;
    unsigned char fg_r;  /* Foreground red */
	unsigned char fg_g;  /* Foreground green */
	unsigned char fg_b;  /* Foreground blue *or* foreground indexed color */
	unsigned char bg_r;  /* Background red */
	unsigned char bg_g;  /* Background blue */
	unsigned char bg_b;  /* Background blue *or* background indexed color */
} cell_video_attr_t;


/* Checks if the foreground or background colour is indexed rather than RGB */
#define cell_video_attr_fg_is_indexed(att) ((att).flags & CK_RGB_FLAG_FG_INDEXED)
#define cell_video_attr_bg_is_indexed(att) ((att).flags & CK_RGB_FLAG_BG_INDEXED)

/* Checks if *both* the foreground and background colour are indexed rather than
 * RGB */
#define cell_video_attr_is_indexed(att) (cell_video_attr_fg_is_indexed(att) && cell_video_attr_bg_is_indexed(att))

#define cell_video_attr_is_rgb(att) (!cell_video_attr_fg_is_indexed(att) && !cell_video_attr_bg_is_indexed(att))

/** Sets all values in a video attribute. For Visual C++ 2013 and up, we just
 * use a compound literal. For older compilers, a function. */
#if _MSC_VER < 1800
cell_video_attr_t cell_video_attr_set(
    unsigned char flags, unsigned char fg_r, unsigned char fg_g,
    unsigned char fg_b, unsigned char bg_r, unsigned char bg_g, unsigned char bg_b);
#else
#define cell_video_attr_set(flags, fg_r, fg_g, fg_b, bg_r, bg_g, bg_b) ( \
    (cell_video_attr_t) { (flags), (fg_r), (fg_g), (fg_b), (bg_r), (bg_g), (bg_b) })
#endif


/** Sets the foreground and background to the supplied indexed colour values.
 * Used by:
 *     QNX terminal emulation (ckoqnx.c)
 *     CSI = Ps F   (Set Normal Foreground Color, ANSI)
 *     SET COMMAND COLOR
 *     SET TERMINAL COLOR
 */
#define cell_video_attr_set_colors(fg, bg) ( \
    cell_video_attr_set(CK_RGB_FLAG_INDEXED, 0, 0, (fg), 0, 0, (bg)))

/* Unlike the rest of the macros here, these to intentionally operate on
 * VIO 4-bit colour pairs */
#define cell_video_attr8_foreground(value) ((value) & 0x0F)
#define cell_video_attr8_background(value) (((value) & 0xF0) >> 4)

/* Initialise a colour attribute at declaration time with a constant
 * 16-color value pair packed into an unsigned char
 *
 * To do this we've got to split the VIO attribute into separate 4-bit FG an BG
 * components, extend them out to 8 bits, and recombine them into a 16bit
 * color attribute
 */
#define cell_video_attr_init_vio_attribute(value) { \
    CK_RGB_FLAG_INDEXED, \
    0, 0, cell_video_attr8_foreground(value), \
    0, 0, cell_video_attr8_background(value) \
}

/* Set a colour attribute to a 16-colour value pair packed into an unsigned
 * char
 */
#define cell_video_attr_from_vio_attribute(value) ( \
    cell_video_attr_set_colors(cell_video_attr8_foreground((value)), \
                               cell_video_attr8_background((value))))

/* Sets the foreground colour to an indexed value preserving the background colour
 */
#define cell_video_attr_set_fg_color(att, fgc) ( \
    cell_video_attr_set((att).flags | CK_RGB_FLAG_FG_INDEXED, \
                        0, 0, (fgc), \
                        (att).bg_r, (att).bg_g, (att).bg_b) )

/* Sets the background colour to an indexed value preserving the foreground colour
 * Used by CSI = Ps K   (Set Graphic Background Color, ANSI)
 */
#define cell_video_attr_set_bg_color(att, bgc) ( \
    cell_video_attr_set((att).flags | CK_RGB_FLAG_BG_INDEXED, \
    (att).fg_r, (att).fg_g, (att).fg_b, \
    0, 0, (bgc)) )

/* Returns the foreground (by clearing the background)
 *
 * This is used by:
 *    DECRQCRA   - the color indexes (if less than 16) are included in the
 *                 checksum calculation
 *    SGR 39     - Restore default FG color
 *    ANSI Report Color Attributes
 */
#define cell_video_attr_foreground(att) ( \
    cell_video_attr_fg_is_indexed(att) ? (att).fg_b : 0)


/* Returns the background colour
 * The & 0xF0 is probably unnecessary. The only place that did it was ckoco3.c at
 * around line 17698 (case 'M')
 *
 * This is used by:
 *    DECRQCRA   - the color indexes (if less than 16) are included in the
 *                 checksum calculation
 *    SGR 39     - Restore default FG color
 *    ANSI Report Color Attributes
 */
#define cell_video_attr_background(att) ( \
    cell_video_attr_bg_is_indexed(att) ? (att).bg_b : 0)

/* Checks if the foreground or background currently contains an indexed colour
 * compatile with the 16-colour aixterm palette */
#define cell_video_attr_fg_is_16colors(att) ( \
    cell_video_attr_fg_is_indexed(att) && cell_video_attr_foreground(att) < 16)
#define cell_video_attr_bg_is_16colors(att) ( \
    cell_video_attr_bg_is_indexed(att) && cell_video_attr_background(att) < 16)

#define cell_video_attr_fgbg_are_16colors(att) ( \
    cell_video_attr_fg_is_16colors(att) && cell_video_attr_bg_is_16colors(att) )


/**** Helper macros to make swapcolors() readable */

/* Swap the lower 3 bits of the two indexed colours perserving the 4th bit */
#define rgbsc_swap_4b(x) (                                                     \
    cell_video_attr_set((x).flags,                                             \
                         0, 0, ((x).fg_b & 0x08) | ((x).bg_b & 0x07),          \
                         0, 0, ((x).bg_b & 0x08) | ((x).fg_b & 0x07) )    )

#define rgbsc_swap_flags(flags) (                                              \
    /* If both indexed bits are set, nothing to swap */                        \
    ((flags) & CK_RGB_FLAG_INDEXED) == CK_RGB_FLAG_INDEXED ? flags             \
         /* if neither indexed bits are set, nothing to swap */                \
        : ((flags) & CK_RGB_FLAG_INDEXED) == CK_RGB_FLAG_RGB ? flags           \
            /* Else, if FG is indexed, set BG indexed and vice versa */        \
            : (flags) & CK_RGB_FLAG_FG_INDEXED                                 \
                ? ((flags) & ~CK_RGB_FLAG_INDEXED) | CK_RGB_FLAG_BG_INDEXED    \
                : ((flags) & ~CK_RGB_FLAG_INDEXED) | CK_RGB_FLAG_FG_INDEXED    \
)

/* Swap 3 bits of the background with all bits of the foreground preserving
 * nothing of the backgrounds remaining bits
 */
#define rgbsc_swap_4b_bg(x) (                                                  \
    cell_video_attr_set(rgbsc_swap_flags((x).flags),                           \
                            0, 0, (x).bg_b & 0x07,                             \
                        (x).fg_r, (x).fg_g, (x).fg_b)    )

/* Swap 3 bits of the foreground with all bits of the background preserving
 * nothing of the foregrounds remaining bits */
#define rgbsc_swap_4b_fg(x) (                                                  \
    cell_video_attr_set(rgbsc_swap_flags((x).flags),                           \
                        (x).bg_r, (x).bg_g, (x).bg_b,                          \
                        0, 0, (x).fg_b & 0x07) )

/* Swap everything */
#define rgbsc_swap_all(x) (                                                    \
    cell_video_attr_set((x).flags, (x).bg_r, (x).bg_g, (x).bg_b,               \
                        (x).fg_r, (x).fg_g, (x).fg_b)   )


/* The swapcolors macro exchanges the fore- and background colors, *possibly*
 * preserving the 4th bit in the fore- and background.
 *
 * The 4th bit for 4-bit indexed colors can have significant meaning - some
 * emulations display bold as an intense colour (4th bit set). So when swapping
 * the FG and BG colour, we want to leave the 4th bit where it is so that the
 * foreground text remains bold rather than ending up with a "bold" background.
 *
 * Things get more difficult with 256-color & RGB support. If the index is 16 or
 * above then we can't assume the 4th bit holds any special significance. So
 * if both colors are 16 or above they'll be swapped verbatim. If only one of
 * the colours is below 16, that colour will have its 4th bit thrown away and
 * only the bottom 3 bits will be swapped over, while the other larger colour
 * will be sawpped over verbatim.
 *
 * I don't know if this is the *correct* strategy. Perhaps if one colour is
 * above 16 we should assume both are from a larger palette and so swap both
 * over verbatim. Or perhaps we need to track how each individual colour was
 * set so we know *why* the 4th bit is set.
*/
#define swapcolors(x) (                                          \
      cell_video_attr_is_rgb(x)            ? rgbsc_swap_all(x)   \
    : cell_video_attr_fgbg_are_16colors(x) ? rgbsc_swap_4b(x)    \
    : cell_video_attr_bg_is_16colors(x)    ? rgbsc_swap_4b_bg(x) \
    : cell_video_attr_fg_is_16colors(x)    ? rgbsc_swap_4b_fg(x) \
    : /* fg and bg are indexed && > 15 ? */  rgbsc_swap_all(x)   \
)

/* In the original K95 open-source release of the v2.2 code, swapcolors() and
 * byteswapcolors() had equivalent implementations (they did the same thing).
 * I don't know why - perhaps at one point there was some difference in behaviour
 * but if so that was long ago now. */
#define byteswapcolors(x) (swapcolors(x))

/* Returns the colour as a Win32 Console color attribute. There is no equivalent
 * to this for OS/2 - there we just pass entire viocells! Because of this we
 * can't really ever change the structure of viocell, so this will always do
 * effectively nothing. It exists only to make it clear we aren't accidentally
 * passing a 16-colour value off to somewhere else.
 *
 * On 256-color and 24-bit color builds, it is undefined so as to produce a
 * build error anywhere it is used.
 *
 * TODO: In the future on Windows we *could* issue SGR colours; The windows 10 v1809
 *       or higher console does support it. But thats not something that could be done
 *       here with a simple macro.
 */
/*
#define cell_video_attr_to_win32_console(value) (value)
*/

/* This inverts the intensity bit. Its used only in ckctel.c to temporarily
 * invert the foreground intensity for telnet debug messages so that they stand
 * out.
 * TODO: For higher colour depths: ??? Maybe use a VT attribute ???
 */
#define cell_video_attr_with_fg_intensity_toggled(attr) ( \
	cell_video_attr_fg_is_16colors(attr) \
	? cell_video_attr_set_fg_color((attr), cell_video_attr_foreground(attr) ^ 0x0008) \
    : attr )

/* These turn the 4th bit on or off if the current color index is less than 16.
 * Used only in ComputeColorFromAttr() depending on whether the normal
 * foreground/background colour (as set by the SET TERM COLOR NORMAL command)
 * has the intensity bit set. This initial normal foreground/background
 * intensity is stored in the fgi and bgi globals.
 * Its used to simulate bold/dim via intensity for when the dim attribute is off.
 */
#define cell_video_attr_with_bg_intensity_set(attr) ( \
    cell_video_attr_bg_is_16colors(attr) \
    ? cell_video_attr_set_bg_color((attr), cell_video_attr_background(attr) | 0x08) \
    : attr )
#define cell_video_attr_with_bg_intensity_unset(attr) ( \
    cell_video_attr_bg_is_16colors(attr) \
    ? cell_video_attr_set_bg_color((attr), cell_video_attr_background(attr) & 0x07) \
    : attr)
#define cell_video_attr_with_fg_intensity_set(attr)   ( \
    cell_video_attr_fg_is_16colors(attr) \
    ? cell_video_attr_set_fg_color((attr), cell_video_attr_foreground(attr) | 0x08) \
    : attr)
#define cell_video_attr_with_fg_intensity_unset(attr) ( \
    cell_video_attr_fg_is_16colors(attr) \
    ? cell_video_attr_set_fg_color((attr), cell_video_attr_foreground(attr) & 0x07) \
    : attr)

/* Sets the background colour to a 3-bit value preserving the background
 * intensity bit if the current background color index is less than 16
 * Equivalent to: attribute = (attribute & 0x8F) | (l << 4);
 * Or:  i = (attribute & 0x8F)
 *      attribute = i | (l << 4)
 * Used by:
 *   Faking the WY-370 64-color palette
 *   SGR 30-37: Select foreground color
 *   SGR 40-47: Select background color
 * */
#define cell_video_attr_set_3bit_bg_color(attr, bg) ( \
	cell_video_attr_bg_is_16colors(attr) \
	? cell_video_attr_set_bg_color((attr), (cell_video_attr_background(attr) & 0x08) | ((bg) & 0x07) ) \
	: cell_video_attr_set_bg_color(attr, bg) \
)

/* Sets the foreground colour to a 3-bit value preserving the foreground
 * intensity bit if the current foreground color index is less than 16
 * Equivalent to:  attribute = (attribute & 0xF8) | l
 * Or:  i = (attribute & 0xF8);
 *      attribute = (i | l);
 * Used by:
 *   Faking the WY-370 64-color palette
 *   SGR 30-37: Select foreground color
 *   SGR 40-47: Select background color
 */
#define cell_video_attr_set_3bit_fg_color(attr, fg) ( \
	cell_video_attr_fg_is_16colors(attr) \
	? cell_video_attr_set_fg_color((attr), (cell_video_attr_foreground(attr) & 0x08) | ((fg) & 0x07) ) \
	: cell_video_attr_set_fg_color(attr, fg) \
)


/* Set a colour attribute to a pair of 8-colour values packed into a byte. The
 * 4th (intensity) bit for both foreground and background is preserved from the
 * colour attributes existing value.
 * This is equivalent to: attribute = ((attribute & 0x88) | (value))
 * Its used only for SCOANSI SGR 2: Set foreground and background colours while
 * preserving intensity. I *assume* intensity is being preserved for situations
 * where bold is emulated via intensity - in current K95 this only occurs when
 * the dim attribute is off.
 */
#define cell_video_attr_set_vio_3bit_colors(attr, value) ( \
	cell_video_attr_set_3bit_fg_color( \
		cell_video_attr_set_3bit_bg_color(attr, cell_video_attr8_background(value)), \
		cell_video_attr8_foreground(value) \
	) \
)

/* Looks up the 4-bit background or foreground colour in colors[]
 * Its used by "SHOW TERMINAL" to show colour names assigned to various elements
 * and attributes. Its also used by getVar() in ikcmd.c to get colour names,
 * presumably for a future options dialog, but the code paths in there aren't
 * called from anywhere currently
 * TODO: We need names for the xterm colours I guess? The colors[] array is
 *     declared separately in ckuus5.c and ikcmd.c, but perhaps we could have
 *     a function somewhere to return names for values > 15? Would need to come
 *     from an array as multiple colour names may be live simultaneously
 */
#define cell_video_attr_background_color_name(value) ( \
    cell_video_attr_bg_is_16colors(value) ? colors[cell_video_attr_background(value)] : "")
#define cell_video_attr_foreground_color_name(value) ( \
    cell_video_attr_fg_is_16colors(value) ? colors[cell_video_attr_foreground(value)] : "")

#define cell_video_attr_fg_to_rgb(value) ( \
    (unsigned)(((unsigned)(value).fg_b << 16) | \
    (unsigned)((unsigned)(value).fg_g << 8) | \
    (unsigned)(value).fg_r) \
)

#define cell_video_attr_bg_to_rgb(value) ( \
    (unsigned)(((unsigned)(value).bg_b << 16) | \
    (unsigned)((unsigned)(value).bg_g << 8) | \
    (unsigned)(value).bg_r) \
)

#define cell_video_attr_bg_rgb_r(value) ( \
    cell_video_attr_bg_is_indexed(value) ? 0 : (value).bg_r )

#define cell_video_attr_bg_rgb_g(value) ( \
    cell_video_attr_bg_is_indexed(value) ? 0 : (value).bg_g )

#define cell_video_attr_bg_rgb_b(value) ( \
    cell_video_attr_bg_is_indexed(value) ? 0 : (value).bg_b )

#define cell_video_attr_fg_rgb_r(value) ( \
    cell_video_attr_fg_is_indexed(value) ? 0 : (value).fg_r )

#define cell_video_attr_fg_rgb_g(value) ( \
    cell_video_attr_fg_is_indexed(value) ? 0 : (value).fg_g )

#define cell_video_attr_fg_rgb_b(value) ( \
    cell_video_attr_fg_is_indexed(value) ? 0 : (value).fg_b )

/* Returns the background colour as an RGB value
 * Called by the terminal rendering code in kclient.cxx
 * TODO: WY370 palette
 */
#define cell_video_attr_foreground_rgb(value) ( \
    cell_video_attr_fg_is_indexed(value) \
    ? (colorpalette == CK_PALETTE_XT256 || colorpalette == CK_PALETTE_XTRGB) \
	  ? RGBTable256[cell_video_attr_foreground((value))%256] \
	  : ((colorpalette == CK_PALETTE_XT88 || colorpalette == CK_PALETTE_XTRGB88) \
	      ? RGBTable88[cell_video_attr_foreground((value))%88] \
  		  : RGBTable[cell_video_attr_foreground((value))%16]) \
  	: cell_video_attr_fg_to_rgb(value) \
)


/* Returns the background colour as an RGB value
 * Called by the terminal rendering code in kclient.cxx
 * TODO: WY370 palette
 */
#define cell_video_attr_background_rgb(value) ( \
    cell_video_attr_bg_is_indexed(value) \
    ? (colorpalette == CK_PALETTE_XT256 || colorpalette == CK_PALETTE_XTRGB) \
	  ? RGBTable256[cell_video_attr_background((value))%256] \
	  : ((colorpalette == CK_PALETTE_XT88 || colorpalette == CK_PALETTE_XTRGB88) \
	      ? RGBTable88[cell_video_attr_background((value))%88] \
		  : RGBTable[cell_video_attr_background((value))%16]) \
	: cell_video_attr_bg_to_rgb(value) \
)

/* Sets the foreground to an RGB value */
#define cell_video_attr_set_fg_rgb(attr, r, g, b) ( \
    cell_video_attr_set((attr).flags & ~CK_RGB_FLAG_FG_INDEXED, \
        (r), (g), (b), (attr).bg_r, (attr).bg_g, (attr).bg_b) \
)

/* Sets the background to an RGB value */
#define cell_video_attr_set_bg_rgb(attr, r, g, b) ( \
    cell_video_attr_set((attr).flags & ~CK_RGB_FLAG_BG_INDEXED, \
        (attr).fg_r, (attr).fg_g, (attr).fg_b, (r), (g), (b) ) \
)

/* Checks to see if the attribute is currently null (black)
 * Its called in a few places in ckoco3.c to see if a colour was previously
 * saved in savcolor and can be overwritten or restored.
 */
#define cell_video_attr_is_null(attr) ( \
 ( cell_video_attr_fg_is_indexed(attr) ? (attr).fg_b == 0 \
     : ((attr).fg_r == 0 && (attr).fg_g == 0 && (attr).fg_b == 0) \
 ) && ( cell_video_attr_bg_is_indexed(attr) ? (attr).bg_b == 0 \
     : ((attr).bg_r == 0 && (attr).bg_g == 0 && (attr).bg_b == 0)) \
)

/* Checks if two given colour attributes are equal.
 * Its used in WrtCellStrDiff to help determine if two cells are equal,
 * VscrnIsClear to determine if the screen is clear, ComputeColorFromAttr to
 * detemine if a color needs to be computed from the attribute, and the rendering
 * code in KClient
 */
#define cell_video_attr_equal(attr_a, attr_b) ( \
  /* Flags must be the same */ \
  (attr_a).flags == (attr_b).flags && \
  /* If FG is indexed, FG must be the same */ \
  (cell_video_attr_fg_is_indexed(attr_a) && cell_video_attr_fg_is_indexed(attr_b)    \
     ?  cell_video_attr_foreground(attr_a) == cell_video_attr_foreground(attr_b)     \
     :  cell_video_attr_fg_to_rgb(attr_a) == cell_video_attr_fg_to_rgb(attr_b)  ) && \
  (cell_video_attr_bg_is_indexed(attr_a) && cell_video_attr_bg_is_indexed(attr_b)    \
     ?  cell_video_attr_background(attr_a) == cell_video_attr_background(attr_b)     \
     :  cell_video_attr_bg_to_rgb(attr_a) == cell_video_attr_bg_to_rgb(attr_b)  ) \
)

#else /* 256 or 16 colors */
#ifdef CK_COLORS_256
/* ***************************** 256-COLORS *****************************/
/* ***************************** 256-COLORS *****************************/
/* ***************************** 256-COLORS *****************************/
#define CK_DEFAULT_PALETTE CK_PALETTE_XT256

/*  In 256-color mode, we pack the foreground and background color indexes
    into an unsigned short int. The lower four bits of the foreground and
    background are compatible with 16-color mode.

    1                                                              16
    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    | Background color index        | Foreground color index        |
    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 */
typedef unsigned short cell_video_attr_t;


/** Sets the foreground and background to the supplied  indexed colour values.
 * Used by:
 *     QNX terminal emulation (ckoqnx.c)
 *     CSI = Ps F   (Set Normal Foreground Color, ANSI)
 *     SET COMMAND COLOR
 *     SET TERMINAL COLOR
 */
#define cell_video_attr_set_colors(fg, bg) (((bg)<<8 | (fg)))

/* Unlike the rest of the macros here, these to intentionally operate on
 * VIO 4-bit colour pairs */
#define cell_video_attr8_foreground(value) ((value) & 0x0F)
#define cell_video_attr8_background(value) (((value) & 0xF0) >> 4)

/* Initialise a colour attribute at declaration time with a constant
 * 16-color value pair packed into an unsigned char
 *
 * To do this we've got to split the VIO attribute into separate 4-bit FG an BG
 * components, extend them out to 8 bits, and recombine them into a 16bit
 * color attribute
 */
#define cell_video_attr_init_vio_attribute(value) ( \
	cell_video_attr_set_colors( \
		cell_video_attr8_foreground(value), \
		cell_video_attr8_background(value) \
	))

/* Set a colour attribute to a 16-colour value pair packed into an unsigned
 * char
 */
#define cell_video_attr_from_vio_attribute(value) (cell_video_attr_init_vio_attribute(value))

/* Sets the foreground colour to an indexed value preserving the background colour
 */
#define cell_video_attr_set_fg_color(att, fgc) (((att) & 0xFF00) | ((fgc) & 0x00FF))

/* Sets the background colour to an indexed value preserving the foreground colour
 * Used by CSI = Ps K   (Set Graphic Background Color, ANSI)
 */
#define cell_video_attr_set_bg_color(att, bgc) (((att) & 0x00FF) | ((bgc) << 8))

/* Returns the foreground (by clearing the background)
 *
 * This is used by:
 *    DECRQCRA   - the color indexes (if less than 16) are included in the
 *                 checksum calculation
 *    SGR 39     - Restore default FG color
 *    ANSI Report Color Attributes
 */
#define cell_video_attr_foreground(value) ((value) & 0x00FF)

/* Returns the background colour
 * The & 0xF0 is probably unnecessary. The only place that did it was ckoco3.c at
 * around line 17698 (case 'M')
 *
 * This is used by:
 *    DECRQCRA   - the color indexes (if less than 16) are included in the
 *                 checksum calculation
 *    SGR 39     - Restore default FG color
 *    ANSI Report Color Attributes
 */
#define cell_video_attr_background(value) (((value) & 0xFF00) >> 8)

/* The swapcolors macro exchanges the fore- and background colors, *possibly*
 * preserving the 4th bit in the fore- and background.
 *
 * The 4th bit for 4-bit indexed colors can have significant meaning - some
 * emulations display bold as an intense colour (4th bit set). So when swapping
 * the FG and BG colour, we want to leave the 4th bit where it is so that the
 * foreground text remains bold rather than ending up with a "bold" background.
 *
 * Things get more difficult with 256-color support. If the index is 16 or
 * above then we can't assume the 4th bit holds any special significance. So
 * if both colors are 16 or above they'll be swapped verbatim. If only one of
 * the colours is below 16, that colour will have its 4th bit thrown away and
 * only the bottom 3 bits will be swapped over, while the other larger colour
 * will be sawpped over verbatim.
 *
 * I don't know if this is the *correct* strategy. Perhaps if one colour is
 * above 16 we should assume both are from a larger palette and so swap both
 * over verbatim. Or perhaps we need to track how each individual colour was
 * set so we know *why* the 4th bit is set.
*/
#define swap_colors_verbatim(x) (((((x)&0xFF00)>>8)|(((x)&0x00FF)<<8)))
#define swap_colors_3bit(x) (((x)&(unsigned short)0x0808)|(((x)&0x0700)>>8)|(((x)&0x0007)<<8))
#define swapcolors(x) ( \
	(cell_video_attr_background(x) < 16 && cell_video_attr_foreground(x) < 16) \
	/* Both colors under 16, preserve intensity bits of both */ \
	? swap_colors_3bit(x) \
	: (cell_video_attr_background(x) > 15 && cell_video_attr_foreground(x) > 15) \
	     ? swap_colors_verbatim(x) /* Both colors 16 or over - swap verbatim */ \
	     : cell_video_attr_background(x) < 16 \
	         ? ((((x)&0x0700)>>8)|(((x)&0x00FF)<<8)) /* BG < 16, discard BG 4th bit */ \
	         : ((((x)&0xFF00)>>8)|(((x)&0x0007)<<8)) /* FG < 16, discard FG 4th bit */ \
)

/* In the original K95 open-source release of the v2.2 code, swapcolors() and
 * byteswapcolors() had equivalent implementations (they did the same thing).
 * I don't know why - perhaps at one point there was some difference in behaviour
 * but if so that was long ago now. */
#define byteswapcolors(x) (swapcolors(x))

/* Returns the colour as a Win32 Console color attribute. There is no equivalent
 * to this for OS/2 - there we just pass entire viocells! Because of this we
 * can't really ever change the structure of viocell, so this will always do
 * effectively nothing. It exists only to make it clear we aren't accidentally
 * passing a 16-colour value off to somewhere else.
 *
 * On 256-color and 24-bit color builds, it is undefined so as to produce a
 * build error anywhere it is used.
 *
 * TODO: In the future on Windows we *could* issue SGR colours; The windows 10 v1809
 *       or higher console does support >16 colours.
 */
/*
#define cell_video_attr_to_win32_console(value) (value)
*/

/* This inverts the intensity bit. Its used only in ckctel.c to temporarily
 * invert the foreground intensity for telnet debug messages so that they stand
 * out.
 * TODO: For higher colour depths: ??? Maybe use a VT attribute ???
 */
#define cell_video_attr_with_fg_intensity_toggled(attr) ( \
	cell_video_attr_foreground(attr) < 16 ? ((attr) ^ 0x0008) : attr)

/* These turn the 4th bit on or off if the current color index is less than 16.
 * Used only in ComputeColorFromAttr() depending on whether the normal
 * foreground/background colour (as set by the SET TERM COLOR NORMAL command)
 * has the intensity bit set. This initial normal foreground/background
 * intensity is stored in the fgi and bgi globals.
 * Its used to simulate bold/dim via intensity for when the dim attribute is off.
 */
#define cell_video_attr_with_bg_intensity_set(attr)   ( cell_video_attr_background(attr) < 16 ? ((attr) | 0x0800 ) : attr)
#define cell_video_attr_with_bg_intensity_unset(attr) ( cell_video_attr_background(attr) < 16 ? ((attr) & 0x07FF ) : attr)
#define cell_video_attr_with_fg_intensity_set(attr)   ( cell_video_attr_foreground(attr) < 16 ? ((attr) | 0x0008 ) : attr)
#define cell_video_attr_with_fg_intensity_unset(attr) ( cell_video_attr_foreground(attr) < 16 ? ((attr) & 0xFF07 ) : attr)

/* Sets the background colour to a 3-bit value preserving the background
 * intensity bit if the current background color index is less than 16
 * Equivalent to: attribute = (attribute & 0x8F) | (l << 4);
 * Or:  i = (attribute & 0x8F)
 *      attribute = i | (l << 4)
 *
 * Used by:
 *   Faking the WY-370 64-color palette
 *   SGR 30-37: Select foreground color
 *   SGR 40-47: Select background color
 * */
#define cell_video_attr_set_3bit_bg_color(attr, bg) ( \
	cell_video_attr_background(attr) < 16 ? ((attr) & 0x08FF) | (((bg) & 0x07) << 8) \
	: cell_video_attr_set_bg_color(attr, bg) \
)

/* Sets the foreground colour to a 3-bit value preserving the foreground
 * intensity bit if the current foreground color index is less than 16
 * Equivalent to:  attribute = (attribute & 0xF8) | l
 * Or:  i = (attribute & 0xF8);
 *      attribute = (i | l);
 *
 * Used by:
 *   Faking the WY-370 64-color palette
 *   SGR 30-37: Select foreground color
 *   SGR 40-47: Select background color
 */
#define cell_video_attr_set_3bit_fg_color(attr, fg) ( \
	cell_video_attr_foreground(attr) < 16 ? (((attr) & 0xFF08) | ((fg) & 0x07)) \
	: cell_video_attr_set_fg_color(attr, fg) \
)


/* Set a colour attribute to a pair of 8-colour values packed into a byte. The
 * 4th (intensity) bit for both foreground and background is preserved from the
 * colour attributes existing value.
 * This is equivalent to: attribute = ((attribute & 0x88) | (value))
 * Its used only for SCOANSI SGR 2: Set foreground and background colours while
 * preserving intensity. I *assume* intensity is being preserved for situations
 * where bold is emulated via intensity - in current K95 this only occurs when
 * the dim attribute is off.
 */
#define cell_video_attr_set_vio_3bit_colors(attr, value) ( \
	cell_video_attr_set_3bit_fg_color( \
		cell_video_attr_set_3bit_bg_color(attr, cell_video_attr8_background(value)), \
		cell_video_attr8_foreground(value) \
	) \
)

/* Looks up the 4-bit background or foreground colour in colors[]
 * Its used by "SHOW TERMINAL" to show colour names assigned to various elements
 * and attributes. Its also used by getVar() in ikcmd.c to get colour names,
 * presumably for a future options dialog, but the code paths in there aren't
 * called from anywhere currently
 * TODO: We need names for the xterm colours I guess? The colors[] array is
 *     declared separately in ckuus5.c and ikcmd.c, but perhaps we could have
 *     a function somewhere to return names for values > 15? Would need to come
 *     from an array as multiple colour names may be live simultaneously
 */
#define cell_video_attr_background_color_name(value) ( \
    cell_video_attr_background(value) < 15 ? colors[cell_video_attr_background(value)] : "")
#define cell_video_attr_foreground_color_name(value) ( \
    cell_video_attr_foreground(value) < 15 ? colors[cell_video_attr_foreground(value)] : "")


/* Returns the background colour as an RGB value
 * Called by the terminal rendering code in kclient.cxx
 * TODO: WY370 palette
 */
#define cell_video_attr_foreground_rgb(value) ( \
    (colorpalette == CK_PALETTE_XT256 || colorpalette == CK_PALETTE_XTRGB) \
	? RGBTable256[cell_video_attr_foreground((value))%256] \
	: ((colorpalette == CK_PALETTE_XT88 || colorpalette == CK_PALETTE_XTRGB88) \
	    ? RGBTable88[cell_video_attr_foreground((value))%88] \
		: RGBTable[cell_video_attr_foreground((value))%16]) \
)


/* Returns the background colour as an RGB value
 * Called by the terminal rendering code in kclient.cxx
 * TODO: WY370 palette
 */
#define cell_video_attr_background_rgb(value) ( \
    (colorpalette == CK_PALETTE_XT256 || colorpalette == CK_PALETTE_XTRGB) \
	? RGBTable256[cell_video_attr_background((value))%256] \
	: ((colorpalette == CK_PALETTE_XT88 || colorpalette == CK_PALETTE_XTRGB88) \
	    ? RGBTable88[cell_video_attr_background((value))%88] \
		: RGBTable[cell_video_attr_background((value))%16]) \
)


/* Checks to see if the attribute is currently null (black)
 * Its called in a few places in ckoco3.c to see if a colour was previously
 * saved in savcolor and can be overwritten or restored.
 */
#define cell_video_attr_is_null(attr) (attr == 0)

/* Checks if two given colour attributes are equal.
 * Its used in WrtCellStrDiff to help determine if two cells are equal,
 * VscrnIsClear to determine if the screen is clear, ComputeColorFromAttr to
 * detemine if a color needs to be computed from the attribute, and the rendering
 * code in KClient  */
#define cell_video_attr_equal(attr_a, attr_b) ((attr_a) == (attr_b))


#else /* 16 colours */
#define CK_COLORS_16

/* For NT only - on OS/2 we can't change the data type of the video attribute
 *  at all */
/*#define CK_COLORS_DEBUG*/

#ifndef CK_COLORS_DEBUG
/* ***************************** 16-COLORS *****************************/
/* ***************************** 16-COLORS *****************************/
/* ***************************** 16-COLORS *****************************/
#define CK_DEFAULT_PALETTE CK_PALETTE_16

typedef unsigned char cell_video_attr_t;  /* this really should be color_attr_t */

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

/* Initialise a colour attribute at declaration time with a constant
 * 16-color value pair packed into an unsigned char
 */
#define cell_video_attr_init_vio_attribute(value) (value)

/* Set a colour attribute to a 16-colour value pair packed into an unsigned
 * char
 */
#define cell_video_attr_from_vio_attribute(value) (value)

/* Sets the foreground colour to a 4-bit value preserving the background colour
 */
#define cell_video_attr_set_fg_color(attr, fg) (((attr) & 0xF0) | ((fg) & 0x0F))

/* Sets the background colour to a 4-bit value preserving the foreground colour
 * Used by CSI = Ps K   (Set Graphic Background Color, ANSI)
 */
#define cell_video_attr_set_bg_color(attr, bg) (((attr) & 0x0F) | ((bg) << 4))

/** Sets the foreground and background the he supplied 16-colour values.
 * Used by:
 *     QNX terminal emulation (ckoqnx.c)
 *     CSI = Ps F   (Set Normal Foreground Color, ANSI)
 *     SET COMMAND COLOR
 *     SET TERMINAL COLOR
 */
#define cell_video_attr_set_colors(fg, bg) (((bg)<<4 | (fg)))

/* Returns the colour as a Win32 Console color attribute. There is no equivalent
 * to this for OS/2 - there we just pass entire viocells! Because of this we
 * can't really ever change the structure of viocell, so this will always do
 * effectively nothing. It exists only to make it clear we aren't accidentally
 * passing a 16-colour value off to somewhere else.
 */
#define cell_video_attr_to_win32_console(value) (value)

/* This inverts the intensity bit. Its used only in ckctel.c to temporarily
 * invert the foreground intensity for telnet debug messages so that they stand
 * out.
 */
#define cell_video_attr_with_fg_intensity_toggled(attr) (cell_video_attr_from_vio_attribute((attr) ^ 0x8))

/* These turn the 4th bit on or off. Used only in ComputeColorFromAttr()
 * depending on whether the normal foreground/background colour (as set by
 * the SET TERM COLOR NORMAL command) has the intensity bit set. This initial
 * normal foreground/background intensity is stored in the fgi and bgi globals.
 * Its used to simulate bold/dim via intensity for when the dim attribute is off.
 */
#define cell_video_attr_with_bg_intensity_set(attr) (cell_video_attr_from_vio_attribute((attr) | 0x80 ))
#define cell_video_attr_with_bg_intensity_unset(attr) (cell_video_attr_from_vio_attribute((attr) & 0x7F ))
#define cell_video_attr_with_fg_intensity_set(attr) (cell_video_attr_from_vio_attribute((attr) | 0x08 ))
#define cell_video_attr_with_fg_intensity_unset(attr) (cell_video_attr_from_vio_attribute((attr) & 0xF7 ))

/* Set a colour attribute to a pair of 8-colour values packed into a byte. The
 * 4th (intensity) bit for both foreground and background is preserved from the
 * colour attributes existing value.
 * This is equivalent to: attribute = ((attribute & 0x88) | (value))
 * Its used only for SCOANSI SGR 2: Set foreground and background colours while
 * preserving intensity. I *assume* intensity is being preserved for situations
 * where bold is emulated via intensity - in current K95 this only occurs when
 * the dim attribute is off.
 */
#define cell_video_attr_set_vio_3bit_colors(attr, value) ((((attr) & 0x88) | (value)))

/* Sets the background colour to a 3-bit value preserving the background
 * intensity bit and the foreground colour
 * Equivalent to: attribute = (attribute & 0x8F) | (l << 4);
 * Or:  i = (attribute & 0x8F)
 *      attribute = i | (l << 4)
 *
 * Used by:
 *   Faking the WY-370 64-color palette
 *   SGR 30-37: Select foreground color
 *   SGR 40-47: Select background color
 * */
#define cell_video_attr_set_3bit_bg_color(attr, bg) (((attr) & 0x8F) | (((bg) & 0x07) << 4))

/* Sets the foreground colour to a 3-bit value preserving the foreground
 * intensity bit and the background colour
 * Equivalent to:  attribute = (attribute & 0xF8) | l
 * Or:  i = (attribute & 0xF8);
 *      attribute = (i | l);
 *
 * Used by:
 *   Faking the WY-370 64-color palette
 *   SGR 30-37: Select foreground color
 *   SGR 40-47: Select background color
 */
#define cell_video_attr_set_3bit_fg_color(attr, fg) ((((attr) & 0xF8) | ((fg) & 0x07)))

/* Returns the foreground (by clearing the background)
 *
 * This is used by:
 *    DECRQCRA   - the color indexes (if less than 16) are included in the
 *                 checksum calculation
 *    SGR 39     - Restore default FG color
 *    ANSI Report Color Attributes
 */
#define cell_video_attr_foreground(value) ((value) & 0x0F)

/* Returns the background colour
 * The & 0xF0 is probably unnecessary. The only place that did it was ckoco3.c at
 * around line 17698 (case 'M')
 *
 * This is used by:
 *    DECRQCRA   - the color indexes (if less than 16) are included in the
 *                 checksum calculation
 *    SGR 39     - Restore default FG color
 *    ANSI Report Color Attributes
 */
#define cell_video_attr_background(value) (((value) & 0xF0) >> 4)

/* Looks up the 4-bit background or foreground colour in colors[]
 * Its used by "SHOW TERMINAL" to show colour names assigned to various elements
 * and attributes. Its also used by getVar() in ikcmd.c to get colour names,
 * presumably for a future options dialog, but the code paths in there aren't
 * called from anywhere currently
 */
#define cell_video_attr_background_color_name(value) (colors[cell_video_attr_background(value)])
#define cell_video_attr_foreground_color_name(value) (colors[cell_video_attr_foreground(value)])


/* Returns the background colour as an RGB value
 * Called by the terminal rendering code in kclient.cxx
 */
#define cell_video_attr_foreground_rgb(value) (RGBTable256[cell_video_attr_foreground((value))])


/* Returns the background colour as an RGB value
 * Called by the terminal rendering code in kclient.cxx
 */
#define cell_video_attr_background_rgb(value) (RGBTable256[cell_video_attr_background((value))])


/* Checks to see if the attribute is currently null (black)
 * Its called in a few places in ckoco3.c to see if a colour was previously
 * saved in savcolor and can be overwritten or restored.
 */
#define cell_video_attr_is_null(attr) (attr == 0)

/* Checks if two given colour attributes are equal.
 * Its used in WrtCellStrDiff to help determine if two cells are equal,
 * VscrnIsClear to determine if the screen is clear, ComputeColorFromAttr to
 * detemine if a color needs to be computed from the attribute, and the rendering
 * code in KClient  */
#define cell_video_attr_equal(attr_a, attr_b) ((attr_a) == (attr_b))

#else /* CK_COLORS_DEBUG */

/* ***************************** DEBUG 16-COLORS *****************************/
/* ***************************** DEBUG 16-COLORS *****************************/
/* ***************************** DEBUG 16-COLORS *****************************/

/* This is a variation of 16-color support where the colour attribute is
 * intentionally incompatible with an integer. The purpose is to catch any
 * situations where the colour attribute is being treated as an integer,
 * operations which are likely to break if the colour attributes type changes
 * to, for example, support more than 16 colours. It was used to identify all
 * code that needed to be updated as part of the work to add support for xterms
 * 256-color and RGB modes.
 */

typedef struct {
    unsigned char a;
    int b; /* This is just junk to make compile errors */
} cell_video_attr_t;

/** Sets all values in a video attribute. For Visual C++ 2013 and up, we just
 * use a compound literal. For older compilers, a function. */
#if _MSC_VER < 1800
cell_video_attr_t cell_video_attr_set(unsigned char value);
#else
#define cell_video_attr_set(value) ((cell_video_attr_t){value,0})
#endif

#define cell_video_attr_init_vio_attribute(value) {value, 0}
#define cell_video_attr_from_vio_attribute(value) (cell_video_attr_set(value))
#define cell_video_attr_set_vio_3bit_colors(attr, value) (cell_video_attr_set((((attr).a & 0x88) | (value))))
#define cell_video_attr_to_win32_console(value) ((value).a)
#define cell_video_attr_with_fg_intensity_toggled(attr) (cell_video_attr_from_vio_attribute((attr).a ^ 0x8))
#define cell_video_attr_with_bg_intensity_set(attr) (cell_video_attr_from_vio_attribute((attr).a | 0x80 ))
#define cell_video_attr_with_bg_intensity_unset(attr) (cell_video_attr_from_vio_attribute((attr).a & 0x7F ))
#define cell_video_attr_with_fg_intensity_set(attr) (cell_video_attr_from_vio_attribute((attr).a | 0x08 ))
#define cell_video_attr_with_fg_intensity_unset(attr) (cell_video_attr_from_vio_attribute((attr).a & 0xF7 ))
#define cell_video_attr_foreground(value) ((value).a & 0x0F)
#define cell_video_attr_background(value) (((value).a & 0xF0) >> 4)
#define cell_video_attr_background_color_name(value) (colors[cell_video_attr_background(value)])
#define cell_video_attr_foreground_color_name(value) (colors[cell_video_attr_foreground(value)])
#define cell_video_attr_background_rgb(value) (RGBTable256[cell_video_attr_background((value))])
#define cell_video_attr_foreground_rgb(value) (RGBTable256[cell_video_attr_foreground((value))])
#define cell_video_attr_set_colors(fg, bg) (cell_video_attr_set(((bg)<<4 | (fg))))
#define cell_video_attr_set_fg_color(attr, fg) (cell_video_attr_set((((attr).a & 0xF0) | ((fg) & 0x0F))))
#define cell_video_attr_set_3bit_fg_color(attr, fg) (cell_video_attr_set((((attr).a & 0xF8) | ((fg) & 0x07) )))
#define cell_video_attr_set_bg_color(attr, bg) (cell_video_attr_set((((attr).a & 0x0F) | ((bg) << 4))))
#define cell_video_attr_set_3bit_bg_color(attr, bg) (cell_video_attr_set(((attr).a & 0x8F) | (((bg) & 0x07) << 4)))
#define cell_video_attr_is_null(attr) ((attr).a == 0)
#define cell_video_attr_equal(attr_a, attr_b) ((attr_a).a == (attr_b).a)
#define swapcolors(x) cell_video_attr_from_vio_attribute((((x.a)&(unsigned)0x88)|(((x.a)&0x70)>>4)|(((x.a)&0x07)<<4)))
#define byteswapcolors(x) cell_video_attr_from_vio_attribute(((((x.a)&0x70)>>4)|(((x.a)&0x07)<<4)|((x.a)&(unsigned char)0x88)))
#endif /* CK_COLORS_DEBUG */
#endif /* CK_COLORS_256 */
#endif /* CK_COLORS_24BIT */

/* Sets the foreground to the nearest color palette entry to the supplied RGB value */
#ifndef cell_video_attr_set_fg_rgb
#define cell_video_attr_set_fg_rgb(attr, r, g, b) ( \
	cell_video_attr_set_fg_color((attr), nearest_palette_color_rgb(colorpalette, (r), (g), (b))) \
)
#endif /* cell_video_attr_set_fg_rgb */

/* Sets the background to the nearest color palette entry to the supplied RGB value */
#ifndef cell_video_attr_set_bg_rgb
#define cell_video_attr_set_bg_rgb(attr, r, g, b) ( \
	cell_video_attr_set_bg_color((attr), nearest_palette_color_rgb(colorpalette, (r), (g), (b))) \
)
#endif /* cell_video_attr_set_bg_rgb */

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
    cell_video_attr_t   cell_att;
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
	unsigned crossedout:1;		/* Crossed out */
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
#define VT_CHAR_ATTR_CROSSEDOUT      ((USHORT) 0x2000)
/* These three are available for use */
#define VT_CHAR_RESERVED_2            ((USHORT) 0x4000)  /* Doubly-underlined */
#define VT_CHAR_RESERVED_1            ((USHORT) 0x8000)

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

/* On OS/2, this struct must be a pair of unsigned chars - the first a
 * character, the second its attributes. */
typedef struct cell_struct {          /* to be used with VioWrtNCell() */
#ifdef NT
    unsigned short c ;                /* unicode character */
#else /* CKOUNI_OUT */
    unsigned char c ;                 /* character */
#endif /* CKOUNI_OUT */
    cell_video_attr_t video_attr;      /* attribute */
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
    unsigned short x, y, p ;  /* p for page */
} position ;

typedef struct popup_struct {
    unsigned short height,
                   width ;                     /* before centering */
    cell_video_attr_t  video_attr ;            /* attribute */
    unsigned char  c[MAXTERMROW][MAXTERMCOL] ; /* character */
    unsigned char  gui;                        /* gui? */
    } videopopup ;

typedef struct vscrn_page_struct {
    unsigned long linecount ;
    videoline *   lines ;      /* of count linecount */
    unsigned long beg,         /* beginning of scrollable region */
                  top,         /* first line of write to terminal screen */
                  scrolltop,   /* top of scroll screen */
                  end ;        /* end of scrollable region */
	int margintop, marginbot, marginleft, marginright;
} vscrn_page_t ;

typedef struct vscrn_struct {
    int           view_page,   /* Which page is on screen */
                  page_count;
    vscrn_page_t* pages;       /* of count page_count */
    position      cursor ;     /* cursor position */
    videopopup *  popup ;      /* popup menu      */
    long          marktop,     /* first line marked */
                  markbot ;    /* last line marked  */
    int           width,       /* width of virtual screen */
                  height ;     /* height of virtual screen */
    int           bookmark[10];/* bookmarks */
    int           hscroll ;    /* column to display from */
    int           display_height;
    BOOL          page_cursor_coupling;
    BOOL          allow_alt_buf; /* Allow switching to alternate buffer? */
} vscrn_t;

/* Multiple Page support
 * ---------------------
 * There are at all times *two* current pages which may or may not be the same
 * page. There is the page that the cursor is on (cursor.p), and the page that
 * is currently on screen (view_page).
 *
 * Code that is rendering the vscrn to the real screen should use the view page.
 * Code that is modifying whatever is on screen should use the cursor page.
 *
 * User-initiated actions (eg, mark mode) happen on the view screen.
 *
 * None of this applies to the status line which, while it is a vscrn, only has
 * a single page.
 *
 * Paging is *currently* only supported for VT420 and newer, plus the
 * K95 terminal type. For all other terminals, the view and cursor page are
 * assumed to always be page 1.
 *
 * The WY370 appears to support DEC-compatible paging, so it *could* be extended
 * to that terminal type at some point in the future. Any changes to WY370
 * emulation present a testing challenge however: I don't have access to one.
 */

/* Gets the vscrn page currently being viewed */
#define vscrn_view_page(m) (vscrn[(m)].pages[vscrn[(m)].view_page])

/* Gets the vscrn page that the cursor is currently on */
#define vscrn_cursor_page(m) (vscrn[(m)].pages[vscrn[(m)].cursor.p])

#define vscrn_current_page_number(m, view) ( \
	view ? vscrn[(m)].view_page : vscrn[(m)].cursor.p )

/* Checks if the view page has been initialised */
#define vscrn_view_page_valid(m) (\
    vscrn[(m)].pages != NULL &&  vscrn_view_page(m).lines != NULL )

/* Checks if the cursor page has been initialised */
#define vscrn_cursor_page_valid(m) (\
    vscrn[(m)].pages != NULL && vscrn_cursor_page(m).lines != NULL )

/* Checks if the specified page has been initialised */
#define vscrn_page_valid(m,p) (\
    vscrn[(m)].pages != NULL && p < vscrn[(m)].page_count && vscrn[(m)].pages[(p)].lines != NULL )

/* Checks if the cursor is currently on the visible page */
#define cursor_on_visible_page(m) (vscrn[(m)].view_page == vscrn[(m)].cursor.p)

#define vscrn_set_page_margin_top(m,p,v) (vscrn[(m)].pages[(p)].margintop = (v))
#define vscrn_set_page_margin_bot(m,p,v) (vscrn[(m)].pages[(p)].marginbot = (v))
#define vscrn_set_page_margin_left(m,p,v) (vscrn[(m)].pages[(p)].marginleft = (v))
#define vscrn_set_page_margin_right(m,p,v) (vscrn[(m)].pages[(p)].marginright = (v))

#define vscrn_setc_page_margin_top(m,v) (vscrn_set_page_margin_top((m),vscrn[(m)].cursor.p,(v)))
#define vscrn_setc_page_margin_bot(m,v) (vscrn_set_page_margin_bot((m),vscrn[(m)].cursor.p,(v)))
#define vscrn_setc_page_margin_left(m,v) (vscrn_set_page_margin_left((m),vscrn[(m)].cursor.p,(v)))
#define vscrn_setc_page_margin_right(m,v) (vscrn_set_page_margin_right((m),vscrn[(m)].cursor.p,(v)))

#define vscrn_page_margin_top(m,p) (vscrn[(m)].pages[(p)].margintop)
#define vscrn_page_margin_bot(m,p) (vscrn[(m)].pages[(p)].marginbot)
#define vscrn_page_margin_left(m,p) (vscrn[(m)].pages[(p)].marginleft)
#define vscrn_page_margin_right(m,p) (vscrn[(m)].pages[(p)].marginright)

#define vscrn_c_page_margin_top(m) (vscrn_page_margin_top((m),vscrn[(m)].cursor.p))
#define vscrn_c_page_margin_bot(m) (vscrn_page_margin_bot((m),vscrn[(m)].cursor.p))
#define vscrn_c_page_margin_left(m) (vscrn_page_margin_left((m),vscrn[(m)].cursor.p))
#define vscrn_c_page_margin_right(m) (vscrn_page_margin_right((m),vscrn[(m)].cursor.p))

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
#define ES_BADLINUXOSC 8                /* Broken linux OSC */

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

/* Clipboard Access settings */
#define CLIPBOARD_ALLOW 1
#define CLIPBOARD_ALLOW_NOTIFY 2
#define CLIPBOARD_DENY 0
#define CLIPBOARD_DENY_NOTIFY -1

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
_PROTOTYP( int  vt_macro_in, (void) );
_PROTOTYP( void savescreen, (ascreen *,int,int) ) ;
_PROTOTYP( int restorescreen, (ascreen *) ) ;
_PROTOTYP( void reverserange, (SHORT, SHORT, SHORT, SHORT) ) ;
_PROTOTYP( USHORT ReadCellStr, ( viocell *, PUSHORT, USHORT, USHORT ) );
_PROTOTYP( USHORT WrtCellStr, ( viocell *, USHORT, USHORT, USHORT ) );
_PROTOTYP( USHORT ReadCharStr, ( viocell *, PUSHORT, USHORT, USHORT ) );
_PROTOTYP( USHORT WrtCharStrAtt, ( PCH, USHORT, USHORT, USHORT, cell_video_attr_t* ) );
#ifndef KUI
_PROTOTYP( USHORT WrtNCell, ( viocell, USHORT, USHORT, USHORT ) );
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
                         USHORT Row, USHORT Column, cell_video_attr_t* Attr ) ) ;
_PROTOTYP( USHORT VscrnWrtUCS2StrAtt, ( BYTE vmode, PUSHORT UCS2Str, USHORT Length,
                                        USHORT Row, USHORT Column, cell_video_attr_t* Attr ) ) ;
#ifndef KUI
_PROTOTYP( void   TermScrnUpd, ( void * ) ) ;
#endif /* KUI */
_PROTOTYP( videoline * VscrnGetPageLineFromTop, ( BYTE, SHORT, int ) ) ;
_PROTOTYP( videoline * VscrnGetLineFromTop, ( BYTE, SHORT, BOOL ) ) ;
_PROTOTYP( videoline * VscrnGetLine, ( BYTE, SHORT ) ) ;
_PROTOTYP( USHORT VscrnGetLineVtAttr, ( BYTE, SHORT ) ) ;
/*_PROTOTYP( USHORT VscrnSetLineVtAttr, ( BYTE, SHORT, USHORT ) ) ;*/
_PROTOTYP( vtattrib VscrnGetVtCharAttr, ( BYTE, SHORT, SHORT ) ) ;
_PROTOTYP( USHORT VscrnSetVtCharAttr, ( BYTE, SHORT, SHORT, vtattrib ) ) ;
_PROTOTYP( viocell * VscrnGetCells, ( BYTE, SHORT, INT ) ) ;
_PROTOTYP( viocell * VscrnGetCellEx, ( BYTE, SHORT, SHORT, int ) ) ;
_PROTOTYP( viocell * VscrnGetCell, ( BYTE, SHORT, SHORT, BOOL ) ) ;
_PROTOTYP( LONG VscrnMoveTop, ( BYTE, LONG ) ) ;
_PROTOTYP( LONG VscrnMoveScrollTop, ( BYTE, LONG ) ) ;
/*_PROTOTYP( LONG VscrnMoveBegin, ( BYTE, LONG ) ) ;
_PROTOTYP( LONG VscrnMoveEnd, ( BYTE, LONG ) ) ;*/
_PROTOTYP( UCHAR VscrnGetLineWidth, ( BYTE, SHORT, BOOL ) ) ;
_PROTOTYP( ULONG VscrnGetPageTop, ( BYTE, BOOL, int ) ) ;
_PROTOTYP( ULONG VscrnGetTop, ( BYTE, BOOL, BOOL ) ) ;
_PROTOTYP( ULONG VscrnGetPageScrollTop, ( BYTE, BOOL, int ) ) ;
_PROTOTYP( ULONG VscrnGetScrollTop, ( BYTE, BOOL ) ) ;
_PROTOTYP( ULONG VscrnGetScrollHorz, ( BYTE ) ) ;
_PROTOTYP( ULONG VscrnGetBegin, ( BYTE, BOOL, BOOL ) ) ;
_PROTOTYP( ULONG VscrnGetPageEnd, ( BYTE, BOOL, int ) ) ;
_PROTOTYP( ULONG VscrnGetEnd, ( BYTE, BOOL, BOOL ) ) ;
_PROTOTYP( LONG VscrnSetPageTop, ( BYTE, LONG, BOOL, int ) ) ;
_PROTOTYP( LONG VscrnSetTop, ( BYTE, LONG, BOOL, BOOL ) ) ;
_PROTOTYP( LONG VscrnSetScrollTop, ( BYTE, LONG ) ) ;
/*_PROTOTYP( LONG VscrnSetScrollHorz, ( BYTE, LONG ) ) ;*/
_PROTOTYP( LONG VscrnSetPageBegin, ( BYTE, LONG, int ) ) ;
_PROTOTYP( LONG VscrnSetBegin, ( BYTE, LONG, BOOL ) ) ;
_PROTOTYP( LONG VscrnSetPageEnd, ( BYTE, LONG, int ) ) ;
_PROTOTYP( LONG VscrnSetEnd, ( BYTE, LONG, BOOL ) ) ;
_PROTOTYP( ULONG VscrnGetPageBufferSize, ( BYTE, BOOL, int ) ) ;
_PROTOTYP( ULONG VscrnGetBufferSize, ( BYTE, BOOL, BOOL ) ) ;
_PROTOTYP( VOID VscrnSetWidth, ( BYTE, int ) ) ;
_PROTOTYP( VOID VscrnSetHeight, ( BYTE, int ) ) ;
_PROTOTYP( VOID VscrnSetDisplayHeight, ( BYTE, int ) ) ;
_PROTOTYP( int VscrnGetWidth, ( BYTE ) ) ;
_PROTOTYP( int VscrnGetHeightEx, ( BYTE, BOOL ) ) ;
_PROTOTYP( int VscrnGetHeight, ( BYTE ) ) ;
_PROTOTYP( int VscrnGetDisplayHeight, ( BYTE ) ) ;
_PROTOTYP( position * VscrnSetCurPosEx, ( BYTE, SHORT, SHORT, BOOL ) ) ;
_PROTOTYP( position * VscrnSetCurPos, ( BYTE, SHORT, SHORT ) ) ;
_PROTOTYP( position * VscrnGetCurPosEx, ( BYTE, BOOL ) ) ;
_PROTOTYP( position * VscrnGetCurPos, ( BYTE ) ) ;
_PROTOTYP( VOID VscrnSetBookmark, ( BYTE, int, int ) ) ;
_PROTOTYP( int VscrnGetBookmark, ( BYTE, int ) ) ;


_PROTOTYP( bool IsWARPed, ( void ) ) ;
_PROTOTYP( APIRET VscrnIsDirty, ( int ) ) ;
_PROTOTYP( void VscrnScroll, (BYTE, int, int, int, int,int, CHAR, BOOL) ) ;
_PROTOTYP( void VscrnScrollPage, (BYTE, int, int, int, int, int, CHAR, int) ) ;
_PROTOTYP( BOOL IsOS2FullScreen, (void) ) ;
_PROTOTYP( void SmoothScroll, (void) ) ;
_PROTOTYP( void JumpScroll, (void ) ) ;


_PROTOTYP( APIRET VscrnSelect, ( BYTE, int ) ) ;
_PROTOTYP( APIRET VscrnURL, ( BYTE, USHORT, USHORT       ) ) ;
_PROTOTYP( APIRET CopyVscrnToKbdBuffer, ( BYTE, int ) ) ;
_PROTOTYP( APIRET CopyToClipboard, ( BYTE* data, ULONG length ) ) ;
_PROTOTYP( APIRET CopyVscrnToClipboard, ( BYTE, int ) ) ;
_PROTOTYP( APIRET CopyVscrnToPrinter, ( BYTE, int ) ) ;
_PROTOTYP( APIRET CopyClipboardToKbdBuffer, ( BYTE ) ) ;
_PROTOTYP( BOOL   VscrnIsLineMarked, ( BYTE, LONG ) ) ;
_PROTOTYP( BOOL   VscrnIsMarked, ( BYTE, LONG, SHORT ) ) ;
_PROTOTYP( BOOL   VscrnIsClear, ( BYTE, int ) ) ;
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
_PROTOTYP( void   markdownhalfscreen, ( BYTE ) ) ;
_PROTOTYP( void   markupscreen, ( BYTE ) ) ;
_PROTOTYP( void   markuphalfscreen, ( BYTE ) ) ;
_PROTOTYP( void   markleftpage, ( BYTE ) ) ;
_PROTOTYP( void   markrightpage, ( BYTE ) ) ;
_PROTOTYP( void   markhomescreen, ( BYTE ) ) ;
_PROTOTYP( void   markendscreen, ( BYTE ) ) ;

_PROTOTYP( void   markcopyclip, ( BYTE, int ) ) ;
_PROTOTYP( void   markselect, ( BYTE, int ) ) ;
_PROTOTYP( void   markcopyhost, ( BYTE, int ) ) ;
_PROTOTYP( void   markprint, ( BYTE, int ) ) ;

#ifdef OS2ONLY
_PROTOTYP( USHORT getshiftstate, ( void ) ) ;
#endif /* OS2ONLY */

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
#ifndef KUI
_PROTOTYP(void clearcmdscreen, (void));
#endif /* KUI */
_PROTOTYP(void cleartermpage, (BYTE,int));
_PROTOTYP(void cleartermscreen, (BYTE));
_PROTOTYP(void clearscrollback, (BYTE) ) ;
_PROTOTYP(cell_video_attr_t geterasecolor, (int));
_PROTOTYP(void clrtoeoln, (BYTE,CHAR));
_PROTOTYP(void clrbol_escape, (BYTE,CHAR));
_PROTOTYP(void clrbos_escape, (BYTE,CHAR));
_PROTOTYP(void clreoscr_escape, (BYTE,CHAR));
_PROTOTYP(void clreol_escape, (BYTE,CHAR));
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
_PROTOTYP(void prtpage, (BYTE,int, int, int));
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
_PROTOTYP(void set_page_margins, (int, int, int, int, int));
_PROTOTYP(void strinsert, (char *, char *));
_PROTOTYP(void wrtch, (unsigned short));
_PROTOTYP(int sendescseq, (CHAR *));
/*_PROTOTYP(static int ckcgetc, (int));*/
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

_PROTOTYP( static BOOL IsConnectMode, ( void ) ) ;
_PROTOTYP( void   SetConnectMode, ( BOOL, int ) ) ;
_PROTOTYP( void   getcmdcolor, ( void ) ) ;
_PROTOTYP( cell_video_attr_t ComputeColorFromAttr, (int, cell_video_attr_t, unsigned short));
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

_PROTOTYP( APIRET CopyToClipboard, ( BYTE* data, ULONG length ));
_PROTOTYP( BYTE * GetClipboardContent, (void));
#ifdef NT
_PROTOTYP( USHORT * GetUnicodeClipboardContent, (void));
#endif /* NT */

_PROTOTYP(void to_alternate_buffer, (BYTE));
_PROTOTYP(void from_alternate_buffer, (BYTE));
_PROTOTYP(void set_alternate_buffer_enabled, (BYTE,BOOL));

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
static _inline
#else
#ifdef __WATCOMC__
inline
#else
_Inline
#endif
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
