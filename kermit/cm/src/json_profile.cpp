#include "json_profile.h"

#include "cJSON/cJSON.h"

JsonProfile::JsonProfile(JsonConfigFile *configFile, cJSON *json)
		:_json(json), _config(configFile) {

	_targetName = NULL;
}

JsonProfile::~JsonProfile() {

}

BOOL JsonProfile::commitChanges() {
	return _config->commitChanges();
}

ConnectionProfile* JsonProfile::nextProfile() const {

	if (_json->next == NULL) {
		// Either we're at the end of the list of profiles,
		// or this profile hasn't been added to the config
		// file yet.
		return NULL;
	}

	return new JsonProfile(_config, _json->next);
}

// -----------------------------------------------------------
// -----------------------------------------------------------
// -----------------------------------------------------------

CMString JsonProfile::getString(cJSON *json, LPCSTR name, 
								CMString defaultValue) const {

	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, name);

	if (obj == NULL || !cJSON_IsString(obj)) {
		// Value either doesn't exist or it isn't a string.
		// Just return the default value.
		return defaultValue;
	}

	return CMString::fromUtf8(obj->valuestring);
}

int JsonProfile::getInteger(cJSON *json, LPCSTR name,
						   int defaultValue) const {
	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, name);

	if (obj == NULL || !cJSON_IsNumber(obj)) {
		// Value either doesn't exist or it isn't an int.
		// Just return the default value.
		return defaultValue;
	}

	return obj->valueint;
}

BOOL JsonProfile::getBool(cJSON *json, LPCSTR name,
						   BOOL defaultValue) const {
	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, name);

	if (obj == NULL || !cJSON_IsBool(obj)) {
		// Value either doesn't exist or it isn't a bool.
		// Just return the default value.
		return defaultValue;
	}

	return cJSON_IsTrue(obj);
}

void JsonProfile::setString(cJSON *json, LPCSTR name, CMString value) {
	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, name);

	if (value.isNull()) {
		setNull(json, name);
		return;
	}

	if (obj != NULL && !cJSON_IsString(obj)) {
		// It already exists but its not a string. Delete.
		cJSON_DeleteItemFromObjectCaseSensitive(json, name);
		obj = NULL;
	}

	if (obj == NULL) {
		// Doesn't exist - add it
		cJSON_AddStringToObject(json, name, value.toUtf8(NULL));
	} else {
		// It exists - update its value.
		cJSON_SetValuestring(obj, value.toUtf8(NULL));
	}
}

void JsonProfile::setInteger(cJSON *json, LPCSTR name, int value) {
	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, name);

	if (obj != NULL && !cJSON_IsNumber(obj)) {
		// It already exists but its not a string. Delete.
		cJSON_DeleteItemFromObjectCaseSensitive(json, name);
		obj = NULL;
	}

	if (obj == NULL) {
		// Doesn't exist - add it
		cJSON_AddNumberToObject(json, name, value);
	} else {
		// It exists - update its value.
		cJSON_SetIntValue(obj, value);
	}
}

void JsonProfile::setBool(cJSON *json, LPCSTR name, BOOL value) {
	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, name);

	if (obj != NULL && !cJSON_IsBool(obj)) {
		// It already exists but its not a string. Delete.
		cJSON_DeleteItemFromObjectCaseSensitive(json, name);
		obj = NULL;
	}

	if (obj == NULL) {
		// Doesn't exist - add it
		cJSON_AddBoolToObject(json, name, value);
	} else {
		// It exists - update its value.
		cJSON_SetBoolValue(obj, value);
	}
}

void JsonProfile::setNull(cJSON *json, LPCSTR name) {
	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, name);

	
	if (obj != NULL && !cJSON_IsNull(obj)) {
		// It already exists but its not null. Delete it.
		cJSON_DeleteItemFromObjectCaseSensitive(json, name);
		obj = NULL;
	}

	if (obj == NULL) {
		// Doesn't exist - add it
		cJSON_AddNullToObject(json, name);
	}

	// Else it already exists and its already null. Nothing
	// to do.
}

CMString JsonProfile::getString(LPCSTR name, CMString defaultValue) const {
	return getString(_json, name, defaultValue);
}

int JsonProfile::getInteger(LPCSTR name, int defaultValue) const {
	return getInteger(_json, name, defaultValue);
}

BOOL JsonProfile::getBool(LPCSTR name, BOOL defaultValue) const {
	return getBool(_json, name, defaultValue);
}

void JsonProfile::setString(LPCSTR name, CMString value) {
	setString(_json, name, value);
}

void JsonProfile::setInteger(LPCSTR name, int value) {
	setInteger(_json, name, value);
}


void JsonProfile::setBool(LPCSTR name, BOOL value) {
	setBool(_json, name, value);
}

CMString JsonProfile::getString(LPCSTR parentName, LPCSTR name, CMString defaultValue) const {
	cJSON *parent = cJSON_GetObjectItemCaseSensitive(_json, parentName);

	if (parent == NULL || !cJSON_IsObject(parent)) {
		// parent object either doesn't exist or its not actually
		// an object. Return the default value.
		return defaultValue;
	}

	// Get the string value from the parent object.
	return getString(parent, name, defaultValue);
}

int JsonProfile::getInteger(LPCSTR parentName, LPCSTR name, int defaultValue) const {
	cJSON *parent = cJSON_GetObjectItemCaseSensitive(_json, parentName);

	if (parent == NULL || !cJSON_IsObject(parent)) {
		// parent object either doesn't exist or its not actually
		// an object. Return the default value.
		return defaultValue;
	}

	// Get the int value from the parent object.
	return getInteger(parent, name, defaultValue);
}

BOOL JsonProfile::getBool(LPCSTR parentName, LPCSTR name, BOOL defaultValue) const {
	cJSON *parent = cJSON_GetObjectItemCaseSensitive(_json, parentName);

	if (parent == NULL || !cJSON_IsObject(parent)) {
		// parent object either doesn't exist or its not actually
		// an object. Return the default value.
		return defaultValue;
	}

	// Get the bool value from the parent object.
	return getBool(parent, name, defaultValue);
}

void JsonProfile::setString(LPCSTR parentName, LPCSTR name, CMString value) {
	cJSON *parent = cJSON_GetObjectItemCaseSensitive(_json, parentName);

	if (parent == NULL) {
		// parent doesn't exist - create it
		parent = cJSON_CreateObject();
		cJSON_AddItemToObject(_json, parentName, parent);
	}

	setString(parent, name, value);
}

void JsonProfile::setInteger(LPCSTR parentName, LPCSTR name, int value) {
	cJSON *parent = cJSON_GetObjectItemCaseSensitive(_json, parentName);

	if (parent == NULL) {
		// parent doesn't exist - create it
		parent = cJSON_CreateObject();
		cJSON_AddItemToObject(_json, parentName, parent);
	}

	setInteger(parent, name, value);
}

void JsonProfile::setBool(LPCSTR parentName, LPCSTR name, BOOL value) {
	cJSON *parent = cJSON_GetObjectItemCaseSensitive(_json, parentName);

	if (parent == NULL) {
		// parent doesn't exist - create it
		parent = cJSON_CreateObject();
		cJSON_AddItemToObject(_json, parentName, parent);
	}

	setBool(parent, name, value);
}


CMString JsonProfile::getString(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, CMString defaultValue) const {
	cJSON *grandparent = cJSON_GetObjectItemCaseSensitive(
		_json, grandParentName);

	if (grandparent == NULL || !cJSON_IsObject(grandparent)) {
		// grandparent object either doesn't exist or its not actually
		// an object. Return the default value.
		return defaultValue;
	}

	cJSON *parent = cJSON_GetObjectItemCaseSensitive(
		grandparent, parentName);

	if (parent == NULL || !cJSON_IsObject(parent)) {
		// grandparent object either doesn't exist or its not actually
		// an object. Return the default value.
		return defaultValue;
	}

	// Get the string value from the parent object.
	return getString(parent, name, defaultValue);
}

int JsonProfile::getInteger(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, int defaultValue) const {
	cJSON *grandparent = cJSON_GetObjectItemCaseSensitive(
		_json, grandParentName);

	if (grandparent == NULL || !cJSON_IsObject(grandparent)) {
		// grandparent object either doesn't exist or its not actually
		// an object. Return the default value.
		return defaultValue;
	}

	cJSON *parent = cJSON_GetObjectItemCaseSensitive(grandparent, parentName);

	if (parent == NULL || !cJSON_IsObject(parent)) {
		// parent object either doesn't exist or its not actually
		// an object. Return the default value.
		return defaultValue;
	}

	// Get the int value from the parent object.
	return getInteger(parent, name, defaultValue);
}

