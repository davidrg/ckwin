/*
 * Copyright 1995 Jyrki Salmi, Online Solutions Oy (www.online.fi)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef _P_H_
#define _P_H_

#include "p_type.h"             /* U32, U16, U8, etc typedefs */

#define CKDEVAPI	cdecl

#ifdef NT
#define _Inline __inline
#define APIRET DWORD
#else
#ifdef __WATCOMC__
#define _Inline inline
#endif /* __WATCOMC__ */
#endif /* NT */

/******************/
/* Protocol types */
/******************/

#define PROTOCOL_X                      0x01    /* Xmodem */
#define PROTOCOL_Y                      0x02    /* Ymodem */
#define PROTOCOL_G                      0x03    /* Ymodem-g */
#define PROTOCOL_Z                      0x04    /* Zmodem */


/****************************************/
/* Supported communication device types */
/****************************************/

#define DEV_TYPE_ASYNC                  0x01 /* COM1, COM2, COM3, etc. */
#define DEV_TYPE_PIPE                   0x02 /* OS/2's named pipe */
#define DEV_TYPE_SOCKET                 0x03 /* TCP/IP's stream socket */
#define DEV_TYPE_EXE_IO                 0x04 /* Use EXE I/O system */

/***************/
/* Error types */
/***************/

#define is_os2_error(i)         (i < 0x30) /* A macro used to distinguish */
                                           /* OS/2 API errors from others */

#define P_ERROR_DOSOPEN                 0x01
 /* Optional argument:
  *  dev_path from P_CFG structure passed to p_transfer() function.
  */
#define P_ERROR_DOSCLOSE                0x02
 /* Optional argument: same as with P_ERROR_DOSOPEN */

#define P_ERROR_DOSREAD                 0x03
 /* Optional argument: same as with P_ERROR_DOSOPEN */

#define P_ERROR_DOSWRITE                0x04
 /* Optional argument: same as with P_ERROR_DOSOPEN */

#define P_ERROR_DOSDEVIOCTL             0x05
 /* Optional argument: same as with P_ERROR_DOSOPEN */

#define P_ERROR_DOSALLOCMEM             0x06
 /* Optional argument: Number of bytes we tried to allocate (U32) */

#define P_ERROR_DOSFREEMEM              0x07
 /* Optional argument: None (NULL) */

/*************************/
/* Named pipe API errors */
/*************************/

#define P_ERROR_DOSCREATENPIPE          0x08
 /* Optional argument: same as with P_ERROR_DOSOPEN */

#define P_ERROR_DOSCONNECTNPIPE         0x09
 /* Optional argument: same as with P_ERROR_DOSOPEN */

#define P_ERROR_DOSDISCONNECTNPIPE      0x0a
 /* Optional argument: same as with P_ERROR_DOSOPEN */

#define P_ERROR_DOSPEEKNPIPE            0x0b
 /* Optional argument: same as with P_ERROR_DOSOPEN */

#define P_ERROR_DOSSETNPHSTATE          0x0c
 /* Optional argument: same as with P_ERROR_DOSOPEN */

/******************/
/* DLL API errors */
/******************/

#define P_ERROR_DOSLOADMODULE           0x0d
 /* Optional argument: Name of DLL we tried to load (U8 *) */

#define P_ERROR_DOSFREEMODULE           0x0e
 /* Optional argument: Name of DLL we tried to free (U8 *) */

#define P_ERROR_DOSQUERYPROCADDR        0x0f
 /* Optional argument: Name of procedure we queried for (U8 *) */

/*********************/
/* Socket API errors */
/*********************/

#define is_tcpip_error(i)               (i >= 0x30) /* A macro used to */
                                                    /* distinguish TCP/IP */
                                                    /* API errors from */
                                                    /* others */

#define P_ERROR_SOCK_INIT               0x30
 /* Optional argument: None (NULL) */

#define P_ERROR_IOCTL                   0x31
 /* Optional argument: Handle of socket error occurred with (U32) */

#define P_ERROR_RECV                    0x32
 /* Optional argument: Handle of socket error occurred with (U32) */

#define P_ERROR_SEND                    0x33
 /* Optional argument: Handle of socket error occurred with (U32) */

#define P_ERROR_SOCKET                  0x34
 /* Optional argument: None (NULL) */

#define P_ERROR_BIND                    0x35
 /* Optional argument: Handle of socket error occurred with (U32) */

#define P_ERROR_LISTEN                  0x36
 /* Optional argument: Handle of socket error occurred with (U32) */

#define P_ERROR_ACCEPT                  0x37
 /* Optional argument: Handle of socket error occurred with (U32) */

#define P_ERROR_GETHOSTBYADDR           0x38
 /* Optional argument: The address we tried to resolve (U8 *) */

