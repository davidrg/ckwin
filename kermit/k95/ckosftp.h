/*	$OpenBSD: sftp-common.h,v 1.4 2002/09/11 22:41:50 djm Exp $	*/

/*
 * Copyright (c) 2001 Markus Friedl.  All rights reserved.
 * Copyright (c) 2001 Damien Miller.  All rights reserved.
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

typedef struct Attrib Attrib;

/* File attributes */
struct Attrib {
	u_int32_t	flags;
	u_int64_t	size;
	u_int32_t	uid;
	u_int32_t	gid;
	u_int32_t	perm;
	u_int32_t	atime;
	u_int32_t	mtime;
};

void	 attrib_clear(Attrib *);
void	 stat_to_attrib(struct _stat *, Attrib *);
void	 attrib_to_stat(Attrib *, struct _stat *);
Attrib	*decode_attrib(Buffer *);
void	 encode_attrib(Buffer *, Attrib *);
char	*ls_file(char *, struct _stat *, int);

const char *fx2txt(int);
/* $OpenBSD: sftp-glob.h,v 1.8 2002/09/11 22:41:50 djm Exp $ */

/*
 * Copyright (c) 2001,2002 Damien Miller.  All rights reserved.
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

/* Remote sftp filename globbing */

#ifndef _SFTP_GLOB_H
#define _SFTP_GLOB_H

int remote_glob(struct sftp_conn *, const char *, int,
    int (*)(const char *, int), glob_t *);

#endif
/* $OpenBSD: sftp-client.h,v 1.11 2002/09/11 22:41:50 djm Exp $ */

/*
 * Copyright (c) 2001,2002 Damien Miller.  All rights reserved.
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

/* Client side of SSH2 filexfer protocol */

#ifndef _SFTP_CLIENT_H
#define _SFTP_CLIENT_H

typedef struct SFTP_DIRENT SFTP_DIRENT;

struct SFTP_DIRENT {
	char *filename;
	char *longname;
	Attrib a;
};

/*
 * Initialiase a SSH filexfer connection. Returns -1 on error or
 * protocol version on success.
 */
struct sftp_conn *do_init(u_int, u_int);

u_int sftp_proto_version(struct sftp_conn *);

/* Close file referred to by 'handle' */
int do_close(struct sftp_conn *, char *, u_int);

/* Read contents of 'path' to NULL-terminated array 'dir' */
int do_readdir(struct sftp_conn *, char *, SFTP_DIRENT ***);

/* Frees a NULL-terminated array of SFTP_DIRENTs (eg. from do_readdir) */
void free_sftp_dirents(SFTP_DIRENT **);

/* Delete file 'path' */
int do_rm(struct sftp_conn *, char *);

/* Create directory 'path' */
int do_mkdir(struct sftp_conn *, char *, Attrib *);

/* Remove directory 'path' */
int do_rmdir(struct sftp_conn *, char *);

/* Get file attributes of 'path' (follows symlinks) */
Attrib *do_stat(struct sftp_conn *, char *, int);

/* Get file attributes of 'path' (does not follow symlinks) */
Attrib *do_lstat(struct sftp_conn *, char *, int);

/* Get file attributes of open file 'handle' */
Attrib *do_fstat(struct sftp_conn *, char *, u_int, int);

/* Set file attributes of 'path' */
int do_setstat(struct sftp_conn *, char *, Attrib *);

/* Set file attributes of open file 'handle' */
int do_fsetstat(struct sftp_conn *, char *, u_int, Attrib *);

/* Canonicalise 'path' - caller must free result */
char *do_realpath(struct sftp_conn *, char *);

/* Rename 'oldpath' to 'newpath' */
int do_rename(struct sftp_conn *, char *, char *);

/* Rename 'oldpath' to 'newpath' */
int do_symlink(struct sftp_conn *, char *, char *);

/* Return target of symlink 'path' - caller must free result */
char *do_readlink(struct sftp_conn *, char *);

/* XXX: add callbacks to do_download/do_upload so we can do progress meter */

/*
 * Download 'remote_path' to 'local_path'. Preserve permissions and times
 * if 'pflag' is set
 */
int do_download(struct sftp_conn *, char *, char *, int);

/*
 * Upload 'local_path' to 'remote_path'. Preserve permissions and times
 * if 'pflag' is set
 */
int do_upload(struct sftp_conn *, char *, char *, int);

#endif
/* $OpenBSD: sftp-int.h,v 1.6 2003/01/08 23:53:26 djm Exp $ */

/*
 * Copyright (c) 2001,2002 Damien Miller.  All rights reserved.
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

int	 interactive_loop(int, int, char *, char *);
/*	$OpenBSD: sftp.h,v 1.4 2002/02/13 00:59:23 djm Exp $	*/

/*
 * Copyright (c) 2001 Markus Friedl.  All rights reserved.
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

/*
 * draft-ietf-secsh-filexfer-01.txt
 */

/* version */
#define	SSH2_FILEXFER_VERSION		3

/* client to server */
#define SSH2_FXP_INIT			1
#define SSH2_FXP_OPEN			3
#define SSH2_FXP_CLOSE			4
#define SSH2_FXP_READ			5
#define SSH2_FXP_WRITE			6
#define SSH2_FXP_LSTAT			7
#define SSH2_FXP_STAT_VERSION_0		7
#define SSH2_FXP_FSTAT			8
#define SSH2_FXP_SETSTAT		9
#define SSH2_FXP_FSETSTAT		10
#define SSH2_FXP_OPENDIR		11
#define SSH2_FXP_READDIR		12
#define SSH2_FXP_REMOVE			13
#define SSH2_FXP_MKDIR			14
#define SSH2_FXP_RMDIR			15
#define SSH2_FXP_REALPATH		16
#define SSH2_FXP_STAT			17
#define SSH2_FXP_RENAME			18
#define SSH2_FXP_READLINK		19
#define SSH2_FXP_SYMLINK		20

/* server to client */
#define SSH2_FXP_VERSION		2
#define SSH2_FXP_STATUS			101
#define SSH2_FXP_HANDLE			102
#define SSH2_FXP_DATA			103
#define SSH2_FXP_NAME			104
#define SSH2_FXP_ATTRS			105

#define SSH2_FXP_EXTENDED		200
#define SSH2_FXP_EXTENDED_REPLY		201

/* attributes */
#define SSH2_FILEXFER_ATTR_SIZE		0x00000001
#define SSH2_FILEXFER_ATTR_UIDGID	0x00000002
#define SSH2_FILEXFER_ATTR_PERMISSIONS	0x00000004
#define SSH2_FILEXFER_ATTR_ACMODTIME	0x00000008
#define SSH2_FILEXFER_ATTR_EXTENDED	0x80000000

/* portable open modes */
#define SSH2_FXF_READ			0x00000001
#define SSH2_FXF_WRITE			0x00000002
#define SSH2_FXF_APPEND			0x00000004
#define SSH2_FXF_CREAT			0x00000008
#define SSH2_FXF_TRUNC			0x00000010
#define SSH2_FXF_EXCL			0x00000020

/* status messages */
#define SSH2_FX_OK			0
#define SSH2_FX_EOF			1
#define SSH2_FX_NO_SUCH_FILE		2
#define SSH2_FX_PERMISSION_DENIED	3
#define SSH2_FX_FAILURE			4
#define SSH2_FX_BAD_MESSAGE		5
#define SSH2_FX_NO_CONNECTION		6
#define SSH2_FX_CONNECTION_LOST		7
#define SSH2_FX_OP_UNSUPPORTED		8
#define SSH2_FX_MAX			8
