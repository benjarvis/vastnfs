#include <linux/module.h>
#include <linux/fs.h>

void* check_type(struct fs_context *fc,
	       int (*test)(struct super_block *, struct fs_context *),
	       int (*set)(struct super_block *, struct fs_context *))
{
	return sget_fc(fc, test, set);
}

MODULE_LICENSE("GPL");
