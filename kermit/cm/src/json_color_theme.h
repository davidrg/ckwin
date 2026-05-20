#ifndef JSON_COLOR_THEME_H
#define JSON_COLOR_THEME_H

#include "color_theme.h"
#include "json_util.h"

class JsonColorTheme: public ColorTheme, private JsonUtilMixin {
	friend class JsonConfigFile;

public:
	~JsonColorTheme();

	// If this theme is currently stored in a config file, causes
	// the config file to be saved.
	virtual BOOL commitChanges();

	// For iterating over themes. Fetches the next theme in the
	// list of themes. Note that the caller is responsible for
	// disposing of the returned theme when finished with it.
	virtual ColorTheme* nextTheme() const;

	// --- Theme Metadata ---
	virtual CMString name();
	virtual void setName(CMString newName);

	// Unique un-changing ID for this theme
	virtual int id() const;

	// --- Theme Colors ---
	virtual RGB8BitColor rgbBlack() const;
	virtual void setRgbBlack(RGB8BitColor c);

	virtual RGB8BitColor rgbBlue() const;
	virtual void setRgbBlue(RGB8BitColor c);

	virtual RGB8BitColor rgbGreen() const;
	virtual void setRgbGreen(RGB8BitColor c);

	virtual RGB8BitColor rgbCyan() const;
	virtual void setRgbCyan(RGB8BitColor c);

	virtual RGB8BitColor rgbRed() const;
	virtual void setRgbRed(RGB8BitColor c);

	virtual RGB8BitColor rgbMagenta() const;
	virtual void setRgbMagenta(RGB8BitColor c);

	virtual RGB8BitColor rgbBrown() const;
	virtual void setRgbBrown(RGB8BitColor c);

	virtual RGB8BitColor rgbLightGray() const;
	virtual void setRgbLightGray(RGB8BitColor c);

	virtual RGB8BitColor rgbDarkGray() const;
	virtual void setRgbDarkGray(RGB8BitColor c);

	virtual RGB8BitColor rgbLightBlue() const;
	virtual void setRgbLightBlue(RGB8BitColor c);

	virtual RGB8BitColor rgbLightGreen() const;
	virtual void setRgbLightGreen(RGB8BitColor c);

	virtual RGB8BitColor rgbLightCyan() const;
	virtual void setRgbLightCyan(RGB8BitColor c);

	virtual RGB8BitColor rgbLightRed() const;
	virtual void setRgbLightRed(RGB8BitColor c);

	virtual RGB8BitColor rgbLightMagenta() const;
	virtual void setRgbLightMagenta(RGB8BitColor c);

	virtual RGB8BitColor rgbYellow() const;
	virtual void setRgbYellow(RGB8BitColor c);

	virtual RGB8BitColor rgbWhite() const;
	virtual void setRgbWhite(RGB8BitColor c);

protected:
	JsonColorTheme(JsonConfigFile *configFile, cJSON *json);
	virtual void setId(int id);
	
	CMString _cached_name;
};

#endif
