#include "kuidef.h"

int GlobalControlID = 0;
int GetGlobalID( void ) 
{ 
    return GlobalControlID++; 
}

char* newstr( char* s )
{
    int len = strlen( s );
    char* sdup = new char[len + 1];
    strcpy( sdup, s );
    return sdup;
}
