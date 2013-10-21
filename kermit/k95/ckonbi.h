/*  C K O N B I  --  NetBios support  */

/*
  COPYRIGHT NOTICE:

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

/*
   developed by Jeffrey E Altman <jaltman@secure-endpoints.com>

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

#ifdef CK_NETBIOS
#ifndef CKCMAI
#ifndef CKONBI_H
#define CKONBI_H

/*------------------------------------------------------------------*/
/*             TYPE DEFINITIONS FOR DATA DECLARATIONS               */
/*------------------------------------------------------------------*/

  typedef unsigned char byte;
  typedef unsigned short word;
  typedef unsigned long dword;
#ifdef OS2ONLY
  #pragma seg16(address)
  typedef unsigned char * _Seg16 address;
  #pragma seg16(HSEM16)
  typedef void * _Seg16 HSEM16;
  typedef HSEM16 * PHSEM16;
#else /* OS2ONLY */
  typedef unsigned char * address;
  #define HSEM16 HANDLE
  #define PHSEM16 PHANDLE
#endif /* OS2ONLY */

/*----------------------------------------------------------------*/
/*                       NETBIOS COMMAND CODES                    */
/*----------------------------------------------------------------*/

     #define   NB_CALL_WAIT                                 0x0010
     #define   NB_LISTEN_WAIT                               0x0011
     #define   NB_HANG_UP_WAIT                              0x0012
     #define   NB_SEND_WAIT                                 0x0014
     #define   NB_RECEIVE_WAIT                              0x0015
     #define   NB_RECEIVE_ANY_WAIT                          0x0016
     #define   NB_CHAIN_SEND_WAIT                           0x0017
     #define   NB_SEND_DATAGRAM_WAIT                        0x0020
     #define   NB_RECEIVE_DATAGRAM_WAIT                     0x0021
     #define   NB_SEND_BROADCAST_DATAGRAM_WAIT              0x0022
     #define   NB_RECEIVE_BROADCAST_DATAGRAM_W              0x0023
     #define   NB_ADD_NAME_WAIT                             0x0030
     #define   NB_DELETE_NAME_WAIT                          0x0031
     #define   NB_RESET_WAIT                                0x0032
     #define   NB_STATUS_WAIT                               0x0033
     #define   NB_SESSION_STATUS_WAIT                       0x0034
     #define   NB_CANCEL_WAIT                               0x0035
     #define   NB_ADD_GROUP_NAME_WAIT                       0x0036
     #define   NB_ENUMERATE_LANA_NUM_WAIT                   0x0037
     #define   NB_UNLINK_WAIT                               0x0070
     #define   NB_SEND_NO_ACK_WAIT                          0x0071
     #define   NB_CHAIN_SEND_NO_ACK_WAIT                    0x0072
     #define   NB_LAN_STATUS_ALERT_WAIT                     0x0073
     #define   NB_ACTION_WAIT                               0x0077
     #define   NB_FIND_NAME_WAIT                            0x0078
     #define   NB_TRACE_WAIT                                0x0079
     #define   NB_CALL                                      0x0090
     #define   NB_LISTEN                                    0x0091
     #define   NB_HANG_UP                                   0x0092
     #define   NB_SEND                                      0x0094
     #define   NB_RECEIVE                                   0x0095
     #define   NB_RECEIVE_ANY                               0x0096
     #define   NB_CHAIN_SEND                                0x0097
     #define   NB_SEND_DATAGRAM                             0x00A0
     #define   NB_RECEIVE_DATAGRAM                          0x00A1
     #define   NB_SEND_BROADCAST_DATAGRAM                   0x00A2
     #define   NB_RECEIVE_BROADCAST_DATAGRAM                0x00A3
     #define   NB_ADD_NAME                                  0x00B0
     #define   NB_DELETE_NAME                               0x00B1
     #define   NB_STATUS                                    0x00B3
     #define   NB_SESSION_STATUS                            0x00B4
     #define   NB_ADD_GROUP_NAME                            0x00B6
     #define   NB_ENUMERATE_LANA_NUM                        0x00B7
     #define   NB_UNLINK                                    0x00F0
     #define   NB_SEND_NO_ACK                               0x00F1
     #define   NB_CHAIN_SEND_NO_ACK                         0x00F2
     #define   NB_LAN_STATUS_ALERT                          0x00F3
     #define   NB_ACTION                                    0x00F7
     #define   NB_FIND_NAME                                 0x00F8
     #define   NB_TRACE                                     0x00F9

     #define   NB_INVALID_COMMAND_CODE                      0x00FF


