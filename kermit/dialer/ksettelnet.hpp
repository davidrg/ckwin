#ifndef K_SET_TELNET_HPP
#define K_SET_TELNET_HPP

#include "ksetdlg.hpp"
class KD_LIST_ITEM ;

class ZIL_EXPORT_CLASS K_DIALOG_TELNET_SETTINGS : public K95_SETTINGS_DIALOG
{
  public:
    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return _className ; } ;

    K_DIALOG_TELNET_SETTINGS(void) ; 
    K_DIALOG_TELNET_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode) ;
    ~K_DIALOG_TELNET_SETTINGS(void) ;

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  protected:
    KD_LIST_ITEM * _entry ;
    void ApplyChangesToEntry(void) ; 

private:
    UI_WINDOW_OBJECT * directoryService ;
    UI_WINDOW_OBJECT * storageService ;

    ZIL_UINT8 _initialized ;
};
#endif
