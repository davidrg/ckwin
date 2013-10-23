#include "kflname.hxx"
#include "karray.hxx"
#include "kuidef.h"
#include "ikcmd.h"
#include "ikterm.h"


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KFileName::KFileName()
{
    filearray = new KArray();
    arraycount = 0L;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KFileName::~KFileName()
{
    K_FILENAME* fn;
    for( long i = 0; i < arraycount; i++ )
    for( i = 0; i < arraycount; i++ )
    {
        fn = (K_FILENAME*)filearray->get( i );
        delete fn->name;
        delete fn->mode;
    }
    delete filearray;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
K_FILENAME* KFileName::add( char* n, char* m ) 
{
    K_FILENAME* fn = new K_FILENAME;
    fn->name = n ? newstr( n ) : 0;
    fn->mode = m ? newstr( m ) : 0;  

    filearray->put( fn, arraycount++ );
    return fn;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFileName::toggle( K_FILENAME* findfn )
{
    K_FILENAME* fn;
    for( long i = 0; i < arraycount; i++ ) {
        fn = (K_FILENAME*)filearray->get( i );
        if( fn == findfn ) {
            char* newmode;
            if( strcmp( fn->mode, "binary" ) == 0 )
                newmode = newstr( "text" );
            else
                newmode = newstr( "binary" );
            delete fn->mode;
            fn->mode = newmode;
            return;
        }
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFileName::removeAll()
{
    K_FILENAME* fn;
    for( long i = 0; i < arraycount; i++ ) {
        fn = (K_FILENAME*)filearray->get( i );
        delete fn->name;
        delete fn->mode;
    }
    arraycount = 0;
    filearray->clear();
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFileName::remove( K_FILENAME* findfn )
{
    K_FILENAME* fn;
    for( long i = 0; i < arraycount; i++ ) {
        fn = (K_FILENAME*)filearray->get( i );
        if( fn == findfn ) {
            delete fn->name;
            delete fn->mode;
            filearray->remove( i );
            arraycount--;
            return;
        }
    }
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KFileName::executeSend()
{
    char tmp[1024];
    K_FILENAME* fn;

    ::putCmdString( "clear send-list\n" );

    for( long i = 0; i < arraycount; i++ )
    {
        fn = (K_FILENAME*)filearray->get( i );
        tmp[0] = '\0';
        strcat( tmp, "add send-list " );
        strcat( tmp, fn->name );
        strcat( tmp, " " );
        strcat( tmp, fn->mode );
        strcat( tmp, "\n" );
        ::putCmdString( tmp );
    }

    ::putCmdString( "send\n" );
}

