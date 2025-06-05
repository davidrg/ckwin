/*
Stuff missing from K95DIAL:
  * General
	- Download Directory  -- Move to the File Transfer page
  * Terminal Settings
    - Autodownload -- Move to the File Transfer page
	- Foreground and Background colors - move elsewhere
  * File Transfer Settings
  * Connection Type Pages
	* TCP/IP Settings  (all connection types except LAT and Serial)
	* Telnet Settings
	* Serial Settings
	* FTP Settings
	* SSH Settings
  * Keyboard Settings
  * Login Settings
  * Printer Settings
  * Log File Settings
  * GUI Settings

  These probably won't all fit in one properties window.
  Maybe have a few?
	General			- General, Connection Settings, Login Settings, GUI, Logging
	Terminal		- Terminal, Keyboard, Printer
	File Transfer	- 
*/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef CKMODERNSHELL
#include <shobjidl_core.h>
#endif /* CKMODERNSHELL */

#include "resource.h"
#include "toolbar.h"
#include "statusbar.h"
#include "conn_list.h"
#include "conn_props.h"
#include "new_conn.h"
#include "util.h"
#include "ipc_messages.h"
#include "kerm_track.h"
#include "json_config.h"
#ifdef CKMODERNSHELL
#include "jumplist.h"
#endif /* CKMODERNSHELL */

// TODO: can Visual C++ 2003 do this too?
// Currently limited to Visual C++ 2005 or newer.
#if _MSC_VER > 1310
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#ifdef CKMODERNSHELL
PCWSTR cszProgramID = L"KermitProject.Kermit95";
#endif /* CKMODERNSHELL */

#define STATUS_UPDATE_TIMER_ID 1
#define STATUS_UPDATE_TIMER_MSEC 1000

/* Function Prototypes */
int PASCAL			WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
BOOL				InitApplication(HINSTANCE);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK		AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK		DisconnectDlgProc(HWND, UINT, WPARAM, LPARAM);
void				LaunchKermit(HWND hwndParent, BOOL k95g);

/* Global Variables */
TCHAR		g_szClassName[] = TEXT("K95ConMan");
HWND		g_hwndMain,
			g_hwndToolbar,
			g_hwndStatusbar,
			g_hwndConnectionList,
			g_hwndPropSheet;
HINSTANCE	g_hInstance;
ConfigFile  *g_ConfigFile = NULL;

