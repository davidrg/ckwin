#define MAX_SCREENSIZE 288		/* maximum number of lines possible */
#define INIT_SCREENSIZE 24		/* PWP: was 24 */
#define MAXCOL      80
#define LINEHEIGHT  11
#define CHARWIDTH    6
#define TOPMARGIN    2		/* Terminal display constants */
#define bottomMARGIN (LINEHEIGHT * screensize + TOPMARGIN)
#define LEFTMARGIN   2
#define rightMARGIN  (CHARWIDTH * MAXCOL + LEFTMARGIN)
#define LINEADJ      2		/* Amount of char below base line */

#define toplin 0
#define botlin (screensize-1)

/* Font Numbers (UoR Mod) to fix bolding problems */
/* These should be placed in the RESOURCE FORK of the executable */

#define VT100FONT  128		/* VT100 Terminal Font (not-bold) */
#define VT100BOLD  129		/* VT100 Bold Font */
#define VTDECTECH  130		/* DEC VT300 Technical */

/* (UoR) constansts that point to the function definitions for arrow keys */
/*  Used by mouse cursor positioning function (chars that are transmitted last) */

# define UPARROW    'A'
# define DOWNARROW  'B'
# define leftARROW  'D'
# define rightARROW 'C'

#define MOVEBEGLINE(v)	MoveTo ((LEFTMARGIN), \
				(((v) + 1) * LINEHEIGHT + TOPMARGIN - LINEADJ))
#define MOVETOCHAR(h,v)	MoveTo (((h) * CHARWIDTH + LEFTMARGIN), \
			        (((v) + 1) * LINEHEIGHT + TOPMARGIN - LINEADJ))

typedef unsigned char *ucharptr;

#define MAX_ARGCOUNT	16	/* max number of CSI <arg>; <arg>; ... CMD args */
#define NUMBUFSIZ	16	/* max. length of digits or intermeadiate chars */
				/*  in a paramater string */

/* stuff for text styles */

			/* these styles we actually do with toolbox */
#define STY_STY		0x0F
#define STY_MSTY	0x07		/* styles that the Mac toobox can do */
#define STY_FONT	0xF0

#define VT_UNDER	1
#define VT_BLINK	2
#define VT_BOLD		4
#define VT_INVERT	8

#define F_VTLAT1	0

extern void term_reset();

#define ANS_VT100AVO	"\033[?1;2c"
#define ANS_VT102	"\033[?6c"
#define ANS_VT320	"\033[63;1;2;8;9c"
#define ANS_VT52	"\033/Z"
#define ANS_H19		"\033/K"
