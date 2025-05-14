#include <linux/module.h>
#include <linux/filelock.h>

void check_type(struct file_lock *waiter)
{
	locks_delete_block(waiter);
}

MODULE_LICENSE("GPL");
