/*****************************************************************************/
/*             Copyright (c) 1994 by Jyrki Salmi <jytasa@jyu.fi>             */
/*        You may modify, recompile and distribute this file freely.         */
/*****************************************************************************/

/*
   Routines that load P.DLL and get the address of p_tranfer() entry
   function.
*/

#include <stdio.h>

#ifdef OS2
#ifdef NT
#include <windows.h>
#else
#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSNLS
#define INCL_DOSASYNCTIMER
#define INCL_DOSDATETIME
#define INCL_DOSERRORS
#include <os2.h>
#undef COMMENT
#endif
#endif /* OS2 */

#include "ckcdeb.h"
#ifndef NOXFER
#ifdef XYZ_INTERNAL
#include "ckcker.h"
#include "ckuusr.h"
#include "ckcnet.h"
#include "ckocon.h"

#include "p_type.h"
#include "p.h"
#include "ckop.h"
#include "p_global.h"
#include "p_callbk.h"
#include "p_common.h"

extern int rpackets, spackets, spktl, rpktl, what ;

#ifdef XYZ_DLL
#ifdef OS2
static HMODULE dll_handle;
U32 (* _System p_transfer)(P_CFG *) = NULL;
#endif /* OS2 */

#define PINBUFSIZE 8192
#define POUTBUFSIZE 4096

int
load_p_dll(void) {
    int rc=0;
    CHAR *exe_path;
    CHAR path[256];

#ifdef NT
    dll_handle = LoadLibrary( "P95.DLL" ) ;
    if ( !dll_handle )
    {
        rc = GetLastError() ;
        debug(F101,"load_p_dll - Unable to load module: rc","",rc);
        return rc;
    }
    (FARPROC) p_transfer = GetProcAddress( dll_handle, "p_transfer" ) ;
    if ( !p_transfer )
    {
        rc = GetLastError() ;
        debug(F101,"load_p_dll - Unable to find p_transfer()","",rc);
        return rc;
    }
#else
    exe_path = GetLoadPath();
    sprintf(path, "%.*sP2.DLL", (int)get_dir_len(exe_path), exe_path);
    rc = DosLoadModule(NULL, 0L, path, &dll_handle);
    if (rc) {
        /* P.DLL was not found in directory specified with LIBPATH, let's look */
        /* up for it from the directory where P.EXE was ran from. */
        rc = DosLoadModule(NULL, 0L, "P2", &dll_handle);
        if (rc)
            debug(F101,"load_p_dll - Unable to load module: rc","",rc);
    }
    /* Query the address of p_transfer() entry function */
    rc = DosQueryProcAddr(dll_handle,
                           0,
                           "p_transfer",
                           (PFN *)&p_transfer);
    if (rc)
        debug(F101,"load_p_dll - Unable to find p_transfer()","",rc);
#endif
    return rc ;
}

int
unload_p_dll(void) {
  int rc=0;
#ifdef NT
   if ( !FreeLibrary( dll_handle ) )
      rc = GetLastError() ;
#else
   rc = DosFreeModule(dll_handle);
#endif
  if (rc)
    debug(F101,"unload_p_dll - Unable to unload module - you must be kidding if you see this one! :-)","",rc);
  else
     p_transfer = NULL ;
   return rc ;
}
#endif /* XYZ_DLL */


U32 _System
pushback_func( U8 * buf, U32 len )
{
    return le_puts( buf, len );
}

U32 _System
in_func( U8 * buf, U32 len, U32 * bytes_received )
{
    extern int network, carrier;
    int rc, avail, read, i ;

    if (!network) {                     /* if not a network */
        if (carrier != CAR_OFF &&       /* && carrier-watch enabled */
             (ttgmdm() & BM_DCD)==0) { /* check for carrier detect */
            debug(F100,"P in_func carrier dropped","",0);
            *bytes_received = 0;
            *buf='\0';
            return(-1);         /* no? Carrier dropped */
        }
    }
    avail = ttchk() ;
    debug(F111,"P in_func","ttchk",avail);

    if ( avail <= 0 )
        return(avail);

    if ( !buf || !bytes_received )
        return -1 ;

    *bytes_received = 0 ;
    *buf = '\0' ;

    if ( !len )
        return 0 ;

    read = len > avail ? avail : len ;
    rc = ttxin( read, buf ) ;

    if ( rc > 0 ) {
        *bytes_received = rc ;
        hexdump("P in_func",buf,rc);

        if ( what == W_RECV ) {
            rpktl = rc ;
            rpackets++;
        }

        return 0;
    } else if ( rc < -1 )
        return(rc);
    return(ERROR_NO_DATA); /* Either no data was received or timeout */
}

