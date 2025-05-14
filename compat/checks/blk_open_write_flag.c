#include <linux/module.h>
#include <linux/blkdev.h>

int test_dummy(void)
{
	return BLK_OPEN_WRITE;
}

MODULE_LICENSE("GPL");
