#include <linux/module.h>
#include <linux/socket.h>

/* v6.5-rc1~163^2~235^2~15 */

int test_dummy(void)
{
	return MSG_SPLICE_PAGES;
}

MODULE_LICENSE("GPL");
