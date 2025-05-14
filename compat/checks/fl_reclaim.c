#include <linux/module.h>
#include <linux/fs.h>

int check_type(void)
{
	return FL_RECLAIM;
}

MODULE_LICENSE("GPL");
