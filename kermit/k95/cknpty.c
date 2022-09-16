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
#include "ckcdeb.h"

#ifdef NT

#ifdef CK_CONPTY
static HPCON hPc = NULL;
static BOOL conPtyAvailable = FALSE, conPtyChecked = FALSE;
static HINSTANCE hkernel=NULL;

typedef HRESULT (WINAPI *CreatePseudoConsole_t)(
    _In_ COORD size,
    _In_ HANDLE hInput,
    _In_ HANDLE hOutput,
    _In_ DWORD dwFlags,
    _Out_ HPCON* phPC
);

typedef void (WINAPI *ClosePseudoConsole_t)(
    _In_ HPCON hPC
);

typedef void (WINAPI *ResizePseudoConsole_t)(
    _In_ HPCON hPC,
    _In_ COORD size
);

typedef BOOL (WINAPI *InitializeProcThreadAttributeList_t)(
    _Out_writes_bytes_to_opt_(*lpSize,*lpSize) LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    _In_ DWORD dwAttributeCount,
    _Reserved_ DWORD dwFlags,
    _When_(lpAttributeList == nullptr,_Out_) _When_(lpAttributeList != nullptr,_Inout_) PSIZE_T lpSize
);


typedef BOOL (WINAPI *UpdateProcThreadAttribute_t) (
    _Inout_ LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    _In_ DWORD dwFlags,
    _In_ DWORD_PTR Attribute,
    _In_reads_bytes_opt_(cbSize) PVOID lpValue,
    _In_ SIZE_T cbSize,
    _Out_writes_bytes_opt_(cbSize) PVOID lpPreviousValue,
    _In_opt_ PSIZE_T lpReturnSize
);

/* These require at least Windows 10 version 1809 */
CreatePseudoConsole_t p_CreatePseudoConsole;
ClosePseudoConsole_t p_ClosePseudoConsole;
ResizePseudoConsole_t p_ResizePseudoConsole;

/* And these require at least Windows Vista */
InitializeProcThreadAttributeList_t p_InitializeProcThreadAttributeList;
UpdateProcThreadAttribute_t p_UpdateProcThreadAttribute;

void load_conpty() {
    FARPROC p;
    /* Only bother doing this load library business once */
    conPtyChecked = TRUE;

    hkernel = LoadLibrary("kernel32.dll");

    p = GetProcAddress(hkernel, "CreatePseudoConsole");
    if (p == NULL) {
        conPtyAvailable = FALSE;
        return;
    }
    p_CreatePseudoConsole = (CreatePseudoConsole_t)p;

    p = GetProcAddress(hkernel, "ClosePseudoConsole");
    p_ClosePseudoConsole = (ClosePseudoConsole_t)p;

    p = GetProcAddress(hkernel, "ResizePseudoConsole");
    p_ResizePseudoConsole = (ResizePseudoConsole_t)p;

    p = GetProcAddress(hkernel, "InitializeProcThreadAttributeList");
    p_InitializeProcThreadAttributeList = (InitializeProcThreadAttributeList_t)p;

    p = GetProcAddress(hkernel, "UpdateProcThreadAttribute");
    p_UpdateProcThreadAttribute = (UpdateProcThreadAttribute_t)p;

    conPtyAvailable = TRUE;
}

#endif

BOOL pseudo_console_available() {
#ifdef CK_CONPTY
    if (!conPtyChecked) {
        load_conpty();
    }
    return conPtyAvailable;
#else
    return FALSE;
#endif
}

BOOL create_pipes(PHANDLE inputReader, PHANDLE inputWriter,
                  PHANDLE outputReader, PHANDLE outputWriter) {
    if (!CreatePipe(inputReader, inputWriter, NULL, 0))
    {
        // Create pipe failed!
        CloseHandle(inputReader); inputReader = NULL;
        CloseHandle(inputWriter); inputWriter = NULL;
        return FALSE;
    }

    if (!CreatePipe(outputReader, outputWriter, NULL, 0))
    {
        // Create pipe failed!
        CloseHandle(inputReader); inputReader = NULL;
        CloseHandle(inputWriter); inputWriter = NULL;
        CloseHandle(outputReader); outputReader = NULL;
        CloseHandle(outputWriter); outputWriter = NULL;
        return FALSE;
    }

    return TRUE;
}

