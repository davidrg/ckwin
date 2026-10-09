#ifndef JSON_TEMPLATE_H
#define JSON_TEMPLATE_H

#include "json_profile.h"

class JsonTemplate: public JsonProfile {
public:

	/*
	 * The various "isSet" functions here allow you to check
	 * if the setting actually exists in the config file or
	 * if calls to the associated "get" method will just
	 * return the default value for that setting.
	 *
	 * When creating a profile (or another template) from a
	 * template, only those settings that have actually been
	 * set should be copied from the template.
     */

	// ----- General -----
	virtual BOOL isSetNotes();
	virtual BOOL isSetDownloadDirectory();
	virtual BOOL isSetStartingDirectory();


	// ----- Terminal -----
	virtual BOOL isSetTerminalType();
	virtual BOOL isSetIs8Bit();
	virtual BOOL isSetApcEnabled();
	virtual BOOL isSetLocalEchoEnabled();
	virtual BOOL isSetAutoWrapEnabled();
	virtual BOOL isSetStatusLineEnabled();
	virtual BOOL isSetCharacterSet();
	virtual BOOL isSetScreenWidth();
	virtual BOOL isSetScreenHeight();
	virtual BOOL isSetScrollbackLines();
	virtual BOOL isSetCursor();

	// ----- Terminal Colors -----
	virtual BOOL isSetTerminalForegroundColor();
	virtual BOOL isSetTerminalBackgroundColor();
	virtual BOOL isSetCommandForegroundColor();
	virtual BOOL isSetCommandBackgroundColor();
	virtual BOOL isSetStatusLineForegroundColor();
	virtual BOOL isSetStatusLineBackgroundColor();
	virtual BOOL isSetMouseSelectionForegroundColor();
	virtual BOOL isSetMouseSelectionBackgroundColor();
	virtual BOOL isSetPopupHelpForegroundColor();
	virtual BOOL isSetPopupHelpBackgroundColor();
	virtual BOOL isSetUnderlineSimulationForegroundColor();
	virtual BOOL isSetUnderlineSimulationBackgroundColor();

	// ----- File Transfer -----
	virtual BOOL isSetFileTransferProtocol();
	virtual BOOL isSetKermitPerformance();
	virtual BOOL isSetPacketLength();
	virtual BOOL isSetWindowSize();
	virtual BOOL isSetControlCharUnprefixing();
	virtual BOOL isSetDefaultToBinaryMode();
	virtual BOOL isSetFileCharacterSet();
	virtual BOOL isSetTransferCharacterSet();
	virtual BOOL isSetFileNameCollisionAction();
	virtual BOOL isSetAutoDownloadMode();
	virtual BOOL isSetTransmitLiteralFilenames();
	virtual BOOL isSetUsePathnames();
	virtual BOOL isSetKeepIncomingFiles();
	virtual BOOL isSetNegotiateStreamingTransferMode();
	virtual BOOL setNegotiateClearChannelTransferMode();
	virtual BOOL isSetForce16bitCRC();

	// ----- Serial -----
	virtual BOOL isSetLine();
	virtual BOOL isSetLineSpeed();
	virtual BOOL isSetFlowControl();
	virtual BOOL isSetParity();
	virtual BOOL isSetStopBits();
	virtual BOOL isSetErrorCorrection();
	virtual BOOL isSetDataCompression();
	virtual BOOL isSetCarrierDetection();

	
	// ----- Telnet -----
	virtual BOOL isSetTelnetAuthenticationOption();
	virtual BOOL isSetTelnetBinaryModeOption();
	virtual BOOL isSetTelnetEncryptionOption();
	virtual BOOL isSetTelnetKermitOption();
	virtual BOOL isSetTelnetStartTLSOption();
	virtual BOOL isSetTelnetForwardXOption();
	virtual BOOL isSetTelnetComPortControlOption();
	virtual BOOL isSetTelnetEnvarAccount();
	virtual BOOL isSetTelnetEnvarDisplay();
	virtual BOOL isSetTelnetEnvarJob();
	virtual BOOL isSetTelnetEnvarLocation();
	virtual BOOL isSetTelnetEnvarPrinter();
	virtual BOOL isSetTelnetEnvarTerminalType();
	virtual BOOL isSetTelnetForwardCredentials();
	virtual BOOL isSetTelnetNegotiationsMustComplete();
	virtual BOOL isSetTelnetDebugMode();
	virtual BOOL isSetTelnetSubnegotiationDelay();

	// ----- SSH -----
	virtual BOOL isSetSshCompressionEnabled();
	virtual BOOL isSetSshX11ForwardingEnabled();
	virtual BOOL isSetSshHostKeyChecking();
	virtual BOOL isSetSshIdentityFile();
	virtual BOOL isSetSshUserKnownHostsFile();

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
	virtual BOOL isSetFtpAuthType();
	virtual BOOL isSetFtpCommandProtectionLevel();
	virtual BOOL isSetFtpDataProtectionLevel();
	virtual BOOL isSetFtpServerCharset();
	virtual BOOL isSetFtpAutoLogin();
	virtual BOOL isSetFtpAutoAuthentication();
	virtual BOOL isSetFtpAutoEncryption();
	virtual BOOL isSetFtpPassiveMode();
	virtual BOOL isSetFtpCredentialForwarding();
	virtual BOOL isSetFtpDates();
	virtual BOOL isSetFtpSendLiteralFilenames();
	virtual BOOL isSetFtpVerboseMessages();
	virtual BOOL isSetFtpDebugMessages();
	virtual BOOL isSetFtpCharacterSetTranslation();