#define P_ERROR_GETHOSTBYNAME           0x39
 /* Optional argument: The address we tried to resolve (U8 *) */

#define P_ERROR_CONNECT                 0x3A
 /* Optional argument: Handle of socket error occurred with (U32) */

#define P_ERROR_SETSOCKOPT              0x3B
 /* Optional argument: Handle of socket error occurred with (U32) */

#define P_ERROR_GETSOCKOPT              0x3C
 /* Optional argument: Handle of socket error occurred with (U32) */

#define P_ERROR_SELECT                  0x3D
 /* Optional argument: Handle of socket error occurred with (U32) */


/***********************/
/* Zmodem header types */
/***********************/

/* One of these is passed as an argument for PS_HEADER ja PS_INVALID_HEADER */
/* messages. It's not necessary really to understand the meaning of these. */

#define HEADER_TIMEOUT                  0
#define HEADER_RQINIT                   1
#define HEADER_RINIT                    2
#define HEADER_SINIT                    3
#define HEADER_ACK                      4
#define HEADER_FILE                     5
#define HEADER_SKIP                     6
#define HEADER_NAK                      7
#define HEADER_ABORT                    8
#define HEADER_FIN                      9
#define HEADER_RPOS                     10
#define HEADER_DATA                     11
#define HEADER_EOF                      12
#define HEADER_FERR                     13
#define HEADER_CRC                      14
#define HEADER_CHALLENGE                15
#define HEADER_COMPL                    16
#define HEADER_CAN                      17
#define HEADER_FREECNT                  18
#define HEADER_COMMAND                  19
#define HEADER_STDERR                   20

/*
   Zmodem headers in string format if one wants to display them in the
   caller program.

   U8 *z_header[] = {

   "TIMEOUT",
   "ZRQINIT",
   "ZRINIT",
   "ZSINIT",
   "ZACK",
   "ZFILE",
   "ZSKIP",
   "ZNAK",
   "ZABORT",
   "ZFIN",
   "ZRPOS",
   "ZDATA",
   "ZEOF",
   "ZFERR",
   "ZCRC",
   "ZCHALLENGE",
   "ZCOMPL",
   "ZCAN",
   "ZFREECNT",
   "ZCOMMAND",
   "ZSTDERR"
   };
*/

/**************/
/* Frame ends */
/**************/

#define FRAME_TIMEOUT                   0   /* Frame receive routine has */
                                            /* timed out */
#define FRAME_CRCE                      1   /* Frame end indicating end of */
                                            /* transfer*/
#define FRAME_CRCG                      2   /* Frame end inditcating */
                                            /* continuous data flow */
#define FRAME_CRCQ                      3   /* Frame end expecting */
                                            /* an acknowledge */
#define FRAME_CRCW                      4   /* Frame end indicating that */
                                            /* a Zmodem header will follow */
                                            /* instead of next frame */
#define FRAME_ERROR                     5   /* An error has occurred while */
                                            /* receiving a frame */
#define FRAME_CAN                       6   /* Other end has sent a CAN */
                                            /* sequence while we were */
                                            /* trying to receive a frame */

/*
   Frame ends in string format:

   U8 *z_frame_end[] = {

   "TIMEOUT",
   "ZCRCE",
   "ZCRCG",
   "ZCRCQ",
   "ZCRCW",
   "ZERROR",
   "ZCAN"
   };
*/

/*********************************************/
/* PS_Z_RECEIVER_FLAGS parameter's bit masks */
/*********************************************/

#define RZ_FLAG_CANFDX                  0x01 /* Receiver is capable of true */
                                             /* full duplex transfer */
#define RZ_FLAG_CANOVIO                 0x02 /* Receiver can receive data */
                                             /* during disk I/O */
#define RZ_FLAG_CANBRK                  0x04 /* Receiver can send a break */
                                             /* signal */
#define RZ_FLAG_CANCRY                  0x08 /* Receiver can decrypt */
#define RZ_FLAG_CANLZW                  0x10 /* Receiver can uncompress */
#define RZ_FLAG_CANFC32                 0x20 /* Receiver can do 32-bit CRC */
                                             /* checking */
#define RZ_FLAG_ESC_CTRL                0x40 /* Receiver wants control */
                                             /* characters to be escaped */
#define RZ_FLAG_ESC_8TH                 0x80 /* Receiver wants characters */
                                             /* with 8th bit set to be */
                                             /* escaped */

/*******************************************/
/* PS_Z_SENDER_FLAGS parameter's bit masks */
/*******************************************/

#define SZ_FLAG_ESC_CTRL                0x40 /* Sender wants control */
                                             /* characters to be escaped */
#define SZ_FLAG_ESC_8TH                 0x80 /* Sender wants characters */
                                             /* with 8th bit set to be */
                                             /* escaped */

