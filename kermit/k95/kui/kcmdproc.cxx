#include "kcmdproc.hxx"
#include "karray.hxx"
#include "ikcmd.h"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdProc::KCmdProc( K_GLOBAL* g )
    : kglob( g )
    , hInst( g->hInst )
    , hWnd( 0 )
    , cmdarray( new KArray() )
    , arraycount( 0L )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KCmdProc::~KCmdProc() 
{
    char* cmd;
    for( long i = 0; i < arraycount; i++ )
    {
        cmd = (char*) cmdarray->get( i );
        delete cmd;
    }
    delete cmdarray;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KCmdProc::bufferCommand( char* c )
{
    char* cmd = newstr( c );
    cmdarray->put( (void*)cmd, arraycount++ );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KCmdProc::verifyChanges()
{
    // cmdarray is a FIFO queue
    //
    int ret;
    char* cmd = (char*) cmdarray->get( 0 );
    while( cmd )
    {
        ret = putCmdString( cmd );
        printf( "%s\n", cmd );
//        if( ret == 0 ) {
//            // the command that failed is left at the top of the stack
//            return FALSE;
//        }
        cmdarray->remove( 0 );
        cmd = (char*) cmdarray->get( 0 );
    }

    return TRUE;
}
