#include <linux/module.h>
#include <linux/cred.h>

int test_dummy(struct cred *a, struct cred *b)
{
	return cred_fscmp(a, b);
}

MODULE_LICENSE("GPL");
