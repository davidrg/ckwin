#ifndef JSON_PROFILE_H
#define JSON_PROFILE_H

#include "json_config.h"

// TODO: make all the getters const

struct jpStringCache;

class JsonProfile : public ConnectionProfile {
	friend class JsonConfigFile;

public:
	~JsonProfile();
	//TODO: JsonProfile &operator=(const JsonProfile &rhs);

	virtual BOOL commitChanges();

	virtual int id() const;

	virtual ConnectionProfile* nextProfile() const;

	// ----- General -----
	virtual CMString name();
	virtual void setName(CMString name);

	virtual CMString description();
	virtual void setDescription(CMString description);

	virtual CMString notes();
	virtual void setNotes(CMString notes);

	virtual CMString downloadDirectory();
	virtual void setDownloadDirectory(CMString dir);

	virtual CMString startingDirectory();
	virtual void setStartingDirectory(CMString dir);

	// ----- Connection -----
	virtual ConnectionType connectionType();

	// If CT_SSH, CT_FTP, CT_IP
	virtual CMString hostname();  // Or IP
	virtual void setHostname(CMString hostname);	

	// If CT_SSH, CT_FTP, CT_IP
	virtual int port();
	virtual void setPort(int p);

	// If CT_IP
	virtual IPProtocol ipProtocol();
	virtual void setIpProtocol(IPProtocol p);

	// If CT_LAT, CT_CTERM
	virtual CMString latService();
	virtual void setLatService(CMString svc);

	// If CT_NAMED_PIPE
	virtual CMString namedPipeName();
	virtual void setNamedPipeName(CMString name);

	// If CT_NAMED_PIPE
	virtual CMString namedPipeHost();
	virtual void setNamedPipeHost(CMString host);

	// if CT_PTY, CT_PIPE
	virtual CMString ptyCommand();
	virtual void setPtyCommand(CMString command);

	// if CT_DLL
	virtual CMString dllName();
	virtual void setDllName(CMString name);

	// if CT_DLL
	virtual CMString dllParameters();
	virtual void setDllParameters(CMString params);

	virtual BOOL exitOnDisconnect();
	virtual void setExitOnDisconnect(BOOL exit);

	// ----- Terminal -----
	virtual Term::TermType terminalType();
	virtual void setTerminalType(Term::TermType type);

	virtual BOOL is8Bit(); // Rather than 7-bit
	virtual void setIs8Bit(BOOL is8bit);

	virtual BOOL apcEnabled();
	virtual void setApcEnabled(BOOL enabled);

	virtual BOOL localEchoEnabled();
	virtual void setLocalEchoEnabled(BOOL enabled);

	virtual BOOL autoWrapEnabled();
	virtual void setAutoWrapEnabled(BOOL enabled);

	virtual BOOL statusLineEnabled();
	virtual void setStatusLineEnabled(BOOL enabled);

	virtual Charset::Charset characterSet();
	virtual void setCharacterSet(Charset::Charset cset);

	virtual int screenWidth();
	virtual void setScreenWidth(int w);

	virtual int screenHeight();
	virtual void setScreenHeight(int h);

	virtual int scrollbackLines();
	virtual void setScrollbackLines(int lines);

	virtual Cursor cursor();
	virtual void setCursor(Cursor cur);

	// ----- Terminal Colors -----
	virtual Term::Color terminalForegroundColor();
	virtual void setTerminalForegroundColor(Term::Color c);

	virtual Term::Color terminalBackgroundColor();
	virtual void setTerminalBackgroundColor(Term::Color c);

	virtual Term::Color commandForegroundColor();
	virtual void setCommandForegroundColor(Term::Color c);

	virtual Term::Color commandBackgroundColor();
	virtual void setCommandBackgroundColor(Term::Color c);

	virtual Term::Color statusLineForegroundColor();
	virtual void setStatusLineForegroundColor(Term::Color c);

	virtual Term::Color statusLineBackgroundColor();
	virtual void setStatusLineBackgroundColor(Term::Color c);

	virtual Term::Color mouseSelectionForegroundColor();
	virtual void setMouseSelectionForegroundColor(Term::Color c);

	virtual Term::Color mouseSelectionBackgroundColor();
	virtual void setMouseSelectionBackgroundColor(Term::Color c);

