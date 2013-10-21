char *cksftpv = "SFTP support, 8.0.003, 23 June 2003";

/*
 * Copyright (c) 2001-2003 Damien Miller.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* XXX: memleaks */
/* XXX: signed vs unsigned */
/* XXX: remove all logging, only return status codes */
/* XXX: copy between two remote sites */

#include "ckcdeb.h"
#ifdef SFTP_BUILTIN
#include "ckuusr.h"

#include "includes.h"

#include <fcntl.h>
#include <sys/stat.h>

#include "openbsd-compat/sys-queue.h"

#include "buffer.h"
#include "bufaux.h"
#include "getput.h"
#include "xmalloc.h"
#include "log.h"
#include "atomicio.h"
#include "xmalloc.h"
#include "pathnames.h"
#include "misc.h"

#include "ckosftp.h"

extern int showprogress;

/* Minimum amount of data to read at at time */
#define MIN_READ_SIZE	512

/* Maximum packet size */
#define MAX_MSG_LENGTH	(256 * 1024)

struct sftp_conn {
	u_int transfer_buflen;
	u_int num_requests;
	u_int version;
	u_int msg_id;
};

static int
send_msg(Buffer *m)
{
	u_char mlen[4];

	if (buffer_len(m) > MAX_MSG_LENGTH) {
		fatal("Outbound message too long %u", buffer_len(m));
		return(-1);
	}

	/* Send length first */
	PUT_32BIT(mlen, buffer_len(m));
	if (ssh_tol(mlen, sizeof(mlen)) <= 0) {
		fatal("Couldn't send packet: %s", strerror(errno));
		return(-1);
	}

	if (ssh_tol(buffer_ptr(m), buffer_len(m)) <= 0) {
		fatal("Couldn't send packet: %s", strerror(errno));
		return(-1);
	}

	buffer_clear(m);
	return(0);
}

static int
get_msg(Buffer *m)
{
	ssize_t len;
	u_int msg_len, msg_read;

	buffer_append_space(m, 4);
	len = ssh_xin(4, buffer_ptr(m));
	if (len == 0) {
		fatal("Connection closed");
		return(-1);
	} else if (len == -1) {
		fatal("Couldn't read packet: %s", strerror(errno));
	    return(-1);
	}
	msg_len = buffer_get_int(m);
	if (msg_len > MAX_MSG_LENGTH) {
		fatal("Received message too long %u", msg_len);
		return(-1);
	}

	buffer_append_space(m, msg_len);
	msg_read = 0;
	while (msg_read < msg_len) {
		len = ssh_xin(msg_len-msg_read, ((char *)buffer_ptr(m))+msg_read);
		if (len == 0) {
			fatal("Connection closed");
			return(-1);
		} else if (len == -1) {
			fatal("Read packet: %s", strerror(errno));
			return(-1);
		}
		msg_read += len;
	}
	return(0);
}

static int
send_string_request(u_int id, u_int code, char *s,
    u_int len)
{
	Buffer msg;
	memset(&msg,0,sizeof(Buffer));
	buffer_init(&msg);
	buffer_put_char(&msg, code);
	buffer_put_int(&msg, id);
	buffer_put_string(&msg, s, len);
	if (send_msg(&msg) < 0)
		return(-1);
	debug3("Sent message T:%u I:%u", code, id);
	buffer_free(&msg);
	return(0);
}

static int
send_string_attrs_request(u_int id, u_int code, char *s,
    u_int len, Attrib *a)
{
	Buffer msg;
        memset(&msg,0,sizeof(Buffer));
	buffer_init(&msg);
	buffer_put_char(&msg, code);
	buffer_put_int(&msg, id);
	buffer_put_string(&msg, s, len);
	encode_attrib(&msg, a);
	if (send_msg(&msg) < 0)
		return(-1);
	debug3("Sent message T:%u I:%u", code, id);
	buffer_free(&msg);
	return(0);
}

static u_int
get_status(u_int expected_id)
{
	Buffer msg;
	u_int type, id, status;

	memset(&msg,0,sizeof(Buffer));
	buffer_init(&msg);
	if (get_msg(&msg) < 0)
		return(-1);
	type = buffer_get_char(&msg);
	id = buffer_get_int(&msg);

	if (id != expected_id) {
		fatal("ID mismatch (%u != %u)", id, expected_id);
		return(-1);
	}
	if (type != SSH2_FXP_STATUS) {
		fatal("Expected SSH2_FXP_STATUS(%u) packet, got %u",
			   SSH2_FXP_STATUS, type);
		return(	-1);
	}

	status = buffer_get_int(&msg);
	buffer_free(&msg);

	debug3("SSH2_FXP_STATUS %u", status);

	return(status);
}

static char *
get_handle(u_int expected_id, u_int *len)
{
	Buffer msg;
	u_int type, id;
	char *handle;

	memset(&msg,0,sizeof(Buffer));
	buffer_init(&msg);
	if (get_msg(&msg) < 0)
		return(NULL);
	type = buffer_get_char(&msg);
	id = buffer_get_int(&msg);

	if (id != expected_id) {
		fatal("ID mismatch (%u != %u)", id, expected_id);
		return(NULL);
	}
	if (type == SSH2_FXP_STATUS) {
		int status = buffer_get_int(&msg);

		error("Couldn't get handle: %s", fx2txt(status));
		return(NULL);
	} else if (type != SSH2_FXP_HANDLE) {
		fatal("Expected SSH2_FXP_HANDLE(%u) packet, got %u",
		    SSH2_FXP_HANDLE, type);
		return(NULL);
	}

	handle = buffer_get_string(&msg, len);
	buffer_free(&msg);

	return(handle);
}

static Attrib *
get_decode_stat(u_int expected_id, int quiet)
{
	Buffer msg;
	u_int type, id;
	Attrib *a;

        memset(&msg,0,sizeof(Buffer));
        buffer_init(&msg);
	if (get_msg(&msg) < 0)
		return(NULL);

	type = buffer_get_char(&msg);
	id = buffer_get_int(&msg);

	debug3("Received stat reply T:%u I:%u", type, id);
	if (id != expected_id) {
		fatal("ID mismatch (%u != %u)", id, expected_id);
		return(NULL);
	}
	if (type == SSH2_FXP_STATUS) {
		int status = buffer_get_int(&msg);

		if (quiet)
			debug1("Couldn't stat remote file: %s", fx2txt(status));
		else
			error("Couldn't stat remote file: %s", fx2txt(status));
		return(NULL);
	} else if (type != SSH2_FXP_ATTRS) {
		fatal("Expected SSH2_FXP_ATTRS(%u) packet, got %u",
		    SSH2_FXP_ATTRS, type);
		return(NULL);
	}
	a = decode_attrib(&msg);
	buffer_free(&msg);

	return(a);
}

struct sftp_conn *
do_init(u_int transfer_buflen, u_int num_requests)
{
	u_int type;
	int version;
	Buffer msg;
	struct sftp_conn *ret;

	memset(&msg,0,sizeof(Buffer));
	buffer_init(&msg);
	buffer_put_char(&msg, SSH2_FXP_INIT);
	buffer_put_int(&msg, SSH2_FILEXFER_VERSION);
	if (send_msg(&msg) < 0)
		return(NULL);

	buffer_clear(&msg);

	if (get_msg(&msg) < 0)
		return(NULL);

	/* Expecting a VERSION reply */
	if ((type = buffer_get_char(&msg)) != SSH2_FXP_VERSION) {
		error("Invalid packet back from SSH2_FXP_INIT (type %u)",
		    type);
		buffer_free(&msg);
		return(NULL);
	}
	version = buffer_get_int(&msg);

	debug2("Remote version: %d", version);

	/* Check for extensions */
	while (buffer_len(&msg) > 0) {
		char *name = buffer_get_string(&msg, NULL);
		char *value = buffer_get_string(&msg, NULL);

		debug2("Init extension: \"%s\"", name);
		xfree(name);
		xfree(value);
	}

	buffer_free(&msg);

	ret = xmalloc(sizeof(*ret));
	ret->transfer_buflen = transfer_buflen;
	ret->num_requests = num_requests;
	ret->version = version;
	ret->msg_id = 1;

	/* Some filexfer v.0 servers don't support large packets */
	if (version == 0)
		ret->transfer_buflen = MIN(ret->transfer_buflen, 20480);

	return(ret);
}

u_int
sftp_proto_version(struct sftp_conn *conn)
{
	return(conn->version);
}

int
do_close(struct sftp_conn *conn, char *handle, u_int handle_len)
{
	u_int id, status;
	Buffer msg;

        memset(&msg,0,sizeof(Buffer));
        buffer_init(&msg);

	id = conn->msg_id++;
	buffer_put_char(&msg, SSH2_FXP_CLOSE);
	buffer_put_int(&msg, id);
	buffer_put_string(&msg, handle, handle_len);
	if (send_msg(&msg) < 0)
		return(-1);
	debug3("Sent message SSH2_FXP_CLOSE I:%u", id);

	status = get_status(id);
	if (status != SSH2_FX_OK)
		error("Couldn't close file: %s", fx2txt(status));

	buffer_free(&msg);

	return(status);
}


