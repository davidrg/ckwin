#include "json_color_theme.h"
#include "cJSON/cJSON.h"



JsonColorTheme::JsonColorTheme(JsonConfigFile *configFile, cJSON *json)
		:JsonUtilMixin(configFile, json) {

}

JsonColorTheme::~JsonColorTheme() {

}

BOOL JsonColorTheme::commitChanges() {
	return _config->commitChanges();
}

ColorTheme* JsonColorTheme::nextTheme() const {

	if (_json->next == NULL) {
		// Either we're at the end of the list of themes,
		// or this theme hasn't been added to the config
		// file yet.
		return NULL;
	}

	return new JsonColorTheme(_config, _json->next);
}


int JsonColorTheme::id() const {
	return getInteger("id", 0);
}

void JsonColorTheme::setId(int id) {
	setInteger("id", id);
}

CMString JsonColorTheme::name() {
	return getStringCached(name, CMString());
}


void JsonColorTheme::setName(CMString name) { 
	setStringCached(name, name);
}


// ----- GUI Colors -----
RGB8BitColor JsonColorTheme::rgbBlack() const { 
	RGB8BitColor x = { 
			getInteger("colors", "black", "r", 0), 
			getInteger("colors", "black", "g", 0), 
			getInteger("colors", "black", "b", 0) 
	}; 
	return x;
}

