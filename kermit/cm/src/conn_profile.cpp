#include <tchar.h>
#include <stdio.h>

#include "conn_profile.h"
#include "util.h"
#include "ipc_messages.h"
#include "kerm_track.h"
#include "term_info.h"
#include "charset.h"

// Maximum length for a line in a generated script
#define BUFFERSIZE 3000

void ConnectionProfile::setTargetName() {
	if (_targetName != NULL) free(_targetName);
	_targetName = NULL;

	switch(connectionType()) {
	case CT_SSH:
	case CT_FTP:
	case CT_IP: 
		if (!hostname().isNullOrWhiteSpace()) {
			LPTSTR temp[50];
			unsigned int p = port();
			if (p > 0) {
				int len = hostname().length() + _tcslen(_itot(p,(LPTSTR)temp,10)) + 2;
				_targetName = (LPTSTR)malloc(sizeof(TCHAR) * len);				
				_sntprintf(_targetName, len, TEXT("%s:%d"), hostname().data(), p);
			} else {
				int len = hostname().length() + 1;
				_targetName = (LPTSTR)malloc(sizeof(TCHAR) * len);				
				_sntprintf(_targetName, len, TEXT("%s"), hostname().data());
			}
		}
		break;
	case CT_SERIAL:
		if (!line().isNullOrWhiteSpace()) {
			int len = line().length() + 1;
			_targetName = (LPTSTR)malloc(sizeof(TCHAR) * len);				
			_sntprintf(_targetName, len, TEXT("%s"), line().data());
		}
		break;
	case CT_LAT:
	case CT_CTERM:
		if (!latService().isNullOrWhiteSpace()) {
			int len = latService().length() + 1;
			_targetName = (LPTSTR)malloc(sizeof(TCHAR) * len);				
			_sntprintf(_targetName, len, TEXT("%s"), latService().data());
		}
		break;
	case CT_NAMED_PIPE:
		if (!namedPipeHost().isNullOrWhiteSpace() 
			&& !namedPipeName().isNullOrWhiteSpace()) {

			int len = namedPipeHost().length() + namedPipeName().length() + 4;
			_targetName = (LPTSTR)malloc(sizeof(TCHAR) * len);				
			_sntprintf(_targetName, len, TEXT("\\\\%s\\%s"), namedPipeHost().data(), namedPipeName().data());
		}
		break;
	case CT_TEMPLATE:
		{
			int len = _tcslen(TEXT("(template)")) + 1;
			_targetName = (LPTSTR)malloc(sizeof(TCHAR) * len);				
			_sntprintf(_targetName, len, TEXT("%s"), TEXT("(template)"));
		}
		break;
	case CT_PTY:
	case CT_PIPE:
		if (!ptyCommand().isNullOrWhiteSpace()) {
			int len = ptyCommand().length() + 1;
			_targetName = (LPTSTR)malloc(sizeof(TCHAR) * len);				
			_sntprintf(_targetName, len, TEXT("%s"), ptyCommand().data());
		}
		break;
	case CT_DLL:
		if (!dllName().isNullOrWhiteSpace()){

			int len = dllName().length() + dllParameters().length() + 2;
			_targetName = (LPTSTR)malloc(sizeof(TCHAR) * len);				
			_sntprintf(_targetName, len, TEXT("%s %s"), 
				dllName().data(), dllParameters().data());
		}
		break;
	}
}



// TODO: Do we actually need to return the process handle? would a process ID
//		 be OK instead?
// Returns the created process handle
DWORD ConnectionProfile::connect(HWND parent) {
	KermitInstance *inst;

	// Prevent the user from making multiple connections to the same serial
	// profile as serial ports can't be shared.
	if (connectionType() == CT_SERIAL) {

		// TODO: We should probably actually check for any other connections
		//       to the same serial port as the user could configure multiple
		//       profiles against the same port.

		inst = KermitInstance::getInstanceByProfile(this);

		while (inst != NULL) {
			if (inst->status() == KermitInstance::S_CONNECTED) {
				MessageBox(parent, 
					TEXT("There is already a connection open for this profile"), 
					TEXT("Error"), MB_OK | MB_ICONWARNING);
				return NULL;
			}

			inst = inst->nextInstanceWithSameProfile(TRUE);
		}
		inst = NULL;
	}



	inst = KermitInstance::nextAvailableInstance();

	// Get a temporary filename to write the connect script to
	LPTSTR tempFileName = (LPTSTR)malloc(sizeof(TCHAR) * MAX_PATH);
	LPTSTR tempFilePath = (LPTSTR)malloc(sizeof(TCHAR) * MAX_PATH);

	{	
		/*  This code will put the temp directory under the directory
		    where the executable lives. Nice and tidy, but it might
			fail in the future if the app ends up being installed somewere
			like Program Files.
		
		//GetModuleFileName(NULL, tempFileName, MAX_PATH);

		// We want only the path - actual module filename.
		int lastSlashPos = 0;

		// Look for the last path separator in the directory.
		for (int i = 0; i < MAX_PATH; i++) {
			if (tempFileName[i] == _T('\\')) {
				lastSlashPos = i;
			}
		}

		// And slap a null termination right after the last slash in the string.
		if (lastSlashPos > 0 && lastSlashPos + 1 < MAX_PATH) {
			tempFileName[lastSlashPos + 1] = NULL;
		}
		
		*/

		GetTempPath(MAX_PATH, tempFileName);

		_sntprintf(tempFilePath, MAX_PATH, TEXT("%sckermit\\"), tempFileName);

		if (!CreateDirectory(tempFilePath, NULL)) {
			if (GetLastError() != ERROR_ALREADY_EXISTS) {
				MessageBox(parent, 
					TEXT("Failed to create temporary directory"), 
					TEXT("Error"), MB_OK | MB_ICONWARNING);
				return NULL;
			}
		}
	}

	int instId = inst != NULL ? inst->instanceId() : KermitInstance::nextInstanceId();

	GetTempFileName(tempFilePath, TEXT("cms"), 0, tempFileName);

	free(tempFilePath);

	// have temp filename, now generate a script.

	BOOL rc = writeScript(parent, tempFileName);
	
	if (!rc) {
		free(tempFileName);
		return NULL;
	}

	if (inst != NULL) {
		// We've found an existing C-Kermit instance thats indicated its
		// ready to be re-used. Tell it to run the script we made for it.
		inst->takeScript(tempFileName, parent);
		
		free(tempFileName);

		return inst->processId();
	}

	LPTSTR command = (LPTSTR)malloc(sizeof(TCHAR) * BUFFERSIZE);

	BOOL consoleCreated = FALSE;

	STARTUPINFO si ;
    memset( &si, 0, sizeof(STARTUPINFO) ) ;
    si.cb = sizeof(STARTUPINFO);

	if (useGUIKermit()) {
		int len = fontName().length();
		LPTSTR font = (LPTSTR)malloc(sizeof(TCHAR) * (len + 1));

		_tcsncpy(font, fontName().data(), len);

		// Transform font name
		for (int i = 0; i < len; i++) {
			font[i] = _totlower(font[i]);
			switch(font[i]) {
			case _T(' '):
				font[i] = _T('_');
				break;
			case _T(','):
				font[i] = _T('.');
				break;
			case _T(';'):
				font[i] = _T(':');
				break;
			case _T('\\'):
				font[i] = _T('/');
				break;
			case _T('?'):
				font[i] = _T('!');
				break;
			case _T('{'):   
				font[i] = _T('[');
				break;
			case _T('}'):
				font[i] = _T(']');
				break;
			}
		}

		if (customStartupPosition()) {
			_sntprintf(command, BUFFERSIZE, 
				TEXT("k95g.exe \"%s\" -W %d %d --xpos:%d --ypos:%d --facename:%s --fontsize:%d%s%s%s"), 
				tempFileName, parent, KermitInstance::nextInstanceId(), 
				startupAtX(), startupAtY(),
				font, fontSize(),
				(removeBars() && !menubarEnabled())   ? TEXT(" --nomenubar")   : TEXT(""),
				(removeBars() && !toolbarEnabled())   ? TEXT(" --notoolbar")   : TEXT(""),
				(removeBars() && !statusbarEnabled()) ? TEXT(" --nostatusbar") : TEXT("")
				);
		} else {
			_sntprintf(command, BUFFERSIZE, 
				TEXT("k95g.exe \"%s\" -W %d %d --facename:%s --fontsize:%d%s%s%s"), 
				tempFileName, parent, KermitInstance::nextInstanceId(), 
				font, fontSize(),
				(removeBars() && !menubarEnabled())   ? TEXT(" --nomenubar")   : TEXT(""),
				(removeBars() && !toolbarEnabled())   ? TEXT(" --notoolbar")   : TEXT(""),
				(removeBars() && !statusbarEnabled()) ? TEXT(" --nostatusbar") : TEXT("")
				);
		}

		free(font);
	} else {
		_sntprintf(command, BUFFERSIZE, 
				TEXT("k95.exe \"%s\" -W %d %d"), 
				tempFileName, parent, KermitInstance::nextInstanceId());

		// Apparently Windows 95 doesn't create console windows properly when
		// starting applications via CreateProcess so we've got to do it ourselves
		// there.
		OSVERSIONINFO vi ;
		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
		GetVersionEx( &vi ) ;
    

		
		if ( vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
			AllocConsole();
			consoleCreated = TRUE;

			HANDLE hOut, hIn;

			hOut = CreateFile(
				TEXT("CONOUT$"), 
				GENERIC_READ | GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				0) ;
			hIn = CreateFile(
				TEXT("CONIN$"), 
				GENERIC_READ | GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				0) ;

			si.dwFlags = (DWORD) STARTF_USESTDHANDLES;

			DuplicateHandle(
				GetCurrentProcess(), 
				hOut, 
				GetCurrentProcess(), 
				&si.hStdOutput,
				DUPLICATE_SAME_ACCESS,
				TRUE, 
				DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS
				);

			si.hStdError = si.hStdOutput;
			DuplicateHandle(
				GetCurrentProcess(),
				hIn,
				GetCurrentProcess(), 
				&si.hStdInput,
				DUPLICATE_SAME_ACCESS, TRUE, 
				DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS
				) ;
		}

		si.dwXCountChars = (DWORD) 80;
		si.dwYCountChars = (DWORD) screenHeight() + (statusLineEnabled()? 1 : 0);
		si.dwFillAttribute = (DWORD) (commandForegroundColor()) 
							| (commandBackgroundColor() << 4) ;
		si.dwFlags |= (DWORD) STARTF_USECOUNTCHARS 
				   | STARTF_USEFILLATTRIBUTE 
				   | STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOWMAXIMIZED;
    }

	// TODO: free tempFileName? We don't need it anymore unless its to delete it.

	// Ready to launch Kermit!

	PROCESS_INFORMATION pi;

	rc = CreateProcess((LPTSTR)NULL,		// executable (if not in command line)
			command,						// command line
			(LPSECURITY_ATTRIBUTES)NULL,	// process attributes
			(LPSECURITY_ATTRIBUTES)NULL,	// thread attributes
			FALSE,							// inherit handles
			(DWORD) CREATE_NEW_PROCESS_GROUP, // creation flags - probably only important for console
			(LPVOID)NULL,					// environment
			(LPTSTR)NULL,					// current directory
			&si,							// startup info
			&pi								// process info
			);


	if (consoleCreated) {
		FreeConsole();
	}
	
	free(command);

	free(tempFileName);

	// TODO: clean up the script?

	if (rc) {
		return pi.dwProcessId;
	} else {
		return NULL;
	}
}


