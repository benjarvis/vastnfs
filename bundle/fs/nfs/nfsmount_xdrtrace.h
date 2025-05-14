/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022 Dan Aloni <dan.aloni@vastdata.com>
 */
#undef TRACE_SYSTEM
#undef TRACE_INCLUDE_FILE
#define TRACE_SYSTEM nfs

#if !defined(_TRACE_NFSMOUNT_XDR_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_NFSMOUNT_XDR_H

#include <linux/tracepoint.h>
#include <linux/iversion.h>
#include <linux/nfs_fs_sb.h>
#include "internal.h"

#include "nfs3trace_classes.h"

DEFINE_NFS3_PROG_EVENT(mnt_encode_dirpath_request,
		       mnt_decode_mountres_response);
DEFINE_NFS3_PROG_EVENT_DECODE(mnt_decode_mountres3_response);

#endif /* _TRACE_NFSMOUNT_XDR_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE nfsmount_xdrtrace
/* This part must be outside protection */
#include <trace/define_trace.h>