static int
do_lsreaddir(struct sftp_conn *conn, char *path, int printflag,
    SFTP_DIRENT ***dir)
{
	Buffer msg;
	u_int type, id, handle_len, i, expected_id, ents = 0;
	char *handle;

	id = conn->msg_id++;

        memset(&msg,0,sizeof(Buffer));
        buffer_init(&msg);
	buffer_put_char(&msg, SSH2_FXP_OPENDIR);
	buffer_put_int(&msg, id);
	buffer_put_cstring(&msg, path);
	if (send_msg(&msg) < 0)
		return(-1);

	buffer_clear(&msg);

	handle = get_handle(id, &handle_len);
	if (handle == NULL)
		return(-1);

	if (dir) {
		ents = 0;
		*dir = xmalloc(sizeof(**dir));
		(*dir)[0] = NULL;
	}

	for (;;) {
		int count;

		id = expected_id = conn->msg_id++;

		debug3("Sending SSH2_FXP_READDIR I:%u", id);

		buffer_clear(&msg);
		buffer_put_char(&msg, SSH2_FXP_READDIR);
		buffer_put_int(&msg, id);
		buffer_put_string(&msg, handle, handle_len);
		if (send_msg(&msg) < 0);
			return(-1);

		buffer_clear(&msg);

		if (get_msg(&msg) < 0)
			return(-1);

		type = buffer_get_char(&msg);
		id = buffer_get_int(&msg);

		debug3("Received reply T:%u I:%u", type, id);

		if (id != expected_id) {
			fatal("ID mismatch (%u != %u)", id, expected_id);
			return(-1);
		}

		if (type == SSH2_FXP_STATUS) {
			int status = buffer_get_int(&msg);

			debug3("Received SSH2_FXP_STATUS %d", status);

			if (status == SSH2_FX_EOF) {
				break;
			} else {
				error("Couldn't read directory: %s",
				    fx2txt(status));
				do_close(conn, handle, handle_len);
				xfree(handle);
				return(status);
			}
		} else if (type != SSH2_FXP_NAME) {
			fatal("Expected SSH2_FXP_NAME(%u) packet, got %u",
			    SSH2_FXP_NAME, type);
			return(-1);
		}

		count = buffer_get_int(&msg);
		if (count == 0)
			break;
		debug3("Received %d SSH2_FXP_NAME responses", count);
		for (i = 0; i < count; i++) {
			char *filename, *longname;
			Attrib *a;

			filename = buffer_get_string(&msg, NULL);
			longname = buffer_get_string(&msg, NULL);
			a = decode_attrib(&msg);

			if (printflag)
				printf("%s\n", longname);

			if (dir) {
				*dir = xrealloc(*dir, sizeof(**dir) *
				    (ents + 2));
				(*dir)[ents] = xmalloc(sizeof(***dir));
				(*dir)[ents]->filename = xstrdup(filename);
				(*dir)[ents]->longname = xstrdup(longname);
				memcpy(&(*dir)[ents]->a, a, sizeof(*a));
				(*dir)[++ents] = NULL;
			}

			xfree(filename);
			xfree(longname);
		}
	}

	buffer_free(&msg);
	do_close(conn, handle, handle_len);
	xfree(handle);

	return(0);
}

int
do_readdir(struct sftp_conn *conn, char *path, SFTP_DIRENT ***dir)
{
	return(do_lsreaddir(conn, path, 0, dir));
}

void 
free_sftp_dirents(SFTP_DIRENT **s)
{
	int i;

	for (i = 0; s[i]; i++) {
		xfree(s[i]->filename);
		xfree(s[i]->longname);
		xfree(s[i]);
	}
	xfree(s);
}

int
do_rm(struct sftp_conn *conn, char *path)
{
	u_int status, id;

	debug2("Sending SSH2_FXP_REMOVE \"%s\"", path);

	id = conn->msg_id++;
	send_string_request(id, SSH2_FXP_REMOVE, path,
	    strlen(path));
	status = get_status(id);
	if (status != SSH2_FX_OK)
		error("Couldn't delete file: %s", fx2txt(status));
	return(status);
}

int
do_mkdir(struct sftp_conn *conn, char *path, Attrib *a)
{
	u_int status, id;

	id = conn->msg_id++;
	send_string_attrs_request(id, SSH2_FXP_MKDIR, path,
	    strlen(path), a);

	status = get_status(id);
	if (status != SSH2_FX_OK)
		error("Couldn't create directory: %s", fx2txt(status));

	return(status);
}

int
do_rmdir(struct sftp_conn *conn, char *path)
{
	u_int status, id;

	id = conn->msg_id++;
	send_string_request(id, SSH2_FXP_RMDIR, path,
	    strlen(path));

	status = get_status(id);
	if (status != SSH2_FX_OK)
		error("Couldn't remove directory: %s", fx2txt(status));

	return(status);
}

Attrib *
do_stat(struct sftp_conn *conn, char *path, int quiet)
{
	u_int id;

	id = conn->msg_id++;

	send_string_request( id,
	    conn->version == 0 ? SSH2_FXP_STAT_VERSION_0 : SSH2_FXP_STAT,
	    path, strlen(path));

	return(get_decode_stat(id, quiet));
}

Attrib *
do_lstat(struct sftp_conn *conn, char *path, int quiet)
{
	u_int id;

	if (conn->version == 0) {
		if (quiet)
			debug1("Server version does not support lstat operation");
		else
			error("Server version does not support lstat operation");
		return(do_stat(conn, path, quiet));
	}

	id = conn->msg_id++;
	send_string_request( id, SSH2_FXP_LSTAT, path,
	    strlen(path));

	return(get_decode_stat(id, quiet));
}

Attrib *
do_fstat(struct sftp_conn *conn, char *handle, u_int handle_len, int quiet)
{
	u_int id;

	id = conn->msg_id++;
	send_string_request( id, SSH2_FXP_FSTAT, handle,
	    handle_len);

	return(get_decode_stat(id, quiet));
}

int
do_setstat(struct sftp_conn *conn, char *path, Attrib *a)
{
	u_int status, id;

	id = conn->msg_id++;
	send_string_attrs_request( id, SSH2_FXP_SETSTAT, path,
	    strlen(path), a);

	status = get_status(id);
	if (status != SSH2_FX_OK)
		error("Couldn't setstat on \"%s\": %s", path,
		    fx2txt(status));

	return(status);
}

int
do_fsetstat(struct sftp_conn *conn, char *handle, u_int handle_len,
    Attrib *a)
{
	u_int status, id;

	id = conn->msg_id++;
	send_string_attrs_request( id, SSH2_FXP_FSETSTAT, handle,
	    handle_len, a);

	status = get_status(id);
	if (status != SSH2_FX_OK)
		error("Couldn't fsetstat: %s", fx2txt(status));

	return(status);
}

char *
do_realpath(struct sftp_conn *conn, char *path)
{
	Buffer msg;
	u_int type, expected_id, count, id;
	char *filename, *longname;
	Attrib *a;

	expected_id = id = conn->msg_id++;
	send_string_request( id, SSH2_FXP_REALPATH, path,
	    strlen(path));

        memset(&msg,0,sizeof(Buffer));
        buffer_init(&msg);

	if (get_msg(&msg) < 0)
		return(NULL);
	type = buffer_get_char(&msg);
	id = buffer_get_int(&msg);

	if (id != expected_id) {
		fatal("ID mismatch (%u != %u)", id, expected_id);
		return(NULL);
	}

	if (type == SSH2_FXP_STATUS) {
		u_int status = buffer_get_int(&msg);

		error("Couldn't canonicalise: %s", fx2txt(status));
		return(NULL);
	} else if (type != SSH2_FXP_NAME) {
		fatal("Expected SSH2_FXP_NAME(%u) packet, got %u",
		    SSH2_FXP_NAME, type);
		return(NULL);
	}

	count = buffer_get_int(&msg);
	if (count != 1) {
		fatal("Got multiple names (%d) from SSH_FXP_REALPATH", count);
		return(NULL);
	}

	filename = buffer_get_string(&msg, NULL);
	longname = buffer_get_string(&msg, NULL);
	a = decode_attrib(&msg);

	debug3("SSH_FXP_REALPATH %s -> %s", path, filename);

	xfree(longname);

	buffer_free(&msg);

	return(filename);
}

int
do_rename(struct sftp_conn *conn, char *oldpath, char *newpath)
{
	Buffer msg;
	u_int status, id;

        memset(&msg,0,sizeof(Buffer));
        buffer_init(&msg);

	/* Send rename request */
	id = conn->msg_id++;
	buffer_put_char(&msg, SSH2_FXP_RENAME);
	buffer_put_int(&msg, id);
	buffer_put_cstring(&msg, oldpath);
	buffer_put_cstring(&msg, newpath);
	if (send_msg(&msg) < 0)
		return(-1);

	debug3("Sent message SSH2_FXP_RENAME \"%s\" -> \"%s\"", oldpath,
	    newpath);
	buffer_free(&msg);

	status = get_status(id);
	if (status != SSH2_FX_OK)
		error("Couldn't rename file \"%s\" to \"%s\": %s", oldpath,
		    newpath, fx2txt(status));

	return(status);
}

