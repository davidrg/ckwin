char * cklibv = "C-Kermit library, 7.0.009, 29 Nov 1999";

#define CKCLIB_C

/* C K C L I B . C  --  C-Kermit Library routines. */

/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1999, 2000,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/*
  General-purpose, system/platform/compiler-independent routines for use
  by all modules.  Most are replacements for commonly used C library
  functions that are not found on every platform, and/or that lack needed
  functionality (e.g. caseless string search/compare).

    ckstrncpy()  - Similar to strncpy() but different (see comments).
    chartostr()  - Converts a char to a string (self or ctrl char name).
    ckstrchr()   - Portable strchr().
    cklower()    - Lowercase a string (in place).
    ckindex()    - Left or right index.
    ckitoa()     - Converts int to string.
    ckltoa()     - Converts long to string.
    ckmatch()    - Pattern matching.
    ckmemcpy()   - Portable memcpy().
    ckrchar()    - Rightmost character of a string.
    ckstrcmp()   - Possibly caseless string comparison.
    ckstrpre()   - Caseless string prefix comparison.
    sh_sort()    - Sorts an array of strings, many options.
    brstrip()    - Strips enclosing braces.
    makelist()   - Splits "{{item}{item}...}" into an array.
    makestr()    - Careful malloc() front end.
    xmakestr()   - ditto (see comments).
    fileselect() - Select a file based on size, date, excption list, etc.
    radix()      - Convert number radix (2-36).
    b8tob64()    - Convert data to base 64.
    b64tob8()    - Convert base 64 to data.
    chknum()     - Checks if string is an integer.
    rdigits()    - Checks if string is composed only of digits.
    isfloat()    - Checks if string is a valid floating-point number.
    parnam()     - Returns parity name string.
    hhmmss()     - Converts seconds to hh:mm:ss string.
    lset()       - Write fixed-length field left-adjusted into a record.
    rset()       - Write fixed-length field right-adjusted into a record.
    ulongtohex() - Converts an unsigned long to a hex string.
    hextoulong() - Converts a hex string to an unsigned long.

  Prototypes are in ckclib.h.
*/
#include "ckcsym.h"
#include "ckcdeb.h"
#include "ckcasc.h"

char *
ccntab[] = {	/* Names of ASCII (C0) control characters 0-31 */
    "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
    "BS",  "HT",  "LF",  "VT",  "FF",  "CR",  "SO",  "SI",
    "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
    "CAN", "EM",  "SUB", "ESC", "FS",  "GS",  "RS",  "US"
};

char *
c1tab[] = {	/* Names of ISO 6429 (C1) control characters 0-32 */
    "XXX", "XXX", "BPH", "NBH", "IND", "NEL", "SSA", "ESA",
    "HTS", "HTJ", "VTS", "PLD", "PLU", "RI",  "SS2", "SS3",
    "DCS", "PU1", "PU2", "STS", "CCH", "MW",  "SPA", "EPA",
    "SOS", "XXX", "SCI", "CSI", "ST",  "OSC", "PM",  "APC", "NBS"
};

/*  C K S T R N C P Y */

/*
  Copies a NUL-terminated string into a buffer whose total length is given,
  ensuring that the result is NUL-terminated even if it had to be truncated.

  Call with:
    dest = pointer to destination buffer
    src  = pointer to source string
    len  = length of destination buffer (the actual length, not one less).

  Returns:
    int, The number of bytes copied, 0 or more.

  NOTE: This is NOT a replacement for strncpy():
   . strncpy() does not require its source string to be NUL-terminated.
   . strncpy() does not necessarily NUL-terminate its result.
   . strncpy() right-pads dest with NULs if it is longer than src.
   . strncpy() treats the length argument as the number of bytes to copy.
   . ckstrncpy() treats the length argument as the size of the dest buffer.
   . ckstrncpy() doesn't dump core if given NULL string pointers.
   . ckstrncpy() returns a number.
*/
int
#ifdef CK_ANSIC
ckstrncpy(char * dest, const char * src, int len)
#else
ckstrncpy(dest,src,len) char * dest, * src; int len;
#endif /* CK_ANSIC */
{
    int i, x;
    if (len < 1 || !src || !dest) {	/* Nothing or nowhere to copy */
	if (dest) *dest = NUL;
	return(0);
    }
#ifndef NOCKSTRNCPY
    for (i = 0; src[i] && (i < len-1); i++) /* Args OK, copy */
      dest[i] = src[i];
    dest[i] = NUL;
#else
    i = strlen(src);
    if (i > len) i = len;
    strncpy(dest,src,i);
    dest[len] = NUL;
#endif /* NOCKSTRNCPY */
    return(i);
}

/*  C H A R T O S T R  */

/*  Convert a character to a string, interpreting controls.  */

char *
chartostr(x) int x; {			/* Call with char x */
    static char buf[2];			/* Returns string pointer. */
    if (x < 32)
      return(ccntab[x]);
    if (x == 127)
      return("DEL");
    if (x > 127 && x < 161)
      return(c1tab[x]);
    if (x == 0xAD)
      return("SHY");
    buf[1] = NUL;
    buf[0] = (unsigned)(x & 0xff);
    return((char *)buf);
}

/*  C K R C H A R */

/*  Returns the rightmost character of the given null-terminated string */

int
ckrchar(s) char * s; {
    register CHAR c = '\0', *p;
    p = (CHAR *)s;
    if (!p) p = (CHAR *)"";		/* Null pointer == empty string */
    if (!*p) return(0);
    while (*p)				/* Crawl to end of string */
      c = *p++;
    return((unsigned)(c & 0xff));	/* Return final character */
}

/*  C K S T R C H R  */

/*  Replacement for strchr(), which is not universal.  */
/*  Call with:
     s = pointer to string to look in.
     c = character to look for.
    Returns:
     NULL if c not found in s or upon any kind of error, or:
     pointer to first occurrence of c in s.
*/
char *
#ifdef CK_ANSIC
ckstrchr(char * s, char c)
#else
ckstrchr(s,c) char *s, c;
#endif /* CK_ANSIC */
/* ckstrchr */ {
    if (!s)
      return(NULL);
    while (*s && *s != c)
      s++;
    return((*s == c) ? s : NULL);
}

/*  C K L O W E R  --  Lowercase a string  */

/* Returns the length of the string */

int
cklower(s) char *s; {
    int n = 0;
    if (!s) return(0);
    while (*s) {
        if (isupper(*s)) *s = (char) tolower(*s);
        s++, n++;
    }
    return(n);
}

/*  C K L T O A  --  Long to string  --  FOR DISCIPLINED USE ONLY  */

