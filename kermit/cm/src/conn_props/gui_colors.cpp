#include <windows.h>
#include <tchar.h>

#include "../config_file.h"
#include "../conn_props.h"
#include "../conn_profile.h"
#include "../resource.h"

/********************
 * SHEET: GUI Colors
 ********************/

typedef struct tagColorFieldSet {
	enum Term::Color color;
	int r_field;
	int g_field;
	int b_field;
} ColorFieldSet;

static char fieldSetCount = 16;

static const ColorFieldSet fieldSets[] = {
	{	Term::COLOR_BLACK,			IDC_BLACK_R,	IDC_BLACK_G,	IDC_BLACK_B		},
	{	Term::COLOR_BLUE,			IDC_BLUE_R,		IDC_BLUE_G,		IDC_BLUE_B		},
	{	Term::COLOR_GREEN,			IDC_GREEN_R,	IDC_GREEN_G,	IDC_GREEN_B		},
	{	Term::COLOR_CYAN,			IDC_CYAN_R,		IDC_CYAN_G,		IDC_CYAN_B		},
	{	Term::COLOR_RED,			IDC_RED_R,		IDC_RED_G,		IDC_RED_B		},
	{	Term::COLOR_MAGENTA,		IDC_MAGENTA_R,	IDC_MAGENTA_G,	IDC_MAGENTA_B	},
	{	Term::COLOR_BROWN,			IDC_BROWN_R,	IDC_BROWN_G,	IDC_BROWN_B		},
	{	Term::COLOR_LIGHT_GRAY,		IDC_L_GRAY_R,	IDC_L_GRAY_G,	IDC_L_GRAY_B	},
	{	Term::COLOR_DARK_GRAY,		IDC_D_GRAY_R,	IDC_D_GRAY_G,	IDC_D_GRAY_B	},
	{	Term::COLOR_LIGHT_BLUE,		IDC_L_BLUE_R,	IDC_L_BLUE_G,	IDC_L_BLUE_B	},
	{	Term::COLOR_LIGHT_GREEN,	IDC_L_GREEN_R,	IDC_L_GREEN_G,	IDC_L_GREEN_B	},
	{	Term::COLOR_LIGHT_CYAN,		IDC_L_CYAN_R,	IDC_L_CYAN_G,	IDC_L_CYAN_B	},
	{	Term::COLOR_LIGHT_RED,		IDC_L_RED_R,	IDC_L_RED_G,	IDC_L_RED_B		},
	{	Term::COLOR_LIGHT_MAGENTA,	IDC_L_MAGENTA_R,IDC_L_MAGENTA_G,IDC_L_MAGENTA_B	},
	{	Term::COLOR_YELLOW,			IDC_YELLOW_R,	IDC_YELLOW_G,	IDC_YELLOW_B	},
	{	Term::COLOR_WHITE,			IDC_WHITE_R,	IDC_WHITE_G,	IDC_WHITE_B		},
	{	Term::COLOR_INVALID,		0,				0,				0				}
};


typedef struct tagSashedColorField {
	enum Term::Color color;
	RGB8BitColor colorValue;
} StashedColorField;

static StashedColorField stashedColors[] = {
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
	{ Term::COLOR_INVALID, {0, 0, 0} },
};


RGB8BitColor Get8bitColorValue(HWND hwndDlg, enum Term::Color color) {
	for (int i = 0; fieldSets[i].color != Term::COLOR_INVALID; i++) {
		if (fieldSets[i].color == color) {
			int r = getFieldInt(hwndDlg, fieldSets[i].r_field);
			int g = getFieldInt(hwndDlg, fieldSets[i].g_field);
			int b = getFieldInt(hwndDlg, fieldSets[i].b_field);

			/* Validation */
			if (r < 0) {
				r = 0;
				setFieldInt(hwndDlg, fieldSets[i].r_field, r);
			}
			
			if (r > 255) {
				r = 255;
				setFieldInt(hwndDlg, fieldSets[i].r_field, r);
			}

			if (g < 0) {
				g = 0;
				setFieldInt(hwndDlg, fieldSets[i].g_field, g);
			}

			if (g > 255) {
				g = 255;
				setFieldInt(hwndDlg, fieldSets[i].g_field, g);
			}
			
			if (b < 0) {
				b = 0;
				setFieldInt(hwndDlg, fieldSets[i].b_field, b);
			}

			if (b > 255) {
				b = 255;
				setFieldInt(hwndDlg, fieldSets[i].b_field, b);
			}
			
			RGB8BitColor result;
			result.r = (unsigned short)r;
			result.g = (unsigned short)g;
			result.b = (unsigned short)b;
			return result;
		}
	}

	RGB8BitColor result = {0, 0, 0};
	return result;
}