int
do_symlink(struct sftp_conn *conn, char *oldpath, char *newpath)
{
	Buffer msg;
	u_int status, id;

	if (conn->version < 3) {
		error("This server does not support the symlink operation");
		return(SSH2_FX_OP_UNSUPPORTED);
	}

	memset(&msg,0,sizeof(Buffer));
	buffer_init(&msg);

	/* Send rename request */
	id = conn->msg_id++;
	buffer_put_char(&msg, SSH2_FXP_SYMLINK);
	buffer_put_int(&msg, id);
	buffer_put_cstring(&msg, oldpath);
	buffer_put_cstring(&msg, newpath);
	if (send_msg(&msg) < 0)
		return(-1);
	debug3("Sent message SSH2_FXP_SYMLINK \"%s\" -> \"%s\"", oldpath,
	    newpath);
	buffer_free(&msg);

	status = get_status(id);
	if (status != SSH2_FX_OK)
		error("Couldn't symlink file \"%s\" to \"%s\": %s", oldpath,
		    newpath, fx2txt(status));

	return(status);
}

char *
do_readlink(struct sftp_conn *conn, char *path)
{
	Buffer msg;
	u_int type, expected_id, count, id;
	char *filename, *longname;
	Attrib *a;

	expected_id = id = conn->msg_id++;
	send_string_request( id, SSH2_FXP_READLINK, path,
						 strlen(path));

	memset(&msg,0,sizeof(Buffer));
	buffer_init(&msg);

	if (get_msg(&msg) < 0)
		return(NULL);
	type = buffer_get_char(&msg);
	id = buffer_get_int(&msg);

	if (id != expected_id) {
		fatal("ID mismatch (%u != %u)", id, expected_id);
		return(NULL);
	}

	if (type == SSH2_FXP_STATUS) {
		u_int status = buffer_get_int(&msg);

		error("Couldn't readlink: %s", fx2txt(status));
		return(NULL);
	} else if (type != SSH2_FXP_NAME) {
		fatal("Expected SSH2_FXP_NAME(%u) packet, got %u",
		    SSH2_FXP_NAME, type);
		return(NULL);
	}

	count = buffer_get_int(&msg);
	if (count != 1) {
		fatal("Got multiple names (%d) from SSH_FXP_READLINK", count);
		return(NULL);
	}

	filename = buffer_get_string(&msg, NULL);
	longname = buffer_get_string(&msg, NULL);
	a = decode_attrib(&msg);

	debug3("SSH_FXP_READLINK %s -> %s", path, filename);

	xfree(longname);

	buffer_free(&msg);

	return(filename);
}

static int
send_read_request(u_int id, u_int64_t offset, u_int len,
    char *handle, u_int handle_len)
{
	Buffer msg;

	memset(&msg,0,sizeof(Buffer));
	buffer_init(&msg);
	buffer_clear(&msg);
	buffer_put_char(&msg, SSH2_FXP_READ);
	buffer_put_int(&msg, id);
	buffer_put_string(&msg, handle, handle_len);
	buffer_put_int64(&msg, offset);
	buffer_put_int(&msg, len);
	if (send_msg(&msg) < 0)
		return(-1);
	buffer_free(&msg);
	return(0);
}

int
do_download(struct sftp_conn *conn, char *remote_path, char *local_path,
    int pflag)
{
	Attrib junk, *a;
	Buffer msg;
	char *handle;
	int local_fd, status, num_req, max_req, write_error;
	int read_error, write_errno;
	u_int64_t offset, size;
	u_int handle_len, mode, type, id, buflen;
	off_t progress_counter;
	struct request {
		u_int id;
		u_int len;
		u_int64_t offset;
		TAILQ_ENTRY(request) tq;
	};
	TAILQ_HEAD(reqhead, request) requests;
	struct request *req;

	TAILQ_INIT(&requests);

	a = do_stat(conn, remote_path, 0);
	if (a == NULL)
		return(-1);

	/* XXX: should we preserve set[ug]id? */
	if (a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS)
		mode = a->perm & 0777;
	else
		mode = 0666;

	if ((a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS) &&
	    (!S_ISREG(a->perm))) {
		error("Cannot download non-regular file: %s", remote_path);
		return(-1);
	}

	if (a->flags & SSH2_FILEXFER_ATTR_SIZE)
		size = a->size;
	else
		size = 0;

	buflen = conn->transfer_buflen;
        memset(&msg,0,sizeof(Buffer));
        buffer_init(&msg);

	/* Send open request */
	id = conn->msg_id++;
	buffer_put_char(&msg, SSH2_FXP_OPEN);
	buffer_put_int(&msg, id);
	buffer_put_cstring(&msg, remote_path);
	buffer_put_int(&msg, SSH2_FXF_READ);
	attrib_clear(&junk); /* Send empty attributes */
	encode_attrib(&msg, &junk);
	if (send_msg(&msg) < 0)
		return(-1);
	debug3("Sent message SSH2_FXP_OPEN I:%u P:%s", id, remote_path);

	handle = get_handle(id, &handle_len);
	if (handle == NULL) {
		buffer_free(&msg);
		return(-1);
	}

	local_fd = open(local_path, _O_WRONLY | _O_CREAT | _O_TRUNC, 
	    mode | _S_IWRITE);
	if (local_fd == -1) {
		error("Couldn't open local file \"%s\" for writing: %s",
		    local_path, strerror(errno));
		buffer_free(&msg);
		xfree(handle);
		return(-1);
	}

	/* Read from remote and write to local */
	write_error = read_error = write_errno = num_req = offset = 0;
	max_req = 1;
	progress_counter = 0;

#ifdef COMMENT
	if (showprogress) {
		if (size)
			start_progress_meter(remote_path, size,
			    &progress_counter);
		else
			printf("Fetching %s to %s\n", remote_path, local_path);
	}
#endif 

	while (num_req > 0 || max_req > 0) {
		char *data;
		u_int len;

		/* Send some more requests */
		while (num_req < max_req) {
			debug3("Request range %llu -> %llu (%d/%d)",
			    (u_int64_t)offset,
			    (u_int64_t)offset + buflen - 1,
			    num_req, max_req);
			req = xmalloc(sizeof(*req));
			req->id = conn->msg_id++;
			req->len = buflen;
			req->offset = offset;
			offset += buflen;
			num_req++;
			TAILQ_INSERT_TAIL(&requests, req, tq);
			if (send_read_request( req->id, req->offset,
			    req->len, handle, handle_len) < 0)
				return(-1);
		}

		buffer_clear(&msg);
		if (get_msg(&msg) < 0)
			return(-1);
		type = buffer_get_char(&msg);
		id = buffer_get_int(&msg);
		debug3("Received reply T:%u I:%u R:%d", type, id, max_req);

		/* Find the request in our queue */
		for(req = TAILQ_FIRST(&requests);
		    req != NULL && req->id != id;
		    req = TAILQ_NEXT(req, tq))
			;
			if (req == NULL) {
				fatal("Unexpected reply %u", id);
				return(-1);
			}

		switch (type) {
		case SSH2_FXP_STATUS:
			status = buffer_get_int(&msg);
			if (status != SSH2_FX_EOF)
				read_error = 1;
			max_req = 0;
			TAILQ_REMOVE(&requests, req, tq);
			xfree(req);
			num_req--;
			break;
		case SSH2_FXP_DATA:
			data = buffer_get_string(&msg, &len);
			debug3("Received data %llu -> %llu",
			    (u_int64_t)req->offset,
			    (u_int64_t)req->offset + len - 1);
			if (len > req->len) {
				fatal("Received more data than asked for "
				    "%u > %u", len, req->len);
				return(-1);
			}
			if ((lseek(local_fd, req->offset, SEEK_SET) == -1 ||
			    write(local_fd, data, len) != len) &&
			    !write_error) {
				write_errno = errno;
				write_error = 1;
				max_req = 0;
			}
			progress_counter += len;
			xfree(data);

			if (len == req->len) {
				TAILQ_REMOVE(&requests, req, tq);
				xfree(req);
				num_req--;
			} else {
				/* Resend the request for the missing data */
				debug3("Short data block, re-requesting "
				    "%llu -> %llu (%2d)",
				    (u_int64_t)req->offset + len,
				    (u_int64_t)req->offset +
				    req->len - 1, num_req);
				req->id = conn->msg_id++;
				req->len -= len;
				req->offset += len;
				if (send_read_request( req->id,
				    req->offset, req->len, handle, handle_len) < 0)
					return(-1);
				/* Reduce the request size */
				if (len < buflen)
					buflen = MAX(MIN_READ_SIZE, len);
			}
			if (max_req > 0) { /* max_req = 0 iff EOF received */
				if (size > 0 && offset > size) {
					/* Only one request at a time
					 * after the expected EOF */
					debug3("Finish at %llu (%2d)",
					    (u_int64_t)offset,
					    num_req);
					max_req = 1;
				}
				else if (max_req < conn->num_requests + 1) {
					++max_req;
				}
			}
			break;
		default:
			fatal("Expected SSH2_FXP_DATA(%u) packet, got %u",
			    SSH2_FXP_DATA, type);
			return(-1);
		}
	}

#ifdef COMMENT
	if (showprogress && size)
		stop_progress_meter();
#endif

	/* Sanity check */
	if (TAILQ_FIRST(&requests) != NULL) {
		fatal("Transfer complete, but requests still in queue");
		return(-1);
	}

	if (read_error) {
		error("Couldn't read from remote file \"%s\" : %s",
		    remote_path, fx2txt(status));
		do_close(conn, handle, handle_len);
	} else if (write_error) {
		error("Couldn't write to \"%s\": %s", local_path,
		    strerror(write_errno));
		status = -1;
		do_close(conn, handle, handle_len);
	} else {
		status = do_close(conn, handle, handle_len);

		/* Override umask and utimes if asked */
#ifdef HAVE_FCHMOD
		if (pflag && fchmod(local_fd, mode) == -1)
#else 
		if (pflag && chmod(local_path, mode) == -1)
#endif /* HAVE_FCHMOD */
			error("Couldn't set mode on \"%s\": %s", local_path,
			    strerror(errno));
		if (pflag && (a->flags & SSH2_FILEXFER_ATTR_ACMODTIME)) {
			struct timeval tv[2];
			tv[0].tv_sec = a->atime;
			tv[1].tv_sec = a->mtime;
			tv[0].tv_usec = tv[1].tv_usec = 0;
			if (utimes(local_path, tv) == -1)
				error("Can't set times on \"%s\": %s",
				    local_path, strerror(errno));
		}
	}
	close(local_fd);
	buffer_free(&msg);
	xfree(handle);

	return(status);
}

