#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <wingdi.h>

#include "../conn_props.h"
#include "../conn_profile.h"
#include "../resource.h"

/******************
 * SHEET: GUI
 ******************/

/*
Controls:
IDC_USE_GUI					Checkbox
IDC_FONT					Drop-down list
IDC_FONT_SIZE				Dop-down list
IDC_STARTUP_DEFAULT			Radio button
IDC_STARTUP_CUSTOM			Radio button
IDC_STARTUP_X				Number edit
IDC_STARTUP_X_SPIN			Spin button
IDC_STARTUP_Y				Number edit
IDC_STARTUP_Y_SPIN			Spin button
IDC_RESIZE_MODE				Drop-down list
IDC_INITIAL_STATE			Drop-down list
IDC_ENABLE_MENUBAR			Checkbox
IDC_ENABLE_TOOLBAR			Checkbox
IDC_ENABLE_STATUSBAR		Checkbox
IDC_REMOVE_BARS				Checkbox  - only enabled if a bar is disabled
IDC_ENABLE_DIALOGS			Checkbox
 */

#define CB_RESIZE_CHANGE_DIMENSIONS (TEXT("Change Dimensions"))
#define CB_RESIZE_SCALE_FONT		(TEXT("Scale Font"))
#define CB_INITIAL_STATE_NORMAL		(TEXT("Normal window"))
#define CB_INITIAL_STATE_MAXIMIZED  (TEXT("Maximized"))
#define CB_INITIAL_STATE_MINIMIZED  (TEXT("Minimized"))

// Connection profile we're editing
static ConnectionProfile *profile;


typedef struct tagDialogFieldStatus {
	int id;
	BOOL dirty;
} DialogFieldStatus;


static const int fieldCount = 13;

static DialogFieldStatus fieldStatus[] = {
	{ IDC_USE_GUI,			FALSE	},
	{ IDC_FONT,				FALSE	},
	{ IDC_FONT_SIZE,		FALSE	},
	{ IDC_STARTUP_CUSTOM,	FALSE	},
	{ IDC_STARTUP_X,		FALSE	},
	{ IDC_STARTUP_Y,		FALSE	},
	{ IDC_RESIZE_MODE,		FALSE	},
	{ IDC_INITIAL_STATE,	FALSE	},
	{ IDC_ENABLE_MENUBAR,	FALSE	},
	{ IDC_ENABLE_TOOLBAR,	FALSE	},
	{ IDC_ENABLE_STATUSBAR,	FALSE	},
	{ IDC_REMOVE_BARS,		FALSE	},
	{ IDC_ENABLE_DIALOGS,	FALSE	}
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

static void setFieldText(HWND hwndDlg, int id, CMString value) {
	SetWindowText(GetDlgItem(hwndDlg, id), value.data());
}


static void CheckUseGUI(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_USE_GUI) == BST_CHECKED;
	BOOL oldValue = profile->useGUIKermit();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_USE_GUI, changed);
		TerminalFieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setUseGUIKermit(newValue);
	}
}


static void SetUseGUIChecked(HWND hwndDlg, BOOL enabled) {
	CheckDlgButton(hwndDlg, IDC_USE_GUI, 
		enabled ? BST_CHECKED : BST_UNCHECKED);	
}


static void CheckEnableMenubar(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_ENABLE_MENUBAR) == BST_CHECKED;
	BOOL oldValue = profile->menubarEnabled();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_ENABLE_MENUBAR, changed);
		TerminalFieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setMenubarEnabled(newValue);
	}
}


static void SetCustomStartupPositionFieldsEnabled(HWND hwndDlg) {
	BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_STARTUP_CUSTOM) == BST_CHECKED;

	EnableWindow(GetDlgItem(hwndDlg,IDC_STARTUP_X_SPIN), enabled);
	EnableWindow(GetDlgItem(hwndDlg,IDC_STARTUP_Y_SPIN), enabled);
	SendMessage(GetDlgItem(hwndDlg, IDC_STARTUP_X),
			EM_SETREADONLY,
			(WPARAM)!enabled,
			(LPARAM)0);
	SendMessage(GetDlgItem(hwndDlg, IDC_STARTUP_Y),
			EM_SETREADONLY,
			(WPARAM)!enabled,
			(LPARAM)0);
}


