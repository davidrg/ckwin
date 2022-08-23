class KD_CONFIG ;

class ZIL_EXPORT_CLASS K_NETWORK_DIR : public ZAF_DIALOG_WINDOW
{
 public:
   K_NETWORK_DIR( KD_CONFIG * config);

   virtual EVENT_TYPE Event( const UI_EVENT & event ) ; 

 private:
   KD_CONFIG * _config ;
};


