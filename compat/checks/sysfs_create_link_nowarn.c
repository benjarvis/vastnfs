#include <linux/module.h>
#include <linux/sysfs.h>

void check(struct kobject *kobj,
	   struct kobject *target,
	   const char *name)
{
	sysfs_create_link_nowarn(kobj, target, name);
}

MODULE_LICENSE("GPL");
