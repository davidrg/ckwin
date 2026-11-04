#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <wingdi.h>

#include "../conn_props.h"
#include "../conn_profile.h"
#include "../resource.h"
#include "../charset.h"

/******************
 * SHEET: Printer
 ******************/

/*
Controls:
IDC_PRINTER_TYPE		Drop-down list
IDC_CHARSET				Drop-down list
IDC_TARGET				Static
IDC_QUEUE				Drop-down list
IDC_DEVICE				Combo Box
IDC_FILE				Text box
IDC_BROWSE_OUT_FILE		Button
IDC_HEADER_FILE			Text box
IDC_BROWSE_HEADER		Button
IDC_TIMEOUT				Text box
IDC_TIMEOUT_SPIN		Spin button
IDC_SPEED				Drop-down list
IDC_PARITY				Drop-down list
IDC_FLOW				Drop-down list
IDC_BIDIRECTIONAL		Checkbox
IDC_PS_OUTPUT			Checkbox
IDC_PS_WIDTH			Text box
IDC_WIDTH_SPIN			Spin button
IDC_PS_HEIGHT			Text box
IDC_HEIGHT_SPIN			Spin button
IDC_SEND_FF				Radio button
IDC_SEND_STRING			Radio button
IDC_EOJ_STRING			Text box
 */

// Connection profile we're editing
static ConnectionProfile *profile;


typedef struct tagDialogFieldStatus {
	int id;
	BOOL dirty;
} DialogFieldStatus;


static const int fieldCount = 17;

static DialogFieldStatus fieldStatus[] = {
	{ IDC_PRINTER_TYPE,		FALSE	},
	{ IDC_CHARSET,			FALSE	},
	{ IDC_QUEUE,			FALSE	},
	{ IDC_DEVICE,			FALSE	},
	{ IDC_FILE,				FALSE	},
	{ IDC_HEADER_FILE,		FALSE	},
	{ IDC_TIMEOUT,			FALSE	},
	{ IDC_SPEED,			FALSE	},
	{ IDC_PARITY,			FALSE	},
	{ IDC_FLOW,				FALSE	},
	{ IDC_BIDIRECTIONAL,	FALSE	},
	{ IDC_PS_OUTPUT,		FALSE	},
	{ IDC_PS_WIDTH,			FALSE	},
	{ IDC_PS_HEIGHT,		FALSE	},
	{ IDC_SEND_FF,			FALSE	},
	{ IDC_SEND_STRING,		FALSE	},
	{ IDC_EOJ_STRING,		FALSE	}
};


typedef struct tagDropListOption {
	int id;
	LPCTSTR string;
} DropListOption;

static DropListOption printerTypeOptions[] = {
	{ ConnectionProfile::PT_NONE, TEXT("None") },
	{ ConnectionProfile::PT_WINDOWS, TEXT("Windows Printer Queue") },
	{ ConnectionProfile::PT_DOS_DEVICE, TEXT("DOS Device") },
	{ ConnectionProfile::PT_FILE, TEXT("File") },
	{ ConnectionProfile::PT_COMMAND, TEXT("Command (Pipe)") },
	{ 0, 0 }
};

static DropListOption deviceOptions[] = {
	{ 0, TEXT("com1") },
	{ 1, TEXT("com2") },
	{ 2, TEXT("com3") },
	{ 3, TEXT("com4") },
	{ 4, TEXT("kermit.prn") },
	{ 5, TEXT("lpt1") },
	{ 6, TEXT("lpt2") },
	{ 7, TEXT("lpt3") },
	{ 8, TEXT("prn") },
	{ 0, 0 }
};


static DropListOption speedOptions[] = {
	{ 50, TEXT("50") },
	{ 75, TEXT("75") },
	{ 110, TEXT("110") },
	{ 150, TEXT("150") },
	{ 300, TEXT("300") },
	{ 600, TEXT("600") },
	{ 1200, TEXT("1200") },
	{ 2400, TEXT("2400") },
	{ 3600, TEXT("3600") },
	{ 4800, TEXT("4800") },
	{ 7200, TEXT("7200") },
	{ 9600, TEXT("9600") },
	{ 14400, TEXT("14400") },
	{ 19200, TEXT("19200") },
	{ 28800, TEXT("28800") },
	{ 38400, TEXT("38400") },
	{ 57600, TEXT("57600") },
	{ 76800, TEXT("76800") },
	{ 115200, TEXT("115200") },
	{ 230400, TEXT("230400") },
	{ 460800, TEXT("460800") },
	{ 0, 0 }
};

