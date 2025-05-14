#include <linux/module.h>
#include <linux/blkdev.h>

int test_dummy(struct block_device_operations *blk,
		struct gendisk *disk,
		u8 id[16],
		enum blk_unique_id id_type)
{
	return blk->get_unique_id(disk, id, id_type);
}

MODULE_LICENSE("GPL");