#define NUMBUF 1024
static char numbuf[NUMBUF+32] = { NUL, NUL };
static int numbp = 0;
/*
  ckltoa() and ckitoa() are like atol() and atoi() in the reverse direction,
  returning a pointer to the string representation of the given number without
  the caller having to worry about allocating or defining a buffer first.
  They manage their own internal buffer, so successive calls return different
  pointers.  However, to keep memory consumption from growing without bound,
  the buffer recycles itself.  So after several hundred calls (depending on
  the size of the numbers), some of the earlier pointers might well find
  themselves referencing something different.  Moral: You can't win in C.
  Therefore, these routines are intended mainly for generating numeric strings
  for short-term use, e.g. for passing numbers in string form as parameters to
  functions.  For long-term use, the result must be copied to a safe place.
*/
char *
#ifdef CK_ANSIC
ckltoa(long n)
#else
ckltoa(n) long n;
#endif /* CK_ANSIC */
/* ckltoa */ {
    char buf[32];			/* Internal working buffer */
    char * p, * s, * q;
    int k, x, sign = 0;
    if (n < 0L) {			/* Sign */
	n = 0L - n;
	sign = 1;
    }
    buf[31] = NUL;
    for (k = 30; k > 0; k--) {		/* Convert number to string */
	x = n % 10L;
	buf[k] = x + '0';
	n = n / 10L;
	if (!n)
	  break;
    }
    if (sign) buf[--k] = '-';		/* Add sign if necessary */
    p = numbuf + numbp;
    q = p;
    s = buf + k;
    while (*p++ = *s++ ) ;		/* Copy */
    if (numbp >= NUMBUF)		/* Update pointer */
      numbp = 0;
    else
      numbp += k;
    return(q);				/* Return pointer */
}


/*  C K I T O A  --  Int to string  -- FOR DISCIPLINED USE ONLY  */

char *
ckitoa(n) int n; {			/* See comments with ckltoa(). */
    long nn;
    nn = n;
    return(ckltoa(nn));
}


/*  C K I N D E X  --  C-Kermit's index function  */
/*
  We can't depend on C libraries to have one, so here is our own.
  Call with:
    s1 - String to look for.
    s2 - String to look in.
     t - Offset from right or left of s2, 0 based; -1 for rightmost char in s2.
     r - 0 for left-to-right search, non-0 for right-to-left.
  icase  0 for case independence, non-0 if alphabetic case matters.
  Returns 0 if string not found, otherwise a 1-based result.
  Also returns 0 on any kind of error, e.g. junk parameters.
*/
int
ckindex(s1,s2,t,r,icase) char *s1, *s2; int t, r, icase; {
    int len1, len2, i, j, x, ot = t;	/* ot = original t */
    char * s;

    if (!s1 || !s2) return(0);
    len1 = (int)strlen(s1);		/* length of string to look for */
    len2 = (int)strlen(s = s2);		/* length of string to look in */
    if (t < 0) t = len2 - 1;

    if (len1 < 0) return(0);		/* paranoia */
    if (len2 < 0) return(0);
    j = len2 - len1;			/* length difference */

    if (j < 0 || (r == 0 && t > j))	/* search string is longer */
      return(0);
    if (r == 0) {			/* Index */
	s = s2 + t;			/* Point to beginning of target */
	for (i = 0; i <= (j - t); i++) { /* Now compare */
	    x = ckstrcmp(s1,s++,len1,icase);
	    if (!x)
	      return(i+1+t);
	}
    } else {				/* Reverse Index */
        i = len2 - len1;		/* Where to start looking */
        if (ot > 0)			/* Figure in offset if any */
	  i -= t;
	for (j = i; j > -1; j--) {
	    if (!ckstrcmp(s1,&s2[j],len1,icase))
	      return(j+1);
	}
    }
    return(0);
}

/*  B R S T R I P  --  Strip enclosing braces from arg string, in place */
/*
  Call with:
    Pointer to string that can be poked.
  Returns:
    Pointer to string without enclosing braces.
    If original string was not braced, this is the arg pointer;
    otherwise it is 1 + the arg pointer, with the matching closing
    brace zero'd out.  If the string starts with a brace but does not
    end with a matching brace, the original pointer to the original
    string is returned.  If the arg pointer is NULL, a pointer to an
    empty string is returned.
*/
char *
brstrip(p) char *p; {
    if (!p) return("");
    if (*p == '{') {
	int x;
	x = (int)strlen(p) - 1;
	if (p[x] == '}') {
	    p[x] = NUL;
	    p++;
	}
    }
    return(p);
}


/*  M A K E L I S T  ---  Breaks {{s1}{s2}..{sn}} into an array of strings */
/*
  Call with:
    s    = pointer to string to break up.
    list = array of string pointers.
    len  = number of elements in array.
  NOTE: The array must be preinitialized to all NULL pointers.
  If any array element is not NULL, it is assumed to have been malloc'd
  and is therefore freed.  Do NOT call this function with an unitialized
  array, or with an array that has had any static elements assigned to it.
*/
VOID
makelist(s,list,len) char * s; char *list[]; int len; {
    int i, n, q, bc = 0;
    char *p = NULL, *s2 = NULL;
    debug(F110,"makelist s",s,0);
    if (!s) {				/* Check for null or empty string */
	list[0] = NULL;
	return;
    }
    n = strlen(s);
    if (n == 0) {
	list[0] = NULL;
	return;
    }
    if (s2 = (char *)malloc(n+1)) {	/* Safe copy for poking */
	strcpy(s2,s);			/* (no need for ckstrncpy here) */
	s = s2;
    }
    s = brstrip(s);			/* Strip braces */
    n = strlen(s);			/* Get length */
    if (*s != '{') {			/* Outer braces only */
	if (p = (char *)malloc(n+1)) {	/* So just one pattern */
	    strcpy(p,s);		/* (no need for ckstrncpy here) */
	    if (list[0])
	      free(list[0]);
	    list[0] = p;
	}
	if (s2) free(s2);
	return;
    }
    q = 0;				/* Inner ones too */
    i = 0;				/* so a list of patterns. */
    n = 0;
    while (*s && i < len) {
	if (*s == CMDQ) {		/* Quote... */
	    q = 1;
	    s++;
	    n++;
	    continue;
	}
	if (*s == '{' && !q) {		/* Opening brace */
	    if (bc++ == 0) {		/* Beginning of a group */
		p = ++s;
		n = 0;
	    } else {			/* It's a brace inside the group */
		n++;
		s++;
	    }
	    continue;
	} else if (*s == '}' && !q) {	/* Closing brace */
	    if (--bc == 0) {		/* End of a group */
		*s++ = NUL;
		debug(F111,"makelist element",p,i);
		if (list[i])
		  free(list[i]);
		if (list[i] = (char *)malloc(n+1)) {
		    ckstrncpy(list[i],p,n+1); /* Note: n+1 */
		    i++;
		}
		while (*s == SP) s++;
		p = s;
		n = 0;
		continue;
	    } else {			/* Within a group */
		n++;
		s++;
	    }
	} else {			/* Regular character */
	    q = 0;
	    s++;
	    n++;
	}
    }
    if (*p && i < len) {		/* Last one */
	if (list[i])
	  free(list[i]);
	if (list[i] = (char *)malloc(n+1)) {
	    ckstrncpy(list[i],p,n+1);
	    debug(F111,"makelist last element",p,i);
	}
    }
    if (s2) free(s2);
}

