#include <linux/module.h>
#include <linux/pagemap.h>

/* Removed in v6.6-rc1~156^2~91 */
void check_type(struct page *page)
{
	wait_on_page_locked_killable(page);
}

MODULE_LICENSE("GPL");
