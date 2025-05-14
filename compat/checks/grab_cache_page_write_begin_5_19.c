#include <linux/module.h>
#include <linux/pagemap.h>

int test_dummy(struct address_space *mapping, pgoff_t index)
{
	return grab_cache_page_write_begin(mapping, index);
}

MODULE_LICENSE("GPL");
