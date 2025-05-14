#include <linux/module.h>
#include <linux/proc_fs.h>

void *compat_check_type(struct inode *inode)
{
	return pde_data(inode);
}

MODULE_LICENSE("GPL");
