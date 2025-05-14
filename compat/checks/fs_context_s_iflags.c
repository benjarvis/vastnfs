#include <linux/module.h>
#include <linux/fs_context.h>

int check_type(struct fs_context *fc)
{
	return fc->s_iflags;
}

MODULE_LICENSE("GPL");