void Set8bitColorValue(HWND hwndDlg, enum Term::Color color, RGB8BitColor value) {
	for (int i = 0; fieldSets[i].color != Term::COLOR_INVALID; i++) {
		if (fieldSets[i].color == color) {
			setFieldInt(hwndDlg, fieldSets[i].r_field, value.r);
			setFieldInt(hwndDlg, fieldSets[i].g_field, value.g);
			setFieldInt(hwndDlg, fieldSets[i].b_field, value.b);
			return;
		}
	}
}


typedef struct tagDialogFieldStatus {
	int id;
	BOOL dirty;
} DialogFieldStatus;

// Connection profile we're editing
static ConnectionProfile *profile;

static const int fieldCount = 49;

static DialogFieldStatus fieldStatus[] = {
	{ IDC_THEME,		FALSE	},
	{ IDC_BLACK_R,		FALSE	},
	{ IDC_BLACK_G,		FALSE	},
	{ IDC_BLACK_B,		FALSE	},
	{ IDC_BLUE_R,		FALSE	},
	{ IDC_BLUE_G,		FALSE	},
	{ IDC_BLUE_B,		FALSE	},
	{ IDC_GREEN_R,		FALSE	},
	{ IDC_GREEN_G,		FALSE	},
	{ IDC_GREEN_B,		FALSE	},
	{ IDC_CYAN_R,		FALSE	},
	{ IDC_CYAN_G,		FALSE	},
	{ IDC_CYAN_B,		FALSE	},
	{ IDC_RED_R,		FALSE	},
	{ IDC_RED_G,		FALSE	},
	{ IDC_RED_B,		FALSE	},
	{ IDC_MAGENTA_R,	FALSE	},
	{ IDC_MAGENTA_G,	FALSE	},
	{ IDC_MAGENTA_B,	FALSE	},
	{ IDC_BROWN_R,		FALSE	},
	{ IDC_BROWN_G,		FALSE	},
	{ IDC_BROWN_B,		FALSE	},
	{ IDC_L_GRAY_R,		FALSE	},
	{ IDC_L_GRAY_G,		FALSE	},
	{ IDC_L_GRAY_B,		FALSE	},
	{ IDC_D_GRAY_R,		FALSE	},
	{ IDC_D_GRAY_G,		FALSE	},
	{ IDC_D_GRAY_B,		FALSE	},
	{ IDC_L_BLUE_R,		FALSE	},
	{ IDC_L_BLUE_G,		FALSE	},
	{ IDC_L_BLUE_B,		FALSE	},
	{ IDC_L_GREEN_R,	FALSE	},
	{ IDC_L_GREEN_G,	FALSE	},
	{ IDC_L_GREEN_B,	FALSE	},
	{ IDC_L_CYAN_R,		FALSE	},
	{ IDC_L_CYAN_G,		FALSE	},
	{ IDC_L_CYAN_B,		FALSE	},
	{ IDC_L_RED_R,		FALSE	},
	{ IDC_L_RED_G,		FALSE	},
	{ IDC_L_RED_B,		FALSE	},
	{ IDC_L_MAGENTA_R,	FALSE	},
	{ IDC_L_MAGENTA_G,	FALSE	},
	{ IDC_L_MAGENTA_B,	FALSE	},
	{ IDC_YELLOW_R,		FALSE	},
	{ IDC_YELLOW_G,		FALSE	},
	{ IDC_YELLOW_B,		FALSE	},
	{ IDC_WHITE_R,		FALSE	},
	{ IDC_WHITE_G,		FALSE	},
	{ IDC_WHITE_B,		FALSE	}
};


static BOOL isDirty(int id) {
	for (int i = 0; i < fieldCount; i++ ) {
		if (fieldStatus[i].id == id) {
			return fieldStatus[i].dirty;
		}
	}

	return FALSE; // Should never happen.
}

static void setDirty(int id, BOOL dirty) {
	for (int i = 0; i < fieldCount; i++ ) {
		if (fieldStatus[i].id == id) {
			fieldStatus[i].dirty = dirty;
			return;
		}
	}
}

