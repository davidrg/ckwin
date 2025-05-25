#ifndef JSON_CONFIG_H
#define JSON_CONFIG_H

#include "config_file.h"

struct cJSON;
class JsonProfile;

class JsonConfigFile: public ConfigFile {

public:
	JsonConfigFile(HWND parent, LPTSTR filename);
	~JsonConfigFile();
	virtual int version() const;
	
	//////// TEMPLATES //////// 
	virtual ConnectionProfile* defaultTemplate();

	/*virtual int templateCount() const;
	virtual ConnectionProfile* getTemplateByID(int id);
	virtual ConnectionProfile* createTemplate(
		int templateId, CMString name);*/


	//////// CONNECTION PROFILES ////////
	virtual int profileCount() const;
	
	virtual ConnectionProfile* firstProfile();
	
	virtual ConnectionProfile* getProfileById(int id);

	virtual ConnectionProfile* createProfile(
		int templateId, CMString name,
		ConnectionProfile::ConnectionType conType);

	//////// COLOR THEMES ////////
	virtual int colorThemeCount() const;
	virtual ColorTheme* firstColorTheme();
	virtual ColorTheme* getColorThemeById(int id);
	virtual ColorTheme* createColorTheme();

	//////// STORAGE ////////
	virtual BOOL loaded() const;

	virtual BOOL commitChanges();

private:
	cJSON *jsonFile;

	int _version, _profileCount, _templateCount, _colorThemeCount;
	int _nextProfileId, _nextColorThemeId;
	BOOL _loaded;

	CMString _filename;

	// ----------------------------
	// Some handy utility functions to getting values in/out
	// out of the JSON document easier
	
	// Get the value named <name> from JSON object <json>
	// returning <defaultValue> if it doesn't exist or
	// exists with the wrong type
	CMString getString(cJSON *json, LPCSTR name, CMString defaultValue=CMString());
	int	getInteger(cJSON *json, LPCSTR name, int defaultValue=0);
	BOOL getBool(cJSON *json, LPCSTR name, BOOL defaultValue=FALSE);

	// Set the value named <name> on JSON object <json>
	// creating it if it doesn't exist or exists but has
	// the wrong type.
	void setString(cJSON *json, LPCSTR name, CMString value);
	void setInteger(cJSON *json, LPCSTR name, int value);
	void setBool(cJSON *json, LPCSTR name, BOOL value);
	void setNull(cJSON *json, LPCSTR name);

};

#endif /* JSON_CONFIG_H */