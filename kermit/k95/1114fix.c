#include <windows.h>

#define _PROTOTYP( func, parms ) func parms
#define FILECOUNT 2

CHAR * FileName[FILECOUNT] ;
HANDLE hFile[FILECOUNT] ;
HANDLE hFileMapping[FILECOUNT] ;
CHAR * pView[FILECOUNT] ;
FILETIME ftLastAccess[FILECOUNT], ftCreation[FILECOUNT], ftLastWrite[FILECOUNT] ;

#ifdef _M_IX86
#define K95_SIZE            1431552    
#define K95_OFFSET          0x14D4C0
#define K95_PREBYTE         ((CHAR)0x0)
#define K95_POSTBYTE        ((CHAR)0x88)
#define K95DIAL_SIZE        739840
#define K95DIAL_OFFSET      0xA0FA8
#define K95DIAL_PREBYTE     ((CHAR)0x0)
#define K95DIAL_POSTBYTE    ((CHAR)0x57)
#else 
#ifdef _M_MPPC
#define K95_SIZE            2049536
#define K95_OFFSET          0x1C2CAB
#define K95_PREBYTE         ((CHAR)0x0)
#define K95_POSTBYTE        ((CHAR)0x0)
#define K95DIAL_SIZE        1196544
#define K95DIAL_OFFSET      0xFE280
#define K95DIAL_PREBYTE     ((CHAR)0x0)
#define K95DIAL_POSTBYTE    ((CHAR)0x0)
#else
#ifdef _M_ALPHA
#define K95_SIZE            2371584
#define K95_OFFSET          0x192C68
#define K95_PREBYTE         ((CHAR)0x46)
#define K95_POSTBYTE        ((CHAR)0x88)
#define K95DIAL_SIZE        897024
#define K95DIAL_OFFSET      0xB3CC8
#define K95DIAL_PREBYTE     ((CHAR)0x0)
#define K95DIAL_POSTBYTE    ((CHAR)0x25)
#else
PLATFORM NOT SUPPORTED
#endif
#endif
#endif

int FixIt = 0;

