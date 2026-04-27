#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <wingdi.h>

#include "../conn_props.h"
#include "../conn_profile.h"
#include "../resource.h"
#include "../charset.h"

/******************
 * SHEET: File Transfer
 ******************/

/*
Controls:
IDC_TRANS_PROTOCOL			Drop-down list
IDC_TRANS_PERFORMANCE		Drop-down list
IDC_TRANS_PKT_LEN			Number edit
IDC_TRANS_PKT_LEN_SPIN		Spin button
IDC_TRANS_WIND_SIZE			Number edit
IDC_TRANS_WIND_SIZE_SPIN	Spin button
IDC_TRANS_CC_UNPREFIXING	Drop-down list
IDC_TRANS_AUTODL			Drop-down list
IDC_TRANS_FN_COLLISION		Drop-down list
IDC_TRANS_FILE_CSET			Drop-down list
IDC_TRANS_CSET				Drop-down list
IDC_TRANS_NEG_STREAMING		Checkbox
IDC_TRANS_NEG_CC			Checkbox
IDC_TRANS_TEXT				Radiobutton
IDC_TRANS_BINARY			Radiobutton
IDC_TRANS_PATHNAMES			Checkbox
IDC_TRANS_KEEP_PARTIAL		Checkbox
IDC_TRANS_LITERAL_FN		Checkbox
IDC_TRANS_16BIT_CRC			Checkbox
 */

// Connection profile we're editing
static ConnectionProfile *profile;


typedef struct tagDialogFieldStatus {
	int id;
	BOOL dirty;
} DialogFieldStatus;


static const int fieldCount = 17;

static DialogFieldStatus fieldStatus[] = {
	{ IDC_TRANS_PROTOCOL,		FALSE	},
	{ IDC_TRANS_PERFORMANCE,	FALSE	},
	{ IDC_TRANS_PKT_LEN,		FALSE	},
	{ IDC_TRANS_WIND_SIZE,		FALSE	},
	{ IDC_TRANS_CC_UNPREFIXING,	FALSE	},
	{ IDC_TRANS_AUTODL,			FALSE	},
	{ IDC_TRANS_FN_COLLISION,	FALSE	},
	{ IDC_TRANS_FILE_CSET,		FALSE	},
	{ IDC_TRANS_CSET,			FALSE	},
	{ IDC_TRANS_NEG_STREAMING,	FALSE	},
	{ IDC_TRANS_NEG_CC,			FALSE	},
	{ IDC_TRANS_TEXT,			FALSE	},
	{ IDC_TRANS_BINARY,			FALSE	},
	{ IDC_TRANS_PATHNAMES,		FALSE	},
	{ IDC_TRANS_KEEP_PARTIAL,	FALSE	},
	{ IDC_TRANS_LITERAL_FN,		FALSE	},
	{ IDC_TRANS_16BIT_CRC,		FALSE	}
};

typedef struct tagDropListOption {
	int id;
	LPCTSTR string;
} DropListOption;

static DropListOption protocolOptions[] = {
	{ ConnectionProfile::FT_KERMIT, TEXT("Kermit") },
	{ ConnectionProfile::FT_ZMODEM, TEXT("Zmodem") },
	{ ConnectionProfile::FT_YMODEM, TEXT("Ymodem") },
	{ ConnectionProfile::FT_YMODEM_G, TEXT("Ymodem-G") },
	{ ConnectionProfile::FT_XMODEM, TEXT("Xmodem") },
	{ 0, 0 }
};

static DropListOption performanceOptions[] = {
	{ ConnectionProfile::KP_FAST, TEXT("Fast") },
	{ ConnectionProfile::KP_CAUTIOUS, TEXT("Cautious") },
	{ ConnectionProfile::KP_ROBUST, TEXT("Robust") },
	{ ConnectionProfile::KP_CUSTOM, TEXT("Custom") },
	{ 0, 0 }
};

static DropListOption ccUnprefixingOptions[] = {
	{ ConnectionProfile::CCU_NEVER, TEXT("Never") },
	{ ConnectionProfile::CCU_CAUTIOUS, TEXT("Cautiously") },
	{ ConnectionProfile::CCU_WITH_WILD_ABANDON, TEXT("With wild abandon") },
	{ 0, 0 }
};

