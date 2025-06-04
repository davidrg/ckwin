#include <windows.h>
#include <commctrl.h>
#include <string.h>

#include "resource.h"
#include "conn_list.h"
#include "statusbar.h"
#include "toolbar.h"
#include "kerm_track.h"
#include "util.h"
#include "jumplist.h"

#define COLUMNS 3

static const UINT ColumnNames[] = { IDS_COL_NAME, IDS_COL_DESCRIPTION, IDS_COL_TARGET };

void ProfileSelected(BOOL, BOOL);

typedef struct myitem_tag { 
    LPTSTR aCols[COLUMNS]; 
} MYITEM; 
 
HWND hwndLV, hwndParentWindow;

void ConnectProfile(HWND hwndParent, ConnectionProfile *profile);

HWND WINAPI CreateConnectionListView(HWND hwndParent, HINSTANCE hInstance) 
{ 
    TCHAR g_achTemp[256];     // temporary buffer 
    
	LV_COLUMN lvc; 
    int iCol;

	hwndParentWindow = hwndParent;
 
    // Create the list view window.  
    hwndLV = CreateWindowEx(
		WS_EX_CLIENTEDGE,	// Extended style
		WC_LISTVIEW,		// Class name
		TEXT(""),			// Window name
        WS_CHILD | LVS_REPORT | LVS_EDITLABELS,	// Window style 
        0,					// horizontal position
		0,					// vertical position
		CW_USEDEFAULT,		// height
		CW_USEDEFAULT,		// width
        hwndParent,			// parent
		NULL,				// menu
		hInstance,			// application instance
		NULL);				// window creation data

	// Turn on full-row select for comctl32 v4.70 and newer
	ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_FULLROWSELECT);

    if (hwndLV == NULL) 
        return NULL; 
 
	// Configure columns
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
    lvc.fmt = LVCFMT_LEFT; 
    lvc.cx = 150; 
    lvc.pszText = g_achTemp; 
 
    // Add the columns. 
    for (iCol = 0; iCol < COLUMNS; iCol++) { 
        lvc.iSubItem = iCol; 
        LoadString(hInstance, ColumnNames[iCol], 
                g_achTemp, sizeof(g_achTemp)); 
        if (ListView_InsertColumn(hwndLV, iCol, &lvc) == -1) 
            break; 
    }

	ShowWindow(hwndLV, SW_SHOW);
 
    return hwndLV;              // return the control's handle 
} 


void AddConnections(HWND hwnd, ConfigFile *cfg) {
	LV_ITEM lvi;

	ListView_DeleteAllItems(hwnd);
	ListView_SetItemCount(hwnd, cfg->profileCount());

	memset(&lvi, NULL, sizeof(LV_ITEM));
    lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM; // LVIF_IMAGE |; 
    lvi.state = 0; 
    lvi.stateMask = 0; 

	ConnectionProfile *profile = cfg->firstProfile();
	while(profile != NULL) {
		if (profile != NULL) {
			lvi.iImage = 0;   // TODO: Icon
			lvi.iItem = profile->id();
			//if (!profile->name().isNullOrWhiteSpace()) {
			//	lvi.pszText = profile->name().data();
			//}
			lvi.pszText = LPSTR_TEXTCALLBACK;
			lvi.lParam = (LPARAM)profile;
			ListView_InsertItem(hwnd, &lvi); 
		}
		profile = profile->nextProfile();
	}
}

void AddConnection(ConnectionProfile *profile) {
	LV_ITEM lvi; 
	memset(&lvi, NULL, sizeof(LV_ITEM));

	CMString name = profile->name();

    lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM; // LVIF_IMAGE |; 
    lvi.state = 0; 
    lvi.stateMask = 0; 
	lvi.iImage = 0;   // TODO: Icon
	lvi.iItem = profile->id();
	lvi.pszText = name.data();
	lvi.lParam = (LPARAM)profile;
	int idx = ListView_InsertItem(hwndLV, &lvi); 

	if (idx == -1) {
		MessageBox(hwndParentWindow, 
			TEXT("Failed to insert list item."), 
			TEXT("Error"), MB_OK | MB_ICONWARNING);
	}
}


