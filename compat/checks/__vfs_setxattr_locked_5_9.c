#include <linux/module.h>
#include <linux/xattr.h>

/*
 * 08b5d5014a27e717826999ad20e394a8811aae92
 * xattr: break delegations in {set,remove}xattr
 */
int test_dummy(struct dentry *dentry,
	       const char *name, const void *value, size_t size,
	       int flags, struct inode **delegated_inode)
{
	return __vfs_setxattr_locked(dentry, name, value,
				     size, flags, delegated_inode);
}

MODULE_LICENSE("GPL");
