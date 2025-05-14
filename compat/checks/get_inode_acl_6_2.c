#include <linux/module.h>
#include <linux/fs.h>

/* v6.2-rc1~158^2~33 - fs: rename current get acl method */
struct posix_acl *check_type(struct inode_operations *ops)
{
	return ops->get_inode_acl(NULL, 0, false);
}

MODULE_LICENSE("GPL");