char *zinptr=NULL;
int zincnt=0, zoutcnt=0;
char *zoutptr=NULL;
void zdstuff(char c) {};


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

   if ( !hFile[i] )
      return 0;
    
   GetFileTime( hFile[i], &ftCreation[i], &ftLastAccess[i], &ftLastWrite[i] ) ;
   hFileMapping[i] = CreateFileMapping( hFile[i],
                                            NULL,
                                            PAGE_READWRITE,
                                            0,
                                            0,
                                            NULL );

   pView[i] = (CHAR *) MapViewOfFile( hFileMapping[i],
                                 FILE_MAP_WRITE,
                                 0,0,0);

   if ( !pView[i] )
      return 0;
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
   CHAR * SearchString =   "====758914360207174068160528073" ;
   int    SearchStringLength = 136 ;
   int SearchLength = strlen(SearchString) ;
   BY_HANDLE_FILE_INFORMATION fileInfo ;
   CHAR * start = NULL, * prebug = NULL, * postbug = NULL ;
   ULONG prebugpos=0, postbugpos=0;
   CHAR * p = NULL ;
   int found = 0 ;
   ULONG k = 0 ;

   GetFileInformationByHandle( hFile[i], &fileInfo ) ;

   //  We only search for the first null terminated portion of the search string 
   //  because we want to be able to find a pre-registered copy

   if ( !pView[i] )
      return 0 ;

    /* are the files the correct length? */
    switch ( i ) {
    case 0:
        if ( fileInfo.nFileSizeLow != K95_SIZE ) {
            printf("ERROR: Incorrect version of K95.EXE.\n");
            return(0);
        }
        break;
    case 1:
        if ( fileInfo.nFileSizeLow != K95DIAL_SIZE ) {
            printf("ERROR: Incorrect version of K95DIAL.EXE.\n");
            return(0);
        }
        break;
    }

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
       prebug = start - 1;
       prebugpos = start - pView[i] - 1;
       postbug = start + SearchStringLength;
       postbugpos = start - pView[i] + SearchStringLength;

       printf("\nRegistration Block of %d bytes starts at %d (0x%X)\n", 
               SearchStringLength, 
               start - pView[i],
               start - pView[i]) ;
       printf(" Pre-Bug position: %d (0x%X)  value: 0x%X\n",
               prebugpos,prebugpos,(CHAR)*prebug);
       printf("Post-Bug position: %d (0x%X)  value: 0x%X\n",
               postbugpos,postbugpos,(CHAR)*postbug);

       switch ( i ) {
       case 0:  /* K95 */
           if ( (prebugpos != K95_OFFSET - 1) ||
                (postbugpos != K95_OFFSET + SearchStringLength)) {
               printf("ERROR: Incorrect version of K95.EXE.\n");
           }
           else {
               if (*prebug == K95_PREBYTE && *postbug == K95_POSTBYTE)
               {
                   printf("This version of K95.EXE does not contain the Patch bug.\n");
               }
               else {
                   printf("This version of K95.EXE contains the Patch bug.");
                   if ( FixIt ) {
                       *prebug = K95_PREBYTE;
                       *postbug = K95_POSTBYTE;
                       printf("  REPAIRED");
                   }
                   else {
                       printf("\nRun again with the /F switch to repair the bug.");
                   }
                   printf("\n");
               }
           }
           break;
       case 1:  /* K95DIAL */
           if ( (prebugpos != K95DIAL_OFFSET - 1) ||
                (postbugpos != K95DIAL_OFFSET + SearchStringLength)) {
               printf("ERROR: Incorrect version of K95DIAL.EXE.\n");
           }
           else {
               if (*prebug == K95DIAL_PREBYTE && *postbug == K95DIAL_POSTBYTE)
               {
                   printf("This version of K95DIAL.EXE does not contain the Patch bug.\n");
               }
               else {
                   printf("This version of K95DIAL.EXE contains the Patch bug.");
                   if ( FixIt ) {
                       *prebug = K95DIAL_PREBYTE;
                       *postbug = K95DIAL_POSTBYTE;
                       printf("  REPAIRED");
                   }
                   else {
                       printf("\nRun again with the /F switch to repair the bug.");
                   }
                   printf("\n");
               }
           }
           break;
       }
   }
   return found;
}


int
main( int argc, char ** argv ) 
{
    int dsn = 0 ;
    int i ;

    FileName[0] = "k95.exe" ;
    FileName[1] = "k95dial.exe" ;

    printf("This program corrects a bug in Kermit-95 Version 1.1.14\n");
    printf("introduced during the patch procedure from a previous release\n");

    if ( argc > 2 || 
         (argc == 2 && ((argv[1][0] != '-' && argv[1][0] != '/') ||
           (argv[1][1] != 'f' && argv[1][1] != 'F')))) {
        printf("ERROR: Usage %s [/F]\n", argv[0]);
        return(2);
    }
    else if ( argc == 2 ) {
        printf("Fix it mode turned on.\n\n");
        FixIt = 1 ;
    }

    for ( i=0 ; i < FILECOUNT ; i++ )
    {
        printf("Correcting %s if necessary ...\n",FileName[i]);

        hFile[i]        = NULL ;
        hFileMapping[i] = NULL ;
        pView[i]        = NULL ;

        if ( !ckOpenFile(i) )
        {
            printf("ERROR: Unable to open %s\n\n", FileName[i]) ;
            continue ;
        }

        if ( !SearchForRegBlock(i) )
        {
            // Registration Block not found.
            printf( "ERROR: %s skipped.\n", FileName[i] ) ;
        }
        else {
            printf("\n");
        }
        ckCloseFile(i) ;

    }
    return(0);
}