/*
   M A K E S T R  --  Creates a dynamically allocated string.

   Makes a new copy of string s and sets pointer p to its address.
   Handles degenerate cases, like when buffers overlap or are the same,
   one or both arguments are NULL, etc.

   The target pointer must be either NULL or else a pointer to a previously
   malloc'ed buffer.  If not, expect a core dump or segmentation fault.

   Note: The caller can tell whether this routine failed as follows:

     malloc(&p,q);
     if (q & !p) { makestr() failed };
*/
VOID
#ifdef CK_ANSIC
makestr(char **p, const char *s)
#else
makestr(p,s) char **p, *s;
#endif
/* makestr */ {
    int x;
    char *q = NULL;

    if (*p == s)			/* The two pointers are the same. */
      return;				/* Don't do anything. */

    if (!s) {				/* New definition is null? */
	if (*p)				/* Free old storage. */
	  free(*p);
	*p = NULL;			/* Return null pointer. */
	return;
    }
    if ((x = strlen(s)) >= 0) {		/* Get length, even of empty string. */
	q = malloc(x+1);		/* Get and point to temp storage. */
	if (q) {
	    strcpy(q,s);		/* (no need for ckstrncpy() here) */
	}
#ifdef DEBUG
	else {				/* This would be a really bad error */
	    char tmp[24];		/* So get a good record of it. */
	    if (x > 23) {
		ckstrncpy(tmp,s,20);
		strcpy(tmp+20,"...");
		tmp[23] = NUL;
	    } else {
		ckstrncpy(tmp,s,24);
	    }
	    debug(F110,"MAKESTR MALLOC FAILURE ",s,0);
	}
#endif /* DEBUG */
    } else
      q = NULL;				/* Length of string is zero */

    if (*p) {				/* Now free the original storage. */
#ifdef BETATEST
	memset(*p,0xFF,sizeof(**p));	/* (not portable) */
#endif /* BETATEST */
	free(*p);
    }
#ifdef COMMENT
    *q = NULL;				/* Set up return pointer */
    if (q)
      *p = q;
#else
    *p = q;				/* This is exactly the same */
#endif /* COMMENT */

}

/*  X M A K E S T R  --  Non-destructive makestr() if s is NULL.  */

VOID
#ifdef CK_ANSIC
xmakestr(char **p, const char *s)
#else
xmakestr(p,s) char **p, *s;
#endif
/* xmakestr */ {
    if (s) makestr(p,s);
}

/* C K M E M C P Y  --  Portable (but slow) memcpy() */

/* Copies n bytes from s to p, allowing for overlap. */
/* For use when real memcpy() not available. */

VOID
ckmemcpy(p,s,n) char *p, *s; int n; {
    char * q = NULL;
    register int i;
    int x;

    if (!s || !p || n <= 0 || p == s)	/* Verify args */
      return;
    x = p - s;				/* Check for overlap */
    if (x < 0)
      x = 0 - x;
    if (x < n) {			/* They overlap */
	q = p;
	if (!(p = (char *)malloc(n)))	/* So use a temporary buffer */
	  return;
    }
    for (i = 0; i < n; i++)		/* Copy n bytes */
      p[i] = s[i];
    if (q) {				/* If we used a temporary buffer */
	for (i = 0; i < n; i++)		/* copy from it to destination */
	  q[i] = p[i];
	if (p) free(p);			/* and free the temporary buffer */
    }
}


/*  C K S T R C M P  --  String comparison with case-matters selection */
/*
  Call with pointers to the two strings, s1 and s2, a length, n,
  and c == 0 for caseless comparison, nonzero for case matters.
  Call with n == -1 to compare without a length limit.
  Compares up to n characters of the two strings and returns:
    1 if s1 > s2
    0 if s1 = s2
   -1 if s1 < s2
*/
int
ckstrcmp(s1,s2,n,c) char *s1, *s2; int n, c; {
    CHAR t1, t2;
    if (n == 0) return(0);
    if (!s1) s1 = "";			/* Watch out for null pointers. */
    if (!s2) s2 = "";
    if (!*s1) return(*s2 ? -1 : 0);
    if (!*s2) return(1);
    while (n--) {
	t1 = (CHAR) *s1++;		/* Get next character from each. */
	t2 = (CHAR) *s2++;
	if (!t1) return(t2 ? -1 : 0);
	if (!t2) return(1);
	if (!c) {			/* If case doesn't matter */
	    if (isupper(t1)) t1 = tolower(t1); /* Convert case. */
	    if (isupper(t2)) t2 = tolower(t2);
	}
	if (t1 < t2) return(-1);	/* s1 < s2 */
	if (t1 > t2) return(1);		/* s1 > s2 */
    }
    return(0);				/* They're equal */
}

/*  C K S T R P R E  --  Caseless string prefix comparison  */

/* Returns position of the first char in the 2 strings that doesn't match */

int
ckstrpre(s1,s2) char *s1, *s2; {
    CHAR t1, t2;
    int n = 0;
    if (!s1) s1 = "";
    if (!s2) s2 = "";
    while (1) {
	t1 = (CHAR) *s1++;
	t2 = (CHAR) *s2++;
	if (!t1 || !t2) return(n);
	if (isupper(t1)) t1 = tolower(t1);
	if (isupper(t2)) t2 = tolower(t2);
	if (t1 != t2)
	  return(n);
	n++;
    }
}

#define GLOBBING

/*  C K M A T C H  --  Match a string against a pattern  */
/*
  Call with a pattern containing * and/or ? metacharacters.
  icase is 1 if case-sensitive, 0 otherwise.
  opts is a bitmask:
    Bit 0: 1 to match strings starting with '.', else 0.
    Bit 1: 1 = file globbing (dirseps are fences, etc), 0 = ordinary string.
    Bit 2 (and beyond): Undefined.
  Works only with NUL-terminated strings.
  Pattern may contain any number of ? and/or *.
  If CKREGEX is defined, also [abc], [a-z], and/or {string,string,...}.

  Returns:
    0 if string does not match pattern,
    1 if it does.

  To be done:
    Find a way to identify the piece of the string that matched the pattern,
    as in Snobol "LINE (PAT . RESULT)".  Some prelinary attempts are commented
    out (see "mstart"); these fail because they always indicate the entire
    string.  The piece we want (I think) is the the part that matches the
    first non-* segment of the pattern through the final non-* part.  If this
    can be done, we can streamline INPUT and friends considerably, and also
    add regexp support to functions like \findex(\fpattern(a*b),\%s).  INPUT
    accomplishes this now by repeated calls to ckmatch, which is overkill.
*/
#ifdef COMMENT
char * ckmstring = NULL;
#endif /* COMMENT */

