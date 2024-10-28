char *version = "0.998 21-May-96";
char *copyright = "Copyright (C) 1991, 1996, Trustees of Columbia University";
/*
  textps - Convert plain text to Postscript.

  DESCRIPTION:
    Converts text files to PostScript Courier-11, 66 lines to the page, 80
    characters to the line.  Handles pagination, tabs, line wrap, overstruck
    characters (via BS) and overstruck lines (via CR).  Swallows and ignores
    (rather than printing) ANSI escape sequences.  If the input file is
    already Postscript (i.e. if its first line starts with "%!"), it is simply
    copied to the output without alteration.  No special effects like page
    headings, landscape, 2-up, etc.

    Unlike other "enscriptors", textps handles 8-bit character sets.  The
    default file character set is CP437 on PCs, the NeXT character set on the
    NeXT, and ISO 8859-1 Latin Alphabet 1 elsewhere.  CP850, DEC MCS, and
    Apple QuickDraw are also supported.  Override the default character set
    with the -c command-line option.  Shift-In/Shift-Out codes within the text
    are handled, so 8-bit characters can be encoded by ^N<char-128>^O in the
    7-bit environment (e.g. e-mail) and still print correctly.

  USAGE:
    textps [ -h ] [ -v ] [ -c charset ] < input > output

    The input file character set is translated from the default character set
    or the one given on the command line to ISO Latin Alphabet 1, and the
    result is converted to Level-1 Postscript.  The -h command line option
    prints a help message and exits immediately.  The -v option includes a
    page showing the textps and PostScript version numbers.  The -c option
    specifies the file's character set; charset may be latin1, cp437, cp850,
    decmcs, apple, or next.

    UNIX example:   textps < file | lpr
    MS-DOS example: textps < file > prn

    Suggestion for use with DOS.  Make a batch file called PSPRINT.BAT:
      @echo off
      textps < %1 > prn

    This assumes PRN is a Postscript printer.  Works with both Postscript
    and non-Postscript files.  Works with Novell CAPTURE.  In VMS, make a
    DCL procedure similar to this batch file.  In UNIX, use this program
    as a print filter (in /etc/printcap, or alias lpr='textps | lpr').

  ERRORS:
    Returns status code of 0 on success, 1 on failure.
    Only fails if it is invoked with invalid command line arguments,
    in which case an error and usage message is printed on stderr.

  BUGS:
    Sometimes a spurious blank page is produced.

    Not all the characters print on early model laserwriters or other very
    old PostScript printers: broken bar, copyright, trade mark, not sign,
    fractions, Y/y-acute and Icelandic Thorn/thorn and Eth/eth, etc.  This
    is because these characters were not present in early PostScript releases.

    8-bit characters are translated into an internal character set, which is
    ISO Latin Alphabet 1 with a few extensions, so any file characters that
    don't don't appear in this character set, such as PC line- and box-
    drawing characters, are approximated with characters like '+', '-', and
    '|'.  Alphabetic or punctuation characters that have no equivalents in
    Latin-1 are shown as '?'.

  TO BUILD:
    Just compile it.  If compiled under DOS with Microsoft C (and probably
    also under Xenix?), the default character set is CP437, on the NeXT
    it's the NeXT character set, otherwise it's Latin-1.

    For OS/2, use the Makefile textps.os2 ("make -f textps.os2 <object>").
    See textps.os2 for a list of objects.

    For Windows NT and Windows 95, the default character set is CP437.

    To build with a particular default character set, include -DCHARSET=x
    on the cc command line, where x = 0 for Latin1, 1 for CP437, 2 for
    CP850, 3 for NeXT, 4 for DEC MCS, 5 for Apple QuickDraw.

    To disable ANSI-escape-sequence elimination, add -DNOESCSEQ.

  UPDATES:
    0.95 5 Aug 91 
      Add L procedure to output n blank lines, and don't bother to output
      blank lines after last text on page.
    0.96 6 Aug 91
      Make sure a file that starts with Ctrl-L still outputs the PostScript
      prolog.
    0.97 8 Aug 91
      Totally rewrite so we don't have to use backspacefont, which doesn't
      work if used too much.  This also allows backspace overstriking to work
      across line wrap boundaries.
    0.98 1 Oct 91
      Fix a few translation table entries.
    0.99 7 Oct 91
      Fix the top and bottom margins so 66th line doesn't sometimes have
      descenders cut off.
    0.995 18 Apr 93
      Fix output of already-PS files to not contain an unnecessary
      bufferful of blanks.  Add #ifdef for default OS/2 character set.
      Remove compiler complaint about buf in printf.
      Reported by Darrel Hankerson at Auburn University.
    0.996 16 Jan 94
      From Darrel Hankerson at Auburn University.  Improved OS/2 and MS-DOS
      support: get the current PC code page from the operating system rather
      than using a hardwired default.  New makefile for OS/2 and DOS.
      Print usage() message if stdin isatty().
    0.997 23 Feb 96
      Added support for Windows NT and Windows 95.
    0.998 21 May 96
      Added page-length support.

  TO DO:
    Add support for 7-bit national ISO 646 character sets so we can
    print e-mail from Sweden, France, etc.

    Add support for other PC code pages that could be translated into
    Latin-1, such as CP860, CP861, CP863, etc.

    Incorporate CP437 font from pc2ps program? (Box drawing chars, etc)

  Author: Frank da Cruz, Columbia University (fdc@columbia.edu), July 1991.
  Acks:   For help with reencoding bugs: Bur Davis, Adobe.
          For OS/2 and DOS improvements, Darrel Hankerson, Auburn University.

  Copyright (C) 1985, 1997, Trustees of Columbia University in the City of New
  York.
*/

