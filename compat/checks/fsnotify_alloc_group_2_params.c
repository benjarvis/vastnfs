#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/fsnotify_backend.h>

struct fsnotify_group *test_dummy(const struct fsnotify_ops *ops)
{
	/* v5.19-rc1~148^2~15: fsnotify: pass flags argument to fsnotify_alloc_group() */
	return fsnotify_alloc_group(ops, 0);
}

MODULE_LICENSE("GPL");
