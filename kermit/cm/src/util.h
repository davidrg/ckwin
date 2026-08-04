#ifndef UTIL_H
#define UTIL_H

#include <windows.h>
#include <commctrl.h>

typedef struct tagAppVersion {
	int major;
	int minor;
	int revision;
	int build;
} AppVersion;

AppVersion GetAppVersion(HINSTANCE hInstance);

// List view full row select extended style
// ----------------------------------------
// ListView_SetExtendedListViewStyle and LVS_EX_FULLROWSELECT are new in Comctl32 v4.70
// which was distributed with Windows 95 OSR2, IE 3.01, 3.02 and Windows NT 4.0.
// 
// This means that SDK support was added in Visual C++ 5.0. Problem is, Visual C++ 5.0
// doesn't support all the platforms that Windows NT 4.0 supports (MIPS and PowerPC).
// So these definitions let us enable Full Row Select when building with Visual C++ 4.0.
// 
#ifndef LVS_EX_FULLROWSELECT
#define LVS_EX_FULLROWSELECT	0x00000020
#endif /* LVS_EX_FULLROWSELECT */

#ifndef LVM_SETEXTENDEDLISTVIEWSTYLE
#define LVM_SETEXTENDEDLISTVIEWSTYLE (LVM_FIRST + 54)
#define ListView_SetExtendedListViewStyle(hwndLV, dw)\
        (DWORD)SNDMSG((hwndLV), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dw)
#endif /* LVM_SETEXTENDEDLISTVIEWSTYLE */


/* The JSON library we use only supports the UTF-8 Unicode encoding, while
 * unicode versions of this app use UTF-16 internally. These functions are
 * for converting between the two. They require Windows NT 4.0 or newer.
 */

int fromUtf8(LPSTR input, LPTSTR output, int outputSizeInChars);
int toUtf8(LPTSTR input, LPSTR output, int outputSizeInChars);

void loadSerialPortDropdown(HWND hwndDlg, int controlId);

#endif /* UTIL_H */