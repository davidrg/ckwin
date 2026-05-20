#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <wingdi.h>

#include "../conn_props.h"
#include "../conn_profile.h"
#include "../resource.h"


/******************
 * SHEET: Logging
 ******************/

/*
Controls:
IDC_LOG_CONNECTIONS			Checkbox
IDC_LOG_CONNECTIONS_FILE	Text edit
IDC_LOG_CONNECTIONS_BROWSE	Button
IDC_LOG_CONNECTIONS_APPEND	Append
IDC_LOG_DEBUG				Checkbox
IDC_LOG_DEBUG_FILE			Text edit
IDC_LOG_DEBUG_BROWSE		Button
IDC_LOG_DEBUG_APPEND		Checkbox
IDC_LOG_PACKETS				Checkbox
IDC_LOG_PACKETS_FILE		Text edit
IDC_LOG_PACKETS_BROWSE		Button
IDC_LOG_PACKETS_APPEND		Checkbox
IDC_LOG_SESSION				Checkbox
IDC_LOG_SESSION_FILE		Text edit
IDC_LOG_SESSION_BROWSE		Button
IDC_LOG_SESSION_TXT			Radio button
IDC_LOG_SESSION_BIN			Radio button
IDC_LOG_SESSION_DBG			Radio button
IDC_LOG_SESSION_APPEND		Checkbox
IDC_LOG_XFER				Checkbox
IDC_LOG_XFER_FILE			Text edit
IDC_LOG_XFER_BROWSE			Button
IDC_LOG_XFER_BRIEF			Checkbox
IDC_LOG_XFER_APPEND			Checkbox
 */

// Connection profile we're editing
static ConnectionProfile *profile;


typedef struct tagDialogFieldStatus {
	int id;
	BOOL dirty;
} DialogFieldStatus;


static const int fieldCount = 19;

static DialogFieldStatus fieldStatus[] = {
	{ IDC_LOG_CONNECTIONS,			FALSE	},
	{ IDC_LOG_CONNECTIONS_FILE,		FALSE	},
	{ IDC_LOG_CONNECTIONS_APPEND,	FALSE	},
	{ IDC_LOG_DEBUG,				FALSE	},
	{ IDC_LOG_DEBUG_FILE,			FALSE	},
	{ IDC_LOG_DEBUG_APPEND,			FALSE	},
	{ IDC_LOG_PACKETS,				FALSE	},
	{ IDC_LOG_PACKETS_FILE,			FALSE	},
	{ IDC_LOG_PACKETS_APPEND,		FALSE	},
	{ IDC_LOG_SESSION,				FALSE	},
	{ IDC_LOG_SESSION_FILE,			FALSE	},
	{ IDC_LOG_SESSION_TXT,			FALSE	},
	{ IDC_LOG_SESSION_BIN,			FALSE	},
	{ IDC_LOG_SESSION_DBG,			FALSE	},
	{ IDC_LOG_SESSION_APPEND,		FALSE	},
	{ IDC_LOG_XFER,					FALSE	},
	{ IDC_LOG_XFER_FILE,			FALSE	},
	{ IDC_LOG_XFER_BRIEF,			FALSE	},
	{ IDC_LOG_XFER_APPEND,			FALSE	}
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


static void BrowseFile(HWND hwndDlg, int targetFieldId, LPCTSTR title) {
	OPENFILENAME ofn;
	LPTSTR szFile[MAX_PATH];

	ZeroMemory(szFile, sizeof(TCHAR) * MAX_PATH);
	ZeroMemory(&ofn, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);					
	ofn.hwndOwner = hwndDlg;
	ofn.lpstrFile = (LPTSTR)szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = TEXT("Log Files (*.log)\0*.LOG\0All Files (*.*)\0*.*\0");
	ofn.lpstrDefExt = TEXT("log");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = title;
	ofn.Flags = 
		OFN_PATHMUSTEXIST | 
		OFN_HIDEREADONLY  ;
	
	// Display the Save dialog box. 
	if (GetSaveFileName(&ofn)==TRUE) {
		SetWindowText(GetDlgItem(hwndDlg, targetFieldId),
			(LPCTSTR)szFile);
	}

}


static void EnableDisableFields(HWND hwndDlg) {
	BOOL connections = IsDlgButtonChecked(hwndDlg, IDC_LOG_CONNECTIONS) == BST_CHECKED;	

	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_CONNECTIONS_FILE), connections);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_CONNECTIONS_BROWSE), connections);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_CONNECTIONS_APPEND), connections);

	BOOL debug = IsDlgButtonChecked(hwndDlg, IDC_LOG_DEBUG) == BST_CHECKED;	

	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_DEBUG_FILE), debug);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_DEBUG_BROWSE), debug);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_DEBUG_APPEND), debug);

	BOOL packets = IsDlgButtonChecked(hwndDlg, IDC_LOG_PACKETS) == BST_CHECKED;	

	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_PACKETS_FILE), packets);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_PACKETS_BROWSE), packets);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_PACKETS_APPEND), packets);

	BOOL session = IsDlgButtonChecked(hwndDlg, IDC_LOG_SESSION) == BST_CHECKED;	

	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_SESSION_FILE), session);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_SESSION_BROWSE), session);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_SESSION_TXT), session);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_SESSION_BIN), session);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_SESSION_DBG), session);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_SESSION_APPEND), session);

	BOOL xfer = IsDlgButtonChecked(hwndDlg, IDC_LOG_XFER) == BST_CHECKED;	

	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_XFER_FILE), xfer);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_XFER_BROWSE), xfer);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_XFER_BRIEF), xfer);
	EnableWindow(GetDlgItem(hwndDlg,IDC_LOG_XFER_APPEND), xfer);


}


