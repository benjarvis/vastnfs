#include <linux/module.h>
#include <linux/pagevec.h>

void check_func(struct pagevec *pvec, int cold)
{
	pagevec_init(pvec, cold);
}

MODULE_LICENSE("GPL");