static BOOL isDirty() {
	for (int i = 0; i < fieldCount; i++ ) {
		if (fieldStatus[i].dirty) {
			return TRUE;
		}
	}
	return FALSE;
}

static void cleanForm() {
	for (int i = 0; i < fieldCount; i++ ) {
		fieldStatus[i].dirty = FALSE;
	}
}

// Call whenever a fields value is changed to toggle the
// apply button on and off as necessary.
static void TerminalFieldChanged(HWND hwndDlg) {
	if (isDirty()) {
		PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
	} else {
		PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);
	}
}


static void PopulateThemesDDL(HWND hwndDlg, int controlId) {
	
	extern ConfigFile  *g_ConfigFile;

	HWND hwnd = GetDlgItem(hwndDlg, controlId);

	int currentThemeId = profile->colorThemeId();
	CMString selected;

	// Add the "Custom" option
	int idx = SendMessage(
		hwnd,
		(UINT) CB_ADDSTRING,
		(WPARAM) 0,
		(LPARAM) TEXT("Custom"));

	SendMessage(
		hwnd, 
		(UINT)CB_SETITEMDATA,
		(WPARAM)idx,
		(LPARAM) 0);

	if (currentThemeId == 0) {
		selected = CMString(TEXT("Custom"));
	}


	// Add all of the themes
	ColorTheme *theme = g_ConfigFile->firstColorTheme();
	while (theme != NULL) {
		
		idx = SendMessage(
			hwnd,
			(UINT) CB_ADDSTRING,
			(WPARAM) 0,
			(LPARAM) theme->name().data());

		SendMessage(
			hwnd, 
			(UINT)CB_SETITEMDATA,
			(WPARAM)idx,
			(LPARAM) theme->id());

		if (currentThemeId == theme->id()) {
			selected = theme->name();
		}

		ColorTheme *next = theme->nextTheme();
		delete theme;
		theme = next;
	}

	SendMessage(
		hwnd,
		CB_SELECTSTRING ,
		(WPARAM)0,
		(LPARAM)selected.data());
}



static RGB8BitColor GetProfileColor(enum Term::Color color) {
	switch(color) {
	case Term::COLOR_BLACK:
		return profile->rgbBlack();
	case Term::COLOR_BLUE:
		return profile->rgbBlue();
	case Term::COLOR_GREEN:
		return profile->rgbGreen();
	case Term::COLOR_CYAN:
		return profile->rgbCyan();
	case Term::COLOR_RED:
		return profile->rgbRed();
	case Term::COLOR_MAGENTA:
		return profile->rgbMagenta();
	case Term::COLOR_BROWN:
		return profile->rgbBrown();
	case Term::COLOR_LIGHT_GRAY:
		return profile->rgbLightGray();
	case Term::COLOR_DARK_GRAY:
		return profile->rgbDarkGray();
	case Term::COLOR_LIGHT_BLUE:
		return profile->rgbLightBlue();
	case Term::COLOR_LIGHT_GREEN:
		return profile->rgbLightGreen();
	case Term::COLOR_LIGHT_CYAN:
		return profile->rgbLightCyan();
	case Term::COLOR_LIGHT_RED:
		return profile->rgbLightRed();
	case Term::COLOR_LIGHT_MAGENTA:
		return profile->rgbLightMagenta();
	case Term::COLOR_YELLOW:
		return profile->rgbYellow();
	case Term::COLOR_WHITE:
		return profile->rgbWhite();
	}


	RGB8BitColor c = {0, 0, 0};
	return c;
}

static RGB8BitColor GetThemeColor(ColorTheme *theme, enum Term::Color color) {
	switch(color) {
	case Term::COLOR_BLACK:
		return theme->rgbBlack();
	case Term::COLOR_BLUE:
		return theme->rgbBlue();
	case Term::COLOR_GREEN:
		return theme->rgbGreen();
	case Term::COLOR_CYAN:
		return theme->rgbCyan();
	case Term::COLOR_RED:
		return theme->rgbRed();
	case Term::COLOR_MAGENTA:
		return theme->rgbMagenta();
	case Term::COLOR_BROWN:
		return theme->rgbBrown();
	case Term::COLOR_LIGHT_GRAY:
		return theme->rgbLightGray();
	case Term::COLOR_DARK_GRAY:
		return theme->rgbDarkGray();
	case Term::COLOR_LIGHT_BLUE:
		return theme->rgbLightBlue();
	case Term::COLOR_LIGHT_GREEN:
		return theme->rgbLightGreen();
	case Term::COLOR_LIGHT_CYAN:
		return theme->rgbLightCyan();
	case Term::COLOR_LIGHT_RED:
		return theme->rgbLightRed();
	case Term::COLOR_LIGHT_MAGENTA:
		return theme->rgbLightMagenta();
	case Term::COLOR_YELLOW:
		return theme->rgbYellow();
	case Term::COLOR_WHITE:
		return theme->rgbWhite();
	}


	RGB8BitColor c = {0, 0, 0};
	return c;
}

