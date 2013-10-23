#ifndef kfont_hxx_included
#define kfont_hxx_included
/*========================================================================
    Copyright (c) 1996, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kfont.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: January 13, 1996

    Description: a font used by main app window.

========================================================================*/
#include "kuidef.h"
#include "ikextern.h"

struct _KFONT_HOLDER;
class KFont
{
public:
    KFont( PLOGFONT = 0 );
    KFont( char * facename, int height = 12, int width = 0 );
    ~KFont();
    void setFont( HDC );

    PLOGFONT getLogFont();
    int getFontW() { return fontW; }
    int getFontH() { return fontH; }
    int getFontSpacedH() { return (int)(tt_linespacing[vmode] * fontH); }

    int getFontPointsW();
    int getFontPointsH();

    long getInternalLeading();

    Bool tryFont( int w, int h, HDC hdc );

    void resetFont( HDC hdc );
    void setBold( HDC hdc );
    void setDim( HDC hdc );
    void setUnderline( HDC hdc );
    void setBoldUnderline( HDC hdc );
    void setBoldItalic( HDC hdc );
    void setUnderlineItalic( HDC hdc );
    void setBoldUnderlineItalic( HDC hdc );
    void setItalic( HDC hdc );
    void setDimUnderline( HDC hdc );
    void setDimItalic( HDC hdc );
    void setDimUnderlineItalic( HDC hdc );

    void displayABCwidths( HDC hdc );

    char * getFaceName();

private:
    _KFONT_HOLDER* holder;
    int fontW;
    int fontH;
};

#endif
