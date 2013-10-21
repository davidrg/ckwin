char *cknnbiv = "Win32 NetBios support, 8.0.001, 6 Feb 97";

/*  C K N N B I  --  Win32-specific NetBios support  */

/*
  COPYRIGHT NOTICE:

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

/*
   developed by Jeffrey Altman <jaltman@secure-endpoints.com>
*/

#ifdef CK_NETBIOS
#include <windows.h>
#include "ckcdeb.h"
#include "ckcker.h"
#include "ckonbi.h"

#define No 0
#define Yes 1

static USHORT Netbeui_Handle[NB_ADAPTERS]={0,0,0,0};
static USHORT Netentries=0;

extern int pid ;
extern int ttyfd ;
extern USHORT netbiosAvail ;
extern BOOL NetbeuiAPI = 0 ;

UCHAR  NetBiosRemote[NETBIOS_NAME_LEN+1] = "                \0" ;
UCHAR  NetBiosName[NETBIOS_NAME_LEN+1] = "                \0" ;
UCHAR  NetBiosAdapter = 0 ;
BYTE   NetBiosLSN = 0 ;
HANDLE hevNetBiosLSN = 0 ;
PNCB   pWorkNCB = 0 ;
PNCB   pListenNCB = 0 ;
PNCB   pRecvNCB = 0 ;
PNCB   pSendNCB[MAXWS] ;
HANDLE ListenThreadID = -1 ;
BYTE   NetBiosRecvBuf[MAXRP]="" ;
BYTE * NetBiosSendBuf[MAXWS] ;
USHORT MaxCmds=0,MaxSess=0,MaxNames=0,MaxWs=0 ;

USHORT
NCBReset( BOOL Netbeui, PNCB Ncb, USHORT lana,USHORT  sessions,
                  USHORT  commands,USHORT names )
{
    int i,rc=NB_INADEQUATE_RESOURCES,nrc=0;
    if(!Netbeui)
    {
        memset( Ncb, 0, BNCBSIZE ) ;
        Ncb->ncb_command  = NB_RESET_WAIT;
        Ncb->ncb_lana_num = lana;
        Ncb->req_sessions = sessions;
        Ncb->req_commands = commands;
        Ncb->req_names = names;

        nrc = Netbios(Ncb );

        rc=Ncb->ncb_retcode;
    } /* end if */
    return rc;
}

USHORT
NCBConfig(BOOL Netbeui, PNCB Ncb, USHORT lana, PUSHORT sessions,
           PUSHORT commands, PUSHORT names )
{
    SHORT rc;

    *sessions=0;
    *commands=0;
    *names=   0;

    if(!Netbeui)
    {
        debug(F100,"ckonbi:NCBConfig: About to NCBReset","",0);
        rc=NCBReset(Netbeui, Ncb, lana, 255, 255,255 );
        debug(F111,"ckonbi: NCBConfig","NCBReset",rc);
        if ( rc == NB_COMMAND_SUCCESSFUL ||
             rc == NB_INADEQUATE_RESOURCES ) {
            *sessions=Ncb->act_sessions;
            *commands=Ncb->act_commands;
            *names=   Ncb->act_names;
        }
        if ( rc == NB_COMMAND_SUCCESSFUL ) {
            debug(F100,"ckonbi:NCBConfig: About to NCBClose","",0);
            NCBClose(Netbeui,Ncb,lana);
        }
    } /* end if */

    return rc;
}

USHORT
GetNCBConfig(BOOL Netbeui,USHORT Adapter,PUSHORT S,PUSHORT C, PUSHORT N)
{
    NCB TempNCB;
    return NCBConfig(Netbeui,&TempNCB,Adapter,S,C,N);
}

