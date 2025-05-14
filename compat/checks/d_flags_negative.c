#include "linux/module.h"
#include "linux/dcache.h"

void test_wrapper(void)
{
	d_flags_negative(1);
}

MODULE_LICENSE("GPL");
