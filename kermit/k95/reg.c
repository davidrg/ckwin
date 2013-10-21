#include <windows.h>

#define _PROTOTYP( func, parms ) func parms
#include "ckoetc.h"
#include "ckcxla.h"
#include "ckuxla.h"
#include <time.h>

static long crcta[16] = { 0L, 010201L, 020402L, 030603L, 041004L,
  051205L, 061406L, 071607L, 0102010L, 0112211L, 0122412L, 0132613L, 0143014L,
  0153215L, 0163416L, 0173617L };

static long crctb[16] = { 0L, 010611L, 021422L, 031233L, 043044L,
  053655L, 062466L, 072277L, 0106110L, 0116701L, 0127532L, 0137323L, 0145154L,
  0155745L, 0164576L, 0174367L };

/* 16-bit CRC */

unsigned int
chk3(register CHAR *pkt) {
    register long c, crc;
    for (crc = 0; *pkt != '\0'; pkt++) {
	c = crc ^ (long)(*pkt);
	crc = (crc >> 8) ^ (crcta[(c & 0xF0) >> 4] ^ crctb[c & 0x0F]);
    }
    return((unsigned int) (crc & 0xFFFF));
}

#define FILECOUNT 2

CHAR * FileName[FILECOUNT] ;
HANDLE hFile[FILECOUNT] ;
HANDLE hFileMapping[FILECOUNT] ;
CHAR * pView[FILECOUNT] ;
struct ck_registration * RegBlock[FILECOUNT] ;
FILETIME ftLastAccess[FILECOUNT], ftCreation[FILECOUNT], ftLastWrite[FILECOUNT] ;
struct ck_registration NewRegBlock;
char *zinptr=NULL;
int zincnt=0, zoutcnt=0;
int byteorder=0;
char *zoutptr=NULL;
void zdstuff(char c) {};

/* The following is to avoid warnings from ckuxla.obj during linking */
int deblog = 0;
int dodebug(int flag, char * s1, char * s2, int n) {
    return(0);
}


int
ckOpenFile(int i)
{
    hFile[i] = CreateFile(
                   FileName[i],	// address of name of the file 
                   GENERIC_READ | GENERIC_WRITE,
                   0, 	// share mode 
                   NULL,// address of security descriptor 
                   OPEN_EXISTING,	// how to create 
                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,	// file attributes 
                   NULL 	// handle of file with attributes to copy  
   );

    if ( !hFile[i] || (ULONG) hFile[i] == -1L) {
        switch ( i ) {
        case 0:
            printf("switching to k2.exe\n");
            FileName[i] = "k2.exe";
            break;
        case 1:
            printf("switching to k2dial.exe\n");
            FileName[i] = "k2dial.exe";
            break;
        }
        hFile[i] = CreateFile(FileName[i],	// address of name of the file 
                               GENERIC_READ | GENERIC_WRITE,
                               0, 	// share mode 
                               NULL,// address of security descriptor 
                               OPEN_EXISTING,	// how to create 
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,	// file attributes 
                               NULL 	// handle of file with attributes to copy  
                               );                              

        if ( !hFile[i] || (ULONG) hFile[i] == -1L ) {
            return 0;
        }
    }
    
   GetFileTime( hFile[i], &ftCreation[i], &ftLastAccess[i], &ftLastWrite[i] ) ;
   hFileMapping[i] = CreateFileMapping( hFile[i],
                                            NULL,
                                            PAGE_READWRITE,
                                            0,
                                            0,
                                            NULL );

   pView[i] = (CHAR *) MapViewOfFile( hFileMapping[i],
                                 FILE_MAP_READ | FILE_MAP_WRITE,
                                 0,0,0);

    if ( !pView[i] ) {
        return 0;
    }
   return 1;
}

int 
ckCloseFile( int i ) 
{
   if (pView[i])
      UnmapViewOfFile( pView[i] ) ;
   if (hFileMapping[i])
      CloseHandle( hFileMapping[i] ) ;
   if (hFile[i])
   {
      SetFileTime( hFile[i], &ftCreation[i], &ftLastAccess[i], &ftLastWrite[i] ) ;
      CloseHandle( hFile[i] ) ;
   }
   return 1;
}

