#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include "../conn_props.h"
#include "../conn_profile.h"
#include "../resource.h"

/******************
 * SHEET: Terminal
 ******************/

/*
	IDC_LOGIN_USER			Username
	IDC_LOGIN_PASS			Password
	IDC_LOGIN_PROMPT		Password Prompt
	IDC_LOGIN_FILENAME		Script filename
	IDC_LOGIN_RUN_FILE		run script file checkbox
	IDC_LOGIN_BROWSE		Browse for script file button
	IDC_LOGIN_SCRIPT		script or script file contents
 */

typedef struct tagDialogFieldStatus {
	int id;
	BOOL dirty;
} DialogFieldStatus;

// Connection profile we're editing
static ConnectionProfile *profile;

static const int fieldCount = 6;

static DialogFieldStatus fieldStatus[] = {
	{ IDC_LOGIN_USER,		FALSE	},
	{ IDC_LOGIN_PASS,		FALSE	},
	{ IDC_LOGIN_PROMPT,		FALSE	},
	{ IDC_LOGIN_FILENAME,	FALSE	},
	{ IDC_LOGIN_RUN_FILE,	FALSE	},
	{ IDC_LOGIN_SCRIPT,		FALSE	},
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

// Caller is responsible for cleaning up the returned string
static LPTSTR getFieldText(HWND hwndDlg, int controlId) {
	HWND hWnd = GetDlgItem(hwndDlg, controlId);

	int len = GetWindowTextLength(hWnd) + 1;

	LPTSTR buf = (LPTSTR)malloc(len * sizeof(TCHAR));
	ZeroMemory(buf, len * sizeof(TCHAR));

	GetWindowText(hWnd, buf, len);

	return buf;
}

static void setLoginScriptFileMode(HWND hwndDlg, BOOL file) {

	if (file) {
		EnableWindow(GetDlgItem(hwndDlg,IDC_LOGIN_BROWSE), TRUE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_LOGIN_FILENAME), TRUE);

		SendMessage(GetDlgItem(hwndDlg, IDC_LOGIN_SCRIPT),
			EM_SETREADONLY,
			(WPARAM)TRUE,
			(LPARAM)0);

	} else {
		EnableWindow(GetDlgItem(hwndDlg,IDC_LOGIN_BROWSE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_LOGIN_FILENAME), FALSE);

		SendMessage(GetDlgItem(hwndDlg, IDC_LOGIN_SCRIPT),
			EM_SETREADONLY,
			(WPARAM)FALSE,
			(LPARAM)0);
	}
}

