#include <linux/module.h>
#include <linux/fs.h>

/* Function modified in v6.6-rc1~220^2~12 */

void check_type(struct mnt_idmap *imap, u32 mask, struct inode *i, struct kstat *k)
{
	return generic_fillattr(imap, mask, i, k);
}

MODULE_LICENSE("GPL");