	virtual Term::Color popupHelpForegroundColor();
	virtual void setPopupHelpForegroundColor(Term::Color c);

	virtual Term::Color popupHelpBackgroundColor();
	virtual void setPopupHelpBackgroundColor(Term::Color c);

	virtual Term::Color underlineSimulationForegroundColor();
	virtual void setUnderlineSimulationForegroundColor(Term::Color c);

	virtual Term::Color underlineSimulationBackgroundColor();
	virtual void setUnderlineSimulationBackgroundColor(Term::Color c);

	// ----- File Transfer -----
	virtual FileTransferProtocol fileTransferProtocol();
	virtual void setFileTransferProtocol(FileTransferProtocol proto);

	virtual KermitPerformance kermitPerformance();
	virtual void setKermitPerformance(KermitPerformance kp);

	virtual int packetLength();
	virtual void setPacketLength(int len);

	virtual int windowSize();
	virtual void setWindowSize(int size);

	virtual ControlCharUnprefixing controlCharUnprefixing();
	virtual void setControlCharUnprefixing(ControlCharUnprefixing ccu);

	virtual BOOL defaultToBinaryMode();
	virtual void setDefaultToBinaryMode(BOOL enabled);

	virtual CMString fileCharacterSet();
	virtual void setFileCharacterSet(CMString cset);

	virtual CMString transferCharacterSet();
	virtual void setTransferCharacterSet(CMString cset);

	virtual FileNameCollision fileNameCollisionAction();
	virtual void setFileNameCollisionAction(FileNameCollision fnc);

	virtual AutoDownload autoDownloadMode();
	virtual void setAutoDownloadMode(AutoDownload ad);

	virtual BOOL transmitLiteralFilenames();
	virtual void setTransmitLiteralFilenames(BOOL enabled);

	virtual BOOL usePathnames();
	virtual void setUsePathnames(BOOL enabled);
	
	virtual BOOL keepIncompleteFiles();
	virtual void setKeepIncomingFiles(BOOL enabled);

	virtual BOOL negotiateStreamingTransferMode();
	virtual void setNegotiateStreamingTransferMode(BOOL enabled);

	virtual BOOL negotiateClearChannelTransferMode();
	virtual void setNegotiateClearChannelTransferMode(BOOL enabled);

	virtual BOOL force16bitCRC();
	virtual void setForce16bitCRC(BOOL enabled);

	// ----- Serial -----
	virtual CMString line();
	virtual void setLine(CMString line);

	virtual unsigned int lineSpeed();
	virtual void setLineSpeed(int speed);

	virtual FlowControl flowControl();
	virtual void setFlowControl(FlowControl fc);

	virtual Parity parity();
	virtual void setParity(Parity p);

	virtual StopBits stopBits();
	virtual void setStopBits(StopBits sb);

	virtual BOOL errorCorrection();
	virtual void setErrorCorrection(BOOL enabled);

	virtual BOOL dataCompression();
	virtual void setDataCompression(BOOL enabled);

	virtual BOOL carrierDetection();
	virtual void setCarrierDetection(BOOL enabled);

	
	// ----- Telnet -----
	virtual NegotiateOption telnetAuthenticationOption();
	virtual void setTelnetAuthenticationOption(NegotiateOption opt);

	virtual NegotiateOption telnetBinaryModeOption();
	virtual void setTelnetBinaryModeOption(NegotiateOption opt);

	virtual NegotiateOption telnetEncryptionOption();
	virtual void setTelnetEncryptionOption(NegotiateOption opt);

	virtual NegotiateOption telnetKermitOption();
	virtual void setTelnetKermitOption(NegotiateOption opt);

	virtual NegotiateOption telnetStartTLSOption();
	virtual void setTelnetStartTLSOption(NegotiateOption opt);

	virtual NegotiateOption telnetForwardXOption();
	virtual void setTelnetForwardXOption(NegotiateOption opt);

	virtual NegotiateOption telnetComPortControlOption();
	virtual void setTelnetComPortControlOption(NegotiateOption opt);

	virtual CMString telnetEnvarAccount();
	virtual void setTelnetEnvarAccount(CMString val);

	virtual CMString telnetEnvarDisplay();
	virtual void setTelnetEnvarDisplay(CMString val);

