#include <linux/module.h>
#include <uapi/linux/types.h>

int check_type(__poll_t *p)
{
	(void)p;
	return 0;
}

MODULE_LICENSE("GPL");