int 
SearchForRegBlock( int i ) 
{
   CHAR * SearchString =   "====758914360207174068160528073\0\0\0\0\0^H^H^H^H^H^H^_{({({({(^^^^^^^^^\0^C^C^C____^_^_^_^____)})})})})}\0$(((((((((((())))))))))))^@^@^@" ;
   int    SearchStringLength = 136 ;
   int SearchLength = strlen(SearchString) ;
   BY_HANDLE_FILE_INFORMATION fileInfo ;
   CHAR * start = NULL ;
   CHAR * p = NULL ;
   int found = 0 ;
   ULONG k = 0 ;

   GetFileInformationByHandle( hFile[i], &fileInfo ) ;

   //  We only search for the first null terminated portion of the search string 
   //  because we want to be able to find a pre-registered copy

   if ( !pView[i] )
      return 0 ;

   start = pView[i] ;
   p = pView[i] ;
   found = 0 ;

   while ( !found && k < fileInfo.nFileSizeLow )
   {
   	  LONG j = 0 ;
      if ( p[k] == SearchString[0] )
      {
         start = &p[k] ;
         for ( j = 0 ; j < SearchLength ; j++ )
         {
            if ( start[j] != SearchString[j] )
               break;
         }
         if ( j == SearchLength )
            found = 1 ;
      }
      k++ ;
   }

   if ( found )
   {
       RegBlock[i] = (struct ck_registration *) start ;
       printf("Registration Block of %d bytes starts at %d (%X)\n", 
               SearchStringLength, start - pView[i] ,start - pView[i] ) ;
       printf("Pre-byte: 0x%X   Post-byte: 0x%X\n",*(start-1),*(start+SearchStringLength));
   }
    
   return found;
}

int
RegisterExe( struct ck_registration * ck_reg, int i )
{
   memcpy( RegBlock[i], ck_reg, sizeof(struct ck_registration) ) ;
   return (1) ;
}

void
usage(char * program) {
    printf("Usage: %s [ -n <name> {-b | -a | -d | -o <org>} -c <count> -s <serial> \\\n",program);
    printf("            [-p <prefix>] [-v <version>]] [-h]\n\n");
    printf(" If no options are given the current registration information is displayed.\n");
    printf(" Otherwise, the following options must be given:\n");
    printf("   -n followed by the Registree's name;\n");
    printf("   -o followed by the organization, or\n");
    printf("   -b which is short-hand for Bulk Right to Copy License, or\n");
    printf("   -a which is short-hand for Academic Site License;\n");
    printf("   -d which is short-hand for 30-Day Demo;\n");
    printf("   -c followed by the license count;\n");
    printf("   -s followed by the unencoded serial number.\n\n");
    printf(" If necessary, the following options are also available:\n");
    printf("   -p followed by the product prefix, otherwise K95\n");
    printf("   -v followed by the version, otherwise 1.1\n\n");
    printf("   -h is this usage text\n\n");
    exit(1);
}

int bulk=0;
int academic=0;
int demo=0;
char name[32]="";
char org[32]="";
int  license=0, count=0;
char prefix[5]="K95-";
char version[5]="-1.1";
int reg_upd = 0;
unsigned long reg_t=0;

void
ParseCmdLine( int argc, char ** argv )
{
    int i=0,j=0,n=0;
    char * p=NULL;

    i = 1;                      /* start after program name */
    while ( i < argc ) {
        if ( argv[i][0] == '-' || argv[i][0] == '/' ) {
            switch ( argv[i][1] ) {
            case 'n':           /* Name of licensee */
            case 'N':
                i++;            /* get next argument (if any) and use a name */
                if ( i >= argc || name[0]) 
                    usage(argv[0]);

                strncpy(name,argv[i],31);
                name[31]='\0';
                break;
            case 'o':           /* Name of Organization */
            case 'O':
                i++;            /* get next argument (if any) and use a name */
                if ( i >= argc ||
                     demo || bulk || academic || org[0] )
                    usage(argv[0]);

                strncpy(org,argv[i],31);
                org[31]='\0';
                break;
            case 'b':
            case 'B':
                if ( demo || bulk || academic || org[0] )
                    usage(argv[0]);
                bulk = 1;
                strcpy(org,"Bulk Right to Copy License");
                break;
            case 'a':
            case 'A':
                if ( demo || bulk || academic || org[0] )
                    usage(argv[0]);
                academic = 1;
                strcpy(org,"Academic Site License");
                break;
            case 'd':
            case 'D':
                if ( demo || bulk || academic || org[0] )
                    usage(argv[0]);
                demo = 1;
                strcpy(org,"For Evaluation Only");
                break;

            case 'c':
            case 'C':
                i++;
                if ( i >= argc || count )
                    usage(argv[0]);
                count = atol(argv[i]);
                break;
            case 's':
            case 'S':
                i++;
                if ( i >= argc || license )
                    usage(argv[0]);
                license = atol(argv[i]);
                if ( license == 98 )
                    reg_t = time(NULL);
                break;
            case 'v':
            case 'V':
                i++;
                if ( i >= argc )
                    usage(argv[0]);
                strncpy(&version[1],argv[i],3);
                version[4]='\0';
                break;
            case 'p':
            case 'P':
                i++;
                if ( i >= argc )
                    usage(argv[0]);
                strncpy(prefix,argv[i],3);
                prefix[3]='-';
                prefix[4]='\0';
                break;
            case 'h':
            case 'H':
            default:
                usage(argv[0]);
            }
        }
        else {
            usage(argv[0]);
        }
        i++;
    }

    /* Verify contents */
    if ( name[0] || org[0] || license || count ) {
        reg_upd = 1;
        if ( !name[0] || !org[0] || !license )
            usage(argv[0]);
        if ( !count )
            count = 1;
    }

    /* Create the Registration Block */
    if ( reg_upd )
    {
        char * serial = NULL;

        ck_encrypt(name);
        strncpy( NewRegBlock.name, name, 31 );
        NewRegBlock.name[31]='\0';
        j = strlen(NewRegBlock.name);
        for ( ;j < 31 ; j++ )
            NewRegBlock.name[j] = '\0';

        ck_encrypt( org ) ;
        strncpy( NewRegBlock.corp, org, 31 ) ;
        NewRegBlock.corp[31] = '\0' ;
        j = strlen(NewRegBlock.corp) ;
        for ( ; j < 31 ; j++ )
            NewRegBlock.corp[j] = '\0' ;
        NewRegBlock.corp[31] = '\0' ;

        strncpy( NewRegBlock.ident, "====758914360207174068160528073", 32 ) ;

        serial = maksn( prefix, license, version ) ;

        if ( chksn( serial ) )
        {
            ck_encrypt( serial ) ;
            strncpy( NewRegBlock.serial, serial, 31 ) ;
            j = strlen(NewRegBlock.serial) ;
            for ( ; j < 31 ; j++ )
                NewRegBlock.serial[j] = '\0' ;
            NewRegBlock.serial[31] = '\0' ;

            NewRegBlock.is_set = count ? count : 1 ;
            setcrc( &NewRegBlock );
        }

        NewRegBlock.time = reg_t;
    }
}