int PASCAL WinMain(HINSTANCE hInstance, 
				   HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, 
				   int nCmdShow) {
	MSG msg;
	HACCEL hAccel;
	BOOL launchK95 = FALSE;
	BOOL launchK95g = FALSE;
	int launchProfileIdWhenReady = -1;

#ifdef CKMODERNSHELL
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (!SUCCEEDED(hr)) {
        return 0;
    }

    hr = SetCurrentProcessExplicitAppUserModelID(cszProgramID);
#endif /* CKMODERNSHELL */

	InitCommonControls();

	HWND hwndSelf = FindWindow(NULL, TEXT("Kermit 95 Connection Manager"));

	// Command line arguments
	// -k hwnd pid	  Kermit 95 letting us know its window handle and process ID
	if (__argc == 4 && strcmp(__argv[1], " -k")) {
		// We've been passed a HWND and a Process ID indicating that
		// Kermit 95 instance is ready to be taken over. This is almost
		// certainly caused by the user clicking the dialer button.

#ifdef _WIN64
		HWND hWndInstance = _atoi64(__argv[2]);
#else
		HWND hWndInstance = (HWND)atoi(__argv[2]);
#endif

		int pidInstance = atoi(__argv[3]);

		if (hwndSelf != NULL) {
			// Let the existing instance of the connection manager handle this
			SendMessage(hwndSelf, OPT_KERMIT_HWND2, 0, (LPARAM)hWndInstance);
			SendMessage(hwndSelf, OPT_KERMIT_PID, 0, (LPARAM)pidInstance);
		} else {
			KermitInstance *inst = new KermitInstance(hWndInstance, pidInstance, NULL);
			inst->setStatus(KermitInstance::S_READY);
			KermitInstance::setPriorityInstanceIdAvailable(inst->instanceId());
		}
	} else if (__argc == 2 && strcmp(__argv[1], " -n")) {
		// -n		Start new Kermit instance
		launchK95g = TRUE;
		nCmdShow = SW_HIDE;
		if (hwndSelf != NULL) {
			SendMessage(hwndSelf, OPT_CM_LAUNCH_K95, TRUE, 0);
		}
	} else if (__argc == 2 && strcmp(__argv[1], " -s")) {
		if (hwndSelf != NULL) {
			ShowWindow(hwndSelf, SW_SHOW);
		}
	} else if (__argc == 3 && strcmp(__argv[1], " -c")) {
		launchProfileIdWhenReady = atoi(__argv[2]);
		nCmdShow = SW_HIDE;

		if (hwndSelf != NULL) {
			SendMessage(hwndSelf, OPT_CM_LAUNCH_PROFILE,
						(WPARAM)launchProfileIdWhenReady, (LPARAM)0);
		}
	}

	if (hwndSelf != NULL) {
		if (nCmdShow != SW_HIDE) {
			ShowWindow(hwndSelf, SW_SHOW);
			SetForegroundWindow(hwndSelf);
		}

#ifdef CKMODERNSHELL
		CoUninitialize();
#endif /* CKMODERNSHELL */

		return 0;
	}

	g_hInstance = hInstance;

	if (!hPrevInstance) {
		if (!InitApplication(hInstance)) {
#ifdef CKMODERNSHELL
			CoUninitialize();
#endif /* CKMODERNSHELL */
			return FALSE;
		}
	}

	if (!InitInstance(hInstance, nCmdShow)) {
#ifdef CKMODERNSHELL
		CoUninitialize();
#endif /* CKMODERNSHELL */
		return FALSE;
	}

	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	if (launchK95g) {
		SendMessage(g_hwndMain, OPT_CM_LAUNCH_K95, TRUE, 0);
		launchK95g = FALSE;
	}

	if (launchProfileIdWhenReady >= 0) {
		SendMessage(g_hwndMain, OPT_CM_LAUNCH_PROFILE,
					(WPARAM)launchProfileIdWhenReady, (LPARAM)0);
		launchProfileIdWhenReady = -1;
	}

	while(GetMessage(&msg, NULL, 0x00, 0x00)) {

		if (g_hwndPropSheet 
			&& (PropSheet_GetCurrentPageHwnd(g_hwndPropSheet) == NULL)) {

			EnableWindow(g_hwndMain, TRUE);
			DestroyWindow(g_hwndPropSheet);
			g_hwndPropSheet = NULL;
		}

		if (!PropSheet_IsDialogMessage(g_hwndPropSheet, &msg)
			&& !TranslateAccelerator(g_hwndMain, hAccel, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

#ifdef CKMODERNSHELL
    CoUninitialize();
#endif /* CKMODERNSHELL */

	return (int)msg.wParam;
}

BOOL InitApplication(HINSTANCE hInstance) {

	OSVERSIONINFO os;

	ZeroMemory(&os, sizeof(OSVERSIONINFO));
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&os);

	if (os.dwMajorVersion >= 4) {
		/* Windows 95, NT 4.0 or newer */
		WNDCLASSEX wcex;
		ZeroMemory(&wcex, sizeof(WNDCLASSEX));

		wcex.cbSize			= sizeof(wcex);
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= (WNDPROC)MainWndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
		wcex.lpszMenuName	= MAKEINTRESOURCE(IDR_MENU);
		wcex.lpszClassName	= g_szClassName;

		return RegisterClassEx(&wcex);
	} else {
		/* Windows NT 3.x */
		WNDCLASS wc;
		ZeroMemory(&wc, sizeof(WNDCLASS));

		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)MainWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= hInstance;
		wc.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName		= MAKEINTRESOURCE(IDR_MENU);
		wc.lpszClassName	= g_szClassName;

		return RegisterClass(&wc);		
	}

}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	
	g_hwndMain = CreateWindowEx(
		0,
		g_szClassName,
		TEXT("Kermit 95 Connection Manager"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,	/* Horizontal Position */
		CW_USEDEFAULT,	/* Vertical Position */
		500,			/* Width */
		250,			/* Height */
		NULL,
		NULL,
		hInstance,
		NULL);
	if (!g_hwndMain) {
		return FALSE;
	}

	g_hwndToolbar = CreateToolBar(g_hwndMain, g_hInstance);
	g_hwndStatusbar = CreateStatusBar(g_hwndMain, g_hInstance);
	g_hwndConnectionList = CreateConnectionListView(g_hwndMain, g_hInstance);

	// Get the config filename
	LPTSTR fileName = (LPTSTR)malloc(sizeof(TCHAR) * MAX_PATH);
	LPTSTR filePath = (LPTSTR)malloc(sizeof(TCHAR) * MAX_PATH);

	{	
		/*  This code will put the temp directory under the directory
		    where the executable lives. Nice and tidy, but it might
			fail in the future if the app ends up being installed somewere
			like Program Files.*/
		
		GetModuleFileName(NULL, filePath, MAX_PATH);

		// We want only the path - actual module filename.
		int lastSlashPos = 0;

		// Look for the last path separator in the directory.
		for (int i = 0; i < MAX_PATH; i++) {
			if (filePath[i] == _T('\\')) {
				lastSlashPos = i;
			}
		}

		// And slap a null termination right after the last slash in the string.
		if (lastSlashPos > 0 && lastSlashPos + 1 < MAX_PATH) {
			filePath[lastSlashPos + 1] = NULL;
		}
		
		

		_sntprintf(fileName, MAX_PATH, TEXT("%scm.json"), filePath);
	}

	g_ConfigFile = new JsonConfigFile(g_hwndMain, fileName);

	free(fileName);
	free(filePath);

	if (!g_ConfigFile->loaded()) return FALSE;

	AddConnections(g_hwndConnectionList, g_ConfigFile);
	ProfileSelected(FALSE, FALSE);

#ifdef CKMODERNSHELL
	UpdateJumpList();
#endif /* CKMODERNSHELL */

	ShowWindow(g_hwndMain, nCmdShow);
	UpdateWindow(g_hwndMain);

	return TRUE;
}


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam) {

	BOOL exit = FALSE;

	switch(uMessage) {

	case WM_CREATE:
		SetTimer(hWnd, STATUS_UPDATE_TIMER_ID, STATUS_UPDATE_TIMER_MSEC, NULL);
		break;

	case OPT_CM_QUIT:
		// File->Exit sends us this. We should exit even if we were keeping an
		// eye on any K95 instances.
		exit = TRUE;
		// fall through
	case WM_CLOSE:
		// If any K95 instances we launched are still running, then we should
		// stay running too. Just hide the window instad of exiting.
		if (KermitInstance::anyInstances() && !exit) {
			ShowWindow(hWnd, SW_HIDE);
			return 0;
		}

		if (IsWindow(g_hwndPropSheet)) {
			DestroyWindow(g_hwndPropSheet);
		}

		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		{
			// Notify all the Kermit 95 instances we've been working with that
			// we're exiting.
			KermitInstance *inst = KermitInstance::firstInstance();
			while (inst != NULL) {
				inst->notifyOfExit();
				inst = inst->nextInstance();
			}

			PostQuitMessage(0);
		}
		break;

	case WM_TIMER:
		{
			if (wParam == STATUS_UPDATE_TIMER_ID) {
				// Refresh the status bar
				StatusRefresh();

				BOOL selected = FALSE, connected = FALSE;

				ConnectionProfile *profile = GetSelectedProfile(g_hwndConnectionList);

				if (profile != NULL) {
					selected = TRUE;
					
					KermitInstance *inst = KermitInstance::getInstanceByProfile(profile);

					if (inst != NULL && inst->status() == KermitInstance::
						S_CONNECTED) {
						connected = TRUE;
					}
				}


				ProfileSelected(selected, connected);
			}
		}
		break;

	case WM_MENUSELECT:
		return StatusBarMenuSelect(hWnd, g_hInstance, 
			wParam, lParam);

	case WM_CONTEXTMENU: {
			RECT rc;
			POINT pt;
			ConnectionProfile *profile = GetSelectedProfile(g_hwndConnectionList);

			if (profile != NULL) {
				GetWindowRect(g_hwndConnectionList, &rc);
#ifdef GET_X_LPARAM
				// These are new to Visual C++ 5.0 and required for multi-monitor setups
				// where the coordinates could have negative values where the window is
				// on a display to the left of or above the primary display.
				pt.x = GET_X_LPARAM(lParam);
				pt.y = GET_Y_LPARAM(lParam);

				if (pt.x == -1 && pt.y == -1) {
					// This indicates the context menu was requested via the keyboard (either
					// Shift+F10 or the context menu key), so get the coordinates of the selected
					// item.
					if (!GetSelectedProfileScreenCoord(&pt)) {
						break;
					}
				}
#else
				pt.x = LOWORD(lParam);
				pt.y = HIWORD(lParam);
#endif

				// TODO: Prevent the context menu appearing for the header
				//		 (if we show one ideally it would be to pick visible columns)

				if (PtInRect((LPRECT)&rc, pt)) {
					HMENU hmenu = NULL, popup = NULL;

					hmenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_CONTEXT_MENU));
					if (hmenu == NULL) break;

					popup = GetSubMenu(hmenu, 0);

					KermitInstance *inst = KermitInstance::getInstanceByProfile(profile);

					BOOL connected = inst != NULL && inst->status() == KermitInstance::S_CONNECTED;

					EnableMenuItem(hmenu, ID_CONTEXT_DISCONNECT, 
						connected ? MF_ENABLED : MF_GRAYED);

					TrackPopupMenu(popup, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
						pt.x, pt.y, 0, hWnd, NULL);

					DestroyMenu(hmenu);
				}
			}
		}
		break;

	case WM_SIZE: {
			int cx = LOWORD (lParam);
            int cy = HIWORD (lParam);
			int cyToolbar = 0, cyStatusbar = 0, x = 0, y = 0;
			DWORD dwStyle ;
			RECT rWindow;

			// Adjust toolbar size
			dwStyle = GetWindowLong (g_hwndToolbar, GWL_STYLE);
      
			SendMessage(g_hwndToolbar, TB_AUTOSIZE, 0, 0L);

			InvalidateRect (g_hwndToolbar, NULL, TRUE);
			GetWindowRect (g_hwndToolbar, &rWindow);
			ScreenToClient (hWnd, (LPPOINT) &rWindow.left);
			ScreenToClient (hWnd, (LPPOINT) &rWindow.right);
			cyToolbar = rWindow.bottom - rWindow.top;

			// Adjust statusbar
			GetWindowRect (g_hwndStatusbar, &rWindow);
            cyStatusbar = rWindow.bottom - rWindow.top;
            MoveWindow(g_hwndStatusbar, 0, cy - cyStatusbar, 
                       cx, cyStatusbar, TRUE);
			/*SendMessage(g_hwndStatusbar, uMessage, wParam, lParam);*/

			// Adjust client window (the list widget)
			x = 0 ;
            y = cyToolbar;
            cy = cy - (cyStatusbar + cyToolbar) ;
            MoveWindow (g_hwndConnectionList, x, y, cx, cy, TRUE) ;

			//return 0;
		}
		break;

	case WM_NOTIFY: {
			LPNMHDR pnmh = (LPNMHDR) lParam ;
			// Toolbar notifications
			if ((pnmh->code >= TBN_LAST) &&
				(pnmh->code <= TBN_FIRST)) {
				return ToolbarNotify (g_hInstance, hWnd, wParam, lParam) ;
			}

			if (pnmh->hwndFrom == g_hwndConnectionList) {
				return ConnectionListViewNotify(
					g_hwndConnectionList, lParam, g_ConfigFile);
			}

			switch (pnmh->code) {
			case TTN_NEEDTEXT: {
					// Fetch tooltip text
					LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT) lParam;
					GetToolbarToolTipText(lpttt, g_hInstance);
					return 0 ;
				}
			}

			

			return 0 ;
		}

	case OPT_KERMIT_CONNECT: 
		{
			int instanceId = (int)wParam;
			// Kermit instance {instanceId} has succcessfully connected

			KermitInstance *inst = KermitInstance::getInstance(instanceId);

			if (inst != NULL) {
				inst->setConnected();
			}
		}
		break;

	case OPT_KERMIT_HANGUP:
		{
			int instanceId = (int)wParam;
			// Kermit instance {instanceId} has disconnected and gone idle
			// it instanceId is 0 then its an instance we're not tracking.

			KermitInstance *inst = KermitInstance::getInstance(instanceId);

			if (inst != NULL) {
				inst->setDisconnected();
			}
		}
		break;

	case OPT_KERMIT_HWND:
		{
			int instanceId = (int)wParam;
			HWND hWndInstance = (HWND)lParam;
			/* Instance {instanceId} is letting us know its window
			 * handle because either:
			 *   -> We started it and its busy starting up
			 *   -> We gave work to an existing instance that claimed 
			 *      it was ready for ready for re-use
			 *   -> The user tried to start the dialer from Kermit 95
			 *      (either via the "dialer" command, toolbar button
			 *      or file menu) but Kermit 95 wasn't started by us.
			 *      It found us by our window title and is letting us
			 *      know it exists. In this case it won't be able to
			 *      supply us with an instanceId, just its window handle.
			 * 
			 * Whatever the reason, it knows about us so if we weren't
			 * keeping an eye on it before we should start.
			 */

			KermitInstance *inst;

			if (instanceId == 0) {
				// Kermit 95 wasn't started by us - it found us by our window
				// title. Go create it an InstanceId.
				inst = new KermitInstance(hWndInstance, NULL, NULL);
				instanceId = inst->instanceId();
				// No need to delete *inst - its part of a linked list now.

				// let the instance know its instance ID. This is new behaviour
				// from Kermit 95 which didn't start tracking Kermit 95 instances
				// until they were handed work.
				PostMessage(hWndInstance, OPT_DIALER_HWND, 
					(WPARAM)instanceId, (LONG) hWnd);
			} else {
				inst = KermitInstance::getInstance(instanceId);
			}

			if (inst != NULL) {
				inst->setHwnd(hWndInstance);
			}

		}
		break;

	case OPT_KERMIT_HWND2:
		{
			int instanceId = (int)wParam;
			HWND hWndInstance = (HWND)lParam;
			/*
			 * This is sent to us by Kermit 95 when the user clicks the
			 * dialer toolbar button, the "Connections" file menu item
			 * or issues the "dialer" command. It indicates that the
			 * Kermit 95 instance is currently idle and we're free to take
			 * it over if the user tries to start a new connection.
			 */

			KermitInstance *inst;
			if (instanceId == 0) {
				// Kermit 95 wasn't started by us - it found us by our window
				// title. Go create it an InstanceId.
				inst = new KermitInstance(hWndInstance, NULL, NULL);
				instanceId = inst->instanceId();
				// No need to delete *inst - its part of a linked list now.

				// let the instance know its instance ID. This is new behaviour
				// from Kermit 95 which didn't start tracking Kermit 95 instances
				// until they were handed work.
				PostMessage(hWndInstance, OPT_DIALER_HWND, 
					(WPARAM)instanceId, (LONG) hWnd);
			} else {
				inst = KermitInstance::getInstance(instanceId);
				inst->setHwnd(hWndInstance); // just in case we didn't already know it
			}

			if (inst != NULL) {
				// Mark it as ready to be taken over
				inst->setStatus(KermitInstance::S_READY);

				// And put it at the head of the queue for re-use
				KermitInstance::setPriorityInstanceIdAvailable(inst->instanceId());
			}
		}
		break;

	case OPT_KERMIT_PID:
		{
			int instanceId = (int)wParam;
			DWORD processId = (DWORD)lParam;
			/*
			 * This is sent to us by Kermit 95 when the user clicks the
			 * dialer toolbar button, the "Connections" file menu item
			 * or issues the "dialer" command. It indicates that the
			 * Kermit 95 instance is currently idle and we're free to take
			 * it over if the user tries to start a new connection.
			 */

			KermitInstance *inst;
			if (instanceId == 0) {
				// This should never happen: OPT_KERMIT_PID always follows
				// a OPT_KERMIT_HWND2 which always follows a OPT_KERMIT_HWND.

				// If we receive either a OPT_KERMIT_HWND or OPT_KERMIT_HWND2
				// without an instance ID we respond by assigning an instance
				// ID. 

				// Kermit 95 wasn't started by us - it found us by our window
				// title. Go create it an InstanceId.
				inst = new KermitInstance(NULL, processId, NULL);
				instanceId = inst->instanceId();
				// No need to delete *inst - its part of a linked list now.
			} else {
				inst = KermitInstance::getInstance(instanceId);
				inst->setProcessId(processId); // in case we don't know it
			}

			if (inst != NULL) {
				// Mark it as ready to be taken over
				inst->setStatus(KermitInstance::S_READY);

				// And put it at the head of the queue for re-use
				KermitInstance::setPriorityInstanceIdAvailable(inst->instanceId());
			}
		}
		break;

	case OPT_KERMIT_EXIT:
		{
			int instanceId = (int)wParam;

			// We have no particular use for its exit status at the moment.
			// int exitStatus = (int)lParam;

			// Send to us by Kermit 95 to let us know its exiting. We don't
			// need to keep an eye on it anymore.
			if (instanceId != 0) {
				// Let the status bar know we're about to delete this instance
				// Id, just in case its currently tracking it.
				StatusKermitInstanceExiting(instanceId);

				// Then delete the instance Id.
				KermitInstance::removeInstance(instanceId);
			}

			if (!KermitInstance::anyInstances() && !IsWindowVisible(hWnd)) {
				// The last Kermit 95 instance we were keeping an eye on just
				// exited. If our UI is currently hidden, for all the user knows
				// we aren't running so we can safely exit now too.
				PostMessage(hWnd, WM_CLOSE, 0, 0);
			}
		}
		break;

	case OPT_CM_LAUNCH_K95:
		{
			BOOL useK95G = (BOOL)wParam;
			LaunchKermit(g_hwndMain, useK95G);
		}
		break;

	case OPT_CM_LAUNCH_PROFILE:
		{
			ConnectProfileId(g_hwndMain, wParam);
		}
		break;

	case WM_COMMAND:
		switch (GET_WM_COMMAND_ID(wParam, lParam)) {

		case ID_FILE_EXPORTCONNECTIONS:
			break;

		case ID_FILE_IMPORTCONNECTIONS:
			break;

		case ID_FILE_EXIT:
			PostMessage(hWnd, OPT_CM_QUIT, 0, 0);
			break;

		case ID_CONNECTION_ADD:
		case ID_CONTEXT_ADD:
		case ID_TB_NEW:
			NewConnection(g_hInstance, hWnd, g_ConfigFile);
			break;

		case ID_CONNECTION_CLONE:
		case ID_CONTEXT_CLONE:
		case ID_TB_COPY:
			break;
			
		case ID_CONNECTION_DELETE:
		case ID_CONTEXT_DELETE:
		case ID_TB_DELETE:
			break;

		case ID_CONNECTION_CONNECT:
		case ID_CONTEXT_CONNECT:
		case ID_TB_CONNECT: 
			{
				ConnectSelectedProfile(g_hwndConnectionList, hWnd);
			}
			return TRUE;

		case ID_CONNECTION_DISCONNECT:
		case ID_CONTEXT_DISCONNECT:
		case ID_TB_DISCONNECT:
			{
				// Firstly, see how many connections there are
				ConnectionProfile *profile = GetSelectedProfile(g_hwndConnectionList);

				if (profile != NULL) {
					KermitInstance *inst = KermitInstance::getInstanceByProfile(profile);
					
					if (inst != NULL) {
						if (inst->nextInstanceWithSameProfile(TRUE) != NULL) {
							// There are multiple instances connected with this profile.
							// We need to ask the user which one to kill.

							DialogBox(g_hInstance, 
								MAKEINTRESOURCE(IDD_DISCONNECT_SESSION),
								hWnd,
								(DLGPROC)DisconnectDlgProc);


						} else {
							// Just the one instance. A message box will do.
							LPTSTR buf = (LPTSTR)malloc(sizeof(TCHAR) * 300);

							_sntprintf(buf, 300, TEXT("Disconnect session %d?"),
								inst->instanceId());

							int rc = MessageBox(hWnd, buf,
								TEXT("Disconnect?"),
								MB_OKCANCEL | MB_ICONQUESTION);

							free(buf);

							if (rc == IDOK) {
								// Tell the session to disconnect
								inst->requestDisconnect();
								
								// We don't need to do anything further here. It will
								// let us know when its done what it was told to do.
							}
						}
					}
				}


			}
			break;


		case ID_CONNECTION_PROPERTIES:
		case ID_CONTEXT_PROPERTIES:
		case ID_TB_PROPERTIES: {
				ConnectionProfile *profile = GetSelectedProfile(g_hwndConnectionList);
				if (profile != NULL)
					DoPropSheet(hWnd, g_hInstance, profile);
			}
			return TRUE;

		case ID_CONNECTION_SHORTCUT:
		case ID_CONTEXT_CREATESHORTCUT:
		case ID_TB_SHORTCUT:
			break;

		case ID_CONNECTION_CREATESCRIPT:
		case ID_CONTEXT_CREATESCRIPT:
			{
				ConnectionProfile *profile = GetSelectedProfile(g_hwndConnectionList);
					if (profile != NULL) {
					// Browse for script file

					OPENFILENAME ofn;
					LPTSTR szFile[MAX_PATH];

					ZeroMemory(szFile, sizeof(TCHAR) * MAX_PATH);
					ZeroMemory(&ofn, sizeof(OPENFILENAME));

					ofn.lStructSize = sizeof(OPENFILENAME);					
					ofn.hwndOwner = hWnd;
					ofn.lpstrFile = (LPTSTR)szFile;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = TEXT("Kermit Scripts (*.ksc)\0*.KSC\0Text Files (*.txt)\0*.TXT\0All Files (*.*)\0*.*\0");
					ofn.lpstrDefExt = TEXT("ksc");
					ofn.nFilterIndex = 1;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.lpstrTitle = TEXT("Save As Script");
					ofn.Flags = 
						OFN_OVERWRITEPROMPT |
						OFN_HIDEREADONLY    ;
	;
					
					// Display the Save dialog box. 
					if (GetSaveFileName(&ofn)==TRUE) {
						profile->writeScript(hWnd, (LPTSTR)szFile);
					}
				}

			}
			break;

		case ID_OPTIONS_MINIMIZEONCONNECT:
			break;

		case ID_HELP_ABOUT:
			DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, (DLGPROC)AboutDlgProc);
			return 0;
	}

	default:
		break;
			
	}

	return DefWindowProc(hWnd, uMessage, wParam, lParam);
}


