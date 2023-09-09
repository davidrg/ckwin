#ifndef CONN_PROFILE_H
#define CONN_PROFILE_H

#include <windows.h>

#include "cmstring.h"

#define DEFAULT_WIN_PRINT_QUEUE TEXT("<default>")

typedef struct tagRGB {
	signed short r;
	signed short g;
	signed short b;
} RGB8BitColor;


class ConnectionProfile {

public:
	enum ConnectionType {
		CT_TEMPLATE = 0,	// Special for the default template (show all tabs)
		CT_SSH = 1,
		CT_FTP = 2,
		CT_SERIAL = 3,
		CT_IP = 4,
		CT_LAT = 5,
		CT_NAMED_PIPE = 6,	// set network type named-pipe [pipename]; set host /connect . (or I guess server name)
		CT_PTY = 7,			// Windows 10 v1809 or newer
		CT_PIPE = 8,		// Regular pipe
		CT_DLL = 9,			// set network type 
		CT_MODEM = 10,		// Dialed serial. We'll probably never support this.
		CT_CTERM = 11
	};

	enum IPProtocol {
		IPP_DEFAULT = 0,
		IPP_TELNET_NEGO = 1,
		IPP_RLOGIN = 2,
		IPP_RAW = 3,			// Raw TCP socket
		IPP_TELNET = 4,			// without negotiations
		IPP_IKS = 5,			// Internet Kermit Service
		IPP_EK4LOGIN = 6,		// Kerberos IV
		IPP_EK5LOGIN = 7,		// Kerberos V
		IPP_K4LOGIN = 8,		// Kerberos IV
		IPP_K5LOGIN = 9,		// Kerberos V
		IPP_SSL = 10,			// Raw SSL socket
		IPP_TELNET_SSL = 11,	// Telnet over SSL
		IPP_TELNET_TLS = 12,	// Telnet over TLS
		IPP_TLS = 13,			// Raw TLS socket
		
	};

	enum FlowControl {
		FC_AUTO = 0,
		FC_NONE = 1,
		FC_RTS_CTS = 2,
		FC_XON_XOFF = 3
	};

	enum Parity {
		PAR_NONE			= 0,
		PAR_EVEN			= 1,
		PAR_EVEN_8BIT	= 2,
		PAR_MARK			= 3,
		PAR_MARK_8BIT	= 4,
		PAR_ODD			= 5,
		PAR_ODD_8BIT		= 6,
		PAR_SPACE		= 7,
		PAR_SPACE_8BIT	= 8
	};

	enum StopBits {
		SB_1_0 = 0,
		//SB_1_5 = 1,   Not output by the K95 dialer or supported by CKWIN ?
		SB_2_0 = 3
	};

	enum Cursor {
		CUR_FULL = 0,
		CUR_HALF = 1,
		CUR_UNDERLINE = 2
	};

	// Note: number assignments are important! They match
	//       the Win32 console colour codes used in the PROCESSINFO
	//		 dwFillAttribute.
	enum Color {
		COLOR_BLACK = 0,
		COLOR_BLUE = 1,
		COLOR_GREEN = 2,
		COLOR_CYAN = 3,
		COLOR_RED = 4,
		COLOR_MAGENTA = 5,
		COLOR_BROWN = 6,
		COLOR_LIGHT_GRAY = 7,
		COLOR_DARK_GRAY = 8,
		COLOR_LIGHT_BLUE = 9,
		COLOR_LIGHT_GREEN = 10,
		COLOR_LIGHT_CYAN = 11,
		COLOR_LIGHT_RED = 12,
		COLOR_LIGHT_MAGENTA = 13,
		COLOR_YELLOW = 14,
		COLOR_WHITE = 15
	};

	enum FileTransferProtocol {
		FT_KERMIT = 0,
		FT_ZMODEM = 1,
		FT_YMODEM = 2,
		FT_YMODEM_G = 3,
		FT_XMODEM = 4
	};

	enum KermitPerformance {
		KP_FAST = 0,
		// In the K95 dialer, KP_FAST and KP_CAUTIOUS both output "do fast".
		// Don't know why - perhaps a bug? Here we'll output "do cautious"
		// otherwise we may as well just do away with the setting. This is
		// something to keep in mind for any eventual export feature in the
		// old K95 dialer.
		KP_CAUTIOUS = 1,	
		KP_ROBUST = 2,
		KP_CUSTOM = 3
	};

