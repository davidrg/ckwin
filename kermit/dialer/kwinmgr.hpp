#ifndef K_WINDOW_MGR
#define K_WINDOW_MGR

class K_WINDOW_MANAGER : public UI_WINDOW_MANAGER 
{
  public:
    K_WINDOW_MANAGER(UI_DISPLAY *display, UI_EVENT_MANAGER *eventManager,
		ZIL_EXIT_FUNCTION exitFunction = ZIL_NULLF(ZIL_EXIT_FUNCTION))
    :UI_WINDOW_MANAGER( display, eventManager, exitFunction ) {} ;

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;
};
#endif /* K_WINDOW_MGR */