#ifdef _M_ALPHA
#ifdef _WIN64
#define CM_BUILD_ARCH TEXT("AlphaAXP-64")
#else
#define CM_BUILD_ARCH TEXT("AlphaAXP")
#endif
#elif _M_PPC
#define CM_BUILD_ARCH TEXT("PowerPC")
#elif _M_MRX000
#define CM_BUILD_ARCH TEXT("MIPS")
#elif _M_IA64
#define CM_BUILD_ARCH TEXT("Itanium")
#elif _M_ARM
#define CM_BUILD_ARCH TEXT("ARM32")
#elif _M_ARM64
#define CM_BUILD_ARCH TEXT("ARM64")
#elif _M_AMD64
#define CM_BUILD_ARCH TEXT("x86-64")
#elif _M_IX86
#define CM_BUILD_ARCH TEXT("x86")
#undef CM_NT_ONLY
#else
#define CM_BUILD_ARCH TEXT("unknown-cpu")
#endif

#ifdef UNICODE
#define CM_BUILD_CSET TEXT("UNICODE")
#define CM_NT_ONLY
#else
#define CM_BUILD_CSET TEXT("ANSI")
#endif

#ifdef _WIN64
#define CM_BUILD_BITS TEXT("64-bit")
#else
#define CM_BUILD_BITS TEXT("32-bit")
#endif

