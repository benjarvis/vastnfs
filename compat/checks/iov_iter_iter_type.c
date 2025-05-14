#include <linux/module.h>
#include <linux/uio.h>

unsigned int test_dummy(struct iov_iter *i)
{
	return i->iter_type;
}

MODULE_LICENSE("GPL");