static void CheckConnections(HWND hwndDlg, BOOL save) {
	int id = IDC_LOG_CONNECTIONS;
	BOOL newValue = IsDlgButtonChecked(hwndDlg, id) == BST_CHECKED;
	BOOL oldValue = profile->logConnections();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setLogConnections(newValue);
	}
}


static void CheckConnectionsAppend(HWND hwndDlg, BOOL save) {
	int id = IDC_LOG_CONNECTIONS_APPEND;
	BOOL newValue = IsDlgButtonChecked(hwndDlg, id) == BST_CHECKED;
	BOOL oldValue = profile->logAppendConnections();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setLogAppendConnections(newValue);
	}
}

static void CheckDebug(HWND hwndDlg, BOOL save) {
	int id = IDC_LOG_DEBUG;
	BOOL newValue = IsDlgButtonChecked(hwndDlg, id) == BST_CHECKED;
	BOOL oldValue = profile->logDebug();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setLogDebug(newValue);
	}
}

static void CheckDebugAppend(HWND hwndDlg, BOOL save) {
	int id = IDC_LOG_DEBUG_APPEND;
	BOOL newValue = IsDlgButtonChecked(hwndDlg, id) == BST_CHECKED;
	BOOL oldValue = profile->logAppendDebug();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setLogAppendDebug(newValue);
	}
}

static void CheckPackets(HWND hwndDlg, BOOL save) {
	int id = IDC_LOG_PACKETS;
	BOOL newValue = IsDlgButtonChecked(hwndDlg, id) == BST_CHECKED;
	BOOL oldValue = profile->logKermitFileTransfers();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setLogKermitFileTransfers(newValue);
	}
}


static void CheckPacketsAppend(HWND hwndDlg, BOOL save) {
	int id = IDC_LOG_PACKETS_APPEND;
	BOOL newValue = IsDlgButtonChecked(hwndDlg, id) == BST_CHECKED;
	BOOL oldValue = profile->logAppendKermitFileTransfers();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setLogAppendKermitFileTransfers(newValue);
	}
}


static void CheckSession(HWND hwndDlg, BOOL save) {
	int id = IDC_LOG_SESSION;
	BOOL newValue = IsDlgButtonChecked(hwndDlg, id) == BST_CHECKED;
	BOOL oldValue = profile->logSessionInput();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setLogSessionInput(newValue);
	}
}

static void CheckSessionAppend(HWND hwndDlg, BOOL save) {
	int id = IDC_LOG_SESSION_APPEND;
	BOOL newValue = IsDlgButtonChecked(hwndDlg, id) == BST_CHECKED;
	BOOL oldValue = profile->logAppendSessionInput();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setLogAppendSessionInput(newValue);
	}
}

static void CheckXfer(HWND hwndDlg, BOOL save) {
	int id = IDC_LOG_XFER;
	BOOL newValue = IsDlgButtonChecked(hwndDlg, id) == BST_CHECKED;
	BOOL oldValue = profile->logFileTransferTransactions();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setLogFileTransferTransactions(newValue);
	}
}

