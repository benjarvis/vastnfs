#include <linux/module.h>
#include <linux/fs.h>

int test_dummy(struct file *file_in, loff_t pos_in,
	       struct file *file_out, loff_t pos_out,
	       u64 len, unsigned int remap_flags)
{
	return vfs_clone_file_range(file_in, pos_in, file_out, pos_out, len, remap_flags);
}

MODULE_LICENSE("GPL");
