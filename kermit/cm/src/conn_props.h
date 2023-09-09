#ifndef CONN_PROPS_H
#define CONN_PROPS_H

#include "conn_profile.h"

int				DoPropSheet(HWND, HINSTANCE, ConnectionProfile*);

// Utility functions
BOOL			textFieldChanged(HWND hwndField, LPTSTR originalValue);

// Property Sheet: GENERAL
BOOL CALLBACK	GeneralPageDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK	GeneralPageProc(HWND, UINT, LPPROPSHEETPAGE);

// Property Sheet: TERMINAL
BOOL CALLBACK	TerminalPageDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK	TerminalPageProc(HWND, UINT, LPPROPSHEETPAGE);



#endif /* CONN_PROPS_H */