BOOL JsonProfile::getBool(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, BOOL defaultValue) const {
	cJSON *grandparent = cJSON_GetObjectItemCaseSensitive(
		_json, grandParentName);

	if (grandparent == NULL || !cJSON_IsObject(grandparent)) {
		// grandparent object either doesn't exist or its not actually
		// an object. Return the default value.
		return defaultValue;
	}

	cJSON *parent = cJSON_GetObjectItemCaseSensitive(grandparent, parentName);

	if (parent == NULL || !cJSON_IsObject(parent)) {
		// parent object either doesn't exist or its not actually
		// an object. Return the default value.
		return defaultValue;
	}

	// Get the bool value from the parent object.
	return getBool(parent, name, defaultValue);
}

void JsonProfile::setString(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, CMString value) {
	cJSON *grandparent = cJSON_GetObjectItemCaseSensitive(
		_json, grandParentName);

	if (grandparent != NULL && !cJSON_IsObject(grandparent)) {
		cJSON_DeleteItemFromObjectCaseSensitive(_json, grandParentName);
		grandparent = NULL;
	}

	if (grandparent == NULL) {
		// grandparent object doesn't exist - create it
		grandparent = cJSON_CreateObject();
		cJSON_AddItemToObject(_json, grandParentName, grandparent);
	}

	cJSON *parent = cJSON_GetObjectItemCaseSensitive(grandparent, parentName);

	if (parent != NULL && !cJSON_IsObject(parent)) {
		cJSON_DeleteItemFromObjectCaseSensitive(grandparent, parentName);
		parent = NULL;
	}

	if (parent == NULL) {
		// parent doesn't exist - create it
		parent = cJSON_CreateObject();
		cJSON_AddItemToObject(_json, parentName, parent);
	}

	setString(parent, name, value);
}

void JsonProfile::setInteger(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, int value) {
	cJSON *grandparent = cJSON_GetObjectItemCaseSensitive(
		_json, grandParentName);

	if (grandparent != NULL && !cJSON_IsObject(grandparent)) {
		cJSON_DeleteItemFromObjectCaseSensitive(_json, grandParentName);
		grandparent = NULL;
	}

	if (grandparent == NULL) {
		// grandparent object doesn't exist - create it
		grandparent = cJSON_CreateObject();
		cJSON_AddItemToObject(_json, grandParentName, grandparent);
	}

	cJSON *parent = cJSON_GetObjectItemCaseSensitive(grandparent, parentName);

	if (parent != NULL && !cJSON_IsObject(parent)) {
		cJSON_DeleteItemFromObjectCaseSensitive(grandparent, parentName);
		parent = NULL;
	}

	if (parent == NULL) {
		// parent doesn't exist - create it
		parent = cJSON_CreateObject();
		cJSON_AddItemToObject(_json, parentName, parent);
	}

	setInteger(parent, name, value);
}

void JsonProfile::setBool(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, BOOL value) {
	cJSON *grandparent = cJSON_GetObjectItemCaseSensitive(
		_json, grandParentName);

	if (grandparent != NULL && !cJSON_IsObject(grandparent)) {
		cJSON_DeleteItemFromObjectCaseSensitive(_json, grandParentName);
		grandparent = NULL;
	}

	if (grandparent == NULL) {
		// grandparent object doesn't exist - create it
		grandparent = cJSON_CreateObject();
		cJSON_AddItemToObject(_json, grandParentName, grandparent);
	}

	cJSON *parent = cJSON_GetObjectItemCaseSensitive(grandparent, parentName);

	if (parent != NULL && !cJSON_IsObject(parent)) {
		cJSON_DeleteItemFromObjectCaseSensitive(grandparent, parentName);
		parent = NULL;
	}

	if (parent == NULL) {
		// parent doesn't exist - create it
		parent = cJSON_CreateObject();
		cJSON_AddItemToObject(_json, parentName, parent);
	}

	setBool(parent, name, value);
}

// -----------------------------------------------------------
// -----------------------------------------------------------
// -----------------------------------------------------------

/*
  These are pretty gross but they get the job done.

  Instead of calling these:
	getString(name, default)
	getString(parent, name, default)
	getString(grandparent, parent, name, default)

  Define the appropriate variable in the class:
    CMString _cached_{name};
    CMString _cached_{parent}_{name};
    CMString _cached_{grandparent}_{parent}_{name};

  Then call:
    getStringCached(name, default)
	getStringCached2(parent, name, default)
	getStringCached3(grandparent, parent name, default)
 */