static void SetRemoveBarsEnabled(HWND hwndDlg) {
	BOOL menubar = IsDlgButtonChecked(hwndDlg, IDC_ENABLE_MENUBAR) == BST_CHECKED;
	BOOL toolbar = IsDlgButtonChecked(hwndDlg, IDC_ENABLE_TOOLBAR) == BST_CHECKED;
	BOOL statusbar = IsDlgButtonChecked(hwndDlg, IDC_ENABLE_STATUSBAR) == BST_CHECKED;

	BOOL enabled = !menubar || !toolbar || !statusbar;

	EnableWindow(GetDlgItem(hwndDlg,IDC_REMOVE_BARS), enabled);
}


static void SetGUIEnabled(HWND hwndDlg, BOOL enabled) {
	// Disable the checkboxes, radio buttons, and spinboxes
	EnableWindow(GetDlgItem(hwndDlg,IDC_FONT), enabled);
	EnableWindow(GetDlgItem(hwndDlg,IDC_FONT_SIZE), enabled);
	EnableWindow(GetDlgItem(hwndDlg,IDC_STARTUP_DEFAULT), enabled);
	EnableWindow(GetDlgItem(hwndDlg,IDC_STARTUP_CUSTOM), enabled);
	EnableWindow(GetDlgItem(hwndDlg,IDC_RESIZE_MODE), enabled);
	EnableWindow(GetDlgItem(hwndDlg,IDC_INITIAL_STATE), enabled);
	EnableWindow(GetDlgItem(hwndDlg,IDC_ENABLE_MENUBAR), enabled);
	EnableWindow(GetDlgItem(hwndDlg,IDC_ENABLE_TOOLBAR), enabled);
	EnableWindow(GetDlgItem(hwndDlg,IDC_ENABLE_STATUSBAR), enabled);
	EnableWindow(GetDlgItem(hwndDlg,IDC_ENABLE_DIALOGS), enabled);

	// Make the input fields read-only
	SendMessage(GetDlgItem(hwndDlg, IDC_FONT_SIZE),
			EM_SETREADONLY,
			(WPARAM)!enabled,
			(LPARAM)0);

	// And only enable the removebars checkbox and custom startup position
	// fields if they should normally be enabled
	if (enabled) {
		SetRemoveBarsEnabled(hwndDlg);
		SetCustomStartupPositionFieldsEnabled(hwndDlg);
	} else {
		EnableWindow(GetDlgItem(hwndDlg,IDC_REMOVE_BARS), enabled);
		EnableWindow(GetDlgItem(hwndDlg,IDC_STARTUP_X_SPIN), enabled);
		EnableWindow(GetDlgItem(hwndDlg,IDC_STARTUP_Y_SPIN), enabled);
		SendMessage(GetDlgItem(hwndDlg, IDC_STARTUP_X),
			EM_SETREADONLY,
			(WPARAM)!enabled,
			(LPARAM)0);
		SendMessage(GetDlgItem(hwndDlg, IDC_STARTUP_Y),
			EM_SETREADONLY,
			(WPARAM)!enabled,
			(LPARAM)0);
	}
}


static void SetEnableMenubarChecked(HWND hwndDlg, BOOL enabled) {
	CheckDlgButton(hwndDlg, IDC_ENABLE_MENUBAR, 
		enabled ? BST_CHECKED : BST_UNCHECKED);	
}


static void CheckEnableToolbar(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_ENABLE_TOOLBAR) == BST_CHECKED;
	BOOL oldValue = profile->toolbarEnabled();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_ENABLE_TOOLBAR, changed);
		TerminalFieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setToolbarEnabled(newValue);
	}
}


static void SetEnableToolbarChecked(HWND hwndDlg, BOOL enabled) {
	CheckDlgButton(hwndDlg, IDC_ENABLE_TOOLBAR, 
		enabled ? BST_CHECKED : BST_UNCHECKED);	
}


