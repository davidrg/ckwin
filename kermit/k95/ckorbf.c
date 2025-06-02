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

#include "ckcdeb.h"
#include "ckorbf.h"

#ifdef SSH_DLL
int debug_logging();  /* defined in ckossh.c */
#undef debug
#define debug(a,b,c,d) \
((void)(debug_logging()?dodebug(a,b,(char *)(c),(CK_OFF_T)(d)):0))
#endif /* SSH_DLL */

struct ring_buffer_t {
    char* buffer;
    size_t head;
    size_t tail;
    size_t size;
    BOOL full;
    HANDLE mutex;

    /* Error is set by ring_buffer_signal_error. If it is non-zero then all
     * waiting calls to ring_buffer_put_bocking and ring_buffer_get_blocking
     * will fail immediately. */
    int error;

    /* These are manual-reset events the blocking read/write functions use to
     * know when data (or free space) becomes available. writeReady is always
     * in the signalled state when the buffer is non-full and readReady is
     * always in the signalled state when the buffer is non-empty */
    HANDLE readReady;
    HANDLE writeReady;
};


ring_buffer_handle_t ring_buffer_new(size_t max_length) {
    debug(F111, "ringbuf - new", "length", max_length);
    ring_buffer_handle_t buf = malloc(sizeof(ring_buffer_t));
    buf->buffer = malloc(max_length * sizeof(char));
    if (buf->buffer == NULL) {
        debug(F111, "ringbuf - allocate buffer failed", "length", max_length);
        free(buf);
        return NULL;
    }
    buf->size = max_length;
    buf->head = 0;
    buf->tail = 0;
    buf->full = FALSE;
    buf->error = 0;
    buf->mutex = CreateMutex(
            NULL,   /* default security attributes */
            FALSE,  /* initially not owned */
            NULL);  /* unnamed */
    if (buf->mutex == NULL) {
        debug(F100, "Failed to create mutex for ringbuf", "", 0);
        free(buf->buffer);
        free(buf);
        return NULL;
    }
    buf->readReady = CreateEvent(NULL, /* Security Attributes */
                                 TRUE, /* Manual Reset */
                                 FALSE, /* Initial state (non-signaled) */
                                 NULL); /* Name */
    buf->writeReady = CreateEvent(NULL, /* Security Attributes */
                                  TRUE, /* Manual Reset */
                                  TRUE, /* Buffer starts out ready for writing */
                                  NULL); /* Name */
    if (buf->readReady == NULL || buf->writeReady == NULL) {
        debug(F100, "Failed to create read/write ready events", "", 0);
        CloseHandle(buf->readReady);
        CloseHandle(buf->writeReady);
        CloseHandle(buf->mutex);
        free(buf->buffer);
        free(buf);
        return NULL;
    }

    return buf;
}


void ring_buffer_free(ring_buffer_handle_t buf) {
    if (buf == NULL) return;

    debug(F100, "ringbuf - free", NULL, 0);

    /* Let anyone waiting on the ring buffer know its about to go away */
    ring_buffer_signal_error(buf, RING_BUFFER_NO_RING_BUFFER);

    /* And make sure we have exclusive access before we go deleting things */
    if (ring_buffer_lock(buf, INFINITE)) {
        if (buf->buffer != NULL) {
            free(buf->buffer);
            buf->buffer = NULL;
        }
        if (buf->readReady != NULL) {
            HANDLE h = buf->readReady;
            buf->readReady = NULL;
            CloseHandle(h);
        }
        if (buf->writeReady != NULL) {
            HANDLE h = buf->writeReady;
            buf->writeReady = NULL;
            CloseHandle(h);
        }
    }

    if (buf->mutex != NULL) {
        HANDLE h = buf->mutex;
        buf->mutex = NULL;
        CloseHandle(h);
    }
    free(buf);
}


BOOL ring_buffer_is_full(ring_buffer_handle_t buf) {
    if (buf == NULL) return FALSE;

    return buf->full;
}


BOOL ring_buffer_is_empty(ring_buffer_handle_t buf) {
    if (buf == NULL) return FALSE;

    return !buf->full && (buf->head == buf->tail);
}


size_t ring_buffer_free_space(ring_buffer_handle_t buf) {
    if (ring_buffer_is_empty(buf))
        return buf->size; /* Empty - all space is free */

    return buf->size - ring_buffer_length(buf);
}


size_t ring_buffer_length(ring_buffer_handle_t buf) {
    if (buf == NULL) return 0; /* No buffer - no elements */

    if (buf->full)
        return buf->size; /* Full - all space is data */

    if (buf->head >= buf->tail) {
        return (buf->head - buf->tail);
    } else {
        return buf->size + buf->head - buf->tail;
    }
}


BOOL ring_buffer_put_noset(ring_buffer_handle_t buf, char data) {
    if (ring_buffer_is_full(buf)) {
        /* Buffer is full - can not put without overwriting data. Put fails. */
        return FALSE;
    }

    if (buf->error) {
        /* An error was signalled. Fail. */
        return FALSE;
    }

    /* Store the value */
    buf->buffer[buf->head] = data;

    /* And advance the pointer wrapping if necessary */
    if (++(buf->head) == buf->size) {
        buf->head = 0;
    }

    buf->full = buf->head == buf->tail;

    return TRUE;
}

