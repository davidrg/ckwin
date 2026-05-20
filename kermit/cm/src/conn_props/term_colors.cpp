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
Drop-down lists
	IDC_COL_TERM_FG		Terminal foreground
	IDC_COL_TERM_BG		Terminal backgroudn
	IDC_COL_COMM_FG		Command foreground
	IDC_COL_COMM_BG		Command background
	IDC_COL_STAT_FG		Status line foreground
	IDC_COL_STAT_BG		Status line background
	IDC_COL_MOU_FG		Mouse selection foregroudn
	IDC_COL_MOU_BG		Mouse selection background
	IDC_COL_POP_FG		Popup help foreground
	IDC_COL_POP_BG		Popup help backgroudn
	IDC_COL_UL_FG		Underline simulation foreground
	IDC_COL_UL_BG		Underline simulation background

Buttons:
	IDC_TERM_COLOR_DEFAULTS
 */

typedef struct tagDialogFieldStatus {
	int id;
	BOOL dirty;
} DialogFieldStatus;

// Connection profile we're editing
static ConnectionProfile *profile;

static const int fieldCount = 11;

static DialogFieldStatus fieldStatus[] = {
	{ IDC_COL_TERM_FG,	FALSE	},
	{ IDC_COL_TERM_BG,	FALSE	},
	{ IDC_COL_COMM_FG,	FALSE	},
	{ IDC_COL_COMM_BG,	FALSE	},
	{ IDC_COL_STAT_BG,	FALSE	},
	{ IDC_COL_MOU_FG,	FALSE	},
	{ IDC_COL_MOU_BG,	FALSE	},
	{ IDC_COL_POP_FG,	FALSE	},
	{ IDC_COL_POP_BG,	FALSE	},
	{ IDC_COL_UL_FG,	FALSE	},
	{ IDC_COL_UL_BG,	FALSE	}
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


static void PopulateColorsDDL(HWND hwndDlg, int controlId) {

	const Term::ColorInfo *colors = Term::getColorInfos();

	HWND hwnd = GetDlgItem(hwndDlg, controlId);

	for (int i = 0; colors[i].color != Term::COLOR_INVALID; i++) {
		const Term::ColorInfo color = colors[i];

		int idx = SendMessage(
			hwnd,
			(UINT) CB_ADDSTRING,
			(WPARAM) 0,
			(LPARAM) color.name);

		SendMessage(
			hwnd, 
			(UINT)CB_SETITEMDATA,
			(WPARAM)idx,
			(LPARAM)color.color);
	}
}

static BOOL CheckColorChanged(HWND hwndDlg, int controlId, Term::Color existing, Term::Color *outNewValue) {
	HWND hwnd = GetDlgItem(hwndDlg, controlId);

	int idx = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

	Term::Color newValue = (Term::Color)SendMessage(hwnd, CB_GETITEMDATA, idx, 0);

	BOOL changed = newValue != existing;

	setDirty(controlId, changed);
	TerminalFieldChanged(hwndDlg);	
	
	if (outNewValue != NULL) {
		*outNewValue = newValue;
	}

	return changed;
}

static void SetColor(HWND hwndDlg, int controlId, Term::Color color) {
	Term::ColorInfo info = Term::getColorInfo(color);

	if (info.color != Term::COLOR_INVALID) {
		SendMessage(
			GetDlgItem(hwndDlg, controlId),
			CB_SELECTSTRING ,
			(WPARAM)0,
			(LPARAM)info.name);
	}
}

BOOL CALLBACK TermColorPageDlgProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch(uMsg) {
	case WM_INITDIALOG: 
		{
			// Populate the drop-down lists...
			PopulateColorsDDL(hwndDlg, IDC_COL_TERM_FG);
			PopulateColorsDDL(hwndDlg, IDC_COL_TERM_BG);
			PopulateColorsDDL(hwndDlg, IDC_COL_COMM_FG);
			PopulateColorsDDL(hwndDlg, IDC_COL_COMM_BG);
			PopulateColorsDDL(hwndDlg, IDC_COL_STAT_FG);
			PopulateColorsDDL(hwndDlg, IDC_COL_STAT_BG);
			PopulateColorsDDL(hwndDlg, IDC_COL_MOU_FG);
			PopulateColorsDDL(hwndDlg, IDC_COL_MOU_BG);
			PopulateColorsDDL(hwndDlg, IDC_COL_POP_FG);
			PopulateColorsDDL(hwndDlg, IDC_COL_POP_BG);
			PopulateColorsDDL(hwndDlg, IDC_COL_UL_FG);
			PopulateColorsDDL(hwndDlg, IDC_COL_UL_BG);

			// Set current values...
			SetColor(hwndDlg, IDC_COL_TERM_FG, profile->terminalForegroundColor());
			SetColor(hwndDlg, IDC_COL_TERM_BG, profile->terminalBackgroundColor());
			SetColor(hwndDlg, IDC_COL_COMM_FG, profile->commandForegroundColor());
			SetColor(hwndDlg, IDC_COL_COMM_BG, profile->commandBackgroundColor());
			SetColor(hwndDlg, IDC_COL_STAT_FG, profile->statusLineForegroundColor());
			SetColor(hwndDlg, IDC_COL_STAT_BG, profile->statusLineBackgroundColor());
			SetColor(hwndDlg, IDC_COL_MOU_FG, profile->mouseSelectionForegroundColor());
			SetColor(hwndDlg, IDC_COL_MOU_BG, profile->mouseSelectionBackgroundColor());
			SetColor(hwndDlg, IDC_COL_POP_FG, profile->popupHelpForegroundColor());
			SetColor(hwndDlg, IDC_COL_POP_BG, profile->popupHelpBackgroundColor());
			SetColor(hwndDlg, IDC_COL_UL_FG, profile->underlineSimulationForegroundColor());
			SetColor(hwndDlg, IDC_COL_UL_BG, profile->underlineSimulationBackgroundColor());
			

			// Form is clean
			cleanForm();

			break;
		}
	case WM_COMMAND: {
			WORD wNotifyCode = HIWORD(wParam);
			WORD wID = LOWORD(wParam);
			

			switch(wID) {
			case IDC_TERM_COLOR_DEFAULTS:
				{
					HWND hwndTermType = GetDlgItem(hwndDlg, IDC_TERM_TYPE);

					Term::TermType termType;

					if (hwndTermType != NULL) {
						// The user has been on the terminal tab and may have changed
						// the value. Get the current term type from there.
						int idx = SendMessage(hwndTermType, LB_GETCURSEL, 0, 0);

						Term::TermType existingType = profile->terminalType();

						termType = (Term::TermType)SendMessage(
								hwndTermType, LB_GETITEMDATA, idx, 0);
					} else {
						// Term type hasn't changed. Get it from the profile.
						termType = profile->terminalType();
					}

					const Term::TermSettings settings = Term::getTermSettings(termType);

					if (settings.type != Term::TT_INVALID) {
						SetColor(hwndDlg, IDC_COL_TERM_FG, settings.termFg);
						SetColor(hwndDlg, IDC_COL_TERM_BG, settings.termBg);
						SetColor(hwndDlg, IDC_COL_UL_FG, settings.underlineFg);
						SetColor(hwndDlg, IDC_COL_UL_BG, settings.underlineBg);
					} else {
						SetColor(hwndDlg, IDC_COL_TERM_FG, Term::COLOR_LIGHT_GRAY);
						SetColor(hwndDlg, IDC_COL_TERM_BG, Term::COLOR_BLUE);
						SetColor(hwndDlg, IDC_COL_UL_FG, settings.underlineFg);
						SetColor(hwndDlg, IDC_COL_UL_BG, settings.underlineBg);
					}

					SetColor(hwndDlg, IDC_COL_COMM_FG, Term::COLOR_LIGHT_GRAY);
					SetColor(hwndDlg, IDC_COL_COMM_BG, Term::COLOR_BLACK);
					SetColor(hwndDlg, IDC_COL_STAT_FG, Term::COLOR_LIGHT_GRAY);
					SetColor(hwndDlg, IDC_COL_STAT_BG, Term::COLOR_CYAN);
					SetColor(hwndDlg, IDC_COL_MOU_FG, Term::COLOR_BLACK);
					SetColor(hwndDlg, IDC_COL_MOU_BG, Term::COLOR_YELLOW);
					SetColor(hwndDlg, IDC_COL_POP_FG, Term::COLOR_LIGHT_GRAY);
					SetColor(hwndDlg, IDC_COL_POP_BG, Term::COLOR_CYAN);

					CheckColorChanged(hwndDlg, IDC_COL_TERM_FG, profile->terminalForegroundColor(), NULL);
					CheckColorChanged(hwndDlg, IDC_COL_TERM_BG, profile->terminalBackgroundColor(), NULL);
					CheckColorChanged(hwndDlg, IDC_COL_COMM_FG, profile->commandForegroundColor(), NULL);
					CheckColorChanged(hwndDlg, IDC_COL_COMM_BG, profile->commandBackgroundColor(), NULL);
					CheckColorChanged(hwndDlg, IDC_COL_STAT_FG, profile->statusLineForegroundColor(), NULL);
					CheckColorChanged(hwndDlg, IDC_COL_STAT_BG, profile->statusLineBackgroundColor(), NULL);
					CheckColorChanged(hwndDlg, IDC_COL_MOU_FG, profile->mouseSelectionForegroundColor(), NULL);
					CheckColorChanged(hwndDlg, IDC_COL_MOU_BG, profile->mouseSelectionBackgroundColor(), NULL);
					CheckColorChanged(hwndDlg, IDC_COL_POP_FG, profile->popupHelpForegroundColor(), NULL);
					CheckColorChanged(hwndDlg, IDC_COL_POP_BG, profile->popupHelpBackgroundColor(), NULL);
					CheckColorChanged(hwndDlg, IDC_COL_UL_FG, profile->underlineSimulationForegroundColor(), NULL);
					CheckColorChanged(hwndDlg, IDC_COL_UL_BG, profile->underlineSimulationBackgroundColor(), NULL);
				}
				break;
			case IDC_COL_TERM_FG: 
				CheckColorChanged(hwndDlg, IDC_COL_TERM_FG, profile->terminalForegroundColor(), NULL);
				break;
			case IDC_COL_TERM_BG:
				CheckColorChanged(hwndDlg, IDC_COL_TERM_BG, profile->terminalBackgroundColor(), NULL);
				break;
			case IDC_COL_COMM_FG:
				CheckColorChanged(hwndDlg, IDC_COL_COMM_FG, profile->commandForegroundColor(), NULL);
				break;
			case IDC_COL_COMM_BG:
				CheckColorChanged(hwndDlg, IDC_COL_COMM_BG, profile->commandBackgroundColor(), NULL);
				break;
			case IDC_COL_STAT_FG:
				CheckColorChanged(hwndDlg, IDC_COL_STAT_FG, profile->statusLineForegroundColor(), NULL);
				break;
			case IDC_COL_STAT_BG:
				CheckColorChanged(hwndDlg, IDC_COL_STAT_BG, profile->statusLineBackgroundColor(), NULL);
				break;
			case IDC_COL_MOU_FG:
				CheckColorChanged(hwndDlg, IDC_COL_MOU_FG, profile->mouseSelectionForegroundColor(), NULL);
				break;
			case IDC_COL_MOU_BG:
				CheckColorChanged(hwndDlg, IDC_COL_MOU_BG, profile->mouseSelectionBackgroundColor(), NULL);
				break;
			case IDC_COL_POP_FG:
				CheckColorChanged(hwndDlg, IDC_COL_POP_FG, profile->popupHelpForegroundColor(), NULL);
				break;
			case IDC_COL_POP_BG:
				CheckColorChanged(hwndDlg, IDC_COL_POP_BG, profile->popupHelpBackgroundColor(), NULL);
				break;
			case IDC_COL_UL_FG:
				CheckColorChanged(hwndDlg, IDC_COL_UL_FG, profile->underlineSimulationForegroundColor(), NULL);
				break;
			case IDC_COL_UL_BG:
				CheckColorChanged(hwndDlg, IDC_COL_UL_BG, profile->underlineSimulationBackgroundColor(), NULL);
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
					Term::Color newValue;
					BOOL changed;

					changed = CheckColorChanged(hwndDlg, IDC_COL_TERM_FG, profile->terminalForegroundColor(), &newValue);
					if (changed) profile->setTerminalForegroundColor(newValue);

					changed = CheckColorChanged(hwndDlg, IDC_COL_TERM_BG, profile->terminalBackgroundColor(), &newValue);
					if (changed) profile->setTerminalBackgroundColor(newValue);

					changed = CheckColorChanged(hwndDlg, IDC_COL_COMM_FG, profile->commandForegroundColor(), &newValue);
					if (changed) profile->setCommandForegroundColor(newValue);

					changed = CheckColorChanged(hwndDlg, IDC_COL_COMM_BG, profile->commandBackgroundColor(), &newValue);
					if (changed) profile->setCommandBackgroundColor(newValue);

					changed = CheckColorChanged(hwndDlg, IDC_COL_STAT_FG, profile->statusLineForegroundColor(), &newValue);
					if (changed) profile->setStatusLineForegroundColor(newValue);

					changed = CheckColorChanged(hwndDlg, IDC_COL_STAT_BG, profile->statusLineBackgroundColor(), &newValue);
					if (changed) profile->setStatusLineBackgroundColor(newValue);

					changed = CheckColorChanged(hwndDlg, IDC_COL_MOU_FG, profile->mouseSelectionForegroundColor(), &newValue);
					if (changed) profile->setMouseSelectionForegroundColor(newValue);

					changed = CheckColorChanged(hwndDlg, IDC_COL_MOU_BG, profile->mouseSelectionBackgroundColor(), &newValue);
					if (changed) profile->setMouseSelectionBackgroundColor(newValue);

					changed = CheckColorChanged(hwndDlg, IDC_COL_POP_FG, profile->popupHelpForegroundColor(), &newValue);
					if (changed) profile->setPopupHelpForegroundColor(newValue);

					changed = CheckColorChanged(hwndDlg, IDC_COL_POP_BG, profile->popupHelpBackgroundColor(), &newValue);
					if (changed) profile->setPopupHelpBackgroundColor(newValue);

					changed = CheckColorChanged(hwndDlg, IDC_COL_UL_FG, profile->underlineSimulationForegroundColor(), &newValue);
					if (changed) profile->setUnderlineSimulationForegroundColor(newValue);

					changed = CheckColorChanged(hwndDlg, IDC_COL_UL_BG, profile->underlineSimulationBackgroundColor(), &newValue);
					if (changed) profile->setUnderlineSimulationBackgroundColor(newValue);

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

UINT CALLBACK TermColorPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp) {
	switch (uMsg) {
	case PSPCB_CREATE:
		profile = (ConnectionProfile*)ppsp->lParam;
		return TRUE;
	case PSPCB_RELEASE:
		return 0;
	}
	return 0;
}