static void CheckEnableStatusbar(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_ENABLE_STATUSBAR) == BST_CHECKED;
	BOOL oldValue = profile->statusbarEnabled();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_ENABLE_STATUSBAR, changed);
		TerminalFieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setStatusbarEnabled(newValue);
	}
}


static void SetEnableStatusbarChecked(HWND hwndDlg, BOOL enabled) {
	CheckDlgButton(hwndDlg, IDC_ENABLE_STATUSBAR, 
		enabled ? BST_CHECKED : BST_UNCHECKED);	
}


static void CheckRemoveBars(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_REMOVE_BARS) == BST_CHECKED;
	BOOL oldValue = profile->removeBars();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_REMOVE_BARS, changed);
		TerminalFieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setRemoveBars(newValue);
	}
}


static void SetRemoveBarsChecked(HWND hwndDlg, BOOL enabled) {
	CheckDlgButton(hwndDlg, IDC_REMOVE_BARS, 
		enabled ? BST_CHECKED : BST_UNCHECKED);	
}


static void CheckEnableDialogs(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_ENABLE_DIALOGS) == BST_CHECKED;
	BOOL oldValue = profile->dialogsEnabled();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_ENABLE_DIALOGS, changed);
		TerminalFieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setDialogsEnabled(newValue);
	}
}


static void SetEnableDialogsChecked(HWND hwndDlg, BOOL enabled) {
	CheckDlgButton(hwndDlg, IDC_ENABLE_DIALOGS, 
		enabled ? BST_CHECKED : BST_UNCHECKED);	
}


static void CheckCustomStartupPosition(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_STARTUP_CUSTOM) == BST_CHECKED;
	BOOL oldValue = profile->customStartupPosition();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_STARTUP_CUSTOM, changed);
		TerminalFieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setCustomStartupPosition(newValue);
	}
}


static void SetCustomStartupPositionChecked(HWND hwndDlg, BOOL enabled) {
	CheckDlgButton(
		hwndDlg, 
		IDC_STARTUP_DEFAULT, 
		enabled ? BST_UNCHECKED : BST_CHECKED);
	CheckDlgButton(
		hwndDlg, 
		IDC_STARTUP_CUSTOM, 
		enabled ? BST_CHECKED : BST_UNCHECKED);
}

static void CheckXPosition(HWND hwndDlg, BOOL save) {
	// We have to get the numeric value off of the spin box
	// rather than backing field itself because it uses commas
	// for numbers >999 which getFieldInt() can't handle
	int newValue = SendMessage(
		GetDlgItem(hwndDlg, IDC_STARTUP_X_SPIN), UDM_GETPOS, 0, 0);
	int oldValue = profile->startupAtX();

	BOOL changed = newValue != oldValue;
	
	if (!save) {
		setDirty(IDC_STARTUP_X, changed);
		TerminalFieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setStartupAtX(newValue);
	}
}

static void SetXPosition(HWND hwndDlg, int position) {
	setFieldText(hwndDlg, IDC_STARTUP_X, 
		CMString::number(position));
}

static void CheckYPosition(HWND hwndDlg, BOOL save) {
	// We have to get the numeric value off of the spin box
	// rather than backing field itself because it uses commas
	// for numbers >999 which getFieldInt() can't handle
	int newValue = SendMessage(
		GetDlgItem(hwndDlg, IDC_STARTUP_Y_SPIN), UDM_GETPOS, 0, 0);
	int oldValue = profile->startupAtY();

	BOOL changed = newValue != oldValue;
	
	if (!save) {
		setDirty(IDC_STARTUP_Y, changed);
		TerminalFieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setStartupAtY(newValue);
	}
}

static void SetYPosition(HWND hwndDlg, int position) {
	setFieldText(hwndDlg, IDC_STARTUP_Y, 
		CMString::number(position));
}

