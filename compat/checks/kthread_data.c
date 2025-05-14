#include <linux/module.h>
#include <linux/kthread.h>

void *test_dummy(struct task_struct *kthread)
{
	return kthread_data(kthread);
}

MODULE_LICENSE("GPL");
