#ifndef ksysmets_hxx_included
#define ksysmets_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: ksysmets.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: March 10, 1996

    Description: System Metrics encapsulation

========================================================================*/
#include "kuidef.h"
enum ePlatform { OS_NOTSUPPORTED, OS_NT5, OS_NT4, OS_NT3, OS_WIN95, OS_WIN31 };
class KSysMetrics
{
public:
    KSysMetrics();
    ~KSysMetrics();

    void reset();

    int screenWidth() { return _screenWidth; }
    int screenHeight() { return _screenHeight; }
    int vscrollWidth() { return _vscrollWidth; }
    int hscrollHeight() { return _hscrollHeight; }
    int edgeWidth() { return _edgeWidth; }
    int edgeHeight() { return _edgeHeight; }
    int sizeframeWidth() { return _sizeframeWidth; }
    int sizeframeHeight() { return _sizeframeHeight; }
    int menuHeight() { return _menuHeight; }
    int captionHeight() { return _captionHeight; }
    int borderHeight() { return _borderHeight; }
    int borderWidth() { return _borderWidth; }
    int iconHeight() { return _iconHeight; }
    int iconWidth() { return _iconWidth; }
	ePlatform platform() { return _platform; }

private:
    int _screenWidth;
    int _screenHeight;
    int _vscrollWidth;
    int _hscrollHeight;
    int _edgeWidth;
    int _edgeHeight;
    int _sizeframeWidth;
    int _sizeframeHeight;
    int _menuHeight;
    int _captionHeight;
    int _borderHeight;
    int _borderWidth;
    int _iconHeight;
    int _iconWidth;
	ePlatform _platform;
};

#endif
