#ifndef ikextern_h_included
#define ikextern_h_included

#ifdef __cplusplus
extern "C" {
#endif

#include "ckcdeb.h"             /* Typedefs, debug formats, etc */
#include "ckcker.h"             /* Kermit definitions */
#include "ckclib.h"
#include "ckcasc.h"             /* ASCII character symbols */
#include "ckcxla.h"             /* Character set translation */
#include "ckcnet.h"             /* Network support */
#include "ckuusr.h"             /* For terminal type definitions, etc. */
#include "ckopcf.h"             /* PC Fonts resource definitions */
#include "ckcuni.h"
#include "ckocon.h"
#include "ckokey.h"
#include "ckoetc.h"

// look in ckucmd.h for keytab
//
extern struct keytab ttyptab[];	// terminal types structure
extern int nttyp;			// number of terminal types
#ifdef TNCODE
extern int ttnum, ttnumend; 
#endif
extern struct keytab ttyclrtab[];	// colors
extern int nclrs;			// number of colors

extern struct keytab lngtab[];		// language
extern int nlng;			// number of languages

extern struct keytab ttcstab[];	// terminal charater sets
extern int ntermc;			// number of terminal character sets

extern struct keytab txrtab[];
extern int ntxrtab;

extern struct keytab tcstab[];		// transfer charater sets
extern int ntcs;			// number of transfer charater sets

extern struct keytab fcstab[];		// file character sets
extern int nfilc;			// number of file character sets

extern struct keytab protos[];		// transfer protocols
extern int nprotos;			// number of transfer protocols

extern int deblog;              /* Flag for debug logging */
extern char debfil[];           /* Debug log filename */
extern int pktlog;              /* Flag for packet logging */
extern char pktfil[];            /* Packet log filename */
extern int seslog;              /* Flat for session logging */
extern char sesfil[];           /* Session log filename */
extern int tralog;              /* Flag for transaction logging */
extern char trafil[];           /* Transaction log filename */

extern struct keytab * term_font;
extern struct keytab * _term_font;
extern int ntermfont;
extern int tt_font, tt_font_size, tt_scroll;

extern int os2gks;              /* Kverb processing active? */
extern int win95lucida, win95hsl;
extern unsigned long startflags;

extern CKFLOAT tt_linespacing[];

int hupok(int);
void setexitwarn(int);
void setguidialog(int);

extern int vmode, autodl, adl_ask, cmask, tt_url_hilite, locus, autolocus;
extern int xitwarn, gui_dialog, xitsta;

#ifdef __cplusplus
}
#endif 

enum eKermitExtern { K_TERMTYPESET = 1, K_COLORSET, K_TERMCHARSET
		, K_TRANSFERCHARSET, K_FILECHARSET, K_LANGUAGESET };

#endif