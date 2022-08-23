#ifndef K_SET_SSH_HPP
#define K_SET_SSH_HPP

#include "ksetdlg.hpp"
class KD_LIST_ITEM ;

class ZIL_EXPORT_CLASS K_DIALOG_SSH_SETTINGS : public K95_SETTINGS_DIALOG
{
  public:
    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return _className ; } ;

    K_DIALOG_SSH_SETTINGS(void) ; 
    K_DIALOG_SSH_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode) ;
    ~K_DIALOG_SSH_SETTINGS(void) ;

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  protected:
    KD_LIST_ITEM * _entry ;
    void ApplyChangesToEntry(void) ; 

    void InitSSH1CipherList(KD_LIST_ITEM *);
    void InitSSH2CipherList(KD_LIST_ITEM *);
    void InitSSH2AuthList(KD_LIST_ITEM *);
    void InitSSH2MacList(KD_LIST_ITEM *);
    void InitSSH2HKAList(KD_LIST_ITEM *);

private:
    UI_WINDOW_OBJECT * directoryService ;
    UI_WINDOW_OBJECT * storageService ;

    ZIL_UINT8 _initialized ;
};
#endif