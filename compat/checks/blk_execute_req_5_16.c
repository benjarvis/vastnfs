#include <linux/module.h>
#include <linux/blk-mq.h>

void test_dummy(struct request *rq, int at_head)
{
	blk_execute_rq(rq, at_head);
}

MODULE_LICENSE("GPL");
