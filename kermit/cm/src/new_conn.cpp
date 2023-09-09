
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>

#include "resource.h"
#include "config_file.h"
#include "conn_list.h"
#include "conn_props.h"

BOOL CALLBACK		NewConnectionDlgProc(HWND, UINT, WPARAM, LPARAM);

ConfigFile  *_configFile;
HINSTANCE _hInstance;


/* IP and other protocol Names */
#define CONOTHER_DEFAULT		TEXT("default")
#define CONOTHER_TELNET			TEXT("telnet with negotiations")
#define CONOTHER_TELNET_NNEGO	TEXT("telnet")
#define CONOTHER_IKS			TEXT("iks")
#define CONOTHER_RLOGIN			TEXT("rlogin")
#define CONOTHER_RAW			TEXT("raw socket")
#define CONOTHER_EK4LOGIN		TEXT("ek4login")
#define CONOTHER_EK5LOGIN		TEXT("ek5login")
#define CONOTHER_K4LOGIN		TEXT("k4login")
#define CONOTHER_K5LOGIN		TEXT("k5login")
#define CONOTHER_TELNET_SSL		TEXT("telnet over ssl")
#define CONOTHER_TELNET_TLS		TEXT("telnet over tls")
#define CONOTHER_SSL			TEXT("ssl")
#define CONOTHER_TLS			TEXT("tls")
#define CONOTHER_DLL			TEXT("Custom (DLL)")
#define CONOTHER_PIPE			TEXT("Pipe")
#define CONOTHER_LAT			TEXT("LAT (Pathworks/SLAT)")
#define CONOTHER_CTERM			TEXT("CTERM (Pathworks)")


void NewConnection(HINSTANCE hInstance, HWND hwndParent, 
				   ConfigFile *configFile) {
	_configFile = configFile;
	_hInstance = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_NEW_CONNECTION), 
		hwndParent, (DLGPROC)NewConnectionDlgProc);
}


BOOL CALLBACK NewConnectionDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			BOOL haveTLS = TRUE, haveKerberosIV = TRUE, haveKerberosV = TRUE,
				 haveLAT = TRUE, havePTY = TRUE, haveSSH = TRUE;
			HWND hwndIPP, hwndSerialSpeed, hwndSerialPort;

			hwndIPP = GetDlgItem(hDlg, IDC_NEW_CONN_OTHER);
			hwndSerialSpeed = GetDlgItem(hDlg, IDC_NEW_CONN_SPEED);
			hwndSerialPort = GetDlgItem(hDlg, IDC_NEW_CONN_SER_PORT);

			// Default to SSH
			SendMessage(GetDlgItem(hDlg, IDC_NEW_CONN_SSH), BM_SETCHECK, BST_CHECKED, 0);
			SetWindowText(GetDlgItem(hDlg, IDC_NEW_CONN_PORT), TEXT("22"));

			// Disable network options we don't have
			if (!haveSSH) EnableWindow(GetDlgItem(hDlg, IDC_NEW_CONN_SSH), FALSE);
			if (!havePTY) EnableWindow(GetDlgItem(hDlg, IDC_NEW_CONN_PTY), FALSE);

			// Populate the IP Protocols drop-down
			SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_DEFAULT);
			SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_TELNET);
			SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_TELNET_NNEGO);
			SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_IKS);
			SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_RLOGIN);
			SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_RAW);
			
			if (haveTLS) {
				SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_TELNET_SSL);
				SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_TELNET_TLS);
				SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_SSL);
				SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_TLS);
			}
			if (haveKerberosIV)
				SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_EK4LOGIN);
			if (haveKerberosV)
				SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_EK5LOGIN);
			if (haveKerberosIV)
				SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_K4LOGIN);
			if (haveKerberosV)
				SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_K5LOGIN);
			if (haveLAT) {
				SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_LAT);
				SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_CTERM);
			}

			SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_DLL);
			SendMessage(hwndIPP,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) CONOTHER_PIPE);

			// And select telnet as the default
			ComboBox_SelectString(hwndIPP, 0, CONOTHER_TELNET);

			// Populate the serial port speeds
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("110"));
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("300"));
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("600"));
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("1200"));
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("2400"));
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("4800"));
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("9600"));
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("14400"));
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("19200"));
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("38400"));
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("57600"));
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("11520"));
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("128000"));
			SendMessage(hwndSerialSpeed,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) TEXT("256000"));

			// And select 9600 as the default
			ComboBox_SelectString(hwndSerialSpeed, 0, TEXT("9600"));
			

			// Now try to populate the list of serial ports. This only works on NT.