LPCTSTR colorToString(Term::Color color) {
	Term::ColorInfo info = Term::getColorInfo(color);
	return info.keyword;
}


LPCTSTR telOptToString(ConnectionProfile::NegotiateOption opt) {
	switch(opt) {
	case ConnectionProfile::NEG_REFUSE:
		return TEXT("refused");
	case ConnectionProfile::NEG_REQUEST:
		return TEXT("request");
	case ConnectionProfile::NEG_REQUIRE:
		return TEXT("require");
	case ConnectionProfile::NEG_ACCEPT:
	default:
		return TEXT("accepted");
	}
}


inline BOOL ScriptWriteLine(HANDLE hFile, LPTSTR string, HWND parent) {
	int len = _tcslen(string) * sizeof(TCHAR);
	BOOL rc, rc2;
	DWORD bytesWritten;

	rc = WriteFile(hFile, string, len, &bytesWritten, NULL);
	rc2 = WriteFile(hFile, TEXT("\r\n"), 2*sizeof(TCHAR), &bytesWritten, NULL);

	if (!rc || !rc2) {
		MessageBox(parent, TEXT("Write failed."), TEXT("Error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}
	return TRUE;
}

inline BOOL ScriptWriteGuiColor(HANDLE hFile, HWND parent, LPTSTR buf, 
								Term::Color color, 
								RGB8BitColor colorValue) {

	_sntprintf(buf, BUFFERSIZE, TEXT("  set gui rgbcolor %s %d %d %d"),
		colorToString(color), colorValue.r, colorValue.g, colorValue.b);
	return ScriptWriteLine(hFile, buf, parent);
}

#define OutLine(x) if(!ScriptWriteLine(hFile, x, parent)) { free(buf); return FALSE; }
#define OutColorLine(x, y) if(!ScriptWriteGuiColor(hFile, parent, buf, x, y)) { free(buf); return FALSE; }

BOOL ConnectionProfile::writeScript(HWND parent, LPTSTR filename) {
	LPTSTR buf = (LPTSTR)malloc(sizeof(TCHAR) * BUFFERSIZE);
	AppVersion ver = GetAppVersion(NULL);
	HANDLE hFile;

	ZeroMemory(buf, sizeof(TCHAR) * BUFFERSIZE);

	hFile = CreateFile(
		filename,
		GENERIC_WRITE,
		0,						// Don't share
		NULL,					// default security
		CREATE_ALWAYS,			// Overwrite the file if it already exists
		FILE_ATTRIBUTE_NORMAL,	// normal file
		NULL);					// No attributes template

	if (hFile == INVALID_HANDLE_VALUE) {
		_sntprintf(buf, BUFFERSIZE, TEXT("Failed to open file: %s"), filename);

		MessageBox(parent, buf, TEXT("Error"), MB_OK | MB_ICONWARNING);
		free(buf);
		return FALSE; // Failed to open file.
	}
			

#ifdef UNICODE
	// Write UTF-16 LE byte order mark. We use this format rather than UTF-8
	// because *all* versions of Windows NT are guaranteed to understand it.
	DWORD bytesWritten;
	if (!WriteFile(hFile, "\xFF\xFE", 2, &bytesWritten, NULL)) {
		MessageBox(parent, TEXT("Failed to write UTF-16LE BOM."), TEXT("Error"), MB_OK | MB_ICONWARNING);
		free(buf);
		return FALSE;
	}
#endif

	_sntprintf(buf, BUFFERSIZE, TEXT("; Kermit 95 Connection Manager Generated Script - Version %d.%d.%d %s"),
             ver.major, ver.minor, ver.revision,
#ifdef UNICODE
			 TEXT("UNICODE")
#else
			 TEXT("ANSI")
#endif
			 );
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("; \r\n; Profile: %s"), name().data());
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("; \r\n; Description: %s\r\n; "), description().data());
	OutLine(buf);


	// Start logging
	if (logDebug()) {
		_sntprintf(buf, BUFFERSIZE, TEXT("log debug {%s}%s"), logDebugFile().data(),
			logAppendDebug() ? TEXT(" append") : TEXT(""));
		OutLine(buf);	
	}
	if (logConnections()) {
		_sntprintf(buf, BUFFERSIZE, TEXT("log connection {%s}%s"), logConnectionsFile().data(),
			logAppendConnections() ? TEXT(" append") : TEXT(""));
		OutLine(buf);	
	}
	if (logSessionInput()) {
		_sntprintf(buf, BUFFERSIZE, TEXT("log session {%s}%s"), logSessionInputFile().data(),
			logAppendSessionInput() ? TEXT(" append") : TEXT(""));
		OutLine(buf);
		switch(logSessionInputMode()) {
		case LSM_TEXT:
			OutLine(TEXT("set session-log text"));
			break;
		case LSM_BINARY:
			OutLine(TEXT("set session-log binary"));
			break;
		case LSM_DEBUG:
			OutLine(TEXT("set session-log debug"));
			break;
		}
	}
	if (logKermitFileTransfers()) {
		_sntprintf(buf, BUFFERSIZE, TEXT("log packet {%s}%s"), logKermitFileTransfersFile().data(),
			logAppendKermitFileTransfers() ? TEXT(" append") : TEXT(""));
		OutLine(buf);	
	}
	if (logFileTransferTransactions()) {
		_sntprintf(buf, BUFFERSIZE, TEXT("set transaction-log %s"),
			logBriefFileTransferTransactions() ? TEXT(" brief") : TEXT("verbose"));
		OutLine(buf)

		_sntprintf(buf, BUFFERSIZE, TEXT("log transaction {%s}%s"), 
			logFileTransferTransactionsFile().data(),
			logAppendFileTransferTransactions() ? TEXT(" append") : TEXT(""));
		OutLine(buf);	
	}

	// Session title
	_sntprintf(buf, BUFFERSIZE, TEXT("set title %s"), name().data());
	OutLine(buf);

	// Command settings
	_sntprintf(buf, BUFFERSIZE, TEXT("set command color %s %s"), 
		colorToString(commandForegroundColor()), 
		colorToString(commandBackgroundColor()));
	OutLine(buf);

	OutLine(TEXT("clear command"));
	
	if (useGUIKermit()) {
		OutLine(TEXT("if gui {"));

		_sntprintf(buf, BUFFERSIZE, TEXT("  set gui window resize-mode %s"), 
			resizeChangesDimensions() ? TEXT("change-dimensions") 
									 : TEXT("scale-font"));
		OutLine(buf);

		_sntprintf(buf, BUFFERSIZE, TEXT("  set gui window run-mode %s"), 
			windowStartupMode() == WSM_MAXIMIZED ? TEXT("maximize") :
			windowStartupMode() == WSM_MINIMIZED ? TEXT("minimize") :
			TEXT("restore"));
		OutLine(buf);

		if (!removeBars()) {
			if (!menubarEnabled()) {
				OutLine(TEXT("  set gui menubar visible off"));
			}
			if (!toolbarEnabled()) {
				OutLine(TEXT("  set gui toolbar visible off"));
			}
			if (!statusbarEnabled()) {
				OutLine(TEXT("  set gui statusbar off"));
			}
		}

		_sntprintf(buf, BUFFERSIZE, TEXT("  set gui dialogs %s"), 
			dialogsEnabled() ? TEXT("on") : TEXT("off"));
		OutLine(buf);

		// Output all the colour definitions
		OutColorLine(Term::COLOR_BLACK, rgbBlack());
		OutColorLine(Term::COLOR_BLUE, rgbBlue());
		OutColorLine(Term::COLOR_GREEN, rgbGreen());
		OutColorLine(Term::COLOR_CYAN, rgbCyan());
		OutColorLine(Term::COLOR_RED, rgbRed());
		OutColorLine(Term::COLOR_MAGENTA, rgbMagenta());
		OutColorLine(Term::COLOR_BROWN, rgbBrown());
		OutColorLine(Term::COLOR_LIGHT_GRAY, rgbLightGray());
		OutColorLine(Term::COLOR_DARK_GRAY, rgbDarkGray());
		OutColorLine(Term::COLOR_LIGHT_BLUE, rgbLightBlue());
		OutColorLine(Term::COLOR_LIGHT_GREEN, rgbLightGreen());
		OutColorLine(Term::COLOR_LIGHT_CYAN, rgbLightCyan());
		OutColorLine(Term::COLOR_LIGHT_RED, rgbLightRed());
		OutColorLine(Term::COLOR_LIGHT_MAGENTA, rgbLightMagenta());
		OutColorLine(Term::COLOR_YELLOW, rgbYellow());
		OutColorLine(Term::COLOR_WHITE, rgbWhite());
		

		OutLine(TEXT("}"));
	}

	// New setting
	if (!startingDirectory().isNull()) {
		_sntprintf(buf, BUFFERSIZE, TEXT("cd {%s}"), 
				startingDirectory().data());
		OutLine(buf);
	}

	// Set applications
	if (!appBrowser().isNullOrWhiteSpace()) {
		_sntprintf(buf, BUFFERSIZE, TEXT("set browser {%s}"), 
			appBrowser().data());
		OutLine(buf);
	}

	if (!appEditor().isNullOrWhiteSpace()) {
		_sntprintf(buf, BUFFERSIZE, TEXT("set editor {%s}"), 
			appEditor().data());
		OutLine(buf);
	}

	// Terminal settings
	if (terminalType() != Term::TT_INVALID) {
		_sntprintf(buf, BUFFERSIZE, TEXT("set terminal type %s"), 
			Term::getTermKeyword(terminalType()));
		OutLine(buf);
	} else {
		OutLine(TEXT("echo No terminal type specified\r\nstop"));
	}

	// Telnet settings
	if (connectionType() == CT_IP/* || connectionType() == CT_FTP*/) {
		// The K95 Dialer also included FTP here, though I don't know
		// why as none of these settings appy to an FTP connection.

		if (!telnetEnvarTerminalType().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("set telnet terminal-type %s"), 
					telnetEnvarTerminalType().data());
			OutLine(buf);
		}

		_sntprintf(buf, BUFFERSIZE, TEXT("set telopt authentication %s"), 
				telOptToString(telnetAuthenticationOption()));
		OutLine(buf);

		// This one is awkward
		{
			NegotiateOption optA, optB, opt = telnetBinaryModeOption();
			
			switch(opt) {
				case NEG_REQUEST:
					optA = NEG_REQUEST;
					optB = NEG_ACCEPT;
					break;
				case NEG_REQUIRE:
					optA = NEG_REQUIRE;
					optB = NEG_ACCEPT;
					break;
				case NEG_ACCEPT:
				case NEG_REFUSE:
				default:
					optA = opt;
					optB = opt;
			}

			_sntprintf(buf, BUFFERSIZE, TEXT("set telopt binary %s %s"), 
				telOptToString(optA), telOptToString(optB));
			OutLine(buf);
		}

		_sntprintf(buf, BUFFERSIZE, TEXT("set telopt encryption %s %s"), 
				telOptToString(telnetEncryptionOption()),
				telOptToString(telnetEncryptionOption()));
		OutLine(buf);

		_sntprintf(buf, BUFFERSIZE, TEXT("set telopt kermit %s %s"), 
				telOptToString(telnetKermitOption()),
				telOptToString(telnetKermitOption()));
		OutLine(buf);

		_sntprintf(buf, BUFFERSIZE, TEXT("set telopt forward-x %s"), 
				telOptToString(telnetForwardXOption()));
		OutLine(buf);

		_sntprintf(buf, BUFFERSIZE, TEXT("set telopt com-port-control %s"), 
				telOptToString(telnetComPortControlOption()));
		OutLine(buf);

		
		OutLine(TEXT("if available ssl {"));
		_sntprintf(buf, BUFFERSIZE, TEXT("  set telopt start-tls %s"), 
				telOptToString(telnetStartTLSOption()));
		OutLine(buf);
		OutLine(TEXT("}"));

		if (telnetDebugMode()) {
			OutLine(TEXT("set telnet debug on"));
		} else {
			OutLine(TEXT("set telnet debug off"));
		}

		if (telnetSubnegotiationDelay()) {
			OutLine(TEXT("set telnet delay-sb on"));
		} else {
			OutLine(TEXT("set telnet delay-sb off"));
		}

		if (!telnetEnvarLocation().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("set telnet location %s"), 
					telnetEnvarLocation().data());
			OutLine(buf);
		}

		if (telnetForwardCredentials()) {
			OutLine(TEXT("set telnet authentication forwarding on"));
		} else {
			OutLine(TEXT("set telnet authentication forwarding off"));
		}

		if (telnetNegotiationsMustComplete()) {
			OutLine(TEXT("set telnet wait-for-negotiations on"));
		} else {
			OutLine(TEXT("set telnet wait-for-negotiations off"));
		}

		if (!telnetEnvarAccount().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("set telnet environment acct %s"), 
				telnetEnvarAccount().data());
			OutLine(buf);
		}

		if (!telnetEnvarDisplay().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("set telnet environment disp %s"), 
				telnetEnvarDisplay().data());
			OutLine(buf);
		}

		if (!telnetEnvarJob().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("set telnet environment job %s"), 
				telnetEnvarJob().data());
			OutLine(buf);
		}

		if (!telnetEnvarPrinter().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("set telnet environment printer %s"), 
				telnetEnvarPrinter().data());
			OutLine(buf);
		}
	}

	// TCP/IP options - SSH excluded for now as CKW doesn't support proxying SSH yet
	if (connectionType() == CT_IP || connectionType() == CT_FTP
		/* || connectionType() == CT_SSH */) {

		// TODO: set tcp address %s

		_sntprintf(buf, BUFFERSIZE, TEXT("set tcp sendbuf %d"), 
				tcpSendBufferSize());
		OutLine(buf);

		_sntprintf(buf, BUFFERSIZE, TEXT("set tcp recvbuf %d"), 
				tcpReceiveBufferSize());
		OutLine(buf);

		switch(tcpReverseDNSLookup()) {
		case RDO_ON:
			OutLine(TEXT("set tcp reverse-dns-lookup on"));
			break;
		case RDO_OFF:
			OutLine(TEXT("set tcp reverse-dns-lookup off"));
			break;
		case RDO_AUTO:
			OutLine(TEXT("set tcp reverse-dns-lookup auto"));
			break;
		}

		if (!tcpSocksHostname().isNullOrWhiteSpace()) {
			if (tcpSocksPort() > 0) {
				_sntprintf(buf, BUFFERSIZE, TEXT("set tcp socks-server /user:%s /pass:%s %s:%d"), 
					tcpSocksUser().data(), 
					tcpSocksPassword().data(), 
					tcpSocksHostname().data(), 
					tcpSocksPort());
			} else {
				_sntprintf(buf, BUFFERSIZE, TEXT("set tcp socks-server /user:%s /pass:%s %s"), 
					tcpSocksUser().data(), 
					tcpSocksPassword().data(), 
					tcpSocksHostname().data());
			}
			OutLine(buf);
		}
		
		if (!tcpHttpHostname().isNullOrWhiteSpace()) {
			if (tcpSocksPort() > 0) {
				_sntprintf(buf, BUFFERSIZE, TEXT("set tcp http-proxy /user:%s /pass:%s %s:%d"), 
					tcpHttpUser().data(), 
					tcpHttpPassword().data(), 
					tcpHttpHostname().data(), 
					tcpHttpPort());
			} else {
				_sntprintf(buf, BUFFERSIZE, TEXT("set tcp socks-server /user:%s /pass:%s %s"), 
					tcpHttpUser().data(), 
					tcpHttpPassword().data(), 
					tcpHttpHostname().data());
			}
			OutLine(buf);
		}

		if (tcpDNSServiceLookup()) {
			OutLine(TEXT("set tcp dns-service-records on"));
		} else {
			OutLine(TEXT("set tcp dns-service-records off"));
		}

		/* TODO: Kerberos IV
			if available kerberos4 {
				set authentication kerberos4 princ %s
				set authentication kerperos4 realm %s
				set authentication kerberos4 autoget on/off
				set authentication kerberos4 autodestroy on-close/never
				set authentication kerberos4 lifetime %d
			}
		*/

		/* TODO: Kerberos V
			if available kerberos5 {
				set authentication kerberos5 princ %s
				set authentication kerberos5 realm %s
				set authentication k5 credentials-cache {%s}
				set authentication kerberos5 autoget on/off
				set authentication kerberos5 autodestroy on-close/never
				set authentication kerberos5 lifetime %d
				set authentication kerberos5 forwardable on/off
				set authentication kerberos5 get-k4-tgt on/off
			}
		*/

		OutLine(TEXT("if available ssl {"));

		if (!tlsCipherList().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("  set auth ssl cipher {%s}"), 
					tlsCipherList().data());
			OutLine(buf);
		}

		if (!tlsClientCertificateFile().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("  set auth ssl dsa-cert-file {%s}"), 
					tlsClientCertificateFile().data());
			OutLine(buf);
		}

		if (!tlsClientPrivateKeyFile().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("  set auth ssl dsa-key-file {%s}"), 
					tlsClientPrivateKeyFile().data());
			OutLine(buf);
		}

		if (!tlsCAVerificationFile().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("  set auth ssl verify-file {%s}"), 
					tlsCAVerificationFile().data());
			OutLine(buf);
		}

		if (!tlsCAVerificationDirectory().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("  set auth ssl verify-dir {%s}"), 
					tlsCAVerificationDirectory().data());
			OutLine(buf);
		}

		if (!tlsCRLFile().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("  set auth ssl crl-file {%s}"), 
					tlsCRLFile().data());
			OutLine(buf);
		}

		if (!tlsCRLDirectory().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("  set auth ssl crl-dir {%s}"), 
					tlsCRLDirectory().data());
			OutLine(buf);
		}

		switch(tlsCertificateVerifyMode()) {
		case CVM_DO_NOT_VERIFY:
			OutLine(TEXT("  set auth ssl verify no"));
			break;
		case CVM_VERIFY_HOSTS_IF_PRESENTED:
			OutLine(TEXT("  set auth ssl verify peer-cert"));
			break;
		case CVM_REQUIRED_AND_VERIFIED:
			OutLine(TEXT("  set auth ssl verify fail-if-no-peer-cert"));
			break;
		}

		if (tlsVerboseMode()) {
			OutLine(TEXT("  set auth ssl verbose on"));
		} else {
			OutLine(TEXT("  set auth ssl verbose off"));
		}

		if (tlsDebugMode()) {
			OutLine(TEXT("  set auth ssl debug on"));
		} else {
			OutLine(TEXT("  set auth ssl debug off"));
		}

		
		OutLine(TEXT("}"));
	}

	// Terminal settings!
	_sntprintf(buf, BUFFERSIZE, TEXT("set term bytesize %d"), 
		is8Bit() ? 8 : 7);
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("set term echo %s"), 
		localEchoEnabled() ? TEXT("on") : TEXT("off"));
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("set term wrap %s"), 
		autoWrapEnabled() ? TEXT("on") : TEXT("off"));
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("set term apc %s"), 
		apcEnabled() ? TEXT("on") : TEXT("off"));
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("set term autodown %s"), 
		autoDownloadMode() == AD_ASK ? TEXT("ask") : 
		autoDownloadMode() == AD_YES ? TEXT("on") : TEXT("off"));
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("set term status %s"), 
		statusLineEnabled() ? TEXT("on") : TEXT("off"));
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("set term width %d"), 
		screenWidth());
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("set term height %d"), 
		screenHeight());
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("set term scrollback %d"), 
		scrollbackLines());
	OutLine(buf);

	switch(cursor()) {
	case CUR_FULL:
		OutLine(TEXT("set term cursor full"));
		break;
	case CUR_HALF:
		OutLine(TEXT("set term cursor half"));
		break;
	case CUR_UNDERLINE:
		OutLine(TEXT("set term cursor underline"));
		break;
	}

	// Terminal colour settings
	_sntprintf(buf, BUFFERSIZE, TEXT("set term color term %s %s"), 
		colorToString(terminalForegroundColor()), 
		colorToString(terminalBackgroundColor()));
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("set term color status %s %s"), 
		colorToString(statusLineForegroundColor()), 
		colorToString(statusLineBackgroundColor()));
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("set term color help %s %s"), 
		colorToString(popupHelpForegroundColor()), 
		colorToString(popupHelpBackgroundColor()));
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("set term color selection %s %s"), 
		colorToString(mouseSelectionForegroundColor()), 
		colorToString(mouseSelectionBackgroundColor()));
	OutLine(buf);

	_sntprintf(buf, BUFFERSIZE, TEXT("set term color underline %s %s"), 
		colorToString(underlineSimulationForegroundColor()), 
		colorToString(underlineSimulationBackgroundColor()));
	OutLine(buf);

	{
		BOOL suffix = FALSE;

		// If the terminal type is at386, scoansi or ansi and the
		// selected charset is not "transparent" then the K95 dialer
		// appends " g1" to this command.
		if (terminalType() == Term::TT_AT386
			|| terminalType() == Term::TT_SCOANSI
			|| terminalType() == Term::TT_ANSI) {

			if (characterSet() == Charset::CS_TRANSP) {
				suffix = TRUE;
			}
		}
		
		_sntprintf(buf, BUFFERSIZE, TEXT("set term remote-char %s%s"), 
			Charset::getCharsetKeyword(characterSet()), suffix ? TEXT(" g1") : TEXT(""));
		OutLine(buf);
	}

	// The one and only exit setting
	_sntprintf(buf, BUFFERSIZE, TEXT("set exit on-disconnect %s"), 
		exitOnDisconnect() ? TEXT("on") : TEXT("off"));
	OutLine(buf);

	// Printer settings
	switch(printerType()) {
	case PT_WINDOWS:
		{
			if (windowsPrintQueue() == CMString(DEFAULT_WIN_PRINT_QUEUE)) {
				OutLine(TEXT("set printer /WINDOWS-QUEUE:"));
			} else {
				LPTSTR q = _tcsdup(windowsPrintQueue().data());

				// Transform to keyword format! There are *probably* more unicode
				// characters we should be replacing here, but K95 doesn't handle
				// unicode scripts yet anyway so we just do what K95 does...
				TCHAR *c = q;
				while (*c) {
					switch ( *c ) {
					case ' ':
						*c = '_';
						break;
					case ',':
						*c = '.';
						break;
					case ';':
						*c = ':';
						break;
					case '\\':
						*c = '/';
						break;
					case '?':
						*c = '!';
						break;
					case '{':
						*c = '[';
						break;
					case '}':
						*c = ']';
						break;
					}
					c++;
				}


				_sntprintf(buf, BUFFERSIZE, TEXT("set printer /WINDOWS-QUEUE:%s"), q);
				free(q);
				OutLine(buf);
			}
		}
		break;
	case PT_COMMAND: // (pipe)
		{
			if (deviceName().isNullOrWhiteSpace()) {
				OutLine(TEXT("set printer /NONE ; Pipe printer name not specified"));
			} else {
				_sntprintf(buf, BUFFERSIZE, TEXT("set printer /PIPE:{%s}"), 
						deviceName().data());
				OutLine(buf);
			}
		}
		break;
	case PT_FILE:
		{
			if (deviceName().isNullOrWhiteSpace()) {
				OutLine(TEXT("set printer /NONE ; File printer name not specified"));
			} else {
				_sntprintf(buf, BUFFERSIZE, TEXT("set printer /FILE:{%s}"), 
						deviceName().data());
				OutLine(buf);
			}
		}
		break;
	case PT_DOS_DEVICE:
		{
			LPCTSTR flow, parity;

			switch(printParity()) {
			case PAR_NONE:
				parity = TEXT("none");
				break;
			case PAR_SPACE:
				parity = TEXT("space");
				break;
			case PAR_MARK:
				parity = TEXT("mark");
				break;
			case PAR_EVEN:
				parity = TEXT("even");
				break;
			case PAR_ODD:
				parity = TEXT("odd");
				break;
			}

			switch(printFlowControl()) {
			case FC_NONE:
				flow = TEXT("none");
				break;
			case FC_XON_XOFF:
				flow = TEXT("xon/xoff");
				break;
			case FC_RTS_CTS:
				flow = TEXT("rts/cts");
				break;
			case FC_AUTO:
				flow = TEXT("auto");
				break;
			}

			_sntprintf(buf, BUFFERSIZE, TEXT("set printer /DOS-DEVICE:{%s} %s/SPEED:%d /PARITY:%s /FLOW:%s"), 
				deviceName().isNullOrWhiteSpace() ? TEXT("prn") : deviceName().data(), 
				bidirectionalPrinting() ? TEXT("/BIDIRECTIONAL"):TEXT(""),
				printSpeed(), parity, flow);
			OutLine(buf);
		}
		break;
	case PT_NONE:
		OutLine(TEXT("set printer /NONE"));
		break;
	default:
		{
			if (deviceName().isNullOrWhiteSpace()) {
				OutLine(TEXT("set printer /NONE"));
			} else {
				_sntprintf(buf, BUFFERSIZE, TEXT("set printer {%s}"), 
						deviceName().data());
				OutLine(buf);
			}
		}
		break;
	}

	if (!headerFile().isNullOrWhiteSpace()) {
		_sntprintf(buf, BUFFERSIZE, TEXT("set printer /JOB-HEADER-FILE:{%s}"), 
					headerFile().data());
		OutLine(buf);
	}

	if (!sendEndOfJobString()) {
		OutLine(TEXT("set printer /END-OF-JOB-STRING:{\\012}"));
	} else {
		if (!endOfJobString().isNullOrWhiteSpace()) {
			_sntprintf(buf, BUFFERSIZE, TEXT("set printer /END-OF-JOB-STRING:{%s}"), 
					endOfJobString().data());
			OutLine(buf);
		}
	}

	_sntprintf(buf, BUFFERSIZE, TEXT("set printer /TIMEOUT:%d"), 
				printWidth(), printHeight());
	OutLine(buf);

	if (printAsPostScript()) {
		_sntprintf(buf, BUFFERSIZE, TEXT("set printer /POSTSCRIPT /WIDTH:%d /LENGTH:%d"), 
					printTimeoutSeconds());
		OutLine(buf);
	}

	if (printCharacterSet() != Charset::CS_INVALID) {
		_sntprintf(buf, BUFFERSIZE, TEXT("set printer /CHARACTER-SET:%s"), 
					Charset::getCharsetKeyword(printCharacterSet()));
		OutLine(buf);
	}
	
	// File transfer settings
	_sntprintf(buf, BUFFERSIZE, TEXT("set file download-directory {%s}"), 
					downloadDirectory().data());
	OutLine(buf);

	if (defaultToBinaryMode()) {
		OutLine(TEXT("set file type binary"));
	} else {
		OutLine(TEXT("set file type text"));
	}

	switch(fileNameCollisionAction()) {
	case FNC_APPEND:
		OutLine(TEXT("set file collision append"));
		break;
	case FNC_BACKUP:
		OutLine(TEXT("set file collision backup"));
		break;
	case FNC_DISCARD:
		OutLine(TEXT("set file collision discard"));
		break;
	case FNC_OVERWRITE:
		OutLine(TEXT("set file collision overwrite"));
		break;
	case FNC_RENAME:
		OutLine(TEXT("set file collision rename"));
		break;
	case FNC_UPDATE:
		OutLine(TEXT("set file collision update"));
		break;
	}

	if (keepIncompleteFiles()) {
		OutLine(TEXT("set file incomplete auto"));
	} else {
		OutLine(TEXT("set file incomplete discard"));
	}

	if (negotiateStreamingTransferMode()) {
		OutLine(TEXT("set streaming auto"));
	} else {
		OutLine(TEXT("set streaming off"));
	}

	if (negotiateClearChannelTransferMode()) {
		OutLine(TEXT("set clear-channel auto"));
	} else {
		OutLine(TEXT("set clear-channel off"));
	}

	if (transmitLiteralFilenames()) {
		OutLine(TEXT("set file names literal"));
	} else {
		OutLine(TEXT("set file names converted"));
	}

	if (usePathnames()) {
		OutLine(TEXT("set receive pathnames on"));
		OutLine(TEXT("set send pathnames on"));
	} else {
		OutLine(TEXT("set receive pathnames off"));
		OutLine(TEXT("set send pathnames off"));
	}

	if (fileCharacterSet() != Charset::CS_INVALID) {
		_sntprintf(buf, BUFFERSIZE, TEXT("set file char %s"),
					Charset::getCharsetKeyword(fileCharacterSet()));
		OutLine(buf);
	}


	// Protocol settings
	{
		BOOL outputUnprefixCC = FALSE;

		switch(fileTransferProtocol()) {
		case FT_KERMIT:
			{
				OutLine(TEXT("set protocol kermit"));

				switch(kermitPerformance()) {
				case KP_FAST:
					OutLine(TEXT("do fast"));
					break;
				case KP_CAUTIOUS:
					// For some reason the K95 Dialer outputs "do fast" for this.
					// Possibly a bug?
					OutLine(TEXT("do cautious"));
					break;
				case KP_ROBUST:
					OutLine(TEXT("do robust"));
					break;
				case KP_CUSTOM:
					{
						_sntprintf(buf, BUFFERSIZE, TEXT("set window-size %d"), 
							windowSize());
						OutLine(buf);

						_sntprintf(buf, BUFFERSIZE, TEXT("set receive packet-length %d"), 
							packetLength());
						OutLine(buf);

						outputUnprefixCC = TRUE;
					}
					break;
				}

				if (force16bitCRC()) {
					OutLine(TEXT("set block 5"));
				}
			}
			break;
		case FT_ZMODEM:
			{
				OutLine(TEXT("set protocol zmodem"));

				_sntprintf(buf, BUFFERSIZE, TEXT("set send window-size %d"), 
					windowSize());
				OutLine(buf);

				outputUnprefixCC = TRUE;
			}
			break;
		case FT_YMODEM:
			{
				OutLine(TEXT("set protocol ymodem"));

				_sntprintf(buf, BUFFERSIZE, TEXT("set send packet-length %d"), 
					packetLength());
				OutLine(buf);

				outputUnprefixCC = TRUE;
			}
			break;
		case FT_YMODEM_G:
			{
				OutLine(TEXT("set protocol ymodem-g"));

				_sntprintf(buf, BUFFERSIZE, TEXT("set send packet-length %d"), 
					packetLength());
				OutLine(buf);

				outputUnprefixCC = TRUE;
			}
			break;
		case FT_XMODEM:
			{
				OutLine(TEXT("set protocol xmodem"));

				_sntprintf(buf, BUFFERSIZE, TEXT("set send packet-length %d"), 
					packetLength());
				OutLine(buf);

				outputUnprefixCC = TRUE;
			}
			break;
		}

		if (outputUnprefixCC) {
			switch(controlCharUnprefixing()) {
			case CCU_NEVER:
				OutLine(TEXT("set prefixing all"));
				break;
			case CCU_CAUTIOUS:
				OutLine(TEXT("set prefixing cautious"));
				break;
			case CCU_WITH_WILD_ABANDON:
				OutLine(TEXT("set prefixing minimal"));
				break;
			}
		}
	}

	_sntprintf(buf, BUFFERSIZE, TEXT("set xfer char %s"), 
				Charset::getCharsetKeyword(transferCharacterSet()));
	OutLine(buf);

	// Mouse settings
	if (mouseEnabled()) {
		OutLine(TEXT("set mouse activate on"));
	} else {
		OutLine(TEXT("set mouse activate off"));
	}

	// Keyboard settings
	switch(keyboardEnterSends()) {
	case KEM_CR:
		{
			OutLine(TEXT("set key \\269  \\13"));
			OutLine(TEXT("set term newline off"));
		}
		break;
	case KEM_CR_LF:
		{
			OutLine(TEXT("set key \\269  \\13"));
			OutLine(TEXT("set term newline on"));
		}
		break;
	case KEM_LF:
		{
			OutLine(TEXT("set key \\269  \\10"));
			OutLine(TEXT("set term newline off"));
		}
		break;
	}

	/* The backspace key is set via the secret undocumented
	 * "set dialer" command so that it can be done in a terminal
	 * type specific manner. The K95 Dialer source code says:
	 *		Use the new backhanded SET DIALER BACKSPACE command to set
	 *		the value of the Backspace key, so that it can be done in
	 *		a terminal type specific manner.  This command does the
	 *		equivalent of SET TERM KEY except in the case of WYSE and
	 *		TVI terminals which by default have a Kverb assigned.  So
	 *		for those two terminal families we must manipulate the UDK
	 *		table.   (What a hack!!!)
	 * The source notes that the keymap file must take precedence over
	 * this setting otherwise would become impossible to customise the
	 * backspace key. So "set dialer backspace" must come before keymaps!
	 */
	switch(keyboardBackspaceSends()) {
	case KBM_DELETE:
		OutLine(TEXT("set dialer backspace \\127"));
		break;
	case KBM_CTRL_H:
		OutLine(TEXT("set dialer backspace \\8"));
		break;
	case KBM_CTRL_Y:
		OutLine(TEXT("set dialer backspace \\25"));
		break;
	}

	if (!keymapFile().isNullOrWhiteSpace()) {
		_sntprintf(buf, BUFFERSIZE, TEXT("take {%s}"), 
				keymapFile().data());
		OutLine(buf);
	}

	if (!additionalKeyMaps().isNullOrWhiteSpace()) {
		OutLine(additionalKeyMaps().toCRLF().data());
	}

	// Login parameters
	BOOL isAnonymous = FALSE;
	if (!userId().isNullOrWhiteSpace()) {
		OutLine(TEXT("set command quoting off"));

		_sntprintf(buf, BUFFERSIZE, TEXT("set login userid %s"), 
					userId().data());
		OutLine(buf);

		OutLine(TEXT("set command quoting on"));

		isAnonymous = userId() == CMString(TEXT("anonymous")) ||
					  userId() == CMString(TEXT("ftp"));
	}

	if (!password().isNullOrWhiteSpace() && !isAnonymous) {
		if (!promptForPassword()) {
			OutLine(TEXT("set command quoting off"));

			_sntprintf(buf, BUFFERSIZE, TEXT("set login password {%s}"), 
							password().data());
			OutLine(buf);

			OutLine(TEXT("set command quoting on"));
		} else {
			// TODO: Prompt the user for the password and output it
			// The K95 Dialer passes it through ck_encrypt() - not sure
			// if thats really worth doing as the algorithm is public and
			// pretty trivial

			// Note: The UI doesn't currently expose the prompt-for-password option.
			// If it were exposed we'd have to explain to the user that its not really
			// any more secure than just saving the password.

			/*LPTSTR pass;

			// TODO: Display password dialog and get result

			OutLine(TEXT("set command quoting off"));

			_sntprintf(buf, BUFFERSIZE, TEXT("set login password {%s}"), 
							pass);
			OutLine(buf);

			OutLine(TEXT("set command quoting on"));*/
		}
	}

	if (!passwordPrompt().isNullOrWhiteSpace()) {
		_sntprintf(buf, BUFFERSIZE, TEXT("set login prompt {%s}"), 
						passwordPrompt().data());
		OutLine(buf);
	}

	// Setup the connection!
	switch(connectionType()) {
	case CT_SERIAL:
		{
			if (carrierDetection()) {
				OutLine(TEXT("set carrier auto"));
			} else {
				OutLine(TEXT("set carrier off"));
			}

			/* TODO: If TAPI Line:
			 * set tapi modem-dialing off
			 * set tapi line %s
			 *
			 * Else...
			 */

			// Not tapi:
			_sntprintf(buf, BUFFERSIZE, TEXT("set port %s"), 
				line().data());
			OutLine(buf);

			_sntprintf(buf, BUFFERSIZE, TEXT("set speed %ld"), 
				lineSpeed());
			OutLine(buf);

			switch(parity()) {
			case PAR_NONE:
				OutLine(TEXT("set parity none"));
				break;
			case PAR_SPACE:
				OutLine(TEXT("set parity space"));
				break;
			case PAR_MARK:
				OutLine(TEXT("set parity mark"));
				break;
			case PAR_EVEN:
				OutLine(TEXT("set parity even"));
				break;
			case PAR_ODD:
				OutLine(TEXT("set parity odd"));
				break;
			case PAR_SPACE_8BIT:
				OutLine(TEXT("set parity hardware space"));
				break;
			case PAR_MARK_8BIT:
				OutLine(TEXT("set parity hardware mark"));
				break;
			case PAR_EVEN_8BIT:
				OutLine(TEXT("set parity hardware even"));
				break;
			case PAR_ODD_8BIT:
				OutLine(TEXT("set parity hardware odd"));
				break;
			}

			switch(stopBits()) {
			case SB_1_0:
				OutLine(TEXT("set stop-bits 1"));
				break;
			case SB_2_0:
				OutLine(TEXT("set stop-bits 2"));
				break;
			}

			switch (flowControl()) {
			case FC_NONE:
				OutLine(TEXT("set flow none"));
				break;
			case FC_XON_XOFF:
				OutLine(TEXT("set flow xon/xoff"));
				break;
			case FC_RTS_CTS:
				OutLine(TEXT("set flow rts/cts"));
				break;
			case FC_AUTO:
				OutLine(TEXT("set flow auto"));
				break;
			}




		}
	case CT_MODEM:
		/*
		Not supported at this time. Probably never will be supported.
		New Zealand won't even *have* land-lines in a few years!
		*/
		break;
	case CT_IP:
		{
			/* We don't currently support network directories.
			 * If we did, we'd let the user specify a list of files and then
			 * hand that list, space-delimited, to C-Kermit with:
			 *		set network directory %s
			 * eg: set network directory file1.txt file2.txt file3.txt etc
			 */
			OutLine(TEXT("set network directory")); // No network directories

			OutLine(TEXT("set network tcp/ip"));
			OutLine(TEXT("if fail end 1 TCP/IP Failed"));

			LPCTSTR port_name, proto_name;
			switch(ipProtocol()) {
			case IPP_DEFAULT:
				port_name = TEXT("telnet");
				proto_name = TEXT("/default");
				break;
			case IPP_TELNET_NEGO:
				port_name = TEXT("telnet");
				proto_name = TEXT("/telnet");
				break;
			case IPP_RLOGIN:
				port_name = TEXT("login");
				proto_name = TEXT("/rlogin");
				break;
			case IPP_RAW:
				port_name = TEXT("telnet");
				proto_name = TEXT("/raw-socket");
				break;
			case IPP_TELNET:
				port_name = TEXT("telnet");
				proto_name = TEXT("/no-telnet-init");
				break;
			case IPP_IKS:
				port_name = TEXT("kermit");
				proto_name = TEXT("/telnet");
				break;
			case IPP_EK4LOGIN:
				port_name = TEXT("eklogin");
				proto_name = TEXT("/ek4login");
				break;
			case IPP_EK5LOGIN:
				port_name = TEXT("eklogin");
				proto_name = TEXT("/ek5login");
				break;
			case IPP_K4LOGIN:
				port_name = TEXT("klogin");
				proto_name = TEXT("/k4login");
				break;
			case IPP_K5LOGIN:
				port_name = TEXT("klogin");
				proto_name = TEXT("/k5login");
				break;
			case IPP_TELNET_SSL:
				port_name = TEXT("telnets");
				proto_name = TEXT("/ssl-telnet");
				break;
			case IPP_TELNET_TLS:
				port_name = TEXT("telnets");
				proto_name = TEXT("/tls-telnet");
				break;
			case IPP_SSL:
				port_name = TEXT("https");
				proto_name = TEXT("/ssl");
				break;
			case IPP_TLS:
				port_name = TEXT("https");
				proto_name = TEXT("/tls");
				break;
			}

			if (hostname().isNullOrWhiteSpace()) {
				OutLine(TEXT("end 1 No host provided"));
			} else {
				if (hostname().contains(L':') ||
					hostname().contains(L' ')) {

					_sntprintf(buf, BUFFERSIZE, TEXT("set host %s"), 
						hostname().data());
				} else if (port() > 0) {
					_sntprintf(buf, BUFFERSIZE, TEXT("set host %s %d %s"), 
						hostname().data(), port(), proto_name);
				} else {
					_sntprintf(buf, BUFFERSIZE, TEXT("set host %s %s %s"), 
						hostname().data(), port_name, proto_name);
				}
				OutLine(buf);
			}
		}
		break;
	case CT_SSH:
		{
			/* We don't currently support network directories.
			 * If we did, we'd let the user specify a list of files and then
			 * hand that list, space-delimited, to C-Kermit with:
			 *		set network directory %s
			 * eg: set network directory file1.txt file2.txt file3.txt etc
			 */
			OutLine(TEXT("set network directory")); // No network directories

			OutLine(TEXT("set network type ssh"));
			OutLine(TEXT("if fail end 1 SSH Failed"));

			/*
			 * set ssh version automatic/1/2
			 *	We only support SSH v2 so no need to do this anymore
			 */

			if (sshCompressionEnabled()) {
				OutLine(TEXT("set ssh compression on"));
			} else {
				OutLine(TEXT("set ssh compression off"));
			}

			if (sshX11ForwardingEnabled()) {
				OutLine(TEXT("set ssh x11-forwarding on"));
			} else {
				OutLine(TEXT("set ssh x11-forwarding off"));
			}

			switch(sshHostKeyChecking()) {
			case SHKC_ON:
				OutLine(TEXT("set ssh strict-host-key-check on"));
				break;
			case SHKC_ASK:
				OutLine(TEXT("set ssh strict-host-key-check ask"));
				break;
			case SHKC_OFF:
				OutLine(TEXT("set ssh strict-host-key-check off"));
				break;
			default:
				OutLine(TEXT("end 1 unknown set strict-host-key-check"));
			}

			if (!sshIdentityFile().isNullOrWhiteSpace()) {
				_sntprintf(buf, BUFFERSIZE, TEXT("set ssh identity-file {%s}"), 
					sshIdentityFile().data());
				OutLine(buf);
			}

			if (!sshUserKnownHostsFile().isNullOrWhiteSpace()) {
				_sntprintf(buf, BUFFERSIZE, TEXT("set ssh v2 user-known-hosts-file {%s}"), 
					sshUserKnownHostsFile().data());
				OutLine(buf);
			}

			/*
			 * TODO: Selection of auth methods and crypto algorithms
			 */

			if (hostname().contains(L':') ||
				hostname().contains(L' ')) {

				_sntprintf(buf, BUFFERSIZE, TEXT("set host %s"), 
					hostname().data());
			} else if (port() > 0) {
				_sntprintf(buf, BUFFERSIZE, TEXT("set host %s %d"), 
					hostname().data(), port());
			} else {
				_sntprintf(buf, BUFFERSIZE, TEXT("set host %s ssh"),
					hostname().data());
			}
			OutLine(buf);
		}
		break;
	case CT_FTP:
		{
			/* We don't currently support network directories.
			 * If we did, we'd let the user specify a list of files and then
			 * hand that list, space-delimited, to C-Kermit with:
			 *		set network directory %s
			 * eg: set network directory file1.txt file2.txt file3.txt etc
			 */
			OutLine(TEXT("set network directory")); // No network directories

			if (ftpAutoLogin()) {
				OutLine(TEXT("set ftp autologin on"));
			} else {
				OutLine(TEXT("set ftp autologin off"));
			}

			if (ftpPassiveMode()) {
				OutLine(TEXT("set ftp passive on"));
			} else {
				OutLine(TEXT("set ftp passive off"));
			}

			if (ftpAutoAuthentication()) {
				OutLine(TEXT("set ftp autoauth on"));
			} else {
				OutLine(TEXT("set ftp autoauth off"));
			}

			if (ftpAutoEncryption()) {
				OutLine(TEXT("set ftp autoenc on"));
			} else {
				OutLine(TEXT("set ftp autoenc off"));
			}

			if (ftpCredentialForwarding()) {
				OutLine(TEXT("set ftp credential-forwarding on"));
			} else {
				OutLine(TEXT("set ftp credential-forwarding off"));
			}

			if (ftpDebugMessages()) {
				OutLine(TEXT("set ftp debug on"));
			} else {
				OutLine(TEXT("set ftp debug off"));
			}

			if (ftpDates()) {
				OutLine(TEXT("set ftp dates on"));
			} else {
				OutLine(TEXT("set ftp dates off"));
			}

			if (ftpSendLiteralFilenames()) {
				OutLine(TEXT("set ftp filenames literal"));
			} else {
				OutLine(TEXT("set ftp filenames automatic"));
			}

			if (ftpVerboseMessages()) {
				OutLine(TEXT("set ftp verbose on"));
			} else {
				OutLine(TEXT("set ftp verbose off"));
			}

			if (ftpCharacterSetTranslation()) {
				OutLine(TEXT("set ftp character-set-translation on"));
			} else {
				OutLine(TEXT("set ftp character-set-translation off"));
			}

			_sntprintf(buf, BUFFERSIZE, TEXT("set ftp server-character-set %s"), 
				ftpServerCharset().data());
			OutLine(buf);

			// TODO: list of space separated auth types:
			// gssapi-krb5 kerberos4 srp ssl tls
			//_sntprintf(buf, BUFFERSIZE, TEXT("set ftp authtype %s%s%s%s%s"), 
			//		ftpAuthType());
			//OutLine(buf);

			if (hostname().contains(L':') ||
				hostname().contains(L' ')) {

				_sntprintf(buf, BUFFERSIZE, TEXT("ftp open %s%s%s%s%s"), 
					hostname().data(), 
					isAnonymous ? TEXT(" /anonymous") :
						!userId().isNullOrWhiteSpace() ? TEXT(" /user:") : TEXT(""),
					(!isAnonymous && !userId().isNullOrWhiteSpace()) ? userId().data() : TEXT(""),
					(isAnonymous && !password().isNullOrWhiteSpace()) ? TEXT(" /password:") : TEXT(""),
					(isAnonymous && !password().isNullOrWhiteSpace()) ? password().data() : TEXT(""));
				;
			} else if (port() > 0) {
				_sntprintf(buf, BUFFERSIZE, TEXT("ftp open %s %d%s%s%s%s"), 
					hostname().data(), port(),
					isAnonymous ? TEXT(" /anonymous") :
						!userId().isNullOrWhiteSpace() ? TEXT(" /user:") : TEXT(""),
					(!isAnonymous && !userId().isNullOrWhiteSpace()) ? userId().data() : TEXT(""),
					(isAnonymous && !password().isNullOrWhiteSpace()) ? TEXT(" /password:") : TEXT(""),
					(isAnonymous && !password().isNullOrWhiteSpace()) ? password().data() : TEXT(""));
			} else {
				_sntprintf(buf, BUFFERSIZE, TEXT("ftp open %s ftp%s%s%s%s"), 
					hostname().data(),
					isAnonymous ? TEXT(" /anonymous") :
						!userId().isNullOrWhiteSpace() ? TEXT(" /user:") : TEXT(""),
					(!isAnonymous && !userId().isNullOrWhiteSpace()) ? userId().data() : TEXT(""),
					(isAnonymous && !password().isNullOrWhiteSpace()) ? TEXT(" /password:") : TEXT(""),
					(isAnonymous && !password().isNullOrWhiteSpace()) ? password().data() : TEXT(""));
			}
			OutLine(buf);
			
			OutLine(TEXT("if success {"));

			switch(ftpCommandProtectionLevel()) {
			case FTPPL_CLEAR:
				OutLine(TEXT("  set ftp command-protection-level clear"));
				break;
			case FTPPL_CONFIDENTIAL:
				OutLine(TEXT("  set ftp command-protection-level confidential"));
				break;
			case FTPPL_SAFE:
				OutLine(TEXT("  set ftp command-protection-level safe"));
				break;
			case FTPPL_PRIVATE:
				OutLine(TEXT("  set ftp command-protection-level private"));
				break;
			}

			switch(ftpDataProtectionLevel()) {
			case FTPPL_CLEAR:
				OutLine(TEXT("  set ftp data-protection-level clear"));
				break;
			case FTPPL_CONFIDENTIAL:
				OutLine(TEXT("  set ftp data-protection-level confidential"));
				break;
			case FTPPL_SAFE:
				OutLine(TEXT("  set ftp data-protection-level safe"));
				break;
			case FTPPL_PRIVATE:
				OutLine(TEXT("  set ftp data-protection-level private"));
				break;
			}

			OutLine(TEXT("}"));

		}
		break;
	case CT_LAT:
		{
			/* We don't currently support network directories.
			 * If we did, we'd let the user specify a list of files and then
			 * hand that list, space-delimited, to C-Kermit with:
			 *		set network directory %s
			 * eg: set network directory file1.txt file2.txt file3.txt etc
			 */
			OutLine(TEXT("set network directory")); // No network directories

			OutLine(TEXT("set network type superlat"));
			OutLine(TEXT("if fail set network type pathworks lat"));
			OutLine(TEXT("if fail end 1 Both SUPERLAT and PATHWORKS LAT Failed"));

			_sntprintf(buf, BUFFERSIZE, TEXT("set host %s"), 
				latService().data());
			OutLine(buf);
		}
		break;
	case CT_CTERM:
		{
			/* We don't currently support network directories.
			 * If we did, we'd let the user specify a list of files and then
			 * hand that list, space-delimited, to C-Kermit with:
			 *		set network directory %s
			 * eg: set network directory file1.txt file2.txt file3.txt etc
			 */
			OutLine(TEXT("set network directory")); // No network directories

			OutLine(TEXT("set network type pathworks cterm"));
			OutLine(TEXT("if fail end 1 PATHWORKS CTERM Failed"));

			_sntprintf(buf, BUFFERSIZE, TEXT("set host %s"), 
				latService().data());
			OutLine(buf);
		}
		break;
	case CT_NAMED_PIPE:
		{
			_sntprintf(buf, BUFFERSIZE, TEXT("set network type named-pipe \\\\%s\\%s"), 
				namedPipeHost().data(), namedPipeName().data());
			OutLine(buf);

			OutLine(TEXT("set host"));
		}
	case CT_PTY:
		{
			OutLine(TEXT("set network type pty"));

			_sntprintf(buf, BUFFERSIZE, TEXT("set host %s"), 
				ptyCommand().data());
			OutLine(buf);
		}
	case CT_PIPE:
		{
			OutLine(TEXT("set network type pipe"));

			_sntprintf(buf, BUFFERSIZE, TEXT("set host %s"), 
				ptyCommand().data());
			OutLine(buf);
		}
	case CT_DLL:
		{
			_sntprintf(buf, BUFFERSIZE, TEXT("set network type dll %s"), 
				dllName().data());
			OutLine(buf);

			_sntprintf(buf, BUFFERSIZE, TEXT("set host %s"), 
				dllParameters().data());
			OutLine(buf);
		}
		break;
	}

	// Make the connection!

	// If we have a login script file...
	if (runLoginScriptFile() && !loginScriptFile().isNullOrWhiteSpace()) {
		switch(connectionType()) {
		case CT_IP:
		case CT_SSH:
		case CT_LAT:
		case CT_CTERM:
		case CT_FTP:
		case CT_PTY:			// Not sure about this
		case CT_PIPE:			// or this
		case CT_NAMED_PIPE:		// Do login scripts work with any of these?
		case CT_DLL:			// should probably test someday.
		case CT_SERIAL:
			{
				OutLine(TEXT("xif success {"));
				
				_sntprintf(buf, BUFFERSIZE, TEXT("  take {%s}"), 
					loginScriptFile().data());
				OutLine(buf);

				OutLine(TEXT("  if success connect"));

				OutLine(TEXT("}"));
			}
			break;
		case CT_MODEM:
			{
				/* We don't support dialed connections at the moment and
				 * likely never will. But if we did, we'd do something like:
				 *
				 *	dial %s
				 *	if fail end 1 DIAL Failed
				 *  take {%s}
				 *  if success connect
				 */

			}
			break;
		}
	} else if (!loginScript().isNullOrWhiteSpace() && connectionType() != CT_FTP) {
		switch(connectionType()) {
		case CT_IP:
		case CT_SSH:
		case CT_LAT:
		case CT_CTERM:
		//case CT_FTP:
		case CT_PTY:			// Not sure about this
		case CT_PIPE:			// or this
		case CT_NAMED_PIPE:		// Do login scripts work with any of these?
		case CT_DLL:			// should probably test someday.
		case CT_SERIAL:
			{
				OutLine(TEXT("xif success {"));
				
				OutLine(loginScript().toCRLF().data());

				OutLine(TEXT("  if success connect"));

				OutLine(TEXT("}"));
			}
			break;
		case CT_MODEM:
			{
				/* We don't support dialed connections at the moment and
				 * likely never will. But if we did, we'd do something like:
				 *
				 *	dial %s
				 *	if fail end 1 DIAL Failed
				 *  (dump contents of login script, CR-LF'd)
				 *  if success connect
				 */

			}
			break;
		}
	} else {
		// No login script - just connect.
		switch(connectionType()) {
		case CT_IP:
		case CT_SSH:
		case CT_LAT:
		case CT_CTERM:
		case CT_PTY:			// Not sure about this
		case CT_PIPE:			// or this
		case CT_NAMED_PIPE:		// Do login scripts work with any of these?
		case CT_DLL:			// should probably test someday.
		case CT_SERIAL:
			{
				OutLine(TEXT("if success connect"));
			}
			break;
		case CT_MODEM:
			{
				/* We don't support dialed connections at the moment and
				 * likely never will. But if we did, we'd do something like:
				 *
				 *	dial %s
				 *  if success connect
				 */
			}
			break;
		case CT_FTP:
			break; // Nothing to do for FTP
		}
	}

	CloseHandle(hFile);

	return TRUE;
}