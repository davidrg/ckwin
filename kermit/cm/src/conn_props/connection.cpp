#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include "../conn_props.h"
#include "../conn_profile.h"
#include "../resource.h"
#include "../term_info.h"
#include "../charset.h"


/*
  All types:
    Exit on disconnect	Checkbox		IDC_CONN_EXIT_ON_DISCON

  SSH:			--> IDD_CONNECTION_BASIC
  FTP:
	Hostname			Line edit		IDC_CONN_HOSTNAME	
	Port				Number edit		IDC_CONN_PORT

  IP:			--> IDD_CONNECTION_IP
	Hostname			Line edit		IDC_CONN_HOSTNAME
	Protocol			Drop-down list	IDC_CONN_PROTO
	Port				Number edit		IDC_CONN_PORT

  Named Pipe:	--> IDD_CONNECTION_CMD
	Hostname			Line edit		IDC_CONN_PARAM_A	IDC_CONN_PARAM_A_LBL
	Pipe name			Line edit		IDC_CONN_PARAM_B	IDC_CONN_PARAM_B_LBL

  DLL:			--> IDD_CONNECTION_CMD
	DLL Name			Line edit		IDC_CONN_PARAM_A	IDC_CONN_PARAM_A_LBL
	Parametrs			Line edit		IDC_CONN_PARAM_B	IDC_CONN_PARAM_B_LBL

  PTY:			--> IDD_CONNECTION_CMD
  Pipe:
	Command				Line edit		IDC_CONN_PARAM_A	IDC_CONN_PARAM_A_LBL

  LAT:			--> IDD_CONNECTION_CMD
	Node/service name	Line edit		IDC_CONN_PARAM_A	IDC_CONN_PARAM_A_LBL

  CTERM:		--> IDD_CONNECTION_CMD
	Node name			Line edit		IDC_CONN_PARAM_A	IDC_CONN_PARAM_A_LBL

 */

typedef struct tagIpProto {
	ConnectionProfile::IPProtocol proto;
	LPCTSTR name;
	int port;
} IpProto;

static const int protoCount = 14;
static const IpProto ipProtocols[] = {
	// Protocol								Name								Port			Assigned?
	{	ConnectionProfile::IPP_DEFAULT,		TEXT("default"),						23		},	// official
	{	ConnectionProfile::IPP_TELNET_NEGO,	TEXT("telnet"),							23		},	// official
	{	ConnectionProfile::IPP_TELNET,		TEXT("telnet without negotiations"),	23		},	// official
	{	ConnectionProfile::IPP_IKS,			TEXT("Internet Kermit Service (iks)"),	1649	},	// official
	{	ConnectionProfile::IPP_RLOGIN,		TEXT("rlogin"),							513		},	// official
	{	ConnectionProfile::IPP_RAW,			TEXT("raw socket"),						80		},
	{	ConnectionProfile::IPP_EK4LOGIN,	TEXT("ek4login"),						2105	},
	{	ConnectionProfile::IPP_EK5LOGIN,	TEXT("ek5login"),						2105	},
	{	ConnectionProfile::IPP_K4LOGIN,		TEXT("k4login"),						543		},	// official
	{	ConnectionProfile::IPP_K5LOGIN,		TEXT("k5login"),						543		},	// official
	{	ConnectionProfile::IPP_TELNET_SSL,	TEXT("Telnet over SSL"),				992		},	// official
	{	ConnectionProfile::IPP_TELNET_TLS,	TEXT("Telnet over TLS"),				992		},	// official
	{	ConnectionProfile::IPP_SSL,			TEXT("Raw SSL socket"),					443		},
	{	ConnectionProfile::IPP_TLS,			TEXT("Raw TLS socket"),					443		}	,
};


typedef struct tagDialogFieldStatus {
	int id;
	BOOL dirty;
} DialogFieldStatus;

// Connection profile we're editing
static ConnectionProfile *profile;

static const int fieldCount = 6;

