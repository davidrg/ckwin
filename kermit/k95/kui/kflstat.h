#ifndef kflstat_h_included
#define kflstat_h_included

/* Screen line numbers */

#define CW_BAN  0   /* Curses Window Banner */
#define CW_DIR  2   /* Current directory */
#define CW_LIN  3   /* Communication device */
#define CW_SPD  4   /* Communication speed */
#define CW_PAR  5   /* Parity */
#define CW_NAM  7   /* Filename */
#define CW_TYP  8   /* File type */
#define CW_SIZ  9   /* File size */
#define CW_PCD 10   /* Percent done */

#define CW_BAR 11   /* Percent Bar Scale */
#define CW_TR  12   /* Time remaining */
#define CW_CP  13   /* Chars per sec */
#define CW_WS  14   /* Window slots */
#define CW_PT  15   /* Packet type */
#define CW_PC  16   /* Packet count */
#define CW_PL  17   /* Packet length */
#define CW_PR  18   /* Packet retry */

#define CW_ERR 19   /* Error message */
#define CW_MSG 20   /* Info message */
#define CW_INT 22   /* Instructions */

#define CW_FFC 99   /* File Characters Sent/Received */

#endif
