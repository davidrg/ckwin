#ifndef CONN_LIST_H
#define CONN_LIST_H

#include "config_file.h"

class ConnectionProfile;

HWND WINAPI			CreateConnectionListView(HWND, HINSTANCE);
void				AddConnections(HWND, ConfigFile *);
void				AddConnection(ConnectionProfile *);
LRESULT				ConnectionListViewNotify(HWND, LPARAM, ConfigFile *);
void				ProfileSelected(BOOL, BOOL);
ConnectionProfile*	GetSelectedProfile(HWND);
void                ConnectSelectedProfile(HWND, HWND);
void                ConnectProfileId(HWND hwndParent, int profileId);
#endif /* CONN_LIST_H */
