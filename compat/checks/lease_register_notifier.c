#include "linux/module.h"
#include "linux/fs.h"

void check_type(void)
{
	lease_register_notifier(NULL);
}

MODULE_LICENSE("GPL");
