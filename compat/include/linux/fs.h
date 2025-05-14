#ifndef __COMPAT_LINUX_FS_H__
#define __COMPAT_LINUX_FS_H__

#include_next <linux/fs.h>
#include <linux/splice.h>

#ifndef COMPAT_DETECT_INODE_WRONG_TYPE
static inline bool inode_wrong_type(const struct inode *inode, umode_t mode)
{
	return (inode->i_mode ^ mode) & S_IFMT;
}
#endif

#ifndef COMPAT_DETECT_IS_IDMAPPED_MNT
static inline bool is_idmapped_mnt(const struct vfsmount *mnt)
{
	/* Cannot be implemented prior to v5.12 - a6435940b62f81a */
	/* mount: attach mappings to mounts */
	return false;
}
#endif

#if defined(COMPAT_DETECT_GENERIC_FILLATTR_6_6)
#define COMPAT_FILLATTR(request_mask) request_mask,
#else
#define COMPAT_FILLATTR(request_mask)
#endif

#ifdef COMPAT_DETECT_VFS_USERNS
#define COMPAT_INIT_USER_NS_ARG    &init_user_ns,
#define COMPAT_INIT_USER_NS_PARAM  struct user_namespace *,
#define COMPAT_INIT_USER_NS_PARAM_N(n)  struct user_namespace *n,
#define COMPAT_INIT_USER_NS_WRAP(n) n,
#else
#ifdef COMPAT_DETECT_GENERIC_PERMISSION_MNT_IDMAP_6_3
#define COMPAT_INIT_USER_NS_ARG &nop_mnt_idmap,
#define COMPAT_INIT_USER_NS_PARAM struct mnt_idmap *,
#define COMPAT_INIT_USER_NS_PARAM_N(n) struct mnt_idmap *idmap,
#define COMPAT_INIT_USER_NS_WRAP(n) idmap,
#else
#define COMPAT_INIT_USER_NS_ARG
#define COMPAT_INIT_USER_NS_PARAM
#define COMPAT_INIT_USER_NS_PARAM_N(n)
#define COMPAT_INIT_USER_NS_WRAP(n)
#endif
#endif

#ifdef COMPAT_DETECT_SET_ACL_GETS_DENTRY_6_2
#define SET_ACL_PARAM_TYPE dentry
#define SET_ACL_PARAM_TO_INODE(param) d_inode(param)
#define SET_ACL_PARAM_ABITRARY(inode, dentry) dentry
#else
#define SET_ACL_PARAM_TYPE inode
#define SET_ACL_PARAM_TO_INODE(param) param
#define SET_ACL_PARAM_ABITRARY(inode, dentry) inode
#endif

#if !defined(COMPAT_DETECT_LOCKS_DELETE_BLOCK) && \
    !defined(COMPAT_DETECT_LOCKS_DELETE_BLOCK_6_3)
#define locks_delete_block posix_unblock_lock
#endif

#if defined(COMPAT_DETECT_GET_INODE_ACL_6_2)
#define get_acl get_inode_acl
#endif

#ifndef COMPAT_DETECT_LEASE_REGISTER_NOTIFIER
struct notifier_block;
static inline int lease_register_notifier(struct notifier_block *nb)
{
	return 0;
}
static inline void lease_unregister_notifier(struct notifier_block *nb)
{
}
#endif

#ifdef COMPAT_DETECT_FS_ATOMIC_OPENED
#define FS_ATOMIC_OPEN_OPENED_PARAM , int *opened
#define FS_ATOMIC_OPEN_OPENED_ARG , opened
#else
#define FS_ATOMIC_OPEN_OPENED_PARAM
#define FS_ATOMIC_OPEN_OPENED_ARG
#endif

#ifndef COMPAT_DETECT_FS_CONTEXT_SGET_FC
#include <linux/fs_context.h>

struct fs_context;
extern struct super_block *compat_sget_fc(struct fs_context *fc,
					  int (*test)(struct super_block *, struct fs_context *),
					  int (*set)(struct super_block *, struct fs_context *));
#define sget_fc compat_sget_fc
#endif

#if !defined(COMPAT_DETECT_GENERIC_COPY_FILE_RANGE)
/* generic_copy_file_range - copy data between two files
 * @file_in:   file structure to read from
 * @pos_in:    file offset to read from
 * @file_out:  file structure to write data to
 * @pos_out:   file offset to write data to
 * @len:       amount of data to copy
 * @flags:     copy flags
 *
 * This is a generic filesystem helper to copy data from one file to another.
 * It has no constraints on the source or destination file owners - the files
 * can belong to different superblocks and different filesystem types. Short
 * copies are allowed.
 *
 * This should be called from the @file_out filesystem, as per the
 * ->copy_file_range() method.
 *
 * Returns the number of bytes copied or a negative error indicating the
 * failure.
 */

static inline
ssize_t generic_copy_file_range(struct file *file_in, loff_t pos_in,
                               struct file *file_out, loff_t pos_out,
                               size_t len, unsigned int flags)
{
       return do_splice_direct(file_in, &pos_in, file_out, &pos_out,
                               len > MAX_RW_COUNT ? MAX_RW_COUNT : len, 0);
}
#endif

