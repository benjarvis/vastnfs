#include <linux/module.h>
#include <linux/fs.h>

struct inode *test_dummy(struct super_block *sb, struct kmem_cache *cache, gfp_t gfp)
{
	return alloc_inode_sb(sb, cache, gfp);
}

MODULE_LICENSE("GPL");
