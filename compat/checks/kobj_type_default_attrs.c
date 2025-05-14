#include <linux/module.h>
#include <linux/kobject.h>

void *check_type(struct kobj_type *kobj_type)
{
	return &kobj_type->default_attrs;
}

MODULE_LICENSE("GPL");
