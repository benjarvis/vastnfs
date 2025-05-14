#include "linux/module.h"
#include "linux/xattr.h"

/*
 * v6.4-rc1~193^2~7 "xattr: remove unused argument"
 */
int test_wrapper(struct inode *inode)
{
	return xattr_supports_user_prefix(inode);
}

MODULE_LICENSE("GPL");
