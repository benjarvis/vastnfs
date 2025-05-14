#include <linux/module.h>
#include <linux/fs.h>

int test_dummy(void)
{
	return COPY_FILE_SPLICE;
}

MODULE_LICENSE("GPL");
