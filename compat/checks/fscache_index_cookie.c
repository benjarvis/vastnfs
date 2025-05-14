#include <linux/module.h>
#include <linux/fscache.h>

void test_dummy(struct fscache_cookie *ck, const void *p, bool b)
{
	__fscache_relinquish_cookie(ck, p, b);
}

MODULE_LICENSE("GPL");
