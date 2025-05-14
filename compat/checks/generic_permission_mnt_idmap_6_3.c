#include <linux/module.h>
#include <linux/fs.h>

/*
 * v6.3-rc1~219^2~12: "fs: port ->permission() to pass mnt_idmap"
 */
int test_dummy(struct mnt_idmap *mnt_idmap, struct inode *inode, int code)
{
	return generic_permission(mnt_idmap, inode, code);
}

MODULE_LICENSE("GPL");
