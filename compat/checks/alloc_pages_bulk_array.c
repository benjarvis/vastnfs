#include <linux/module.h>
#include <linux/gfp.h>

void test_dummy(void)
{
	struct page **page = NULL;
	alloc_pages_bulk_array(GFP_KERNEL, 0, page);
}

MODULE_LICENSE("GPL");
