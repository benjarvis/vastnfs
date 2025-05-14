#include <linux/module.h>
#include <linux/kref.h>

int check_func_existence(struct kref *kref)
{
	return kref_read(kref);
}

MODULE_LICENSE("GPL");
