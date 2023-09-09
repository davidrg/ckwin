#ifndef CONN_PROPS_H
#define CONN_PROPS_H

#include "conn_profile.h"

int				DoPropSheet(HWND, HINSTANCE, ConnectionProfile*);

// Property Sheet: GENERAL
BOOL CALLBACK	GeneralPageDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK	GeneralPageProc(HWND, UINT, LPPROPSHEETPAGE);

#endif /* CONN_PROPS_H */