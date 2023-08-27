/*
  Author: Jeffrey E Altman <jaltman@secure-endpoints.com>,
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

#ifndef CKOKER_H
#define CKOKER_H

#include <stdlib.h>
#include <string.h>
#ifndef _SIZE_T_DEFINED
#ifdef _WIN64
typedef unsigned __int64 size_t;
#else  /* _WIN64 */
typedef unsigned int size_t;
#endif  /* _WIN64 */
#define _SIZE_T_DEFINED
#endif /* _SIZE_T_DEFINED */

/* Like malloc, but calls fatal() if out of memory. */
void   *kmalloc(size_t size);

/* Like realloc, but calls fatal() if out of memory. */
void   *krealloc(void *ptr, size_t new_size);

/* Frees memory allocated using kmalloc or krealloc. */
void    kfree(void *ptr);

/* Allocates memory using kmalloc, and copies the string into that memory. */
char   *kstrdup(const char *str);

#define malloc(x) kmalloc(x)
#define realloc(x,y) krealloc(x,y)
#define free(x) kfree(x)
#define strdup(x) kstrdup(x)

#ifdef NT
#ifdef __STDC__
#define stricmp _stricmp
#define putenv _putenv
#define sopen _sopen
#define strupr _strupr
#define close _close
#ifndef __WATCOMC__
#ifndef __GNUC__
#define stat _stat
#endif /* __GNUC__ */
#define fileno _fileno
#ifndef __GNUC__
#define fstat _fstat
#define ftime _ftime
#endif /* __GNUC__ */
#define getpid _getpid
#endif /* __WATCOMC__ */
#define sys_errlist _sys_errlist
#define unlink _unlink
#define write _write
#define creat _creat
#ifndef __GNUC__
#ifdef _CRT_DECLARE_NONSTDC_NAMES
/* _CRT_DECLARE_NONSTDC_NAMES is only defined to work around an issue in the
 * OpenSSL 3.x headers which introduced a dependency on some non-standard
 * types, specifically off_t. This issue remains as of OpenSSL 3.0.10.
 *
 * On newer versions of Visual C++, defining _CRT_DECLARE_NONSTDC_NAMES will
 * bring in off_t along with _utime, so we don't want to define utime as _utime
 * in that case. But on older versions of Visual C++, _CRT_DECLARE_NONSTDC_NAMES
 * may not bring in the definition of off_t we need, so we'll define it below
 * if required.
 */
#ifndef _OFF_T_DEFINED
#define _OFF_T_DEFINED
typedef long _off_t;
typedef _off_t off_t;
#endif
#else
/* _CRT_DECLARE_NONSTDC_NAMES in sufficiently new versions of Visual C++
 * normally defines*/
#define utime _utime
#endif /* _CRT_DECLARE_NONSTDC_NAMES */
#endif /* __GNUC__ */
#define mktemp _mktemp
#define strnicmp _strnicmp
#define read _read
#define open _open
#define access _access
#define wcsdup _wcsdup
#define chmod _chmod
#endif /* __STDC__ */
#ifndef __WATCOMC__
#define isascii __isascii
#endif
#endif /* NT */

#define NOJC

/* For OS/2 debugging... */
#ifdef __IBMC__
#ifdef __DEBUG
#endif /* __DEBUG */
#endif /* __IBMC__ */
#endif /* CKOKER_H */
