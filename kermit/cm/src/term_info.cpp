#include "term_info.h"

namespace Term {

static const TermInfo terminals[] = {
	{	TT_ADM3A,	TEXT("adm3a"),		TEXT("Lear Siegler ADM-3A"),		0	},
	{	TT_ADM5,	TEXT("adm5"),		TEXT("Lear Siegler ADM-5"),			0	},
	{	TT_AIXTERM,	TEXT("aixterm"),	TEXT("IBM AIXterm"),				0	},
	{	TT_AAA,		TEXT("annarbor"),	TEXT("Ann Arbor Ambassador"),		0	},
	{	TT_ANSI,	TEXT("ansi-bbs"),	TEXT("ANSI.SYS (BBS)"),				0	},
	{	TT_AT386,	TEXT("at386"),		TEXT("Unixware ANSI"),				0	},
	{	TT_AVATAR,	TEXT("avatar/0+"),	TEXT("AVATAR/0+"),					0	},
	{	TT_BA80,	TEXT("ba80"),		TEXT("Nixdorf BA80"),				0	},
	{	TT_BEOS,	TEXT("beterm"),		TEXT("BeOS Terminal"),				0	},
	{	TT_DG200,	TEXT("dg200"),		TEXT("Data General DASHER D200"),	0	},
	{	TT_DG210,	TEXT("dg210"),		TEXT("Data General DASHER D210"),	0	},
	{	TT_DG217,	TEXT("dg217"),		TEXT("Data General DASHER D217"),	0	},
	{	TT_H19,		TEXT("heath19"),	TEXT("Heath-19"),					0	},
	{	TT_HFT,		TEXT("hft"),		TEXT("IBM High Function Terminal"),	0	},
	{	TT_HP2621,	TEXT("hp2621a"),	TEXT("HP 2621A"),					0	},
	{	TT_HPTERM,	TEXT("hpterm"),		TEXT("HPTERM"),						0	},
	{	TT_HZL1500,	TEXT("hz1500"),		TEXT("Hazeltine 1500"),				0	},
	{	TT_IBM31,	TEXT("ibm3151"),	TEXT("IBM 3101-xx/3161"),			0	},
	{   TT_K95,     TEXT("k95"),        TEXT("Kermit 95"),                  0   },
	{	TT_LINUX,	TEXT("linux"),		TEXT("Linux Console"),				0	},
	{	TT_QANSI,	TEXT("qansi"),		TEXT("QNX ANSI"),					0	},
	{	TT_QNX,		TEXT("qnx"),		TEXT("QNX Console"),				0	},
	{	TT_SCOANSI,	TEXT("scoansi"),	TEXT("SCO ANSI"),					0	},
	{	TT_97801,	TEXT("sni-97801"),	TEXT("Siemens-Nixdorf 97801"),		0	},
	{	TT_SUN,		TEXT("sun"),		TEXT("Sun Console"),				0	},
	//{	TT_TEK40,	TEXT("tek4014"),	TEXT("Tektronix 4014"),				0	},
	{	TT_NONE,	TEXT("tty"),		TEXT("Teletypewriter"),				0	},
	{	TT_TVI910,	TEXT("tvi910+"),	TEXT("Televideo 910+"),				0	},
	{	TT_TVI925,	TEXT("tvi925"),		TEXT("Televideo 925"),				0	},
	{	TT_TVI950,	TEXT("tvi950"),		TEXT("Televideo 950"),				0	},
	{	TT_VC4404,	TEXT("vc404"),		TEXT("Volker Craig 404"),			0	},
	{	TT_VIP7809,	TEXT("vip7809"),	TEXT("Honeywell VIP-7809"),			0	},
	{	TT_VT100,	TEXT("vt100"),		TEXT("DEC VT100"),					0	},
	{	TT_VT102,	TEXT("vt102"),		TEXT("DEC VT102"),					0	},
	{	TT_VT220,	TEXT("vt220"),		TEXT("DEC VT220"),					0	},
	{	TT_VT220PC,	TEXT("vt220pc"),	TEXT("DEC VT220 with PC Keyboard"),	0	},
	{	TT_VT320,	TEXT("vt320"),		TEXT("DEC VT320"),					0	},
	{	TT_VT320PC,	TEXT("vt320pc"),	TEXT("DEC VT320 with PC Keyboard"),	0	},
	//{	TT_VT420,	TEXT("vt420"),		TEXT("DEC VT420"),					0	},
	//{	TT_VT420PC,	TEXT("vt420pc"),	TEXT("DEC VT420 with PC Keyboard"),	0	},
	//{	TT_VT520,	TEXT("vt520"),		TEXT("DEC VT520"),					0	},
	//{	TT_VT520PC,	TEXT("vt520pc"),	TEXT("DEC VT520 with PC KEyboard"),	0	},
	{	TT_VT52,	TEXT("vt52"),		TEXT("DEC VT52"),					0	},
	{	TT_VTNT,	TEXT("vtnt"),		TEXT("Microsoft VTNT"),				0	},
	{	TT_WY160,	TEXT("wy160"),		TEXT("Wyse WY-160/120/350"),		0	},
	{	TT_WY30,	TEXT("wy30"),		TEXT("Wyse WY-30/30+/35"),			0	},
	{	TT_WY370,	TEXT("wy370"),		TEXT("Wyse WY-370"),				0	},
	{	TT_WY50,	TEXT("wy50"),		TEXT("Wyse WY-35"),					0	},
	{	TT_WY60,	TEXT("wy60"),		TEXT("Wyse WY-60"),					0	},
	//{	TT_XTERM,	TEXT("xterm"),		TEXT("Xterm"),						0	},
	{	TT_INVALID,	0,					0,									0	},
};

// Default settings for the various terminals. The important ones are
// character size (7bits or 8bits) and the character set.
static const TermSettings terminalSettings[] = {
	// Terminal		Character set			status? 8bit  lines Term Foreground		Term Bg			Underline Fg		Underline Bg
	{	TT_ADM3A,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	// The dialer didn't have a definition for the ADM5. It seems to be a 7 bit ASCII terminal based on the manual.
	{	TT_ADM5,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_AIXTERM,	Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_AAA,		Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	// Change from the dialer: ANSI and AT386 previously used CS_TRANSP which was different from how K95 configured
	// these terminal types.
	{	TT_ANSI,	Charset::CS_CP437,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLACK,	COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_AT386,	Charset::CS_CP437,		FALSE,	TRUE,	25,	COLOR_LIGHT_GRAY,	COLOR_BLACK,	COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_AVATAR,	Charset::CS_TRANSP,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLACK,	COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_BA80,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_BEOS,	Charset::CS_TRANSP,		FALSE,	TRUE,	25,	COLOR_LIGHT_GRAY,	COLOR_BLACK,	COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_DG200,	Charset::CS_DGI,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_DG210,	Charset::CS_DGI,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_DG217,	Charset::CS_DGI,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_H19,		Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_HFT,		Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_HP2621,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_HPTERM,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_HZL1500,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_IBM31,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{   TT_K95,     Charset::CS_UTF8,       TRUE,   TRUE,   24, COLOR_LIGHT_GRAY,   COLOR_BLACK,    COLOR_LIGHT_GRAY,   COLOR_RED   },
	// Change from the dialer: turn off the status line for linux to make it consistent with K95G (the dialer set it to on)
	// And use UTF-8 as the charset rather than CP437 as most linux in the last 20 years will be sending unicode.
	// And set the default background colour to black rather than blue as blue is a weird background colour for a linux console
	// that you'll likely never encounter in the wild.
	{	TT_LINUX,	Charset::CS_UTF8,		FALSE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLACK,	COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_QANSI,	Charset::CS_TRANSP,		FALSE,	TRUE,	25,	COLOR_LIGHT_GRAY,	COLOR_BLACK,	COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_QNX,		Charset::CS_TRANSP,		FALSE,	TRUE,	25,	COLOR_LIGHT_GRAY,	COLOR_BLACK,	COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_SCOANSI,	Charset::CS_TRANSP,		FALSE,	TRUE,	25,	COLOR_LIGHT_GRAY,	COLOR_BLACK,	COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_97801,	Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_SUN,		Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	//{	TT_TEK40,	Charset::CS_INVALID,	TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLACK,	COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_NONE,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_TVI910,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_TVI925,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_TVI950,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_VC4404,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_VIP7809,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_VT100,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_VT102,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_VT220,	Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_VT220PC,	Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_VT320,	Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_VT320PC,	Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	//{	TT_VT420,	Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	//{	TT_VT420PC,	Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	//{	TT_VT520,	Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	//{	TT_VT520PC,	Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_VT52,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	// VTNT is a weird one: it just sends serialised windows console structs. Character set is treated as UTF-16 unless K95 is 
	// running on Windows 9x. So not much point specifying a character set for it - the setting is practically ignored unless perhaps
	// you're on Windows 95/98/ME.
	{	TT_VTNT,	Charset::CS_INVALID,	TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_WY160,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_WY30,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_WY370,	Charset::CS_8859_1,		TRUE,	TRUE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_WY50,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	{	TT_WY60,	Charset::CS_ASCII,		TRUE,	FALSE,	24,	COLOR_LIGHT_GRAY,	COLOR_BLUE,		COLOR_LIGHT_GRAY,	COLOR_RED	},
	////{	TT_XTERM,	Charset::CS_UTF8,	TRUE,	TRUE,	24,	COLOR_BLACK,		COLOR_WHITE,	COLOR_LIGHT_GRAY,	COLOR_YELLOW	},
	{	TT_INVALID,	Charset::CS_INVALID,	FALSE,	FALSE,	1,	COLOR_BLACK,		COLOR_BLACK,	COLOR_BLACK,		COLOR_BLACK	},
	// Terminal		Character set			status? 8bit  lines Term Foreground		Term Background		Underline Fg		Underline Bg
};

static const ColorInfo colorInfos[] = {
	{	COLOR_BLACK,			TEXT("black"),			TEXT("Black")			},
	{	COLOR_BLUE,				TEXT("blue"),			TEXT("Blue")			},
	{	COLOR_GREEN,			TEXT("green"),			TEXT("Green")			},
	{	COLOR_CYAN,				TEXT("cyan"),			TEXT("Cyan")			},
	{	COLOR_RED,				TEXT("red"),			TEXT("Red")				},
	{	COLOR_MAGENTA,			TEXT("magenta"),		TEXT("Magenta")			},
	{	COLOR_BROWN,			TEXT("brown"),			TEXT("Brown")			},
	{	COLOR_LIGHT_GRAY,		TEXT("lightgray"),		TEXT("Light Gray")		},
	{	COLOR_DARK_GRAY,		TEXT("dgray"),			TEXT("Dark Gray")		},
	{	COLOR_LIGHT_BLUE,		TEXT("lightblue"),		TEXT("Light Blue")		},
	{	COLOR_LIGHT_GREEN,		TEXT("lightgreen"),		TEXT("Light Green")		},
	{	COLOR_LIGHT_CYAN,		TEXT("lightcyan"),		TEXT("Light Cyan")		},
	{	COLOR_LIGHT_RED,		TEXT("lightred"),		TEXT("Light Red")		},
	{	COLOR_LIGHT_MAGENTA,	TEXT("lightmagenta"),	TEXT("Light Magenta")	},
	{	COLOR_YELLOW,			TEXT("yellow"),			TEXT("Yellow")			},
	{	COLOR_WHITE,			TEXT("white"),			TEXT("White")			},
	{	COLOR_INVALID,			TEXT(""),				TEXT("")				},
};

const TermInfo* getTerminalInfos() {
	return terminals;
}

LPCTSTR getTermKeyword(TermType type) {
	for (int i = 0; terminals[i].type != TT_INVALID; i++) {
		if (terminals[i].type == type) {
			return terminals[i].keyword;
		}
	}

	return NULL;
}


LPCTSTR getTermName(TermType type) {
	for (int i = 0; terminals[i].type != TT_INVALID; i++) {
		if (terminals[i].type == type) {
			return terminals[i].name;
		}
	}

	return NULL;
}

TermSettings getTermSettings(TermType type) {
	for (int i = 0; terminalSettings[i].type != TT_INVALID; i++) {
		if (terminalSettings[i].type == type) {
			return terminalSettings[i];
		}
	}

	// We should never get here. But just in case...
	TermSettings inv = { TT_INVALID, Charset::CS_INVALID, FALSE, FALSE, 1, 
		COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK };
	return inv;
}

ColorInfo getColorInfo(Color color) {
	for (int i = 0; colorInfos[i].color != COLOR_INVALID; i++) {
		if (colorInfos[i].color == color) {
			return colorInfos[i];
		}
	}

	// We should never get here. But just in case...
	ColorInfo inv = { COLOR_INVALID, TEXT(""), TEXT("") };
	return inv;
}

const ColorInfo* getColorInfos() {
	return colorInfos;
}

}