int
ckmatch(pattern, string, icase, opts) char *pattern,*string; int icase, opts; {
    int q = 0, i = 0, k = -1, x, flag = 0;
    CHAR cp;				/* Current character from pattern */
    CHAR cs;				/* Current character from string */
    char * psave = NULL;
    int dot, globbing;

#ifdef COMMENT
    char * mstart = NULL;		/* Pointer to beginning of match */
#endif /* COMMENT */

    dot = opts & 1;
    globbing = opts & 2;

#ifdef COMMENT
    makestr(&ckmstring,NULL);
#endif /* COMMENT */
    if (!pattern) pattern = "";
    if (!*pattern) return(1);		/* Null pattern always matches */
    if (!string) string = "";

    debug(F110,"ckmatch string",string,0);
    debug(F111,"ckmatch pattern",pattern,opts);

#ifdef COMMENT
    mstart = string;
#endif /* COMMENT */

#ifdef UNIX
    if (!dot) {				/* For UNIX file globbing */
	if (*string == '.' && *pattern != '.' && !matchdot) {
	    if (
#ifdef CKREGEX
		*pattern != '{' && *pattern != '['
#else
		1
#endif /* CKREGEX */
		) {
		debug(F110,"ckmatch skip",string,0);
		return(0);
	    }
	}
    }
#endif /* UNIX */
    while (1) {
	k++;
	cp = *pattern;			/* Character from pattern */
	cs = *string;			/* Character from string */

	if (!cs) {			/* End of string - done. */
	    x = (!cp || (cp == '*' && !*(pattern+1))) ? 1 : 0;
#ifdef COMMENT
	    if (x) makestr(&ckmstring,mstart);
#endif /* COMMENT */
	    return(x);
	}
        if (!icase) {			/* If ignoring case */
	    if (isupper(cp))		/* convert both to lowercase. */
	      cp = tolower(cp);
	    if (isupper(cs))
	      cs = tolower(cs);
        }
	if (q) {			/* This character was quoted */
	    q = 0;			/* Turn off quote flag */
	    if (cs == cp)		/* Compare directly */
	      pattern++, string++;	/* no metacharacters */
	    continue;
	}
	if (cp == CMDQ && !q) {		/* Quote in pattern */
	    q = 1;			/* Set flag */
	    pattern++;			/* Advance to next pattern character */
	    cp = *pattern;		/* Case conversion... */
	    if (!icase)
	      if (isupper(cp))
		cp = tolower(cp);
	    if (cp != cs)		/* Literal char so compare now */
	      return(0);		/* No match, done. */
	    string++, pattern++;	/* They match so advance pointers */
	    continue;			/* and continue. */
	}
	if (cs && cp == '?') {		/* '?' matches any char */
	    pattern++, string++;
	    continue;
#ifdef CKREGEX
	} else if (cp == '[') {		/* Have bracket */
	    int q = 0;			/* My own private q */
	    char * psave = NULL;	/* and backup pointer */
	    CHAR clist[256];		/* Character list from brackets */
	    CHAR c, c1, c2;
	    for (i = 0; i < 256; i++)	/* memset() etc not portable */
	      clist[i] = NUL;
	    psave = ++pattern;		/* Where pattern starts */
	    for (flag = 0; !flag; pattern++) { /* Loop thru pattern */
		c = (unsigned)*pattern;	/* Current char */
		if (q) {		/* Quote within brackets */
		    q = 0;
		    clist[c] = 1;
		    continue;
		}
		if (!icase)		/* Case conversion */
		  if (isupper(c))
		    c = tolower(c);
		switch (c) {		/* Handle unquoted character */
		  case NUL:		/* End of string */
		    return(0);		/* No matching ']' so fail */
		  case CMDQ:		/* Next char is quoted */
		    q = 1;		/* Set flag */
		    continue;		/* and continue. */
		  case '-':		/* A range is specified */
		    c1 = (pattern > psave) ? (unsigned)*(pattern-1) : NUL;
		    c2 = (unsigned)*(pattern+1);
		    if (c2 == ']') c2 = NUL;
		    if (c1 == NUL) c1 = c2;
		    for (c = c1; c <= c2; c++)
		      clist[c] = 1;
		    continue;
		  case ']':		/* End of bracketed sequence */
		    flag = 1;		/* Done with FOR loop */
		    break;		/* Compare what we have */
		  default:		/* Just a char */
		    clist[c] = 1;	/* Record it */
		    continue;
		}
	    }
	    if (!clist[(unsigned)cs]) 	/* Match? */
	      return(0);		/* Nope, done. */
	    string++;			/* Yes, advance string pointer */
	    continue;			/* and go on. */
	} else if (cp == '{') {		/* Braces with list of strings */
	    char * p, * s, * s2, * buf = NULL;
	    int n, bc = 0;
	    int len = 0;
	    for (p = pattern++; *p; p++) {
		if (*p == '{') bc++;
		if (*p == '}') bc--;
		if (bc < 1) break;
	    }
	    if (bc != 0) {		/* Braces don't match */
		return(0);		/* Fail */
	    } else {			/* Braces do match */
		int q = 0, done = 0;
		len = *p ? strlen(p+1) : 0; /* Length of rest of pattern */
		n = p - pattern;	/* Size of list in braces */
		if (buf = (char *)malloc(n+1)) { /* Copy so we can poke it */
		    char * tp = NULL;
		    int k;
		    ckstrncpy(buf,pattern,n+1);
		    n = 0;
		    for (s = s2 = buf; 1; s++) { /* Loop through segments */
			n++;
			if (q) {	/* This char is quoted */
			    q = 0;
			    if (!*s)
			      done = 1;
			    continue;
			}
			if (*s == CMDQ && !q) {	/* Quote next char */
			    q = 1;
			    continue;
			}
			if (!*s || *s == ',') {	/* End of this segment */
			    if (!*s)	/* If end of buffer */
			      done = 1;	/* then end of last segment */
			    *s = NUL;	/* Overwrite comma with NUL */
			    if (!*s2) {	/* Empty segment, no advancement */
				k = 0;
			    } else if (tp = (char *)malloc(n+len+1)) {
				strcpy(tp,s2);  /* Current segment */
				strcat(tp,p+1);	/* Add rest of pattern */
				tp[n+len] = NUL;
				k = ckmatch(tp,string,icase,opts);
				free(tp);
				if (k > 0) { /* If it matched we're done */
#ifdef COMMENT
				    makestr(&ckmstring,mstart);
#endif /* COMMENT */
				    return(1);
				}
			    } else {	/* Malloc failure, just compare */
				k = !ckstrcmp(tp,string,n-1,icase);
			    }
			    if (k) {	/* Successful comparison */
				string += n-1; /* Advance pointers */
				pattern = p+1;
				break;
			    }
			    if (done)	/* If no more segments */
			      break;	/* break out of segment loop. */
			    s2 = s+1;	/* Otherwise, on to next segment */
			    n = 0;
			}
		    }
		    free(buf);
		}
	    }
#endif /* CKREGEX */
	} else if (cp == '*') {		/* Asterisk */
	    char * p, * s = NULL;
	    int k, n, q = 0;
	    while (*pattern == '*')	/* Collapse successive asterisks */
	      pattern++;
	    psave = pattern;		/* First non-asterisk after asterisk */
	    for (n = 0, p = psave; *p; p++,n++) { /* Find next meta char */
		if (!q) {
		    if (*p == '?' || *p == '*' || *p == CMDQ
#ifdef CKREGEX
			|| *p == '[' || *p == '{'
#endif /* CKREGEX */
			)
		      break;
#ifdef GLOBBING
		    if (globbing
#ifdef UNIXOROSK
			&& *p == '/'
#else
#ifdef VMS
			&& (*p == '.' || *p == ']' ||
			    *p == '<' || *p == '>' ||
			    *p == ':' || *p == ';')
#else
#ifdef datageneral
			&& *p == ':'
#else
#ifdef STRATUS
			&& *p == '>'
#endif /* STRATUS */
#endif /* datageneral */
#endif /* VMS */
#endif /* UNIXOROSK */
			)
		      break;
#endif /* GLOBBING */
		}
	    }
	    if (n > 0) {		/* Literal string to match  */
		s = (char *)malloc(n+1);
		if (s) {
		    ckstrncpy(s,psave,n+1); /* Copy cuz no poking original */
		    debug(F111,"XXX",s,n+1);
		    if (*p == '*')
		      k = ckindex(s,string,0,0,icase); /* 1-based index() */
		    else
		      k = ckindex(s,string,-1,1,icase); /* 1-based rindex() */
		    free(s);
		    if (k < 1)
		      return(0);
		    string += k + n - 1;
		    pattern += n;
		    continue;
		}
	    } else if (!*p) {		/* Asterisk at end matches the rest */
		if (!globbing) {	/* (if not filename globbing) */
#ifdef COMMENT
		    makestr(&ckmstring,mstart);
#endif /* COMMENT */
		    return(1);
		}
#ifdef GLOBBING
		while (*string) {	/* Globbing so don't cross fields */
		    if (globbing
#ifdef UNIXOROSK
			&& *string == '/'
#else
#ifdef VMS
			&& (*string == '.' || *string == ']' ||
			    *string == '<' || *string == '>' ||
			    *string == ':' || *string == ';')
#else
#ifdef datageneral
			&& *string == ':'
#else
#ifdef STRATUS
			&& *string == '>'
#endif /* STRATUS */
#endif /* datageneral */
#endif /* VMS */
#endif /* UNIXOROSK */
			)
		      return(0);
		    string++;
		}
#endif /* GLOBBING */
#ifdef COMMENT
		makestr(&ckmstring,mstart);
#endif /* COMMENT */
		return(1);

	    } else {			/* A meta char follows asterisk */
		while (*string && (k = ckmatch(p,string,icase,opts) < 1))
		  string++;
#ifdef COMMENT
		if (*string) makestr(&ckmstring,mstart);
#endif /* COMMENT */
		return(*string ? 1 : 0);
	    }
	} else if (cs == cp) {
	    pattern++, string++;
	    continue;
	} else
	  return(0);
    }
}


