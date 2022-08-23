#include <ui_win.hpp>
#include <ui_dsp.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#include "kcolor.hpp"

ZIL_ICHAR * 
K_COLOR::Name( void )
{
   switch ( _color ) {
      case K_BLACK:
         return("Black") ;
         break;
      case K_BLUE:
         return( "Blue" ) ;
         break;
      case K_BROWN:
         return( "Brown" ) ;
         break;
      case K_CYAN:
         return( "Cyan" ) ;
         break;
      case K_DARKGRAY:
         return( "DarkGray" ) ;
         break;
      case K_GREEN:
         return( "Green" ) ;
         break;
      case K_LIGHTBLUE:
         return( "LightBlue" ) ;
         break;
      case K_LIGHTCYAN:
         return( "LightCyan" ) ;
         break;
      case K_LIGHTGRAY:
         return( "LightGray" ) ;
         break;
      case K_LIGHTGREEN:
         return( "LightGreen" ) ;
         break;
      case K_LIGHTMAGENTA:
         return( "LightMagenta" ) ;
         break;
      case K_LIGHTRED:
         return( "LightRed" ) ;
         break;
      case K_MAGENTA:
         return( "Magenta" ) ;
         break;
      case K_RED:
         return( "Red" ) ;
         break;
      case K_WHITE:
         return( "White" ) ;
         break;
      case K_YELLOW:
         return( "Yellow" ) ;
         break;
   }
   return NULL ; 
}

K_COLOR & K_COLOR::Set( DWORD color ) 
{
    _color = color ;
   return (*this);
}

K_COLOR & K_COLOR::Set( ZIL_ICHAR * string )
{
   if ( !string )
      return (*this) ;
   else if ( !strcmp(string,"Black") )
      _color = K_BLACK ;
   else if ( !strcmp(string,"Blue") )
      _color = K_BLUE ;
   else if ( !strcmp(string,"Brown") )
      _color = K_BROWN ;
   else if ( !strcmp(string,"Cyan") )
      _color = K_CYAN ;
   else if ( !strcmp(string,"DarkGray") )
      _color = K_DARKGRAY ;
   else if ( !strcmp(string,"Green") )
      _color = K_GREEN ;
   else if ( !strcmp(string,"LightBlue") )
      _color = K_LIGHTBLUE ;
   else if ( !strcmp(string,"LightCyan") )
      _color = K_LIGHTCYAN ;
   else if ( !strcmp(string,"LightGray") )
      _color = K_LIGHTGRAY;
   else if ( !strcmp(string,"LightGreen") )
      _color = K_LIGHTGREEN;
   else if ( !strcmp(string,"LightMagenta") )
      _color = K_LIGHTMAGENTA;
   else if ( !strcmp(string,"LightRed") )
      _color = K_LIGHTRED;
   else if ( !strcmp(string,"Magenta") )
      _color = K_MAGENTA;
   else if ( !strcmp(string,"Red") )
      _color = K_RED;
   else if ( !strcmp(string,"White") )
      _color = K_WHITE ;
   else if ( !strcmp(string,"Yellow") )
      _color = K_YELLOW ;
   return (*this) ;
}

K_COLOR::K_COLOR( ZIL_ICHAR * string )
{
   if (!string)
      _color = K_BLACK ;
   else 
      Set(string) ;
}

void 
K_COLOR::InitList( UIW_VT_LIST * color_list )
{
   *color_list 
         + new UIW_BUTTON( 0,0,0,"Black",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"Blue",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"Brown",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"Cyan",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"DarkGray",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"Green",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"LightBlue",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"LightCyan",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"LightGray",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"LightGreen",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"LightMagenta",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"LightRed",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"Magenta",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"Red",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"White",BTF_NO_3D, WOF_NO_FLAGS )
         + new UIW_BUTTON( 0,0,0,"Yellow",BTF_NO_3D, WOF_NO_FLAGS ) ;
}


