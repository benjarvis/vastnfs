#include "linux/module.h"
#include "linux/blkdev.h"

void check_type(struct block_device *bdev)
{
	(void)bdev_is_partition(bdev);
}

MODULE_LICENSE("GPL");