#ifdef CKFLOAT
/*  I S F L O A T  -- Verify that arg represents a floating-point number */

/*
  Portable replacement for atof(), strtod(), scanf(), etc.

  Call with:
    s = pointer to string
    flag == 0 means entire string must be a (floating-pointing) number.
    flag != 0 means to terminate scan on first character that is not legal.

  Returns:
    1 if result is a floating point number;
    0 if not or if string empty.

  Side effect:
    Sets global floatval to floating-point value if successful.

  Number need not contain a decimal point -- integer is subcase of float.
  Scientific notation not supported.
*/
CKFLOAT floatval = 0.0;			/* For returning value */

int
isfloat(s,flag) char *s; int flag; {
    int state = 0;
    int sign = 0;
    char c;
    CKFLOAT d = 0.0, f = 0.0;

    if (!s) return(0);
    if (!*s) return(0);

    while (isspace(*s)) s++;

    if (*s == '-') {			/* Handle optional sign */
	sign = 1;
	s++;
    } else if (*s == '+')
      s++;
    while (c = *s++) {			/* Handle numeric part */
	switch (state) {
	  case 0:			/* Mantissa... */
	    if (isdigit(c)) {
		f = f * 10.0 + (CKFLOAT)(c - '0');
		continue;
	    } else if (c == '.') {
		state = 1;
		d = 1.0;
		continue;
	    }
	    if (flag)			/* Not digit or period */
	      goto done;		/* break if flag != 0 */
	    return(0);			/* otherwise fail. */
	  case 1:			/* Fraction... */
	    if (isdigit(c)) {
		d *= 10.0;
		f += (CKFLOAT)(c - '0') / d;
		continue;
	    }
	  default:
	    if (flag)			/* Illegal character */
	      goto done;		/* Break */
	    return(0);			/* or fail, depending on flag */
	}
    }
  done:
    if (sign) f = 0.0 - f;		/* Apply sign to result */
    floatval = f;			/* Set result */
    return(1);				/* Succeed */
}
#endif /* CKFLOAT */

/* Sorting routines... */

#ifdef USE_QSORT
/*
  Quicksort works but it's not measurably faster than shell sort,
  probably because it makes a lot more comparisons, since
  it was originally designed for sorting an array of integers.
  It would need more thorough testing and debugging before production use.
*/
static int			/* Internal comparison routine for ckqsort() */
compare(s1,s2,k,r,c) char *s1, *s2; int k, r, c; {
    int x;
    char *t, *t1, *t2;
#ifdef CKFLOAT
    CKFLOAT f1, f2;
#else
    long n1, n2;
#endif /* CKFLOAT */

    t = t2 = s1;			/* String 1 */
    if (!t)				/* If it's NULL */
      t2 = "";				/* make it the empty string */
    if (k > 0 && *t2) {
	if ((int)strlen(t2) < k)	/* If key too big */
	  t2 = "";			/* make key the empty string */
	else				/* Key is in string */
	  t2 += k;			/* so point to key position */
    }
    t1 = s2;
    if (!t1)				/* Same deal for s2 */
      t1 = "";
    if (k > 0 && *t1) {
	if ((int)strlen(t1) < k)
	  t1 = "";
	else
	  t1 += k;
    }
    if (c == 2) {			/* Numeric comparison */
	x = 0;
#ifdef CKFLOAT
	f2 = 0.0;
	f1 = 0.0;
	if (isfloat(t1,1)) {
	    f1 = floatval;
	    if (isfloat(t2,1))
	      f2 = floatval;
	    else
	      f1 = 0.0;
	}
	if (f2 < f1)
	  x = 1;
	else
	  x = -1;
#else
	n2 = 0L;
	n1 = 0L;
	if (rdigits(t1)) {
	    n1 = atol(t1);
	    if (rdigits(t2))
	      n2 = atol(t2);
	    else
	      n1 = 0L;
	}
	if (n2 < n1)
	  x = 1;
	else
	  x = -1;
#endif /* CKFLOAT */
    } else {
	x = ckstrcmp(t1,t2,-1,c);
    }
    return(x);
}

/* It's called sh_sort() but it's really quicksort... */

