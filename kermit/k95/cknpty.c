/*
 * Windows ConPTY Interface for C-Kermit
 * Copyright (C) 2022, David Goodwin <david@zx.net.nz>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  + Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  + Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  + Neither the name of Columbia University nor the names of any
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef CK_CONPTY
/* Needed for PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE
 * to appear in the windows headers (not sure why NTDDI_VERSION is set to
 * some lower version by default) */
#define NTDDI_VERSION NTDDI_WIN10_RS5
#endif

#include "cknpty.h"


#ifdef NT

#ifdef CK_CONPTY
static HPCON hPc = NULL;
static BOOL conPtyAvailable = FALSE, conPtyChecked = FALSE;
#endif

BOOL pseudo_console_available() {
#ifdef CK_CONPTY
    HINSTANCE hLib;
    FARPROC procAddress;

    if (conPtyChecked) {
        return conPtyAvailable;
    }

    /* Only bother doing this load library business once */
    conPtyChecked = TRUE;

    hLib = LoadLibrary(TEXT("Kernel32.dll"));

    if (hLib == NULL) {
        return FALSE; /* Failed to load library - not available */
    }

    procAddress = (FARPROC) GetProcAddress(hLib, "CreatePseudoConsole");

    conPtyAvailable = NULL != procAddress;

    FreeLibrary(hLib);

    return conPtyAvailable;
#else
    return FALSE;
#endif
}


HRESULT open_pseudo_console(COORD size, HANDLE input_pipe, HANDLE output_pipe)
{
    HRESULT hr = S_OK;
#ifdef CK_CONPTY
    hr = CreatePseudoConsole(size, input_pipe, output_pipe, 0, &hPc);
#endif
    return hr;
}

#if _MSC_VER >= 1900
HRESULT prepare_startup_info(STARTUPINFOEX * psi)
{
    // Prepare Startup Information structure
    STARTUPINFOEX si;
    ZeroMemory(&si, sizeof(si));
    si.StartupInfo.cb = sizeof(STARTUPINFOEX);
#ifdef CK_CONPTY
    // Discover the size required for the list
    size_t bytesRequired;
    InitializeProcThreadAttributeList(NULL, 1, 0, &bytesRequired);

    // Allocate memory to represent the list
    si.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(
            GetProcessHeap(), 0, bytesRequired);
    if (!si.lpAttributeList)
    {
        return E_OUTOFMEMORY;
    }

    // Initialize the list memory location
    if (!InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &bytesRequired))
    {
        HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Set the pseudoconsole information into the list
    if (!UpdateProcThreadAttribute(si.lpAttributeList,
                                   0,
                                   PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                                   hPc,
                                   sizeof(hPc),
                                   NULL,
                                   NULL))
    {
        HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
        return HRESULT_FROM_WIN32(GetLastError());
    }
#endif
    *psi = si;

    return S_OK;
}
#endif

void resize_pseudo_console(COORD new_size) {
#ifdef CK_CONPTY
    ResizePseudoConsole(hPc, new_size);
#endif
}

void close_pseudo_console() {
#ifdef CK_CONPTY

    ClosePseudoConsole(hPc);

#endif
}

#endif