U32 _System
out_func( U8 * buf, U32 len, U32 * bytes_written )
{
    int rc = 0 ;
   if ( !buf || !bytes_written )
      return -1 ;

   *bytes_written = 0 ;

   if ( !len )
      return 0 ;

    if ( what == W_SEND ) {
        spktl = len ;
        spackets++ ;
    }

    hexdump("P out_func",buf,len);
    rc = ttxout( buf, len ) ;
    if ( rc >= 0 ) {
        *bytes_written = rc ;
        return 0;
    }
    else if ( rc == -1 )
        return 0;       /* No data written but connection not dropped */
    else return 1 ;
}

#ifdef OS2ONLY
USHORT DosDevIOCtl32(PVOID pData, USHORT cbData, PVOID pParms, USHORT cbParms,
                     USHORT usFunction, USHORT usCategory, HFILE hDevice);
#endif /* OS2ONLY */

U32 _System
break_func( U8 on )
{
   extern int ttyfd, ttmdm ;
#ifndef NT
    MODEMSTATUS ms;
    UINT data, i;

#endif /* NT */

    debug(F101,"P break_func","",on);

#ifdef NETCONN
    if (ttmdm < 0)
    {
       if (on)
          return os2_netbreak();
       else return (0);
    }
#endif /* NETCONN */
#ifdef NT
    if (on) SetCommBreak( (HANDLE) ttyfd ) ;
    else ClearCommBreak( (HANDLE) ttyfd ) ;
#else /* NT */
         ms.fbModemOn = RTS_ON;
     ms.fbModemOff = 255;
     DosDevIOCtl32(&data,sizeof(data),&ms,sizeof(ms),
                ASYNC_SETMODEMCTRL,IOCTL_ASYNC,ttyfd);

   if ( on )
    DosDevIOCtl32(&i,sizeof(i),NULL,0,
                ASYNC_SETBREAKON,IOCTL_ASYNC,ttyfd);    /* Break on */
   else
    DosDevIOCtl32(&i,sizeof(i),NULL,0,
                ASYNC_SETBREAKOFF,IOCTL_ASYNC,ttyfd);   /* Break off */
#endif /* NT */
    return 0;
}

U32 _System
available_func( U32 * available )
{
    int rc = ttchk() ;

    if ( rc < 0 ) {
        *available = 0 ;
        return(1);
    }
    else {
        *available = rc ;
        return(0);
    }
}


