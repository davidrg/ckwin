class ZIL_EXPORT_CLASS K_STATUS 
{
 public:
   static ZIL_INT32  Instance ;
   static K_STATUS * List ;

   K_STATUS( ZIL_ICHAR *, ZIL_UINT32, 
	     K_STATUS ** = &K_STATUS::List ) ;
   static ZIL_INT32  Count( ZIL_ICHAR * ) ;   
   static K_STATUS * Find( ZIL_ICHAR * ) ;
   static K_STATUS * Find( ZIL_INT32 ) ;
   static K_STATUS * Next( K_STATUS * ) ;
   static void Remove( ZIL_ICHAR * ) ;
   static void Remove( ZIL_INT32 ) ;

   ZIL_ICHAR      _name[29] ;
   ZIL_UINT32     _process ;
   ZIL_INT32      _kermit_id ;
   HWND           _hwnd ;
   K_STATUS *     _next ;

   ZIL_UTIME       _connect_time ;
   ZIL_UTIME       _disconnect_time ;

   enum _State 
   {
      IDLE,
      DIALING,
      CONNECTED
   }	          _state ;
};
