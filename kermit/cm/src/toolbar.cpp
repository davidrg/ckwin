#include <windows.h>
#include <commctrl.h>

#include "toolbar.h"
#include "resource.h"

/* TODO:
 *	-> Save toolbar customisation
 *  -> Add help for the toolbar customisation dialog help button
 */

// Bitmaps (in the IDR_TOOLBAR1 resource)
#define TB_BM_CONNECT		0
#define TB_BM_DISCONNECT	1
#define TB_BM_QUICK_CONNECT	2
#define TB_BM_SHORTCUT		3
#define TB_BM_NEW			4
#define TB_BM_COPY			5
#define TB_BM_PROPERTIES	6
#define TB_BM_DELETE		7

TBBUTTON buttons[] = {
	//                                                                           dwData
	// iBitmap          idCommand            fsState          fsStyle      bReserved   iString
	TB_BM_CONNECT,	    ID_TB_CONNECT,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, IDS_TB_CONNECT,
	TB_BM_DISCONNECT,   ID_TB_DISCONNECT,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, IDS_TB_DISCONNECT,
	TB_BM_QUICK_CONNECT,ID_TB_QUICK_CONNECT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, IDS_TB_QUICK_CONNECT,
	TB_BM_SHORTCUT,     ID_TB_SHORTCUT,      TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, IDS_TB_SHORTCUT,
    0,                  NULL,                TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, 0,
	TB_BM_NEW,          ID_TB_NEW,           TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, IDS_TB_NEW,
	TB_BM_COPY,         ID_TB_COPY,          TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, IDS_TB_COPY,
	TB_BM_PROPERTIES,   ID_TB_PROPERTIES,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, IDS_TB_PROPERTIES,
	TB_BM_DELETE,       ID_TB_DELETE,        TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, IDS_TB_DELETE,
};

static HWND hwndToolbar;

// Max length for a toolbar button string
#define TB_STRING_MAX	30


HWND CreateToolBar(HWND hwndParent, HINSTANCE hInstance) 
{ 
    TBADDBITMAP tbab; 
 
    // Create a toolbar that the user can customize and that has a 
    // tooltip associated with it. 
    hwndToolbar = CreateWindowEx(
		0,						// extended window style
		TOOLBARCLASSNAME,		// class name
		(LPTSTR) NULL,			// window name
        WS_CHILD | TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE,	// window style
        0,						// X position
		0,						// Y position
		0,						// Width
		0,						// Height
		hwndParent,				// parent
		(HMENU) IDR_TOOLBAR1,	// child window identifier
		hInstance,				// application instance
		NULL);					// window-creation data pointer
 
    // Send the TB_BUTTONSTRUCTSIZE message, which is required for 
    // backward compatibility. 
    SendMessage(hwndToolbar, TB_BUTTONSTRUCTSIZE, 
        (WPARAM) sizeof(TBBUTTON), 0); 
 
    // Add the bitmap containing button images to the toolbar. 
    tbab.hInst = hInstance; 
    tbab.nID   = IDR_TOOLBAR1; 
	SendMessage(hwndToolbar, TB_ADDBITMAP, 
		(WPARAM) sizeof(buttons) / sizeof(TBBUTTON), 
        (WPARAM) &tbab); 

	SendMessage(hwndToolbar, TB_ADDBUTTONS, 
		(WPARAM) sizeof(buttons) / sizeof(TBBUTTON), 
        (LPARAM) (LPTBBUTTON) &buttons);

	
	// The button size defaults to something weird so we
	// have to specify it
	SendMessage(hwndToolbar, TB_SETBUTTONSIZE, 
        0, (LPARAM)MAKELONG(16, 16));

    ShowWindow(hwndToolbar, SW_SHOW); 
    return hwndToolbar; 
}

LRESULT ToolbarNotify(HINSTANCE hInstance, HWND hWnd, WPARAM wParam, LPARAM lParam) {
	LPNMHDR pnmh = (LPNMHDR) lParam;
	int idCtrl = (int) wParam;

	// Allow toolbar customisation
	if ((pnmh->code == TBN_QUERYDELETE) ||
		(pnmh->code == TBN_QUERYINSERT)) {
		return 1;
	}

	// Provide details of all buttons for the
	// customise dialog
	if (pnmh->code == TBN_GETBUTTONINFO) {
		
		LPTBNOTIFY ptbn = (LPTBNOTIFY) lParam ;
		int idx = ptbn->iItem;

		if (idx < sizeof(buttons) / sizeof(TBBUTTON)) {
			TCHAR szBuf[TB_STRING_MAX];
			LoadString(hInstance, buttons[idx].iString, 
					   (LPTSTR) &szBuf, TB_STRING_MAX); 
			lstrcpy (ptbn->pszText, szBuf) ;
			memcpy (&ptbn->tbButton, &buttons[idx], sizeof (TBBUTTON)) ;
			return 1 ;
		}
	}

	if (pnmh->code == TBN_CUSTHELP) {
		// TODO: User clicked the Help button in the toolbar customisation dialog.
		//		 Give them some help.
	}

	if (pnmh->code == TBN_TOOLBARCHANGE) {
		// TODO: Save the toolbar state
	}

	return 0;
}


void GetToolbarToolTipText(LPTOOLTIPTEXT lpttt, HINSTANCE hInstance) {
	TCHAR szBuf[TB_STRING_MAX];

	for (int i = 0; i < sizeof(buttons) / sizeof(TBBUTTON); i++) {
		if ((UINT)buttons[i].idCommand == lpttt->hdr.idFrom) {
			LoadString(hInstance, buttons[i].iString, 
					   (LPTSTR) &szBuf, TB_STRING_MAX);
			lstrcpy(lpttt->lpszText, szBuf);
			return;
		}
	}
}


void SetToolbarButtonEnabled(WORD buttonId, BOOL enabled) {
	SendMessage(hwndToolbar, TB_ENABLEBUTTON, buttonId,
		(LPARAM)MAKELONG(enabled,0));
}