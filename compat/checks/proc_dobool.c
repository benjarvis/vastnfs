#include <linux/module.h>
#include <linux/sysctl.h>

void *check_existence(void)
{
	return (void *)&proc_dobool;
}

MODULE_LICENSE("GPL");
