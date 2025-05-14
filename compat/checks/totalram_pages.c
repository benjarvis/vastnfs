#include <linux/module.h>
#include <linux/swap.h>

int test_dummy(void)
{
	unsigned long p = totalram_pages();
	return (int)p;
}

MODULE_LICENSE("GPL");