/******************************/
/* Zmodem conversion options. */
/******************************/

/* One of these is passed as 7th argument to r_open_func() and */
/* s_open_func() */

#define Z_CONVERSION_UNDEFINED          0x00
#define Z_CONVERSION_BINARY             0x01
#define Z_CONVERSION_TEXT               0x02
#define Z_CONVERSION_RESUME             0x03

/*****************************/
/* Zmodem management options */
/*****************************/

/* One of these is passed as 8th argument to r_open_func() and */
/* s_open_func() */

#define Z_MANAGEMENT_UNDEFINED          0x00 /* No special management */
                                             /* option given */
#define Z_MANAGEMENT_UPDATE             0x01 /* Already existing files */
                                             /* should be replaced only */
                                             /* if the new file is longer */
                                             /* or newer*/
#define Z_MANAGEMENT_COMPARE            0x02 /* A 32-bit CRC of already */
                                             /* existing files should be */
                                             /* calculated and compared to */
                                             /* that of new file, and */
                                             /* only if they differ the */
                                             /* should proceeed. This */
                                             /* option is not supported by */
                                             /* P.DLL, for following */
                                             /* reasons: */
                                             /* 1 - Implementing would */
                                             /* need at least one more */
                                             /* entry function. That would */
                                             /* make things much more */
                                             /* complex and break the */
                                             /* modularity somewhat. */
                                             /* 2 - Haven't found any */
                                             /* other implementation that */
                                             /* supports this. */
                                             /* 3 - Isn't really important */
                                             /* feature, isn't it? */
#define Z_MANAGEMENT_APPEND             0x03 /* If some files already exist */
                                             /* the new data should be */
                                             /* appended to the old files */
#define Z_MANAGEMENT_REPLACE            0x04 /* Already existing files (if */
                                             /* any) should be replaced */
#define Z_MANAGEMENT_NEWER              0x05 /* Already existing files */
                                             /* should be replaced if the */
                                             /* new file is newer. */
#define Z_MANAGEMENT_DIFFERENT          0x06 /* Already existing files */
                                             /* should be replaced only */
                                             /* if the date or length of the */
                                             /* new and old file differ. */
#define Z_MANAGEMENT_PROTECT            0x07 /* Already existing files */
                                             /* should NOT be replaced */
                                             /* under any circumstances. */
#define Z_MANAGEMENT_RENAME             0x08 /* If file already exists */
                                             /* it should be written with */
                                             /* some other name. */
#define Z_MANAGEMENT_BACKUP             0x09 /* If file already exists  */
                                             /* The existing file should be renamed */

/* Mask for all of the options in previous list */

#define Z_MANAGEMENT_MASK               0x1f

/* This can be masked (ORred) to the management options */

#define Z_MANAGEMENT_MUST_EXIST         0x80

/****************************/
/* Zmodem transport options */
/****************************/

/* NONE OF THESE ARE SUPPORTED BY P.DLL */

#define Z_TRANSPORT_UNDEFINED           0x00 /* Nothing special will */
                                             /* be done */
#define Z_TRANSPORT_LZW                 0x01 /* Data should compressed */
                                             /* with Lempel-Ziv algorithm. */
#define Z_TRANSPORT_CRYPT               0x02 /* Data should encrypted */
#define Z_TRANSPORT_RLE                 0x03 /* Data should compressed with */
                                             /* RLE algorithm. */

/**************************/
/* Error checking methods */
/**************************/

#define CHECKING_CHECKSUM               0x01 /* 1 byte checksum of the */
                                             /* data, not very reliable */
                                             /* method */
#define CHECKING_CRC16                  0x02 /* 2 byte CRC of the data, */
                                             /* pretty reliable */
#define CHECKING_CRC32                  0x03 /* 4 byte CRC of the data, */
                                             /* very reliable */


/***********************/
/* Transfer directions */
/***********************/

#define DIR_RECV                        0x01 /* We receive files */
#define DIR_SEND                        0x02 /* We send files */


/************************/
/* Status message types */
/************************/

#define PS_ERROR                        0x00
/*
   A system error has occurred in P.DLL

   Parameters:

    1 - U32 error_type

      Indicates in which system function the error occurred.
      See P_ERROR_* defines above.

    2 - U32 error_code

      The error code of failed system function. In case of TCP/IP API
      error the value returned by psock_errno(), otherwise the return
      value of the system function itself.

    3 - U32 module

      ID of the module where the error occurred, this is meant
      for possible bug reports.

    4 - U32 line_number

      Line number of the module where the error occurred. Also meant
      only for bug reports.

    5 - U32 opt_arg

      An optional argument depending on error_type. See P_ERROR_* defines.

   p_transfer() will return soon after this with a return value of 1.
*/

