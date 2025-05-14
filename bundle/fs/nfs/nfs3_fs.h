/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2014 Anna Schumaker.
 *
 * NFSv3-specific filesystem definitions and declarations
 */
#ifndef __LINUX_FS_NFS_NFS3_FS_H
#define __LINUX_FS_NFS_NFS3_FS_H

/*
 * nfs3acl.c
 */
#ifdef CONFIG_NFS_V3_ACL
#if defined(COMPAT_DETECT_GET_ACL_RCU_PARAM)
extern struct posix_acl *nfs3_get_acl(struct inode *inode, int type, bool rcu);
#else
extern struct posix_acl *nfs3_get_acl(struct inode *inode, int type);
#endif
extern int nfs3_set_acl(COMPAT_INIT_USER_NS_PARAM_N(mnt_userns) struct SET_ACL_PARAM_TYPE *param,
			struct posix_acl *acl, int type);
extern int nfs3_proc_setacls(struct inode *inode, struct posix_acl *acl,
		struct posix_acl *dfacl);
extern ssize_t nfs3_listxattr(struct dentry *, char *, size_t);
#if defined(COMPAT_DETECT_POSIX_ACL_ACCESS_XATTR_HANDLER)
extern const struct xattr_handler *nfs3_xattr_handlers[];
#endif
#else
static inline int nfs3_proc_setacls(struct inode *inode, struct posix_acl *acl,
		struct posix_acl *dfacl)
{
	return 0;
}
#define nfs3_listxattr NULL
#endif /* CONFIG_NFS_V3_ACL */

/* nfs3client.c */
struct nfs_server *nfs3_create_server(struct fs_context *);
struct nfs_server *nfs3_clone_server(struct nfs_server *, struct nfs_fh *,
				     struct nfs_fattr *, rpc_authflavor_t);

/* nfs3super.c */
extern struct nfs_subversion nfs_v3;

#endif /* __LINUX_FS_NFS_NFS3_FS_H */
