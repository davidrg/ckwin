/*
  C K O Z L I . C --  ZLIB Interface Source for Kermit 95

  Copyright (C) 2001,2004, Trustees of Columbia University in the City of New
  York.

  Author:  Jeffrey E Altman (jaltman@secure-endpoints.com)
             Secure Endpoints Inc., New York City
*/

#include "ckcdeb.h"

#ifdef NT
#include <windows.h>
#else /* NT */
#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES
#include <os2.h>
#endif /* NT */
#include <stdarg.h>

#ifdef OS2
#ifdef ZLIB

#include "ckozli.h"

#ifdef NT
HINSTANCE hZLIB = NULL;
#else /* NT */
HMODULE hZLIB = NULL;
#endif /* NT */

#ifdef NOT_KERMIT
static int deblog = 0;
#ifdef debug
#undef debug
#endif
#define debug(x,a,b,c)
#ifdef printf
#undef printf
#endif
static int ssl_finished_messages = 0;
static unsigned long startflags = 0;
#endif /* NOT_KERMIT */

int zlib_dll_loaded=0;

int
ck_zlib_is_installed()
{
    debug(F111,"ck_zlib_is_installed","hZLIB",hZLIB);
    return(zlib_dll_loaded);
}


int (WINAPI *p_deflateInit_)(z_streamp strm, int level, const char *version, int stream_size)=NULL;
int (WINAPI *p_inflateInit_)(z_streamp strm, const char *version, int stream_size)=NULL;
int (WINAPI *p_deflateEnd)(z_streamp strm)=NULL;
int (WINAPI *p_inflateEnd)(z_streamp strm)=NULL;
int (WINAPI *p_inflate)(z_streamp strm, int flush)=NULL;
int (WINAPI *p_deflate)(z_streamp strm, int flush)=NULL;

int
ck_deflateInit_(z_streamp strm, int level, const char *version, int stream_size)
{
    if ( p_deflateInit_ )
        return p_deflateInit_(strm,level,version,stream_size);
    return 0;
}

int
ck_inflateInit_(z_streamp strm, const char *version, int stream_size)
{
    if ( p_inflateInit_ )
        return p_inflateInit_(strm, version, stream_size);
    return 0;

}
int
ck_deflateEnd(z_streamp strm)
{
    if ( p_deflateEnd )
        return p_deflateEnd(strm);
    return 0;
}

int
ck_inflateEnd(z_streamp strm)
{
    if ( p_inflateEnd )
        return p_inflateEnd(strm);
    return 0;
}

int
ck_inflate(z_streamp strm, int flush)
{
    if ( p_inflate )
        return p_inflate(strm, flush);
    return 0;
}

int
ck_deflate(z_streamp strm, int flush)
{
    if ( p_deflate )
        return p_deflate(strm, flush);
    return 0;
}

int
ck_zlib_loaddll_eh(void)
{
    if ( hZLIB ) {
#ifdef NT
        FreeLibrary(hZLIB);
        hZLIB = NULL;
#else /* NT */
        DosFreeModule(hZLIB);
        hZLIB = 0;
#endif  /* NT */
    }

    p_inflateInit_ = NULL;
    p_deflateInit_ = NULL;
    p_inflateEnd = NULL;
    p_deflateEnd = NULL;
    p_inflate = NULL;
    p_deflate = NULL;

    return(1);
}

