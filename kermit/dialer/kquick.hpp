class KD_LIST_ITEM ;
class KD_CONFIG ;
class K_CONNECTOR ;

class ZIL_EXPORT_CLASS K_QUICK : public UIW_WINDOW
{
 public:
   K_QUICK( KD_CONFIG *, KD_LIST_ITEM * ) ; 
   ~K_QUICK(void) ;

   virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

 private:
   void PopulateList( enum TRANSPORT );

   KD_LIST_ITEM * _template ;
   KD_CONFIG * _config ;
};


