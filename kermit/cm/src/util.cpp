#include "util.h"

// UTF-8 requires Windows NT 4.0 or Windows 98 or newer.
// Its definition doesn't appear in Visual C++ 4.0, but it is
// present in 4.2. Included here for compatibility with 4.0.
#ifndef CP_UTF8
#define CP_UTF8              65001          /* UTF-8 translation */
#endif /* CP_UTF8 */

#define MAX_DATA_LEN 50

AppVersion GetAppVersion(HINSTANCE hInstance) {
	AppVersion result;
	DWORD verHandle = 0, verSize = 0;
	UINT size = 0;
	VS_FIXEDFILEINFO *verInfo;
	LPSTR verData;
	
	LPTSTR buf = (LPTSTR)malloc(sizeof(TCHAR) * MAX_PATH );
	
	ZeroMemory(buf, sizeof(TCHAR) * MAX_PATH );
	
	GetModuleFileName(hInstance, buf, MAX_PATH );

	verSize = GetFileVersionInfoSize( buf, &verHandle);
	verData = (LPSTR)malloc(verSize);
	
	if (GetFileVersionInfo( buf, verHandle, verSize, verData)) {
		if (VerQueryValue(verData,TEXT("\\"),(LPVOID *)&verInfo,&size)) {
			if (size) {
				if (verInfo->dwSignature == 0xfeef04bd) {
					result.major = ( verInfo->dwFileVersionMS >> 16 ) & 0xffff;
					result.minor = ( verInfo->dwFileVersionMS >>  0 ) & 0xffff;
					result.revision = ( verInfo->dwFileVersionLS >> 16 ) & 0xffff;
					result.build = ( verInfo->dwFileVersionLS >>  0 ) & 0xffff;
				}
			}
		}
	}
	
	free(verData);

	return result;
}


int fromUtf8(LPSTR input, LPTSTR output, int outputSizeInChars) {
#ifdef UNICODE
	return MultiByteToWideChar(
		CP_UTF8, 
		0,			// Must be 0 for CP_UTF8
		input,
		-1,			// -1 for null terminated string
		output,
		outputSizeInChars);
#else
	int inputLen = strlen(input);
	int len = min((outputSizeInChars - 1), inputLen);

	strncpy(input, output, outputSizeInChars);
	output[len] = '\0';  // Make sure its null terminated

	return 1; // non-zero for success
#endif
}

int toUtf8(LPTSTR input, LPSTR output, int outputSizeInChars) {
#ifdef UNICODE
	return WideCharToMultiByte(
		CP_UTF8,
		0,			// Must be 0 for CP_UTF8
		input,
		-1,			// -1 for null terminated string
		output,
		outputSizeInChars,
		NULL,		// Must be NULL for CP_UTF8
		NULL		// Must be NULL for CP_UTF8
		);
#else
	int inputLen = strlen(input);
	int len = min((outputSizeInChars - 1), inputLen);

	strncpy(input, output, outputSizeInChars);
	output[len] = '\0';  // Make sure its null terminated

	return 1; // non-zero for success
#endif
}


void loadSerialPortDropdown(HWND hwndDlg, int controlId) {
	HWND hwndSerialPort = GetDlgItem(hwndDlg, controlId);

	// Now try to populate the list of serial ports. This only works on NT.
	HKEY hKey = NULL;
	TCHAR tempData[MAX_DATA_LEN], tempName[MAX_DATA_LEN];
	DWORD tempDataLen = MAX_DATA_LEN, tempNameLen = MAX_DATA_LEN;
	DWORD dwIndex = 0;
	ZeroMemory(tempData, MAX_DATA_LEN);
	ZeroMemory(tempName, MAX_DATA_LEN);

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, 
					 KEY_READ, &hKey) == ERROR_SUCCESS) {
		while (TRUE) {
			LONG rc = RegEnumValue(hKey, dwIndex, tempName, &tempNameLen,NULL, NULL, 
							(unsigned char*)tempData, &tempDataLen);

			if (rc == ERROR_NO_MORE_ITEMS) {
				break;
			} 

			SendMessage(hwndSerialPort,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) tempData);
			dwIndex++;
		}
	} else {
		// Couldn't find the registry key. Either no serial ports, or not on NT
		// (95 doesn't seem to provide this info via the registry)

		// We *could* brute-force it if we really cared - just try to CreateFile
		// COM1..COM10 and if it succeeds add it to the list.
	}
}