int
ck_zlib_loaddll( void )
{
    ULONG rc = 0 ;
    extern unsigned long startflags;
    int load_error = 0;
#ifdef OS2ONLY
    CHAR *exe_path;
    CHAR path[256];
    CHAR * dllname = "ZLIB";
    CHAR errbuf[256];
#endif /* OS2ONLY */

    if ( zlib_dll_loaded )
        return(1);

    if ( startflags & 8 )       /* do not load if we are not supposed to */
        return(0);

#ifdef NT
    hZLIB = LoadLibrary("ZLIB");
    if ( !hZLIB ) {
        rc = GetLastError() ;
        debug(F111, "ZLIB LoadLibrary failed","ZLIB",rc) ;
        return(0);
    }

    if (((FARPROC) p_inflateInit_ =
          GetProcAddress( hZLIB, "inflateInit_" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111,"ZLIB GetProcAddress failed","inflateInit_",rc);
        load_error = 1;
    }
    if (((FARPROC) p_deflateInit_ =
          GetProcAddress( hZLIB, "deflateInit_" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111,"ZLIB GetProcAddress failed","deflateInit_",rc);
        load_error = 1;
    }
    if (((FARPROC) p_inflateEnd =
          GetProcAddress( hZLIB, "inflateEnd" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111,"ZLIB GetProcAddress failed","inflateEnd",rc);
        load_error = 1;
    }
    if (((FARPROC) p_deflateEnd =
          GetProcAddress( hZLIB, "deflateEnd" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111,"ZLIB GetProcAddress failed","deflateEnd",rc);
        load_error = 1;
    }
    if (((FARPROC) p_inflate =
          GetProcAddress( hZLIB, "inflate" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111,"ZLIB GetProcAddress failed","inflate",rc);
        load_error = 1;
    }
    if (((FARPROC) p_deflate =
          GetProcAddress( hZLIB, "deflate" )) == NULL )
    {
        rc = GetLastError() ;
        debug(F111,"ZLIB GetProcAddress failed","deflate",rc);
        load_error = 1;
    }
#else /* NT */
    exe_path = GetLoadPath();
    sprintf(path, "%.*s%s.DLL", (int)get_dir_len(exe_path), exe_path,dllname);
    rc = DosLoadModule(errbuf, 256, path, &hZLIB);
    if (rc) {
        debug(F111, "ZLIB LoadLibrary failed",path,rc) ;
        rc = DosLoadModule(errbuf, 256, dllname, &hZLIB);
    }
    if ( rc ) {
        debug(F111, "ZLIB LoadLibrary failed",errbuf,rc) ;
        return(0);
    }

    if (rc = DosQueryProcAddr( hZLIB, 0, "inflateInit_",
                               (PFN*) &p_inflateInit_) )
    {
        debug(F111,"ZLIB GetProcAddress failed","inflateInit_",rc);
        load_error = 1;
    }
    if (rc = DosQueryProcAddr( hZLIB, 0, "deflateInit_",
                               (PFN*) &p_deflateInit_) )
    {
        debug(F111,"ZLIB GetProcAddress failed","deflateInit_",rc);
        load_error = 1;
    }
    if (rc = DosQueryProcAddr( hZLIB, 0, "inflateEnd",
                               (PFN*) &p_inflateEnd) )
    {
        debug(F111,"ZLIB GetProcAddress failed","inflateEnd",rc);
        load_error = 1;
    }
    if (rc = DosQueryProcAddr( hZLIB, 0, "deflateEnd",
                               (PFN*) &p_deflateEnd) )
    {
        debug(F111,"ZLIB GetProcAddress failed","deflateEnd",rc);
        load_error = 1;
    }
    if (rc = DosQueryProcAddr( hZLIB, 0, "inflate",
                               (PFN*) &p_inflate) )
    {
        debug(F111,"ZLIB GetProcAddress failed","inflate",rc);
        load_error = 1;
    }
    if (rc = DosQueryProcAddr( hZLIB, 0, "deflate",
                               (PFN*) &p_deflate) )
    {
        debug(F111,"ZLIB GetProcAddress failed","deflate",rc);
        load_error = 1;
    }
#endif /* NT */

    if ( load_error ) {
        ck_zlib_loaddll_eh();
        return 0;
    }

    zlib_dll_loaded = 1;
    if ( deblog ) {
        printf("ZLIB DLL Loaded\n");
        debug(F100,"ZLIB DLL Loaded","",0);
    }

    return(1);
}

int
ck_zlib_unloaddll( void )
{
    if ( !zlib_dll_loaded )
        return(1);

    /* unload dlls */
    ck_zlib_loaddll_eh();

    /* success */
    zlib_dll_loaded = 0;
    return(1);
}
#else /* ZLIB */
int
ck_zlib_loaddll( void )
{
    return(0);
}

int
ck_zlib_unloaddll( void )
{
    return(0);
}
#endif /* ZLIB */
#endif /* OS2 */
