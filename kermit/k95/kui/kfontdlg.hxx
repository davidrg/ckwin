#ifndef kfontdlg_hxx_included
#define kfontdlg_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kfontdlg.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: font dialog box.

========================================================================*/

#include "kwin.hxx"

class KFont;
class KFontDialog : public KWin
{
public:
    KFontDialog( K_GLOBAL*, KWin* par );
    ~KFontDialog();

    void show( PLOGFONT );
    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

protected:
    Bool doCommand( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl );
};

#endif