static DropListOption autoDownloadOptions[] = {
	{ ConnectionProfile::AD_ASK, TEXT("Ask") },
	{ ConnectionProfile::AD_YES, TEXT("Yes") },
	{ ConnectionProfile::AD_NO, TEXT("No") },
	{ 0, 0 }
};

static DropListOption fnCollisionOptions[] = {
	{ ConnectionProfile::FNC_BACKUP, TEXT("Backup") },
	{ ConnectionProfile::FNC_UPDATE, TEXT("Update") },
	{ ConnectionProfile::FNC_OVERWRITE, TEXT("Overwrite") },
	{ ConnectionProfile::FNC_APPEND, TEXT("Append") },
	{ ConnectionProfile::FNC_DISCARD, TEXT("Discard") },
	{ ConnectionProfile::FNC_RENAME, TEXT("Rename") },
	{ 0, 0 }
};

static DropListOption fnCollisionNoUpdateOptions[] = {
	{ ConnectionProfile::FNC_BACKUP, TEXT("Backup") },
	{ ConnectionProfile::FNC_OVERWRITE, TEXT("Overwrite") },
	{ ConnectionProfile::FNC_APPEND, TEXT("Append") },
	{ ConnectionProfile::FNC_DISCARD, TEXT("Discard") },
	{ ConnectionProfile::FNC_RENAME, TEXT("Rename") },
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


static void PopulateFilenameCollisionDDL(HWND hwndDlg, BOOL allowUpdate) {
	HWND hwndCtl = GetDlgItem(hwndDlg, IDC_TRANS_FN_COLLISION);

	// Backup should always be in the list. If it isn't, the list is currently clear.
	BOOL listClear = ComboBox_FindString(hwndCtl, 0, TEXT("Backup")) == CB_ERR;


	if (ComboBox_FindString(hwndCtl, 0, TEXT("Update")) != CB_ERR && !allowUpdate) {
		// Update is in the list, and it shouldn't be. Clear the list.
		ComboBox_ResetContent(hwndCtl);
		listClear = TRUE;

	} else if (ComboBox_FindString(hwndCtl, 0, TEXT("Update")) == CB_ERR && allowUpdate) {
		// Update is not in the list, and it should be. repopulate the list.
		ComboBox_ResetContent(hwndCtl);
		listClear = TRUE;
	}
	
	if (listClear) {
		if (allowUpdate)
			InsertDropListItems(hwndDlg, IDC_TRANS_FN_COLLISION, fnCollisionOptions);
		else
			InsertDropListItems(hwndDlg, IDC_TRANS_FN_COLLISION, fnCollisionNoUpdateOptions);

		ComboBox_SelectString(hwndDlg, 0, fnCollisionOptions[0].string);
	}
}

static void SetCharset(HWND hwndDlg, int resourceId, Charset::Charset cset) {
	LPTSTR selectedCS = Charset::getCharsetLabel(
				cset, TRUE);
	if (selectedCS != NULL) {
		SendMessage(
			GetDlgItem(hwndDlg, resourceId),
			CB_SELECTSTRING ,
			(WPARAM)0,
			(LPARAM)selectedCS);
		free(selectedCS);
	}
}

static void CheckTransferProtocolDDL(HWND hwndDlg, BOOL save) {
	int selectedId = CheckDropList(
		hwndDlg, 
		IDC_TRANS_PROTOCOL, 
		protocolOptions, 
		profile->fileTransferProtocol(),
		save);

	if (save && selectedId != -1) {
		profile->setFileTransferProtocol((ConnectionProfile::FileTransferProtocol)selectedId);
	}
}

static void CheckPerformanceDDL(HWND hwndDlg, BOOL save) {
	int selectedId = CheckDropList(
		hwndDlg, 
		IDC_TRANS_PERFORMANCE, 
		performanceOptions, 
		profile->kermitPerformance(),
		save);

	if (save && selectedId != -1) {
		profile->setKermitPerformance((ConnectionProfile::KermitPerformance)selectedId);
	}
}

static void CheckUnprefixingDDL(HWND hwndDlg, BOOL save) {
	int selectedId = CheckDropList(
		hwndDlg, 
		IDC_TRANS_CC_UNPREFIXING, 
		ccUnprefixingOptions, 
		profile->controlCharUnprefixing(),
		save);

	if (save && selectedId != -1) {
		profile->setControlCharUnprefixing((ConnectionProfile::ControlCharUnprefixing)selectedId);
	}
}

static void CheckAutoDlDDL(HWND hwndDlg, BOOL save) {
	int selectedId = CheckDropList(
		hwndDlg, 
		IDC_TRANS_AUTODL, 
		autoDownloadOptions, 
		profile->autoDownloadMode(),
		save);

	if (save && selectedId != -1) {
		profile->setAutoDownloadMode((ConnectionProfile::AutoDownload)selectedId);
	}
}

static void CheckFnCollisionDDL(HWND hwndDlg, BOOL save) {
	int selectedId = CheckDropList(
		hwndDlg, 
		IDC_TRANS_FN_COLLISION, 
		fnCollisionOptions, 
		profile->fileNameCollisionAction(),
		save);

	if (save && selectedId != -1) {
		profile->setFileNameCollisionAction((ConnectionProfile::FileNameCollision)selectedId);
	}
}

static void CheckFileCharset(HWND hwndDlg, BOOL save) {
	HWND hwndCset = GetDlgItem(hwndDlg, IDC_TRANS_FILE_CSET);

	int idx = SendMessage(hwndCset, CB_GETCURSEL, 0, 0);

	Charset::Charset existing = profile->fileCharacterSet();

	Charset::Charset newValue =  
			(Charset::Charset)SendMessage(
				hwndCset, CB_GETITEMDATA, idx, 0);

	BOOL changed = newValue != existing;

	if (!save) {
		setDirty(IDC_TRANS_FILE_CSET, changed);
		FieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setFileCharacterSet(newValue);
	}
}

static void CheckTransferCharset(HWND hwndDlg, BOOL save) {
	HWND hwndCset = GetDlgItem(hwndDlg, IDC_TRANS_CSET);

	int idx = SendMessage(hwndCset, CB_GETCURSEL, 0, 0);

	Charset::Charset existing = profile->transferCharacterSet();

	Charset::Charset newValue =  
			(Charset::Charset)SendMessage(
				hwndCset, CB_GETITEMDATA, idx, 0);

	BOOL changed = newValue != existing;

	if (!save) {
		setDirty(IDC_TRANS_CSET, changed);
		FieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setTransferCharacterSet(newValue);
	}
}

static void CheckPacketLength(HWND hwndDlg, BOOL save) {

	// We have to get the numeric value off of the spin box
	// rather than backing field itself because it uses commas
	// for numbers >999 which getFieldInt() can't handle
	int newValue = GetSpinBoxValue(hwndDlg, IDC_TRANS_PKT_LEN_SPIN);

	int oldValue = profile->packetLength();

	BOOL changed = newValue != oldValue;
	
	if (!save) {
		setDirty(IDC_TRANS_PKT_LEN, changed);
		FieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setPacketLength(newValue);
	}
}

static void CheckWindowSize(HWND hwndDlg, BOOL save) {
	int newValue = getFieldInt(hwndDlg, IDC_TRANS_WIND_SIZE);
	int oldValue = profile->windowSize();

	BOOL changed = newValue != oldValue;
	
	if (!save) {
		setDirty(IDC_TRANS_WIND_SIZE, changed);
		FieldChanged(hwndDlg);	
		return;
	}

	if (changed) {
		profile->setWindowSize(newValue);
	}
}

static void CheckNegoStreaming(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_TRANS_NEG_STREAMING) == BST_CHECKED;
	BOOL oldValue = profile->negotiateStreamingTransferMode();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_TRANS_NEG_STREAMING, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setNegotiateStreamingTransferMode(newValue);
	}
}