#define getStringCached(name, def)		(_cached_##name .isNull() ? _cached_##name = getString(#name,def) : _cached_##name )
#define setStringCached(name, value)	_cached_##name = value ; \
	setString( #name , value );

#define getStringCached2(parent, name, def)		(_cached_##parent##_##name .isNull() ? \
	_cached_##parent##_##name = getString(#parent, #name, def) : \
	_cached_##parent##_##name )

#define setStringCached2(parent, name, value)	_cached_##parent##_##name = value ; \
	setString( #parent, #name , value );

#define getStringCached3(grandparent, parent, name, def)		(_cached_##grandparent##_##parent##_##name .isNull() ? \
	_cached_##grandparent##_##parent##_##name = getString(#grandparent, #parent, #name,def) : \
	_cached_##grandparent##_##parent##_##name )

#define setStringCached3(grandparent, parent, name, value)	_cached_##grandparent##_##parent##_##name = value ; \
	setString( #parent, #name , value );

// -----------------------------------------------------------
// -----------------------------------------------------------
// -----------------------------------------------------------

int JsonProfile::id() const {
	return getInteger("id", 0);
}

void JsonProfile::setId(int id) {
	setInteger("id", id);
}

CMString JsonProfile::name() {
	return getStringCached(name, CMString());
}


void JsonProfile::setName(CMString name) { 
	setStringCached(name, name);
}


CMString JsonProfile::description() { 
	return getStringCached(desc, CMString());
}


void JsonProfile::setDescription(CMString description) { 
	setStringCached(desc, description);
}


CMString JsonProfile::notes() { 
	return getStringCached(note, CMString());
}


void JsonProfile::setNotes(CMString notes) { 
	setStringCached(note, notes);
}


CMString JsonProfile::downloadDirectory() { 
	return getStringCached2(dirs, download, CMString(TEXT("\\v(home)downloads")));
}

void JsonProfile::setDownloadDirectory(CMString dir) { 
	setStringCached2(dirs, download, dir);
}

CMString JsonProfile::startingDirectory() { 
	return getStringCached2(dirs, start, CMString(TEXT("\\v(home)")));
}

void JsonProfile::setStartingDirectory(CMString dir) { 
	setStringCached2(dirs, start, dir);
}

// ----- Connection -----
ConnectionProfile::ConnectionType JsonProfile::connectionType() { 
	return (ConnectionType)getInteger("type", 0);
}


void JsonProfile::setConnectionType(ConnectionType conType) {
	setInteger("type", (int)conType);
}

// If CT_SSH, CT_FTP, CT_IP
CMString JsonProfile::hostname() { 
	return getStringCached2(ip, host, CMString());
}

void JsonProfile::setHostname(CMString hostname) { 
	setStringCached2(ip, host, hostname);
}	

// If CT_SSH, CT_FTP, CT_IP
int JsonProfile::port() { 
	return getInteger("ip", "port", 0);
}

void JsonProfile::setPort(int p) { 
	setInteger("ip", "port", p);
}

// If CT_IP
ConnectionProfile::IPProtocol JsonProfile::ipProtocol() { 
	return (IPProtocol)getInteger("ip", "protocol", 
		(int)IPP_DEFAULT);
}

void JsonProfile::setIpProtocol(IPProtocol p) { 
	setInteger("ip", "protocol", (int)p);
}

// If CT_LAT, CT_CTERM
CMString JsonProfile::latService() { 
	return getStringCached(lat_service, CMString());
}

void JsonProfile::setLatService(CMString svc) {
	setStringCached(lat_service, svc);
}

// If CT_NAMED_PIPE
CMString JsonProfile::namedPipeName() { 
	return getStringCached2(named_pipe, name, CMString());
}
void JsonProfile::setNamedPipeName(CMString name) { 
	setStringCached2(named_pipe, name, name);
}

// If CT_NAMED_PIPE
CMString JsonProfile::namedPipeHost() { 
	return getStringCached2(named_pipe, host, CMString());
}
void JsonProfile::setNamedPipeHost(CMString host) { 
	setStringCached2(named_pipe, host, host);
}

// if CT_PTY, CT_PIPE
CMString JsonProfile::ptyCommand() { 
	return getStringCached(pty_command, CMString());
}
void JsonProfile::setPtyCommand(CMString command) { 
	setStringCached(pty_command, command);
}

// if CT_DLL
CMString JsonProfile::dllName() { 
	return getStringCached2(dll, name, CMString());
}
void JsonProfile::setDllName(CMString name) { 
	setStringCached2(dll, name, name,);
}

// if CT_DLL
CMString JsonProfile::dllParameters() { 
	return getStringCached2(dll, params, CMString());
}

void JsonProfile::setDllParameters(CMString params) { 
	setStringCached2(dll, params, params);
}

BOOL JsonProfile::exitOnDisconnect() { 
	return getBool("exit_on_disconnect", TRUE);
}

void JsonProfile::setExitOnDisconnect(BOOL exit) { 
	setBool("exit_on_disconnect", exit);
}


// ----- Terminal -----
CMString JsonProfile::terminalType() { 
	return getStringCached2(terminal, type, CMString(TEXT("vt320"))); // was: vt100
}

void JsonProfile::setTerminalType(CMString type) {
	setStringCached2(terminal, type, type);
}

BOOL JsonProfile::is8Bit() { 
	return getBool("terminal", "8bit", FALSE);
}

void JsonProfile::setIs8Bit(BOOL is8bit) {
	setBool("terminal", "8bit", is8bit);
}

BOOL JsonProfile::apcEnabled() { 
	return getBool("terminal", "apc", FALSE);
}

void JsonProfile::setApcEnabled(BOOL enabled) {
	setBool("terminal", "apc", enabled);
}

BOOL JsonProfile::localEchoEnabled() { 
	return getBool("terminal", "local_echo", FALSE);
}

void JsonProfile::setLocalEchoEnabled(BOOL enabled) {
	setBool("terminal", "local_echo", enabled);
}

BOOL JsonProfile::autoWrapEnabled() { 
	return getBool("temrinal", "auto_wrap", TRUE);
}

void JsonProfile::setAutoWrapEnabled(BOOL enabled) {
	setBool("terminal", "auto_wrap", enabled);
}

BOOL JsonProfile::statusLineEnabled() {
	return getBool("terminal", "status_line", TRUE);
}

void JsonProfile::setStatusLineEnabled(BOOL enabled) {
	JsonProfile::setBool("terminal", "status_line", enabled);
}

CMString JsonProfile::characterSet() { 
	return getStringCached2(terminal, charset, CMString(TEXT("utf8")));	// was: ascii
}

void JsonProfile::setCharacterSet(CMString cset) {
	setStringCached2(terminal, charset, cset);
}

int JsonProfile::screenWidth() { 	// shouldn't be unsigned
	return getInteger("terminal", "width", 80);
}

void JsonProfile::setScreenWidth(int w) {
	setInteger("terminal", "width", w);
}

int JsonProfile::screenHeight() { 	// shouldn't be unsigned
	return getInteger("terminal", "height", 24);
}

void JsonProfile::setScreenHeight(int h) {
	setInteger("terminal", "width", h);
}

int JsonProfile::scrollbackLines() {  // shouldn't be unsigned
	return getInteger("terminal", "scrollback_lines", 512);
}

void JsonProfile::setScrollbackLines(int lines) {
	setInteger("terminal", "width", lines);
}

ConnectionProfile::Cursor JsonProfile::cursor() { 
	return (Cursor)getInteger("terminal", "cursor", (int)CUR_FULL);
}

void JsonProfile::setCursor(Cursor cur) {
	setInteger("terminal", "cursor", (int)cur);
}

// ----- Terminal Colors -----
ConnectionProfile::Color JsonProfile::terminalForegroundColor() { 
	return (Color)getInteger("terminal", "color", "fg", (int)COLOR_LIGHT_GRAY);
}

void JsonProfile::setTerminalForegroundColor(Color c) {
	setInteger("terminal", "color", "fg", (int)c);
}

ConnectionProfile::Color JsonProfile::terminalBackgroundColor() {  
	return (Color)getInteger("terminal", "color", "bg", (int)COLOR_BLUE);
}

void JsonProfile::setTerminalBackgroundColor(ConnectionProfile::Color c) {
	setInteger("terminal", "color", "bg", (int)c);
}

ConnectionProfile::Color JsonProfile::commandForegroundColor() {  
	return (Color)getInteger("terminal", "color", "cmd_fg", (int)COLOR_LIGHT_GRAY);
}

void JsonProfile::setCommandForegroundColor(ConnectionProfile::Color c) {
	setInteger("terminal", "color", "cmd_fg", (int)c);
}


ConnectionProfile::Color JsonProfile::commandBackgroundColor() {  
	return (Color)getInteger("terminal", "color", "cm_bg", (int)COLOR_BLACK);
}

void JsonProfile::setCommandBackgroundColor(ConnectionProfile::Color c) {
	setInteger("terminal", "color", "cmd_bg", (int)c);
}


ConnectionProfile::Color JsonProfile::statusLineForegroundColor() {  
	return (Color)getInteger("terminal", "color", "status_fg", (int)COLOR_LIGHT_GRAY);
}

void JsonProfile::setStatusLineForegroundColor(ConnectionProfile::Color c) {
	setInteger("terminal", "color", "status_fg", (int)c);
}

ConnectionProfile::Color JsonProfile::statusLineBackgroundColor() {  
	return (Color)getInteger("terminal", "color", "status_bg", (int)COLOR_CYAN);
}

void JsonProfile::setStatusLineBackgroundColor(ConnectionProfile::Color c) {
	setInteger("terminal", "color", "status_bg", (int)c);
}

ConnectionProfile::Color JsonProfile::mouseSelectionForegroundColor() { 
	return (Color)getInteger("terminal", "color", "sel_fg", (int)COLOR_BLACK);
}

void JsonProfile::setMouseSelectionForegroundColor(ConnectionProfile::Color c) {
	setInteger("terminal", "color", "sel_fg", (int)c);
}

ConnectionProfile::Color JsonProfile::mouseSelectionBackgroundColor() { 
	return (Color)getInteger("terminal", "color", "sel_bg", (int)COLOR_YELLOW);
}

void JsonProfile::setMouseSelectionBackgroundColor(ConnectionProfile::Color c) {
	setInteger("terminal", "color", "sel_bg", (int)c);
}


ConnectionProfile::Color JsonProfile::popupHelpForegroundColor() {  
	return (Color)getInteger("terminal", "color", "pop_fg", (int)COLOR_LIGHT_GRAY);
}

void JsonProfile::setPopupHelpForegroundColor(ConnectionProfile::Color c) {
	setInteger("terminal", "color", "pop_fg", (int)c);
}


ConnectionProfile::Color JsonProfile::popupHelpBackgroundColor() {  
	return (Color)getInteger("terminal", "color", "pop_bg", (int)COLOR_CYAN);
}

void JsonProfile::setPopupHelpBackgroundColor(ConnectionProfile::Color c) {
	setInteger("terminal", "color", "pop_bg", (int)c);
}


ConnectionProfile::Color JsonProfile::underlineSimulationForegroundColor() {  
	return (Color)getInteger("terminal", "color", "ul_fg", (int)COLOR_LIGHT_GRAY);
}

void JsonProfile::setUnderlineSimulationForegroundColor(ConnectionProfile::Color c) {
	setInteger("terminal", "color", "ul_fg", (int)c);
}


ConnectionProfile::Color JsonProfile::underlineSimulationBackgroundColor() {  
	return (Color)getInteger("terminal", "color", "ul_bg", (int)COLOR_RED);
}

void JsonProfile::setUnderlineSimulationBackgroundColor(ConnectionProfile::Color c) {
	setInteger("terminal", "color", "ul_bg", (int)c);
}


// ----- File Transfer -----
ConnectionProfile::FileTransferProtocol JsonProfile::fileTransferProtocol() {
	return (FileTransferProtocol)getInteger("xfer", "protocol", (int)FT_KERMIT);
};

void JsonProfile::setFileTransferProtocol(FileTransferProtocol proto) {
	setInteger("xfer", "protocol", (int)proto);
}

ConnectionProfile::KermitPerformance JsonProfile::kermitPerformance() {
	return (KermitPerformance)getInteger("xfer", "kermit", (int)KP_FAST);
}

void JsonProfile::setKermitPerformance(KermitPerformance kp) {
	setInteger("xfer", "kermit", (int)kp);
}

int JsonProfile::packetLength() {
	return getInteger("xfer", "packet_length", 4096);
}

void JsonProfile::setPacketLength(int len) {
	setInteger("xfer", "packet_length", len);
}

int JsonProfile::windowSize() {
	return getInteger("xfer", "window_size", 20);
}

void JsonProfile::setWindowSize(int size) {
	setInteger("xfer", "window_size", size);
}

ConnectionProfile::ControlCharUnprefixing JsonProfile::controlCharUnprefixing() {
	return (ControlCharUnprefixing)getInteger("xfer", "cc_unprefixing", (int)CCU_CAUTIOUS);
}

void JsonProfile::setControlCharUnprefixing(ControlCharUnprefixing ccu) {
	setInteger("xfer", "cc_unprefixing", (int)ccu);
}

BOOL JsonProfile::defaultToBinaryMode() { 
	return getBool("xfer", "default_binary", TRUE);
}

void JsonProfile::setDefaultToBinaryMode(BOOL enabled) {
	setBool("xfer", "default_binary", enabled);
}

CMString JsonProfile::fileCharacterSet() { 
	return getStringCached2(xfer, file_cset, CMString(TEXT("cp437")));
}

void JsonProfile::setFileCharacterSet(CMString cset) {
	setStringCached2(xfer, file_cset, cset);
}

CMString JsonProfile::transferCharacterSet() {
	return getStringCached2(xfer, cset, CMString(TEXT("latin1-iso")));
}

void JsonProfile::setTransferCharacterSet(CMString cset) {
	setStringCached2(xfer, cset, cset);
}

ConnectionProfile::FileNameCollision JsonProfile::fileNameCollisionAction() {
	return (FileNameCollision)getInteger("xfer", "collision", (int)FNC_BACKUP);
}

void JsonProfile::setFileNameCollisionAction(FileNameCollision fnc) {
	setInteger("xfer", "collision", (int)fnc);
}

ConnectionProfile::AutoDownload JsonProfile::autoDownloadMode() { 
	return (AutoDownload)getInteger("xfer", "autodownload", (int)AD_YES);
}

void JsonProfile::setAutoDownloadMode(AutoDownload ad) {
	setInteger("xfer", "autodownload", (int)ad);
}

BOOL JsonProfile::transmitLiteralFilenames() { 
	return getBool("xfer", "send_literal_pathnames", TRUE);
}

void JsonProfile::setTransmitLiteralFilenames(BOOL enabled) {
	setBool("xfer", "send_literal_pathnames", enabled);
}

BOOL JsonProfile::usePathnames() { 
	return getBool("xfer", "use_pathnames", FALSE);
}

void JsonProfile::setUsePathnames(BOOL enabled) {
	setBool("xfer", "use_pathnames", enabled);
}

BOOL JsonProfile::keepIncompleteFiles() { 
	return getBool("xfer", "keep_incomplete", TRUE);
}

void JsonProfile::setKeepIncomingFiles(BOOL enabled) {
	setBool("xfer", "keep_incomplete", enabled);
}

BOOL JsonProfile::negotiateStreamingTransferMode() { 
	return getBool("xfer", "neg_streaming", TRUE);
}

void JsonProfile::setNegotiateStreamingTransferMode(BOOL enabled) {
	setBool("xfer", "neg_streaming", enabled);
}

BOOL JsonProfile::negotiateClearChannelTransferMode() { 
	return getBool("xfer", "neg_clear_channel", TRUE);
}

void JsonProfile::setNegotiateClearChannelTransferMode(BOOL enabled) {
	setBool("xfer", "neg_clear_channel", enabled);
}

BOOL JsonProfile::force16bitCRC() { 
	return getBool("xfer", "force3", FALSE);
}

void JsonProfile::setForce16bitCRC(BOOL enabled) {
	setBool("xfer", "force3", enabled);
}

// ----- Serial -----
CMString JsonProfile::line() { 
	return getStringCached2(serial, line, CMString("COM1"));
}
void JsonProfile::setLine(CMString line) { 
	setStringCached2(serial, line, line);
}

unsigned int JsonProfile::lineSpeed() { 
	return getInteger("serial", "speed", 9600);	// TODO: Check
}

void JsonProfile::setLineSpeed(int speed) { 
	setInteger("serial", "speed", speed);
}

ConnectionProfile::FlowControl JsonProfile::flowControl() { 
	return (FlowControl)getInteger("serial", "flow_control", (int)FC_AUTO);
}

void JsonProfile::setFlowControl(FlowControl fc) {
	setInteger("serial", "flow_control", (int)fc);
}

ConnectionProfile::Parity JsonProfile::parity() { 
	return (Parity)getInteger("serial", "parity", (int)PAR_NONE);
}

void JsonProfile::setParity(Parity p) {
	setInteger("serial", "parity", (int)p);
}

ConnectionProfile::StopBits JsonProfile::stopBits() { 
	return (StopBits)getInteger("serial", "stop_bits", (int)SB_1_0);
}

void JsonProfile::setStopBits(StopBits sb) {
	setInteger("serial", "stop_bits", (int)sb);
}

BOOL JsonProfile::errorCorrection() { 
	return getBool("serial", "error_correction", TRUE);	// TODO: Check
}

void JsonProfile::setErrorCorrection(BOOL enabled) {
	setBool("serial", "error_correction", enabled);
}

BOOL JsonProfile::dataCompression() { 
	return getBool("serial", "compression", TRUE);  	// TODO: Check
}

void JsonProfile::setDataCompression(BOOL enabled) {
	setBool("serial", "compression", enabled);
}

BOOL JsonProfile::carrierDetection() { 
	return getBool("serial", "carrier_detection", TRUE);	// TODO: Check
}

void JsonProfile::setCarrierDetection(BOOL enabled) {
	setBool("serial", "carrier_detection", enabled);
}

// ----- Telnet -----
ConnectionProfile::NegotiateOption JsonProfile::telnetAuthenticationOption() { 
	return (NegotiateOption)getInteger("telnet", "options", "authentication", (int)NEG_ACCEPT);
}

void JsonProfile::setTelnetAuthenticationOption(NegotiateOption opt) {
	setInteger("telnet", "options", "authentication", (int)opt);
}

ConnectionProfile::NegotiateOption JsonProfile::telnetBinaryModeOption() { 
	return (NegotiateOption)getInteger("telnet", "options", "binary", (int)NEG_ACCEPT);
}

void JsonProfile::setTelnetBinaryModeOption(NegotiateOption opt) {
	setInteger("telnet", "options", "binary", (int)opt);
}

ConnectionProfile::NegotiateOption JsonProfile::telnetEncryptionOption() { 
	return (NegotiateOption)getInteger("telnet", "options", "encryption", (int)NEG_ACCEPT);
}

void JsonProfile::setTelnetEncryptionOption(NegotiateOption opt) {
	setInteger("telnet", "options", "encryptions", (int)opt);
}

ConnectionProfile::NegotiateOption JsonProfile::telnetKermitOption() { 
	return (NegotiateOption)getInteger("telnet", "options", "kermit", (int)NEG_ACCEPT);
}

void JsonProfile::setTelnetKermitOption(NegotiateOption opt) {
	setInteger("telnet", "options", "kermit", (int)opt);
}

ConnectionProfile::NegotiateOption JsonProfile::telnetStartTLSOption() { 
	return (NegotiateOption)getInteger("telnet", "options", "start_tls", (int)NEG_ACCEPT);
}

void JsonProfile::setTelnetStartTLSOption(NegotiateOption opt) {
	setInteger("telnet", "options", "start_tls", (int)opt);
}

ConnectionProfile::NegotiateOption JsonProfile::telnetForwardXOption() { 
	return (NegotiateOption)getInteger("telnet", "options", "forward_x", (int)NEG_ACCEPT);
}

void JsonProfile::setTelnetForwardXOption(NegotiateOption opt) {
	setInteger("telnet", "options", "forward_x", (int)opt);
}

ConnectionProfile::NegotiateOption JsonProfile::telnetComPortControlOption() { 
	return (NegotiateOption)getInteger("telnet", "options", "com_port_control", (int)NEG_ACCEPT);
}

void JsonProfile::setTelnetComPortControlOption(NegotiateOption opt) {
	setInteger("telnet", "options", "com_port_control", (int)opt);
}

CMString JsonProfile::telnetEnvarAccount() { 
	return getStringCached3(telnet, envar, account, CMString());
}

void JsonProfile::setTelnetEnvarAccount(CMString val) {
	setStringCached3(telnet, envar, account, val);
}

CMString JsonProfile::telnetEnvarDisplay() { 
	return getStringCached3(telnet, envar, display, CMString());
}

void JsonProfile::setTelnetEnvarDisplay(CMString val) {
	setStringCached3(telnet, envar, display, val);
}

CMString JsonProfile::telnetEnvarJob() { 
	return getStringCached3(telnet, envar, job, CMString());
}

void JsonProfile::setTelnetEnvarJob(CMString val) {
	setStringCached3(telnet, envar, job, val);
}

CMString JsonProfile::telnetEnvarLocation() { 
	return getStringCached3(telnet, envar, location, CMString());
}

void JsonProfile::setTelnetEnvarLocation(CMString val) {
	setStringCached3(telnet, envar, location, val);
}

CMString JsonProfile::telnetEnvarPrinter() { 
	return getStringCached3(telnet, envar, printer, CMString());
}

void JsonProfile::setTelnetEnvarPrinter(CMString val) {
	setStringCached3(telnet, envar, printer, val);
}

CMString JsonProfile::telnetEnvarTerminalType() { 
	return getStringCached3(telnet, envar, terminal, CMString());
}

void JsonProfile::setTelnetEnvarTerminalType(CMString val) {
	setStringCached3(telnet, envar, terminal, val);
}

BOOL JsonProfile::telnetForwardCredentials() { 
	return getBool("telnet", "forward_creds", FALSE);
}

void JsonProfile::setTelnetForwardCredentials(BOOL enabled) {
	setBool("telnet", "forward_creds", enabled);
}

BOOL JsonProfile::telnetNegotiationsMustComplete() { 
	return getBool("telnet", "nego_must_finish", TRUE);
}

void JsonProfile::setTelnetNegotiationsMustComplete(BOOL enabled) {
	setBool("telnet", "nego_must_finish", enabled);
}

BOOL JsonProfile::telnetDebugMode() { 
	return getBool("telnet", "debug", FALSE);
}

void JsonProfile::setTelnetDebugMode(BOOL enabled) {
	setBool("telnet", "debug", enabled);
}

BOOL JsonProfile::telnetSubnegotiationDelay() { 
	return getBool("telnet", "subnego_delay", TRUE);
}

void JsonProfile::setTelnetSubnegotiationDelay(BOOL enabled) {
	setBool("telnet", "subnego_delay", enabled);
}

// ----- SSH -----
BOOL JsonProfile::sshCompressionEnabled() { 
	return getBool("ssh", "compression", FALSE);
}

void JsonProfile::setSshCompressionEnabled(BOOL enabled) {
	setBool("ssh", "compression", enabled);
}

BOOL JsonProfile::sshX11ForwardingEnabled() { 
	return getBool("ssh", "forward_x", FALSE);
}

void JsonProfile::setSshX11ForwardingEnabled(BOOL enabled) {
	setBool("ssh", "forward_x", enabled);
}

ConnectionProfile::SshHostKeyCheck JsonProfile::sshHostKeyChecking() { 
	return (SshHostKeyCheck)getInteger("ssh", "host_key_check", (int)SHKC_ASK);
}

void JsonProfile::setSshHostKeyChecking(SshHostKeyCheck shkc) {
	setInteger("ssh", "host_key_check", (int)shkc);
}

CMString JsonProfile::sshIdentityFile() { 
	return getStringCached2(ssh, ident_file, CMString());
}

void JsonProfile::setSshIdentityFile(CMString file) {
	setStringCached2(ssh, ident_file, file);
}

CMString JsonProfile::sshUserKnownHostsFile() { 
	return getStringCached2(ssh, known_hosts_file, CMString());
}

void JsonProfile::setSshUserKnownHostsFile(CMString file) {
	getStringCached2(ssh, known_hosts_file, file);
}
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
CMString JsonProfile::ftpAuthType() { // TODO: this needs to be bit flags?
	return NULL;
}

void JsonProfile::setFtpAuthType(CMString type) {

}

ConnectionProfile::FTPProtectionLevel JsonProfile::ftpCommandProtectionLevel() { 
	return (FTPProtectionLevel)getInteger("ftp", "protection", "command", (int)FTPPL_PRIVATE);
}

void JsonProfile::setFtpCommandProtectionLevel(FTPProtectionLevel pl) {
	setInteger("ftp", "protection", "command", (int)pl);
}

ConnectionProfile::FTPProtectionLevel JsonProfile::ftpDataProtectionLevel() { 
	return (FTPProtectionLevel)getInteger("ftp", "protection", "data", (int)FTPPL_PRIVATE);
}

void JsonProfile::setFtpDataProtectionLevel(FTPProtectionLevel pl) {
	setInteger("ftp", "protection", "data", (int)pl);
}

CMString JsonProfile::ftpServerCharset() { 
	return getStringCached2(ftp, charset, CMString(TEXT("ascii")));
}

void JsonProfile::setFtpServerCharset(CMString cset) {
	setStringCached2(ftp, charset, cset);
}

BOOL JsonProfile::ftpAutoLogin() { 
	return getBool("ftp", "autologin", TRUE);
}

void JsonProfile::setFtpAutoLogin(BOOL enabled) {
	setBool("ftp", "autologin", enabled);
}

BOOL JsonProfile::ftpAutoAuthentication() { 
	return getBool("ftp", "auto_auth", TRUE);
}

void JsonProfile::setFtpAutoAuthentication(BOOL enabled) {
	setBool("ftp", "auto_auth", enabled);
}

BOOL JsonProfile::ftpAutoEncryption() { 
	return getBool("ftp", "auto_enc", TRUE);
}

void JsonProfile::setFtpAutoEncryption(BOOL enabled) {
	setBool("ftp", "auto_enc", enabled);
}

BOOL JsonProfile::ftpPassiveMode() { 
	return getBool("ftp", "passive", TRUE);
}

void JsonProfile::setFtpPassiveMode(BOOL enabled) {
	setBool("ftp", "passive", enabled);
}

BOOL JsonProfile::ftpCredentialForwarding() { 
	return getBool("ftp", "cred_forwarding", FALSE);
}

void JsonProfile::setFtpCredentialForwarding(BOOL enabled) {
	setBool("ftp", "cred_forwarding", enabled);
}

BOOL JsonProfile::ftpDates() { 
	return getBool("ftp", "dates", TRUE);
}

void JsonProfile::setFtpDates(BOOL enabled) {
	setBool("ftp", "dates", enabled);
}

BOOL JsonProfile::ftpSendLiteralFilenames() { 
	return getBool("ftp", "send_literal_filenames", TRUE);
}

void JsonProfile::setFtpSendLiteralFilenames(BOOL enabled) {
	setBool("ftp", "send_literal_filenames", enabled);
}

BOOL JsonProfile::ftpVerboseMessages() { 
	return getBool("ftp", "verbose_msgs", TRUE);
}

void JsonProfile::setFtpVerboseMessages(BOOL enabled) {
	setBool("ftp", "verbose_msgs", enabled);
}

BOOL JsonProfile::ftpDebugMessages() { 
	return getBool("ftp", "debug_msgs", FALSE);
}

void JsonProfile::setFtpDebugMessages(BOOL enabled) {
	setBool("ftp", "debug_msgs", enabled);
}

BOOL JsonProfile::ftpCharacterSetTranslation() { 
	return getBool("ftp", "cset_translation", FALSE);
}

void JsonProfile::setFtpCharacterSetTranslation(BOOL enabled) { 
	setBool("ftp", "cset_translation", enabled);
}

// ----- TCP/IP -----
int JsonProfile::tcpSendBufferSize() { 
	return getInteger("ip", "buffer_size", "send", 8192);
}

void JsonProfile::setTcpSendBufferSize(int size) {
	setInteger("ip", "buffer_size", "send", size);
}

int JsonProfile::tcpReceiveBufferSize() { 
	return getInteger("ip", "buffer_size", "recv", 8192);
}

void JsonProfile::setTcpReceiveBufferSize(int size) {
	setInteger("ip", "buffer_size", "recv", size);
}

ConnectionProfile::ReverseDNSOption JsonProfile::tcpReverseDNSLookup() { 
	return (ReverseDNSOption)getInteger("ip", "reverse_dns", (int)RDO_AUTO);
}

void JsonProfile::setTcpReverseDNSLookup(ReverseDNSOption opt) {
	setInteger("ip", "reverse_dns", (int)opt);
}

BOOL JsonProfile::tcpDNSServiceLookup() { 
	return getBool("ip", "dns_svc_lookup", FALSE);
}

void JsonProfile::setTcpDNSServiceLookup(BOOL enabled) {
	setBool("ip", "dns_svc_lookup", enabled);
}

CMString JsonProfile::tcpSocksHostname() { 
	return getStringCached3(ip, socks, host, CMString());
}

void JsonProfile::setTcpSocksHostname(CMString host) {
	setStringCached3(ip, socks, host, host);
}

int JsonProfile::tcpSocksPort() { 
	return getInteger("ip", "socks", "port", 0);
}	

void JsonProfile::setTcpSocksPort(int port) {
	setInteger("ip", "socks", "port", port);
}

CMString JsonProfile::tcpSocksUser() { 
	return getStringCached3(ip, socks, user, CMString());
}

void JsonProfile::setTcpSocksUser(CMString user) {
	setStringCached3(ip, socks, user, user);
}

CMString JsonProfile::tcpSocksPassword() { 
	return getStringCached3(ip, socks, pass, CMString());
}

void JsonProfile::setTcpSocksPassword(CMString pass) {
	setStringCached3(ip, socks, pass, pass);
}

CMString JsonProfile::tcpHttpHostname() { 
	return getStringCached3(ip, http, host, CMString());
}

void JsonProfile::setTcpHttpHostname(CMString host) {
	setStringCached3(ip, http, host, host);
}

int JsonProfile::tcpHttpPort() { 
	return getInteger("ip", "http", "port", 3128);		// was: nothing
}

void JsonProfile::setTcpHttpPort(int port) {
	setInteger("ip", "http", "port", port);
}

CMString JsonProfile::tcpHttpUser() { 
	return getStringCached3(ip, http, user, CMString());
}

void JsonProfile::setTcpHttpUser(CMString user) {
	setStringCached3(ip, http, user, user);
}

CMString JsonProfile::tcpHttpPassword() { 
	return getStringCached3(ip, http, pass, CMString());
}

void JsonProfile::setTcpHttpPassword(CMString pass) {
	setStringCached3(ip, http, pass, pass);
}


// ----- Kerberos -----
// Someday

// ----- TLS -----
CMString JsonProfile::tlsCipherList() { 
	return getStringCached2(tls, cipher_list, CMString());
}

void JsonProfile::setTlsCipherList(CMString file) {
	setStringCached2(tls, cipher_list, file);
}

CMString JsonProfile::tlsClientCertificateFile() { 
	return getStringCached2(tls, cert_file, CMString());
}

void JsonProfile::setTlsClientCertificateFile(CMString file) {
	setStringCached2(tls, cert_file, file);
}

CMString JsonProfile::tlsClientPrivateKeyFile() { 
	return getStringCached2(tls, pk_file, CMString());
}

void JsonProfile::setTlsClientPrivateKeyFile(CMString file) {
	setStringCached2(tls, pk_file, file);
}

CMString JsonProfile::tlsCAVerificationFile() { 
	return getStringCached2(tls, ca_verification_file, CMString());
}

void JsonProfile::setTlsCAVerificationFile(CMString file) {
	setStringCached2(tls, ca_verification_file, file);
}

CMString JsonProfile::tlsCAVerificationDirectory() { 
	return getStringCached2(tls, ca_verification_dir, CMString());
}

void JsonProfile::setTlsCAVerificationDirectory(CMString dir) {
	setStringCached2(tls, ca_verification_dir, dir);
}

CMString JsonProfile::tlsCRLFile() { 
	return getStringCached2(tls, crl_file, CMString());
}

void JsonProfile::setTlsCRLFile(CMString file) {
	setStringCached2(tls, crl_file, file);
}

CMString JsonProfile::tlsCRLDirectory() { 
	return getStringCached2(tls, crl_dir, CMString());
}

void JsonProfile::setTlsCRLDirectory(CMString dir) {
	setStringCached2(tls, crl_dir, dir);
}

ConnectionProfile::CertificateVerifyMode JsonProfile::tlsCertificateVerifyMode() { 
	return (CertificateVerifyMode)getInteger("tls", "verify_mode", 
		(int)CVM_VERIFY_HOSTS_IF_PRESENTED);
}

void JsonProfile::setTlsCertificateVerifyMode(CertificateVerifyMode mode) {
	setInteger("tls", "verify_mode", (int)mode);
}

BOOL JsonProfile::tlsVerboseMode() { 
	return getBool("tls", "verbose_mode", FALSE);
}

void JsonProfile::setTlsVerboseMode(BOOL enabled) {
	setBool("tls", "verbose_mode", enabled);
}

BOOL JsonProfile::tlsDebugMode() { 
	return getBool("tls", "debug_mode", FALSE);
}

void JsonProfile::setTlsDebugMode(BOOL enabled) {
	setBool("tls", "debug_mode", enabled);
}


// ----- Keyboard -----
ConnectionProfile::KeyboardBackspaceMode JsonProfile::keyboardBackspaceSends() { 
	return (KeyboardBackspaceMode)getInteger("keyboard", "backspace", (int)KBM_DELETE); 
}

void JsonProfile::setKeyboardBackspaceSends(KeyboardBackspaceMode kbm) {
	setInteger("keyboard", "backspace", (int)kbm);
}

ConnectionProfile::KeyboardEnterMode JsonProfile::keyboardEnterSends() { 
	return (KeyboardEnterMode)getInteger("keyboard", "enter", (int)KEM_CR);
}

void JsonProfile::setKeyboardEnterSends(KeyboardEnterMode kem) {
	setInteger("keyboard", "enter", (int)kem);
}

BOOL JsonProfile::mouseEnabled() { 
	return getBool("mouse", "enabled", TRUE);
}

void JsonProfile::setMouseEnabled(BOOL enabled) {
	setBool("mouse", "enabled", enabled);
}

BOOL JsonProfile::defaultKeyMap() { 
	return getBool("keyboard", "use_default", TRUE);
}

void JsonProfile::setDefaultKeyMap(BOOL enabled) {
	setBool("keyboard", "use_default", enabled);
}

CMString JsonProfile::keymapFile() { 
	return getStringCached2(keyboard, map_file, CMString());
}

void JsonProfile::setKeymapFile(CMString file) {
	setStringCached2(keyboard, map_file, file);
}

CMString JsonProfile::additionalKeyMaps() { 
	return getStringCached2(keyboard, additional_keymaps, CMString());
}

void JsonProfile::setAdditionalKeyMaps(CMString str) {
	setStringCached2(keyboard, additional_keymaps, str);
}


// ----- Login -----
CMString JsonProfile::userId() { 
	return getStringCached2(login, user, CMString()); 
}

void JsonProfile::setUserId(CMString uid) {
	setStringCached2(login, user, uid);
}

CMString JsonProfile::password() { 
	return getStringCached2(login, pass, CMString());
}

void JsonProfile::setPassword(CMString pass) {
	setStringCached2(login, pass, pass);
}

BOOL JsonProfile::promptForPassword() { 
	return getBool("login", "prompt_for_pass", FALSE);
}

void JsonProfile::setPromptForPassword(BOOL prompt) {
	setBool("login", "prompt_for_pass", prompt);
}

CMString JsonProfile::passwordPrompt() { 
	return getStringCached2(login, pass_prompt, CMString()); 
}

void JsonProfile::setPasswordPrompt(CMString prompt) {
	setStringCached2(login, pass_prompt, prompt);
}

BOOL JsonProfile::runLoginScriptFile() { 
	return getBool("login", "run_file"); 
}

void JsonProfile::setRunLoginScriptFile(BOOL run) {
	setBool("login", "run_file", run);
}

CMString JsonProfile::loginScriptFile() { 
	return getStringCached2(login, script_file, CMString()); 
}

void JsonProfile::setLoginScriptFile(CMString file) {
	setStringCached2(login, script_file, file);
}

CMString JsonProfile::loginScript() { 
	return getStringCached2(login, script, CMString()); 
}

void JsonProfile::setLoginScript(CMString script) {
	setStringCached2(login, script, script);
}


// ----- Printer -----
ConnectionProfile::PrinterType JsonProfile::printerType() { 
	return (PrinterType)getInteger("printer", "type", (int)PT_NONE); // was: windows print queue
}

void JsonProfile::setPrinterType(PrinterType pt) {
	setInteger("printer", "type", (int)pt);
}

// aka aka file aka command
CMString JsonProfile::deviceName() { 
	return getStringCached2(printer, device_name, CMString(TEXT("LPT1")));
}

void JsonProfile::setDeviceName(CMString device) {
	setStringCached2(printer, device_name, device);
}

CMString JsonProfile::windowsPrintQueue() { 
	return getStringCached2(printer, queue, CMString(DEFAULT_WIN_PRINT_QUEUE));
}

void JsonProfile::setWindowsPrintQueue(CMString queue) {
	setStringCached2(printer, queue, queue);
}

CMString JsonProfile::headerFile() { 
	return getStringCached2(printer, header_file, CMString()); 
}

void JsonProfile::setHeaderFile(CMString file) {
	setStringCached2(printer, header_file, file);
}

BOOL JsonProfile::sendEndOfJobString() { 
	return getBool("printer", "send_end_string", FALSE); 
}

void JsonProfile::setSendEndOfJobString(BOOL enabled) {
	setBool("printer", "send_end_string", enabled);
}

CMString JsonProfile::endOfJobString() { 
	return getStringCached2(printer, end_string, CMString()); 
}

void JsonProfile::setEndOfJobString(CMString string) {
	setStringCached2(printer, end_string, string);
}

CMString JsonProfile::printCharacterSet() { 
	return getStringCached2(printer, cset, CMString(TEXT("cp437")));
}

void JsonProfile::setPrintCharacterSet(CMString cset) {
	setStringCached2(printer, cset, cset);
}

BOOL JsonProfile::printAsPostScript() { 
	return getBool("printer", "as_ps", FALSE); 
}

void JsonProfile::setPrintAsPostScript(BOOL enabled) {
	setBool("printer", "as_ps", enabled);
}

int JsonProfile::printWidth() { 
	return getInteger("printer", "ps_width", 80);
}

void JsonProfile::setPrintWidth(int w) {
	setInteger("printer", "ps_width", w);
}

int JsonProfile::printHeight() { 
	return getInteger("printer", "ps_height", 66);
}

void JsonProfile::setPrintHeight(int h) {
	setInteger("printer", "ps_height", h);
}

int JsonProfile::printSpeed() { 
	return getInteger("printer", "speed", 9600);
}

void JsonProfile::setPrintSpeed(int speed) {
	setInteger("printer", "speed", speed);
}

ConnectionProfile::FlowControl JsonProfile::printFlowControl() { 
	return (FlowControl)getInteger("printer", "flow_control", (int)FC_NONE); 
}

void JsonProfile::setPrintFlowControl(FlowControl fc) {
	setInteger("printer", "flow_control", (int)fc);
}

ConnectionProfile::Parity JsonProfile::printParity() { 
	return (Parity)getInteger("printer", "parity", (int)PAR_NONE); 
}

void JsonProfile::setPrintParity(Parity p) {
	setInteger("printer", "parity", (int)p);
}

BOOL JsonProfile::bidirectionalPrinting() { 
	return getBool("printer", "bidirectional", FALSE); 
}

void JsonProfile::setBidirectionalPrinting(BOOL enabled) {
	setBool("printer", "bidirectoinal", enabled);
}


// TODO: missing from the UI
int JsonProfile::printTimeoutSeconds() { 
	return getInteger("printer", "timeouts", 0);  
}

void JsonProfile::setPrintTimeoutSeconds(int seconds) {
	setInteger("printer", "timeouts", seconds);
}


// ----- GUI -----
BOOL JsonProfile::useGUIKermit() { 
	return getBool("gui", "enabled", TRUE);;
}

void JsonProfile::setUseGUIKermit(BOOL enabled) {
	setBool("gui", "enabled", enabled);
}

CMString JsonProfile::fontName() { 
	return getStringCached3(gui, font, name, CMString(TEXT("Courier New")));
}

void JsonProfile::setFontName(CMString font) {
	setStringCached3(gui, font, name, font);
}

int JsonProfile::fontSize() { 
	return getInteger("gui", "font", "size", 12);
}

void JsonProfile::setFontSize(int size) {
	setInteger("gui", "font", "size", size);
}

BOOL JsonProfile::customStartupPosition() { 
	return getBool("gui", "pos", "enabled", FALSE); 
}

void JsonProfile::setCustomStartupPosition(BOOL enabled) {
	setBool("gui", "pos", "enabled", enabled);
}

int JsonProfile::startupAtX() { 
	return getInteger("gui", "pos", "x", 0); 
}

void JsonProfile::setStartupAtX(int x) {
	setInteger("gui", "pos", "x", x);
}

int JsonProfile::startupAtY() { 
	return getInteger("gui", "pos", "y", 0); 
}

void JsonProfile::setStartupAtY(int y) {
	setInteger("gui", "pos", "y", y);
}

BOOL JsonProfile::resizeChangesDimensions() { 
	return getBool("gui", "resize_dimensions", TRUE);
}

void JsonProfile::setResizeChangesDimensions(BOOL enabled) {
	setBool("gui", "reisze_dimensions", enabled);
}

ConnectionProfile::WindowStartupMode JsonProfile::windowStartupMode() { 
	return (WindowStartupMode)getInteger("gui", "window_mode", (int)WSM_NORMAL);
}

void JsonProfile::setWindowStartupMode(WindowStartupMode wsm) {
	setInteger("gui", "window_mode", (int)wsm);
}

BOOL JsonProfile::menubarEnabled() { 
	return getBool("gui", "menubar", TRUE); 
}

void JsonProfile::setMenubarEnabled(BOOL enabled) {
	setBool("gui", "menubar", enabled);
}

BOOL JsonProfile::toolbarEnabled() { 
	return getBool("gui", "toolbar", TRUE); 
}

void JsonProfile::setToolbarEnabled(BOOL enabled) {
	setBool("gui", "toolbar", enabled);
}

BOOL JsonProfile::statusbarEnabled() { 
	return getBool("gui", "statusbar", TRUE); 
}

void JsonProfile::setStatusbarEnabled(BOOL enabled) {
	setBool("gui", "statusbar", enabled);
}

BOOL JsonProfile::dialogsEnabled() { 
	return getBool("gui", "dialogs", TRUE); 
}

void JsonProfile::setDialogsEnabled(BOOL enabled) {
	setBool("gui", "dialogs", enabled);
}

// ----- GUI Colors -----
RGB8BitColor JsonProfile::rgbBlack() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_black", "r", 0), 
			getInteger("gui", "color_black", "g", 0), 
			getInteger("gui", "color_black", "b", 0) 
	}; 
	return x;
}

void JsonProfile::setRgbBlack(RGB8BitColor c) {
	setInteger("gui", "color_black", "r", c.r);
	setInteger("gui", "color_black", "g", c.g);
	setInteger("gui", "color_black", "b", c.b);
}

RGB8BitColor JsonProfile::rgbBlue() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_blue", "r", 0), 
			getInteger("gui", "color_blue", "g", 0), 
			getInteger("gui", "color_blue", "b", 128) 
	}; 
	return x;
}