#define PS_CARRIER_LOST                 0x01
/*
   Indicates that carrier signal has been lost. p_transfer() will return
   very soon after this with a return value of 1.

   No parameters.
*/

#define PS_TIMEOUT                      0x02
/*
   Indicates that we have timed out abnormally long

   Parameters:

    1 - U32 secs

      How many seconds did we time out. Note that is only
      an approximation, might be few seconds off.

      Could be also zero (0) indicating that the timeout
      period was not specified.
*/

#define PS_TRANSFER_DONE                0x03
/*
   Indicates that we are successfully done with transferring, p_transfer()
   will exit very soon.

   No parameters.
*/

#define PS_PROGRESS                     0x04
/*
   Indicates the progress of the file transfer.

   Parameters:

    1 - U32 offset

      The offset of the file currently being transferred.
*/

#define PS_CANNOT_SEND_BLOCK            0x05
/*
   We have tried to send one block for several times and the retry
   count has been exceeded. p_transfer() will return soon after this with
   return value of 1.

   No parameters.
*/

#define PS_CHECKING_METHOD              0x06
/*
   We have determined the error checking method to be used.

   Parameters:

    1 - U32 checking_method

      The method to be used, see CHECKING_* defines above for values.

   Note: This will be called several times during a transfer of
         multiple files, even if there is no change in the method.
*/

#define PS_INVALID_FILE_INFO            0x07
/*
   We have received an invalid file information from the other end.
   The transfer cannot proceed, p_transfer() will return soon with
   a return value of 1.

   No parameters.
*/

#define PS_NON_STD_FILE_INFO            0x08
/*
   We have received a non-standard file information (one missing some
   null-terminations). The transfer will proceed.

   No parameters.
*/

#define PS_XY_FALLBACK_TO_CHECKSUM      0x09
/*
   We have failed to initialize the file transfer for CRC-16 checking,
   from now on we will try to initialize it for checksum checking.

   No parameters.
*/

#define PS_CHECK_FAILED                 0x0a
/*
   We have found an error in a transferred block of data. If we are
   transferring with Ymodem-g, we will get a PS_G_ABORTED status
   soon and the transfer will be aborted. Otherwise we will retry to
   transfer the failed block of data.

   Parameters

    1 - U32 checking_method

      The error checking method we used. For possible values see
      CHECKING_* defines above.

*/

#define PS_REMOTE_ABORTED               0x0b
/*
   The other end has aborted the file transfer.

   No parameters.
*/

#define PS_G_ABORTED                    0x0c
/*
   Some sort of error has occurred in a Ymodem-g transfer and due to a fact
   that Ymodem-g does not provide any kind of error recovery the transfer
   will be aborted.

   No parameters.
*/

#define PS_XYG_NAK                      0x0d
/*
   We have got a NAK response to a block we sent to the remote.

   Parameters:

    1 - U32 offset

      The offset for which we got the NAK. If all bits are set
      (offset == 4294967295), we got the NAK on the file information
      block.
*/

#define PS_XYG_BLK_NUM_MISMATCH         0x0e
/*
   We have found inconsistency in the block numbering. The block will be
   retried or if the protocol is Ymodem-g the transfer will be aborted.

   Parameters:

    1 - U8 rcvd_blk_num1
    2 - U8 rcvd_blk_num2

      The block numbers we got from the remote.

    3 - U8 expected_blk_num1
    4 - U8 expected_blk_num2

      The block numbers we expected from the remote.
*/

#define PS_Z_HEADER                     0x0f
/*
   We have received a Zmodem header.

   Parameters:

    1 - U32 header_type

      The type of received header. For possible values see Z* defines
      above.

    2 - U32 header_value

      The value of received header.
*/

#define PS_Z_UNEXPECTED_HEADER          0x10
/*
   We have received an unexpected Zmodem header.

   Parameters:

    < See PS_Z_HEADER above >
*/

#define PS_Z_FRAME_END                  0x11
/*
   We have received a Zmodem frame and this is its end type.

   Parameters:

    1 - U32 frame_end

      Received frame's end type. For possible values see FRAME_*
      defines below.
*/

#define PS_Z_INVALID_FRAME_END          0x12
/*
   We have receive a Zmodem frame with an invalid frame end.

   Parameters:

    < See PS_Z_FRAME_END above >

*/

#define PS_Z_PHONY_ZEOF                 0x13
/*
   A phony ZEOF header has been received.

   No parameters.
*/

#define PS_Z_RETRY_CNT_EXCEEDED         0x14
/*
   We have got 100 consecutive non-ZDATA Zmodem headers. The transfer
   will be aborted.

   No parameters.
*/

