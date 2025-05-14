#include <linux/module.h>
#include <linux/namei.h>

int test_dummy(int dfd, const char __user *name, unsigned flags,
		 struct path *path)
{
	return user_path_at(dfd, name, flags, path);
}

MODULE_LICENSE("GPL");
