#include <linux/module.h>
#include <linux/kobject.h>

/* v6.2-rc1~66^2~20 -  kobject: make kobject_get_ownership() take a constant kobject */

void test_get_ownership(const struct kobject *kobj, kuid_t *uid, kgid_t *gid);

const struct kobj_type xxx = {
	.get_ownership = test_get_ownership,
};

int check_type(void)
{
	return 0;
}

MODULE_LICENSE("GPL");
