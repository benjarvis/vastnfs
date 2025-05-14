#include <linux/module.h>
#include <linux/fs.h>

ssize_t test_dummy(struct kiocb *kiocb, struct iov_iter *iov)
{
	return generic_perform_write(kiocb, iov);
}

MODULE_LICENSE("GPL");
