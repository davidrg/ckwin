#ifndef karray_hxx_included
#define karray_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: karray.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: simple utility array class used to hold anything.

========================================================================*/

class KArray 
{
public:
    KArray( short init = 32, short exsize = 16 );
    ~KArray();

    void put( void* val, long index );
    void* get( long index );
    void clear();
    void insert( void* val, long index );
    void* remove( long index );

protected:
    void expand( long index );
    long arraysize;
    long expandsize;
    void** container;
};

#endif