#define PS_Z_DATA_FROM_INVALID_POS      0x15
/*
   We have received data from an invalid offset.

   Parameters:

    1 - U32 offset

      The invalid offset we got data from.

*/

#define PS_Z_COMMAND                    0x16
/*
   We have received a ZCOMMAND.

   Parameters:

    1 - U8 *command;

      This is a command meant to be executed by command shell, but for
      security reasons it might be unwise to allow this. Usually used
      only to execute shell command "echo" to announce something.
*/

#define PS_Z_CTRL_CHAR_IGNORED          0x17
/*
   We have received an control character though the remote should
   escape them.

   Parameters:

    1 - U8 ctrl_ch;

      The control character we received.
*/

#define PS_Z_INVALID_ZDLE_SEQUENCE      0x18
/*
   We have received an invalid ZDLE escape sequence, we will retry.

   No parameters.
*/

#define PS_Z_CHECK_FAILED_FOR_HEADER    0x19
/*
   We have found a CRC error in a received Zmodem header.

   Parameters:

    1 - U32 checking_method

      The error checking method we are using, for possible values
      see CHECKING:_* defines above.
*/

#define PS_Z_INVALID_HEX_HEADER         0x1a
/*
   We have received an invalid Zmodem hexadecimal header. We will retry.

   No parameters.
*/

#define PS_Z_SUBPACKET_TOO_LONG         0x1b
/*
   We have received too long Zmodem subpacket.

   Parameters:

    1 - U32 max_length

      The maximum length of Zmodem subpackets.
*/

#define PS_Z_CRASH_RECOVERY             0x1c
/*
   We are recovering from a crashed file transfer.

   Parameters:

    1 - U32 offset

      The offset from where on we are recovering.
*/

#define PS_Z_RECEIVER_FLAGS             0x1d
/*
   The have found out what the receiver is capable of.

   Parameters:

    1 - U32 receiver_flags

      The receiver capabilities are saved in this variable as bit
      values, for possible values see RFLAG_* defines below.
*/

#define PS_Z_RECEIVER_WINDOW_SIZE       0x1e
/*
   We have found out that the receiver wants a transmit window of specified
   bytes (in parameter #1) to be used. However, if a window size was
   specified in the parameter structure passed to p_transfer() it will
   be used instead of what the receiver proposes.

   Parameters:

    1 - U32 remote_window_size

      The proposed transfer window size.
*/


#define PS_Z_SENDER_FLAGS               0x1f
/*
   The have found out what the receiver is capable of.

   Parameters:

    1 - U32 receiver_flags

      The receiver capabilities are saved in this variable as bit
      values, for possible values see SFLAG_* defines below.
*/

#define PS_SERVER_WAITING               0x20

/*
   This status message is send when we are acting as a server for a file
   transfer (named pipes or stream sockets) and waiting for a client to make
   a connection. This is always called at least once.

   When the status_func() returns a zero value the DLL will recheck for any
   inbound connections. Thus, time-outting can be implemented by returning
   a non-zero value when waited long enough.

   Parameters:

    1 - U32 count

      The number of PS_SERVER_WAITING status message.
*/

#define PS_FILE_SKIPPED                 0x21

/*
   This status is posted when we have sent a file information to the receiver
   and as response got a skip request (indicating that the receiver already
   has the file, or had some problems with creating it.)

   No parameters.
*/

#define PS_Z_SERIAL_NUM                 0x22

/*
   This status is posted when we are sending files with Zmodem and have
   received the serial number of the receiver. The receiver sends the
   serial number as a response to ZSINIT header that's send by us to tell
   that we wish to have control characters escaped or our attention sequence,
   or when we simply want to query the serial number (See
   CFG_QUERY_SERIAL_NUM configuration flag).

   Parameters:

    1 - U32 serial_num

      A 32-bit serial number we've received.
*/

#define PS_PACKET_LENGTH                0x23



/***************************/
/* P_CFG structure version */
/***************************/

#define P_INTERFACE_VERSION             1 /* This is the version number of */
                                          /* P.DLL interface we use */


/****************************/
/* Configuration attributes */
/****************************/

#define CFG_ALTERNATIVE_CHECKING        0x00000001
/*
   If this attribute is set, an alternative checking method of
   corresponding protocol will be used.

   Protocol        Default         Alternative
   --------        -------         -----------
   Zmodem          CRC-32          CRC-16
   Ymodem-g        CRC-16          N/A
   Ymodem          CRC-16          Checksum
   Xmodem          Checksum        CRC-16

   Note: with X and Y transfers the receiver decides the error checking
   method to be used.
*/

#define CFG_1K_BLOCKS                   0x00000002
/*
   If set and sending files with X, Y or G protocol, a 1024 byte blocks
   will be used instead of default 128 byte.

   Has no effect to Z transfers.
*/