static DropListOption flowControlOptions[] = {
	{ ConnectionProfile::FC_NONE, TEXT("None") },
	{ ConnectionProfile::FC_RTS_CTS, TEXT("Rts/Cts") },
	{ ConnectionProfile::FC_XON_XOFF, TEXT("Xon/Xoff") },
	{ 0, 0 }
};

static DropListOption parityOptions[] = {
	{ ConnectionProfile::PAR_NONE, TEXT("None") },
	{ ConnectionProfile::PAR_SPACE, TEXT("Space") },
	{ ConnectionProfile::PAR_EVEN, TEXT("Even") },
	{ ConnectionProfile::PAR_ODD, TEXT("Odd") },
	{ ConnectionProfile::PAR_MARK, TEXT("Mark") },
	{ 0, 0 }
};


static BOOL isDirty(int id) {
	for (int i = 0; i < fieldCount; i++ ) {
		if (fieldStatus[i].id == id) {
			return fieldStatus[i].dirty;
		}
	}

	return FALSE; // Should never happen.
}

static void setDirty(int id, BOOL dirty) {
	for (int i = 0; i < fieldCount; i++ ) {
		if (fieldStatus[i].id == id) {
			fieldStatus[i].dirty = dirty;
			return;
		}
	}
}


static BOOL isDirty() {
	for (int i = 0; i < fieldCount; i++ ) {
		if (fieldStatus[i].dirty) {
			return TRUE;
		}
	}
	return FALSE;
}


static void cleanForm() {
	for (int i = 0; i < fieldCount; i++ ) {
		fieldStatus[i].dirty = FALSE;
	}
}

// Call whenever a fields value is changed to toggle the
// apply button on and off as necessary.
static void FieldChanged(HWND hwndDlg) {
	if (isDirty()) {
		PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
	} else {
		PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);
	}
}

static void InsertDropListItems(HWND hwndDlg, int resourceId, DropListOption options[]) {
	HWND hwndCtl = GetDlgItem(hwndDlg, resourceId);

	for (int i = 0; options[i].string != NULL; i++) {
		int idx = ComboBox_AddString(hwndCtl, options[i].string);
		ComboBox_SetItemData(hwndCtl, idx, options[i].id);
	}
}

// Selecting options by item data seems to be unreliable (crashes), so instead we
// do it this way.
static void SelectDropListItem(HWND hwndDlg, int resourceId, DropListOption options[], int optionId) {
	HWND hwndCtl = GetDlgItem(hwndDlg, resourceId);

	for (int i = 0; options[i].string != NULL; i++) {
		if (options[i].id == optionId) {
			ComboBox_SelectString(hwndCtl, 0, options[i].string);
			return;
		}
	}
}

static int GetSelectedDropListId(HWND hwnd, DropListOption options[]) {
	int selectedOptionId = -1;
	int length = ComboBox_GetTextLength(hwnd) + 1;

	LPTSTR text = (LPTSTR)malloc(length * sizeof(TCHAR));
	ComboBox_GetText(hwnd, text, length);
	for (int i = 0; options[i].string != NULL; i++) {
		if (lstrcmp(text, options[i].string) == 0) {
			selectedOptionId = options[i].id;
			break;
		}
	}
	free(text);

	return selectedOptionId;
}

static int CheckDropList(HWND hwndDlg, int resourceId, DropListOption options[], int savedOptionId, BOOL save) {
	HWND hwnd = GetDlgItem(hwndDlg, resourceId);

	int idx = ComboBox_GetCurSel(hwnd);

	int savedIdx = -1;
	for (int i = 0; options[i].string != NULL; i++) {
		if (options[i].id == savedOptionId) {
			savedIdx = ComboBox_FindString(hwnd, 0, options[i].string);
			break;
		}
	}

	BOOL changed = idx != savedIdx;

	if (!save) {
		setDirty(resourceId, changed);
		FieldChanged(hwndDlg);	
		return -1;
	}

	int selectedOptionId = GetSelectedDropListId(hwnd, options);

	return selectedOptionId;
}


