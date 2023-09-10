#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>

#include "conn_props.h"
#include "resource.h"

void CALLBACK		PropSheetCallback(HWND, UINT, LPARAM);
LRESULT CALLBACK	SimplePropSheetProc(HWND, UINT, WPARAM, LPARAM);

void SetupPropertyPage(HINSTANCE hInstance, PROPSHEETPAGE *page, WORD dialogId,
						DLGPROC pfnDlgProc, LPFNPSPCALLBACK pfnCallback, LPARAM lParam) {

	ZeroMemory(page, sizeof(PROPSHEETPAGE));

	page->dwSize		= sizeof(PROPSHEETPAGE);
	page->dwFlags		= PSP_USECALLBACK;
	page->hInstance		= hInstance;
	page->pszTemplate	= MAKEINTRESOURCE(dialogId);
	page->pszIcon		= NULL;
	page->pfnDlgProc	= pfnDlgProc;
	page->pfnCallback   = pfnCallback;
	page->lParam		= lParam;

}

int DoPropSheet(HWND hWnd, HINSTANCE hInstance, ConnectionProfile *profile) {
	HWND result = NULL;
	ConnectionProfile::ConnectionType conType = profile->connectionType();
	int pageCount;

	// Figure out how many pages we have. This varies by connection type
	// (don't need FTP settings for an SSH connection)
	switch(conType) {
	case ConnectionProfile::CT_TEMPLATE:
		pageCount = 15; break;
	case ConnectionProfile::CT_LAT:
	case ConnectionProfile::CT_CTERM:
	case ConnectionProfile::CT_SERIAL:
	case ConnectionProfile::CT_NAMED_PIPE:
	case ConnectionProfile::CT_PTY:
	case ConnectionProfile::CT_PIPE:
	case ConnectionProfile::CT_DLL:
	case ConnectionProfile::CT_SSH:
		pageCount = 11; break; // Excluded: IDD_TELNET, IDD_FTP, IDD_TCPIP, IDD_TLS
	case ConnectionProfile::CT_FTP:
		pageCount = 12; break; // Excluded: IDD_TERMINAL, IDD_TERM_COLORS, IDD_TELNET
	case ConnectionProfile::CT_IP:
		pageCount = 14; break; // Excluded: IDD_FTP
	default:
		OutputDebugString(TEXT("ConnProps: Unrecognised Connection Type"));
		return NULL; // we should never get here
	}

	// Create property pages
	PROPSHEETPAGE *psp = (PROPSHEETPAGE *)malloc(pageCount * sizeof(PROPSHEETPAGE));
	PROPSHEETHEADER psh;
	int rc;
	int page = 0;

	ZeroMemory(psp, pageCount * sizeof(PROPSHEETPAGE));
	ZeroMemory(&psh, sizeof(PROPSHEETHEADER));

	// ----- General -----
	SetupPropertyPage(hInstance, &psp[page], IDD_GENERAL, (DLGPROC)GeneralPageDlgProc, GeneralPageProc, (LPARAM)profile); page++; // *

	// ----- Connection -----

	// The dialog resource varies by connection type to account
	// for the different fields required.
	switch(conType) {
	case ConnectionProfile::CT_SSH:
	case ConnectionProfile::CT_FTP:
		// These just need hostname and port.
		SetupPropertyPage(hInstance, &psp[page], IDD_CONNECTION_BASIC, (DLGPROC)ConnectionPageDlgProc, ConnectionPageProc, (LPARAM)profile); page++;
		break;
	case ConnectionProfile::CT_IP:
		// This requires hostname, port plus a drop-down for protocol.
		SetupPropertyPage(hInstance, &psp[page], IDD_CONNECTION_IP, (DLGPROC)ConnectionPageDlgProc, ConnectionPageProc, (LPARAM)profile); page++;
		break;
	case ConnectionProfile::CT_MODEM:
	case ConnectionProfile::CT_SERIAL:
		// These require a bunch of drop-downs and checkboxes to configure
		// the serial line
		SetupPropertyPage(hInstance, &psp[page], IDD_CONNECTION_SERIAL, 
			(DLGPROC)NULL, NULL, (LPARAM)profile); page++;
		break;
	case ConnectionProfile::CT_LAT:
	case ConnectionProfile::CT_CTERM:
	case ConnectionProfile::CT_NAMED_PIPE:
	case ConnectionProfile::CT_PTY:
	case ConnectionProfile::CT_PIPE:
	case ConnectionProfile::CT_DLL:
	default:
		// These all require either one or two text fields.
		SetupPropertyPage(hInstance, &psp[page], IDD_CONNECTION_CMD, 
			(DLGPROC)ConnectionPageDlgProc, ConnectionPageProc, (LPARAM)profile); page++;
		break;
	}


	if (conType == ConnectionProfile::CT_SSH) {
		// TODO: SSH page
	}

	if (conType == ConnectionProfile::CT_IP) {
		
		SetupPropertyPage(hInstance, &psp[page],  IDD_TELNET,	NULL, NULL, NULL); page++;
	}

	if (conType == ConnectionProfile::CT_FTP) {
		SetupPropertyPage(hInstance, &psp[page],  IDD_FTP,		NULL, NULL, NULL); page++;
	}

	// ----- Terminal -----
	if (conType != ConnectionProfile::CT_FTP) {
		SetupPropertyPage(hInstance, &psp[page], IDD_TERMINAL,   (DLGPROC)TerminalPageDlgProc, TerminalPageProc, (LPARAM)profile); page++;
		SetupPropertyPage(hInstance, &psp[page], IDD_TERM_COLORS,(DLGPROC)TermColorPageDlgProc, TermColorPageProc, (LPARAM)profile); page++; // *
	}
	SetupPropertyPage(hInstance, &psp[page], IDD_KEYBOARD,	NULL, NULL, NULL); page++; // *
	// TODO: Mouse

	// ----- Transfer -----
	SetupPropertyPage(hInstance, &psp[page],  IDD_TRANSFER,	NULL, NULL, NULL); page++;

	// ----- GUI -----
	SetupPropertyPage(hInstance, &psp[page], IDD_GUI,		NULL, NULL, NULL); page++; // *
	SetupPropertyPage(hInstance, &psp[page], IDD_GUI_COLORS,NULL, NULL, NULL); page++; // *

	// ----- Advanced stuff -----
	SetupPropertyPage(hInstance, &psp[page], IDD_LOGIN,	(DLGPROC)LoginPageDlgProc, LoginPageProc, (LPARAM)profile); page++; // *
	SetupPropertyPage(hInstance, &psp[page], IDD_PRINTER,	NULL, NULL, NULL); page++; // *

	if (conType == ConnectionProfile::CT_IP
		|| conType == ConnectionProfile::CT_FTP) {
		SetupPropertyPage(hInstance, &psp[page],  IDD_TCPIP,	NULL, NULL, NULL); page++;
		SetupPropertyPage(hInstance, &psp[page],  IDD_TLS,		NULL, NULL, NULL); page++;
		// TODO: Kerberos
	}

	SetupPropertyPage(hInstance, &psp[page], IDD_LOGGING,	NULL, NULL, NULL); 

	// Setup the property sheet	
	psh.dwSize		= sizeof(PROPSHEETHEADER);
	psh.dwFlags		= PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_USECALLBACK | PSH_PROPTITLE;
	psh.hwndParent	= hWnd;
	psh.hInstance	= hInstance;
	//psh.pszIcon		= MAKEINTRESOURCE(IDI_BACKCOLOR);
	psh.pszCaption	= profile->name().data(); //titleBuf; //TEXT("Connection");
	psh.nPages		= page;
	psh.ppsp		= (LPCPROPSHEETPAGE) psp;
	psh.pfnCallback	= (PFNPROPSHEETCALLBACK)PropSheetCallback;

	// and go!
	rc = PropertySheet(&psh);

	free(psp);

	if (rc > 0) {
		profile->commitChanges();
	}

	return rc;
}