	enum ControlCharUnprefixing {
		CCU_NEVER = 0,
		CCU_CAUTIOUS = 1,
		CCU_WITH_WILD_ABANDON = 2
	};

	enum FileNameCollision {
		FNC_BACKUP = 0,
		FNC_UPDATE = 1,
		FNC_OVERWRITE = 2,
		FNC_APPEND = 3,
		FNC_DISCARD = 4,
		FNC_RENAME = 5
	};

	enum AutoDownload {
		AD_ASK = 0,
		AD_YES = 1,
		AD_NO = 2
	};

	enum NegotiateOption {
		NEG_ACCEPT = 0,
		NEG_REFUSE = 1,
		NEG_REQUEST = 2,
		NEG_REQUIRE = 3
	};

	enum SshHostKeyCheck {
		SHKC_ON = 0,
		SHKC_ASK = 1,
		SHKC_OFF = 2
	};

	enum FTPProtectionLevel {
		FTPPL_CLEAR = 0,
		FTPPL_CONFIDENTIAL = 1,
		FTPPL_PRIVATE = 2,
		FTPPL_SAFE = 3
	};

	enum ReverseDNSOption {
		RDO_AUTO = 0,
		RDO_ON = 1,
		RDO_OFF = 2
	};

	enum CertificateVerifyMode {
		CVM_DO_NOT_VERIFY = 0, // Do not verify host certificates
		CVM_VERIFY_HOSTS_IF_PRESENTED = 1, // Verify host certificates if presented
		CVM_REQUIRED_AND_VERIFIED = 2 //Peer certificates must be presented and verified; if not, fail
	};

	enum KeyboardBackspaceMode {
		KBM_DELETE = 0,
		KBM_CTRL_H = 1,
		KBM_CTRL_Y = 2
	};

	enum KeyboardEnterMode {
		KEM_CR = 0,
		KEM_CR_LF = 1,
		KEM_LF = 2
	};

	enum PrinterType {
		PT_NONE = 0,
		PT_WINDOWS = 1,
		PT_DOS_DEVICE = 2,
		PT_FILE = 3,
		PT_COMMAND = 4
	};

	enum WindowStartupMode {
		WSM_NORMAL = 0,
		WSM_MAXIMIZED = 1,
		WSM_MINIMIZED = 2
	};


	enum LogSessionMode {
		LSM_TEXT = 0,
		LSM_BINARY = 1,
		LSM_DEBUG = 2
	};



	// If this profile is currently stored in a config file, causes
	// the config file to be saved.
	virtual BOOL commitChanges() { return TRUE; }

	// Unique un-changing ID for this profile
	virtual int id() const = 0;

	// For iterating over profiles. Fetches the next profile in the
	// list of profiles. Note that the caller is responsible for
	// disposing of the returned profile when finished with it.
	virtual ConnectionProfile* nextProfile() const = 0;

	// ----- General -----
	virtual CMString name() = 0;
	virtual void setName(CMString name) = 0;

	virtual CMString description() = 0;
	virtual void setDescription(CMString description) = 0;

	virtual CMString notes() = 0;
	virtual void setNotes(CMString notes) = 0;

	virtual BOOL isTemplate() { return connectionType() == CT_TEMPLATE; }

	virtual CMString downloadDirectory() = 0;
	virtual void setDownloadDirectory(CMString dir) = 0;

	virtual CMString startingDirectory() = 0;
	virtual void setStartingDirectory(CMString dir) = 0;

	// ----- Connection -----
	virtual ConnectionType connectionType() = 0;

	// If CT_SSH, CT_FTP, CT_IP
	virtual CMString hostname() = 0;  // Or IP
	virtual void setHostname(CMString hostname) = 0;	

	// If CT_SSH, CT_FTP, CT_IP
	virtual int port() = 0;
	virtual void setPort(int p) = 0;

	// If CT_IP
	virtual IPProtocol ipProtocol() = 0;
	virtual void setIpProtocol(IPProtocol p) = 0;

