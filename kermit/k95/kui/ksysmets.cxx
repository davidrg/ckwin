#include "ksysmets.hxx"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KSysMetrics::KSysMetrics()
{
    reset();
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KSysMetrics::~KSysMetrics()
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KSysMetrics::reset()
{
    /* SM_CXVIRTUALSCREEN only exists in Visual C++ 6.0 and newer */
#ifndef SM_CXVIRTUALSCREEN
    _screenWidth = GetSystemMetrics( SM_CXSCREEN );
    _screenHeight = GetSystemMetrics( SM_CYSCREEN );
#else
    _screenWidth = GetSystemMetrics( SM_CXVIRTUALSCREEN );
    _screenHeight = GetSystemMetrics( SM_CYVIRTUALSCREEN );
    
    /* SM_CXVIRTUALSCREEN and SM_CYVIRTUALSCREEN are new to Windows 98.
     * So check for a sensible value and if we don't get one, fall back
     * to using the dimensions of the primary display */
    if (_screenWidth < 1) {
        _screenWidth = GetSystemMetrics( SM_CXSCREEN );
    }
    if (_screenHeight < 1) {
        _screenHeight = GetSystemMetrics( SM_CYSCREEN );
    }
#endif

    _vscrollWidth = GetSystemMetrics( SM_CXVSCROLL );
    _hscrollHeight = GetSystemMetrics( SM_CYHSCROLL );

#ifndef CKT_NT35_OR_31
    _edgeWidth = GetSystemMetrics( SM_CXEDGE );
    _edgeHeight = GetSystemMetrics( SM_CYEDGE );

    _sizeframeWidth = GetSystemMetrics( SM_CXSIZEFRAME );
    _sizeframeHeight = GetSystemMetrics( SM_CYSIZEFRAME );
#else
    /* SM_CXEDGE, SMCYEDGE, SM_CXSIZEFRAME, and SM_CYSIZEFRAME were introduced
     * in Windows 95 and are not known to Visual C++ 2.0 and earlier */
    _edgeWidth = 0;
    _edgeHeight = 0;
    _sizeframeWidth = 0;
    _sizeframeHeight = 0;
#endif /* CKT_NT35_OR_31 */

    _menuHeight = GetSystemMetrics( SM_CYMENU );
    _captionHeight = GetSystemMetrics( SM_CYCAPTION );

    _borderHeight = GetSystemMetrics( SM_CYBORDER );
    _borderWidth = GetSystemMetrics( SM_CXBORDER );

    _iconWidth = GetSystemMetrics( SM_CXICON );
    _iconHeight = GetSystemMetrics( SM_CYICON );
}
