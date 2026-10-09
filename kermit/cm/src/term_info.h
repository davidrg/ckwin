#ifndef TERM_INFO_H
#define TERM_INFO_H

#include <windows.h>

#include "charset.h"

namespace Term {
	enum TermType {
		TT_INVALID = 0,
		TT_ADM3A = 1,
		TT_ADM5 = 2,
		TT_AIXTERM = 3,
		TT_AAA = 4,
		TT_ANSI = 5,
		TT_AT386 = 6,
		TT_AVATAR = 7,
		TT_BA80 = 8,
		TT_BEOS = 9,
		TT_DG200 = 10,
		TT_DG210 = 11,
		TT_DG217 = 12,
		TT_H19 = 13,
		TT_HFT = 14,
		TT_HP2621 = 15,
		TT_HPTERM = 16,
		TT_HZL1500 = 17,
		TT_IBM31 = 18,
		TT_LINUX = 19,
		TT_QANSI = 20,
		TT_QNX = 21,
		TT_SCOANSI = 22,
		TT_97801 = 23,
		TT_SUN = 24,
		TT_TEK40 = 25,		// not yet supported
		TT_NONE = 26,
		TT_TVI910 = 27,
		TT_TVI925 = 28,
		TT_TVI950 = 29,
		TT_VC4404 = 30,
		TT_VIP7809 = 31,
		TT_VT100 = 32,
		TT_VT102 = 33,
		TT_VT220 = 34,
		TT_VT220PC = 35,
		TT_VT320 = 36,
		TT_VT320PC = 37,
		TT_VT420 = 38,		// not yet supported
		TT_VT420PC = 39,	// not yet supported
		TT_VT520 = 40,		// not yet supported
		TT_VT520PC = 41,	// not yet supported
		TT_VT52 = 42,
		TT_VTNT = 43,
		TT_WY160 = 44,
		TT_WY30 = 45,
		TT_WY370 = 46,
		TT_WY50 = 47,
		TT_WY60 = 48,
		TT_XTERM = 49,		// not yet supported
		TT_K95 = 50
	};

	// Note: number assignments are important! They match
	//       the Win32 console colour codes used in the PROCESSINFO
	//		 dwFillAttribute.
	enum Color {
		COLOR_BLACK = 0,
		COLOR_BLUE = 1,
		COLOR_GREEN = 2,
		COLOR_CYAN = 3,
		COLOR_RED = 4,
		COLOR_MAGENTA = 5,
		COLOR_BROWN = 6,
		COLOR_LIGHT_GRAY = 7,
		COLOR_DARK_GRAY = 8,
		COLOR_LIGHT_BLUE = 9,
		COLOR_LIGHT_GREEN = 10,
		COLOR_LIGHT_CYAN = 11,
		COLOR_LIGHT_RED = 12,
		COLOR_LIGHT_MAGENTA = 13,
		COLOR_YELLOW = 14,
		COLOR_WHITE = 15,
		COLOR_INVALID = 16
	};

	typedef struct tagTermInfo {
		TermType type;
		LPCTSTR keyword;
		LPCTSTR name;
		int descriptionId;
	} TermInfo;

	typedef struct tagTermSettings {
		TermType type;
		Charset::Charset charset;
		BOOL statusLine;
		BOOL is8Bit;
		int height;
		Color termFg;
		Color termBg;
		Color underlineFg;
		Color underlineBg;
	} TermSettings;

	typedef struct tagColorInfo {
		Color color;
		LPCTSTR keyword;
		LPCTSTR name;
	} ColorInfo;

	const TermInfo* getTerminalInfos();

	LPCTSTR getTermKeyword(TermType type);

	LPCTSTR getTermName(TermType type);

	TermSettings getTermSettings(TermType type);

	ColorInfo getColorInfo(Color color);

	const ColorInfo* getColorInfos();
}

#endif TERM_INFO_H