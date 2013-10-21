char *ckonbiv = "OS/2 NetBios support, 8.0.010, 18 Sep 96";

/*  C K O N B I  --  OS/2-specific NetBios support  */

/*
  COPYRIGHT NOTICE:

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

/*
   Author: Jeffrey E Altman <jaltman@secure-endpoints.com>
             Secure Endpoints Inc., New York City     

   Supports:
      Traditional NetBios interface:
         IBM Lan Adapter and Protocol Support
         IBM OS/2 Extended Services
         IBM Communication Manager/2

      Newer Netbeui Interface:
         Microsoft LAN Manager Requester
         IBM LAN Server Requester
         Novell Netware Requester
*/

#ifndef __32BIT__
#ifdef CK_NETBIOS
#undef CK_NETBIOS
#endif /* CK_NETBIOS */
#endif /* __32BIT__ */

#ifdef CK_NETBIOS
#ifdef NT
#include <windows.h>
#define APIRET16 UCHAR
#define APIENTRY16
#else /* NT */
#define INCL_NOPM
#define INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_ERRORS
#define INCL_DOSSEMAPHORES
#include <os2.h>
#undef COMMENT
#endif /* NT */

#include "ckcdeb.h"
#include "ckcker.h"
#ifndef NT
#include "ckonbi.h"
#endif /* NT */

#define No 0
#define Yes 1

#ifdef _loadds
#undef _loadds
#define _loadds
#endif

static APIRET16 (* APIENTRY16 netbios)(PNCB)=NULL;
static APIRET16 (* APIENTRY16 netbios_Submit)(USHORT, USHORT, PNCB)=NULL;
static APIRET16 (* APIENTRY16 netbios_Close)(USHORT, USHORT)=NULL;
static APIRET16 (* APIENTRY16 netbios_Open)(PSZ, PSZ, USHORT, PUSHORT)=NULL;
static APIRET16 (* APIENTRY16 netbios_Enum)(PSZ, USHORT, PBYTE,USHORT,
   PUSHORT,PUSHORT)=NULL;

static USHORT Netbeui_Handle[NB_ADAPTERS]={0,0,0,0};
static PNETINFO1 pNetinfo=NULL;
static USHORT Netentries=0;

extern int pid ;
extern int ttyfd ;
extern USHORT netbiosAvail ;
extern BOOL NetbeuiAPI = 0 ;

UCHAR  NetBiosRemote[NETBIOS_NAME_LEN+1] = "                \0" ;
UCHAR  NetBiosName[NETBIOS_NAME_LEN+1] = "                \0" ;
UCHAR  NetBiosAdapter = 0 ;
BYTE   NetBiosLSN = 0 ;
HEV    hevNetBiosLSN = 0 ;
PNCB   pWorkNCB = 0 ;
PNCB   pListenNCB = 0 ;
PNCB   pRecvNCB = 0 ;
PNCB   pSendNCB[MAXWS] ;
TID    ListenThreadID = -1 ;
#ifndef NT
#pragma seg16 (NetBiosRecvBuf)
#endif /* NT */
BYTE   NetBiosRecvBuf[MAXRP]="" ;
#ifndef NT
#pragma seg16 (NetBiosSendBuf)
#endif /* NT */
BYTE * NetBiosSendBuf[MAXWS] ;
USHORT MaxCmds=0,MaxSess=0,MaxNames=0,MaxWs=0 ;

static USHORT
NetbeuiConfig(USHORT lana, PUSHORT sessions, PUSHORT commands,
               PUSHORT names)
{
    USHORT rc=NB_INADEQUATE_RESOURCES,blen,MaxEntries,i;
    PNETINFO1 temp=NULL;

    if(netbios_Enum)
    {
        if(!pNetinfo)
        {
            if(pNetinfo=(PNETINFO1)malloc(blen=sizeof(NETINFO1)*NB_ADAPTERS))
            {
                if(rc=(*netbios_Enum)(NULL,1,(PBYTE)pNetinfo,blen,&Netentries,
                                       &MaxEntries))
                {
                    free(pNetinfo);
                    pNetinfo=NULL;
                }
            } /* end if */
        }
        if(pNetinfo)
        {
            if(lana<=Netentries)
            {
                *sessions=pNetinfo[lana].nb1_max_sess;
                *commands=pNetinfo[lana].nb1_max_ncbs;
                *names=   pNetinfo[lana].nb1_max_names;
                rc=NB_COMMAND_SUCCESSFUL;
            } /* end if */
            else
            {
                rc=NB_INVALID_ADAPTER;
            } /* end else */
        } /* end if */
    } /* end if */
    return rc;
}

