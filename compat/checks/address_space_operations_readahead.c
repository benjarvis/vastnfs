#include <linux/module.h>
#include <linux/fs.h>

void check_type(const struct address_space_operations *aops, struct readahead_control *ractl)
{
	/* v5.18-rc1~64^2~89 */
	aops->readahead(ractl);
}

MODULE_LICENSE("GPL");
