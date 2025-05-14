#include <linux/module.h>
#include <linux/fs.h>

/*
 * v6.5-rc1~236^2~2 removes the function.
 */
ssize_t test_dummy(struct file *file, loff_t *off,
		   struct pipe_inode_info *info,
		   size_t s, unsigned int f)
{
	return generic_file_splice_read(file, off, info, s, f);
}

MODULE_LICENSE("GPL");
