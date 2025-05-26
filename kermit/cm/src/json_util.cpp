#include "json_util.h"

#include "cJSON/cJSON.h"

JsonUtilMixin::JsonUtilMixin(JsonConfigFile *configFile, cJSON *json)
	: _json(json), _config(configFile) {

}


CMString JsonUtilMixin::getString(cJSON *json, LPCSTR name, 
								CMString defaultValue) const {

	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, name);

	if (obj == NULL || !cJSON_IsString(obj)) {
		// Value either doesn't exist or it isn't a string.
		// Just return the default value.
		return defaultValue;
	}

	return CMString::fromUtf8(obj->valuestring);
}

int JsonUtilMixin::getInteger(cJSON *json, LPCSTR name,
						   int defaultValue) const {
	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, name);

	if (obj == NULL || !cJSON_IsNumber(obj)) {
		// Value either doesn't exist or it isn't an int.
		// Just return the default value.
		return defaultValue;
	}

	return obj->valueint;
}

BOOL JsonUtilMixin::getBool(cJSON *json, LPCSTR name,
						   BOOL defaultValue) const {
	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, name);

	if (obj == NULL || !cJSON_IsBool(obj)) {
		// Value either doesn't exist or it isn't a bool.
		// Just return the default value.
		return defaultValue;
	}

	return cJSON_IsTrue(obj);
}

void JsonUtilMixin::setString(cJSON *json, LPCSTR name, CMString value) {
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

	char* valueStr = value.toUtf8(NULL);

	if (obj == NULL) {
		// Doesn't exist - add it	
		cJSON_AddStringToObject(json, name, valueStr);
	} else {
		// It exists - update its value.
		cJSON_SetValuestring(obj, valueStr);
	}

	free(valueStr);
}

void JsonUtilMixin::setInteger(cJSON *json, LPCSTR name, int value) {
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

void JsonUtilMixin::setBool(cJSON *json, LPCSTR name, BOOL value) {
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

void JsonUtilMixin::setNull(cJSON *json, LPCSTR name) {
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

CMString JsonUtilMixin::getString(LPCSTR name, CMString defaultValue) const {
	return getString(_json, name, defaultValue);
}

int JsonUtilMixin::getInteger(LPCSTR name, int defaultValue) const {
	return getInteger(_json, name, defaultValue);
}

BOOL JsonUtilMixin::getBool(LPCSTR name, BOOL defaultValue) const {
	return getBool(_json, name, defaultValue);
}

void JsonUtilMixin::setString(LPCSTR name, CMString value) {
	setString(_json, name, value);
}

void JsonUtilMixin::setInteger(LPCSTR name, int value) {
	setInteger(_json, name, value);
}


void JsonUtilMixin::setBool(LPCSTR name, BOOL value) {
	setBool(_json, name, value);
}

CMString JsonUtilMixin::getString(LPCSTR parentName, LPCSTR name, CMString defaultValue) const {
	cJSON *parent = cJSON_GetObjectItemCaseSensitive(_json, parentName);

	if (parent == NULL || !cJSON_IsObject(parent)) {
		// parent object either doesn't exist or its not actually
		// an object. Return the default value.
		return defaultValue;
	}

	// Get the string value from the parent object.
	return getString(parent, name, defaultValue);
}

int JsonUtilMixin::getInteger(LPCSTR parentName, LPCSTR name, int defaultValue) const {
	cJSON *parent = cJSON_GetObjectItemCaseSensitive(_json, parentName);

	if (parent == NULL || !cJSON_IsObject(parent)) {
		// parent object either doesn't exist or its not actually
		// an object. Return the default value.
		return defaultValue;
	}

	// Get the int value from the parent object.
	return getInteger(parent, name, defaultValue);
}

BOOL JsonUtilMixin::getBool(LPCSTR parentName, LPCSTR name, BOOL defaultValue) const {
	cJSON *parent = cJSON_GetObjectItemCaseSensitive(_json, parentName);

	if (parent == NULL || !cJSON_IsObject(parent)) {
		// parent object either doesn't exist or its not actually
		// an object. Return the default value.
		return defaultValue;
	}

	// Get the bool value from the parent object.
	return getBool(parent, name, defaultValue);
}

void JsonUtilMixin::setString(LPCSTR parentName, LPCSTR name, CMString value) {
	cJSON *parent = cJSON_GetObjectItemCaseSensitive(_json, parentName);

	if (parent == NULL) {
		// parent doesn't exist - create it
		parent = cJSON_CreateObject();
		cJSON_AddItemToObject(_json, parentName, parent);
	}

	setString(parent, name, value);
}

void JsonUtilMixin::setInteger(LPCSTR parentName, LPCSTR name, int value) {
	cJSON *parent = cJSON_GetObjectItemCaseSensitive(_json, parentName);

	if (parent == NULL) {
		// parent doesn't exist - create it
		parent = cJSON_CreateObject();
		cJSON_AddItemToObject(_json, parentName, parent);
	}

	setInteger(parent, name, value);
}

void JsonUtilMixin::setBool(LPCSTR parentName, LPCSTR name, BOOL value) {
	cJSON *parent = cJSON_GetObjectItemCaseSensitive(_json, parentName);

	if (parent == NULL) {
		// parent doesn't exist - create it
		parent = cJSON_CreateObject();
		cJSON_AddItemToObject(_json, parentName, parent);
	}

	setBool(parent, name, value);
}


CMString JsonUtilMixin::getString(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, CMString defaultValue) const {
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

int JsonUtilMixin::getInteger(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, int defaultValue) const {
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

BOOL JsonUtilMixin::getBool(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, BOOL defaultValue) const {
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

void JsonUtilMixin::setString(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, CMString value) {
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
		cJSON_AddItemToObject(grandparent, parentName, parent);
	}

	setString(parent, name, value);
}

void JsonUtilMixin::setInteger(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, int value) {
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
		cJSON_AddItemToObject(grandparent, parentName, parent);
	}

	setInteger(parent, name, value);
}

void JsonUtilMixin::setBool(LPCSTR grandParentName, LPCSTR parentName, LPCSTR name, BOOL value) {
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
