#include <linux/module.h>
#include <linux/blkdev.h>

blk_status_t test_dummy(struct gendisk *bd_disk, struct request *rq, int at_head)
{
	return blk_execute_rq(bd_disk, rq, at_head);
}

MODULE_LICENSE("GPL");
