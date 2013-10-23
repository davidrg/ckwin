#ifndef kdefs_h_included
#define kdefs_h_included
/*
  Indices represent the global variables shared between KUI and Kermit:

    Bits 28-31 = Type modifier flags (unsigned, pointer, array).
    Bits 24-27 = Basic type (short, int, long, char).
    Bits 16-23 = Functional group number, 0-255.
    Bits 00-15 = Index within functional group, 0-32767.

   Note: some items are listed in multiple groups.
   For example, the session log is in the Log group and the Terminal group.
*/

/* Type modifier Flags (high-order nibble) */

#define CK_F_U        (0x01<<28)         /* unsigned bit    */
#define CK_F_P        (0x02<<28)         /* pointer bit     */
#define CK_F_A        (0x04<<28)         /* array bit       */

/* Basic Types (second nibble) */

#define CK_T_SHORT    (0x01<<24)          /* short          */
#define CK_T_INT      (0x02<<24)          /* int            */
#define CK_T_LONG     (0x03<<24)          /* long           */
#define CK_T_CHAR     (0x04<<24)          /* char           */

/* Shorthands for combinations */

#define CK_USHORT     (CK_F_U|CK_T_SHORT) /* unsigned short */
#define CK_UINT       (CK_F_U|CK_T_INT)   /* unsigned int   */
#define CK_ULONG      (CK_F_U|CK_T_LONG)  /* unsigned long  */
#define CK_UCHAR      (CK_F_U|CK_T_CHAR)  /* unsigned char  */
#define CK_ACHAR      (CK_F_A|CK_T_CHAR)  /* array of char  */

/* Group definitions, 0-255 */

#define CK_G_XFR     (0x01<<16)           /* File transfer settings */
#define CK_G_LOG     (0x02<<16)           /* Log files */
#define CK_G_COM     (0x03<<16)           /* General communications settings */
#define CK_G_SER     (0x04<<16)           /* General communications settings */
#define CK_G_MOD     (0x05<<16)           /* Modem parameters */
#define CK_G_DIA     (0x06<<16)           /* Dialing parameters */
#define CK_G_NET     (0x07<<16)           /* Network parameters */
#define CK_G_TER     (0x09<<16)           /* Terminal emulation settings */
#define CK_G_PRO     (0x09<<16)           /* Protocol settings */
#define CK_G_SCR     (0x0a<<16)           /* Script programming parameters */
#define CK_G_KEY     (0x0b<<16)           /* Key assignments */
#define CK_G_MOU     (0x0c<<16)           /* Mouse assignments */
#define CK_G_CMD     (0x0d<<16)           /* Command screen parameters */

/* Indices - Terminal Group */

