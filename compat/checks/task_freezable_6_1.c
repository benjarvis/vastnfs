#include <linux/module.h>
#include <linux/sched.h>

/* See commit v6.1-rc1~98^2~6 */

int check_type(void)
{
	return TASK_FREEZABLE;
}

MODULE_LICENSE("GPL");
