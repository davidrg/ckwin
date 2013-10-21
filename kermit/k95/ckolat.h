/*
  Author:  Jeffrey E Altman (jaltman@secure-endpoints.com),
             Secure Endpoints Inc., New York City

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#ifndef CKOLAT_H
#define CKOLAT_H

/* Definitions for the C interface to the DECnet LAT protocol */
/* Part of DEC PATHWORKS 2.0 for OS/2 */

#define uchar unsigned char
#define uint  unsigned short
#define ulong unsigned long

#define def_num_sessions        4       /* default # of sessions */
#define max_num_sessions        10      /* maximum # of sessions */
#define min_num_sessions        4       /* minimum # of sessions */

#define def_num_buffers         4       /* default number of buffers */
#define max_num_buffers         8       /* maximum number of buffers */
#define min_num_buffers         2       /* minimun number of buffers */

#define def_num_services        50      /* default number of services */
#define max_num_services        2048    /* maximum number of services */
#define min_num_services        10      /* minimum number of services */

#define def_num_retx            8       /* default number of retransmit */
#define max_num_retx            255     /* maximum number of retransmit */
#define min_num_retx            4       /* minimum number of retransmit */

#define SEND_CHAR       1               /* send a character */
#define GET_CHAR        2               /* read a character */
#define GET_CHAR_BLK    3               /* read a block of characters */
#define GET_STATUS      4               /* slot status */
#define START_SESSION   5               /* start a session */
#define START_SESS_PSW  6               /* start session with password */
#define STOP_SESSION    7               /* stop a session */
#define SEND_BREAK      8               /* send a break signal */
#define ADD_PREFER_NODE 9               /* add preferred node */
#define DEL_PREFER_NODE 10              /* delete preferred node */
#define RESET_COUNTERS  11              /* reset lat counter */
#define READ_COUNTERS   12              /* read lat internal counters */
#define RESET_SERVICES  13              /* reset service index */
#define READ_SERVICES   14              /* read a service entry */
#define READ_SCB        15              /* read all of the scbs */
#define READ_VCB        16              /* read all of the vcbs */
#define READ_LMCB       17              /* read the lmcb */
#define READ_LATINFO    18              /* read lat info */
#define READ_SERVICE_ENTRY 19           /* read service entry info */
#define SET_RETX_LIMIT  20              /* set retransmit limit in lmcb */
#define SET_MULTI_STATE 21              /* set state of multicast receiver */
#define SET_GROUP_CODE  22              /* enable a specific group code  */
#define CLEAR_GROUP_CODE 23             /* clear a specific group code  */

/* Status of a remote service */
#define UNREACH         0x01    /* the node for this service is unreachable */

/* State of service table */
#define NOT_OVERFLOW    0x00    /* service table not in overflow state */
#define OVERFLOW        0xFF    /* service table in overflow state     */

/* Multicast States */
#define MULTICAST_ENABLE   0x00 /* enable the reception of multicast msgs  */
#define MULTICAST_DISABLE  0xFF /* disable the reception of mutlicast msgs */

/* Wait Times */
#define LAT_INDEFINITE_WAIT  -1  /* Wait until the desired event occurs  */
#define LAT_IMMEDIATE_RETURN  0  /* Do not wait */

/* Masks for fields which may be set/shown in SET_SERVICE/GET_SERVICE call */
#define SRV_FLAGS          0x0001  /* set/show flags field            */
#define SRV_RATING         0x0002  /* set/show rating field           */
#define SRV_IDENT          0x0004  /* set/show identification field   */
#define SRV_PASSWORD       0x0008  /* set/show password field         */
#define SRV_CURSESSIONS    0x0010  /* show current sessions field     */
#define SRV_MAXSESSIONS    0x0020  /* set/show maximum sessions field */
#define SRV_SERVICE        0x0040  /* show service namae              */
#define SRV_SET_ALL \
 (SRV_FLAGS|SRV_RATING|SRV_IDENT|SRV_PASSWORD|SRV_MAXSESSIONS)
#define SRV_GET_ALL        (SRV_SET_ALL|SRV_SERVICE|SRV_CURSESSIONS)

/* Masks for flags field of service structure */
#define SRV_ADVERTISE      0x01   /* send out service announcements   */
#define SRV_ENABLE         0x02   /* enable service for connections   */
#define SRV_SLAVE          0x04   /* allow incoming connections       */