int
pxyz(int sstate) {
    extern struct ck_p ptab[] ;
    extern int ttyfd, protocol, mdmtyp, fncact, binary, moving, sendmode,
      prefixing, carrier, local, fdispla, nfils, parity, ttprty;
    extern int network;
#ifndef NOLOCAL
    extern term_io;
    int term_io_sav = term_io;
#endif /* NOLOCAL */
#ifdef TCPSOCKET
    extern int u_binary, me_binary, ttnproto, tn_b_meu, tn_b_ume ;
#endif /* TCPSOCKET */
    extern int retrans, crunched ;
    extern int timeouts, tsecs ;
    extern char * cmarg, * cmarg2, ** cmlist ;
    extern long ffc, filcnt ;
    extern short ctlp[] ;
#ifdef NT
    extern int owwait, maxow ;  /* overlapped writes wait for return ? */
#endif /* NT */
    extern int nzxpand(char *,int) ;
    extern int znext( char *) ;
#ifdef CK_TMPDIR
    extern char * dldir ;
    extern int f_tmpdir;
    extern char savdir[] ;
    extern char *fncnam[] ;
#endif /* CK_TMPDIR */
#ifndef NOMSEND
    extern struct filelist * filehead, * filenext;
    extern int addlist;
#endif /* NOMSEND */
    char filename[260] ;
    int savbin = binary ;
    char *tp;
    APIRET rc = 0;
    int i;
#ifdef GFTIMER
    extern float fptsecs;
#endif /* GFTIMER */

#ifdef PIPESEND
#undef PIPESEND
#endif /* PIPESEND */
#ifdef PIPESEND
    extern int pipesend;
#endif /* PIPESEND */

    savfnc = fncact ;

#ifdef XYZ_DLL
    if ( !p_transfer )
      if ( load_p_dll() )
        return -1 ;
#endif /* XYZ_DLL */

    memset(&p_cfg, '\0', sizeof(p_cfg));
    p_cfg.inbuf_size = PINBUFSIZE;
    p_cfg.outbuf_size = POUTBUFSIZE;

    /* Initialize Receiver state flags */
    checking_method = 0;
    receiver_flags = 0;
    receiver_window_size = -1;

    if (sstate != 's' && sstate != 'v') {
        printf("?Invalid start state for %s\n",
               ptab[protocol].p_name);
        return -1 ;
    }

    p_cfg.version = P_INTERFACE_VERSION;
    p_cfg.serial_num = 0 ;              /* 0 means no serial number */
    p_cfg.attn_seq = NULL;              /* By default, we don't */

    switch ( protocol ) {
    case PROTO_X:
        debug(F111,"pxyz()","Xmodem",protocol);
        p_cfg.protocol_type = PROTOCOL_X ;
        if (ptab[protocol].spktlen >= 1000)
            p_cfg.attr |= CFG_1K_BLOCKS ;
        break;
    case PROTO_XC:
        debug(F111,"pxyz()","Xmodem C",protocol);
        p_cfg.protocol_type = PROTOCOL_X ;
        p_cfg.attr |= CFG_ALTERNATIVE_CHECKING;
        if (ptab[protocol].spktlen >= 1000)
            p_cfg.attr |= CFG_1K_BLOCKS;
        break;
    case PROTO_Y:
        debug(F111,"pxyz()","Ymodem",protocol);
        p_cfg.protocol_type = PROTOCOL_Y ;
        if (ptab[protocol].spktlen >= 1000)
            p_cfg.attr |= CFG_1K_BLOCKS ;
        break;
    case PROTO_G:
        debug(F111,"pxyz()","Ymodem G",protocol);
        p_cfg.protocol_type = PROTOCOL_G ;
        if (ptab[protocol].spktlen >= 1000)
            p_cfg.attr |= CFG_1K_BLOCKS ;
        break;
    case PROTO_Z:
        debug(F111,"pxyz()","Zmodem",protocol);
        p_cfg.protocol_type = PROTOCOL_Z ;
#ifdef COMMENT
        /* this is handled by ptab[protocol].h_init now */
        p_cfg.attr |= CFG_SEND_RZ_CR ;
#endif /* COMMENT */
        if (ptab[protocol].winsize > -1)
          p_cfg.blk_size = ptab[protocol].winsize ;
        break;
    default:
        debug(F101,"P - unsupported protocol","",protocol);
        return -1;
    }
    p_cfg.dev_type = DEV_TYPE_EXE_IO ;
#ifdef TCPSOCKET
    /* Just for debug purposes */
    if ( deblog && mdmtyp < 0 && network && ttnproto == NP_TELNET) {
        debug(F110,"pxyz","CFG_DEV_TELNET",0);
        if ( TELOPT_U(TELOPT_BINARY) || (TELOPT_ME(TELOPT_BINARY) && tn_b_meu) ) {
            debug(F110,"pxyz","CFG_DEV_TELNET_U_BINARY",0);
        }
        if ( TELOPT_ME(TELOPT_BINARY) || (TELOPT_U(TELOPT_BINARY) && tn_b_ume) ) {
            debug(F110,"pxyz","CFG_DEV_TELNET_ME_BINARY",0);
        }
    }
#endif /* TCPSOCKET */
    if ( moving )
      p_cfg.attr |= CFG_FILE_MOVE ;

    if (local && fdispla ) {
        opt_mileage = 1 ;
        opt_frameends = 0 ;
        opt_headers = 0 ;
    }

    switch (fncact) {                   /* Filename collision */
      case XYFX_A:                      /* Append */
        opt_management = Z_MANAGEMENT_APPEND ;
        break;
      case XYFX_B:                      /* Backup */
        opt_management = Z_MANAGEMENT_BACKUP ;
        break;
      case XYFX_D:                      /* Discard */
        opt_management = Z_MANAGEMENT_PROTECT ;
        break;
      case XYFX_R:                      /* Rename */
        opt_management = Z_MANAGEMENT_RENAME ;
        break;
      case XYFX_X:                      /* Replace */
        opt_management = Z_MANAGEMENT_REPLACE ;
        break;
      case XYFX_U:                      /* Update */
        opt_management = Z_MANAGEMENT_NEWER ;
        break;
    }
      opt_text = !binary ; /* Text mode */

    if (!cmarg2) cmarg2 = "";

    if (sstate == 'v') {                /* Receiving */
#ifdef CK_TMPDIR
        if (dldir && !f_tmpdir) {       /* If they have a download directory */
            char * s = NULL ;
            debug(F110,"pxyz() download directory",dldir,0);
            if (s = zgtdir()) {         /* Get current directory */
                if (zchdir(dldir)) {    /* Change to download directory */
                    strncpy(savdir,s,TMPDIRLEN);
                    f_tmpdir = 1;       /* Remember that we did this */
                }
            }
        }
#endif /* CK_TMPDIR */

        if (ptab[protocol].fnrp == 0) {
            opt_paths = 1 ;
            opt_create = 1 ;
        }
        what = W_RECV ;
        p_cfg.transfer_direction = DIR_RECV; /* Must supply output filename */
        if (p_cfg.protocol_type == PROTOCOL_X) { /* for XMODEM receive */
            if (!*cmarg2)
              cmarg2 = "XMODEM.XXX";
        }
        if (*cmarg2) {
            tl_add( &tl, cmarg2, 0, NULL, !binary ) ;
            cmarg2 = "";
        }
    } else {                            /* Sending */
        int i, j, x, y;
        if (ptab[protocol].fnsp == 0)
          opt_paths = 1 ;
        if (binary && sendmode == SM_RESEND && protocol == PROTO_Z ) {
            opt_resume = 1 ;
        }
        for (i=0;i<256;i++) {
            if (ctlp[i]) {
                p_cfg.attr |= CFG_ESC_TABLE ;
                p_cfg.control_prefix_table = (unsigned short *)ctlp ;
                break;
            }
        }
        debug(F111,"ckop()","parity",parity);
        debug(F111,"ckop()","ttprty",ttprty);
        if ( parity ) {
            p_cfg.attr |= CFG_ESC_8TH ;
        }
        p_cfg.transfer_direction = DIR_SEND ;
        what = W_SEND ;

        if ( nfils < 0 ) {
			if (!cmarg[0])
				return -1;

#ifdef PIPESEND
            if ( pipesend ) {
                tl_add( &tl, cmarg, 0,
                        (cmarg2 && *cmarg2) ? cmarg2 : NULL,
                        !binary ) ;
            }
            else
#endif /* PIPESEND */
            for ( i=0, y=x=nzxpand(cmarg,1); i < x ; i++ ) {
                y = znext(filename);
                if ( !isdir(filename) )
                    tl_add( &tl, filename, zchki(filename),
                            (x==1 && cmarg2 && *cmarg2) ? cmarg2 : NULL,
                            !binary ) ;
            }
        }
        else {
#ifndef NOMSEND
           if ( addlist ) {
              while( filenext )
                 if ( filenext->fl_name )
                 {
                    for ( i=0, y=x=nzxpand(filenext->fl_name,1);
                          i < x ; i++ ) {
                        y = znext(filename);
                        if ( !isdir(filename) )
                            tl_add( &tl, filename, zchki(filename),
                               (x == 1 && filenext->fl_alias) ?
                               filenext->fl_alias : NULL,
                               !filenext->fl_mode ) ;
                    }
                    filenext = filenext->fl_next ;
                 }
                 else
                 {
                    printf("?Internal error expanding ADD list\n");
                    return(-1);
                 }
           }
           else
#endif /* NOMSEND */
              for ( j=0; j<nfils ; j++ )
              {
                  for ( i=0,y=x=nzxpand(cmlist[j],1); i < x ; i++ ) {
                      y = znext(filename);
                      tl_add( &tl, filename, zchki(filename),
                              (x==1 && cmarg2 && *cmarg2) ? cmarg2 : NULL,
                              !binary ) ;
                  }
              }
        }
        files_left = tl->cnt ;
        bytes_left = tl->size ;
    }
    p_cfg.status_func = status_func;
    p_cfg.r_open_func = r_open_func;
    p_cfg.s_open_func = s_open_func;
    p_cfg.close_func = close_func;
    p_cfg.seek_func = seek_func;
    p_cfg.read_func = read_func;
    p_cfg.write_func = write_func;
    p_cfg.exe_out_func = out_func;
    p_cfg.exe_in_func = in_func;
    p_cfg.exe_break_func = break_func;
    p_cfg.exe_available_func = available_func;
    p_cfg.exe_pushback_func = pushback_func;

    /* Transaction Log Begin */
    ztime(&tp);
    tlog(F110,"Transaction begins",tp,0L); /* Make transaction log entry */
    tlog(F110,"Global file mode:", binary ? "binary" : "text", 0L);
    if ( p_cfg.transfer_direction == DIR_RECV )
        tlog(F110,"Collision action:", fncnam[fncact],0);
    tlog(F100,"","",0);

    ckscreen(SCR_PT,'S',0L,"");
    sprintf(filename, "%s %s is being initiated",
           ptab[protocol].p_name,
           p_cfg.transfer_direction == DIR_RECV ?
           "receiving" : "sending" );
    ckscreen(SCR_ST,ST_MSG,0L,filename);

    install_interrupt_handler( 1 ) ;
    resetc() ;                          /* Reset per transaction counters */
    rtimer() ;                          /* Reset timers for file transfer */
#ifdef GFTIMER
    rftimer();
#endif /* GFTIMER */
    spktl = 0 ;
    rpktl = 0 ;
    tsecs = -1 ;
#ifndef NOLOCAL
    term_io = 0;                        /* Disable Emulator I/O */
#endif /* NOLOCAL */
    filcnt = 0;

    if ( rc = p_transfer( &p_cfg ) ) {
       bleep(BP_FAIL) ;
       if ( we_aborted )
          ckscreen(SCR_EM,0,0L,"Transfer cancelled");
       else
           ckscreen(SCR_EM,0,0L,"Transfer failed");

    }
    else
       bleep(BP_NOTE);

    install_interrupt_handler( 0 ) ;
    tl_free(&tl) ;

#ifdef CK_TMPDIR
    /* If we were cd'd temporarily to another device or directory ... */
    if (f_tmpdir) {
        int x;
        x = zchdir((char *) savdir);    /* ... restore previous directory */
        f_tmpdir = 0;                   /* and remember we did it. */
        debug(F111,"ckcpro.w B tmpdir restoring",savdir,x);
    }
#endif /* CK_TMPDIR */

    tsecs = gtimer();
#ifdef GFTIMER
    fptsecs = gftimer();
#endif /* GFTIMER */
    ckscreen(SCR_TC,0,0L,"");             /* Close Display */

    tstats() ;                          /* Generate output for Transaction Log */

    binary = savbin ;                   /* Restore original settings */
    fncact = savfnc ;
#ifndef NOLOCAL
    term_io = term_io_sav;
#endif /* NOLOCAL */
#ifdef PIPESEND
    pipesend = 0;                       /* Reset it for the next time */
#endif /* PIPESEND */

    return rc ;
}
#endif /* XYZ_INTERNAL */
#endif /* NOXFER */