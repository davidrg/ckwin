#include "config_file.h"
#include "conn_profile.h"

typedef struct tagDefColorTheme {
	LPCTSTR name;
	RGB8BitColor black;
	RGB8BitColor blue;
	RGB8BitColor green;
	RGB8BitColor cyan;
	RGB8BitColor red;
	RGB8BitColor magenta;
	RGB8BitColor brown;
	RGB8BitColor lightGray;

	RGB8BitColor darkGray;
	RGB8BitColor lightBlue;
	RGB8BitColor lightGreen;
	RGB8BitColor lightCyan;
	RGB8BitColor lightRed;
	RGB8BitColor lightMagenta;
	RGB8BitColor yellow;
	RGB8BitColor white;
} defColorTheme;

static int defaultThemes = 6;

// Default color themes. Used for creating new files.
static defColorTheme defThemes[] = {
						// Black		Blue			Green		Cyan			Red			Magenta			Brown			L-Gray
						// D-Gray		L-Blue			L-Green		L-Cyan			L-Red		L-Magenta		Yellow			White
	{TEXT("Default"),	{0,0,0},		{0,0,128},		{0,128,0},	{ 0,128,128},	{128,0,0},	{128,0,128},	{128,128,0},	{192,192,192},
						{128,128,128},	{0,0,255},		{0,255,0},	{ 0,255,255},	{255,0,0},	{255,0,255},	{255,255,0},	{255,255,255}	},

	{TEXT("CGA"),		{0,0,0},		{0,0,170},		{0,170,0},	{ 0,170,170},	{170,0,0},	{170,0,170},	{170, 85,0},	{170,170,170},
						{ 85, 85, 85},	{85,85,255},	{85,255,85},{85,255,255},	{255,85,85},{255,85,255},	{255,255,85},	{255,255,255}	},

	{TEXT("IBM 5153"),	{0,0,0},		{0,0,170},		{0,170,0},	{ 0,170,170},	{170,0,0},	{170,0,170},	{196,126,0},	{170,170,170},
						{ 85, 85, 85},	{85,85,255},	{85,255,85},{85,255,255},	{255,85,85},{255,85,255},	{255,255,85},	{255,255,255}	},

	{TEXT("Campbell"),	{ 12, 12, 12},	{ 0, 55,218},	{19,161,14},{58,150,221},	{197,15,31},{136,23,152},	{193,156,0},	{204,204,204},
						{118,118,118},	{59,120,255},	{22,198,12},{97,214,214},	{231,72,86},{180, 0,158},	{249,241,165},	{242,242,242}	},

	{TEXT("PuTTY"),		{  0,  0,  0},	{ 0,  0,187},	{ 0,187, 0},{ 0,187,187},	{187, 0, 0},{187, 0,187},	{187,187,  0},	{187,187,187},
						{ 85, 85, 85},	{85, 85,255},	{85,255,85},{85,255,255},	{255,85,85},{180, 0,158},	{255,255, 85},	{255,255,255}	},

	{TEXT("XTerm"),		{  0,  0,  0},	{ 0,  0,238},	{ 0,205, 0},{ 0,205,205},	{205, 0, 0},{205, 0,205},	{205,205,  0},	{229,229,229},
						{127,127,127},	{92, 92,255},	{ 0,255, 0},{ 0,255,255},	{255, 0, 0},{255, 0,255},	{255,255,  0},	{255,255,255}	},
// TODO: More themes: Campbell Powershell, Solarized dark/light, tango dark/light, ottoson, one half dark/light, dark+
// TODO: support for storing FG/BG/Cursor/Selection colors in the theme

};

ConfigFile::ConfigFile() : _defaultThemeId(-1) {
}

void ConfigFile::createDefaultThemes() {
	ColorTheme *t;

	for (int i = 0; i < defaultThemes; i++) {
		t = createColorTheme();
		t->setName(CMString(defThemes[i].name));
		t->setRgbBlack(defThemes[i].black);
		t->setRgbBlue(defThemes[i].blue);
		t->setRgbGreen(defThemes[i].green);
		t->setRgbCyan(defThemes[i].cyan);
		t->setRgbRed(defThemes[i].red);
		t->setRgbMagenta(defThemes[i].magenta);
		t->setRgbBrown(defThemes[i].brown);
		t->setRgbLightGray(defThemes[i].lightGray);
		t->setRgbDarkGray(defThemes[i].darkGray);
		t->setRgbLightBlue(defThemes[i].lightBlue);
		t->setRgbLightGreen(defThemes[i].lightGreen);
		t->setRgbLightCyan(defThemes[i].lightCyan);
		t->setRgbLightRed(defThemes[i].lightRed);
		t->setRgbLightMagenta(defThemes[i].lightMagenta);
		t->setRgbYellow(defThemes[i].yellow);
		t->setRgbWhite(defThemes[i].white);

		// The above will have updated the internal JSON data, so we don't need
		// the accessor object anymore. Free it.
		delete t;
	}
}

int ConfigFile::defaultThemeId() {
	if (_defaultThemeId >= 0) return _defaultThemeId;

	CMString def(TEXT("Default"));

	ColorTheme *theme = firstColorTheme();
	while (theme != NULL) {
		
		if (theme->name() == def) {
			_defaultThemeId = theme->id();
			delete theme;
			return _defaultThemeId;
		}

		ColorTheme *next = theme->nextTheme();
		delete theme;
		theme = next;
	}

	return 0;
}