int
do_upload(struct sftp_conn *conn, char *local_path, char *remote_path,
    int pflag)
{
	int local_fd, status;
	u_int handle_len, id, type;
	u_int64_t offset;
	char *handle, *data;
	Buffer msg;
	struct _stat sb;
	Attrib a;
	u_int32_t startid;
	u_int32_t ackid;
	struct outstanding_ack {
		u_int id;
		u_int len;
		u_int64_t offset;
		TAILQ_ENTRY(outstanding_ack) tq;
	};
	TAILQ_HEAD(ackhead, outstanding_ack) acks;
	struct outstanding_ack *ack;

	TAILQ_INIT(&acks);

	if ((local_fd = open(local_path, _O_RDONLY, 0)) == -1) {
		error("Couldn't open local file \"%s\" for reading: %s",
		    local_path, strerror(errno));
		return(-1);
	}
	if (fstat(local_fd, &sb) == -1) {
		error("Couldn't fstat local file \"%s\": %s",
		    local_path, strerror(errno));
		close(local_fd);
		return(-1);
	}
	if (!S_ISREG(sb.st_mode)) {
		error("%s is not a regular file", local_path);
		close(local_fd);
		return(-1);
	}
	stat_to_attrib(&sb, &a);

	a.flags &= ~SSH2_FILEXFER_ATTR_SIZE;
	a.flags &= ~SSH2_FILEXFER_ATTR_UIDGID;
	a.perm &= 0777;
	if (!pflag)
		a.flags &= ~SSH2_FILEXFER_ATTR_ACMODTIME;

        memset(&msg,0,sizeof(Buffer));
        buffer_init(&msg);

	/* Send open request */
	id = conn->msg_id++;
	buffer_put_char(&msg, SSH2_FXP_OPEN);
	buffer_put_int(&msg, id);
	buffer_put_cstring(&msg, remote_path);
	buffer_put_int(&msg, SSH2_FXF_WRITE|SSH2_FXF_CREAT|SSH2_FXF_TRUNC);
	encode_attrib(&msg, &a);
	if (send_msg(&msg) < 0)
		return(-1);
	debug3("Sent message SSH2_FXP_OPEN I:%u P:%s", id, remote_path);

	buffer_clear(&msg);

	handle = get_handle(id, &handle_len);
	if (handle == NULL) {
		close(local_fd);
		buffer_free(&msg);
		return(-1);
	}

	startid = ackid = id + 1;
	data = xmalloc(conn->transfer_buflen);

	/* Read from local and write to remote */
	offset = 0;
#ifdef COMMENT
	if (showprogress)
		start_progress_meter(local_path, sb.st_size, &offset);
	else
		printf("Uploading %s to %s\n", local_path, remote_path);
#endif

	for (;;) {
		int len;

		/*
		 * Can't use atomicio here because it returns 0 on EOF, thus losing
		 * the last block of the file
		 */
		do
			len = read(local_fd, data, conn->transfer_buflen);
		while ((len == -1) && (errno == EINTR || errno == EAGAIN));

		if (len == -1) {
			fatal("Couldn't read from \"%s\": %s", local_path,
			    strerror(errno));
			return(-1);
		}
		if (len != 0) {
			ack = xmalloc(sizeof(*ack));
			ack->id = ++id;
			ack->offset = offset;
			ack->len = len;
			TAILQ_INSERT_TAIL(&acks, ack, tq);

			buffer_clear(&msg);
			buffer_put_char(&msg, SSH2_FXP_WRITE);
			buffer_put_int(&msg, ack->id);
			buffer_put_string(&msg, handle, handle_len);
			buffer_put_int64(&msg, offset);
			buffer_put_string(&msg, data, len);
			if (send_msg(&msg) < 0)
				return(-1);
			debug3("Sent message SSH2_FXP_WRITE I:%u O:%llu S:%u",
			    id, (u_int64_t)offset, len);
		} else if (TAILQ_FIRST(&acks) == NULL)
			break;

			if (ack == NULL) {
				fatal("Unexpected ACK %u", id);
				return(-1);
			}

		if (id == startid || len == 0 ||
		    id - ackid >= conn->num_requests) {
			u_int r_id;

			buffer_clear(&msg);
			if (get_msg(&msg) < 0)
				return(-1);
			type = buffer_get_char(&msg);
			r_id = buffer_get_int(&msg);
			
			if (type != SSH2_FXP_STATUS) {
				fatal("Expected SSH2_FXP_STATUS(%d) packet, "
				    "got %d", SSH2_FXP_STATUS, type);
				return(-1);
			}

			status = buffer_get_int(&msg);
			debug3("SSH2_FXP_STATUS %d", status);

			/* Find the request in our queue */
			for(ack = TAILQ_FIRST(&acks);
			    ack != NULL && ack->id != r_id;
			    ack = TAILQ_NEXT(ack, tq))
				;
			if (ack == NULL) {
				fatal("Can't find request for ID %u", r_id);
				return(-1);
			}
			TAILQ_REMOVE(&acks, ack, tq);

			if (status != SSH2_FX_OK) {
				error("Couldn't write to remote file \"%s\": %s",
				    remote_path, fx2txt(status));
				do_close(conn, handle, handle_len);
				close(local_fd);
				xfree(data);
				xfree(ack);
				goto done;
			}
			debug3("In write loop, ack for %u %u bytes at %llu",
			   ack->id, ack->len, (u_int64_t)ack->offset);
			++ackid;
			xfree(ack);
		}
		offset += len;
	}
#ifdef COMMENT
	if (showprogress)
		stop_progress_meter();
#endif
	xfree(data);

	if (close(local_fd) == -1) {
		error("Couldn't close local file \"%s\": %s", local_path,
		    strerror(errno));
		do_close(conn, handle, handle_len);
		status = -1;
		goto done;
	}

	/* Override umask and utimes if asked */
	if (pflag)
		do_fsetstat(conn, handle, handle_len, &a);

	status = do_close(conn, handle, handle_len);

done:
	xfree(handle);
	buffer_free(&msg);
	return(status);
}

/* File to read commands from */
extern FILE *infile;

/* Size of buffer used when copying files */
size_t copy_buffer_len = 32768;

/* Number of concurrent outstanding requests */
size_t num_requests = 16;

/* This is set to 0 if the progressmeter is not desired. */
int showprogress = 1;

/* Seperators for interactive commands */
#define WHITESPACE " \t\r\n"

/* Commands for interactive mode */
#define I_CHDIR		1
#define I_CHGRP		2
#define I_CHMOD		3
#define I_CHOWN		4
#define I_GET		5
#define I_HELP		6
#define I_LCHDIR	7
#define I_LLS		8
#define I_LMKDIR	9
#define I_LPWD		10
#define I_LS		11
#define I_LUMASK	12
#define I_MKDIR		13
#define I_PUT		14
#define I_PWD		15
#define I_QUIT		16
#define I_RENAME	17
#define I_RM		18
#define I_RMDIR		19
#define I_SYMLINK	21
#define I_VERSION	22
#define I_PROGRESS	23

struct CMD {
	const char *c;
	const int n;
};

static const struct CMD cmds[] = {
	{ "bye",	I_QUIT },
	{ "cd",		I_CHDIR },
	{ "chdir",	I_CHDIR },
	{ "chgrp",	I_CHGRP },
	{ "chmod",	I_CHMOD },
	{ "chown",	I_CHOWN },
	{ "dir",	I_LS },
	{ "exit",	I_QUIT },
	{ "get",	I_GET },
	{ "mget",	I_GET },
	{ "help",	I_HELP },
	{ "lcd",	I_LCHDIR },
	{ "lchdir",	I_LCHDIR },
	{ "lls",	I_LLS },
	{ "lmkdir",	I_LMKDIR },
	{ "ln",		I_SYMLINK },
	{ "lpwd",	I_LPWD },
	{ "ls",		I_LS },
	{ "lumask",	I_LUMASK },
	{ "mkdir",	I_MKDIR },
	{ "progress",	I_PROGRESS },
	{ "put",	I_PUT },
	{ "mput",	I_PUT },
	{ "pwd",	I_PWD },
	{ "quit",	I_QUIT },
	{ "rename",	I_RENAME },
	{ "rm",		I_RM },
	{ "rmdir",	I_RMDIR },
	{ "symlink",	I_SYMLINK },
	{ "version",	I_VERSION },
	{ "?",		I_HELP },
	{ NULL,			-1}
};

