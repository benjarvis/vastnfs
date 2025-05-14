#include <linux/module.h>
#include <linux/bio.h>

int test_dummy(void)
{
	return bio_max_segs(1);
}

MODULE_LICENSE("GPL");