static void CheckReiszeMode(HWND hwndDlg, BOOL save) {
	HWND hwnd = GetDlgItem(hwndDlg, IDC_RESIZE_MODE);

	int idx = ComboBox_GetCurSel(hwnd);

	int idxChange = ComboBox_FindString(hwnd, 0, CB_RESIZE_CHANGE_DIMENSIONS);

	BOOL existing = profile->resizeChangesDimensions();

	BOOL newValue = idx == idxChange;

	BOOL changed = newValue != existing;

	if (!save) {
		setDirty(IDC_RESIZE_MODE, changed);
		TerminalFieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setResizeChangesDimensions(newValue);
	}
}

static void CheckInitialState(HWND hwndDlg, BOOL save) {
	HWND hwnd = GetDlgItem(hwndDlg, IDC_INITIAL_STATE);

	int idx = ComboBox_GetCurSel(hwnd);

	//int idxNormal = ComboBox_FindString(hwnd, 0, CB_INITIAL_STATE_NORMAL);
	int idxMaximized = ComboBox_FindString(hwnd, 0, CB_INITIAL_STATE_MAXIMIZED);
	int idxMinimized = ComboBox_FindString(hwnd, 0, CB_INITIAL_STATE_MINIMIZED);

	ConnectionProfile::WindowStartupMode existing = profile->windowStartupMode();
	ConnectionProfile::WindowStartupMode newValue = ConnectionProfile::WSM_NORMAL;

	if (idx == idxMaximized) newValue = ConnectionProfile::WSM_MAXIMIZED;
	else if (idx == idxMinimized) newValue = ConnectionProfile::WSM_MINIMIZED;

	BOOL changed = newValue != existing;

	if (!save) {
		setDirty(IDC_INITIAL_STATE, changed);
		TerminalFieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setWindowStartupMode(newValue);
	}
}

static int CALLBACK EnumFontFamiliesProc(
		ENUMLOGFONT *lpelfe,      // logical-font data
		NEWTEXTMETRIC *lpntme,    // physical-font data
		DWORD FontType,           // type of font
		LPARAM lParam) {          // application-supplied data

	HWND hwndDlg = (HWND)lParam;

	// Apparently Windows 9x handles font names differently from the
	// Windows NT family.
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	BCHAR *bcharFontName = NULL;
	TCHAR *tcharFontName = NULL;

	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
		// Windows 9x
		if (FontType == TRUETYPE_FONTTYPE) {
			bcharFontName = lpelfe->elfFullName;
		} else {
			tcharFontName = lpelfe->elfLogFont.lfFaceName;
		}
	} else {
		// Windows NT
		bcharFontName = lpelfe->elfFullName;
	}

	// We may see the same font name multiple times, so before adding it to
	// the font combo box we need to check it isn't already in there.
	HWND hwndFontCombo = GetDlgItem(hwndDlg, IDC_FONT);
	if (bcharFontName) {
		if (ComboBox_FindString(hwndFontCombo, 0, bcharFontName) == CB_ERR) {
			ComboBox_AddString(hwndFontCombo, bcharFontName);
		}
	} else if (tcharFontName) {
		if (ComboBox_FindString(hwndFontCombo, 0, tcharFontName) == CB_ERR) {
			ComboBox_AddString(hwndFontCombo, tcharFontName);
		}
	}

	return 1; // Non-zero to continue enumeration
}

static int CALLBACK EnumFontFamiliesExProc( 
		ENUMLOGFONTEX *lpelfe,    // logical-font data
		NEWTEXTMETRICEX *lpntme,  // physical-font data
		DWORD FontType,           // type of font
		LPARAM lParam) {          // application-supplied data

	HWND hwndDlg = (HWND)lParam;

	// Apparently Windows 9x handles font names differently from the
	// Windows NT family.
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	char *bcharFontName = NULL;
	TCHAR *tcharFontName = NULL;

	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
		// Windows 9x
		if (FontType == TRUETYPE_FONTTYPE) {
			bcharFontName = (char*)lpelfe->elfFullName;
		} else {
			tcharFontName = lpelfe->elfLogFont.lfFaceName;
		}
	} else {
		// Windows NT
		bcharFontName = (char*)lpelfe->elfFullName;
	}

	// We may see the same font name multiple times, so before adding it to
	// the font combo box we need to check it isn't already in there.
	HWND hwndFontCombo = GetDlgItem(hwndDlg, IDC_FONT);
	if (bcharFontName) {
		if (ComboBox_FindString(hwndFontCombo, 0, bcharFontName) == CB_ERR) {
			ComboBox_AddString(hwndFontCombo, bcharFontName);
		}
	} else if (tcharFontName) {
		if (ComboBox_FindString(hwndFontCombo, 0, tcharFontName) == CB_ERR) {
			ComboBox_AddString(hwndFontCombo, tcharFontName);
		}
	}

	return 1; // Non-zero to continue enumeration
}

