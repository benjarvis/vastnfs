#include <linux/module.h>
#include <linux/mm.h>

/*
 * v6.8-rc1~180^2~339 "mm: remove test_set_page_writeback()"
 */
bool test_dummy(struct page *page)
{
	return test_set_page_writeback(page);
}

MODULE_LICENSE("GPL");
