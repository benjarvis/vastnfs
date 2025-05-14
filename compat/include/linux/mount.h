#ifndef __COMPAT_MOUNT_H__
#define __COMPAT_MOUNT_H__

#include_next <linux/mount.h>
#include <linux/cred.h>

#ifndef COMPAT_DETECT_MNT_USER_NS
static inline struct user_namespace *mnt_user_ns(const struct vfsmount *mnt)
{
	return &init_user_ns;
}

#endif

#ifndef COMPAT_DETECT_INCLUDE_FS_CONTEXT
#include <linux/fs_context_backport.h>

extern struct vfsmount *compat_vfs_create_mount(struct fs_context *fc);
#define vfs_create_mount compat_vfs_create_mount
extern struct vfsmount *compat_fc_mount(struct fs_context *fc);
#define fc_mount compat_fc_mount

#endif

#endif