	// If CT_LAT, CT_CTERM
	virtual CMString latService() = 0;
	virtual void setLatService(CMString svc) = 0;

	// If CT_NAMED_PIPE
	virtual CMString namedPipeName() = 0;
	virtual void setNamedPipeName(CMString name) = 0;

	// If CT_NAMED_PIPE
	virtual CMString namedPipeHost() = 0;
	virtual void setNamedPipeHost(CMString host) = 0;

	// if CT_PTY, CT_PIPE
	virtual CMString ptyCommand() = 0;
	virtual void setPtyCommand(CMString command) = 0;

	// if CT_DLL
	virtual CMString dllName() = 0;
	virtual void setDllName(CMString name) = 0;

	// if CT_DLL
	virtual CMString dllParameters() = 0;
	virtual void setDllParameters(CMString params) = 0;

	virtual BOOL exitOnDisconnect() = 0;
	virtual void setExitOnDisconnect(BOOL exit) = 0;

	virtual void setIpConnectionDetails(
		CMString hostname, int port, 
		IPProtocol protocol = ConnectionProfile::IPP_DEFAULT) {

		setHostname(hostname);
		setPort(port);
		setIpProtocol(protocol);
	}
	
	virtual void setSerialConnectionDetails(CMString line, int speed) {
		setLine(line);
		setLineSpeed(speed);
	}
	
	virtual void setNamedPipeConnectionDetails(CMString name, CMString host) {
		setNamedPipeName(name);
		setNamedPipeHost(host);
	}

	virtual void setDllConnectionDetails(CMString name, CMString parameters) {
		setDllName(name);
		setDllParameters(parameters);
	}

	// ----- Terminal -----
	virtual CMString terminalType() = 0;
	virtual void setTerminalType(CMString type) = 0;

	virtual BOOL is8Bit() = 0; // Rather than 7-bit
	virtual void setIs8Bit(BOOL is8bit) = 0;

	virtual BOOL apcEnabled() = 0;
	virtual void setApcEnabled(BOOL enabled) = 0;

	virtual BOOL localEchoEnabled() = 0;
	virtual void setLocalEchoEnabled(BOOL enabled) = 0;

	virtual BOOL autoWrapEnabled() = 0;
	virtual void setAutoWrapEnabled(BOOL enabled) = 0;

	virtual BOOL statusLineEnabled() = 0;
	virtual void setStatusLineEnabled(BOOL enabled) = 0;

	virtual CMString characterSet() = 0;
	virtual void setCharacterSet(CMString cset) = 0;

	virtual int screenWidth() = 0;
	virtual void setScreenWidth(int w) = 0;

	virtual int screenHeight() = 0;
	virtual void setScreenHeight(int h) = 0;

	virtual int scrollbackLines() = 0;
	virtual void setScrollbackLines(int lines) = 0;

	virtual Cursor cursor() = 0;
	virtual void setCursor(Cursor cur) = 0;

	// ----- Terminal Colors -----
	virtual Color terminalForegroundColor() = 0;
	virtual void setTerminalForegroundColor(ConnectionProfile::Color c) = 0;

	virtual Color terminalBackgroundColor() = 0;
	virtual void setTerminalBackgroundColor(ConnectionProfile::Color c) = 0;

	virtual Color commandForegroundColor() = 0;
	virtual void setCommandForegroundColor(ConnectionProfile::Color c) = 0;

	virtual Color commandBackgroundColor() = 0;
	virtual void setCommandBackgroundColor(ConnectionProfile::Color c) = 0;

	virtual Color statusLineForegroundColor() = 0;
	virtual void setStatusLineForegroundColor(ConnectionProfile::Color c) = 0;

	virtual Color statusLineBackgroundColor() = 0;
	virtual void setStatusLineBackgroundColor(ConnectionProfile::Color c) = 0;

	virtual Color mouseSelectionForegroundColor() = 0;
	virtual void setMouseSelectionForegroundColor(ConnectionProfile::Color c) = 0;

	virtual Color mouseSelectionBackgroundColor() = 0;
	virtual void setMouseSelectionBackgroundColor(ConnectionProfile::Color c) = 0;

