#include <linux/module.h>
#include <linux/fs.h>
#include <linux/posix_acl_xattr.h>

/*
 * v6.4-rc1~193^2~3 - "fs: rename generic posix acl handlers"
 * Removed this symbol.
 */
struct xattr_handler *check_type(void)
{
	return &posix_acl_access_xattr_handler;
}

MODULE_LICENSE("GPL");