void JsonProfile::setRgbBlue(RGB8BitColor c) {
	setInteger("gui", "color_blue", "r", c.r);
	setInteger("gui", "color_blue", "g", c.g);
	setInteger("gui", "color_blue", "b", c.b);
}

RGB8BitColor JsonProfile::rgbGreen() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_green", "r", 0), 
			getInteger("gui", "color_green", "g", 128), 
			getInteger("gui", "color_green", "b", 0) 
	}; 
	return x;
}

void JsonProfile::setRgbGreen(RGB8BitColor c) {
	setInteger("gui", "color_green", "r", c.r);
	setInteger("gui", "color_green", "g", c.g);
	setInteger("gui", "color_green", "b", c.b);
}

RGB8BitColor JsonProfile::rgbCyan() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_cyan", "r", 0), 
			getInteger("gui", "color_cyan", "g", 128), 
			getInteger("gui", "color_cyan", "b", 128) 
	}; 
	return x;
}

void JsonProfile::setRgbCyan(RGB8BitColor c) {
	setInteger("gui", "color_cyan", "r", c.r);
	setInteger("gui", "color_cyan", "g", c.g);
	setInteger("gui", "color_cyan", "b", c.b);
}

RGB8BitColor JsonProfile::rgbRed() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_red", "r", 127), 
			getInteger("gui", "color_red", "g", 0), 
			getInteger("gui", "color_red", "b", 0) 
	}; 
	return x;
}

