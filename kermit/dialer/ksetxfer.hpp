#ifndef K_SET_TRANSFER_HPP
#define K_SET_TRANSFER_HPP

#include "ksetdlg.hpp"
class KD_LIST_ITEM ;

class ZIL_EXPORT_CLASS K_DIALOG_TRANSFER_SETTINGS : public K95_SETTINGS_DIALOG
{
  public:
    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return _className ; } ;

    K_DIALOG_TRANSFER_SETTINGS(void) ; 
    K_DIALOG_TRANSFER_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode) ;
    ~K_DIALOG_TRANSFER_SETTINGS(void) ;

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  protected:
    KD_LIST_ITEM * _entry ;

    void SetComboToCharset( UIW_COMBO_BOX *, enum FILECSET ) ;   
    void SetComboToCharset( UIW_COMBO_BOX *, enum XFERCSET ) ;   

    enum FILECSET GetFileCharsetFromCombo( UIW_COMBO_BOX * ) ;
    enum XFERCSET GetXferCharsetFromCombo( UIW_COMBO_BOX * ) ;

    void ApplyChangesToEntry(void) ; 

    void InitFileCharsetList( UIW_VT_LIST * ) ;
    void InitXferCharsetList( UIW_VT_LIST * ) ;

private:
    UI_WINDOW_OBJECT * directoryService ;
    UI_WINDOW_OBJECT * storageService ;

    ZIL_UINT8 _initialized ;
};
#endif
