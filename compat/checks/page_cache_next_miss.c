#include <linux/module.h>
#include <linux/pagemap.h>

pgoff_t test_dummy(struct address_space *mapping,
		   pgoff_t index, unsigned long max_scan)
{
	return page_cache_next_miss(mapping, index, max_scan);

}

MODULE_LICENSE("GPL");
