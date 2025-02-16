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

#ifndef _CKNPTY_H
#define _CKNPTY_H

#ifdef NT

#ifdef CK_WS2
#include <winsock2.h>
#endif
#include <windows.h>
#include <winbase.h>

/** Checks if this version of Windows supports pseudo consoles.
 *
 *
 */
BOOL pseudo_console_available();
#ifdef CK_CONPTY
/** Creates pipes for communicating with the PTY.
 *
 * inputReader and outputWriter should be assigned to the pseudo
 * console (as input_pipe and output_pipe) then closed.
 *
 * inputWriter can be written to to send data to the subprocess
 * and outputReader can be read from to get output from the
 * subprocess.
 */
BOOL create_pipes(PHANDLE inputReader, PHANDLE inputWriter,
                  PHANDLE outputReader, PHANDLE outputWriter);

/** Creates a pseudo console of the specified size using the supplied
 * pipes for communication
 *
 * Parameters:
 * size: Size of the console in characters
 * input_pipe: Pipe the subprocess will read from
 * output_pipe: Pipe the subprocess will write to
 * Pipes can be closed after calling CreateProcess
 */
HRESULT open_pseudo_console(COORD size, HANDLE input_pipe, HANDLE output_pipe);

/** Prepares the supplied STARTUPINFOEX for use with
 * the already crated PTY (call open_pseudo_console first)
 *
 * psi: a STARTUPINFOEX
 */
HRESULT prepare_startup_info(STARTUPINFOEX * psi);

/** Starts a subprocess in a pseudoconsole of the specified size
 * and provides handles to communicate with it.
 *
 * size                  IN     Size of the pseudo console
 * lpCommandLine         IN     Command line to execute (command+parameters)
 * lpProcessInformation  OUT    Process information
 * hInputWriter          OUT    Handle to send input to the subprocess
 * hOutputReader         OUT    Handle to read output from the subprocess
 *
 * Returns: TRUE on success, FALSE on failure.
 */
BOOL start_subprocess_in_pty(COORD size, LPSTR lpCommandLine,
                             LPPROCESS_INFORMATION lpProcessInformation,
                             PHANDLE hInputWriter, PHANDLE hOutputReader );

/** Changes the pseudo consoles size to that specified
 *
 * size: New size for the console
 */
void resize_pseudo_console(COORD new_size);

/** Closes the pseudo console. This will kill any attached subprocesses.
 *
 * WARNING:
 * The ouput pipe *MUST* continue to be serviced until broken by the
 * terminating subprocess otherwise a deadlock may occur.
 */
void close_pseudo_console();
#endif
#endif /* NT */

#endif /* _CKNPTY_H */
