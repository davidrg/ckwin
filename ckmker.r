/* Resource file for the MPW Kermit version (Macintosh) */
/* Compile this resource definition with Rez */
/* Created: 10/5/87 Matthias Aebi */
/* Modifications: */
/* 03/23/88 PWP: made just about everything preload (because I */
/* 			object to swaping disks a lot), and purgable (in */
/*			hope of making this run on a 128k Mac */
/* 12/17/87 PWP: added the new KSET and MSET for Kermit */
/* 11/??/87 Paul Placeway: added MultiFinder support */
/* 10/13/87 Added field for protocol version to "About Kermit"	*/
/*			and changed it from DLOG to ALRT. */
/* 10/12/87 Moved KR09 0 declaration to ckmkr2.r to get the */
/*			compilation date on each link operation. */
/* 10/8/87	Changed settings filetype in FREF to KERS; added */
/*			header. Changed "Restore SettingsI" to "Load */
/*			Settings". Removed output port selection from */
/* 			Communications dialog. Made packet length fields */
/*			wider (for long packets. Removed blank cursor re- */
/*			source. (M.Aebi) */
/* 10/7/87	Changed version info to October 1987 (M.Aebi) */

#include "Types.r"		/* To get system types */

/* The KR09 0 resource is created separately by ckmkr2.r */
/* The ... character is \311 and is done that way so no 8 bit chars */
/* are in the file */

resource 'MENU' (1, preload) {
	1,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{	/* array: 2 elements */
		/* [1] */
		"About Kermit\311", noIcon, "", "", plain,
		/* [2] */
		"-", noIcon, "", "", plain
	}
};

resource 'MENU' (2, preload) {
	2,
	textMenuProc,
	0x7FFFFFBB,
	enabled,
	"File",
	{	/* array: 8 elements */
		/* [1] */
		"Load Settings\311", noIcon, noKey, noMark, plain,
		/* [2] */
		"Save Settings\311", noIcon, noKey, noMark, plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain,
		/* [4] */
		"Get file from server\311", noIcon, "G", noMark, plain,
		/* [5] */
		"Send file\311", noIcon, "S", noMark, plain,
		/* [6] */
		"Receive file\311", noIcon, "R", noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Transfer\311", noIcon, "T", "", plain,
		/* [9] */
		"Quit", noIcon, "Q", "", plain
	}
};

resource 'MENU' (3, preload) {
	3,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	"Edit",
	{	/* array: 8 elements */
		/* [1] */
		"Undo", noIcon, "", "", plain,
		/* [2] */
		"-", noIcon, "", "", plain,
		/* [3] */
		"Cut", noIcon, "", "", plain,
		/* [4] */
		"Copy", noIcon, "", "", plain,
		/* [5] */
		"Paste", noIcon, "", "", plain,
		/* [6] */
		"Clear", noIcon, "", "", plain,
		/* [7] */
		/* "-", noIcon, "", "", plain, */
		/* [8] */
		/* "Select All", noIcon, "A", "", plain */
	}
};

