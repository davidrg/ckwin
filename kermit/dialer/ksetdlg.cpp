#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#include "kenum.hpp"
#include "ksetdlg.hpp"

ZIL_ICHAR K95_SETTINGS_DIALOG::_className[] = "K95_SETTINGS_DIALOG" ;

K95_SETTINGS_DIALOG::
K95_SETTINGS_DIALOG(void)
   : ZAF_DIALOG_WINDOW("K95_SETTINGS_DIALOG",defaultStorage),
   _cancelled(1), _mode(ADD_ENTRY)
{
}

K95_SETTINGS_DIALOG::
K95_SETTINGS_DIALOG(ZIL_ICHAR * name, enum ENTRYMODE mode)
   : ZAF_DIALOG_WINDOW(name,defaultStorage),
   _cancelled(1), _mode(mode)
{
}

