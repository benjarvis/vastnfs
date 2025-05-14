#include <linux/module.h>
#include <linux/fs.h>

/*
 * Check for commit 735e4ae5ba28c886d249ad04d3c8cc097dad6336
 *
 * vfs: track per-sb writeback errors and report them to syncfs
 */
errseq_t check_type(struct super_block *s)
{
	return s->s_wb_err;
}

MODULE_LICENSE("GPL");
