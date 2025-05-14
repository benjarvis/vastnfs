#include <linux/module.h>
#include <linux/cred.h>

const struct cred *test_dummy(struct cred *a)
{
	return get_cred_rcu(a);
}

MODULE_LICENSE("GPL");