LRESULT CALLBACK SimplePropSheetProc(HWND hdlg, UINT uMessage, 
									 WPARAM wParam, LPARAM lParam) {
	
	switch (uMessage) {
	case WM_COMMAND:
			PropSheet_Changed(GetParent(hdlg), hdlg);
			break;

	default:
		break;
	}

	return FALSE;
}

void CALLBACK PropSheetCallback(HWND hWnd, UINT uMsg, LPARAM lParam) {
	switch (uMsg) {

	/* Called before the dialog is created. hWnd will be null
	   and lParam a dialog resource */
	case PSCB_PRECREATE: {
		LPDLGTEMPLATE lpTemplate = (LPDLGTEMPLATE)lParam;
		if (!(lpTemplate->style & WS_SYSMENU)) {
			lpTemplate->style |= WS_SYSMENU;
		}
	}

	/* Called after the dialog is created */
	case PSCB_INITIALIZED:
		break;

	}
}


// This function checks to see if the current value in the
// specified text field matches the supplied original value.
BOOL textFieldChanged(HWND hwndField, LPTSTR originalValue) {
	int buflen = 0;
	LPTSTR buf = NULL;
	BOOL changed = FALSE;

	// Get current value
	buflen = GetWindowTextLength(hwndField) + 1;

	// allocate buffer
	buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
	ZeroMemory(buf, buflen * sizeof(TCHAR));

	// Get current value
	GetWindowText(hwndField, buf, buflen);

	if (lstrcmp(buf, originalValue) != 0) {
		changed = TRUE;
	}

	free(buf);
	return changed;
}

// Caller is responsible for cleaning up the returned string
LPTSTR getFieldText(HWND hwndDlg, int controlId) {
	HWND hWnd = GetDlgItem(hwndDlg, controlId);

	int len = GetWindowTextLength(hWnd) + 1;

	LPTSTR buf = (LPTSTR)malloc(len * sizeof(TCHAR));
	ZeroMemory(buf, len * sizeof(TCHAR));

	GetWindowText(hWnd, buf, len);

	return buf;
}

int getFieldInt(HWND hwndDlg, int id) {
	LPTSTR buf[20];

	HWND hwnd = GetDlgItem(hwndDlg, id);

	GetWindowText(hwnd, (LPTSTR)buf, 20);
	return _ttoi((LPCTSTR)buf);
}