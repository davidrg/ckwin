#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include "../conn_props.h"
#include "../conn_profile.h"
#include "../resource.h"

/******************
 * SHEET: Keyboard
 ******************/


/*
					
Radio Buttons:
  IDC_KBD_BS_DELETE		Backspace key sends: Delete (Rubout)
  IDC_KBD_BS_BS			Backspace key sends: Backspace (Ctrl+H)
  IDC_KBD_BS_CTRL_Y		Backspace key sends: EM (Ctrl+Y)

  IDC_KBD_RET_CR		Enter Key Sends: Carriage return
  IDC_KBD_RET_CRLF		Enter Key Sends: CR-LF
  IDC_KBD_RET_LF		Enter Key Sends: Line Feed
  
  IDC_MOU_ON			Mouse: On
  IDC_MOU_OFF			Mouse: Off
  
Edit:
  IDC_KBD_KM_FILENAME	Key Map: read from file: filename
  IDC_KBD_KM_KEYMAP		Key Map: additional mapping

Buttons:
  IDC_KBD_KM_FILE_BROWSE	Key Map: read from file: browse for file
  */


typedef struct tagDialogFieldStatus {
	int id;
	BOOL dirty;
} DialogFieldStatus;

// Connection profile we're editing
static ConnectionProfile *profile;

static const int fieldCount = 10;

static DialogFieldStatus fieldStatus[] = {
	{ IDC_KBD_BS_DELETE,	FALSE	},
	{ IDC_KBD_BS_BS,		FALSE	},
	{ IDC_KBD_BS_CTRL_Y,	FALSE	},

	{ IDC_KBD_RET_CR,		FALSE	},
	{ IDC_KBD_RET_CRLF,		FALSE	},
	{ IDC_KBD_RET_LF,		FALSE	},

	{ IDC_MOU_ON,			FALSE	},
	{ IDC_MOU_OFF,			FALSE	},

	{ IDC_KBD_KM_FILENAME,	FALSE	},
	{ IDC_KBD_KM_KEYMAP,	FALSE	}

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
static void FieldChanged(HWND hwndDlg) {
	if (isDirty()) {
		PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
	} else {
		PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);
	}
}

ConnectionProfile::KeyboardBackspaceMode selectedBackspaceMode(HWND hwndDlg) {
	ConnectionProfile::KeyboardBackspaceMode newValue;

	if (IsDlgButtonChecked(hwndDlg, IDC_KBD_BS_DELETE) == BST_CHECKED) {
		newValue = ConnectionProfile::KBM_DELETE;
	} else if (IsDlgButtonChecked(hwndDlg, IDC_KBD_BS_BS) == BST_CHECKED) {
		newValue = ConnectionProfile::KBM_CTRL_H;
	} else { /* IDC_KBD_BS_CTRL_Y */
		newValue = ConnectionProfile::KBM_CTRL_Y;
	}

	return newValue;
}

ConnectionProfile::KeyboardEnterMode selectedEnterMode(HWND hwndDlg) {
	ConnectionProfile::KeyboardEnterMode newValue;

	if (IsDlgButtonChecked(hwndDlg, IDC_KBD_RET_CR) == BST_CHECKED) {
		newValue = ConnectionProfile::KEM_CR;
	} else if (IsDlgButtonChecked(hwndDlg, IDC_KBD_RET_CRLF) == BST_CHECKED) {
		newValue = ConnectionProfile::KEM_CR_LF;
	} else { /* IDC_KBD_RET_LF */
		newValue = ConnectionProfile::KEM_LF;
	}

	return newValue;
}


