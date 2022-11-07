#ifndef K_SETUP_DIALOG_HPP
#define K_SETUP_DIALOG_HPP

class K_SETUP_GLOBALS;
class ZIL_EXPORT_CLASS K_SETUP_DIALOG : public ZAF_DIALOG_WINDOW
{
    friend class K_SETUP_GLOBALS;
  public:
    static ZIL_ICHAR _className[];
    virtual ZIL_ICHAR *ClassName(void) { return _className; };

    K_SETUP_DIALOG(void);
    K_SETUP_DIALOG( ZIL_ICHAR * WindowName );
    ~K_SETUP_DIALOG(void);

    virtual EVENT_TYPE Event (const UI_EVENT & event ) = 0;

    virtual K_SETUP_DIALOG * Previous( void ) = 0;
    virtual K_SETUP_DIALOG * Next( void ) = 0;

    K_SETUP_DIALOG * next;
};
#endif /* K_SETUP_DIALOG_HPP */