void
os2_netbiosinit( void ) {
    int rc = 1, i ;

    if (deblog)
    {
        printf( "  NetBIOS support..." ) ;
        debug(F101,"NetBIOS support...","",NetBiosAdapter);
    }

    if ( NetBiosAdapter < 0 ) {
        /* NetBios support has been disabled */
        if (deblog)
        {
            printf( "has been disabled\n" ) ;
            debug(F100,"has been disabled","",0);
        }
        return ;
    }

    /* Initialize Send NCBs to zero */
    for ( i=0; i<MAXWS ; i++ )
    {
        pSendNCB[i] = 0 ;
        NetBiosSendBuf[i] = malloc( MAXSP ) ;
    }

    rc = netbios_avail( NetbeuiAPI=0 ) ;  /* Microsoft Windows NETBIOS 3.0  */

    if ( !rc ) {
        char semname[19] ;

        if (deblog)
        {
            printf( "NETBIOS 3.0 OK\n") ;
            debug(F100,"NETBIOS 3.0 OK","",0);
        }

#ifdef COMMENT
        DosCreateEventSem( NULL, &hevNetBiosLSN, 0, 0 );

        sprintf(semname,"\\SEM\\%#6.6X.CK%s",pid,"W");
        if ( pWorkNCB = (PNCB) malloc( NCBSIZE ) )
            rc = Dos16CreateSem( DC_SEM_SHARED,
                                 (PHSEM16) &(pWorkNCB->ncb_semaphore),
                                 semname ) ;

        sprintf(semname,"\\SEM\\%#6.6X.CK%s",pid,"L");
        if ( pListenNCB = (PNCB) malloc( NCBSIZE ) )
            rc = Dos16CreateSem( DC_SEM_SHARED,
                                 (PHSEM16) &(pListenNCB->ncb_semaphore),
                                 semname ) ;

        sprintf(semname,"\\SEM\\%#6.6X.CK%s",pid,"R");
        if ( pRecvNCB = (PNCB) malloc( NCBSIZE ) )
            rc = Dos16CreateSem( DC_SEM_SHARED,
                                 (PHSEM16) &(pRecvNCB->ncb_semaphore),
                                 semname ) ;

        for ( i=0 ; i < MAXWS ; i++ ) {
            sprintf(semname,"\\SEM\\%#6.6X.%s%2d",pid,"S",i);
            if ( pSendNCB[i] = (PNCB) malloc( NCBSIZE ) )
                rc = Dos16CreateSem( DC_SEM_SHARED,
                                     (PHSEM16) &(pSendNCB[i]->ncb_semaphore),
                                     semname ) ;
        }
#endif /* COMMENT */

        debug(F111,"os2_netbiosinit - About to GetNCBConfig","",0);
        GetNCBConfig( NetbeuiAPI, NetBiosAdapter, &MaxSess,
                      &MaxCmds, &MaxNames ) ;
        debug( F101,"MaxSess","",MaxSess ) ;
        debug( F101, "MaxCmds", "", MaxCmds ) ;
        debug( F101, "MaxNames", "", MaxNames ) ;

        if ( NB_LSN < MaxSess )
            MaxSess = NB_LSN ;
        if ( NB_NCB < MaxCmds )
            MaxCmds = NB_NCB ;
        MaxWs = MaxCmds - 3 ;
        if ( MAXWS  < MaxWs )
            MaxWs = MAXWS ;
        if ( NB_NAMES < MaxNames )
            MaxNames = NB_NAMES ;

        rc = NCBReset( NetbeuiAPI, pWorkNCB, NetBiosAdapter,
                       MaxSess, MaxCmds, MaxNames ) ;

        if ( rc ) {
            if (deblog)
            {
                printf( "NetBIOS Adapter Reset failed\n" ) ;
                debug(F100,"NetBIOS Adapter Reset failed","",0);
            }
        }
        else {
            netbiosAvail = 1 ;
            if (deblog)
            {
                printf( "NetBIOS Adapter Reset successful\n" ) ;
                debug(F100,"NetBIOS Adapter Reset successful","",0);
            }
        }
    }
    else if (deblog)
    {
        printf( "Not installed\n" ) ;
        debug(F100,"Not installed","",0);
    }
}

