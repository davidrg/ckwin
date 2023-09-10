#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include "../conn_props.h"
#include "../conn_profile.h"
#include "../resource.h"
#include "../util.h"

/*
  Serial		--> IDD_CONNECTION_SERIAL
	Port				Combo box		IDC_CONN_LINE
	Speed				Drop-down list	IDC_CONN_SPEED
	Flow Control		Drop-down list	IDC_CONN_FLOW	(four opts)
	Parity				Drop-down list	IDC_CONN_PARITY
	Stop Bits			Drop-down list	IDC_CONN_STOP	(two opts? three opts?)
	Error Correction	Checkbox		IDC_CONN_ERROR_CORRECTION
	Data Compression	Checkbox		IDC_CONN_DATA_COMPRESSION
	Carrier Detection	Checkbox		IDC_CONN_CARRIER_DETECT
*/


// Valid settings from here: 
// https://learn.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-commprop
static const unsigned int serialSpeeds[] = {
	   110,    300,   600,  1200,  2400, 4800, 
 	  9600,  14400, 19200, 38400, 57600, 11520, 
	128000, 256000, 0
};


typedef struct tagFlowControlOption {
	ConnectionProfile::FlowControl flow;
	LPCTSTR name;
} FlowControlOption;

static const int flowControlOptionCount = 4;
static const FlowControlOption flowControlOptions[] = {
	{	ConnectionProfile::FC_AUTO,			TEXT("Auto")		},
	{	ConnectionProfile::FC_NONE,			TEXT("None")		},
	{	ConnectionProfile::FC_RTS_CTS,		TEXT("Rts/Cts")		},
	{	ConnectionProfile::FC_XON_XOFF,		TEXT("Xon/Xoff")	}
};

typedef struct tagParityOption {
	ConnectionProfile::Parity parity;
	LPCTSTR name;
} ParityOption;

static const int parityOptionCount = 9;
static const ParityOption parityOptions[] = {
	{	ConnectionProfile::PAR_NONE,		TEXT("None")		},
	{	ConnectionProfile::PAR_EVEN,		TEXT("Even")		},
	{	ConnectionProfile::PAR_EVEN_8BIT,	TEXT("Even 8bits")	},
	{	ConnectionProfile::PAR_MARK,		TEXT("Mark")		},
	{	ConnectionProfile::PAR_MARK_8BIT,	TEXT("Mark 8bits")	},
	{	ConnectionProfile::PAR_ODD,			TEXT("Odd")			},
	{	ConnectionProfile::PAR_ODD_8BIT,	TEXT("Odd 8bits")	},
	{	ConnectionProfile::PAR_SPACE,		TEXT("Space")		},
	{	ConnectionProfile::PAR_SPACE_8BIT,	TEXT("Space 8bits")	}
};

typedef struct tagStopBitsOption {
	ConnectionProfile::StopBits bits;
	LPCTSTR name;
} StopBitsOption;

static const int stopBitsOptionCount = 2;
static const StopBitsOption stopBitsOptions[] = {
	{	ConnectionProfile::SB_1_0,		TEXT("1.0")		},
	//{	ConnectionProfile::SB_1_5,		TEXT("1.5")		},	// Not output by the K95 dialer or supported by CKWIN ?
	{	ConnectionProfile::SB_2_0,		TEXT("2.0")		}
};


typedef struct tagDialogFieldStatus {
	int id;
	BOOL dirty;
} DialogFieldStatus;

// Connection profile we're editing
static ConnectionProfile *profile;

static const int fieldCount = 9;