#define TERM_AB_MSG           (CK_ACHAR|CK_G_TER|0x01)  /* answerback msg */
#define TERM_ANSWER           (CK_T_INT|CK_G_TER|0x02)  /* tt_answer */
#define TERM_APC              (CK_T_INT|CK_G_TER|0x03)  /* apcstatus */
#define TERM_ARROW            (CK_T_INT|CK_G_TER|0x04)  /* tt_arrow */
#define TERM_AUTODL           (CK_T_INT|CK_G_TER|0x05)  /* autodl */
#define TERM_BELL             (CK_T_INT|CK_G_TER|0x06)  /* tt_bell */
#define TERM_BYTESIZE         (CK_T_INT|CK_G_TER|0x07)  /* cmask */
#define TERM_CHAR_LOCAL       (CK_T_INT|CK_G_TER|0x08)  /* tcsl */
#define TERM_CHAR_REMOTE      (CK_T_INT|CK_G_TER|0x09)  /* tcsr */
#define TERM_CHAR_G0          (CK_T_INT|CK_G_TER|0x0a)  /* (see shotcs()) */
#define TERM_CHAR_G1          (CK_T_INT|CK_G_TER|0x0b)  /* (see shotcs()) */
#define TERM_CHAR_G2          (CK_T_INT|CK_G_TER|0x0c)  /* (see shotcs()) */
#define TERM_CHAR_G3          (CK_T_INT|CK_G_TER|0x0d)  /* (see shotcs()) */
#define TERM_CHAR_GL          (CK_T_INT|CK_G_TER|0x0e)  /* (see shotcs()) */
#define TERM_CHAR_GR          (CK_T_INT|CK_G_TER|0x0f)  /* (see shotcs()) */
#define TERM_CODEPAGE         (CK_T_INT|CK_G_TER|0x10)  /* tt_codepage */
#define TERM_COLOR_BORDER     (CK_UCHAR|CK_G_TER|0x11)  /* colorborder */
#define TERM_COLOR_CURSOR     (CK_UCHAR|CK_G_TER|0x12)  /* (not used yet) */
#define TERM_COLOR_HELP       (CK_UCHAR|CK_G_TER|0x13)  /* colorhelp */
#define TERM_COLOR_SELECTION  (CK_UCHAR|CK_G_TER|0x14)  /* colorselect */
#define TERM_COLOR_STATUS     (CK_UCHAR|CK_G_TER|0x15)  /* colorstatus */
#define TERM_COLOR_TERM       (CK_UCHAR|CK_G_TER|0x16)  /* colornormal */
#define TERM_COLOR_UNDERLINE  (CK_UCHAR|CK_G_TER|0x17)  /* colorunderline */
#define TERM_CPLIST           (CK_ACHAR|CK_G_TER|0x18)  /* avail codepages */
#define TERM_CRD              (CK_T_INT|CK_G_TER|0x19)  /* tt_crd */
#define TERM_CTSTMO           (CK_T_INT|CK_G_TER|0x1a)  /* tt_ctstmo */
#define TERM_CURSOR           (CK_T_INT|CK_G_TER|0x1b)  /* tt_cursor */
#define TERM_DEBUG            (CK_T_INT|CK_G_TER|0x1c)  /* debses */
#define TERM_ECHO             (CK_T_INT|CK_G_TER|0x1d)  /* duplex */
#define TERM_ESCAPE           (CK_T_INT|CK_G_TER|0x1e)  /* escape */
#define TERM_FONT             (CK_T_INT|CK_G_TER|0x1f)  /* tt_font */
#define TERM_HEIGHT           (CK_T_INT|CK_G_TER|0x20)  /* tt_cols */
#define TERM_ID               (CK_T_INT|CK_G_TER|0x21)  /* tt_info[].x_id */
#define TERM_KEYPAD           (CK_T_INT|CK_G_TER|0x22)  /* tt_keypad */
#define TERM_NAME             (CK_T_INT|CK_G_TER|0x23)  /* tt_info[].x_name */
#define TERM_NLM              (CK_T_INT|CK_G_TER|0x24)  /* tnlm */
#define TERM_PACING           (CK_UCHAR|CK_G_TER|0x25)  /* tt_pacing */
#define TERM_ROLL             (CK_T_INT|CK_G_TER|0x26)  /* tt_roll */
#define TERM_SCRSIZE          (CK_T_INT|CK_G_TER|0x27)  /* tt_scrsize */
#define TERM_SESFIL           (CK_ACHAR|CK_G_TER|0x28)  /* sesfil */
#define TERM_SESLOG           (CK_T_INT|CK_G_TER|0x29)  /* seslog */
#define TERM_SOSI             (CK_T_INT|CK_G_TER|0x2a)  /* sosi */
#define TERM_TYPE             (CK_T_INT|CK_G_TER|0x2b)  /* tt_type */
#define TERM_UPD_FREQ         (CK_T_INT|CK_G_TER|0x2c)  /* tt_update */
#define TERM_UPD_MODE         (CK_T_INT|CK_G_TER|0x2d)  /* tt_updmode */
#define TERM_VIDEO_CHANGE     (CK_T_INT|CK_G_TER|0x2e)  /* tt_modechg */
#define TERM_WIDTH            (CK_T_INT|CK_G_TER|0x2f)  /* tt_rows */
#define TERM_WRAP             (CK_T_INT|CK_G_TER|0x30)  /* tt_wrap */
#define TERM_TYPE_MODE        (CK_T_INT|CK_G_TER|0x31)  /* tt_type_mode */
#define TERM_FONT_SIZE        (CK_T_INT|CK_G_TER|0x32)  /* tt_font_size */
#define TERM_COLOR_ITALIC     (CK_UCHAR|CK_G_TER|0x33)  /* coloritalic */