int
main( int argc, char ** argv ) 
{
    int dsn = 0 ;
    int i ;
    extern int tcsl;

    /* Parse command line and use it to register the files */
    ParseCmdLine(argc,argv);

    /* Get current registration information */

    tcsl = FC_CP850;

#ifdef OS2
    FileName[0] = "k2.exe" ;
    FileName[1] = "k2dial.exe" ;
#else
    FileName[0] = "k95.exe" ;
    FileName[1] = "k95dial.exe" ;
#endif

    for ( i=0 ; i < FILECOUNT ; i++ )
    {
        hFile[i]        = NULL ;
        hFileMapping[i] = NULL ;
        pView[i]        = NULL ;
        RegBlock[i] = NULL ;

        if ( !ckOpenFile(i) )
        {
            printf("Unable to open %s\n", FileName[i]) ;
            return 1 ;
        }

        printf("\n%s\n++++++++++++\n",FileName[i]);
        SearchForRegBlock(i) ;
        if ( !RegBlock[i] )
        {
            // Registration Block not found.  Invalid K95.EXE.
            printf( "Invalid %s file.\n", FileName[i] ) ;
            return 2 ;
        }

        if ( (dsn = isregistered( RegBlock[i] )) > 0 )
        {
            char temp[32] ;
            strcpy( temp, RegBlock[i]->name ) ;
            ck_decrypt( temp ) ;
            printf("Name   : %s\n",temp ) ;
            strcpy( temp, RegBlock[i]->corp ) ;
            ck_decrypt( temp ) ;
            printf("Corp   : %s\n",temp ) ;

            printf("Count  : %d\n",RegBlock[i]->is_set ) ;

            strcpy( temp, RegBlock[i]->serial ) ;
            ck_decrypt( temp ) ;
            printf("Encoded: %s\n",temp);
            temp[3] = '\0' ;
            printf("Product: %s\n",temp ) ;
            printf("Serial : %d\n",dsn ) ;
            printf("Version: %s\n",&temp[16] ) ;
        }
        printf("\n");

        if ( reg_upd ) {
            /* Create the new regblock and store it */
            RegisterExe(&NewRegBlock, i);
        }

        ckCloseFile(i) ;
    }

    if ( reg_upd ) {
        char temp[32];
        printf("\nRegistration Information set to:\n");
        ck_decrypt(name);
        printf("Name   : %s\n",name );
        ck_decrypt(org);
        printf("Corp   : %s\n",org );

        printf("Count  : %d\n",count ) ;

        strcpy( temp, NewRegBlock.serial ) ;
        ck_decrypt( temp ) ;
        printf("Encoded: %s\n",temp);

        prefix[3]='\0';
        printf("Product: %s\n",prefix ) ;
        printf("Serial : %d\n",license ) ;
        printf("Version: %s\n",&version[1] ) ;
        printf("Time:    %u\n",reg_t);
    }
    return(0);
}

