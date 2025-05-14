#include <linux/module.h>
#include <linux/namei.h>

int test_dummy(struct path *path)
{
	return follow_down(path, 0);
}

MODULE_LICENSE("GPL");
