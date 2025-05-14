/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022 Dan Aloni <dan.aloni@vastdata.com>
 */
#undef TRACE_SYSTEM
#undef TRACE_INCLUDE_FILE
#define TRACE_SYSTEM nfs4

#if !defined(_TRACE_NFS4XDR_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_NFS4XDR_H

#include <linux/tracepoint.h>

#if defined(CONFIG_NFS_V4_1)

#include "nfs4trace_classes.h"

DEFINE_NFS4_PROG_EVENT(nfs4_encode_read_request,
		       nfs4_decode_read_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_write_request,
		       nfs4_decode_write_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_commit_request,
		       nfs4_decode_commit_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_open_request,
		       nfs4_decode_open_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_open_confirm_request,
		       nfs4_decode_open_confirm_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_open_noattr_request,
		       nfs4_decode_open_noattr_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_open_downgrade_request,
		       nfs4_decode_open_downgrade_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_close_request,
		       nfs4_decode_close_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_setattr_request,
		       nfs4_decode_setattr_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_fsinfo_request,
		       nfs4_decode_fsinfo_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_renew_request,
		       nfs4_decode_renew_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_setclientid_request,
		       nfs4_decode_setclientid_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_setclientid_confirm_request,
		       nfs4_decode_setclientid_confirm_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_lock_request,
		       nfs4_decode_lock_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_lockt_request,
		       nfs4_decode_lockt_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_locku_request,
		       nfs4_decode_locku_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_access_request,
		       nfs4_decode_access_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_getattr_request,
		       nfs4_decode_getattr_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_lookup_request,
		       nfs4_decode_lookup_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_lookup_root_request,
		       nfs4_decode_lookup_root_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_remove_request,
		       nfs4_decode_remove_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_rename_request,
		       nfs4_decode_rename_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_link_request,
		       nfs4_decode_link_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_symlink_request,
		       nfs4_decode_symlink_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_create_request,
		       nfs4_decode_create_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_pathconf_request,
		       nfs4_decode_pathconf_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_statfs_request,
		       nfs4_decode_statfs_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_readlink_request,
		       nfs4_decode_readlink_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_readdir_request,
		       nfs4_decode_readdir_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_server_caps_request,
		       nfs4_decode_server_caps_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_delegreturn_request,
		       nfs4_decode_delegreturn_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_getacl_request,
		       nfs4_decode_getacl_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_setacl_request,
		       nfs4_decode_setacl_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_fs_locations_request,
		       nfs4_decode_fs_locations_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_release_lockowner_request,
		       nfs4_decode_release_lockowner_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_secinfo_request,
		       nfs4_decode_secinfo_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_fsid_present_request,
		       nfs4_decode_fsid_present_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_exchange_id_request,
		       nfs4_decode_exchange_id_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_create_session_request,
		       nfs4_decode_create_session_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_destroy_session_request,
		       nfs4_decode_destroy_session_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_sequence_request,
		       nfs4_decode_sequence_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_get_lease_time_request,
		       nfs4_decode_get_lease_time_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_reclaim_complete_request,
		       nfs4_decode_reclaim_complete_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_getdeviceinfo_request,
		       nfs4_decode_getdeviceinfo_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_layoutget_request,
		       nfs4_decode_layoutget_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_layoutcommit_request,
		       nfs4_decode_layoutcommit_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_layoutreturn_request,
		       nfs4_decode_layoutreturn_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_secinfo_no_name_request,
		       nfs4_decode_secinfo_no_name_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_test_stateid_request,
		       nfs4_decode_test_stateid_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_free_stateid_request,
		       nfs4_decode_free_stateid_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_bind_conn_to_session_request,
		       nfs4_decode_bind_conn_to_session_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_destroy_clientid_request,
		       nfs4_decode_destroy_clientid_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_seek_request,
		       nfs4_decode_seek_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_allocate_request,
		       nfs4_decode_allocate_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_deallocate_request,
		       nfs4_decode_deallocate_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_layoutstats_request,
		       nfs4_decode_layoutstats_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_clone_request,
		       nfs4_decode_clone_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_copy_request,
		       nfs4_decode_copy_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_offload_cancel_request,
		       nfs4_decode_offload_cancel_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_copy_notify_request,
		       nfs4_decode_copy_notify_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_lookupp_request,
		       nfs4_decode_lookupp_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_layouterror_request,
		       nfs4_decode_layouterror_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_getxattr_request,
		       nfs4_decode_getxattr_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_setxattr_request,
		       nfs4_decode_setxattr_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_listxattrs_request,
		       nfs4_decode_listxattrs_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_removexattr_request,
		       nfs4_decode_removexattr_response);
DEFINE_NFS4_PROG_EVENT(nfs4_encode_read_plus_request,
		       nfs4_decode_read_plus_response);

#endif /* CONFIG_NFS_V4_1 */

#endif /* _TRACE_NFS4XDR_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE nfs4xdrtrace
/* This part must be outside protection */
#include <trace/define_trace.h>
