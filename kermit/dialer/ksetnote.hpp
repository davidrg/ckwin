#ifndef K_SET_NOTE_HPP
#define K_SET_NOTE_HPP

#include "kcolor.hpp"

class KD_LIST_ITEM ;
class K_COLOR ;

class ZIL_EXPORT_CLASS K_SETTINGS_NOTEBOOK : public ZAF_DIALOG_WINDOW
{
  public:
    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return _className ; } ;

    K_SETTINGS_NOTEBOOK(void) ; 
    K_SETTINGS_NOTEBOOK( KD_LIST_ITEM * entry, enum ENTRYMODE mode ) ;
    ~K_SETTINGS_NOTEBOOK(void) ;

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  protected:
    KD_LIST_ITEM * _entry ;
    ZIL_ICHAR *    _original_name ;
    enum ENTRYMODE _mode ;

    void SetComboToColor( UIW_COMBO_BOX *, K_COLOR & ) ;
    void SetComboToCharset( UIW_COMBO_BOX *, enum TERMCSET ) ;   
    void SetComboToCharset( UIW_COMBO_BOX *, enum FILECSET ) ;   
    void SetComboToCharset( UIW_COMBO_BOX *, enum XFERCSET ) ;   
    void SetComboToCursor( UIW_COMBO_BOX *, enum CURSOR_TYPE ) ;

    K_COLOR GetColorFromCombo( UIW_COMBO_BOX * ) ;
    enum TERMCSET GetTermCharsetFromCombo( UIW_COMBO_BOX * ) ;
    enum FILECSET GetFileCharsetFromCombo( UIW_COMBO_BOX * ) ;
    enum XFERCSET GetXferCharsetFromCombo( UIW_COMBO_BOX * ) ;
    enum CURSOR_TYPE GetCursorFromCombo( UIW_COMBO_BOX * ) ;

    void ApplyChangesToEntry(void) ; 

    void InitTermTypeList( void ) ;
    void SetTermTypeList( ZIL_ICHAR * ) ;
    void InitColorLists( void ) ;
    void InitLineList( ZIL_UINT8 ) ;
    void InitModemList( void ) ;
    void InitFlowList( void );
    void InitParityList( void );
    void InitStopBitList( void );
    void InitSpeedList( void ) ;
    void InitHeightList( void ) ;
    void InitWidthList( void ) ;
    void InitCursorList( void ) ;
    void InitTermCharsetList( UIW_VT_LIST * ) ;
    void InitFileCharsetList( UIW_VT_LIST * ) ;
    void InitXferCharsetList( UIW_VT_LIST * ) ;
    void InitPrinterTypeList(void);
    enum PRINTER_TYPE GetPrinterTypeFromCombo( UIW_COMBO_BOX * );
    void InitPrinterDosList(ZIL_ICHAR * user = "");
    void InitPrinterWindowsList(void);
    void InitPrinterFlowList( void );
    void InitPrinterParityList( void );
    void InitPrinterSpeedList( void ) ;

private:
    UI_WINDOW_OBJECT * directoryService ;
    UI_WINDOW_OBJECT * storageService ;

    ZIL_UINT8 _initialized ;
    enum TRANSPORT _transport;
    ZIL_ICHAR _phone[257];
    ZIL_ICHAR _ip[257];
    ZIL_ICHAR _lat[257];
};
#endif
