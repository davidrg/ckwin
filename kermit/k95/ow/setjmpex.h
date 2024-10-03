/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the w64 mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within this package.
 */
 
 /* Note: Open Watcom is missing this file. It has been adjusted from the mingw version to work with Open Watcom. */
 
#ifndef _SETJMPEX_H_INCLUDED_
#define _SETJMPEX_H_INCLUDED_

#ifndef _WIN32
#error Only Win32 target is supported!
#endif /* not _WIN32 */

#if (defined(__X86__) && !defined(__x86_64))
#define setjmp _setjmp
#define longjmp _longjmpex
#else /* Not x86*/
#ifdef setjmp
#undef setjmp
#endif /* ifdef setjmp */
#define setjmp _setjmpex
#endif /* Else */

#include <setjmp.h>
#endif /* _SETJMPEX_H_INCLUDED_ */
