#include <linux/module.h>
#include <linux/prandom.h>

unsigned int check_type(void)
{
	return prandom_u32();
}

MODULE_LICENSE("GPL");
