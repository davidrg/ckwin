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
   These are much more smaller than "unsigned long" etc to write and
   make porting to systems with different word sizes a lot easier...
*/

#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

#ifdef NT
#include <io.h>
#endif

#ifndef CK_HAVE_INTPTR_T
/* Any windows compiler too old to support this will be 32-bits (or less) */
typedef int intptr_t;
#define CK_HAVE_INTPTR_T
#endif

typedef unsigned long   U32;
typedef long            S32;
typedef unsigned short  U16;
typedef short           S16;
typedef unsigned char   U8;
typedef char            S8;
#ifndef NT
typedef unsigned long   BOOLEAN;
#endif

#ifdef XYZ_DLL
#ifdef NT
#ifndef __WATCOMC__
#if _MSC_VER > 800
/* Visual C++ 1.0 32-bit edition (MSC_VER==800) and OpenWatcom don't like this */
typedef long APIRET ;
#endif
#endif /* __WATCOMC__ */
#define DosSleep Sleep
#endif
#else /* XYZ_DLL */
typedef long APIRET ;
#define DosSleep msleep
#endif /* XYZ_DLL */

#ifdef XYZ_DLL
#ifndef CK_ANSIC
#define CK_ANSIC
#endif /* CK_ANSIC */
#endif /* XYZ_DLL */

#ifdef OS2
#ifndef VOID
#define VOID void
#endif /* VOID */
#endif /* OS2 */

#ifndef OS2
#define _System
#endif /* OS2 */

#ifndef _PROTOTYP
#ifdef CK_ANSIC
#define _PROTOTYP( func, parms ) func parms
#else /* Not ANSI C */
#define _PROTOTYP( func, parms ) func()
#endif /* CK_ANSIC */
#endif /* _PROTOTYP */
#endif /* _TYPEDEFS_H_ */
