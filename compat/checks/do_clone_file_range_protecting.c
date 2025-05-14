#include <linux/module.h>
#include <linux/fs.h>

/* CHECK UNDEFINED: vfs_clone_file_range */

int test_dummy(struct file *file_in, loff_t pos_in,
	       struct file *file_out, loff_t pos_out,
	       u64 len)
{
	return do_clone_file_range(file_in, pos_in, file_out, pos_out, len);
}

MODULE_LICENSE("GPL");