// If this build *requires* Windows NT. This means its either a Unicode build,
// or a 64bit build, or a non-x86 build. Probably we ought to change the compiler
// version too as, IIRC, anything built with Visual C++ 2008 or newer *requires* NT.
#define CM_NT_ONLY
#ifdef _M_IX86
#ifndef _WIN64
#ifndef UNICODE
#undef CM_NT_ONLY
#endif /* UNICODE */
#endif /* _WIN64 */
#endif /* _M_IX85 */

// size of buffer for some stuff in the about dialog.
#define CM_BUF_LEN 256

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG: 
		{
			LPTSTR fmt = (LPTSTR)malloc(sizeof(TCHAR) * CM_BUF_LEN);
			LPTSTR buf = (LPTSTR)malloc(sizeof(TCHAR) * CM_BUF_LEN);
			ZeroMemory(fmt, sizeof(TCHAR) * CM_BUF_LEN);
			ZeroMemory(buf, sizeof(TCHAR) * CM_BUF_LEN);
			LoadString(g_hInstance, IDS_ABOUT_BUILD_FMT, fmt, sizeof(TCHAR) * CM_BUF_LEN); 

			_sntprintf(buf, CM_BUF_LEN, fmt, CM_BUILD_ARCH, CM_BUILD_CSET, CM_BUILD_BITS);

			SetWindowText(GetDlgItem(hDlg, IDC_ABOUT_BUILD), buf);

			AppVersion vers = GetAppVersion(g_hInstance);

			LoadString(NULL, IDS_ABOUT_VER_FMT, fmt, sizeof(TCHAR) * CM_BUF_LEN); 

#ifdef CM_NT_ONLY							
			LPTSTR ntTag = (LPTSTR)malloc(sizeof(TCHAR) * 50);
			LoadString(g_hInstance, IDS_ABOUT_VER_FOR_NT, ntTag, sizeof(TCHAR) * 50); 
#endif 

			_sntprintf(buf, CM_BUF_LEN, fmt,
					vers.major,
					vers.minor,
					vers.revision,
					//vers.build,
#ifdef CM_NT_ONLY
					ntTag
#else
					TEXT("")
#endif 
					);

#ifdef CM_NT_ONLY
			free(ntTag);
#endif 

			SetWindowText(GetDlgItem(hDlg, IDC_ABOUT_VERSION), buf);

			free(buf);
			free(fmt);

			return TRUE;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam)) {
			case IDOK:
			case IDCANCEL:
				EndDialog(hDlg, 0);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}


