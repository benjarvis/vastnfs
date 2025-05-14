#include "linux/module.h"
#include "linux/vmalloc.h"

void *test_wrapper(unsigned long size, gfp_t gfp_mask)
{
	return __vmalloc(size, gfp_mask);
}

MODULE_LICENSE("GPL");