	virtual CMString telnetEnvarJob();
	virtual void setTelnetEnvarJob(CMString val);

	virtual CMString telnetEnvarLocation();
	virtual void setTelnetEnvarLocation(CMString val);

	virtual CMString telnetEnvarPrinter();
	virtual void setTelnetEnvarPrinter(CMString val);

	virtual CMString telnetEnvarTerminalType();
	virtual void setTelnetEnvarTerminalType(CMString val);

	virtual BOOL telnetForwardCredentials();
	virtual void setTelnetForwardCredentials(BOOL enabled);

	virtual BOOL telnetNegotiationsMustComplete();
	virtual void setTelnetNegotiationsMustComplete(BOOL enabled);

	virtual BOOL telnetDebugMode();
	virtual void setTelnetDebugMode(BOOL enabled);

	virtual BOOL telnetSubnegotiationDelay();
	virtual void setTelnetSubnegotiationDelay(BOOL enabled);

	// ----- SSH -----
	virtual BOOL sshCompressionEnabled();		// off
	virtual void setSshCompressionEnabled(BOOL enabled);

	virtual BOOL sshX11ForwardingEnabled();		// off
	virtual void setSshX11ForwardingEnabled(BOOL enabled);

	virtual SshHostKeyCheck sshHostKeyChecking();	// ask
	virtual void setSshHostKeyChecking(SshHostKeyCheck shkc);

	virtual CMString sshIdentityFile();			// NULL
	virtual void setSshIdentityFile(CMString file);

	virtual CMString sshUserKnownHostsFile();		// NULL
	virtual void setSshUserKnownHostsFile(CMString file);

	// set ssh v2 authentication [gssapi, publickey, password, keyboard-interactive]
	// set ssh v2 ciphers
	// set ssh v2 macs
	// set ssh v2 hostkey-algorithms
	// set ssh v2 key-exchange-methods
	// set ssh gssapi delegate-credentials {on,off}


	// ----- FTP -----
	/*
	Use bit fields for ftpAuthType:
struct Date
{
    unsigned nWeekDay  : 3;    // 0..7   (3 bits)
    unsigned nMonthDay : 6;    // 0..31  (6 bits)
    unsigned nMonth    : 5;    // 0..12  (5 bits)
    unsigned nYear     : 8;    // 0..100 (8 bits)
};
  

  */
	virtual CMString ftpAuthType();	// TODO: this needs to be bit flags?
	virtual void setFtpAuthType(CMString type);

	virtual FTPProtectionLevel ftpCommandProtectionLevel();
	virtual void setFtpCommandProtectionLevel(FTPProtectionLevel pl);

	virtual FTPProtectionLevel ftpDataProtectionLevel();
	virtual void setFtpDataProtectionLevel(FTPProtectionLevel pl);

	virtual CMString ftpServerCharset();
	virtual void setFtpServerCharset(CMString cset);

	virtual BOOL ftpAutoLogin();
	virtual void setFtpAutoLogin(BOOL enabled);

	virtual BOOL ftpAutoAuthentication();
	virtual void setFtpAutoAuthentication(BOOL enabled);

	virtual BOOL ftpAutoEncryption();
	virtual void setFtpAutoEncryption(BOOL enabled);

	virtual BOOL ftpPassiveMode();
	virtual void setFtpPassiveMode(BOOL enabled);

	virtual BOOL ftpCredentialForwarding();
	virtual void setFtpCredentialForwarding(BOOL enabled);

	virtual BOOL ftpDates();
	virtual void setFtpDates(BOOL enabled);

	virtual BOOL ftpSendLiteralFilenames();
	virtual void setFtpSendLiteralFilenames(BOOL enabled);

	virtual BOOL ftpVerboseMessages();
	virtual void setFtpVerboseMessages(BOOL enabled);

	virtual BOOL ftpDebugMessages();
	virtual void setFtpDebugMessages(BOOL enabled);

	virtual BOOL ftpCharacterSetTranslation();
	virtual void setFtpCharacterSetTranslation(BOOL enabled);


	// ----- TCP/IP -----
	virtual int tcpSendBufferSize();
	virtual void setTcpSendBufferSize(int size);

	virtual int tcpReceiveBufferSize();
	virtual void setTcpReceiveBufferSize(int size);

