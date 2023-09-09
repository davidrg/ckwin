#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include "../conn_props.h"
#include "../conn_profile.h"
#include "../resource.h"
#include "../term_info.h"
#include "../charset.h"

/******************
 * SHEET: Terminal
 ******************/

/*
Controls:
IDC_TERM_TYPE				List Box			Terminal type list
IDC_TERM_WIDTH				Drop Down List		Terminal width: 80, 132
IDC_TERM_HEIGHT				Drop Down List		Terminal height: 24, 25, 42, 43, 49, 50
IDC_TERM_SCROLLBACK			Line Edit			Scrollback lines.
IDC_TERM_SCROLLBACK_SPIN	Spin Box			Buttons for scrollback lines box
IDC_TERM_CURSOR				Drop Down List		Options: full, half, underline
IDC_TERM_CSET				Drop Down List		Many options
IDC_TERM_BITS_7				Radio Button		
IDC_TERM_BITS_8				Radio Button
IDC_TERM_LOCAL_ECHO			Check Box
IDC_TERM_AUTO_WRAP			Check Box
IDC_TERM_APC				Check Box
IDC_TERM_STATUS_LINE		Check Box
 */

// These are the options the Kermit 95 dialer provided.
// I don't know the significance of these numbers.
static const int terminalWidths[] = { 80, 132, 0 };
static const int terminalHeights[] = { 24, 25, 42, 43, 49, 50, 0 };

// The indexes in this array *must* match up with the
// values of the ConnectionProfile::Cursor enum
static LPCTSTR cursorOptions[] = { TEXT("Full"), TEXT("Half"), TEXT("Underline"), NULL };

typedef struct tagDialogFieldStatus {
	int id;
	BOOL dirty;
} DialogFieldStatus;

// Connection profile we're editing
static ConnectionProfile *profile;

static const int fieldCount = 13;

static DialogFieldStatus fieldStatus[] = {
	{ IDC_TERM_TYPE,			FALSE	},
	{ IDC_TERM_WIDTH,			FALSE	},
	{ IDC_TERM_HEIGHT,			FALSE	},
	{ IDC_TERM_SCROLLBACK,		FALSE	},
	{ IDC_TERM_SCROLLBACK_SPIN,	FALSE	},
	{ IDC_TERM_CURSOR,			FALSE	},
	{ IDC_TERM_CSET,			FALSE	},
	{ IDC_TERM_BITS_7,			FALSE	},
	{ IDC_TERM_BITS_8,			FALSE	},
	{ IDC_TERM_LOCAL_ECHO,		FALSE	},
	{ IDC_TERM_AUTO_WRAP,		FALSE	},
	{ IDC_TERM_APC,				FALSE	},
	{ IDC_TERM_STATUS_LINE,		FALSE	}
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

static int getFieldInt(HWND hwndDlg, int id) {
	LPTSTR buf[20];

	HWND hwnd = GetDlgItem(hwndDlg, id);

	GetWindowText(hwnd, (LPTSTR)buf, 20);
	return _ttoi((LPCTSTR)buf);
}


static void CheckTermType(HWND hwndDlg, BOOL save) {
	HWND hwndTermType = GetDlgItem(hwndDlg, IDC_TERM_TYPE);

	int idx = SendMessage(hwndTermType, LB_GETCURSEL, 0, 0);

	Term::TermType existingType = profile->terminalType();

	Term::TermType newType = 
		(Term::TermType)SendMessage(
			hwndTermType, LB_GETITEMDATA, idx, 0);

	BOOL changed = newType != existingType;

	if (!save) {
		setDirty(IDC_TERM_TYPE, changed);
		TerminalFieldChanged(hwndDlg);
		return;
	}

	if (changed) {
		profile->setTerminalType(newType);
	}
}

static void CheckWidth(HWND hwndDlg, BOOL save) {
	int newValue = getFieldInt(hwndDlg, IDC_TERM_WIDTH);
	int oldValue = profile->screenWidth();

	BOOL changed = newValue != oldValue;

	if (!save) {
		setDirty(IDC_TERM_WIDTH, changed);
		TerminalFieldChanged(hwndDlg);
		return;
	}

	if (changed) {
		profile->setScreenWidth(newValue);
	}
}

static void CheckHeight(HWND hwndDlg, BOOL save) {
	int newValue = getFieldInt(hwndDlg, IDC_TERM_HEIGHT);
	int oldValue = profile->screenHeight();

	BOOL changed = newValue != oldValue;
	
	if (!save) {
		setDirty(IDC_TERM_HEIGHT, changed);
		TerminalFieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setScreenHeight(newValue);
	}
}

static void CheckScrollback(HWND hwndDlg, BOOL save) {
	int newValue = getFieldInt(hwndDlg, IDC_TERM_SCROLLBACK);
	int oldValue = profile->scrollbackLines();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_TERM_SCROLLBACK, changed);
		TerminalFieldChanged(hwndDlg);
		return;
	}

	if (changed) {
		profile->setScrollbackLines(newValue);
	}
}

