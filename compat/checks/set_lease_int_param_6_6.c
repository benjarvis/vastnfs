#include <linux/module.h>
#include <linux/fs.h>

int callback_setlease(struct file *filp, int arg, struct file_lock **fl, void **ops)
{
	return 0;
}

/* Since v6.6-rc1~218^2~30, the second parameter type is int */
void check_type(struct file_operations *fops)
{
	fops->setlease = callback_setlease;
}

MODULE_LICENSE("GPL");
