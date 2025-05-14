#include <linux/module.h>
#include <linux/writeback.h>

/*
 * v6.3-rc1~113^2~171 "fs: convert writepage_t callback to pass a folio"
 */
int test_dummy(writepage_t writepage,
	       struct folio *folio, struct writeback_control *wbc,
	       void *data)
{
	return writepage(folio, wbc, data);
}

MODULE_LICENSE("GPL");