/* Defines and Includes... */

/* For portability, we can't use logical operators in the preprocessor. */
/* Here we define OS2NTDOS if either OS2, MSDOS, or NT is defined, for items */
/* common to OS/2, MS-DOS, and Windows NT/95 ... */

#ifdef __EMX__
#ifndef OS2
#define OS2
#endif /* OS2 */
#endif /* __EMX__ */

#ifdef OS2
#ifndef OS2NTDOS
#define OS2NTDOS
#endif /* OS2NTDOS */
#endif /* OS2 */

#ifdef NT
#include <windows.h>
#ifndef OS2NTDOS
#define OS2NTDOS
#endif
#endif

#ifdef MSDOS
#ifndef OS2NTDOS
#define OS2NTDOS
#endif /* OS2NTDOS */
#endif /* MSDOS */

#ifndef NOESCSEQ	/* Swallow ANSI escape and control sequences */
#define ESCSEQ		/* unless -DNOESCSEQ given on cc command line. */
#endif /* NOESCSEQ */

#define WIDTH 80	/* Portrait printer line width, characters */
#define LENGTH 66	/* Portrait printer page length, lines */
#define MAXLENGTH 256

/* Character set translations */

#define UNK '?'      /* What to translate an untranslatable character into */
#define SP ' '				/* Space character */
#define DEL 0177			/* DEL character */

/* Character set symbols */

#define LATIN1 0			/* ISO Latin Alphabet 1 */
#define CP437  1			/* IBM code page 437 */
#define CP850  2			/* IBM code page 850 */
#define NEXT   3			/* NeXT character set */
#define DECMCS 4			/* DEC multinational character set */
#define APPLE  5			/* Apple QuickDraw character set */

/* Default character set depends on where we're being compiled. */

#ifndef CHARSET				/* If default not already defined */
#ifdef NT
#define CHARSET CP437
#else /* NT */
#ifdef OS2				/* See also the Dos call in main() */
#define CHARSET CP850			/* A little more modern for OS/2 */
#else
#ifdef MSDOS				/* Symbol predefined by Microsoft C */
#define CHARSET CP437			/* Default character set for PCs */
#else
#ifdef NeXT				/* Predefined by NeXT compiler */
#define CHARSET NEXT
#else
#define CHARSET LATIN1			/* Default character set for others */
#endif /* NeXT */
#endif /* OS2 */
#endif /* MSDOS */
#endif /* NT */
#endif /* CHARSET */

#include <stdio.h>			/* For EOF definition */
#ifdef OS2NTDOS
#include <io.h>				/* For isatty() */
#include <string.h>
#endif /* OS2NTDOS */

