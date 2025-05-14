#include <linux/module.h>
#include <linux/fs.h>

u16 test_dummy(struct kiocb *kiocb)
{
	return kiocb->ki_ioprio;
}

MODULE_LICENSE("GPL");
