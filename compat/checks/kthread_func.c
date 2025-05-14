#include "linux/module.h"
#include "linux/kthread.h"

int check_type(void)
{
	kthread_func(current);
	return 0;
}

MODULE_LICENSE("GPL");
