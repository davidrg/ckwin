#ifdef COMMENT
#undef COMMENT
#endif

EVENT_TYPE AddPopUpMenu(UI_WINDOW_OBJECT *object, UI_EVENT &, EVENT_TYPE ccode)
{
    if (ccode != L_SELECT)
	return (0);
    // Array of UI_ITEMS used to initialize the pop-up menu
    UI_ITEM menuItems[]=
    {
	{S_CLOSE_TEMPORARY, ZIL_NULLP(void), "Cancel",MNIF_SEND_MESSAGE},
	{0, ZIL_NULLP(void), "", MNIF_SEPARATOR},
	{L_EXIT_FUNCTION, ZIL_NULLP(void), "Exit",MNIF_SEND_MESSAGE},
	{0, CloseWindow, "Close", MNIF_SEND_MESSAGE},
	{0, ZIL_NULLP(void), 0, 0}
     };
    UIW_POP_UP_MENU *menu = new UIW_POP_UP_MENU(object->true.right, object->
						 true.bottom, WNF_NO_FLAGS, menuItems);
    menu->woStatus |= WOS_GRAPHICS;
    menu->woAdvancedFlags |= WOAF_TEMPORARY;
    *object->windowManager
	 + menu;
    return (0);
}