/** Sets buffer writeReady and readReady events
 *
 * @param buf Buffer to set events for
 */
void ring_buffer_set_events(ring_buffer_handle_t buf) {
    if (buf->writeReady != NULL) {
        if (buf->full) {
            /* Buffer is now full - no longer ready to write */
            ResetEvent(buf->writeReady);
        } else {
            /* Buffer has free space. If anyone was waiting to write they can do
             * so now */
            SetEvent(buf->writeReady);
        }
    }

    if (buf->readReady != NULL) {
        if (ring_buffer_is_empty(buf)) {
            /* Buffer is empty - not ready for reading */
            ResetEvent(buf->readReady);
        } else {
            /* If anyone was waiting for data to read, data is now available */
            SetEvent(buf->readReady);
        }
    }
}


BOOL ring_buffer_put(ring_buffer_handle_t buf, char data) {
    BOOL result;

    result = ring_buffer_put_noset(buf, data);
    if (!result) return result;

    ring_buffer_set_events(buf);

    return result;
}


int ring_buffer_put_blocking(ring_buffer_handle_t buf, char data, int timeout) {
    int rc;

    if (buf->writeReady == NULL) return RING_BUFFER_NO_RING_BUFFER;

    /* Wait for the writeReady event. This is always signalled when the buffer
     * is non-full, so we may not have to spend any time waiting. */
    rc = WaitForSingleObject(buf->writeReady, timeout);
    if (rc == WAIT_TIMEOUT) {
        return RING_BUFFER_TIMEOUT; /* Timeout */
    } else if (rc == WAIT_FAILED) {
        return RING_BUFFER_LOCK_ERROR;
    }

    if (buf->error) {
        /* An error was signalled. Fail. */
        return buf->error;
    }

    /* Buffer should have free space now */
    if (ring_buffer_lock(buf, timeout)) {
        BOOL success = ring_buffer_put(buf, data);
        ring_buffer_unlock(buf);

        if (!success) {
            /* Ring buffer was filled between checking for write ready and
             * trying to write */
            debug(F100, "ring buffer put - try again!", "", 0);
            return RING_BUFFER_TRY_AGAIN;
        }
    } else {
        /* Timeout trying to acquire a lock on the buffer to even check its
         * status */
        return RING_BUFFER_TIMEOUT;
    }

    return RING_BUFFER_SUCCESS;
}

BOOL ring_buffer_get_noset(ring_buffer_handle_t buf, char *data) {
    if (ring_buffer_is_empty(buf)) {
        /* Buffer is empty - nothing to get. Get fails. */
        ResetEvent(buf->readReady);
        return FALSE;
    }

    /* Fetch the next value from the buffer */
    *data = buf->buffer[buf->tail];

    /* Then advance the tail pointer wrapping if necessary */
    if (++(buf->tail) == buf->size) {
        buf->tail = 0;
    }

    /* If it was full before it is no longer. Ready to write! */
    buf->full = FALSE;

    return TRUE;
}

BOOL ring_buffer_get(ring_buffer_handle_t buf, char *data) {
    BOOL result;

    result = ring_buffer_get_noset(buf, data);

    if (!result) return result;

#ifdef COMMENT
    if (buf->writeReady == NULL) return FALSE;
    SetEvent(buf->writeReady);

    if (ring_buffer_is_empty(buf)) {
        /* We got the last element from the buffer. Its now empty and no longer
         * ready for reading */
        ResetEvent(buf->readReady);
    }
#endif

    ring_buffer_set_events(buf);

    return result;
}


int ring_buffer_get_blocking(ring_buffer_handle_t buf, char *data, int timeout) {
    int rc;

    if (buf == NULL) return RING_BUFFER_NO_RING_BUFFER;

    if (buf->readReady == NULL) return RING_BUFFER_NO_RING_BUFFER;

    /* Wait for the readReady event. This is always signalled when the buffer
     * is non-empty, so we may not have to spend any time waiting */
    rc = WaitForSingleObject(buf->readReady, timeout);
    if (rc == WAIT_TIMEOUT) {
        return RING_BUFFER_TIMEOUT; /* Timeout */
    } else if (rc == WAIT_FAILED) {
        return RING_BUFFER_LOCK_ERROR;
    }

    if (buf->error) {
        /* An error was signalled. Fail. */
        return buf->error;
    }

    /* Buffer should have data available now */
    if (ring_buffer_lock(buf, timeout)) {
        BOOL success = ring_buffer_get(buf, data);
        ring_buffer_unlock(buf);

        if (!success) {
            /* Ring buffer was emptied between checking for read ready and
             * trying to read */
            debug(F100, "ring buffer get - try again!", "", 0);
            return RING_BUFFER_TRY_AGAIN;
        }
    } else {
        /* Timeout trying to acquire a lock on the buffer to even check its
         * status */
        return RING_BUFFER_TIMEOUT;
    }

    return RING_BUFFER_SUCCESS;
}