#define TEMP_BUF_SIZE 256
// The Disconnect Session dialog: Let the user pick which session to disconnect.
BOOL CALLBACK DisconnectDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			LV_COLUMN lvc; 
			UINT columnNameIDs[] = { IDS_COL_ID, IDS_COL_START_TIME, IDS_COL_ELAPSED_TIME };
			UINT columnWidths[] = { 40, 100, 100};
			HWND lv = GetDlgItem(hDlg, IDC_DISCONNECT_LIST);
			LPTSTR temp = (LPTSTR)malloc(sizeof(TCHAR) * TEMP_BUF_SIZE);
			int i;

			// Turn on full-row select for comctl32 v4.70 and newer
			ListView_SetExtendedListViewStyle(lv, LVS_EX_FULLROWSELECT);

			// Configure columns
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.fmt = LVCFMT_LEFT; 
			lvc.cx = 150; 
			lvc.pszText = temp; 

			// Add the columns. 
			for (i = 0; i < 3; i++) { 
				lvc.iSubItem = i; 
				lvc.cx = columnWidths[i];

				LoadString(g_hInstance, columnNameIDs[i], temp, TEMP_BUF_SIZE); 

				if (ListView_InsertColumn(lv, i, &lvc) == -1) 
					break; 
			}

			ListView_DeleteAllItems(lv);


			LV_ITEM lvi; 
			memset(&lvi, NULL, sizeof(LV_ITEM));
			lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM; // LVIF_IMAGE |; 
			lvi.state = 0; 
			lvi.stateMask = 0;

			ConnectionProfile*	profile = GetSelectedProfile(g_hwndConnectionList);
			KermitInstance *inst = KermitInstance::getInstanceByProfile(profile);
			i = 0;

			while (inst != NULL) {
				_sntprintf(temp, TEMP_BUF_SIZE, TEXT("%d"), inst->instanceId()); 

				lvi.iImage = 0;   // TODO: Icon
				lvi.iItem = i;
				lvi.pszText = temp;
				lvi.lParam = (LPARAM)inst;
				ListView_InsertItem(lv, &lvi); 

				i++;
				inst = inst->nextInstanceWithSameProfile(TRUE);
			}
			
			free(temp);

		}
		break;
	case WM_COMMAND:
		{
			int cmd = LOWORD(wParam);

			switch(cmd) {
			case IDOK:
				{
					HWND lv = GetDlgItem(hDlg, IDC_DISCONNECT_LIST);

					if (IsDlgButtonChecked(hDlg, IDC_DISCONNECT_ALL) == BST_CHECKED) {
						// Disconnect all!

						LV_ITEM item;

						int idx = SendMessage(lv, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_ALL);

						while (idx != -1) {
							item.iItem = idx;
							item.iSubItem = 0;
							item.mask = LVIF_PARAM;
							ListView_GetItem(lv, (LV_ITEM*)&item);

							if (item.iItem == -1)
								return NULL;

							KermitInstance *inst = (KermitInstance*)item.lParam;

							inst->requestDisconnect();
					
							// Find the next item
							idx = SendMessage(lv, LVM_GETNEXTITEM, (WPARAM)idx, LVNI_ALL);
						}

					} else {
						// Disconnect only those that have been selected.
						
						int idx = SendMessage(lv, 
							LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);

						LV_ITEM selected;
						while (idx != -1) {
							selected.iItem = idx;
							selected.iSubItem = 0;
							selected.mask = LVIF_PARAM;
							ListView_GetItem(lv, (LV_ITEM*)&selected);

							if (selected.iItem == -1)
								return NULL;

							KermitInstance *inst = (KermitInstance*)selected.lParam;

							inst->requestDisconnect();

							// See if there is another selected item
							idx = SendMessage(lv, LVM_GETNEXTITEM, (WPARAM)idx, 
								LVNI_SELECTED);
						}
					}

					EndDialog(hDlg, 0);
				}
			case IDCANCEL:
				EndDialog(hDlg, 0);
				return TRUE;

			case IDC_DISCONNECT_ALL:
				{
					EnableWindow(
						GetDlgItem(hDlg, IDC_DISCONNECT_LIST),
						IsDlgButtonChecked(hDlg, IDC_DISCONNECT_ALL) != BST_CHECKED
						);
				}
				break;
			}

		}
		break;

		
	case WM_NOTIFY: 
		{
			LPNMHDR pnmh = (LPNMHDR) lParam ;
			
			switch(pnmh->code) {
			case LVN_GETDISPINFO: 
				{
					LV_DISPINFO *lpdi = (LV_DISPINFO *)lParam;

					if(lpdi->item.iSubItem)	{
						if(lpdi->item.mask & LVIF_TEXT)	{
							KermitInstance *inst = (KermitInstance*)lpdi->item.lParam;

							switch(lpdi->item.iSubItem) {
							case 1:
								{
									// Connect time
									int mon = 0, dd = 0, hh = 0, mm = 0, ss = 0;

									SYSTEMTIME connected = inst->connectTime();
									SYSTEMTIME localTime;

									LPTSTR buf[20];

									// DD/MM HH:SS:mm  = 14 chars max

									if (SystemTimeToTzSpecificLocalTime(NULL, &connected,
											&localTime)) {
										_sntprintf((LPTSTR)buf, 20, 
											TEXT("%d/%02d %02d:%02d:%02d"),
											localTime.wDay,
											localTime.wMonth,
											localTime.wHour,
											localTime.wMinute,
											localTime.wSecond
											);
										lstrcpy(lpdi->item.pszText, (LPTSTR)buf);
									}
								}
								break;
							case 2:
								{
									// Elapsed time
									int dd = 0, hh = 0, mm = 0, ss = 0;
									int elapsed = inst->elapsedConnectTime();
									LPTSTR buf[20];

									if (elapsed > 0) {
										dd =  elapsed / 86400;
										elapsed -= 86400 * dd;
										hh = elapsed / 3600;
										elapsed -= 3600 * hh;
										mm = elapsed / 60;
										elapsed -= mm * 60;
										ss = elapsed;
									}

									if (dd > 0) {
										_sntprintf((LPTSTR)buf, 20, TEXT("%d:%02d:%02d:%02d"),
											dd, hh, mm, ss);
									} else {
										_sntprintf((LPTSTR)buf, 20, TEXT("%d:%02d:%02d"),
											hh, mm, ss);
									}

									lstrcpy(lpdi->item.pszText, (LPTSTR)buf);
								}
								break;
							}

						} else if(lpdi->item.mask & LVIF_IMAGE) {
							lpdi->item.iImage = 0;
						}
					}
					return 0;
				}

			case NM_DBLCLK: 
				{	// Double click
					LV_ITEM selected;
					HWND lv = GetDlgItem(hDlg, IDC_DISCONNECT_LIST);

					int idx = SendMessage(lv, 
						LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);

					selected.iItem = idx;
					selected.iSubItem = 0;
					selected.mask = LVIF_PARAM;
					ListView_GetItem(lv, (LV_ITEM*)&selected);

					if (selected.iItem == -1)
						return NULL;

					KermitInstance *inst = (KermitInstance*)selected.lParam;

					inst->requestDisconnect();

					EndDialog(hDlg, 0);
					return TRUE;
				}
				
			}
			
			return 0 ;
		}
	}

	return FALSE;
}