resource 'MENU' (4, preload) {
	4,
	textMenuProc,
	0x7FFFFFEF,
	enabled,
	"Settings",
	{	/* array: 12 elements */
		/* [1] */
		"File Defaults\311", noIcon, noKey, noMark, plain,
		/* [2] */
		"Communications\311", noIcon, noKey, noMark, plain,
		/* [3] */
		"Protocol\311", noIcon, noKey, noMark, plain,
		/* [4] */
		"Terminal\311", noIcon, noKey, noMark, plain,
		/* [5] */
		"-", noIcon, noKey, noMark, plain,
		/* [6] */
		"\0x11-Shift-1I\0x11-Shift-9 active", noIcon, noKey,
		    check, plain,
		/* [7] */
		"Menu \0x11-Keys active", noIcon, "M", noMark, plain,
		/* [8] */
		"Set key macros\311", noIcon, noKey, noMark, plain,
		/* [9] */
		"Set modifiers\311", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (5, preload) {
	5,
	textMenuProc,
	0x7FFFDFED,
	enabled,
	"Remote",
	{	/* array: 15 elements */
		/* [1] */
		"Show Response", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"Finish", noIcon, noKey, noMark, plain,
		/* [4] */
		"Bye", noIcon, noKey, noMark, plain,
		/* [5] */
		"-", noIcon, noKey, noMark, plain,
		/* [6] */
		"Cwd\311", noIcon, noKey, noMark, plain,
		/* [7] */
		"Delete file\311", noIcon, noKey, noMark, plain,
		/* [8] */
		"Directory\311", noIcon, noKey, noMark, plain,
		/* [9] */
		"Help\311", noIcon, noKey, noMark, plain,
		/* [10] */
		"Host\311", noIcon, noKey, noMark, plain,
		/* [11] */
		"Space\311", noIcon, noKey, noMark, plain,
		/* [12] */
		"Type\311", noIcon, noKey, noMark, plain,
		/* [13] */
		"Who\311", noIcon, noKey, noMark, plain,
		/* [14] */
		"-", noIcon, noKey, noMark, plain,
		/* [15] */
		"Be a Server", noIcon, "H", noMark, plain
	}
};

resource 'MENU' (6, preload) {
	6,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	"Log",
	{	/* array: 3 elements */
		/* [1] */
		"Session", noIcon, noKey, noMark, plain,
		/* [2] */
		"Dump screen to session log", noIcon, noKey, noMark, plain
#ifdef TLOG
		,
		/* [3] */
		"Transactions", noIcon, noKey, noMark, plain
#endif
	}
};

resource 'WIND' (1000, preload) {
	{40, 5, 335, 505},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	"Mac Kermit"
};

resource 'WIND' (1001, preload) {
	{45, 10, 315, 490},
	documentProc,
	invisible,
	goAway,
	0x0,
	"Remote Response"
};

/* vertical scroll bar */
resource 'CNTL' (1000, preload) {
	{-1, 395, 236, 411},
	0,
	visible,
	0,
	0,
	scrollBarProc,
	0,
	""
};

/* horizontal scroll bar */
resource 'CNTL' (1001, preload) {
	{235, -1, 251, 396},
	0,
	visible,
	0,
	0,
	scrollBarProc,
	0,
	""
};

resource 'ALRT' (1009, preload, purgeable) {
	{40, 55, 285, 460},
	1009,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, silent
	}
};

resource 'ALRT' (1010, preload, purgeable) {
	{113, 121, 222, 378},
	1010,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

#ifdef DEBUG
resource 'ALRT' (1011, preload, purgeable) {
	{60, 81, 180, 431},
	1011,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};
#endif

resource 'ALRT' (1012, preload, purgeable) {
	{108, 147, 212, 372},
	1012,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'ALRT' (1013, preload, purgeable) {
	{34, 16, 318, 494},
	1013,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, silent
	}
};

/* DLOG 230 - Status display (during transfer) */
resource 'DLOG' (230, preload, purgeable) {
	{60, 60, 288, 450},
	noGrowDocProc,		/* PWP: was dBoxProc */
	visible,
	noGoAway,
	0x0,
	230,
	"File Transfer Status"
};

/* About Kermit */
resource 'DLOG' (257, preload, purgeable) {
	{40, 28, 328, 454},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	257,
	""
};

resource 'DLOG' (1001, preload, purgeable) {
	{50, 60, 250, 455},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1001,
	""
};

resource 'DLOG' (1002, preload, purgeable) {
	{40, 65, 325, 470},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1002,
	""
};

resource 'DLOG' (1003, preload, purgeable) {
	{50, 37, 192, 415},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1003,
	""
};

resource 'DLOG' (1004, preload, purgeable) {
	{55, 85, 305, 430},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1004,
	""
};

resource 'DLOG' (1005, preload, purgeable) {
	{34, 50, 290, 456},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1005,
	""
};

resource 'DLOG' (1006, preload, purgeable) {
	{70, 75, 240, 435},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1006,
	""
};

resource 'DLOG' (1007, preload, purgeable) {
	{120, 100, 220, 410},
	plainDBox,
	visible,
	noGoAway,
	0x0,
	1007,
	""
};

resource 'DLOG' (1008, preload, purgeable) {
	{30, 14, 250, 494},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1008,
	""
};

resource 'DLOG' (3998, preload, purgeable) {
	{70, 77, 203, 360},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	3998,
	""
};

resource 'DLOG' (3999, preload, purgeable) {
	{54, 68, 304, 418},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	3999,
	""
};

resource 'DLOG' (4000, preload, purgeable) {
	{60, 55, 315, 455},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	4000,
	""
};

resource 'DITL' (230, preload, purgeable) {
	{	/* array DITLarray: 21 elements */
		/* [1] */
		{184, 88, 216, 180},
		Button {
			enabled,
			"Cancel File"
		},
		/* [2] */
		{184, 192, 216, 284},
		Button {
			enabled,
			"Cancel Group"
		},
		/* [3] */
		{8, 8, 24, 77},
		StaticText {
			disabled,
			"Receiving"
		},
		/* [4] */
		{8, 80, 25, 268},
		StaticText {
			disabled,
			""
		},
		/* [5] */
		{32, 48, 49, 74},
		StaticText {
			disabled,
			"As"
		},
		/* [6] */
		{32, 80, 49, 268},
		StaticText {
			disabled,
			""
		},
		/* [7] */
		{56, 8, 73, 62},
		StaticText {
			disabled,
			"KBytes:"
		},
		/* [8] */
		{56, 63, 73, 126},
		StaticText {
			disabled,
			""
		},
		/* [9] */
		{56, 136, 73, 196},
		StaticText {
			disabled,
			"Packets:"
		},
		/* [10] */
		{56, 200, 73, 256},
		StaticText {
			disabled,
			""
		},
		/* [11] */
		{56, 272, 73, 327},
		StaticText {
			disabled,
			"Retries:"
		},
		/* [12] */
		{56, 328, 73, 375},
		StaticText {
			disabled,
			""
		},
		/* [13] */
		{32, 288, 49, 357},
		StaticText {
			disabled,
			""
		},
		/* [14] */
		{8, 288, 24, 379},
		StaticText {
			disabled,
			""
		},
		/* [15] */
		{136, 24, 170, 355},
		StaticText {
			disabled,
			"Emergency exit: hold down \0x11 and type a "
			"period."
		},
		/* [16] */
		{80, 136, 96, 221},
		StaticText {
			disabled,
			"Packet size:"
		},
		/* [17] */
		{80, 223, 96, 263},
		StaticText {
			disabled,
			"0000"
		},
		/* [18] */
		{80, 272, 96, 344},
		StaticText {
			disabled,
			"Checksum:"
		},
		/* [19] */
		{80, 346, 96, 366},
		StaticText {
			disabled,
			"0"
		},
		/* [20] */
		{80, 8, 96, 99},
		StaticText {
			disabled,
			"Window size:"
		},
		/* [21] */
		{80, 100, 96, 121},
		StaticText {
			disabled,
			"00"
		}
	}
};

/* About Kermit */
resource 'DITL' (257, preload, purgeable) {
	{	/* array DITLarray: 25 elements */
		/* [1] */
		{248, 312, 272, 376},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{8, 16, 24, 410},
		StaticText {
			disabled,
			""
		},
		/* [3] */
		{26, 56, 42, 410},
		StaticText {
			disabled,
			""
		},
		/* [4] */
		{192, 176, 208, 248},
		StaticText {
			disabled,
			""
		},
		/* [5] */
		{208, 176, 224, 248},
		StaticText {
			disabled,
			""
		},
		/* [6] */
		{224, 176, 240, 248},
		StaticText {
			disabled,
			""
		},
		/* [7] */
		{240, 176, 256, 248},
		StaticText {
			disabled,
			""
		},
		/* [8] */
		{192, 42, 208, 162},
		StaticText {
			disabled,
			"Transaction time (seconds)"
		},
		/* [9] */
		{208, 16, 224, 160},
		StaticText {
			disabled,
			"Characters from line"
		},
		/* [10] */
		{224, 34, 240, 162},
		StaticText {
			disabled,
			"Characters to line"
		},
		/* [11] */
		{240, 17, 256, 161},
		StaticText {
			disabled,
			"Effective characters"
		},
		/* [12] */
		{44, 56, 60, 410},
		StaticText {
			disabled,
			")1986, 1988 Columbia University, "
			"All Rights Reserved"
		},
		/* [13] */
		{26, 16, 58, 48},
		Icon {
			disabled,
			1000
		},
		/* [14] */
		{72, 80, 104, 112},
		Icon {
			disabled,
			2000
		},
		/* [15] */
		{104, 48, 120, 139},
		StaticText {
			disabled,
			"Bill Catchings"
		},
		/* [16] */
		{72, 288, 104, 320},
		Icon {
			disabled,
			2001
		},
		/* [17] */
		{104, 272, 120, 339},
		StaticText {
			disabled,
			"Bill Schilit"
		},
		/* [18] */
		{72, 184, 104, 216},
		Icon {
			disabled,
			2002
		},
		/* [19] */
		{104, 152, 120, 248},
		StaticText {
			disabled,
			"Frank da Cruz"
		},
		/* [20] */
		{160, 48, 176, 144},
		StaticText {
			disabled,
			"Matthias Aebi"
		},
		/* [21] */
		{128, 80, 160, 112},
		Icon {
			disabled,
			2003
		},
		/* [22] */
		{128, 184, 160, 216},
		Icon {
			disabled,
			2004
		},
		/* [23] */
		{160, 152, 177, 251},
		StaticText {
			disabled,
			"Paul Placeway"
		},
		/* [24] */
		{256, 24, 272, 160},
		StaticText {
			disabled,
			"Effective baud rate"
		},
		/* [25] */
		{256, 176, 272, 248},
		StaticText {
			disabled,
			""
		}
	}
};

resource 'DITL' (1001, preload, purgeable) {
	{	/* array DITLarray: 20 elements */
		/* [1] */
		{165, 105, 185, 185},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{165, 205, 185, 285},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{5, 5, 21, 172},
		StaticText {
			disabled,
			"Communications Settings"
		},
		/* [4] */
		{35, 5, 50, 75},
		StaticText {
			disabled,
			"Baud Rate"
		},
		/* [5] */
		{35, 80, 50, 130},
		RadioButton {
			enabled,
			"300"
		},
		/* [6] */
		{35, 155, 50, 205},
		RadioButton {
			enabled,
			"600"
		},
		/* [7] */
		{35, 230, 50, 280},
		RadioButton {
			enabled,
			"1200"
		},
		/* [8] */
		{35, 305, 50, 355},
		RadioButton {
			enabled,
			"1800"
		},
		/* [9] */
		{55, 80, 70, 130},
		RadioButton {
			enabled,
			"2400"
		},
		/* [10] */
		{55, 155, 70, 205},
		RadioButton {
			enabled,
			"4800"
		},
		/* [11] */
		{55, 230, 70, 280},
		RadioButton {
			enabled,
			"7200"
		},
		/* [12] */
		{55, 305, 70, 355},
		RadioButton {
			enabled,
			"9600"
		},
		/* [13] */
		{75, 80, 90, 140},
		RadioButton {
			enabled,
			"19.2K"
		},
		/* [14] */
		{75, 155, 90, 215},
		RadioButton {
			enabled,
			"57.6K"
		},
		/* [15] */
		{125, 5, 141, 48},
		StaticText {
			disabled,
			"Parity"
		},
		/* [16] */
		{125, 255, 141, 312},
		RadioButton {
			enabled,
			"Mark"
		},
		/* [17] */
		{125, 319, 141, 384},
		RadioButton {
			enabled,
			"Space"
		},
		/* [18] */
		{125, 80, 141, 130},
		RadioButton {
			enabled,
			"Even"
		},
		/* [19] */
		{125, 140, 141, 190},
		RadioButton {
			enabled,
			"Odd"
		},
		/* [20] */
		{125, 197, 141, 251},
		RadioButton {
			enabled,
			"None"
		}
	}
};

resource 'DITL' (1002, preload, purgeable) {
	{	/* array DITLarray: 35 elements */
		/* [1] */
		{225, 320, 245, 390},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{255, 320, 275, 390},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{5, 5, 21, 182},
		StaticText {
			disabled,
			"Protocol Settings"
		},
		/* [4] */
		{35, 5, 50, 135},
		StaticText {
			disabled,
			"Block Check Type"
		},
		/* [5] */
		{35, 144, 50, 199},
		RadioButton {
			enabled,
			"1"
		},
		/* [6] */
		{35, 207, 50, 262},
		RadioButton {
			enabled,
			"2"
		},
		/* [7] */
		{35, 271, 50, 326},
		RadioButton {
			enabled,
			"3"
		},
		/* [8] */
		{57, 5, 72, 135},
		StaticText {
			disabled,
			"Handshake"
		},
		/* [9] */
		{57, 332, 72, 387},
		RadioButton {
			enabled,
			"Bell"
		},
		/* [10] */
		{57, 144, 72, 199},
		RadioButton {
			enabled,
			"CR"
		},
		/* [11] */
		{57, 271, 72, 326},
		RadioButton {
			enabled,
			"ESC"
		},
		/* [12] */
		{57, 207, 72, 262},
		RadioButton {
			enabled,
			"LF"
		},
		/* [13] */
		{79, 271, 94, 326},
		RadioButton {
			enabled,
			"None"
		},
		/* [14] */
		{79, 144, 94, 199},
		RadioButton {
			enabled,
			"XON"
		},
		/* [15] */
		{79, 207, 94, 261},
		RadioButton {
			enabled,
			"XOFF"
		},
		/* [16] */
		{114, 237, 146, 313},
		StaticText {
			disabled,
			"SEND (outbound)"
		},
		/* [17] */
		{113, 156, 146, 224},
		StaticText {
			disabled,
			"RECEIVE (inbound)"
		},
		/* [18] */
		{152, 11, 168, 148},
		StaticText {
			disabled,
			"Start of packet char"
		},
		/* [19] */
		{152, 180, 168, 200},
		EditText {
			enabled,
			""
		},
		/* [20] */
		{173, 21, 189, 148},
		StaticText {
			disabled,
			"End of packet char"
		},
		/* [21] */
		{173, 180, 189, 200},
		EditText {
			enabled,
			""
		},
		/* [22] */
		{195, 87, 212, 148},
		StaticText {
			disabled,
			"Pad char"
		},
		/* [23] */
		{194, 180, 210, 200},
		EditText {
			enabled,
			""
		},
		/* [24] */
		{215, 92, 232, 148},
		StaticText {
			disabled,
			"Padding"
		},
		/* [25] */
		{215, 180, 231, 200},
		EditText {
			enabled,
			""
		},
		/* [26] */
		{236, 11, 252, 148},
		StaticText {
			disabled,
			"Seconds for timeout "
		},
		/* [27] */
		{236, 180, 252, 200},
		EditText {
			enabled,
			""
		},
		/* [28] */
		{258, 53, 274, 148},
		StaticText {
			disabled,
			"Packet length"
		},
		/* [29] */
		{257, 180, 273, 220},
		EditText {
			enabled,
			""
		},
		/* [30] */
		{152, 260, 168, 280},
		EditText {
			enabled,
			""
		},
		/* [31] */
		{173, 260, 189, 280},
		EditText {
			enabled,
			""
		},
		/* [32] */
		{194, 260, 210, 280},
		EditText {
			enabled,
			""
		},
		/* [33] */
		{215, 260, 231, 280},
		EditText {
			enabled,
			""
		},
		/* [34] */
		{236, 260, 252, 280},
		EditText {
			enabled,
			""
		},
		/* [35] */
		{257, 260, 273, 300},
		EditText {
			enabled,
			""
		}
	}
};

resource 'DITL' (1003, preload, purgeable) {
	{	/* array DITLarray: 8 elements */
		/* [1] */
		{83, 100, 113, 185},
		Button {
			enabled,
			"Do It"
		},
		/* [2] */
		{83, 203, 113, 288},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{5, 8, 21, 298},
		StaticText {
			disabled,
			"Remote command is ^0"
		},
		/* [4] */
		{31, 8, 47, 83},
		StaticText {
			disabled,
			"^1"
		},
		/* [5] */
		{31, 90, 47, 365},
		EditText {
			enabled,
			""
		},
		/* [6] */
		{56, 8, 72, 83},
		StaticText {
			disabled,
			"^2"
		},
		/* [7] */
		{57, 90, 73, 365},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{120, 30, 136, 352},
		StaticText {
			disabled,
			"Emergency exit: hold down \0x11 and type a "
			"period."
		}
	}
};

resource 'DITL' (1004, preload, purgeable) {
	{	/* array DITLarray: 15 elements */
		/* [1] */
		{222, 79, 242, 159},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{222, 190, 242, 270},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{10, 10, 26, 93},
		StaticText {
			disabled,
			"File Settings"
		},
		/* [4] */
		{45, 10, 65, 325},
		RadioButton {
			enabled,
			"Attended: dialog on each file received."
		},
		/* [5] */
		{75, 10, 95, 330},
		RadioButton {
			enabled,
			"Unattended: with the following defaults."
			".."
		},
		/* [6] */
		{100, 30, 115, 335},
		RadioButton {
			enabled,
			"Supersede existing files of the same name."
		},
		/* [7] */
		{120, 30, 135, 320},
		RadioButton {
			enabled,
			"Create new file names to avoid conflicts."
		},
		/* [8] */
		{175, 25, 190, 70},
		StaticText {
			disabled,
			"Mode:"
		},
		/* [9] */
		{175, 175, 190, 210},
		StaticText {
			disabled,
			"Fork:"
		},
		/* [10] */
		{48, 410, 56, 426},
		StaticText {
			disabled,
			"-"
		},
		/* [11] */
		{175, 220, 190, 270},
		RadioButton {
			enabled,
			"Data"
		},
		/* [12] */
		{190, 220, 205, 300},
		RadioButton {
			enabled,
			"Resource"
		},
		/* [13] */
		{175, 80, 190, 125},
		RadioButton {
			enabled,
			"Text"
		},
		/* [14] */
		{190, 80, 205, 140},
		RadioButton {
			enabled,
			"Binary"
		},
		/* [15] */
		{145, 10, 160, 220},
		CheckBox {
			enabled,
			"Keep partially received files"
		}
	}
};

/* PWP was here... (many times) */
resource 'DITL' (1005, preload, purgeable) {
	{	/* array DITLarray: 15 elements */
		/* [1] */
		{208, 304, 232, 384},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{208, 208, 232, 288},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{10, 10, 26, 130},
		StaticText {
			disabled,
			"Terminal Settings"
		},
		/* [4] */
		{64, 24, 80, 168},
		CheckBox {
			enabled,
			"Auto Wrap around"
		},
		/* [5] */
		{88, 24, 104, 160},
		CheckBox {
			enabled,
			"Auto Repeat Keys"
		},
		/* [6] */
		{40, 208, 56, 336},
		CheckBox {
			enabled,
			"Inverted Screen"
		},
		/* [7] */
		{112, 208, 128, 344},
		CheckBox {
			enabled,
			"Smooth Scrolling"
		},
		/* [8] */
		{40, 24, 56, 152},
		CheckBox {
			enabled,
			"Auto LF with CR"
		},
		/* [9] */
		{64, 208, 80, 304},
		CheckBox {
			enabled,
			"Local Echo"
		},
		/* [10] */
		{136, 208, 152, 380},
		CheckBox {
			enabled,
			"Symbolic Control Chars"
		},
		/* [11] */
		{112, 24, 128, 128},
		CheckBox {
			enabled,
			"Block Cursor"
		},
		/* [12] */
		{88, 208, 104, 368},
		CheckBox {
			enabled,
			"Mouse -> Arrow Keys"
		},
		/* [13] */
		{160, 208, 176, 304},
		CheckBox {
			enabled,
			"Visible Bell"
		},
		/* [14] */
		{160, 24, 176, 200},
		CheckBox {
			enabled,
			"Extended Character Set"
		},
		/* [15] */
		{136, 24, 152, 144},
		CheckBox {
			enabled,
			"Blinking Cursor"
		},
		/* [16] */
		{208, 24, 232, 136},
		Button {
			enabled,
			"Reset Terminal"
		}
	}
};


resource 'DITL' (1006, preload, purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{115, 235, 135, 315},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{0, 0, 0, 0},
		UserItem {
			disabled
		},
		/* [3] */
		{20, 15, 35, 345},
		StaticText {
			enabled,
			""
		}
	}
};

resource 'DITL' (1007, preload, purgeable) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{65, 210, 85, 290},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{65, 20, 85, 100},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{15, 15, 50, 295},
		EditText {
			enabled,
			""
		},
		/* [4] */
		{65, 115, 85, 195},
		Button {
			enabled,
			"Help"
		}
	}
};

