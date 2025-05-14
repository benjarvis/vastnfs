/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022 Dan Aloni <dan.aloni@vastdata.com>
 */
#undef TRACE_SYSTEM
#undef TRACE_INCLUDE_FILE
#define TRACE_SYSTEM nfs

#if !defined(_TRACE_NFSXDR_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_NFSXDR_H

#include <linux/tracepoint.h>
#include <linux/iversion.h>
#include <linux/nfs_fs_sb.h>
#include "internal.h"

#include "nfs3trace_classes.h"

DEFINE_NFS3_PROG_EVENT(nfs3_encode_getattr_request,
		       nfs3_decode_getattr_response);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_setattr_request,
		       nfs3_decode_setattr_response);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_lookup_request, nfs3_decode_lookup_response);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_access_request, nfs3_decode_access_response);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_readlink_request, nfs3_decode_readlink_response);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_read_request, nfs3_decode_read_response);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_write_request, nfs3_decode_write_response);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_create_request, nfs3_decode_create_response);
DEFINE_NFS3_PROG_EVENT_ENCODE(nfs3_encode_mkdir_request);
DEFINE_NFS3_PROG_EVENT_ENCODE(nfs3_encode_symlink_request);
DEFINE_NFS3_PROG_EVENT_ENCODE(nfs3_encode_mknod_request);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_rename_request, nfs3_decode_rename_response);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_remove_request, nfs3_decode_remove_response);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_link_request, nfs3_decode_link_response);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_readdir_request, nfs3_decode_readdir_response);
DEFINE_NFS3_PROG_EVENT_ENCODE(nfs3_encode_readdirplus_request);
DEFINE_NFS3_PROG_EVENT_DECODE(nfs3_decode_fsstat_response);
DEFINE_NFS3_PROG_EVENT_DECODE(nfs3_decode_fsinfo_response);
DEFINE_NFS3_PROG_EVENT_DECODE(nfs3_decode_pathconf_response);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_commit_request, nfs3_decode_commit_response);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_setacl_request, nfs3_decode_setacl_response);
DEFINE_NFS3_PROG_EVENT(nfs3_encode_getacl_request, nfs3_decode_getacl_response);

#endif /* _TRACE_NFSXDR_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE nfsxdrtrace
/* This part must be outside protection */
#include <trace/define_trace.h>
