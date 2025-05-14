#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>

void print_of_field(struct task_struct *task)
{
	printk("%d\n", task->flags);
}

static int __init init_test(void)
{
	print_of_field(current);
	return 0;
}

static void __exit exit_test(void)
{
}

module_init(init_test);
module_exit(exit_test);
MODULE_LICENSE("GPL");
