#include <linux/module.h>
#include <linux/exportfs.h>
#include <linux/fs.h>

int check_type(struct inode *inode)
{
	void *x = (void *)inode->i_sb->s_export_op->fetch_iversion;
	return 0;
}

MODULE_LICENSE("GPL");
