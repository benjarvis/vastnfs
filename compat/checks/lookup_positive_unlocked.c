#include "linux/module.h"
#include "linux/namei.h"

void* check_type(const char *name, struct dentry *base, int len)
{
	return lookup_positive_unlocked(name, base, len);
}

MODULE_LICENSE("GPL");