/*
  Postscript Prolog, to be inserted at the beginning of the output file.
  The %% Comments are to make the file conformant with Adobe's "Postscript 
  File Structuring Conventions", which allow page-oriented operations in
  Postscript previewers, page pickers, etc.
*/
char *prolog[] = {			/* Standard prolog */
    "%!PS-Adobe-1.0",			/* Works with Postscript 1.0 */
    "%%Title: oofa",
    "%%DocumentFonts: Courier CourierLatin1", 
    "%%Creator: textps",
    "%%Pages: (atend)",
    "%%EndComments",
/*
  Postscript font reencoding.  The standard encoding does not have the
  characters needed for Latin-1.

  Unfortunately, the font reencoding methods listed in the Postscript
  Cookbook simply do not work with the Apple Laserwriter (even though they
  did work with other Postscript devices).  The method described in the
  Adobe PostScript Language Reference Manual (2nd Ed) to change from the
  StandardEncoding vector to the ISOLatin1Encoding vector works only with
  the LaserWriter-II, but not older LaserWriters.

  This method, suggested by Bur Davis at Adobe, works around the fact that
  Courier was a "stroke font" in pre-version-47.0 Postscript, in which many of
  the accented characters are composed from other characters (e.g. i-grave =
  dotless i + grave).  It is probably not the most efficient possible solution
  (an iterative method might be better), but it works.
*/
    "/CourierLatin1 /Courier findfont dup dup maxlength dict begin",
    "{",
    "    1 index /FID ne { def } { pop pop } ifelse", 
    "} forall",
    "/Encoding exch 1 index get 256 array copy def", 
/*
  The following characters are added at the C1 positions 128-153, for printing
  non-Latin1 character sets such as IBM code pages, DEC MCS, NeXT, etc.  Note
  that we cannot use characters from the Symbol font.  Characters from
  different fonts cannot be mixed.  Anyway, the Symbol font is not fixed-width.
*/
    "Encoding 128 /quotesingle put",
    "Encoding 129 /quotedblleft put",
    "Encoding 131 /fi put",
    "Encoding 132 /endash put",
    "Encoding 133 /dagger put",
    "Encoding 134 /periodcentered put",
    "Encoding 135 /bullet put",
    "Encoding 136 /quotesinglbase put",
    "Encoding 137 /quotedblbase put",
    "Encoding 138 /quotedblright put",
    "Encoding 139 /ellipsis put",
    "Encoding 140 /perthousand put",
    "Encoding 141 /dotaccent put",
    "Encoding 142 /hungarumlaut put",
    "Encoding 143 /ogonek put",
    "Encoding 144 /caron put",
    "Encoding 145 /fl put",
    "Encoding 146 /emdash put",
    "Encoding 147 /Lslash put",
    "Encoding 148 /OE put",
    "Encoding 149 /lslash put",
    "Encoding 150 /oe put",
    "Encoding 151 /florin put",
    "Encoding 152 /fraction put",
    "Encoding 153 /daggerdbl put",
/*
  The following six characters are required for pre-47.0 PostScript versions,
  which compose accented Courier characters by putting together the base
  character and the accent.
*/
    "Encoding 154 /dotlessi put",
    "Encoding 155 /grave put",
    "Encoding 156 /circumflex put",
    "Encoding 157 /tilde put",
    "Encoding 158 /breve put",
    "Encoding 159 /ring put",
/*
  The remainder follow the normal ISO 8859-1 encoding.
*/
    "Encoding 160 /space put",
    "Encoding 161 /exclamdown put",
    "Encoding 162 /cent put", 
    "Encoding 163 /sterling put", 
    "Encoding 164 /currency put", 
    "Encoding 165 /yen put", 
    "Encoding 166 /brokenbar put", 
    "Encoding 167 /section put", 
    "Encoding 168 /dieresis put", 
    "Encoding 169 /copyright put", 
    "Encoding 170 /ordfeminine put", 
    "Encoding 171 /guillemotleft put", 
    "Encoding 172 /logicalnot put", 
    "Encoding 173 /hyphen put", 
    "Encoding 174 /registered put", 
    "Encoding 175 /macron put", 
    "Encoding 176 /degree put", 
    "Encoding 177 /plusminus put", 
    "Encoding 178 /twosuperior put", 
    "Encoding 179 /threesuperior put", 
    "Encoding 180 /acute put", 
    "Encoding 181 /mu put", 
    "Encoding 182 /paragraph put", 
    "Encoding 183 /bullet put", 
    "Encoding 184 /cedilla put", 
    "Encoding 185 /onesuperior put", 
    "Encoding 186 /ordmasculine put", 
    "Encoding 187 /guillemotright put", 
    "Encoding 188 /onequarter put", 
    "Encoding 189 /onehalf put", 
    "Encoding 190 /threequarters put", 
    "Encoding 191 /questiondown put", 
    "Encoding 192 /Agrave put", 
    "Encoding 193 /Aacute put", 
    "Encoding 194 /Acircumflex put", 
    "Encoding 195 /Atilde put", 
    "Encoding 196 /Adieresis put", 
    "Encoding 197 /Aring put", 
    "Encoding 198 /AE put", 
    "Encoding 199 /Ccedilla put", 
    "Encoding 200 /Egrave put", 
    "Encoding 201 /Eacute put", 
    "Encoding 202 /Ecircumflex put", 
    "Encoding 203 /Edieresis put", 
    "Encoding 204 /Igrave put", 
    "Encoding 205 /Iacute put", 
    "Encoding 206 /Icircumflex put", 
    "Encoding 207 /Idieresis put", 
    "Encoding 208 /Eth put", 
    "Encoding 209 /Ntilde put", 
    "Encoding 210 /Ograve put", 
    "Encoding 211 /Oacute put", 
    "Encoding 212 /Ocircumflex put", 
    "Encoding 213 /Otilde put", 
    "Encoding 214 /Odieresis put", 
    "Encoding 215 /multiply put", 
    "Encoding 216 /Oslash put", 
    "Encoding 217 /Ugrave put", 
    "Encoding 218 /Uacute put", 
    "Encoding 219 /Ucircumflex put", 
    "Encoding 220 /Udieresis put", 
    "Encoding 221 /Yacute put", 
    "Encoding 222 /Thorn put", 
    "Encoding 223 /germandbls put", 
    "Encoding 224 /agrave put", 
    "Encoding 225 /aacute put", 
    "Encoding 226 /acircumflex put", 
    "Encoding 227 /atilde put", 
    "Encoding 228 /adieresis put", 
    "Encoding 229 /aring put", 
    "Encoding 230 /ae put", 
    "Encoding 231 /ccedilla put", 
    "Encoding 232 /egrave put", 
    "Encoding 233 /eacute put", 
    "Encoding 234 /ecircumflex put", 
    "Encoding 235 /edieresis put", 
    "Encoding 236 /igrave put", 
    "Encoding 237 /iacute put", 
    "Encoding 238 /icircumflex put", 
    "Encoding 239 /idieresis put", 
    "Encoding 240 /eth put", 
    "Encoding 241 /ntilde put", 
    "Encoding 242 /ograve put", 
    "Encoding 243 /oacute put", 
    "Encoding 244 /ocircumflex put", 
    "Encoding 245 /otilde put", 
    "Encoding 246 /odieresis put", 
    "Encoding 247 /divide put", 
    "Encoding 248 /oslash put", 
    "Encoding 249 /ugrave put", 
    "Encoding 250 /uacute put", 
    "Encoding 251 /ucircumflex put", 
    "Encoding 252 /udieresis put", 
    "Encoding 253 /yacute put", 
    "Encoding 254 /thorn put", 
    "Encoding 255 /ydieresis put", 
    "currentdict end definefont", 
/*
  Set the font and define functions for adding lines and printing pages.
*/
    "/CourierLatin1 findfont 11 scalefont setfont",
    "/StartPage{/sv save def 48 765 moveto}def",
    "/ld -11.4 def",			/* Line spacing */
    "/yline 765 def",			/* Position of top line */
    "/U{show",				/* Show line, don't advance */
    "  48 yline moveto}def",
    "/S{show",				/* Show line, move to next line */
    "  /yline yline ld add def",
    "  48 yline moveto}def",
    "/L{ld mul yline add /yline exch def", /* Move down n lines  */
    "  48 yline moveto}def",
    "/EndPage{showpage sv restore}def",
    "%%EndProlog",			/* End of prolog. */
    "%%Page: 1 1",			/* Number the first page. */
    "StartPage",			/* And start it. */
    ""					/* Empty string = end of array. */
};