static BOOL CheckColorChanged(HWND hwndDlg, enum Term::Color color, 
							  RGB8BitColor *outNewValue) {

	RGB8BitColor existingValue = GetProfileColor(color);
	RGB8BitColor newValue = Get8bitColorValue(hwndDlg, color);

	// Find the fields that make up this color
	int rField, gField, bField;
	for (int i = 0; fieldSets[i].color != Term::COLOR_INVALID; i++) {
		if (fieldSets[i].color == color) {
			rField = fieldSets[i].r_field;
			gField = fieldSets[i].g_field;
			bField = fieldSets[i].b_field;
			break;
		}
	}

	BOOL rChanged = newValue.r != existingValue.r;
	BOOL gChanged = newValue.g != existingValue.g;
	BOOL bChanged = newValue.b != existingValue.b;
	BOOL changed = rChanged || gChanged || bChanged;

	setDirty(rField, rChanged);
	setDirty(gField, gChanged);
	setDirty(bField, bChanged);
	TerminalFieldChanged(hwndDlg);

	if (outNewValue != NULL) {
		*outNewValue = newValue;
	}

	return changed;
}


void LoadColorFields(HWND hwndDlg) {
	// Set current values...

	extern ConfigFile  *g_ConfigFile;

	HWND hwndTheme = GetDlgItem(hwndDlg, IDC_THEME);
	int idx = SendMessage(hwndTheme, CB_GETCURSEL, 0, 0);
	int themeId = SendMessage(hwndTheme, CB_GETITEMDATA, idx, 0);

	if (themeId == 0) {
		for (int i = 0; i < fieldSetCount; i++) {
			Set8bitColorValue(hwndDlg, fieldSets[i].color, GetProfileColor(fieldSets[i].color));
		}
	} else {
		ColorTheme *theme = g_ConfigFile->getColorThemeById(themeId);
		for (int i = 0; i < fieldSetCount; i++) {
			Set8bitColorValue(hwndDlg, fieldSets[i].color, GetThemeColor(theme, fieldSets[i].color));
		}
		delete theme;
	}
}


void SetColorFieldsEnabled(HWND hwndDlg, BOOL enabled) {
	for (int i = 0; i < fieldSetCount; i++) {
		EnableWindow(GetDlgItem(hwndDlg, fieldSets[i].r_field), enabled);
		EnableWindow(GetDlgItem(hwndDlg, fieldSets[i].g_field), enabled);
		EnableWindow(GetDlgItem(hwndDlg, fieldSets[i].b_field), enabled);
	}
}


void StashColorFields(HWND hwndDlg) {
	if (stashedColors[0].color != Term::COLOR_INVALID) {
		return; // Already have values stashed.
	}

	int i = 0;

	for (int j = 0; j < fieldSetCount; j++) {
		RGB8BitColor newValue;
		if (CheckColorChanged(hwndDlg, fieldSets[j].color, &newValue)) { 
			stashedColors[i].color = fieldSets[j].color;
			stashedColors[i].colorValue = newValue;
			i++;
		}
	}
}

void RestoreStashedColorFields(HWND hwndDlg) {

	// Firstly, load values from the profile
	LoadColorFields(hwndDlg);

	// Then load any stashed changes we may have
	for (int i = 0; i < fieldSetCount; i++) {
		if (stashedColors[i].color != Term::COLOR_INVALID) {
			Set8bitColorValue(hwndDlg, stashedColors[i].color, stashedColors[i].colorValue);
			stashedColors[i].color = Term::COLOR_INVALID;
		}

		// Set or reset the dirty flag as required
		CheckColorChanged(hwndDlg, fieldSets[i].color, NULL);
	}
}


