#include <linux/module.h>
#include <rdma/rdma_cm.h>

void check_compat_fill_super(struct super_block *sb)
{
	sb->s_time_min = 0;
	sb->s_time_max = 0;
}

MODULE_LICENSE("GPL");
