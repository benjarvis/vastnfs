#include <linux/module.h>
#include <linux/fs.h>

ssize_t test_dummy(void)
{
	return generic_copy_file_range(NULL, 0, NULL, 0, 0, 0);
}

MODULE_LICENSE("GPL");
