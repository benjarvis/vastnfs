#include <linux/module.h>
#include <linux/file.h>

struct file *check_type(struct inode *inode, struct vfsmount *vfsmount,
	const char *p, int flags, const struct file_operations *ops)
{
	return alloc_file_pseudo(inode, vfsmount, p, flags, ops);
}

MODULE_LICENSE("GPL");
