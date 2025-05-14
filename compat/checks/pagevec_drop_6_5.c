#include <linux/module.h>
#include <linux/pagevec.h>

struct pagevec test_dummy(struct pagevec pagevec)
{
	return pagevec;
}

MODULE_LICENSE("GPL");