static void CheckCursor(HWND hwndDlg, BOOL save) {
	HWND hwndCursor = GetDlgItem(hwndDlg, IDC_TERM_CURSOR);

	LPTSTR buf[20];

	GetWindowText(hwndCursor, (LPTSTR)buf, 20);

	ConnectionProfile::Cursor newValue;

	for (int i = 0; cursorOptions[i] != NULL; i++) {
		if (lstrcmp((LPCTSTR)buf, cursorOptions[i]) == 0) {
			newValue = (ConnectionProfile::Cursor)i;
			
			break;
		}
	}

	BOOL changed = newValue != profile->cursor();

	if (!save) {
		setDirty(IDC_TERM_CURSOR, changed);
		TerminalFieldChanged(hwndDlg);
		return;
	}

	if (changed) {
		profile->setCursor(newValue);
	}
}

static void CheckCharset(HWND hwndDlg, BOOL save) {
	HWND hwndCset = GetDlgItem(hwndDlg, IDC_TERM_CSET);

	int idx = SendMessage(hwndCset, CB_GETCURSEL, 0, 0);

	Charset::Charset existing = profile->characterSet();

	Charset::Charset newValue =  
			(Charset::Charset)SendMessage(
				hwndCset, CB_GETITEMDATA, idx, 0);

	BOOL changed = newValue != existing;

	

	if (!save) {
		setDirty(IDC_TERM_CSET, changed);
		TerminalFieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setCharacterSet(newValue);
	}
}


static void CheckBits(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_TERM_BITS_8) == BST_CHECKED;
	BOOL oldValue = profile->is8Bit();

	BOOL changed = newValue != oldValue;

	if (!save) {
		setDirty(IDC_TERM_BITS_8, changed);
		setDirty(IDC_TERM_BITS_7, changed);
		TerminalFieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setIs8Bit(newValue);
	}
}

static void CheckLocalEcho(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_TERM_LOCAL_ECHO) == BST_CHECKED;
	BOOL oldValue = profile->localEchoEnabled();

	BOOL changed = newValue != oldValue;

	if (!save) {
		setDirty(IDC_TERM_LOCAL_ECHO, changed);
		TerminalFieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setLocalEchoEnabled(newValue);
	}
}

static void CheckAutoWrap(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_TERM_AUTO_WRAP) == BST_CHECKED;
	BOOL oldValue = profile->autoWrapEnabled();

	BOOL changed = newValue != oldValue;

	if (!save) {
		setDirty(IDC_TERM_AUTO_WRAP, changed);
		TerminalFieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setAutoWrapEnabled(newValue);
	}
}

static void CheckAPC(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_TERM_APC) == BST_CHECKED;
	BOOL oldValue = profile->apcEnabled();

	BOOL changed = newValue != oldValue;

	
	if (!save) {
		setDirty(IDC_TERM_APC, changed);
		TerminalFieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setApcEnabled(newValue);
	}
}

static void CheckStatusLine(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_TERM_STATUS_LINE) == BST_CHECKED;
	BOOL oldValue = profile->statusLineEnabled();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_TERM_STATUS_LINE, changed);
		TerminalFieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setStatusLineEnabled(newValue);
	}
}

