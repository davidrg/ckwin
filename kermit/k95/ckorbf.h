/* A simple ring-buffer implementation for the SSH Subsystem.
 *
 * Kermit 95 SSH Subsystem
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

#ifndef CKWIN_RINGBUF_H
#define CKWIN_RINGBUF_H

#include <windows.h>

typedef struct ring_buffer_t ring_buffer_t;
typedef ring_buffer_t* ring_buffer_handle_t;


#define RING_BUFFER_SUCCESS 0
#define RING_BUFFER_TIMEOUT 1
#define RING_BUFFER_TRY_AGAIN 2
#define RING_BUFFER_LOCK_ERROR 3
#define RING_BUFFER_NO_RING_BUFFER 4

/** Creates a new ring buffer. Pass to ring_buffer_free() to delete it.
 *
 * @param max_length Maximum size for the ring buffer.
 * @return A new ring buffer handle.
 */
ring_buffer_handle_t ring_buffer_new(size_t max_length);

/** Deletes a ring buffer.
 *
 * @param buf Ring buffer to delete.
 */
void ring_buffer_free(ring_buffer_handle_t buf);

/** Checks if the buffer is full returning the result
 *
 * @param buf Buffer to check the fullness of
 * @return TRUE if full
 */
BOOL ring_buffer_is_full(ring_buffer_handle_t buf);

/** Checks if the buffer is empty returning the result.
 *
 * @param buf Buffer to check emptiness of
 * @return TRUE if empty
 */
BOOL ring_buffer_is_empty(ring_buffer_handle_t buf);

/** Gets the amount of free space in the ring buffer
 *
 * @param buf Buffer to get free space of
 * @return Number of bytes available in the ring buffer or 0 if full.
 */
size_t ring_buffer_free_space(ring_buffer_handle_t buf);

/** Get the number of elements in the buffer.
 *
 * @param buf Ring buffer to get the length of
 * @return Number of elements in the ring buffer or 0 if empty.
 */
size_t ring_buffer_length(ring_buffer_handle_t buf);

/** Appends a single element to the ring buffer if there is free space.
 *
 * @param buf Buffer to append to
 * @param data Data to append
 * @return TRUE on success, FALSE if the buffer is full.
 */
BOOL ring_buffer_put(ring_buffer_handle_t buf, char data);

/** Appends a single element to the ring buffer if there is free space. If there
 * is not sufficient free space the call blocks until the timeout expires or
 * free space becomes available.
 *
 * You do NOT need to lock the ring buffer before calling this function - that
 * will be handled automatically.
 *
 * The function may return RING_BUFFER_TRY_AGAIN if the buffer was unexpectedly
 * filled between checking for write ready and attempting the write. In this
 * instance you can try the call again or just treat it like a timeout.
 *
 * @param buf Buffer to append to
 * @param data Data to append
 * @param timeout Maximum time to wait for free space to become available in
 *                milliseconds. Pass INFINITE to wait forever.
 * @return RING_BUFFER_SUCCESS on success, RING_BUFFER_TIMEOUT on timeout,
 *         or any other value on error.
 */
int ring_buffer_put_blocking(ring_buffer_handle_t buf, char data, int timeout);

/** Gets a single element from the ring buffer if it is not empty.
 *
 * @param buf Buffer to get one element from
 * @param data Output element
 * @return TRUE on success, FALSE if the buffer is empty.
 */
BOOL ring_buffer_get(ring_buffer_handle_t buf, char *data);

/** Gets a single element from the ring buffer. If the buffer is empty the call
 * blocks until data becomes available or the timeout expires.
 *
 * You do NOT need to lock the ring buffer before calling this function - that
 * will be handled automatically.
 *
 * Note that a timeout is still possible when requesting an infinite timeout.
 *
 * @param buf Buffer to get one element from
 * @param data Output element
 * @param timeout Maximum time to wait for data to become available in
 *                milliseconds. Pass INFINITE to wait forever.
 * @return RING_BUFFER_SUCCESS on success, RING_BUFFER_TIMEOUT on timeout,
 *         or any other value on error.
 */
int ring_buffer_get_blocking(ring_buffer_handle_t buf, char *data, int timeout);

/** Writes multiple elements to the ring buffer returning the total count
 * written which may be less than count requested.
 *
 * @param buf Ring buffer to write to
 * @param data Data to write to the ring buffer
 * @param length Maximum amount of data to be written
 * @return Amount that was written
 */
size_t ring_buffer_write(ring_buffer_handle_t buf, char* data, size_t length);

/** Reads zero or more bytes from the ring buffer up to the specified maximum
 * length returning the total number of bytes read.
 *
 * @param buf Ring buffer to read from
 * @param data Buffer to read into
 * @param max_length Maximum number of bytes to read (length of buf)
 * @return Number of bytes that were read
 */
size_t ring_buffer_read(ring_buffer_handle_t buf, char* data, size_t max_length);

/** Reads zero or more bytes from the ring buffer up to the specified maximum
 * length returning the total number of bytes read. It does *not* alter the
 * state of the ring buffer - the data read remains on the ring buffer until
 * it is read again with ring_buffer_read() or until it is removed with
 * ring_buffer_consume()
 *
 * @param buf Ring buffer to read from
 * @param data Buffer to read into
 * @param max_length Maximum number of bytes to read (length of buf)
 * @return Number of bytes that were read
 */
size_t ring_buffer_peek(ring_buffer_handle_t buf, char* data, size_t max_length);

/** Consumes the specified number of bytes fron the ring buffer. The consumed
 * data is just discarded. Useful to remove any bytes you are interested in from
 * a call to ring_buffer_peek().
 *
 * @param buf Ring buffer to consume from
 * @param length Number of bytes to consume
 * @return If the consume was successful or not.
 */
BOOL ring_buffer_consume(ring_buffer_handle_t buf, size_t length);

/** Locks a ring buffer for exclusive access via a mutex.
 *
 * @param buf Buffer to request mutex for
 * @param msTimeout Timeout in milliseconds. Pass INFINITE to not timeout.
 * @returns TRUE on success, FALSE on timeout.
 */
BOOL ring_buffer_lock(ring_buffer_handle_t buf, DWORD msTimeout);

/** Releases a lock on the ring buffer so other threads can work with it
 *
 * @param buf Buffer to release the mutex on
 */
void ring_buffer_unlock(ring_buffer_handle_t buf);

/** Causes any waiting calls to ring_buffer_get_blocking or
 * ring_buffer_put_blocking to immediately fail. This is to allow a failing
 * writer or writer to signal to users of the blocking get/put calls that
 * future calls may block forever asn the reader or writer is no longer
 * servicing the buffer.
 *
 * This function blocks until the ring buffer mutex can be acquired
 *
 * @param buf Buffer to signal an error on.
 * @param error Error code to signal. Should be negative.
 */
void ring_buffer_signal_error(ring_buffer_handle_t buf, int error);

/** Gets any error code that was set on the ring buffer via
 * ring_buffer_signal_error.
 *
 * This function blocks until the ring buffer mutex can be acquired.
 *
 * @param buf
 */
void ring_buffer_get_error(ring_buffer_handle_t buf);

/** Gets an event that will be set whenever data is written to the
 * ring buffer.
 *
 * @param buf Buffer to get an event for
 * @return An event
 */
HANDLE ring_buffer_get_ready_read_event(ring_buffer_handle_t buf);

#endif /* CKWIN_RINGBUF_H */
