#ifndef K_MODEM_DIALOG_HPP
#define K_MODEM_DIALOG_HPP

class K_MODEM ;
class K_CONNECTOR;
class ZIL_EXPORT_CLASS K_MODEM_DIALOG : public ZAF_DIALOG_WINDOW
{
    friend class K_LOCATION;
  public:
    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return _className ; } ;

    K_MODEM_DIALOG(void) ; 
    K_MODEM_DIALOG( UI_LIST * Locations );
    ~K_MODEM_DIALOG(void);

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

    void AddModem( ZIL_ICHAR * );
    void CopyValuesToCurrent();
    void CopyValuesFromCurrent();
    void InitParityList();
    void InitStopBitsList();
    void InitFlowList();
    void InitSpeedList();
    void InitPortList();
#if defined(WIN32)
    void InitTapiPortList();
#endif
    void InitModemTypeList();

    K_MODEM *  _current ;

  private:

    UI_LIST *     SourceList;
    UIW_VT_LIST * _modem_list ;
    UI_LIST       _delete_list;
    K_CONNECTOR * _connector;

   void ApplyChanges(void) ;
};

class ZIL_EXPORT_CLASS K_MODEM_NAME : public ZAF_DIALOG_WINDOW
{
  public:
    K_MODEM_NAME( K_MODEM_DIALOG * ) ; 
    ~K_MODEM_NAME(void) ;

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  private:
    K_MODEM_DIALOG * _parent ;
};


class ZIL_EXPORT_CLASS K_MODEM_CONFIG : public ZAF_DIALOG_WINDOW
{
  public:
    K_MODEM_CONFIG( K_MODEM_DIALOG *, K_MODEM * ) ; 
    ~K_MODEM_CONFIG(void) ;

    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return _className ; } ;
    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  private:

    void ApplyChanges();
    void InitSpeedList();

    K_MODEM * _modem ;
    K_MODEM_DIALOG * _parent ;
};
#endif