void JsonProfile::setRgbRed(RGB8BitColor c) {
	setInteger("gui", "color_red", "r", c.r);
	setInteger("gui", "color_red", "g", c.g);
	setInteger("gui", "color_red", "b", c.b);
}

RGB8BitColor JsonProfile::rgbMagenta() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_magenta", "r", 128), 
			getInteger("gui", "color_magenta", "g", 0), 
			getInteger("gui", "color_magenta", "b", 128) 
	}; 
	return x;
}

void JsonProfile::setRgbMagenta(RGB8BitColor c) {
	setInteger("gui", "color_magenta", "r", c.r);
	setInteger("gui", "color_magenta", "g", c.g);
	setInteger("gui", "color_magenta", "b", c.b);
}

RGB8BitColor JsonProfile::rgbBrown() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_brown", "r", 128), 
			getInteger("gui", "color_brown", "g", 0), 
			getInteger("gui", "color_brown", "b", 128) 
	}; 
	return x;
}

void JsonProfile::setRgbBrown(RGB8BitColor c) {
	setInteger("gui", "color_brown", "r", c.r);
	setInteger("gui", "color_brown", "g", c.g);
	setInteger("gui", "color_brown", "b", c.b);
}

RGB8BitColor JsonProfile::rgbLightGray() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_lgray", "r", 192), 
			getInteger("gui", "color_lgray", "g", 192), 
			getInteger("gui", "color_lgray", "b", 192) 
	}; 
	return x;
}

