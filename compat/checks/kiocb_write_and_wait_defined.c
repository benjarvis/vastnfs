#include <linux/module.h>
#include <linux/pagemap.h>

/*
 * Detects v6.5-rc1~176^2~214. "filemap: add a kiocb_write_and_wait helper"
 * We really care about v6.5-rc1~176^2~215:
 *
 *   "filemap: update ki_pos in generic_perform_write"
 *   https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?h=v6.5-rc1~176%5e2~215
 *
 * Because it changed behavior without changing APIs.
 */

void *check_type(void)
{
	return (void *)&kiocb_write_and_wait;
}

MODULE_LICENSE("GPL");