resource 'DITL' (1008, preload, purgeable) {
	{	/* array DITLarray: 51 elements */
		/* [1] */
		{184, 296, 208, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{184, 112, 208, 184},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{72, 320, 88, 340},
		CheckBox {
			enabled,
			""
		},
		/* [4] */
		{72, 288, 88, 308},
		CheckBox {
			enabled,
			""
		},
		/* [5] */
		{72, 256, 88, 276},
		CheckBox {
			enabled,
			""
		},
		/* [6] */
		{72, 200, 88, 220},
		CheckBox {
			enabled,
			""
		},
		/* [7] */
		{72, 144, 88, 164},
		CheckBox {
			enabled,
			""
		},
		/* [8] */
		{72, 112, 88, 132},
		CheckBox {
			enabled,
			""
		},
		/* [9] */
		{72, 80, 88, 100},
		CheckBox {
			enabled,
			""
		},
		/* [10] */
		{72, 48, 88, 68},
		CheckBox {
			enabled,
			""
		},
		/* [11] */
		{72, 16, 88, 36},
		CheckBox {
			enabled,
			""
		},
		/* [12] */
		{96, 320, 112, 340},
		CheckBox {
			enabled,
			""
		},
		/* [13] */
		{96, 288, 112, 308},
		CheckBox {
			enabled,
			""
		},
		/* [14] */
		{96, 256, 112, 276},
		CheckBox {
			enabled,
			""
		},
		/* [15] */
		{96, 200, 112, 220},
		CheckBox {
			enabled,
			""
		},
		/* [16] */
		{96, 144, 112, 164},
		CheckBox {
			enabled,
			""
		},
		/* [17] */
		{96, 112, 112, 132},
		CheckBox {
			enabled,
			""
		},
		/* [18] */
		{96, 80, 112, 100},
		CheckBox {
			enabled,
			""
		},
		/* [19] */
		{96, 48, 112, 68},
		CheckBox {
			enabled,
			""
		},
		/* [20] */
		{96, 16, 112, 36},
		CheckBox {
			enabled,
			""
		},
		/* [21] */
		{120, 320, 136, 340},
		CheckBox {
			enabled,
			""
		},
		/* [22] */
		{120, 288, 136, 308},
		CheckBox {
			enabled,
			""
		},
		/* [23] */
		{120, 256, 136, 276},
		CheckBox {
			enabled,
			""
		},
		/* [24] */
		{120, 200, 136, 220},
		CheckBox {
			enabled,
			""
		},
		/* [25] */
		{120, 144, 136, 164},
		CheckBox {
			enabled,
			""
		},
		/* [26] */
		{120, 112, 136, 132},
		CheckBox {
			enabled,
			""
		},
		/* [27] */
		{120, 80, 136, 100},
		CheckBox {
			enabled,
			""
		},
		/* [28] */
		{120, 48, 136, 68},
		CheckBox {
			enabled,
			""
		},
		/* [29] */
		{120, 16, 136, 36},
		CheckBox {
			enabled,
			""
		},
		/* [30] */
		{144, 320, 160, 340},
		CheckBox {
			enabled,
			""
		},
		/* [31] */
		{144, 288, 160, 308},
		CheckBox {
			enabled,
			""
		},
		/* [32] */
		{144, 256, 160, 276},
		CheckBox {
			enabled,
			""
		},
		/* [33] */
		{144, 200, 160, 220},
		CheckBox {
			enabled,
			""
		},
		/* [34] */
		{144, 144, 160, 164},
		CheckBox {
			enabled,
			""
		},
		/* [35] */
		{144, 112, 160, 132},
		CheckBox {
			enabled,
			""
		},
		/* [36] */
		{144, 80, 160, 100},
		CheckBox {
			enabled,
			""
		},
		/* [37] */
		{144, 48, 160, 68},
		CheckBox {
			enabled,
			""
		},
		/* [38] */
		{144, 16, 160, 40},
		CheckBox {
			enabled,
			""
		},
		/* [39] */
		{72, 352, 88, 464},
		EditText {
			enabled,
			""
		},
		/* [40] */
		{96, 352, 112, 464},
		EditText {
			enabled,
			""
		},
		/* [41] */
		{120, 352, 136, 464},
		EditText {
			enabled,
			""
		},
		/* [42] */
		{144, 352, 160, 464},
		EditText {
			enabled,
			""
		},
		/* [43] */
		{64, 8, 65, 176},
		UserItem {
			enabled
		},
		/* [44] */
		{64, 176, 168, 177},
		UserItem {
			enabled
		},
		/* [45] */
		{64, 176, 65, 466},
		UserItem {
			enabled
		},
		/* [46] */
		{168, 8, 169, 466},
		UserItem {
			enabled
		},
		/* [47] */
		{40, 8, 56, 162},
		StaticText {
			enabled,
			"Ctrl  Opt  Lock  Shift  \0x11"
		},
		/* [48] */
		{40, 176, 56, 456},
		StaticText {
			enabled,
			"Unmodify  Caps  Ctrl  Meta  Prefix strin"
			"g:"
		},
		/* [49] */
		{16, 8, 32, 160},
		StaticText {
			disabled,
			"Modifier Pattern:   -->"
		},
		/* [50] */
		{16, 176, 32, 472},
		StaticText {
			disabled,
			"Modification:  (default is modified as t"
			"yped)"
		},
		/* [51] */
		{184, 208, 208, 272},
		Button {
			enabled,
			"Help"
		}
	}
};

resource 'DITL' (1009, preload, purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{220, 165, 240, 245},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{10, 5, 105, 405},
		StaticText {
			disabled,
			"Enter the string to be sent to the line "
			"into the edit box and click on OK.\n\nTo d"
			"efine unprintable characters like ESC, B"
			"ell, BS, DEL, I enter a \\ followed by th"
			"e octal representation of the ASCII ch"
			"aracter desired.  To enter a \\ just double it."
		},
		/* [3] */
		{110, 5, 210, 405},
		StaticText {
			disabled,
			"Examples:  BS: \\10   LF: \\12   CR: \\15   "
			"ESC: \\33   DEL: \\177\n\nYou cannot combine "
			"both characters and breaks in a single key macro.  "
			"For a short break "
			"enter \\break or \\shortbreak and for a "
			"long break enter \\longbreak\n"
		}
	}
};