void JsonProfile::setRgbLightGray(RGB8BitColor c) {
	setInteger("gui", "color_lgray", "r", c.r);
	setInteger("gui", "color_lgray", "g", c.g);
	setInteger("gui", "color_lgray", "b", c.b);
}

RGB8BitColor JsonProfile::rgbDarkGray() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_dgray", "r", 128), 
			getInteger("gui", "color_dgray", "g", 128), 
			getInteger("gui", "color_dgray", "b", 128) 
	}; 
	return x;
}

void JsonProfile::setRgbDarkGray(RGB8BitColor c) {
	setInteger("gui", "color_dgray", "r", c.r);
	setInteger("gui", "color_dgray", "g", c.g);
	setInteger("gui", "color_dgray", "b", c.b);
}

RGB8BitColor JsonProfile::rgbLightBlue() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_lblue", "r", 0), 
			getInteger("gui", "color_lblue", "g", 0), 
			getInteger("gui", "color_lblue", "b", 255) 
	}; 
	return x;
}

void JsonProfile::setRgbLightBlue(RGB8BitColor c) {
	setInteger("gui", "color_lblue", "r", c.r);
	setInteger("gui", "color_lblue", "g", c.g);
	setInteger("gui", "color_lblue", "b", c.b);
}

RGB8BitColor JsonProfile::rgbLightGreen() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_lgreen", "r", 0), 
			getInteger("gui", "color_lgreen", "g", 255), 
			getInteger("gui", "color_lgreen", "b", 0) 
	}; 
	return x;
}

