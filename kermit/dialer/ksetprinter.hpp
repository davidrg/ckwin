#ifndef K_SET_PRINTER_HPP
#define K_SET_PRINTER_HPP

#include "ksetdlg.hpp"
class KD_LIST_ITEM ;

class ZIL_EXPORT_CLASS K_DIALOG_PRINTER_SETTINGS : public K95_SETTINGS_DIALOG
{
  public:
    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return _className ; } ;

    K_DIALOG_PRINTER_SETTINGS(void) ; 
    K_DIALOG_PRINTER_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode) ;
    ~K_DIALOG_PRINTER_SETTINGS(void) ;

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  protected:
    KD_LIST_ITEM * _entry ;

    void ApplyChangesToEntry(void) ; 

    void InitPrinterTypeList(void);
    enum PRINTER_TYPE GetPrinterTypeFromCombo( UIW_COMBO_BOX * );
    void InitPrinterDosList(ZIL_ICHAR * user = "");
    void InitPrinterWindowsList(void);
    void InitPrinterFlowList( void );
    void InitPrinterParityList( void );
    void InitPrinterSpeedList( void ) ;

    void InitTermCharsetList( UIW_VT_LIST * ) ;
    void SetTermTypeList( ZIL_ICHAR * ) ;
    enum TERMCSET GetTermCharsetFromCombo( UIW_COMBO_BOX * ) ;
    void SetComboToCharset( UIW_COMBO_BOX *, enum TERMCSET ) ;   

private:
    UI_WINDOW_OBJECT * directoryService ;
    UI_WINDOW_OBJECT * storageService ;

    ZIL_UINT8 _initialized ;
};
#endif
