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
typedef unsigned int size_t;

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
#define stat _stat
#define fileno _fileno
#define sys_errlist _sys_errlist
#define unlink _unlink
#define write _write
#define creat _creat
#define getpid _getpid
#define utime _utime
#define mktemp _mktemp
#define strnicmp _strnicmp
#define read _read
#define open _open
#define access _access
#define wcsdup _wcsdup
#define chmod _chmod
#define fstat _fstat
#define ftime _ftime
#endif /* __STDC__ */
#define isascii __isascii
#endif /* NT */

#define NOJC

/* For OS/2 debugging... */
#ifdef __IBMC__
#ifdef __DEBUG
#endif /* __DEBUG */
#endif /* __IBMC__ */
#endif /* CKOKER_H */
