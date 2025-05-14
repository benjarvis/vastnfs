#include <linux/module.h>
#include <linux/sched.h>
#include <linux/wait_bit.h>

void test_dummy(atomic_t *var)
{
	wait_var_event(var, !atomic_read(var));
}

MODULE_LICENSE("GPL");
