/* F O O . C  --  Stub for Kermit 95 */
/*
  Authors: Frank da Cruz (fdc@columbia.edu)
           Jeffrey Altman (jaltman@columbia.edu),
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1995-2003, Trustees of Columbia University in the City of
  New York.  All rights reserved.
*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

char k95path[512];

int
main( int argc, const char **argv ) {
    char *p;
	int i;

    GetModuleFileName(NULL, k95path, 512);
    p = k95path + strlen(k95path);
    while ( *p != '\\' && p > k95path ) p--;
    if ( p != k95path ) 
        p++;
    strcpy(p,"k95.exe");

	p = (char *)(argv[0] + strlen(argv[0]));
    while ( *p != '\\' && p > argv[0] ) p--;
    if ( p != argv[0] ) 
        p++;
	argv[0] = (const char *)p;

    return spawnv( _P_WAIT, k95path, argv ) ;
}
