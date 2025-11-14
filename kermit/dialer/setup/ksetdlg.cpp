#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT 
#endif /* COMMENT */
#include <stdio.h>
#include "ksetdlg.hpp"

ZIL_ICHAR K_SETUP_DIALOG::_className[] = "K_SETUP_DIALOG";

K_SETUP_DIALOG::
K_SETUP_DIALOG(void) : ZAF_DIALOG_WINDOW("DIALOG_WELCOME",defaultStorage)
{
    next = 0;
}

K_SETUP_DIALOG::
K_SETUP_DIALOG( ZIL_ICHAR * WindowName ) :
    ZAF_DIALOG_WINDOW(WindowName,defaultStorage)
{
    next = 0;
}

K_SETUP_DIALOG::
~K_SETUP_DIALOG()
{
        
}