	virtual ReverseDNSOption tcpReverseDNSLookup();
	virtual void setTcpReverseDNSLookup(ReverseDNSOption opt);

	virtual BOOL tcpDNSServiceLookup();
	virtual void setTcpDNSServiceLookup(BOOL enabled);

	virtual CMString tcpSocksHostname();
	virtual void setTcpSocksHostname(CMString host);

	virtual int tcpSocksPort();	// -1 for unspecified
	virtual void setTcpSocksPort(int port);

	virtual CMString tcpSocksUser();
	virtual void setTcpSocksUser(CMString user);

	virtual CMString tcpSocksPassword();
	virtual void setTcpSocksPassword(CMString pass);

	virtual CMString tcpHttpHostname();
	virtual void setTcpHttpHostname(CMString host);

	virtual int tcpHttpPort();	// -1 for unspecified
	virtual void setTcpHttpPort(int port);

	virtual CMString tcpHttpUser();
	virtual void setTcpHttpUser(CMString user);

	virtual CMString tcpHttpPassword();
	virtual void setTcpHttpPassword(CMString pass);


	// ----- Kerberos -----
	// Someday

	// ----- TLS -----
	virtual CMString tlsCipherList();
	virtual void setTlsCipherList(CMString file);

	virtual CMString tlsClientCertificateFile();
	virtual void setTlsClientCertificateFile(CMString file);

	virtual CMString tlsClientPrivateKeyFile();
	virtual void setTlsClientPrivateKeyFile(CMString file);

	virtual CMString tlsCAVerificationFile();
	virtual void setTlsCAVerificationFile(CMString file);

	virtual CMString tlsCAVerificationDirectory();
	virtual void setTlsCAVerificationDirectory(CMString dir);

	virtual CMString tlsCRLFile();
	virtual void setTlsCRLFile(CMString file);

	virtual CMString tlsCRLDirectory();
	virtual void setTlsCRLDirectory(CMString dir);

	virtual CertificateVerifyMode tlsCertificateVerifyMode();
	virtual void setTlsCertificateVerifyMode(CertificateVerifyMode mode);

	virtual BOOL tlsVerboseMode();
	virtual void setTlsVerboseMode(BOOL enabled);

	virtual BOOL tlsDebugMode();
	virtual void setTlsDebugMode(BOOL enabled);


	// ----- Keyboard -----
	virtual KeyboardBackspaceMode keyboardBackspaceSends();
	virtual void setKeyboardBackspaceSends(KeyboardBackspaceMode kbm);

	virtual KeyboardEnterMode keyboardEnterSends();
	virtual void setKeyboardEnterSends(KeyboardEnterMode kem);

	virtual BOOL mouseEnabled();	// Add a mouse tab?
	virtual void setMouseEnabled(BOOL enabled);

	virtual BOOL defaultKeyMap();
	virtual void setDefaultKeyMap(BOOL enabled);

	virtual CMString keymapFile();
	virtual void setKeymapFile(CMString file);

	virtual CMString additionalKeyMaps();
	virtual void setAdditionalKeyMaps(CMString str);

	// ----- Login -----
	virtual CMString userId();
	virtual void setUserId(CMString uid);

	virtual CMString password();
	virtual void setPassword(CMString pass);

	virtual BOOL promptForPassword();
	virtual void setPromptForPassword(BOOL prompt);

	virtual CMString passwordPrompt();
	virtual void setPasswordPrompt(CMString prompt);

	virtual BOOL runLoginScriptFile();
	virtual void setRunLoginScriptFile(BOOL run);

	virtual CMString loginScriptFile();
	virtual void setLoginScriptFile(CMString file);

	virtual CMString loginScript();
	virtual void setLoginScript(CMString script);


	// ----- Printer -----
	virtual PrinterType printerType();
	virtual void setPrinterType(PrinterType pt);

	// aka aka file aka command
	virtual CMString deviceName();
	virtual void setDeviceName(CMString device);

	virtual CMString windowsPrintQueue();	// use "<default>" for the default
	virtual void setWindowsPrintQueue(CMString queue);

	virtual CMString headerFile();
	virtual void setHeaderFile(CMString file);