/*
  Translation tables from local character sets into CourierLatin1.
*/

/*
  IBM Code Page 437.  Line- and box-drawing characters are simulated with
  dashes, bars, and plus signs.  Black and gray blobs (fill characters)
  are replaced by X's.  Peseta is shown as P.  Greek letters that don't
  exist in CourierLatin1 are shown as ?.  Untranslatable math symbols are
  shown as ?.
*/
unsigned char
y43l1[] = {
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
199, 252, 233, 226, 228, 224, 229, 231, 234, 235, 232, 239, 238, 236, 196, 197,
201, 230, 198, 244, 246, 242, 251, 249, 255, 214, 220, 162, 163, 165, 'P', 151,
225, 237, 243, 250, 241, 209, 170, 186, 191, '+', 172, 189, 188, 161, 171, 187,
'X', 'X', 'X', '|', '+', '+', '+', '+', '+', '+', '|', '+', '+', '+', '+', '+',
'+', '+', '+', '+', '-', '+', '+', '+', '+', '+', '+', '+', '+', '-', '+', '+',
'+', '+', '+', '+', '+', '+', '+', '+', '+', '+', '+', 'X', 'X', 'X', 'X', 'X',
UNK, 223, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK, UNK,
UNK, 177, UNK, UNK, UNK, UNK, UNK, UNK, 176, 134, 135, UNK, 'n', 178, 'X', 160
};

/*
  IBM Code Page 850.  Line- and box-drawing characters are simulated with 
  dashes, bars, and plus signs.  Black blobs are replaced by X's.
*/
unsigned char
y85l1[] = {
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
199, 252, 233, 226, 228, 224, 229, 231, 234, 235, 232, 239, 238, 236, 196, 197,
201, 230, 198, 244, 246, 242, 251, 249, 255, 214, 220, 248, 163, 216, 215, 151,
225, 237, 243, 250, 241, 209, 170, 186, 191, 174, 172, 189, 188, 161, 171, 187,
'X', 'X', 'X', '|', '+', 193, 194, 192, 169, '+', '|', '+', '+', 162, 165, '+',
'+', '+', '+', '+', '-', '+', 227, 195, '+', '+', '+', '+', '+', '-', '+', 164,
240, 208, 202, 203, 200, 154, 205, 206, 207, '+', '+', 'X', 'X', 166, 204, 'X',
211, 223, 212, 210, 245, 213, 181, 254, 222, 218, 219, 217, 253, 221, 175, 180,
173, 177, '=', 190, 182, 167, 247, 184, 176, 168, 134, 185, 179, 178, 'X', 160
};

/*
  NeXT character set.  Here we have a full translation.
*/
unsigned char
ynel1[] = {
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
160, 192, 193, 194, 195, 196, 197, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 217, 218, 219, 220, 221, 222, 181, 215, 247,
169, 161, 162, 163, 152, 165, 151, 167, 164, 128, 129, 171, '<', '>', 131, 145,
174, 132, 133, 153, 134, 166, 182, 135, 136, 137, 138, 187, 139, 140, 172, 191,
185,  96, 180,  94, 126, 175, 158, 141, 168, 178, 176, 184, 179, 142, 143, 144,
146, 177, 188, 189, 190, 224, 225, 226, 227, 228, 229, 231, 232, 233, 234, 235,
236, 198, 237, 170, 238, 239, 240, 241, 147, 216, 148, 186, 242, 243, 244, 245,
246, 230, 249, 250, 251, 154, 252, 253, 149, 248, 150, 223, 254, 255, ' ', ' '
};

/*
  DEC Multinational Character Set (MCS).
*/
unsigned char
ydml1[] = {
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, ' ', 165, ' ', 167, 164, 169, 170, 171, ' ', ' ', ' ', ' ',
176, 177, 178, 179, ' ', 181, 182, 134, ' ', 185, 186, 187, 188, 189, ' ', 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
' ', 209, 210, 211, 212, 213, 214, 148, 216, 217, 218, 219, 220, 221, ' ', 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
' ', 241, 242, 243, 244, 245, 246, 150, 248, 249, 250, 251, 252, 255, ' ', ' ',
};

