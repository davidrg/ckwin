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

    _edgeWidth = GetSystemMetrics( SM_CXEDGE );
    _edgeHeight = GetSystemMetrics( SM_CYEDGE );

    _sizeframeWidth = GetSystemMetrics( SM_CXSIZEFRAME );
    _sizeframeHeight = GetSystemMetrics( SM_CYSIZEFRAME );

    _menuHeight = GetSystemMetrics( SM_CYMENU );
    _captionHeight = GetSystemMetrics( SM_CYCAPTION );

    _borderHeight = GetSystemMetrics( SM_CYBORDER );
    _borderWidth = GetSystemMetrics( SM_CXBORDER );

    _iconWidth = GetSystemMetrics( SM_CXICON );
    _iconHeight = GetSystemMetrics( SM_CYICON );
}
