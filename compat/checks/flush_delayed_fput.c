#include <linux/module.h>
#include <linux/file.h>

int check_type(void)
{
	flush_delayed_fput();
	return 0;
}

MODULE_LICENSE("GPL");
