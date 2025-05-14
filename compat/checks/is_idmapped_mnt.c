#include <linux/module.h>
#include <linux/fs.h>

bool test_is_idmapped_mnt(const struct vfsmount *mnt)
{
	return is_idmapped_mnt(mnt);
}

MODULE_LICENSE("GPL");