/* Bit definitions of LatStatus in LAT_CB */
#define LS_NoError              0x0000  /* No error for this function */
#define LS_NoChar               0x8000  /* No character read */
#define LS_CharNotSent          0x8000  /* Character not sent */
#define LS_BrkNotSent           0x8000  /* Break not sent */
#define LS_NotInRun             0x4000  /* Slot session not in run state */
#define LS_TxBufEmpty           0x2000  /* Transmit buffer empty */
#define LS_InvalidSize          0x1000  /* Bad size specified */
#define LS_SesStart             0x0800  /* Session in starting state */
#define LS_InvalidHdl           0x0400  /* Invalid session handle */
#define LS_NoMoreSession        0x0400  /* No more slot session available */
#define LS_TxBufBusy            0x0200  /* Unable to queue transmit data */
#define LS_NoCircuit            0x0200  /* No more virtual circuit */
#define LS_RxData               0x0100  /* Receive data available */
#define LS_NoService            0x0100  /* Service name not found */
#define LS_NoMoreService        0x0080  /* no more new service */
#define LS_NoDLL                0x0040  /* No DLL handle        */
#define LS_OtherError           0x0020  /* another error has happened */
#define LS_IllegalFnc           0xFFFF  /* illegal function */

/* LAT_CB Session Status Word */
#define SS_TxBufBusy            0x00    /* Slot session tx buffer busy */
#define SS_Stopped              0x08    /* Slot session stopped! */
#define SS_HostStop             0x10    /* Host sent stop slot */

#define IS_InitOK               0x0001  /* LAT driver init'ed */
#define IS_SessionErr           0x0002  /* Session specifier error */
#define IS_BufferErr            0x0004  /* Buffer specifier error */
#define IS_ServiceErr           0x0008  /* Service specifier error */
#define IS_ReTxErr              0x0010  /* Retransmit specifier error */
#define IS_GetMemErr            0x0800  /* Allocate memory error */
#define IS_LatPortailFailed     0x2000  /* Unable to open LAT portal */
#define IS_NeedDll              0x4000  /* DLL driver needed */
#define IS_AlreadyInit          0x8000  /* LAT driver already init'ed */

struct  lat_init {
    uint        MaxSessions;            /* Maximum number of slot sessions */
    uint        MaxBuffers;             /* Maximum buffers per session */
    uint        MaxServices;            /* Maximum number of services */
    uint        ReTxLimit;              /* Retransmit limit */
    uint        InitStatus;             /* Initialization status */
};

struct  lat_info {
    uchar       vermjr;       /* Major LAT version number  */
    uchar       vermir;       /* Minor LAT version number  */
    uint        NSessions;    /* Maximum number of sessions */
    uint        NBuffers;     /* Maximum number of buffers per session */
    uint        NServices;    /* Maximum number of services */
    uint        scbsize;      /* Memory used by each session (in bytes) */
};

struct service_info {
    uchar       node_leng;              /* node name length */
    uchar       node_name[16];          /* node name string */
    uchar       node_addr[6];           /* node address */
    uchar       node_status;            /* node status */
    uchar       serv_rate;              /* service rating */
    uchar       serv_leng;              /* service length */
    uchar       serv_name[16];          /* service string */
    uchar       serv_status;            /* service status */
};

struct  scb_info {
    uint        scb_inuse;              /* scb inuse flag */
    uint        reserved1;              /* reserved */
    uint        reserved2;              /* reserved */
};

struct  lat_cb  {
    uchar       LatFunction;            /* function code */
    uint        SessionHandle;          /* lat handle */
    uint        SessionStatus;          /* Session status */
    uint        StopReason;             /* Stop reason */
    uint        BufferSize;             /* size of buffer */
    ulong * _Seg16 BufferPtr;           /* pointer to buffer */
    ulong       WaitTime;               /* waiting time */
    uchar       CharByte;               /* I/O character */
    uint        LatStatus;              /* function returned status */
};

/*
  The following structures are no longer used but are kept for backwards
  compatibility.
*/
struct  lmcb_struct {
    uchar       lat_mess_retrans_limit; /* retransmit limit */
    uchar       group_enable[32];       /* group code */
    uchar       num_active_circuit;     /* number of active circuits */
    uchar       num_active_session;     /* number of active sessions */
    uint        lat_min_rcv_datagram_size; /* minimun datagram size */
    uchar       protocol_version;       /* protocol version */
    uchar       protocol_eco;
    uchar       max_sim_slots;          /* maximum slots on this circuit */
    uchar       nbr_dl_bufs;            /* number datalink buffers minus 1 */
    uchar       server_circuit_timer;   /* data transfer timer */
    uchar       lat_keep_alive_timer;   /* keep alive timer (sec)*/
    uint        facility_number;        /* Not used */
    uint        product_code;           /* PC LAT server assigned code */
    uchar       server_retransmit_timer; /* unacked retransmit timer(sec) */
    uchar       lat_min_rcv_slot_size;  /* minimum slot size */
    uchar       lat_min_rcv_att_slot_size; /* minimum attention slot size */
    uchar       server_name_len;        /* lat server name length */
    uchar       server_name[16];        /* lat server name */
};

struct  node_entry {
    uchar       node_name_len;
    uchar       node_name[16];
    uchar       node_address[6];
    uchar       status_flag;
    uchar       msg_incarnation;
    uchar       node_change_flags;
};