resource 'DITL' (1010, preload, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{65, 20, 94, 63},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{12, 72, 94, 236},
		StaticText {
			disabled,
			"^0 ^1 ^2^3"
		}
	}
};

#ifdef DEBUG
resource 'DITL' (1011, preload, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{90, 13, 110, 83},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{10, 60, 70, 350},
		StaticText {
			disabled,
			"Debug: ^0 ^1 ^2 ^3"
		}
	}
};
#endif

resource 'DITL' (1012, preload, purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{63, 75, 83, 125},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{63, 154, 83, 204},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{10, 75, 46, 216},
		StaticText {
			disabled,
			"Emergency Exit?"
		}
	}
};

resource 'DITL' (1013, preload, purgeable) {
	{	/* array DITLarray: 7 elements */
		/* [1] */
		{251, 192, 275, 272},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{8, 8, 72, 472},
		StaticText {
			disabled,
			"For each row, the left side is a pattern"
			" of modifier keys to match, and the righ"
			"t side is what to do to the character wh"
			"en that pattern is used to produce it.  "
			"The modifier keys are Control, Option, C"
			"aps Lock, Shift, and Command (\0x11). "
		},
		/* [3] */
		{184, 8, 200, 472},
		StaticText {
			disabled,
			"Meta means set the 8th (high or parity) "
			"bit of this character."
		},
		/* [4] */
		{80, 8, 112, 472},
		StaticText {
			disabled,
			"Unmodify means use the code that would h"
			"ave been produced if the same key were t"
			"yped without holding down any modifier k"
			"eys. "
		},
		/* [5] */
		{120, 8, 136, 472},
		StaticText {
			disabled,
			"Caps means pretend that the Caps Lock ke"
			"y was down."
		},
		/* [6] */
		{144, 8, 176, 472},
		StaticText {
			disabled,
			"Ctrl means make this letter into the con"
			"trol character of the same name (actuall"
			"y: use only the low 5 bits)."
		},
		/* [7] */
		{208, 8, 240, 472},
		StaticText {
			disabled,
			"If anything is entered as the Prefix str"
			"ing, it will be sent before the modified"
			" key."
		}
	}
};

resource 'DITL' (3998, preload, purgeable) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{90, 38, 112, 117},
		Button {
			enabled,
			"Get"
		},
		/* [2] */
		{90, 155, 112, 225},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{32, 12, 48, 184},
		StaticText {
			disabled,
			"Get remote file:"
		},
		/* [4] */
		{57, 15, 76, 247},
		EditText {
			enabled,
			""
		},
		/* [5] */
		{11, 216, 43, 248},
		Icon {
			disabled,
			1000
		}
	}
};

resource 'DITL' (3999, preload, purgeable) {
	{	/* array DITLarray: 15 elements */
		/* [1] */
		{215, 16, 235, 86},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{215, 96, 235, 166},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{119, 16, 135, 184},
		StaticText {
			enabled,
			"Receive as:"
		},
		/* [4] */
		{88, 216, 112, 344},
		UserItem {
			enabled
		},
		/* [5] */
		{123, 240, 143, 320},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{150, 240, 170, 320},
		Button {
			enabled,
			"Drive"
		},
		/* [7] */
		{144, 16, 158, 184},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{30, 16, 110, 184},
		UserItem {
			enabled
		},
		/* [9] */
		{36, 262, 68, 294},
		Icon {
			enabled,
			1000
		},
		/* [10] */
		{23, 207, 175, 208},
		UserItem {
			enabled
		},
		/* [11] */
		{168, 112, 184, 170},
		RadioButton {
			enabled,
			"Data"
		},
		/* [12] */
		{185, 112, 202, 194},
		RadioButton {
			enabled,
			"Resource"
		},
		/* [13] */
		{168, 16, 184, 79},
		RadioButton {
			enabled,
			"Text"
		},
		/* [14] */
		{185, 16, 202, 81},
		RadioButton {
			enabled,
			"Binary"
		},
		/* [15] */
		{215, 176, 235, 336},
		Button {
			enabled,
			"Proceed Automatically"
		}
	}
};

resource 'DITL' (4000, preload, purgeable) {
	{	/* array DITLarray: 19 elements */
		/* [1] */
		{51, 174, 69, 254},
		Button {
			enabled,
			"Send"
		},
		/* [2] */
		{148, 488, 166, 568},
		Button {
			enabled,
			"0"
		},
		/* [3] */
		{91, 173, 109, 253},
		Button {
			enabled,
			"Cancel"
		},
		/* [4] */
		{83, 284, 101, 380},
		UserItem {
			disabled
		},
		/* [5] */
		{121, 290, 139, 370},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{154, 291, 172, 371},
		Button {
			enabled,
			"Drive"
		},
		/* [7] */
		{10, 9, 124, 149},
		UserItem {
			enabled
		},
		/* [8] */
		{10, 148, 124, 164},
		UserItem {
			enabled
		},
		/* [9] */
		{10, 270, 246, 271},
		UserItem {
			enabled
		},
		/* [10] */
		{32, 435, 128, 536},
		StaticText {
			disabled,
			"h"
		},
		/* [11] */
		{186, 169, 203, 227},
		RadioButton {
			enabled,
			"Data"
		},
		/* [12] */
		{202, 169, 219, 251},
		RadioButton {
			enabled,
			"Resource"
		},
		/* [13] */
		{186, 49, 203, 112},
		RadioButton {
			enabled,
			"Text"
		},
		/* [14] */
		{202, 49, 220, 114},
		RadioButton {
			enabled,
			"Binary"
		},
		/* [15] */
		{132, 12, 150, 44},
		StaticText {
			disabled,
			"As:"
		},
		/* [16] */
		{52, 455, 148, 556},
		EditText {
			enabled,
			""
		},
		/* [17] */
		{155, 15, 176, 255},
		EditText {
			enabled,
			""
		},
		/* [18] */
		{31, 311, 63, 343},
		Icon {
			disabled,
			1000
		},
		/* [19] */
		{225, 15, 245, 260},
		CheckBox {
			enabled,
			"Send all files of the current folder"
		}
	}
};

resource 'ICON' (1000, preload, purgeable) {
	$"0000 0000 0000 0000 0000 0004 0003 7616"
	$"0005 8D0E 0007 071C 0008 0084 0014 0244"
	$"0016 0644 5011 988C 300C 6318 5FC3 0CF8"
	$"20F7 FFE0 001F 5700 0003 AA00 0001 5600"
	$"0001 AF00 0001 F7C0 0003 83E0 0003 FFE0"
	$"0007 B7E0 0007 AFE0 000F 9F60 000D AF60"
	$"000D B760 001D FF78 007C 0078 007C 0040"
	$"0004 0F40 07E5 0040 FC1C 0040 0007 FFC0",
};

resource 'ICON' (2000, preload, purgeable) {
	$"0000 0000 0003 FE00 001D 1580 0034 46C0"
	$"0051 1160 0044 4720 0091 18D0 00C4 6050"
	$"0111 4050 0145 8028 0316 0028 0249 E3D8"
	$"0312 0038 0250 E5D0 02B0 4490 0360 0410"
	$"0360 0410 0360 0010 03C0 1110 0110 0E10"
	$"0150 0020 00C8 40A0 0048 3F20 0004 0040"
	$"0022 0040 0001 8080 0020 6100 0000 1E"
};

resource 'ICON' (2001, preload, purgeable) {
	$"0000 0000 000F F800 0035 5600 00EA AB80"
	$"0155 5540 01AA AAC0 0355 57E0 02AA ADA0"
	$"0355 78E0 06AB C0B0 057E 00D0 06E0 00F0"
	$"05DE 3DD0 0761 C370 055C 9D50 0748 8970"
	$"0541 C150 0322 2260 029C 1C20 0181 4060"
	$"0180 00C0 0143 E1C0 00C7 F280 00AC 1F80"
	$"007B EB00 0028 8E00 0038 8A00 001F FC00"
	$"000D 5800 0007 E0"
};