static void ShowHideFields(HWND hwndDlg) {
	ConnectionProfile::PrinterType printerType = (ConnectionProfile::PrinterType)
		GetSelectedDropListId(GetDlgItem(hwndDlg,IDC_PRINTER_TYPE), printerTypeOptions);

	EnableWindow(GetDlgItem(hwndDlg,IDC_CHARSET), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_QUEUE), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_DEVICE), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_FILE), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_BROWSE_OUT_FILE), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_HEADER_FILE), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_BROWSE_HEADER), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_TIMEOUT), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_TIMEOUT_SPIN), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_SPEED), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_PARITY), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_FLOW), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_BIDIRECTIONAL), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_PS_OUTPUT), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_PS_WIDTH), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_WIDTH_SPIN), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_PS_HEIGHT), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_HEIGHT_SPIN), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_SEND_FF), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_SEND_STRING), printerType != ConnectionProfile::PT_NONE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_EOJ_STRING), printerType != ConnectionProfile::PT_NONE);

	if (printerType == ConnectionProfile::PT_NONE) {
		// Reset to Windows printer queue display
		ShowWindow(GetDlgItem(hwndDlg,IDC_DEVICE), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg,IDC_QUEUE), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg,IDC_FILE), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg,IDC_BROWSE_OUT_FILE), SW_HIDE);
		SetWindowText(GetDlgItem(hwndDlg, IDC_TARGET), TEXT("Windows Printer Queue:"));
		return;
	}

	IsDlgButtonChecked(hwndDlg, IDC_TRANS_LITERAL_FN) == BST_CHECKED;

	// Speed/Flow/Parity/Bidirectional and Deviced are only valid for DOS devices
	EnableWindow(GetDlgItem(hwndDlg,IDC_SPEED), printerType == ConnectionProfile::PT_DOS_DEVICE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_PARITY), printerType == ConnectionProfile::PT_DOS_DEVICE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_FLOW), printerType == ConnectionProfile::PT_DOS_DEVICE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_BIDIRECTIONAL), printerType == ConnectionProfile::PT_DOS_DEVICE);
	EnableWindow(GetDlgItem(hwndDlg,IDC_DEVICE), printerType == ConnectionProfile::PT_DOS_DEVICE);

	// Queue is only available for Windows printers
	EnableWindow(GetDlgItem(hwndDlg,IDC_QUEUE), printerType == ConnectionProfile::PT_WINDOWS);

	BOOL filePrinter = printerType == ConnectionProfile::PT_FILE ||
		printerType == ConnectionProfile::PT_COMMAND;
	EnableWindow(GetDlgItem(hwndDlg,IDC_FILE), filePrinter);
	EnableWindow(GetDlgItem(hwndDlg,IDC_BROWSE_OUT_FILE), filePrinter);
	

	switch(printerType) {
	case ConnectionProfile::PT_DOS_DEVICE:
		ShowWindow(GetDlgItem(hwndDlg,IDC_DEVICE), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg,IDC_QUEUE), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg,IDC_FILE), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg,IDC_BROWSE_OUT_FILE), SW_HIDE);
		SetWindowText(GetDlgItem(hwndDlg, IDC_TARGET), TEXT("DOS Device:"));
		break;
	case ConnectionProfile::PT_WINDOWS:
		ShowWindow(GetDlgItem(hwndDlg,IDC_DEVICE), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg,IDC_QUEUE), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg,IDC_FILE), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg,IDC_BROWSE_OUT_FILE), SW_HIDE);
		SetWindowText(GetDlgItem(hwndDlg, IDC_TARGET), TEXT("Windows Printer Queue:"));
		break;
	case ConnectionProfile::PT_FILE:
	case ConnectionProfile::PT_COMMAND:
		ShowWindow(GetDlgItem(hwndDlg,IDC_DEVICE), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg,IDC_QUEUE), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg,IDC_FILE), SW_SHOW);
		if (printerType == ConnectionProfile::PT_FILE) {
			SetWindowText(GetDlgItem(hwndDlg, IDC_TARGET), TEXT("Filename:"));
			ShowWindow(GetDlgItem(hwndDlg,IDC_BROWSE_OUT_FILE), SW_SHOW);
		}
		else {
			SetWindowText(GetDlgItem(hwndDlg, IDC_TARGET), TEXT("Command:"));
			ShowWindow(GetDlgItem(hwndDlg,IDC_BROWSE_OUT_FILE), SW_HIDE);
		}
		break;
	}

	BOOL outputPS = IsDlgButtonChecked(hwndDlg, IDC_PS_OUTPUT) == BST_CHECKED;
	EnableWindow(GetDlgItem(hwndDlg,IDC_PS_WIDTH), outputPS);
	EnableWindow(GetDlgItem(hwndDlg,IDC_WIDTH_SPIN), outputPS);
	EnableWindow(GetDlgItem(hwndDlg,IDC_PS_HEIGHT), outputPS);
	EnableWindow(GetDlgItem(hwndDlg,IDC_HEIGHT_SPIN), outputPS);


	BOOL eojString = IsDlgButtonChecked(hwndDlg, IDC_SEND_STRING) == BST_CHECKED;
	EnableWindow(GetDlgItem(hwndDlg,IDC_EOJ_STRING), eojString);
}


