#include <linux/module.h>
#include <linux/fs.h>

/*
 * v6.8-rc1~180^2~315 "fs: convert error_remove_page to error_remove_folio"
 */
const void *check_func(struct address_space_operations *aops)
{
	return aops->error_remove_folio;
}

MODULE_LICENSE("GPL");
