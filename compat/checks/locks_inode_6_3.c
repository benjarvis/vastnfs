#include <linux/module.h>
#include <linux/fs.h>

/*
 * `locks_inode` replaced by `file_inode` in v6.3-rc1~221^2
 */
const struct inode *test_dummy(struct file *file)
{
	return locks_inode(file);
}

MODULE_LICENSE("GPL");