static void BrowseFile(HWND hwndDlg, int targetFieldId, LPCTSTR title, BOOL save) {
	OPENFILENAME ofn;
	LPTSTR szFile[MAX_PATH];

	ZeroMemory(szFile, sizeof(TCHAR) * MAX_PATH);
	ZeroMemory(&ofn, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);					
	ofn.hwndOwner = hwndDlg;
	ofn.lpstrFile = (LPTSTR)szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = TEXT("Text Files (*.txt)\0*.TXT\0All Files (*.*)\0*.*\0");
	ofn.lpstrDefExt = TEXT("txt");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = title;
	ofn.Flags = 
		OFN_PATHMUSTEXIST | 
		OFN_HIDEREADONLY  ;
	
	BOOL result;
	if (save)
		result = GetSaveFileName(&ofn);
	else
		result = GetOpenFileName(&ofn);

	if (result) {
		SetWindowText(GetDlgItem(hwndDlg, targetFieldId),
			(LPCTSTR)szFile);
	}
}


static void CheckPrinterTypeDDL(HWND hwndDlg, BOOL save) {
	int selectedId = CheckDropList(
		hwndDlg, 
		IDC_PRINTER_TYPE, 
		printerTypeOptions, 
		profile->printerType(),
		save);

	if (save && selectedId != -1) {
		profile->setPrinterType((ConnectionProfile::PrinterType)selectedId);
	}
}


static void CheckSpeedDDL(HWND hwndDlg, BOOL save) {
	int selectedId = CheckDropList(
		hwndDlg, 
		IDC_SPEED, 
		speedOptions, 
		profile->printSpeed(),
		save);

	if (save && selectedId != -1) {
		profile->setPrintSpeed(selectedId);
	}
}


static void CheckParityDDL(HWND hwndDlg, BOOL save) {
	int selectedId = CheckDropList(
		hwndDlg, 
		IDC_PARITY, 
		parityOptions, 
		profile->printParity(),
		save);

	if (save && selectedId != -1) {
		profile->setPrintParity((ConnectionProfile::Parity)selectedId);
	}
}


static void CheckFlowDDL(HWND hwndDlg, BOOL save) {
	int selectedId = CheckDropList(
		hwndDlg, 
		IDC_FLOW, 
		flowControlOptions, 
		profile->printFlowControl(),
		save);

	if (save && selectedId != -1) {
		profile->setPrintFlowControl((ConnectionProfile::FlowControl)selectedId);
	}
}

static void CheckBidirectional(HWND hwndDlg, BOOL save) {
	int id = IDC_BIDIRECTIONAL;
	BOOL newValue = IsDlgButtonChecked(hwndDlg, id) == BST_CHECKED;
	BOOL oldValue = profile->bidirectionalPrinting();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setBidirectionalPrinting(newValue);
	}
}


static void CheckPsOutput(HWND hwndDlg, BOOL save) {
	int id = IDC_PS_OUTPUT;
	BOOL newValue = IsDlgButtonChecked(hwndDlg, id) == BST_CHECKED;
	BOOL oldValue = profile->printAsPostScript();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setPrintAsPostScript(newValue);
	}
}


static void CheckUseEndOfJobString(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_SEND_STRING) == BST_CHECKED;
	BOOL oldValue = profile->sendEndOfJobString();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_SEND_STRING, changed);
		setDirty(IDC_SEND_FF, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setSendEndOfJobString(newValue);
	}
}


static void CheckCharset(HWND hwndDlg, BOOL save) {
	HWND hwndCset = GetDlgItem(hwndDlg, IDC_CHARSET);

	int idx = SendMessage(hwndCset, CB_GETCURSEL, 0, 0);

	Charset::Charset existing = profile->printCharacterSet();

	Charset::Charset newValue =  
			(Charset::Charset)SendMessage(
				hwndCset, CB_GETITEMDATA, idx, 0);

	BOOL changed = newValue != existing;

	if (!save) {
		setDirty(IDC_CHARSET, changed);
		FieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setPrintCharacterSet(newValue);
	}
}


static void CheckTargetFile(HWND hwndDlg, BOOL save) {
	ConnectionProfile::PrinterType printerType = (ConnectionProfile::PrinterType)
		GetSelectedDropListId(GetDlgItem(hwndDlg,IDC_PRINTER_TYPE), printerTypeOptions);

	// Only check or save this text box if its being used,
	// as this field and the DOS Device combo box are backed
	// by the same field
	if (printerType != ConnectionProfile::PT_FILE && 
		printerType != ConnectionProfile::PT_COMMAND) return;

	// Only load the stored value if the stored type is of the correct type,
	// otherwise we might be comparing a file against a DOS device.
	CMString originalValue;
	if (profile->printerType() == ConnectionProfile::PT_FILE ||
		profile->printerType() == ConnectionProfile::PT_COMMAND) {

		originalValue = profile->deviceName();
	}

	int id = IDC_FILE;
	
	HWND hwndField = GetDlgItem(hwndDlg, id);
	LPTSTR buf = 0;
	BOOL changed = FALSE;

	// Get current value
	int buflen = GetWindowTextLength(hwndField) + 1;

	// No point going to the effort of allocating memory and fetching
	// the string out of the field if we don't even have a stored value.
	if (originalValue.isNullOrWhiteSpace() && buflen > 1 && !save) {
		changed = TRUE;
	} else {
		// allocate buffer
		buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
		ZeroMemory(buf, buflen * sizeof(TCHAR));

		// Get current value
		GetWindowText(hwndField, buf, buflen);

		if (lstrcmp(buf, originalValue.data()) != 0) {
			changed = TRUE;
		}
	}

	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);
	} else if (changed) {
		profile->setDeviceName(CMString(buf));
	}

	if (buf) {
		free(buf);
	}
}



