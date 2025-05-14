#include <linux/module.h>
#include <linux/fs.h>

void test_dummy(struct kiocb *kiocb, struct kiocb *kiocb_src,
		struct file *filp)
{
	return kiocb_clone(kiocb, kiocb_src, filp);
}

MODULE_LICENSE("GPL");