/*
  Apple QuickDraw character set.
*/
unsigned char
yaql1[] = {
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
196, 197, 199, 201, 209, 214, 220, 225, 224, 226, 228, 227, 229, 231, 233, 232,
234, 235, 237, 236, 238, 239, 241, 243, 242, 244, 246, 245, 250, 249, 251, 252,
133, 176, 162, 163, 167, 135, 182, 223, 174, 169, UNK, 180, 168, UNK, 198, 216,
UNK, 177, UNK, UNK, 165, 181, UNK, UNK, UNK, UNK, UNK, 170, 186, UNK, 230, 248,
191, 161, 172, UNK, 151, UNK, UNK, 171, 187, 139, ' ', 193, 195, 213, 148, 150,
132, 146, 129, 138,  47,  47, 247, UNK, 255, UNK, 152, 164, '<', '>', 131, 145,
153, 134, 136, 137, 140, 194, 202, 192, 203, 200, 205, 206, 207, 204, 211, 212,
UNK, 210, 218, 219, 217, 154, 156, 157, 175, 158, 141, 176, 184, 142, 143, 144
};

/*
  Data structures and functions for parsing and displaying character set name.
*/
struct keytab {				/* Keyword table template */
    char *kwd;				/* Pointer to keyword string */
    int kwval;				/* Associated value */
};

struct keytab csets[] = {		/* Character sets, alphabetical */
    "apple",   APPLE,			/* Apple QuickDraw */
    "cp437",   CP437,			/* IBM Code Page 437 */
    "cp850",   CP850,			/* IBM Code Page 850 */
    "decmcs",  DECMCS,			/* DEC Multinational Character Set */
    "latin1",  LATIN1,			/* ISO 8859-1 Latin Alphabet 1 */
    "next",    NEXT			/* NeXT character set */
};
int ncsets = (sizeof(csets) / sizeof(struct keytab));

int
lower(s) char *s; {			/* Lowercase the string */
    int n = 0;				/* return its length */
    while (*s) {
	if (*s >= 'A' && *s <= 'Z')
	  *s += ('A' - 'a');
        s++, n++;
    }
    return(n);
}

/* Look up keyword, return value */

int
lookup(table,cmd,n,x) char *cmd; struct keytab table[]; int n, *x; {
 
    int i, v, cmdlen;
 
/* Lowercase & get length of target, if it's null return code -3. */
 
    if ((((cmdlen = lower(cmd))) == 0) || (n < 1)) return(-3);
 
/* Not null, look it up */
 
    for (i = 0; i < n-1; i++) {
        if (!strcmp(table[i].kwd,cmd) ||
           ((v = !strncmp(table[i].kwd,cmd,cmdlen)) &&
             strncmp(table[i+1].kwd,cmd,cmdlen))) {
                *x = i;
                return(table[i].kwval);
             }
        if (v) return(-2);
    }   
 
/* Last (or only) element */
 
    if (!strncmp(table[n-1].kwd,cmd,cmdlen)) {
        *x = n-1;
        return(table[n-1].kwval);
    } else return(-1);
}

/* Look up value, return keyword */

char *
getname(x,table,n) int x, n; struct keytab table[]; {
    int i;
    for (i = 0; i < n; i++)
      if (table[i].kwval == x)
	return(table[i].kwd);
    return("");
}

/* Global data */

int charset = CHARSET,			/* Character set */
  hpos = 0,				/* Character number in line buffer */
  maxhpos = 0,				/* Longest line in buffer */
  page = 0,				/* Page number */
  line = 0,				/* Line number */
  blank = 0,				/* Blank line count */
  pagefull = 0,				/* Flag for page overflow */
  psflag = 0,				/* Flag for file already postscript */
  proflg = 0,				/* Prolog done */
  shift = 0;				/* Shift state */

int width = WIDTH;			/* Paper width, characters */
int length = LENGTH;			/* Paper length, characters */

/* Data structures */

/*
  buf is the line buffer.  columns (indexed by hpos) are the characters
  in the line, rows are overstruck lines.  At display time (see addline),
  buf is treated as a 3-dimensional array, with the extra dimension being
  for wraparound.  The total size of the buffer is 80 chars per line times
  66 lines per page times 10 levels of overstriking = 52,800.  This allows
  files that contain absolutely no linefeeds to still print correctly.
*/
#define BUFNUM 10			/* Number of overstrike buffers */
#define BUFWID 5280			/* Max characters per line */

unsigned char buf[BUFNUM][BUFWID];	/* Line buffers */
unsigned char outbuf[400];		/* Output buffer */
int linesize[BUFNUM];			/* Size of each line in buffer */
int bufs[MAXLENGTH];			/* # overstrike buffers per line */

/* Line and page display routines */

/* Forward declarations */

void addline();				/* Add line to page */
void addchar();				/* Add character to line */
void newpage();				/* New page */
void usage();				/* Usage message */