static void CheckTargetDosDevice(HWND hwndDlg, BOOL save) {
	ConnectionProfile::PrinterType printerType = (ConnectionProfile::PrinterType)
		GetSelectedDropListId(GetDlgItem(hwndDlg,IDC_PRINTER_TYPE), printerTypeOptions);

	// Only check or save this text box if its being used,
	// as this field and the DOS Device combo box are backed
	// by the same field
	if (printerType != ConnectionProfile::PT_DOS_DEVICE) return;

	// Only load the stored value if the stored type is of the correct type,
	// otherwise we might be comparing a file against a DOS device.
	CMString originalValue;
	if (profile->printerType() == ConnectionProfile::PT_DOS_DEVICE) {
		originalValue = profile->deviceName();
	}

	int id = IDC_DEVICE;
	
	HWND hwndField = GetDlgItem(hwndDlg, id);
	LPTSTR buf = 0;
	BOOL changed = FALSE;

	// Get current value
	int buflen = GetWindowTextLength(hwndField) + 1;

	// No point going to the effort of allocating memory and fetching
	// the string out of the field if we don't even have a stored value.
	if (originalValue.isNullOrWhiteSpace() && buflen > 1 && !save) {
		changed = TRUE;
	} else {
		// allocate buffer
		buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
		ZeroMemory(buf, buflen * sizeof(TCHAR));

		// Get current value
		GetWindowText(hwndField, buf, buflen);

		if (lstrcmp(buf, originalValue.data()) != 0) {
			changed = TRUE;
		}
	}

	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);
	} else if (changed) {
		profile->setDeviceName(CMString(buf));
	}

	if (buf) {
		free(buf);
	}
}


static void CheckHeaderFile(HWND hwndDlg, BOOL save) {
	CMString originalValue = profile->headerFile();
	int id = IDC_HEADER_FILE;
	
	HWND hwndField = GetDlgItem(hwndDlg, id);

	// Get current value
	int buflen = GetWindowTextLength(hwndField) + 1;

	// allocate buffer
	LPTSTR buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
	ZeroMemory(buf, buflen * sizeof(TCHAR));

	// Get current value
	GetWindowText(hwndField, buf, buflen);

	BOOL changed = FALSE;
	if (lstrcmp(buf, originalValue.data()) != 0) {
		changed = TRUE;
	}

	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);
	} else if (changed) {
		profile->setHeaderFile(CMString(buf));
	}

	free(buf);
}


static void CheckEojString(HWND hwndDlg, BOOL save) {
	CMString originalValue = profile->endOfJobString();
	int id = IDC_EOJ_STRING;
	
	HWND hwndField = GetDlgItem(hwndDlg, id);

	// Get current value
	int buflen = GetWindowTextLength(hwndField) + 1;

	// allocate buffer
	LPTSTR buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
	ZeroMemory(buf, buflen * sizeof(TCHAR));

	// Get current value
	GetWindowText(hwndField, buf, buflen);

	BOOL changed = FALSE;
	if (lstrcmp(buf, originalValue.data()) != 0) {
		changed = TRUE;
	}

	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);
	} else if (changed) {
		profile->setEndOfJobString(CMString(buf));
	}

	free(buf);
}


static void CheckTimeout(HWND hwndDlg, BOOL save) {
	int newValue = getFieldInt(hwndDlg, IDC_TIMEOUT);
	int oldValue = profile->printTimeoutSeconds();

	BOOL changed = newValue != oldValue;
	
	if (!save) {
		setDirty(IDC_TIMEOUT, changed);
		FieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setPrintTimeoutSeconds(newValue);
	}
}

static void CheckPsWidth(HWND hwndDlg, BOOL save) {
	int newValue = getFieldInt(hwndDlg, IDC_PS_WIDTH);
	int oldValue = profile->printWidth();

	BOOL changed = newValue != oldValue;
	
	if (!save) {
		setDirty(IDC_PS_WIDTH, changed);
		FieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setPrintWidth(newValue);
	}
}


