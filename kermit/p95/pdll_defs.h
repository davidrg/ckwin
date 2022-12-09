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

/* 
 * Definitions for X, Y and Zmodem protocols. Based on public domain
 * ZMODEM.H by Chuck Forsberg. Here's excerpt from ZMODEM.DOC by Chuck
 * Forsberg: 
 *
 * ZMODEM was developed for the public domain under a Telenet contract.  The
 * ZMODEM protocol descriptions and the Unix rz/sz program source code are
 * public domain.  No licensing, trademark, or copyright restrictions apply
 * to the use of the protocol, the Unix rz/sz source code and the ZMODEM
 * name.
 */

#define SOH	0x01
#define STX	0x02
#define EOT	0x04
#define ACK	0x06
#define XON	0x11
#define XOFF	0x13
#define NAK	0x15
#define CAN	0x18

/* Abort reasons */
#define A_REMOTE		1
#define A_LOCAL			2
#define A_CARRIER_LOST		3
#define A_MISC 			4

/* Zmodem */
#define GOTADD		(257 - ZCRCE)
#define GOTCRCE 	(ZCRCE + GOTADD)	/* ZDLE-ZCRCE received */
#define GOTCRCG 	(ZCRCG + GOTADD)	/* ZDLE-ZCRCG received */
#define GOTCRCQ         (ZCRCQ + GOTADD)	/* ZDLE-ZCRCQ received */
#define GOTCRCW 	(ZCRCW + GOTADD)	/* ZDLE-ZCRCW received */
#define GOTERROR	(ZCRCW + GOTADD + 1)
#define GOTCAN		(ZCRCW + GOTADD + 2)

#define ZPAD		'*' /* 052 Padding character begins frames */
#define ZDLE		030 /* Ctrl-X Zmodem escape - `ala BISYNC DLE */
#define ZDLEE		(ZDLE^0100) /* Escaped ZDLE as transmitted */
#define ZBIN		'A' /* Binary frame indicator */
#define ZHEX		'B' /* HEX frame indicator */
#define ZBIN32		'C' /* Binary frame with 32 bit FCS */

/* Macro used to determine whether byte indicates Zmodem header */
#define isheader(c)	(c <= 19)

/* Macro used to convert header value to index to z_header[] table */
/* described in p.h */
#define outheader(c)	(c > 19 ? 0 : (c + 1))

/* Macro used to convert frame end value to index to z_frame_end[] */
/* table described in p.h */
#define outframe(c)	(c - 256)

/* Zmodem header types */
#define ZRQINIT		0 /* Request receive init */
#define ZRINIT		1 /* Receive init */
#define ZSINIT		2 /* Send init sequence (optional) */
#define ZACK		3 /* ACK to above */
#define ZFILE		4 /* File name from sender */
#define ZSKIP		5 /* To sender: skip this file */
#define ZNAK		6 /* Last packet was garbled */
#define ZABORT		7 /* Abort batch transfers */
#define ZFIN		8 /* Finish session */
#define ZRPOS		9 /* Resume data trans at this position */
#define ZDATA		10 /* Data packet(s) follow */
#define ZEOF		11 /* End of file */
#define ZFERR		12 /* Fatal Read or Write error Detected */
#define ZCRC		13 /* Request for file CRC and response */
#define ZCHALLENGE	14 /* Receiver's Challenge */
#define ZCOMPL		15 /* Request is complete */
#define ZCAN		16 /* Other end canned session with CAN*5 */
#define ZFREECNT	17 /* Request for free bytes on filesystem */
#define ZCOMMAND	18 /* Command from sending program */
#define ZSTDERR		19 /* Output to standard error, data follows */

/* ZDLE sequences */
#define ZCRCE		'h' /* CRC next, frame ends, header packet follows */
#define ZCRCG		'i' /* CRC next, frame continues nonstop */
#define ZCRCQ		'j' /* CRC next, frame continues, ZACK expected */
#define ZCRCW		'k' /* CRC next, ZACK expected, end of frame */
#define ZRUB0		'l' /* Translate to rubout 0177 */
#define ZRUB1		'm' /* Translate to rubout 0377 */

/* Byte positions in header */
#define ZF0		3 /* First flags byte */
#define ZF1		2
#define ZF2		1
#define ZF3		0
#define ZP0		0 /* Low order 8 bits of position */
#define ZP1		1
#define ZP2		2
#define ZP3		3 /* High order 8 bits of file position */

/* ZRINIT bit masks */
#define CANFDX		1 /* Rx can send and receive true FDX */
#define CANOVIO		2 /* Rx can receive data during disk I/O */
#define CANBRK		4 /* Rx can send a break signal */
#define CANCRY		8 /* Receiver can decrypt */
#define CANLZW		16 /* Receiver can uncompress */
#define CANFC32		32 /* Receiver can use 32 bit Frame Check */

/* ZFILE bit masks - Conversion options one of these in ZF0 */
#define ZCBIN		1 /* Binary transfer - inhibit conversion */
#define ZCNL		2 /* Convert NL to local end of line convention */
#define ZCRESUM		3 /* Resume interrupted file transfer */

/* Management include options, one of these ored in ZF1 */
#define ZMSKNOLOC	128 /* Skip file if not present at rx */

/* Management options, one of these ored in ZF1 */
#define ZMMASK		31 /* Mask for the choices below */
#define ZMNEWL		1 /* Transfer if source newer or longer */
#define ZMCRC		2 /* Transfer if different file CRC or length */
#define ZMAPND		3 /* Append contents to existing file (if any) */
#define ZMCLOB		4 /* Replace existing file */
#define ZMNEW		5 /* Transfer if source newer */
	/* Number 5 is alive ... */
#define ZMDIFF		6 /* Transfer if dates or lengths different */
#define ZMPROT		7 /* Protect destination file */
#define ZMCHNG		8 /* Change filename if destination exists */
/* Transport options, one of these in ZF2 */
#define ZTLZW		1 /* Lempel-Ziv compression */
#define ZTRLE		3 /* Run Length encoding */

/* ZSINIT bit masks */
#define TESCCTL		0100 /* Transmitter expects ctl chars to be escaped */
#define TESC8		0200 /* Transmitter expects 8th bit to be escaped */