/*----------------------------------------------------------------*/
/*                       NETBIOS RETURN CODES                     */
/*----------------------------------------------------------------*/

     #define   NB_COMMAND_SUCCESSFUL                        0x0000
     #define   NB_ILLEGAL_BUFFER_LEN                        0x0001
     #define   NB_INVALID_COMMAND                           0x0003
     #define   NB_COMMAND_TIME_OUT                          0x0005
     #define   NB_MESSAGE_INCOMPLETE                        0x0006
     #define   NB_DATA_NOT_RECEIVED                         0x0007
     #define   NB_ILLEGAL_LOCAL_SESSION                     0x0008
     #define   NB_NO_RES_AVAILABLE                          0x0009
     #define   NB_SESSION_CLOSED                            0x000A
     #define   NB_COMMAND_CANCELLED                         0x000B
     #define   NB_DUPLICATE_NAME                            0x000D
     #define   NB_NAME_TABLE_FULL                           0x000E
     #define   NB_CMND_CMPL_DEREGISTERED                    0x000F
     #define   NB_SESSION_TABLE_FULL                        0x0011
     #define   NB_SES_OPEN_REJECTED                         0x0012
     #define   NB_ILLEGAL_NAME_NUMBER                       0x0013
     #define   NB_REMOTE_NAME_NOT_FOUND                     0x0014
     #define   NB_LOCAL_NAME_NOT_FOUND                      0x0015
     #define   NB_NAME_IN_USE                               0x0016
     #define   NB_NAME_DELETED                              0x0017
     #define   NB_SESSION_ENDED_ABNORMALLY                  0x0018
     #define   NB_NAME_CONFLICT                             0x0019
     #define   NB_INTERFACE_BUSY                            0x0021
     #define   NB_MAX_CMNDS_EXCEEDED                        0x0022
     #define   NB_INVALID_ADAPTER                           0x0023
     #define   NB_CMND_ALREADY_COMPLETED                    0x0024
     #define   NB_CMND_INVALID_TO_CANCEL                    0x0026
     #define   NB_NAME_DEFINED_BY_OTHERS                    0x0030
     #define   NB_ENVIRONMENT_NOT_DEFINED                   0x0034
     #define   NB_NO_OS_RESOURCES                           0x0035
     #define   NB_MAX_APPL_EXCEEDED                         0x0036
     #define   NB_NO_SAP_AVAILABLE                          0x0037
     #define   NB_INADEQUATE_RESOURCES                      0x0038
     #define   NB_INVALID_NCB_ADDRESS                       0x0039
     #define   NB_RESET_INVALID                             0x003A
     #define   NB_INVALID_DD_ID                             0x003B
     #define   NB_SEGMENT_LOCK_UNSUCCESSFUL                 0x003C
     #define   NB_DD_OPEN_ERROR                             0x003F
     #define   NB_OS_ERROR_DETECTED                         0x0040
     #define   NB_PERM_RING_STATUS                          0x004F

     #define   NB_UNEXPECTED_CCB_ERROR                      0x00F6
     #define   NB_ADAPTER_OPEN_ERROR                        0x00F8
     #define   NB_ADAPTER_HANDLER_ERROR                     0x00F9
     #define   NB_ADAPTER_CHECK                             0x00FA
     #define   NB_CODE_NOT_OPERATIONAL                      0x00FB
     #define   NB_OPEN_FAILURES                             0x00FC
     #define   NB_UNEXPECTED_CLOSE                          0x00FD

     #define   NB_COMMAND_IN_PROCESS                        0x00FF

