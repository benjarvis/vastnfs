#include <linux/module.h>
#include <linux/blk-mq.h>

void test_dummy(struct gendisk *gendisk, struct request *rq, int at_head)
{
	blk_execute_rq(gendisk, rq, at_head);
}

MODULE_LICENSE("GPL");