#define CFG_SEND_RZ_CR                  0x00000004
/*
   If set, we will send "rz\r" string to the remote in the initialization
   phase of Zmodem sending. The "rz\r" string is used to trigger UN*X
   systems to start their Zmodem receive programs.

   Has an effect to Zmodem sending only.
*/
#define CFG_SHARED_DEVICE               0x00000008
/*
   If set, we will open the communication device in DENYNONE mode.
   Needed, for example, if you are passing a path, instead of handle, to
   an already open communication device.

   Has an effect only if dev_handle is zero.
*/

#define CFG_WATCH_CARRIER               0x00000010
/*
   If set and the communication device is an asynchronous one, we will
   monitor the carrier signal and abort the transfer if it drops.

   Has no effect if the device is a named pipe.
*/

#define CFG_DEV_SERVER                  0x00000020
/*
   If set and dev_handle is zero, we will act as a server for named pipe
   and socket connections.

   In case of named pipe, a new pipe will be created to which other
   programs can connect to.

   In case of socket, we will listen a socket for incoming connections.

   Has no effect if dev_handle is non-zero or the dev_path refers to
   an asynchronous device.
*/

#define CFG_DEV_TELNET                  0x00000040

#define CFG_ESC_MINIMAL                 0x00000080
/*
   If set, we will try to get more throughput by minimizing the escaping
   used for Zmodem transfers.

   Affects only to Zmodem transfers.
*/

#define CFG_ESC_CTRL                    0x00000100
/*
   If set, we will escape all control characters (ASCII 0-31) with
   Zmodem transfers. Needed if the connection can not pass through
   some of the control characters.

   Affects only to Zmodem transfers.
*/

#define CFG_QUERY_SERIAL_NUM            0x00000200
/*
   If set, we will query the remote's serial number and tell it
   to the calling application with PS_Z_SERIAL_NUM status message.

   Affects only to Zmodem sending.
*/

#define CFG_DEV_TELNET_U_BINARY         0x000000400
#define CFG_DEV_TELNET_ME_BINARY        0x000000800
#define CFG_DEV_TELNET_IAC_NOT_ESCAPED  0x000001000

#define CFG_FILE_MOVE                   0x000002000
/*
   If set, a file transfer is only SUCCESSFUL if the file was
   actually sent across the wire.  In other words, a SKIPPED file
   will not be considered a successfully transfered file.
*/

#define CFG_ESC_TABLE              0x000002000
/*
   If set, we will use a table specified by control_prefix_table
   to determine which control characters should be escaped

   Affects only Zmodem transfers.
*/

#define CFG_ESC_8TH                0x000004000
/*
   If set, we will escape all characters with the 8th bit set.
   Needed on 7-bit connections.

   Affects only Zmodem transfers.
*/

#pragma pack(1)                     /* Use 1 byte alignment */
                                    /* in the structure */