VOID
sh_sort(s,s2,n,k,r,how) char **s, **s2; int n, k, r, how; {
    int x, lp, up, p, lv[16], uv[16], m, c;
    char * y, * y2;

    if (!s) return;
    if (n < 2) return;
    if (k < 0) k = 0;

    lv[0] = 0;
    uv[0] = n-1;
    p = 0;
stb:					/* Hmmm, looks like Fortran... */
    if (p < 0)
    return;
stc:
    lp = lv[p];
    up = uv[p];
    m = up - lp + 1;
    if (m < 2) {
	p--;
	goto stb;
    }
    if (m == 2) {
	x = compare(s[lp],s[up],k,r,how);
	if (x > 0) {
	    y = s[lp];
	    s[lp] = s[up];
	    s[up] = y;
	    if (s2) {
		y2 = s2[lp];
		s2[lp] = s2[up];
		s2[up] = y2;
	    }
	}
	p--;
	goto stb;
    }
    c = (lp+up) / 2;
    if (m < 10)
      goto std;
    x = compare(s[lp],s[c],k,r,how);
    if (x < 1) {
	if (s[c] <= s[up]) {
	    goto std;
        } else {
	    x = compare(s[lp],s[up],k,r,how);
	    if (x < 1)
	      c = up;
	    else
	      c = lp;
	    goto std;
	}
    } else {
	x = compare(s[up],s[c],k,r,how);
	if (x < 1) {
	    goto std;
	} else {
	    x = compare(s[lp],s[up],k,r,how);
	    if (x < 1)
	      c = lp;
	    else
	      c = up;
	    goto std;
	}
    }
std:
    y = s[c];
    s[c] = s[up];
    if (s2) {
	y2 = s2[c];
	s2[c] = s2[up];
    }
    lp--;
stf:
    if ((up - lp) < 2)
      goto stk;
    lp++;
    x = compare(s[lp],y,k,r,how);
    if (x < 1)
      goto stf;
    s[up] = s[lp];
sth:
    if ((up - lp) < 2)
      goto stj;
    up--;
    x = compare(s[up],y,k,r,how);
    if (x > 0)
      goto sth;
    s[lp] = s[up];
    goto stf;
stj:
    up--;
stk:
    if (up == uv[p]) {
	lp = lv[p] - 1;
stl:
	if ((up - lp) < 2)
	  goto stq;
	lp++;
	x = compare(s[lp],y,k,r,how);
	if (x < 0)
	  goto stl;
	s[up] = s[lp];
stn:
	if ((up - lp) < 2)
	  goto stp;
	up--;
	x = compare(s[up],y,k,r,how);
	if (x >= 0)
	  goto stn;
	s[lp] = s[up];
	goto stl;
stp:
	up--;
stq:
	s[up] = y;
	if (s2)
	  s2[up] = y2;
        if (up == lv[p]) {
	    p--;
	    goto stb;
	}
	uv[p] = up - 1;
	goto stc;
    }
    s[up] = y;
    if (s2)
      s2[up] = y2;
    if ((up - lv[p]) < (uv[p] - up)) {
	lv[p+1] = lv[p];
	uv[p+1] = up - 1;
	lv[p] = up + 1;
    } else {
	lv[p+1] = up + 1;
	uv[p+1] = uv[p];
	uv[p] = up - 1;
    }
    p++;
    goto stc;
}

#else  /* !USE_QSORT */

/* S H _ S O R T  --  Shell sort -- sorts string array s in place. */

/*
  Highly defensive and relatively quick.
  Uses shell sort algorithm.

  Args:
   s = pointer to array of strings.
   p = pointer to a second array to sort in parallel s, or NULL for none.
   n = number of elements in s.
   k = position of key.
   r = ascending lexical order if zero, reverse lexical order if nonzero.
   c = 0 for case independence, 1 for case matters, 2 for numeric.

  If k is past the right end of a string, the string is considered empty
  for comparison purposes.

  Hint:
   To sort a piece of an array, call with s pointing to the first element
   and n the number of elements to sort.

  Return value:
   None.  Always succeeds, unless any of s[0]..s[n-1] are bad pointers,
   in which case memory violations are possible, but C offers no defense
   against this, so no way to gracefully return an error code.
*/
VOID
sh_sort(s,p,n,k,r,c) char **s, **p; int n, k, r, c; {
    int m, i, j, x;
    char *t, *t1, *t2, *u = NULL;
#ifdef CKFLOAT
    CKFLOAT f1, f2;
#else
    long n1, n2;
#endif /* CKFLOAT */

    if (!s) return;			/* Nothing to sort? */
    if (n < 2) return;			/* Not enough elements to sort? */
    if (k < 0) k = 0;			/* Key */

    m = n;				/* Initial group size is whole array */
    while (1) {
	m = m / 2;			/* Divide group size in half */
	if (m < 1)			/* Small as can be, so done */
	  break;
	for (j = 0; j < n-m; j++) {	/* Sort each group */
	    t = t2 = s[j+m];		/* Compare this one... */
	    if (!t)			/* But if it's NULL */
	      t2 = "";			/* make it the empty string */
	    if (p)			/* Handle parallel array, if any */
	      u = p[j+m];
	    if (k > 0 && *t2) {
		if ((int)strlen(t2) < k) /* If key too big */
		  t2 = "";		/* make key the empty string */
		else			/* Key is in string */
		  t2 = t + k;		/* so point to key position */
	    }
	    for (i = j; i >= 0; i -= m) { /* Loop thru comparands s[i..]*/
		t1 = s[i];
		if (!t1)		/* Same deal */
		  t1 = "";
		if (k > 0 && *t1) {
		    if ((int)strlen(t1) < k)
		      t1 = "";
		    else
		      t1 = s[i]+k;
		}
		if (c == 2) {		/* Numeric comparison */
		    x = 0;
#ifdef CKFLOAT
		    f2 = 0.0;
		    f1 = 0.0;
		    if (isfloat(t1,1)) {
			f1 = floatval;
			if (isfloat(t2,1))
			  f2 = floatval;
			else
			  f1 = 0.0;
		    }
		    if (f2 < f1)
		      x = 1;
		    else
		      x = -1;
#else
		    n2 = 0L;
		    n1 = 0L;
		    if (rdigits(t1)) {
			n1 = atol(t1);
			if (rdigits(t2))
			  n2 = atol(t2);
			else
			  n1 = 0L;
		    }
		    if (n2 < n1)
		      x = 1;
		    else
		      x = -1;
#endif /* CKFLOAT */
		} else {
		    x = ckstrcmp(t1,t2,-1,c); /* Compare */
		}
		if (r == 0 && x < 0)
		  break;
		if (r != 0 && x > 0)
		  break;
		s[i+m] = s[i];
		if (p) p[i+m] = p[i];
	    }
	    s[i+m] = t;
	    if (p) p[i+m] = u;
	}
    }
}
#endif /* COMMENT */

/*  F I L E S E L E C T  --  Select this file for sending  */

int
fileselect(f,sa,sb,sna,snb,minsiz,maxsiz,nbu,nxlist,xlist)
 char *f,*sa,*sb,*sna,*snb; long minsiz,maxsiz; int nbu,nxlist; char ** xlist;
