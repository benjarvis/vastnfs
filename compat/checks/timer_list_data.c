#include <linux/module.h>
#include <linux/timer.h>

unsigned long check_type(struct timer_list *tl)
{
	return tl->data;
}

MODULE_LICENSE("GPL");