void LaunchKermit(HWND hwndParent, BOOL k95g) {
	const int BUFFERSIZE = 3000;
	LPTSTR command = (LPTSTR)malloc(sizeof(TCHAR) * BUFFERSIZE);

	BOOL consoleCreated = FALSE;

	STARTUPINFO si ;
    memset( &si, 0, sizeof(STARTUPINFO) ) ;
    si.cb = sizeof(STARTUPINFO);

	if (hwndParent != NULL) {
		_sntprintf(command, BUFFERSIZE, TEXT("%s -W %d %d"),
			k95g ? TEXT("k95g.exe") : TEXT("k95.exe"),
			hwndParent,
			KermitInstance::nextInstanceId());
	} else {
		_sntprintf(command, BUFFERSIZE,
			k95g ? TEXT("k95g.exe") : TEXT("k95.exe"));
	}

	if (!k95g) {
		// Apparently Windows 95 doesn't create console windows properly when
		// starting applications via CreateProcess so we've got to do it ourselves
		// there.
		OSVERSIONINFO vi ;
		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
		GetVersionEx( &vi ) ;

		if ( vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
			AllocConsole();
			consoleCreated = TRUE;

			HANDLE hOut, hIn;

			hOut = CreateFile(
				TEXT("CONOUT$"),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				0) ;
			hIn = CreateFile(
				TEXT("CONIN$"),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				0) ;

			si.dwFlags = (DWORD) STARTF_USESTDHANDLES;

			DuplicateHandle(
				GetCurrentProcess(),
				hOut,
				GetCurrentProcess(),
				&si.hStdOutput,
				DUPLICATE_SAME_ACCESS,
				TRUE,
				DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS
				);

			si.hStdError = si.hStdOutput;
			DuplicateHandle(
				GetCurrentProcess(),
				hIn,
				GetCurrentProcess(),
				&si.hStdInput,
				DUPLICATE_SAME_ACCESS, TRUE,
				DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS
				) ;
		}

		si.dwFlags |= (DWORD) STARTF_USECOUNTCHARS
				   | STARTF_USEFILLATTRIBUTE
				   | STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOWMAXIMIZED;
    }

	// Ready to launch Kermit!

	PROCESS_INFORMATION pi;

	BOOL rc = CreateProcess(
			(LPTSTR)NULL,		// executable (if not in command line)
			command,						// command line
			(LPSECURITY_ATTRIBUTES)NULL,	// process attributes
			(LPSECURITY_ATTRIBUTES)NULL,	// thread attributes
			FALSE,							// inherit handles
			(DWORD) CREATE_NEW_PROCESS_GROUP, // creation flags - probably only important for console
			(LPVOID)NULL,					// environment
			(LPTSTR)NULL,					// current directory
			&si,							// startup info
			&pi								// process info
			);


	if (consoleCreated) {
		FreeConsole();
	}

	free(command);

	if (rc && pi.dwProcessId != NULL) {
		KermitInstance *inst = new KermitInstance(NULL, pi.dwProcessId, NULL);
	}
}