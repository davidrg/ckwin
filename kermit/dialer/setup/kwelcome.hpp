#ifndef K_WELCOME_DIALOG_HPP
#define K_WELCOME_DIALOG_HPP

class K_SETUP_GLOBALS;
class ZIL_EXPORT_CLASS K_WELCOME_DIALOG : public K_SETUP_DIALOG
{
    friend class K_SETUP_GLOBALS;
  public:
    static ZIL_ICHAR _className[];
    virtual ZIL_ICHAR *ClassName(void) { return _className; };

    K_WELCOME_DIALOG(void);
    ~K_WELCOME_DIALOG(void);

    virtual EVENT_TYPE Event (const UI_EVENT & event );

    virtual K_SETUP_DIALOG * Previous( void );
    virtual K_SETUP_DIALOG * Next( void );
};

#endif /* K_WELCOME_DIALOG_HPP */
