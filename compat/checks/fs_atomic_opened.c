#include <linux/module.h>
#include <linux/fs.h>

int check_type(struct inode_operations *ops)
{
	int opened;
	return ops->atomic_open(NULL, NULL, NULL, 0, 0, &opened);
}

MODULE_LICENSE("GPL");