/*------------------------------------------------------------------*/
/*               NETBIOS NCB SESSION STATES                         */
/*------------------------------------------------------------------*/

     #define   NB_SESSION_STATE_CLOSED                      0x0000
     #define   NB_SESSION_STATE_LISTEN_PENDING              0x0001
     #define   NB_SESSION_STATE_CALL_PENDING                0x0002
     #define   NB_SESSION_STATE_SESSION_ACTIVE              0x0003
     #define   NB_SESSION_STATE_HANGUP_PENDING              0x0004
     #define   NB_SESSION_STATE_HANGUP_COMPLETE             0x0005
     #define   NB_SESSION_STATE_SESSION_ABORTED             0x0006

#define NETBIOS_NAME_LEN 16

#ifdef OS2ONLY
/*------------------------------------------------------------------*/
/*               NETBIOS NCB DATA DECLARATIONS                      */
/*------------------------------------------------------------------*/

struct     network_control_block
 {
  byte       ncb_command;               /* Netbios command code      */
  byte       ncb_retcode;               /* Return code               */
  byte       ncb_lsn;                   /* Local session number      */
  byte       ncb_num;                   /* Number of application name*/
  address    ncb_buffer_address;        /* Address of message buffer */
  word       ncb_length;                /* length of message buffer  */
  byte       ncb_callname[16];          /* Destination name          */
  byte       ncb_name[16];              /* Source name               */
  byte       ncb_rto;                   /* Receive timeout           */
  byte       ncb_sto;                   /* Send timeout              */
  union                                 /* Offset 44 parameters      */
   {
   address   ncb_post_address;          /* Address of post routine   */
   HSEM16    ncb_post_handle;           /* Handle to event semaphore */
   struct
    {
    word     ncb_post_addr_offset;      /* Offset of post routine    */
    word     ncb_dd_id;                 /* Device driver ID          */
    } DD;
   } off44;
  byte       ncb_lana_num;              /* Adapter number            */
  byte       ncb_cmd_cplt;              /* Command status            */
  byte       ncb_reserve[14];           /* Reserved (except RESET)   */
 };


struct     ncb_status_information
 {
  byte       burned_in_addr[6];         /* Adapter's burned in addr  */
  byte       reserved1[2];              /* RESERVED always X'0000'   */
  word       software_level_number;     /* X'FFnn' - nn is level num */
  word       reporting_period;          /* reporting period (minutes)*/
  word       frmr_frames_received;      /* Number of FRMR received   */
  word       frmr_frames_sent;          /* Number of FRMR sent       */
  word       bad_iframes_received;      /* # bad Iframes received    */
  word       aborted_transmissions;     /* # aborted transmits       */
  dword      packets_transmitted;       /* # Successfully transmitted*/
  dword      packets_received;          /* # Successfully received   */
  word       bad_iframes_transmitted;   /* # bad Iframes transmitted */
  word       lost_data_count;           /* Lost SAP buffer data cnt  */
  word       t1_expiration_count;       /* Number of T1 expirations  */
  word       ti_expiration_count;       /* Number of Ti expirations  */
  address    extended_status_table;     /* Address of extended status*/
  word       number_of_free_ncbs;       /* Number of NCBs available  */
  word       max_configured_ncbs;       /* Configured NCB maximum    */
  word       max_allowed_ncbs;          /* Maximum NCBs (always 255) */
  word       busy_condition_count;      /* Local station busy count  */
  word       max_datagram_size;         /* Maximum datagram packet   */
  word       pending_sessions;          /* Number of pending sessions*/
  word       max_configured_sessions;   /* Configured session maximum*/
  word       max_allowed_sessions;      /* Maximum sessions (254)    */
  word       max_data_packet_size;      /* Maximum session packet    */
  word       number_of_names_present;   /* Number of names in table  */
 };


struct     ncb_extended_status
 {
  byte       reserved[40];              /* RESERVED                  */
  byte       local_adapter_address[6];  /* Adapter's local address   */
 };