/* fileselect */ {
    char *fdate;
    int n;
    long z;

    if (!sa) sa = "";
    if (!sb) sb = "";
    if (!sna) sna = "";
    if (!snb) snb = "";

    debug(F110,"fileselect",f,0);
    if (*sa || *sb || *sna || *snb) {
	fdate = zfcdat(f);		/* Date/time of this file */
	if (!fdate) fdate = "";
	n = strlen(fdate);
	debug(F111,"fileselect fdate",fdate,n);
	if (n != 17)			/* Failed to get it */
	  return(1);
	/* /AFTER: */
	if (sa[0] && (strcmp(fdate,(char *)sa) <= 0)) {
	    debug(F110,"fileselect sa",sa,0);
	    /* tlog(F110,"Skipping (too old)",f,0); */
	    return(0);
	}
	/* /BEFORE: */
	if (sb[0] && (strcmp(fdate,(char *)sb) >= 0)) {
	    debug(F110,"fileselect sb",sb,0);
	    /* tlog(F110,"Skipping (too new)",f,0); */
	    return(0);
	}
	/* /NOT-AFTER: */
	if (sna[0] && (strcmp(fdate,(char *)sna) > 0)) {
	    debug(F110,"fileselect sna",sna,0);
	    /* tlog(F110,"Skipping (too new)",f,0); */
	    return(0);
	}
	/* /NOT-BEFORE: */
	if (snb[0] && (strcmp(fdate,(char *)snb) < 0)) {
	    debug(F110,"fileselect snb",snb,0);
	    /* tlog(F110,"Skipping (too old)",f,0); */
	    return(0);
	}
    }
    if (minsiz > -1L || maxsiz > -1L) { /* Smaller or larger */
	z = zchki(f);			/* Get size */
	debug(F101,"fileselect filesize","",z);
	if (z < 0)
	  return(1);
	if ((minsiz > -1L) && (z >= minsiz)) {
	    debug(F111,"fileselect minsiz skipping",f,minsiz);
	    /* tlog(F111,"Skipping (too big)",f,z); */
	    return(0);
	}
	if ((maxsiz > -1L) && (z <= maxsiz)) {
	    debug(F111,"fileselect maxsiz skipping",f,maxsiz);
	    /* tlog(F110,"Skipping (too small)",f,0); */
	    return(0);
	}
    }
    if (nbu) {				/* Skipping backup files? */
	if (ckmatch(
#ifdef CKREGEX
		    "*.~[0-9]*~"	/* Not perfect but close enough. */
#else
		    "*.~*~"		/* Less close. */
#endif /* CKREGEX */
		    ,f,filecase,2+1)) {
	    debug(F110,"fileselect skipping backup",f,0);
	    return(0);
	}
    }
    for (n = 0; xlist && n < nxlist; n++) {
	if (!xlist[n]) {
	    debug(F101,"fileselect xlist empty",0,n);
	    break;
	}
	if (ckmatch(xlist[n],f,filecase,2+1)) {
	    debug(F111,"fileselect xlist",xlist[n],n);
	    debug(F110,"fileselect skipping",f,0);
	    return(0);
	}
    }
    debug(F110,"fileselect selecting",f,0);
    return(1);
}

/* C K R A D I X  --  Radix converter */
/*
   Call with:
     s:   a number in string format.
     in:  int, specifying the radix of s, 2-36.
     out: int, specifying the radix to convert to, 2-36.
   Returns:
     NULL on error (illegal radix, illegal number, etc.).
     "-1" on overflow (number too big for unsigned long).
     Otherwise: Pointer to result.
*/
#define RXRESULT 127
static char rxdigits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static char rxresult[RXRESULT+1];

char *
ckradix(s,in,out) char * s; int in, out; {
    char c, *r = rxresult;
    int d, minus = 0;
    unsigned long zz = 0L;
    long z;
    if (in < 2 || in > 36)		/* Verify legal input radix */
      return(NULL);
    if (out < 2 || out > 36)		/* and output radix. */
      return(NULL);
    if (*s == '+') {			/* Get sign if any */
	s++;
    } else if (*s == '-') {
	minus++;
	s++;
    }
    while (*s == SP || *s == '0')	/* Trim leading blanks or 0's */
      s++;
/*
  For detecting overflow, we use a signed copy of the unsigned long
  accumulator.  If it goes negative, we know we'll overflow NEXT time
  through the loop.
*/
    for (; *s;  s++) {			/* Convert from input radix to */
	c = *s;				/* unsigned long */
	if (islower(c)) c = toupper(c);
	if (c >= '0' && c <= '9')
	  d = c - '0';
	else if (c >= 'A' && c <= 'Z')
	  d = c - 'A' + 10;
	else
	  return(NULL);
	zz = zz * in + d;
	if (z < 0L)			/* Clever(?) overflow detector */
	  return("-1");
        z = zz;
    }
    if (!zz) return("0");
    r = &rxresult[RXRESULT];		/* Convert from unsigned long */
    *r-- = NUL;				/* to output radix. */
    while (zz > 0 && r > rxresult) {
	d = zz % out;
	*r-- = rxdigits[d];
	zz = zz / out;
    }
    if (minus) *r-- = '-';		/* Replace original sign */
    return((char *)(r+1));
}

#ifndef NOB64
/* Base-64 conversion routines */