	virtual BOOL sendEndOfJobString();
	virtual void setSendEndOfJobString(BOOL enabled);

	virtual CMString endOfJobString();	// SHOULD BE CHECKED BY DEFAULT
	virtual void setEndOfJobString(CMString string);

	virtual CMString printCharacterSet();
	virtual void setPrintCharacterSet(CMString cset);

	virtual BOOL printAsPostScript();
	virtual void setPrintAsPostScript(BOOL enabled);

	virtual int printWidth();
	virtual void setPrintWidth(int w);

	virtual int printHeight();
	virtual void setPrintHeight(int h);

	virtual int printSpeed();
	virtual void setPrintSpeed(int speed);

	virtual FlowControl printFlowControl();
	virtual void setPrintFlowControl(FlowControl fc);

	virtual Parity printParity();
	virtual void setPrintParity(Parity p);

	virtual BOOL bidirectionalPrinting();
	virtual void setBidirectionalPrinting(BOOL enabled);

	virtual int printTimeoutSeconds();	// TODO: missing from the UI!
	virtual void setPrintTimeoutSeconds(int seconds);

	// ----- GUI -----
	virtual BOOL useGUIKermit();
	virtual void setUseGUIKermit(BOOL enabled);

	virtual CMString fontName();
	virtual void setFontName(CMString font);

	virtual int fontSize();
	virtual void setFontSize(int size);

	virtual BOOL customStartupPosition();
	virtual void setCustomStartupPosition(BOOL enabled);

	virtual int startupAtX();
	virtual void setStartupAtX(int x);

	virtual int startupAtY();
	virtual void setStartupAtY(int y);

	virtual BOOL resizeChangesDimensions();
	virtual void setResizeChangesDimensions(BOOL enabled);

	virtual WindowStartupMode windowStartupMode();
	virtual void setWindowStartupMode(WindowStartupMode wsm);

	virtual BOOL menubarEnabled();
	virtual void setMenubarEnabled(BOOL enabled);

	virtual BOOL toolbarEnabled();
	virtual void setToolbarEnabled(BOOL enabled);

	virtual BOOL statusbarEnabled();
	virtual void setStatusbarEnabled(BOOL enabled);

	virtual BOOL dialogsEnabled();
	virtual void setDialogsEnabled(BOOL enabled);

	// ----- GUI Colors -----
	virtual RGB8BitColor rgbBlack();
	virtual void setRgbBlack(RGB8BitColor c);

	virtual RGB8BitColor rgbBlue();
	virtual void setRgbBlue(RGB8BitColor c);

	virtual RGB8BitColor rgbGreen();
	virtual void setRgbGreen(RGB8BitColor c);

	virtual RGB8BitColor rgbCyan();
	virtual void setRgbCyan(RGB8BitColor c);

	virtual RGB8BitColor rgbRed();
	virtual void setRgbRed(RGB8BitColor c);

	virtual RGB8BitColor rgbMagenta();
	virtual void setRgbMagenta(RGB8BitColor c);

	virtual RGB8BitColor rgbBrown();
	virtual void setRgbBrown(RGB8BitColor c);

	virtual RGB8BitColor rgbLightGray();
	virtual void setRgbLightGray(RGB8BitColor c);

	virtual RGB8BitColor rgbDarkGray();
	virtual void setRgbDarkGray(RGB8BitColor c);

	virtual RGB8BitColor rgbLightBlue();
	virtual void setRgbLightBlue(RGB8BitColor c);

	virtual RGB8BitColor rgbLightGreen();
	virtual void setRgbLightGreen(RGB8BitColor c);

	virtual RGB8BitColor rgbLightCyan();
	virtual void setRgbLightCyan(RGB8BitColor c);

	virtual RGB8BitColor rgbLightRed();
	virtual void setRgbLightRed(RGB8BitColor c);

	virtual RGB8BitColor rgbLightMagenta();
	virtual void setRgbLightMagenta(RGB8BitColor c);

	virtual RGB8BitColor rgbYellow();
	virtual void setRgbYellow(RGB8BitColor c);

	virtual RGB8BitColor rgbWhite();
	virtual void setRgbWhite(RGB8BitColor c);

	// ----- Logging -----
	virtual BOOL logConnections();
	virtual void setLogConnections(BOOL enabled);