resource 'ICON' (2002, preload, purgeable) {
	$"0003 F000 000E 5800 0031 0600 00FA 2F80"
	$"01C0 8EC0 013A 5940 0357 3360 0331 E1B0"
	$"02E0 00B0 0680 00F8 0580 0058 0580 007C"
	$"0D00 002C 0B3E 3E34 0F41 413C 051C 9C48"
	$"0788 8858 0280 8068 0380 8050 0280 00F0"
	$"0341 40C0 01C6 B0C0 00CB 6980 0068 0980"
	$"0077 F700 0030 8600 001D 5C00 000A A800"
	$"0005 5000 0003 E0"
};

resource 'ICON' (2003, preload, purgeable) {
	$"000F C000 001F F800 007F FE00 00FF FF80"
	$"01FF FF80 03FF FFC0 03FF FFE0 03FF FFE0"
	$"07FF FFE0 07FF FFF0 07FC AFF0 0FF8 97F0"
	$"0FE0 41F0 0FC0 00F0 0F1E 38F0 0F3E 7CF0"
	$"0F3E 7CF0 0702 40D0 0780 00D0 0782 41B0"
	$"0380 41E0 03C7 C3E0 03C7 C1E0 01E7 E7C0"
	$"01EF FFC0 00FC 3F80 00FE FF80 007F FF00"
	$"007F FE00 003F FC00 001F F800 0007 E0"
};

resource 'ICON' (2004, preload, purgeable) {
	$"0000 0000 0007 E000 0038 0E00 00F0 0780"
	$"01E0 03C0 01C0 01C0 03C0 0060 0380 00F0"
	$"0380 0078 0700 00F8 0780 0058 0700 0078"
	$"0700 002C 0578 7454 0703 0224 0539 3828"
	$"0311 1048 0302 0028 0102 0050 0102 0030"
	$"0081 8040 0080 0040 0088 1080 0047 E080"
	$"0060 0100 0020 0200 0010 0400 0008 1800"
	$"0007 E0"
};

resource 'ICN#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"0000 0000 0000 0000 0000 0004 0003 7616"
		$"0005 8D0E 0007 071C 0008 0084 0014 0244"
		$"0016 0644 5011 988C 300C 6318 5FC3 0CF8"
		$"20F7 FFE0 001F 5700 0003 AA00 0001 5600"
		$"0001 AF00 0001 F7C0 0003 83E0 0003 FFE0"
		$"0007 B7E0 0007 AFE0 000F 9F60 000D AF60"
		$"000D B760 001D FF78 007C 0078 007C 0040"
		$"0004 0F40 07E5 0040 FC1C 0040 0007 FFC0",
		/* [2] */
		$"0000 0000 0000 0000 0003 FE1E 0007 FF3F"
		$"000F FFBF 000F FFBF 003F FFCE 003F FFEE"
		$"F03F FFDE F83F FFBE FFDF FFFC FFFF FFFC"
		$"FFFF FFFC 71FF FFE0 003F FF00 0007 FF00"
		$"0003 FFC0 0007 FFE0 0007 FFF0 000F 7DF0"
		$"000E FEF0 001E FF70 001D FFF0 001F FFF0"
		$"003F FFF8 007F FFFC 00FF FFFC 00FF FFF8"
		$"007B FFE0 07E7 FFE0 FC1F FFE0 0007 FFE0"
	}
};

resource 'ICN#' (129) {
	{	/* array: 2 elements */
		/* [1] */
		$"1FFF FC00 1000 0600 1000 0500 1000 0480"
		$"1000 0440 1000 0420 1000 07F0 101B B050"
		$"102C 6970 1038 38F0 1040 05D0 10A0 1250"
		$"10B0 3250 108C C4D0 1063 1990 1018 2790"
		$"10CF FE10 10FA B010 110D 6010 120A A010"
		$"130D 6010 170F E010 1200 0010 1AFF FFF0"
		$"1000 0010 17FD 5370 1915 7490 1595 7550"
		$"1315 7530 1D95 57D0 1000 0010 1FFF FFF0",
		/* [2] */
		$"1FFF FC00 1FFF FE00 1FFF FF00 1FFF FF80"
		$"1FFF FFC0 1FFF FFE0 1FFF FFF0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0"
	}
};

resource 'FREF' (128) {
	'APPL',
	0,
	""
};

resource 'FREF' (129) {
	'KERS',
	1,
	""
};

resource 'BNDL' (128) {
	'KR09',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'ICN#',
		{	/* array IDArray: 2 elements */
			/* [1] */
			0, 128,
			/* [2] */
			1, 129
		},
		/* [2] */
		'FREF',
		{	/* array IDArray: 2 elements */
			/* [1] */
			0, 128,
			/* [2] */
			1, 129
		}
	}
};

/* PWP -- is this even used any more???? */
data 'FSET' (2, purgeable) {
	$"1203 1B4F 7003 1B4F 7103 1B4F 7203 1B4F"	/* ...Op..Oq..Or..O */
	$"7303 1B4F 7403 1B4F 7503 1B4F 7603 1B4F"	/* s..Ot..Ou..Ov..O */
	$"7703 1B4F 7803 1B4F 7903 1B4F 6D03 1B4F"	/* w..Ox..Oy..Om..O */
	$"6C03 1B4F 6E03 1B4F 4D02 1B50 021B 5102"	/* l..On..OM..P..Q. */
	$"1B52 021B 5300"                         	/* .R..S. */
};

/* PWP: new KSET and MSET -- does vt100 arrow keys */
data 'KSET' (4, purgeable) {
	$"0000 0000 0000 3000 2284 0000 0000 001E"	/* ......0."...... */
	$"0000 0000 0000 2000 0000 0000 0000 0000"	/* ...... ......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 2000 0000 0000 0000 0000"	/* ...... ......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
};

data 'MSET' (5, purgeable) {
	$"0102 00BC 0045 83FC 0002 6AE2 0000 0000"	/* ...<.E...j..... */
	$"0002 6AE2 0045 0012 00BC 0045 83FC 0002"	/* ..j..E...<.E... */
	$"6AE2 0000 0000 0002 6AE2 0045 0000 00BC"	/* j.......j..E...< */
	$"0045 83FC 0002 6AE2 0000 0000 0002 6AE2"	/* .E...j.......j. */
	$"0045 0000 00BC 0045 83FC 0002 6AE2 0000"	/* .E...<.E...j... */
	$"0000 0002 6AE2 0045 000C 0033 0001 7F00"	/* ....j..E...3.... */
	$"3200 011B 00B2 0001 6000 7E00 031B 4F41"	/* 2....2..`.~...OA */
	$"007D 0003 1B4F 4200 7C00 031B 4F43 007B"	/* .}...OB.|...OC.{ */
	$"0003 1B4F 4400 4600 031B 4F44 004D 0003"	/* ...OD.F...OD.M.. */
	$"1B4F 4100 4800 031B 4F42 0042 0003 1B4F"	/* .OA.H...OB.B...O */
	$"4308 3200 0160 00DC 011E 011F 7FFF 0096"	/* C.2..`......... */
	$"00DC"                                   	/* .. */
};

/* yes, we NEED the FOND resource -- PWP */
data 'FOND' (128, "VT100", preload, purgeable) {
	$"6000 0080 0000 0000 0000 0000 0000 0000"	/* `.............. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0001 0000 0009 0000 4009"          	/* .......F..@F */
};

data 'FOND' (129, "VT100 Bold", preload, purgeable) {
	$"6000 0081 0000 0000 0000 0000 0000 0000"	/* `.............. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0001 0000 0009 0000 4089"          	/* .......F..@	 */
};