static char b64[] = {			/* Encoding vector */
#ifdef pdp11
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="
#else
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S',
  'T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l',
  'm','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4',
  '5','6','7','8','9','+','/','=','\0'
#endif /* pdp11 */
};
static int b64tbl[] = {			/* Decoding vector */
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -2, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

/*
   B 8 T O B 6 4  --  Converts 8-bit data to Base64 encoding.

   Call with:
     s   = Pointer to 8-bit data;
     n   = Number of source bytes to encode (SEE NOTE).
           If it's a null-terminated string, you can use -1 here.
     out = Address of output buffer.
     len = Length of output buffer (should > 4/3 longer than input).

   Returns:
     >= 0 if OK, number of bytes placed in output buffer,
          with the subsequent byte set to NUL if space permits.
     -1 on error (output buffer space exhausted).

   NOTE:
     If this function is to be called repeatedly, e.g. to encode a data
     stream a chunk at a time, the source length must be a multiple of 3
     in all calls but the final one to avoid the generation of extraneous
     pad characters that would throw the decoder out of sync.  When encoding
     only a single string, this is not a consideration.  No internal state
     is kept, so there is no reset function.
*/
int
b8tob64(s,n,out,len) char * s,* out; int n, len; {
    int b3, b4, i, x = 0;
    unsigned int t;

    if (n < 0) n = strlen(s);

    for (i = 0; i < n; i += 3,x += 4) { /* Loop through source bytes */
	b3 = b4 = 0;
	t = (unsigned)((unsigned)((unsigned)s[i] & 0xff) << 8);
	if (n - 1 > i) {		/* Do we have another after this? */
            t |= (unsigned)(s[i+1] & 0xff); /* Yes, OR it in */
            b3 = 1;			/* And remember */
        }
        t <<= 8;			/* Move over */
        if (n - 2 > i) {		/* Another one after that? */
            t |= (unsigned)(s[i+2] & 0xff); /* Yes, OR it in */
            b4 = 1;			/* and remember */
        }
	if (x + 4 > len)		/* Check output space */
	  return(-1);
	out[x+3] = b64[b4 ? (t & 0x3f) : 64]; /* 64 = code for '=' */
        t >>= 6;
        out[x+2] = b64[b3 ? (t & 0x3f) : 64];
        t >>= 6;
        out[x+1] = b64[t & 0x3f];
        t >>= 6;
        out[x]   = b64[t & 0x3f];
    }
    if (x < len) out[x] = NUL;		/* Null-terminate the string */
    return(x);
}


/*
   B 6 4 T O B 8  --  Converts Base64 string to 8-bit data.

   Call with:
     s   = pointer to Base64 string (whitespace ignored).
     n   = length of string, or -1 if null terminated, or 0 to reset.
     out = address of output buffer.
     len = length of output buffer.

   Returns:
     >= 0 if OK, number of bytes placed in output buffer,
          with the subsequent byte set to NUL if space permits.
     <  0 on error:
       -1 = output buffer too small for input.
       -2 = input contains illegal characters.
       -3 = internal coding error.

   NOTE:
     Can be called repeatedly to decode a Base64 stream, one chunk at a
     time.  However, if it is to be called for multiple streams in
     succession, its internal state must be reset at the beginning of
     the new stream.
*/
int
b64tob8(s,n,out,len) char * s,* out; int len; {	/* Decode */
    static int bits = 0;
    static unsigned int r = 0;
    int i, k = 0, x, t;
    unsigned char c;

    if (n == 0) {			/* Reset state */
	bits = 0;
	r = 0;
	return(0);
    }
    x = (n < 0) ? strlen(s) : n;	/* Source length */

    n = ((x + 3) / 4) * 3;		/* Compute destination length */
    if (x > 0 && s[x-1] == '=') n--;	/* Account for padding */
    if (x > 1 && s[x-2] == '=') n--;
    if (n > len)			/* Destination not big enough */
      return(-1);			/* Fail */

    for (i = 0; i < x; i++) {		/* Loop thru source */
	c = (unsigned)s[i];		/* Next char */
        t = b64tbl[c];			/* Code for this char */
	if (t == -2) {			/* Whitespace or Ctrl */
	    n--;			/* Ignore */
	    continue;
	} else if (t == -1) {		/* Illegal code */
	    return(-2);			/* Fail. */
	} else if (t > 63 || t < 0)	/* Illegal value */
	  return(-3);			/* fail. */
	bits += 6;			/* Count bits */
	r <<= 6;			/* Make space */
	r |= (unsigned) t;		/* OR in new code */
	if (bits >= 8) {		/* Have a byte yet? */
	    bits -= 8;			/* Output it */
	    c = (unsigned) ((r >> bits) & 0xff);
	    out[k++] = c;
	}
    }
    if (k < len) out[k] = NUL;		/* Null-terminate in case it's */
    return(k);				/* a text string */
}
#endif /* NOB64 */

/* C H K N U M  --  See if argument string is an integer  */

/* Returns 1 if OK, zero if not OK */
/* If OK, string should be acceptable to atoi() */
/* Allows leading space, sign */

int
chknum(s) char *s; {			/* Check Numeric String */
    int x = 0;				/* Flag for past leading space */
    int y = 0;				/* Flag for digit seen */
    char c;
    debug(F110,"chknum",s,0);
    while (c = *s++) {			/* For each character in the string */
	switch (c) {
	  case SP:			/* Allow leading spaces */
	  case HT:
	    if (x == 0) continue;
	    else return(0);
	  case '+':			/* Allow leading sign */
	  case '-':
	    if (x == 0) x = 1;
	    else return(0);
	    break;
	  default:			/* After that, only decimal digits */
	    if (c >= '0' && c <= '9') {
		x = y = 1;
		continue;
	    } else return(0);
	}
    }
    return(y);
}


/*  R D I G I T S  -- Verify that all characters in arg ARE DIGITS  */

/*  Returns 1 if so, 0 if not or if string is empty */

int
rdigits(s) char *s; {
    if (!s) return(0);
    do {
        if (!isdigit(*s)) return(0);
        s++;
    } while (*s);
    return(1);
}

/*  P A R N A M  --  Return parity name */

char *
#ifdef CK_ANSIC
parnam(char c)
#else
parnam(c) char c;
#endif /* CK_ANSIC */
/* parnam */ {
    switch (c) {
	case 'e': return("even");
	case 'o': return("odd");
	case 'm': return("mark");
	case 's': return("space");
	case 0:   return("none");
	default:  return("invalid");
    }
}

char *					/* Convert seconds to hh:mm:ss */
#ifdef CK_ANSIC
hhmmss(long x)
#else
hhmmss(x) long x;
#endif /* CK_ANSIC */
/* hhmmss(x) */ {
    static char buf[10];
    long s, h, m;
    h = x / 3600L;			/* Hours */
    x = x % 3600L;
    m = x / 60L;			/* Minutes */
    s = x % 60L;			/* Seconds */
    if (x > -1L)
      sprintf(buf,"%02ld:%02ld:%02ld",h,m,s);
    else
      buf[0] = NUL;
    return((char *)buf);
}

/* L S E T  --  Set s into p, right padding to length n with char c; */
/*
   s is a NUL-terminated string.
   If length(s) > n, only n bytes are moved.
   The result is NOT NUL terminated unless c == NUL and length(s) < n.
   The intended of this routine is for filling in fixed-length record fields.
*/
VOID
lset(p,s,n,c) char *s; char *p; int n; int c; {
    int x;
#ifndef USE_MEMCPY
    int i;
#endif /* USE_MEMCPY */
    if (!s) s = "";
    x = strlen(s);
    if (x > n) x = n;
#ifdef USE_MEMCPY
    memcpy(p,s,x);
    if (n > x)
      memset(p+x,c,n-x);
#else
    for (i = 0; i < x; i++)
      *p++ = *s++;
    for (; i < n; i++)
      *p++ = c;
#endif /* USE_MEMCPY */
}

/* R S E T  --  Right-adjust s in p, left padding to length n with char c */

VOID
rset(p,s,n,c) char *s; char *p; int n; int c; {
    int x;
#ifndef USE_MEMCPY
    int i;
#endif /* USE_MEMCPY */
    if (!s) s = "";
    x = strlen(s);
    if (x > n) x = n;
#ifdef USE_MEMCPY
    memset(p,c,n-x);
    memcpy(p+n-x,s,x);
#else
    for (i = 0; i < (n - x); i++)
      *p++ = c;
    for (; i < n; i++)
      *p++ = *s++;
#endif /* USE_MEMCPY */
}

/*  U L O N G T O H E X  --  Unsigned long to hex  */

/*
  Converts unsigned long arg to hex and returns string pointer to
  rightmost n hex digits left padded with 0's.  Allows for longs
  up to 64 bits.  Returns pointer to result.
*/
char *
ulongtohex(z,n) unsigned long z; int n; {
    static char hexbuf[17];
    int i = 16, x, k = 0;
    hexbuf[16] = '\0';
    if (n > 16) n = 16;
    k = 2 * (sizeof(long));
    for (i = 0; i < n; i++) {
	if (i > k || z == 0) {
	    hexbuf[15-i] = '0';
	} else {
	    x = z & 0x0f;
	    z = z >> 4;
	    hexbuf[15-i] = x + ((x < 10) ? '0' : 0x37);
	}
    }
    return((char *)(&hexbuf[16-i]));
}

/*  H E X T O U L O N G  --  Hex string to unsigned long  */

/*
  Converts n chars from s from hex to unsigned long.
  Returns:
   0L or positive, good result (0L is returned if arg is NULL or empty).
  -1L on error: non-hex arg or overflow.
*/
long
hextoulong(s,n) char *s; int n; {
    char buf[64];
    unsigned long result = 0L;
    int d, count = 0, i;
    int flag = 0;
    if (!s) s = "";
    if (!*s) {
	return(0L);
    }
    if (n < 1)
      return(0L);
    if (n > 63) n = 63;
    strncpy(buf,s,n);
    buf[n] = '\0';
    s = buf;
    while (*s) {
	d = *s++;
	if ((d == '0' || d == ' ')) {
	    if (!flag)
	      continue;
	} else {
	    flag = 1;
	}
	if (islower(d))
	  d = toupper(d);
	if (d >= '0' && d <= '9') {
	    d -= 0x30;
	} else if (d >= 'A' && d <= 'F') {
	    d -= 0x37;
	} else {
	    return(-1L);
	}
	if (++count > (sizeof(long) * 2))
	  return(-1L);
	result = (result << 4) | (d & 0x0f);
    }
    return(result);
}

/* End of ckclib.c */
