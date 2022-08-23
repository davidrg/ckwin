#ifndef K_SET_DLG_HPP
#define K_SET_DLG_HPP

class ZIL_EXPORT_CLASS K95_SETTINGS_DIALOG : public ZAF_DIALOG_WINDOW
{
  protected:
    enum ENTRYMODE _mode ;

  public:
    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return _className ; } ;

    K95_SETTINGS_DIALOG(void); 
    K95_SETTINGS_DIALOG(ZIL_ICHAR * name, enum ENTRYMODE mode); 
    ~K95_SETTINGS_DIALOG(void) {};

    ZIL_UINT8 _cancelled;
    enum ENTRYMODE GetMode(void) { return _mode; };
};
#endif /* K_SET_DLG_HPP */
