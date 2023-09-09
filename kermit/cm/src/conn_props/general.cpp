#include <windows.h>
#include <commctrl.h>

#include "../conn_props.h"
#include "../conn_profile.h"
#include "../resource.h"

/******************
 * SHEET: General
 ******************/

/*
Controls:
IDC_GENERAL_NAME			Line edit
IDC_GENERAL_DESCRIPTION		Line edit
IDC_GENERAL_NOTES			Multiline Edit
 */

// Connection profile we're editing
static ConnectionProfile *profile;

// Handles for fields in the form
static HWND hwndName, hwndDescription, hwndNotes, hwndDownloadDir, hwndStartDir;

// For tracking fields that have changed
static BOOL dirtyName, dirtyDescription, dirtyNotes, dirtyDownloadDir, dirtyStartDir;



// Call whenever a fields value is changed to toggle the
// apply button on and off as necessary.
static void GeneralFieldChanged(HWND hwndDlg) {
	if (dirtyName || dirtyDescription || dirtyNotes || dirtyDownloadDir || dirtyStartDir) {
		PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
	} else {
		PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);
	}
}


BOOL CALLBACK GeneralPageDlgProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch(uMsg) {
	case WM_INITDIALOG: {
			hwndName = GetDlgItem(hwndDlg,IDC_GENERAL_NAME);
			hwndDescription = GetDlgItem(hwndDlg,IDC_GENERAL_DESCRIPTION);
			hwndNotes = GetDlgItem(hwndDlg,IDC_GENERAL_NOTES);
			hwndDownloadDir = GetDlgItem(hwndDlg, IDC_GENERAL_DOWNLOAD);
			hwndStartDir = GetDlgItem(hwndDlg, IDC_GENERAL_START);
			
			dirtyName = FALSE;
			dirtyDescription = FALSE;
			dirtyNotes = FALSE;
			dirtyDownloadDir = FALSE;
			dirtyStartDir = FALSE;

			SetWindowText(hwndName, profile->name().data());
			SetWindowText(hwndDescription, profile->description().data());
			SetWindowText(hwndNotes, profile->notes().data());
			SetWindowText(hwndDownloadDir, profile->downloadDirectory().data());
			SetWindowText(hwndStartDir, profile->startingDirectory().data());

			// unset the apply button
			//PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);
			
			break;
		}
	case WM_COMMAND: {
			WORD wNotifyCode = HIWORD(wParam);
			WORD wID = LOWORD(wParam);
			
			switch(wID) {
			case IDC_GENERAL_NAME:
				dirtyName = textFieldChanged(hwndName, profile->name().data());
				GeneralFieldChanged(hwndDlg);
				break;

			case IDC_GENERAL_DESCRIPTION:
				dirtyDescription = textFieldChanged(hwndDescription, profile->description().data());
				GeneralFieldChanged(hwndDlg);
				break;

			case IDC_GENERAL_NOTES: 
				dirtyNotes = textFieldChanged(hwndNotes, profile->notes().data());
				GeneralFieldChanged(hwndDlg);
				break;

			case IDC_GENERAL_DOWNLOAD:
				dirtyDownloadDir = textFieldChanged(hwndDownloadDir, profile->downloadDirectory().data());
				GeneralFieldChanged(hwndDlg);
				break;

			case IDC_GENERAL_START:
				dirtyStartDir = textFieldChanged(hwndStartDir, profile->startingDirectory().data());
				GeneralFieldChanged(hwndDlg);
				break;

			}
			return TRUE;
			break;
		}
	case WM_NOTIFY: {
			LPNMHDR pnmh = (LPNMHDR)lParam;
			switch (pnmh->code) {
			case PSN_APPLY:	{		// Apply or OK button clicked
				int buflen = 0, temp=0;
				LPTSTR buf = NULL;

				// Figure out the buffer size we need.
				buflen = GetWindowTextLength(hwndName);
				temp = GetWindowTextLength(hwndDescription);
				buflen = max(buflen, temp);
				temp = GetWindowTextLength(hwndNotes);
				buflen = max(buflen, temp);
				temp = GetWindowTextLength(hwndDownloadDir);
				buflen = max(buflen, temp);
				temp = GetWindowTextLength(hwndStartDir);
				buflen = max(buflen, temp);

				// The sizes reported don't include space for
				// null termination.
				buflen += 1;

				buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
				ZeroMemory(buf, buflen * sizeof(TCHAR));

				// Now we have a buffer large enough for any string we
				// encounter, go save any changes.
				GetWindowText(hwndName, buf, buflen);
				profile->setName(buf);

				GetWindowText(hwndDescription, buf, buflen);
				profile->setDescription(buf);

				GetWindowText(hwndNotes, buf, buflen);
				profile->setNotes(buf);

				GetWindowText(hwndDownloadDir, buf, buflen);
				profile->setDownloadDirectory(buf);

				GetWindowText(hwndStartDir, buf, buflen);
				profile->setStartingDirectory(buf);
				
				
				free(buf);
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

UINT CALLBACK GeneralPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp) {
	switch (uMsg) {
	case PSPCB_CREATE:
		profile = (ConnectionProfile*)ppsp->lParam;
		return TRUE;
	case PSPCB_RELEASE:
		return 0;
	}
	return 0;
}