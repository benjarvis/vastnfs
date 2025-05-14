#include <linux/module.h>
#include <linux/uio.h>

void test_dummy(struct iov_iter *i, unsigned int direction, size_t count)
{
	iov_iter_discard(i, direction, count);
}

MODULE_LICENSE("GPL");
