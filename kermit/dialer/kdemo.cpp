#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#include "dialer.hpp"
#include "kdemo.hpp"

K_DEMO::K_DEMO(int export) 
   :UIW_WINDOW(export?"DIALOG_EXPORT_DEMO":"DIALOG_DEMO",defaultStorage)
{ 
   UIW_STRING * string = NULL ;
   windowManager->Center(this) ;
};
