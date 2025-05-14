#include <linux/module.h>
#include <linux/idr.h>

void test_wrapper(struct idr *idr, unsigned long *nextid) {
	idr_get_next_ul(idr, nextid);
}

MODULE_LICENSE("GPL");