#define MAX_DATA_LEN 50
			HKEY hKey = NULL;
			TCHAR tempData[MAX_DATA_LEN], tempName[MAX_DATA_LEN];
			DWORD tempDataLen = MAX_DATA_LEN, tempNameLen = MAX_DATA_LEN;
			DWORD dwIndex = 0;
			ZeroMemory(tempData, MAX_DATA_LEN);
			ZeroMemory(tempName, MAX_DATA_LEN);

			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, 
							 KEY_READ, &hKey) == ERROR_SUCCESS) {
				while (TRUE) {
					LONG rc = RegEnumValue(hKey, dwIndex, tempName, &tempNameLen,NULL, NULL, 
									(unsigned char*)tempData, &tempDataLen);

					if (rc == ERROR_NO_MORE_ITEMS) {
						break;
					} 

					SendMessage(hwndSerialPort,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) tempData);
					dwIndex++;
				}
			} else {
				// Couldn't find the registry key. Either no serial ports, or not on NT
				// (95 doesn't seem to provide this info via the registry)

				// We *could* brute-force it if we really cared - just try to CreateFile
				// COM1..COM10 and if it succeeds add it to the list.
			}

			return TRUE;
		}
	case WM_COMMAND:
		{
			HWND lblTarget, lblParameter, leHost, lePort, ddSerial, ddSpeed, ddIpProto;
			int cmd = LOWORD(wParam);

			lblTarget = GetDlgItem(hDlg, IDC_NEW_CONN_TARGET);
			lblParameter = GetDlgItem(hDlg, IDC_NEW_CONN_PARAMETER);
			leHost = GetDlgItem(hDlg, IDC_NEW_CONN_HOSTNAME);
			lePort = GetDlgItem(hDlg, IDC_NEW_CONN_PORT);
			ddSerial = GetDlgItem(hDlg, IDC_NEW_CONN_SER_PORT);
			ddSpeed = GetDlgItem(hDlg, IDC_NEW_CONN_SPEED);
			ddIpProto = GetDlgItem(hDlg, IDC_NEW_CONN_OTHER);

			// note: iks port is 1649, rlogin is 513, telnet is 23
			//       klogin is 543, eklogin is 2105 (unofficial)

			switch(cmd) {
			case IDOK:
				{
					LPTSTR name = NULL, host = NULL, port = NULL;
					int nameLen = 0, hostLen = 0, portLen = 0;
					ConnectionProfile::ConnectionType conType;
					ConnectionProfile::IPProtocol ipType = ConnectionProfile::IPP_DEFAULT;
					BOOL showProps = FALSE;
					BOOL ok = TRUE;

					showProps = IsDlgButtonChecked(hDlg, IDC_NEW_CONN_OPEN_PROPS) == BST_CHECKED;

					if (IsDlgButtonChecked(hDlg, IDC_NEW_CONN_SSH) == BST_CHECKED) {
						conType = ConnectionProfile::CT_SSH;
					} else if (IsDlgButtonChecked(hDlg, IDC_NEW_CONN_SERIAL) == BST_CHECKED) {
						conType = ConnectionProfile::CT_SERIAL;
					} else if (IsDlgButtonChecked(hDlg, IDC_NEW_CONN_FTP) == BST_CHECKED) {
						conType = ConnectionProfile::CT_FTP;
					} else if (IsDlgButtonChecked(hDlg, IDC_NEW_CONN_NAMED_PIPE) == BST_CHECKED) {
						conType = ConnectionProfile::CT_NAMED_PIPE;
					} else if (IsDlgButtonChecked(hDlg, IDC_NEW_CONN_PTY) == BST_CHECKED) {
						conType = ConnectionProfile::CT_PTY;
					} else if (IsDlgButtonChecked(hDlg, IDC_NEW_CONN_IP) == BST_CHECKED) {
						// Other.

						int buflen;
						LPTSTR buf = NULL;

						buflen = GetWindowTextLength(ddIpProto) + 1;
						buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
						ZeroMemory(buf, sizeof(TCHAR) * buflen);

						GetWindowText(ddIpProto, buf, buflen);

						// Assume IP
						conType = ConnectionProfile::CT_IP;

						
						if (lstrcmp(buf, CONOTHER_DEFAULT) == 0) {
							ipType = ConnectionProfile::IPP_DEFAULT;
						} else if (lstrcmp(buf, CONOTHER_TELNET) == 0) {
							ipType = ConnectionProfile::IPP_TELNET_NEGO;
						} else if (lstrcmp(buf, CONOTHER_TELNET_NNEGO) == 0) {
							ipType = ConnectionProfile::IPP_TELNET;
						} else if (lstrcmp(buf, CONOTHER_RLOGIN) == 0) {
							ipType = ConnectionProfile::IPP_RLOGIN;
						} else if (lstrcmp(buf, CONOTHER_IKS) == 0) {
							ipType = ConnectionProfile::IPP_IKS;
						} else if (lstrcmp(buf, CONOTHER_RAW) == 0) {
							ipType = ConnectionProfile::IPP_RAW;
						} else if (lstrcmp(buf, CONOTHER_EK4LOGIN) == 0) {
							ipType = ConnectionProfile::IPP_EK4LOGIN;
						} else if (lstrcmp(buf, CONOTHER_EK5LOGIN) == 0) {
							ipType = ConnectionProfile::IPP_EK5LOGIN;
						} else if (lstrcmp(buf, CONOTHER_K4LOGIN) == 0) {
							ipType = ConnectionProfile::IPP_K4LOGIN;
						} else if (lstrcmp(buf, CONOTHER_K5LOGIN) == 0) {
							ipType = ConnectionProfile::IPP_K5LOGIN;
						} else if (lstrcmp(buf, CONOTHER_TELNET_SSL) == 0) {
							ipType = ConnectionProfile::IPP_TELNET_SSL;
						} else if (lstrcmp(buf, CONOTHER_TELNET_TLS) == 0) {
							ipType = ConnectionProfile::IPP_TELNET_TLS;
						} else if (lstrcmp(buf, CONOTHER_SSL) == 0) {
							ipType = ConnectionProfile::IPP_SSL;
						} else if (lstrcmp(buf, CONOTHER_TLS) == 0) {
							ipType = ConnectionProfile::IPP_TLS;
						} else if (lstrcmp(buf, CONOTHER_DLL) == 0) {
							ipType = ConnectionProfile::IPP_DEFAULT;
							conType = ConnectionProfile::CT_DLL;
						} else if (lstrcmp(buf, CONOTHER_PIPE) == 0) {
							ipType = ConnectionProfile::IPP_DEFAULT;
							conType = ConnectionProfile::CT_PIPE;
						} else if (lstrcmp(buf, CONOTHER_LAT) == 0) {
							ipType = ConnectionProfile::IPP_DEFAULT;
							conType = ConnectionProfile::CT_LAT;
						} else if (lstrcmp(buf, CONOTHER_CTERM) == 0) {
							ipType = ConnectionProfile::IPP_DEFAULT;
							conType = ConnectionProfile::CT_CTERM;
						}

						free(buf);
					} 

					nameLen = GetWindowTextLength(GetDlgItem(hDlg, IDC_NEW_CONN_NAME)) + 1;
					name = (LPTSTR)malloc(nameLen * sizeof(TCHAR));
					ZeroMemory(name, nameLen * sizeof(TCHAR));
					GetWindowText(GetDlgItem(hDlg, IDC_NEW_CONN_NAME), name, nameLen);

					if (conType == ConnectionProfile::CT_SERIAL) {
						hostLen = GetWindowTextLength(ddSerial) + 1;
						host = (LPTSTR)malloc(hostLen * sizeof(TCHAR));
						ZeroMemory(host, hostLen * sizeof(TCHAR));
						GetWindowText(ddSerial, host, hostLen);

						portLen = GetWindowTextLength(ddSpeed) + 1;
						port = (LPTSTR)malloc(portLen * sizeof(TCHAR));
						ZeroMemory(port, portLen * sizeof(TCHAR));
						GetWindowText(ddSpeed, port, portLen);

					} else {
						hostLen = GetWindowTextLength(leHost) + 1;
						host = (LPTSTR)malloc(hostLen * sizeof(TCHAR));
						ZeroMemory(host, hostLen * sizeof(TCHAR));
						GetWindowText(leHost, host, hostLen);

						if (conType == ConnectionProfile::CT_SSH ||
							conType == ConnectionProfile::CT_FTP ||
							conType == ConnectionProfile::CT_IP) {

							portLen = GetWindowTextLength(lePort) + 1;
							port = (LPTSTR)malloc(portLen * sizeof(TCHAR));
							ZeroMemory(port, portLen * sizeof(TCHAR));
							GetWindowText(lePort, port, portLen);
						}
					}

					if (lstrlen(name) == 0) {
						ok = FALSE;
						MessageBox(hDlg, TEXT("A name is required"),
							TEXT("Error"), MB_OK | MB_ICONWARNING);
					}

					if (ok) {
						// Create the connection!
						ConnectionProfile *prof = _configFile->createProfile(
							0, CMString(name), conType);	// Null to use the default template

						switch(conType) {
						case ConnectionProfile::CT_SSH:
						case ConnectionProfile::CT_FTP:
						case ConnectionProfile::CT_IP:
							prof->setIpConnectionDetails(host, _ttoi(port), ipType);
							break;
						case ConnectionProfile::CT_SERIAL:
							prof->setSerialConnectionDetails(host, _ttoi(port));
							break;
						case ConnectionProfile::CT_LAT:
						case ConnectionProfile::CT_CTERM:
							prof->setLatService(host);
							break;
						case ConnectionProfile::CT_NAMED_PIPE:
							// TODO: support setting the host
							prof->setNamedPipeConnectionDetails(host, TEXT("."));
							break;
						case ConnectionProfile::CT_PTY:
						case ConnectionProfile::CT_PIPE:
							prof->setPtyCommand(host);
							break;
						case ConnectionProfile::CT_DLL:
							// TODO: support setting the parameters
							prof->setDllConnectionDetails(host, CMString());
							break;
						}

						BOOL saved = FALSE;

						if (showProps) {
							int rc = DoPropSheet(hDlg, _hInstance, prof);
							if (rc > 0) {
								saved = TRUE;
							}
						}

						if (!saved) {
							prof->commitChanges();
						}

						// And add it to the list view
						AddConnection(prof);
					}
					

					if (name != NULL) free(name);
					if (host != NULL) free(host);
					if (port != NULL) free(port);

					if (ok) {
						EndDialog(hDlg, 0);
					}
					return TRUE;
				}

			case IDCANCEL:
				EndDialog(hDlg, 0);
				return TRUE;

			case IDC_NEW_CONN_OTHER:
				{
					int evt = HIWORD(wParam);
					if (evt == CBN_SELCHANGE) {
						// User has picked a protocol
						int buflen;
						LPTSTR buf = NULL;

						buflen = GetWindowTextLength(ddIpProto) + 1;
						buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
						ZeroMemory(buf, sizeof(TCHAR) * buflen);

						GetWindowText(ddIpProto, buf, buflen);

						// Assume we're doing IP and need both fields
						SetWindowText(lblTarget, TEXT("Hostname or IP address:"));
						ShowWindow(leHost, SW_SHOW);
						ShowWindow(ddSerial, SW_HIDE);

						SetWindowText(lblParameter, TEXT("Port:"));
						ShowWindow(lblParameter, SW_SHOW);
						ShowWindow(lePort, SW_SHOW);
						ShowWindow(ddSpeed, SW_HIDE);

						if (lstrcmp(buf, CONOTHER_TELNET_SSL) == 0 || 
								   lstrcmp(buf, CONOTHER_TELNET_TLS) == 0) {
							// Telnet over SSL/TLS is officially on port 992.
							SetWindowText(lePort, TEXT("992"));
						} else if (lstrcmp(buf, CONOTHER_IKS) == 0) {
							// IKS official port is 1649
							SetWindowText(lePort, TEXT("1649"));
						} else if (lstrcmp(buf, CONOTHER_RLOGIN) == 0) {
							// RLOGIN official port is 513
							SetWindowText(lePort, TEXT("513"));
						} else if (lstrcmp(buf, CONOTHER_K4LOGIN) == 0 || 
								   lstrcmp(buf, CONOTHER_K5LOGIN) == 0) {
							// KLOGIN official port is 543
							SetWindowText(lePort, TEXT("543"));
						} else if (lstrcmp(buf, CONOTHER_EK4LOGIN) == 0 || 
								   lstrcmp(buf, CONOTHER_EK5LOGIN) == 0) {
							// EKLOGIN is unofficially on port 2105
							SetWindowText(lePort, TEXT("2105"));
						} else if (lstrcmp(buf, CONOTHER_TELNET) == 0 || 
								   lstrcmp(buf, CONOTHER_DEFAULT) == 0 || 
								   lstrcmp(buf, CONOTHER_TELNET_NNEGO) == 0 || 
								   lstrcmp(buf, CONOTHER_TELNET_SSL) == 0) {
							// Telnet is officially on port 23. I assume the SSL and TLS ones are too
							SetWindowText(lePort, TEXT("23"));
						} if (lstrcmp(buf, CONOTHER_SSL) == 0 || 
							  lstrcmp(buf, CONOTHER_TLS) == 0) {
							SetWindowText(lePort, TEXT("443"));
						} if (lstrcmp(buf, CONOTHER_RAW) == 0) {
							SetWindowText(lePort, TEXT("80"));
						} else {
							BOOL ok = FALSE;
							if (lstrcmp(buf, CONOTHER_DLL) == 0) {
								ok = TRUE;
								SetWindowText(lblTarget, TEXT("DLL Name:"));
							} else if (lstrcmp(buf, CONOTHER_PIPE) == 0) {
								ok = TRUE;
								SetWindowText(lblTarget, TEXT("Command:"));
							} else if (lstrcmp(buf, CONOTHER_LAT) == 0) {
								ok = TRUE;
								SetWindowText(lblTarget, TEXT("Service name or node/port:"));
							} else if (lstrcmp(buf, CONOTHER_CTERM) == 0) {
								ok = TRUE;
								SetWindowText(lblTarget, TEXT("Node name:"));
							}

							if (ok) {
								// Single field required
								ShowWindow(leHost, SW_SHOW);
								ShowWindow(ddSerial, SW_HIDE);

								ShowWindow(lblParameter, SW_HIDE);
								ShowWindow(lePort, SW_HIDE);
								ShowWindow(ddSpeed, SW_HIDE);
							}
						}

						free(buf);

						return TRUE;
					}
				}
				break;

			case IDC_NEW_CONN_FTP:
			case IDC_NEW_CONN_IP:
			case IDC_NEW_CONN_SSH: 
				{

					SetWindowText(lblTarget, TEXT("Hostname or IP address:"));
					ShowWindow(leHost, SW_SHOW);
					ShowWindow(ddSerial, SW_HIDE);

					SetWindowText(lblParameter, TEXT("Port:"));
					ShowWindow(lblParameter, SW_SHOW);
					ShowWindow(lePort, SW_SHOW);
					ShowWindow(ddSpeed, SW_HIDE);

					if (cmd == IDC_NEW_CONN_SSH) {
						SetWindowText(lePort, TEXT("22"));
						EnableWindow(ddIpProto, FALSE);
					} else if (cmd == IDC_NEW_CONN_FTP) {
						SetWindowText(lePort, TEXT("21"));
						EnableWindow(ddIpProto, FALSE);
					} else if (cmd == IDC_NEW_CONN_IP) {
						SetWindowText(lePort, TEXT("23"));
						EnableWindow(ddIpProto, TRUE);
					} 
				}	
				break;
			
			// also: lat, pipe, dll
			/// These only need a single field (command, node, pipe name, etc)
			case IDC_NEW_CONN_NAMED_PIPE:
			case IDC_NEW_CONN_PTY:
				{
					if (cmd == IDC_NEW_CONN_NAMED_PIPE) {
						SetWindowText(lblTarget, TEXT("Pipe Name:"));
					} else if (cmd == IDC_NEW_CONN_PTY) {
						SetWindowText(lblTarget, TEXT("Command or Shell:"));
					}
					

					ShowWindow(leHost, SW_SHOW);
					ShowWindow(ddSerial, SW_HIDE);

					ShowWindow(lblParameter, SW_HIDE);
					ShowWindow(lePort, SW_HIDE);
					ShowWindow(ddSpeed, SW_HIDE);

					EnableWindow(ddIpProto, FALSE);
				}	
				break;
			case IDC_NEW_CONN_SERIAL:
				{
					SetWindowText(lblTarget, TEXT("Port:"));
					ShowWindow(leHost, SW_HIDE);
					ShowWindow(ddSerial, SW_SHOW);

					SetWindowText(lblParameter, TEXT("Speed:"));
					ShowWindow(lblParameter, SW_SHOW);
					ShowWindow(lePort, SW_HIDE);
					ShowWindow(ddSpeed, SW_SHOW);

					EnableWindow(ddIpProto, FALSE);
				}	
				break;

			}
		}
		break;
	}
	return FALSE;
}