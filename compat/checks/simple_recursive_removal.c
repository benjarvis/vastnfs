#include <linux/module.h>
#include <linux/fs.h>

/*
 * v5.6-rc1~59^2 "simple_recursive_removal(): kernel-side rm -rf for ramfs-style filesystems"
 */
const void* test_dummy(void)
{
	return simple_recursive_removal;
}

MODULE_LICENSE("GPL");