HRESULT open_pseudo_console(COORD size, HANDLE input_pipe, HANDLE output_pipe)
{
    HRESULT hr = S_OK;
#ifdef CK_CONPTY
    hr = p_CreatePseudoConsole(size, input_pipe, output_pipe, 0, &hPc);
#endif
    return hr;
}

#ifdef CK_CONPTY
HRESULT prepare_startup_info(STARTUPINFOEX * psi)
{
    // Prepare Startup Information structure
    STARTUPINFOEX si;
    ZeroMemory(&si, sizeof(si));
    si.StartupInfo.cb = sizeof(STARTUPINFOEX);

    // Discover the size required for the list
    size_t bytesRequired;
    p_InitializeProcThreadAttributeList(NULL, 1, 0, &bytesRequired);

    // Allocate memory to represent the list
    si.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(
            GetProcessHeap(), 0, bytesRequired);
    if (!si.lpAttributeList)
    {
        return E_OUTOFMEMORY;
    }

    // Initialize the list memory location
    if (!p_InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &bytesRequired))
    {
        HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Set the pseudoconsole information into the list
    if (!p_UpdateProcThreadAttribute(si.lpAttributeList,
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

    *psi = si;

    return S_OK;
}
#endif

BOOL start_subprocess_in_pty(COORD size, LPSTR lpCommandLine,
                             LPPROCESS_INFORMATION lpProcessInformation,
                             PHANDLE hInputWriter, PHANDLE hOutputReader ) {
#ifdef CK_CONPTY
    HANDLE hInputReader, hOutputWriter;
    STARTUPINFOEX startupInfo;
    BOOL result;
    HRESULT hResult;

    result = create_pipes(&hInputReader, hInputWriter,
                           hOutputReader, &hOutputWriter);
    if (!result) {
        debug(F100, "Failed to create I/O pipes!", "", 0);
        return FALSE;
    }

    hResult = open_pseudo_console(size, hInputReader, hOutputWriter);

    // These are no longer required
    CloseHandle(hInputReader); hInputReader = NULL;
    CloseHandle(hOutputWriter); hOutputWriter = NULL;

    if (FAILED(hResult)) {
        debug(F101, "ConPTY open failed with result", "", result);
        CloseHandle(hInputWriter); hInputWriter = NULL;
        CloseHandle(hOutputReader); hOutputReader = NULL;
        return FALSE;
    }

    memset(&startupInfo, 0, sizeof(STARTUPINFOEX));
    hResult = prepare_startup_info(&startupInfo);

    if (FAILED(hResult)) {
        debug(F101, "Closing PTY; Prepare startup info failed with result", "", result);
        close_pseudo_console();
        CloseHandle(hInputWriter); hInputWriter = NULL;
        CloseHandle(hOutputReader); hOutputReader = NULL;
        return FALSE;
    }

    result = CreateProcess(
                     NULL,                         /* application name */
                     lpCommandLine,                /* command line */
                     NULL,                         /* process security attributes */
                     NULL,                         /* primary thread security attrs */
                     FALSE,                        /* inherit handles */
                     EXTENDED_STARTUPINFO_PRESENT, /* creation flags */
                     NULL,                         /* use parent's environment */
                     NULL,                         /* use parent's current directory */
                     &startupInfo.StartupInfo,     /* startup info */
                     lpProcessInformation ) ;      /* process info */

   if (!result) {
        debug(F100, "Create process failed", "", 0);
        close_pseudo_console();
        CloseHandle(hInputWriter); hInputWriter = NULL;
        CloseHandle(hOutputReader); hOutputReader = NULL;
        return FALSE;
   }

   debug(F100, "Subprocess started successfully", "", 0);

   return TRUE;
#else
   return FALSE;
#endif
}

void resize_pseudo_console(COORD new_size) {
#ifdef CK_CONPTY
    p_ResizePseudoConsole(hPc, new_size);
#endif
}

void close_pseudo_console() {
#ifdef CK_CONPTY

    p_ClosePseudoConsole(hPc);

#endif
}

#endif