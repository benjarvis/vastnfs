#include <linux/module.h>
#include <linux/fs.h>

void *check_type(struct file_operations *ops)
{
	return (void *)&ops->remap_file_range;
}

MODULE_LICENSE("GPL");