	virtual Color popupHelpForegroundColor() = 0;
	virtual void setPopupHelpForegroundColor(ConnectionProfile::Color c) = 0;

	virtual Color popupHelpBackgroundColor() = 0;
	virtual void setPopupHelpBackgroundColor(ConnectionProfile::Color c) = 0;

	virtual Color underlineSimulationForegroundColor() = 0;
	virtual void setUnderlineSimulationForegroundColor(ConnectionProfile::Color c) = 0;

	virtual Color underlineSimulationBackgroundColor() = 0;
	virtual void setUnderlineSimulationBackgroundColor(ConnectionProfile::Color c) = 0;

	// ----- File Transfer -----
	virtual FileTransferProtocol fileTransferProtocol() = 0;
	virtual void setFileTransferProtocol(FileTransferProtocol proto) = 0;

	virtual KermitPerformance kermitPerformance() = 0;
	virtual void setKermitPerformance(KermitPerformance kp) = 0;

	virtual int packetLength() = 0;
	virtual void setPacketLength(int len) = 0;

	virtual int windowSize() = 0;
	virtual void setWindowSize(int size) = 0;

	virtual ControlCharUnprefixing controlCharUnprefixing() = 0;
	virtual void setControlCharUnprefixing(ControlCharUnprefixing ccu) = 0;

	virtual BOOL defaultToBinaryMode() = 0;
	virtual void setDefaultToBinaryMode(BOOL enabled) = 0;

	virtual CMString fileCharacterSet() = 0;
	virtual void setFileCharacterSet(CMString cset) = 0;

	virtual CMString transferCharacterSet() = 0;
	virtual void setTransferCharacterSet(CMString cset) = 0;

	virtual FileNameCollision fileNameCollisionAction() = 0;
	virtual void setFileNameCollisionAction(FileNameCollision fnc) = 0;

	virtual AutoDownload autoDownloadMode() = 0;
	virtual void setAutoDownloadMode(AutoDownload ad) = 0;

	virtual BOOL transmitLiteralFilenames() = 0;
	virtual void setTransmitLiteralFilenames(BOOL enabled) = 0;

	virtual BOOL usePathnames() = 0;
	virtual void setUsePathnames(BOOL enabled) = 0;

	virtual BOOL keepIncompleteFiles() = 0;
	virtual void setKeepIncomingFiles(BOOL enabled) = 0;

	virtual BOOL negotiateStreamingTransferMode() = 0;
	virtual void setNegotiateStreamingTransferMode(BOOL enabled) = 0;

	virtual BOOL negotiateClearChannelTransferMode() = 0;
	virtual void setNegotiateClearChannelTransferMode(BOOL enabled) = 0;

	virtual BOOL force16bitCRC() = 0;
	virtual void setForce16bitCRC(BOOL enabled) = 0;

	// ----- Serial -----
	virtual CMString line() = 0;
	virtual void setLine(CMString line) = 0;

	virtual unsigned int lineSpeed() = 0;
	virtual void setLineSpeed(int speed) = 0;

	virtual FlowControl flowControl() = 0;
	virtual void setFlowControl(FlowControl fc) = 0;

	virtual Parity parity() = 0;
	virtual void setParity(Parity p) = 0;

	virtual StopBits stopBits() = 0;
	virtual void setStopBits(StopBits sb) = 0;

	virtual BOOL errorCorrection() = 0;
	virtual void setErrorCorrection(BOOL enabled) = 0;

	virtual BOOL dataCompression() = 0;
	virtual void setDataCompression(BOOL enabled) = 0;

	virtual BOOL carrierDetection() = 0;
	virtual void setCarrierDetection(BOOL enabled) = 0;

	
	// ----- Telnet -----
	virtual NegotiateOption telnetAuthenticationOption() = 0;
	virtual void setTelnetAuthenticationOption(NegotiateOption opt) = 0;

	virtual NegotiateOption telnetBinaryModeOption() = 0;
	virtual void setTelnetBinaryModeOption(NegotiateOption opt) = 0;

	virtual NegotiateOption telnetEncryptionOption() = 0;
	virtual void setTelnetEncryptionOption(NegotiateOption opt) = 0;