void JsonProfile::setRgbLightGreen(RGB8BitColor c) {
	setInteger("gui", "color_lgreen", "r", c.r);
	setInteger("gui", "color_lgreen", "g", c.g);
	setInteger("gui", "color_lgreen", "b", c.b);
}

RGB8BitColor JsonProfile::rgbLightCyan() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_lcyan", "r", 0), 
			getInteger("gui", "color_lcyan", "g", 255), 
			getInteger("gui", "color_lcyan", "b", 255) 
	}; 
	return x;
}

void JsonProfile::setRgbLightCyan(RGB8BitColor c) {
	setInteger("gui", "color_lcyan", "r", c.r);
	setInteger("gui", "color_lcyan", "g", c.g);
	setInteger("gui", "color_lcyan", "b", c.b);
}

RGB8BitColor JsonProfile::rgbLightRed() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_lred", "r", 255), 
			getInteger("gui", "color_lred", "g", 0), 
			getInteger("gui", "color_lred", "b", 0) 
	}; 
	return x;
}

void JsonProfile::setRgbLightRed(RGB8BitColor c) {
	setInteger("gui", "color_lred", "r", c.r);
	setInteger("gui", "color_lred", "g", c.g);
	setInteger("gui", "color_lred", "b", c.b);
}

RGB8BitColor JsonProfile::rgbLightMagenta() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_lmagenta", "r", 255), 
			getInteger("gui", "color_lmagenta", "g", 0), 
			getInteger("gui", "color_lmagenta", "b", 255) 
	}; 
	return x;
}

