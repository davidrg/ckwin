#ifndef K_SET_TERMINAL_HPP
#define K_SET_TERMINAL_HPP

#include "ksetdlg.hpp"
#include "kcolor.hpp"

class KD_LIST_ITEM ;
class K_COLOR ;

class ZIL_EXPORT_CLASS K_DIALOG_TERMINAL_SETTINGS : public K95_SETTINGS_DIALOG
{
  public:
    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return _className ; } ;

    K_DIALOG_TERMINAL_SETTINGS(void) ; 
    K_DIALOG_TERMINAL_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode ) ;
    ~K_DIALOG_TERMINAL_SETTINGS(void) {};

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  protected:
    KD_LIST_ITEM * _entry ;

    void SetComboToColor( UIW_COMBO_BOX *, K_COLOR & ) ;
    void SetComboToCharset( UIW_COMBO_BOX *, enum TERMCSET ) ;   
    void SetComboToCursor( UIW_COMBO_BOX *, enum CURSOR_TYPE ) ;

    K_COLOR GetColorFromCombo( UIW_COMBO_BOX * ) ;
    enum TERMCSET GetTermCharsetFromCombo( UIW_COMBO_BOX * ) ;
    enum CURSOR_TYPE GetCursorFromCombo( UIW_COMBO_BOX * ) ;

    void ApplyChangesToEntry(void) ; 

    void InitTermTypeList( void ) ;
    void SetTermTypeList( ZIL_ICHAR * ) ;
    void InitColorLists( void ) ;
    void InitHeightList( void ) ;
    void InitWidthList( void ) ;
    void InitCursorList( void ) ;
    void InitTermCharsetList( UIW_VT_LIST * ) ;

    void InitAutoDownList(void);
    void SetAutoDownList(enum YNA);
    enum YNA GetAutoDownList(void);

private:
    UI_WINDOW_OBJECT * directoryService ;
    UI_WINDOW_OBJECT * storageService ;

    ZIL_UINT8 _initialized ;
};
#endif