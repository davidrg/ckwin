#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>

#include "statusbar.h"
#include "resource.h"
#include "kerm_track.h"

static HWND hwndStatus;

static ConnectionProfile *currentProfile;
static KermitInstance *currentInstance;


// DoCreateStatusWindow - creates a status window and divides it into 
//     the specified number of parts. 
// Returns the handle to the status window. 
// hwndParent - parent window for the status window 
// nStatusID - child window identifier 
// hinst - handle to the application instance 
// nParts - number of parts into which to divide the status window 
HWND CreateStatusBar(HWND hwndParent, HINSTANCE hInstance) 
{ 

	// These aren't widths as such, they're the coordinates for the right
	// side of each panel with -1 signalling to extend all the way to the end.
	// So panel 1 is from 0-200, panel 2 is from 200-271, and panel 3 occupies
	// whatever space is left. Panel 2 should currently wide enough 
	// for ddd:hh:mm:ss
	int widths[] = {200, 271, -1};
 
    // Create the status window. 
    hwndStatus = CreateWindowEx( 
        0,                       // extended window style
        STATUSCLASSNAME,         // class name
        (LPCTSTR) NULL,          // window name
        SBARS_SIZEGRIP |         // window style - include size grib
        WS_CHILD,                // window style - child window
        0, 0, 0, 0,              // position (X, Y) and size (Width,Height)
        hwndParent,              // parent window
        (HMENU) IDC_STATUSBAR,   // child window identifier
        hInstance,               // application instance
        NULL);                   // window-creation data pointer
 
	

	SendMessage(hwndStatus, SB_SETPARTS, 3, (LPARAM)widths);
	ShowWindow(hwndStatus, SW_SHOW);

	StatusSetCurrentConnection(NULL);

	currentProfile = NULL;
	currentInstance = NULL;

    return hwndStatus; 
} 


LRESULT	StatusBarMenuSelect(HWND hwnd, HINSTANCE hInstance,
							WPARAM wParam, LPARAM lParam) {
	int stringBase = 0;

	MenuHelp(WM_MENUSELECT, wParam, lParam, NULL, hInstance, 
			 hwndStatus, (UINT *) &stringBase);
	

	/*UINT fuFlags = (UINT) HIWORD (wParam) ;
	HMENU hMainMenu = NULL ;
	int iMenu = 0 ;

	// Handle non-system popup menu descriptions.
	if ((fuFlags & MF_POPUP) && (!(fuFlags & MF_SYSMENU))) {
		for (iMenu = 1 ; iMenu < MAX_MENUS ; iMenu++) {
			if ((HMENU) lParam == popstr[iMenu].hMenu) {
				hMainMenu = (HMENU) lParam ;
				break ;
			}
		}
	}

	// Display helpful text in status bar
	MenuHelp(WM_MENUSELECT, wParam, lParam, hMainMenu, hInst, 
			 hwndStatus, (UINT *) &popstr[iMenu]) ;*/

	return 0 ;
}

void StatusKermitInstanceExiting(int instanceId) {
	if (currentInstance == NULL) return;

	if (currentInstance->instanceId() == instanceId) {
		// The instance we're currently tracking is exiting!;
		currentInstance = NULL;
	}
}

// Set the connection profile the status bar should be showing.
void StatusSetCurrentConnection(ConnectionProfile *profile) {
	if (currentProfile != NULL && 
		(profile == NULL || currentProfile->name() == profile->name())) {
		// We're already monitoring this profile. We'll trigger a refresh
		// but otherwise there is nothing to do.
		StatusRefresh();

		return;
	}

	// Profile has changed. Clear out the current kermit instance we're
	// monitoring and do a refresh to see what the status of the new
	// profile is.
	currentProfile = profile;
	currentInstance = NULL;

	StatusRefresh();
}

void StatusRefresh() {
	if (currentProfile == NULL) {
		// No connection profile selected.

		SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)TEXT(""));
		SendMessage(hwndStatus, SB_SETTEXT, 1, (LPARAM)TEXT("365:24:59:59"));
		SendMessage(hwndStatus, SB_SETTEXT, 2, (LPARAM)TEXT("No profile selected"));

	} else {
		// A profile is selected. Try and show its connection
		// status - if its connected.

		SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)currentProfile->name().data());

		if (currentInstance == NULL) {
			// We don't have a particular connection instance we're
			// tracking! Try find one.

			currentInstance = KermitInstance::getInstanceByProfile(currentProfile);
			
			if (currentInstance != NULL) {
				int connectElapsed = currentInstance->elapsedConnectTime();

				KermitInstance *inst = currentInstance->nextInstanceWithSameProfile(TRUE);

				// See if there are more instances connected to this profile. If
				// there are, we'll show the one with the longest connect time.
				while (inst != NULL) {
					int instElapsed = inst->elapsedConnectTime();

					if (instElapsed > connectElapsed) {
						// This instance has been connected longer than the last
						// one we found. We'll go with this if we find no other
						// longer connected options
						currentInstance = inst;
						connectElapsed = instElapsed;
					}

					// Keep searching.
					inst = inst->nextInstanceWithSameProfile(TRUE);
				}
			}			
		}
		
		if (currentInstance != NULL) {
			int dd = 0, hh = 0, mm = 0, ss = 0;
			int elapsed = currentInstance->elapsedConnectTime();
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

			SendMessage(hwndStatus, SB_SETTEXT, 1, (LPARAM)buf);

			switch(currentInstance->status()) {
			case KermitInstance::S_CONNECTING:
				SendMessage(hwndStatus, SB_SETTEXT, 2, (LPARAM)TEXT("Connecting..."));
				break;
			case KermitInstance::S_CONNECTED:
				SendMessage(hwndStatus, SB_SETTEXT, 2, (LPARAM)TEXT("Connected"));
				break;
			case KermitInstance::S_IDLE:
				SendMessage(hwndStatus, SB_SETTEXT, 2, (LPARAM)TEXT("Idle"));
				break;
			default:
				SendMessage(hwndStatus, SB_SETTEXT, 2, (LPARAM)TEXT("Unknown status"));
				break;
			}
		} else {
			SendMessage(hwndStatus, SB_SETTEXT, 1, (LPARAM)TEXT(""));
			SendMessage(hwndStatus, SB_SETTEXT, 2, (LPARAM)TEXT("Not connected"));
		}
	}
}