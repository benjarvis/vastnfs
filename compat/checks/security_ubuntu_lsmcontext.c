#include <linux/security.h>
#include <linux/fs.h>
#include <linux/module.h>

void *test_dummy(struct lsmcontext *cp)
{
	/*
	 * There's no upstream kernel in which this function receives only a
	 * single parameter, lsmcontext. Found only on Ubuntu so far.
	 */
	security_release_secctx(cp);
	return NULL;
}

MODULE_LICENSE("GPL");