static void CheckXferBrief(HWND hwndDlg, BOOL save) {
	int id = IDC_LOG_XFER_BRIEF;
	BOOL newValue = IsDlgButtonChecked(hwndDlg, id) == BST_CHECKED;
	BOOL oldValue = profile->logBriefFileTransferTransactions();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setLogBriefFileTransferTransactions(newValue);
	}
}



static void CheckXferAppend(HWND hwndDlg, BOOL save) {
	int id = IDC_LOG_XFER_APPEND;
	BOOL newValue = IsDlgButtonChecked(hwndDlg, id) == BST_CHECKED;
	BOOL oldValue = profile->logAppendFileTransferTransactions();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setLogAppendFileTransferTransactions(newValue);
	}
}

static void CheckSessionMode(HWND hwndDlg, BOOL save) {
	BOOL txt = IsDlgButtonChecked(hwndDlg, IDC_LOG_SESSION_TXT) == BST_CHECKED;
	BOOL bin = IsDlgButtonChecked(hwndDlg, IDC_LOG_SESSION_BIN) == BST_CHECKED;
	BOOL dbg = IsDlgButtonChecked(hwndDlg, IDC_LOG_SESSION_DBG) == BST_CHECKED;

	ConnectionProfile::LogSessionMode newValue, oldValue;
	if (txt) newValue = ConnectionProfile::LSM_TEXT;
	if (bin) newValue = ConnectionProfile::LSM_BINARY;
	if (dbg) newValue = ConnectionProfile::LSM_DEBUG;
	oldValue = profile->logSessionInputMode();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_LOG_SESSION_TXT, changed);
		setDirty(IDC_LOG_SESSION_BIN, changed);
		setDirty(IDC_LOG_SESSION_DBG, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setLogSessionInputMode(newValue);
	}
}

static void CheckConnectionsFile(HWND hwndDlg, BOOL save) {
	CMString originalValue = profile->logConnectionsFile();
	int id = IDC_LOG_CONNECTIONS_FILE;
	
	HWND hwndField = GetDlgItem(hwndDlg, id);

	// Get current value
	int buflen = GetWindowTextLength(hwndField) + 1;

	// allocate buffer
	LPTSTR buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
	ZeroMemory(buf, buflen * sizeof(TCHAR));

	// Get current value
	GetWindowText(hwndField, buf, buflen);

	BOOL changed = FALSE;
	if (lstrcmp(buf, originalValue.data()) != 0) {
		changed = TRUE;
	}

	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);
	} else if (changed) {
		profile->setLogConnectionsFile(CMString(buf));
	}

	free(buf);
}


static void CheckDebugFile(HWND hwndDlg, BOOL save) {
	CMString originalValue = profile->logDebugFile();
	int id = IDC_LOG_DEBUG_FILE;
	
	HWND hwndField = GetDlgItem(hwndDlg, id);

	// Get current value
	int buflen = GetWindowTextLength(hwndField) + 1;

	// allocate buffer
	LPTSTR buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
	ZeroMemory(buf, buflen * sizeof(TCHAR));

	// Get current value
	GetWindowText(hwndField, buf, buflen);

	BOOL changed = FALSE;
	if (lstrcmp(buf, originalValue.data()) != 0) {
		changed = TRUE;
	}

	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);
	} else if (changed) {
		profile->setLogDebugFile(CMString(buf));
	}

	free(buf);
}

static void CheckPacketsFile(HWND hwndDlg, BOOL save) {
	CMString originalValue = profile->logKermitFileTransfersFile();
	int id = IDC_LOG_PACKETS_FILE;
	
	HWND hwndField = GetDlgItem(hwndDlg, id);

	// Get current value
	int buflen = GetWindowTextLength(hwndField) + 1;

	// allocate buffer
	LPTSTR buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
	ZeroMemory(buf, buflen * sizeof(TCHAR));

	// Get current value
	GetWindowText(hwndField, buf, buflen);

	BOOL changed = FALSE;
	if (lstrcmp(buf, originalValue.data()) != 0) {
		changed = TRUE;
	}

	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);
	} else if (changed) {
		profile->setLogKermitFileTransfersFile(CMString(buf));
	}

	free(buf);
}


