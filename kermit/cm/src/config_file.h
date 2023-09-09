#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include "conn_profile.h"

class ConfigFile {

public:
	virtual int version() const = 0;
	
	virtual ConnectionProfile* defaultTemplate() = 0;

	virtual int profileCount() const = 0;

	// Get the first profile. To iterate, call nextProfile()
	virtual ConnectionProfile* firstProfile() = 0;

	// Get a profile by its unique ID
	virtual ConnectionProfile* getProfileById(int id) = 0;

	//virtual int templateCount() = 0;

	virtual ConnectionProfile* createProfile(int templateId,
		CMString name, ConnectionProfile::ConnectionType conType) = 0;

	virtual BOOL loaded() const { return TRUE; }

	virtual BOOL commitChanges() { return TRUE; }
};

#endif /* CONFIG_FILE_H */
