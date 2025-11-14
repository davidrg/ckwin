#ifndef K_CINIT_HPP
#define K_CINIT_HPP

#define OPEN_COMM   12001
#define SEND_DATA   12002
#define CLOSE_COMM  12003
#define COMM_DONE   12004

class ZIL_EXPORT_CLASS K_CINIT : public UIW_WINDOW
{
  public:
    K_CINIT(ZIL_ICHAR * portname) ; 
    ~K_CINIT(void) ; 

  virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  private:
    UIW_STRING * _fPort, * _fStatus;
    ZIL_ICHAR  * _portname;
    int idComDev;
};

#endif