static DialogFieldStatus fieldStatus[] = {
	{ IDC_CONN_LINE,				FALSE	},
	{ IDC_CONN_SPEED,				FALSE	},
	{ IDC_CONN_FLOW,				FALSE	},
	{ IDC_CONN_PARITY,				FALSE	},
	{ IDC_CONN_STOP,				FALSE	},
	{ IDC_CONN_ERROR_CORRECTION,	FALSE	},
	{ IDC_CONN_DATA_COMPRESSION,	FALSE	},
	{ IDC_CONN_CARRIER_DETECT,		FALSE	},
	{ IDC_CONN_EXIT_ON_DISCON,		FALSE	}

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



BOOL CALLBACK SerialConnectionPageDlgProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch(uMsg) {
	case WM_INITDIALOG: 
		{	
			int i;

			loadSerialPortDropdown(hwndDlg, IDC_CONN_LINE);
			SetWindowText(GetDlgItem(hwndDlg, IDC_CONN_LINE), profile->line().data());
			
			// Populate line speeds
			HWND hwndLineSpeed = GetDlgItem(hwndDlg, IDC_CONN_SPEED);
			for (i = 0; serialSpeeds[i] != 0; i++) {
				unsigned int speed = serialSpeeds[i];

				CMString strSpeed = CMString::number(speed);

				
				int idx = SendMessage(
					hwndLineSpeed,
					(UINT) CB_ADDSTRING,
					(WPARAM) 0,
					(LPARAM) strSpeed.data());

				SendMessage(
					hwndLineSpeed, 
					(UINT)CB_SETITEMDATA,
					(WPARAM)idx,
					(LPARAM)speed);

				// Set its initial value
				if (speed == profile->lineSpeed()) {
					SendMessage(
						hwndLineSpeed,
						CB_SELECTSTRING ,
						(WPARAM)0,
						(LPARAM)strSpeed.data());
				}	
			}

			// Populate flow controls
			HWND hwndFlowControl = GetDlgItem(hwndDlg, IDC_CONN_FLOW);
			for ( i = 0; i < flowControlOptionCount; i++) {
				
				const FlowControlOption fco = flowControlOptions[i];
				
				int idx = SendMessage(
					hwndFlowControl,
					(UINT) CB_ADDSTRING,
					(WPARAM) 0,
					(LPARAM) fco.name);

				SendMessage(
					hwndFlowControl, 
					(UINT)CB_SETITEMDATA,
					(WPARAM)idx,
					(LPARAM)fco.flow);

				// Set its initial value
				if (fco.flow == profile->flowControl()) {
					SendMessage(
						hwndFlowControl,
						CB_SELECTSTRING ,
						(WPARAM)0,
						(LPARAM)fco.name);
				}	
			}

			// Populate paritys
			HWND hwndParity = GetDlgItem(hwndDlg, IDC_CONN_PARITY);
			for (i = 0; i < parityOptionCount; i++) {
				
				const ParityOption par = parityOptions[i];
				
				int idx = SendMessage(
					hwndParity,
					(UINT) CB_ADDSTRING,
					(WPARAM) 0,
					(LPARAM) par.name);

				SendMessage(
					hwndParity, 
					(UINT)CB_SETITEMDATA,
					(WPARAM)idx,
					(LPARAM)par.parity);

				// Set its initial value
				if (par.parity == profile->parity()) {
					SendMessage(
						hwndParity,
						CB_SELECTSTRING ,
						(WPARAM)0,
						(LPARAM)par.name);
				}	
			}

			// Populate stop bits
			HWND hwndStop = GetDlgItem(hwndDlg, IDC_CONN_STOP);
			for (i = 0; i < stopBitsOptionCount; i++) {
				
				const StopBitsOption stop = stopBitsOptions[i];
				
				int idx = SendMessage(
					hwndStop,
					(UINT) CB_ADDSTRING,
					(WPARAM) 0,
					(LPARAM) stop.name);

				SendMessage(
					hwndStop, 
					(UINT)CB_SETITEMDATA,
					(WPARAM)idx,
					(LPARAM)stop.bits);

				// Set its initial value
				if (stop.bits == profile->stopBits()) {
					SendMessage(
						hwndStop,
						CB_SELECTSTRING ,
						(WPARAM)0,
						(LPARAM)stop.name);
				}	
			}

			CheckDlgButton(hwndDlg, IDC_CONN_EXIT_ON_DISCON, 
				profile->exitOnDisconnect() ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_CONN_ERROR_CORRECTION, 
				profile->errorCorrection() ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_CONN_DATA_COMPRESSION, 
				profile->dataCompression() ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_CONN_CARRIER_DETECT, 
				profile->carrierDetection() ? BST_CHECKED : BST_UNCHECKED);
		

			// Form is clean
			cleanForm();

			break;
		}
	case WM_COMMAND: {
			WORD wNotifyCode = HIWORD(wParam);
			WORD wID = LOWORD(wParam);
			
			switch(wID) {
			case IDC_CONN_EXIT_ON_DISCON:
				{
					BOOL newValue = IsDlgButtonChecked(
						hwndDlg, wID) == BST_CHECKED;
				
					setDirty(wID, newValue != profile->exitOnDisconnect());

					FieldChanged(hwndDlg);
				}
				break;
			case IDC_CONN_ERROR_CORRECTION:
				{
					BOOL newValue = IsDlgButtonChecked(
						hwndDlg, wID) == BST_CHECKED;
				
					setDirty(wID, newValue != profile->errorCorrection());

					FieldChanged(hwndDlg);
				}
				break;
			case IDC_CONN_DATA_COMPRESSION:
				{
					BOOL newValue = IsDlgButtonChecked(
						hwndDlg, wID) == BST_CHECKED;
				
					setDirty(wID, newValue != profile->dataCompression());

					FieldChanged(hwndDlg);
				}
				break;
			case IDC_CONN_CARRIER_DETECT:
				{
					BOOL newValue = IsDlgButtonChecked(
						hwndDlg, wID) == BST_CHECKED;
				
					setDirty(wID, newValue != profile->carrierDetection());

					FieldChanged(hwndDlg);
				}
				break;
			case IDC_CONN_LINE:
				{
					BOOL changed  = textFieldChanged(
						GetDlgItem(hwndDlg, IDC_CONN_LINE),
						profile->line().data());

					setDirty(wID, changed);
					FieldChanged(hwndDlg);
				}
				break;
			case IDC_CONN_SPEED:
				{
					HWND hwnd = GetDlgItem(hwndDlg, IDC_CONN_SPEED);
					int idx = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
					unsigned int newValue = 
						(unsigned int)SendMessage(hwnd, CB_GETITEMDATA, idx, 0);

					setDirty(wID, newValue != profile->lineSpeed());
					FieldChanged(hwndDlg);
				}
				break;
			case IDC_CONN_FLOW:
				{
					HWND hwnd = GetDlgItem(hwndDlg, IDC_CONN_FLOW);
					int idx = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
					ConnectionProfile::FlowControl newValue = 
						(ConnectionProfile::FlowControl)SendMessage(
							hwnd, CB_GETITEMDATA, idx, 0);

					setDirty(wID, newValue != profile->flowControl());
					FieldChanged(hwndDlg);
				}
				break;
			case IDC_CONN_PARITY:
				{
					HWND hwnd = GetDlgItem(hwndDlg, wID);
					int idx = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
					ConnectionProfile::Parity newValue = 
						(ConnectionProfile::Parity)SendMessage(
							hwnd, CB_GETITEMDATA, idx, 0);

					setDirty(wID, newValue != profile->parity());
					FieldChanged(hwndDlg);
				}
				break;
			case IDC_CONN_STOP:
				{
					HWND hwnd = GetDlgItem(hwndDlg, wID);
					int idx = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
					ConnectionProfile::StopBits newValue = 
						(ConnectionProfile::StopBits)SendMessage(
							hwnd, CB_GETITEMDATA, idx, 0);

					setDirty(wID, newValue != profile->stopBits());
					FieldChanged(hwndDlg);
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
					if (isDirty(IDC_CONN_EXIT_ON_DISCON)) {
						profile->setExitOnDisconnect(
							IsDlgButtonChecked(hwndDlg, IDC_CONN_EXIT_ON_DISCON) == BST_CHECKED);
					}

					if (isDirty(IDC_CONN_ERROR_CORRECTION)) {
						profile->setErrorCorrection(
							IsDlgButtonChecked(hwndDlg, IDC_CONN_ERROR_CORRECTION) == BST_CHECKED);
					}

					if (isDirty(IDC_CONN_DATA_COMPRESSION)) {
						profile->setDataCompression(
							IsDlgButtonChecked(hwndDlg, IDC_CONN_DATA_COMPRESSION) == BST_CHECKED);
					}

					if (isDirty(IDC_CONN_CARRIER_DETECT)) {
						profile->setCarrierDetection(
							IsDlgButtonChecked(hwndDlg, IDC_CONN_CARRIER_DETECT) == BST_CHECKED);
					}

					if (isDirty(IDC_CONN_LINE)) {
						LPTSTR buf = getFieldText(hwndDlg, IDC_CONN_LINE);
						
						profile->setLine(buf);

						free(buf);
					}

					if (isDirty(IDC_CONN_SPEED)) {
						HWND hwnd = GetDlgItem(hwndDlg, IDC_CONN_SPEED);
						int idx = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
						unsigned int newValue = 
							(unsigned int)SendMessage(hwnd, CB_GETITEMDATA, idx, 0);
						profile->setLineSpeed(newValue);
					}

					if (isDirty(IDC_CONN_FLOW)) {
						HWND hwnd = GetDlgItem(hwndDlg, IDC_CONN_FLOW);
						int idx = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
						ConnectionProfile::FlowControl newValue = 
							(ConnectionProfile::FlowControl)SendMessage(
								hwnd, CB_GETITEMDATA, idx, 0);
						profile->setFlowControl(newValue);
					}

					if (isDirty(IDC_CONN_PARITY)) {
						HWND hwnd = GetDlgItem(hwndDlg, IDC_CONN_PARITY);
						int idx = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
						ConnectionProfile::Parity newValue = 
							(ConnectionProfile::Parity)SendMessage(
								hwnd, CB_GETITEMDATA, idx, 0);
						profile->setParity(newValue);
					}

					if (isDirty(IDC_CONN_STOP)) {
						HWND hwnd = GetDlgItem(hwndDlg, IDC_CONN_STOP);
						int idx = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
						ConnectionProfile::StopBits newValue = 
							(ConnectionProfile::StopBits)SendMessage(
								hwnd, CB_GETITEMDATA, idx, 0);
						profile->setStopBits(newValue);
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

UINT CALLBACK SerialConnectionPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp) {
	switch (uMsg) {
	case PSPCB_CREATE:
		profile = (ConnectionProfile*)ppsp->lParam;
		return TRUE;
	case PSPCB_RELEASE:
		return 0;
	}
	return 0;
}