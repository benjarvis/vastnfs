#include <linux/module.h>
#include <linux/blkdev.h>

void test_dummy(struct gendisk *bd_disk, struct request *rq, int at_head)
{
	blk_execute_rq(bd_disk, rq, at_head);
}

MODULE_LICENSE("GPL");
