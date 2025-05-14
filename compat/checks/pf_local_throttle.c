#include "linux/module.h"
#include "linux/sched.h"

int check_type(void)
{
	return PF_LOCAL_THROTTLE;
}

MODULE_LICENSE("GPL");