static void
help(void)
{
	printf("Available commands:\n");
	printf("cd path                       Change remote directory to 'path'\n");
	printf("lcd path                      Change local directory to 'path'\n");
	printf("chgrp grp path                Change group of file 'path' to 'grp'\n");
	printf("chmod mode path               Change permissions of file 'path' to 'mode'\n");
	printf("chown own path                Change owner of file 'path' to 'own'\n");
	printf("help                          Display this help text\n");
	printf("get remote-path [local-path]  Download file\n");
	printf("lls [ls-options [path]]       Display local directory listing\n");
	printf("ln oldpath newpath            Symlink remote file\n");
	printf("lmkdir path                   Create local directory\n");
	printf("lpwd                          Print local working directory\n");
	printf("ls [path]                     Display remote directory listing\n");
	printf("lumask umask                  Set local umask to 'umask'\n");
	printf("mkdir path                    Create remote directory\n");
	printf("progress                      Toggle display of progress meter\n");
	printf("put local-path [remote-path]  Upload file\n");
	printf("pwd                           Display remote working directory\n");
	printf("exit                          Quit sftp\n");
	printf("quit                          Quit sftp\n");
	printf("rename oldpath newpath        Rename remote file\n");
	printf("rmdir path                    Remove remote directory\n");
	printf("rm path                       Delete remote file\n");
	printf("symlink oldpath newpath       Symlink remote file\n");
	printf("version                       Show SFTP version\n");
	printf("!command                      Execute 'command' in local shell\n");
	printf("!                             Escape to local shell\n");
	printf("?                             Synonym for help\n");
}

static void
local_do_ls(const char *args)
{
    /* XXX - Need to implement */
    error("Need to implement local_do_ls()");
}

/* Strip one path (usually the pwd) from the start of another */
static char *
path_strip(char *path, char *strip)
{
	size_t len;

	if (strip == NULL)
		return (xstrdup(path));

	len = strlen(strip);
	if (strip != NULL && strncmp(path, strip, len) == 0) {
		if (strip[len - 1] != '/' && path[len] == '/')
			len++;
		return (xstrdup(path + len));
	}

	return (xstrdup(path));
}

static char *
path_append(char *p1, char *p2)
{
	char *ret;
	int len = strlen(p1) + strlen(p2) + 2;

	ret = xmalloc(len);
	strlcpy(ret, p1, len);
	if (p1[strlen(p1) - 1] != '/')
		strlcat(ret, "/", len);
	strlcat(ret, p2, len);

	return(ret);
}

static char *
make_absolute(char *p, char *pwd)
{
	char *abs;

	/* Derelativise */
	if (p && p[0] != '/') {
		abs = path_append(pwd, p);
		xfree(p);
		return(abs);
	} else
		return(p);
}

static int
infer_path(const char *p, char **ifp)
{
	char *cp;

	cp = strrchr(p, '/');
	if (cp == NULL) {
		*ifp = xstrdup(p);
		return(0);
	}

	if (!cp[1]) {
		error("Invalid path");
		return(-1);
	}

	*ifp = xstrdup(cp + 1);
	return(0);
}

static int
parse_getput_flags(const char **cpp, int *pflag)
{
	const char *cp = *cpp;

	/* Check for flags */
	if (cp[0] == '-' && cp[1] && strchr(WHITESPACE, cp[2])) {
		switch (cp[1]) {
		case 'p':
		case 'P':
			*pflag = 1;
			break;
		default:
			error("Invalid flag -%c", cp[1]);
			return(-1);
		}
		cp += 2;
		*cpp = cp + strspn(cp, WHITESPACE);
	}

	return(0);
}

static int
parse_ls_flags(const char **cpp, int *lflag)
{
	const char *cp = *cpp;

	/* Check for flags */
	if (cp++[0] == '-') {
		for(; strchr(WHITESPACE, *cp) == NULL; cp++) {
			switch (*cp) {
			case 'l':
				*lflag = 1;
				break;
			default:
				error("Invalid flag -%c", *cp);
				return(-1);
			}
		}
		*cpp = cp + strspn(cp, WHITESPACE);
	}

	return(0);
}

static int
get_pathname(const char **cpp, char **path)
{
	const char *cp = *cpp, *end;
	char quot;
	int i;

	cp += strspn(cp, WHITESPACE);
	if (!*cp) {
		*cpp = cp;
		*path = NULL;
		return (0);
	}

	/* Check for quoted filenames */
	if (*cp == '\"' || *cp == '\'') {
		quot = *cp++;

		end = strchr(cp, quot);
		if (end == NULL) {
			error("Unterminated quote");
			goto fail;
		}
		if (cp == end) {
			error("Empty quotes");
			goto fail;
		}
		*cpp = end + 1 + strspn(end + 1, WHITESPACE);
	} else {
		/* Read to end of filename */
		end = strpbrk(cp, WHITESPACE);
		if (end == NULL)
			end = strchr(cp, '\0');
		*cpp = end + strspn(end, WHITESPACE);
	}

	i = end - cp;

	*path = xmalloc(i + 1);
	memcpy(*path, cp, i);
	(*path)[i] = '\0';
	return(0);

 fail:
	*path = NULL;
	return (-1);
}

static int
is_dir(char *path)
{
	struct _stat sb;

	/* XXX: report errors? */
	if (stat(path, &sb) == -1)
		return(0);

	return(sb.st_mode & _S_IFDIR);
}

static int
is_reg(char *path)
{
	struct _stat sb;

	if (stat(path, &sb) == -1) {
		fatal("stat %s: %s", path, strerror(errno));
		return(-1);
	}

	return(S_ISREG(sb.st_mode));
}

static int
remote_is_dir(struct sftp_conn *conn, char *path)
{
	Attrib *a;

	/* XXX: report errors? */
	if ((a = do_stat(conn, path, 1)) == NULL)
		return(0);
	if (!(a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS))
		return(0);
	return(a->perm & _S_IFDIR);
}

static int
process_get(struct sftp_conn *conn, char *src, char *dst, char *pwd, int pflag)
{
	char *abs_src = NULL;
	char *abs_dst = NULL;
	char *tmp;
	glob_t g;
	int err = 0;
	int i;

	abs_src = xstrdup(src);
	abs_src = make_absolute(abs_src, pwd);

	memset(&g, 0, sizeof(g));
	debug3("Looking up %s", abs_src);
	if (remote_glob(conn, abs_src, 0, NULL, &g)) {
		error("File \"%s\" not found.", abs_src);
		err = -1;
		goto out;
	}

	/* Only one match, dst may be file, directory or unspecified */
	if (g.gl_pathv[0] && g.gl_matchc == 1) {
		if (dst) {
			/* If directory specified, append filename */
			if (is_dir(dst)) {
				if (infer_path(g.gl_pathv[0], &tmp)) {
					err = 1;
					goto out;
				}
				abs_dst = path_append(dst, tmp);
				xfree(tmp);
			} else
				abs_dst = xstrdup(dst);
		} else if (infer_path(g.gl_pathv[0], &abs_dst)) {
			err = -1;
			goto out;
		}
		err = do_download(conn, g.gl_pathv[0], abs_dst, pflag);
		goto out;
	}

	/* Multiple matches, dst may be directory or unspecified */
	if (dst && !is_dir(dst)) {
		error("Multiple files match, but \"%s\" is not a directory",
		    dst);
		err = -1;
		goto out;
	}

	for (i = 0; g.gl_pathv[i]; i++) {
		if (infer_path(g.gl_pathv[i], &tmp)) {
			err = -1;
			goto out;
		}
		if (dst) {
			abs_dst = path_append(dst, tmp);
			xfree(tmp);
		} else
			abs_dst = tmp;

		printf("Fetching %s to %s\n", g.gl_pathv[i], abs_dst);
		if (do_download(conn, g.gl_pathv[i], abs_dst, pflag) == -1)
			err = -1;
		xfree(abs_dst);
		abs_dst = NULL;
	}

out:
	xfree(abs_src);
	if (abs_dst)
		xfree(abs_dst);
	globfree(&g);
	return(err);
}

