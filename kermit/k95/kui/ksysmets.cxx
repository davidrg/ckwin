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
    _screenWidth = GetSystemMetrics( SM_CXSCREEN );
    _screenHeight = GetSystemMetrics( SM_CYSCREEN );

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