static void CheckPsHeight(HWND hwndDlg, BOOL save) {
	int newValue = getFieldInt(hwndDlg, IDC_PS_HEIGHT);
	int oldValue = profile->printHeight();

	BOOL changed = newValue != oldValue;
	
	if (!save) {
		setDirty(IDC_PS_HEIGHT, changed);
		FieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setPrintHeight(newValue);
	}
}

static void PopulatePrinterQueueDDL(HWND hwndDlg) {
	HWND hwndQueue = GetDlgItem(hwndDlg, IDC_QUEUE);

	// Add the default queue
	ComboBox_AddString(hwndQueue, DEFAULT_WIN_PRINT_QUEUE);

	// Some of the printer flags depend on if we're running on Windows 9x or NT
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	// What sorts of printers do we want to enumerate?
	DWORD dwEnumFlags = PRINTER_ENUM_LOCAL;
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
		// This flag is only valid on Windows NT - enumerate printers
		// the user has previously made connections to
		dwEnumFlags |= PRINTER_ENUM_CONNECTIONS;
	}

	// Find out how much memory we need to allocate to store the list
	DWORD  pcbNeeded, pcReturned;
	EnumPrinters(dwEnumFlags,	// Types of pinters to enumerate
				 NULL,			// Name of the printer we're interested in
				 2,				// Printer info structure level
				 NULL,			// Output array pointer
				 0,				// Size (in bytes) of the output array
				 &pcbNeeded,	// Number of bytes needed
                 &pcReturned);	// Number of printer info structures copied

	if (pcbNeeded == 0) {
		return; // No printers setup!
	}

	// Allocate memory to receive printer info
	LPPRINTER_INFO_2 printerInfo = NULL;
	if ((printerInfo = (LPPRINTER_INFO_2)LocalAlloc(LPTR, pcbNeeded)) == NULL)
    {
        return; // Fail.
    }


	// Enumerate printers for real this time.
	if (!EnumPrinters(dwEnumFlags, NULL, 2, (LPBYTE)printerInfo,
                      pcbNeeded, &pcbNeeded, &pcReturned))
    {
        LocalFree(printerInfo);
        return; // Fail.
    }

	// We now have all available printers in printerInfo!
	for (unsigned int i = 0; i < pcReturned; i++) {
		ComboBox_AddString(hwndQueue, printerInfo[i].pPrinterName);
	}

	// Done!
	LocalFree(printerInfo);

	// Now set whatever the currently set value is, adding it to the list if
	// necessary.
	LPCTSTR currentSetting = profile->windowsPrintQueue().data();
	if (ComboBox_FindString(hwndQueue, 0, currentSetting) == CB_ERR) {
		ComboBox_AddString(hwndQueue, currentSetting);
	}
	ComboBox_SelectString(hwndQueue, 0, currentSetting);
}


static void CheckTargetQueue(HWND hwndDlg, BOOL save) {
	CMString originalValue = profile->windowsPrintQueue();
	int id = IDC_QUEUE;
	
	HWND hwndField = GetDlgItem(hwndDlg, id);
	LPTSTR buf = 0;
	BOOL changed = FALSE;

	// Get current value
	int buflen = GetWindowTextLength(hwndField) + 1;

	// No point going to the effort of allocating memory and fetching
	// the string out of the field if we don't even have a stored value.
	if (originalValue.isNullOrWhiteSpace() && buflen > 1 && !save) {
		changed = TRUE;
	} else {
		// allocate buffer
		buf = (LPTSTR)malloc(buflen * sizeof(TCHAR));
		ZeroMemory(buf, buflen * sizeof(TCHAR));

		// Get current value
		GetWindowText(hwndField, buf, buflen);

		if (lstrcmp(buf, originalValue.data()) != 0) {
			changed = TRUE;
		}
	}

	if (!save) {
		setDirty(id, changed);
		FieldChanged(hwndDlg);
	} else if (changed) {
		profile->setWindowsPrintQueue(CMString(buf));
	}

	if (buf) {
		free(buf);
	}
}


