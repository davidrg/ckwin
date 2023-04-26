#include "kuidef.h"

intptr_t GlobalControlID = 0;
intptr_t GetGlobalID( void )
{ 
    return GlobalControlID++; 
}

char* newstr( char* s )
{
    size_t len = strlen( s );
    char* sdup = new char[len + 1];
    strcpy( sdup, s );
    return sdup;
}
