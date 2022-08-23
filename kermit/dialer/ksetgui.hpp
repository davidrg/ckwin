#ifndef K_SET_GUI_HPP
#define K_SET_GUI_HPP

#include "ksetdlg.hpp"
class KD_LIST_ITEM ;

class ZIL_EXPORT_CLASS K_DIALOG_GUI_SETTINGS : public K95_SETTINGS_DIALOG
{
  public:
    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return _className ; } ;

    K_DIALOG_GUI_SETTINGS(void) ; 
    K_DIALOG_GUI_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode) ;
    ~K_DIALOG_GUI_SETTINGS(void) ;

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  protected:
    KD_LIST_ITEM * _entry ;
    void ApplyChangesToEntry(void) ; 
    void InitFacenameList(void);
    char * GetFacenameFromCombo(UIW_COMBO_BOX * combo);
    void SetComboToFacename(UIW_COMBO_BOX * combo, char * facename);
    enum GUI_RESIZE GetResizeMode(UIW_COMBO_BOX * combo);
    void SetResizeMode(UIW_COMBO_BOX * combo, enum GUI_RESIZE);
    enum GUI_RUN GetRunMode(UIW_COMBO_BOX * combo);
    void SetRunMode(UIW_COMBO_BOX * combo, enum GUI_RUN);

    UIW_TABLE *table;
    UIW_BIGNUM *totalField;

private:

    UI_WINDOW_OBJECT * directoryService ;
    UI_WINDOW_OBJECT * storageService ;

    ZIL_UINT8 _initialized ;
};

#endif