#include <linux/module.h>
#include <linux/fsnotify_backend.h>

void test_wrapper(struct dentry *dentry, int isdir)
{
	fsnotify_nameremove(dentry, isdir);
}

MODULE_LICENSE("GPL");
