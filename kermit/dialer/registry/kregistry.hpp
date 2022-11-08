#ifndef K_WINDOW_HPP
#define K_WINDOW_HPP

class ZIL_EXPORT_CLASS K_WINDOW : public UIW_WINDOW
{
  public:
    K_WINDOW(void);
    ~K_WINDOW(void);

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  private:
    static const int KSC_ASSOC         ;
    static const int TELNET_DEFAULT    ;
    static const int REMOVE_K95        ;
    static const int DIALER_SHORTCUT   ;
    static const int K95_SHORTCUT      ;
    static const int DIALER_START_MENU ;
    static const int K95_START_MENU    ;
    static const int DOCS_START_MENU   ;

    ZIL_UINT32 CreateShortcut( ZIL_ICHAR *, ZIL_ICHAR *, 
                               ZIL_INT8=1, ZIL_INT8=0,
                               ZIL_INT8=1, ZIL_INT8=0, BOOL=TRUE );
    ZIL_UINT8  VerifyAssociations( void );
    ZIL_UINT8  CreateAssociations( void );
    ZIL_UINT8  CreateDocAssociations( void );
    ZIL_UINT8  DeleteAssociations( void );
    ZIL_UINT8  MakeK95DefaultTelnet( void );
    ZIL_UINT8  VerifyClassMIME( void );
    ZIL_UINT8  VerifyClassAppID( void );
    ZIL_UINT8  VerifyClassKSC( void );
    HRESULT    CreateLink( LPSTR, LPSTR, LPSTR, LPSTR, LPSTR, ZIL_UINT8);
    void       Success( void );
    void       Failure( void );
};

#endif /* K_WINDOW_HPP */
