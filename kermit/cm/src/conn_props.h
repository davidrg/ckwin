#ifndef CONN_PROPS_H
#define CONN_PROPS_H

#include "conn_profile.h"

int				DoPropSheet(HWND, HINSTANCE, ConnectionProfile*);

// Utility functions
BOOL			textFieldChanged(HWND hwndField, LPTSTR originalValue);
LPTSTR			getFieldText(HWND hwndDlg, int controlId);
int				getFieldInt(HWND hwndDlg, int id);
void			setFieldInt(HWND hwndDlg, int id, int value);

// Property Sheet: GENERAL
BOOL CALLBACK	GeneralPageDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK	GeneralPageProc(HWND, UINT, LPPROPSHEETPAGE);

// Property Sheet: CONNECTION (Serial)
BOOL CALLBACK	SerialConnectionPageDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK	SerialConnectionPageProc(HWND, UINT, LPPROPSHEETPAGE);

// Property Sheet: CONNECTION (all others)
BOOL CALLBACK	ConnectionPageDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK	ConnectionPageProc(HWND, UINT, LPPROPSHEETPAGE);

// Property Sheet: TERMINAL
BOOL CALLBACK	TerminalPageDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK	TerminalPageProc(HWND, UINT, LPPROPSHEETPAGE);

// Property Sheet: TERMINAL COLORS
BOOL CALLBACK	TermColorPageDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK	TermColorPageProc(HWND, UINT, LPPROPSHEETPAGE);

// Property Sheet: GUI COLORS
BOOL CALLBACK	GuiColorPageDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK	GuiColorPageProc(HWND, UINT, LPPROPSHEETPAGE);

// Property Sheet: LOGIN
BOOL CALLBACK	LoginPageDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK	LoginPageProc(HWND, UINT, LPPROPSHEETPAGE);

// Property Sheet: KEYBOARD
BOOL CALLBACK	KeyboardPageDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK	KeyboardPageProc(HWND, UINT, LPPROPSHEETPAGE);

#endif /* CONN_PROPS_H */