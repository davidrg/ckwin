#ifndef _K_COLOR
#define _K_COLOR

typedef unsigned long DWORD ;
const DWORD K_BLACK                           = 0x00;
const DWORD K_BLUE                            = 0x01;
const DWORD K_GREEN                           = 0x02;
const DWORD K_CYAN                            = 0x03;
const DWORD K_RED                             = 0x04;
const DWORD K_MAGENTA                         = 0x05;
const DWORD K_BROWN                           = 0x06;
const DWORD K_LIGHTGRAY                       = 0x07;
const DWORD K_DARKGRAY                        = 0x08;
const DWORD K_LIGHTBLUE                       = 0x09;
const DWORD K_LIGHTGREEN                      = 0x0A;
const DWORD K_LIGHTCYAN                       = 0x0B;
const DWORD K_LIGHTRED                        = 0x0C;
const DWORD K_LIGHTMAGENTA            	      = 0x0D;
const DWORD K_YELLOW                          = 0x0E;
const DWORD K_WHITE                           = 0x0F;

class ZIL_EXPORT_CLASS K_COLOR
{
 public:

   K_COLOR() { _color = K_BLACK ; }
   K_COLOR( DWORD );
   K_COLOR( ZIL_ICHAR *) ;

   static void InitList( UIW_VT_LIST * );

   ZIL_ICHAR * Name( void ) ;
   K_COLOR & Set( DWORD ) ;
   K_COLOR & Set( ZIL_ICHAR * ) ;

   K_COLOR & operator=( DWORD color ) 
   {
      return Set( color ) ;
   };
   K_COLOR & operator=( ZIL_ICHAR * string )
   {
      return Set( string ) ;    
   };
   operator DWORD ( void )
   {
      return _color;
   };

 private:
   DWORD _color ;
};

#endif
