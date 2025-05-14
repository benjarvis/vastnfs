#include <linux/module.h>
#include <linux/cred.h>

/*
 * Upstream commit f8fa5d76925991976b3e7076f9d1052515ec1fca  from 6.18.
 *
 * It also winds up in older kernels such as 4.19.x.
 */

void test_dummy(struct cred *a)
{
	atomic_long_inc(&a->usage);
}

MODULE_LICENSE("GPL");
