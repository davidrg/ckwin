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

/* More friendlier interface to OS/2's DosAllocMem and DosFreeMem API calls */

#include "ckcdeb.h"
#ifndef NOXFER

#ifdef OS2
#ifdef NT
#include <windows.h>
#else /* NT */
#define INCL_DOSMEMMGR
#include <os2.h>
#undef COMMENT
#endif /* NT */
#else /* OS2 */
#include <stdlib.h>
#endif /* OS2 */


#include "p.h"
#include "p_type.h"
#include "p_error.h"

VOID
#ifdef CK_ANSIC
p_omalloc(void **buf, U32 size, U32 module, U32 line)
#else
p_omalloc(buf,size,module,line)
     void **buf; U32 size; U32 module; U32 line;
#endif
{
  APIRET rc=0;

  if ( buf == NULL )
      return;

#ifdef OS2
#ifdef NT
   *buf = GlobalAlloc( 0, size ) ;
   if ( !buf ) rc = GetLastError() ;
#else /* NT */
   rc = DosAllocMem(buf, size, PAG_COMMIT | PAG_WRITE | PAG_READ);
#endif /* NT */
#else /* OS2 */
   *buf = malloc(size);
   if ( !*buf ) rc = 1 ;
#endif /* OS2 */

  if (rc)
    os2_error(P_ERROR_DOSALLOCMEM, rc, module, line, NULL);
}

VOID
#ifdef CK_ANSIC
p_ofree(void **buf, U32 module, U32 line)
#else
p_ofree(buf, module,line)
     void **buf; U32 module; U32 line;
#endif
{
    APIRET rc=0;

    if ( *buf == NULL )
        return;

#ifdef OS2
#ifdef NT
   if ( GlobalFree(*buf) )
      rc = GetLastError() ;
#else /* NT */
   rc = DosFreeMem(*buf);
#endif /* NT */
#else /* OS2 */
    rc = free(*buf);
#endif /* OS2 */
  if (rc)
    os2_error(P_ERROR_DOSFREEMEM, rc, module, line, 0);
  *buf = NULL;
}
#endif /* NOXFER */
