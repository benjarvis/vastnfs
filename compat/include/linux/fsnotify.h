#ifndef __COMPAT_LINUX_FS_NOTIFY_H__
#define __COMPAT_LINUX_FS_NOTIFY_H__

#include_next <linux/fsnotify.h>
#include_next <linux/fsnotify_backend.h>

#ifndef COMPAT_DETECT_FSNOTIFY_RMDIR
static inline void fsnotify_rmdir(struct inode *dir, struct dentry *dentry)
{
#ifdef COMPAT_DETECT_FSNOTIFY_NAMEREMOVE
	fsnotify_nameremove(dentry, 1);
#endif
}
#endif

#ifndef COMPAT_DETECT_FSNOTIFY_UNLINK
static inline void fsnotify_unlink(struct inode *dir, struct dentry *dentry)
{
#ifdef COMPAT_DETECT_FSNOTIFY_NAMEREMOVE
	fsnotify_nameremove(dentry, 0);
#endif
}
#endif

#ifndef COMPAT_DETECT_FSNOTIFY_WAIT_MARKS_DESTROYED
static inline void fsnotify_wait_marks_destroyed(void)
{
}
#endif

#ifndef COMPAT_DETECT_FSNOTIFY_DENTRY
static inline void fsnotify_dentry(struct dentry *dentry, __u32 mask)
{
       struct inode *inode = d_inode(dentry);

       if (S_ISDIR(inode->i_mode))
               mask |= FS_ISDIR;

       fsnotify_parent(NULL, dentry, mask);
       fsnotify(inode, mask, inode, FSNOTIFY_EVENT_INODE, NULL, 0);
}
#endif


#endif
