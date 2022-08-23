#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#include "kstatus.hpp"

K_STATUS * K_STATUS::List = NULL ;
ZIL_INT32 K_STATUS::Instance = 1 ;

K_STATUS::K_STATUS( ZIL_ICHAR * name, ZIL_UINT32 process, 
		    K_STATUS ** plist )
{
    strncpy( _name, name, 28 ) ;
    _process = process ;
    _next = NULL ;
    _hwnd = NULL ;
    _kermit_id = K_STATUS::Instance++ ;
    _state = IDLE ;
    if ( plist ) {
		if ( *plist ) {
	while ( (*plist)->_next )
	    plist = &((*plist)->_next) ;
	(*plist)->_next = this ;
		}
		else {
			(*plist) = this ;
		}
    }
};

K_STATUS * 
K_STATUS::Find( ZIL_ICHAR * name ) 
{
   if ( !K_STATUS::List )
       return NULL;

   K_STATUS * status = K_STATUS::List ;
   while ( status )
   {
      if ( !strcmp( status->_name, name) )
         return status ;
      status = status->_next ;
   }
   return NULL;
}

K_STATUS *
K_STATUS::Next( K_STATUS * current ) 
{
    K_STATUS * status = current->_next ;
    while ( status )
    {
	if ( !strcmp( status->_name, current->_name) )
	    return status ;
	status = status->_next ;
    }
    return NULL;
}

ZIL_INT32
K_STATUS::Count( ZIL_ICHAR * name ) 
{
    ZIL_INT32 n = 0 ;

    if ( !K_STATUS::List )
	return 0;

    K_STATUS * status = K_STATUS::List ;
    while ( status )
    {
	if ( !strcmp( status->_name, name) )
	    n++;
	status = status->_next ;
    }
    return(n);
}


K_STATUS * 
K_STATUS::Find( ZIL_INT32 id ) 
{
   if ( !K_STATUS::List )
       return NULL;

   K_STATUS * status = K_STATUS::List ;
   while ( status )
   {
      if ( id == status->_kermit_id )
         return status ;
      status = status->_next ;
   }
   return NULL;
}


void
K_STATUS::Remove( ZIL_ICHAR * name ) 
{
   if ( !K_STATUS::List )
      return ;

   if ( !strcmp( K_STATUS::List->_name, name ) )
   {
     K_STATUS::List = K_STATUS::List->_next ;
	 return ;
   }

   K_STATUS * p = K_STATUS::List ;
   K_STATUS * n = K_STATUS::List->_next ;

   while ( n )
   {
      if ( !strcmp( n->_name, name ) )
      {
         p->_next = n->_next ;
         return ;
      }
      p = n ;
      n = n->_next ;
   }
}

void
K_STATUS::Remove( ZIL_INT32 id ) 
{
   if ( !K_STATUS::List )
      return ;

   if ( id == K_STATUS::List->_kermit_id )
   {
      K_STATUS::List = K_STATUS::List->_next ;
	  return ;
   }


   K_STATUS * p = K_STATUS::List ;
   K_STATUS * n = K_STATUS::List->_next ;

   while ( n )
   {
      if ( n->_kermit_id == id )
      {
         p->_next = n->_next ;
         return ;
      }
      p = n ;
      n = n->_next ;
   }
}