BOOL CALLBACK KeyboardPageDlgProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch(uMsg) {
	case WM_INITDIALOG: 
		{
			// Uncheck all the things!
			CheckDlgButton(hwndDlg, IDC_KBD_BS_DELETE, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_KBD_BS_BS, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_KBD_BS_CTRL_Y, BST_UNCHECKED);
			
			CheckDlgButton(hwndDlg, IDC_KBD_RET_CR, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_KBD_RET_CRLF, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_KBD_RET_LF, BST_UNCHECKED);
			
			CheckDlgButton(hwndDlg, IDC_MOU_ON, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MOU_OFF, BST_UNCHECKED);

			switch(profile->keyboardBackspaceSends()) {
			case ConnectionProfile::KBM_DELETE:
				CheckDlgButton(hwndDlg, IDC_KBD_BS_DELETE, BST_CHECKED);
				break;
			case ConnectionProfile::KBM_CTRL_H:
				CheckDlgButton(hwndDlg, IDC_KBD_BS_BS, BST_CHECKED);
				break;
			case ConnectionProfile::KBM_CTRL_Y:
				CheckDlgButton(hwndDlg, IDC_KBD_BS_CTRL_Y, BST_CHECKED);
				break;
			}

			switch(profile->keyboardEnterSends()) {
			case ConnectionProfile::KEM_CR:
				CheckDlgButton(hwndDlg, IDC_KBD_RET_CR, BST_CHECKED);
				break;
			case ConnectionProfile::KEM_CR_LF:
				CheckDlgButton(hwndDlg, IDC_KBD_RET_CRLF, BST_CHECKED);
				break;
			case ConnectionProfile::KEM_LF:
				CheckDlgButton(hwndDlg, IDC_KBD_RET_LF, BST_CHECKED);
				break;
			}

			if (profile->mouseEnabled()) {
				CheckDlgButton(hwndDlg, IDC_MOU_ON, BST_CHECKED);
			} else {
				CheckDlgButton(hwndDlg, IDC_MOU_OFF, BST_CHECKED);
			}


			// Populate text
			SetWindowText(
				GetDlgItem(hwndDlg, IDC_KBD_KM_FILENAME),
				profile->keymapFile().data());
			SetWindowText(
				GetDlgItem(hwndDlg, IDC_KBD_KM_KEYMAP),
				profile->additionalKeyMaps().data());


			// Form is clean
			cleanForm();

			break;
		}
	case WM_COMMAND: {
			WORD wNotifyCode = HIWORD(wParam);
			WORD wID = LOWORD(wParam);
			
			switch(wID) {
			case IDC_KBD_BS_DELETE:
			case IDC_KBD_BS_BS:
			case IDC_KBD_BS_CTRL_Y:
				{
					ConnectionProfile::KeyboardBackspaceMode oldValue, newValue;

					oldValue = profile->keyboardBackspaceSends();
					newValue = selectedBackspaceMode(hwndDlg);

					BOOL changed = oldValue != newValue;
					
					setDirty(IDC_KBD_BS_DELETE, changed);
					setDirty(IDC_KBD_BS_BS, changed);
					setDirty(IDC_KBD_BS_CTRL_Y, changed);

					FieldChanged(hwndDlg);
				}
				break;
			case IDC_KBD_RET_CR:
			case IDC_KBD_RET_CRLF:
			case IDC_KBD_RET_LF:
				{
					ConnectionProfile::KeyboardEnterMode oldValue, newValue;

					oldValue = profile->keyboardEnterSends();
					newValue = selectedEnterMode(hwndDlg);

					BOOL changed = oldValue != newValue;
					
					setDirty(IDC_KBD_RET_CR, changed);
					setDirty(IDC_KBD_RET_CRLF, changed);
					setDirty(IDC_KBD_RET_LF, changed);

					FieldChanged(hwndDlg);
				}
				break;
			case IDC_MOU_ON:
			case IDC_MOU_OFF:
				{
					BOOL oldValue, newValue;

					oldValue = profile->mouseEnabled();

					newValue = IsDlgButtonChecked(hwndDlg, IDC_MOU_ON) == BST_CHECKED;

					BOOL changed = oldValue != newValue;
					
					setDirty(IDC_MOU_ON, changed);
					setDirty(IDC_MOU_OFF, changed);

					FieldChanged(hwndDlg);
				}
				break;
			case IDC_KBD_KM_FILENAME:
				{
					setDirty(wID,
						textFieldChanged(
							GetDlgItem(hwndDlg,wID),
							profile->keymapFile().data()));
					FieldChanged(hwndDlg);
				}
				break;
			case IDC_KBD_KM_KEYMAP:
				{
					setDirty(wID,
						textFieldChanged(
							GetDlgItem(hwndDlg,wID),
							profile->additionalKeyMaps().data()));
					FieldChanged(hwndDlg);
				}
				break;
			case IDC_KBD_KM_FILE_BROWSE:
				{
					// Browse for keymap script file

					OPENFILENAME ofn;
					LPTSTR szFile[MAX_PATH];

					ZeroMemory(szFile, sizeof(TCHAR) * MAX_PATH);
					ZeroMemory(&ofn, sizeof(OPENFILENAME));

					ofn.lStructSize = sizeof(OPENFILENAME);					
					ofn.hwndOwner = hwndDlg;
					ofn.lpstrFile = (LPTSTR)szFile;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = TEXT("Kermit Scripts (*.ksc)\0*.KSC\0Text Files (*.txt)\0*.TXT\0All Files (*.*)\0*.*\0");
					ofn.nFilterIndex = 1;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.lpstrTitle = TEXT("Select Keymap Script");
					ofn.Flags = 
						OFN_PATHMUSTEXIST | 
						OFN_FILEMUSTEXIST | 
						OFN_HIDEREADONLY  ;
;
					
					// Display the Open dialog box. 
					if (GetOpenFileName(&ofn)==TRUE) {
						SetWindowText(GetDlgItem(hwndDlg, IDC_KBD_KM_FILENAME),
							(LPCTSTR)szFile);
					}

				}
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
					if (isDirty(IDC_KBD_BS_DELETE) ||
						isDirty(IDC_KBD_BS_BS) ||
						isDirty(IDC_KBD_BS_CTRL_Y)) {

						profile->setKeyboardBackspaceSends(
							selectedBackspaceMode(hwndDlg));
					}

					if (isDirty(IDC_KBD_RET_CR) ||
						isDirty(IDC_KBD_RET_CRLF) ||
						isDirty(IDC_KBD_RET_LF)) {

						
						
						profile->setKeyboardEnterSends(
							selectedEnterMode(hwndDlg));
					}

					if (isDirty(IDC_MOU_ON) ||
						isDirty(IDC_MOU_OFF)) {

						profile->setMouseEnabled(
							IsDlgButtonChecked(hwndDlg, IDC_MOU_ON) == BST_CHECKED);
					}


					if (isDirty(IDC_KBD_KM_FILENAME)) {

						LPTSTR buf = getFieldText(hwndDlg, IDC_KBD_KM_FILENAME);
						
						profile->setKeymapFile(buf);

						free(buf);
					}

					if (isDirty(IDC_KBD_KM_KEYMAP)) {

						LPTSTR buf = getFieldText(hwndDlg, IDC_KBD_KM_KEYMAP);
						
						profile->setAdditionalKeyMaps(buf);

						free(buf);
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

UINT CALLBACK KeyboardPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp) {
	switch (uMsg) {
	case PSPCB_CREATE:
		profile = (ConnectionProfile*)ppsp->lParam;
		return TRUE;
	case PSPCB_RELEASE:
		return 0;
	}
	return 0;
}