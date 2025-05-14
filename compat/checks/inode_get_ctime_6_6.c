#include <linux/module.h>
#include <linux/fs.h>

void test_dummy(struct inode *inode, struct timespec64 ts)
{
	inode_set_ctime_to_ts(inode, ts);
}

MODULE_LICENSE("GPL");