static int
process_put(struct sftp_conn *conn, char *src, char *dst, char *pwd, int pflag)
{
	char *tmp_dst = NULL;
	char *abs_dst = NULL;
	char *tmp;
	glob_t g;
	int err = 0;
	int i;

	if (dst) {
		tmp_dst = xstrdup(dst);
		tmp_dst = make_absolute(tmp_dst, pwd);
	}

	memset(&g, 0, sizeof(g));
	debug3("Looking up %s", src);
	if (glob(src, 0, NULL, &g)) {
		error("File \"%s\" not found.", src);
		err = -1;
		goto out;
	}

	/* Only one match, dst may be file, directory or unspecified */
	if (g.gl_pathv[0] && g.gl_matchc == 1) {
		if (!is_reg(g.gl_pathv[0])) {
			error("Can't upload %s: not a regular file",
			    g.gl_pathv[0]);
			err = 1;
			goto out;
		}
		if (tmp_dst) {
			/* If directory specified, append filename */
			if (remote_is_dir(conn, tmp_dst)) {
				if (infer_path(g.gl_pathv[0], &tmp)) {
					err = 1;
					goto out;
				}
				abs_dst = path_append(tmp_dst, tmp);
				xfree(tmp);
			} else
				abs_dst = xstrdup(tmp_dst);
		} else {
			if (infer_path(g.gl_pathv[0], &abs_dst)) {
				err = -1;
				goto out;
			}
			abs_dst = make_absolute(abs_dst, pwd);
		}
		err = do_upload(conn, g.gl_pathv[0], abs_dst, pflag);
		goto out;
	}

	/* Multiple matches, dst may be directory or unspecified */
	if (tmp_dst && !remote_is_dir(conn, tmp_dst)) {
		error("Multiple files match, but \"%s\" is not a directory",
		    tmp_dst);
		err = -1;
		goto out;
	}

	for (i = 0; g.gl_pathv[i]; i++) {
		if (!is_reg(g.gl_pathv[i])) {
			error("skipping non-regular file %s", 
			    g.gl_pathv[i]);
			continue;
		}
		if (infer_path(g.gl_pathv[i], &tmp)) {
			err = -1;
			goto out;
		}
		if (tmp_dst) {
			abs_dst = path_append(tmp_dst, tmp);
			xfree(tmp);
		} else
			abs_dst = make_absolute(tmp, pwd);

		printf("Uploading %s to %s\n", g.gl_pathv[i], abs_dst);
		if (do_upload(conn, g.gl_pathv[i], abs_dst, pflag) == -1)
			err = -1;
	}

out:
	if (abs_dst)
		xfree(abs_dst);
	if (tmp_dst)
		xfree(tmp_dst);
	return(err);
}

static int
sdirent_comp(const void *aa, const void *bb)
{
	SFTP_DIRENT *a = *(SFTP_DIRENT **)aa;
	SFTP_DIRENT *b = *(SFTP_DIRENT **)bb;

	return (strcmp(a->filename, b->filename));
}

/* sftp ls.1 replacement for directories */
static int
do_ls_dir(struct sftp_conn *conn, char *path, char *strip_path, int lflag)
{
	int n;
	SFTP_DIRENT **d;

	if ((n = do_readdir(conn, path, &d)) != 0)
		return (n);

	/* Count entries for sort */
	for (n = 0; d[n] != NULL; n++)
		;

	qsort(d, n, sizeof(*d), sdirent_comp);

	for (n = 0; d[n] != NULL; n++) {
		char *tmp, *fname;

		tmp = path_append(path, d[n]->filename);
		fname = path_strip(tmp, strip_path);
		xfree(tmp);

		if (lflag) {
			char *lname;
			struct _stat sb;

			memset(&sb, 0, sizeof(sb));
			attrib_to_stat(&d[n]->a, &sb);
			lname = ls_file(fname, &sb, 1);
			printf("%s\n", lname);
			xfree(lname);
		} else {
			/* XXX - multicolumn display would be nice here */
			printf("%s\n", fname);
		}

		xfree(fname);
	}

	free_sftp_dirents(d);
	return (0);
}

/* sftp ls.1 replacement which handles path globs */
static int
do_globbed_ls(struct sftp_conn *conn, char *path, char *strip_path,
    int lflag)
{
	glob_t g;
	int i;
	Attrib *a;
	struct _stat sb;

	memset(&g, 0, sizeof(g));

	if (remote_glob(conn, path, GLOB_MARK|GLOB_NOCHECK|GLOB_BRACE,
	    NULL, &g)) {
		error("Can't ls: \"%s\" not found", path);
		return (-1);
	}

	/*
	 * If the glob returns a single match, which is the same as the
	 * input glob, and it is a directory, then just list its contents
	 */
	if (g.gl_pathc == 1 &&
	    strncmp(path, g.gl_pathv[0], strlen(g.gl_pathv[0]) - 1) == 0) {
		if ((a = do_lstat(conn, path, 1)) == NULL) {
			globfree(&g);
	    		return (-1);
		}
		if ((a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS) &&
		    S_ISDIR(a->perm)) {
			globfree(&g);
			return (do_ls_dir(conn, path, strip_path, lflag));
		}
	}

	for (i = 0; g.gl_pathv[i]; i++) {
		char *fname, *lname;

		fname = path_strip(g.gl_pathv[i], strip_path);

		if (lflag) {
			/*
			 * XXX: this is slow - 1 roundtrip per path
			 * A solution to this is to fork glob() and
			 * build a sftp specific version which keeps the
			 * attribs (which currently get thrown away)
			 * that the server returns as well as the filenames.
			 */
			memset(&sb, 0, sizeof(sb));
			a = do_lstat(conn, g.gl_pathv[i], 1);
			if (a != NULL)
				attrib_to_stat(a, &sb);
			lname = ls_file(fname, &sb, 1);
			printf("%s\n", lname);
			xfree(lname);
		} else {
			/* XXX - multicolumn display would be nice here */
			printf("%s\n", fname);
		}
		xfree(fname);
	}

	if (g.gl_pathc)
		globfree(&g);

	return (0);
}

static int
parse_args(const char **cpp, int *pflag, int *lflag, int *iflag,
    unsigned long *n_arg, char **path1, char **path2)
{
	const char *cmd, *cp = *cpp;
	char *cp2;
	int base = 0;
	long l;
	int i, cmdnum;

	/* Skip leading whitespace */
	cp = cp + strspn(cp, WHITESPACE);

	/* Ignore blank lines and lines which begin with comment '#' char */
	if (*cp == '\0' || *cp == '#')
		return (0);

	/* Check for leading '-' (disable error processing) */
	*iflag = 0;
	if (*cp == '-') {
		*iflag = 1;
		cp++;
	}
		
	/* Figure out which command we have */
	for (i = 0; cmds[i].c; i++) {
		int cmdlen = strlen(cmds[i].c);

		/* Check for command followed by whitespace */
		if (!ckstrcmp((char *)cp, (char *)cmds[i].c, cmdlen, 0) &&
		    strchr(WHITESPACE, cp[cmdlen])) {
			cp += cmdlen;
			cp = cp + strspn(cp, WHITESPACE);
			break;
		}
	}
	cmdnum = cmds[i].n;
	cmd = cmds[i].c;

    if (cmdnum == -1) {
		error("Invalid command.");
		return (-1);
	}

	/* Get arguments and parse flags */
	*lflag = *pflag = *n_arg = 0;
	*path1 = *path2 = NULL;
	switch (cmdnum) {
	case I_GET:
	case I_PUT:
		if (parse_getput_flags(&cp, pflag))
			return(-1);
		/* Get first pathname (mandatory) */
		if (get_pathname(&cp, path1))
			return(-1);
		if (*path1 == NULL) {
			error("You must specify at least one path after a "
			    "%s command.", cmd);
			return(-1);
		}
		/* Try to get second pathname (optional) */
		if (get_pathname(&cp, path2))
			return(-1);
		break;
	case I_RENAME:
	case I_SYMLINK:
		if (get_pathname(&cp, path1))
			return(-1);
		if (get_pathname(&cp, path2))
			return(-1);
		if (!*path1 || !*path2) {
			error("You must specify two paths after a %s "
			    "command.", cmd);
			return(-1);
		}
		break;
	case I_RM:
	case I_MKDIR:
	case I_RMDIR:
	case I_CHDIR:
	case I_LCHDIR:
	case I_LMKDIR:
		/* Get pathname (mandatory) */
		if (get_pathname(&cp, path1))
			return(-1);
		if (*path1 == NULL) {
			error("You must specify a path after a %s command.",
			    cmd);
			return(-1);
		}
		break;
	case I_LS:
		if (parse_ls_flags(&cp, lflag))
			return(-1);
		/* Path is optional */
		if (get_pathname(&cp, path1))
			return(-1);
		break;
	case I_LLS:
		/* Uses the rest of the line */
		break;
	case I_LUMASK:
		base = 8;
	case I_CHMOD:
		base = 8;
	case I_CHOWN:
	case I_CHGRP:
		/* Get numeric arg (mandatory) */
		l = strtol(cp, &cp2, base);
		if (cp2 == cp || ((l == LONG_MIN || l == LONG_MAX) &&
		    errno == ERANGE) || l < 0) {
			error("You must supply a numeric argument "
			    "to the %s command.", cmd);
			return(-1);
		}
		cp = cp2;
		*n_arg = l;
		if (cmdnum == I_LUMASK && strchr(WHITESPACE, *cp))
			break;
		if (cmdnum == I_LUMASK || !strchr(WHITESPACE, *cp)) {
			error("You must supply a numeric argument "
			    "to the %s command.", cmd);
			return(-1);
		}
		cp += strspn(cp, WHITESPACE);

		/* Get pathname (mandatory) */
		if (get_pathname(&cp, path1))
			return(-1);
		if (*path1 == NULL) {
			error("You must specify a path after a %s command.",
			    cmd);
			return(-1);
		}
		break;
	case I_QUIT:
	case I_PWD:
	case I_LPWD:
	case I_HELP:
	case I_VERSION:
	case I_PROGRESS:
		break;
	default:
		fatal("Command not implemented");
		return(-1);
	}

	*cpp = cp;
	return(cmdnum);
}

