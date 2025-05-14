#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/string_helpers.h>

int test_dummy(void)
{
	void *x = (void *)&seq_escape_mem;
	return ESCAPE_NAP;
}

MODULE_LICENSE("GPL");
