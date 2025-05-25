#include "json_config.h"

#include "cJSON/cJSON.h"

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include "json_profile.h"
#include "json_color_theme.h"


JsonConfigFile::JsonConfigFile(HWND parent, LPTSTR filename) : ConfigFile() {

	_version = 0;
	_profileCount = 0;
	_loaded = FALSE;
	_filename = CMString(filename);

	// Check the file actually exists before we go trying to
	// open it.
	DWORD attributes = GetFileAttributes(filename);
	if (attributes != 0xFFFFFFFF && 
         !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
		
		// File exists. Try to read it.
		HANDLE hFile;


		hFile = CreateFile(
			filename,
			GENERIC_READ,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);


		if (hFile == INVALID_HANDLE_VALUE) {
			LPTSTR buf = (LPTSTR)malloc(sizeof(TCHAR) * MAX_PATH);
			_sntprintf(buf, MAX_PATH, TEXT("Failed to open file: %s"), filename);

			MessageBox(parent, buf, TEXT("Error"), MB_OK | MB_ICONWARNING);
			free(buf);
		}

		// We're assuming the size of the JSON file can be represented
		// by a 32bit file size. If it can't, its almost certainly an
		// error. So no need to pass the second parameter - the high
		// 32 bits of a 64bit file size. Also we add one to acount for
		// null termination.
		DWORD fileSize = GetFileSize(hFile, NULL);

		LPSTR fileData = (LPSTR)malloc(fileSize + 1);

		ZeroMemory(fileData, fileSize + 1);
		
		DWORD bytesRead;
		ReadFile(hFile, fileData, fileSize, &bytesRead, NULL);

		CloseHandle(hFile);

		jsonFile = cJSON_Parse(fileData);

		free(fileData);
	} else {
		// File does not exist. Create it.

		jsonFile = cJSON_CreateObject();

		_version = 1;
		_nextProfileId = 1;
		_nextColorThemeId = 1;
		_profileCount = 0;
		_templateCount = 0;

		setInteger(jsonFile, "version", _version);
		setInteger(jsonFile, "next_profile_id", _nextProfileId);
		setInteger(jsonFile, "next_color_theme_id", _nextColorThemeId);
	}
	

	if (jsonFile != NULL) {	
		const cJSON *profiles = NULL;
		const cJSON *profile = NULL;
		const cJSON *colorThemes = NULL;
		const cJSON *theme = NULL;

		// Get file version and next object IDs
		_version = getInteger(jsonFile, "version", 1);
		_nextProfileId = getInteger(jsonFile, "next_profile_id", 1);
		_nextColorThemeId = getInteger(jsonFile, "next_color_theme_id", 1);

		// Counts of things
		_profileCount = 0;
		_templateCount = 0;
		_colorThemeCount = 0;

		// Count the profiles
		profiles = cJSON_GetObjectItemCaseSensitive(jsonFile, "profiles");
		cJSON_ArrayForEach(profile, profiles) {
			_profileCount++;
		}

		// .. the templates
		profiles = cJSON_GetObjectItemCaseSensitive(jsonFile, "templates");
		cJSON_ArrayForEach(profile, profiles) {
			_templateCount++;
		}

		// .. the themes
		colorThemes = cJSON_GetObjectItemCaseSensitive(jsonFile, "color_themes");
		cJSON_ArrayForEach(theme, colorThemes) {
			_colorThemeCount++;
		}

		// TODO: Create default templates

		// Create default themes if required
		if (_colorThemeCount == 0) {
			createDefaultThemes();
		}

		_loaded = TRUE;
	} else {

		// Error: Couldn't open the JSON file.
		MessageBox(NULL, 
			TEXT("Failed open configuration file."), 
			TEXT("Error"), MB_OK | MB_ICONWARNING);
	}
}


JsonConfigFile::~JsonConfigFile() {
	// TODO: Save?

	cJSON_Delete(jsonFile);
}

BOOL JsonConfigFile::loaded() const {
	return _loaded;
}

int JsonConfigFile::version() const {
	return _version;
}


