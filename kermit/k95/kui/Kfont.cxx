#include "kfont.hxx"
#include "karray.hxx"
#include "ikextern.h"

typedef struct _KFONT_HOLDER {
    PLOGFONT logfont;
    long totdim;
    long InternalLeading;
    HFONT hfont;
    HFONT hfontBold;
    HFONT hfontDim;
    HFONT hfontUnderline;
    HFONT hfontBoldUnderline;
    HFONT hfontBoldItalic;
    HFONT hfontDimUnderline;
    HFONT hfontDimItalic;
    HFONT hfontUnderlineItalic;
    HFONT hfontBoldUnderlineItalic;
    HFONT hfontDimUnderlineItalic;
    HFONT hfontItalic;
} KFONT_HOLDER;

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KFont::KFont( PLOGFONT plog )
{
    fontW = fontH = 0;
    holder = new KFONT_HOLDER;
    memset( holder, '\0', sizeof(KFONT_HOLDER) );

    PLOGFONT logfont = new LOGFONT;

    if( plog ) {
        memcpy( logfont, plog, sizeof(LOGFONT) );
    }
    else {
        memset( logfont, '\0', sizeof(LOGFONT) );
        logfont->lfWeight = FW_MEDIUM;
        logfont->lfCharSet = 1 /* Default = 1 */;
        logfont->lfOutPrecision = 31;
        logfont->lfClipPrecision = 2;
        logfont->lfQuality = 1;
        logfont->lfPitchAndFamily = 49;
//        logfont->lfWidth = 0;
        HDC hDC = CreateDC("DISPLAY",NULL,NULL,NULL);
        logfont->lfWidth = 
            -MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSX), 72 * 2);
        logfont->lfHeight = 
            -MulDiv(12, GetDeviceCaps(hDC, LOGPIXELSY), 72);
        DeleteDC(hDC);

        if (ntermfont == 0)
            BuildFontTable(&term_font, &_term_font, &ntermfont);

        char * names[] = {
            "Andale Mono WT J",
            "Andale Mono 64",
            "Bitstream Vera Sans Mono",
            "Everson Mono Terminal",
            "Monotype.com",
            "Andale Mono",
            "Lucida Console",
            "Courier",
            "Terminal",
            "System",
            0
        };
        int i=0,j=0;

        do {
            for( i = 0; i < ntermfont; i++ )
            {
                if ( !strcmp(term_font[i].kwd,names[j]) )
                    break;
            }
        } while ( i == ntermfont && names[++j] );

        if (i < ntermfont )
            ckstrncpy( logfont->lfFaceName, term_font[i].kwd, 32 );
    }
    holder->logfont = logfont;
    debug(F110,"KFont() logfont->lfFaceName",logfont->lfFaceName,0);
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KFont::KFont( char * facename, int height, int width )
{
    fontW = fontH = 0;
    holder = new KFONT_HOLDER;
    memset( holder, '\0', sizeof(KFONT_HOLDER) );

    PLOGFONT logfont = new LOGFONT;

    memset( logfont, '\0', sizeof(LOGFONT) );
    logfont->lfWeight = FW_MEDIUM;
    logfont->lfCharSet = 1 /* Default = 1 */;
    logfont->lfOutPrecision = 31;
    logfont->lfClipPrecision = 2;
    logfont->lfQuality = 1;
    logfont->lfPitchAndFamily = 49;
    HDC hDC = CreateDC("DISPLAY",NULL,NULL,NULL);
    logfont->lfWidth = 
        -MulDiv(width, GetDeviceCaps(hDC, LOGPIXELSX), 72 * 2);
    logfont->lfHeight = 
        -MulDiv(height, GetDeviceCaps(hDC, LOGPIXELSY), 72 * 2);
    setFont(hDC);       /* so we can compute InternalLeading */
    DeleteDC(hDC);
    ckstrncpy( logfont->lfFaceName, facename, 32 );

    holder->logfont = logfont;
    debug(F110,"KFont(facename,height,width) logfont->lfFaceName",logfont->lfFaceName,0);
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KFont::~KFont()
{
    if( holder->hfont )
        DeleteObject( holder->hfont );
    if( holder->hfontBold )
        DeleteObject( holder->hfontBold );
    if( holder->hfontDim )
        DeleteObject( holder->hfontDim );
    if( holder->hfontUnderline )
        DeleteObject( holder->hfontUnderline );
    if( holder->hfontBoldUnderline )
        DeleteObject( holder->hfontBoldUnderline );
    if( holder->hfontBoldItalic )
        DeleteObject( holder->hfontBoldItalic );
    if( holder->hfontDimUnderline )
        DeleteObject( holder->hfontDimUnderline );
    if( holder->hfontDimItalic )
        DeleteObject( holder->hfontDimItalic );
    if( holder->hfontItalic )
        DeleteObject( holder->hfontItalic );
    if( holder->hfontUnderlineItalic )
        DeleteObject( holder->hfontUnderlineItalic );
    if( holder->hfontBoldUnderlineItalic )
        DeleteObject( holder->hfontBoldUnderlineItalic );
    if( holder->hfontDimUnderlineItalic )
        DeleteObject( holder->hfontDimUnderlineItalic );

    delete holder->logfont;
    delete holder;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
PLOGFONT KFont::getLogFont()
{
    return holder->logfont;
}

long KFont::getInternalLeading()
{
    return holder->InternalLeading;
}

int KFont::getFontPointsW() { 
    int w;

    HDC hDC = CreateDC("DISPLAY",NULL,NULL,NULL);
    if ( holder->logfont->lfWidth < 0 )
        w = -MulDiv(2 * holder->logfont->lfWidth, 72, GetDeviceCaps(hDC, LOGPIXELSX));
    else
        w = MulDiv(2 * holder->logfont->lfWidth, 72, GetDeviceCaps(hDC, LOGPIXELSX));
    DeleteDC(hDC);

    return w;
}

int KFont::getFontPointsH() { 
    int h;
    HDC hDC = CreateDC("DISPLAY",NULL,NULL,NULL);

    if ( holder->logfont->lfHeight < 0 )
        h = -MulDiv(2 * (holder->logfont->lfHeight), 72, GetDeviceCaps(hDC, LOGPIXELSY));
    else
        h = MulDiv(2 * (holder->logfont->lfHeight - holder->InternalLeading), 72, 
                    GetDeviceCaps(hDC, LOGPIXELSY));
    DeleteDC(hDC);

    return h; 
}

    /*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFont::setFont( HDC hdc )
{
    HFONT htemp = CreateFontIndirect( holder->logfont );
    SelectObject( hdc, htemp );
    if( holder->hfont ) {
        DeleteObject( holder->hfont );
    }
    holder->hfont = htemp;
    if ( holder->hfontBold ) {
        DeleteObject( holder->hfontBold );
        holder->hfontBold = NULL;
    }
    if ( holder->hfontUnderline ) {
        DeleteObject( holder->hfontUnderline );
        holder->hfontUnderline = NULL;
    }
    if ( holder->hfontBoldUnderline ) {
        DeleteObject( holder->hfontBoldUnderline );
        holder->hfontBoldUnderline = NULL;
    }

    if( holder->hfontDim ) {
        DeleteObject( holder->hfontDim );
        holder->hfontDim = NULL;
    }
    if( holder->hfontBoldItalic ) {
        DeleteObject( holder->hfontBoldItalic );
        holder->hfontBoldItalic = NULL;
    }
    if( holder->hfontDimUnderline ) {
        DeleteObject( holder->hfontDimUnderline );
        holder->hfontDimUnderline = NULL;
    }
    if( holder->hfontDimItalic ) {
        DeleteObject( holder->hfontDimItalic );
        holder->hfontDimItalic = NULL;
    }
    if( holder->hfontItalic ) {
        DeleteObject( holder->hfontItalic );
        holder->hfontItalic = NULL;
    }
    if( holder->hfontUnderlineItalic ) {
        DeleteObject( holder->hfontUnderlineItalic );
        holder->hfontUnderlineItalic = NULL;
    }
    if( holder->hfontBoldUnderlineItalic ) {
        DeleteObject( holder->hfontBoldUnderlineItalic );
        holder->hfontBoldUnderlineItalic = NULL;
    }
    if( holder->hfontDimUnderlineItalic ) {
        DeleteObject( holder->hfontDimUnderlineItalic );
        holder->hfontDimUnderlineItalic = NULL;
    }

    SIZE size;
    GetTextExtentPoint32( hdc, "_", 1, &size );
    fontW = size.cx;    // fixed width fonts!
    fontH = size.cy;

    TEXTMETRIC tm;
    GetTextMetrics( hdc, &tm );
    holder->InternalLeading = tm.tmInternalLeading;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFont::resetFont( HDC hdc )
{
    SelectObject( hdc, holder->hfont );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFont::setBold( HDC hdc )
{
    if( !holder->hfontBold )
    {
        LOGFONT tmplogfont;
        memcpy( &tmplogfont, holder->logfont, sizeof(LOGFONT) );
        tmplogfont.lfWeight = FW_BOLD;
        holder->hfontBold = CreateFontIndirect( &tmplogfont );
    }
    SelectObject( hdc, holder->hfontBold );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFont::setDim( HDC hdc )
{
    if( !holder->hfontDim )
    {
        LOGFONT tmplogfont;
        memcpy( &tmplogfont, holder->logfont, sizeof(LOGFONT) );
        tmplogfont.lfWeight = FW_BOLD;
        holder->hfontDim = CreateFontIndirect( &tmplogfont );
    }
    SelectObject( hdc, holder->hfontDim );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFont::setUnderline( HDC hdc )
{
    if( !holder->hfontUnderline )
    {
        LOGFONT tmplogfont;
        memcpy( &tmplogfont, holder->logfont, sizeof(LOGFONT) );
        tmplogfont.lfWeight = FW_NORMAL;
        tmplogfont.lfUnderline = TRUE;
        holder->hfontUnderline = CreateFontIndirect( &tmplogfont );
    }
    SelectObject( hdc, holder->hfontUnderline );
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFont::setBoldUnderline( HDC hdc )
{
    if( !holder->hfontBoldUnderline )
    {
        LOGFONT tmplogfont;
        memcpy( &tmplogfont, holder->logfont, sizeof(LOGFONT) );
        tmplogfont.lfWeight = FW_BOLD;
        tmplogfont.lfUnderline = TRUE;
        holder->hfontBoldUnderline = CreateFontIndirect( &tmplogfont );
    }
    SelectObject( hdc, holder->hfontBoldUnderline );
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFont::setBoldItalic( HDC hdc )
{
    if( !holder->hfontBoldItalic )
    {
        LOGFONT tmplogfont;
        memcpy( &tmplogfont, holder->logfont, sizeof(LOGFONT) );
        tmplogfont.lfWeight = FW_BOLD;
        tmplogfont.lfItalic = TRUE;
        holder->hfontBoldItalic = CreateFontIndirect( &tmplogfont );
    }
    SelectObject( hdc, holder->hfontBoldItalic );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFont::setDimUnderline( HDC hdc )
{
    if( !holder->hfontDimUnderline )
    {
        LOGFONT tmplogfont;
        memcpy( &tmplogfont, holder->logfont, sizeof(LOGFONT) );
        tmplogfont.lfWeight = FW_BOLD;
        tmplogfont.lfUnderline = TRUE;
        holder->hfontDimUnderline = CreateFontIndirect( &tmplogfont );
    }
    SelectObject( hdc, holder->hfontDimUnderline );
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFont::setDimItalic( HDC hdc )
{
    if( !holder->hfontDimItalic )
    {
        LOGFONT tmplogfont;
        memcpy( &tmplogfont, holder->logfont, sizeof(LOGFONT) );
        tmplogfont.lfWeight = FW_BOLD;
        tmplogfont.lfItalic = TRUE;
        holder->hfontDimItalic = CreateFontIndirect( &tmplogfont );
    }
    SelectObject( hdc, holder->hfontDimItalic );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFont::setUnderlineItalic( HDC hdc )
{
    if( !holder->hfontUnderlineItalic )
    {
        LOGFONT tmplogfont;
        memcpy( &tmplogfont, holder->logfont, sizeof(LOGFONT) );
        tmplogfont.lfWeight = FW_NORMAL;
        tmplogfont.lfUnderline = TRUE;
        tmplogfont.lfItalic = TRUE;
        holder->hfontUnderlineItalic = CreateFontIndirect( &tmplogfont );
    }
    SelectObject( hdc, holder->hfontUnderlineItalic );
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFont::setBoldUnderlineItalic( HDC hdc )
{
    if( !holder->hfontBoldUnderlineItalic )
    {
        LOGFONT tmplogfont;
        memcpy( &tmplogfont, holder->logfont, sizeof(LOGFONT) );
        tmplogfont.lfWeight = FW_BOLD;
        tmplogfont.lfUnderline = TRUE;
        tmplogfont.lfItalic = TRUE;
        holder->hfontBoldUnderlineItalic = CreateFontIndirect( &tmplogfont );
    }
    SelectObject( hdc, holder->hfontBoldUnderlineItalic );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFont::setDimUnderlineItalic( HDC hdc )
{
    if( !holder->hfontDimUnderlineItalic )
    {
        LOGFONT tmplogfont;
        memcpy( &tmplogfont, holder->logfont, sizeof(LOGFONT) );
        tmplogfont.lfWeight = FW_BOLD;
        tmplogfont.lfUnderline = TRUE;
        tmplogfont.lfItalic = TRUE;
        holder->hfontDimUnderlineItalic = CreateFontIndirect( &tmplogfont );
    }
    SelectObject( hdc, holder->hfontDimUnderlineItalic );
}
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFont::setItalic( HDC hdc )
{
    if( !holder->hfontItalic )
    {
        LOGFONT tmplogfont;
        tmplogfont.lfWeight = FW_NORMAL;
        memcpy( &tmplogfont, holder->logfont, sizeof(LOGFONT) );
        tmplogfont.lfItalic = TRUE;
        holder->hfontItalic = CreateFontIndirect( &tmplogfont );
    }
    SelectObject( hdc, holder->hfontItalic );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KFont::tryFont( int w, int h, HDC hdc )
{
    if( holder->logfont->lfWidth == w && holder->logfont->lfHeight == h )
        return FALSE;

    holder->logfont->lfWidth = w;
    holder->logfont->lfHeight = h;
    if( hdc )
        setFont( hdc );
    return TRUE;
}

void KFont::displayABCwidths( HDC hdc )
{
#ifdef FONT_DEBUG
    ABC abc;
    unsigned int firstchar = 0;

    FILE * file = fopen("abcwidth","w+");

    for ( firstchar = 0; firstchar < 65536; firstchar++ ) 
    {

        if (!GetCharABCWidthsW(hdc, firstchar, firstchar, &abc)) {
            DWORD gle = GetLastError();
            fprintf(file,"error (%d) on glyph %d\r\n", gle, firstchar);
            continue;
        } 

#ifdef COMMENT
        if ( abc.abcA < 0 && abc.abcC < 0 )
        {
            fprintf(file,"glyph %d (U+%X) both A and C negative (%d,%d)\r\n",
                     firstchar,firstchar,abc.abcA,abc.abcC);
        } 
        else if ( abc.abcA < 0 )
        {
            fprintf(file,"glyph %d (0x%x) only A negative (%d)\r\n",
                     firstchar,firstchar,abc.abcA);
        } 
        else if ( abc.abcC < 0 )
        {
            fprintf(file,"glyph %d (0x%x) only C negative (%d)\r\n",
                     firstchar,firstchar,abc.abcC);
        }
#else
        fprintf(file,"glyph U%05d (U+%04X)  %3d  %3d  %3d\r\n",
                 firstchar,firstchar,abc.abcA,abc.abcB,abc.abcC);
#endif /* COMMENT */
    }
    fclose(file);
#endif /* FONT_DEBUG */
}

char * KFont::getFaceName()
{
    return(holder->logfont->lfFaceName);
}