typedef struct _P_CFG {

  U32 version;                      /* This structure's version, use */
                                    /* the P_VERSION constant */

  U32 attr;                         /* Configuration attributes, see */
                                    /* CFG_* defines above. */

  U32 transfer_direction;           /* Transfer direction, see DIR_* */
                                    /* defines above. */
  U32 protocol_type;                /* Protocol to use, see PROTOCOL_* */
                                    /* defines above. */

  U32 serial_num;                   /* Our serial number that will be sent */
                                    /* to the remote when requested. 0 for */
                                    /* no serial number. For more */
                                    /* information, see PS_Z_SERIAL_NUM */
                                    /* define. */
  U8 *attn_seq;                     /* This is a pointer to a string that */
                                    /* will be sent when we are receving */
                                    /* files and have detected an error or */
                                    /* have something else to report to the */
                                    /* sender. Usually not needed can be set */
                                    /* point to NULL. */
                                    /* In attention sequence following */
                                    /* characters have a special meaning: */
                                    /* ASCII 221 -- Break signal (275 ms) */
                                    /* ASCII 222 -- One second pause */
                                    /* Maximum length of the attention */
                                    /* sequence is 31 characters + */
                                    /* terminating NULL */
  U32 dev_type;                     /* Communication device type, for values */
                                    /* see DEV_TYPE_* defines */
  U8 *dev_path;                     /* Path to the communication device, */
                                    /* used only if dev_handle is zero. */
  U8 *socket_host;                  /* Internet address of the host to be */
                                    /* connected to */
  U16 socket_port;                  /* Stream socket port to be used. */

  intptr_t dev_handle;              /* Handle to an already open */
                                    /* communication device */

  U32 inbuf_size;                   /* Size of communication input buffer */
  U32 outbuf_size;                  /* Size of communication output buffer */

  U32 blk_size;                     /* Block size if protocol is X, Y or G. */
                                    /* If Z, this is the window size to be */
                                    /* used. */

/*
   Here are the callback functions that P.DLL calls to inform the
   parent program about the progress of the file transfer, to get more
   data to be sent and to give away the received data.

   Each of the callback functions has its own specific parameters, but
   the meaning of return values is common.

   A return value of zero (0) is used to indicate that the processing
   should continue normally.

   A non-zero return value is used to indicate that the calling
   program wants to abort the file transfer for some reason (user
   aborted, an unrecoverable error has occurred, etc).  In case of
   non-zero return value the p_transfer() entry will return very soon,
   however some calls to the callback/ functions may still come, so if
   the error condition prevents the proper callback function handling,
   it should be taken in consideration when writing the callback
   functions.
*/

  U32 (CKDEVAPI * status_func)(U32, status_args *);
/*
   This function is called to inform the calling program about
   progress of file transfer.

   Parameters:

   1 - U32 status_type

        The type of posted status message, for possible values
        see PS_* defines above.

   n - A number of parameter will follow depending on the

        status_type. For each status_type's parameters see
        the comments in PS_* defines above.

	if no parameter is passed then use NULL pointer
	if single parameter then cast it to pointer
	if more then single parameter then use pointer to status_args
	  structure which contains up to 5 parameters
*/

  U32 (CKDEVAPI * r_open_func)(U8 **,
                                U32,
                                U32,
                                U32,
                                U32,
                                U32,
                                U8,
                                U8,
                                U8,
                                U32 *);

/*
   This function is called before we start receiving a file.

   Parameters:

   1 - U8 **path

        This is a pointer to a character string which has been
        dynamically allocated with OS/2's DosAllocMem() function.

        If we are receiving files with a protocol other than Xmodem,
        this string will contain the file path information that the
        sender has provided to us. The variable can be freely changed
        in the function and if the new value is longer than the old
        one the variable must be freed (with DosFreeMem) first and
        then reallocated (with DosAllocMem).  File can also be skipped
        (works only with Zmodem transfers, others will abort) by freeing
        the variable and assigning a NULL value to it.

        In case of Xmodem, this parameter will be a pointer to a NULL
        string and the parent program will have to allocate memory (with
        DosAllocMem) for it and save the name of the file to be received
        in it.

        If path contains directory or drive information, backslashes are
        used as separators. For example: C:\FISH\FOO.BAR

   2 - U32 length

        The length of the file to be received. In case of Zmodem this
        can be an estimate, with Ymodem and Ymodem-g it will be accurate, and
        with Xmodem (which doesn't provide this information) it will be -1.

   3 - U32 date

        The date of the file to be received in seconds since 1st of January
        1970, 00:00. In case of Xmodem this will have a value of -1. Might be
        -1 also if a non-standard file information has been received.

   4 - U32 mode

        This is a UNIX style mode information for the file. Zero (0) if
        information is not available.

   5 - U32 f_left

        This is the number files left to transfer. Zero (0) if information
        is not available.

   6 - U32 b_left

        This is the number bytes left to transfer. Zero (0) if information
        is not available.

   7 - U32 zconv

        Zmodem conversion option. One of the Z_CONVERSION_* defines.

   8 - U32 zmanag

        Zmodem management option. Constructed of the Z_MANAGEMENT_* defines.

   9 - U32 ztrans

        Zmodem transport option. One of the Z_TRANSPORT_* defines.

   10 - U32 *offset

        This value will have a NULL value if the protocol is something
        else than Zmodem.

        In case of Zmodem, this is a pointer to a variable telling
        the offset where at the sender should start sending the file.
*/

  U32 (CKDEVAPI * s_open_func)(U8 **,
                                U32 *,
                                U32 *,
                                U32 *,
                                U32 *,
                                U32 *,
                                U8 *,
                                U8 *,
                                U8 *);

/*
   This function is called before we start sending a file.

   Parameters:

   1 - U8 **path

        This is a pointer to a character string which has initially a
        value of NULL. The calling program must allocate memory (with
        DosAllocMem) for the string and save the path to the file to be
        sent in it. Either backslashes ('\') or slashes ('/') can be used
        as separators for possible directory information.

        If there are no more files to be transferred the string value
        should be left as NULL.

   2 - U32 *length

        This is a pointer to a variable where function should save
        the length of the file to be sent. With Zmodem an estimate
        will do, and Xmodem does not need this at all.

   3 - U32 *date

        To the variable pointed by this pointer, the function should
        save the last modification time of the file to be sent, in
        seconds since 00:00, 1st of January 1970.

        This is not required for Xmodem transfers.

   4 - U32 *mode

        This is UNIX style mode information for the file. Zero (0) if
        information is not available.

   5 - U32 f_left

        This is the number files left to transfer. Zero (0) if information
        is not available.

   6 - U32 b_left

        This is the number bytes left to transfer. Zero (0) if information
        is not available.

   7 - U32 zconv

        Zmodem conversion option. One of the Z_CONVERSION_* defines.

   8 - U32 zmanag

        Zmodem management option. Constructed of the Z_MANAGEMENT_* defines.

   9 - U32 ztrans

        Zmodem transport option. One of the Z_TRANSPORT_* defines.

*/

  U32 (CKDEVAPI * close_func)(U8 **,
                               U32,
                               U32,
                               U32,
                               BOOLEAN,
                               U32);

/*
   This function will be called when we are done with transferring a file
   (successful or not).

   Parameters:

   1 - U8 **path

        This is a pointer to character string containing path to the
        file we have been transferring. This function should free this
        DosAllocMem allocated variable and place a NULL value to it.

   2 - U32 length

        The file length information the sender sent us in the beginning of
        the file transfer. With Zmodem transfer might have been just an
        estimate and might not equal to offset (parameter 6).

   3 - U32 date

        The file date informating provided to us in the beginning of the
        transfer. The user function should set the date of the actual file
        to this. Value will be -1 if the information has not been provided
        by the sender.

   4 - U32 retransmits

        Number of times data was retransmitted during the file transfer.

   5 - U32 successful

        Non-zero if the file transfer was successful.

   6 - U32 offset

        This variable contains the offset where the transfer ended
        (successful or not). Note that is does not necessary equal to
        the bytes transferred, we might have recovered from a crashed
        transfer.

*/
  U32 (CKDEVAPI * seek_func)(U32);
/*
   This function is called when we are starting to send a file with Zmodem
   protocol and the remote indicates that it already has a part of that
   file and it would like to recover from it by not starting the transfer
   from the beginning.

   Parameters:

   1 - U32 offset

        This is the position the receiver would like to start receiving
        from, rather than from the beginning of the file.
*/

  U32 (CKDEVAPI * read_func)(U8 *,
                              U32,
                              U32 *);
  /*
   This function is called when we need more data to send to the remote.

   Parameters:

   1 - U8 *buf

        Pointer to a character buffer where the function should place
        the data.

   2 - U32 bytes_wanted

        Bytes of data we want to get.

   3 - U32 *bytes_got

        To this variable the function should place the number of bytes
        it got received. If this is less than bytes_wanted we expect
        that there are no more data in the file and read_func() will
        not be called for this file anymore (unless there is a
        transmission error).
*/

  U32 (CKDEVAPI * write_func)(U8 *,
                               U32);
/*
   This function is called when we have received data to be written to the
   file.

   Parameters:

   1 - U8 *buf

        Pointer to character buffer where the data to be written is.

   2 - U32 bytes_to_write

        The number of bytes to write from buf.
*/

  U32 (CKDEVAPI * exe_in_func)(U8 *,
                              U32,
                              U32 *);
  /*
   This function is called when we need more data received from the remote.

   Parameters:

   1 - U8 *buf

        Pointer to a character buffer where the function should place
        the data.

   2 - U32 bytes_wanted

        Bytes of data we want to get.

   3 - U32 *bytes_got

        To this variable the function should place the number of bytes
        it got received. If this is less than bytes_wanted we expect
        that there are no more data in the file and read_func() will
        not be called for this file anymore (unless there is a
        transmission error).
*/

  U32 (CKDEVAPI * exe_out_func)(U8 *,
                               U32, U32 *);
/*
   This function is called when we have data to send to the host.

   Parameters:

   1 - U8 *buf

        Pointer to character buffer where the data to be written is.

   2 - U32 bytes_to_write

        The number of bytes to write from buf.

   3 - U32 * bytes_written

        Returns number of bytes actually written.
*/
  U32 (CKDEVAPI * exe_break_func)(U8);

/*
   This function is called to manipulate the Break Signal

   Parameters:

   1 - U8 toggle

       0 for Off; 1 for On.
*/

  U32 (CKDEVAPI * exe_available_func)(U32 *);
/*
  This function is called to check the number of available bytes

  Parameters:

  1 - U32 * available_bytes

       Returns the number of available bytes

*/

    U16 * control_prefix_table ;

    /* A pointer to a 256 element array in which each cell is either */
    /* 0 or 1 depending on whether that character position is to be  */
    /* prefixed.                                                     */

    U32 (CKDEVAPI * exe_pushback_func)(U8 *, U32);
    /*
    This function is called to push back the unused bytes left over
    at the end of the transfer.

    Parameters:

    1 - U8 * bytes to restore
    2 - U32  number of bytes

    Returns 0 for success

    */

} P_CFG;

#pragma pack()                  /* Back to default alignment */

#endif /* _P_H_ */
