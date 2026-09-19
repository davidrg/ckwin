#include "charset.h"

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

namespace Charset {

static const CharsetInfo charsets[] = {
	//  ID              C-Kermit Name				MIME Name			Term?	File?	Xfer?	Ftp?	Print?
	{	CS_APL4,		TEXT("apl2-ibm"),			TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_APL5,		TEXT("apl-2741"),			TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_APL2,		TEXT("apl-dyadic"),			TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_APL1,		TEXT("apl-iso"),			TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_APL3,		TEXT("apl-plus-2000"),		TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_8859_6,		TEXT("arabic-iso"),			TEXT("ISO-8859-6"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_ASCII,		TEXT("ascii"),				TEXT("US-ASCII"),	TRUE,	TRUE,	TRUE,	TRUE,	TRUE	},
	{	CS_BRITISH,		TEXT("british"),			TEXT("ISO646-GB"),	TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_CN_FRENCH,	TEXT("canadian-french"),	TEXT("ISO646-CA"),	TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_CP856,		TEXT("bulgaria-pc"),		TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_CP10000,		TEXT("cp10000"),			TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP1051,		TEXT("cp1051"),				TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP1089,		TEXT("cp1089"),				TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP1250,		TEXT("cp1250"),				TEXT("WINDOWS-1250"), TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_CP1251,		TEXT("cp1251"),				TEXT("WINDOWS-1251"), TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP1251_CYR,	TEXT("cp1251-cyrillic"),	TEXT(""),			FALSE,	TRUE,	FALSE,	TRUE,	FALSE	},
	{	CS_CP1252,		TEXT("cp1252"),				TEXT("WINDOWS-1252"), TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_CP1253,		TEXT("cp1253"),				TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP1254,		TEXT("cp1254"),				TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP1255,		TEXT("cp1255"),				TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP1256,		TEXT("cp1256"),				TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP1257,		TEXT("cp1257"),				TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP1258,		TEXT("cp1258"),				TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP437,		TEXT("cp437"),				TEXT("IBM437"),		TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_CP813,		TEXT("cp813"),				TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP819,		TEXT("cp819"),				TEXT("ISO-8859-1"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP850,		TEXT("cp850"),				TEXT("IBM850"),		TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_CP852,		TEXT("cp852"),				TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_CP855,		TEXT("cp855"),				TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP855_CYR,	TEXT("cp855-cyrillic"),		TEXT(""),			FALSE,	TRUE,	FALSE,	TRUE,	FALSE	},
	{	CS_CP857,		TEXT("cp857"),				TEXT("ISO-8859-5"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP858,		TEXT("cp858"),				TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_CP862,		TEXT("cp862-hebrew"),		TEXT("IBM862"),		TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_CP864,		TEXT("cp864"),				TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP866,		TEXT("cp866"),				TEXT("IBM866"),		TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP866_CYR,	TEXT("cp866-cyrillic"),		TEXT(""),			FALSE,	TRUE,	FALSE,	TRUE,	FALSE	},
	{	CS_CP869,		TEXT("cp869"),				TEXT("IBM869"),		TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP869_GRE,	TEXT("cp869-greek"),		TEXT("cp869-greek"),FALSE,	TRUE,	FALSE,	TRUE,	FALSE	},
	{	CS_CP912,		TEXT("cp912"),				TEXT("ISO-8859-2"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP913,		TEXT("cp913"),				TEXT("ISO-8859-3"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP914,		TEXT("cp914"),				TEXT("ISO-8859-4"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP915,		TEXT("cp915"),				TEXT("ISO-8859-5"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP916,		TEXT("cp916"),				TEXT("ISO-8859-8"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP920,		TEXT("cp920"),				TEXT("ISO-8859-9"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_CP923,		TEXT("cp923"),				TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_8859_5,		TEXT("cyrillic-iso"),		TEXT("ISO 8859-5"),	TRUE,	TRUE,	TRUE,	TRUE,	TRUE	},
	{	CS_DANISH,		TEXT("danish"),				TEXT("ISO646-DK"),	TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_DECMCS,		TEXT("dec-multinational"),	TEXT("DEC-MCS"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_DECTECH,		TEXT("dec-technical"),		TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_DGI,			TEXT("dg-international"),	TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_DGLDGRPH,	TEXT("dg-linedrawing"),		TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_DGPCGRPH,	TEXT("dg-specialgraphcs"),	TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	//  ID              C-Kermit Name				MIME Name			Term?	File?	Xfer?	Ftp?	Print?
	{	CS_DGWPGRPH,	TEXT("dg-wordprocessing"),	TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_DUTCH,		TEXT("dutch"),				TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_ELOT927,		TEXT("elot927-greek"),		TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_ELOT928,		TEXT("elot928-greek"),		TEXT("ISO-8859-7"),	TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_FINNISH,		TEXT("finnish"),			TEXT("ISO646-FI"),	TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_FRENCH,		TEXT("french"),				TEXT("ISO646-FR"),	TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_GERMAN,		TEXT("german"),				TEXT("ISO646-DE"),	TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_8859_7,		TEXT("greek-iso"),			TEXT("ISO-8859-7"),	TRUE,	TRUE,	TRUE,	TRUE,	TRUE	},
	{	CS_HE7,			TEXT("hebrew-7"),			TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_8859_8,		TEXT("hebrew-iso"),			TEXT("ISO-8859-8"),	TRUE,	TRUE,	TRUE,	TRUE,	TRUE	},
	{	CS_HPLINE,		TEXT("hp-line-drawing"),	TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_HPMATH,		TEXT("hp-math/technical"),	TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_HPR8,		TEXT("hp-roman8"),			TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_HUNGARIAN,	TEXT("hungarian"),			TEXT("ISO646-HU"),	TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_ITALIAN,		TEXT("italian"),			TEXT("ISO646-IT"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_J201R,		TEXT("japanese-roman"),		TEXT("ISO646-JP"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_J201K,		TEXT("katakana"),			TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_JDEC,		TEXT("dec-kanji"),			TEXT(""),			FALSE,	TRUE,	FALSE,	TRUE,	FALSE	},
	{	CS_JEUC,		TEXT("euc-jp"),				TEXT("EUC-JP"),		FALSE,	TRUE,	TRUE,	TRUE,	FALSE	},
	{	CS_JIS7,		TEXT("jis7-kanji"),			TEXT(""),			FALSE,	TRUE,	FALSE,	TRUE,	FALSE	},
	{	CS_KOI8,		TEXT("koi8"),				TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_KOI8R,		TEXT("koi8r"),				TEXT("KOI8-R"),		TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_KOI8U,		TEXT("koi8u"),				TEXT("KOI8-U"),		TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_8859_1,		TEXT("latin1-iso"),			TEXT("ISO-8859-1"),	TRUE,	TRUE,	TRUE,	TRUE,	TRUE	},
	{	CS_8859_2,		TEXT("latin2-iso"),			TEXT("ISO-8859-2"),	TRUE,	TRUE,	TRUE,	TRUE,	TRUE	},
	{	CS_8859_3,		TEXT("latin3-iso"),			TEXT("ISO-8859-3"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_8859_4,		TEXT("latin4-iso"),			TEXT("ISO 8859-4"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_8859_9,		TEXT("latin5-iso"),			TEXT("ISO-8859-9"),	TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_8859_10,		TEXT("latin6-iso"),			TEXT("ISO-8859-10"),TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_8859_15,		TEXT("latin9-iso"),			TEXT("ISO-8859-15"),TRUE,	FALSE,	TRUE,	TRUE,	TRUE	},
	{	CS_MACL1,		TEXT("macintosh-latin"),	TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_MAZOVIA,		TEXT("mazovia-pc"),			TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_NEXT,		TEXT("next-multinational"),	TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_NORWEGIAN,	TEXT("norwegian"),			TEXT("ISO646-NO"),	TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_PORTUGUESE,	TEXT("portuguese"),			TEXT("ISO646-PT"),	TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_QNXGRPH,		TEXT("qnx-console"),		TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_KOI7,		TEXT("koi7"),				TEXT(""),			FALSE,	TRUE,	FALSE,	TRUE,	FALSE	},	
	{	CS_SHORT_KOI,	TEXT("short-koi"),			TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_SHJIS,		TEXT("shift-jis"),			TEXT("SHIFT_JIS"),	FALSE,	TRUE,	FALSE,	TRUE,	FALSE	},
	{	CS_SNIBLANK,	TEXT("sni-blanks"),			TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_SNIBRACK,	TEXT("sni-brackets"),		TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_SNIEURO,		TEXT("sni-euro"),			TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_SNIFACET,	TEXT("sni-facet"),			TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_SNIIBM,		TEXT("sni-ibm"),			TEXT(""),			TRUE,	FALSE,	FALSE,	FALSE,	TRUE	},
	{	CS_SPANISH,		TEXT("spanish"),			TEXT("ISO646-ES"),	TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_SWEDISH,		TEXT("swedish"),			TEXT("ISO646-SE"),	TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_SWISS,		TEXT("swiss"),				TEXT(""),			TRUE,	TRUE,	FALSE,	TRUE,	TRUE	},
	{	CS_TRANSP,		TEXT("transparent"),		TEXT(""),			TRUE,	FALSE,	TRUE,	FALSE,	TRUE	},
	{	CS_UCS2,		TEXT("ucs2"),				TEXT("UCS-2"),		FALSE,	TRUE,	TRUE,	TRUE,	FALSE	},
	{	CS_UTF8,		TEXT("utf8"),				TEXT("UTF-8"),		TRUE,	TRUE,	TRUE,	TRUE,	TRUE	},
	{	CS_INVALID,		0,							0,					0,		0,		0,		0,		0		}
};



void populateDropList(HWND hwnd, CharsetUse use, BOOL includeMimeName) {

	for (int i = 0; charsets[i].charset != CS_INVALID; i++) {
		CharsetInfo cs = charsets[i];

		if ((use == CU_TERMINAL && cs.useTerminal) ||
			(use == CU_FILE && cs.useFile) ||
			(use == CU_TRANSFER && cs.useTransfer) ||
			(use == CU_FTP && cs.useFtp) ||
			(use == CU_PRINT && cs.usePrint)) {

			int idx;

			if (includeMimeName && cs.mime[0] != TEXT('\0')) {
				// +4 to len for " (", ")" and null termination
				int len = _tcsclen(cs.name) + _tcsclen(cs.mime) + 4;

				LPTSTR buf = (LPTSTR)malloc(sizeof(TCHAR) * len);
				ZeroMemory(buf, sizeof(TCHAR) * len);

				_sntprintf(buf, len-1, TEXT("%s (%s)"), cs.name, cs.mime);

				idx = SendMessage(
					hwnd,
					(UINT) CB_ADDSTRING,
					(WPARAM) 0,
					(LPARAM) buf);

				free(buf);

			} else {
				idx = SendMessage(
					hwnd,
					(UINT) CB_ADDSTRING,
					(WPARAM) 0,
					(LPARAM) cs.name);
			}

			SendMessage(
				hwnd, 
				(UINT)CB_SETITEMDATA,
				(WPARAM)idx,
				(LPARAM)cs.charset);
		}
	}
	

}

LPCTSTR getCharsetKeyword(Charset cs) {
	for (int i = 0; charsets[i].charset != CS_INVALID; i++) {
		if (charsets[i].charset == cs) {
			return charsets[i].name;
		}
	}

	return NULL;
}

LPTSTR getCharsetLabel(Charset cs, BOOL includeMimeName) {
	for (int i = 0; charsets[i].charset != CS_INVALID; i++) {
		if (charsets[i].charset == cs) {
			if (!includeMimeName || charsets[i].mime[0] == TEXT('\0')) {
				int len = _tcsclen(charsets[i].name) +1;

				LPTSTR buf = (LPTSTR)malloc(sizeof(TCHAR) * len);
				ZeroMemory(buf, sizeof(TCHAR) * len);

				lstrcpy(buf, charsets[i].name);

				return buf;
			} else {
				// +4 to len for " (", ")" and null termination
				int len = _tcsclen(charsets[i].name) + _tcsclen(charsets[i].mime) + 4;

				LPTSTR buf = (LPTSTR)malloc(sizeof(TCHAR) * len);
				ZeroMemory(buf, sizeof(TCHAR) * len);

				_sntprintf(buf, len-1, TEXT("%s (%s)"), 
					charsets[i].name, charsets[i].mime);

				return buf;
			}
		}
	}
	return NULL;
}

}