//	HELLO1.CPP (HELLO) - Simple "Hello, World!" in a window.
//	COPYRIGHT (C) 1990-1995.  All Rights Reserved.
//	Zinc Software Incorporated.  Pleasant Grove, Utah  USA
//  May be freely copied, used and distributed.

#include <ui_win.hpp>

//static ZIL_ICHAR ZIL_FARDATA _i18nName[] = ZIL_TEXT("xxx.dat");
//ZIL_ICHAR *ZIL_I18N::i18nName = _i18nName;
//static ZIL_ICHAR title[] = ZIL_TEXT("Hello World Window");
//static ZIL_ICHAR text[] = ZIL_TEXT("Hello, World!");
static ZIL_ICHAR title[] = { 'H','e','l','l','o',' ','W','o','r','l','d',' ','W','i','n','d','o','w',0 };
static ZIL_ICHAR text[] = { 'H','e','l','l','o',',',' ','W','o','r','l','d','!',0 };

int UI_APPLICATION::Main(void)
{
	// The UI_APPLICATION constructor automatically initializes the
	// display, eventManager, and windowManager variables.

	// This line fixes linkers that don't look for main in the .LIBs.
	UI_APPLICATION::LinkMain();

	// Create the hello world window.
	UIW_WINDOW *window = new UIW_WINDOW(5, 5, 40, 6);

	// Add the window objects to the window.
	*window
		+ new UIW_BORDER
		+ new UIW_MAXIMIZE_BUTTON
		+ new UIW_MINIMIZE_BUTTON
		+ new UIW_SYSTEM_BUTTON(SYF_GENERIC)
		+ new UIW_TITLE(title)
		+ new UIW_TEXT(0, 0, 0, 0, text, 256, WNF_NO_FLAGS, WOF_NON_FIELD_REGION);

	// Add the window to the window manager.
	*windowManager + window;

	// Process user responses.
	UI_APPLICATION::Control();

	// Clean up.
	return (0);
}

