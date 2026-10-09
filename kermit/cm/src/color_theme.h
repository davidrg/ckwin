#ifndef COLOR_THEME_H
#define COLOR_THEME_H

#include "conn_profile.h"


class ColorTheme {

public:
	// If this theme is currently stored in a config file, causes
	// the config file to be saved.
	virtual BOOL commitChanges() { return TRUE; }

	// For iterating over themes. Fetches the next theme in the
	// list of themes. Note that the caller is responsible for
	// disposing of the returned theme when finished with it.
	virtual ColorTheme* nextTheme() const = 0;

	// --- Theme Metadata ---
	virtual CMString name() = 0;
	virtual void setName(CMString newName) = 0;

	// Unique un-changing ID for this theme
	virtual int id() const = 0;

	// --- Theme Colors ---
	virtual RGB8BitColor rgbBlack() const = 0;
	virtual void setRgbBlack(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbBlue() const = 0;
	virtual void setRgbBlue(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbGreen() const = 0;
	virtual void setRgbGreen(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbCyan() const = 0;
	virtual void setRgbCyan(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbRed() const = 0;
	virtual void setRgbRed(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbMagenta() const = 0;
	virtual void setRgbMagenta(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbBrown() const = 0;
	virtual void setRgbBrown(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbLightGray() const = 0;
	virtual void setRgbLightGray(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbDarkGray() const = 0;
	virtual void setRgbDarkGray(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbLightBlue() const = 0;
	virtual void setRgbLightBlue(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbLightGreen() const = 0;
	virtual void setRgbLightGreen(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbLightCyan() const = 0;
	virtual void setRgbLightCyan(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbLightRed() const = 0;
	virtual void setRgbLightRed(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbLightMagenta() const = 0;
	virtual void setRgbLightMagenta(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbYellow() const = 0;
	virtual void setRgbYellow(RGB8BitColor c) = 0;

	virtual RGB8BitColor rgbWhite() const = 0;
	virtual void setRgbWhite(RGB8BitColor c) = 0;
};

#endif