static void CheckSessionFile(HWND hwndDlg, BOOL save) {
	CMString originalValue = profile->logSessionInputFile();
	int id = IDC_LOG_SESSION_FILE;
	
	HWND hwndField = GetDlgItem(hwndDlg, id);

	// Get current value
	int buflen = GetWindowTextLength(hwndField) + 1;

	// allocate buffer
	LPTSTR buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
	ZeroMemory(buf, buflen * sizeof(TCHAR));

	// Get current value
	GetWindowText(hwndField, buf, buflen);

	BOOL changed = FALSE;
	if (lstrcmp(buf, originalValue.data()) != 0) {
		changed = TRUE;
	}

	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);
	} else if (changed) {
		profile->setLogSessionInputFile(CMString(buf));
	}

	free(buf);
}

static void CheckXferFile(HWND hwndDlg, BOOL save) {
	CMString originalValue = profile->logFileTransferTransactionsFile();
	int id = IDC_LOG_XFER_FILE;
	
	HWND hwndField = GetDlgItem(hwndDlg, id);

	// Get current value
	int buflen = GetWindowTextLength(hwndField) + 1;

	// allocate buffer
	LPTSTR buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
	ZeroMemory(buf, buflen * sizeof(TCHAR));

	// Get current value
	GetWindowText(hwndField, buf, buflen);

	BOOL changed = FALSE;
	if (lstrcmp(buf, originalValue.data()) != 0) {
		changed = TRUE;
	}

	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);
	} else if (changed) {
		profile->setLogFileTransferTransactionsFile(CMString(buf));
	}

	free(buf);
}


