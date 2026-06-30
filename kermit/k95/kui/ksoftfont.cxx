#include "ksoftfont.hxx"

extern "C" {
    extern int tt_type_mode;
    extern drcs_t *drcsbuf[];
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KSoftFont::KSoftFont() {
    width = height = 0;
    terminalWidth = terminalHeight = 0;
    terminalRendition = 0;
    stretchedHeight = stretchedWidth = 0;

    for (int i = 0; i < DRCS_BUFFERS; i++) {
        currentRenditions[i] = 0;
        serials[i] = 0;
        renderHints[i] = 0;
        fonts[i] = NULL;
        stretchedFonts[i] = NULL;
    }
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KSoftFont::~KSoftFont() {
    for (int i = 0; i < DRCS_BUFFERS; i++) {
        if (fonts[i] != NULL) {
            DeleteObject(fonts[i]);
        }
        if (stretchedFonts[i] != NULL) {
            DeleteObject(stretchedFonts[i]);
        }
    }
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KSoftFont::setSize(int width, int height) {
    if (width != stretchedWidth ||
        height != stretchedHeight) {
        this->width = width;
        this->height = height;
        stretchSoftFont(0);
    }
}


/*------------------------------------------------------------------------
Refreshes the KUI bitmap representation of soft fonts from the DRCS font
buffer(s)
------------------------------------------------------------------------*/
//
// Parameters:
//      width: Terminal width in character cells
//     height: Terminal height in character cells
void KSoftFont::refresh(int width, int height) {
    int rendition = DRCS_RENDITION_01_80x24;

//#define SWITCH_SOFTFONT_RENDITION
#ifdef SWITCH_SOFTFONT_RENDITION
    /* This all works correctly, its just not worth *only* switching the
     * soft-font based on screen size if we're not also going to switch the
     * main font to match */

    int cell_height = 0;
    int screen_rendition = rendition = terminalRendition;

    // The VT220 does not switch renditions for DECCOLM. It stores only one
    // rendition, and it uses it for both 80 column and 132 column views
    if (ISVT320(tt_type_mode) &&
        (width != terminalWidth || height != terminalHeight)) {

        if (width < 132) { /* 80 columns */
            rendition = DRCS_RENDITION_01_80x24;
            if (height > 24) rendition = DRCS_RENDITION_11_80x36;
            if (height > 36) rendition = DRCS_RENDITION_21_80x48;
        } else { /* 132 columns */
            rendition = DRCS_RENDITION_02_132_24;
            if (height > 24) rendition = DRCS_RENDITION_12_132x36;
            if (height > 36) rendition = DRCS_RENDITION_22_132x48;
        }

        screen_rendition = rendition;

        // The VT520 doesn't switch fonts for different screen heights.
        if (tt_type_mode == TT_VT520 || tt_type_mode == TT_VT525) {
            /* The VT520 doesn't switch soft-fonts when the screen height changes.
             * It only switches when the width changes. But we still want to render
             * a correct-sized replacement character, so we'll still keep track of
             * the proper rendition but only use it for that. */

            if (width < 132) { /* 80 columns */
                rendition = DRCS_RENDITION_01_80x24;
                cell_height = 16;

                if (height > 24) {
                    cell_height = 10;
                }
                if (height > 36) {
                    cell_height = 8;
                }
            } else { /* 132 columns */
                rendition = DRCS_RENDITION_02_132_24;
                cell_height = 16;

                if (height > 24) {
                    cell_height = 10;
                }
                if (height > 36) {
                    cell_height = 8;
                }
            }
        }

        terminalRendition = rendition;
        terminalWidth = width;
        terminalHeight = height;
    }
#endif

    EnterDRCSBufferCriticalSection();
    for (int i = 0; i < DRCS_BUFFERS; i++) {
        if (drcsbuf[i] != NULL && (drcsbuf[i]->serial != serials[i] ||
            fonts[i] == NULL || rendition != currentRenditions[i])) {
            // Soft font has changed or is new. Render to a bitmap.

            debug(F101, "KClient::refreshSoftFonts - refresh soft font", 0, i);

            int font_rendition = rendition;

#ifdef SWITCH_SOFTFONT_RENDITION
            // The VT520 only stores two renditions: an 80 column, and a 132
            // column. It does not switch renditions for different screen
            // heights. K95 uses the 80x24 and 132x24 renditions for 80 and 132
            // columns.
            //
            // On the VT520 both renditions start out as null and when the
            // rendition is null the appropriate replacement character from the
            // hard font will be used instead. A DECDLD will 'create' which ever
            // rendition it's pointed at (80x24 by default) and bake in a
            // suitable replacement character. If Pe=2, the other rendition will
            // be left null. This has the effect that doing a DCS 0;0;2{ @ ST
            // will display the replacement characters chopped in half on an
            // 80x48 screen while they'll render correctly on a 132x48 screen.
            // The 80 column rendition has the 24-line replacement character
            // baked in, while the 132 column rendition is null so the character
            // from the hard font is used instead.
            //
            // K95 always initializes all renditions with a suitable replacement
            // character when a DRCS buffer is initialized, and the
            // uninitialised flag tracks whether its null (true) or not (false)
            // to produce the same behaviour. When a rendition is null, instead
            // of using the 80x24 or 132x24 rendition we'll use the correct
            // rendition for the screen to get the correct replacement
            // character.
            if (drcsbuf[i]->renditions[rendition]->undefined) {
                font_rendition = screen_rendition;
            }
#endif /* SWITCH_SOFTFONT_RENDITION */

            HDC hDC = GetDC(NULL);
            HDC hMDC = CreateCompatibleDC(hDC);

            int display_width = drcsbuf[i]->renditions[font_rendition]->cell_width;
            int h = drcsbuf[i]->renditions[font_rendition]->cell_height;

#ifdef SWITCH_SOFTFONT_RENDITION
            // For VT520, take the cell height from the current screen
            // dimensions rather than the current font rendition
            if (cell_height != 0) h = cell_height;
#endif /* SWITCH_SOFTFONT_RENDITION */

            if (drcsbuf[i]->render_hints & DRCS_RENDER_HINT_VT220) {
                /* To account for VT220 glyphs being stretched horizontally
                 * into the next cell, which we don't really do. */
                display_width += 1;
            }

            int w = display_width * 96;

            HBITMAP hbmp = CreateBitmap(w,h,1,1,NULL);

            ReleaseDC(NULL, hDC);

            HBITMAP hOldBmp = (HBITMAP)SelectObject(hMDC, hbmp);

            // Paint the bitmap white
            RECT r;
            r.left = r.top = 0;
            r.right = w;
            r.bottom = h;
            HBRUSH bgBrush = CreateSolidBrush( RGB(255, 255, 255) );
            FillRect( hMDC, &r, bgBrush);
            DeleteObject( bgBrush );

            // And draw pixels in black. This gives vastly better scaling using
            // the BLACKONWHITE StretchBlt mode.
            COLORREF pixelValue = RGB(0, 0, 0);

            for (int glyph = 0; glyph < 96; glyph++) {
                if (glyph == 0 && !drcsbuf[i]->is_96_chars) continue;
                if (glyph == 95 && !drcsbuf[i]->is_96_chars) continue;
                int offset = glyph * display_width;
                int target_row = -3;
                for (int row = 0; row < h; row++) {
                    int row_pixels = drcsbuf[i]->renditions[font_rendition]->glyphs[glyph].pixels[row];
                    target_row += 3;
                    if (row_pixels == 0) {
                        continue;
                    }

                    for (int col = 0; col < display_width; col++) {
                        int bit = 0x8000 >> col;

                        /* VT220 terminals implement dot stretching: at render
                         * time, each pixel is stretched about half-way into the
                         * next pixel to the right. We can't really do
                         * half-pixels here, and it would be too expensive to
                         * do this dot stretching at render time so instead we
                         * stretch pixel all the way into its neighbour to the
                         * right when turning the glyphs to a bitmap here.
                         *
                         * Additionally, columns 9 and 10 are not addressable.
                         * These columns are just a copy of column 8. And dot
                         * stretching is applied, so column 10 stretches into
                         * column 11 (the first column of the cell to the
                         * right).
                         *
                         * At render time, the VT220 appears to render glyphs
                         * one pixel to the right; so column 1 in the glyph is
                         * really column 2 in the character cell. And the 10th
                         * column stretches into the first column of the next
                         * cell. We don't currently implement the "stretches
                         * into the next cell" rendering behaviour, so instead
                         * we treat glyphs as being 11 columns wide with columns
                         * 8-11 having the same value.
                         *
                         * The rendering we do here doesn't quite match the
                         * behaviour of the VT220 as described here:
                         *   https://www.masswerk.at/nowgobang/2019/dec-crt-typography
                         * but it's the best that can be reasonably done here
                         * without a significant performance impact. Doing it
                         * properly with reasonable performance might require
                         * something like pixel shaders.
                         *
                         * So, to summarise the rendering differences from a
                         * real VT220:
                         *   - No scan lines (they're quite visible on the VT220)
                         *   - We stretch each pixel fully into its neighbour
                         *     rather than half-way. This means the glyph
                         *     ~?~~~~~~/~~~~~~~~ will appear as a solid block
                         *     when it should have a thin vertical line in the
                         *     top half where the ? is.
                         *   - Column 10 doesn't stretch into column 1 on the
                         *     adjacent cell
                         *   - Dot stretching is applied before rather than after
                         *     double-wide stretching, so double-wide characters
                         *     come out less detailed than on a real VT220.
                         */
                        if (drcsbuf[i]->render_hints & DRCS_RENDER_HINT_VT220) {
                            int prevbit;

                            if (col < 10) {
                                prevbit = bit;
                            }

                            if (col == 9 || col == 10) {
                                /* Columns 9 and 10 are copies of Column 8 */
                                bit = 0x8000 >> 8;
                                prevbit = 0x8000 >> 7;
                            } else if (col > 0) { /* Stretch pixels */
                                prevbit = 0x8000 >> (col-1);
                            }

                            if (row_pixels & bit || row_pixels & prevbit) {
                                SetPixel(hMDC, col + offset, row,
                                    pixelValue);
                            }
                        } else if (row_pixels & bit) {
                            /* Not VT220 and not VT320. Don't do any weird
                             * tweaks - just render as is */
                            SetPixel(hMDC, col + offset, row,
                                pixelValue);
                        }
                    }
                }
            }

            SelectObject(hMDC, hOldBmp);
            DeleteDC(hMDC);
            if (fonts[i] != NULL) {
                DeleteObject(fonts[i]);
                fonts[i] = NULL;
            }
            if (stretchedFonts[i] != NULL) {
                DeleteObject(stretchedFonts[i]);
                stretchedFonts[i] = NULL;
            }
            fonts[i] = hbmp;
            serials[i] = drcsbuf[i]->serial;
            renderHints[i] = drcsbuf[i]->render_hints;
            currentRenditions[i] = rendition;
            stretchSoftFont(i);
        } else if (drcsbuf[i] == NULL) {
            debug(F101, "KClient::refreshSoftFonts - buffer is null", 0, i);
            // drcs font buffer has been *completely* erased.
            serials[i] = 0;
            currentRenditions[i] = -1;
            renderHints[i] = DRCS_RENDER_HINT_NONE;
            DeleteObject(fonts[i]);
            fonts[i] = NULL;
            DeleteObject(stretchedFonts[i]);
            stretchedFonts[i] = NULL;
        }
    }
    LeaveDRCSBufferCriticalSection();
}


/*------------------------------------------------------------------------
Stretches the original soft-font bitmap representation to match the
currently selected font size.
------------------------------------------------------------------------*/
// Parameters:
//      fontId: font to stretch, or 0 to stretch all fonts.
void KSoftFont::stretchSoftFont(int fontId) {
    int maxFont = DRCS_BUFFERS - 1;
    if (fontId != 0) maxFont = fontId;

    if (width == 0 || height == 0) return;

    EnterDRCSBufferCriticalSection();
    for (int i = fontId; i <= maxFont; i++) {
        if (fonts[i] == NULL) {
            debug(F101, "KClient::stretchSoftFont - buffer is null", 0, i);
            continue;
        }
        debug(F101, "KClient::stretchSoftFont - buffer", 0, i);

        BITMAP bitmapInfo;
        GetObject(fonts[i], sizeof(BITMAP), &bitmapInfo);

        HDC hDC = GetDC(NULL);

        HDC hMDCsrc = CreateCompatibleDC(hDC);
        HBITMAP hOldSrcBmp = (HBITMAP)SelectObject(hMDCsrc, fonts[i]);

        HDC hMDCdest = CreateCompatibleDC(hDC);
        HBITMAP hbmp = CreateBitmap(width * 96, height, 1, 1, NULL);
        HBITMAP hOldDestBmp = (HBITMAP)SelectObject(hMDCdest, hbmp);

        int display_height = bitmapInfo.bmHeight;
        int display_width = drcsbuf[i]->renditions[currentRenditions[i]]->cell_width;
        if (drcsbuf[i]->render_hints & DRCS_RENDER_HINT_VT220) {
            /* To account for VT220 glyphs being stretched horizontally one
             * pixel into the next cell, which we don't really do. */
            display_width += 1;
        }

        SetStretchBltMode(hMDCdest, BLACKONWHITE);

        StretchBlt(hMDCdest, 0, 0, width * 96, height,
                   hMDCsrc,  0, 0, display_width * 96,
                   display_height, SRCCOPY);

        ReleaseDC(NULL, hDC);
        SelectObject(hMDCdest, hOldDestBmp);
        SelectObject(hMDCsrc, hOldSrcBmp);
        DeleteDC(hMDCdest);
        DeleteDC(hMDCsrc);
        if (stretchedFonts[i] != NULL) {
            DeleteObject(stretchedFonts[i]);
            stretchedFonts[i] = NULL;
        }
        stretchedFonts[i] = hbmp;
    }
    LeaveDRCSBufferCriticalSection();

    stretchedWidth = width;
    stretchedHeight = height;
}


/*------------------------------------------------------------------------
Paints the supplied text using the specified soft-font
------------------------------------------------------------------------*/
/* 
 * Parameters:
 *      hdc: Device context to paint to
 *        x: X coordinate for top left corner
 *        y: Y coordinate for top left corner
 *      str: String to paint
 *   length: Length of string to paint
 *     font: Font to use - either 1 or 2
 */
void KSoftFont::textOut(HDC hdc, int x, int y, LPCWSTR str,
    unsigned int length, int font) {

    int fontIdx = font - 1;

    if (font < 1 || font > DRCS_BUFFERS) return;
    if (stretchedFonts[fontIdx] == NULL) return;

    // Paint the characters one at a time by copying from the font
    HDC hMDC = CreateCompatibleDC(hdc);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hMDC, stretchedFonts[fontIdx]);
    DeleteObject(hOldBmp);
    int font_start = DRCS_START(font);;
    int render_hints = renderHints[fontIdx];

    for (int i = 0; i < length; i++) {
        int glyph = str[i] - font_start;

        /* The VT220s centering effectively draws glyphs offset by
         * 1 from the grid, so the 10th column gets rendered in the
         * first column of the next cell to the right. The VT220
         * does this for all fonts, but we'll only do this for fonts
         * that don't use columns 8+, so that drawing soft-fonts can
         * still line up with regular non-soft-fonts that aren't
         * offset like this. */

        BitBlt(hdc,
               x + i * width
                    + (render_hints & DRCS_RENDER_HINT_VT220_TEXT ? 1 : 0),
               y,
               width+1,
               height,
               hMDC,
               glyph * width, 
               0,
               SRCCOPY);
    }
    SelectObject(hMDC, hOldBmp);
    DeleteDC(hMDC);
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL KSoftFont::fontValid(int fontId) const {
    return stretchedFonts[fontId-1] != NULL;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
HBITMAP KSoftFont::getFontBitmap(int fontId, BOOL stretched) {
    if (stretched) {
        return stretchedFonts[fontId-1];
    }
    return fonts[fontId-1];
}