void
os2_netbioscleanup( void ) {
    if ( netbiosAvail )
    {
        int i ;

        if (strcmp( NetBiosName, "                " ) )
            NCBDeleteName( NetbeuiAPI, pWorkNCB, NetBiosAdapter, NetBiosName ) ;

        NCBClose( NetbeuiAPI, pWorkNCB, NetBiosAdapter ) ;

#ifdef COMMENT
        if ( pWorkNCB )
        {
            Dos16CloseSem( pWorkNCB->ncb_semaphore ) ;
            free( pWorkNCB ) ;
            pWorkNCB = 0 ;
         }

        if ( pListenNCB )
        {
            Dos16CloseSem( pListenNCB->ncb_semaphore ) ;
            free( pListenNCB ) ;
            pListenNCB = 0 ;
        }

        if ( pRecvNCB )
        {
            Dos16CloseSem( pRecvNCB->ncb_semaphore ) ;
            free( pRecvNCB ) ;
            pRecvNCB = 0 ;
        }

        for ( i=0 ; i<MAXWS ; i++ ) {
            if ( pSendNCB[i] )
            {
                Dos16CloseSem( pSendNCB[i]->ncb_semaphore ) ;
                free( pSendNCB[i] ) ;
                pSendNCB[i] = 0 ;

                if ( NetBiosSendBuf[i] )
                {
                    free( NetBiosSendBuf[i] ) ;
                    NetBiosSendBuf[i] = 0 ;
                }
            }
        }

        DosCloseEventSem( hevNetBiosLSN );
#endif /* COMMENT */
    }
}

#pragma stack16(256)
VOID CDECL16
ncbpost(USHORT Junk, PNCB16 NcbPointer)
{
    APIRET rc = 0 ;
    NCB ncb = *NcbPointer ;
    rc = Dos16SemClear(NcbPointer->ncb_semaphore);
}


USHORT
NCBClose( BOOL Netbeui, PNCB Ncb, USHORT lana)
{
    USHORT rc;
    if(!Netbeui)
    {
        memset( Ncb, 0, BNCBSIZE ) ;
        Ncb->ncb_command  = NB_RESET_WAIT;
        Ncb->ncb_lsn=255;
        Ncb->ncb_lana_num = lana;
        rc=Netbios(Ncb);
    } /* end if */
    debug(F111,"ckonbi:NCBClose","rc",rc);
    return rc;
}
USHORT
NCBAddName( BOOL Netbeui, PNCB Ncb,USHORT lana, PBYTE name )
{
    memset( Ncb, 0, BNCBSIZE ) ;
    Ncb->bncb.ncb_command  = NB_ADD_NAME_WAIT;
    Ncb->bncb.ncb_lana_num = lana;
    strncpy( Ncb->bncb.ncb_name, name, NETBIOS_NAME_LEN );
    if(!Netbeui)
    {
        Netbios(Ncb );
    } /* end else */

    return (Ncb->bncb.ncb_retcode);
}

USHORT
NCBDeleteName(BOOL Netbeui, PNCB Ncb, USHORT lana, PBYTE name )
{
    memset( Ncb, 0, BNCBSIZE ) ;
    Ncb->bncb.ncb_command  = NB_DELETE_NAME_WAIT;
    Ncb->bncb.ncb_lana_num = lana;
    strncpy( Ncb->bncb.ncb_name, name, NETBIOS_NAME_LEN );

    if(!Netbeui)
    {
        Netbios(Ncb );
    }

    return (Ncb->bncb.ncb_retcode);
}
USHORT
NCBAddGroupName(BOOL Netbeui, PNCB Ncb,USHORT lana, PBYTE name )
{
    memset( Ncb, 0, BNCBSIZE ) ;
    Ncb->bncb.ncb_command  = NB_ADD_GROUP_NAME_WAIT;
    Ncb->bncb.ncb_lana_num = lana;
    strncpy( Ncb->bncb.ncb_name, name, NETBIOS_NAME_LEN );

    if(!Netbeui)
    {
        Netbios(Ncb );
    }

    return (Ncb->bncb.ncb_retcode);
}

