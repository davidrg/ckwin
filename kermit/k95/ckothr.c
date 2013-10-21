/* C K O T H R  --  Kermit thread management functions for OS/2 and NT systems */

/*
  Author: Jeffrey Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1996,2004, Trustees of Columbia University in the City of New
  York.
*/

#include "ckcdeb.h"                     /* Debug & other symbols */
#ifdef NT
#include <windows.h>
#else /* NT */
#define INCL_WIN
#define INCL_VIO
#define INCL_ERRORS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVIOCTL
#define INCL_WINCLIPBOARD
#define INCL_DOSDATETIME
#include <os2.h>
#undef COMMENT                /* COMMENT is defined in os2.h */
#endif /* NT */
#include "ckothr.h"

#ifdef NT
#include <setjmpex.h>
#else /* NT */
#include <setjmp.h>
#endif /* NT */
#include "ckcsig.h"
extern int isinterrupted ;

#define MAXTHREAD 16
struct _threadinfo threads[MAXTHREAD];

static int
findfreeindex( void )
{
   int i = 0 ;
   for ( i = 0 ; i<MAXTHREAD ; i++ )
   {
      if ( !threads[i].inuse )
         return i ;
   }
   return -1 ;
}

static int
#ifdef NT
findthreadindex( HANDLE id )
#else
findthreadindex( TID id )
#endif
{
   int i = 0 ;
   for ( i = 0 ; i<MAXTHREAD ; i++ )
   {
      if ( threads[i].inuse && threads[i].id == id )
      {
         debug(F101,"ckothr findthreadindex - found","",i);
         return i ;
      }
   }
   debug(F101,"ckothr findthreadindex - not found","",-1);
   return -1 ;
}

static void
insertchildintoparent( int child, int parent )
{
   int sibling = 0 ;

   debug(F101,"ckothr insertchildintoparent - parent","",parent) ;
   debug(F101,"ckothr insertchildintoparent - child","",child) ;

   if ( parent < 0 || parent >= MAXTHREAD )
      return ;

   if ( threads[parent].child < 0 )
   {
      threads[parent].child = child ;
   }
   else
   {
      sibling = threads[parent].child ;
      while ( threads[sibling].sibling >= 0 )
         sibling = threads[sibling].sibling ;
      threads[sibling].child = child ;
   }
}

static void
removechildfromparent( int child, int parent )
{
   int sibling = 0 ;

   debug(F101,"ckothr removechildfromparent - parent","",parent) ;
   debug(F101,"ckothr removechildfromparent - child","",child) ;

   if ( parent < 0 || parent >= MAXTHREAD )
      return ;


   if ( threads[parent].child < 0 )
   {
      return ;
   }
   else if ( threads[parent].child == child )
   {
      threads[parent].child = threads[child].sibling ;
#ifdef NT
      threads[child].parent = (HANDLE) -1 ;
#else
      threads[child].parent = (TID) -1 ;
#endif

   }
   else
   {
      sibling = threads[parent].child ;
      while ( threads[sibling].sibling != child &&
              threads[sibling].sibling >= 0)
         sibling = threads[sibling].sibling ;

      if ( threads[sibling].sibling < 0 )
         return ;

      threads[sibling].sibling = threads[child].sibling ;
   }
}

