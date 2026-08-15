#ifndef STATUSBAR_H
#define STATUSBAR_H

#include "conn_profile.h"

HWND		CreateStatusBar(HWND, HINSTANCE);
LRESULT		StatusBarMenuSelect(HWND, HINSTANCE, WPARAM, LPARAM);
void		StatusKermitInstanceExiting(int instanceId);
void		StatusSetCurrentConnection(ConnectionProfile *);
void		StatusRefresh();

#endif /* STATUSBAR_H */