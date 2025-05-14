#include "linux/module.h"
#include "linux/xattr.h"

/*
 * v5.8-rc5-3-gcab8d289c5ad5: "xattr: add a function to check if a namespace is supported"
 */
int test_wrapper(struct inode *inode, const char *prefix)
{
	return xattr_supported_namespace(inode, prefix);
}

MODULE_LICENSE("GPL");
