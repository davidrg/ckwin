#ifndef ktoolbar_hxx_included
#define ktoolbar_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: ktoolbar.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: main window's toolbar.

========================================================================*/

#include "kwin.hxx"

typedef struct _ToolBitmapDef {
#ifndef NOTOOLBAR
    TBBUTTON tbbutton;
#endif
    char* helptext;
} ToolBitmapDef;

class KToolBar : public KWin
{
public:
    KToolBar( K_GLOBAL*, int resid );
    ~KToolBar();

    void createWin( KWin* par );
    void size( int width, int height );

    Bool message( HWND hpar, UINT msg, WPARAM wParam, LPARAM lParam );

    void initButtons( int numbut, int numbit, ToolBitmapDef* );
    void createTermTypeCombo();
    void createFontNameCombo();
    void createFontHeightCombo();
    void createCharsetCombo();
    WNDPROC getComboProc() { return comboProc; }
    void setTermType( char* tt );
    void setCharset( char* cs );
    void setFontName( char * fnt );
    void setFontHeight( int );

protected:
#if defined(_MSC_VER) && _MSC_VER < 1300
    char* findBubbleHelp( UINT idx );
#else
    char* findBubbleHelp( UINT_PTR idx );
#endif

private:
    ToolBitmapDef* buttonDefs;
    int numButtons;
    int numBitmaps;
    int toolbarid;
    int disabled;

    HWND hWndTermTypeCombo;
    HWND hWndFontNameCombo;
    HWND hWndFontHeightCombo;
    HWND hWndCharsetCombo;
    WNDPROC comboProc;
    HFONT comboFont;
};

#endif