data 'FONT' (16393, preload, purgeable) {
	$"9000 0000 00FE 0006 FFD1 FFF7 00FF 000C"	/* ........Q...... */
	$"0376 0009 0003 0000 0034 6186 1861 8618"	/* .v.F.....4a.a. */
	$"6184 1041 0410 4104 10E3 8E38 E38E 38E3"	/* a.A..A...8.8. */
	$"8E38 0000 0000 0000 0000 0000 0000 0000"	/* 8.............. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0404 1000"	/* ................ */
	$"0000 0412 0080 0000 0000 0000 0000 0000"	/* ............... */
	$"0000 9249 2492 4924 924C 30C3 0C30 C30C"	/* ..I$I$L0C.0C. */
	$"3010 4104 1041 0410 4104 0000 0000 0000"	/* 0.A..A..A....... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0380 0000"	/* ............... */
	$"0000 0000 0404 11F8 0000 0412 0080 0000"	/* ............... */
	$"0000 0000 3FC0 8000 0000 9249 2492 4924"	/* ....?@...I$I$ */
	$"9244 1041 0410 4104 1061 8618 6186 1861"	/* D.A..A..a.a.a */
	$"8618 CA89 944C 2000 02E4 EF8B EEFB 9C04"	/* .J	L ....... */
	$"10E7 13CE F7FE E8F3 C611 CBBC EF3B F18C"	/* ...N....F.K<.;. */
	$"631F F8E4 0404 0010 1810 430C 0000 0000"	/* c.........C..... */
	$"1000 0000 0549 095A 7190 624A 8404 1000"	/* .....IFZqbJ... */
	$"0000 0412 0092 0004 7800 0000 3FC0 8000"	/* .......x...?@. */
	$"0000 9249 2492 4924 9244 1041 0410 4104"	/* ..I$I$D.A..A. */
	$"1082 0820 8208 2082 0104 CA9F BA52 A900"	/* .. . ..J:R). */
	$"031D 109A 110C 6208 0918 A931 4C21 18A1"	/* .....b.F.)1L!.! */
	$"4A1B CC63 18C4 918C 6310 C82A 0204 0010"	/* J.Lc.Dc.H*.... */
	$"2430 0104 0000 0000 1000 0000 0555 1CAA"	/* $0...........U.* */
	$"4210 926A 8404 1007 E000 0412 00A1 004A"	/* B.j........!.J */
	$"4800 0000 3FC0 8000 0000 6186 1861 8618"	/* H...?@...a.a. */
	$"618E 38E3 8E38 E38E 38F3 CF3C F3CF 3CF3"	/* a8.8.8.O<.O<. */
	$"CE38 CFE8 6A61 7100 0534 112B D014 6313"	/* N8O.jaq..4.+P.c. */
	$"E41B C530 4C21 08A1 521F AC63 18C0 918C"	/* ..E0L!.!R.,c.@ */
	$"5511 4431 0175 9CD7 21F6 C325 AB3B CFB3"	/* U.D1.uW!.C%+;O3 */
	$"FA51 8C63 F552 3F5E 6210 9FDA 8404 1000"	/* .Qc.R?^b..... */
	$"0000 0412 00C0 FFE8 4800 0000 3FC0 8000"	/* .....@..H...?@. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 829C 4621 27C7"	/* ..........F!'G */
	$"C554 634A 3E23 9E60 022A C5D0 4FBD 0FA1"	/* ETcJ>#`.*EPO=.! */
	$"6215 AC7D 1F38 9155 48A2 4420 000E 6338"	/* b.,}.8UH"D ..c8 */
	$"F239 4345 5CC6 314C 1251 8AA2 2820 3EAA"	/* .9CE\F1L.Q
"( >* */
	$"419C 6249 1FFF FE00 1F80 07FF FFA1 289D"	/* AbI........!( */
	$"4800 0000 3FC0 8000 0000 1823 8E08 F18F"	/* H...?@....#.. */
	$"1861 8238 E08F 18F1 8618 238E 08E1 8F18"	/* .a8....#... */
	$"6182 87CA C9A1 7100 0994 80FC 3124 4213"	/* aJI!q.F.1$B. */
	$"E44B 7D30 4C21 38A1 5211 9C61 1A04 9155"	/* .K}0L!8!R.a..U */
	$"5444 4220 007C 611F A231 43C5 58C6 3143"	/* TDB .|a."1CEXF1C */
	$"9251 A926 4540 1D5A 5C07 0248 00C0 1000"	/* Q)&E@.Z\..H.@.. */
	$"0000 0410 0892 2BE8 4800 0000 0000 0000"	/* .....+.H....... */
	$"0000 2460 4118 8201 2492 4604 1188 2012"	/* ..$`A..$F.. . */
	$"4924 6041 1882 0124 9246 02BD B912 A900"	/* I$`A..$F.=9.). */
	$"0915 088A 3144 6308 0808 4531 4C21 18A9"	/* F..
1Dc...E1L!.) */
	$"4A11 9C61 2944 9127 E248 4220 00CE 6138"	/* J.a)D'.HB .Na8 */
	$"21F1 4325 58C7 CF40 564A FA9A 8540 08A3"	/* !.C%XGO@VJ.@.# */
	$"90C4 0F90 E0C0 1000 007E 0410 0880 290C"	/* D..@...~...). */
	$"4800 0000 3FC0 8000 0000 2421 8628 E382"	/* H...?@...$!(. */
	$"1872 4218 628E 3821 8724 2186 28E3 8218"	/* .rB.b8!$!(.. */
	$"7242 8289 368C 2018 30EF F709 CE43 9C44"	/* rB	6 .0..FNCD */
	$"1047 47CE F7E0 E8F6 47F1 8BA0 D8B8 8E22"	/* .GGN....G. X8" */
	$"A24F F1E0 F87D 9ED7 A031 E31F 18BA 0147"	/* "O...}W 1...:.G */
	$"89E4 5443 F540 0151 18A6 0010 40C0 1000"	/* 	.TC.@.Q.&..@@.. */
	$"0000 0410 08FF C81A 7800 0000 0000 0000"	/* ......H.x....... */
	$"0000 2422 013C 1244 2412 4220 13C1 2442"	/* ..$".<.D$.B .A$B */
	$"4124 2201 3C12 4424 1242 0000 0000 0010"	/* A$".<.D$.B...... */
	$"1000 0000 0000 0040 0000 0000 0000 0000"	/* .......@........ */
	$"0000 0000 0000 0000 0000 0100 0000 0000"	/* ................ */
	$"0220 1200 0002 0100 0000 0022 0380 0001"	/* . ........."... */
	$"10C4 0010 40C0 1000 0001 FC10 0880 0008"	/* .D..@@......... */
	$"0000 0000 0000 0000 0000 1873 CE08 E184"	/* ...........sN.. */
	$"1861 873C E08E 1841 8618 73CE 08E1 8418"	/* .a<..A.sN... */
	$"6187 0000 0000 0020 0000 0000 0000 0080"	/* a..... ....... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 01C0 0C00 0002 0100"	/* .........@...... */
	$"0000 001C 0000 0001 10A4 001C 40C0 1000"	/* .........$..@@.. */
	$"0000 0410 0880 0000 0000 0000 0000 0000"	/* ............... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 00C0 1000 0000 0410 0880 0000"	/* .....@......... */
	$"0000 0000 0000 0000 0000 0000 0006 000C"	/* ................ */
	$"0012 0018 001E 0024 002A 0030 0036 003C"	/* .......$.*.0.6.< */
	$"0042 0048 004E 0054 005A 0060 0066 006C"	/* .B.H.N.T.Z.`.f.l */
	$"0072 0078 007E 0084 008A 0090 0096 009C"	/* .r.x.~..
... */
	$"00A2 00A8 00AE 00B4 00BA 00C0 00C0 00C1"	/* .".(...4.:.@.@.A */
	$"00C5 00CA 00CF 00D4 00D9 00DA 00DD 00E0"	/* .E.J.O.T........ */
	$"00E5 00EA 00ED 00F2 00F3 00F7 00FC 00FF"	/* ................ */
	$"0104 0109 010E 0113 0118 011D 0122 0127"	/* ...F.........".' */
	$"0128 012A 012E 0133 0137 013C 0141 0146"	/* .(.*...3.7.<.A.F */
	$"014B 0150 0155 015A 015F 0164 0169 016C"	/* .K.P.U.Z._.d.i.l */
	$"0171 0176 017B 0180 0185 018A 018F 0194"	/* .q.v.{...
.. */
	$"0199 019E 01A3 01A8 01AD 01B2 01B7 01BC"	/* ...#.(.-.2.7.< */
	$"01C1 01C4 01C8 01CB 01D0 01D5 01D8 01DD"	/* .A.D.H.K.P.U.X.. */
	$"01E2 01E7 01EC 01F1 01F6 01FB 0200 0203"	/* ................ */
	$"0207 020C 020F 0214 0219 021E 0223 0228"	/* .............#.( */
	$"022D 0232 0236 023B 0240 0245 024A 024F"	/* .-.2.6.;.@.E.J.O */
	$"0254 0257 0258 025B 0260 0261 0261 0261"	/* .T.W.X.[.`.a.a.a */
	$"0261 0261 0261 0261 0261 0261 0261 0261"	/* .a.a.a.a.a.a.a.a */
	$"0261 0261 0261 0261 0261 0261 0261 0261"	/* .a.a.a.a.a.a.a.a */
	$"0261 0261 0261 0261 0261 0261 0261 0261"	/* .a.a.a.a.a.a.a.a */
	$"0261 0261 0261 0261 0261 0261 0261 0261"	/* .a.a.a.a.a.a.a.a */
	$"0261 0261 0261 0261 0261 0261 0261 0261"	/* .a.a.a.a.a.a.a.a */
	$"0261 0261 0261 0261 0261 0261 0261 0261"	/* .a.a.a.a.a.a.a.a */
	$"0261 0261 0261 0261 0261 0261 0261 0261"	/* .a.a.a.a.a.a.a.a */
	$"0261 0261 0261 0261 0261 0261 0261 0261"	/* .a.a.a.a.a.a.a.a */
	$"0261 0261 0261 0261 0261 0261 0261 0261"	/* .a.a.a.a.a.a.a.a */
	$"0261 0261 0261 0261 0261 0261 0261 0261"	/* .a.a.a.a.a.a.a.a */
	$"0261 0261 0261 0261 0261 0261 0261 0261"	/* .a.a.a.a.a.a.a.a */
	$"0261 0261 0261 0261 0261 0262 0267 026C"	/* .a.a.a.a.a.b.g.l */
	$"0271 0276 027B 0280 0284 0289 028E 0293"	/* .q.v.{...	.. */
	$"0296 0299 029D 02A1 02A7 02AD 02B3 02B9"	/* ....!.'.-.3.9 */
	$"02BF 02C5 02C9 02CC 02D2 02D8 02D9 02DD"	/* .?.E.I.L.R.X.... */
	$"02E1 02E6 02EB 02EF 02F0 02F6 2F06 2F06"	/* ...........././. */
	$"2F06 2F06 2F06 2F06 2F06 2F06 2F06 2F06"	/* /./././././././. */
	$"2F06 2F06 2F06 2F06 2F06 2F06 2F06 2F06"	/* /./././././././. */
	$"2F06 2F06 2F06 2F06 2F06 2F06 2F06 2F06"	/* /./././././././. */
	$"2F06 2F06 2F06 2F06 2F06 2F06 2606 3106"	/* /./././././.&.1. */
	$"3006 2F06 2F06 2F06 2F06 3106 3006 3006"	/* 0././././.1.0.0. */
	$"2F06 2F06 3006 2F06 3106 3006 2F06 3006"	/* /./.0./.1.0./.0. */
	$"2F06 2F06 2F06 2F06 2F06 2F06 2F06 2F06"	/* /./././././././. */
	$"3106 3006 3006 2F06 2F06 2F06 2F06 2F06"	/* 1.0.0./././././. */
	$"2F06 2F06 2F06 2F06 2F06 2F06 2F06 3006"	/* /././././././.0. */
	$"2F06 2F06 2F06 2F06 2F06 2F06 2F06 2F06"	/* /./././././././. */
	$"2F06 2F06 2F06 2F06 2F06 2F06 2F06 2F06"	/* /./././././././. */
	$"2F06 3006 3006 3006 2F06 2F06 3006 2F06"	/* /.0.0.0././.0./. */
	$"2F06 2F06 2F06 2F06 2F06 2F06 2F06 3006"	/* /././././././.0. */
	$"2F06 2F06 3006 2F06 2F06 2F06 2F06 2F06"	/* /./.0./././././. */
	$"2F06 2F06 2F06 2F06 2F06 2F06 2F06 2F06"	/* /./././././././. */
	$"2F06 3006 3106 3006 2F06 3106 2306 FFFF"	/* /.0.1.0./.1.#... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF 3206 2F06 2F06"	/* ..........2././. */
	$"2F06 2F06 2F06 2F06 2F06 3006 2F06 2F06"	/* /././././.0././. */
	$"2F06 2F06 3106 3106 2F06 2F06 2F06 2F06"	/* /./.1.1././././. */
	$"2F06 2F06 3106 2F06 2F06 2F06 3106 2F06"	/* /./.1./././.1./. */
	$"3006 2F06 2F06 2F06 3106 2F06 0000"     	/* 0./././.1./... */
};