static DialogFieldStatus fieldStatus[] = {
	{ IDC_CONN_EXIT_ON_DISCON,	FALSE	},		// All
	{ IDC_CONN_HOSTNAME,		FALSE	},		// SSH, FTP, IP
	{ IDC_CONN_PORT,			FALSE	},		// SSH, FTP, IP
	{ IDC_CONN_PROTO,			FALSE	},		// IP
	{ IDC_CONN_PARAM_A,			FALSE	},		// Named Pipe, DLL, PTY, Pipe, LAT, CTERM
	{ IDC_CONN_PARAM_B,			FALSE	}		// Named Pipe, DLL
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

BOOL CALLBACK ConnectionPageDlgProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch(uMsg) {
	case WM_INITDIALOG: 
		{			
			ConnectionProfile::ConnectionType conType = profile->connectionType();

			CheckDlgButton(hwndDlg, IDC_CONN_EXIT_ON_DISCON, 
				profile->exitOnDisconnect() ? BST_CHECKED : BST_UNCHECKED);

			switch (conType) {
			case ConnectionProfile::CT_FTP:
			case ConnectionProfile::CT_SSH:
			case ConnectionProfile::CT_IP:
				{
					// Hostname + Port
					HWND hwndHost = GetDlgItem(hwndDlg, IDC_CONN_HOSTNAME);
					HWND hwndPort = GetDlgItem(hwndDlg, IDC_CONN_PORT);

					CMString portStr = CMString::number(profile->port());

					SetWindowText(hwndHost, profile->hostname().data());
					SetWindowText(hwndPort, portStr.data());

					// And for IP, we have the proto drop-down
					if (conType == ConnectionProfile::CT_IP) {
						HWND hwndProto = GetDlgItem(hwndDlg, IDC_CONN_PROTO);


						ConnectionProfile::IPProtocol currentSelection = 
							profile->ipProtocol();

						// Populate the dropdown list
						for (int i = 0; i < protoCount; i++) {
							IpProto proto = ipProtocols[i];

							int idx = SendMessage(
								hwndProto,
								(UINT) CB_ADDSTRING,
								(WPARAM) 0,
								(LPARAM) proto.name);

							SendMessage(
								hwndProto, 
								(UINT)CB_SETITEMDATA,
								(WPARAM)idx,
								(LPARAM)proto.proto);

							// Set its initial value
							if (proto.proto == currentSelection) {
								SendMessage(
									hwndProto,
									CB_SELECTSTRING ,
									(WPARAM)0,
									(LPARAM)proto.name);
							}	
						}
					}
				}
				break;
			case ConnectionProfile::CT_NAMED_PIPE:
				{
					HWND hwndFieldA = GetDlgItem(hwndDlg, IDC_CONN_PARAM_A);
					HWND hwndFieldALabel = GetDlgItem(hwndDlg, IDC_CONN_PARAM_A_LBL);
					HWND hwndFieldB = GetDlgItem(hwndDlg, IDC_CONN_PARAM_B);
					HWND hwndFieldBLabel = GetDlgItem(hwndDlg, IDC_CONN_PARAM_B_LBL);

					// Field A: Hostname
					SetWindowText(hwndFieldA, profile->namedPipeHost().data());
					SetWindowText(hwndFieldALabel, TEXT("Hostname:"));

					// Field B: Pipe name
					SetWindowText(hwndFieldB, profile->namedPipeName().data());
					SetWindowText(hwndFieldBLabel, TEXT("Pipe name:"));
				}
				break;
			case ConnectionProfile::CT_DLL:
				{
					HWND hwndFieldA = GetDlgItem(hwndDlg, IDC_CONN_PARAM_A);
					HWND hwndFieldALabel = GetDlgItem(hwndDlg, IDC_CONN_PARAM_A_LBL);
					HWND hwndFieldB = GetDlgItem(hwndDlg, IDC_CONN_PARAM_B);
					HWND hwndFieldBLabel = GetDlgItem(hwndDlg, IDC_CONN_PARAM_B_LBL);

					// Field A: DLL filename
					SetWindowText(hwndFieldA, profile->dllName().data());
					SetWindowText(hwndFieldALabel, TEXT("DLL Filename:"));

					// Field B: Parameters
					SetWindowText(hwndFieldB, profile->dllParameters().data());
					SetWindowText(hwndFieldBLabel, TEXT("Parameters:"));
				}
				break;
			case ConnectionProfile::CT_PTY:
			case ConnectionProfile::CT_PIPE:
			case ConnectionProfile::CT_LAT:
			case ConnectionProfile::CT_CTERM:
				{
					HWND hwndFieldA = GetDlgItem(hwndDlg, IDC_CONN_PARAM_A);
					HWND hwndFieldALabel = GetDlgItem(hwndDlg, IDC_CONN_PARAM_A_LBL);
					HWND hwndFieldB = GetDlgItem(hwndDlg, IDC_CONN_PARAM_B);
					HWND hwndFieldBLabel = GetDlgItem(hwndDlg, IDC_CONN_PARAM_B_LBL);

					// Field A: Command or Node/service name or Node name
					switch(conType) {
					case ConnectionProfile::CT_PTY:
					case ConnectionProfile::CT_PIPE:
						{
							// Field A: Command
							SetWindowText(hwndFieldA, profile->ptyCommand().data());
							SetWindowText(hwndFieldALabel, TEXT("Command:"));
						}
						break;
					case ConnectionProfile::CT_LAT:
						{
							// Field A: Node/service name
							SetWindowText(hwndFieldA, profile->latService().data());
							SetWindowText(hwndFieldALabel, TEXT("Node/service Name:"));
						}
						break;
					case ConnectionProfile::CT_CTERM:
						{
							// Field A: Node name
							SetWindowText(hwndFieldA, profile->latService().data());
							SetWindowText(hwndFieldALabel, TEXT("Node Name:"));
						}
					default:
						break;
					}

					// Field B: (hidden)
					ShowWindow(hwndFieldB, SW_HIDE);
					ShowWindow(hwndFieldBLabel, SW_HIDE);
				}
				break;
			}
			

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
			case IDC_CONN_HOSTNAME:
				{
					setDirty(wID,
						textFieldChanged(
							GetDlgItem(hwndDlg,wID),
							profile->hostname().data()));
					FieldChanged(hwndDlg);
				}
				break;

			case IDC_CONN_PORT:
				{
					int port = getFieldInt(hwndDlg, wID);
					setDirty(wID, port != profile->port());
					FieldChanged(hwndDlg);
				}
				break;
				
			case IDC_CONN_PROTO:
				{
					HWND hwnd = GetDlgItem(hwndDlg, wID);
					int idx = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

					ConnectionProfile::IPProtocol existing = profile->ipProtocol();
					ConnectionProfile::IPProtocol newValue = 
						(ConnectionProfile::IPProtocol)SendMessage(
							hwnd, CB_GETITEMDATA, idx, 0);

					BOOL changed = newValue != existing;

					setDirty(wID, changed);
					
					for (int i = 0; i < protoCount; i++) {
						if (ipProtocols[i] .proto == newValue) {
							int port = ipProtocols[i].port;
							CMString strPort = CMString::number(port);
							
							SetWindowText(GetDlgItem(hwndDlg, IDC_CONN_PORT),
								strPort.data());

							setDirty(IDC_CONN_PORT,
								port != profile->port());

							break;
						}	
					}
					
					FieldChanged(hwndDlg);
				}
				break;
				
			case IDC_CONN_PARAM_A:
				{
					switch(profile->connectionType()) {
					case ConnectionProfile::CT_NAMED_PIPE:
						{
							// Named pipe host
							setDirty(wID,
								textFieldChanged(
									GetDlgItem(hwndDlg,wID),
									profile->namedPipeHost().data()));
							FieldChanged(hwndDlg);
						}
						break;
					case ConnectionProfile::CT_DLL:
						{
							// DLL filename
							setDirty(wID,
								textFieldChanged(
									GetDlgItem(hwndDlg,wID),
									profile->dllName().data()));
							FieldChanged(hwndDlg);
						}
						break;
					case ConnectionProfile::CT_PTY:
					case ConnectionProfile::CT_PIPE:
						{
							// Pty Command
							setDirty(wID,
								textFieldChanged(
									GetDlgItem(hwndDlg,wID),
									profile->ptyCommand().data()));
							FieldChanged(hwndDlg);
						}
						break;
					case ConnectionProfile::CT_LAT:
					case ConnectionProfile::CT_CTERM:
						{
							// CTERM or LAT node/service name
							setDirty(wID,
								textFieldChanged(
									GetDlgItem(hwndDlg,wID),
									profile->latService().data()));
							FieldChanged(hwndDlg);
						}
					}
				}
				// Dirty field
				break;
				
			case IDC_CONN_PARAM_B:
				{
					switch(profile->connectionType()) {
					case ConnectionProfile::CT_NAMED_PIPE:
						{
							// pipe name
							setDirty(wID,
								textFieldChanged(
									GetDlgItem(hwndDlg,wID),
									profile->namedPipeName().data()));
							FieldChanged(hwndDlg);
						}
						break;
					case ConnectionProfile::CT_DLL:
						{
							// DLL parameters
							setDirty(wID,
								textFieldChanged(
									GetDlgItem(hwndDlg,wID),
									profile->dllParameters().data()));
							FieldChanged(hwndDlg);
						}
						break;
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
					if (isDirty(IDC_CONN_EXIT_ON_DISCON)) {
						profile->setExitOnDisconnect(
							IsDlgButtonChecked(
								hwndDlg, IDC_CONN_EXIT_ON_DISCON) == BST_CHECKED);
					}

					if (isDirty(IDC_CONN_HOSTNAME)) {
						LPTSTR buf = getFieldText(hwndDlg, IDC_CONN_HOSTNAME);
						
						profile->setHostname(buf);

						free(buf);
					}

					if (isDirty(IDC_CONN_PORT)) {						
						profile->setPort(getFieldInt(hwndDlg, IDC_CONN_PORT));
					}

					if (isDirty(IDC_CONN_PROTO)) {

						HWND hwnd = GetDlgItem(hwndDlg, IDC_CONN_PROTO);
						int idx = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

						ConnectionProfile::IPProtocol newValue = 
							(ConnectionProfile::IPProtocol)SendMessage(
								hwnd, CB_GETITEMDATA, idx, 0);

						profile->setIpProtocol(newValue);
					}

					if (isDirty(IDC_CONN_PARAM_A)) {
						LPTSTR buf = getFieldText(hwndDlg, IDC_CONN_PARAM_A);
						
						
						switch(profile->connectionType()) {
						case ConnectionProfile::CT_NAMED_PIPE:
							profile->setNamedPipeHost(buf);
							break;
						case ConnectionProfile::CT_DLL:
							profile->setDllName(buf);
							break;
						case ConnectionProfile::CT_PTY:
						case ConnectionProfile::CT_PIPE:
							profile->setPtyCommand(buf);
							break;
						case ConnectionProfile::CT_LAT:
						case ConnectionProfile::CT_CTERM:
							profile->setLatService(buf);
						}

						free(buf);
					}

					
					if (isDirty(IDC_CONN_PARAM_B)) {
						LPTSTR buf = getFieldText(hwndDlg, IDC_CONN_PARAM_B);
						
						switch(profile->connectionType()) {
						case ConnectionProfile::CT_NAMED_PIPE:
							profile->setNamedPipeName(buf);
							break;
						case ConnectionProfile::CT_DLL:
							profile->setDllParameters(buf);
							break;
						}

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

UINT CALLBACK ConnectionPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp) {
	switch (uMsg) {
	case PSPCB_CREATE:
		profile = (ConnectionProfile*)ppsp->lParam;
		return TRUE;
	case PSPCB_RELEASE:
		return 0;
	}
	return 0;
}