/*
  Header file to allow 1985 C-Kermit 4.0 pre-ANSI code to build
  with GCC, April 2020.
*/
#include <string.h>
#include <stdlib.h>
#ifdef NETBSD
/* This still doesn't work because stty() and gtty() are missing */
#include <sys/ioctl_compat.h>
#ifndef ECHO
#define ECHO 0x00000008
#endif  /* ECHO */
#endif  /* NETBSD */