void JsonColorTheme::setRgbBlack(RGB8BitColor c) {
	setInteger("colors", "black", "r", c.r);
	setInteger("colors", "black", "g", c.g);
	setInteger("colors", "black", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbBlue() const { 
	RGB8BitColor x = { 
			getInteger("colors", "blue", "r", 0), 
			getInteger("colors", "blue", "g", 0), 
			getInteger("colors", "blue", "b", 128) 
	}; 
	return x;
}

void JsonColorTheme::setRgbBlue(RGB8BitColor c) {
	setInteger("colors", "blue", "r", c.r);
	setInteger("colors", "blue", "g", c.g);
	setInteger("colors", "blue", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbGreen() const { 
	RGB8BitColor x = { 
			getInteger("colors", "green", "r", 0), 
			getInteger("colors", "green", "g", 128), 
			getInteger("colors", "green", "b", 0) 
	}; 
	return x;
}

void JsonColorTheme::setRgbGreen(RGB8BitColor c) {
	setInteger("colors", "green", "r", c.r);
	setInteger("colors", "green", "g", c.g);
	setInteger("colors", "green", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbCyan() const { 
	RGB8BitColor x = { 
			getInteger("colors", "cyan", "r", 0), 
			getInteger("colors", "cyan", "g", 128), 
			getInteger("colors", "cyan", "b", 128) 
	}; 
	return x;
}

void JsonColorTheme::setRgbCyan(RGB8BitColor c) {
	setInteger("colors", "cyan", "r", c.r);
	setInteger("colors", "cyan", "g", c.g);
	setInteger("colors", "cyan", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbRed() const { 
	RGB8BitColor x = { 
			getInteger("colors", "red", "r", 127), 
			getInteger("colors", "red", "g", 0), 
			getInteger("colors", "red", "b", 0) 
	}; 
	return x;
}

void JsonColorTheme::setRgbRed(RGB8BitColor c) {
	setInteger("colors", "red", "r", c.r);
	setInteger("colors", "red", "g", c.g);
	setInteger("colors", "red", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbMagenta() const { 
	RGB8BitColor x = { 
			getInteger("colors", "magenta", "r", 128), 
			getInteger("colors", "magenta", "g", 0), 
			getInteger("colors", "magenta", "b", 128) 
	}; 
	return x;
}

void JsonColorTheme::setRgbMagenta(RGB8BitColor c) {
	setInteger("colors", "magenta", "r", c.r);
	setInteger("colors", "magenta", "g", c.g);
	setInteger("colors", "magenta", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbBrown() const { 
	RGB8BitColor x = { 
			getInteger("colors", "brown", "r", 128), 
			getInteger("colors", "brown", "g", 0), 
			getInteger("colors", "brown", "b", 128) 
	}; 
	return x;
}

void JsonColorTheme::setRgbBrown(RGB8BitColor c) {
	setInteger("colors", "brown", "r", c.r);
	setInteger("colors", "brown", "g", c.g);
	setInteger("colors", "brown", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbLightGray() const { 
	RGB8BitColor x = { 
			getInteger("colors", "lgray", "r", 192), 
			getInteger("colors", "lgray", "g", 192), 
			getInteger("colors", "lgray", "b", 192) 
	}; 
	return x;
}

void JsonColorTheme::setRgbLightGray(RGB8BitColor c) {
	setInteger("colors", "lgray", "r", c.r);
	setInteger("colors", "lgray", "g", c.g);
	setInteger("colors", "lgray", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbDarkGray() const { 
	RGB8BitColor x = { 
			getInteger("colors", "dgray", "r", 128), 
			getInteger("colors", "dgray", "g", 128), 
			getInteger("colors", "dgray", "b", 128) 
	}; 
	return x;
}

void JsonColorTheme::setRgbDarkGray(RGB8BitColor c) {
	setInteger("colors", "dgray", "r", c.r);
	setInteger("colors", "dgray", "g", c.g);
	setInteger("colors", "dgray", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbLightBlue() const { 
	RGB8BitColor x = { 
			getInteger("colors", "lblue", "r", 0), 
			getInteger("colors", "lblue", "g", 0), 
			getInteger("colors", "lblue", "b", 255) 
	}; 
	return x;
}

void JsonColorTheme::setRgbLightBlue(RGB8BitColor c) {
	setInteger("colors", "lblue", "r", c.r);
	setInteger("colors", "lblue", "g", c.g);
	setInteger("colors", "lblue", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbLightGreen() const { 
	RGB8BitColor x = { 
			getInteger("colors", "lgreen", "r", 0), 
			getInteger("colors", "lgreen", "g", 255), 
			getInteger("colors", "lgreen", "b", 0) 
	}; 
	return x;
}

void JsonColorTheme::setRgbLightGreen(RGB8BitColor c) {
	setInteger("colors", "lgreen", "r", c.r);
	setInteger("colors", "lgreen", "g", c.g);
	setInteger("colors", "lgreen", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbLightCyan() const { 
	RGB8BitColor x = { 
			getInteger("colors", "lcyan", "r", 0), 
			getInteger("colors", "lcyan", "g", 255), 
			getInteger("colors", "lcyan", "b", 255) 
	}; 
	return x;
}

void JsonColorTheme::setRgbLightCyan(RGB8BitColor c) {
	setInteger("colors", "lcyan", "r", c.r);
	setInteger("colors", "lcyan", "g", c.g);
	setInteger("colors", "lcyan", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbLightRed() const { 
	RGB8BitColor x = { 
			getInteger("colors", "lred", "r", 255), 
			getInteger("colors", "lred", "g", 0), 
			getInteger("colors", "lred", "b", 0) 
	}; 
	return x;
}

void JsonColorTheme::setRgbLightRed(RGB8BitColor c) {
	setInteger("colors", "lred", "r", c.r);
	setInteger("colors", "lred", "g", c.g);
	setInteger("colors", "lred", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbLightMagenta() const { 
	RGB8BitColor x = { 
			getInteger("colors", "lmagenta", "r", 255), 
			getInteger("colors", "lmagenta", "g", 0), 
			getInteger("colors", "lmagenta", "b", 255) 
	}; 
	return x;
}

void JsonColorTheme::setRgbLightMagenta(RGB8BitColor c) {
	setInteger("colors", "lmagenta", "r", c.r);
	setInteger("colors", "lmagenta", "g", c.g);
	setInteger("colors", "lmagenta", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbYellow() const { 
	RGB8BitColor x = { 
			getInteger("colors", "yellow", "r", 255), 
			getInteger("colors", "yellow", "g", 255), 
			getInteger("colors", "yellow", "b", 0) 
	}; 
	return x;
}

void JsonColorTheme::setRgbYellow(RGB8BitColor c) {
	setInteger("colors", "yellow", "r", c.r);
	setInteger("colors", "yellow", "g", c.g);
	setInteger("colors", "yellow", "b", c.b);
}

RGB8BitColor JsonColorTheme::rgbWhite() const { 
	RGB8BitColor x = { 
			getInteger("colors", "white", "r", 255), 
			getInteger("colors", "white", "g", 255), 
			getInteger("colors", "white", "b", 255) 
	}; 
	return x;
}

void JsonColorTheme::setRgbWhite(RGB8BitColor c) {
	setInteger("colors", "white", "r", c.r);
	setInteger("colors", "white", "g", c.g);
	setInteger("colors", "white", "b", c.b);
}
