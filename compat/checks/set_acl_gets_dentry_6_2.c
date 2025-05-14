#include <linux/module.h>
#include <linux/fs.h>

/* v6.2-rc1~158^2~34 - fs: pass dentry to set acl method */
struct posix_acl *check_type(struct inode_operations *ops, struct dentry *dentry)
{
	return ops->set_acl(NULL, dentry, NULL, 0);
}

MODULE_LICENSE("GPL");
