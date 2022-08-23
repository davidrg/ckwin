#ifndef K_DIAL_OPT_HPP
#define K_DIAL_OPT_HPP

class K_LOCATION ;
class K_CONNECTOR;
class ZIL_EXPORT_CLASS K_DIAL_OPTIONS : public ZAF_DIALOG_WINDOW
{
    friend class K_LOCATION;
  public:
    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return _className ; } ;

    K_DIAL_OPTIONS(void) ; 
    K_DIAL_OPTIONS( UI_LIST * Locations );
    ~K_DIAL_OPTIONS(void);

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

    void AddLocation( ZIL_ICHAR * );
    void CopyValuesToCurrent();
    void CopyValuesFromCurrent();

    K_LOCATION *  _current ;

  private:

    UI_LIST *     SourceList;
    UIW_VT_LIST * _location_list ;
    UI_LIST       _delete_list;
    K_CONNECTOR * _connector;

   void ApplyChanges(void) ;
};

class ZIL_EXPORT_CLASS K_LOC_NAME : public ZAF_DIALOG_WINDOW
{
  public:
    K_LOC_NAME( K_DIAL_OPTIONS * ) ; 
    ~K_LOC_NAME(void) ;

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  private:

    K_DIAL_OPTIONS * _parent ;
};
#endif