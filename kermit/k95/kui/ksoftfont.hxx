/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ksoftfont_hxx_included
#define ksoftfont_hxx_included
/*========================================================================
    Copyright (c) 2026, David Goodwin
-----------------------------------------------------------------------

    File.......: ksoftfont.hxx
    Author.....: David Goodwin (david@zx.net.nz)
    Created....: June 29, 2026
    License....: BSD-3-Clause

    Description: Handles one or more soft-fonts (eg DEC VT DRCS font
        buffers) used to render the terminal screen.

========================================================================*/

#include "kuidef.h"
#include "ikextern.h"

class KSoftFont {
public:
    KSoftFont();
    ~KSoftFont();

    void setSize(int width, int height);
    void refresh(int width, int height);

    void textOut(HDC hdc, int x, int y, LPCWSTR str, unsigned int length,
        int font);

    BOOL fontValid(int fontId) const;

    HBITMAP getFontBitmap(int fontId, BOOL stretched);

private:
    void stretchSoftFont(int fontId);

    // Selected font size
    int width;
    int height;

    // Last terminal dimensions seen
    int terminalWidth;
    int terminalHeight;
    int terminalRendition;

    // Current fonts
    char currentRenditions[DRCS_BUFFERS];
    char serials[DRCS_BUFFERS];
    char renderHints[DRCS_BUFFERS];
    HBITMAP fonts[DRCS_BUFFERS];

    // And versions of the current fonts stretched to size
    HBITMAP stretchedFonts[DRCS_BUFFERS];
    int stretchedWidth;
    int stretchedHeight;
};

#endif // ksoftfont_hxx_included