BOOL CALLBACK LoggingPageDlgProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {


	switch(uMsg) {
	case WM_INITDIALOG: 
		{
			// Load form values
			SetWindowText(
				GetDlgItem(hwndDlg, IDC_LOG_CONNECTIONS_FILE),
				profile->logConnectionsFile().data());
			SetWindowText(
				GetDlgItem(hwndDlg, IDC_LOG_DEBUG_FILE),
				profile->logDebugFile().data());
			SetWindowText(
				GetDlgItem(hwndDlg, IDC_LOG_PACKETS_FILE),
				profile->logKermitFileTransfersFile().data());
			SetWindowText(
				GetDlgItem(hwndDlg, IDC_LOG_SESSION_FILE),
				profile->logSessionInputFile().data());
			SetWindowText(
				GetDlgItem(hwndDlg, IDC_LOG_XFER_FILE),
				profile->logFileTransferTransactionsFile().data());


			CheckDlgButton(hwndDlg, IDC_LOG_CONNECTIONS, 
				profile->logConnections() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LOG_DEBUG, 
				profile->logDebug() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LOG_PACKETS, 
				profile->logKermitFileTransfers() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LOG_SESSION, 
				profile->logSessionInput() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LOG_XFER, 
				profile->logFileTransferTransactions() ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_LOG_CONNECTIONS_APPEND, 
				profile->logAppendConnections() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LOG_DEBUG_APPEND, 
				profile->logAppendDebug() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LOG_PACKETS_APPEND, 
				profile->logAppendKermitFileTransfers() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LOG_SESSION_APPEND, 
				profile->logAppendSessionInput() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LOG_XFER_APPEND, 
				profile->logAppendFileTransferTransactions() ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_LOG_XFER_BRIEF, 
				profile->logBriefFileTransferTransactions() ? BST_CHECKED : BST_UNCHECKED);


			switch (profile->logSessionInputMode()) {
			case ConnectionProfile::LSM_TEXT:
				CheckDlgButton(hwndDlg, IDC_LOG_SESSION_TXT, BST_CHECKED);
				break;
			case ConnectionProfile::LSM_BINARY:
				CheckDlgButton(hwndDlg, IDC_LOG_SESSION_BIN, BST_CHECKED);
				break;
			case ConnectionProfile::LSM_DEBUG:
				CheckDlgButton(hwndDlg, IDC_LOG_SESSION_DBG, BST_CHECKED);
				break;
			}			

			EnableDisableFields(hwndDlg);

			// Form is clean
			cleanForm();

			break;
		}
	case WM_COMMAND: {
			WORD wNotifyCode = HIWORD(wParam);
			WORD wID = LOWORD(wParam);

			switch(wID) {
			case IDC_LOG_CONNECTIONS:
				CheckConnections(hwndDlg, FALSE);
				EnableDisableFields(hwndDlg);
				break;
			case IDC_LOG_CONNECTIONS_FILE:
				CheckConnectionsFile(hwndDlg, FALSE);
				break;
			case IDC_LOG_CONNECTIONS_BROWSE:
				BrowseFile(hwndDlg, IDC_LOG_CONNECTIONS_FILE, TEXT("Save Connections Log"));
				break;
			case IDC_LOG_CONNECTIONS_APPEND:
				CheckConnectionsAppend(hwndDlg, FALSE);
				break;
			case IDC_LOG_DEBUG:
				CheckDebug(hwndDlg, FALSE);
				EnableDisableFields(hwndDlg);
				break;
			case IDC_LOG_DEBUG_FILE:
				CheckDebugFile(hwndDlg, FALSE);
				break;
			case IDC_LOG_DEBUG_BROWSE:
				BrowseFile(hwndDlg, IDC_LOG_DEBUG_FILE, TEXT("Save Debug Log"));
				break;
			case IDC_LOG_DEBUG_APPEND:
				CheckDebugAppend(hwndDlg, FALSE);
				break;
			case IDC_LOG_PACKETS:
				CheckPackets(hwndDlg, FALSE);
				EnableDisableFields(hwndDlg);
				break;
			case IDC_LOG_PACKETS_FILE:
				CheckPacketsFile(hwndDlg, FALSE);
				break;
			case IDC_LOG_PACKETS_BROWSE:
				BrowseFile(hwndDlg, IDC_LOG_PACKETS_FILE, TEXT("Save Packet Log"));
				break;
			case IDC_LOG_PACKETS_APPEND:
				CheckPacketsAppend(hwndDlg, FALSE);
				break;
			case IDC_LOG_SESSION:
				CheckSession(hwndDlg, FALSE);
				EnableDisableFields(hwndDlg);
				break;
			case IDC_LOG_SESSION_FILE:
				CheckSessionFile(hwndDlg, FALSE);
				break;
			case IDC_LOG_SESSION_BROWSE:
				BrowseFile(hwndDlg, IDC_LOG_SESSION_FILE, TEXT("Save Session Log"));
				break;
			case IDC_LOG_SESSION_TXT:
			case IDC_LOG_SESSION_BIN:
			case IDC_LOG_SESSION_DBG:
				CheckSessionMode(hwndDlg, FALSE);
				break;
			case IDC_LOG_SESSION_APPEND:
				CheckSessionAppend(hwndDlg, FALSE);
				break;
			case IDC_LOG_XFER:
				CheckXfer(hwndDlg, FALSE);
				EnableDisableFields(hwndDlg);
				break;
			case IDC_LOG_XFER_FILE:
				CheckXferFile(hwndDlg, FALSE);
				break;
			case IDC_LOG_XFER_BROWSE:
				BrowseFile(hwndDlg, IDC_LOG_XFER_FILE, TEXT("Save Transfer Log"));
				break;
			case IDC_LOG_XFER_BRIEF:
				CheckXferBrief(hwndDlg, FALSE);
				break;
			case IDC_LOG_XFER_APPEND:
				CheckXferAppend(hwndDlg, FALSE);
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
					CheckConnections(hwndDlg, TRUE);
					CheckConnectionsAppend(hwndDlg, TRUE);
					CheckDebug(hwndDlg, TRUE);
					CheckDebugAppend(hwndDlg, TRUE);
					CheckPackets(hwndDlg, TRUE);
					CheckPacketsAppend(hwndDlg, TRUE);
					CheckSession(hwndDlg, TRUE);
					CheckSessionAppend(hwndDlg, TRUE);
					CheckXfer(hwndDlg, TRUE);
					CheckXferBrief(hwndDlg, TRUE);
					CheckXferAppend(hwndDlg, TRUE);

					CheckSessionMode(hwndDlg, TRUE);

					CheckConnectionsFile(hwndDlg, TRUE);
					CheckDebugFile(hwndDlg, TRUE);
					CheckPacketsFile(hwndDlg, TRUE);
					CheckSessionFile(hwndDlg, TRUE);
					CheckXferFile(hwndDlg, TRUE);

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

UINT CALLBACK LoggingPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp) {
	switch (uMsg) {
	case PSPCB_CREATE:
		profile = (ConnectionProfile*)ppsp->lParam;
		return TRUE;
	case PSPCB_RELEASE:
		return 0;
	}
	return 0;
}