struct     ncb_session_status
 {
  byte       name_number_of_sessions;   /* Name number for sessions  */
  byte       sessions_using_name;       /* # of sessions using name  */
  byte       active_rcv_datagrams;      /* # of receive datagrams out*/
  byte       active_receive_anys;       /* # of RECEIVE.ANY cmnds out*/
  byte       local_session_number;      /* Local session number      */
  byte       session_state;             /* State of session          */
  byte       local_name[16];            /* Local name                */
  byte       remote_name[16];           /* Remote name               */
  byte       active_receives;           /* # of RECEIVE cmnds out    */
  byte       active_sends;              /* # of SEND, CHAIN.SEND out */
 };


struct     ncb_find_name_info
 {
  word       nodes_responding;          /* Number of nodes responding*/
  byte       reserved;                  /* RESERVED                  */
  byte       name_status;               /* Unique/Group name flag    */
 };


struct     ncb_lan_header_entry
 {
  byte       lan_entry_length;          /* Length of entry           */
  byte       lan_pcf0;                  /* Physical control field 0  */
  byte       lan_pcf1;                  /* Physical control field 1  */
  byte       lan_destination_addr[6];   /* Destination address       */
  byte       lan_source_addr[6];        /* Source address            */
  byte       lan_routing_info[18];      /* Routing information       */
 };

struct     ncb_chain_send
 {
  byte       ncb_command;               /* Netbios command code      */
  byte       ncb_retcode;               /* Return code               */
  byte       ncb_lsn;                   /* Local session number      */
  byte       not_used1;                 /* Not used                  */
  address    ncb_buffer_address;        /* Address of message buffer */
  word       ncb_length;                /* Length of message buffer  */
  word       buffer_two_length;         /* Length of second buffer   */
  address    buffer_two_address;        /* Address to second buffer  */
  byte       reserved[10];              /* RESERVED                  */
  byte       not_used2[18];             /* Not used                  */
  union                                 /* Offset 44 parameters      */
   {
   address   ncb_post_address;          /* Address of post routine   */
   struct
    {
    word     ncb_post_addr_offset;      /* Offset of post routine    */
    word     ncb_dd_id;                 /* Device driver ID          */
    } DD;
   } off44 ;
  byte       ncb_lana_num;              /* Adapter number            */
  byte       ncb_cmd_cplt;              /* Command status            */
  byte       ncb_reserve[14];           /* Reserved                  */
 };


struct     ncb_reset
 {
  byte       ncb_command;               /* Netbios command code      */
  byte       ncb_retcode;               /* Return code               */
  byte       ncb_lsn;                   /* Local session number      */
  byte       ncb_num;                   /* Number of application name*/
  address    dd_name_address;           /* Device drive name address */
  byte       not_used1[2];              /* Not used                  */
  byte       req_sessions;              /* # of sessions requested   */
  byte       req_commands;              /* # of commands requested   */
  byte       req_names;                 /* # of names requested      */
  byte       req_name_one;              /* Name number one request   */
  byte       not_used2[12];             /* Not used                  */
  byte       act_sessions;              /* # of sessions obtained    */
  byte       act_commands;              /* # of commands obtained    */
  byte       act_names;                 /* # of names obtained       */
  byte       act_name_one;              /* Name number one response  */
  byte       not_used3[4];              /* Not used                  */
  byte       load_session;              /* Number of sessions at load*/
  byte       load_commands;             /* Number of commands at load*/
  byte       load_names;                /* Number of names at load   */
  byte       load_stations;             /* Number of stations at load*/
  byte       not_used4[2];              /* Not used                  */
  byte       load_remote_names;         /* Number of remote names    */
  byte       not_used5[5];              /* Not used                  */
  word       ncb_dd_id;                 /* NCB device driver ID      */
  byte       ncb_lana_num;              /* Adapter number            */
  byte       not_used6;                 /* Not used                  */
  byte       ncb_reserve[14];           /* NCB error information     */
 };

/*------------------------------------------------------------------*/
/*               NETBIOS TRACE DATA DECLARATIONS                    */
/*------------------------------------------------------------------*/