BOOL CALLBACK GuiColorPageDlgProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	static int currentThemeId;

	switch(uMsg) {
	case WM_INITDIALOG: 
		{
			// Populate the theme drop-down list...
			PopulateThemesDDL(hwndDlg, IDC_THEME);
			currentThemeId = profile->colorThemeId();

			// Set current values from the profile
			LoadColorFields(hwndDlg);

			// Disable color fields if we're not using a custom theme
			SetColorFieldsEnabled(hwndDlg, currentThemeId == 0);

			// Form is clean
			cleanForm();

			break;
		}
	case WM_COMMAND: {
			WORD wNotifyCode = HIWORD(wParam);
			WORD wID = LOWORD(wParam);

			switch(wID) {
			case IDC_THEME:
				{
					HWND hwndTheme = GetDlgItem(hwndDlg, IDC_THEME);
					int idx = SendMessage(hwndTheme, CB_GETCURSEL, 0, 0);
					int newThemeId = SendMessage(hwndTheme, CB_GETITEMDATA, idx, 0);

					if (newThemeId != currentThemeId && newThemeId != CB_ERR) {
						int profileThemeId = profile->colorThemeId();
						currentThemeId = newThemeId;

						if (newThemeId == 0) { 
							// Custom option. Restore values from profile:
							LoadColorFields(hwndDlg);

							// Enable color fields
							SetColorFieldsEnabled(hwndDlg, TRUE);

							// And then any values we saved before the use switched to a theme:
							RestoreStashedColorFields(hwndDlg);
						} else {
							// Color theme. Save any dirty field values...
							StashColorFields(hwndDlg);

							// Disable the color fields
							SetColorFieldsEnabled(hwndDlg, FALSE);

							// And then reload everything from the theme
							LoadColorFields(hwndDlg);

							// Reset dirty flag on everything
							cleanForm();
						}

						setDirty(IDC_THEME, newThemeId != profileThemeId);
						TerminalFieldChanged(hwndDlg);
					}
				} 
				break;
			case IDC_BLACK_R: 
			case IDC_BLACK_G:
			case IDC_BLACK_B:
				CheckColorChanged(hwndDlg, Term::COLOR_BLACK, NULL);
				break;
			case IDC_BLUE_R:
			case IDC_BLUE_G:
			case IDC_BLUE_B:
				CheckColorChanged(hwndDlg, Term::COLOR_BLUE, NULL);
				break;
			case IDC_GREEN_R:
			case IDC_GREEN_G:
			case IDC_GREEN_B:
				CheckColorChanged(hwndDlg, Term::COLOR_GREEN, NULL);
				break;
			case IDC_CYAN_R:
			case IDC_CYAN_G:
			case IDC_CYAN_B:
				CheckColorChanged(hwndDlg, Term::COLOR_CYAN, NULL);
				break;
			case IDC_RED_R:
			case IDC_RED_G:
			case IDC_RED_B:
				CheckColorChanged(hwndDlg, Term::COLOR_RED, NULL);
				break;
			case IDC_MAGENTA_R:
			case IDC_MAGENTA_G:
			case IDC_MAGENTA_B:
				CheckColorChanged(hwndDlg, Term::COLOR_MAGENTA, NULL);
				break;
			case IDC_BROWN_R:
			case IDC_BROWN_G:
			case IDC_BROWN_B:
				CheckColorChanged(hwndDlg, Term::COLOR_BROWN, NULL);
				break;
			case IDC_L_GRAY_R:
			case IDC_L_GRAY_G:
			case IDC_L_GRAY_B:
				CheckColorChanged(hwndDlg, Term::COLOR_LIGHT_GRAY, NULL);
				break;
			case IDC_D_GRAY_R:
			case IDC_D_GRAY_G:
			case IDC_D_GRAY_B:
				CheckColorChanged(hwndDlg, Term::COLOR_DARK_GRAY, NULL);
				break;
			case IDC_L_BLUE_R:
			case IDC_L_BLUE_G:
			case IDC_L_BLUE_B:
				CheckColorChanged(hwndDlg, Term::COLOR_LIGHT_BLUE, NULL);
				break;
			case IDC_L_GREEN_R:
			case IDC_L_GREEN_G:
			case IDC_L_GREEN_B:
				CheckColorChanged(hwndDlg, Term::COLOR_LIGHT_GREEN, NULL);
				break;
			case IDC_L_CYAN_R:
			case IDC_L_CYAN_G:
			case IDC_L_CYAN_B:
				CheckColorChanged(hwndDlg, Term::COLOR_LIGHT_CYAN, NULL);
				break;
			case IDC_L_RED_R:
			case IDC_L_RED_G:
			case IDC_L_RED_B:
				CheckColorChanged(hwndDlg, Term::COLOR_LIGHT_RED, NULL);
				break;
			case IDC_L_MAGENTA_R:
			case IDC_L_MAGENTA_G:
			case IDC_L_MAGENTA_B:
				CheckColorChanged(hwndDlg, Term::COLOR_LIGHT_MAGENTA, NULL);
				break;
			case IDC_YELLOW_R:
			case IDC_YELLOW_G:
			case IDC_YELLOW_B:
				CheckColorChanged(hwndDlg, Term::COLOR_YELLOW, NULL);
				break;
			case IDC_WHITE_R:
			case IDC_WHITE_G:
			case IDC_WHITE_B:
				CheckColorChanged(hwndDlg, Term::COLOR_WHITE, NULL);
				break;
			}
			return TRUE;
			break;
		}
	case WM_NOTIFY: {
			LPNMHDR pnmh = (LPNMHDR)lParam;
			switch (pnmh->code) {
			case PSN_APPLY:	{		// Apply or OK button clicked
				if (isDirty()) {

					// Save the color theme ID if its changed.
					HWND hwndTheme = GetDlgItem(hwndDlg, IDC_THEME);
					int idx = SendMessage(hwndTheme, CB_GETCURSEL, 0, 0);
					int newThemeId = SendMessage(hwndTheme, CB_GETITEMDATA, idx, 0);
					int currentThemeId = profile->colorThemeId();

					if (newThemeId != currentThemeId) {
						profile->setColorThemeId(newThemeId);
					}

					// Only save individual color fields if we're using a custom theme.
					if (newThemeId == 0) {

						RGB8BitColor newValue;
						BOOL changed;

						changed = CheckColorChanged(hwndDlg, Term::COLOR_BLACK, &newValue);
						if (changed) profile->setRgbBlack(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_BLUE, &newValue);
						if (changed) profile->setRgbBlue(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_GREEN, &newValue);
						if (changed) profile->setRgbGreen(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_CYAN, &newValue);
						if (changed) profile->setRgbCyan(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_RED, &newValue);
						if (changed) profile->setRgbRed(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_MAGENTA, &newValue);
						if (changed) profile->setRgbMagenta(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_BROWN, &newValue);
						if (changed) profile->setRgbBrown(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_LIGHT_GRAY, &newValue);
						if (changed) profile->setRgbLightGray(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_DARK_GRAY, &newValue);
						if (changed) profile->setRgbDarkGray(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_LIGHT_BLUE, &newValue);
						if (changed) profile->setRgbLightBlue(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_LIGHT_GREEN, &newValue);
						if (changed) profile->setRgbLightGreen(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_LIGHT_CYAN, &newValue);
						if (changed) profile->setRgbLightCyan(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_LIGHT_RED, &newValue);
						if (changed) profile->setRgbLightRed(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_LIGHT_MAGENTA, &newValue);
						if (changed) profile->setRgbLightMagenta(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_YELLOW, &newValue);
						if (changed) profile->setRgbYellow(newValue);

						changed = CheckColorChanged(hwndDlg, Term::COLOR_WHITE, &newValue);
						if (changed) profile->setRgbWhite(newValue);

					}

					cleanForm();
				}
				return TRUE;
			}
			break;
				
			case PSN_QUERYCANCEL:	// User clicked cancel - OK??
			case PSN_RESET:			// User clicked cancel
			case PSN_HELP:			// User clicked the Help button
			case PSN_SETACTIVE:		// page activated (user switched to this page) 
			case PSN_KILLACTIVE:	// page de-activated (user switched away)
			case PSN_WIZNEXT:		// We're a wizard and the user clicked next
			case PSN_WIZBACK:		// We're a wizard and the user clicked back
			case PSN_WIZFINISH:		// We're a wizard and the user clicked finish
			default:
				break;
			}

			break;
		}
	}
	return 0; // Message not processed
}

UINT CALLBACK GuiColorPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp) {
	switch (uMsg) {
	case PSPCB_CREATE:
		profile = (ConnectionProfile*)ppsp->lParam;
		return TRUE;
	case PSPCB_RELEASE:
		return 0;
	}
	return 0;
}
