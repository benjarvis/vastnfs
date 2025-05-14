#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mount.h>

long (*TYPE_MATCH_do_mount)(const char *, const char __user *,
			   const char *, unsigned long, void *) = NULL;
int test_dummy(void)
{
	typeof(do_mount) *f;
	f = TYPE_MATCH_do_mount;
	return 0;
}

MODULE_LICENSE("GPL");