BOOL CALLBACK TerminalPageDlgProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch(uMsg) {
	case WM_INITDIALOG: 
		{
			HWND hwndTermType = GetDlgItem(hwndDlg, IDC_TERM_TYPE);
			HWND hwndTermWidths = GetDlgItem(hwndDlg, IDC_TERM_WIDTH);
			HWND hwndTermHeights = GetDlgItem(hwndDlg, IDC_TERM_HEIGHT);
			HWND hwndCursor = GetDlgItem(hwndDlg, IDC_TERM_CURSOR);
			
			int i;

			// Populate lists:
			
			// Terminal Widths...
			for (i = 0; terminalWidths[i] != 0; i++) {
				CMString str = CMString::number(terminalWidths[i]);
				SendMessage(
					hwndTermWidths,
					(UINT) CB_ADDSTRING,
					(WPARAM) 0,
					(LPARAM) str.data());
			}

			// Terminal Heights...
			for (i = 0; terminalHeights[i] != 0; i++) {
				CMString str = CMString::number(terminalHeights[i]);
				SendMessage(
					hwndTermHeights,
					(UINT) CB_ADDSTRING,
					(WPARAM) 0,
					(LPARAM) str.data());
			}

			// Cursors...
			for (i = 0; cursorOptions[i] != 0; i++) {
				SendMessage(
					hwndCursor,
					(UINT) CB_ADDSTRING,
					(WPARAM) 0,
					(LPARAM) cursorOptions[i]);
			}
			

			// Terminal Types...
			const Term::TermInfo *terminals = Term::getTerminalInfos();

			for (i = 0; terminals[i].type != Term::TT_INVALID; i++) {
				SendMessage(hwndTermType, 
					LB_ADDSTRING, 
					(WPARAM) 0, 
                    (LPARAM) terminals[i].keyword); 

                SendMessage(hwndTermType, 
					LB_SETITEMDATA, 
					(WPARAM) i, 
					(LPARAM) terminals[i].type); 
			}

			// Charsets...
			Charset::populateDropList(
				GetDlgItem(hwndDlg, IDC_TERM_CSET),
				Charset::CU_TERMINAL,
				TRUE);
			

			// Set initial values
			SendMessage(hwndTermType, 
				LB_SELECTSTRING, 
				(WPARAM)0,
				(LPARAM)Term::getTermKeyword(profile->terminalType()));

			setFieldText(hwndDlg, IDC_TERM_WIDTH, CMString::number(profile->screenWidth()));

			setFieldText(hwndDlg, IDC_TERM_HEIGHT, CMString::number(profile->screenHeight()));

			setFieldText(hwndDlg, IDC_TERM_SCROLLBACK, CMString::number(profile->scrollbackLines()));

			// Cursor
			ConnectionProfile::Cursor currentCursorOpt = profile->cursor();
			int cursorInt = (int)currentCursorOpt;
			LPCTSTR selectedCursor = cursorOptions[cursorInt];
			SendMessage(
					GetDlgItem(hwndDlg, IDC_TERM_CURSOR),
					CB_SELECTSTRING ,
					(WPARAM)0,
					(LPARAM)selectedCursor);
			
			// Select the selected charset
			LPTSTR selectedCS = Charset::getCharsetLabel(
				profile->characterSet(), TRUE);
			if (selectedCS != NULL) {
				SendMessage(
					GetDlgItem(hwndDlg, IDC_TERM_CSET),
					CB_SELECTSTRING ,
					(WPARAM)0,
					(LPARAM)selectedCS);
				free(selectedCS);
			}

			if (profile->is8Bit()) {
				CheckDlgButton(hwndDlg, IDC_TERM_BITS_8, BST_CHECKED);
			} else {
				CheckDlgButton(hwndDlg, IDC_TERM_BITS_7, BST_CHECKED);
			}


			CheckDlgButton(hwndDlg, IDC_TERM_LOCAL_ECHO, 
				profile->localEchoEnabled() ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_TERM_AUTO_WRAP, 
				profile->autoWrapEnabled() ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_TERM_APC, 
				profile->apcEnabled() ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_TERM_STATUS_LINE, 
				profile->statusLineEnabled() ? BST_CHECKED : BST_UNCHECKED);	
			
			break;
		}
	case WM_COMMAND: {
			WORD wNotifyCode = HIWORD(wParam);
			WORD wID = LOWORD(wParam);
			

			switch(wID) {
			case IDC_TERM_TYPE:
				{
					if (HIWORD(wParam) == LBN_SELCHANGE) {
						CheckTermType(hwndDlg, FALSE);
					}
				}

				break;

			case IDC_TERM_WIDTH: 
				CheckWidth(hwndDlg, FALSE);
				break;

			case IDC_TERM_HEIGHT: 
				CheckHeight(hwndDlg, FALSE);
				break;

			case IDC_TERM_SCROLLBACK:
				CheckScrollback(hwndDlg, FALSE);
				break;

			case IDC_TERM_CURSOR:
				CheckCursor(hwndDlg, FALSE);
				break;

			case IDC_TERM_CSET:
				CheckCharset(hwndDlg, FALSE);
				break;

			case IDC_TERM_BITS_8:
			case IDC_TERM_BITS_7:
				CheckBits(hwndDlg, FALSE);
				break;
			case IDC_TERM_LOCAL_ECHO:
				CheckLocalEcho(hwndDlg, FALSE);
				break;
			case IDC_TERM_AUTO_WRAP:
				CheckAutoWrap(hwndDlg, FALSE);
				break;
			case IDC_TERM_APC:
				CheckAPC(hwndDlg, FALSE);
				break;
			case IDC_TERM_STATUS_LINE:
				CheckStatusLine(hwndDlg, FALSE);
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
					CheckTermType(hwndDlg, TRUE);
					CheckWidth(hwndDlg, TRUE);
					CheckHeight(hwndDlg, TRUE);
					CheckScrollback(hwndDlg, TRUE);
					CheckCursor(hwndDlg, TRUE);
					CheckCharset(hwndDlg, TRUE);
					CheckBits(hwndDlg, TRUE);
					CheckLocalEcho(hwndDlg, TRUE);
					CheckAutoWrap(hwndDlg, TRUE);
					CheckAPC(hwndDlg, TRUE);
					CheckStatusLine(hwndDlg, TRUE);
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

UINT CALLBACK TerminalPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp) {
	switch (uMsg) {
	case PSPCB_CREATE:
		profile = (ConnectionProfile*)ppsp->lParam;
		return TRUE;
	case PSPCB_RELEASE:
		return 0;
	}
	return 0;
}