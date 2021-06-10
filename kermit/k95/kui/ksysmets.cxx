#include "ksysmets.hxx"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KSysMetrics::KSysMetrics()
{
    reset();

    OSVERSIONINFO verinfo;
    verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx( &verinfo );

    switch( verinfo.dwPlatformId )
    {
    case VER_PLATFORM_WIN32s:			// Win32s on Windows 3.1 
        _platform = OS_WIN31;
        break;
    case VER_PLATFORM_WIN32_WINDOWS:	// Win32 on Windows 95
        _platform = OS_WIN95;
        break;
    case VER_PLATFORM_WIN32_NT:			// Windows NT 
        switch (verinfo.dwMajorVersion) {
        case 6:
            _platform = OS_NT6;
            break;
        case 5:
            _platform = OS_NT5;
            break;
        case 4:
            _platform = OS_NT4;
            break;
        case 3:
            _platform = OS_NT3;
            break;
        default:
            _platform = OS_NOTSUPPORTED;
            break;
        }           
        break;
    default:
        _platform = OS_NOTSUPPORTED;
        break;
    }
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