static void CheckNegoClearChannel(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_TRANS_NEG_CC) == BST_CHECKED;
	BOOL oldValue = profile->negotiateClearChannelTransferMode();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_TRANS_NEG_CC, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setNegotiateClearChannelTransferMode(newValue);
	}
}

static void CheckBinaryMode(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_TRANS_BINARY) == BST_CHECKED;
	BOOL oldValue = profile->defaultToBinaryMode();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_TRANS_BINARY, changed);
		setDirty(IDC_TRANS_TEXT, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setDefaultToBinaryMode(newValue);
	}
}

static void CheckUsePathnames(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_TRANS_PATHNAMES) == BST_CHECKED;
	BOOL oldValue = profile->usePathnames();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_TRANS_PATHNAMES, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setUsePathnames(newValue);
	}
}


static void CheckKeepPartial(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_TRANS_KEEP_PARTIAL) == BST_CHECKED;
	BOOL oldValue = profile->keepIncompleteFiles();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_TRANS_KEEP_PARTIAL, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setKeepIncomingFiles(newValue);
	}
}

static void CheckLiteralFilenames(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_TRANS_LITERAL_FN) == BST_CHECKED;
	BOOL oldValue = profile->transmitLiteralFilenames();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_TRANS_LITERAL_FN, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setTransmitLiteralFilenames(newValue);
	}
}

