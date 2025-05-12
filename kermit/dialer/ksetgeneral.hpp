#ifndef K_SET_GENERAL_HPP
#define K_SET_GENERAL_HPP

#include "ksetdlg.hpp"
class KD_LIST_ITEM ;

class ZIL_EXPORT_CLASS K_DIALOG_GENERAL_SETTINGS : public K95_SETTINGS_DIALOG
{
  public:
    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return _className ; } ;

    K_DIALOG_GENERAL_SETTINGS(void) ; 
    K_DIALOG_GENERAL_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode ) ;
    ~K_DIALOG_GENERAL_SETTINGS(void) ;

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  protected:
    KD_LIST_ITEM * _entry ;
    ZIL_ICHAR *    _original_name ;

    void ApplyChangesToEntry(void) ; 
    void InitTCPProtoList();
    void InitSSHProtoList();
    UIW_BUTTON * FindTCPButton(enum TCPPROTO);
    UIW_BUTTON * FindSSHButton(enum SSHPROTO);
    enum TCPPROTO GetTCPProto(void);
    enum SSHPROTO GetSSHProto(void);

  private:
    UI_WINDOW_OBJECT * directoryService ;
    UI_WINDOW_OBJECT * storageService ;

    ZIL_UINT8 _initialized ;
    enum TRANSPORT _transport;
    ZIL_ICHAR _phone[257];
    ZIL_ICHAR _ip[257];
    ZIL_ICHAR _lat[257];
    ZIL_ICHAR _ipport[33];
    ZIL_ICHAR _sshport[33];
    ZIL_ICHAR _ftpport[33];
    enum TCPPROTO _tcpproto;
    enum SSHPROTO _sshproto;
};
#endif