data 'FONT' (16384, "VT100", preload, purgeable) {
};

data 'FONT' (16521, preload, purgeable) {
	$"9000 0000 00FE 0007 FFDA FFF7 00FF 000C"	/* ............... */
	$"0316 0009 0003 0000 002C 0000 0000 0000"	/* ...F.....,...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0001 8061 9F80 0000 6198"	/* ........a..a */
	$"0600 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 007E 0000 0078 0000 0001"	/* .......~...x.... */
	$"8061 9F80 0000 6198 0600 0000 0000 0000"	/* a..a........ */
	$"3FC0 0000 1EF7 8000 0000 F6A2 6D99 E100"	/* ?@........"m.. */
	$"001B 99DF 1FDD F738 0306 1CE2 79DE FFDD"	/* ......8....y... */
	$"BF3F 788C DDE7 79DF EF7B DEFF F390 180C"	/* ??x..y..{..... */
	$"0060 3030 C3C7 0000 0000 0600 0000 00DB"	/* .`00CG.......... */
	$"3413 FB60 EC38 13D9 8061 8000 0000 6198"	/* 4..`.8..a...a */
	$"0678 0118 7800 0000 3FC0 0000 1EF7 8000"	/* .x..x...?@..... */
	$"0000 F7F7 EF5B 3548 001E FB63 3E36 3DEF"	/* .....[5H...c>6=. */
	$"06FB 3797 6F7B C637 B61B 78DE F7BD EF66"	/* ..7o{F76.x..=.f */
	$"6F7B DEFF B1B8 0C0C 0060 6870 00C3 0000"	/* o{..18...`hp.C.. */
	$"0000 0600 0000 00DB 7C3B 7B71 8C6C 9BD9"	/* ........|;{ql. */
	$"8061 807E 0000 6198 06CC 1FB4 4800 0000"	/* a~..a.L.4H... */
	$"3FC0 0000 1EF7 8000 0000 F7FE 1B5E 1FC8"	/* ?@..........^.H */
	$"0036 D866 7FB0 6DEF 6CF9 87FD EF7B C631"	/* .6Xf.0m.l...{F1 */
	$"B61B D8FF F7BD EF06 6F7B 76CD 99EC 06EF"	/* 6.X..=..o{vM... */
	$"39EE 63FD C3DB 7BCE F3FC FF6F 75DE FEDB"	/* 9.c.C.{N...ou... */
	$"587E BF61 8C6C 9FF9 8061 807E 0000 6198"	/* X~?al.a~..a */
	$"0787 FFB0 4800 0000 3FC0 0000 18C6 0000"	/* ..0H...?@...F.. */
	$"0000 C0A7 3986 1BBE 3E37 D8CE DEFC C73C"	/* ..@'9.>>7XN..G< */
	$"7800 CDDD FB1B F7B7 F61B 98FF F7ED F9C6"	/* x.M....7......F */
	$"6F7F 2399 9980 003D FF7B F6F6 C3F3 7F7B"	/* o.#.=.{..C..{ */
	$"DEF7 C66F 7576 CF81 807F 7B60 EF3B FB77"	/* ..FouvO.{`.;.w */
	$"FFFF E001 F800 7FFF FECD E278 4800 0000"	/* .........M.xH... */
	$"3FC0 0000 18C6 0000 0000 C1F3 B366 1FFE"	/* ?@...F....A.3f.. */
	$"3E36 D983 F8F6 CD8C 0CF9 99EF EF7B C633"	/* >6...M....{F3 */
	$"B6DB D8FF F78D EC66 6FFF 71B1 9980 00FD"	/* 6.X..fo.q1.. */
	$"E77F 66F6 C3E3 577B DEF0 E66F 7F26 DCDB"	/* ..f.C.W{...o.&.. */
	$"007E BB1E 0783 F927 FFFF E001 F800 7FFF"	/* .~;...'........ */
	$"FE78 AFB3 4800 0000 0000 0000 0000 0000"	/* .x/3H........... */
	$"0000 01FF 6F43 3548 01E6 DB1B 1EF7 8DEF"	/* ....oC5H...... */
	$"06FB 018D EF7B C63F B6DB 78DE F78F 6F66"	/* ...{F?6.x..of */
	$"6DDB D9BF 8D80 01BD E378 63F6 C3F3 577B"	/* m..?=.xc.C.W{ */
	$"F3F0 76ED DF73 F8DB 003B 6798 E600 8C78"	/* ..v..s...;g..x */
	$"1E01 8000 07E0 6180 6600 AFBB 4800 0000"	/* .....af./;H... */
	$"3FC0 0000 18C6 0000 0000 C0A2 6DA1 E108"	/* ?@...F....@"m!.. */
	$"C1E3 BFEE 1B9D 873B 6306 18ED F9DE FE1B"	/* A.?..;c....... */
	$"BF73 7FDE 5D87 EDC6 3891 D9BF CF83 E0FF"	/* ?s..].F8.?O.. */
	$"3DEF 60F7 FBDF D76E C0F1 E33C 8AD8 FEDB"	/* =.`...Wn@..<
X.. */
	$"0013 E31C D703 EC30 1E01 8000 07E0 6180"	/* ....W..0.....a */
	$"67FF 2474 7800 0000 0000 0000 0000 0000"	/* g.$tx........... */
	$"0000 0000 0000 0000 C000 0000 0000 0000"	/* ........@....... */
	$"6000 0000 0000 0000 0000 0000 0000 0000"	/* `............... */
	$"0000 0000 0003 E000 0000 06C0 1B00 0000"	/* ...........@.... */
	$"C0C0 0000 0006 C07E 0000 0318 E603 EC30"	/* @@....@~.......0 */
	$"1E01 8000 001F E180 67FE 0030 0000 0000"	/* ......g..0.... */
	$"0000 0000 0000 0000 0000 0000 0000 0001"	/* ................ */
	$"8000 0000 0000 0000 C000 0000 0000 0000"	/* .......@....... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0380 0E00 0000 C0C0 0000 0003 8000"	/* .......@@..... */
	$"0000 0318 D600 0F30 1E01 8000 001F E180"	/* ....V..0...... */
	$"6600 0000 0000 0000 0000 0000 0000 0000"	/* f............... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"1E01 8000 0000 6180 6600 0000 0000 0000"	/* .....af....... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"	/* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0002"	/* ................ */
	$"0007 000C 0011 0016 001B 001D 0021 0025"	/* .............!.% */
	$"002A 002F 0032 0037 0039 003D 0042 0046"	/* .*...2.7.9.=.B.F */
	$"004B 0050 0055 005A 005F 0064 0069 006E"	/* .K.P.U.Z._.d.i.n */
	$"0070 0073 0078 007D 0082 0087 008C 0091"	/* .p.s.x.}.... */
	$"0096 009B 00A0 00A5 00AA 00AF 00B4 00B8"	/* ... .%.*./.4.8 */
	$"00BE 00C3 00C8 00CD 00D2 00D7 00DC 00E1"	/* .>.C.H.M.R.W.... */
	$"00E6 00EB 00F1 00F6 00FB 0100 0105 010A"	/* ................ */
	$"010F 0112 0116 0119 011E 0123 0127 012C"	/* ...........#.'., */
	$"0131 0136 013B 0140 0145 014A 014F 0153"	/* .1.6.;.@.E.J.O.S */
	$"0158 015D 0161 0166 016B 0170 0175 017A"	/* .X.].a.f.k.p.u.z */
	$"017F 0184 0189 018E 0193 0198 019D 01A2"	/* ....	....." */
	$"01A7 01AB 01AD 01B1 01B6 01B7 01B8 01B8"	/* .'.+.-.1.6.7.8.8 */
	$"01B8 01B8 01B8 01B8 01B8 01B8 01B8 01B8"	/* .8.8.8.8.8.8.8.8 */
	$"01B8 01B8 01B8 01B8 01B8 01B8 01B8 01B8"	/* .8.8.8.8.8.8.8.8 */
	$"01B8 01B8 01B8 01B8 01B8 01B8 01B8 01B8"	/* .8.8.8.8.8.8.8.8 */
	$"01B8 01B8 01B8 01B8 01B8 01B8 01B8 01B8"	/* .8.8.8.8.8.8.8.8 */
	$"01B8 01B8 01B8 01B8 01B8 01B8 01B8 01B8"	/* .8.8.8.8.8.8.8.8 */
	$"01B8 01B8 01B8 01B8 01B8 01B8 01B8 01B8"	/* .8.8.8.8.8.8.8.8 */
	$"01B8 01B8 01B8 01B8 01B8 01B8 01B8 01B8"	/* .8.8.8.8.8.8.8.8 */
	$"01B8 01B8 01B8 01B8 01B8 01B8 01B8 01B8"	/* .8.8.8.8.8.8.8.8 */
	$"01B8 01B8 01B8 01B8 01B8 01B8 01B8 01B8"	/* .8.8.8.8.8.8.8.8 */
	$"01B8 01B8 01B8 01B8 01B8 01B8 01B8 01B8"	/* .8.8.8.8.8.8.8.8 */
	$"01B8 01B8 01B8 01B8 01B8 01B8 01B8 01B8"	/* .8.8.8.8.8.8.8.8 */
	$"01B8 01B8 01B8 01B8 01B8 01B9 01BE 01C3"	/* .8.8.8.8.8.9.>.C */
	$"01C9 01CF 01D4 01D9 01DE 01E3 01E8 01ED"	/* .I.O.T.......... */
	$"01F1 01F5 01F9 01FD 0203 0209 020F 0215"	/* ...........F.... */
	$"021B 0221 0225 0229 022F 0235 0237 023B"	/* ...!.%.)./.5.7.; */
	$"023F 0244 0249 024E 0250 0256 0000 2600"	/* .?.D.I.N.P.V..&. */
	$"2600 2600 2600 2600 FFFF FFFF FFFF 2C06"	/* &.&.&.&.......,. */
	$"FFFF FFFF FFFF 2600 FFFF FFFF FFFF FFFF"	/* ......&......... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 1D06 2806"	/* ..............(. */
	$"2606 2606 2606 2606 2606 2706 2606 2706"	/* &.&.&.&.&.'.&.'. */
	$"2606 2606 2706 2606 2806 2706 2606 2706"	/* &.&.'.&.(.'.&.'. */
	$"2606 2606 2606 2606 2606 2606 2606 2606"	/* &.&.&.&.&.&.&.&. */
	$"2806 2706 2606 2606 2606 2606 2606 2606"	/* (.'.&.&.&.&.&.&. */
	$"2606 2606 2606 2606 2606 2606 2606 2706"	/* &.&.&.&.&.&.&.'. */
	$"2606 2606 2606 2606 2606 2606 2606 2606"	/* &.&.&.&.&.&.&.&. */
	$"2606 2606 2606 2606 2606 2606 2606 2606"	/* &.&.&.&.&.&.&.&. */
	$"2606 2706 2706 2706 2606 2606 2706 2606"	/* &.'.'.'.&.&.'.&. */
	$"2606 2606 2606 2606 2606 2606 2606 2706"	/* &.&.&.&.&.&.&.'. */
	$"2606 2606 2706 2606 2606 2606 2606 2606"	/* &.&.'.&.&.&.&.&. */
	$"2606 2606 2606 2606 2606 2606 2606 2606"	/* &.&.&.&.&.&.&.&. */
	$"2606 2706 2806 2606 2606 2806 2906 FFFF"	/* &.'.(.&.&.(.)... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"	/* ................ */
	$"FFFF FFFF FFFF FFFF FFFF 2806 2606 2606"	/* ..........(.&.&. */
	$"2606 2606 2606 2606 2607 2606 2606 2606"	/* &.&.&.&.&.&.&.&. */
	$"2606 2606 2806 2806 2606 2606 2606 2606"	/* &.&.(.(.&.&.&.&. */
	$"2606 2606 2806 2606 2606 2606 2806 2606"	/* &.&.(.&.&.&.(.&. */
	$"2706 2606 2606 2606 2706 2606 0000"     	/* '.&.&.&.'.&... */
};