static void PopulateFontsDDL(HWND hwndDlg) {
	HDC hdc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	if (!hdc) return;

	// EnumFontFamiliesEx is new in Windows 95, so for NT 3.51
	// we've got to use EnumFontFamilies instead.

	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	if (osvi.dwMajorVersion < 4) {
		// Windows NT 3.x
		EnumFontFamilies(
			(HDC)hdc,			// Device context
			NULL,				// Font Family Name
			(FONTENUMPROC) EnumFontFamiliesProc,  // Callback
			(LPARAM)hwndDlg);	// Application-supplied data
	} else {
		// TODO: Try to filter to monospace fonts only
		LOGFONT lf;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfFaceName[0] = '\0';
		lf.lfPitchAndFamily = 0;

		EnumFontFamiliesEx(
			(HDC)hdc,		// Device context
			(LPLOGFONT)&lf,	// Font information
			(FONTENUMPROC) EnumFontFamiliesExProc,	// Callback
			(LPARAM)hwndDlg,// Application-supplied data
			0);				// Reserved
	}

	DeleteDC(hdc);
}

static void CheckFont(HWND hwndDlg, BOOL save) {
	HWND hwnd = GetDlgItem(hwndDlg, IDC_FONT);

	int idx = ComboBox_GetCurSel(hwnd);

	int idxSaved = ComboBox_FindString(hwnd, 0, profile->fontName().data());

	BOOL changed = idx != idxSaved;

	if (!save) {
		setDirty(IDC_FONT, changed);
		TerminalFieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		int length = ComboBox_GetTextLength(hwnd) + 1;

		LPTSTR text = (LPTSTR)malloc(length * sizeof(TCHAR));

		ComboBox_GetText(hwnd, text, length);

		profile->setFontName(CMString(text));

		free(text);
	}
}

static void PopulateFontSizeDDL(HWND hwndDlg) {

	HWND hwnd = GetDlgItem(hwndDlg, IDC_FONT_SIZE);

	// K95 lists sizes from 1pt to 48pt in 0.5pt increments, so we'll do the same.
    for( int i = 1; i < 48; i++ )
    {
        CMString num = CMString::number(i);

		ComboBox_AddString(hwnd, num.data());

		CMString num2 = num + CMString(TEXT(".5"));
        
        ComboBox_AddString(hwnd, num2.data());
    }
}

static void CheckFontSize(HWND hwndDlg, BOOL save) {
	HWND hwnd = GetDlgItem(hwndDlg, IDC_FONT_SIZE);

	int idx = ComboBox_GetCurSel(hwnd);

	int idxSaved = ComboBox_FindString(hwnd, 0, profile->fontSize().data());

	BOOL changed = idx != idxSaved;

	if (!save) {
		setDirty(IDC_FONT_SIZE, changed);
		TerminalFieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		int length = ComboBox_GetTextLength(hwnd) + 1;

		LPTSTR text = (LPTSTR)malloc(length * sizeof(TCHAR));

		ComboBox_GetText(hwnd, text, length);

		profile->setFontSize(CMString(text));

		free(text);
	}
}