static inline int compat_vfs_clone_file_range(struct file *file_in, loff_t pos_in,
					      struct file *file_out, loff_t pos_out,
					      u64 len,
					      unsigned int remap_flags)
{
#if defined(COMPAT_DETECT_DO_CLONE_FILE_RANGE_PROTECTING)
	BUG_ON(remap_flags != 0);
	return do_clone_file_range(file_in, pos_in, file_out, pos_out, len);
#elif defined(COMPAT_DETECT_VFS_CLONE_FILE_RANGE_REMAP_FLAGS)
	return vfs_clone_file_range(file_in, pos_in, file_out, pos_out, len,
				    remap_flags);
#else
	BUG_ON(remap_flags != 0);
	return vfs_clone_file_range(file_in, pos_in, file_out, pos_out, len);
#endif
}

#if defined(COMPAT_DETECT_INCLUDE_LINUX_FILELOCK)
#include <linux/filelock.h>
#endif

#if !defined(COMPAT_DETECT_LOCKS_INODE_6_3)
#define locks_inode(f) file_inode(f)
#endif

#if !defined(COMPAT_DETECT_INODE_GET_CTIME_6_6)
/**
 * inode_set_ctime_to_ts - set the ctime in the inode
 * @inode: inode in which to set the ctime
 * @ts: value to set in the ctime field
 *
 * Set the ctime in @inode to @ts
 */
static inline struct timespec64 inode_set_ctime_to_ts(struct inode *inode,
                                                      struct timespec64 ts)
{
        inode->i_ctime = ts;
        return ts;
}

/**
 * inode_set_ctime - set the ctime in the inode
 * @inode: inode in which to set the ctime
 * @sec: tv_sec value to set
 * @nsec: tv_nsec value to set
 *
 * Set the ctime in @inode to { @sec, @nsec }
 */
static inline struct timespec64 inode_set_ctime(struct inode *inode,
                                                time64_t sec, long nsec)
{
        struct timespec64 ts = { .tv_sec  = sec,
                                 .tv_nsec = nsec };

        return inode_set_ctime_to_ts(inode, ts);
}

static inline long inode_get_ctime_nsec(const struct inode *inode)
{
	return inode->i_ctime.tv_nsec;
}

static inline struct timespec64 inode_get_ctime(const struct inode *inode)
{
	return inode->i_ctime;
}

static inline struct timespec64 inode_set_ctime_current(struct inode *inode)
{
	struct timespec64 now = current_time(inode);

	inode_set_ctime(inode, now.tv_sec, now.tv_nsec);
	return now;
}

#endif

#if !defined(COMPAT_DETECT_INODE_GET_ATIME_6_7)
static inline time64_t inode_get_ctime_sec(const struct inode *inode)
{
#if !defined(COMPAT_DETECT_INODE_GET_CTIME_6_6)
	return inode->i_ctime.tv_sec;
#else
	return inode_get_ctime(inode).tv_sec;
#endif
}

static inline struct timespec64 inode_set_mtime_to_ts(struct inode *inode,
						      struct timespec64 ts)
{
	inode->i_mtime = ts;
	return ts;
}

static inline struct timespec64 inode_set_mtime(struct inode *inode,
						time64_t sec, long nsec)
{
	struct timespec64 ts = { .tv_sec  = sec, .tv_nsec = nsec };
	return inode_set_mtime_to_ts(inode, ts);
}

static inline time64_t inode_get_mtime_sec(const struct inode *inode)
{
	return inode->i_mtime.tv_sec;
}

static inline long inode_get_mtime_nsec(const struct inode *inode)
{
	return inode->i_mtime.tv_nsec;
}

static inline struct timespec64 inode_get_mtime(const struct inode *inode)
{
	return inode->i_mtime;
}

static inline time64_t inode_get_atime_sec(const struct inode *inode)
{
	return inode->i_atime.tv_sec;
}

static inline long inode_get_atime_nsec(const struct inode *inode)
{
	return inode->i_atime.tv_nsec;
}

static inline struct timespec64 inode_get_atime(const struct inode *inode)
{
	return inode->i_atime;
}

static inline struct timespec64 inode_set_atime_to_ts(struct inode *inode,
						      struct timespec64 ts)
{
	inode->i_atime = ts;
	return ts;
}

static inline struct timespec64 inode_set_atime(struct inode *inode,
						time64_t sec, long nsec)
{
	struct timespec64 ts = { .tv_sec  = sec, .tv_nsec = nsec };
	return inode_set_atime_to_ts(inode, ts);
}

static inline struct timespec64 simple_inode_init_ts(struct inode *inode)
{
	struct timespec64 ts = inode_set_ctime_current(inode);

	inode_set_atime_to_ts(inode, ts);
	inode_set_mtime_to_ts(inode, ts);
	return ts;
}

#endif

#if defined(COMPAT_DETECT_SET_LEASE_INT_PARAM_6_6)
#define FS_SETLEASE_ARG_TYPE int
#else
#define FS_SETLEASE_ARG_TYPE long
#endif

#if !defined(COMPAT_DETECT_KIOCB_CLONE_5_6)
static inline void kiocb_clone(struct kiocb *kiocb, struct kiocb *kiocb_src,
			       struct file *filp)
{
	*kiocb = (struct kiocb) {
		.ki_filp = filp,
		.ki_flags = kiocb_src->ki_flags,
		.ki_hint = kiocb_src->ki_hint,
#if defined(COMPAT_DETECT_KIOCB_KI_IOPRIO_4_18)
		.ki_ioprio = kiocb_src->ki_ioprio,
#endif
		.ki_pos = kiocb_src->ki_pos,
	};
}
#endif

static inline spinlock_t* compat_address_space_private_lock(struct address_space* mapping)
{
#ifdef COMPAT_DETECT_ADDRESS_SPACE_I_PRIVATE
	return &mapping->i_private_lock;
#else
	return &mapping->private_lock;
#endif
}

#endif