	// ----- TCP/IP -----
	virtual BOOL isSetTcpSendBufferSize();
	virtual BOOL isSetTcpReceiveBufferSize();
	virtual BOOL isSetTcpReverseDNSLookup();
	virtual BOOL isSetTcpDNSServiceLookup();
	virtual BOOL isSetTcpSocksHostname();
	virtual BOOL isSetTcpSocksPort();
	virtual BOOL isSetTcpSocksUser();
	virtual BOOL isSetTcpSocksPassword();
	virtual BOOL isSetTcpHttpHostname();
	virtual BOOL isSetTcpHttpPort();
	virtual BOOL isSetTcpHttpUser();
	virtual BOOL isSetTcpHttpPassword();


	// ----- Kerberos -----
	// Someday

	// ----- TLS -----
	virtual BOOL isSetTlsCipherList();
	virtual BOOL isSetTlsClientCertificateFile();
	virtual BOOL isSetTlsClientPrivateKeyFile();
	virtual BOOL isSetTlsCAVerificationFile();
	virtual BOOL isSetTlsCAVerificationDirectory();
	virtual BOOL isSetTlsCRLFile();
	virtual BOOL isSetTlsCRLDirectory();
	virtual BOOL isSetTlsCertificateVerifyMode();
	virtual BOOL isSetTlsVerboseMode();
	virtual BOOL isSetTlsDebugMode();


	// ----- Keyboard -----
	virtual BOOL isSetKeyboardBackspaceSends();
	virtual BOOL isSetKeyboardEnterSends();
	virtual BOOL isSetMouseEnabled();
	virtual BOOL isSetDefaultKeyMap();
	virtual BOOL isSetKeymapFile();
	virtual BOOL isSetAdditionalKeyMaps();

	// ----- Login -----
	virtual BOOL isSetUserId();
	virtual BOOL isSetPassword();
	virtual BOOL isSetPromptForPassword();
	virtual BOOL isSetPasswordPrompt();
	virtual BOOL isSetRunLoginScriptFile();
	virtual BOOL isSetLoginScriptFile();
	virtual BOOL isSetLoginScript();


	// ----- Printer -----
	virtual BOOL isSetPrinterType();
	virtual BOOL isSetDeviceName();
	virtual BOOL isSetWindowsPrintQueue();
	virtual BOOL isSetHeaderFile();
	virtual BOOL isSetSendEndOfJobString();
	virtual BOOL isSetEndOfJobString();
	virtual BOOL isSetPrintCharacterSet();
	virtual BOOL isSetPrintAsPostScript();
	virtual BOOL isSetPrintWidth();
	virtual BOOL isSetPrintHeight();
	virtual BOOL isSetPrintSpeed();
	virtual BOOL isSetPrintFlowControl();
	virtual BOOL isSetPrintParity();
	virtual BOOL isSetBidirectionalPrinting();
	virtual BOOL isSetPrintTimeoutSeconds();

	// ----- GUI -----
	virtual BOOL isSetUseGUIKermit();
	virtual BOOL isSetFontName();
	virtual BOOL isSetFontSize();
	virtual BOOL isSetCustomStartupPosition();
	virtual BOOL isSetStartupAtX();
	virtual BOOL isSetStartupAtY();
	virtual BOOL isSetResizeChangesDimensions();
	virtual BOOL isSetWindowStartupMode();
	virtual BOOL isSetMenubarEnabled();
	virtual BOOL isSetToolbarEnabled();
	virtual BOOL isSetStatusbarEnabled();
	virtual BOOL isSetDialogsEnabled();

	// ----- GUI Colors -----
	virtual BOOL isSetRgbBlack();
	virtual BOOL isSetRgbBlue();
	virtual BOOL isSetRgbGreen();
	virtual BOOL isSetRgbCyan();
	virtual BOOL isSetRgbRed();
	virtual BOOL isSetRgbMagenta();
	virtual BOOL isSetRgbBrown();
	virtual BOOL isSetRgbLightGray();
	virtual BOOL isSetRgbDarkGray();
	virtual BOOL isSetRgbLightBlue();
	virtual BOOL isSetRgbLightGreen();
	virtual BOOL isSetRgbLightCyan();
	virtual BOOL isSetRgbLightRed();
	virtual BOOL isSetRgbLightMagenta();
	virtual BOOL isSetRgbYellow();
	virtual BOOL isSetRgbWhite();

	// ----- Logging -----
	virtual BOOL isSetLogConnections();
	virtual BOOL isSetLogAppendConnections();
	virtual BOOL isSetLogConnectionsFile();
	virtual BOOL isSetLogDebug();
	virtual BOOL isSetLogAppendDebug();
	virtual BOOL isSetLogDebugFile();
	virtual BOOL isSetLogKermitFileTransfers();
	virtual BOOL isSetLogAppendKermitFileTransfers();
	virtual BOOL isSetLogKermitFileTransfersFile();
	virtual BOOL isSetLogSessionInput();
	virtual BOOL isSetLogAppendSessionInput();
	virtual BOOL isSetLogSessionInputFile();
	virtual BOOL isSetLogSessionInputMode();
	virtual BOOL isSetLogFileTransferTransactions();
	virtual BOOL isSetLogAppendFileTransferTransactions();
	virtual BOOL isSetLogBriefFileTransferTransactions();
	virtual BOOL isSetLogFileTransferTransactionsFile();
};

#endif /* JSON_TEMPLATE_H */