	virtual BOOL logAppendConnections();
	virtual void setLogAppendConnections(BOOL enabled);

	virtual CMString logConnectionsFile();
	virtual void setLogConnectionsFile(CMString file);

	virtual BOOL logDebug();
	virtual void setLogDebug(BOOL enabled);

	virtual BOOL logAppendDebug();
	virtual void setLogAppendDebug(BOOL enabled);

	virtual CMString logDebugFile();
	virtual void setLogDebugFile(CMString file);

	virtual BOOL logKermitFileTransfers();
	virtual void setLogKermitFileTransfers(BOOL enabled);

	virtual BOOL logAppendKermitFileTransfers();
	virtual void setLogAppendKermitFileTransfers(BOOL enabled);

	virtual CMString logKermitFileTransfersFile();
	virtual void setLogKermitFileTransfersFile(CMString file);
	
	virtual BOOL logSessionInput();
	virtual void setLogSessionInput(BOOL enabled);

	virtual BOOL logAppendSessionInput();
	virtual void setLogAppendSessionInput(BOOL enabled);

	virtual CMString logSessionInputFile();
	virtual void setLogSessionInputFile(CMString file);

	virtual LogSessionMode logSessionInputMode();
	virtual void setLogSessionInputMode(LogSessionMode mode);

	virtual BOOL logFileTransferTransactions();
	virtual void setLogFileTransferTransactions(BOOL enabled);

	virtual BOOL logAppendFileTransferTransactions();
	virtual void setLogAppendFileTransferTransactions(BOOL enabled);

	virtual BOOL logBriefFileTransferTransactions();
	virtual void setLogBriefFileTransferTransactions(BOOL enabled);

	virtual CMString logFileTransferTransactionsFile();
	virtual void setLogFileTransferTransactionsFile(CMString file);

protected:
	JsonProfile(JsonConfigFile *configFile, cJSON *json);	

	virtual void setId(int id);
	virtual void setConnectionType(ConnectionType conType);

	cJSON *_json;
	JsonConfigFile *_config;

	/* These are a cache of every string returned by this
	 * class. CMString reference-counts the data it holds
	 * and automatically deletes it when the reference count
	 * hits zero meaning you can't do something like this:
	 *		LPTSTR name = profile->name().data();
	 *		setWindowText(hwndName, name);
	 * 
	 * profile->name() constructs a new CMString and returns
	 * it. The data() method returns its internal LPTSTRING
	 * which is assigned to LPTSTR name;
	 *
	 * The string returned by profile->name() then goes out
	 * of scope causing the string now pointed to by 
	 * LPTSTR name to be deleted. setWindowText then receives
	 * a dangling pointer.
	 *
	 * A workaround is to do this instead:
	 *		CMString strName = profile->name();
	 *		LPTSTR name = strName.data();
	 *      SetWindowText(name);
	 *
	 * But this is pretty inconvenient and results in a lot
	 * of temporary variables on the stack.
	 *
	 * This is why we cache the strings here. As long as we
	 * hold on to the string, the reference count will not
	 * drop below 0 and LPTSTR returned by data() will remain
	 * valid until a setter is called.
	 *
	 * These caches are maintained with a collection if fairly
	 * gross macros defined over in json_profile.cpp
	 */
	CMString _cached_name;
	CMString _cached_desc;
	CMString _cached_note;
	CMString _cached_dirs_download;
	CMString _cached_dirs_start;
	CMString _cached_ip_host;
	CMString _cached_lat_service;
	CMString _cached_named_pipe_name;
	CMString _cached_named_pipe_host;
	CMString _cached_pty_command;
	CMString _cached_dll_name;
	CMString _cached_dll_params;
	CMString _cached_xfer_file_cset;
	CMString _cached_xfer_cset;
	CMString _cached_serial_line;
	CMString _cached_telnet_envar_account;
	CMString _cached_telnet_envar_display;
	CMString _cached_telnet_envar_job;
	CMString _cached_telnet_envar_location;
	CMString _cached_telnet_envar_printer;
	CMString _cached_telnet_envar_terminal;
	CMString _cached_ssh_ident_file;
	CMString _cached_ssh_known_hosts_file;
	CMString _cached_ftp_charset;
	CMString _cached_ip_socks_host;
	CMString _cached_ip_socks_user;
	CMString _cached_ip_socks_pass;
	CMString _cached_ip_http_host;
	CMString _cached_ip_http_user;
	CMString _cached_ip_http_pass;
	CMString _cached_tls_cipher_list;
	CMString _cached_tls_cert_file;
	CMString _cached_tls_pk_file;
	CMString _cached_tls_ca_verification_file;
	CMString _cached_tls_ca_verification_dir;
	CMString _cached_tls_crl_file;
	CMString _cached_tls_crl_dir;
	CMString _cached_keyboard_map_file;
	CMString _cached_keyboard_additional_keymaps;
	CMString _cached_login_user;
	CMString _cached_login_pass;
	CMString _cached_login_pass_prompt;
	CMString _cached_login_script_file;
	CMString _cached_login_script;
	CMString _cached_printer_device_name;
	CMString _cached_printer_queue;
	CMString _cached_printer_header_file;
	CMString _cached_printer_end_string;
	CMString _cached_printer_cset;
	CMString _cached_gui_font_name;
	CMString _cached_log_connections_file;
	CMString _cached_log_debug_file;
	CMString _cached_log_transfers_file;
	CMString _cached_log_session_file;
	CMString _cached_log_xfer_act_file;