struct     ncb_entry_header
 {
  byte       trace_adapter_number;      /* Adapter number for trace  */
  byte       trace_environment_id;      /* Trace environment ID      */
  byte       trace_type;                /* Trace type                */
  byte       trace_modifier;            /* Trace modifier            */
  byte       reserved[4];               /* RESERVED                  */
 };

struct     ncb_trace_ncb
 {
  struct     ncb_entry_header common;   /* Trace entry header        */
  byte       byte_preceding_ncb;        /* Byte preceding NCB        */
  byte       byte_following_ncb;        /* Byte following NCB        */
  address    post_address;              /* Address of post routine   */
  address    data_address;              /* Address of data           */
  byte       data_sample[16];           /* Sample of data at address */
 };


struct     ncb_trace_status
 {
  struct     ncb_entry_header common;   /* Trace entry header        */
  word       ds_register;               /* DS register contents      */
  word       ss_register;               /* SS register contents      */
  word       sp_register;               /* SP register contents      */
  address    data_address;              /* Address of data           */
 };


struct     ncb_trace_ring_status
 {
  struct     ncb_trace_status common;   /* Trace entry header        */
  word       ring_status;               /* Ring status               */
 };


struct     ncb_system_action
 {
  struct     ncb_trace_status common;   /* Trace entry header        */
  byte       system_action_id;          /* System action ID          */
  byte       sap_value;                 /* SAP value affected        */
 };


struct     ncb_trace_pc_error
 {
  struct     ncb_trace_status common;   /* Trace entry header        */
  word       error_code;                /* PC-Detected error code    */
 };


struct     ncb_trace_adapter_status
 {
  struct     ncb_trace_status common;   /* Trace entry header        */
  word       status_code;               /* Adapter Check reason code */
 };


struct     ncb_trace_dlc_status
 {
  struct     ncb_trace_status common;   /* Trace entry header        */
  word       station_id;                /* Station ID                */
  word       dlc_status;                /* DLC Status code           */
  byte       frmr_data[5];              /* FRMR data                 */
  byte       access_priority;           /* Access priority           */
  byte       remote_node_addr[6];       /* Remote node address       */
 };


struct     ncb_trace_return_code
 {
  struct     ncb_trace_status common;   /* Trace entry header        */
  byte       msg_header_xmit[16];       /* Message header transmitted*/
 };


struct     ncb_trace_receive
 {
  struct     ncb_trace_status common;   /* Trace entry header        */
  byte       msg_header_rcv[16];        /* Message header received   */
 };


struct     ncb_trace_ccb
 {
  struct     ncb_trace_status common;   /* Trace entry header        */
  byte       ccb_data[16];              /* CCB data structure        */
 };

extern unsigned NETBIOS (char *);
#pragma linkage(NETBIOS, far16 pascal)

union ncb_types
   {
   struct fncb {
       struct network_control_block bncb;
       HSEM16 ncb_semaphore;
   } basic_ncb;
   struct ncb_chain_send send;
   struct ncb_reset reset;
   };

/****************************************************************
 *                                                              *
 *              Begin NETBEUI declarations and constants        *
 *                                                              *
 ****************************************************************/

#define DEVLEN 8

typedef struct netbios_info_0 {
    char           nb0_net_name[NETBIOS_NAME_LEN+1];
} NETINFO0, *PNETINFO0;      /* netbios_info_0 */

typedef struct netbios_info_1 {
    char           nb1_net_name[NETBIOS_NAME_LEN+1];
    char           nb1_driver_name[DEVLEN+1];/* OS/2 device driver name  */
    unsigned char  nb1_lana_num;       /* LAN adapter number of this net */
    char           nb1_pad_1;
    unsigned short nb1_driver_type;
    unsigned short nb1_net_status;
    unsigned long  nb1_net_bandwidth;  /* Network bandwidth, bits/second */
    unsigned short nb1_max_sess;       /* Max number of sessions         */
    unsigned short nb1_max_ncbs;       /* Max number of outstanding NCBs */
    unsigned short nb1_max_names;      /* Max number of names            */
} NETINFO1, *PNETINFO1;      /* netbios_info_1 */

