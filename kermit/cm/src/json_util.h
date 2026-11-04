#ifndef JSON_UTIL_H
#define JSON_UTIL_H

#include "json_config.h"


// Provids some handy utility functions to getting values in/out
// out of the JSON document easier
class JsonUtilMixin {

protected:
	JsonUtilMixin(JsonConfigFile *configFile, cJSON *json);

	cJSON *_json;
	JsonConfigFile *_config;

	
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
	setString( #grandparent, #parent, #name , value );


#endif