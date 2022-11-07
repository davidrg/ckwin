class KD_CONFIG ;

class ZIL_EXPORT_CLASS K_DIAL_DIR : public ZAF_DIALOG_WINDOW
{
 public:
   K_DIAL_DIR( KD_CONFIG * ) ;

   virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

 private:
   KD_CONFIG * _config ;
};


