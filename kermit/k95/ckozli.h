/*
  C K O Z L I . H --  ZLIB Interface Header for Kermit 95

  Copyright (C) 2001,2004 Trustees of Columbia University in the City of New
  York.  The C-Kermit software may not be, in whole or in part, licensed or
  sold for profit as a software product itself, nor may it be included in or
  distributed with commercial products or otherwise distributed by commercial
  concerns to their clients or customers without written permission of the
  Office of Kermit Development and Distribution, Columbia University.  This
  copyright notice must not be removed, altered, or obscured.

  Author:  Jeffrey E Altman (jaltman@secure-endpoints.com)
             Secure Endpoints Inc., New York City
*/

#ifdef OS2
#ifdef ZLIB
#include <zlib.h>

#define deflateInit_            ck_deflateInit_
#define deflateEnd              ck_deflateEnd
#define inflateInit_            ck_inflateInit_
#define inflateEnd              ck_inflateEnd
#define inflate                 ck_inflate
#define deflate                 ck_deflate

int ck_deflateInit_(z_streamp strm, int level, const char *version, int stream_size);
int ck_inflateInit_(z_streamp strm, const char *version, int stream_size);
int ck_deflateEnd(z_streamp strm);
int ck_inflateEnd(z_streamp strm);
int ck_inflate(z_streamp strm, int flush);
int ck_deflate(z_streamp strm, int flush);
#endif /* ZLIB */
#endif /* OS2 */
