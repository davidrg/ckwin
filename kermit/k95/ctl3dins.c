/*
  Author: Jeffrey Altman (jaltman@columbia.edu),
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 1997, Trustees of Columbia University in the City of New
  York.
*/

/* Installs CTL3D32.DLL and MSVCRT.DLL into the proper directory */

#include <windows.h>

void
DisplayHelp( char * name )
{
    printf("%s [-f]: Installs CTL3D32.DLL and MSVCRT.DLL.  -f forces the install upon error.\n\n",name);
}

void
InstallFile(char * prgname, char * srcfile, char * destfile, int forceinstall) {
    char   curdir[1024];
    char   windir[1024];
    char   curpath[1024];
    DWORD  curlen = 1024;
    char   destpath[1024];
    DWORD  destlen = 1024;
    char   tmpfile[1024];
    DWORD  tmplen = 1024;
    DWORD  vff_bitmask = 0;
    DWORD  vif_bitmask = 0;
    int    deletetmpfile = 0;

    GetWindowsDirectory( windir, 1024 );
    GetCurrentDirectory( 1024, curdir );

    /* Use VerFindFile() to figure out where CTL3D32.DLL */
    /* should be installed                               */
    vff_bitmask = VerFindFile( VFFF_ISSHAREDFILE,
                               destfile,
                               windir,
                               curdir,
                               curpath, &curlen,
                               destpath, &destlen );
    if ( vff_bitmask & VFF_CURNEDEST ) {
        printf("%s is installed but in the wrong directory\n",destfile);
        printf("Currently in \"%s\" but should be in \"%s\"\n",
                curpath, destpath );
    }
    if ( vff_bitmask & VFF_FILEINUSE ) {
        printf("%s is installed and in use.\n",destfile);
    }

    printf("Installing %s as %s in \"%s\".\n",
            srcfile, destfile, destpath);

    /* Use VerInstallFile() to actually install it     */
    vif_bitmask = VerInstallFile( forceinstall ? VIFF_FORCEINSTALL : 0,
                                  srcfile,
                                  destfile,
                                  curdir,
                                  destpath,
                                  curpath,
                                  tmpfile,
                                  &tmplen);

    if ( vif_bitmask & VIF_TEMPFILE ) {
        printf("The temporary copy of the new file is in \"%s\".\n\n",
                tmpfile);
        deletetmpfile = 1;
    }
    if ( vif_bitmask & VIF_MISMATCH ) {
        printf("The new and preexisting files differ in one or more attributes.\n");
        printf("To force the installation, run %s again with the \"-f\" switch.",prgname);;
    }
    if ( vif_bitmask & VIF_SRCOLD ) {
        printf("The file to install is older than the preexisting file.\n");
        printf("To force the installation, run %s again with the \"-f\" switch.",prgname);;
    }
    if ( vif_bitmask & VIF_DIFFLANG ) {
        printf("The new and preexisting files have different language or code-page\n");
        printf("values.\n\n");
        printf("To force the installation, run %s again with the \"-f\" switch.",prgname);;
    }   
    if ( vif_bitmask & VIF_DIFFCODEPG ) {
        printf("The new file requires a code page that cannot be displayed by the\n");
        printf("version of Windows currently running.\n\n");
        printf("To force the installation, run %s again with the \"-f\" switch.",prgname);;
    }
    if ( vif_bitmask & VIF_DIFFTYPE ) {
        printf("The new file has a different type, subtype, or operating system\n");
        printf("from the preexisting file.\n\n");
        printf("To force the installation, run %s again with the \"-f\" switch.",prgname);;
    }
    if ( vif_bitmask & VIF_WRITEPROT ) {
        printf("The preexisting file is write protected\n\n");
        printf("To force the installation, run %s again with the \"-f\" switch.",prgname);;
    }
    if ( vif_bitmask & VIF_FILEINUSE ) {
        printf("The preexisting file is in use and cannot be deleted\n\n");
    }
    if ( vif_bitmask & VIF_OUTOFSPACE ) {
        printf("The function cannot create the temporary file due to insufficient\n");
        printf("disk space on the destination drive.\n\n");
    }
    if ( vif_bitmask & VIF_ACCESSVIOLATION ) {
        printf("A read, create, delete, or rename operation failed due to a access\n");
        printf("violation.\n\n");
    }
    if ( vif_bitmask & VIF_SHARINGVIOLATION ) {
        printf("A read, create, delete, or rename operation failed due to a sharing\n");
        printf("violation.\n\n");
    }
    if ( vif_bitmask & VIF_CANNOTCREATE ) {
        printf("The temporary file cannot be created.\n\n");
    }
    if ( vif_bitmask & VIF_CANNOTDELETE ) {
        printf("Either cannot delete the destination file or a preexisting file in\n");
        printf("another directory.\n\n");
    }
    if ( vif_bitmask & VIF_CANNOTDELETECUR ) {
        printf("The preexisting version of the file could not be deleted.\n\n");
    }
    if ( vif_bitmask & VIF_CANNOTRENAME ) {
        printf("The temporary file cannot be renamed but the preexisting file\n");
        printf("has already been deleted.\n\n");
    }
    if ( vif_bitmask & VIF_OUTOFMEMORY ) {
        printf("Ran out of memory.\n\n");
    }
    if ( vif_bitmask & VIF_CANNOTREADSRC ) {
        printf("Source file could not be read.\n\n");
    }
    if ( vif_bitmask & VIF_CANNOTREADDST ) {
        printf("Destination file could not be read.\n");
    }
    if ( vif_bitmask & VIF_BUFFTOOSMALL ) {
        printf("one of the string buffers is too small.\n");
    }
}

int
main( int argc, char * argv[] )
{
    OSVERSIONINFO osverinfo ;
    char * srcfile = NULL;
    char * destfile = "ctl3d32.dll";
    int    forceinstall = 0;

    if ( argc > 2 ) {
        DisplayHelp( argv[0] );
        return 1;
    }

    if ( argc == 2 ) {
        if ( (argv[1][0] == '/' || argv[1][0] == '-') && 
             (argv[1][1] == 'f' || argv[1][1] == 'F') )
            forceinstall = 1;
        else {
            DisplayHelp( argv[0] );
            return 1;
        }
    }

    /* Determine if we are on Windows 95 or Windows NT */
    /* and set the proper file names                   */
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;
    switch ( osverinfo.dwPlatformId ) {
    case VER_PLATFORM_WIN32_WINDOWS:
        srcfile = "ctl3d95.dll";
        break;
    case VER_PLATFORM_WIN32_NT:
        srcfile = "ctl3dnt.dll";
        break;
    default:
        printf("Invalid operating system type\n");
        return 2;
    }

    InstallFile(argv[0],srcfile,"ctl3d32.dll",forceinstall);
    InstallFile(argv[0],"msvcrt__.dll","msvcrt.dll",forceinstall);

    return 0;
}