USHORT
NCBCall(BOOL Netbeui, PNCB  Ncb, USHORT lana, PBYTE lclname,
         PBYTE rmtname,USHORT recv_timeout,USHORT send_timeout,BOOL wait)
{
    int rc;
    ;

    memset( Ncb, 0, BNCBSIZE ) ;
    Ncb->bncb.ncb_command  = (wait)?NB_CALL_WAIT:NB_CALL;
    Ncb->bncb.ncb_lana_num = lana;
    Ncb->bncb.ncb_rto      = recv_timeout*2;  /* times 2 since in   */
    Ncb->bncb.ncb_sto      = send_timeout*2;  /* steps of 500 msecs */

    if ( !Netbeui )
    {
        Ncb->bncb.off44.ncb_post_address=
            (address)((!wait)?ncbpost:NULL);
    }

    strncpy( Ncb->bncb.ncb_name, lclname, NETBIOS_NAME_LEN );
    strncpy( Ncb->bncb.ncb_callname, rmtname, NETBIOS_NAME_LEN );
    if(!wait)
        rc= Dos16SemSet(Ncb->ncb_semaphore);

    if(!Netbeui)
    {
        rc=Netbios(Ncb );
    }

    return (wait)?Ncb->bncb.ncb_retcode:rc;
}

USHORT
NCBListen(BOOL Netbeui, PNCB  Ncb, USHORT lana, PBYTE lclname,
           PBYTE rmtname,USHORT recv_timeout,USHORT send_timeout,BOOL wait)
{
    int rc;
    ;
    memset( Ncb, 0, BNCBSIZE ) ;
    Ncb->bncb.ncb_command  = (wait)?NB_LISTEN_WAIT:NB_LISTEN;
    Ncb->bncb.ncb_lana_num = lana;
    Ncb->bncb.ncb_rto      = recv_timeout*2; /* times 2 since in   */
    Ncb->bncb.ncb_sto      = send_timeout*2; /* steps of 500 msecs */

    if ( !Netbeui )
    {
        Ncb->bncb.off44.ncb_post_address=
            (address)((!wait)?ncbpost:NULL);
    }

    strncpy( Ncb->bncb.ncb_name, lclname, NETBIOS_NAME_LEN );
    strncpy( Ncb->bncb.ncb_callname, rmtname, NETBIOS_NAME_LEN );
    if(!wait)
    {
        rc= Dos16SemSet(Ncb->ncb_semaphore);
    }

    if(!Netbeui)
    {
        rc=Netbios(Ncb );
    }

    return (wait)?Ncb->bncb.ncb_retcode:rc;
}

/**********************************************************************/
/*
** NCBSend      Sends data to the session partner as defined by the
**              session number in the NCB.LSN field.  The data to send
**              is in the buffer pointed to by the NCB.BUFFER field.
**
**              Accepts the adapter number, the session number,
**              the char array holding the message to be sent, and
**              the length of the message in that array.
**
**              Returns the NCB return code.
*/

USHORT  NCBSend(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE message, word length,BOOL wait)
{
    int rc;

    memset( Ncb, 0, BNCBSIZE ) ;
    Ncb->bncb.ncb_command  = (wait)?NB_SEND_WAIT:NB_SEND;
    Ncb->bncb.ncb_lana_num = lana;
    Ncb->bncb.ncb_lsn      = lsn;
    Ncb->bncb.ncb_buffer_address = message;
    Ncb->bncb.ncb_length   = length;

    if ( !Netbeui )
    {
        Ncb->bncb.off44.ncb_post_address=
            (address)((!wait)?ncbpost:NULL);
    }

    if(!wait)
        rc= Dos16SemSet(Ncb->ncb_semaphore);

    if(!Netbeui)
    {
        rc=Netbios(Ncb );
    }

    return (wait)?Ncb->bncb.ncb_retcode:rc;
}

USHORT NCBSendDatagram(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PSZ rmtname, PBYTE message, word length,BOOL wait)
{
    int rc;

    memset( Ncb, 0, BNCBSIZE ) ;
    Ncb->bncb.ncb_command  =
                     (wait)?NB_SEND_DATAGRAM_WAIT:NB_SEND_DATAGRAM;
    Ncb->bncb.ncb_lana_num = lana;
    Ncb->bncb.ncb_num      = lsn;
    Ncb->bncb.ncb_buffer_address = message;
    Ncb->bncb.ncb_length   = length;

    if ( !Netbeui )
    {
        Ncb->bncb.off44.ncb_post_address=
            (address)((!wait)?ncbpost:NULL);
    }

    strncpy( Ncb->bncb.ncb_callname, rmtname, NETBIOS_NAME_LEN );
    if(!wait)
      rc= Dos16SemSet(Ncb->ncb_semaphore );

    if(!Netbeui)
    {
        rc=Netbios(Ncb );
    }

    return (wait)?Ncb->bncb.ncb_retcode:rc;
}