/****************************************************************
 *                                                              *
 *              Special values and constants                    *
 *                                                              *
 ****************************************************************/


/*
 *      Driver types (nb1_driver_type).
 */

#define NB_TYPE_NCB     1
#define NB_TYPE_MCB     2

/*
 *      Bits defined in nb1_net_status.
 */

#define NB_LAN_FLAGS_MASK       0x3FFF  /* Mask for LAN Flags */
#define NB_LAN_MANAGED          0x0001  /* LAN is managed by redirector */
#define NB_LAN_LOOPBACK         0x0002  /* LAN is a loopback driver */
#define NB_LAN_SENDNOACK        0x0004  /* LAN allows SendNoAck NCBs */
#define NB_LAN_LMEXT            0x0008  /* LAN supports LAN Manager
                                                            extended NCBs */
#define NB_LAN_INTNCB           0x0010  /* LAN allows NCB submission at */
                                        /* interrupt time (from NCBDone) */
#define NB_LAN_NORESET          0x0040

#define NB_OPEN_MODE_MASK       0xC000  /* Mask for NetBios Open Modes */
#define NB_OPEN_REGULAR         0x4000  /* NetBios opened in Regular mode */
#define NB_OPEN_PRIVILEGED      0x8000  /* NetBios opened in Privileged mode */
#define NB_OPEN_EXCLUSIVE       0xC000  /* NetBios opened in Exclusive mode */

/*
 *      Open modes for NetBiosOpen.
 */

#define NB_REGULAR      1
#define NB_PRIVILEGED   2
#define NB_EXCLUSIVE    3

#define NCBSIZE sizeof(union ncb_types)
#define BNCBSIZE sizeof(struct network_control_block)
typedef union ncb_types NCB;
typedef union ncb_types * PNCB, * _Seg16 PNCB16 ;
typedef struct ncb_status_information STATUSINFO, *PSTATUSINFO;
typedef struct ncb_session_status SESSIONINFO, *PSESSIONINFO;
typedef struct ncb_find_name FINDNAMEINFO, *PFINDNAMEINFO;
typedef struct pbuf2 { USHORT Length; PBYTE Buffer; } *PBuf2;
#else /* OS2ONLY */
#define NCBSIZE sizeof(NCB)
#define BNCBSIZE sizeof(NCB)
typedef ADAPTER_STATUS STATUSINFO, *PSTATUSINFO;
typedef SESSION_HEADER SESSIONINFO, *PSESSIONINFO;
typedef FIND_NAME_HEADER FINDNAMEINFO, *PFINDNAMEINFO;
typedef struct pbuf2 { USHORT Length; PBYTE Buffer; } *PBuf2;
#endif /* OS2ONLY */

USHORT NCBAddGroupName(BOOL Netbeui, PNCB Ncb,USHORT lana,PBYTE name);
USHORT NCBAddName(BOOL Netbeui, PNCB Ncb,USHORT lana,PBYTE name);
USHORT NCBCall(BOOL Netbeui, PNCB Ncb,USHORT lana,PBYTE lclname,
   PBYTE rmtname,USHORT recv_timeout,USHORT send_timeout,BOOL wait);
USHORT NCBCancel(BOOL Netbeui, PNCB Ncb,USHORT lana,PNCB NcbToCancel);
USHORT NCBChainSend(BOOL Netbeui, PNCB Ncb,USHORT lana,USHORT lsn,
   PBYTE message,USHORT length,PBYTE Buffer2, USHORT Length2,BOOL wait);
USHORT NCBChainSendNoAck(BOOL Netbeui, PNCB Ncb,USHORT lana,USHORT lsn,
   PBYTE message,USHORT length,PBYTE Buffer2, USHORT Length2,BOOL wait);
USHORT NCBClose(BOOL Netbeui, PNCB Ncb,USHORT lana);
USHORT NCBConfig(BOOL Netbeui, PNCB Ncb,USHORT lana,PUSHORT sessions,
   PUSHORT commands,PUSHORT names);
