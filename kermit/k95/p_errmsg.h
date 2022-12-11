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


U8 *os2_error_msg[] = {

  "Failed to open file or device: \"%s\"",
  "Failed to close file or device: \"%s\"",
  "Failed to read from file or device: \"%s\"",
  "Failed to write to file or device: \"%s\"",
  "Failed to control device: \"%s\"",
  "Failed to allocate %lu bytes of memory",
  "Failed to free memory",
  "Failed to create named pipe: \"%s\"",
  "Failed to connect to named pipe: \"%s\"",
  "Failed to disconnect named pipe: \"%s\"",
  "Failed to peek in named pipe: \"%s\"",
  "Failed to set handle state of named pipe: \"%s\"",
  "Failed to load module: \"%s\"",
  "Failed to free module: \"%s\"",
  "Failed to query address of procedure: \"%s\"",
  "Failed to change priority of process or thread",
  "Failed to read the keyboard"
};

U8 *tcpip_error_msg[] = {

  "Failed to initialize sockets",
  "Failed to control socket #%lu",
  "Failed to receive from socket #%lu",
  "Failed to send to socket #%lu",
  "Failed to create socket",
  "Failed to bind to socket #%lu",
  "Failed to listen to socket #%lu",
  "Failed to accept connection from socket #%lu",
  "Failed to resolve name of address \"%s\"",
  "Failed to resolve IP address of \"%s\"",
  "Failed to connect to socket #%lu",
  "Failed to set option for socket #%lu",
  "Failed to get option of socket #%lu",
  "Failed to select from socket #%lu"
};

