#include "linux/module.h"
#include "linux/fs_context.h"

void *check_type(struct lock_manager_operations *ops)
{
	return (void *)&ops->lm_breaker_owns_lease;
}

MODULE_LICENSE("GPL");
