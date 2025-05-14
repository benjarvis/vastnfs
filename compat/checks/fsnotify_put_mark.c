#include <linux/module.h>
#include <linux/fsnotify.h>

void test_wrapper(struct fsnotify_mark *mark)
{
	fsnotify_put_mark(mark);
}

MODULE_LICENSE("GPL");
