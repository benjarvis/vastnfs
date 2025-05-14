#include <linux/module.h>
#include <linux/mount.h>

void test_dummy(void)
{
	/* v5.17-rc1~67^2~34 (exit: Rename module_put_and_exit to
	 * module_put_and_kthread_exit) */
	module_put_and_kthread_exit(0);
}

MODULE_LICENSE("GPL");
