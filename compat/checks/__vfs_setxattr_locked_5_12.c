#include <linux/module.h>
#include <linux/xattr.h>

/*
 * c7c7a1a18af4c3bb7749d33e3df3acdf0a95bb
 * xattr: handle idmapped mounts
 */
int test_dummy(struct user_namespace *mnt_userns, struct dentry *dentry,
	       const char *name, const void *value, size_t size,
	       int flags, struct inode **delegated_inode)
{
	return __vfs_setxattr_locked(mnt_userns, dentry, name, value,
				     size, flags, delegated_inode);
}

MODULE_LICENSE("GPL");
