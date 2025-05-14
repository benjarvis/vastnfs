#include <linux/module.h>
#include <linux/netfs.h>

void check_type(struct page *page)
{
	/* v5.13-rc1 */
	wait_on_page_fscache(page);
}

MODULE_LICENSE("GPL");