BOOL CALLBACK LoginPageDlgProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch(uMsg) {
	case WM_INITDIALOG: 
		{
			// Populate fields
			SetWindowText(
				GetDlgItem(hwndDlg, IDC_LOGIN_USER),
				profile->userId().data());
			SetWindowText(
				GetDlgItem(hwndDlg, IDC_LOGIN_PASS),
				profile->password().data());
			SetWindowText(
				GetDlgItem(hwndDlg, IDC_LOGIN_PROMPT),
				profile->passwordPrompt().data());
			SetWindowText(
				GetDlgItem(hwndDlg, IDC_LOGIN_FILENAME),
				profile->loginScriptFile().data());


			CheckDlgButton(hwndDlg, IDC_LOGIN_RUN_FILE, 
				profile->runLoginScriptFile() ? BST_CHECKED : BST_UNCHECKED);
			
			BOOL scriptFile = profile->runLoginScriptFile();

			setLoginScriptFileMode(hwndDlg, scriptFile);

			

			if (!scriptFile) {
				SetWindowText(
					GetDlgItem(hwndDlg, IDC_LOGIN_SCRIPT),
					profile->loginScript().data());
			}

/*
	IDC_LOGIN_USER			Username
	IDC_LOGIN_PASS			Password
	IDC_LOGIN_PROMPT		Password Prompt
	IDC_LOGIN_FILENAME		Script filename
	IDC_LOGIN_RUN_FILE		run script file checkbox
	IDC_LOGIN_BROWSE		Browse for script file button
	IDC_LOGIN_SCRIPT		script or script file contents
 */
			// Form is clean
			cleanForm();

			break;
		}
	case WM_COMMAND: {
			WORD wNotifyCode = HIWORD(wParam);
			WORD wID = LOWORD(wParam);
			

			switch(wID) {
			case IDC_LOGIN_USER:
				{
					setDirty(wID,
						textFieldChanged(
							GetDlgItem(hwndDlg,wID),
							profile->userId().data()));
					FieldChanged(hwndDlg);
				}
				break;
			case IDC_LOGIN_PASS:
				{
					setDirty(wID,
						textFieldChanged(
							GetDlgItem(hwndDlg,wID),
							profile->password().data()));
					FieldChanged(hwndDlg);
				}
				break;
			case IDC_LOGIN_PROMPT:
				{
					setDirty(wID,
						textFieldChanged(
							GetDlgItem(hwndDlg,wID),
							profile->passwordPrompt().data()));
					FieldChanged(hwndDlg);
				}
				break;
			case IDC_LOGIN_FILENAME:
				{
					setDirty(wID,
						textFieldChanged(
							GetDlgItem(hwndDlg,wID),
							profile->loginScriptFile().data()));
					FieldChanged(hwndDlg);
				}
				break;
			case IDC_LOGIN_RUN_FILE:
				{
					BOOL fileMode = IsDlgButtonChecked(
						hwndDlg, wID) == BST_CHECKED;

					setLoginScriptFileMode(hwndDlg, fileMode);
				
					setDirty(wID,
						fileMode != profile->runLoginScriptFile());
					FieldChanged(hwndDlg);
				}
				break;
			case IDC_LOGIN_SCRIPT:
				{
					setDirty(wID,
						textFieldChanged(
							GetDlgItem(hwndDlg,wID),
							profile->loginScript().data()));
					FieldChanged(hwndDlg);
				}
				break;
			case IDC_LOGIN_BROWSE:
				{
					// Browse for login script file

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
					ofn.lpstrTitle = TEXT("Select Login Script");
					ofn.Flags = 
						OFN_PATHMUSTEXIST | 
						OFN_FILEMUSTEXIST | 
						OFN_HIDEREADONLY  ;
;
					
					// Display the Open dialog box. 
					if (GetOpenFileName(&ofn)==TRUE) {
						SetWindowText(GetDlgItem(hwndDlg, IDC_LOGIN_FILENAME),
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
					if (isDirty(IDC_LOGIN_USER)) {

						LPTSTR buf = getFieldText(hwndDlg, IDC_LOGIN_USER);
						
						profile->setUserId(buf);

						free(buf);
					}

					if (isDirty(IDC_LOGIN_PASS)) {

						LPTSTR buf = getFieldText(hwndDlg, IDC_LOGIN_PASS);
						
						profile->setPassword(buf);

						free(buf);
					}

					if (isDirty(IDC_LOGIN_PROMPT)) {

						LPTSTR buf = getFieldText(hwndDlg, IDC_LOGIN_PROMPT);
						
						profile->setPasswordPrompt(buf);

						free(buf);
					}

					if (isDirty(IDC_LOGIN_RUN_FILE)) {
						profile->setRunLoginScriptFile(
							IsDlgButtonChecked(
								hwndDlg, IDC_LOGIN_RUN_FILE) == BST_CHECKED);
					}

					if (isDirty(IDC_LOGIN_FILENAME)) {

						LPTSTR buf = getFieldText(hwndDlg, IDC_LOGIN_FILENAME);
						
						profile->setLoginScriptFile(buf);

						free(buf);
					}

					if (isDirty(IDC_LOGIN_SCRIPT)) {

						LPTSTR buf = getFieldText(hwndDlg, IDC_LOGIN_SCRIPT);
						
						profile->setLoginScript(buf);

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

UINT CALLBACK LoginPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp) {
	switch (uMsg) {
	case PSPCB_CREATE:
		profile = (ConnectionProfile*)ppsp->lParam;
		return TRUE;
	case PSPCB_RELEASE:
		return 0;
	}
	return 0;
}