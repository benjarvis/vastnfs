#include <linux/module.h>
#include <linux/fs.h>

/* v6.1-rc1~146^2~1 - Change calling conventions for filldir_t */

bool bla_filldir(struct dir_context *d, const char *p, int i, loff_t o, u64 x, unsigned int a)
{
	return false;
}

filldir_t check_type(void)
{
	return bla_filldir;
}

MODULE_LICENSE("GPL");
