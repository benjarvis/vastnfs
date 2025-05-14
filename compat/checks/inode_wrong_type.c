#include <linux/module.h>
#include <linux/fs.h>

void test_dummy(const struct inode *inode, umode_t mode)
{
	inode_wrong_type(inode, mode);
}

MODULE_LICENSE("GPL");