struct  vcb_struct {
    uint        vc_state;               /* VC state             */
    uchar       circuit_name[18];       /* circuit name         */
    uchar       vc_type;                /* DCB type             */
    uchar       vc_format;              /* FORMAT of portal     */
    uchar       rem_address[6];         /* Remote adapter address */
    uchar       loc_address[6];         /* local adapter address  */
    uint        cp_typ;                 /* Protocol type          */
    uint        cpad;                   /* padding                */
    uchar       cctl;                   /* mo' paddin'            */
    uchar       cp_ident[5];            /* even mo' paddin'       */
    uint        cportal_id;             /* portal number          */
    ulong       unacked_xmtb;           /* unacked message */
    uint        unacked_xmtb_len;
    uchar       param_len[5];           /* even mo' paddin'       */
    uchar       msg_typ;                /* message type */
    uint        loc_cir_id;             /* local circuit ID */
    uint        rem_cir_id;             /* remote circuit ID */
    uchar       nxmt;                   /* next message number to tx */
    uchar       ack;                    /* most recent message acked */
    uchar       rrf;                    /* response request flag */
    uchar       dwf;                    /* data waiting flag */
    uchar       vc_status;              /* number of sessions */
    uint        slot_index; /* index to first slot session on this circuit */
    uint        last_xmit;  /* index to last slot tx'ed on this circuit */
    uchar       retransmit_counter;     /* retransmits on this message */
    uchar       uxmt;                   /* unack'ed message # */
    uchar       retransmit_timer_on;    /* re-transmit timer turned on */
    uchar       retransmit_timer;       /* count down for retransmit */
    uchar       balanced_mode_on;       /* balance mode active flag */
    uint        balanced_mode_timer;    /* tick counts for lat_keep_alive */
    uint        max_buf_size;           /* max tx buffer size */
    uchar       max_circuit_sessions;   /* max circuits */
    uint        disconnect_reason;      /* disconnect reason */
};

struct  scb_struct {
    uchar       service[18];            /* service name */
    uchar       node[18];
    uchar       port[18];
    ulong       scb_sem;
    uint        session_status;         /* session status word */
    uint        stop_reason;            /* session stop reason */
    uint        slot_state;             /* slot state 0=halted */
    uchar       local_credits;          /* available credits to tx slots */
    uint        vcb_offset;
    uint        vcb_segment;
    uint        back_slot;              /* index to back slot on this circ */
    uint        forward_slot;           /* index to forward slot */
    uchar       rem_slot_id;            /* remote slot connect ID */
    uchar       loc_slot_id;            /* local slot ID */
    uchar       slot_byte_count;        /* number of bytes in tx_slot_data */
    uchar       remote_credits;         /* credits from remote node */
    uchar       tx_slot_data[255];      /* data to be tx'ed */
    uchar       num_slots;              /* number of slots in this buffer */
    uchar       num_occupied;           /* number of slots in use */
    uchar       next_rx_slot;           /* index to next slot to be used */
    uchar       cur_buf_slot;           /* index to current slot in use */
    uint        rx_slot_pntr;           /* offset to next char to be taken */
    uint        slot_ptr_table[2];      /* pointers to the slots */
};

struct flagbits {
    unsigned SW_duplicate : 1;
    unsigned SW_invalid_value : 1;
    unsigned SW_found : 1;
};

struct  service_entry {
    uint        node_pointer;
    uchar       service_rating;
    uchar       service_name_len;
    uchar       service_name[16];
    uchar       service_status_flags;
};

struct lat_count {
    uint        n_count;        /* number of counters */
    uint        ids[12];
    ulong       c1;
    ulong       c2;
    ulong       c3;
    ulong       c4;
    ulong       c5;
    ulong       c6;
    ulong       c7;
    ulong       c8;
    ulong       c9;
    ulong       c10;
    ulong       c11;
    ulong       c12;
    ulong       c13;
    ulong       c14;
};

struct  lat_counter {
    ulong       second_since_zeroed;
    ulong       messages_transmitted;
    ulong       messages_received;
    ulong       messages_retransmitted;
    ulong       out_of_seq_mess_received;
    ulong       illegal_messages_received;
    ulong       illegal_slots_received;
    ulong       buf_que_entry_unavailable;
    ulong       transmit_buffer_unavailable;
    ulong       invalid_messages_received;
    ulong       invalid_slot_received;
    ulong       invalid_multicast_received;
};

struct decnode {
        unsigned char decnode_flags;
        unsigned short decnode_address;
        char decnode_name[7];
        long decnode_pointer;
        };

struct SIB {
    char schar;         /* switch character */
    short value;        /* switch value */
    short defv;         /* default value */
    short minv;         /* min value */
    short maxv;         /* max value */
    struct flagbits flags;              /* switch status flags */
};

#endif /* CKOLAT_H */