static void Check16bitCRC(HWND hwndDlg, BOOL save) {
	BOOL newValue = IsDlgButtonChecked(hwndDlg, IDC_TRANS_16BIT_CRC) == BST_CHECKED;
	BOOL oldValue = profile->force16bitCRC();

	BOOL changed = newValue != oldValue;


	if (!save) {
		setDirty(IDC_TRANS_16BIT_CRC, changed);
		FieldChanged(hwndDlg);		
		return;
	}

	if (changed) {
		profile->setForce16bitCRC(newValue);
	}
}


static void SetFieldsEnabled(HWND hwndDlg) {

	ConnectionProfile::FileTransferProtocol proto = (ConnectionProfile::FileTransferProtocol)
		GetSelectedDropListId(GetDlgItem(hwndDlg,IDC_TRANS_PROTOCOL), protocolOptions);
	ConnectionProfile::KermitPerformance perf = (ConnectionProfile::KermitPerformance)
		GetSelectedDropListId(GetDlgItem(hwndDlg,IDC_TRANS_PERFORMANCE), performanceOptions);

	switch(proto) {
	case ConnectionProfile::FT_KERMIT: {
		// These only get enabled for the Custon performance profile
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_PKT_LEN), perf == ConnectionProfile::KP_CUSTOM);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_PKT_LEN_SPIN), perf == ConnectionProfile::KP_CUSTOM);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_WIND_SIZE), perf == ConnectionProfile::KP_CUSTOM);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_WIND_SIZE_SPIN), perf == ConnectionProfile::KP_CUSTOM);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_CC_UNPREFIXING), perf == ConnectionProfile::KP_CUSTOM);

		switch(perf) {
		case ConnectionProfile::KP_FAST:
			// Packet Length = 4000
			SendMessage(GetDlgItem(hwndDlg, IDC_TRANS_PKT_LEN_SPIN),
				UDM_SETPOS,
				(WPARAM)0,
				(LPARAM)4000);

			// Window size = 30
			SendMessage(GetDlgItem(hwndDlg, IDC_TRANS_WIND_SIZE_SPIN),
				UDM_SETPOS,
				(WPARAM)0,
				(LPARAM)30);

			// Control Character Unprefixing = Cautiously
			SelectDropListItem(hwndDlg, IDC_TRANS_CC_UNPREFIXING, 
				ccUnprefixingOptions, 
				ConnectionProfile::CCU_CAUTIOUS);

			break;

		case ConnectionProfile::KP_CAUTIOUS:
			// Packet Length = 1000
			SendMessage(GetDlgItem(hwndDlg, IDC_TRANS_PKT_LEN_SPIN),
				UDM_SETPOS,
				(WPARAM)0,
				(LPARAM)1000);

			// Window size = 4
			SendMessage(GetDlgItem(hwndDlg, IDC_TRANS_WIND_SIZE_SPIN),
				UDM_SETPOS,
				(WPARAM)0,
				(LPARAM)4);

			// Control Character Unprefixing = Cautiously
			SelectDropListItem(hwndDlg, IDC_TRANS_CC_UNPREFIXING, 
				ccUnprefixingOptions, 
				ConnectionProfile::CCU_CAUTIOUS);

			break;
		case ConnectionProfile::KP_ROBUST:
			// Packet Length = 94
			SendMessage(GetDlgItem(hwndDlg, IDC_TRANS_PKT_LEN_SPIN),
				UDM_SETPOS,
				(WPARAM)0,
				(LPARAM)944);

			// Window size = 1
			SendMessage(GetDlgItem(hwndDlg, IDC_TRANS_WIND_SIZE_SPIN),
				UDM_SETPOS,
				(WPARAM)0,
				(LPARAM)1);

			// Control Character Unprefixing = Never
			SelectDropListItem(hwndDlg, IDC_TRANS_CC_UNPREFIXING, 
				ccUnprefixingOptions, 
				ConnectionProfile::CCU_NEVER);

			break;
		}

		// Enable fields that get disabled by Zmodem
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_PERFORMANCE), TRUE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_16BIT_CRC), TRUE);
		}
		break;
	
	case ConnectionProfile::FT_ZMODEM:
		// These fields don't apply to Zmodem
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_PERFORMANCE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_PKT_LEN), FALSE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_PKT_LEN_SPIN), FALSE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_16BIT_CRC), FALSE);

		// But these do
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_WIND_SIZE), TRUE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_WIND_SIZE_SPIN), TRUE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_CC_UNPREFIXING), TRUE);
		break;
	case ConnectionProfile::FT_YMODEM:
	case ConnectionProfile::FT_YMODEM_G:
	case ConnectionProfile::FT_XMODEM:
		// These fields don't apply to Ymodem or Xmodem
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_PERFORMANCE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_WIND_SIZE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_WIND_SIZE_SPIN), FALSE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_CC_UNPREFIXING), FALSE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_16BIT_CRC), FALSE);

		// But these do
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_PKT_LEN), TRUE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_PKT_LEN_SPIN), TRUE);
	}
}


