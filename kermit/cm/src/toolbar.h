#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <windows.h>

HWND	CreateToolBar(HWND, HINSTANCE);
LRESULT ToolbarNotify(HINSTANCE, HWND, WPARAM, LPARAM);
void	GetToolbarToolTipText(LPTOOLTIPTEXT, HINSTANCE);
void    SetToolbarButtonEnabled(WORD buttonId, BOOL enabled);

#endif /* TOOLBAR_H */