size_t ring_buffer_write(ring_buffer_handle_t buf, char* data, size_t length) {
    size_t written = 0;
    while (written < length) {
        BOOL result = ring_buffer_put_noset(buf, data[written]);
        if (!result) {
            /* Put failed - buffer full? No more will be written. We are done.
             * The caller will have to try again with the rest of the data
             * later.*/
            ring_buffer_set_events(buf);
            return written;
        }
        written++;
    }

    ring_buffer_set_events(buf);
    return written;
}


size_t ring_buffer_read(ring_buffer_handle_t buf, char* data, size_t max_length) {
    size_t bytes_read = 0;
    size_t buf_len;

    buf_len = ring_buffer_length(buf);

    /* Don't read past the "end" of the ring buffer, or we'll just get repeat
     * data. Really the caller should check this first. */
    if (max_length > buf_len) {
        max_length = buf_len;
    }

    /*
     * No need to check there is data in the buffer - the first call to
     * ring_buffer_get will do that.
     */

    memset(data, 0, max_length);
    while (bytes_read < max_length) {
        char c;
        BOOL result = ring_buffer_get_noset(buf, &c);
        if (!result) {
            /* Get failed - ring buffer is probably empty. */
            ring_buffer_set_events(buf);
            return bytes_read;
        }
        data[bytes_read] = c;
        bytes_read++;
    }
    ring_buffer_set_events(buf);
    return bytes_read;
}


size_t ring_buffer_peek(ring_buffer_handle_t buf, char* data, size_t max_length) {
    size_t bytes_read = 0;
    size_t tail;
    size_t buf_len;

    /*
     * This function works just like ring_buffer_read, but we operate on a copy
     * of the tail pointer
     */
    tail = buf->tail;

    if (ring_buffer_is_empty(buf)) {
        return bytes_read; /* Nothing to read */
    }

    buf_len = ring_buffer_length(buf);

    /* Don't peek past the "end" of the ring buffer, or we'll just get repeat
     * data. Really the caller should check this first. */
    if (max_length > buf_len) {
        max_length = buf_len;
    }

    memset(data, 0, max_length);
    while (bytes_read < max_length) {
        /* Fetch the next value from the buffer */
        data[bytes_read] = buf->buffer[tail];

        /* Then advance our copy of tail pointer wrapping if necessary */
        if (++(tail) == buf->size) {
            tail = 0;
        }

        bytes_read++;
    }
    return bytes_read;
}


BOOL ring_buffer_consume(ring_buffer_handle_t buf, size_t length) {

    if (ring_buffer_length(buf) < length) {
        /* Can't consume more elements than there are in the buffer */
        return FALSE;
    }

    buf->tail = (buf->tail + length) % buf->size;

    if (length > 0) {
        /* If it was full before it is no longer */
        buf->full = FALSE;
        SetEvent(buf->writeReady);

        if (ring_buffer_is_empty(buf)) {
            /* We emptied the buffer. It's no longer ready for reading. */
            ResetEvent(buf->readReady);
        }
    }

    return TRUE;
}


BOOL ring_buffer_lock(ring_buffer_handle_t buf, DWORD msTimeout) {
    DWORD dwWaitResult;

    if (buf->mutex == NULL) {
        debug(F100, "ringbuf mutex does not exist in ring_buffer_lock.", "", 0);
        return FALSE;
    };

    dwWaitResult = WaitForSingleObject(
            buf->mutex,
            msTimeout);
    switch(dwWaitResult) {
        case WAIT_OBJECT_0:
            return TRUE;
        case WAIT_TIMEOUT:
            return FALSE;
        case WAIT_ABANDONED:
            debug(F100, "ringbuf mutex acquired in abandoned state. Buffer may be inconsistent", "", 0);
            return TRUE;
        case WAIT_FAILED:
            debug(F101, "ringbuf wait on mutex failed with error", "", GetLastError());
            return FALSE;
    }
    return FALSE; /* Should never happen */
}


void ring_buffer_unlock(ring_buffer_handle_t buf) {
    if (buf->mutex == NULL) {
        debug(F100, "ringbuf mutex does not exist in ring_buffer_unlock.", "", 0);
        return;
    };
    ReleaseMutex(buf->mutex);
}

void ring_buffer_signal_error(ring_buffer_handle_t buf, int error) {
    if (ring_buffer_lock(buf, INFINITE)) {
        buf->error = error;
        ring_buffer_unlock(buf);

        if (buf->readReady == NULL) {
            debug(F100, "ringbuf readReady does not exist in ring_buffer_signal_error.", "", 0);
        };

        if (buf->writeReady == NULL) {
            debug(F100, "ringbuf writeReady does not exist in ring_buffer_signal_error.", "", 0);
        };

        if (buf->readReady == NULL || buf->writeReady == NULL) {
            return;
        };

        /* Set read/write events to immediately unblock calls to the blocking
         * get/put operations */
        SetEvent(buf->readReady);
        SetEvent(buf->writeReady);
    }
}

HANDLE ring_buffer_get_ready_read_event(ring_buffer_handle_t buf) {
    return buf->readReady;
}
