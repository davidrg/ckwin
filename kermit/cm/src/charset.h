#ifndef CHARSET_H
#define CHARSET_H

#include <windows.h>

namespace Charset {

	enum Charset {
		CS_INVALID = 0,
		CS_APL4 = 1,
		CS_APL5 = 2,
		CS_APL2 = 3,
		CS_APL1 = 4,
		CS_APL3 = 5,
		CS_8859_6 = 6,
		CS_ASCII = 7,
		CS_BRITISH = 8,
		CS_CN_FRENCH = 9,
		CS_CP856 = 10,
		CS_CP10000 = 11,
		CS_CP1051 = 12,
		CS_CP1089 = 13,
		CS_CP1250 = 14,
		CS_CP1251 = 15,
		CS_CP1251_CYR = 16,
		CS_CP1252 = 17,
		CS_CP1253 = 18,
		CS_CP1254 = 19,
		CS_CP1255 = 20,
		CS_CP1256 = 21,
		CS_CP1257 = 22,
		CS_CP1258 = 23,
		CS_CP437 = 24,
		CS_CP813 = 25,
		CS_CP819 = 26,
		CS_CP850 = 27,
		CS_CP852 = 28,
		CS_CP855 = 29,
		CS_CP855_CYR = 30,
		CS_CP857 = 31,
		CS_CP858 = 32,
		CS_CP862 = 33,
		CS_CP864 = 34,
		CS_CP866 = 35,
		CS_CP866_CYR = 36,
		CS_CP869 = 37,
		CS_CP869_GRE = 38,
		CS_CP912 = 39,
		CS_CP913 = 40,
		CS_CP914 = 41,
		CS_CP915 = 42,
		CS_CP916 = 43,
		CS_CP920 = 44,
		CS_CP923 = 45,
		CS_8859_5 = 46,
		CS_DANISH = 47,
		CS_DECMCS = 48,
		CS_DECSPEC = 49,
		CS_DECTECH = 50,
		CS_DGI = 51,
		CS_DGLDGRPH = 52,
		CS_DGPCGRPH = 53,
		CS_DGWPGRPH = 54,
		CS_DUTCH = 55,
		CS_ELOT927 = 56,
		CS_ELOT928 = 57,
		CS_FINNISH = 58,
		CS_FRENCH = 59,
		CS_GERMAN = 60,
		CS_8859_7 = 61,
		CS_HE7 = 62,
		CS_8859_8 = 63,
		CS_HPLINE = 64,
		CS_HPMATH = 65,
		CS_HPR8 = 66,
		CS_HUNGARIAN = 67,
		CS_ITALIAN = 68,
		CS_J201R = 69,
		CS_J201K = 70,
		CS_JDEC = 71,
		CS_JEUC = 72,
		CS_JIS7	= 73,
		CS_KOI8 = 74,
		CS_KOI8R = 75,
		CS_KOI8U = 76,
		CS_8859_1 = 77,
		CS_8859_2 = 78,
		CS_8859_3 = 79,
		CS_8859_4 = 80,
		CS_8859_9 = 81,
		CS_8859_10 = 82,
		CS_8859_15 = 83,
		CS_MACL1 = 84,
		CS_MAZOVIA = 85,
		CS_NEXT = 86,
		CS_NORWEGIAN = 87,
		CS_PORTUGUESE = 88,
		CS_QNXGRPH = 89,
		CS_KOI7 = 90,
		CS_SHORT_KOI = 91,
		CS_SHJIS = 92,
		CS_SNIBLANK = 93,
		CS_SNIBRACK = 94,
		CS_SNIEURO = 95,
		CS_SNIFACET = 96,
		CS_SNIIBM = 97,
		CS_SPANISH = 98,
		CS_SWEDISH = 99,
		CS_SWISS = 100,
		CS_TRANSP = 101,
		CS_UCS2 = 102,
		CS_UTF8 = 103
	};

	enum CharsetUse {
		CU_TERMINAL,
		CU_FILE,
		CU_TRANSFER,
		CU_FTP,
		CU_PRINT
	};

	typedef struct tagCharsetInfo {
		Charset charset;
		LPCTSTR name;
		LPCTSTR mime;

		// Allowable uses for the various character sets
		unsigned char useTerminal	: 1;
		unsigned char useFile		: 1;
		unsigned char useTransfer	: 1;
		unsigned char useFtp		: 1;
		unsigned char usePrint		: 1;
		//unsigned char reservedA		: 1;
		//unsigned char reservedB		: 1;
		//unsigned char reservedC		: 1;
	} CharsetInfo;


	void populateDropList(HWND hwnd, 
		CharsetUse use, BOOL includeMimeName);

	LPCTSTR getCharsetKeyword(Charset cs);

	// Returns a copy of the charsets name, optionally
	// including its MIME name in brackets (if it has one).
	// The caller is responsible for freeing the returned
	// string.
	LPTSTR getCharsetLabel(Charset cs, BOOL includeMimeName);
}

#endif /* CHARSET_H */