void
clearbuf() {				/* Clear line buffer */
    int i;
/*
  Note: if a loop is used instead of memset, this program runs
  veeeery slooooooowly.
*/
    memset(buf,SP,BUFNUM * BUFWID);	/* Clear buffers and counts */
    for (i = 0; i < BUFNUM; linesize[i++] = -1) ;
    for (i = 0; i < length; bufs[i++] = 0) ;
    hpos = 0;				/* Reset line buffer pointer */
    maxhpos = 0;			/* And maximum line length */
}

void
doprolog() {				/* Output the PostScript prolog */
    int i;
    for (i = 0; *prolog[i]; i++) {
	printf("%s\n",prolog[i]);
	proflg++;
    }
}

void
addchar(c) unsigned char c; {		/* Add character to line buffer */
    int i, m;
    
    if (c < SP || c == DEL) c = SP;	/* ASCII controls become spaces. */

    if (c > 127) {			/* 8-bit values are translated */
	switch (charset) {		/* according to character set. */
	  case LATIN1:
	    if (c > 127 && c < 161)	/* C1 characters are controls */
	      c = SP;			/* in Latin-1. */
	    break;
	  case CP437:  c = y43l1[c]; break;
	  case CP850:  c = y85l1[c]; break;
	  case NEXT:   c = ynel1[c]; break;
	  case DECMCS: c = ydml1[c]; break;
	  case APPLE:  c = yaql1[c]; break;
	}
    }
    for (i = 0; i < BUFNUM; i++) {	/* Find first */
	if (hpos > linesize[i]) {	/* available overstrike buffer */
	    buf[i][hpos] = c;		/* Deposit character */
	    linesize[i] = hpos;		/* Remember size of this buffer. */
	    m = hpos / width;		/* Line-wrap segment number. */
	    if (i > bufs[m]) bufs[m] = i; /* Highest overstrike buffer used */
	    break;			/*   for this line-wrap segment. */
	}
    }
    if (hpos > maxhpos) maxhpos = hpos;	/* Remember maximum line position. */
    if (++hpos >= BUFWID)		/* Increment line position. */
      addline();			/* If buffer full, dump it. */
}

void
addline() {				/* Add a line to the current page */
    int i, j, k, m, n, y, wraps;
    unsigned char *p, *q, c;

    if (line == 0 && page == 1) {	/* First line of file? */
	if (!strncmp(buf[0],"%!",2)) {	/* Already Postscript? */
	    psflag++;			/* Yes, set this flag & just copy */
	    buf[0][hpos] = '\0';	/* Trim trailing blanks */
	    printf("%s\n",buf[0]);	/* Send this line to stdout */
	    return;
	} else if (!proflg) {		/* Not Postscript, print prolog. */
	    doprolog();
	}
    }
    if (linesize[0] < 0) {		/* If line is empty, */
	blank++;			/* just count it. */
	return;
    }
    if (blank > 0) {			/* Any previous blank lines? */
	if (blank == 1)			/* One */
	  printf("()S\n");
	else				/* Many */
	  printf("%d L\n",blank);
    }
    line += blank;			/* Count the blank lines */
    blank = 0;				/* Reset blank line counter */

    wraps = maxhpos / width;		/* Number of times line will wrap */
    if (wraps > length) wraps = length;	/* (within reason) */

    for (k = 0; k <= wraps; k++) {	/* For each wrapped line */
	m = k * width;			/* Starting position in buffer */
	for (i = 0; i <= bufs[k]; i++) { /* For each overstrike buffer */
	    y = linesize[i] + 1;	/* Actual character count */
	    if (y <= m)			/* Nothing there, next buffer. */
	      continue;
	    /* Ending position of this wrap region in buffer. */
	    n = (y < m + width) ? y : m + width;
	    q = outbuf;
	    *q++ = '(';			/* Start text arg */
	    for (j = m, p = buf[i]+m; j < n; j++) { /* For each character */
		c = *p++;
		if (c == '(' || c == ')' || c =='\\') /* Quote specials */
		  *q++ = '\\';		/*  with backslash. */
		if ((int) c < 128)	/* Insert 7-bit character literally */
		  *q++ = c;
		else {			/* Insert 8-bit character */
		    *q++ = '\\';	/* as octal backslash escape */
		    *q++ = (c >> 6) + '0'; /* (this avoids call to sprintf) */
		    *q++ = ((c >> 3) & 07) + '0';
		    *q++ = (c & 07) + '0';
		}
	    }
	    *q = '\0';
	    printf("%s%s",outbuf, (i == bufs[k]) ? ")S\n" : ")U\n");
	}
    }
    clearbuf();				/* Clear line buffer */
    line += wraps + 1;			/* Increment line number */
    if (line > (length - 1)) {		/* If page is full */
	newpage();			/* print it and start new one */
	pagefull = 1;
    }
}

void
newpage() {				/* Print current page, start new one */
    if (pagefull) {			/* If we just overflowed onto a */
	pagefull = 0;			/* new page, but then got a formfeed */
	return;				/* immediately after... */
    }
    if (!proflg)			/* Do prolog if not done already */
      doprolog();			/*  (in case file starts with ^L) */
    if (hpos)				/* Add any partial line */
      addline();
    line = hpos = 0;			/* Reset line, advance page */
    page++;
    printf("EndPage\n%%%%Page: %d %d\nStartPage\n",page,page);
    blank = 0;
}

