/*
 * 16bit version of ncbpost() for compatibility with Open Watcom
 */

#ifdef _M_IX86
#define APIRET		USHORT
#define APIRET16	USHORT
#define CDECL16         FAR
#define APIENTRY16	APIENTRY
#endif

#include "ckcdeb.h"
#include "ckcker.h"
#include "ckonbi.h"

#ifdef __WATCOMC__

#pragma stack16(256)
VOID FAR _cdecl ncbpost(USHORT Junk, PNCB16 NcbPointer)
{
   APIRET rc = 0 ;
   NCB ncb = *NcbPointer ;
   rc = Dos16SemClear(NcbPointer->basic_ncb.ncb_semaphore);

}

#endif
#endif
