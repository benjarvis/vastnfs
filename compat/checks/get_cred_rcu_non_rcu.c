#include <linux/module.h>
#include <linux/cred.h>

int test_dummy(struct cred *a)
{
	return a->non_rcu;
}

MODULE_LICENSE("GPL");
