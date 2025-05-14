#include <linux/module.h>
#include <linux/mount.h>

struct user_namespace *test_dummy(const struct vfsmount *mnt)
{
	return mnt_user_ns(mnt);
}

MODULE_LICENSE("GPL");
