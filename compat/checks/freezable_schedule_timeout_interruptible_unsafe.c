#include "linux/module.h"
#include "linux/freezer.h"

int check_type(void)
{
	freezable_schedule_timeout_interruptible_unsafe(3);
	return 0;
}

MODULE_LICENSE("GPL");
