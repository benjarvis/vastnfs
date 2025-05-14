#include <linux/module.h>
#include <linux/sched.h>

/* See commit v6.5-rc1~176^2~217 */

struct backing_dev_info *check_type(void)
{
	return current->backing_dev_info;
}

MODULE_LICENSE("GPL");