USHORT
NCBReset( BOOL Netbeui, PNCB Ncb, USHORT lana,USHORT  sessions,
                  USHORT  commands,USHORT names )
{
    int i,rc=NB_INADEQUATE_RESOURCES,nrc=0;
    if(!Netbeui)
    {
        memset( Ncb, 0, BNCBSIZE ) ;
        Ncb->reset.ncb_command  = NB_RESET_WAIT;
        Ncb->reset.ncb_lana_num = lana;
        Ncb->reset.req_sessions = sessions;
        Ncb->reset.req_commands = commands;
        Ncb->reset.req_names = names;

        nrc = (*netbios)(Ncb );

        rc=Ncb->reset.ncb_retcode;
    } /* end if */
    else
    {
        if(!pNetinfo)
        {
            rc=NetbeuiConfig(lana,&sessions,&commands,&names);
        } /* end if */
        if(pNetinfo)
        {
            if(lana<=Netentries)
            {
                if(pNetinfo[lana].nb1_max_sess>=sessions &&
                    pNetinfo[lana].nb1_max_ncbs>=commands &&
                    pNetinfo[lana].nb1_max_names>=names)
                  rc=(*netbios_Open)(pNetinfo[lana].nb1_net_name,NULL,1,
                                      &Netbeui_Handle[lana]);
            } /* end if */
            else
            {
                rc=NB_INVALID_ADAPTER;
            } /* end else */
        } /* end if */
    } /* end else */
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
            *sessions=Ncb->reset.act_sessions;
            *commands=Ncb->reset.act_commands;
            *names=   Ncb->reset.act_names;
        }
        if ( rc == NB_COMMAND_SUCCESSFUL ) {
            debug(F100,"ckonbi:NCBConfig: About to NCBClose","",0);
            NCBClose(Netbeui,Ncb,lana);
        }
    } /* end if */
    else
    {
        memset( Ncb, 0, BNCBSIZE ) ;
        if(!(rc=NetbeuiConfig(lana,sessions,commands,names)))
        {
            Ncb->basic_ncb.bncb.ncb_name[8]=*sessions;
            Ncb->basic_ncb.bncb.ncb_name[9]=*commands;
            Ncb->basic_ncb.bncb.ncb_name[10]=*names;
        }
    } /* end else */

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

    rc = netbios_avail( NetbeuiAPI=0 ) ;  /* ACSNETB.DLL - NTS/2 */

    if ( rc )
        rc = netbios_avail( NetbeuiAPI=1 ) ;  /* NETAPI.DLL - Lan Manager,
                                                         Lan Server 2.x,
                                                         Novell Requester */

    if ( !rc ) {
        char semname[19] ;

        if ( NetbeuiAPI ) {
            if (deblog)
            {
                printf( "NETAPI.DLL loaded OK\n") ;
                debug(F100,"NETAPI.DLL loaded OK","",0);
            }
        }
        else {
            if (deblog)
            {
                printf( "ACSNETB.DLL loaded OK\n") ;
                debug(F100,"ACSNETB.DLL loaded OK","",0);
            }
        }

        DosCreateEventSem( NULL, &hevNetBiosLSN, 0, 0 );

        sprintf(semname,"\\SEM\\%#6.6X.CK%s",pid,"W");
        if ( pWorkNCB = (PNCB) malloc( NCBSIZE ) )
            rc = Dos16CreateSem( DC_SEM_SHARED,
                                 (PHSEM16) &(pWorkNCB->basic_ncb.ncb_semaphore),
                                 semname ) ;

        sprintf(semname,"\\SEM\\%#6.6X.CK%s",pid,"L");
        if ( pListenNCB = (PNCB) malloc( NCBSIZE ) )
            rc = Dos16CreateSem( DC_SEM_SHARED,
                                 (PHSEM16) &(pListenNCB->basic_ncb.ncb_semaphore),
                                 semname ) ;

        sprintf(semname,"\\SEM\\%#6.6X.CK%s",pid,"R");
        if ( pRecvNCB = (PNCB) malloc( NCBSIZE ) )
            rc = Dos16CreateSem( DC_SEM_SHARED,
                                 (PHSEM16) &(pRecvNCB->basic_ncb.ncb_semaphore),
                                 semname ) ;

        for ( i=0 ; i < MAXWS ; i++ ) {
            sprintf(semname,"\\SEM\\%#6.6X.%s%2d",pid,"S",i);
            if ( pSendNCB[i] = (PNCB) malloc( NCBSIZE ) )
                rc = Dos16CreateSem( DC_SEM_SHARED,
                                     (PHSEM16) &(pSendNCB[i]->basic_ncb.ncb_semaphore),
                                     semname ) ;
        }

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

        if ( pWorkNCB )
        {
            Dos16CloseSem( pWorkNCB->basic_ncb.ncb_semaphore ) ;
            free( pWorkNCB ) ;
            pWorkNCB = 0 ;
         }

        if ( pListenNCB )
        {
            Dos16CloseSem( pListenNCB->basic_ncb.ncb_semaphore ) ;
            free( pListenNCB ) ;
            pListenNCB = 0 ;
        }

        if ( pRecvNCB )
        {
            Dos16CloseSem( pRecvNCB->basic_ncb.ncb_semaphore ) ;
            free( pRecvNCB ) ;
            pRecvNCB = 0 ;
        }

        for ( i=0 ; i<MAXWS ; i++ ) {
            if ( pSendNCB[i] )
            {
                Dos16CloseSem( pSendNCB[i]->basic_ncb.ncb_semaphore ) ;
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
    }
}

USHORT
loadapi(PSZ module, PSZ proc, PFN FAR *addr)
{
    int rc,rc1;
    HMODULE mh;
    rc1=0;                         /* load adapter processor */

    debug(F100,"ckonbi:loadapi DosQueryModuleHandle","",0);
    rc=DosQueryModuleHandle(module,&mh);
    debug(F111,"ckonbi:loadapi DosQueryModuleHandle","rc",rc);
    if(rc==0)
    {
        debug(F100,"ckonbi:loadapi DosQueryProcAddr","",0);
        /* loaded, check for this process */
        rc1=DosQueryProcAddr(mh,0,proc,addr);
        debug(F111,"ckonbi:loadapi DosQueryProcAddr","rc",rc1);
    }
    if(rc || rc1)   /* either not loaded, or not loaded for this process */
    {
        /* so load it */
        debug(F100,"ckonbi:loadapi DosLoadModule","",0);
        rc=DosLoadModule(NULL,0,module,&mh);
        debug(F111,"ckonbi:loadapi DosLoadModule","rc",rc);
        rc1=1;                       /* force getprocaddr */
    }
    if(rc==0)
    {                            /* loaded ok? */
        if(rc1) {
            /* no address to call, so get it */
            debug(F100,"ckonbi:loadapi DosQueryProcAddr","",0);
            rc=DosQueryProcAddr(mh,0,proc,addr); /* get entry */
            debug(F111,"ckonbi:loadapi DosQueryProcAddr","rc",rc);
        }
    }
    debug(F111,"ckonbi:loadapi","rc",rc);
    return rc;
}

USHORT
netbios_avail(BOOL Netbeui)
{
    int rc=0;
    if(!Netbeui)
    {
        if(!netbios)
            rc=loadapi("ACSNETB","NETBIOS",(PFN *)&netbios);
    } /* end if */
    else
    {
        if(!netbios_Submit)
        {
            rc|=loadapi("NETAPI","NETBIOSSUBMIT",(PFN *) &netbios_Submit);
            rc|=loadapi("NETAPI","NETBIOSCLOSE", (PFN *) &netbios_Close );
            rc|=loadapi("NETAPI","NETBIOSOPEN",  (PFN *) &netbios_Open  );
            rc|=loadapi("NETAPI","NETBIOSENUM",  (PFN *) &netbios_Enum  );
        } /* end if */
    } /* end else */
    debug(F111,"ckonbi:netbios_avail","rc",rc);
    return rc;
}

#pragma stack16(256)
VOID CDECL16
ncbpost(USHORT Junk, PNCB16 NcbPointer)
{
    APIRET rc = 0 ;
    NCB ncb = *NcbPointer ;
    rc = Dos16SemClear(NcbPointer->basic_ncb.ncb_semaphore);
}


USHORT
NCBClose( BOOL Netbeui, PNCB Ncb, USHORT lana)
{
    USHORT rc;
    if(!Netbeui)
    {
        memset( Ncb, 0, BNCBSIZE ) ;
        Ncb->reset.ncb_command  = NB_RESET_WAIT;
        Ncb->reset.ncb_lsn=255;
        Ncb->reset.ncb_lana_num = lana;
        rc=(*netbios)(Ncb);
    } /* end if */
    else
    {
        if(Netbeui_Handle[lana])
        {
            rc=(*netbios_Close)(Netbeui_Handle[lana],0);
        } /* end if */
        else
        {
            rc=NB_ENVIRONMENT_NOT_DEFINED;
        } /* end else */
    } /* end else */
    debug(F111,"ckonbi:NCBClose","rc",rc);
    return rc;
}
USHORT
NCBAddName( BOOL Netbeui, PNCB Ncb,USHORT lana, PBYTE name )
{
    memset( Ncb, 0, BNCBSIZE ) ;
    Ncb->basic_ncb.bncb.ncb_command  = NB_ADD_NAME_WAIT;
    Ncb->basic_ncb.bncb.ncb_lana_num = lana;
    strncpy( Ncb->basic_ncb.bncb.ncb_name, name, NETBIOS_NAME_LEN );
    if(Netbeui)
    {
        (*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
    } /* end if */
    else
    {
        (*netbios)(Ncb );
    } /* end else */

    return (Ncb->basic_ncb.bncb.ncb_retcode);
}

USHORT
NCBDeleteName(BOOL Netbeui, PNCB Ncb, USHORT lana, PBYTE name )
{
    memset( Ncb, 0, BNCBSIZE ) ;
    Ncb->basic_ncb.bncb.ncb_command  = NB_DELETE_NAME_WAIT;
    Ncb->basic_ncb.bncb.ncb_lana_num = lana;
    strncpy( Ncb->basic_ncb.bncb.ncb_name, name, NETBIOS_NAME_LEN );

    if(Netbeui)
    {
        (*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
    } /* end if */
    else
    {
        (*netbios)(Ncb );
    }

    return (Ncb->basic_ncb.bncb.ncb_retcode);
}
USHORT
NCBAddGroupName(BOOL Netbeui, PNCB Ncb,USHORT lana, PBYTE name )
{
    memset( Ncb, 0, BNCBSIZE ) ;
    Ncb->basic_ncb.bncb.ncb_command  = NB_ADD_GROUP_NAME_WAIT;
    Ncb->basic_ncb.bncb.ncb_lana_num = lana;
    strncpy( Ncb->basic_ncb.bncb.ncb_name, name, NETBIOS_NAME_LEN );

    if(Netbeui)
    {
        (*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
    } /* end if */
    else
    {
        (*netbios)(Ncb );
    }

    return (Ncb->basic_ncb.bncb.ncb_retcode);
}

USHORT
NCBCall(BOOL Netbeui, PNCB  Ncb, USHORT lana, PBYTE lclname,
         PBYTE rmtname,USHORT recv_timeout,USHORT send_timeout,BOOL wait)
{
    int rc;
    ;

    memset( Ncb, 0, BNCBSIZE ) ;
    Ncb->basic_ncb.bncb.ncb_command  = (wait)?NB_CALL_WAIT:NB_CALL;
    Ncb->basic_ncb.bncb.ncb_lana_num = lana;
    Ncb->basic_ncb.bncb.ncb_rto      = recv_timeout*2;  /* times 2 since in   */
    Ncb->basic_ncb.bncb.ncb_sto      = send_timeout*2;  /* steps of 500 msecs */

    if ( Netbeui )
    {
        Ncb->basic_ncb.bncb.off44.ncb_post_handle=
            ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
    } else {
        Ncb->basic_ncb.bncb.off44.ncb_post_address=
            (address)((!wait)?ncbpost:NULL);
    }

    strncpy( Ncb->basic_ncb.bncb.ncb_name, lclname, NETBIOS_NAME_LEN );
    strncpy( Ncb->basic_ncb.bncb.ncb_callname, rmtname, NETBIOS_NAME_LEN );
    if(!wait)
        rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore);

    if(Netbeui)
    {
        rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
    } /* end if */
    else
    {
        rc=(*netbios)(Ncb );
    }

    return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
}

USHORT
NCBListen(BOOL Netbeui, PNCB  Ncb, USHORT lana, PBYTE lclname,
           PBYTE rmtname,USHORT recv_timeout,USHORT send_timeout,BOOL wait)
{
    int rc;
    ;
    memset( Ncb, 0, BNCBSIZE ) ;
    Ncb->basic_ncb.bncb.ncb_command  = (wait)?NB_LISTEN_WAIT:NB_LISTEN;
    Ncb->basic_ncb.bncb.ncb_lana_num = lana;
    Ncb->basic_ncb.bncb.ncb_rto      = recv_timeout*2; /* times 2 since in   */
    Ncb->basic_ncb.bncb.ncb_sto      = send_timeout*2; /* steps of 500 msecs */

    if ( Netbeui )
    {
        Ncb->basic_ncb.bncb.off44.ncb_post_handle=
            ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
    } else {
        Ncb->basic_ncb.bncb.off44.ncb_post_address=
            (address)((!wait)?ncbpost:NULL);
    }

    strncpy( Ncb->basic_ncb.bncb.ncb_name, lclname, NETBIOS_NAME_LEN );
    strncpy( Ncb->basic_ncb.bncb.ncb_callname, rmtname, NETBIOS_NAME_LEN );
    if(!wait)
    {
        rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore);
    }

    if(Netbeui)
    {
        rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
    } /* end if */
    else
    {
        rc=(*netbios)(Ncb );
    }

    return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
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
 ;
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->basic_ncb.bncb.ncb_command  = (wait)?NB_SEND_WAIT:NB_SEND;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   Ncb->basic_ncb.bncb.ncb_lsn      = lsn;
   Ncb->basic_ncb.bncb.ncb_buffer_address = message;
   Ncb->basic_ncb.bncb.ncb_length   = length;

   if ( Netbeui )
   {
      Ncb->basic_ncb.bncb.off44.ncb_post_handle=
         ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
   } else {
      Ncb->basic_ncb.bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore);

   if(Netbeui)
     {
     rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
     } /* end if */
   else
     {
     rc=(*netbios)(Ncb );
     }

   return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
}

USHORT NCBSendDatagram(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PSZ rmtname, PBYTE message, word length,BOOL wait)
{
int rc;
 ;
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->basic_ncb.bncb.ncb_command  =
                     (wait)?NB_SEND_DATAGRAM_WAIT:NB_SEND_DATAGRAM;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   Ncb->basic_ncb.bncb.ncb_num      = lsn;
   Ncb->basic_ncb.bncb.ncb_buffer_address = message;
   Ncb->basic_ncb.bncb.ncb_length   = length;

   if ( Netbeui )
   {
      Ncb->basic_ncb.bncb.off44.ncb_post_handle=
         ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
   } else {
      Ncb->basic_ncb.bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

    strncpy( Ncb->basic_ncb.bncb.ncb_callname, rmtname, NETBIOS_NAME_LEN );
    if(!wait)
      rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore );

    if(Netbeui)
      {
      rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
      } /* end if */
    else
      {
      rc=(*netbios)(Ncb );
      }

    return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
}
USHORT  NCBSendBroadcast(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE message, word length,BOOL wait)
{
int rc;
 ;
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->basic_ncb.bncb.ncb_command  =
         (wait)?NB_SEND_BROADCAST_DATAGRAM_WAIT:NB_SEND_BROADCAST_DATAGRAM;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   Ncb->basic_ncb.bncb.ncb_num      = lsn;
   Ncb->basic_ncb.bncb.ncb_buffer_address = message;
   Ncb->basic_ncb.bncb.ncb_length   = length;

   if ( Netbeui )
   {
      Ncb->basic_ncb.bncb.off44.ncb_post_handle=
         ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
   } else {
      Ncb->basic_ncb.bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore );

   if(Netbeui)
     {
     rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
     } /* end if */
   else
     {
     rc=(*netbios)(Ncb );
     }

   return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
}

USHORT  NCBSendNoAck(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE message, word length,BOOL wait)
{
int rc;
 ;
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->basic_ncb.bncb.ncb_command  =
                        (wait)?NB_SEND_NO_ACK_WAIT:NB_SEND_NO_ACK;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   Ncb->basic_ncb.bncb.ncb_lsn      = lsn;
   Ncb->basic_ncb.bncb.ncb_buffer_address = message;
   Ncb->basic_ncb.bncb.ncb_length   = length;

   if ( Netbeui )
   {
      Ncb->basic_ncb.bncb.off44.ncb_post_handle=
         ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
   } else {
      Ncb->basic_ncb.bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

  if(!wait)
    rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore);

  if(Netbeui)
    {
    rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
    } /* end if */
  else
    {
    rc=(*netbios)(Ncb );
    }

  return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
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

USHORT  NCBChainSend(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE  message, word length, PBYTE Buffer2, word Length2,BOOL wait)
{
int rc;
PBuf2 b2;
 ;
   memset( Ncb, 0, BNCBSIZE ) ;
   b2=(PBuf2)&Ncb->basic_ncb.bncb.ncb_callname;
   Ncb->basic_ncb.bncb.ncb_command  =
                             (wait)?NB_CHAIN_SEND_WAIT:NB_CHAIN_SEND;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   Ncb->basic_ncb.bncb.ncb_lsn      = lsn;
   Ncb->basic_ncb.bncb.ncb_buffer_address = message;
   Ncb->basic_ncb.bncb.ncb_length   = length;

   if ( Netbeui )
   {
      Ncb->basic_ncb.bncb.off44.ncb_post_handle=
         ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
   } else {
      Ncb->basic_ncb.bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   b2->Length=Length2;
   b2->Buffer=Buffer2;
   if(!wait)
     rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore);

   if(Netbeui)
     {
     rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
     } /* end if */
   else
     {
     rc=(*netbios)(Ncb );
     }

   return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
}
USHORT  NCBChainSendNoAck(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE message, word length, PBYTE Buffer2, word Length2,BOOL wait)
{
int rc;
PBuf2 b2;
 ;
   memset( Ncb, 0, BNCBSIZE ) ;
   b2=(PBuf2)&Ncb->basic_ncb.bncb.ncb_callname;
   Ncb->basic_ncb.bncb.ncb_command  =
                  (wait)?NB_CHAIN_SEND_NO_ACK_WAIT:NB_CHAIN_SEND_NO_ACK;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   Ncb->basic_ncb.bncb.ncb_lsn      = lsn;
   Ncb->basic_ncb.bncb.ncb_buffer_address = message;
   Ncb->basic_ncb.bncb.ncb_length   = length;

   if ( Netbeui )
   {
      Ncb->basic_ncb.bncb.off44.ncb_post_handle=
         ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
   } else {
      Ncb->basic_ncb.bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   b2->Length=Length2;
   b2->Buffer=Buffer2;
   if(!wait)
     rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore);

   if(Netbeui)
     {
     rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
     } /* end if */
   else
     {
     rc=(*netbios)(Ncb );
     }

   return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
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

USHORT NCBReceive(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE buffer, word length, BOOL wait )
{
int rc;
 ;
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->basic_ncb.bncb.ncb_command  = (wait)?NB_RECEIVE_WAIT:NB_RECEIVE;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   Ncb->basic_ncb.bncb.ncb_lsn      = lsn;
   Ncb->basic_ncb.bncb.ncb_buffer_address = buffer;
   Ncb->basic_ncb.bncb.ncb_length   = length;

   if ( Netbeui )
   {
      Ncb->basic_ncb.bncb.off44.ncb_post_handle=
         ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
   } else {
      Ncb->basic_ncb.bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore);

   if(Netbeui)
     {
     rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
     } /* end if */
   else
     {
     rc=(*netbios)(Ncb );
     }

   return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
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

USHORT NCBReceiveAny(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE buffer, word length,BOOL wait )
{
int rc;
 ;
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->basic_ncb.bncb.ncb_command  =
                           (wait)?NB_RECEIVE_ANY_WAIT:NB_RECEIVE_ANY;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   Ncb->basic_ncb.bncb.ncb_num      = lsn;
   Ncb->basic_ncb.bncb.ncb_buffer_address = buffer;
   Ncb->basic_ncb.bncb.ncb_length   = length;

   if ( Netbeui )
   {
      Ncb->basic_ncb.bncb.off44.ncb_post_handle=
         ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
   } else {
      Ncb->basic_ncb.bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore );

   if(Netbeui)
     {
     rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
     } /* end if */
   else
     {
     rc=(*netbios)(Ncb );
     }

   return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
}

USHORT NCBReceiveDatagram(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn,
   PBYTE buffer, word length,BOOL wait )
{
int rc;
 ;
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->basic_ncb.bncb.ncb_command  =
               (wait)?NB_RECEIVE_DATAGRAM_WAIT:NB_RECEIVE_DATAGRAM;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   Ncb->basic_ncb.bncb.ncb_num      = lsn;
   Ncb->basic_ncb.bncb.ncb_buffer_address = buffer;
   Ncb->basic_ncb.bncb.ncb_length   = length;

   if ( Netbeui )
   {
      Ncb->basic_ncb.bncb.off44.ncb_post_handle=
         ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
   } else {
      Ncb->basic_ncb.bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore );

   if(Netbeui)
     {
     rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
     } /* end if */
   else
     {
     rc=(*netbios)(Ncb );
     }

   return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
}

USHORT NCBReceiveBroadcast(BOOL Netbeui, PNCB  Ncb,USHORT  lana, USHORT lsn,
   PBYTE buffer, word length,BOOL wait )
{
int rc;
 ;
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->basic_ncb.bncb.ncb_command  =
       (wait)?NB_RECEIVE_BROADCAST_DATAGRAM_W:NB_RECEIVE_BROADCAST_DATAGRAM;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   Ncb->basic_ncb.bncb.ncb_num      = lsn;
   Ncb->basic_ncb.bncb.ncb_buffer_address = buffer;
   Ncb->basic_ncb.bncb.ncb_length   = length;

   if ( Netbeui )
   {
      Ncb->basic_ncb.bncb.off44.ncb_post_handle=
         ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
   } else {
      Ncb->basic_ncb.bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore );

   if(Netbeui)
     {
     rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
     } /* end if */
   else
     {
     rc=(*netbios)(Ncb );
     }

   return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
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

USHORT  NCBHangup(BOOL Netbeui, PNCB  Ncb, USHORT lana, USHORT lsn )
{
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->basic_ncb.bncb.ncb_command  = NB_HANG_UP_WAIT;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   Ncb->basic_ncb.bncb.ncb_lsn      = lsn;

   if(Netbeui)
     {
     (*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
     } /* end if */
   else
     {
     (*netbios)(Ncb );
     }

   return (Ncb->basic_ncb.bncb.ncb_retcode);
}

/**********************************************************************/
/*
** NCBCancel    Cancels a previously issued NCB if possible.
**
**              Accepts the adapter number and a ptr to the NCB to cancel.
**
**              Returns the NCB return code.
*/

USHORT  NCBCancel(BOOL Netbeui, PNCB  Ncb, USHORT lana, PNCB NcbToCancel)
{
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->basic_ncb.bncb.ncb_command  = NB_CANCEL_WAIT;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   Ncb->basic_ncb.bncb.ncb_buffer_address = (address)NcbToCancel;

   if(Netbeui)
     {
     (*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
     } /* end if */
   else
     {
     (*netbios)(Ncb );
     }

   return (Ncb->basic_ncb.bncb.ncb_retcode);
}

/**********************************************************************/
/*
** NCBStatus
**
**              Accepts the adapter number and a ptr to the NCB to cancel.
**
**              Returns the NCB return code.
*/

USHORT NCBStatus(BOOL Netbeui, PNCB Ncb, USHORT lana, PBYTE callname,
   PSTATUSINFO pInfo, word length, BOOL wait )
{
int rc;
 ;
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->basic_ncb.bncb.ncb_command  = (wait)?NB_STATUS_WAIT:NB_STATUS;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   strncpy( Ncb->basic_ncb.bncb.ncb_callname, callname, NETBIOS_NAME_LEN );
   Ncb->basic_ncb.bncb.ncb_buffer_address = (PBYTE) pInfo;
   Ncb->basic_ncb.bncb.ncb_length   = length;

   if ( Netbeui )
   {
      Ncb->basic_ncb.bncb.off44.ncb_post_handle=
         ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
   } else {
      Ncb->basic_ncb.bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore);

   if(Netbeui)
     {
     rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
     } /* end if */
   else
     {
     rc=(*netbios)(Ncb );
     }

   return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
}

USHORT NCBSessionStatus(BOOL Netbeui, PNCB Ncb, USHORT lana, PBYTE lclname,
   PSESSIONINFO pInfo, word length, BOOL wait )
{
int rc;
 ;
   memset( Ncb, 0, BNCBSIZE ) ;
   Ncb->basic_ncb.bncb.ncb_command  =
                     (wait)?NB_SESSION_STATUS_WAIT:NB_SESSION_STATUS;
   Ncb->basic_ncb.bncb.ncb_lana_num = lana;
   strncpy( Ncb->basic_ncb.bncb.ncb_name, lclname, NETBIOS_NAME_LEN );
   Ncb->basic_ncb.bncb.ncb_buffer_address = (PBYTE) pInfo;
   Ncb->basic_ncb.bncb.ncb_length   = length;

   if ( Netbeui )
   {
      Ncb->basic_ncb.bncb.off44.ncb_post_handle=
         ((!wait)?Ncb->basic_ncb.ncb_semaphore:0L);
   } else {
      Ncb->basic_ncb.bncb.off44.ncb_post_address=
         (address)((!wait)?ncbpost:NULL);
   }

   if(!wait)
     rc= Dos16SemSet(Ncb->basic_ncb.ncb_semaphore);

   if(Netbeui)
     {
     rc=(*netbios_Submit)(Netbeui_Handle[lana],0,Ncb);
     } /* end if */
   else
     {
     rc=(*netbios)(Ncb );
     }

   return (wait)?Ncb->basic_ncb.bncb.ncb_retcode:rc;
}

VOID
NCBCancelOutstanding( VOID )
{
    if (netbiosAvail) {
       int i;
       NCB CancelNCB;
       /* Cancel all outstanding Netbios Send or Work requests */

       for ( i = 0 ; i < MAXWS ; i++) {
           if (pSendNCB[i]->basic_ncb.bncb.ncb_retcode ==
               NB_COMMAND_IN_PROCESS ) {
               NCBCancel( NetbeuiAPI,
                         &CancelNCB,
                         NetBiosAdapter,
                         pSendNCB[i] ) ;
               Dos16SemWait( pSendNCB[i]->basic_ncb.ncb_semaphore,
                            SEM_INDEFINITE_WAIT);
           }
       }
       if ( pWorkNCB->basic_ncb.bncb.ncb_retcode == NB_COMMAND_IN_PROCESS ) {
           NCBCancel( NetbeuiAPI, &CancelNCB, NetBiosAdapter, pWorkNCB ) ;
           Dos16SemWait( pWorkNCB->basic_ncb.ncb_semaphore,
                        SEM_INDEFINITE_WAIT ) ;
       }
   }
}

#endif /* CK_NETBIOS */