	virtual NegotiateOption telnetKermitOption() = 0;
	virtual void setTelnetKermitOption(NegotiateOption opt) = 0;

	virtual NegotiateOption telnetStartTLSOption() = 0;
	virtual void setTelnetStartTLSOption(NegotiateOption opt) = 0;

	virtual NegotiateOption telnetForwardXOption() = 0;
	virtual void setTelnetForwardXOption(NegotiateOption opt) = 0;

	virtual NegotiateOption telnetComPortControlOption() = 0;
	virtual void setTelnetComPortControlOption(NegotiateOption opt) = 0;

	virtual CMString telnetEnvarAccount() = 0;
	virtual void setTelnetEnvarAccount(CMString val) = 0;

	virtual CMString telnetEnvarDisplay() = 0;
	virtual void setTelnetEnvarDisplay(CMString val) = 0;

	virtual CMString telnetEnvarJob() = 0;
	virtual void setTelnetEnvarJob(CMString val) = 0;

	virtual CMString telnetEnvarLocation() = 0;
	virtual void setTelnetEnvarLocation(CMString val) = 0;

	virtual CMString telnetEnvarPrinter() = 0;
	virtual void setTelnetEnvarPrinter(CMString val) = 0;

	virtual CMString telnetEnvarTerminalType() = 0;
	virtual void setTelnetEnvarTerminalType(CMString val) = 0;

	virtual BOOL telnetForwardCredentials() = 0;
	virtual void setTelnetForwardCredentials(BOOL enabled) = 0;

	virtual BOOL telnetNegotiationsMustComplete() = 0;
	virtual void setTelnetNegotiationsMustComplete(BOOL enabled) = 0;

	virtual BOOL telnetDebugMode() = 0;
	virtual void setTelnetDebugMode(BOOL enabled) = 0;

	virtual BOOL telnetSubnegotiationDelay() = 0;
	virtual void setTelnetSubnegotiationDelay(BOOL enabled) = 0;

	// ----- SSH -----
	virtual BOOL sshCompressionEnabled() = 0;		// off
	virtual void setSshCompressionEnabled(BOOL enabled) = 0;

	virtual BOOL sshX11ForwardingEnabled() = 0;		// off
	virtual void setSshX11ForwardingEnabled(BOOL enabled) = 0;

	virtual SshHostKeyCheck sshHostKeyChecking() = 0;	// ask
	virtual void setSshHostKeyChecking(SshHostKeyCheck shkc) = 0;

	virtual CMString sshIdentityFile() = 0;			// NULL
	virtual void setSshIdentityFile(CMString file) = 0;

	virtual CMString sshUserKnownHostsFile() = 0;		// NULL
	virtual void setSshUserKnownHostsFile(CMString file) = 0;

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
	virtual CMString ftpAuthType() = 0;	// TODO: this needs to be bit flags?
	virtual void setFtpAuthType(CMString type) = 0;

	virtual FTPProtectionLevel ftpCommandProtectionLevel() = 0;
	virtual void setFtpCommandProtectionLevel(FTPProtectionLevel pl) = 0;

	virtual FTPProtectionLevel ftpDataProtectionLevel() = 0;
	virtual void setFtpDataProtectionLevel(FTPProtectionLevel pl) = 0;

	virtual CMString ftpServerCharset() = 0;
	virtual void setFtpServerCharset(CMString cset) = 0;

	virtual BOOL ftpAutoLogin() = 0;
	virtual void setFtpAutoLogin(BOOL enabled) = 0;

	virtual BOOL ftpAutoAuthentication() = 0;
	virtual void setFtpAutoAuthentication(BOOL enabled) = 0;

	virtual BOOL ftpAutoEncryption() = 0;
	virtual void setFtpAutoEncryption(BOOL enabled) = 0;

	virtual BOOL ftpPassiveMode() = 0;
	virtual void setFtpPassiveMode(BOOL enabled) = 0;

	virtual BOOL ftpCredentialForwarding() = 0;
	virtual void setFtpCredentialForwarding(BOOL enabled) = 0;

	virtual BOOL ftpDates() = 0;
	virtual void setFtpDates(BOOL enabled) = 0;