/*
    If manage is TRUE, then we will add the new thread to our thread structures.
    This requires that parent be a valid thread handle
*/
TID
#ifdef NT
ckThreadBegin( void (*func)(void *), ULONG stacksize, void * param, BOOL manage, HANDLE parent )
#else
ckThreadBegin( void (*func)(void *), ULONG stacksize, void * param, BOOL manage, TID parent )
#endif
{
    int newthread, parentthread ;
#ifdef NT
    HANDLE hevThread ;
#else /* NT */
    HEV hevThread ;
#endif /* NT */

    while (RequestThreadMgmtMutex(250))
    {
#ifdef NTSIG
        ck_ih();
#else
        ;
#endif /* NTSIG */
    }

    if ( manage )
    {
        newthread = findfreeindex() ;
        debug(F101,"ckothr ckThreadBegin newthread","",newthread ) ;
        if ( newthread < 0 )
        {
            ReleaseThreadMgmtMutex() ;
            return (TID) -1 ;
        }

        threads[newthread].inuse = TRUE ;
        threads[newthread].child = -1 ;
        threads[newthread].sibling = -1 ;
        threads[newthread].id = 0 ;


        if (param)
            threads[newthread].CompletionSem =
#ifdef NT
                *((HANDLE *) param) ;
#else
                *((HEV *) param) ;
#endif /* NT*/

      /* create an event semaphore for new thread        */
#ifdef NT
        hevThread = CreateEvent( NULL, TRUE, FALSE, NULL ) ;
#else
        DosCreateEventSem( NULL, &hevThread, DC_SEM_SHARED, FALSE ) ;
#endif /* NT */
        threads[newthread].DieSem = hevThread ;

        threads[newthread].id = (TID) _beginthread( func,
#ifndef NT
                                                  0,
#endif /* NT */
                                                  stacksize,
                                                  (void *)&threads[newthread]
                                                  ) ;

        threads[newthread].parent = parent ;
        parentthread = findthreadindex( parent ) ;
        insertchildintoparent( newthread, parentthread ) ;

        debug(F101,"ckothr threads->inuse","",  threads[newthread].inuse) ;
        debug(F101,"ckothr threads->child","",  threads[newthread].child) ;
        debug(F101,"ckothr threads->sibling","",threads[newthread].sibling) ;
        debug(F101,"ckothr ckThreadBegin TID","",threads[newthread].id) ;
        ReleaseThreadMgmtMutex() ;
        return threads[newthread].id;
   }
   else
   {
       TID tid ;
       tid = (TID) _beginthread( func,
#ifndef NT
                                0,
#endif /* NT */
                                stacksize,
                                param ) ;
       debug(F101,"ckothr ckThreadBegin TID","",tid) ;
       ReleaseThreadMgmtMutex() ;
       return tid;
   }
}

BOOL
ckThreadKill( TID startthread )
{
    int found = 0 ;
    int where = 999 ;
    int i ;
    int myindex = 0, parentindex = 0 ;

    while (RequestThreadMgmtMutex(250))
    {
#ifdef NTSIG
        ck_ih();
#else
        ;
#endif /* NTSIG */
    }

    debug(F101,"ckothr ckThreadKill TID","",startthread ) ;
    myindex = findthreadindex( startthread ) ;
    if ( myindex >= 0 )
    {
        debug(F101,"ckothr threads->inuse","",  threads[myindex].inuse) ;
        debug(F101,"ckothr threads->child","",  threads[myindex].child) ;
        debug(F101,"ckothr threads->sibling","",threads[myindex].sibling) ;

        parentindex = findthreadindex( threads[myindex].parent ) ;
        removechildfromparent( myindex, parentindex ) ;

        while ( threads[myindex].child >= 0 )
        {
            int child = threads[myindex].child ;
            ckThreadKill( threads[child].id ) ;

            debug(F101,"ckothr threads->inuse","",  threads[myindex].inuse) ;
            debug(F101,"ckothr threads->child","",  threads[myindex].child) ;
            debug(F101,"ckothr threads->sibling","",threads[myindex].sibling) ;

            if ( threads[myindex].child >= 0 )
            {
                debug(F101,"ckThreadKill failed to find kill child","",0);
                threads[myindex].child = -1 ;
            }
        }

#ifdef NT
        isinterrupted = 1 ;
        /* Not safe to terminate threads */
        /* TerminateThread( threads[threadindex].id, -1 ) ; */
        PostSem( threads[myindex].DieSem ) ;
        if ( threads[myindex].CompletionSem ) {
            WaitSem( threads[myindex].CompletionSem, SEM_INDEFINITE_WAIT );
            /* CompletionSem will be cleared by starting thread */
        }
        else
            msleep(50);

        /* we would like to wait for the completion of the thread but that is
           not currently practical
        */
        CloseHandle( threads[myindex].DieSem ) ;

#ifdef COMMENT
        /* Do not do this.  The handle will be closed by endthread() */
        if (threads[myindex].id) {
            CloseHandle( threads[myindex].id ) ;
        }
#endif /* COMMENT */
        threads[myindex].handle = 0 ;
#else /* NT */
        DosKillThread( threads[myindex].id ) ;
#endif /* NT */
        threads[myindex].id = 0 ;
        threads[myindex].CompletionSem = 0 ;
        threads[myindex].DieSem = 0 ;
        threads[myindex].inuse = 0 ;
    }

    ReleaseThreadMgmtMutex() ;
    return TRUE;
}

