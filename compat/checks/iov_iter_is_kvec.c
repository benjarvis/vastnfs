#include <linux/module.h>
#include <linux/uio.h>

bool test_dummy(const struct iov_iter *i)
{
	return iov_iter_is_kvec(i);
}

MODULE_LICENSE("GPL");