static int
parse_dispatch_command(struct sftp_conn *conn, const char *cmd, char **pwd,
    int err_abort)
{
	char *path1, *path2, *tmp;
	int pflag, lflag, iflag, cmdnum, i;
	unsigned long n_arg;
	Attrib a, *aa;
	char path_buf[MAXPATHLEN];
	int err = 0;
	glob_t g;

	path1 = path2 = NULL;
	cmdnum = parse_args(&cmd, &pflag, &lflag, &iflag, &n_arg,
	    &path1, &path2);

	if (iflag != 0)
		err_abort = 0;

	memset(&g, 0, sizeof(g));

	/* Perform command */
	switch (cmdnum) {
	case 0:
		/* Blank line */
		break;
	case -1:
		/* Unrecognized command */
		err = -1;
		break;
	case I_GET:
		err = process_get(conn, path1, path2, *pwd, pflag);
		break;
	case I_PUT:
		err = process_put(conn, path1, path2, *pwd, pflag);
		break;
	case I_RENAME:
		path1 = make_absolute(path1, *pwd);
		path2 = make_absolute(path2, *pwd);
		err = do_rename(conn, path1, path2);
		break;
	case I_SYMLINK:
		path2 = make_absolute(path2, *pwd);
		err = do_symlink(conn, path1, path2);
		break;
	case I_RM:
		path1 = make_absolute(path1, *pwd);
		remote_glob(conn, path1, GLOB_NOCHECK, NULL, &g);
		for (i = 0; g.gl_pathv[i]; i++) {
			printf("Removing %s\n", g.gl_pathv[i]);
			err = do_rm(conn, g.gl_pathv[i]);
			if (err != 0 && err_abort)
				break;
		}
		break;
	case I_MKDIR:
		path1 = make_absolute(path1, *pwd);
		attrib_clear(&a);
		a.flags |= SSH2_FILEXFER_ATTR_PERMISSIONS;
		a.perm = 0777;
		err = do_mkdir(conn, path1, &a);
		break;
	case I_RMDIR:
		path1 = make_absolute(path1, *pwd);
		err = do_rmdir(conn, path1);
		break;
	case I_CHDIR:
		path1 = make_absolute(path1, *pwd);
		if ((tmp = do_realpath(conn, path1)) == NULL) {
			err = 1;
			break;
		}
		if ((aa = do_stat(conn, tmp, 0)) == NULL) {
			xfree(tmp);
			err = 1;
			break;
		}
		if (!(aa->flags & SSH2_FILEXFER_ATTR_PERMISSIONS)) {
			error("Can't change directory: Can't check target");
			xfree(tmp);
			err = 1;
			break;
		}
		if (!S_ISDIR(aa->perm)) {
			error("Can't change directory: \"%s\" is not "
			    "a directory", tmp);
			xfree(tmp);
			err = 1;
			break;
		}
		xfree(*pwd);
		*pwd = tmp;
		break;
	case I_LS:
		if (!path1) {
			do_globbed_ls(conn, *pwd, *pwd, lflag);
			break;
		}

		/* Strip pwd off beginning of non-absolute paths */
		tmp = NULL;
		if (*path1 != '/')
			tmp = *pwd;

		path1 = make_absolute(path1, *pwd);
		err = do_globbed_ls(conn, path1, tmp, lflag);
		break;
	case I_LCHDIR:
		if (chdir(path1) == -1) {
			error("Couldn't change local directory to "
			    "\"%s\": %s", path1, strerror(errno));
			err = 1;
		}
		break;
	case I_LMKDIR:
		if (mkdir(path1, 0777) == -1) {
			error("Couldn't create local directory "
			    "\"%s\": %s", path1, strerror(errno));
			err = 1;
		}
		break;
	case I_LLS:
		local_do_ls(cmd);
		break;
	case I_LUMASK:
		umask(n_arg);
		printf("Local umask: %03lo\n", n_arg);
		break;
	case I_CHMOD:
		path1 = make_absolute(path1, *pwd);
		attrib_clear(&a);
		a.flags |= SSH2_FILEXFER_ATTR_PERMISSIONS;
		a.perm = n_arg;
		remote_glob(conn, path1, GLOB_NOCHECK, NULL, &g);
		for (i = 0; g.gl_pathv[i]; i++) {
			printf("Changing mode on %s\n", g.gl_pathv[i]);
			err = do_setstat(conn, g.gl_pathv[i], &a);
			if (err != 0 && err_abort)
				break;
		}
		break;
	case I_CHOWN:
	case I_CHGRP:
		path1 = make_absolute(path1, *pwd);
		remote_glob(conn, path1, GLOB_NOCHECK, NULL, &g);
		for (i = 0; g.gl_pathv[i]; i++) {
			if (!(aa = do_stat(conn, g.gl_pathv[i], 0))) {
				if (err != 0 && err_abort)
					break;
				else
					continue;
			}
			if (!(aa->flags & SSH2_FILEXFER_ATTR_UIDGID)) {
				error("Can't get current ownership of "
				    "remote file \"%s\"", g.gl_pathv[i]);
				if (err != 0 && err_abort)
					break;
				else
					continue;
			}
			aa->flags &= SSH2_FILEXFER_ATTR_UIDGID;
			if (cmdnum == I_CHOWN) {
				printf("Changing owner on %s\n", g.gl_pathv[i]);
				aa->uid = n_arg;
			} else {
				printf("Changing group on %s\n", g.gl_pathv[i]);
				aa->gid = n_arg;
			}
			err = do_setstat(conn, g.gl_pathv[i], aa);
			if (err != 0 && err_abort)
				break;
		}
		break;
	case I_PWD:
		printf("Remote working directory: %s\n", *pwd);
		break;
	case I_LPWD:
		if (!getcwd(path_buf, sizeof(path_buf))) {
			error("Couldn't get local cwd: %s", strerror(errno));
			err = -1;
			break;
		}
		printf("Local working directory: %s\n", path_buf);
		break;
	case I_QUIT:
		/* Processed below */
		break;
	case I_HELP:
		help();
		break;
	case I_VERSION:
		printf("SFTP protocol version %u\n", sftp_proto_version(conn));
		break;
	case I_PROGRESS:
		showprogress = !showprogress;
		if (showprogress)
			printf("Progress meter enabled\n");
		else
			printf("Progress meter disabled\n");
		break;
	default:
		fatal("%d is not implemented", cmdnum);
		return(-1);
	}

	if (g.gl_pathc)
		globfree(&g);
	if (path1)
		xfree(path1);
	if (path2)
		xfree(path2);

	/* If an unignored error occurs in batch mode we should abort. */
	if (err_abort && err != 0)
		return (-1);
	else if (cmdnum == I_QUIT)
		return (1);

	return (0);
}

static 	char *pwd = NULL;
static  struct sftp_conn *conn = NULL;

int
sftp_do_init(void)
{
    if ( conn ) {
        xfree(conn);
        conn = NULL;
    }
    if ( pwd ) {
        xfree(pwd);
        pwd = NULL;
    }

    conn = do_init(copy_buffer_len, num_requests);
    if (conn == NULL) {
        fatal("Couldn't initialise connection to server");
        return 0;
    }
    pwd = do_realpath(conn, ".");
    if (pwd == NULL) {
        fatal("Need cwd");
        return 0;
    }

    return 1;
}

int
sftp_do_cmd(int kval, char * args)
{
    char cmd[2048]="";
    int err;

    switch ( kval ) {
    case SFTP_CD:
        ckstrncpy(cmd,"cd ",sizeof(cmd));
        break;
    case SFTP_CHGRP:
        ckstrncpy(cmd,"chgrp ",sizeof(cmd));
        break;
    case SFTP_CHMOD:
        ckstrncpy(cmd,"chmod ",sizeof(cmd));
        break;
    case SFTP_CHOWN:
        ckstrncpy(cmd,"chown ",sizeof(cmd));
        break;
    case SFTP_RM:
        ckstrncpy(cmd,"rm ",sizeof(cmd));
        break;
    case SFTP_DIR:
        ckstrncpy(cmd,"ls ",sizeof(cmd));
        break;
    case SFTP_GET:
        ckstrncpy(cmd,"get ",sizeof(cmd));
        break;
    case SFTP_MKDIR:
        ckstrncpy(cmd,"mkdir ",sizeof(cmd));
        break;
    case SFTP_PUT:
        ckstrncpy(cmd,"put ",sizeof(cmd));
        break;
    case SFTP_PWD:
        ckstrncpy(cmd,"pwd ",sizeof(cmd));
        break;
    case SFTP_REN:
        ckstrncpy(cmd,"rename ",sizeof(cmd));
        break;
    case SFTP_RMDIR:
        ckstrncpy(cmd,"rmdir ",sizeof(cmd));
        break;
    case SFTP_LINK:
        ckstrncpy(cmd,"symlink ",sizeof(cmd));
        break;
    case SFTP_VER:
        ckstrncpy(cmd,"version ",sizeof(cmd));
        break;
    }

    ckstrncat(cmd,args,sizeof(cmd)); 

    err = parse_dispatch_command(conn, cmd, &pwd, 0);
    return (err >= 0 ? 1 : 0);
}