LRESULT ConnectionListViewNotify(HWND hwndListView, 
								 LPARAM lParam, 
								 ConfigFile *cfg) {

	LPNMHDR  lpnmh = (LPNMHDR) lParam;

	switch(lpnmh->code) {
	case LVN_GETDISPINFO: {
			LV_DISPINFO *lpdi = (LV_DISPINFO *)lParam;

			ConnectionProfile *profile = (ConnectionProfile *)lpdi->item.lParam;

			if(lpdi->item.iSubItem)	{
				if(lpdi->item.mask & LVIF_TEXT)	{
					switch(lpdi->item.iSubItem) {
					case 1:
						if (!profile->description().isNull())
							lstrcpy(lpdi->item.pszText, profile->description().data());
						break;
					case 2:
						if (profile->targetName())
							lstrcpy(lpdi->item.pszText, profile->targetName());
						break;
					case 3:
						lstrcpy(lpdi->item.pszText, 
							   TEXT("Unused column"));
						break;

					}
				}
			} else {
				if(lpdi->item.mask & LVIF_TEXT) {
					if (!profile->name().isNullOrWhiteSpace()) {
						lstrcpy(lpdi->item.pszText, profile->name().data());
					}
				}

				if(lpdi->item.mask & LVIF_IMAGE) {
					lpdi->item.iImage = 0;
				}
			}
		}
		return 0;

	case LVN_ITEMCHANGED:
	case NM_CLICK: {	// Single click
			/* If we were willing to take a dependency on commctl32 v4.71 (introduced
			 * with Internet Explorer 4.0) we could just cast lParam to LPNMITEMACTIVATE
			 * which contains the item Id and its lparam among other things.
			 *
			 * MIPS and PowerPC are limited to commctl32 v4.70 though, which is why
			 * we're doing things in a more complicated way here.
			 */
			ConnectionProfile *profile = GetSelectedProfile(hwndListView);
        
			StatusSetCurrentConnection(profile);

			if (profile != NULL) {	
				KermitInstance *inst = KermitInstance::getInstanceByProfile(profile);

				ProfileSelected(TRUE, 
					inst != NULL && inst->status() == KermitInstance::S_CONNECTED
					);
				
			} else {
				// No profile selected. Disable menu & toolbar items

				ProfileSelected(FALSE, FALSE);
			}
			
		}
		break;
	case NM_DBLCLK: 
		{	// Double click
			ConnectSelectedProfile(hwndListView, hwndParentWindow);
		}
		
		break;
	}

	return 0;
}
 

ConnectionProfile*	GetSelectedProfile(HWND hwndListView) {
	LV_ITEM selected;

	int idx = SendMessage(hwndListView, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
	
	selected.iItem = idx;
	selected.iSubItem = 0;
	selected.mask = LVIF_PARAM;
    ListView_GetItem(hwndListView, (LV_ITEM*)&selected);
    
	if (selected.iItem == -1)
		return NULL;

	return (ConnectionProfile*)selected.lParam;
}

// Enabled and disables toolbar buttons and menu items
// based on whether a profile is currently selected and
// if it is currently connected or not.
void ProfileSelected(BOOL selected, BOOL isConnected) {
	HMENU menu = GetMenu(hwndParentWindow);
	UINT menuState = selected ? MF_ENABLED : MF_GRAYED;

	SetToolbarButtonEnabled(ID_TB_CONNECT, selected);
	SetToolbarButtonEnabled(ID_TB_DISCONNECT, isConnected);
	SetToolbarButtonEnabled(ID_TB_SHORTCUT, selected);
	SetToolbarButtonEnabled(ID_TB_COPY, selected);
	SetToolbarButtonEnabled(ID_TB_PROPERTIES, selected);
	SetToolbarButtonEnabled(ID_TB_DELETE, selected);
	
	EnableMenuItem(menu, ID_CONNECTION_CLONE, menuState);
	EnableMenuItem(menu, ID_CONNECTION_DELETE, menuState);
	EnableMenuItem(menu, ID_CONNECTION_PROPERTIES, menuState);
	EnableMenuItem(menu, ID_CONNECTION_CONNECT, menuState);
	EnableMenuItem(menu, ID_CONNECTION_SHORTCUT, menuState);

	EnableMenuItem(menu, ID_CONNECTION_DISCONNECT, 
		isConnected? MF_ENABLED : MF_GRAYED);
}


void ConnectProfile(HWND hwndParent, ConnectionProfile *profile) {
	DWORD processId = profile->connect(hwndParent);

	if (processId != NULL) {
		KermitInstance *inst = KermitInstance::getInstanceByProcessId(processId);

		if (inst != NULL) {
			// We already know about this instance - must have
			// taken it over with a new connection
			inst->setStatus(KermitInstance::S_CONNECTING);
			inst->setProfile(profile);
		} else {
			// We launched a new kermit instance. Start tracking.
			// It's window handle won't be known until it begins
			// starting.
			inst = new KermitInstance(NULL, processId, profile);
		}

		// Update the MRU list
		extern ConfigFile *g_ConfigFile;
		g_ConfigFile->setMostRecentlyUsedProfile(profile);
		g_ConfigFile->commitChanges();

		// And update the Jump list if we're on Windows 7 or newer
		UpdateJumpList();
	}
}

void ConnectSelectedProfile(HWND hwndConnectionList, HWND hwndParent) {
	ConnectionProfile *profile = GetSelectedProfile(hwndConnectionList);

	StatusSetCurrentConnection(profile);

	if (profile != NULL) {
		ProfileSelected(TRUE, TRUE);
	
		ConnectProfile(hwndParent, profile);
	} else {
		// Nothing selected
		ProfileSelected(FALSE, FALSE);
	}
}

void ConnectProfileId(HWND hwndParent, int profileId) {
	LV_ITEM lvi;
	memset(&lvi, 0, sizeof(lvi));
	lvi.mask = LVIF_PARAM;

	// Search the list for the profile...
	int itemCount = ListView_GetItemCount(hwndLV);
	for (int i = 0; i < itemCount; i++) {
		lvi.iItem = i;

		if (ListView_GetItem(hwndLV, &lvi)) {
			ConnectionProfile *profile = (ConnectionProfile *)lvi.lParam;
			if (profile->id() == profileId) {
				ConnectProfile(hwndParent, profile);
				return;
			}
		}
	}

}