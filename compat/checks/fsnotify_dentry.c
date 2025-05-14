#include "linux/module.h"
#include "linux/fs.h"
#include "linux/fsnotify.h"

void test_wrapper(struct dentry *dentry)
{
	fsnotify_dentry(dentry, 0);
}

MODULE_LICENSE("GPL");
