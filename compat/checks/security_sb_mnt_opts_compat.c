#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/string_helpers.h>
#include <linux/security.h>

int test_dummy(struct super_block *sb, void *mnt_opts)
{
	security_sb_mnt_opts_compat(sb, mnt_opts);
	return 0;
}

MODULE_LICENSE("GPL");
