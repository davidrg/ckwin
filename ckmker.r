/* Resource file for the MPW Kermit version (Macintosh) */
/* Compile this resource definition with Rez */
/* Created: 10/5/87 Matthias Aebi */
/* Modifications: */
/* 05/22/89 John A. Oberschelp for Emory University -- vt102 printer support */
/*                 Emory contact is Peter W. Day, ospwd@emoryu1.cc.emory.edu */ 
/* 02/27/89 PWP: moved all fonts to ckmfnt.r */
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
/*			header. Changed "Restore Settings…" to "Load */
/*			Settings". Removed output port selection from */
/* 			Communications dialog. Made packet length fields */
/*			wider (for long packets. Removed blank cursor re- */
/*			source. (M.Aebi) */
/* 10/7/87	Changed version info to October 1987 (M.Aebi) */

#include "Types.r"		/* To get system types */
#include "SysTypes.r"	/* get more system types */

/* The KR09 0 resource is created separately by ckmkr2.r */

/*
 * (PWP) I'm borrowing yet another idea from NCSA Telnet: there are
 * two sets of menus, one with clover marks (1..31) and one without
 * (33..63).  If clover keys are selected, then the marked ones are
 * displayed, and if not, the unmarked ones (thus not confusing people
 * who are running on Pluses and such).
 */
 
