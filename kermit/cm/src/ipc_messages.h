#ifndef IPC_MESSAGES_H
#define IPC_MESSAGES_H

/* These window messages are used for communcation between
 * the connection manager and C-Kermit */

#define OPT_KERMIT_CONNECT          12003
#define OPT_KERMIT_HANGUP           12004
#define OPT_KERMIT_HWND             12005
#define OPT_KERMIT_EXIT             12006
#define OPT_KERMIT_FG               12007
#define OPT_KERMIT_PID              12008
#define OPT_KERMIT_HWND2            12009
#define OPT_DIALER_CONNECT          13001
#define OPT_DIALER_HWND             13002
#define OPT_DIALER_EXIT             13003
#define OPT_CM_LAUNCH_K95           13004
#define OPT_CM_LAUNCH_PROFILE       13005
#define OPT_CM_QUIT                 13006

#endif /* IPC_MESSAGES_H */