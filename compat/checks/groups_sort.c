#include <linux/module.h>
#include <linux/cred.h>

void check_type(struct group_info *gi)
{
	groups_sort(gi);
}

MODULE_LICENSE("GPL");
