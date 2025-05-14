#include "linux/module.h"
#include "linux/slab.h"

void check_type(void)
{
	kfree_sensitive(NULL);
}

MODULE_LICENSE("GPL");
