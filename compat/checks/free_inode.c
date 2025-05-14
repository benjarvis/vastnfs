#include "linux/module.h"
#include "linux/fs.h"

int check_type(struct super_operations *ops)
{
	ops->free_inode(NULL);
	return 0;
}

MODULE_LICENSE("GPL");
