#include <ui_win.hpp>
#define USE_DERIVED_OBJECTS
#include "k95cinit.hpp"
void z_jump_dummy(void) { }   // Bug fix for broken linkers.

static UI_ITEM _userTable[] =
{
	{ ID_END, ZIL_NULLP(void), ZIL_NULLP(ZIL_ICHAR), 0 }
};
UI_ITEM *UI_WINDOW_OBJECT::userTable = _userTable;

static UI_ITEM _objectTable[] =
{
	{ ID_BORDER, ZIL_VOIDF(UIW_BORDER::New), UIW_BORDER::_className, 0 },
	{ ID_PROMPT, ZIL_VOIDF(UIW_PROMPT::New), UIW_PROMPT::_className, 0 },
	{ ID_STRING, ZIL_VOIDF(UIW_STRING::New), UIW_STRING::_className, 0 },
	{ ID_SYSTEM_BUTTON, ZIL_VOIDF(UIW_SYSTEM_BUTTON::New), UIW_SYSTEM_BUTTON::_className, 0 },
	{ ID_TITLE, ZIL_VOIDF(UIW_TITLE::New), UIW_TITLE::_className, 0 },
	{ ID_WINDOW, ZIL_VOIDF(UIW_WINDOW::New), UIW_WINDOW::_className, 0 },
	{ ID_END, ZIL_NULLP(void), ZIL_NULLP(ZIL_ICHAR), 0 }
};
UI_ITEM *UI_WINDOW_OBJECT::objectTable = _objectTable;