resource 'MENU' (1, preload) {
	1,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{	/* array: 2 elements */
		/* [1] */
		"About Kermit…", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (2) {
	2,
	textMenuProc,
	0x7FFFFEFB,
	enabled,
	"File",
	{	/* array: 8 elements */
		/* [1] */
		"Load Settings…", noIcon, noKey, noMark, plain,
		/* [2] */
		"Save Settings…", noIcon, noKey, noMark, plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain,
		/* [4] */
		"Get file from server…", noIcon, "G", noMark, plain,
		/* [5] */
		"Send file…", noIcon, "S", noMark, plain,
		/* [6] */
		"Receive file…", noIcon, "R", noMark, plain,
		/* [7] */
		"Transfer stats…", noIcon, noKey, noMark, plain,
		/* [8] */
		"Set transfer directory…", noIcon, noKey, noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Transfer to App…", noIcon, "T", noMark, plain,
		/* [11] */
		"Quit", noIcon, "Q", noMark, plain
	}
};

resource 'MENU' (34) {
	2,
	textMenuProc,
	0x7FFFFEFB,
	enabled,
	"File",
	{	/* array: 8 elements */
		/* [1] */
		"Load Settings…", noIcon, noKey, noMark, plain,
		/* [2] */
		"Save Settings…", noIcon, noKey, noMark, plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain,
		/* [4] */
		"Get file from server…", noIcon, noKey, noMark, plain,
		/* [5] */
		"Send file…", noIcon, noKey, noMark, plain,
		/* [6] */
		"Receive file…", noIcon, noKey, noMark, plain,
		/* [7] */
		"Transfer stats…", noIcon, noKey, noMark, plain,
		/* [8] */
		"Set transfer directory…", noIcon, noKey, noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Transfer to App…", noIcon, noKey, noMark, plain,
		/* [11] */
		"Quit", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (3) {
	3,
	textMenuProc,
	0x7FFFFFBD,
	enabled,
	"Edit",
	{	/* array: 10 elements */
		/* [1] */
		"Undo", noIcon, "Z", noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"Cut", noIcon, "X", noMark, plain,
		/* [4] */
		"Copy", noIcon, "C", noMark, plain,
		/* [5] */
		"Paste", noIcon, "V", noMark, plain,
		/* [6] */
		"Clear", noIcon, noKey, noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Send Break", noIcon, "B", noMark, plain,
		/* [9] */
		"Send Long Break", noIcon, noKey, noMark, plain,
		/* [10] */
		"Send XON", noIcon, noKey, noMark, plain,
		/* [11] */
		"Toggle DTR", noIcon, noKey, noMark, plain
	}
};
resource 'MENU' (35) {
	3,
	textMenuProc,
	0x7FFFFFBD,
	enabled,
	"Edit",
	{	/* array: 10 elements */
		/* [1] */
		"Undo", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"Cut", noIcon, noKey, noMark, plain,
		/* [4] */
		"Copy", noIcon, noKey, noMark, plain,
		/* [5] */
		"Paste", noIcon, noKey, noMark, plain,
		/* [6] */
		"Clear", noIcon, noKey, noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Send Break", noIcon, noKey, noMark, plain,
		/* [9] */
		"Send Long Break", noIcon, noKey, noMark, plain,
		/* [10] */
		"Send XON", noIcon, noKey, noMark, plain,
		/* [11] */
		"Toggle DTR", noIcon, noKey, noMark, plain
	}
};


resource 'MENU' (4) {
	4,
	textMenuProc,
	0x7FFFFFDF,
	enabled,
	"Settings",
	{	/* array: 12 elements */
		/* [1] */
		"File Defaults…", noIcon, noKey, noMark, plain,
		/* [2] */
		"Communications…", noIcon, noKey, noMark, plain,
		/* [3] */
		"Protocol…", noIcon, noKey, noMark, plain,
		/* [4] */
		"Terminal…", noIcon, noKey, noMark, plain,
		/* [5] */
		"Character Set…", noIcon, noKey, noMark, plain,
		/* [6] */
		"-", noIcon, noKey, noMark, plain,
		/* [7] */
		"\0x11-Shift-1…\0x11-Shift-9 active", noIcon, noKey,
		    check, plain,
		/* [8] */
		"Menu \0x11-Keys active", noIcon, "M", noMark, plain,
		/* [9] */
		"Set key macros…", noIcon, noKey, noMark, plain,
		/* [10] */
		"Set modifiers…", noIcon, noKey, noMark, plain
	}
};
resource 'MENU' (36) {
	4,
	textMenuProc,
	0x7FFFFFDF,
	enabled,
	"Settings",
	{	/* array: 12 elements */
		/* [1] */
		"File Defaults…", noIcon, noKey, noMark, plain,
		/* [2] */
		"Communications…", noIcon, noKey, noMark, plain,
		/* [3] */
		"Protocol…", noIcon, noKey, noMark, plain,
		/* [4] */
		"Terminal…", noIcon, noKey, noMark, plain,
		/* [5] */
		"Character Set…", noIcon, noKey, noMark, plain,
		/* [6] */
		"-", noIcon, noKey, noMark, plain,
		/* [7] */
		"\0x11-Shift-1…\0x11-Shift-9 active", noIcon, noKey,
		    check, plain,
		/* [8] */
		"Menu \0x11-Keys active", noIcon, noKey, noMark, plain,
		/* [9] */
		"Set key macros…", noIcon, noKey, noMark, plain,
		/* [10] */
		"Set modifiers…", noIcon, noKey, noMark, plain
	}
};


resource 'MENU' (5) {
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
		"Logout", noIcon, noKey, noMark, plain,
		/* [5] */
		"-", noIcon, noKey, noMark, plain,
		/* [6] */
		"Cwd…", noIcon, noKey, noMark, plain,
		/* [7] */
		"Delete file…", noIcon, noKey, noMark, plain,
		/* [8] */
		"Directory…", noIcon, noKey, noMark, plain,
		/* [9] */
		"Help…", noIcon, noKey, noMark, plain,
		/* [10] */
		"Host…", noIcon, noKey, noMark, plain,
		/* [11] */
		"Space…", noIcon, noKey, noMark, plain,
		/* [12] */
		"Type…", noIcon, noKey, noMark, plain,
		/* [13] */
		"Who…", noIcon, noKey, noMark, plain,
		/* [14] */
		"-", noIcon, noKey, noMark, plain,
		/* [15] */
		"Be a Server", noIcon, "H", noMark, plain
	}
};
resource 'MENU' (37) {
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
		"Logout", noIcon, noKey, noMark, plain,
		/* [5] */
		"-", noIcon, noKey, noMark, plain,
		/* [6] */
		"Cwd…", noIcon, noKey, noMark, plain,
		/* [7] */
		"Delete file…", noIcon, noKey, noMark, plain,
		/* [8] */
		"Directory…", noIcon, noKey, noMark, plain,
		/* [9] */
		"Help…", noIcon, noKey, noMark, plain,
		/* [10] */
		"Host…", noIcon, noKey, noMark, plain,
		/* [11] */
		"Space…", noIcon, noKey, noMark, plain,
		/* [12] */
		"Type…", noIcon, noKey, noMark, plain,
		/* [13] */
		"Who…", noIcon, noKey, noMark, plain,
		/* [14] */
		"-", noIcon, noKey, noMark, plain,
		/* [15] */
		"Be a Server", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (6) {
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
resource 'MENU' (38) {
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

resource 'MENU' (7) {										/*JAO*/
	7,																/*JAO*/
	textMenuProc,													/*JAO*/
	0x7FFFFFFF,														/*JAO*/
	disabled,														/*JAO*/
	"Print",														/*JAO*/
	{	/* array: 3 elements */										/*JAO*/
		/* [1] */													/*JAO*/
		"Print Captured Text…", noIcon, "P", noMark, plain,			/*JAO*/
		/* [2] */													/*JAO*/
		"Status…", noIcon, noKey, noMark, plain,					/*JAO*/
		/* [3] */													/*JAO*/
		"Discard Captured Text", noIcon, noKey, noMark, plain		/*JAO*/
	}																/*JAO*/
};
resource 'MENU' (39) {										/*JAO*/
	7,																/*JAO*/
	textMenuProc,													/*JAO*/
	0x7FFFFFFF,														/*JAO*/
	disabled,														/*JAO*/
	"Print",														/*JAO*/
	{	/* array: 3 elements */										/*JAO*/
		/* [1] */													/*JAO*/
		"Print Captured Text…", noIcon, noKey, noMark, plain,		/*JAO*/
		/* [2] */													/*JAO*/
		"Status…", noIcon, noKey, noMark, plain,					/*JAO*/
		/* [3] */													/*JAO*/
		"Discard Captured Text", noIcon, noKey, noMark, plain		/*JAO*/
	}																/*JAO*/
};

resource 'WIND' (1000, preload) {
	{40, 5, 335, 505},
	documentProc,		/* was: noGrowDocProc */
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
resource 'DLOG' (230, purgeable, preload) {
	{60, 40, 300, 458},
	noGrowDocProc,
	visible,
	noGoAway,
	0x0,
	230,
	"File Transfer Status"
};

/* About Kermit */
resource 'DLOG' (257, purgeable, preload) {
	{38, 16, 306, 500},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	257,
	""
};

resource 'DLOG' (1001, purgeable, preload) {
	{40, 60, 292, 454},
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

resource 'DLOG' (1004, purgeable, preload) {
	{44, 80, 306, 426},
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

resource 'DLOG' (1014, purgeable, preload) {
	{34, 50, 306, 458},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1014,
	""
};

resource 'DLOG' (1015) {
	{0, 0, 184, 304},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	1015,
	""
};

resource 'DLOG' (1016, purgeable, preload) {
	{46, 94, 288, 370},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1016,
	""
};

resource 'DLOG' (2001, preload, purgeable) {		/*JAO*/
	{93, 70, 250, 381},								/*JAO*/
	dBoxProc,										/*JAO*/
	visible,										/*JAO*/
	noGoAway,										/*JAO*/
	0x0,											/*JAO*/
	2001,											/*JAO*/
	""												/*JAO*/
};													/*JAO*/
 
resource 'DLOG' (2002, preload, purgeable) {		/*JAO*/
	{30, 70, 70, 300},								/*JAO*/
	dBoxProc,										/*JAO*/
	visible,										/*JAO*/
	noGoAway,										/*JAO*/
	0x0,											/*JAO*/
	2002,											/*JAO*/
	""												/*JAO*/
};													/*JAO*/
 
resource 'DLOG' (2003, preload, purgeable) {		/*JAO*/
	{190, 70, 245, 346},							/*JAO*/
	dBoxProc,										/*JAO*/
	visible,										/*JAO*/
	noGoAway,										/*JAO*/
	0x0,											/*JAO*/
	2003,											/*JAO*/
	""												/*JAO*/
};													/*JAO*/

resource 'DLOG' (2004, preload, purgeable) {		/*JAO*/
	{93, 70, 250, 381},								/*JAO*/
	dBoxProc,										/*JAO*/
	visible,										/*JAO*/
	noGoAway,										/*JAO*/
	0x0,											/*JAO*/
	2004,											/*JAO*/
	""												/*JAO*/
};													/*JAO*/

resource 'DLOG' (2005, preload, purgeable) {		/*JAO*/
	{70, 100, 120, 300},							/*JAO*/
	dBoxProc,										/*JAO*/
	visible,										/*JAO*/
	noGoAway,										/*JAO*/
	0x0,											/*JAO*/
	2005,											/*JAO*/
	""												/*JAO*/
};													/*JAO*/

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

resource 'DITL' (230, purgeable, preload) {
	{	/* array DITLarray: 24 elements */
		/* [1] */
		{168, 88, 200, 180},
		Button {
			enabled,
			"Cancel File"
		},
		/* [2] */
		{168, 224, 200, 316},
		Button {
			enabled,
			"Cancel Group"
		},
		/* [3] */
		{8, 8, 24, 76},
		StaticText {
			disabled,
			"Receiving"
		},
		/* [4] */
		{8, 80, 24, 272},
		StaticText {
			disabled,
			""
		},
		/* [5] */
		{32, 56, 48, 76},
		StaticText {
			disabled,
			"As"
		},
		/* [6] */
		{32, 80, 48, 272},
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
		{56, 144, 73, 204},
		StaticText {
			disabled,
			"Packets:"
		},
		/* [10] */
		{56, 205, 73, 261},
		StaticText {
			disabled,
			""
		},
		/* [11] */
		{56, 288, 73, 343},
		StaticText {
			disabled,
			"Retries:"
		},
		/* [12] */
		{56, 344, 73, 391},
		StaticText {
			disabled,
			""
		},
		/* [13] */
		{32, 288, 48, 408},
		StaticText {
			disabled,
			""
		},
		/* [14] */
		{8, 288, 24, 408},
		StaticText {
			disabled,
			""
		},
		/* [15] */
		{104, 40, 120, 368},
		StaticText {
			disabled,
			""
		},
		/* [16] */
		{80, 144, 96, 229},
		StaticText {
			disabled,
			"Packet size:"
		},
		/* [17] */
		{80, 230, 96, 270},
		StaticText {
			disabled,
			""
		},
		/* [18] */
		{80, 288, 96, 360},
		StaticText {
			disabled,
			"Checksum:"
		},
		/* [19] */
		{80, 361, 96, 381},
		StaticText {
			disabled,
			""
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
			""
		},
		/* [22] */
		{208, 40, 224, 368},
		StaticText {
			disabled,
			"Emergency exit: hold down \0x11 and type a p"
			"eriod."
		},
		/* [23] */
		{124, 40, 140, 368},
		StaticText {
			disabled,
			""
		},
		/* [24] */
		{144, 40, 160, 368},
		StaticText {
			disabled,
			""
		}
	}
};


/* About Kermit */
resource 'DITL' (257, purgeable, preload) {
	{	/* array DITLarray: 20 elements */
		/* [1] */
		{224, 360, 248, 448},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{8, 16, 24, 464},
		StaticText {
			disabled,
			""
		},
		/* [3] */
		{26, 56, 42, 464},
		StaticText {
			disabled,
			""
		},
		/* [4] */
		{44, 56, 60, 464},
		StaticText {
			disabled,
			"©1986—1989 Columbia University, All Righ"
			"ts Reserved"
		},
		/* [5] */
		{26, 16, 58, 48},
		Icon {
			disabled,
			1000
		},
		/* [6] */
		{136, 16, 168, 48},
		Icon {
			disabled,
			2000
		},
		/* [7] */
		{136, 52, 168, 124},
		StaticText {
			disabled,
			"Bill Catchings"
		},
		/* [8] */
		{136, 260, 168, 292},
		Icon {
			disabled,
			2001
		},
		/* [9] */
		{136, 296, 168, 344},
		StaticText {
			disabled,
			"Bill Schilit"
		},
		/* [10] */
		{136, 148, 168, 180},
		Icon {
			disabled,
			2002
		},
		/* [11] */
		{136, 184, 168, 240},
		StaticText {
			disabled,
			"Frank da Cruz"
		},
		/* [12] */
		{136, 400, 168, 464},
		StaticText {
			disabled,
			"Matthias Aebi"
		},
		/* [13] */
		{136, 364, 168, 396},
		Icon {
			disabled,
			2003
		},
		/* [14] */
		{176, 16, 208, 48},
		Icon {
			disabled,
			2004
		},
		/* [15] */
		{176, 52, 208, 124},
		StaticText {
			disabled,
			"Paul Placeway"
		},
		/* [16] */
		{64, 16, 80, 464},
		StaticText {
			disabled,
			"Permission is granted to any individual "
			"or institution to use, copy,"
		},
		/* [17] */
		{224, 16, 256, 337},
		StaticText {
			disabled,
			"Please send bugs, comments, and question"
			"s to: Info-Kermit@cunixc.cc.columbia.edu"
		},
		/* [18] */
		{176, 152, 208, 240},
		StaticText {
			disabled,
			"And many, many others"
		},
		/* [19] */
		{80, 16, 96, 464},
		StaticText {
			disabled,
			"or redistribute this software so long as"
			" it is not sold for profit,"
		},
		/* [20] */
		{96, 16, 128, 464},
		StaticText {
			disabled,
			"provided this copyright notice is retain"
			"ed.  “Kermit the Frog” used by permissio"
			"n of Henson Associates, Inc."
		}
	}
};

resource 'DITL' (1001, purgeable, preload) {
	{	/* array DITLarray: 31 elements */
		/* [1] */
		{216, 104, 236, 184},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{216, 200, 236, 280},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{0, 104, 16, 280},
		StaticText {
			disabled,
			"Communications Settings"
		},
		/* [4] */
		{24, 8, 40, 80},
		StaticText {
			disabled,
			"Baud Rate:"
		},
		/* [5] */
		{24, 96, 40, 152},
		RadioButton {
			enabled,
			"300"
		},
		/* [6] */
		{24, 168, 40, 224},
		RadioButton {
			enabled,
			"600"
		},
		/* [7] */
		{24, 240, 40, 296},
		RadioButton {
			enabled,
			"1200"
		},
		/* [8] */
		{24, 312, 40, 368},
		RadioButton {
			enabled,
			"1800"
		},
		/* [9] */
		{40, 96, 56, 152},
		RadioButton {
			enabled,
			"2400"
		},
		/* [10] */
		{40, 168, 56, 224},
		RadioButton {
			enabled,
			"3600"
		},
		/* [11] */
		{40, 240, 56, 296},
		RadioButton {
			enabled,
			"4800"
		},
		/* [12] */
		{40, 312, 56, 368},
		RadioButton {
			enabled,
			"7200"
		},
		/* [13] */
		{56, 96, 72, 152},
		RadioButton {
			enabled,
			"9600"
		},
		/* [14] */
		{56, 168, 72, 232},
		RadioButton {
			enabled,
			"14400"
		},
		/* [15] */
		{56, 240, 72, 304},
		RadioButton {
			enabled,
			"19200"
		},
		/* [16] */
		{56, 312, 72, 376},
		RadioButton {
			enabled,
			"28800"
		},
		/* [17] */
		{72, 96, 88, 160},
		RadioButton {
			enabled,
			"38400"
		},
		/* [18] */
		{72, 168, 88, 232},
		RadioButton {
			enabled,
			"57600"
		},
		/* [19] */
		{104, 8, 120, 88},
		StaticText {
			disabled,
			"Parity/Bits:"
		},
		/* [20] */
		{104, 272, 120, 352},
		RadioButton {
			enabled,
			"Mark/7"
		},
		/* [21] */
		{120, 272, 136, 360},
		RadioButton {
			enabled,
			"Space/7"
		},
		/* [22] */
		{104, 184, 120, 256},
		RadioButton {
			enabled,
			"Even/7"
		},
		/* [23] */
		{120, 184, 136, 256},
		RadioButton {
			enabled,
			"Odd/7"
		},
		/* [24] */
		{104, 96, 120, 168},
		RadioButton {
			enabled,
			"None/8"
		},
		/* [25] */
		{184, 184, 200, 360},
		CheckBox {
			enabled,
			"XOn/XOff flow control"
		},
		/* [26] */
		{152, 8, 168, 88},
		StaticText {
			disabled,
			"Serial Port:"
		},
		/* [27] */
		{152, 96, 168, 116},
		RadioButton {
			enabled,
			""
		},
		/* [28] */
		{144, 116, 176, 148},
		Icon {
			enabled,
			3000
		},
		/* [29] */
		{152, 184, 168, 204},
		RadioButton {
			enabled,
			""
		},
		/* [30] */
		{144, 204, 176, 236},
		Icon {
			enabled,
			3001
		},
		/* [31] */
		{184, 8, 200, 152},
		CheckBox {
			enabled,
			"Drop DTR on Quit"
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

resource 'DITL' (1004, purgeable, preload) {
	{	/* array DITLarray: 17 elements */
		/* [1] */
		{224, 72, 248, 152},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{224, 184, 248, 264},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{8, 8, 24, 96},
		StaticText {
			disabled,
			"File Settings:"
		},
		/* [4] */
		{32, 8, 48, 320},
		RadioButton {
			enabled,
			"Attended: dialog on each file received."
		},
		/* [5] */
		{56, 8, 72, 328},
		RadioButton {
			enabled,
			"Unattended: with the following defaults."
			".."
		},
		/* [6] */
		{80, 24, 96, 336},
		RadioButton {
			enabled,
			"Supersede existing files of the same nam"
			"e."
		},
		/* [7] */
		{96, 24, 112, 320},
		RadioButton {
			enabled,
			"Create new file names to avoid conflicts"
			"."
		},
		/* [8] */
		{144, 24, 160, 72},
		StaticText {
			disabled,
			"Mode:"
		},
		/* [9] */
		{144, 176, 160, 216},
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
		{144, 224, 160, 288},
		RadioButton {
			enabled,
			"Data"
		},
		/* [12] */
		{160, 224, 176, 312},
		RadioButton {
			enabled,
			"Resource"
		},
		/* [13] */
		{144, 80, 160, 136},
		RadioButton {
			enabled,
			"Text"
		},
		/* [14] */
		{160, 80, 176, 144},
		RadioButton {
			enabled,
			"Binary"
		},
		/* [15] */
		{176, 80, 192, 176},
		RadioButton {
			enabled,
			"MacBinary"
		},
		/* [16] */
		{120, 24, 136, 232},
		CheckBox {
			enabled,
			"Keep partially received files"
		},
		/* [17] */
		{200, 8, 216, 304},
		CheckBox {
			enabled,
			"Use above defaults for sending too"
		}
	}
};


/* PWP was here… (many times) */
resource 'DITL' (1005, purgeable, preload) {
	{	/* array DITLarray: 18 elements */
		/* [1] */
		{216, 304, 240, 384},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{216, 208, 240, 288},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{8, 128, 24, 256},
		StaticText {
			disabled,
			"Terminal Settings:"
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
		{160, 24, 176, 152},
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
		{184, 24, 200, 304},
		CheckBox {
			enabled,
			"Accept Eight Bit Characters"
		},
		/* [15] */
		{136, 24, 152, 144},
		CheckBox {
			enabled,
			"Blinking Cursor"
		},
		/* [16] */
		{216, 24, 240, 136},
		Button {
			enabled,
			"Reset Terminal"
		},
		/* [17] */
		{40, 200, 56, 220},
		EditText {
			enabled,
			"00"
		},
		/* [18] */
		{40, 225, 56, 377},
		StaticText {
			disabled,
			"Lines on the screen"
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
			"ell, BS, DEL, … enter a \\ followed by th"
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

resource 'DITL' (1014, purgeable, preload) {
	{	/* array DITLarray: 29 elements */
		/* [1] */
		{240, 304, 264, 384},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{240, 208, 264, 288},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{8, 104, 24, 280},
		StaticText {
			disabled,
			"Terminal Character Sets:"
		},
		/* [4] */
		{56, 8, 72, 168},
		RadioButton {
			enabled,
			"USA National (ASCII)"
		},
		/* [5] */
		{72, 8, 88, 168},
		RadioButton {
			enabled,
			"UK National"
		},
		/* [6] */
		{88, 8, 104, 168},
		RadioButton {
			enabled,
			"Dutch National"
		},
		/* [7] */
		{104, 8, 120, 168},
		RadioButton {
			enabled,
			"Finnish National"
		},
		/* [8] */
		{120, 8, 136, 168},
		RadioButton {
			enabled,
			"French National"
		},
		/* [9] */
		{136, 8, 152, 200},
		RadioButton {
			enabled,
			"French Canadian National"
		},
		/* [10] */
		{152, 8, 168, 168},
		RadioButton {
			enabled,
			"German National"
		},
		/* [11] */
		{168, 8, 184, 168},
		RadioButton {
			enabled,
			"Italian National"
		},
		/* [12] */
		{184, 8, 200, 192},
		RadioButton {
			enabled,
			"Norwegian/Danish Nat."
		},
		/* [13] */
		{200, 8, 216, 168},
		RadioButton {
			enabled,
			"Portuguese National"
		},
		/* [14] */
		{216, 8, 232, 168},
		RadioButton {
			enabled,
			"Spanish National"
		},
		/* [15] */
		{232, 8, 248, 168},
		RadioButton {
			enabled,
			"Swedish National"
		},
		/* [16] */
		{248, 8, 264, 168},
		RadioButton {
			enabled,
			"Swiss National"
		},
		/* [17] */
		{88, 208, 104, 376},
		RadioButton {
			enabled,
			"ISO Latin 1 (Western)"
		},
		/* [18] */
		{104, 208, 120, 368},
		RadioButton {
			disabled,
			"ISO Latin 2 (Eastern)"
		},
		/* [19] */
		{120, 208, 136, 376},
		RadioButton {
			disabled,
			"ISO Latin 3 (Romance)"
		},
		/* [20] */
		{136, 208, 152, 400},
		RadioButton {
			disabled,
			"ISO Latin 4 (Scandinavian)"
		},
		/* [21] */
		{152, 208, 168, 368},
		RadioButton {
			disabled,
			"ISO Latin/Cyrillic"
		},
		/* [22] */
		{168, 208, 184, 368},
		RadioButton {
			disabled,
			"ISO Latin/Arabic"
		},
		/* [23] */
		{184, 208, 200, 368},
		RadioButton {
			disabled,
			"ISO Latin/Greek"
		},
		/* [24] */
		{200, 208, 216, 368},
		RadioButton {
			disabled,
			"ISO Latin/Hebrew"
		},
		/* [25] */
		{216, 208, 232, 368},
		RadioButton {
			disabled,
			"ISO Latin 5 (Turkish)"
		},
		/* [26] */
		{56, 208, 72, 368},
		RadioButton {
			enabled,
			"DEC Graphics"
		},
		/* [27] */
		{72, 208, 88, 368},
		RadioButton {
			enabled,
			"DEC Technical"
		},
		/* [28] */
		{32, 8, 48, 176},
		StaticText {
			disabled,
			"G0 (normal characters):"
		},
		/* [29] */
		{32, 208, 48, 360},
		StaticText {
			disabled,
			"G1 (8 bit characters):"
		}
	}
};


resource 'DITL' (1015) {
	{	/* array DITLarray: 8 elements */
		/* [1] */
		{132, 218, 150, 288},
		Button {
			enabled,
			"Set"
		},
		/* [2] */
		{158, 218, 176, 288},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{10, 15, 26, 198},
		StaticText {
			disabled,
			"Select Default Directory"
		},
		/* [4] */
		{29, 198, 49, 302},
		UserItem {
			disabled
		},
		/* [5] */
		{56, 218, 74, 288},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{82, 218, 100, 288},
		Button {
			enabled,
			"Drive"
		},
		/* [7] */
		{0, 1500, 50, 1550},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{59, 14, 157, 197},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (1016, purgeable, preload) {
	{	/* array DITLarray: 14 elements */
		/* [1] */
		{208, 104, 232, 168},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{16, 56, 32, 216},
		StaticText {
			disabled,
			"File Transfer Statistics"
		},
		/* [3] */
		{48, 50, 64, 170},
		StaticText {
			disabled,
			"Transaction time:"
		},
		/* [4] */
		{72, 24, 88, 168},
		StaticText {
			disabled,
			"Characters from line:"
		},
		/* [5] */
		{96, 42, 112, 170},
		StaticText {
			disabled,
			"Characters to line:"
		},
		/* [6] */
		{120, 25, 136, 169},
		StaticText {
			disabled,
			"Effective characters:"
		},
		/* [7] */
		{144, 32, 160, 168},
		StaticText {
			disabled,
			"Effective baud rate:"
		},
		/* [8] */
		{48, 184, 64, 248},
		StaticText {
			disabled,
			""
		},
		/* [9] */
		{72, 184, 88, 248},
		StaticText {
			disabled,
			""
		},
		/* [10] */
		{96, 184, 112, 248},
		StaticText {
			disabled,
			""
		},
		/* [11] */
		{120, 184, 136, 248},
		StaticText {
			disabled,
			""
		},
		/* [12] */
		{144, 184, 160, 248},
		StaticText {
			disabled,
			""
		},
		/* [13] */
		{168, 95, 184, 167},
		StaticText {
			disabled,
			"Efficiency:"
		},
		/* [14] */
		{168, 184, 184, 248},
		StaticText {
			disabled,
			""
		}
	}
};

resource 'DITL' (2001, preload, purgeable) {	/*JAO*/
	{	/* array DITLarray: 5 elements */		/*JAO*/
		/* [1] */								/*JAO*/
		{120, 216, 144, 296},					/*JAO*/
		Button {								/*JAO*/
			enabled,							/*JAO*/
			"OK"								/*JAO*/
		},										/*JAO*/
		/* [2] */								/*JAO*/
		{88, 34, 104, 227},						/*JAO*/
		StaticText {							/*JAO*/
			enabled,							/*JAO*/
			"Characters Lost: ^0"				/*JAO*/
		},										/*JAO*/
		/* [3] */								/*JAO*/
		{65, 34, 81, 258},						/*JAO*/
		StaticText {							/*JAO*/
			enabled,							/*JAO*/
			"Characters Buffered: ^1"			/*JAO*/
		},										/*JAO*/
		/* [4] */								/*JAO*/
		{40, 34, 56, 202},						/*JAO*/
		StaticText {							/*JAO*/
			enabled,							/*JAO*/
			"Buffer Size: ^2"					/*JAO*/
		},										/*JAO*/
		/* [5] */								/*JAO*/
		{8, 8, 24, 127},						/*JAO*/
		StaticText {							/*JAO*/
			enabled,							/*JAO*/
			"Status…"							/*JAO*/
		}										/*JAO*/
	}											/*JAO*/
};												/*JAO*/
 
resource 'DITL' (2002, preload, purgeable) {	/*JAO*/
	{	/* array DITLarray: 1 element */		/*JAO*/
		/* [1] */								/*JAO*/
		{10, 15, 500, 500},						/*JAO*/
		StaticText {							/*JAO*/
			enabled,							/*JAO*/
			"Capturing text to be printed"		/*JAO*/
		},										/*JAO*/
	}											/*JAO*/
};												/*JAO*/
 
 
resource 'DITL' (2003, preload, purgeable) {	/*JAO*/
	{	/* array DITLarray: 1 element */		/*JAO*/
		/* [1] */								/*JAO*/
		{10, 15, 500, 500},						/*JAO*/
		StaticText {							/*JAO*/
			enabled,							/*JAO*/
			"Warning, captured text overflowed\nits storage space, some text is lost."	/*JAO*/
		},										/*JAO*/
	}											/*JAO*/
};												/*JAO*/
 
 
resource 'DITL' (2004, preload, purgeable) {	/*JAO*/
	{	/* array DITLarray: 4 elements */		/*JAO*/
		/* [1] */								/*JAO*/
		{55, 56, 79, 216},						/*JAO*/
		Button {								/*JAO*/
			enabled,							/*JAO*/
			"Print anyway"						/*JAO*/
		},										/*JAO*/
		/* [2] */								/*JAO*/
		{90, 56, 114, 216},						/*JAO*/
		Button {								/*JAO*/
			enabled,							/*JAO*/
			"Cancel"							/*JAO*/
		},										/*JAO*/
		/* [3] */								/*JAO*/
		{125, 56, 149, 216},					/*JAO*/
		Button {								/*JAO*/
			enabled,							/*JAO*/
			"Cancel, discard text"				/*JAO*/
		},										/*JAO*/
		/* [4] */								/*JAO*/
		{10, 34, 42, 327},						/*JAO*/
		StaticText {							/*JAO*/
			enabled,							/*JAO*/
			"Captured text overflowed storage,\nsome text is lost.  Print anyway?"	/*JAO*/
		}										/*JAO*/
	}											/*JAO*/
};												/*JAO*/
 
 
resource 'DITL' (2005, preload, purgeable) {	/*JAO*/
	{	/* array DITLarray: 3 elements */		/*JAO*/
		/* [1] */								/*JAO*/
		{10, 34, 42, 174},						/*JAO*/
		StaticText {							/*JAO*/
			enabled,							/*JAO*/
			"Printing…\nTo cancel press \0x11-."/*JAO*/
		}										/*JAO*/
	}											/*JAO*/
};												/*JAO*/

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

resource 'DITL' (3999, purgeable, preload) {
	{	/* array DITLarray: 16 elements */
		/* [1] */
		{216, 16, 236, 86},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{216, 96, 236, 166},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{112, 16, 128, 184},
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
		{136, 16, 152, 184},
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
		{160, 112, 176, 168},
		RadioButton {
			enabled,
			"Data"
		},
		/* [12] */
		{176, 112, 192, 192},
		RadioButton {
			enabled,
			"Resource"
		},
		/* [13] */
		{160, 16, 176, 80},
		RadioButton {
			enabled,
			"Text"
		},
		/* [14] */
		{176, 16, 192, 88},
		RadioButton {
			enabled,
			"Binary"
		},
		/* [15] */
		{192, 16, 208, 112},
		RadioButton {
			enabled,
			"MacBinary"
		},
		/* [16] */
		{216, 176, 236, 336},
		Button {
			enabled,
			"Proceed Automatically"
		}
	}
};

resource 'DITL' (4000, purgeable, preload) {
	{	/* array DITLarray: 20 elements */
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
		{176, 168, 192, 224},
		RadioButton {
			enabled,
			"Data"
		},
		/* [12] */
		{192, 168, 208, 248},
		RadioButton {
			enabled,
			"Resource"
		},
		/* [13] */
		{176, 48, 192, 112},
		RadioButton {
			enabled,
			"Text"
		},
		/* [14] */
		{192, 48, 208, 112},
		RadioButton {
			enabled,
			"Binary"
		},
		/* [15] */
		{208, 48, 224, 144},
		RadioButton {
			enabled,
			"MacBinary"
		},
		/* [16] */
		{132, 12, 150, 44},
		StaticText {
			disabled,
			"As:"
		},
		/* [17] */
		{52, 455, 148, 556},
		EditText {
			enabled,
			""
		},
		/* [18] */
		{155, 15, 171, 255},
		EditText {
			enabled,
			""
		},
		/* [19] */
		{31, 311, 63, 343},
		Icon {
			disabled,
			1000
		},
		/* [20] */
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

resource 'ICON' (2004, purgeable, preload) {
	$"0000 0000 0007 E000 0038 7E00 00F0 0F80"
	$"01E0 1BC0 01C0 07C0 03C0 06E0 0380 01F0"
	$"0380 0278 0700 00F8 0780 00D8 0700 0078"
	$"0700 006C 0578 7454 0703 0224 0539 3828"
	$"0311 1048 0302 0028 0102 0050 0102 0030"
	$"0081 8040 0080 0040 0088 1080 0047 E080"
	$"0060 0100 0020 0200 0010 0400 0008 1800"
	$"0007 E0"
};

/* Modem port (phone) Icon */
resource 'ICON' (3000, purgeable) {
	$"FFFF FFFF 8000 0001 8000 0001 80E0 0001"
	$"8110 0301 8208 0481 8208 0481 8208 1301"
	$"8210 0801 8220 6401 8220 9001 8210 9001"
	$"8108 6001 8104 0001 8082 0001 8041 0001"
	$"8020 8701 8010 4881 8008 3041 8004 0041"
	$"8002 0041 8001 8081 8000 7F01 8000 0001"
	$"9100 4001 9B00 4001 9531 CCD1 914A 5EA9"
	$"914A 5089 9131 CC89 8000 0001 FFFF FFFF"
};

/* Printer port icon */
resource 'ICON' (3001, purgeable) {
	$"FFFF FFFF 8000 0001 80FF FE01 8080 2201"
	$"8080 3201 8080 2A01 8080 3E01 8080 0201"
	$"8080 0201 8080 0201 8080 0201 8F80 03E1"
	$"8880 0221 8880 0239 8880 0229 88FF FE29"
	$"8800 0039 8800 0021 8FFF FFE1 8800 0021"
	$"8800 0021 8FFF FFE1 8000 0001 8000 0001"
	$"8C00 0001 8A10 2001 8CC6 7361 8895 2741"
	$"8895 2441 8895 2341 8000 0001 FFFF FFFF"
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
data 'KSET' (4) {
	$"0000 0000 0000 3000 779E 7FD8 0000 001E"        /* ......0.wû.ÿ.... */
	$"0000 0000 0000 2000 0100 0000 0000 0000"        /* ...... ......... */
	$"0000 0000 0000 0000 2284 0000 0000 0000"        /* ........"Ñ...... */
	$"0000 0210 0000 2000 0000 0000 0000 0000"        /* ...... ......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 2000 0000 0000 0000 0000"        /* ...... ......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"        /* ................ */
};

data 'MSET' (5) {
	$"0102 00BC 0045 83FC 0002 6AE2 0000 0000"        /* ...º.EÉ...j..... */
	$"0002 6AE2 0045 0012 00BC 0045 83FC 0002"        /* ..j..E...º.EÉ... */
	$"6AE2 0000 0000 0002 6AE2 0045 0000 00BC"        /* j.......j..E...º */
	$"0045 83FC 0002 6AE2 0000 0000 0002 6AE2"        /* .EÉ...j.......j. */
	$"0045 0000 00BC 0045 83FC 0002 6AE2 0000"        /* .E...º.EÉ...j... */
	$"0000 0002 6AE2 0045 0026 0033 0001 7F00"        /* ....j..E.&.3.... */
	$"3200 011B 00B2 0001 6000 7E05 0000 7D06"        /* 2....≤..`.~...}. */
	$"0000 7C04 0000 7B03 0000 4609 0000 4D0C"        /* ..|...{...F∆..M. */
	$"0000 480B 0000 420A 0008 3200 0160 004C"        /* ..H...B...2..`.L */
	$"1900 00C7 0100 019B 0001 1F00 4707 0000"        /* ...«...õ....G... */
	$"4E0C 0000 520F 0000 5310 0000 5411 0000"        /* N...R...S...T... */
	$"5512 0000 5613 0000 5714 0000 5815 0000"        /* U...V...W...X... */
	$"5916 0000 5B17 0000 5C18 0000 410D 0001"        /* Y...[...\...A¬.. */
	$"9600 011E 01B2 0001 1E01 4808 0001 4D09"        /* ñ....≤....H...M∆ */
	$"0001 420A 0001 460B 0000 450B 0000 430A"        /* ..B...F...E...C. */
	$"0000 4B09 0000 5108 0043 616E 6365 6C00"        /* ..K∆..Q..Cancel. */
	$"0000 0020 0000 4CAC 5361 7665 2076 6172"        /* ... ..L¨Save var */
	$"6961 626C 6573 2069 6E20 6669 6C65 3A"          /* iables in file: */
};

/* for the benifit of MultiFinder.. */
	/* 12th bit (2^0 == 0th) is "can background", 14th is */
	/*  does accept suspend and resume events */
	/*     { this  } is the (long) prefered memory size */
	/*  and          { this  } is the minimum memory size */
	/* we are currently playing it safe with these (160K) */
data 'SIZE' (-1) {
	$"5000 0004 0000 0004 0000"                       /* X......... */
};
data 'SIZE' (0) {
	$"5000 0004 0000 0004 0000"                       /* X......... */
};

/* so that the Apple keyboard doesn't do dead keys on OPTION */
resource 'KCHR' (13000, "US-NoDeadKeys", sysheap, locked) {
	13000,
	{	/* array: 256 elements */
		/* [1] */
		0,
		/* [2] */
		0,
		/* [3] */
		1,
		/* [4] */
		0,
		/* [5] */
		2,
		/* [6] */
		2,
		/* [7] */
		1,
		/* [8] */
		0,
		/* [9] */
		3,
		/* [10] */
		6,
		/* [11] */
		4,
		/* [12] */
		4,
		/* [13] */
		5,
		/* [14] */
		5,
		/* [15] */
		4,
		/* [16] */
		4,
		/* [17] */
		7,
		/* [18] */
		7,
		/* [19] */
		7,
		/* [20] */
		7,
		/* [21] */
		7,
		/* [22] */
		7,
		/* [23] */
		7,
		/* [24] */
		7,
		/* [25] */
		7,
		/* [26] */
		7,
		/* [27] */
		7,
		/* [28] */
		7,
		/* [29] */
		7,
		/* [30] */
		7,
		/* [31] */
		7,
		/* [32] */
		7,
		/* [33] */
		1,
		/* [34] */
		0,
		/* [35] */
		1,
		/* [36] */
		0,
		/* [37] */
		1,
		/* [38] */
		0,
		/* [39] */
		1,
		/* [40] */
		0,
		/* [41] */
		4,
		/* [42] */
		4,
		/* [43] */
		4,
		/* [44] */
		4,
		/* [45] */
		4,
		/* [46] */
		4,
		/* [47] */
		4,
		/* [48] */
		4,
		/* [49] */
		7,
		/* [50] */
		7,
		/* [51] */
		7,
		/* [52] */
		7,
		/* [53] */
		7,
		/* [54] */
		7,
		/* [55] */
		7,
		/* [56] */
		7,
		/* [57] */
		7,
		/* [58] */
		7,
		/* [59] */
		7,
		/* [60] */
		7,
		/* [61] */
		7,
		/* [62] */
		7,
		/* [63] */
		7,
		/* [64] */
		7,
		/* [65] */
		3,
		/* [66] */
		6,
		/* [67] */
		4,
		/* [68] */
		4,
		/* [69] */
		5,
		/* [70] */
		5,
		/* [71] */
		4,
		/* [72] */
		4,
		/* [73] */
		3,
		/* [74] */
		6,
		/* [75] */
		4,
		/* [76] */
		4,
		/* [77] */
		5,
		/* [78] */
		5,
		/* [79] */
		4,
		/* [80] */
		4,
		/* [81] */
		7,
		/* [82] */
		7,
		/* [83] */
		7,
		/* [84] */
		7,
		/* [85] */
		7,
		/* [86] */
		7,
		/* [87] */
		7,
		/* [88] */
		7,
		/* [89] */
		7,
		/* [90] */
		7,
		/* [91] */
		7,
		/* [92] */
		7,
		/* [93] */
		7,
		/* [94] */
		7,
		/* [95] */
		7,
		/* [96] */
		7,
		/* [97] */
		4,
		/* [98] */
		4,
		/* [99] */
		4,
		/* [100] */
		4,
		/* [101] */
		4,
		/* [102] */
		4,
		/* [103] */
		4,
		/* [104] */
		4,
		/* [105] */
		4,
		/* [106] */
		4,
		/* [107] */
		4,
		/* [108] */
		4,
		/* [109] */
		4,
		/* [110] */
		4,
		/* [111] */
		4,
		/* [112] */
		4,
		/* [113] */
		7,
		/* [114] */
		7,
		/* [115] */
		7,
		/* [116] */
		7,
		/* [117] */
		7,
		/* [118] */
		7,
		/* [119] */
		7,
		/* [120] */
		7,
		/* [121] */
		7,
		/* [122] */
		7,
		/* [123] */
		7,
		/* [124] */
		7,
		/* [125] */
		7,
		/* [126] */
		7,
		/* [127] */
		7,
		/* [128] */
		7,
		/* [129] */
		7,
		/* [130] */
		7,
		/* [131] */
		7,
		/* [132] */
		7,
		/* [133] */
		7,
		/* [134] */
		7,
		/* [135] */
		7,
		/* [136] */
		7,
		/* [137] */
		7,
		/* [138] */
		7,
		/* [139] */
		7,
		/* [140] */
		7,
		/* [141] */
		7,
		/* [142] */
		7,
		/* [143] */
		7,
		/* [144] */
		7,
		/* [145] */
		7,
		/* [146] */
		7,
		/* [147] */
		7,
		/* [148] */
		7,
		/* [149] */
		7,
		/* [150] */
		7,
		/* [151] */
		7,
		/* [152] */
		7,
		/* [153] */
		7,
		/* [154] */
		7,
		/* [155] */
		7,
		/* [156] */
		7,
		/* [157] */
		7,
		/* [158] */
		7,
		/* [159] */
		7,
		/* [160] */
		7,
		/* [161] */
		7,
		/* [162] */
		7,
		/* [163] */
		7,
		/* [164] */
		7,
		/* [165] */
		7,
		/* [166] */
		7,
		/* [167] */
		7,
		/* [168] */
		7,
		/* [169] */
		7,
		/* [170] */
		7,
		/* [171] */
		7,
		/* [172] */
		7,
		/* [173] */
		7,
		/* [174] */
		7,
		/* [175] */
		7,
		/* [176] */
		7,
		/* [177] */
		7,
		/* [178] */
		7,
		/* [179] */
		7,
		/* [180] */
		7,
		/* [181] */
		7,
		/* [182] */
		7,
		/* [183] */
		7,
		/* [184] */
		7,
		/* [185] */
		7,
		/* [186] */
		7,
		/* [187] */
		7,
		/* [188] */
		7,
		/* [189] */
		7,
		/* [190] */
		7,
		/* [191] */
		7,
		/* [192] */
		7,
		/* [193] */
		7,
		/* [194] */
		7,
		/* [195] */
		7,
		/* [196] */
		7,
		/* [197] */
		7,
		/* [198] */
		7,
		/* [199] */
		7,
		/* [200] */
		7,
		/* [201] */
		7,
		/* [202] */
		7,
		/* [203] */
		7,
		/* [204] */
		7,
		/* [205] */
		7,
		/* [206] */
		7,
		/* [207] */
		7,
		/* [208] */
		7,
		/* [209] */
		7,
		/* [210] */
		7,
		/* [211] */
		7,
		/* [212] */
		7,
		/* [213] */
		7,
		/* [214] */
		7,
		/* [215] */
		7,
		/* [216] */
		7,
		/* [217] */
		7,
		/* [218] */
		7,
		/* [219] */
		7,
		/* [220] */
		7,
		/* [221] */
		7,
		/* [222] */
		7,
		/* [223] */
		7,
		/* [224] */
		7,
		/* [225] */
		7,
		/* [226] */
		7,
		/* [227] */
		7,
		/* [228] */
		7,
		/* [229] */
		7,
		/* [230] */
		7,
		/* [231] */
		7,
		/* [232] */
		7,
		/* [233] */
		7,
		/* [234] */
		7,
		/* [235] */
		7,
		/* [236] */
		7,
		/* [237] */
		7,
		/* [238] */
		7,
		/* [239] */
		7,
		/* [240] */
		7,
		/* [241] */
		7,
		/* [242] */
		7,
		/* [243] */
		7,
		/* [244] */
		7,
		/* [245] */
		7,
		/* [246] */
		7,
		/* [247] */
		7,
		/* [248] */
		7,
		/* [249] */
		7,
		/* [250] */
		7,
		/* [251] */
		7,
		/* [252] */
		7,
		/* [253] */
		7,
		/* [254] */
		7,
		/* [255] */
		7,
		/* [256] */
		7
	},
	{	/* array TableArray: 8 elements */
		/* [1] */
		{	/* array: 128 elements */
			/* [1] */
			"a",
			/* [2] */
			"s",
			/* [3] */
			"d",
			/* [4] */
			"f",
			/* [5] */
			"h",
			/* [6] */
			"g",
			/* [7] */
			"z",
			/* [8] */
			"x",
			/* [9] */
			"c",
			/* [10] */
			"v",
			/* [11] */
			"",
			/* [12] */
			"b",
			/* [13] */
			"q",
			/* [14] */
			"w",
			/* [15] */
			"e",
			/* [16] */
			"r",
			/* [17] */
			"y",
			/* [18] */
			"t",
			/* [19] */
			"1",
			/* [20] */
			"2",
			/* [21] */
			"3",
			/* [22] */
			"4",
			/* [23] */
			"6",
			/* [24] */
			"5",
			/* [25] */
			"=",
			/* [26] */
			"9",
			/* [27] */
			"7",
			/* [28] */
			"-",
			/* [29] */
			"8",
			/* [30] */
			"0",
			/* [31] */
			"]",
			/* [32] */
			"o",
			/* [33] */
			"u",
			/* [34] */
			"[",
			/* [35] */
			"i",
			/* [36] */
			"p",
			/* [37] */
			"\n",
			/* [38] */
			"l",
			/* [39] */
			"j",
			/* [40] */
			"'",
			/* [41] */
			"k",
			/* [42] */
			";",
			/* [43] */
			"\\",
			/* [44] */
			",",
			/* [45] */
			"/",
			/* [46] */
			"n",
			/* [47] */
			"m",
			/* [48] */
			".",
			/* [49] */
			"\t",
			/* [50] */
			" ",
			/* [51] */
			"`",
			/* [52] */
			"\b",
			/* [53] */
			"\0x03",
			/* [54] */
			"\0x1B",
			/* [55] */
			"",
			/* [56] */
			"",
			/* [57] */
			"",
			/* [58] */
			"",
			/* [59] */
			"",
			/* [60] */
			"",
			/* [61] */
			"",
			/* [62] */
			"",
			/* [63] */
			"",
			/* [64] */
			"",
			/* [65] */
			"",
			/* [66] */
			".",
			/* [67] */
			"\0x1D",
			/* [68] */
			"*",
			/* [69] */
			"",
			/* [70] */
			"+",
			/* [71] */
			"\0x1C",
			/* [72] */
			"\0x1B",
			/* [73] */
			"\0x1F",
			/* [74] */
			"",
			/* [75] */
			"",
			/* [76] */
			"/",
			/* [77] */
			"\0x03",
			/* [78] */
			"\0x1E",
			/* [79] */
			"-",
			/* [80] */
			"",
			/* [81] */
			"",
			/* [82] */
			"=",
			/* [83] */
			"0",
			/* [84] */
			"1",
			/* [85] */
			"2",
			/* [86] */
			"3",
			/* [87] */
			"4",
			/* [88] */
			"5",
			/* [89] */
			"6",
			/* [90] */
			"7",
			/* [91] */
			"",
			/* [92] */
			"8",
			/* [93] */
			"9",
			/* [94] */
			"",
			/* [95] */
			"",
			/* [96] */
			"",
			/* [97] */
			"\0x10",
			/* [98] */
			"\0x10",
			/* [99] */
			"\0x10",
			/* [100] */
			"\0x10",
			/* [101] */
			"\0x10",
			/* [102] */
			"\0x10",
			/* [103] */
			"\0x10",
			/* [104] */
			"\0x10",
			/* [105] */
			"\0x10",
			/* [106] */
			"\0x10",
			/* [107] */
			"\0x10",
			/* [108] */
			"\0x10",
			/* [109] */
			"\0x10",
			/* [110] */
			"\0x10",
			/* [111] */
			"\0x10",
			/* [112] */
			"\0x10",
			/* [113] */
			"\0x10",
			/* [114] */
			"\0x10",
			/* [115] */
			"\0x05",
			/* [116] */
			"\0x01",
			/* [117] */
			"\v",
			/* [118] */
			"\?",
			/* [119] */
			"\0x10",
			/* [120] */
			"\0x04",
			/* [121] */
			"\0x10",
			/* [122] */
			"\f",
			/* [123] */
			"\0x10",
			/* [124] */
			"\0x1C",
			/* [125] */
			"\0x1D",
			/* [126] */
			"\0x1F",
			/* [127] */
			"\0x1E",
			/* [128] */
			""
		},
		/* [2] */
		{	/* array: 128 elements */
			/* [1] */
			"A",
			/* [2] */
			"S",
			/* [3] */
			"D",
			/* [4] */
			"F",
			/* [5] */
			"H",
			/* [6] */
			"G",
			/* [7] */
			"Z",
			/* [8] */
			"X",
			/* [9] */
			"C",
			/* [10] */
			"V",
			/* [11] */
			"",
			/* [12] */
			"B",
			/* [13] */
			"Q",
			/* [14] */
			"W",
			/* [15] */
			"E",
			/* [16] */
			"R",
			/* [17] */
			"Y",
			/* [18] */
			"T",
			/* [19] */
			"!",
			/* [20] */
			"@",
			/* [21] */
			"#",
			/* [22] */
			"$",
			/* [23] */
			"^",
			/* [24] */
			"%",
			/* [25] */
			"+",
			/* [26] */
			"(",
			/* [27] */
			"&",
			/* [28] */
			"_",
			/* [29] */
			"*",
			/* [30] */
			")",
			/* [31] */
			"}",
			/* [32] */
			"O",
			/* [33] */
			"U",
			/* [34] */
			"{",
			/* [35] */
			"I",
			/* [36] */
			"P",
			/* [37] */
			"\n",
			/* [38] */
			"L",
			/* [39] */
			"J",
			/* [40] */
			"\"",
			/* [41] */
			"K",
			/* [42] */
			":",
			/* [43] */
			"|",
			/* [44] */
			"<",
			/* [45] */
			"?",
			/* [46] */
			"N",
			/* [47] */
			"M",
			/* [48] */
			">",
			/* [49] */
			"\t",
			/* [50] */
			" ",
			/* [51] */
			"~",
			/* [52] */
			"\b",
			/* [53] */
			"\0x03",
			/* [54] */
			"\0x1B",
			/* [55] */
			"",
			/* [56] */
			"",
			/* [57] */
			"",
			/* [58] */
			"",
			/* [59] */
			"",
			/* [60] */
			"",
			/* [61] */
			"",
			/* [62] */
			"",
			/* [63] */
			"",
			/* [64] */
			"",
			/* [65] */
			"",
			/* [66] */
			".",
			/* [67] */
			"*",
			/* [68] */
			"*",
			/* [69] */
			"",
			/* [70] */
			"+",
			/* [71] */
			"+",
			/* [72] */
			"\0x1B",
			/* [73] */
			"=",
			/* [74] */
			"",
			/* [75] */
			"",
			/* [76] */
			"/",
			/* [77] */
			"\0x03",
			/* [78] */
			"/",
			/* [79] */
			"-",
			/* [80] */
			"",
			/* [81] */
			"",
			/* [82] */
			"=",
			/* [83] */
			"0",
			/* [84] */
			"1",
			/* [85] */
			"2",
			/* [86] */
			"3",
			/* [87] */
			"4",
			/* [88] */
			"5",
			/* [89] */
			"6",
			/* [90] */
			"7",
			/* [91] */
			"",
			/* [92] */
			"8",
			/* [93] */
			"9",
			/* [94] */
			"",
			/* [95] */
			"",
			/* [96] */
			"",
			/* [97] */
			"\0x10",
			/* [98] */
			"\0x10",
			/* [99] */
			"\0x10",
			/* [100] */
			"\0x10",
			/* [101] */
			"\0x10",
			/* [102] */
			"\0x10",
			/* [103] */
			"\0x10",
			/* [104] */
			"\0x10",
			/* [105] */
			"\0x10",
			/* [106] */
			"\0x10",
			/* [107] */
			"\0x10",
			/* [108] */
			"\0x10",
			/* [109] */
			"\0x10",
			/* [110] */
			"\0x10",
			/* [111] */
			"\0x10",
			/* [112] */
			"\0x10",
			/* [113] */
			"\0x10",
			/* [114] */
			"\0x10",
			/* [115] */
			"\0x05",
			/* [116] */
			"\0x01",
			/* [117] */
			"\v",
			/* [118] */
			"\?",
			/* [119] */
			"\0x10",
			/* [120] */
			"\0x04",
			/* [121] */
			"\0x10",
			/* [122] */
			"\f",
			/* [123] */
			"\0x10",
			/* [124] */
			"\0x1C",
			/* [125] */
			"\0x1D",
			/* [126] */
			"\0x1F",
			/* [127] */
			"\0x1E",
			/* [128] */
			""
		},
		/* [3] */
		{	/* array: 128 elements */
			/* [1] */
			"A",
			/* [2] */
			"S",
			/* [3] */
			"D",
			/* [4] */
			"F",
			/* [5] */
			"H",
			/* [6] */
			"G",
			/* [7] */
			"Z",
			/* [8] */
			"X",
			/* [9] */
			"C",
			/* [10] */
			"V",
			/* [11] */
			"",
			/* [12] */
			"B",
			/* [13] */
			"Q",
			/* [14] */
			"W",
			/* [15] */
			"E",
			/* [16] */
			"R",
			/* [17] */
			"Y",
			/* [18] */
			"T",
			/* [19] */
			"1",
			/* [20] */
			"2",
			/* [21] */
			"3",
			/* [22] */
			"4",
			/* [23] */
			"6",
			/* [24] */
			"5",
			/* [25] */
			"=",
			/* [26] */
			"9",
			/* [27] */
			"7",
			/* [28] */
			"-",
			/* [29] */
			"8",
			/* [30] */
			"0",
			/* [31] */
			"]",
			/* [32] */
			"O",
			/* [33] */
			"U",
			/* [34] */
			"[",
			/* [35] */
			"I",
			/* [36] */
			"P",
			/* [37] */
			"\n",
			/* [38] */
			"L",
			/* [39] */
			"J",
			/* [40] */
			"'",
			/* [41] */
			"K",
			/* [42] */
			";",
			/* [43] */
			"\\",
			/* [44] */
			",",
			/* [45] */
			"/",
			/* [46] */
			"N",
			/* [47] */
			"M",
			/* [48] */
			".",
			/* [49] */
			"\t",
			/* [50] */
			" ",
			/* [51] */
			"`",
			/* [52] */
			"\b",
			/* [53] */
			"\0x03",
			/* [54] */
			"\0x1B",
			/* [55] */
			"",
			/* [56] */
			"",
			/* [57] */
			"",
			/* [58] */
			"",
			/* [59] */
			"",
			/* [60] */
			"",
			/* [61] */
			"",
			/* [62] */
			"",
			/* [63] */
			"",
			/* [64] */
			"",
			/* [65] */
			"",
			/* [66] */
			".",
			/* [67] */
			"\0x1D",
			/* [68] */
			"*",
			/* [69] */
			"",
			/* [70] */
			"+",
			/* [71] */
			"\0x1C",
			/* [72] */
			"\0x1B",
			/* [73] */
			"\0x1F",
			/* [74] */
			"",
			/* [75] */
			"",
			/* [76] */
			"/",
			/* [77] */
			"\0x03",
			/* [78] */
			"\0x1E",
			/* [79] */
			"-",
			/* [80] */
			"",
			/* [81] */
			"",
			/* [82] */
			"=",
			/* [83] */
			"0",
			/* [84] */
			"1",
			/* [85] */
			"2",
			/* [86] */
			"3",
			/* [87] */
			"4",
			/* [88] */
			"5",
			/* [89] */
			"6",
			/* [90] */
			"7",
			/* [91] */
			"",
			/* [92] */
			"8",
			/* [93] */
			"9",
			/* [94] */
			"",
			/* [95] */
			"",
			/* [96] */
			"",
			/* [97] */
			"\0x10",
			/* [98] */
			"\0x10",
			/* [99] */
			"\0x10",
			/* [100] */
			"\0x10",
			/* [101] */
			"\0x10",
			/* [102] */
			"\0x10",
			/* [103] */
			"\0x10",
			/* [104] */
			"\0x10",
			/* [105] */
			"\0x10",
			/* [106] */
			"\0x10",
			/* [107] */
			"\0x10",
			/* [108] */
			"\0x10",
			/* [109] */
			"\0x10",
			/* [110] */
			"\0x10",
			/* [111] */
			"\0x10",
			/* [112] */
			"\0x10",
			/* [113] */
			"\0x10",
			/* [114] */
			"\0x10",
			/* [115] */
			"\0x05",
			/* [116] */
			"\0x01",
			/* [117] */
			"\v",
			/* [118] */
			"\?",
			/* [119] */
			"\0x10",
			/* [120] */
			"\0x04",
			/* [121] */
			"\0x10",
			/* [122] */
			"\f",
			/* [123] */
			"\0x10",
			/* [124] */
			"\0x1C",
			/* [125] */
			"\0x1D",
			/* [126] */
			"\0x1F",
			/* [127] */
			"\0x1E",
			/* [128] */
			""
		},
		/* [4] */
		{	/* array: 128 elements */
			/* [1] */
			"å",
			/* [2] */
			"ß",
			/* [3] */
			"∂",
			/* [4] */
			"ƒ",
			/* [5] */
			"\0xFA",
			/* [6] */
			"©",
			/* [7] */
			"Ω",
			/* [8] */
			"≈",
			/* [9] */
			"ç",
			/* [10] */
			"√",
			/* [11] */
			"",
			/* [12] */
			"∫",
			/* [13] */
			"œ",
			/* [14] */
			"∑",
			/* [15] */
			"é",
			/* [16] */
			"®",
			/* [17] */
			"¥",
			/* [18] */
			"†",
			/* [19] */
			"¡",
			/* [20] */
			"™",
			/* [21] */
			"£",
			/* [22] */
			"¢",
			/* [23] */
			"§",
			/* [24] */
			"∞",
			/* [25] */
			"≠",
			/* [26] */
			"ª",
			/* [27] */
			"¶",
			/* [28] */
			"–",
			/* [29] */
			"•",
			/* [30] */
			"º",
			/* [31] */
			"‘",
			/* [32] */
			"ø",
			/* [33] */
			"ü",
			/* [34] */
			"“",
			/* [35] */
			"î",
			/* [36] */
			"π",
			/* [37] */
			"\n",
			/* [38] */
			"¬",
			/* [39] */
			"∆",
			/* [40] */
			"æ",
			/* [41] */
			"\0xFB",
			/* [42] */
			"…",
			/* [43] */
			"«",
			/* [44] */
			"≤",
			/* [45] */
			"÷",
			/* [46] */
			"ñ",
			/* [47] */
			"µ",
			/* [48] */
			"≥",
			/* [49] */
			"\t",
			/* [50] */
			" ",
			/* [51] */
			"è",
			/* [52] */
			"\b",
			/* [53] */
			"\0x03",
			/* [54] */
			"\0x1B",
			/* [55] */
			"",
			/* [56] */
			"",
			/* [57] */
			"",
			/* [58] */
			"",
			/* [59] */
			"",
			/* [60] */
			"",
			/* [61] */
			"",
			/* [62] */
			"",
			/* [63] */
			"",
			/* [64] */
			"",
			/* [65] */
			"",
			/* [66] */
			".",
			/* [67] */
			"\0x1D",
			/* [68] */
			"*",
			/* [69] */
			"",
			/* [70] */
			"+",
			/* [71] */
			"\0x1C",
			/* [72] */
			"\0x1B",
			/* [73] */
			"\0x1F",
			/* [74] */
			"",
			/* [75] */
			"",
			/* [76] */
			"/",
			/* [77] */
			"\0x03",
			/* [78] */
			"\0x1E",
			/* [79] */
			"-",
			/* [80] */
			"",
			/* [81] */
			"",
			/* [82] */
			"=",
			/* [83] */
			"0",
			/* [84] */
			"1",
			/* [85] */
			"2",
			/* [86] */
			"3",
			/* [87] */
			"4",
			/* [88] */
			"5",
			/* [89] */
			"6",
			/* [90] */
			"7",
			/* [91] */
			"",
			/* [92] */
			"8",
			/* [93] */
			"9",
			/* [94] */
			"",
			/* [95] */
			"",
			/* [96] */
			"",
			/* [97] */
			"\0x10",
			/* [98] */
			"\0x10",
			/* [99] */
			"\0x10",
			/* [100] */
			"\0x10",
			/* [101] */
			"\0x10",
			/* [102] */
			"\0x10",
			/* [103] */
			"\0x10",
			/* [104] */
			"\0x10",
			/* [105] */
			"\0x10",
			/* [106] */
			"\0x10",
			/* [107] */
			"\0x10",
			/* [108] */
			"\0x10",
			/* [109] */
			"\0x10",
			/* [110] */
			"\0x10",
			/* [111] */
			"\0x10",
			/* [112] */
			"\0x10",
			/* [113] */
			"\0x10",
			/* [114] */
			"\0x10",
			/* [115] */
			"\0x05",
			/* [116] */
			"\0x01",
			/* [117] */
			"\v",
			/* [118] */
			"\?",
			/* [119] */
			"\0x10",
			/* [120] */
			"\0x04",
			/* [121] */
			"\0x10",
			/* [122] */
			"\f",
			/* [123] */
			"\0x10",
			/* [124] */
			"\0x1C",
			/* [125] */
			"\0x1D",
			/* [126] */
			"\0x1F",
			/* [127] */
			"\0x1E",
			/* [128] */
			""
		},
		/* [5] */
		{	/* array: 128 elements */
			/* [1] */
			"Å",
			/* [2] */
			"\0xEA",
			/* [3] */
			"\0xEB",
			/* [4] */
			"\0xEC",
			/* [5] */
			"\0xEE",
			/* [6] */
			"\0xED",
			/* [7] */
			"\0xF3",
			/* [8] */
			"\0xF4",
			/* [9] */
			"Ç",
			/* [10] */
			"◊",
			/* [11] */
			"",
			/* [12] */
			"\0xF5",
			/* [13] */
			"Œ",
			/* [14] */
			"\0xE3",
			/* [15] */
			"\0xE4",
			/* [16] */
			"\0xE5",
			/* [17] */
			"\0xE7",
			/* [18] */
			"\0xE6",
			/* [19] */
			"\0xDA",
			/* [20] */
			"\0xDB",
			/* [21] */
			"\0xDC",
			/* [22] */
			"\0xDD",
			/* [23] */
			"\0xDF",
			/* [24] */
			"\0xDE",
			/* [25] */
			"±",
			/* [26] */
			"\0xE1",
			/* [27] */
			"\0xE0",
			/* [28] */
			"—",
			/* [29] */
			"°",
			/* [30] */
			"\0xE2",
			/* [31] */
			"’",
			/* [32] */
			"Ø",
			/* [33] */
			"\0xE8",
			/* [34] */
			"”",
			/* [35] */
			"\0xE9",
			/* [36] */
			"∏",
			/* [37] */
			"\n",
			/* [38] */
			"\0xF1",
			/* [39] */
			"\0xEF",
			/* [40] */
			"Æ",
			/* [41] */
			"\0xF0",
			/* [42] */
			"\0xF2",
			/* [43] */
			"»",
			/* [44] */
			"\0xF8",
			/* [45] */
			"¿",
			/* [46] */
			"\0xF6",
			/* [47] */
			"\0xF7",
			/* [48] */
			"\0xF9",
			/* [49] */
			"\t",
			/* [50] */
			" ",
			/* [51] */
			"\0xD9",
			/* [52] */
			"\b",
			/* [53] */
			"\0x03",
			/* [54] */
			"\0x1B",
			/* [55] */
			"",
			/* [56] */
			"",
			/* [57] */
			"",
			/* [58] */
			"",
			/* [59] */
			"",
			/* [60] */
			"",
			/* [61] */
			"",
			/* [62] */
			"",
			/* [63] */
			"",
			/* [64] */
			"",
			/* [65] */
			"",
			/* [66] */
			".",
			/* [67] */
			"*",
			/* [68] */
			"*",
			/* [69] */
			"",
			/* [70] */
			"+",
			/* [71] */
			"+",
			/* [72] */
			"\0x1B",
			/* [73] */
			"=",
			/* [74] */
			"",
			/* [75] */
			"",
			/* [76] */
			"/",
			/* [77] */
			"\0x03",
			/* [78] */
			"/",
			/* [79] */
			"-",
			/* [80] */
			"",
			/* [81] */
			"",
			/* [82] */
			"=",
			/* [83] */
			"0",
			/* [84] */
			"1",
			/* [85] */
			"2",
			/* [86] */
			"3",
			/* [87] */
			"4",
			/* [88] */
			"5",
			/* [89] */
			"6",
			/* [90] */
			"7",
			/* [91] */
			"",
			/* [92] */
			"8",
			/* [93] */
			"9",
			/* [94] */
			"",
			/* [95] */
			"",
			/* [96] */
			"",
			/* [97] */
			"\0x10",
			/* [98] */
			"\0x10",
			/* [99] */
			"\0x10",
			/* [100] */
			"\0x10",
			/* [101] */
			"\0x10",
			/* [102] */
			"\0x10",
			/* [103] */
			"\0x10",
			/* [104] */
			"\0x10",
			/* [105] */
			"\0x10",
			/* [106] */
			"\0x10",
			/* [107] */
			"\0x10",
			/* [108] */
			"\0x10",
			/* [109] */
			"\0x10",
			/* [110] */
			"\0x10",
			/* [111] */
			"\0x10",
			/* [112] */
			"\0x10",
			/* [113] */
			"\0x10",
			/* [114] */
			"\0x10",
			/* [115] */
			"\0x05",
			/* [116] */
			"\0x01",
			/* [117] */
			"\v",
			/* [118] */
			"\?",
			/* [119] */
			"\0x10",
			/* [120] */
			"\0x04",
			/* [121] */
			"\0x10",
			/* [122] */
			"\f",
			/* [123] */
			"\0x10",
			/* [124] */
			"\0x1C",
			/* [125] */
			"\0x1D",
			/* [126] */
			"\0x1F",
			/* [127] */
			"\0x1E",
			/* [128] */
			""
		},
		/* [6] */
		{	/* array: 128 elements */
			/* [1] */
			"Å",
			/* [2] */
			"\0xEA",
			/* [3] */
			"\0xEB",
			/* [4] */
			"\0xEC",
			/* [5] */
			"\0xEE",
			/* [6] */
			"\0xED",
			/* [7] */
			"\0xF3",
			/* [8] */
			"\0xF4",
			/* [9] */
			"Ç",
			/* [10] */
			"◊",
			/* [11] */
			"",
			/* [12] */
			"\0xF5",
			/* [13] */
			"Œ",
			/* [14] */
			"\0xE3",
			/* [15] */
			"\0xE4",
			/* [16] */
			"\0xE5",
			/* [17] */
			"\0xE7",
			/* [18] */
			"\0xE6",
			/* [19] */
			"¡",
			/* [20] */
			"™",
			/* [21] */
			"£",
			/* [22] */
			"¢",
			/* [23] */
			"§",
			/* [24] */
			"∞",
			/* [25] */
			"≠",
			/* [26] */
			"ª",
			/* [27] */
			"¶",
			/* [28] */
			"–",
			/* [29] */
			"•",
			/* [30] */
			"º",
			/* [31] */
			"‘",
			/* [32] */
			"Ø",
			/* [33] */
			"\0xE8",
			/* [34] */
			"“",
			/* [35] */
			"\0xE9",
			/* [36] */
			"∏",
			/* [37] */
			"\n",
			/* [38] */
			"\0xF1",
			/* [39] */
			"\0xEF",
			/* [40] */
			"Æ",
			/* [41] */
			"\0xF0",
			/* [42] */
			"…",
			/* [43] */
			"«",
			/* [44] */
			"≤",
			/* [45] */
			"÷",
			/* [46] */
			"\0xF6",
			/* [47] */
			"\0xF7",
			/* [48] */
			"≥",
			/* [49] */
			"\t",
			/* [50] */
			" ",
			/* [51] */
			"`",
			/* [52] */
			"\b",
			/* [53] */
			"\0x03",
			/* [54] */
			"\0x1B",
			/* [55] */
			"",
			/* [56] */
			"",
			/* [57] */
			"",
			/* [58] */
			"",
			/* [59] */
			"",
			/* [60] */
			"",
			/* [61] */
			"",
			/* [62] */
			"",
			/* [63] */
			"",
			/* [64] */
			"",
			/* [65] */
			"",
			/* [66] */
			".",
			/* [67] */
			"\0x1D",
			/* [68] */
			"*",
			/* [69] */
			"",
			/* [70] */
			"+",
			/* [71] */
			"\0x1C",
			/* [72] */
			"\0x1B",
			/* [73] */
			"\0x1F",
			/* [74] */
			"",
			/* [75] */
			"",
			/* [76] */
			"/",
			/* [77] */
			"\0x03",
			/* [78] */
			"\0x1E",
			/* [79] */
			"-",
			/* [80] */
			"",
			/* [81] */
			"",
			/* [82] */
			"=",
			/* [83] */
			"0",
			/* [84] */
			"1",
			/* [85] */
			"2",
			/* [86] */
			"3",
			/* [87] */
			"4",
			/* [88] */
			"5",
			/* [89] */
			"6",
			/* [90] */
			"7",
			/* [91] */
			"",
			/* [92] */
			"8",
			/* [93] */
			"9",
			/* [94] */
			"",
			/* [95] */
			"",
			/* [96] */
			"",
			/* [97] */
			"\0x10",
			/* [98] */
			"\0x10",
			/* [99] */
			"\0x10",
			/* [100] */
			"\0x10",
			/* [101] */
			"\0x10",
			/* [102] */
			"\0x10",
			/* [103] */
			"\0x10",
			/* [104] */
			"\0x10",
			/* [105] */
			"\0x10",
			/* [106] */
			"\0x10",
			/* [107] */
			"\0x10",
			/* [108] */
			"\0x10",
			/* [109] */
			"\0x10",
			/* [110] */
			"\0x10",
			/* [111] */
			"\0x10",
			/* [112] */
			"\0x10",
			/* [113] */
			"\0x10",
			/* [114] */
			"\0x10",
			/* [115] */
			"\0x05",
			/* [116] */
			"\0x01",
			/* [117] */
			"\v",
			/* [118] */
			"\?",
			/* [119] */
			"\0x10",
			/* [120] */
			"\0x04",
			/* [121] */
			"\0x10",
			/* [122] */
			"\f",
			/* [123] */
			"\0x10",
			/* [124] */
			"\0x1C",
			/* [125] */
			"\0x1D",
			/* [126] */
			"\0x1F",
			/* [127] */
			"\0x1E",
			/* [128] */
			""
		},
		/* [7] */
		{	/* array: 128 elements */
			/* [1] */
			"å",
			/* [2] */
			"ß",
			/* [3] */
			"∂",
			/* [4] */
			"ƒ",
			/* [5] */
			"\0xFA",
			/* [6] */
			"©",
			/* [7] */
			"Ω",
			/* [8] */
			"≈",
			/* [9] */
			"ç",
			/* [10] */
			"√",
			/* [11] */
			"",
			/* [12] */
			"∫",
			/* [13] */
			"œ",
			/* [14] */
			"∑",
			/* [15] */
			"´",
			/* [16] */
			"®",
			/* [17] */
			"¥",
			/* [18] */
			"†",
			/* [19] */
			"¡",
			/* [20] */
			"™",
			/* [21] */
			"£",
			/* [22] */
			"¢",
			/* [23] */
			"§",
			/* [24] */
			"∞",
			/* [25] */
			"≠",
			/* [26] */
			"ª",
			/* [27] */
			"¶",
			/* [28] */
			"–",
			/* [29] */
			"•",
			/* [30] */
			"º",
			/* [31] */
			"‘",
			/* [32] */
			"ø",
			/* [33] */
			"¨",
			/* [34] */
			"“",
			/* [35] */
			"^",
			/* [36] */
			"π",
			/* [37] */
			"\n",
			/* [38] */
			"¬",
			/* [39] */
			"∆",
			/* [40] */
			"æ",
			/* [41] */
			"\0xFB",
			/* [42] */
			"…",
			/* [43] */
			"«",
			/* [44] */
			"≤",
			/* [45] */
			"÷",
			/* [46] */
			"~",
			/* [47] */
			"µ",
			/* [48] */
			"≥",
			/* [49] */
			"\t",
			/* [50] */
			" ",
			/* [51] */
			"`",
			/* [52] */
			"\b",
			/* [53] */
			"\0x03",
			/* [54] */
			"\0x1B",
			/* [55] */
			"",
			/* [56] */
			"",
			/* [57] */
			"",
			/* [58] */
			"",
			/* [59] */
			"",
			/* [60] */
			"",
			/* [61] */
			"",
			/* [62] */
			"",
			/* [63] */
			"",
			/* [64] */
			"",
			/* [65] */
			"",
			/* [66] */
			".",
			/* [67] */
			"\0x1D",
			/* [68] */
			"*",
			/* [69] */
			"",
			/* [70] */
			"+",
			/* [71] */
			"\0x1C",
			/* [72] */
			"\0x1B",
			/* [73] */
			"\0x1F",
			/* [74] */
			"",
			/* [75] */
			"",
			/* [76] */
			"/",
			/* [77] */
			"\0x03",
			/* [78] */
			"\0x1E",
			/* [79] */
			"-",
			/* [80] */
			"",
			/* [81] */
			"",
			/* [82] */
			"=",
			/* [83] */
			"0",
			/* [84] */
			"1",
			/* [85] */
			"2",
			/* [86] */
			"3",
			/* [87] */
			"4",
			/* [88] */
			"5",
			/* [89] */
			"6",
			/* [90] */
			"7",
			/* [91] */
			"",
			/* [92] */
			"8",
			/* [93] */
			"9",
			/* [94] */
			"",
			/* [95] */
			"",
			/* [96] */
			"",
			/* [97] */
			"\0x10",
			/* [98] */
			"\0x10",
			/* [99] */
			"\0x10",
			/* [100] */
			"\0x10",
			/* [101] */
			"\0x10",
			/* [102] */
			"\0x10",
			/* [103] */
			"\0x10",
			/* [104] */
			"\0x10",
			/* [105] */
			"\0x10",
			/* [106] */
			"\0x10",
			/* [107] */
			"\0x10",
			/* [108] */
			"\0x10",
			/* [109] */
			"\0x10",
			/* [110] */
			"\0x10",
			/* [111] */
			"\0x10",
			/* [112] */
			"\0x10",
			/* [113] */
			"\0x10",
			/* [114] */
			"\0x10",
			/* [115] */
			"\0x05",
			/* [116] */
			"\0x01",
			/* [117] */
			"\v",
			/* [118] */
			"\?",
			/* [119] */
			"\0x10",
			/* [120] */
			"\0x04",
			/* [121] */
			"\0x10",
			/* [122] */
			"\f",
			/* [123] */
			"\0x10",
			/* [124] */
			"\0x1C",
			/* [125] */
			"\0x1D",
			/* [126] */
			"\0x1F",
			/* [127] */
			"\0x1E",
			/* [128] */
			""
		},
		/* [8] */
		{	/* array: 128 elements */
			/* [1] */
			"\0x01",
			/* [2] */
			"\0x13",
			/* [3] */
			"\0x04",
			/* [4] */
			"\0x06",
			/* [5] */
			"\b",
			/* [6] */
			"\0x07",
			/* [7] */
			"\0x1A",
			/* [8] */
			"\0x18",
			/* [9] */
			"\0x03",
			/* [10] */
			"\0x16",
			/* [11] */
			"0",
			/* [12] */
			"\0x02",
			/* [13] */
			"\0x11",
			/* [14] */
			"\0x17",
			/* [15] */
			"\0x05",
			/* [16] */
			"\0x12",
			/* [17] */
			"\0x19",
			/* [18] */
			"\0x14",
			/* [19] */
			"1",
			/* [20] */
			"2",
			/* [21] */
			"3",
			/* [22] */
			"4",
			/* [23] */
			"6",
			/* [24] */
			"5",
			/* [25] */
			"=",
			/* [26] */
			"9",
			/* [27] */
			"7",
			/* [28] */
			"\0x1F",
			/* [29] */
			"8",
			/* [30] */
			"0",
			/* [31] */
			"\0x1D",
			/* [32] */
			"\0x0F",
			/* [33] */
			"\0x15",
			/* [34] */
			"\0x1B",
			/* [35] */
			"\t",
			/* [36] */
			"\0x10",
			/* [37] */
			"\n",
			/* [38] */
			"\f",
			/* [39] */
			"\0x0A",
			/* [40] */
			"'",
			/* [41] */
			"\v",
			/* [42] */
			";",
			/* [43] */
			"\0x1C",
			/* [44] */
			",",
			/* [45] */
			"/",
			/* [46] */
			"\0x0E",
			/* [47] */
			"\n",
			/* [48] */
			".",
			/* [49] */
			"\t",
			/* [50] */
			" ",
			/* [51] */
			"`",
			/* [52] */
			"\b",
			/* [53] */
			"\0x03",
			/* [54] */
			"\0x1B",
			/* [55] */
			"",
			/* [56] */
			"",
			/* [57] */
			"",
			/* [58] */
			"",
			/* [59] */
			"",
			/* [60] */
			"",
			/* [61] */
			"",
			/* [62] */
			"",
			/* [63] */
			"",
			/* [64] */
			"",
			/* [65] */
			"",
			/* [66] */
			".",
			/* [67] */
			"\0x1D",
			/* [68] */
			"*",
			/* [69] */
			"",
			/* [70] */
			"+",
			/* [71] */
			"\0x1C",
			/* [72] */
			"\0x1B",
			/* [73] */
			"\0x1F",
			/* [74] */
			"",
			/* [75] */
			"",
			/* [76] */
			"/",
			/* [77] */
			"\0x03",
			/* [78] */
			"\0x1E",
			/* [79] */
			"-",
			/* [80] */
			"",
			/* [81] */
			"",
			/* [82] */
			"=",
			/* [83] */
			"0",
			/* [84] */
			"1",
			/* [85] */
			"2",
			/* [86] */
			"3",
			/* [87] */
			"4",
			/* [88] */
			"5",
			/* [89] */
			"6",
			/* [90] */
			"7",
			/* [91] */
			"",
			/* [92] */
			"8",
			/* [93] */
			"9",
			/* [94] */
			"",
			/* [95] */
			"",
			/* [96] */
			"",
			/* [97] */
			"\0x10",
			/* [98] */
			"\0x10",
			/* [99] */
			"\0x10",
			/* [100] */
			"\0x10",
			/* [101] */
			"\0x10",
			/* [102] */
			"\0x10",
			/* [103] */
			"\0x10",
			/* [104] */
			"\0x10",
			/* [105] */
			"\0x10",
			/* [106] */
			"\0x10",
			/* [107] */
			"\0x10",
			/* [108] */
			"\0x10",
			/* [109] */
			"\0x10",
			/* [110] */
			"\0x10",
			/* [111] */
			"\0x10",
			/* [112] */
			"\0x10",
			/* [113] */
			"\0x10",
			/* [114] */
			"\0x10",
			/* [115] */
			"\0x05",
			/* [116] */
			"\0x01",
			/* [117] */
			"\v",
			/* [118] */
			"\?",
			/* [119] */
			"\0x10",
			/* [120] */
			"\0x04",
			/* [121] */
			"\0x10",
			/* [122] */
			"\f",
			/* [123] */
			"\0x10",
			/* [124] */
			"\0x1C",
			/* [125] */
			"\0x1D",
			/* [126] */
			"\0x1F",
			/* [127] */
			"\0x1E",
			/* [128] */
			""
		}
	},
	{	/* array DeadArray: 0 elements */
	}
};

/* and the icon for our new key map */
resource 'SICN' (13000) {
	{	/* array: 1 elements */
		/* [1] */
		$"0DD8 1634 1C1C 2002 5009 5819 4662 318C"
		$"0C30 1FFE 7AB2 C953 8AB1 8D79 8FF8 9FF8"
	}
};
