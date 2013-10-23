#include "karray.hxx"
#include "kuidef.h"
extern "C" {
#include <memory.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KArray::KArray( short init, short exsize )
    : container( 0 )
    , arraysize( 0 )
    , expandsize( exsize )
{
    expand( init );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KArray::~KArray()
{
    delete container;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KArray::clear()
{
    memset( container, '\0', (size_t)(arraysize * sizeof(void*)) );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KArray::expand( long size )
{
    long newlen = arraysize + expandsize;

    if( size >= newlen )
        newlen = size + 1;

    void** p = new void* [ newlen ];
    memset( p, '\0', (size_t)(newlen * sizeof(void*)) );

    if( container )
      memcpy( p, container, (size_t)(arraysize * sizeof( void * )) );

    arraysize = newlen;
    delete container;
    container = p;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KArray::insert( void* val, long index )
{
    if( container[arraysize - 1] )
        expand( arraysize + 1 );
    memmove( &container[index + 1], &container[index]
        , (size_t)(((arraysize - 1) - index) * sizeof(void*)) );

    container[index] = val;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void* KArray::remove( long index )
{
    void* ret = container[ index ];
    memcpy( &container[index], &container[index + 1]
        , (size_t)((arraysize - (index + 1)) * sizeof(void*)) );

    container[arraysize - 1] = 0;
    return ret;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KArray::put( void* val, long index )
{
    if( arraysize <= index )
        expand( index );

    container[ index ] = val;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void* KArray::get( long index )
{
    if( arraysize <= index )
        return 0;

    return container[ index ];
}