data 'FONT' (16512, "VT100 Bold", preload, purgeable) {
};

/* for the benifit of MultiFinder.. */
data 'SIZE' (-1) {
	$"5000 0002 8000 0002 8000"               	/* .......... */
	/* 12th bit (2^0 == 0th) is "can background", 14th is */
	/*  does accept suspend and resume events */
	/*     { this  } is the (long) prefered memory size */
	/*  and          { this  } is the minimum memory size */
	/* we are currently playing it safe with these (160K) */
};

#ifdef SCRIPTMGR
/* so that the Apple keyboard doesn't do dead keys on OPTION */
data 'KCHR' (13000, "US-NODEADKEYS", sysheap, locked) {
	$"32C8 0000 0100 0202 0100 0303 0303 0303"	/* 2H.............. */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0100 0100 0100 0100 0303 0303 0303"	/* ................ */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0303 0303 0303 0303 0303 0303 0303"	/* ................ */
	$"0303 0005 6173 6466 6867 7A78 6376 0062"	/* ....asdfhgzxcv.b */
	$"7177 6572 7974 3132 3334 3635 3D39 372D"	/* qweryt123465=97- */
	$"3830 5D6F 755B 6970 0D6C 6A27 6B3B 5C2C"	/* 80]ou[ipBlj'k;\, */
	$"2F6E 6D2E 0920 6008 031B 0000 0000 0000"	/* /nm.F `......... */
	$"0000 0000 002E 1D2A 002B 1C1B 1F00 002F"	/* .......*.+...../ */
	$"031E 2D00 003D 3031 3233 3435 3637 0038"	/* ..-..=01234567.8 */
	$"3900 0000 1010 1010 1010 1010 1010 1010"	/* 9............... */
	$"1010 1010 1010 0501 0B7F 1004 100C 101C"	/* ................ */
	$"1D1F 1E00 4153 4446 4847 5A58 4356 0042"	/* ....ASDFHGZXCV.B */
	$"5157 4552 5954 2140 2324 5E25 2B28 265F"	/* QWERYT!@#$^%+(&_ */
	$"2A29 7D4F 557B 4950 0D4C 4A22 4B3A 7C3C"	/* *)}OU{IPBLJ"K:|< */
	$"3F4E 4D3E 0920 7E08 031B 0000 0000 0000"	/* ?NM>F ~......... */
	$"0000 0000 002E 2A2A 002B 2B1B 3D00 002F"	/* ......**.++.=../ */
	$"032F 2D00 003D 3031 3233 3435 3637 0038"	/* ./-..=01234567.8 */
	$"3900 0000 1010 1010 1010 1010 1010 1010"	/* 9............... */
	$"1010 1010 1010 0501 0B7F 1004 100C 101C"	/* ................ */
	$"1D1F 1E00 4153 4446 4847 5A58 4356 0042"	/* ....ASDFHGZXCV.B */
	$"5157 4552 5954 3132 3334 3635 3D39 372D"	/* QWERYT123465=97- */
	$"3830 5D4F 555B 4950 0D4C 4A27 4B3B 5C2C"	/* 80]OU[IPBLJ'K;\, */
	$"2F4E 4D2E 0920 6008 031B 0000 0000 0000"	/* /NM.F `......... */
	$"0000 0000 002E 1D2A 002B 1C1B 1F00 002F"	/* .......*.+...../ */
	$"031E 2D00 003D 3031 3233 3435 3637 0038"	/* ..-..=01234567.8 */
	$"3900 0000 1010 1010 1010 1010 1010 1010"	/* 9............... */
	$"1010 1010 1010 0501 0B7F 1004 100C 101C"	/* ................ */
	$"1D1F 1E00 8CA7 B6C4 FAA9 BDC5 8DC3 00BA"	/* ....'6D.)=E.: */
	$"CFB7 8EA8 B4A0 C1AA A3A2 A4B0 ADBB A6D0"	/* O7(4 A*#"$0-;&P */
	$"A5BC D4BF 9FD2 94B9 0DC2 C6BE FBC9 C7B2"	/* %<T?R9BBF>.IG2 */
	$"D696 B5B3 09CA 6008 031B 0000 0000 0000"	/* V53FJ`......... */
	$"0000 0000 002E 1D2A 002B 1C1B 1F00 002F"	/* .......*.+...../ */
	$"031E 2D00 003D 3031 3233 3435 3637 0038"	/* ..-..=01234567.8 */
	$"3900 0000 1010 1010 1010 1010 1010 1010"	/* 9............... */
	$"1010 1010 1010 0501 0B7F 1004 100C 101C"	/* ................ */
	$"1D1F 1E00 81EA EBEC EEED F3F4 82D7 00F5"	/* ...........W.. */
	$"CEE3 E4E5 E7E6 DADB DCDD DFDE B1E1 E0D1"	/* N...........1..Q */
	$"A1E2 D5AF E8D3 E9B8 0DF1 EFAE F0F2 C8F8"	/* !.U/.S.8B.....H. */
	$"C0F6 F7F9 09CA D908 031B 0000 0000 0000"	/* @...FJ.......... */
	$"0000 0000 002E 2A2A 002B 2B1B 3D00 002F"	/* ......**.++.=../ */
	$"032F 2D00 003D 3031 3233 3435 3637 0038"	/* ./-..=01234567.8 */
	$"3900 0000 1010 1010 1010 1010 1010 1010"	/* 9............... */
	$"1010 1010 1010 0501 0B7F 1004 100C 101C"	/* ................ */
	$"1D1F 1E00 0000"                         	/* ...... */
};

/* and the icon for our new key map */
resource 'SICN' (13000) {
	{	/* array: 1 elements */
		/* [1] */
		$"0003 0023 0050 00FC 013C 0E7C 38E8 4328"
		$"8C50 8850 88D8 E9F8 5978 7620 3FE0 07F0"
	}
};
#endif SCRIPTMGR
