/*  C K C M D . H  --  Header file for cmd package  */

/* Sizes of things */

#define HLPLW  78			/* Width of ?-help line */
#define HLPCW  19			/* Width of ?-help column */
#define CMDBL  200			/* Command buffer length */
#define HLPBL  100			/* Help string buffer length */
#define ATMBL  100			/* Command atom buffer length*/

/* Special characters */

#define NUL  '\0'			/* Null */
#define HT   '\t'			/* Horizontal Tab */
#define NL   '\n'			/* Newline */
#define FF   0014			/* Formfeed    (^L) */
#define RDIS 0022			/* Redisplay   (^R) */
#define LDEL 0025			/* Delete line (^U) */
#define WDEL 0027			/* Delete word (^W) */
#define ESC  0033			/* Escape */
#define RUB  0177			/* Rubout */

#ifndef BEL
#define BEL  0007			/* Bell */
#endif

#ifndef BS
#define BS   0010			/* Backspace */
#endif

#ifndef SP
#define SP   0040			/* Space */
#endif

/* Keyword table flags */

#define CM_INV 1			/* Invisible keyword */

/* Keyword Table Template */

struct keytab {				/* Keyword table */
    char *kwd;				/* Pointer to keyword string */
    int val;				/* Associated value */
    int flgs;				/* Flags (as defined above) */
};
