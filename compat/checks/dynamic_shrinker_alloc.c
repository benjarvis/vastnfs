#include <linux/module.h>
#include <linux/shrinker.h>

const void* test_dummy(void)
{
	return shrinker_alloc;
}

MODULE_LICENSE("GPL");