/* Indices - Command Group */

#define CMD_BYTESIZE          (CK_T_INT|CK_G_CMD|0x01)  /* cmdmsk */
#define CMD_HEIGHT            (CK_T_INT|CK_G_CMD|0x02)  /* cmd_rows */
#define CMD_QUOTING           (CK_T_INT|CK_G_CMD|0x03)  /* cmdgquo() */
#define CMD_RECALL_BUFSIZ     (CK_T_INT|CK_G_CMD|0x04)  /* cm_recall */
#define CMD_RETRY             (CK_T_INT|CK_G_CMD|0x05)  /* cm_retry */
#define CMD_WIDTH             (CK_T_INT|CK_G_CMD|0x06)  /* cmd_cols */

/* Indices - Mouse Group */

#define MOU_ACTIVE            (CK_T_INT|CK_G_MOU|0x01)  /* tt_mouse */
#define MOU_BUTTON            (CK_T_INT|CK_G_MOU|0x02)  /* (popup?) */

/* Indices - Key Group */

#define KEY_MAP               (CK_T_INT|CK_G_KEY|0x01)  /* (popup?) */

/* Indices - Log Group */

#define LOG_DEBUG_FILE        (CK_ACHAR|CK_G_LOG|0x01)  /* debfil */
#define LOG_DEBUG_ACTIVE      (CK_T_INT|CK_G_LOG|0x02)  /* deblog */
#define LOG_PACKET_FILE       (CK_ACHAR|CK_G_LOG|0x03)  /* pktfil */
#define LOG_PACKET_ACTIVE     (CK_T_INT|CK_G_LOG|0x04)  /* pktlog */
#define LOG_SESSION_FILE      (CK_ACHAR|CK_G_LOG|0x05)  /* sesfil */
#define LOG_SESSION_ACTIVE    (CK_T_INT|CK_G_LOG|0x06)  /* seslog */
#define LOG_TRANSACT_FILE     (CK_ACHAR|CK_G_LOG|0x07)  /* trafil */
#define LOG_TRANSACT_ACTIVE   (CK_T_INT|CK_G_LOG|0x08)  /* tralog */

/* doesn't belong here... but there's no general section */
#define LOG_HOME_DIR          (CK_ACHAR|CK_G_LOG|0x09)  /* home directory */

/* Indices - General Communications Group */

#define COM_MODE              (CK_T_INT|CK_G_COM|0x01)  /* local (/remote) */
#define COM_TYPE              (CK_T_INT|CK_G_COM|0x02)  /* network (/serial) */
#define COM_DEVICE            (CK_ACHAR|CK_G_COM|0x03)  /* ttname[] */
#define COM_PARITY            (CK_T_INT|CK_G_COM|0x04)  /* parity */
#define COM_FLOW              (CK_T_INT|CK_G_COM|0x05)  /* flow */

/* Indices - Serial Communications Group */

#define SER_SPEED             (CK_T_LONG|CK_G_COM|0x01) /* speed */

/* Indices - Modem Group */

#define MOD_TYPE              (CK_T_INT|CK_G_COM|0x01)  /* mdmtyp */
#define MOD_NAME              (CK_ACHAR|CK_G_COM|0x02)  /* (modem type name) */

/* Much more to fill in ... */

/* Indices - Dialing Group */

/* etc etc */

#endif /* kdefs_h_included */
