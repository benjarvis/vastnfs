#include <linux/module.h>
#include <linux/exportfs.h>

int check_type(void)
{
	void *x = (void *)&exportfs_decode_fh_raw;
	return 0;
}

MODULE_LICENSE("GPL");
