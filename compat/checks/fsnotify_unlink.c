#include "linux/module.h"
#include "linux/fsnotify.h"

void test_wrapper(struct inode *dir, struct dentry *dentry)
{
	fsnotify_unlink(dir, dentry);
}

MODULE_LICENSE("GPL");
