#include <linux/module.h>
#include <linux/fs.h>

void *check_func(struct address_space_operations *aops)
{
	return &aops->dirty_folio;
}

MODULE_LICENSE("GPL");
