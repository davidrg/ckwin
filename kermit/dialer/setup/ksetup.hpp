#ifndef K_SETUP_HPP
#define K_SETUP_HPP

class ZIL_EXPORT_CLASS K_SETUP : public ZAF_DIALOG_WINDOW
{
 public:
   K_SETUP(void) ; 
   ~K_SETUP(void) ; 

   virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

};

#endif