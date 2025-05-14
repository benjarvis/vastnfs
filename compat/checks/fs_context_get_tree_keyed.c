#include "linux/module.h"
#include "linux/fs_context.h"

int check_type(struct fs_context *fc,
	       int (*fill_super)(struct super_block *sb,
				 struct fs_context *fc),
	       void *key)
{
	return get_tree_keyed(fc, fill_super, key);
}

MODULE_LICENSE("GPL");
