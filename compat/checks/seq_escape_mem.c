#include <linux/seq_file.h>
#include <linux/module.h>

void *test_dummy(void)
{
	void *x = (void *)&seq_escape_mem;
	return x;
}

MODULE_LICENSE("GPL");
