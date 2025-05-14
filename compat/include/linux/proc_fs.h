#ifndef __PROCFS_COMPAT_H__
#define __PROCFS_COMPAT_H__

#include_next <linux/proc_fs.h>

#ifndef COMPAT_DETECT_PROC_OPS

#define proc_ops file_operations
#define proc_open open
#define proc_release release
#define proc_read read
#define proc_write write
#define proc_poll poll
#define proc_lseek llseek
#define proc_ioctl unlocked_ioctl
#define proc_compat_ioctl compat_ioctl

#endif

#if !defined(COMPAT_DETECT_PROC_CREATE_NET)

#define PROC_CREATE_NET_COMPAT(new_def, var, ops, func, size) \
\
static const struct file_operations var; \
static int func(struct inode *inode, struct file *file) \
{ \
       return seq_open_net(inode, file, &ops, size); \
} \
\
static const struct file_operations var = { \
       .open           = func, \
       .read           = seq_read, \
       .llseek         = seq_lseek, \
       .release        = seq_release_net, \
}; \
\
static inline struct proc_dir_entry *new_def( \
	const char *name, umode_t mode, \
	struct proc_dir_entry *parent) \
{ \
	return proc_create(name, mode, parent, &var); \
} \

#else

#define PROC_CREATE_NET_COMPAT(new_def, var, ops, func, size) \
\
static inline struct proc_dir_entry *new_def( \
	const char *name, umode_t mode, \
	struct proc_dir_entry *parent) \
{ \
	return proc_create_net(name, mode, parent, &ops, size); \
} \

#endif

#if defined(COMPAT_DETECT_PDE_DATA_LOWER_CASE)
#define PDE_DATA(inode) pde_data(inode)
#endif

#endif