/* Clear contents of attributes structure */
void
attrib_clear(Attrib *a)
{
	a->flags = 0;
	a->size = 0;
	a->uid = 0;
	a->gid = 0;
	a->perm = 0;
	a->atime = 0;
	a->mtime = 0;
}

/* Convert from struct _stat to filexfer attribs */
void
stat_to_attrib(struct _stat *st, Attrib *a)
{
	attrib_clear(a);
	a->flags = 0;
	a->flags |= SSH2_FILEXFER_ATTR_SIZE;
	a->size = st->st_size;
	a->flags |= SSH2_FILEXFER_ATTR_UIDGID;
	a->uid = st->st_uid;
	a->gid = st->st_gid;
	a->flags |= SSH2_FILEXFER_ATTR_PERMISSIONS;
	a->perm = st->st_mode;
	a->flags |= SSH2_FILEXFER_ATTR_ACMODTIME;
	a->atime = st->st_atime;
	a->mtime = st->st_mtime;
}

/* Convert from filexfer attribs to struct _stat */
void
attrib_to_stat(Attrib *a, struct _stat *st)
{
	memset(st, 0, sizeof(*st));

	if (a->flags & SSH2_FILEXFER_ATTR_SIZE)
		st->st_size = a->size;
	if (a->flags & SSH2_FILEXFER_ATTR_UIDGID) {
		st->st_uid = a->uid;
		st->st_gid = a->gid;
	}
	if (a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS)
		st->st_mode = a->perm;
	if (a->flags & SSH2_FILEXFER_ATTR_ACMODTIME) {
		st->st_atime = a->atime;
		st->st_mtime = a->mtime;
	}
}

/* Decode attributes in buffer */
Attrib *
decode_attrib(Buffer *b)
{
	static Attrib a;

	attrib_clear(&a);
	a.flags = buffer_get_int(b);
	if (a.flags & SSH2_FILEXFER_ATTR_SIZE)
		a.size = buffer_get_int64(b);
	if (a.flags & SSH2_FILEXFER_ATTR_UIDGID) {
		a.uid = buffer_get_int(b);
		a.gid = buffer_get_int(b);
	}
	if (a.flags & SSH2_FILEXFER_ATTR_PERMISSIONS)
		a.perm = buffer_get_int(b);
	if (a.flags & SSH2_FILEXFER_ATTR_ACMODTIME) {
		a.atime = buffer_get_int(b);
		a.mtime = buffer_get_int(b);
	}
	/* vendor-specific extensions */
	if (a.flags & SSH2_FILEXFER_ATTR_EXTENDED) {
		char *type, *data;
		int i, count;

		count = buffer_get_int(b);
		for (i = 0; i < count; i++) {
			type = buffer_get_string(b, NULL);
			data = buffer_get_string(b, NULL);
			debug3("Got file attribute \"%s\"", type);
			xfree(type);
			xfree(data);
		}
	}
	return &a;
}

/* Encode attributes to buffer */
void
encode_attrib(Buffer *b, Attrib *a)
{
	buffer_put_int(b, a->flags);
	if (a->flags & SSH2_FILEXFER_ATTR_SIZE)
		buffer_put_int64(b, a->size);
	if (a->flags & SSH2_FILEXFER_ATTR_UIDGID) {
		buffer_put_int(b, a->uid);
		buffer_put_int(b, a->gid);
	}
	if (a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS)
		buffer_put_int(b, a->perm);
	if (a->flags & SSH2_FILEXFER_ATTR_ACMODTIME) {
		buffer_put_int(b, a->atime);
		buffer_put_int(b, a->mtime);
	}
}

/* Convert from SSH2_FX_ status to text error message */
const char *
fx2txt(int status)
{
	switch (status) {
	case SSH2_FX_OK:
		return("No error");
	case SSH2_FX_EOF:
		return("End of file");
	case SSH2_FX_NO_SUCH_FILE:
		return("No such file or directory");
	case SSH2_FX_PERMISSION_DENIED:
		return("Permission denied");
	case SSH2_FX_FAILURE:
		return("Failure");
	case SSH2_FX_BAD_MESSAGE:
		return("Bad message");
	case SSH2_FX_NO_CONNECTION:
		return("No connection");
	case SSH2_FX_CONNECTION_LOST:
		return("Connection lost");
	case SSH2_FX_OP_UNSUPPORTED:
		return("Operation unsupported");
	default:
		return("Unknown status");
	}
	/* NOTREACHED */
}

/*
 * drwxr-xr-x    5 markus   markus       1024 Jan 13 18:39 .ssh
 */
char *
ls_file(char *name, struct _stat *st, int remote)
{
	int ulen, glen, sz = 0;
	struct passwd *pw;
	struct group *gr;
	struct tm *ltime = localtime(&st->st_mtime);
	char *user, *group;
	char buf[1024], mode[11+1], tbuf[12+1], ubuf[11+1], gbuf[11+1];

	strmode(st->st_mode, mode);
	if (!remote && (pw = getpwuid(st->st_uid)) != NULL) {
		user = pw->pw_name;
	} else {
		snprintf(ubuf, sizeof ubuf, "%u", (u_int)st->st_uid);
		user = ubuf;
	}
	if (!remote && (gr = getgrgid(st->st_gid)) != NULL) {
		group = gr->gr_name;
	} else {
		snprintf(gbuf, sizeof gbuf, "%u", (u_int)st->st_gid);
		group = gbuf;
	}
	if (ltime != NULL) {
		if (time(NULL) - st->st_mtime < (365*24*60*60)/2)
			sz = strftime(tbuf, sizeof tbuf, "%b %d %H:%M", ltime);
		else
			sz = strftime(tbuf, sizeof tbuf, "%b %d  %Y", ltime);
	}
	if (sz == 0)
		tbuf[0] = '\0';
	ulen = MAX(strlen(user), 8);
	glen = MAX(strlen(group), 8);
	snprintf(buf, sizeof buf, "%s %3d %-*s %-*s %8lu %s %s", mode,
	    st->st_nlink, ulen, user, glen, group,
	    st->st_size, tbuf, name);
	return xstrdup(buf);
}

struct SFTP_OPENDIR {
	SFTP_DIRENT **dir;
	int offset;
};

static struct {
	struct sftp_conn *conn;
} cur;

static void *
fudge_opendir(const char *path)
{
	struct SFTP_OPENDIR *r;

	r = xmalloc(sizeof(*r));

	if (do_readdir(cur.conn, (char *)path, &r->dir)) {
		xfree(r);
		return(NULL);
	}

	r->offset = 0;

	return((void *)r);
}

static struct dirent *
fudge_readdir(struct SFTP_OPENDIR *od)
{
	/* Solaris needs sizeof(dirent) + path length (see below) */
	static char buf[sizeof(struct dirent) + MAXPATHLEN];
	struct dirent *ret = (struct dirent *)buf;
	
	if (od->dir[od->offset] == NULL)
		return(NULL);

	memset(buf, 0, sizeof(buf));

	/*
	 * Solaris defines dirent->d_name as a one byte array and expects
	 * you to hack around it.
	 */
#ifdef BROKEN_ONE_BYTE_DIRENT_D_NAME
	strlcpy(ret->d_name, od->dir[od->offset++]->filename, MAXPATHLEN);
#else
	strlcpy(ret->d_name, od->dir[od->offset++]->filename,
	    sizeof(ret->d_name));
#endif
	return(ret);
}

static void
fudge_closedir(struct SFTP_OPENDIR *od)
{
	free_sftp_dirents(od->dir);
	xfree(od);
}

static int
fudge_lstat(const char *path, struct _stat *st)
{
	Attrib *a;

	if (!(a = do_lstat(cur.conn, (char *)path, 0)))
		return(-1);

	attrib_to_stat(a, st);

	return(0);
}

static int
fudge_stat(const char *path, struct _stat *st)
{
	Attrib *a;

	if (!(a = do_stat(cur.conn, (char *)path, 0)))
		return(-1);

	attrib_to_stat(a, st);

	return(0);
}

int
remote_glob(struct sftp_conn *conn, const char *pattern, int flags,
    int (*errfunc)(const char *, int), glob_t *pglob)
{
	pglob->gl_opendir = fudge_opendir;
	pglob->gl_readdir = (struct dirent *(*)(void *))fudge_readdir;
	pglob->gl_closedir = (void (*)(void *))fudge_closedir;
	pglob->gl_lstat = fudge_lstat;
	pglob->gl_stat = fudge_stat;

	memset(&cur, 0, sizeof(cur));
	cur.conn = conn;

	return(glob(pattern, flags | GLOB_ALTDIRFUNC, errfunc, pglob));
}
#endif /* SFTP_BUILTIN */