void JsonProfile::setRgbLightMagenta(RGB8BitColor c) {
	setInteger("gui", "color_lmagenta", "r", c.r);
	setInteger("gui", "color_lmagenta", "g", c.g);
	setInteger("gui", "color_lmagenta", "b", c.b);
}

RGB8BitColor JsonProfile::rgbYellow() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_yellow", "r", 255), 
			getInteger("gui", "color_yellow", "g", 255), 
			getInteger("gui", "color_yellow", "b", 0) 
	}; 
	return x;
}

void JsonProfile::setRgbYellow(RGB8BitColor c) {
	setInteger("gui", "color_yellow", "r", c.r);
	setInteger("gui", "color_yellow", "g", c.g);
	setInteger("gui", "color_yellow", "b", c.b);
}

RGB8BitColor JsonProfile::rgbWhite() { 
	RGB8BitColor x = { 
			getInteger("gui", "color_white", "r", 255), 
			getInteger("gui", "color_white", "g", 255), 
			getInteger("gui", "color_white", "b", 255) 
	}; 
	return x;
}

void JsonProfile::setRgbWhite(RGB8BitColor c) {
	setInteger("gui", "color_white", "r", c.r);
	setInteger("gui", "color_white", "g", c.g);
	setInteger("gui", "color_white", "b", c.b);
}

// ----- Logging -----
BOOL JsonProfile::logConnections() { 
	return getBool("log", "connections", "enabled", FALSE);
	return FALSE; // TODO
}

void JsonProfile::setLogConnections(BOOL enabled) {
	setBool("log", "connections", "enabled", enabled);
}

BOOL JsonProfile::logAppendConnections() { 
	return getBool("log", "connections", "append", FALSE);
}

void JsonProfile::setLogAppendConnections(BOOL enabled) {
	setBool("log", "connections", "append", enabled);
}

CMString JsonProfile::logConnectionsFile() { 
	return getStringCached3(log, connections, file, CMString(TEXT("connection.log")));
}

void JsonProfile::setLogConnectionsFile(CMString file) {
	setStringCached3(log, connections, file, file);
}

BOOL JsonProfile::logDebug() { 
	return getBool("log", "debug", "enabled", FALSE);
}

void JsonProfile::setLogDebug(BOOL enabled) {
	setBool("log", "debug", "enabled", enabled);
}

BOOL JsonProfile::logAppendDebug() { 
	return getBool("log", "debug", "append", FALSE);
}

void JsonProfile::setLogAppendDebug(BOOL enabled) {
	setBool("log", "debug", "append", enabled);
}

CMString JsonProfile::logDebugFile() { 
	return getStringCached3(log, debug, file, CMString(TEXT("debug.log")));
}

void JsonProfile::setLogDebugFile(CMString file) {
	setStringCached3(log, debug, file, file);
}


BOOL JsonProfile::logKermitFileTransfers() { 
	return getBool("log", "transfers", "enabled", FALSE);
}

void JsonProfile::setLogKermitFileTransfers(BOOL enabled) {
	setBool("log", "transfers", "enabled", enabled);
}

BOOL JsonProfile::logAppendKermitFileTransfers() { 
	return getBool("log", "transfers", "append", FALSE);
}

void JsonProfile::setLogAppendKermitFileTransfers(BOOL enabled) {
	setBool("log", "transfers", "append", enabled);
}

CMString JsonProfile::logKermitFileTransfersFile() { 
	return getStringCached3(log, transfers, file, CMString(TEXT("packet.log")));
}

void JsonProfile::setLogKermitFileTransfersFile(CMString file) {
	setStringCached3(log, transfers, file, file);
}

BOOL JsonProfile::logSessionInput() { 
	return getBool("log", "session", "enabled", FALSE);
}

void JsonProfile::setLogSessionInput(BOOL enabled) {
	setBool("log", "session", "enabled", enabled);
}

BOOL JsonProfile::logAppendSessionInput() { 
	return getBool("log", "session", "append", FALSE);
}

void JsonProfile::setLogAppendSessionInput(BOOL enabled) {
	setBool("log", "session", "append", enabled);
}

CMString JsonProfile::logSessionInputFile() { 
	return getStringCached3(log, session, file, CMString(TEXT("sesson.log")));
}

void JsonProfile::setLogSessionInputFile(CMString file) {
	setStringCached3(log, session, file, file);
}

ConnectionProfile::LogSessionMode JsonProfile::logSessionInputMode() { 
	return (LogSessionMode)getInteger("log", "session", "mode", (int)LSM_BINARY);
}

void JsonProfile::setLogSessionInputMode(LogSessionMode mode) {
	setInteger("log", "session", "mode", (int)mode);
}

BOOL JsonProfile::logFileTransferTransactions() { 
	return getBool("log", "xfer_act", "enabled", FALSE);
}

void JsonProfile::setLogFileTransferTransactions(BOOL enabled) {
	setBool("log", "xfer_act", "enabled", enabled);
}

BOOL JsonProfile::logAppendFileTransferTransactions() { 
	return getBool("log", "xfer_act", "append", FALSE);
}

void JsonProfile::setLogAppendFileTransferTransactions(BOOL enabled) {
	setBool("log", "xfer_act", "append", enabled);
}

BOOL JsonProfile::logBriefFileTransferTransactions() { 
	return getBool("log", "xfer_act", "brief", FALSE);
}

void JsonProfile::setLogBriefFileTransferTransactions(BOOL enabled) {
	setBool("log", "xfer_act", "brief", enabled);
}

CMString JsonProfile::logFileTransferTransactionsFile() { 
	return getStringCached3(log, xfer_act, file, CMString(TEXT("transact.log")));
}

void JsonProfile::setLogFileTransferTransactionsFile(CMString file) {
	setStringCached3(log, xfer_act, file, file);
}