	virtual BOOL ftpSendLiteralFilenames() = 0;
	virtual void setFtpSendLiteralFilenames(BOOL enabled) = 0;

	virtual BOOL ftpVerboseMessages() = 0;
	virtual void setFtpVerboseMessages(BOOL enabled) = 0;

	virtual BOOL ftpDebugMessages() = 0;
	virtual void setFtpDebugMessages(BOOL enabled) = 0;

	virtual BOOL ftpCharacterSetTranslation() = 0;
	virtual void setFtpCharacterSetTranslation(BOOL enabled) = 0;


	// ----- TCP/IP -----
	virtual int tcpSendBufferSize() = 0;
	virtual void setTcpSendBufferSize(int size) = 0;

	virtual int tcpReceiveBufferSize() = 0;
	virtual void setTcpReceiveBufferSize(int size) = 0;

	virtual ReverseDNSOption tcpReverseDNSLookup() = 0;
	virtual void setTcpReverseDNSLookup(ReverseDNSOption opt) = 0;

	virtual BOOL tcpDNSServiceLookup() = 0;
	virtual void setTcpDNSServiceLookup(BOOL enabled) = 0;

	virtual CMString tcpSocksHostname() = 0;
	virtual void setTcpSocksHostname(CMString host) = 0;

	virtual int tcpSocksPort() = 0;	// 0 for unspecified
	virtual void setTcpSocksPort(int port) = 0;

	virtual CMString tcpSocksUser() = 0;
	virtual void setTcpSocksUser(CMString user) = 0;

	virtual CMString tcpSocksPassword() = 0;
	virtual void setTcpSocksPassword(CMString pass) = 0;

	virtual CMString tcpHttpHostname() = 0;
	virtual void setTcpHttpHostname(CMString host) = 0;

	virtual int tcpHttpPort() = 0;	// 0 for unspecified
	virtual void setTcpHttpPort(int port) = 0;

	virtual CMString tcpHttpUser() = 0;
	virtual void setTcpHttpUser(CMString user) = 0;

	virtual CMString tcpHttpPassword() = 0;
	virtual void setTcpHttpPassword(CMString pass) = 0;


	// ----- Kerberos -----
	// Someday

	// ----- TLS -----
	virtual CMString tlsCipherList() = 0;
	virtual void setTlsCipherList(CMString file) = 0;

	virtual CMString tlsClientCertificateFile() = 0;
	virtual void setTlsClientCertificateFile(CMString file) = 0;

	virtual CMString tlsClientPrivateKeyFile() = 0;
	virtual void setTlsClientPrivateKeyFile(CMString file) = 0;

	virtual CMString tlsCAVerificationFile() = 0;
	virtual void setTlsCAVerificationFile(CMString file) = 0;

	virtual CMString tlsCAVerificationDirectory() = 0;
	virtual void setTlsCAVerificationDirectory(CMString dir) = 0;

	virtual CMString tlsCRLFile() = 0;
	virtual void setTlsCRLFile(CMString file) = 0;

	virtual CMString tlsCRLDirectory() = 0;
	virtual void setTlsCRLDirectory(CMString dir) = 0;

	virtual CertificateVerifyMode tlsCertificateVerifyMode() = 0;
	virtual void setTlsCertificateVerifyMode(CertificateVerifyMode mode) = 0;

	virtual BOOL tlsVerboseMode() = 0;
	virtual void setTlsVerboseMode(BOOL enabled) = 0;

	virtual BOOL tlsDebugMode() = 0;
	virtual void setTlsDebugMode(BOOL enabled) = 0;

	// ----- Keyboard -----
	virtual KeyboardBackspaceMode keyboardBackspaceSends() = 0;
	virtual void setKeyboardBackspaceSends(KeyboardBackspaceMode kbm) = 0;

	virtual KeyboardEnterMode keyboardEnterSends() = 0;
	virtual void setKeyboardEnterSends(KeyboardEnterMode kem) = 0;

	virtual BOOL mouseEnabled() = 0;	// Add a mouse tab?
	virtual void setMouseEnabled(BOOL enabled) = 0;

	virtual BOOL defaultKeyMap() = 0;
	virtual void setDefaultKeyMap(BOOL enabled) = 0;

