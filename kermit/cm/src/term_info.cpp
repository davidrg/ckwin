#include "term_info.h"

namespace Term {

static const TermInfo terminals[] = {
	{	TT_ADM3A,	TEXT("adm3a"),		TEXT("LSI ADM-3A"),					0	},
	{	TT_ADM5,	TEXT("adm5"),		TEXT("LSI ADM-5"),					0	},
	{	TT_AIXTERM,	TEXT("aixterm"),	TEXT("IBM AIXterm"),				0	},
	{	TT_AAA,		TEXT("annarbor"),	TEXT("AnnArbor"),					0	},
	{	TT_ANSI,	TEXT("ansi-bbs"),	TEXT("ANSI.SYS (BBS)"),				0	},
	{	TT_AT386,	TEXT("at386"),		TEXT("Unixware ANSI"),				0	},
	{	TT_AVATAR,	TEXT("avatar/0+"),	TEXT("AVATAR/0+"),					0	},
	{	TT_BA80,	TEXT("ba80"),		TEXT("Nixdorf BA80"),				0	},
	{	TT_BEOS,	TEXT("beterm"),		TEXT("BeOS Terminal"),				0	},
	{	TT_DG200,	TEXT("dg200"),		TEXT("Data General DASHER 200"),	0	},
	{	TT_DG210,	TEXT("dg210"),		TEXT("Data General DASHER 210"),	0	},
	{	TT_DG217,	TEXT("dg217"),		TEXT("Data General DASHER 217"),	0	},
	{	TT_H19,		TEXT("heath19"),	TEXT("Heath-19"),					0	},
	{	TT_HFT,		TEXT("hft"),		TEXT("IBM High Function Terminal"),	0	},
	{	TT_HP2621,	TEXT("hp2621a"),	TEXT("HP 2621A"),					0	},
	{	TT_HPTERM,	TEXT("hpterm"),		TEXT("HP TERM"),					0	},
	{	TT_HZL1500,	TEXT("hz1500"),		TEXT("Hazeltine 1500"),				0	},
	{	TT_IBM31,	TEXT("ibm3151"),	TEXT("IBM 3101-xx,3161"),			0	},
	{	TT_LINUX,	TEXT("linux"),		TEXT("Linux"),						0	},
	{	TT_QANSI,	TEXT("qansi"),		TEXT("QNX ANSI"),					0	},
	{	TT_QNX,		TEXT("qnx"),		TEXT("QNX Console"),				0	},
	{	TT_SCOANSI,	TEXT("scoansi"),	TEXT("SCO ANSI"),					0	},
	{	TT_97801,	TEXT("sni-97801"),	TEXT("SNI 97801"),					0	},
	{	TT_SUN,		TEXT("sun"),		TEXT("SUN Console"),				0	},
	//{	TT_TEK40,	TEXT("tek4014"),	TEXT(""),							0	},
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

}