#include <linux/module.h>
#include <linux/sched.h>

struct task_struct *check_type(void)
{
	return find_task_by_pid_ns(1, NULL);
}

MODULE_LICENSE("GPL");