USHORT NCBDeleteName(BOOL Netbeui, PNCB Ncb,USHORT lana,PBYTE lclname);
USHORT NCBHangup(BOOL Netbeui, PNCB Ncb,USHORT lana,USHORT lsn);
USHORT NCBListen(BOOL Netbeui, PNCB Ncb,USHORT lana,PBYTE lclname,
   PBYTE rmtname,USHORT recv_timeout,USHORT send_timeout,BOOL wait);
USHORT NCBReceive(BOOL Netbeui, PNCB Ncb,USHORT lana,USHORT lsn,PBYTE buffer,
   USHORT length,BOOL wait);
USHORT NCBReceiveAny(BOOL Netbeui, PNCB Ncb,USHORT lana,USHORT lsn,
   PBYTE buffer,USHORT length,BOOL wait);
USHORT NCBReceiveBroadcast(BOOL Netbeui, PNCB Ncb,USHORT lana,USHORT lsn,
   PBYTE buffer,USHORT length,BOOL wait);
USHORT NCBReceiveDatagram(BOOL Netbeui, PNCB Ncb,USHORT lana,USHORT lsn,
   PBYTE buffer,USHORT length,BOOL wait);
USHORT NCBReset(BOOL Netbeui, PNCB Ncb,USHORT lana,USHORT sessions,
   USHORT commands,USHORT names);
USHORT NCBSend(BOOL Netbeui, PNCB Ncb,USHORT lana,USHORT lsn,PBYTE message,
   USHORT length,BOOL wait);
USHORT NCBSendBroadcast(BOOL Netbeui, PNCB Ncb,USHORT lana,USHORT lsn,
   PBYTE message,USHORT length,BOOL wait);
USHORT NCBSendDatagram(BOOL Netbeui, PNCB Ncb,USHORT lana,USHORT lsn,
   PBYTE rmtname,PBYTE message,USHORT length,BOOL wait);
USHORT NCBSendNoAck(BOOL Netbeui, PNCB Ncb,USHORT lana,USHORT lsn,
   PBYTE message,USHORT length,BOOL wait);
USHORT NCBStatus(BOOL Netbeui, PNCB Ncb, USHORT lana, PBYTE callname,
   PSTATUSINFO pInfo, word length, BOOL wait );
USHORT NCBSessionStatus(BOOL Netbeui, PNCB Ncb, USHORT lana, PBYTE lclname,
   PSESSIONINFO pInfo, word length, BOOL wait );
USHORT NCBFindName( BOOL Netbeui, PNCB Ncb, USHORT lana, PFINDNAMEINFO pInfo,
   USHORT length, BOOL wait ) ;
VOID NCBCancelOutstanding( VOID );

#ifdef OS2ONLY
USHORT netbios_avail(BOOL Netbeui);
USHORT loadapi(PSZ module, PSZ proc, PFN FAR *addr);
#endif /* OS2ONLY */

#define NB_ADAPTERS 4               /* number of virtual adapters */
#define NB_LSN 2                    /* number of NetBios sessions */
#define NB_NCB MAXWS+3              /* number of outstanding NCBs */
#define NB_NAMES 2                  /* number of names            */
#define NB_RECV_TIMEOUT 0           /* seconds                    */
#define NB_SEND_TIMEOUT 0           /* seconds                    */

extern BOOL NetbeuiAPI ;
extern USHORT netbiosAvail ;
_PROTOTYP( void os2_netbiosinit, (void) ) ;
_PROTOTYP( void os2_netbioscleanup,(void) ) ;


#ifdef OS2ONLY
/* Declarations of 16-bit Semaphore interface */

APIRET16 APIENTRY16 Dos16CreateSem( USHORT, PHSEM16, PSZ ) ;
APIRET16 APIENTRY16 Dos16SemWait( HSEM16, LONG ) ;
APIRET16 APIENTRY16 Dos16SemSet( HSEM16 ) ;
APIRET16 APIENTRY16 Dos16SemClear( HSEM16 ) ;
APIRET16 APIENTRY16 Dos16CloseSem( HSEM16 ) ;
#endif /* OS2ONLY */

#endif /* CKONBI_H */
#endif /* CKCMAI */
#endif /* CK_NETBIOS */