void			/* Show program & PostScript version numbers */
showver() {
    printf("%%!\n/Courier findfont 11 scalefont setfont\n");
    printf("/EndPage{version (PostScript version )\n");
    printf("48 720 moveto show show showpage sv restore}def\n");
    printf("/sv save def 48 740 moveto\n");
    printf("(textps version %s) show\n",version);
    printf("/sv save def 48 700 moveto\n");
    printf("(textps %s) show\n",copyright);
    printf("EndPage\n");
}

/*
   gcharset  -  Set the default character set.

   Under OS/2, use the DosQueryCp() or DosGetCp() call;
   under MSDOS, use int 21h; on others just return CHARSET.

   Note that in an MSC bound program, DosGetCp() will return the
   current code page and no more than one prepared code page.

   INT 21 - DOS 3.3+ - GET GLOBAL CODE PAGE TABLE
              AX = 6601h
   Return: CF set on error
              AX = error code (see AH=59h)
           CF clear if successful
              BX = active code page (see AX=6602h)
	      DX = system code page

   Values for code page:
     437 US (hardware code page on most PCs)
     850 Multilingual (OS/2 default)

   The following are similar to CP437, and are treated like CP437:
     857 Turkish         
     860 Portugal
     861 Iceland
     863 Canada (French)
     865 Norway/Denmark

   The following are not supported by textps,
   because Courier does not have the needed characters:
     852 Slavic/Latin-2 (DOS 5+)
     862 Hebrew
     866 Cyrillic
     982 Japanese Shift-JIS
     etc etc.
*/
int
gcharset() {

/* Note that charset=CHARSET is initialized above */

#ifdef OS2NTDOS
    int i;
#ifdef NT
   i = GetConsoleCP() ;
#else /* NT */
#ifdef OS2				/* Then get the code page... */
#define INCL_DOSNLS
#include <os2.h>
/*
  Get the current code page and the first two prepared code pages.
  Only the current code page is used in the following.
*/
#ifdef __EMX__
    ULONG CpList[3], CpSize, rc;
    rc = DosQueryCp(sizeof(CpList), CpList, &CpSize);
#else /* Watcom or MSC */
#ifdef __WATCOMC__
    /* Same as EMX */
    ULONG CpList[3], CpSize, rc;
    rc = DosQueryCp(sizeof(CpList), CpList, &CpSize);
#else /* MSC */
    USHORT CpList[3], CpSize, rc;
    rc = DosGetCp(sizeof(CpList), CpList, &CpSize);
#endif
#endif /* __EMX__ */
    i = (int) CpList[0];
#else /* MSDOS */
#include <dos.h>
    union REGS regs;

    regs.x.ax = 0x6601;
    intdos(&regs, &regs);
    i = regs.x.bx;
#endif /* OS2 */
#endif /* NT */
    switch (i) {
      case 437: /* CP437 */
      case 860: /* Portugal */
      case 857: /* Turkey */
      case 861: /* Iceland */
      case 863: /* Canadian French */
      case 865: /*  Norway and Denmark */
	charset = CP437;
	break;
      case 850: /* Multilingual (West European) code page */
	charset = CP850;
	break;
    }
#endif /* OS2NTDOS */

	return (charset);
}


/* Main program */

