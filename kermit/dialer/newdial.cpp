#include <ui_win.hpp>
#define USE_DERIVED_OBJECTS
#include "newdial.hpp"
void z_jump_dummy(void) { }   // Bug fix for broken linkers.

static UI_ITEM _userTable[] =
{
	{ ID_END, ZIL_NULLP(void), ZIL_NULLP(ZIL_ICHAR), 0 }
};
UI_ITEM *UI_WINDOW_OBJECT::userTable = _userTable;

static UI_ITEM _objectTable[] =
{
	{ ID_BORDER, ZIL_VOIDF(UIW_BORDER::New), UIW_BORDER::_className, 0 },
	{ ID_BUTTON, ZIL_VOIDF(UIW_BUTTON::New), UIW_BUTTON::_className, 0 },
	{ ID_COMBO_BOX, ZIL_VOIDF(UIW_COMBO_BOX::New), UIW_COMBO_BOX::_className, 0 },
	{ ID_DATE, ZIL_VOIDF(UIW_DATE::New), UIW_DATE::_className, 0 },
	{ ID_FORMATTED_STRING, ZIL_VOIDF(UIW_FORMATTED_STRING::New), UIW_FORMATTED_STRING::_className, 0 },
	{ ID_GROUP, ZIL_VOIDF(UIW_GROUP::New), UIW_GROUP::_className, 0 },
	{ ID_ICON, ZIL_VOIDF(UIW_ICON::New), UIW_ICON::_className, 0 },
	{ ID_INTEGER, ZIL_VOIDF(UIW_INTEGER::New), UIW_INTEGER::_className, 0 },
	{ ID_MAXIMIZE_BUTTON, ZIL_VOIDF(UIW_MAXIMIZE_BUTTON::New), UIW_MAXIMIZE_BUTTON::_className, 0 },
	{ ID_MINIMIZE_BUTTON, ZIL_VOIDF(UIW_MINIMIZE_BUTTON::New), UIW_MINIMIZE_BUTTON::_className, 0 },
	{ ID_NOTEBOOK, ZIL_VOIDF(UIW_NOTEBOOK::New), UIW_NOTEBOOK::_className, 0 },
	{ ID_POP_UP_ITEM, ZIL_VOIDF(UIW_POP_UP_ITEM::New), UIW_POP_UP_ITEM::_className, 0 },
	{ ID_POP_UP_MENU, ZIL_VOIDF(UIW_POP_UP_MENU::New), UIW_POP_UP_MENU::_className, 0 },
	{ ID_PROMPT, ZIL_VOIDF(UIW_PROMPT::New), UIW_PROMPT::_className, 0 },
	{ ID_PULL_DOWN_ITEM, ZIL_VOIDF(UIW_PULL_DOWN_ITEM::New), UIW_PULL_DOWN_ITEM::_className, 0 },
	{ ID_PULL_DOWN_MENU, ZIL_VOIDF(UIW_PULL_DOWN_MENU::New), UIW_PULL_DOWN_MENU::_className, 0 },
	{ ID_REAL, ZIL_VOIDF(UIW_REAL::New), UIW_REAL::_className, 0 },
	{ ID_SCROLL_BAR, ZIL_VOIDF(UIW_SCROLL_BAR::New), UIW_SCROLL_BAR::_className, 0 },
	{ ID_SPIN_CONTROL, ZIL_VOIDF(UIW_SPIN_CONTROL::New), UIW_SPIN_CONTROL::_className, 0 },
	{ ID_STATUS_BAR, ZIL_VOIDF(UIW_STATUS_BAR::New), UIW_STATUS_BAR::_className, 0 },
	{ ID_STRING, ZIL_VOIDF(UIW_STRING::New), UIW_STRING::_className, 0 },
	{ ID_SYSTEM_BUTTON, ZIL_VOIDF(UIW_SYSTEM_BUTTON::New), UIW_SYSTEM_BUTTON::_className, 0 },
	{ ID_TEXT, ZIL_VOIDF(UIW_TEXT::New), UIW_TEXT::_className, 0 },
	{ ID_TIME, ZIL_VOIDF(UIW_TIME::New), UIW_TIME::_className, 0 },
	{ ID_TITLE, ZIL_VOIDF(UIW_TITLE::New), UIW_TITLE::_className, 0 },
	{ ID_TOOL_BAR, ZIL_VOIDF(UIW_TOOL_BAR::New), UIW_TOOL_BAR::_className, 0 },
	{ ID_VT_LIST, ZIL_VOIDF(UIW_VT_LIST::New), UIW_VT_LIST::_className, 0 },
	{ ID_WINDOW, ZIL_VOIDF(UIW_WINDOW::New), UIW_WINDOW::_className, 0 },
	{ ID_GEOMETRY_MANAGER, ZIL_VOIDF(UI_GEOMETRY_MANAGER::New), UI_GEOMETRY_MANAGER::_className, 0 },
	{ ID_ATTACHMENT, ZIL_VOIDF(UI_ATTACHMENT::New), UI_ATTACHMENT::_className, 0 },
	{ ID_END, ZIL_NULLP(void), ZIL_NULLP(ZIL_ICHAR), 0 }
};
UI_ITEM *UI_WINDOW_OBJECT::objectTable = _objectTable;

