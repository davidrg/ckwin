#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include "conn_profile.h"
#include "color_theme.h"

class ConfigFile {

public:
	ConfigFile();

	virtual int version() const = 0;
	
	//////// TEMPLATES //////// 
	virtual ConnectionProfile* defaultTemplate() = 0;
	//virtual int templateCount() = 0;

	//////// CONNECTION PROFILES ////////
	virtual int profileCount() const = 0;

	// Get the first profile. To iterate, call nextProfile()
	virtual ConnectionProfile* firstProfile() = 0;

	// Get a profile by its unique ID
	virtual ConnectionProfile* getProfileById(int id) = 0;

	virtual ConnectionProfile* createProfile(int templateId,
		CMString name, ConnectionProfile::ConnectionType conType) = 0;

	virtual void setMostRecentlyUsedProfile(ConnectionProfile* profile) = 0;
	virtual void removeMostRecentlyUsedProfile(ConnectionProfile* profile) = 0;
	virtual unsigned int getMostRecentlyUsedProfiles(int outProfileIds[], int outProfileLength) = 0;

	//////// COLOR THEMES ////////
	virtual int defaultThemeId();
	virtual int colorThemeCount() const = 0;
	virtual ColorTheme* firstColorTheme() = 0;
	virtual ColorTheme* getColorThemeById(int id) = 0;
	virtual ColorTheme* createColorTheme() = 0;

	//////// STORAGE ////////
	virtual BOOL loaded() const { return TRUE; }

	virtual BOOL commitChanges() { return TRUE; }

protected:
	virtual void createDefaultThemes();
	int _defaultThemeId;
};

#endif /* CONFIG_FILE_H */