void
main(argc, argv) int argc; char *argv[]; {

    int i, j,				/* Worker ints */
      escape;				/* Flag when Esc seen. */
    unsigned char c;			/* Input character */

    gcharset();				/* Get the default character set */
    while (--argc > 0) {		/* argv/argc "User interface"... */
	argv++;
	if (**argv == '-') {		/* Look for '-' */
	    c = *(*argv+1);		/* Get option letter */
	    switch (c) {
	      case 'l': case 'L':	/* Paper length */
		argv++, argc--;
		length = atol(*argv);
		break;
	      case 'w': case 'W':	/* Paper width */
		argv++, argc--;
		width = atol(*argv);
		break;
	      case 'h': case 'H': case '?': /* Help */
		usage(-1);
	      case 'c': case 'C':	/* Character set */
		argv++, argc--;
		if (argc < 1) usage(4);
		i = lookup(csets,*argv,ncsets,&j);
		if (i < 0) usage(-i);
		charset = i;
		break;
	      case 'v': case 'V':	/* Show version numbers */
		showver();
		break;
	      default:
		usage(5);
	    }
	} else {			/* Options must begin with '-' */
	    usage(0);
	}
    }
#ifdef OS2NTDOS
#ifdef NT
#ifdef __WATCOMC__
    if (_isatty(stdin->_handle))
#else
#ifdef _MSC_VER
#if _MSC_VER < 1900
    /* Visual C++ 2013 or older */
    if (_isatty(stdin->_file))
#else /* else _MSC_VER >= 1900 */
    /* Visual C++ 2015 or newer */
    if (isatty(fileno(stdin)))
#endif /* if _MSC_VER < 1900 */
#else
    /* Not Visual C++ - assume it behaves like old Visual C++ */
    if (_isatty(stdin->_file))
#endif
#endif /* __WATCOMC__ */
#else
	if (isatty(fileno(stdin)))
#endif /* NT */
      usage(-1);
#endif /* OS2NTDOS */
    hpos = line = psflag = 0;		/* Initialize these... */ 
    escape = blank = 0;
    page = 1;

    clearbuf();				/* Clear line buffer. */

    while ((i = getchar()) != EOF) {	/* Read a file character */
	c = i;				/* Convert to unsigned char */
	if (psflag) {			/* File already postscript? */
	    putchar(c);			/* Just copy the bytes. */
	    continue;
	}
#ifdef ESCSEQ
	if (escape) {			/* Swallow ANSI escape sequences */
	    switch (escape) {
	      case 1:			/* ESC */
		if (c < 040 || c > 057) /* Not intermediate character */
		  escape = 0;
		continue;
	      case 2:			/* CSI */
		if (c < 040 || c > 077)	/* Not parameter or intermediate */
		  escape = 0;
		continue;
	      default:			/* Bad escape value, */
		escape = 0;		/* shouldn't happen. */
		break;
	    }
	}
#endif /* ESCSEQ */

	if (shift && c > 31 && c < 127) 
	  c |= 0200;			/* Handle shift state. */

	if (pagefull && c != 014)	/* Spurious blank page suppression */
	  pagefull = 0;

	switch (c) {			/* Handle the input character */

	  case 010:			/* Backspace */
	    hpos--;
	    if (hpos < 0) hpos = 0;
	    continue;

	  case 011: 			/* Tab */
	    hpos = (hpos | 7) + 1;
	    continue;

	  case 012:			/* Linefeed */
	    addline();			/* Add the line to the page */
	    continue;

	  case 014:			/* Formfeed */
	    newpage();			/* Print current page */
	    continue;

	  case 015:			/* Carriage return */
	    hpos = 0;			/* Back to left margin */
	    continue;

	  case 016:			/* Shift-Out */
	    shift = 1;			/* Set shift state */
	    continue;

	  case 017:			/* Shift-In */
	    shift = 0;			/* Reset shift state */
	    continue;

#ifdef ESCSEQ				/* Swallow ANSI escape sequences */
/*
  1 = ANSI escape sequence
  2 = ANSI control sequence
*/
	  case 033:			/* ESC or 7-bit CSI */
	    escape = ((c = getchar()) == 0133) ? 2 : 1;
	    if (c != 033 && c != 0133 && c != 233) /* Not ANSI after all */
	      ungetc(c,stdin);		/* put it back, avoid loops */
	    continue;

	  case 0233:			/* 8-bit CSI */
	    if (charset == LATIN1) {
		escape = 2;		/* 0233 is graphic char on PC, etc */
		continue;
	    }				/* Otherwise fall thru & print it */
#endif /* ESCSEQ */

	  default:
	    addchar(c);
	}
    }
    if (!psflag) {			/* Done. If not postscript already, */
	if (hpos)			/* if last line was not empty, */
	  addline();			/* add it to the page. */
	if (page != 1 || line != 0) {	/* Add trailer. */
	    printf("EndPage\n%%%%Trailer\n%%%%Pages: %d\n",page);
	}
    }
    exit(0);				/* Done, return success code. */
}

void
usage(x) int x; {			/* Give usage message and quit. */
    int i;
    switch (x) {
      case 0:
	fprintf(stderr,"textps: only options, not filenames, allowed;");
	fprintf(stderr," use standard input.\n");
	break;
      case 1:
	fprintf(stderr,"textps: invalid character set name\n");
	break;
      case 2:
	fprintf(stderr,"textps: ambiguous option\n");
	break;
      case 3:
	fprintf(stderr,"textps: option required after -\n");
	break;
      case 4:
	fprintf(stderr,"textps: option requires an argument\n");
	break;
      case 5:
	fprintf(stderr,"textps: invalid option\n");
	break;	
      default:
	break;
    }
    fprintf(stderr,
"textps converts standard input to PostScript on standard output.\n");
    fprintf(stderr,
"if standard input is already in PostScript format, it is simply copied.\n");
    fprintf(stderr,
"usage:  textps -h -v -c charset -l number -w number < infile > outfile\n");
    fprintf(stderr,"  -h displays this usage message.\n");

    fprintf(stderr,
"  -v produces a page showing textps and PostScript version numbers.\n");
    fprintf(stderr,
"  -c specifies the file's character set, one of the following:\n");
    for (i = 0; i < ncsets; i++)
      fprintf(stderr,"      %s\n",csets[i].kwd);
    gcharset();
    fprintf(stderr,"  the default character set is %s.\n",
	    getname(charset,csets,ncsets));
    fprintf(stderr,"  -l number is paper length in lines (default 66).\n");
    fprintf(stderr,"  -w number is paper width in characters (default 80).\n");
    fprintf(stderr,"examples:\n");
    fprintf(stderr,"  textps < infile > outfile\n");
    fprintf(stderr,"  textps -v < infile > outfile\n");
    fprintf(stderr,"  textps -c cp850 < infile > outfile\n");
    fprintf(stderr,"  textps -v -c next < infile > outfile\n");
    fprintf(stderr,"  textps < infile | lpr (UNIX)\n");
    fprintf(stderr,"  textps < infile > prn (DOS)\n");
    fprintf(stderr,"textps version: %s.\n",version);
    fprintf(stderr,"%s\n",copyright);
    exit(1);
}
