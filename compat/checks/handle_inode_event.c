#include <linux/module.h>
#include <linux/fs_parser.h>
#include <linux/fsnotify.h>

static struct fsnotify_ops ops = {
	.handle_inode_event = NULL,
};

const void* test_wrapper(void)
{
	return &ops;
}

MODULE_LICENSE("GPL");
