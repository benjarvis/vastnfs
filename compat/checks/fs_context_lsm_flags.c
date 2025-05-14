#include <linux/module.h>
#include <linux/fs_context.h>

/*
 * Flag was removed in v6.4-rc1~192^2~8 "fs_context: drop the unused lsm_flags member"
 */
unsigned int check_type(struct fs_context *fc)
{
	return fc->lsm_flags;
}

MODULE_LICENSE("GPL");