USHORT
NCBSendBroadcast(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE message, word length,BOOL wait)
{
    int rc;

   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->bncb.ncb_command  =
         (wait)?NB_SEND_BROADCAST_DATAGRAM_WAIT:NB_SEND_BROADCAST_DATAGRAM;
   Ncb->bncb.ncb_lana_num = lana;
   Ncb->bncb.ncb_num      = lsn;
   Ncb->bncb.ncb_buffer_address = message;
   Ncb->bncb.ncb_length   = length;

   if ( !Netbeui )
   {
      Ncb->bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->ncb_semaphore );

   if(!Netbeui)
   {
       rc=Netbios(Ncb );
   }

   return (wait)?Ncb->bncb.ncb_retcode:rc;
}

USHORT
NCBSendNoAck(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE message, word length,BOOL wait)
{
    int rc;
 ;
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->bncb.ncb_command  =
                        (wait)?NB_SEND_NO_ACK_WAIT:NB_SEND_NO_ACK;
   Ncb->bncb.ncb_lana_num = lana;
   Ncb->bncb.ncb_lsn      = lsn;
   Ncb->bncb.ncb_buffer_address = message;
   Ncb->bncb.ncb_length   = length;

   if ( !Netbeui )
   {
      Ncb->bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

  if(!wait)
    rc= Dos16SemSet(Ncb->ncb_semaphore);

  if(!Netbeui)
  {
      rc=Netbios(Ncb );
  }

  return (wait)?Ncb->bncb.ncb_retcode:rc;
}
/**********************************************************************/
/*
** NCBChainSend      Sends data to the session partner as defined by the
**              session number in the NCB.LSN field.  The data to send
**              is in the buffer pointed to by the NCB.BUFFER field.
**
**              Accepts the adapter number, the session number,
**              the char array holding the message to be sent, and
**              the length of the message in that array.
**
**              Returns the NCB return code.
*/

USHORT
NCBChainSend(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE  message, word length, PBYTE Buffer2, word Length2,BOOL wait)
{
    int rc;
    PBuf2 b2;

   memset( Ncb, 0, BNCBSIZE ) ;
   b2=(PBuf2)&Ncb->bncb.ncb_callname;
   Ncb->bncb.ncb_command  =
                             (wait)?NB_CHAIN_SEND_WAIT:NB_CHAIN_SEND;
   Ncb->bncb.ncb_lana_num = lana;
   Ncb->bncb.ncb_lsn      = lsn;
   Ncb->bncb.ncb_buffer_address = message;
   Ncb->bncb.ncb_length   = length;

   if ( !Netbeui )
   {
      Ncb->bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   b2->Length=Length2;
   b2->Buffer=Buffer2;
   if(!wait)
     rc= Dos16SemSet(Ncb->ncb_semaphore);

   if(!Netbeui)
   {
       rc=Netbios(Ncb );
   }

   return (wait)?Ncb->bncb.ncb_retcode:rc;
}

USHORT
NCBChainSendNoAck(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE message, word length, PBYTE Buffer2, word Length2,BOOL wait)
{
    int rc;
    PBuf2 b2;

    memset( Ncb, 0, BNCBSIZE ) ;
   b2=(PBuf2)&Ncb->bncb.ncb_callname;
   Ncb->bncb.ncb_command  =
                  (wait)?NB_CHAIN_SEND_NO_ACK_WAIT:NB_CHAIN_SEND_NO_ACK;
   Ncb->bncb.ncb_lana_num = lana;
   Ncb->bncb.ncb_lsn      = lsn;
   Ncb->bncb.ncb_buffer_address = message;
   Ncb->bncb.ncb_length   = length;

   if ( !Netbeui )
   {
      Ncb->bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   b2->Length=Length2;
   b2->Buffer=Buffer2;
   if(!wait)
     rc= Dos16SemSet(Ncb->ncb_semaphore);

   if(!Netbeui)
   {
       rc=Netbios(Ncb );
   }

   return (wait)?Ncb->bncb.ncb_retcode:rc;
}

/**********************************************************************/
/*
** NCBReceive   Receives data from the session partner that sends data
**              to this station.
**
**              Accepts the adapter number, the session number,
**              the char array to hold the message received, and
**              the maximum length the message may occupy in that
**              array.
**
**              Returns the NCB return code and, if successful,
**              the received data in the buffer.
*/

USHORT
NCBReceive(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE buffer, word length, BOOL wait )
{
    int rc;

   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->bncb.ncb_command  = (wait)?NB_RECEIVE_WAIT:NB_RECEIVE;
   Ncb->bncb.ncb_lana_num = lana;
   Ncb->bncb.ncb_lsn      = lsn;
   Ncb->bncb.ncb_buffer_address = buffer;
   Ncb->bncb.ncb_length   = length;

   if ( !Netbeui )
   {
      Ncb->bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->ncb_semaphore);

   if(!Netbeui)
   {
       rc=Netbios(Ncb );
   }

   return (wait)?Ncb->bncb.ncb_retcode:rc;
}
/**********************************************************************/
/*
** NCBReceiveAny Receives data from the session partner that sends data
**              to this station.
**
**              Accepts the adapter number, the session number,
**              the char array to hold the message received, and
**              the maximum length the message may occupy in that
**              array.
**
**              Returns the NCB return code and, if successful,
**              the received data in the buffer.
*/

USHORT
NCBReceiveAny(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE buffer, word length,BOOL wait )
{
    int rc;

   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->bncb.ncb_command  =
                           (wait)?NB_RECEIVE_ANY_WAIT:NB_RECEIVE_ANY;
   Ncb->bncb.ncb_lana_num = lana;
   Ncb->bncb.ncb_num      = lsn;
   Ncb->bncb.ncb_buffer_address = buffer;
   Ncb->bncb.ncb_length   = length;

   if ( !Netbeui )
   {
      Ncb->bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->ncb_semaphore );

   if(!Netbeui)
   {
       rc=Netbios(Ncb );
   }

   return (wait)?Ncb->bncb.ncb_retcode:rc;
}

USHORT
NCBReceiveDatagram(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE buffer, word length,BOOL wait )
{
    int rc;

   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->bncb.ncb_command  =
               (wait)?NB_RECEIVE_DATAGRAM_WAIT:NB_RECEIVE_DATAGRAM;
   Ncb->bncb.ncb_lana_num = lana;
   Ncb->bncb.ncb_num      = lsn;
   Ncb->bncb.ncb_buffer_address = buffer;
   Ncb->bncb.ncb_length   = length;

   if ( !Netbeui )
   {
      Ncb->bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->ncb_semaphore );

   if(!Netbeui)
   {
       rc=Netbios(Ncb );
   }

   return (wait)?Ncb->bncb.ncb_retcode:rc;
}

USHORT
NCBReceiveBroadcast(BOOL Netbeui, PNCB  Ncb,USHORT  lana, USHORT lsn,
   PBYTE buffer, word length,BOOL wait )
{
    int rc;
    ;
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->bncb.ncb_command  =
       (wait)?NB_RECEIVE_BROADCAST_DATAGRAM_W:NB_RECEIVE_BROADCAST_DATAGRAM;
   Ncb->bncb.ncb_lana_num = lana;
   Ncb->bncb.ncb_num      = lsn;
   Ncb->bncb.ncb_buffer_address = buffer;
   Ncb->bncb.ncb_length   = length;

   if ( !Netbeui )
   {
      Ncb->bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->ncb_semaphore );

   if(!Netbeui)
   {
       rc=Netbios(Ncb );
   }

   return (wait)?Ncb->bncb.ncb_retcode:rc;
}

/**********************************************************************/
/*
** NCBHangup    Closes the session with another name on the network
**              specified by the session number.
**
**              Accepts the adapter number and session number.
**
**              Returns the NCB return code.
*/

USHORT
NCBHangup(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn )
{
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->bncb.ncb_command  = NB_HANG_UP_WAIT;
   Ncb->bncb.ncb_lana_num = lana;
   Ncb->bncb.ncb_lsn      = lsn;

   if(!Netbeui)
   {
     Netbios(Ncb );
   }

   return (Ncb->bncb.ncb_retcode);
}

/**********************************************************************/
/*
** NCBCancel    Cancels a previously issued NCB if possible.
**
**              Accepts the adapter number and a ptr to the NCB to cancel.
**
**              Returns the NCB return code.
*/

USHORT
NCBCancel(BOOL Netbeui, PNCB  Ncb, USHORT lana, PNCB NcbToCancel)
{
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->bncb.ncb_command  = NB_CANCEL_WAIT;
   Ncb->bncb.ncb_lana_num = lana;
   Ncb->bncb.ncb_buffer_address = (address)NcbToCancel;

   if(!Netbeui)
   {
       Netbios(Ncb );
   }

   return (Ncb->bncb.ncb_retcode);
}

/**********************************************************************/
/*
** NCBStatus
**
**              Accepts the adapter number and a ptr to the NCB to cancel.
**
**              Returns the NCB return code.
*/

USHORT
NCBStatus(BOOL Netbeui, PNCB Ncb, USHORT lana, PBYTE callname,
   PSTATUSINFO pInfo, word length, BOOL wait )
{
    int rc;

   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->bncb.ncb_command  = (wait)?NB_STATUS_WAIT:NB_STATUS;
   Ncb->bncb.ncb_lana_num = lana;
   strncpy( Ncb->bncb.ncb_callname, callname, NETBIOS_NAME_LEN );
   Ncb->bncb.ncb_buffer_address = (PBYTE) pInfo;
   Ncb->bncb.ncb_length   = length;

   if ( !Netbeui )
   {
      Ncb->bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->ncb_semaphore);

   if(!Netbeui)
   {
       rc=Netbios(Ncb );
   }

   return (wait)?Ncb->bncb.ncb_retcode:rc;
}

USHORT
NCBSessionStatus(BOOL Netbeui, PNCB Ncb, USHORT lana, PBYTE lclname,
   PSESSIONINFO pInfo, word length, BOOL wait )
{
    int rc;

   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->bncb.ncb_command  =
                     (wait)?NB_SESSION_STATUS_WAIT:NB_SESSION_STATUS;
   Ncb->bncb.ncb_lana_num = lana;
   strncpy( Ncb->bncb.ncb_name, lclname, NETBIOS_NAME_LEN );
   Ncb->bncb.ncb_buffer_address = (PBYTE) pInfo;
   Ncb->bncb.ncb_length   = length;

   if ( !Netbeui )
   {
      Ncb->bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->ncb_semaphore);

   if(!Netbeui)
   {
       rc=Netbios(Ncb );
   }

   return (wait)?Ncb->bncb.ncb_retcode:rc;
}

VOID
NCBCancelOutstanding( VOID )
{
    if (netbiosAvail) {
       int i;
       NCB CancelNCB;
       /* Cancel all outstanding Netbios Send or Work requests */

       for ( i = 0 ; i < MAXWS ; i++) {
           if (pSendNCB[i]->bncb.ncb_retcode ==
               NB_COMMAND_IN_PROCESS ) {
               NCBCancel( NetbeuiAPI,
                         &CancelNCB,
                         NetBiosAdapter,
                         pSendNCB[i] ) ;
               Dos16SemWait( pSendNCB[i]->ncb_semaphore,
                            SEM_INDEFINITE_WAIT);
           }
       }
       if ( pWorkNCB->bncb.ncb_retcode == NB_COMMAND_IN_PROCESS ) {
           NCBCancel( NetbeuiAPI, &CancelNCB, NetBiosAdapter, pWorkNCB ) ;
           Dos16SemWait( pWorkNCB->ncb_semaphore,
                        SEM_INDEFINITE_WAIT ) ;
       }
   }
}
#endif /* CK_NETBIOS */