	virtual CMString keymapFile() = 0;
	virtual void setKeymapFile(CMString file) = 0;

	virtual CMString additionalKeyMaps() = 0;
	virtual void setAdditionalKeyMaps(CMString str) = 0;

	// ----- Login -----
	virtual CMString userId() = 0;
	virtual void setUserId(CMString uid) = 0;

	virtual CMString password() = 0;
	virtual void setPassword(CMString pass) = 0;

	virtual BOOL promptForPassword() = 0;
	virtual void setPromptForPassword(BOOL prompt) = 0;

	virtual CMString passwordPrompt() = 0;
	virtual void setPasswordPrompt(CMString prompt) = 0;

	virtual BOOL runLoginScriptFile() = 0;
	virtual void setRunLoginScriptFile(BOOL run) = 0;

	virtual CMString loginScriptFile() = 0;
	virtual void setLoginScriptFile(CMString file) = 0;

	virtual CMString loginScript() = 0;
	virtual void setLoginScript(CMString script) = 0;

	// ----- Printer -----
	virtual PrinterType printerType() = 0;
	virtual void setPrinterType(PrinterType pt) = 0;

	// aka aka file aka command
	virtual CMString deviceName() = 0;
	virtual void setDeviceName(CMString device) = 0;

	virtual CMString windowsPrintQueue() = 0;	// use "<default>" for the default
	virtual void setWindowsPrintQueue(CMString queue) = 0;
	
	virtual CMString headerFile() = 0;
	virtual void setHeaderFile(CMString file) = 0;

	virtual BOOL sendEndOfJobString() = 0;
	virtual void setSendEndOfJobString(BOOL enabled) = 0;

	virtual CMString endOfJobString() = 0;	// SHOULD BE CHECKED BY DEFAULT
	virtual void setEndOfJobString(CMString string) = 0;

	virtual CMString printCharacterSet() = 0;
	virtual void setPrintCharacterSet(CMString cset) = 0;

	virtual BOOL printAsPostScript() = 0;
	virtual void setPrintAsPostScript(BOOL enabled) = 0;

	virtual int printWidth() = 0;
	virtual void setPrintWidth(int w) = 0;

	virtual int printHeight() = 0;
	virtual void setPrintHeight(int h) = 0;

	virtual int printSpeed() = 0;
	virtual void setPrintSpeed(int speed) = 0;

	virtual FlowControl printFlowControl() = 0;
	virtual void setPrintFlowControl(FlowControl fc) = 0;

	virtual Parity printParity() = 0;
	virtual void setPrintParity(Parity p) = 0;

	virtual BOOL bidirectionalPrinting() = 0;
	virtual void setBidirectionalPrinting(BOOL enabled) = 0;

	virtual int printTimeoutSeconds() = 0;	// TODO: missing from the UI!
	virtual void setPrintTimeoutSeconds(int seconds) = 0;

	// ----- GUI -----
	virtual BOOL useGUIKermit() = 0;
	virtual void setUseGUIKermit(BOOL enabled) = 0;

	virtual CMString fontName() = 0;
	virtual void setFontName(CMString font) = 0;

	virtual int fontSize() = 0;
	virtual void setFontSize(int size) = 0;

	virtual BOOL customStartupPosition() = 0;
	virtual void setCustomStartupPosition(BOOL enabled) = 0;

	virtual int startupAtX() = 0;
	virtual void setStartupAtX(int x) = 0;

	virtual int startupAtY() = 0;
	virtual void setStartupAtY(int y) = 0;

	virtual BOOL resizeChangesDimensions() = 0;
	virtual void setResizeChangesDimensions(BOOL enabled) = 0;

	virtual WindowStartupMode windowStartupMode() = 0;
	virtual void setWindowStartupMode(WindowStartupMode wsm) = 0;

	virtual BOOL menubarEnabled() = 0;
	virtual void setMenubarEnabled(BOOL enabled) = 0;

	virtual BOOL toolbarEnabled() = 0;
	virtual void setToolbarEnabled(BOOL enabled) = 0;

	virtual BOOL statusbarEnabled() = 0;
	virtual void setStatusbarEnabled(BOOL enabled) = 0;