BOOL CALLBACK GuiPageDlgProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {


	switch(uMsg) {
	case WM_INITDIALOG: 
		{
			// Populate drop-lists
			HWND hwndResize = GetDlgItem(hwndDlg, IDC_RESIZE_MODE);
			ComboBox_AddString(hwndResize, CB_RESIZE_CHANGE_DIMENSIONS);
			ComboBox_AddString(hwndResize, CB_RESIZE_SCALE_FONT);

			HWND hwndInitialState = GetDlgItem(hwndDlg, IDC_INITIAL_STATE);
			ComboBox_AddString(hwndInitialState, CB_INITIAL_STATE_NORMAL);
			ComboBox_AddString(hwndInitialState, CB_INITIAL_STATE_MAXIMIZED);
			ComboBox_AddString(hwndInitialState, CB_INITIAL_STATE_MINIMIZED);

			PopulateFontsDDL(hwndDlg);
			PopulateFontSizeDDL(hwndDlg);
			
			// Configure spinboxes
			HWND udm_xpos = GetDlgItem(hwndDlg, IDC_STARTUP_X_SPIN);
			SendMessage(udm_xpos,
				UDM_SETBUDDY,
				(WPARAM)GetDlgItem(hwndDlg,IDC_STARTUP_X),
				(LPARAM)0);
			SendMessage(udm_xpos,
				UDM_SETRANGE,
				(WPARAM)0,
				(LPARAM)32767);
			SendMessage(udm_xpos,
				UDM_SETPOS,
				(WPARAM)0,
				(LPARAM)profile->startupAtX());


			HWND udm_ypos = GetDlgItem(hwndDlg, IDC_STARTUP_Y_SPIN);
			SendMessage(udm_ypos,
				UDM_SETBUDDY,
				(WPARAM)GetDlgItem(hwndDlg,IDC_STARTUP_Y),
				(LPARAM)0);
			SendMessage(udm_ypos,
				UDM_SETRANGE,
				(WPARAM)0,
				(LPARAM)32767);
			SendMessage(udm_ypos,
				UDM_SETPOS,
				(WPARAM)0,
				(LPARAM)profile->startupAtY());


			// load initial values from profile
			SetUseGUIChecked(hwndDlg, profile->useGUIKermit());
			SetEnableMenubarChecked(hwndDlg, profile->menubarEnabled());
			SetEnableToolbarChecked(hwndDlg, profile->toolbarEnabled());
			SetEnableStatusbarChecked(hwndDlg, profile->statusbarEnabled());
			SetRemoveBarsChecked(hwndDlg, profile->removeBars());
			SetEnableDialogsChecked(hwndDlg, profile->dialogsEnabled());
			SetCustomStartupPositionChecked(hwndDlg, profile->customStartupPosition());
			if (profile->resizeChangesDimensions()) {
				ComboBox_SelectString(hwndResize, 0, CB_RESIZE_CHANGE_DIMENSIONS);
			} else {
				ComboBox_SelectString(hwndResize, 0, CB_RESIZE_SCALE_FONT);
			}
			switch(profile->windowStartupMode()) {
			case ConnectionProfile::WSM_NORMAL:
				ComboBox_SelectString(hwndInitialState, 0, CB_INITIAL_STATE_NORMAL);
				break;
			case ConnectionProfile::WSM_MAXIMIZED:
				ComboBox_SelectString(hwndInitialState, 0, CB_INITIAL_STATE_MAXIMIZED);
				break;
			case ConnectionProfile::WSM_MINIMIZED:
				ComboBox_SelectString(hwndInitialState, 0, CB_INITIAL_STATE_MINIMIZED);
				break;
			}

			// Try to select the current font. If it isn't in the list, add it then
			// select it. This is really for future migration from the dialer purposes.
			// Ideally here we filter the list to monospace fonts only, but we want to
			// handle if someone migrates a profile that doesn't use a monospace font.
			if (ComboBox_SelectString(GetDlgItem(hwndDlg, IDC_FONT), 
						0, profile->fontName().data()) == CB_ERR) {
				ComboBox_AddString(GetDlgItem(hwndDlg, IDC_FONT), 
					profile->fontName().data());
				ComboBox_SelectString(GetDlgItem(hwndDlg, IDC_FONT), 
					0, profile->fontName().data());
			}

			if (ComboBox_SelectString(GetDlgItem(hwndDlg, IDC_FONT_SIZE), 
						0, profile->fontSize().data()) == CB_ERR) {
				ComboBox_AddString(GetDlgItem(hwndDlg, IDC_FONT_SIZE), 
					profile->fontSize().data());
				ComboBox_SelectString(GetDlgItem(hwndDlg, IDC_FONT_SIZE), 
					0, profile->fontSize().data());
			}

			// Set fields enabled or disabled
			SetGUIEnabled(hwndDlg, profile->useGUIKermit());
			SetRemoveBarsEnabled(hwndDlg) ;

			// Form is clean
			cleanForm();

			break;
		}
	case WM_COMMAND: {
			WORD wNotifyCode = HIWORD(wParam);
			WORD wID = LOWORD(wParam);

			switch(wID) {
			
			case IDC_USE_GUI: 
				CheckUseGUI(hwndDlg, FALSE);
				SetGUIEnabled(
					hwndDlg, 
					IsDlgButtonChecked(hwndDlg, IDC_USE_GUI) == BST_CHECKED);
				break;

			case IDC_FONT:
				CheckFont(hwndDlg, FALSE);
				break;
			case IDC_FONT_SIZE:
				CheckFontSize(hwndDlg, FALSE);
				break;
			case IDC_STARTUP_DEFAULT:
			case IDC_STARTUP_CUSTOM:
				CheckCustomStartupPosition(hwndDlg, FALSE);
				SetCustomStartupPositionFieldsEnabled(hwndDlg);
				break;
			case IDC_STARTUP_X:
				CheckXPosition(hwndDlg, FALSE);
				break;
			case IDC_STARTUP_Y:
				CheckYPosition(hwndDlg, FALSE);
				break;
			case IDC_STARTUP_X_SPIN:
			case IDC_STARTUP_Y_SPIN:
				// Nothing needs to be done for these - they handle themselves.
				break;
			case IDC_RESIZE_MODE:
				CheckReiszeMode(hwndDlg, FALSE);
				break;
			case IDC_INITIAL_STATE:
				CheckInitialState(hwndDlg, FALSE);
				break;
			case IDC_ENABLE_MENUBAR:
				CheckEnableMenubar(hwndDlg, FALSE);
				SetRemoveBarsEnabled(hwndDlg);
				break;
			case IDC_ENABLE_TOOLBAR:
				CheckEnableToolbar(hwndDlg, FALSE);
				SetRemoveBarsEnabled(hwndDlg);
				break;
			case IDC_ENABLE_STATUSBAR:
				CheckEnableStatusbar(hwndDlg, FALSE);
				SetRemoveBarsEnabled(hwndDlg);
				break;
			case IDC_REMOVE_BARS:
				CheckRemoveBars(hwndDlg, FALSE);
				break;
			case IDC_ENABLE_DIALOGS:
				CheckEnableDialogs(hwndDlg, FALSE);
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
					// Save any values that have changed

					// Enable/Disable GUI
					CheckUseGUI(hwndDlg, TRUE);

					// Font
					CheckFont(hwndDlg, TRUE);
					CheckFontSize(hwndDlg, TRUE);

					// Startup position
					CheckCustomStartupPosition(hwndDlg, TRUE);
					CheckXPosition(hwndDlg, TRUE);
					CheckYPosition(hwndDlg, TRUE);

					// Window
					CheckReiszeMode(hwndDlg, TRUE);
					CheckInitialState(hwndDlg, TRUE);

					// GUI Features
					CheckEnableMenubar(hwndDlg, TRUE);
					CheckEnableToolbar(hwndDlg, TRUE);
					CheckEnableStatusbar(hwndDlg, TRUE);
					CheckRemoveBars(hwndDlg, TRUE);
					CheckEnableDialogs(hwndDlg, TRUE);

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

UINT CALLBACK GuiPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp) {
	switch (uMsg) {
	case PSPCB_CREATE:
		profile = (ConnectionProfile*)ppsp->lParam;
		return TRUE;
	case PSPCB_RELEASE:
		return 0;
	}
	return 0;
}