	// ----------------------------
	// Some handy utility functions to getting values in/out
	// out of the JSON document easier
	
	// Get the value named <name> from JSON object <json>
	// returning <defaultValue> if it doesn't exist or
	// exists with the wrong type
	CMString getString(cJSON *json, LPCSTR name, CMString defaultValue=CMString()) const;
	int	getInteger(cJSON *json, LPCSTR name, int defaultValue) const;
	BOOL getBool(cJSON *json, LPCSTR name, BOOL defaultValue=FALSE) const;

	// Set the value named <name> on JSON object <json>
	// creating it if it doesn't exist or exists but has
	// the wrong type.
	void setString(cJSON *json, LPCSTR name, CMString value);
	void setInteger(cJSON *json, LPCSTR name, int value);
	void setBool(cJSON *json, LPCSTR name, BOOL value);
	void setNull(cJSON *json, LPCSTR name);


	// Get the value named <name> from _json returning
	// <defaultValue> if it doesn't exist or exists but
	// has the wrong type
	CMString getString(LPCSTR name, CMString defaultValue=CMString()) const;
	int getInteger(LPCSTR name, int defaultValue) const;
	BOOL getBool(LPCSTR name, BOOL defaultValue=FALSE) const;
	
	// Set the value named <name> in _json creating it if
	// it doesn't exist or exists with the wrong type.
	void setString(LPCSTR name, CMString value);
	void setInteger(LPCSTR name, int value);
	void setBool(LPCSTR name, BOOL value);

	// Get the value named <name> from the object named 
	// <parentName> in _json, returning <defaultValue>
	// if it doesn't exist
	CMString getString(LPCSTR parentName, LPCSTR name, CMString defaultValue=CMString()) const;
	int getInteger(LPCSTR parentName, LPCSTR name, int defaultValue) const;
	BOOL getBool(LPCSTR parentName, LPCSTR name, BOOL defaultValue=FALSE) const;

	// Set the value named <name> on the object named
	// <parentName> in _json, creating both the value and
	// the parent if they don't exist or exist with the
	// wrong type.
	void setString(LPCSTR parentName, LPCSTR name, CMString value);
	void setInteger(LPCSTR parentName, LPCSTR name, int value);
	void setBool(LPCSTR parentName, LPCSTR name, BOOL value);

	// Get the value named <name> from the object named 
	// <parentName> in the object named <grandParentName> in _json, 
	// returning <defaultValue> if it doesn't exist
	CMString getString(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, CMString defaultValue=CMString()) const;
	int getInteger(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, int defaultValue) const;
	BOOL getBool(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, BOOL defaultValue=FALSE) const;

	// Set the value named <name> on the object named
	// <parentName> in the object named <grandParentName> in  _json, 
	// creating both the value and the parent and the grandparent
	// if they don't exist or exist with the wrong type.
	void setString(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, CMString value);
	void setInteger(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, int value);
	void setBool(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, BOOL value);
};

#endif /* JSON_PROFILE_H */