void
ckThreadEnd( void * param )
{
    struct _threadinfo * threadinfo ;
    debug(F100,"ckothr ckThreadEnd","",0);
    while (RequestThreadMgmtMutex(250))
    {
#ifdef NTSIG
        ck_ih();
#else
        ;
#endif /* NTSIG */
    }
    if ( param )
    {
        int myindex = 0 ;
        int parentindex = 0 ;

        threadinfo = (struct _threadinfo *) param ;
        debug(F101,"ckothr ckThreadEnd","",threadinfo->id) ;
        myindex = findthreadindex( threadinfo->id ) ;
        if ( myindex >= 0 )
        {
            debug(F101,"ckothr threads->inuse","",  threads[myindex].inuse) ;
            debug(F101,"ckothr threads->child","",  threads[myindex].child) ;
            debug(F101,"ckothr threads->sibling","",threads[myindex].sibling) ;

            parentindex = findthreadindex( threads[myindex].parent ) ;
            removechildfromparent( myindex, parentindex ) ;

            while ( threads[myindex].child >= 0 )
            {
                int child = threads[myindex].child ;
                ckThreadKill( threads[child].id ) ;

                debug(F101,"ckothr threads->inuse","",  threads[myindex].inuse) ;
                debug(F101,"ckothr threads->child","",  threads[myindex].child) ;
                debug(F101,"ckothr threads->sibling","",threads[myindex].sibling) ;
            }

            if ( threadinfo->CompletionSem )
                PostSem( threadinfo->CompletionSem ) ;
            /* if CompletionSem was set, then it was set by the thread started it */
#ifdef NT
            CloseHandle( threads[myindex].DieSem ) ;
#else /* NT */
            DosCloseEventSem( threads[myindex].DieSem ) ;
#endif /* NT */
            threads[myindex].id = 0 ;
            threads[myindex].CompletionSem = 0 ;
            threads[myindex].DieSem = 0 ;
            threads[myindex].inuse = 0 ;
        }
        else
        {
            PostSem( threadinfo->CompletionSem ) ;
#ifdef NT
            CloseHandle( threadinfo->DieSem ) ;
#else /* NT */
            DosCloseEventSem( threadinfo->DieSem ) ;
#endif /* NT */
            threadinfo->id = 0 ;
            threadinfo->CompletionSem = 0 ;
            threadinfo->DieSem = 0 ;
            threadinfo->inuse = 0 ;
        }
    }
    else
    {
        /* must look up the threadinfo block */
        /* Post the Completion Sem and then  */
        /* call ckThreadCleanup on the id    */
    }
    ReleaseThreadMgmtMutex() ;
    _endthread() ;
}

void
ckThreadDie( void * param )
{
   struct _threadinfo * threadinfo ;
   debug(F100,"ckothr ckThreadDie","",0);
#ifdef NT
   if ( param )
   {
      threadinfo = (struct _threadinfo *) param ;
      debug(F101,"ckothr ckThreadDie","",threadinfo->id) ;
      PostSem( threadinfo->CompletionSem ) ;

      debug(F101,"ckothr threads->inuse","",  threadinfo->inuse) ;
      debug(F101,"ckothr threads->child","",  threadinfo->child) ;
      debug(F101,"ckothr threads->sibling","",threadinfo->sibling) ;
   }
   else
   {
    /* must look up the threadinfo block */
    /* Post the Completion Sem and then  */
    /* call ckThreadCleanup on the id    */
   }
#endif
  _endthread() ;
}

