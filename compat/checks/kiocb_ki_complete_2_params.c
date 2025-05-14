#include <linux/module.h>
#include <linux/fs.h>

void test_dummy(struct kiocb *kiocb)
{
	/* v5.16-rc1 */
	kiocb->ki_complete(kiocb, 0);
}

MODULE_LICENSE("GPL");
