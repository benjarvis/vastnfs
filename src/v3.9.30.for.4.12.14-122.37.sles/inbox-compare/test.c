#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>

static int __init init_test(void)
{
	printk("%zd\n", sizeof(struct task_struct));
	return 0;
}

static void __exit exit_test(void)
{
}

module_init(init_test);
module_exit(exit_test);