BOOL JsonConfigFile::commitChanges() {

	TCHAR tempFileName[MAX_PATH];
	TCHAR tempPath[MAX_PATH];
	DWORD rc;

	rc = GetTempPath(MAX_PATH, tempPath);
	if (rc > MAX_PATH || rc == 0) {
		// Failed to get a temp path
		return FALSE;
	}

	rc = GetTempFileName(tempPath, TEXT("cmc"), 0, tempFileName);
	if (rc == 0) {
		// Failed to get a temp filename
		return FALSE;
	}

	HANDLE hFile = CreateFile(
		tempFileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		// Failed to create temporary file
		return FALSE;
	}

	// We should now have a temporary file we can write the config
	// data to. Now to JSONify the config data.

	// cJSON_Print pretty-prints by default. If we wanted to reduce
	// file size (and so perhaps improve performance on very slow
	// machines) we could use cJSON_PrintUnformatted instead.
	char* data = cJSON_Print(jsonFile);
	size_t dataSize = strlen(data);

	if (data == NULL) {
		CloseHandle(hFile);
		// TODO: try to close hFile
		return FALSE;
	}

	// We now have the data in JSON format. Write it to the temp
	// file.

	DWORD bytesWritten;
	if (!WriteFile(hFile, data, dataSize, &bytesWritten, NULL)) {
		// Failed to write to file.
		CloseHandle(hFile);
		return FALSE;
	}

	CloseHandle(hFile);
	free(data);

	// TODO: for maximum safety, perhaps we should rename the old file
	//       and delete it once the move has completed, rather than 
	//		 move-overwriting it 

	// Now move the temp file into place
	if (!MoveFileEx(tempFileName, 
			_filename.data(), 
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {

		// Failed to move file
		return FALSE;
	}

	return TRUE;
}

ConnectionProfile *JsonConfigFile::defaultTemplate() {
	return NULL; // TODO
}

int JsonConfigFile::profileCount() const {
	return _profileCount;
}

ConnectionProfile *JsonConfigFile::firstProfile() {
	cJSON *profiles = NULL;
	cJSON *profile = NULL;

	profiles = cJSON_GetObjectItemCaseSensitive(jsonFile, "profiles");

	if (profiles == NULL) return NULL;

	profile = cJSON_GetArrayItem(profiles, 0);

	if (profile == NULL) return NULL;

	return new JsonProfile(this, profile);
}

ConnectionProfile *JsonConfigFile::getProfileById(int id) {
	cJSON *profiles = NULL;
	cJSON *profile = NULL;

	profiles = cJSON_GetObjectItemCaseSensitive(jsonFile, "profiles");

	if (profiles == NULL) return NULL;

	profile = cJSON_GetObjectItemCaseSensitive(profiles, CMString::number(id).toUtf8(NULL));

	if (profile == NULL) return NULL;

	return new JsonProfile(this, profile);

}

ConnectionProfile *JsonConfigFile::createProfile(
		int templateId, CMString name,
		ConnectionProfile::ConnectionType conType) {

	/*
		When creating from the default template:
			Only copy values that actually exist in the JSON file.

		Perhaps we get a JsonProfile subclass that adds a bunch of
		methods that let you check if a setting is actually set or not
	*/

	cJSON *profiles = cJSON_GetObjectItemCaseSensitive(jsonFile, "profiles");

	if (profiles == NULL) {
		profiles = cJSON_CreateObject();
		
		if (profiles == NULL) return NULL;

		cJSON_AddItemToObject(jsonFile, "profiles", profiles);
	}

	cJSON *profileJson = cJSON_CreateObject();
	JsonProfile *profile = NULL;

	if (profileJson != NULL) {
		profile = new JsonProfile(this, profileJson);
		profile->setName(name);
		profile->setConnectionType(conType);
		profile->setId(_nextProfileId);

		_nextProfileId++;
		setInteger(jsonFile, "next_profile_id", _nextProfileId);

		char* idString = CMString::number(profile->id()).toUtf8(NULL);

		cJSON_AddItemToObject(profiles, 
			idString, 
			profileJson);

		free(idString);
	}

	return profile;
}


// -----------------------------------------------------------

int JsonConfigFile::colorThemeCount() const {
	return _colorThemeCount;
}

ColorTheme *JsonConfigFile::firstColorTheme() {
	cJSON *themes = NULL;
	cJSON *theme = NULL;

	themes = cJSON_GetObjectItemCaseSensitive(jsonFile, "color_themes");

	if (themes == NULL) return NULL;

	theme = cJSON_GetArrayItem(themes, 0);

	if (theme == NULL) return NULL;

	return new JsonColorTheme(this, theme);
}

ColorTheme *JsonConfigFile::getColorThemeById(int id) {
	cJSON *themes = NULL;
	cJSON *theme = NULL;

	themes = cJSON_GetObjectItemCaseSensitive(jsonFile, "color_themes");

	if (themes == NULL) return NULL;

	theme = cJSON_GetObjectItemCaseSensitive(themes, CMString::number(id).toUtf8(NULL));

	if (theme == NULL) return NULL;

	return new JsonColorTheme(this, theme);

}

ColorTheme *JsonConfigFile::createColorTheme() {
	cJSON *themes = cJSON_GetObjectItemCaseSensitive(jsonFile, "color_themes");

	if (themes == NULL) {
		themes = cJSON_CreateObject();
		
		if (themes == NULL) return NULL;

		cJSON_AddItemToObject(jsonFile, "color_themes", themes);
	}

	cJSON *themeJson = cJSON_CreateObject();
	JsonColorTheme *theme = NULL;

	if (themeJson != NULL) {
		theme = new JsonColorTheme(this, themeJson);
		theme->setId(_nextColorThemeId);

		_nextColorThemeId++;
		setInteger(jsonFile, "next_color_theme_id", _nextColorThemeId);

		char* idString = CMString::number(theme->id()).toUtf8(NULL);

		cJSON_AddItemToObject(themes, 
			idString, 
			themeJson);

		free(idString);
	}

	return theme;
}

// -----------------------------------------------------------

CMString JsonConfigFile::getString(cJSON *json, LPCSTR name, 
								CMString defaultValue) {

	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, name);

	if (obj == NULL || !cJSON_IsString(obj)) {
		// Value either doesn't exist or it isn't a string.
		// Just return the default value.
		return defaultValue;
	}

	return CMString::fromUtf8(obj->valuestring);
}

int JsonConfigFile::getInteger(cJSON *json, LPCSTR name,
						   int defaultValue) {
	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, name);

	if (obj == NULL || !cJSON_IsNumber(obj)) {
		// Value either doesn't exist or it isn't an int.
		// Just return the default value.
		return defaultValue;
	}

	return obj->valueint;
}

BOOL JsonConfigFile::getBool(cJSON *json, LPCSTR name,
						   BOOL defaultValue) {
	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, name);

	if (obj == NULL || !cJSON_IsBool(obj)) {
		// Value either doesn't exist or it isn't a bool.
		// Just return the default value.
		return defaultValue;
	}

	return cJSON_IsTrue(obj);
}

void JsonConfigFile::setString(cJSON *json, LPCSTR name, CMString value) {
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

void JsonConfigFile::setInteger(cJSON *json, LPCSTR name, int value) {
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

void JsonConfigFile::setBool(cJSON *json, LPCSTR name, BOOL value) {
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

void JsonConfigFile::setNull(cJSON *json, LPCSTR name) {
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