/*
Controls:
IDC_PRINTER_TYPE		Drop-down list
IDC_CHARSET				Drop-down list
IDC_TARGET				Static
IDC_QUEUE				Drop-down list
IDC_DEVICE				Combo Box
IDC_FILE				Text box
IDC_BROWSE_OUT_FILE		Button
IDC_HEADER_FILE			Text box
IDC_BROWSE_HEADER		Button
IDC_TIMEOUT				Text box
IDC_TIMEOUT_SPIN		Spin button
IDC_SPEED				Drop-down list
IDC_PARITY				Drop-down list
IDC_FLOW				Drop-down list
IDC_BIDIRECTIONAL		Checkbox
IDC_PS_OUTPUT			Checkbox
IDC_PS_WIDTH			Text box
IDC_WIDTH_SPIN			Spin button
IDC_PS_HEIGHT			Text box
IDC_HEIGHT_SPIN			Spin button
IDC_SEND_FF				Radio button
IDC_SEND_STRING			Radio button
IDC_EOJ_STRING			Text box
 */


// TODO: Range isn't working for spin boxes

BOOL CALLBACK PrinterPageDlgProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {


	switch(uMsg) {
	case WM_INITDIALOG: 
		{
			HWND hwndCharset = GetDlgItem(hwndDlg, IDC_CHARSET);

			// Populate drop-lists
			InsertDropListItems(hwndDlg, IDC_PRINTER_TYPE, printerTypeOptions);
			InsertDropListItems(hwndDlg, IDC_DEVICE, deviceOptions);
			InsertDropListItems(hwndDlg, IDC_SPEED, speedOptions);
			InsertDropListItems(hwndDlg, IDC_PARITY, parityOptions);
			InsertDropListItems(hwndDlg, IDC_FLOW, flowControlOptions);
			Charset::populateDropList(
				hwndCharset,
				Charset::CU_PRINT, 
				TRUE);
			PopulatePrinterQueueDDL(hwndDlg);

			// Configure spinboxes
			ConfigureSpinBox(hwndDlg, IDC_TIMEOUT_SPIN, IDC_TIMEOUT, 
							 0, 999, profile->printTimeoutSeconds());
			ConfigureSpinBox(hwndDlg, IDC_WIDTH_SPIN, IDC_PS_WIDTH, 
							 10, 255, profile->printWidth());
			ConfigureSpinBox(hwndDlg, IDC_HEIGHT_SPIN, IDC_PS_HEIGHT, 
							 5, 120, profile->printHeight());

			// Set drop-lists
			SelectDropListItem(hwndDlg, IDC_PRINTER_TYPE, printerTypeOptions, 
				profile->printerType());
			SelectDropListItem(hwndDlg, IDC_SPEED, speedOptions, 
				profile->printSpeed());
			SelectDropListItem(hwndDlg, IDC_PARITY, parityOptions, 
				profile->printParity());
			SelectDropListItem(hwndDlg, IDC_FLOW, flowControlOptions, 
				profile->printFlowControl());

			HWND hwndDevice = GetDlgItem(hwndDlg, IDC_DEVICE);
			SetWindowText(hwndDevice, TEXT("lpt1"));


			// Set checkboxes
			CheckDlgButton(hwndDlg, IDC_BIDIRECTIONAL, 
				profile->bidirectionalPrinting() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_PS_OUTPUT, 
				profile->printAsPostScript() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SEND_STRING, 
				profile->sendEndOfJobString() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SEND_FF, 
				profile->sendEndOfJobString() ? BST_UNCHECKED :BST_CHECKED);

			// Set text boxes
			SetWindowText(
				GetDlgItem(hwndDlg, IDC_HEADER_FILE),
				profile->headerFile().data());
			SetWindowText(
				GetDlgItem(hwndDlg, IDC_EOJ_STRING),
				profile->endOfJobString().data());

			// Set DOS device or 
			switch(profile->printerType()) {
			case ConnectionProfile::PT_DOS_DEVICE:
				{
					LPCTSTR currentSetting = profile->deviceName().data();
					if (ComboBox_FindString(hwndDevice, 0, currentSetting) == CB_ERR) {
						ComboBox_AddString(hwndDevice, currentSetting);
					}
					ComboBox_SelectString(hwndDevice, 0, currentSetting);
				}
				break;
			case ConnectionProfile::PT_FILE:
			case ConnectionProfile::PT_COMMAND:
				SetWindowText(
					GetDlgItem(hwndDlg, IDC_FILE),
					profile->deviceName().data());
				break;
			case ConnectionProfile::PT_NONE:
			default:
				// Nothing
				break;
			}
			

			// Select charset
			LPTSTR selectedCS = Charset::getCharsetLabel(
						profile->printCharacterSet(), TRUE);
			if (selectedCS != NULL) {
				SendMessage(
					hwndCharset,
					CB_SELECTSTRING ,
					(WPARAM)0,
					(LPARAM)selectedCS);
				free(selectedCS);
			}

			// Set form state
			ShowHideFields(hwndDlg);

			// Form is clean
			cleanForm();

			break;
		}
	case WM_COMMAND: {
			WORD wNotifyCode = HIWORD(wParam);
			WORD wID = LOWORD(wParam);

			switch(wID) {
			case IDC_PRINTER_TYPE:
				CheckPrinterTypeDDL(hwndDlg, FALSE);
				ShowHideFields(hwndDlg);
				break;
			case IDC_CHARSET:
				CheckCharset(hwndDlg, FALSE);
				break;
			case IDC_QUEUE:
				CheckTargetQueue(hwndDlg, FALSE);
				break;
			case IDC_DEVICE:
				CheckTargetDosDevice(hwndDlg, FALSE);
				break;
			case IDC_FILE:
				CheckTargetFile(hwndDlg, FALSE);
				break;
			case IDC_BROWSE_OUT_FILE:
				BrowseFile(hwndDlg, IDC_FILE, TEXT("Print to file"), TRUE);
				break;
			case IDC_HEADER_FILE:
				CheckHeaderFile(hwndDlg, FALSE);
				break;
			case IDC_BROWSE_HEADER:
				BrowseFile(hwndDlg, IDC_HEADER_FILE, TEXT("Print Job Header Page"), FALSE);
				break;
			case IDC_TIMEOUT:
				CheckTimeout(hwndDlg, FALSE);
				break;
			case IDC_SPEED:
				CheckSpeedDDL(hwndDlg, FALSE);
				break;
			case IDC_PARITY:
				CheckParityDDL(hwndDlg, FALSE);
				break;
			case IDC_FLOW:
				CheckFlowDDL(hwndDlg, FALSE);
				break;
			case IDC_BIDIRECTIONAL:
				CheckBidirectional(hwndDlg, FALSE);
				break;
			case IDC_PS_OUTPUT:
				CheckPsOutput(hwndDlg, FALSE);
				ShowHideFields(hwndDlg);
				break;
			case IDC_PS_WIDTH:
				CheckPsWidth(hwndDlg, FALSE);
				break;
			case IDC_PS_HEIGHT:
				CheckPsHeight(hwndDlg, FALSE);
				break;
			case IDC_SEND_FF:
			case IDC_SEND_STRING:
				CheckUseEndOfJobString(hwndDlg, FALSE);
				ShowHideFields(hwndDlg);
				break;
			case IDC_EOJ_STRING:
				CheckEojString(hwndDlg, FALSE);
				break;
			case IDC_TIMEOUT_SPIN:
			case IDC_WIDTH_SPIN:
			case IDC_HEIGHT_SPIN:
				// These take care of themselves
				break;
			}
			return TRUE;
			break;
		}
	case WM_NOTIFY: {
			LPNMHDR pnmh = (LPNMHDR)lParam;
			switch (pnmh->code) {
			case PSN_APPLY:	{		// Apply or OK button clicked
				if (isDirty()) {
					// Save any values that have changed
					CheckPrinterTypeDDL(hwndDlg, TRUE);
					CheckCharset(hwndDlg, TRUE);
					CheckTargetQueue(hwndDlg, TRUE);
					CheckTargetDosDevice(hwndDlg, TRUE);
					CheckTargetFile(hwndDlg, TRUE);
					CheckHeaderFile(hwndDlg, TRUE);
					CheckTimeout(hwndDlg, TRUE);
					CheckSpeedDDL(hwndDlg, TRUE);
					CheckParityDDL(hwndDlg, TRUE);
					CheckFlowDDL(hwndDlg, TRUE);
					CheckBidirectional(hwndDlg, TRUE);
					CheckPsOutput(hwndDlg, TRUE);
					CheckPsWidth(hwndDlg, TRUE);
					CheckPsHeight(hwndDlg, TRUE);
					CheckUseEndOfJobString(hwndDlg, TRUE);
					CheckEojString(hwndDlg, TRUE);

					cleanForm();
				}
				return TRUE;
			}
			break;
				
			case PSN_QUERYCANCEL:	// User clicked cancel - OK??
			case PSN_RESET:			// User clicked cancel
			case PSN_HELP:			// User clicked the Help button
			case PSN_SETACTIVE:		// page activated (user switched to this page) 
			case PSN_KILLACTIVE:	// page de-activated (user switched away)
			case PSN_WIZNEXT:		// We're a wizard and the user clicked next
			case PSN_WIZBACK:		// We're a wizard and the user clicked back
			case PSN_WIZFINISH:		// We're a wizard and the user clicked finish
			default:
				break;
			}

			break;
		}
	}
	return 0; // Message not processed
}

UINT CALLBACK PrinterPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp) {
	switch (uMsg) {
	case PSPCB_CREATE:
		profile = (ConnectionProfile*)ppsp->lParam;
		return TRUE;
	case PSPCB_RELEASE:
		return 0;
	}
	return 0;
}
