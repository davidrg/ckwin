/*  C K D E B U . H  */

/*
 This file is included by all C-Kermit modules, including the modules
 that aren't specific to Kermit (like the command parser and the ck[xz]*
 modules.  It specifies format codes for debug(), tlog(), and similar
 functions, and includes any necessary typedefs.
*/

#define F000 0				/* Formats for debug() */

#define F001 1
#define F010 2
#define F011 3
#define F100 4
#define F101 5
#define F110 6
#define F111 7

/* Compiler dependencies */

#ifdef PROVX1
typedef char CHAR;
typedef long LONG;
typedef int void;
#else
typedef unsigned char CHAR;
typedef unsigned long LONG;
#endif
