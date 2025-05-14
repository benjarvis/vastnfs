#include <linux/module.h>
#include <linux/fs.h>

void *check_func(struct address_space_operations *aops)
{
	return &aops->migrate_folio;
}

MODULE_LICENSE("GPL");