BOOL CALLBACK FileTransferPageDlgProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {


	switch(uMsg) {
	case WM_INITDIALOG: 
		{
			// Populate drop-lists
			InsertDropListItems(hwndDlg, IDC_TRANS_PROTOCOL, protocolOptions);
			InsertDropListItems(hwndDlg, IDC_TRANS_PERFORMANCE, performanceOptions);
			InsertDropListItems(hwndDlg, IDC_TRANS_CC_UNPREFIXING, ccUnprefixingOptions);
			InsertDropListItems(hwndDlg, IDC_TRANS_AUTODL, autoDownloadOptions);

			ConnectionProfile::FileTransferProtocol currentProto = profile->fileTransferProtocol();
			PopulateFilenameCollisionDDL(
				hwndDlg,
				currentProto == ConnectionProfile::FT_KERMIT || 
				currentProto == ConnectionProfile::FT_ZMODEM);

			Charset::populateDropList(
				GetDlgItem(hwndDlg, IDC_TRANS_FILE_CSET),
				Charset::CU_FILE, 
				TRUE);
			Charset::populateDropList(
				GetDlgItem(hwndDlg, IDC_TRANS_CSET),
				Charset::CU_TRANSFER,
				TRUE);
			
			// Configure spinboxes
			ConfigureSpinBox(hwndDlg, IDC_TRANS_PKT_LEN_SPIN, IDC_TRANS_PKT_LEN, 
							10, 9024, profile->packetLength());
			ConfigureSpinBox(hwndDlg, IDC_TRANS_WIND_SIZE_SPIN, IDC_TRANS_WIND_SIZE, 
							0, 32, profile->windowSize());


			// load initial values from profile
			SelectDropListItem(hwndDlg, IDC_TRANS_PROTOCOL, protocolOptions, 
				profile->fileTransferProtocol());
			SelectDropListItem(hwndDlg, IDC_TRANS_PERFORMANCE, performanceOptions, 
				profile->kermitPerformance());
			SelectDropListItem(hwndDlg, IDC_TRANS_CC_UNPREFIXING, ccUnprefixingOptions, 
				profile->controlCharUnprefixing());
			SelectDropListItem(hwndDlg, IDC_TRANS_AUTODL, autoDownloadOptions, 
				profile->autoDownloadMode());
			SelectDropListItem(hwndDlg, IDC_TRANS_FN_COLLISION, fnCollisionOptions, 
				profile->fileNameCollisionAction());
			SetCharset(hwndDlg, IDC_TRANS_FILE_CSET, profile->fileCharacterSet());
			SetCharset(hwndDlg, IDC_TRANS_CSET, profile->transferCharacterSet());
			CheckDlgButton(hwndDlg, IDC_TRANS_NEG_STREAMING, 
				profile->negotiateStreamingTransferMode() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TRANS_NEG_CC, 
				profile->negotiateClearChannelTransferMode() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TRANS_TEXT, 
				profile->defaultToBinaryMode() ? BST_UNCHECKED : BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_TRANS_BINARY, 
				profile->defaultToBinaryMode() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TRANS_PATHNAMES, 
				profile->usePathnames() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TRANS_KEEP_PARTIAL, 
				profile->keepIncompleteFiles() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TRANS_LITERAL_FN, 
				profile->transmitLiteralFilenames() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TRANS_16BIT_CRC, 
				profile->force16bitCRC() ? BST_CHECKED : BST_UNCHECKED);

			// Set fields enabled/disabled
			SetFieldsEnabled(hwndDlg);

			// Form is clean
			cleanForm();

			break;
		}
	case WM_COMMAND: {
			WORD wNotifyCode = HIWORD(wParam);
			WORD wID = LOWORD(wParam);

			switch(wID) {
			
			// CheckX(hwndDlg, FALSE);
			case IDC_TRANS_PROTOCOL: 
				CheckTransferProtocolDDL(hwndDlg, FALSE);
				SetFieldsEnabled(hwndDlg);
				break;
			case IDC_TRANS_PERFORMANCE: 
				CheckPerformanceDDL(hwndDlg, FALSE);

				// This also sets field values for the seleted performance setting
				SetFieldsEnabled(hwndDlg);
				break;
			case IDC_TRANS_PKT_LEN: 
				CheckPacketLength(hwndDlg, FALSE);
				break;
			case IDC_TRANS_WIND_SIZE: 
				CheckWindowSize(hwndDlg, FALSE);
				break;
			case IDC_TRANS_PKT_LEN_SPIN:
			case IDC_TRANS_WIND_SIZE_SPIN:
				// These handle themselves - nothing for us to do
				break;
			case IDC_TRANS_CC_UNPREFIXING: 
				CheckUnprefixingDDL(hwndDlg, FALSE);
				break;
			case IDC_TRANS_AUTODL: 
				CheckAutoDlDDL(hwndDlg, FALSE);
				break;
			case IDC_TRANS_FN_COLLISION: 
				CheckFnCollisionDDL(hwndDlg, FALSE);
				break;
			case IDC_TRANS_FILE_CSET: 
				CheckFileCharset(hwndDlg, FALSE);
				break;
			case IDC_TRANS_CSET: 
				CheckTransferCharset(hwndDlg, FALSE);
				break;
			case IDC_TRANS_NEG_STREAMING: 
				CheckNegoStreaming(hwndDlg, FALSE);
				break;
			case IDC_TRANS_NEG_CC: 
				CheckNegoClearChannel(hwndDlg, FALSE);
				break;
			case IDC_TRANS_TEXT:
			case IDC_TRANS_BINARY:
				CheckBinaryMode(hwndDlg, FALSE);
				break;
			case IDC_TRANS_PATHNAMES: 
				CheckUsePathnames(hwndDlg, FALSE);
				break;
			case IDC_TRANS_KEEP_PARTIAL: 
				CheckKeepPartial(hwndDlg, FALSE);
				break;
			case IDC_TRANS_LITERAL_FN: 
				CheckLiteralFilenames(hwndDlg, FALSE);
				break;
			case IDC_TRANS_16BIT_CRC: 
				Check16bitCRC(hwndDlg, FALSE);
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

					CheckTransferProtocolDDL(hwndDlg, TRUE);
					CheckPerformanceDDL(hwndDlg, TRUE);
					CheckUnprefixingDDL(hwndDlg, TRUE);
					CheckAutoDlDDL(hwndDlg, TRUE);
					CheckFnCollisionDDL(hwndDlg, TRUE);
					CheckFileCharset(hwndDlg, TRUE);
					CheckTransferCharset(hwndDlg, TRUE);

					CheckPacketLength(hwndDlg, TRUE);
					CheckWindowSize(hwndDlg, TRUE);

					CheckNegoStreaming(hwndDlg, TRUE);
					CheckNegoClearChannel(hwndDlg, TRUE);
					CheckBinaryMode(hwndDlg, TRUE);
					CheckUsePathnames(hwndDlg, TRUE);
					CheckKeepPartial(hwndDlg, TRUE);
					CheckLiteralFilenames(hwndDlg, TRUE);
					Check16bitCRC(hwndDlg, TRUE);

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

UINT CALLBACK FileTransferPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp) {
	switch (uMsg) {
	case PSPCB_CREATE:
		profile = (ConnectionProfile*)ppsp->lParam;
		return TRUE;
	case PSPCB_RELEASE:
		return 0;
	}
	return 0;
}