	virtual BOOL dialogsEnabled() = 0;
	virtual void setDialogsEnabled(BOOL enabled) = 0;

	// ----- GUI Colors -----
	virtual RGB8BitColor rgbBlack() = 0;
	virtual void setRgbBlack(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbBlue() = 0;
	virtual void setRgbBlue(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbGreen() = 0;
	virtual void setRgbGreen(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbCyan() = 0;
	virtual void setRgbCyan(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbRed() = 0;
	virtual void setRgbRed(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbMagenta() = 0;
	virtual void setRgbMagenta(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbBrown() = 0;
	virtual void setRgbBrown(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbLightGray() = 0;
	virtual void setRgbLightGray(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbDarkGray() = 0;
	virtual void setRgbDarkGray(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbLightBlue() = 0;
	virtual void setRgbLightBlue(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbLightGreen() = 0;
	virtual void setRgbLightGreen(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbLightCyan() = 0;
	virtual void setRgbLightCyan(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbLightRed() = 0;
	virtual void setRgbLightRed(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbLightMagenta() = 0;
	virtual void setRgbLightMagenta(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbYellow() = 0;
	virtual void setRgbYellow(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbWhite() = 0;
	virtual void setRgbWhite(RGB8BitColor c) = 0;

	// ----- Logging -----
	virtual BOOL logConnections() = 0;
	virtual void setLogConnections(BOOL enabled) = 0;

	virtual BOOL logAppendConnections() = 0;
	virtual void setLogAppendConnections(BOOL enabled) = 0;

	virtual CMString logConnectionsFile() = 0;
	virtual void setLogConnectionsFile(CMString file) = 0;

	virtual BOOL logDebug() = 0;
	virtual void setLogDebug(BOOL enabled) = 0;

	virtual BOOL logAppendDebug() = 0;
	virtual void setLogAppendDebug(BOOL enabled) = 0;

	virtual CMString logDebugFile() = 0;
	virtual void setLogDebugFile(CMString file) = 0;

	virtual BOOL logKermitFileTransfers() = 0;
	virtual void setLogKermitFileTransfers(BOOL enabled) = 0;

	virtual BOOL logAppendKermitFileTransfers() = 0;
	virtual void setLogAppendKermitFileTransfers(BOOL enabled) = 0;

	virtual CMString logKermitFileTransfersFile() = 0;
	virtual void setLogKermitFileTransfersFile(CMString file) = 0;
	
	virtual BOOL logSessionInput() = 0;
	virtual void setLogSessionInput(BOOL enabled) = 0;

	virtual BOOL logAppendSessionInput() = 0;
	virtual void setLogAppendSessionInput(BOOL enabled) = 0;

	virtual CMString logSessionInputFile() = 0;
	virtual void setLogSessionInputFile(CMString file) = 0;

	virtual LogSessionMode logSessionInputMode() = 0;
	virtual void setLogSessionInputMode(LogSessionMode mode) = 0;

	virtual BOOL logFileTransferTransactions() = 0;
	virtual void setLogFileTransferTransactions(BOOL enabled) = 0;

	virtual BOOL logAppendFileTransferTransactions() = 0;
	virtual void setLogAppendFileTransferTransactions(BOOL enabled) = 0;

	virtual BOOL logBriefFileTransferTransactions() = 0;
	virtual void setLogBriefFileTransferTransactions(BOOL enabled) = 0;

	virtual CMString logFileTransferTransactionsFile() = 0;
	virtual void setLogFileTransferTransactionsFile(CMString file) = 0;

	// ===== Misc =====

	// These should call through to ConfigFile to get an app-wide
	// setting rather than being profile-specific.
	virtual CMString appBrowser() { return NULL; }
	virtual CMString appEditor() { return NULL; }


	// ===== Utility =====
	virtual LPTSTR targetName() {
		if (_targetName == NULL) {
			setTargetName();
		}
		return _targetName;
	}

	// ===== Connection launching =====
	virtual DWORD connect(HWND parent); // returns process Id on success

protected:
	virtual void setTargetName();
	virtual BOOL writeScript(HWND parent, LPTSTR filename);

	LPTSTR _targetName;

};

#endif /* CONN_PROFILE_H */