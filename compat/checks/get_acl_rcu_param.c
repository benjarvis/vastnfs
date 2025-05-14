#include <linux/module.h>
#include <linux/fs.h>

void* check_type(struct inode_operations *ops)
{
	return ops->get_acl(NULL, 0, false);
}

MODULE_LICENSE("GPL");
